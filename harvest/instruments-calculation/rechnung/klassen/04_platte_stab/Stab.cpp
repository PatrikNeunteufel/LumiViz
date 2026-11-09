/**
 * @file Stab.cpp
 * @brief Klasse 4: der Ton eines Stabspiels (Herleitung im Kopf von `Stab.hpp`)
 */
#include "klassen/04_platte_stab/Stab.hpp"

#include "gemeinsam/Balken.hpp"
#include "gemeinsam/Reibung.hpp"
#include "gemeinsam/Werkzeug.hpp"
#include "messung/Bausteine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace ic::stab
{

namespace
{

constexpr double kPi = std::numbers::pi;
/// Am Ende blendet der Ton so lange aus (wie der Fluegel).
constexpr double kAusblendenS = 0.005;
/// Hoechstens so viele Torsionsmoden.
constexpr int kTorsionMax = 6;

/// Die Formen des gleichfoermigen Balkens (frei–frei oder eingespannt–frei), einmal gerechnet, je Knoten auf die
/// modale Masse 1 (Gesamtmasse 1 kg) normiert.
struct Formen
{
    Balken balken;
    std::vector<BalkenMode> moden;
};

const Formen& formen(bool krag)
{
    static const Formen frei = [] {
        Formen f;
        f.balken.laengeM = 1.0;
        f.balken.dichteKgM3 = 1.0 / (0.05 * 0.02);  // ρ·A·L = 1 kg
        f.balken.elemente = 120;
        f.moden = biegeModen(f.balken, kBiegeModenMax);
        return f;
    }();
    static const Formen kragF = [] {
        Formen f;
        f.balken.laengeM = 1.0;
        f.balken.dichteKgM3 = 1.0 / (0.05 * 0.02);
        f.balken.elemente = 120;
        f.balken.links = Lager::Eingespannt;
        f.moden = biegeModen(f.balken, kBiegeModenMax);
        return f;
    }();
    return krag ? kragF : frei;
}

double lin(const Kurve& k, int n)
{
    return wertAn(k, n, Skala::Linear);
}

double logw(const Kurve& k, int n)
{
    return wertAn(k, n, Skala::Logarithmisch);
}

/// Die Oeffnung der Klappe beim Winkel θ: zwei Perioden je Umdrehung, Restoeffnung r.
double oeffnung(double theta, double r)
{
    return r + (1.0 - r) * 0.5 * (1.0 + std::cos(2.0 * theta));
}

/// Die Drehzahl des Motors zur Zeit t (U/s): die Spur mit Hoch- und Auslauf (linear in `hochlauf`).
class Motor
{
public:
    Motor(const std::vector<MotorStelle>& spur, double hochlauf) : m_spur(spur), m_hochlauf(std::max(1e-3, hochlauf))
    {
        std::stable_sort(m_spur.begin(), m_spur.end(), [](const MotorStelle& a, const MotorStelle& b) { return a.zeitS < b.zeitS; });
    }

    [[nodiscard]] double drehzahl(double t) const
    {
        // vor der ersten Stelle steht der Motor; an jeder Stelle laeuft er linear vom Wert, den er hatte, zum neuen
        double wert = 0.0;
        double ab = -1e9;
        double von = 0.0;
        double ziel = 0.0;
        for (const MotorStelle& s : m_spur)
        {
            if (s.zeitS > t) break;
            von = wertBei(von, ziel, s.zeitS - ab);
            ab = s.zeitS;
            ziel = s.drehzahl;
        }
        wert = wertBei(von, ziel, t - ab);
        return wert;
    }

private:
    [[nodiscard]] double wertBei(double von, double ziel, double seit) const
    {
        const double schritt = (ziel - von) / m_hochlauf;
        const double t = std::clamp(seit, 0.0, m_hochlauf);
        return von + schritt * t;
    }

    std::vector<MotorStelle> m_spur;
    double m_hochlauf;
};

}  // namespace

int stufeAus(double laut)
{
    if (!(laut > 0.0)) return -40;
    return std::clamp(static_cast<int>(std::lround(20.0 * std::log10(laut))), -40, 0);
}

StabModen stabModen(const StabSpiel& s, const StabAnschlag& a)
{
    StabModen ergebnis;
    if (a.rate <= 0) return ergebnis;
    const int n = std::clamp(a.note, s.noteMin, s.noteMax);
    const double grenze = kTeiltonGrenze * a.rate;
    const double f1 = a.hz * lin(s.grundJeNote, n);
    const Formen& f = formen(s.kragbalken);
    const double stelle = std::clamp(lin(s.anschlagStelle, n) + a.stelleOffset, 0.0, 1.0);
    const double abnehmer = std::clamp(lin(s.abnehmerStelle, n), 0.0, 1.0);
    const double r2 = lin(s.verhaeltnis2, n);
    const double r3 = lin(s.verhaeltnis3, n);
    const double xi1 = sigmaAusT60(logw(s.t60S, n)) / (2.0 * kPi * f1);
    const double kappaXi = lin(s.daempfungExponent, n);
    const double gExp = lin(s.abstrahlExponent, n);
    const auto anzahl = s.festeVerhaeltnisse.empty() ? f.moden.size() : std::min(f.moden.size(), s.festeVerhaeltnisse.size());
    for (int k = 0; k < static_cast<int>(anzahl); ++k)
    {
        double r = 1.0;
        if (!s.festeVerhaeltnisse.empty())
            r = s.festeVerhaeltnisse[static_cast<std::size_t>(k)];
        else if (k == 1)
            r = r2;
        else if (k >= 2)
        {
            // ueber der dritten Mode der Abstand des gleichfoermigen Balkens (offen.md)
            r = r3 * f.moden[static_cast<std::size_t>(k)].hz / f.moden[2].hz;
        }
        const double hz = f1 * r;
        if (hz >= grenze) break;
        StabMode m;
        m.hz = hz;
        const double xi = xi1 * std::pow(r, kappaXi);
        m.t60S = kLn1000 / (xi * 2.0 * kPi * hz);
        m.form = formAn(f.moden[static_cast<std::size_t>(k)], f.balken, stelle);
        m.abnehmer = formAn(f.moden[static_cast<std::size_t>(k)], f.balken, abnehmer);
        m.abstrahl = std::pow(static_cast<double>(k + 1), gExp);
        ergebnis.stab.push_back(m);
    }
    // Torsion: nur ausserhalb der Laengsachse angeregt
    const double versatz = lin(s.querVersatz, n);
    const double rt = lin(s.torsionVerhaeltnis, n);
    // ohne Versatz bekommt die Torsion nichts — dann wird sie nicht gerechnet (die Wirkung ist exakt null)
    if (rt > 0.0 && versatz != 0.0)
        for (int k = 1; k <= kTorsionMax; ++k)
        {
            const double hz = k * rt * f1;
            if (hz >= grenze) break;
            StabMode m;
            m.hz = hz;
            m.t60S = logw(s.torsionT60S, n);
            m.form = std::sqrt(24.0) * versatz * std::cos(k * kPi * stelle);
            m.abnehmer = 0.0;
            m.abstrahl = std::pow(static_cast<double>(k), gExp);
            m.torsion = true;
            ergebnis.stab.push_back(m);
        }
    // Partner
    if (s.partner != PartnerArt::Keiner)
    {
        const double fr = f1 * lin(s.partnerVerhaeltnis, n);
        const double xiP = logw(s.partnerXi, n);
        const double kJeKrit = lin(s.kopplungJeKritisch, n);
        const double gP = logw(s.partnerAbstrahl, n);
        for (int m = 1; m <= (s.partner == PartnerArt::RohrGeschlossen ? 12 : 1); ++m)
        {
            const double hz = (2 * m - 1) * fr;
            if (hz >= grenze) break;
            PartnerMode p;
            p.hz = hz;
            p.t60S = kLn1000 / (xiP * 2.0 * kPi * hz);
            p.abstrahl = gP;
            // die naechste Biegemode im Fenster
            int beste = -1;
            double abstand = kPaarFenster;
            for (std::size_t i = 0; i < ergebnis.stab.size(); ++i)
            {
                const StabMode& sm = ergebnis.stab[i];
                if (sm.torsion || sm.partner >= 0) continue;
                const double d = std::abs(sm.hz - hz) / sm.hz;
                if (d < abstand)
                {
                    abstand = d;
                    beste = static_cast<int>(i);
                }
            }
            if (beste >= 0)
            {
                StabMode& sm = ergebnis.stab[static_cast<std::size_t>(beste)];
                sm.partner = static_cast<int>(ergebnis.partner.size());
                const double kritisch = 0.5 * std::abs(sigmaAusT60(p.t60S) - sigmaAusT60(sm.t60S));
                p.kopplung = kJeKrit * kritisch;
            }
            ergebnis.partner.push_back(p);
        }
    }
    return ergebnis;
}

std::int64_t stabNachklang(const StabSpiel& s, const StabAnschlag& a, std::int64_t maxFrames)
{
    const StabModen m = stabModen(s, a);
    double sigma = 1e300;
    for (const StabMode& sm : m.stab)
    {
        double sig = sigmaAusT60(sm.t60S);
        if (sm.partner >= 0)
        {
            // gekoppelt: der langsamere der beiden Eigenwerte des Paars
            const PartnerMode& p = m.partner[static_cast<std::size_t>(sm.partner)];
            const Komplex ls(-sig, 0.0), lp(-sigmaAusT60(p.t60S), 0.0);
            const Komplex d = std::sqrt(0.25 * (ls - lp) * (ls - lp) - p.kopplung * p.kopplung);
            sig = -std::max((0.5 * (ls + lp) + d).real(), (0.5 * (ls + lp) - d).real());
        }
        if (std::abs(sm.form) > 1e-9) sigma = std::min(sigma, sig);
    }
    if (!(sigma < 1e299)) return 1;
    return std::clamp<std::int64_t>(static_cast<std::int64_t>(std::ceil(kLn1000 / sigma * a.rate)), 1, std::max<std::int64_t>(1, maxFrames));
}

void rechneStabTon(const StabSpiel& s, const StabAnschlag& a, std::vector<float>& mono, std::int64_t frames, StabMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0) return;
    const int note = std::clamp(a.note, s.noteMin, s.noteMax);
    const double fs = a.rate;
    const double dt = 1.0 / fs;
    const StabModen sm = stabModen(s, a);
    const double masse = logw(s.masseKg, note);
    const bool akustisch = s.abnahme == Abnahme::Akustisch;

    std::vector<Mode> moden;
    for (const StabMode& m : sm.stab)
    {
        Mode mode = modeAus(m.hz, m.t60S, masse, m.form, m.form, akustisch ? m.abstrahl : m.abnehmer,
                            akustisch ? Ausgang::Beschleunigung : Ausgang::Auslenkung);
        moden.push_back(mode);
    }
    ModalBank bank(moden, fs);
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    std::int64_t n = 0;
    const bool gestrichen = s.bogen && a.gestrichen;
    if (gestrichen)
    {
        // gestrichen: kein Anschlag — der Bogen wirkt im freien Lauf unten
    }
    else if (s.anregung == Anregung::Stoss)
    {
        const double tempo = s.tempoMax * std::pow(10.0, std::clamp(a.stufe, -40, 0) / 20.0);
        std::vector<double> stoss;
        const StossMessung st = stosse(bank, s.schlaegel, tempo, stoss, s.ueberabtastung);
        n = std::min<std::int64_t>(frames, st.frames);
        // waehrend des Kontakts nur die Ausgabe der Stabmoden (die Partner sind in 1 ms kaum beteiligt: κ·t ≪ 1);
        // beim Tonabnehmer ist sie die Auslenkung am Abnehmer
        for (std::int64_t i = 0; i < n; ++i) summe[static_cast<std::size_t>(i)] = stoss[static_cast<std::size_t>(i)];
        if (messung != nullptr)
        {
            messung->kontaktS = st.kontaktS;
            messung->kraftSpitzeN = st.kraftSpitzeN;
            messung->kontakte = st.kontakte;
        }
    }
    else
    {
        // Zupfen: die Zunge statisch an der Stelle ausgelenkt (Auslenkung δ = `tempoMax` mm bei Stufe 0), dann frei:
        // q_n(0) = F·φ_n/(m·ω_n²), q̇_n(0) = 0 → z = i·q(0) (Im(i·q·e^(λt)) = q·e^(−σt)·cos ωt)
        double nachgiebig = 0.0;
        for (const Mode& m : bank.moden())
        {
            const double w2 = std::norm(m.lambda);
            nachgiebig += m.abnahme * m.abnahme / (masse * w2);
        }
        const double delta = s.tempoMax * 1e-3 * std::pow(10.0, std::clamp(a.stufe, -40, 0) / 20.0);
        const double kraft = nachgiebig > 0.0 ? delta / nachgiebig : 0.0;
        for (Mode& m : bank.moden()) m.zustand = Komplex(0.0, kraft * m.abnahme / (masse * std::norm(m.lambda)));
        if (messung != nullptr) messung->kraftSpitzeN = kraft;
    }

    // --- frei: Takt fuer Takt (Partner, Motor, Daempfer, Tonabnehmer) ---
    const Motor motor(a.motor, s.hochlaufS);
    const bool mitMotor = s.motor && !a.motor.empty();
    double theta = a.motorPhaseStart;
    const double sigmaD = s.daempfer != DaempferArt::Keiner ? sigmaAusT60(logw(s.daempferT60S, note)) * std::clamp(a.andruck, 0.0, 1.0) : 0.0;
    std::vector<Mode>& bm = bank.moden();
    std::vector<Komplex> partner(sm.partner.size(), Komplex(0.0, 0.0));
    std::vector<Komplex> lambdaP;
    for (const PartnerMode& p : sm.partner) lambdaP.push_back(Komplex(-sigmaAusT60(p.t60S), 2.0 * kPi * p.hz));
    std::vector<Komplex> pol(bm.size()), polD(bm.size());
    for (std::size_t i = 0; i < bm.size(); ++i)
    {
        pol[i] = std::exp(bm[i].lambda * dt);
        polD[i] = std::exp(Komplex(std::min(bm[i].lambda.real(), -sigmaD), bm[i].lambda.imag()) * dt);
    }
    // feste Paare ohne Motor: die Matrix einmal (mit und ohne Daempfer)
    std::vector<std::array<Komplex, 4>> paar(bm.size()), paarD(bm.size());
    for (std::size_t i = 0; i < bm.size(); ++i)
    {
        const int p = sm.stab[i].partner;
        if (p < 0) continue;
        const double kappa = sm.partner[static_cast<std::size_t>(p)].kopplung;
        paar[i] = paarSchritt(bm[i].lambda, lambdaP[static_cast<std::size_t>(p)], kappa, dt);
        paarD[i] = paarSchritt(Komplex(std::min(bm[i].lambda.real(), -sigmaD), bm[i].lambda.imag()), lambdaP[static_cast<std::size_t>(p)], kappa, dt);
    }
    const double d = s.abnehmerAbstandM;
    const double y0 = s.abnehmerVersatzM;
    const auto tonabnehmer = [&](double y) {
        if (s.abnahme == Abnahme::Magnetisch) return 1.0 / std::sqrt((y - y0) * (y - y0) + d * d);
        if (s.abnahme == Abnahme::Elektrostatisch) return 1.0 / (d - std::min(y, 0.9 * d));
        return 0.0;
    };
    double vorige = tonabnehmer(0.0);
    if (!akustisch)
        for (std::int64_t i = 0; i < n; ++i)
        {
            if (messung != nullptr) messung->auslenkungSpitzeM = std::max(messung->auslenkungSpitzeM, std::abs(summe[static_cast<std::size_t>(i)]));
            const double phi = tonabnehmer(summe[static_cast<std::size_t>(i)]);
            summe[static_cast<std::size_t>(i)] = (phi - vorige) * fs;
            vorige = phi;
        }
    // die Paare der quadratischen Kopplung (f_j ≈ 2·f_i, auf 3 %)
    struct Quadrat
    {
        std::size_t i, j;
        double k;  // c·m·ω_j²/L0
    };
    std::vector<Quadrat> quadrate;
    if (s.quadratischeKopplung != 0.0)
        for (std::size_t i = 0; i < bm.size(); ++i)
            for (std::size_t j = 0; j < bm.size(); ++j)
                if (!sm.stab[i].torsion && !sm.stab[j].torsion && std::abs(sm.stab[j].hz / sm.stab[i].hz - 2.0) < 0.06)
                    quadrate.push_back({i, j, s.quadratischeKopplung * masse * std::pow(2.0 * kPi * sm.stab[j].hz, 2.0) / 1e-3});
    std::vector<double> modalKraft(bm.size(), 0.0);
    // --- Bogen: die Formen am Bogenpunkt (Ende, auf der Achse — die Torsion bleibt still), v_B und F_B aus dem Weg ---
    std::vector<double> phiB(bm.size(), 0.0);
    double durchgriff = 0.0;  // Δv am Bogenpunkt je N Kraft im Takt: Δt·Σφ²/m
    double vb = 0.0, fbVoll = 0.0;
    const auto halten = static_cast<std::int64_t>(std::llround(std::max(0.0, a.haltenS) * fs));
    const double ansatz = std::max(1e-3, s.bogenAnsatzS);
    bool haftet = false;
    double letzte = -1.0;
    std::int64_t haftZahl = 0, messZahl = 0;
    if (gestrichen)
    {
        StabSpiel sb = s;
        sb.anschlagStelle = kurveAusPunkten({{note, std::clamp(s.bogenStelle, 0.0, 1.0)}});
        sb.querVersatz = kurveAusPunkten({{note, 0.0}});
        StabAnschlag ab = a;
        ab.stelleOffset = 0.0;
        const StabModen smB = stabModen(sb, ab);
        for (std::size_t k = 0; k < bm.size() && k < smB.stab.size(); ++k)
        {
            phiB[k] = smB.stab[k].form;
            durchgriff += phiB[k] * phiB[k] * dt / masse;
        }
        const double e = std::clamp(a.expression, 0.0, 1.0);
        vb = a.bogenVbMS > 0.0 ? a.bogenVbMS : s.bogenVbPp * std::pow(s.bogenVbFf / s.bogenVbPp, e);
        if (a.bogenKraftN > 0.0)
            fbVoll = a.bogenKraftN;
        else
        {
            // die Mitten der Fenster bei pp, mf, ff (fehlt eins, das naechste), stueckweise geometrisch
            std::array<double, 3> mitte = {0.0, 0.0, 0.0};
            for (std::size_t k = 0; k < 3; ++k)
                if (!s.bogenKraftUnten[k].punkte.empty() && !s.bogenKraftOben[k].punkte.empty())
                    mitte[k] = std::sqrt(logw(s.bogenKraftUnten[k], note) * logw(s.bogenKraftOben[k], note));
            if (mitte[1] <= 0.0) mitte[1] = mitte[0] > 0.0 ? mitte[0] : mitte[2];
            if (mitte[0] <= 0.0) mitte[0] = mitte[1];
            if (mitte[2] <= 0.0) mitte[2] = mitte[1];
            if (mitte[1] > 0.0)
                fbVoll = e < 0.5 ? mitte[0] * std::pow(mitte[1] / mitte[0], 2.0 * e) : mitte[1] * std::pow(mitte[2] / mitte[1], 2.0 * e - 1.0);
        }
    }
    for (std::int64_t i = n; i < frames; ++i)
    {
        if (gestrichen && durchgriff > 0.0)
        {
            const double t = static_cast<double>(i) * dt;
            double huelle = std::min(1.0, t / ansatz);
            if (i >= halten) huelle *= std::max(0.0, 1.0 - (t - a.haltenS) / ansatz);
            const double fb = fbVoll * huelle;
            double vh = 0.0;
            for (std::size_t k = 0; k < bm.size(); ++k) vh += phiB[k] * (bm[k].lambda * bm[k].zustand).imag();
            const double f = reibung(vh, vb, fb, 1.0 / durchgriff, s.bogenMuS, haftet, letzte);
            if (fb <= 0.0) haftet = false;
            for (std::size_t k = 0; k < bm.size(); ++k)
                if (phiB[k] != 0.0) bm[k].zustand += phiB[k] * f * dt / (masse * bm[k].lambda.imag());
            if (i >= halten / 2 && i < halten && fb > 0.0)
            {
                ++messZahl;
                if (haftet) ++haftZahl;
            }
        }
        const bool gedaempft = a.daempferAb >= 0 && i >= a.daempferAb && sigmaD > 0.0;
        double w = 1.0;
        if (mitMotor)
        {
            const double t = static_cast<double>(i) * dt;
            theta += 2.0 * kPi * motor.drehzahl(t) * dt;
            w = oeffnung(theta, s.restOeffnung);
        }
        double aus = 0.0;
        double y = 0.0;
        if (!quadrate.empty())
        {
            std::fill(modalKraft.begin(), modalKraft.end(), 0.0);
            for (const Quadrat& q : quadrate)
            {
                const double qi = bm[q.i].zustand.imag();
                const double qj = bm[q.j].zustand.imag();
                modalKraft[q.i] -= 2.0 * q.k * qi * qj;
                modalKraft[q.j] -= q.k * qi * qi;
            }
            for (std::size_t k = 0; k < bm.size(); ++k)
                bm[k].zustand += modalKraft[k] * dt / (masse * bm[k].lambda.imag());
        }
        for (std::size_t k = 0; k < bm.size(); ++k)
        {
            Mode& m = bm[k];
            const double wert = (m.aus * m.zustand).imag();
            if (akustisch)
                aus += wert;
            else
                y += wert;
            const int p = sm.stab[k].partner;
            if (p < 0)
            {
                m.zustand *= gedaempft ? polD[k] : pol[k];
                continue;
            }
            Komplex& zp = partner[static_cast<std::size_t>(p)];
            const PartnerMode& pm = sm.partner[static_cast<std::size_t>(p)];
            if (akustisch)
            {
                // der Partner strahlt mit seiner Beschleunigung; Gewicht gegen den Stab
                const Komplex lp = lambdaP[static_cast<std::size_t>(p)];
                aus += (pm.abstrahl * w * lp * lp * zp).imag();
            }
            std::array<Komplex, 4> e;
            if (mitMotor)
            {
                const Komplex ls = gedaempft ? Komplex(std::min(m.lambda.real(), -sigmaD), m.lambda.imag()) : m.lambda;
                e = paarSchritt(ls, lambdaP[static_cast<std::size_t>(p)], pm.kopplung * w, dt);
            }
            else
                e = gedaempft ? paarD[k] : paar[k];
            const Komplex zs = m.zustand;
            m.zustand = e[0] * zs + e[1] * zp;
            zp = e[2] * zs + e[3] * zp;
        }
        // nicht gekoppelte Partnermoden klingen mit (vom Stab nicht angeregt, bleiben null)
        if (!akustisch)
        {
            if (messung != nullptr) messung->auslenkungSpitzeM = std::max(messung->auslenkungSpitzeM, std::abs(y));
            const double phi = tonabnehmer(y);
            aus = (phi - vorige) * fs;
            vorige = phi;
        }
        summe[static_cast<std::size_t>(i)] += aus;
    }
    if (gestrichen && messung != nullptr)
    {
        messung->haftAnteil = messZahl > 0 ? static_cast<double>(haftZahl) / static_cast<double>(messZahl) : 0.0;
        const auto haltenF = static_cast<std::size_t>(std::min<std::int64_t>(frames, halten));
        const auto ab = static_cast<std::size_t>(0.5 * a.haltenS * fs);
        if (haltenF > ab + 4800)
        {
            std::vector<double> stueck(summe.begin() + static_cast<std::ptrdiff_t>(ab), summe.begin() + static_cast<std::ptrdiff_t>(haltenF));
            const messung::Tonhoehe th = messung::tonhoehe(stueck, a.rate, 30.0, 4000.0);
            std::vector<double> hz;
            for (const double x : th.hz)
                if (x > 0.0) hz.push_back(x);
            if (hz.size() > th.hz.size() / 2)
            {
                std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
                messung->spielHz = hz[hz.size() / 2];
                messung->schwingt = true;
            }
        }
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

}  // namespace ic::stab

namespace ic::stab
{

void stimmeBogen(StabSpiel& s)
{
    if (!s.bogen) return;
    // die Note spielt gestrichen: sie schwingt an — der Pegel der zweiten Haelfte mindestens 20 dB ueber dem bei der
    // kleinsten Kraft (dort nur der Ansatz) —, die Tonhoehe liegt auf ±15 Cent an der ersten Biegemode (am Modell gibt
    // es daneben einen Haft-Gleit-Zustand 80 … 100 Cent zu hoch, einen gedrueckten 40 Cent zu tief und ein reines
    // Gleiten, das kaum anschwingt — nicht der Klang des gestrichenen Stabs), und der Bogen haftet in 5 … 98 % der Takte.
    // Raster 2^(1/8) von 10 mN bis 200 N je v_B (pp, mf = √(v_pp·v_ff), ff); der laengste Lauf, ohne je einen
    // Randschritt (wie Klasse 3)
    std::vector<double> raster;
    for (double f = 1e-2; f <= 200.0; f *= std::pow(2.0, 0.125)) raster.push_back(f);
    std::vector<int> noten;
    for (int n = s.noteMin; n <= s.noteMax; n += 3) noten.push_back(n);
    if (noten.back() != s.noteMax) noten.push_back(s.noteMax);
    const std::array<double, 3> vbs = {s.bogenVbPp, std::sqrt(s.bogenVbPp * s.bogenVbFf), s.bogenVbFf};
    std::vector<std::array<double, 2>> fenster(3 * noten.size(), {0.0, 0.0});
    parallelJe(3 * noten.size(), [&](std::size_t jj) {
        const std::size_t j = jj / 3, stufe = jj % 3;
        const int note = noten[j];
        const double vb = vbs[stufe];
        std::vector<double> pegel(raster.size(), 0.0), cent(raster.size(), 1e9), haft(raster.size(), 0.0);
        for (std::size_t i = 0; i < raster.size(); ++i)
        {
            StabAnschlag a;
            a.note = note;
            a.hz = 440.0 * std::exp2((note - 69) / 12.0);
            a.gestrichen = true;
            a.haltenS = 0.8;
            a.bogenKraftN = raster[i];
            a.bogenVbMS = vb;
            std::vector<float> ton;
            StabMessung m;
            const auto frames = static_cast<std::int64_t>(0.8 * a.rate);
            rechneStabTon(s, a, ton, frames, &m);
            double e = 0.0;
            for (std::int64_t k = frames / 2; k < frames; ++k) e += double(ton[static_cast<std::size_t>(k)]) * ton[static_cast<std::size_t>(k)];
            pegel[i] = e;
            if (m.schwingt) cent[i] = 1200.0 * std::log2(m.spielHz / (a.hz * lin(s.grundJeNote, note)));
            haft[i] = m.haftAnteil;
        }
        std::vector<bool> spielt(raster.size(), false);
        for (std::size_t i = 0; i < raster.size(); ++i)
            spielt[i] = pegel[i] > 100.0 * pegel[0] && std::abs(cent[i]) < 15.0 && haft[i] > 0.05 && haft[i] < 0.98;
        std::size_t besterA = 0, besteL = 0;
        for (std::size_t i = 0; i < raster.size();)
        {
            if (!spielt[i])
            {
                ++i;
                continue;
            }
            std::size_t k = i;
            while (k < raster.size() && spielt[k]) ++k;
            if (k - i > besteL)
            {
                besteL = k - i;
                besterA = i;
            }
            i = k;
        }
        if (besteL == 0) return;
        std::size_t lo = besterA, hi = besterA + besteL - 1;
        if (besteL >= 4)
        {
            ++lo;
            --hi;
        }
        fenster[jj] = {raster[lo], raster[hi]};
    });
    for (std::size_t stufe = 0; stufe < 3; ++stufe)
        for (int q = 0; q < 2; ++q)
        {
            Kurve k;
            k.belegkraft = Belegkraft::Rechnung;
            k.herkunft = "Fenster der Bogenkraft am Modell (stimmeBogen): schwingt an (+20 dB), ±15 Cent an f_1, Haften 5–98 %";
            for (std::size_t j = 0; j < noten.size(); ++j)
            {
                const double w = fenster[3 * j + stufe][static_cast<std::size_t>(q)];
                if (w > 0.0) k.punkte.push_back({noten[j], w});
            }
            (q == 0 ? s.bogenKraftUnten : s.bogenKraftOben)[stufe] = k;
        }
}

}  // namespace ic::stab
