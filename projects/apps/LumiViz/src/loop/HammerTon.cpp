/**
 * @file HammerTon.cpp
 * @brief Loop: ein Ton der Hammer-Saite (Stufe 31b; Herleitung des Chors in
 *        docs/audio/Spielweisen_Konzept.md §4.1 nach HT3 a)
 */
#include "loop/HammerTon.hpp"

#include "loop/Eigenzerlegung.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace lumi::loop
{

namespace
{

using Komplex = std::complex<double>;

/// −60 dB: e^(−σ·T60) = 1/1000.
constexpr double kLn1000 = 6.907755278982137;
/// So lange laeuft der Kontakt hoechstens (ein Bass-Hammer liegt rund 4 ms an).
constexpr double kKontaktMaxS = 0.03;
/// So lange muss der Hammer frei sein, bis der Kontakt als vorbei gilt.
constexpr double kFreiS = 0.001;
/// Am Ende blendet der Ton so lange aus.
constexpr double kAusblendenS = 0.005;
/// Die Laengsmoden klingen zehnmal schneller ab als die Quermoden: „the decay time of the
/// longitudinal modes are an order of a magnitude lower than that of the transverse modes“
/// (Bank, Dissertation 2006, S. 80).
constexpr double kLaengsSchneller = 10.0;
/// Die Laengskraft wird gerechnet, bis ihr Spannungsanteil 100 dB unter seiner Spitze liegt
/// (sie ist quadratisch in der Auslenkung und klingt doppelt so schnell ab) — hoechstens so lange.
constexpr double kLaengsMaxS = 10.0;
/// Der Hochpass auf der Laengskraft: die Spannung traegt einen Gleichanteil, den sonst erst der
/// Resonanzboden (31c) wegnaehme (von Claude gesetzt).
constexpr double kLaengsHochpassHz = 20.0;

/// Die Werte der Kurven an einer Taste, in SI.
struct Werte
{
    double f0 = 0.0;           ///< Grundfrequenz der Saite ohne Steife (der erste Teilton liegt bei f0·√(1+B))
    double b = 0.0;
    double sigmaFrueh1 = 0.0;  ///< erste Stufe am ersten Teilton, 1/s
    double spaetJeFrueh = 1.0; ///< σ_spät/σ_früh, ueber die Teiltoene fest
    double hoch = 0.0;         ///< σ_k = σ_1·(1 + hoch·((f_k/f_1)² − 1))
    int chor = 1;
    double cent = 0.0;
    double hammerKg = 0.01;
    double q0 = 1.0;           ///< N/mm^p
    double p = 2.5;
    double stelle = 0.12;
    double sigmaDaempfer = 0.0;
    double laenge = 1.0;       ///< m
    double mu = 0.005;         ///< kg/m
    double t0 = 700.0;         ///< N
    double es = 0.0;           ///< N — aus f_l0 = (1/2L)·√(ES/µ)
    double laengsF0 = 0.0;     ///< Hz, die Grundfrequenz der Laengsschwingung
    double winkel = 0.0;       ///< rad, der Winkel der Saite am Steg (HT4 a)
    double gamma = 0.0;        ///< rad, die erste Eigenrichtung gegen die Senkrechte (P2 a)
    double deltaCent = 0.0;    ///< so viel hoeher liegt die zweite Eigenrichtung
    double verhaeltnisH = 0.0; ///< so viel nimmt der Steg waagrecht im Verhaeltnis zu senkrecht (P3-1 a)
};

Werte werteAn(const HammerSaite& s, const HammerAnschlag& a)
{
    const int t = std::clamp(a.taste, kTasteMin, kTasteMax);
    const auto log = [t](const Tastenkurve& k) { return wertAn(k, t, KurvenSkala::Logarithmisch); };
    const auto lin = [t](const Tastenkurve& k) { return wertAn(k, t, KurvenSkala::Linear); };
    Werte w;
    w.b = log(s.inharmonizitaet);
    w.f0 = a.hz / std::sqrt(1.0 + w.b);  // der erste Teilton liegt auf der Tonhoehe
    const double frueh = log(s.abklingS);
    const double spaet = std::max(frueh, log(s.abklingSpaetS));
    w.sigmaFrueh1 = kLn1000 / frueh;
    w.spaetJeFrueh = frueh / spaet;
    w.hoch = (1.0 / log(s.abklingHoch) - 1.0) / 99.0;
    w.chor = chorAn(s, t);
    w.cent = lin(s.verstimmungCent);
    w.hammerKg = lin(s.hammerMasseG) * 1e-3;
    w.q0 = log(s.hammerSteife);
    w.p = lin(s.hammerExponent);
    w.stelle = lin(s.anschlagStelle);
    w.sigmaDaempfer = kLn1000 / log(s.daempferS);
    w.laenge = log(s.saitenLaengeM);
    w.mu = log(s.saitenMasseGm) * 1e-3;
    w.t0 = 4.0 * w.laenge * w.laenge * w.f0 * w.f0 * w.mu;  // f0 = (1/2L)·√(T0/µ)
    w.laengsF0 = lin(s.laengsVerhaeltnis) * w.f0;
    w.es = w.mu * std::pow(2.0 * w.laenge * w.laengsF0, 2.0);
    w.winkel = lin(s.stegWinkelGrad) * std::numbers::pi / 180.0;
    w.gamma = lin(s.polarisationWinkelGrad) * std::numbers::pi / 180.0;
    w.deltaCent = lin(s.polarisationCent);
    w.verhaeltnisH = std::clamp(lin(s.waagrechtVerhaeltnis), 0.0, 1.0);
    return w;
}

/// Die Verstimmung der Saiten im Chor in Cent: bei zwei ±c/2, bei drei −c/2, 0, +c/2.
std::vector<double> chorCent(int n, double c)
{
    if (n <= 1) return {0.0};
    if (n == 2) return {-c / 2.0, c / 2.0};
    return {-c / 2.0, 0.0, c / 2.0};
}

/// Eine Mode beim Rechnen: ihr Zustand ist komplex, die Auslenkung sein Imaginaerteil.
struct Mode
{
    Komplex zustand;
    Komplex eingang;   ///< (b0/ω)·g·sin(kπx_H): so viel bringt eine Kraft von 1 N·s
    Komplex pol;       ///< e^(λΔt)
    Komplex polFein;   ///< e^(λΔt/U), waehrend des Kontakts
    Komplex polDaempfer;
    double abnahme = 0.0;  ///< sin(kπx_H)/N: die Auslenkung am Hammer
    double steg = 0.0;     ///< kπT0/L: die Kraft am Steg
    int teilton = 1;
    double hz = 0.0;       ///< die Frequenz des Teiltons
    /// Je Stelle 2j + a (Saite j, Eigenrichtung a) die Auslenkung je Einheit des Zustands — nur die von null
    /// verschiedenen (`EbenenMode::form`; fuer die Laengsschwingung, P5 a).
    std::vector<std::pair<int, Komplex>> form;
};

/// Eine Laengsmode (Bank 2010, Gl. 20, 27, 28): nur ihre dynamische Antwort zaehlt — die volle
/// minus die statische, ein Hochpass zweiter Ordnung.
struct LaengsMode
{
    Komplex zustand;
    Komplex pol;
    double eingang = 0.0;   ///< 2/(Lµω): die volle Antwort je N·s
    double statisch = 0.0;  ///< 2/(Lµω²): die Antwort unter der Resonanz
    double steg = 0.0;      ///< N·ES·kπ/L: die Kraft am Steg aus allen Saiten
    bool gerade = false;    ///< gerade Moden nehmen die Kraft von Mode 4, ungerade die von Mode 3
    double vorige = 0.0;    ///< die Kraft des vorigen Schritts (die volle Antwort kommt einen Takt spaeter)
};

/// Die Laengskraft am Steg (Bank 2010, Abschnitt V): die gleichmaessige Spannung (Gl. 26) und
/// die dynamische Antwort der Laengsmoden (Gl. 27, 28), angeregt von Paaren der Quermoden
/// (Gl. 22b) — eine Kraft fuer die ungeraden (wie Mode 3), eine fuer die geraden (wie Mode 4).
/// Je Saite aus ihren beiden Ebenen (Bank 2006, Gl. 2.9–2.10: (∂y/∂x)² + (∂z/∂x)²; P5 a), die
/// Auslenkungen aus den Eigenvektoren der Moden; die Laengsmoden aller Saiten sind gleich, ihre
/// Antwort am Steg ist die auf die Summe der Kraefte.
class Laengs
{
public:
    Laengs(const Werte& w, const std::vector<Mode>& moden, double fs)
    {
        if (w.es <= 0.0 || w.laengsF0 <= 0.0) return;
        const double pi = std::numbers::pi;
        const double l2 = w.laenge * w.laenge;
        m_stellen = 2 * w.chor;
        m_spannung = w.es * pi * pi / (4.0 * l2);                   // Gl. 26, je Saite
        m_anregung = -2.0 * w.es * std::pow(pi, 3.0) / (8.0 * l2);  // Gl. 22b
        // Quermoden in die Produkte nur bis fs/4 (Bank 2010, S. 8: kein Aliasing)
        for (const Mode& m : moden)
            if (m.hz < 0.25 * fs) m_teiltoene = std::max(m_teiltoene, m.teilton);
        m_x.assign((static_cast<std::size_t>(m_teiltoene) + 5) * static_cast<std::size_t>(m_stellen), 0.0);
        // nur die Stellen, die eine Mode traegt (mit γ = 0 die senkrechten)
        std::vector<bool> getragen(static_cast<std::size_t>(m_stellen), false);
        for (const Mode& m : moden)
            for (const auto& [stelle, f] : m.form) getragen[static_cast<std::size_t>(stelle)] = true;
        for (int i = 0; i < m_stellen; ++i)
            if (getragen[static_cast<std::size_t>(i)]) m_getragen.push_back(static_cast<std::size_t>(i));
        const double sigma = w.sigmaFrueh1 * kLaengsSchneller;
        for (int k = 1;; ++k)
        {
            const double fk = k * w.laengsF0;
            if (fk >= kHammerTeiltonGrenze * fs) break;
            const double omega = 2.0 * pi * fk;
            LaengsMode lm;
            lm.pol = std::exp(Komplex(-sigma, omega) / fs);
            lm.eingang = 2.0 / (w.laenge * w.mu * omega);
            lm.statisch = 2.0 / (w.laenge * w.mu * omega * omega);
            lm.steg = w.es * k * pi / w.laenge;  // je Saite; angeregt von der Summe ueber die Saiten
            lm.gerade = k % 2 == 0;
            m_moden.push_back(lm);
        }
        m_gewicht = std::sin(w.winkel);  // am Steg wirkt sie mit sin α (HT4 a)
        m_an = m_teiltoene > 0 && m_gewicht > 0.0;
        const double rc = 1.0 / (2.0 * pi * kLaengsHochpassHz);
        m_hochpass = rc / (rc + 1.0 / fs);
    }

    [[nodiscard]] bool an() const { return m_an; }

    /// Einen Schritt: die Laengskraft am Steg aus den Zustaenden der Quermoden.
    double schritt(const std::vector<Mode>& moden, double dt)
    {
        std::fill(m_x.begin(), m_x.end(), 0.0);
        const auto stellen = static_cast<std::size_t>(m_stellen);
        for (const Mode& m : moden)
        {
            if (m.teilton > m_teiltoene) continue;
            const std::size_t ab = static_cast<std::size_t>(m.teilton) * stellen;
            // Im(f·z), ausgeschrieben
            for (const auto& [stelle, f] : m.form)
                m_x[ab + static_cast<std::size_t>(stelle)] += f.real() * m.zustand.imag() + f.imag() * m.zustand.real();
        }
        // je Saite und Ebene; die Summen sind gegen die Drehung γ unveraendert (beide Teiltoene einer Taste)
        double quadrate = 0.0;
        double ungerade = 0.0;
        double gerade = 0.0;
        for (int n = 1; n <= m_teiltoene; ++n)
            for (const std::size_t i : m_getragen)
            {
                const double xn = m_x[static_cast<std::size_t>(n) * stellen + i];
                quadrate += static_cast<double>(n) * n * xn * xn;
                if (n + 3 <= m_teiltoene) ungerade += n * (n + 3.0) * xn * m_x[static_cast<std::size_t>(n + 3) * stellen + i];
                if (n + 4 <= m_teiltoene) gerade += n * (n + 4.0) * xn * m_x[static_cast<std::size_t>(n + 4) * stellen + i];
            }
        const double spannung = m_spannung * quadrate;
        const double kraftUngerade = m_anregung * 3.0 * ungerade;
        const double kraftGerade = m_anregung * 4.0 * gerade;
        double dynamisch = 0.0;
        for (LaengsMode& lm : m_moden)
        {
            const double kraft = lm.gerade ? kraftGerade : kraftUngerade;
            dynamisch += lm.steg * (lm.zustand.imag() - lm.statisch * lm.vorige);
            lm.zustand = lm.pol * (lm.zustand + Komplex(0.0, lm.eingang * kraft * dt));
            lm.vorige = kraft;
        }
        m_spitze = std::max(m_spitze, spannung);
        m_letzte = spannung;
        // der Gleichanteil der Spannung strahlt nicht ab: ein Hochpass erster Ordnung
        const double eingang = spannung + dynamisch;
        m_aus = m_hochpass * (m_aus + eingang - m_vorEingang);
        m_vorEingang = eingang;
        return m_aus * m_gewicht;
    }

    /// Liegt die Spannung so weit unter ihrer Spitze, dass die Laengskraft nicht mehr zaehlt?
    [[nodiscard]] bool verklungen() const { return m_spitze > 0.0 && m_letzte < 1e-5 * m_spitze; }

private:
    bool m_an = false;
    int m_stellen = 2;  ///< 2N: Saite j, Eigenrichtung a an 2j + a
    int m_teiltoene = 0;
    double m_spannung = 0.0;
    double m_anregung = 0.0;
    std::vector<double> m_x;  ///< je Teilton n und Stelle die Auslenkung, an n·2N + Stelle
    std::vector<std::size_t> m_getragen;  ///< die Stellen, die eine Mode traegt
    std::vector<LaengsMode> m_moden;
    double m_spitze = 0.0;
    double m_letzte = 0.0;
    double m_hochpass = 0.0;
    double m_aus = 0.0;
    double m_vorEingang = 0.0;
    double m_gewicht = 0.0;
};

}  // namespace

std::vector<ChorMode> chorModen(const std::vector<double>& omega, double sigmaSaite, double sigmaSteg)
{
    const auto n = static_cast<int>(omega.size());
    if (n == 0) return {};
    double mitte = 0.0;
    for (const double o : omega) mitte += o;
    mitte /= n;
    const auto [klein, gross] = std::minmax_element(omega.begin(), omega.end());
    if (n == 1 || *gross - *klein <= 1e-12 * std::max(1.0, mitte))
    {
        // ohne Verstimmung bleibt die Gleichlage allein: die Gegenlagen bekommen nichts
        return {ChorMode{Komplex(-sigmaSaite - n * sigmaSteg, mitte), Komplex(1.0, 0.0)}};
    }
    // A = D − σ_b·J: die Eigenwerte sind die Nullstellen von R(z) = Q(z) + σ_b·Q'(z),
    // Q(z) = Π (z − d_j), d_j = i(ω_j − ω̄) − σ_s (eine Stoerung vom Rang eins)
    std::vector<Komplex> d(n);
    for (int j = 0; j < n; ++j) d[j] = Komplex(-sigmaSaite, omega[j] - mitte);
    const auto r = [&](Komplex z) {
        Komplex q(1.0, 0.0);
        Komplex dq(0.0, 0.0);
        for (const Komplex& dj : d)
        {
            dq = dq * (z - dj) + q;
            q *= z - dj;
        }
        return q + sigmaSteg * dq;
    };
    // Durand–Kerner, begonnen an den Diagonalwerten (leicht versetzt, damit sie verschieden sind)
    std::vector<Komplex> z(n);
    for (int j = 0; j < n; ++j) z[j] = d[j] - sigmaSteg + Komplex(0.0, 1e-3 * (j + 1) * (*gross - *klein));
    for (int runde = 0; runde < 200; ++runde)
    {
        double schritt = 0.0;
        for (int j = 0; j < n; ++j)
        {
            Komplex nenner(1.0, 0.0);
            for (int l = 0; l < n; ++l)
                if (l != j) nenner *= z[j] - z[l];
            const Komplex delta = r(z[j]) / nenner;
            z[j] -= delta;
            schritt = std::max(schritt, std::abs(delta));
        }
        if (schritt < 1e-13 * std::max(1.0, sigmaSteg + (*gross - *klein))) break;
    }
    std::vector<ChorMode> moden;
    moden.reserve(n);
    for (const Komplex& wert : z)
    {
        // Eigenvektor v = (D − λ)⁻¹·1; A ist komplex symmetrisch, der linke ist vᵀ:
        // g = (1ᵀv)² / (N·vᵀv)
        Komplex summe(0.0, 0.0);
        Komplex quadrate(0.0, 0.0);
        for (const Komplex& dj : d)
        {
            const Komplex v = 1.0 / (dj - wert);
            summe += v;
            quadrate += v * v;
        }
        moden.push_back(ChorMode{wert + Komplex(0.0, mitte), summe * summe / (static_cast<double>(n) * quadrate)});
    }
    return moden;
}

std::vector<EbenenMode> ebenenModen(const std::vector<double>& omega, double deltaCent, double gamma, double sigmaSaite,
                                    double sigmaV, double sigmaH)
{
    const auto n = static_cast<int>(omega.size());
    if (n == 0) return {};
    double mitte = 0.0;
    for (const double o : omega) mitte += o;
    mitte /= n;
    const double hoeher = std::exp2(deltaCent / 1200.0);
    // Stelle 2j + a: Saite j, Eigenrichtung a; um die Mitte verschoben (wie `chorModen`)
    KleineMatrix a(2 * n);
    std::vector<double> senk(static_cast<std::size_t>(2 * n));
    std::vector<double> waag(static_cast<std::size_t>(2 * n));
    for (int j = 0; j < n; ++j)
    {
        const double o = omega[static_cast<std::size_t>(j)];
        a(2 * j, 2 * j) = Komplex(-sigmaSaite, o - mitte);
        a(2 * j + 1, 2 * j + 1) = Komplex(-sigmaSaite, o * hoeher - mitte);
        senk[static_cast<std::size_t>(2 * j)] = std::cos(gamma);
        senk[static_cast<std::size_t>(2 * j + 1)] = -std::sin(gamma);
        waag[static_cast<std::size_t>(2 * j)] = std::sin(gamma);
        waag[static_cast<std::size_t>(2 * j + 1)] = std::cos(gamma);
    }
    // der Steg nimmt auf die Summe: senkrecht σ_V, waagrecht σ_H (P2 a, ohne Kreuzterm)
    for (int z = 0; z < 2 * n; ++z)
        for (int s = 0; s < 2 * n; ++s)
            a(z, s) -= sigmaV * senk[static_cast<std::size_t>(z)] * senk[static_cast<std::size_t>(s)] +
                       sigmaH * waag[static_cast<std::size_t>(z)] * waag[static_cast<std::size_t>(s)];
    const Eigenzerlegung zer = zerlegeKlein(a);
    // der Hammer stoesst senkrecht mit F/N, am Steg zaehlt die Summe der senkrechten Anteile
    std::vector<EbenenMode> moden;
    double groesster = 0.0;
    for (int m = 0; m < 2 * n; ++m)
    {
        Komplex aus(0.0, 0.0);
        Komplex ein(0.0, 0.0);
        for (int i = 0; i < 2 * n; ++i)
        {
            aus += senk[static_cast<std::size_t>(i)] * zer.rechts(i, m);
            ein += zer.links(m, i) * senk[static_cast<std::size_t>(i)];
        }
        EbenenMode mode;
        mode.lambda = zer.werte[static_cast<std::size_t>(m)] + Komplex(0.0, mitte);
        mode.anteil = aus * ein / static_cast<double>(n);
        if (std::abs(aus) > 0.0)
            for (int i = 0; i < 2 * n; ++i) mode.form.push_back(zer.rechts(i, m) / aus);
        groesster = std::max(groesster, std::abs(mode.anteil));
        moden.push_back(std::move(mode));
    }
    std::erase_if(moden, [&](const EbenenMode& m) { return std::abs(m.anteil) < kEbenenAnteilMin * groesster; });
    return moden;
}

std::int64_t hammerNachklang(const HammerSaite& saite, const HammerAnschlag& anschlag, std::int64_t maxFrames)
{
    const Werte w = werteAn(saite, anschlag);
    const double sigma = anschlag.gedaempft ? std::max(w.sigmaDaempfer, w.sigmaFrueh1 * w.spaetJeFrueh)
                                            : w.sigmaFrueh1 * w.spaetJeFrueh;
    const auto frames = static_cast<std::int64_t>(std::ceil(kLn1000 / sigma * anschlag.rate));
    return std::clamp<std::int64_t>(frames, 1, std::max<std::int64_t>(1, maxFrames));
}

void rechneHammerTon(const HammerSaite& saite, const HammerAnschlag& anschlag, std::vector<float>& mono, std::int64_t frames,
                     HammerMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || anschlag.rate <= 0) return;
    const Werte w = werteAn(saite, anschlag);
    const double fs = anschlag.rate;
    const double dt = 1.0 / fs;
    const double dtFein = dt / kHammerUeberabtastung;
    const double pi = std::numbers::pi;

    // --- die Moden: je Teilton der Chor (HT3 a) ---
    std::vector<Mode> moden;
    const double b0 = 2.0 / (w.laenge * w.mu);  // Bank 2010, Gl. 4c
    const double f1 = w.f0 * std::sqrt(1.0 + w.b);
    const std::vector<double> cents = chorCent(w.chor, w.cent);
    for (int k = 1; k <= kHammerTeiltoeneMax; ++k)
    {
        const double fk = k * w.f0 * std::sqrt(1.0 + w.b * k * k);
        if (fk >= kHammerTeiltonGrenze * fs) break;
        const double omegaK = 2.0 * pi * fk;
        const double verh = fk / f1;
        const double sigmaFrueh = w.sigmaFrueh1 * std::max(1.0, 1.0 + w.hoch * (verh * verh - 1.0));
        const double sigmaSpaet = sigmaFrueh * w.spaetJeFrueh;
        // P3-1 a: die zweite Stufe ist der innere Verlust, die erste dazu der senkrechte am Steg; waagrecht
        // nimmt der Steg im Verhaeltnis h
        const double sigmaSaite = sigmaSpaet;
        const double sigmaV = (sigmaFrueh - sigmaSaite) / w.chor;
        const double sigmaH = w.verhaeltnisH * sigmaV;
        std::vector<double> omega;
        for (const double c : cents) omega.push_back(omegaK * std::exp2(c / 1200.0));
        const double sinus = std::sin(k * pi * w.stelle);
        for (const EbenenMode& m : ebenenModen(omega, w.deltaCent, w.gamma, sigmaSaite, sigmaV, sigmaH))
        {
            Mode mode;
            mode.eingang = (b0 / omegaK) * m.anteil * sinus;
            mode.pol = std::exp(m.lambda * dt);
            mode.polFein = std::exp(m.lambda * dtFein);
            // der Daempfer verkuerzt die Abklingzeit (Bank 2010, S. 12), nie verlaengert er sie
            const Komplex gedaempft(std::min(m.lambda.real(), -w.sigmaDaempfer), m.lambda.imag());
            mode.polDaempfer = std::exp(gedaempft * dt);
            mode.abnahme = sinus / w.chor;
            // Gl. 16; am Steg wirkt die Querkraft mit cos α (HT4 a)
            mode.steg = k * pi * w.t0 / w.laenge * std::cos(w.winkel);
            mode.teilton = k;
            mode.hz = fk;
            // nur, was zaehlt: mit γ = 0 bleibt die waagrechte Seite einer senkrechten Mode beim Zerlegen
            // nur im Rauschen (um 10⁻¹⁷) von null verschieden
            double groesste = 0.0;
            for (const Komplex& f : m.form) groesste = std::max(groesste, std::abs(f));
            for (std::size_t i = 0; i < m.form.size(); ++i)
                if (m.form[i] != Komplex(0.0, 0.0) && std::abs(m.form[i]) >= 1e-12 * groesste)
                    mode.form.emplace_back(static_cast<int>(i), m.form[i]);
            moden.push_back(std::move(mode));
        }
    }
    std::vector<double> summe(mono.size(), 0.0);
    Laengs laengs(w, moden, fs);

    // --- der Kontakt: Hammer und Saite, achtfach feiner ---
    const double tempo = hammerTempo(anschlag.stufe);
    double hammerY = 0.0;  // m, zur Saite hin positiv
    double hammerV = tempo;
    const auto kontaktMax = std::min<std::int64_t>(frames, std::llround(kKontaktMaxS * fs));
    const auto freiNoetig = std::max<std::int64_t>(1, std::llround(kFreiS * fs * kHammerUeberabtastung));
    std::int64_t frei = 0;
    bool beruehrt = false;
    double erster = 0.0;
    std::int64_t n = 0;
    if (messung != nullptr) *messung = HammerMessung{};
    // die groesste Auslenkung einer Saite an neun Stellen (nur mit `messung`)
    const auto auslenkung = [&]() {
        if (messung == nullptr) return;
        for (int stelle = 1; stelle <= 9; ++stelle)
        {
            double y = 0.0;
            for (const Mode& m : moden) y += std::sin(m.teilton * pi * stelle / 10.0) * m.zustand.imag();
            messung->auslenkungSpitzeM = std::max(messung->auslenkungSpitzeM, std::abs(y) / w.chor);
        }
    };
    const auto messEnde = std::llround(0.03 * fs);
    for (; n < kontaktMax; ++n)
    {
        double aus = 0.0;
        for (const Mode& m : moden) aus += m.steg * m.zustand.imag();
        if (laengs.an()) aus += laengs.schritt(moden, dt);
        summe[static_cast<std::size_t>(n)] = aus;
        if (n < messEnde) auslenkung();
        for (int u = 0; u < kHammerUeberabtastung; ++u)
        {
            double saiteY = 0.0;
            for (const Mode& m : moden) saiteY += m.abnahme * m.zustand.imag();
            const double druck = (hammerY - saiteY) * 1000.0;  // mm
            const double kraft = druck > 0.0 ? w.q0 * std::pow(druck, w.p) : 0.0;
            // die Moden sehen die Kraft erst im naechsten Schritt (Impulsinvarianz, Gl. 10)
            for (Mode& m : moden) m.zustand = m.polFein * (m.zustand + m.eingang * (kraft * dtFein));
            hammerV -= kraft / w.hammerKg * dtFein;
            hammerY += hammerV * dtFein;
            if (kraft > 0.0)
            {
                if (messung != nullptr)
                {
                    const double t = (static_cast<double>(n) * kHammerUeberabtastung + u) * dtFein;
                    if (!beruehrt) erster = t;
                    messung->kontaktS = t - erster;
                    messung->kraftSpitzeN = std::max(messung->kraftSpitzeN, kraft);
                }
                beruehrt = true;
                frei = 0;
            }
            else if (beruehrt && hammerV < 0.0)
            {
                ++frei;
            }
        }
        if (beruehrt && frei >= freiNoetig)
        {
            ++n;
            break;
        }
    }

    // --- frei mit Laengskraft: Takt fuer Takt ueber alle Moden, bis die Spannung verklungen ist ---
    if (laengs.an())
    {
        const auto laengsEnde = std::min<std::int64_t>(frames, std::llround(kLaengsMaxS * fs));
        for (; n < laengsEnde && !laengs.verklungen(); ++n)
        {
            const bool daempfer = anschlag.gedaempft && n >= anschlag.halten;
            if (n < messEnde) auslenkung();
            double aus = laengs.schritt(moden, dt);
            for (Mode& m : moden)
            {
                aus += m.steg * m.zustand.imag();
                m.zustand *= daempfer ? m.polDaempfer : m.pol;
            }
            summe[static_cast<std::size_t>(n)] = aus;
        }
    }

    // --- frei: je Mode ein abklingender Zeiger; beim Loslassen greift der Daempfer ---
    double groesste = 0.0;
    for (const Mode& m : moden) groesste += std::abs(m.steg * m.zustand);
    const double schwelle = 1e-7 * groesste;
    for (const Mode& m : moden)
    {
        Komplex z = m.zustand;
        for (std::int64_t i = n; i < frames; ++i)
        {
            const bool daempfer = anschlag.gedaempft && i >= anschlag.halten;
            summe[static_cast<std::size_t>(i)] += m.steg * z.imag();
            z *= daempfer ? m.polDaempfer : m.pol;
            if ((i & 63) == 0 && std::abs(m.steg * z) < schwelle) break;
        }
    }

    // in Einheiten von kHammerKraftBezug; am Ende ausgeblendet, damit der Klang bei null endet
    const auto blende = std::min<std::int64_t>(frames, std::max<std::int64_t>(1, std::llround(kAusblendenS * fs)));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double wert = summe[static_cast<std::size_t>(i)] / kHammerKraftBezug;
        const std::int64_t rest = frames - 1 - i;
        if (rest < blende) wert *= static_cast<double>(rest) / static_cast<double>(blende);
        mono[static_cast<std::size_t>(i)] = static_cast<float>(wert);
    }
}

}  // namespace lumi::loop
