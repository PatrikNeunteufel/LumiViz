/**
 * @file HammerTon.cpp
 * @brief Loop: ein Ton der Hammer-Saite (Stufe 31b; Herleitung des Chors in
 *        docs/audio/Spielweisen_Konzept.md §4.1 nach HT3 a)
 */
#include "loop/HammerTon.hpp"

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

std::int64_t hammerNachklang(const HammerSaite& saite, const HammerAnschlag& anschlag, std::int64_t maxFrames)
{
    const Werte w = werteAn(saite, anschlag);
    const double sigma = anschlag.gedaempft ? std::max(w.sigmaDaempfer, w.sigmaFrueh1 * w.spaetJeFrueh)
                                            : w.sigmaFrueh1 * w.spaetJeFrueh;
    const auto frames = static_cast<std::int64_t>(std::ceil(kLn1000 / sigma * anschlag.rate));
    return std::clamp<std::int64_t>(frames, 1, std::max<std::int64_t>(1, maxFrames));
}

void rechneHammerTon(const HammerSaite& saite, const HammerAnschlag& anschlag, std::vector<float>& mono, std::int64_t frames)
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
        const double sigmaSteg = (sigmaFrueh - sigmaSpaet) / w.chor;
        std::vector<double> omega;
        for (const double c : cents) omega.push_back(omegaK * std::exp2(c / 1200.0));
        const double sinus = std::sin(k * pi * w.stelle);
        for (const ChorMode& m : chorModen(omega, sigmaSpaet, sigmaSteg))
        {
            Mode mode;
            mode.eingang = (b0 / omegaK) * m.anteil * sinus;
            mode.pol = std::exp(m.lambda * dt);
            mode.polFein = std::exp(m.lambda * dtFein);
            // der Daempfer verkuerzt die Abklingzeit (Bank 2010, S. 12), nie verlaengert er sie
            const Komplex gedaempft(std::min(m.lambda.real(), -w.sigmaDaempfer), m.lambda.imag());
            mode.polDaempfer = std::exp(gedaempft * dt);
            mode.abnahme = sinus / w.chor;
            mode.steg = k * pi * w.t0 / w.laenge;  // Gl. 16
            moden.push_back(mode);
        }
    }
    std::vector<double> summe(mono.size(), 0.0);

    // --- der Kontakt: Hammer und Saite, achtfach feiner ---
    const double tempo = hammerTempo(anschlag.stufe);
    double hammerY = 0.0;  // m, zur Saite hin positiv
    double hammerV = tempo;
    const auto kontaktMax = std::min<std::int64_t>(frames, std::llround(kKontaktMaxS * fs));
    const auto freiNoetig = std::max<std::int64_t>(1, std::llround(kFreiS * fs * kHammerUeberabtastung));
    std::int64_t frei = 0;
    bool beruehrt = false;
    std::int64_t n = 0;
    for (; n < kontaktMax; ++n)
    {
        double aus = 0.0;
        for (const Mode& m : moden) aus += m.steg * m.zustand.imag();
        summe[static_cast<std::size_t>(n)] = aus;
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
