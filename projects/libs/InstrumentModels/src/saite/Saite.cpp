/**
 * @file Saite.cpp
 * @brief Klasse 1 (Rest): der Saitenkern fuer Clavinet, Hackbrett, Cimbalom (Herleitung im Kopf von `Saite.hpp`)
 */
#include "InstrumentModels/saite/Saite.hpp"

#include "InstrumentModels/hammer/HammerTon.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace lumi::kern::saite
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr double kTeiltonGrenze = 0.45;
constexpr int kTeiltoeneMax = 200;
constexpr double kAusblendenS = 0.005;
/// Zwei Teiltoene beider Abschnitte koppeln am Steg, wenn sie auf 0,5 % gleich liegen (Schwelle, gesetzt — L4).
constexpr double kStegFenster = 0.005;
/// Stahl (Lehrbuch): μ = ρ·π·d²/4 aus dem Durchmesser (CL1 a, S183).
constexpr double kStahlDichte = 7850.0;

double lin(const Kurve& k, int n)
{
    return wertAn(k, n, Skala::Linear);
}

double logw(const Kurve& k, int n)
{
    return wertAn(k, n, Skala::Logarithmisch);
}

int chorAn(const SaitenSpiel& s, int n)
{
    return std::clamp(static_cast<int>(std::lround(lin(s.chor, n))), 1, 5);
}

std::vector<double> chorCent(int n, double c)
{
    std::vector<double> v;
    for (int j = 0; j < n; ++j) v.push_back(n == 1 ? 0.0 : -c / 2.0 + c * j / (n - 1));
    return v;
}

/// Eine Mode der Saite beim Rechnen.
struct SMode
{
    Mode m;
    int teilton = 1;
    double hz = 0.0;
    double sigmaFrueh = 0.0;  ///< σ der ersten Stufe dieses Teiltons (Massstab der Kopplung am Mittelsteg)
    double tangente = 0.0; ///< Antrieb durch das bewegte Ende: g/ω·(−2/(kπ)) je ḧ
    int partner = -1;      ///< Index der Mode des anderen Abschnitts
    double gewicht = 0.0;  ///< |Anteil| (fuer die Wahl der staerksten je Teilton)
};

struct Werte
{
    double f0 = 0.0, b = 0.0, sigma1 = 0.0, spaetJeFrueh = 1.0, hoch = 0.0, cent = 0.0, gamma = 0.0, delta = 0.0, h = 0.0;
    double laenge = 1.0, mu = 0.001, t0 = 100.0, stelle = 0.1;
    double dMm = 1.0;  ///< Durchmesser in mm — das Gewicht des magnetischen Tonabnehmers ist d² (CL1 a)
    int chor = 1;
};

Werte werteAn(const SaitenSpiel& s, const SaitenAnschlag& a)
{
    const int n = std::clamp(a.note, s.noteMin, s.noteMax);
    Werte w;
    w.b = logw(s.inharmonizitaet, n);
    const double druck = std::clamp(a.tastendruckCent, 0.0, std::max(0.0, s.tastendruckGrenzeCent));
    const double hz = a.hz * std::exp2(druck / 1200.0);
    w.f0 = hz / std::sqrt(1.0 + w.b);
    const double frueh = logw(s.t60S, n);
    const double spaet = std::max(frueh, logw(s.t60SpaetS, n));
    w.sigma1 = kLn1000 / frueh;
    w.spaetJeFrueh = frueh / spaet;
    w.hoch = (1.0 / logw(s.abklingHoch, n) - 1.0) / 99.0;
    w.chor = chorAn(s, n);
    w.cent = lin(s.verstimmungCent, n);
    w.gamma = lin(s.polarisationWinkelGrad, n) * kPi / 180.0;
    w.delta = lin(s.polarisationCent, n);
    w.h = std::clamp(lin(s.waagrechtVerhaeltnis, n), 0.0, 1.0);
    w.laenge = logw(s.laengeM, n);
    w.dMm = logw(s.durchmesserMm, n);
    w.mu = kStahlDichte * kPi * (w.dMm * 1e-3) * (w.dMm * 1e-3) / 4.0;  // CL1 a (S183): aus dem Durchmesser
    w.t0 = 4.0 * w.laenge * w.laenge * w.f0 * w.f0 * w.mu;
    w.stelle = std::clamp(lin(s.anschlagStelle, n), 0.0, 1.0);
    return w;
}

/// Die Moden eines Abschnitts mit Grundfrequenz f0 (Laenge L) — wie `hammerTeiltonModen` im Vorbild.
std::vector<SMode> abschnitt(const Werte& w, double f0, double laenge, double stelle, double fs, const std::vector<double>& abnehmer,
                             bool magnetisch, double sigmaZusatz, const std::vector<double>& gewichte = {})
{
    std::vector<SMode> moden;
    const double f1 = f0 * std::sqrt(1.0 + w.b);
    const double b0 = 2.0 / (laenge * w.mu);
    const double t0 = 4.0 * laenge * laenge * f0 * f0 * w.mu;
    for (int k = 1; k <= kTeiltoeneMax; ++k)
    {
        const double fk = k * f0 * std::sqrt(1.0 + w.b * k * k);
        if (fk >= kTeiltonGrenze * fs) break;
        const double omegaK = 2.0 * kPi * fk;
        const double verh = fk / f1;
        const double sigmaFrueh = w.sigma1 * std::max(1.0, 1.0 + w.hoch * (verh * verh - 1.0)) + sigmaZusatz;
        const double sigmaSaite = sigmaFrueh * w.spaetJeFrueh;
        const double sigmaV = (sigmaFrueh - sigmaSaite) / w.chor;
        std::vector<double> omega;
        for (const double c : chorCent(w.chor, w.cent)) omega.push_back(omegaK * std::exp2(c / 1200.0));
        const double sinus = std::sin(k * kPi * stelle);
        for (const hammer::EbenenMode& e : hammer::ebenenModen(omega, w.delta, w.gamma, sigmaSaite, sigmaV, w.h * sigmaV))
        {
            SMode m;
            m.teilton = k;
            m.hz = fk;
            m.sigmaFrueh = sigmaFrueh;
            m.gewicht = std::abs(e.anteil);
            m.m.lambda = e.lambda;
            m.m.eingang = (b0 / omegaK) * e.anteil * sinus;
            m.m.abnahme = sinus / w.chor;
            m.tangente = (std::abs(e.anteil) > 0.0 ? (e.anteil / omegaK).real() : 0.0) * (-2.0 / (k * kPi));
            if (magnetisch)
            {
                double summe = 0.0;
                for (std::size_t i = 0; i < abnehmer.size(); ++i)  // x vom Steg aus
                    summe += (i < gewichte.size() ? gewichte[i] : 1.0) * std::sin(k * kPi * (1.0 - abnehmer[i]));
                // Geschwindigkeit an den Tonabnehmern, gewichtet mit d² (CL1 a, S183: der Tonabnehmer sieht das Volumen der Saite)
                m.m.aus = e.lambda * e.anteil * summe * (w.dMm * w.dMm);
            }
            else
                m.m.aus = Komplex(k * kPi * t0 / laenge, 0.0);  // Kraft am Steg (Bank 2010 Gl. 16)
            moden.push_back(m);
        }
    }
    return moden;
}

}  // namespace

std::int64_t saitenNachklang(const SaitenSpiel& s, const SaitenAnschlag& a, std::int64_t maxFrames)
{
    const int n = std::clamp(a.note, s.noteMin, s.noteMax);
    const Werte w = werteAn(s, a);
    double sigma = w.sigma1 * w.spaetJeFrueh;
    if (s.ganzJeKlingend > 1.0) sigma = std::max(sigma, kLn1000 / logw(s.garnT60S, n));
    if (s.daempfer && a.gedaempft) sigma = std::max(sigma, kLn1000 / logw(s.daempferT60S, n));
    return std::clamp<std::int64_t>(static_cast<std::int64_t>(std::ceil(kLn1000 / sigma * a.rate)), 1, std::max<std::int64_t>(1, maxFrames));
}

void rechneSaitenTon(const SaitenSpiel& s, const SaitenAnschlag& a, std::vector<float>& mono, std::int64_t frames, SaitenMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0) return;
    const int note = std::clamp(a.note, s.noteMin, s.noteMax);
    const double fs = a.rate;
    const double dt = 1.0 / fs;
    const Werte w = werteAn(s, a);
    const bool magnetisch = s.abnahme == Abnahme::Magnetisch;
    const double schieber = std::clamp(a.schieber, 0.0, 1.0) * kLn1000 / logw(s.schieberT60S, note);
    // der angeschlagene Abschnitt
    const double teilung = s.stegTeilung > 0.0 ? s.stegTeilung : 1.0;
    // die Wahl der Tonabnehmer als Gewichte (Steg = Index 0, Mitte = Index 1)
    std::vector<double> gewichte(s.abnehmerStellen.size(), 1.0);
    if (gewichte.size() >= 2)
    {
        if (a.abnehmerWahl == AbnehmerWahl::Steg) gewichte[1] = 0.0;
        if (a.abnehmerWahl == AbnehmerWahl::Mitte) gewichte[0] = 0.0;
        if (a.abnehmerWahl == AbnehmerWahl::Gegenphase) gewichte[1] = -1.0;
    }
    std::vector<SMode> moden = abschnitt(w, w.f0, w.laenge, w.stelle, fs, s.abnehmerStellen, magnetisch, schieber, gewichte);
    // der andere Abschnitt hinter dem Mittelsteg: L_b = L_a·(1 − t)/t, f_b = f_a·t/(1 − t); er wird nur ueber den Steg
    // angeregt — gekoppelt werden die staerksten Moden zweier gleich hoher Teiltoene
    std::vector<SMode> partner;
    if (s.stegTeilung > 0.0 && s.stegTeilung < 1.0)
    {
        const double lb = w.laenge * (1.0 - teilung) / teilung;
        const double fb = w.f0 * w.laenge / lb;
        std::vector<SMode> b = abschnitt(w, fb, lb, 0.5, fs, s.abnehmerStellen, magnetisch, 0.0, gewichte);
        for (SMode& sb : b)
        {
            // die staerkste Mode des Teiltons von b
            bool staerkste = true;
            for (const SMode& x : b)
                if (x.teilton == sb.teilton && x.gewicht > sb.gewicht) staerkste = false;
            if (!staerkste) continue;
            int beste = -1;
            for (std::size_t i = 0; i < moden.size(); ++i)
            {
                if (moden[i].partner >= 0 || std::abs(moden[i].hz - sb.hz) > kStegFenster * sb.hz) continue;
                if (beste < 0 || moden[i].gewicht > moden[static_cast<std::size_t>(beste)].gewicht) beste = static_cast<int>(i);
            }
            if (beste < 0) continue;
            sb.m.eingang = 0.0;
            moden[static_cast<std::size_t>(beste)].partner = static_cast<int>(partner.size());
            partner.push_back(sb);
        }
    }
    // Mitschwingen (je Clip): die staerkste Mode jedes Teiltons an die naechste Mode eines anderen Chors (auf 0,5 %)
    std::vector<SMode> mit;
    std::vector<int> mitIndex(moden.size(), -1);
    // (mit Daempferleiste und ohne Pedal liegt der Daempfer auf allen Choeren: kein Mitschwingen)
    if (a.mitschwingen && s.mitschwingen && s.anregung == Anregung::Schlaegel && !(s.daempfer && a.gedaempft))
    {
        std::vector<SMode> kandidaten;
        for (int c = s.noteMin; c <= s.noteMax; ++c)
        {
            if (c == note) continue;
            SaitenAnschlag ac = a;
            ac.note = c;
            ac.hz = 440.0 * std::exp2((c - 69) / 12.0);
            ac.tastendruckCent = 0.0;
            const Werte wc = werteAn(s, ac);
            const std::vector<SMode> b = abschnitt(wc, wc.f0, wc.laenge, 0.5, fs, s.abnehmerStellen, magnetisch, 0.0, gewichte);
            for (const SMode& x : b)
            {
                bool staerkste = true;
                for (const SMode& y : b)
                    if (y.teilton == x.teilton && y.gewicht > x.gewicht) staerkste = false;
                if (!staerkste) continue;
                kandidaten.push_back(x);
                kandidaten.back().m.eingang = 0.0;
            }
        }
        std::vector<bool> vergeben(kandidaten.size(), false);
        for (std::size_t j = 0; j < moden.size(); ++j)
        {
            bool staerkste = true;
            for (const SMode& y : moden)
                if (y.teilton == moden[j].teilton && y.gewicht > moden[j].gewicht) staerkste = false;
            if (!staerkste) continue;
            int beste = -1;
            double abstand = kStegFenster;
            for (std::size_t k = 0; k < kandidaten.size(); ++k)
            {
                if (vergeben[k]) continue;
                const double d = std::abs(kandidaten[k].hz - moden[j].hz) / moden[j].hz;
                if (d < abstand)
                {
                    abstand = d;
                    beste = static_cast<int>(k);
                }
            }
            if (beste < 0) continue;
            vergeben[static_cast<std::size_t>(beste)] = true;
            mitIndex[j] = static_cast<int>(mit.size());
            mit.push_back(kandidaten[static_cast<std::size_t>(beste)]);
        }
    }
    std::vector<Mode> bm;
    for (const SMode& m : moden) bm.push_back(m.m);
    ModalBank bank(bm, fs);
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    std::int64_t n = 0;
    const double tempo = s.tempoMax * std::pow(10.0, std::clamp(a.stufe, -40, 0) / 20.0);
    double h = 0.0;  // die Auslenkung an der Tangente
    if (s.anregung == Anregung::Schlaegel)
    {
        std::vector<double> stoss;
        const StossMessung st = stosse(bank, s.schlaegel, tempo, stoss, s.ueberabtastung);
        n = std::min<std::int64_t>(frames, st.frames);
        for (std::int64_t i = 0; i < n; ++i) summe[static_cast<std::size_t>(i)] = stoss[static_cast<std::size_t>(i)];
        if (messung != nullptr)
        {
            messung->kontaktS = st.kontaktS;
            messung->kraftSpitzeN = st.kraftSpitzeN;
        }
    }
    else
    {
        // die Tangente: h(t) = d/2·(1 − cos(πt/τ)), τ = d/v, ḧ = d/2·(π/τ)²·cos(πt/τ); die Moden getrieben mit −2/(kπ)·ḧ
        const double d = s.tangenteWegMm * 1e-3;
        const double tau = d / std::max(tempo, 1e-6);
        const int u = std::max(1, s.ueberabtastung);
        const double dtFein = dt / u;
        std::vector<Mode>& m = bank.moden();
        std::vector<Komplex> polFein(m.size());
        for (std::size_t j = 0; j < m.size(); ++j) polFein[j] = std::exp(m[j].lambda * dtFein);
        const auto schritte = static_cast<std::int64_t>(std::ceil(tau / dtFein));
        for (std::int64_t i = 0; i < schritte; ++i)
        {
            if (i % u == 0 && n < frames) summe[static_cast<std::size_t>(n++)] = bank.ausgang();
            const double t = (i + 0.5) * dtFein;
            const double beschl = 0.5 * d * (kPi / tau) * (kPi / tau) * std::cos(kPi * t / tau);
            for (std::size_t j = 0; j < m.size(); ++j) m[j].zustand = polFein[j] * (m[j].zustand + moden[j].tangente * beschl * dtFein);
        }
        // auf den groben Takt: die Feinschritte bis zum naechsten vollen Takt frei
        const std::int64_t rest = (u - schritte % u) % u;
        for (std::int64_t i = 0; i < rest; ++i)
            for (std::size_t j = 0; j < m.size(); ++j) m[j].zustand *= polFein[j];
        h = d;
        if (messung != nullptr) messung->kontaktS = tau;
    }

    // --- frei: Takt fuer Takt (Paare am Mittelsteg, Daempfer) bis zum Loslassen bzw. zum Ende ---
    std::vector<Mode>& m = bank.moden();
    const double sigmaD = s.daempfer && a.gedaempft ? kLn1000 / logw(s.daempferT60S, note) : 0.0;
    std::vector<Komplex> zp(partner.size(), Komplex(0.0, 0.0));
    std::vector<std::array<Komplex, 4>> paar(m.size()), paarD(m.size());
    std::vector<Komplex> pol(m.size()), polD(m.size());
    for (std::size_t j = 0; j < m.size(); ++j)
    {
        pol[j] = std::exp(m[j].lambda * dt);
        const Komplex ld(std::min(m[j].lambda.real(), -sigmaD), m[j].lambda.imag());
        polD[j] = std::exp(ld * dt);
        const int p = moden[j].partner;
        if (p < 0) continue;
        const Komplex c(-s.mittelstegKopplung * moden[j].sigmaFrueh, 0.0);
        paar[j] = paarSchrittMit(m[j].lambda, partner[static_cast<std::size_t>(p)].m.lambda, c, dt);
        paarD[j] = paarSchrittMit(ld, partner[static_cast<std::size_t>(p)].m.lambda, c, dt);
    }
    // die Paare des Mitschwingens: ohne Partner am Mittelsteg exakt, mit ihm nur die Kopplung (geteilt, s. Kopf)
    std::vector<Komplex> zm(mit.size(), Komplex(0.0, 0.0));
    std::vector<Komplex> polM(mit.size());
    for (std::size_t k = 0; k < mit.size(); ++k) polM[k] = std::exp(mit[k].m.lambda * dt);
    std::vector<std::array<Komplex, 4>> paarM(m.size()), paarMD(m.size());
    for (std::size_t j = 0; j < m.size(); ++j)
    {
        const int q = mitIndex[j];
        if (q < 0) continue;
        const Komplex c(-s.mitschwingKopplung * moden[j].sigmaFrueh, 0.0);
        if (moden[j].partner < 0)
        {
            const Komplex ld(std::min(m[j].lambda.real(), -sigmaD), m[j].lambda.imag());
            paarM[j] = paarSchrittMit(m[j].lambda, mit[static_cast<std::size_t>(q)].m.lambda, c, dt);
            paarMD[j] = paarSchrittMit(ld, mit[static_cast<std::size_t>(q)].m.lambda, c, dt);
        }
        else
        {
            const Komplex ch = std::cosh(c * dt), sh = std::sinh(c * dt);
            paarM[j] = paarMD[j] = {ch, sh, sh, ch};
        }
    }
    const bool clavinet = s.anregung == Anregung::Tangente && s.ganzJeKlingend > 1.0;
    const std::int64_t ende = clavinet ? std::min(frames, std::max(n, a.halten)) : frames;
    // die Moden ohne Partner zeigerweise (wie das Vorbild), die Paare Takt fuer Takt
    {
        std::vector<Mode> allein;
        std::vector<std::size_t> index;
        for (std::size_t j = 0; j < m.size(); ++j)
            if (moden[j].partner < 0 && mitIndex[j] < 0)
            {
                allein.push_back(m[j]);
                index.push_back(j);
            }
        const ModalBank frei(allein, fs);
        frei.klingeAus(summe, n, ende, sigmaD > 0.0 ? a.halten : -1, sigmaD);
        // der Zustand am Ende (fuer das Loslassen des Clavinets): geschlossen fortgeschrieben
        if (clavinet)
            for (std::size_t i = 0; i < index.size(); ++i)
                m[index[i]].zustand *= std::exp(m[index[i]].lambda * (static_cast<double>(ende - n) * dt));
    }
    if (!partner.empty() || !mit.empty())
        for (std::int64_t i = n; i < ende; ++i)
        {
            const bool d = sigmaD > 0.0 && i >= a.halten;
            double aus = 0.0;
            for (std::size_t j = 0; j < m.size(); ++j)
            {
                const int p = moden[j].partner;
                const int q = mitIndex[j];
                if (p < 0 && q < 0) continue;
                aus += (m[j].aus * m[j].zustand).imag();
                if (p >= 0)
                {
                    Komplex& b = zp[static_cast<std::size_t>(p)];
                    aus += (partner[static_cast<std::size_t>(p)].m.aus * b).imag();
                    const std::array<Komplex, 4>& e = d ? paarD[j] : paar[j];
                    const Komplex za = m[j].zustand;
                    m[j].zustand = e[0] * za + e[1] * b;
                    b = e[2] * za + e[3] * b;
                }
                if (q >= 0)
                {
                    Komplex& b = zm[static_cast<std::size_t>(q)];
                    aus += (mit[static_cast<std::size_t>(q)].m.aus * b).imag();
                    const std::array<Komplex, 4>& e = d ? paarMD[j] : paarM[j];
                    if (p >= 0) b *= polM[static_cast<std::size_t>(q)];  // geteilt: die eigene Bewegung, dann die Kopplung
                    const Komplex za = m[j].zustand;
                    m[j].zustand = e[0] * za + e[1] * b;
                    b = e[2] * za + e[3] * b;
                }
            }
            summe[static_cast<std::size_t>(i)] += aus;
        }

    // --- Clavinet: Loslassen — der klingende Teil und der stille (eine Gerade auf h) auf die ganze Saite projiziert ---
    if (clavinet && ende < frames)
    {
        const double ls = w.laenge;
        const double lg = ls * s.ganzJeKlingend;
        const double ln = lg - ls;
        const double fg = w.f0 * ls / lg;
        const double bg = w.b * (ls / lg) * (ls / lg);  // B ∝ 1/L² (gleiche Saite, gleiche Spannung)
        const double sigmaG = kLn1000 / logw(s.garnT60S, note);
        const int punkte = 1500;
        std::vector<double> y(punkte + 1), v(punkte + 1);
        for (int i = 0; i <= punkte; ++i)
        {
            const double x = lg * i / punkte;  // von der Wicklung (x = 0) bis zum Steg
            if (x <= ln)
            {
                y[static_cast<std::size_t>(i)] = h * x / ln;
                v[static_cast<std::size_t>(i)] = 0.0;
                continue;
            }
            const double xs = x - ln;  // im klingenden Teil, ab der Tangente
            double yy = h * (1.0 - xs / ls), vv = 0.0;
            for (std::size_t j = 0; j < m.size(); ++j)
            {
                const double sn = std::sin(moden[j].teilton * kPi * xs / ls);
                yy += m[j].zustand.imag() * sn;
                vv += (m[j].lambda * m[j].zustand).imag() * sn;
            }
            y[static_cast<std::size_t>(i)] = yy;
            v[static_cast<std::size_t>(i)] = vv;
        }
        std::vector<Mode> ganz;
        for (int k = 1; k <= kTeiltoeneMax; ++k)
        {
            const double fk = k * fg * std::sqrt(1.0 + bg * k * k);
            if (fk >= kTeiltonGrenze * fs) break;
            // Trapezregel: a_k = (2/L)∫y·sin(kπx/L)dx
            double ak = 0.0, bk = 0.0;
            for (int i = 0; i <= punkte; ++i)
            {
                const double gew = (i == 0 || i == punkte) ? 0.5 : 1.0;
                const double sn = std::sin(k * kPi * i / punkte);
                ak += gew * y[static_cast<std::size_t>(i)] * sn;
                bk += gew * v[static_cast<std::size_t>(i)] * sn;
            }
            ak *= 2.0 / punkte;
            bk *= 2.0 / punkte;
            const double sigma = std::max(sigmaG, w.sigma1) + schieber;
            Mode g;
            g.lambda = Komplex(-sigma, 2.0 * kPi * fk);
            // Im(z) = a, Im(λz) = b → z = (b − λ_r·a)/λ_i + i·a
            g.zustand = Komplex((bk - g.lambda.real() * ak) / g.lambda.imag(), ak);
            if (magnetisch)
            {
                double s2 = 0.0;
                for (std::size_t q = 0; q < s.abnehmerStellen.size(); ++q)
                    s2 += gewichte[q] * std::sin(k * kPi * (ln + (1.0 - s.abnehmerStellen[q]) * ls) / lg);
                g.aus = g.lambda * s2;
            }
            else
                g.aus = Komplex(k * kPi * 4.0 * lg * fg * fg * w.mu, 0.0);
            ganz.push_back(g);
        }
        if (messung != nullptr && !ganz.empty()) messung->hzNachLoslassen = ganz.front().lambda.imag() / (2.0 * kPi);
        const ModalBank frei(ganz, fs);
        frei.klingeAus(summe, ende, frames);
    }
    if (s.verstaerker)
    {
        // Shelf-Filter zweiter Ordnung (Bristow-Johnson, S = 1): tief −3 dB an 130 Hz, hoch +3 dB an 4 kHz
        const auto shelf = [&](double f0, double db, bool hoch) {
            const double aa = std::pow(10.0, db / 40.0), w0 = 2.0 * kPi * f0 / fs, cw = std::cos(w0), al = std::sin(w0) / 2.0 * std::sqrt(2.0);
            const double sq = 2.0 * std::sqrt(aa) * al, sg = hoch ? 1.0 : -1.0;
            const double b0 = aa * ((aa + 1.0) + sg * (aa - 1.0) * cw + sq), b1 = -2.0 * sg * aa * ((aa - 1.0) + sg * (aa + 1.0) * cw),
                         b2 = aa * ((aa + 1.0) + sg * (aa - 1.0) * cw - sq), a0 = (aa + 1.0) - sg * (aa - 1.0) * cw + sq,
                         a1 = 2.0 * sg * ((aa - 1.0) - sg * (aa + 1.0) * cw), a2 = (aa + 1.0) - sg * (aa - 1.0) * cw - sq;
            double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
            for (double& x : summe)
            {
                const double y = (b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
                x2 = x1;
                x1 = x;
                y2 = y1;
                y1 = y;
                x = y;
            }
        };
        shelf(130.0, -3.0, false);
        shelf(4000.0, 3.0, true);
    }
    const auto blende = std::min<std::int64_t>(frames, std::max<std::int64_t>(1, std::llround(kAusblendenS * fs)));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double wert = summe[static_cast<std::size_t>(i)] / s.ausgangBezug;
        const std::int64_t rest = frames - 1 - i;
        if (rest < blende) wert *= static_cast<double>(rest) / static_cast<double>(blende);
        mono[static_cast<std::size_t>(i)] = static_cast<float>(wert);
    }
}

void rechneSaitenClip(const SaitenSpiel& s, const std::vector<SaitenClipNote>& noten, std::vector<float>& mono, std::int64_t frames)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    std::vector<double> summe(mono.size(), 0.0);
    for (const SaitenClipNote& cn : noten)
    {
        SaitenAnschlag a = cn.anschlag;
        a.mitschwingen = true;
        const auto ab = static_cast<std::int64_t>(std::llround(std::max(0.0, cn.abS) * a.rate));
        if (ab >= frames) continue;
        std::vector<float> ton;
        rechneSaitenTon(s, a, ton, frames - ab);
        for (std::size_t i = 0; i < ton.size(); ++i) summe[static_cast<std::size_t>(ab) + i] += ton[i];
    }
    for (std::size_t i = 0; i < mono.size(); ++i) mono[i] = static_cast<float>(summe[i]);
}

void richteSaitenSpiel(SaitenSpiel& s, const SaitenSpiel& v)
{
    const auto r = [&](Kurve& k, double von, double bis, const Kurve& e) { richteKurve(k, von, bis, e, kSaitenUmfangMin, kSaitenUmfangMax); };  // KL4-1 a: der Umfang der Tafel
    r(s.laengeM, 0.02, 8.0, v.laengeM);
    r(s.durchmesserMm, 0.05, 5.0, v.durchmesserMm);
    r(s.inharmonizitaet, 1e-7, 0.1, v.inharmonizitaet);
    r(s.t60S, 0.01, 120.0, v.t60S);
    r(s.t60SpaetS, 0.01, 120.0, v.t60SpaetS);
    r(s.abklingHoch, 0.001, 1.0, v.abklingHoch);
    r(s.chor, 1.0, 5.0, v.chor);
    r(s.verstimmungCent, 0.0, 20.0, v.verstimmungCent);
    r(s.polarisationWinkelGrad, 0.0, 90.0, v.polarisationWinkelGrad);
    // δ ≥ 0,01 Cent, wenn γ > 0: sonst sind die Ebenen entartet (L11, Vorbild S163)
    r(s.polarisationCent, 0.01, 20.0, v.polarisationCent);
    r(s.waagrechtVerhaeltnis, 0.0, 1.0, v.waagrechtVerhaeltnis);
    r(s.anschlagStelle, 0.0, 1.0, v.anschlagStelle);
    r(s.daempferT60S, 0.01, 10.0, v.daempferT60S);
    r(s.garnT60S, 0.01, 10.0, v.garnT60S);
    r(s.schieberT60S, 0.01, 10.0, v.schieberT60S);
    s.tempoMax = std::clamp(s.tempoMax, 0.01, 20.0);
    s.tangenteWegMm = std::clamp(s.tangenteWegMm, 0.01, 10.0);
    s.stegTeilung = std::clamp(s.stegTeilung, 0.0, 0.95);
    s.ganzJeKlingend = std::clamp(s.ganzJeKlingend, 1.0, 4.0);
    s.tastendruckGrenzeCent = std::clamp(s.tastendruckGrenzeCent, 0.0, 100.0);
    s.ueberabtastung = std::clamp(s.ueberabtastung, 1, 256);
}

}  // namespace lumi::kern::saite
