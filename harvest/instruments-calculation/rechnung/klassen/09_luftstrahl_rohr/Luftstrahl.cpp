/**
 * @file Luftstrahl.cpp
 * @brief Klasse 9: der Ton einer Floete oder Labialpfeife (Herleitung im Kopf von `Luftstrahl.hpp`)
 */
#include "klassen/09_luftstrahl_rohr/Luftstrahl.hpp"

#include "gemeinsam/Werkzeug.hpp"
#include "messung/Bausteine.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic::luftstrahl
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr double kAusblendenS = 0.005;

double radiusAn(const StrahlSpiel& s, int note)
{
    return s.radiusJeNote.punkte.empty() ? s.radiusM : wertAn(s.radiusJeNote, note, Skala::Logarithmisch);
}

Bohrung rohr(const StrahlSpiel& s, int note, double laenge)
{
    Bohrung b;
    const double r = radiusAn(s, note);
    b.abschnitte = {{laenge, r, r}};
    b.ende = Abschluss::Offen;
    return b;
}

std::vector<ImpedanzMode> modenBei(const StrahlSpiel& s, int note, double laenge, std::size_t anzahl)
{
    const Bohrung b = rohr(s, note, laenge);
    const double f1 = 343.0 / (2.0 * laenge);
    return modenAusBetrag([&](double f) { return 1.0 / std::abs(eingangsImpedanz(b, f)); }, 0.3 * f1, 0.45 * 48000.0,
                          std::max(0.5, f1 * 2e-3), anzahl);
}

struct Zufall
{
    std::uint64_t s = 0x9E3779B97F4A7C15ull;
    double naechste()
    {
        s ^= s >> 12;
        s ^= s << 25;
        s ^= s >> 27;
        return static_cast<double>((s * 0x2545F4914F6CDD1Dull) >> 11) / 9007199254740992.0 - 0.5;
    }
};

}  // namespace

std::vector<ImpedanzMode> rohrModen(const StrahlSpiel& s, int note)
{
    const int n = std::clamp(note, s.noteMin, s.noteMax);
    return modenBei(s, n, wertAn(s.laengeJeNote, n, Skala::Logarithmisch), static_cast<std::size_t>(s.moden));
}

void rechneStrahlTon(const StrahlSpiel& s, const StrahlAnschlag& a, std::vector<float>& mono, std::int64_t frames, StrahlMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0 || a.kette.empty() || !s.spielbar || s.laengeJeNote.punkte.empty()) return;
    const double fs = a.rate;
    const double dt = 1.0 / fs;
    const double rho = 1.2041;
    std::vector<std::vector<ImpedanzMode>> griffe;
    for (const KettenNote& k : a.kette) griffe.push_back(rohrModen(s, k.note));
    RohrKern kern(griffe.front(), fs);
    const double e = std::clamp(a.expression, 0.0, 1.0);
    // Die Laufzeit des Strahls im Verhaeltnis zur Periode bestimmt das Register (Fletcher & Douglas S. 767): bei festem
    // Verhaeltnis κ haelt die Note ihr Register. Floeten: der Druck je Note, P = ρ/2·(W/(c_p/U_j·κ·T))², Expression
    // als Weg um diese Mitte (×2^(e − 0,5), T3a a); Orgel: fester Wind, der Aufschnitt W je Pfeife (die Intonation)
    // und Expression als Schweller.
    const double periode = 1.0 / a.kette.front().hz;
    double abstand = s.abstandM;
    double druck0 = 0.0;
    const int n0 = std::clamp(a.kette.front().note, s.noteMin, s.noteMax);
    if (s.orgel)
    {
        druck0 = s.druckMaxPa;
        abstand = !s.aufschnittJeNote.punkte.empty() ? wertAn(s.aufschnittJeNote, n0, Skala::Logarithmisch)
                                                     : s.cpJeUj * std::sqrt(2.0 * druck0 / rho) * s.laufzeitJePeriode * periode;
    }
    else if (!s.druckUntenJeNote.punkte.empty())
    {
        const double unten = wertAn(s.druckUntenJeNote, n0, Skala::Logarithmisch);
        const double oben = std::max(unten, wertAn(s.druckObenJeNote, n0, Skala::Logarithmisch));
        druck0 = unten * std::pow(oben / unten, e);
    }
    else
    {
        const double uMitte = s.abstandM / (s.cpJeUj * s.laufzeitJePeriode * periode);
        druck0 = std::clamp(0.5 * rho * uMitte * uMitte * std::exp2(e - 0.5), s.druckMinPa, s.druckMaxPa);
    }
    double mundBreite = s.mundBreiteM, mundFlaeche = s.mundFlaecheM2;
    if (s.mundJeUmfang > 0.0)
    {
        mundBreite = s.mundJeUmfang * 2.0 * kPi * radiusAn(s, n0);
        mundFlaeche = mundBreite * abstand;
    }
    const double b = 0.4 * s.kanalHoeheM;
    const double deltaD = 4.0 / kPi * std::sqrt(2.0 * s.kanalHoeheM * abstand);
    const auto halten = static_cast<std::int64_t>(std::llround(a.haltenS * fs));
    // der Ring der Schnelle am Mund (fuer die Laufzeit des Strahls)
    std::vector<double> ring(1 << 14, 0.0);
    std::size_t pos = 0;
    double qVor = 0.0, uVor = 0.0, etaTp = 0.0;
    Zufall z;
    double phaseV = 0.0, phaseE = 0.0;
    // der Schweller: Pegel und ein Tiefpass erster Ordnung (offen)
    const double schwellPegel = s.orgel ? 0.1 + 0.9 * e : 1.0;
    const double fc = s.orgel ? 2000.0 + 10000.0 * e : 1e9;
    const double tp = 1.0 - std::exp(-2.0 * kPi * fc / fs);
    double tpZ = 0.0;
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    std::size_t aktiv = 0;
    for (std::int64_t n = 0; n < frames; ++n)
    {
        const double t = static_cast<double>(n) * dt;
        while (aktiv + 1 < a.kette.size() && t >= a.kette[aktiv + 1].abS)
        {
            ++aktiv;
            kern.blendeAuf(griffe[aktiv], static_cast<int>(0.01 * fs));
        }
        double env = std::min(1.0, t / std::max(1e-4, s.anblasS));
        if (n >= halten) env *= std::max(0.0, 1.0 - (t - a.haltenS) / 0.03);
        phaseV += 2.0 * kPi * a.vibratoHz * dt;
        double mod = 1.0;
        if (s.orgel)
            mod = 1.0 + s.tremulantTiefe * std::clamp(a.vibratoCent / 100.0, 0.0, 1.0) * std::sin(phaseV);
        else
            mod = 1.0 + 0.002 * a.vibratoCent * std::sin(phaseV);  // Druckmodulation (Abbildung Cent → Druck offen)
        const double druck = std::max(0.0, druck0 * env * mod);
        const double uj = std::sqrt(2.0 * druck / rho);
        // die Laufzeit des Strahls und die Auslenkung am Labium
        const double tau = uj > 0.1 ? abstand / (s.cpJeUj * uj) : 0.0;
        const double lesen = static_cast<double>(pos) + ring.size() - tau * fs;
        const auto i0 = static_cast<std::size_t>(std::floor(lesen));
        const double fr = lesen - std::floor(lesen);
        const double vAlt = (1.0 - fr) * ring[i0 % ring.size()] + fr * ring[(i0 + 1) % ring.size()];
        const double eta = uj > 0.1 ? s.kanalHoeheM / uj * vAlt * std::exp(s.alphaI * abstand) + 1e-7 * z.naechste() : 0.0;
        // die Antwort des Strahls faellt bei hohen Frequenzen ab: erster Ordnung bei f_j = 0,1·U_j/W (offen; ohne sie
        // schwang das Modell auf Moden um 3 kHz — Befund IC Session 4)
        const double wj = 2.0 * kPi * s.strahlTiefpass * std::max(uj, 0.1) / abstand;
        etaTp += (1.0 - std::exp(-wj * dt)) * (eta - etaTp);
        const double q = b * mundBreite * uj * (1.0 + std::tanh((etaTp - s.versatzM) / b));
        const double dq = (q - qVor) * fs;
        qVor = q;
        // die Druckquelle am Mund, die Verluste der Abloesung (mit der Schnelle des vorigen Takts)
        const double vm = uVor / mundFlaeche;
        double dp = rho * deltaD / mundFlaeche * dq - rho / (2.0 * s.alphaVc * s.alphaVc) * vm * std::abs(vm);
        // Schneidenton (halbphysikalisch, T5 a): f_e = 0,3·U_j/W, abklingend im Einschwingen; Rauschen der Turbulenz
        if (a.schneidenton && uj > 0.1)
        {
            phaseE += 2.0 * kPi * 0.3 * uj / abstand * dt;
            dp += s.schneidentonPegel * druck * std::exp(-t / 0.03) * std::sin(phaseE);
        }
        dp += s.rauschPegel * druck * z.naechste();
        const double u = kern.vorher() + kern.durchgriff() * dp;
        kern.nachher(dp);
        ring[pos] = u / mundFlaeche;
        pos = (pos + 1) % ring.size();
        // abgestrahlt: Monopol ρ/(4π)·dU/dt am Mund (offen: das Ende gleich gewichtet, Vorzeichen −U)
        double aus = rho / (4.0 * kPi) * (u - uVor) * fs;
        uVor = u;
        tpZ += tp * (aus - tpZ);
        summe[static_cast<std::size_t>(n)] = schwellPegel * tpZ;
    }
    if (messung != nullptr)
    {
        const auto ab = static_cast<std::size_t>(std::max(a.kette.back().abS + 0.1, 0.5 * a.haltenS) * fs);
        const auto ende = static_cast<std::size_t>(std::min<std::int64_t>(frames, halten));
        if (ende > ab + 4800)
        {
            std::vector<double> stueck(summe.begin() + static_cast<std::ptrdiff_t>(ab), summe.begin() + static_cast<std::ptrdiff_t>(ende));
            const messung::Tonhoehe th = messung::tonhoehe(stueck, a.rate, 60.0, 4000.0);
            std::vector<double> hz;
            for (const double f : th.hz)
                if (f > 0.0) hz.push_back(f);
            if (hz.size() > th.hz.size() / 2)
            {
                std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
                messung->spielHz = hz[hz.size() / 2];
                messung->schwingt = true;
            }
        }
        std::vector<double> alles(summe.begin(), summe.begin() + static_cast<std::ptrdiff_t>(std::min<std::int64_t>(frames, halten)));
        messung->einschwingS = messung::einschwingen(alles, a.rate);
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

void stimme(StrahlSpiel& s)
{
    if (!s.spielbar) return;
    const std::size_t anzahl = static_cast<std::size_t>(s.noteMax - s.noteMin + 1);
    std::vector<double> laengen(anzahl), unten(anzahl, 0.0), oben(anzahl, 0.0), aufschnitt(anzahl, 0.0);
    std::vector<int> register_(anzahl, 1);
    const double rho = 1.2041;
    // je Note unabhaengig (deterministisch auf Threads: jede Note schreibt nur ihre Werte)
    parallelJe(anzahl, [&](std::size_t i) {
        const int note = s.noteMin + static_cast<int>(i);
        const double f = 440.0 * std::exp2((note - 69) / 12.0);
        StrahlSpiel t = s;
        // ein Ton: die Floete beim Druck x, die Orgel beim Aufschnitt x (fester Wind)
        const auto spielt = [&](double x, double laenge) {
            t.laengeJeNote = kurveAusPunkten({{note, laenge}});
            if (s.orgel)
                t.aufschnittJeNote = kurveAusPunkten({{note, x}});
            else
            {
                t.druckUntenJeNote = kurveAusPunkten({{note, x}});
                t.druckObenJeNote = kurveAusPunkten({{note, x}});
            }
            StrahlAnschlag a;
            a.kette = {{0.0, note, f}};
            a.expression = 0.5;
            a.haltenS = 0.6;
            a.schneidenton = false;
            StrahlMessung m;
            std::vector<float> ton;
            rechneStrahlTon(t, a, ton, static_cast<std::int64_t>(0.6 * a.rate), &m);
            return m;
        };
        // Das Fenster, in dem die Note auf ±100 Cent spielt: der laengste zusammenhaengende Lauf auf einem Raster von
        // 2^(1/4) — Floeten im Druck von pMin/2 bis 2·pMax, die Orgel im Aufschnitt um den Wert aus κ (×1/8 … ×8).
        // Mehr Druck bzw. ein kleinerer Aufschnitt verkuerzen die Laufzeit und heben das Register (Fletcher & Douglas
        // S. 767); ohne diese Suche sprang das Modell auf hoehere Moden (Befund IC Session 4).
        const double w0 = s.cpJeUj * std::sqrt(2.0 * s.druckMaxPa / rho) * s.laufzeitJePeriode / f;
        const double von = s.orgel ? w0 / 8.0 : 0.5 * s.druckMinPa;
        const double schritt = std::exp2(0.25);
        // ueberblasen braucht bei gleichem κ den Druck ×Mode² (U_j ∝ f): die Obergrenze der Suche mit
        const auto fenster = [&](double laenge, int mode) {
            const double bis = s.orgel ? 8.0 * w0 : 2.0 * s.druckMaxPa * mode * mode;
            std::pair<double, double> best{0.0, 0.0};
            int bestLauf = 0, lauf = 0;
            double anfang = 0.0;
            for (double x = von; x <= bis * 1.0001; x *= schritt)
            {
                const StrahlMessung m = spielt(x, laenge);
                if (m.schwingt && std::abs(1200.0 * std::log2(m.spielHz / f)) < 100.0)
                {
                    if (lauf++ == 0) anfang = x;
                    if (lauf > bestLauf)
                    {
                        bestLauf = lauf;
                        best = {anfang, x};
                    }
                }
                else
                    lauf = 0;
            }
            // weg von den Raendern (dort sprang der Ton bei laengerem Halten noch): je ein Schritt, wenn das Fenster
            // breit genug ist; die Floeten hoechstens ×2 um die Mitte (die Tonhoehe steigt mit dem Druck — Fletcher &
            // Douglas; die Spieler gleichen das mit dem Ansatz aus, der hier nicht gebaut ist; offen). Ein Fenster ganz
            // unter pMin ist nicht spielbar.
            if (best.second >= best.first * schritt * schritt * schritt)
            {
                best.first *= schritt;
                best.second /= schritt;
            }
            if (!s.orgel && best.second > 2.0 * best.first)
            {
                const double mitte = std::sqrt(best.first * best.second);
                best = {mitte / std::sqrt(2.0), mitte * std::sqrt(2.0)};
            }
            if (!s.orgel && best.second < s.druckMinPa) best = {0.0, 0.0};
            return best;
        };
        // erst das erste Register; ohne Fenster ueberblasen (die Spitze 2 auf der Note — die zweite Oktave der Floeten,
        // gegriffen wie die erste)
        double l = 0.0;
        std::pair<double, double> w{0.0, 0.0};
        for (int mode = 1; mode <= 2 && w.first <= 0.0; ++mode)
        {
            double lo = 0.02, hi = 4.0;
            for (int k = 0; k < 30; ++k)
            {
                const double lm = std::sqrt(lo * hi);
                const auto m = modenBei(s, note, lm, static_cast<std::size_t>(mode));
                if (static_cast<int>(m.size()) >= mode && m[static_cast<std::size_t>(mode - 1)].hz > f)
                    lo = lm;
                else
                    hi = lm;
            }
            l = std::sqrt(lo * hi);
            w = fenster(l, mode);
            if (w.first <= 0.0) continue;
            register_[i] = mode;
            // Feinstimmung der Laenge in der Mitte des Fensters (f ∝ 1/L genaehert), dann das Fenster neu — zweimal,
            // weil sich mit der Laenge die Mitte verschiebt
            for (int gang = 0; gang < 2; ++gang)
            {
                const double mitte = std::sqrt(w.first * w.second);
                for (int runde = 0; runde < 4; ++runde)
                {
                    const StrahlMessung m = spielt(mitte, l);
                    if (!m.schwingt) break;
                    const double fehler = 1200.0 * std::log2(m.spielHz / f);
                    if (std::abs(fehler) < 3.0 || std::abs(fehler) > 400.0) break;
                    l *= m.spielHz / f;
                }
                const std::pair<double, double> w2 = fenster(l, mode);
                if (w2.first > 0.0) w = w2;
            }
        }
        laengen[i] = l;
        if (w.first > 0.0)
        {
            if (s.orgel)
                aufschnitt[i] = std::sqrt(w.first * w.second);
            else
            {
                unten[i] = w.first;
                oben[i] = w.second;
            }
        }
    });
    Kurve laenge, du, dob, auf;
    laenge.belegkraft = du.belegkraft = dob.belegkraft = auf.belegkraft = Belegkraft::Rechnung;
    laenge.herkunft = "wirksame Laenge je Note: die Spitze des Registers (1, ohne Fenster 2) auf der Note, dann die Spielfrequenz durch das Modell (stimme)";
    du.herkunft = "unterer Rand des Drucks, in dem die Note spielt (±100 Cent, ohne die Raender, hoechstens ×2; stimme)";
    dob.herkunft = "oberer Rand dieses Fensters (stimme)";
    auf.herkunft = "Aufschnitt in der geometrischen Mitte des Fensters beim Wind des Werks (stimme)";
    for (std::size_t i = 0; i < anzahl; ++i)
    {
        const int note = s.noteMin + static_cast<int>(i);
        laenge.punkte.push_back({note, laengen[i]});
        // ohne Fenster: kein Punkt (die Nachbarn tragen; offen.md)
        if (unten[i] > 0.0)
        {
            du.punkte.push_back({note, unten[i]});
            dob.punkte.push_back({note, oben[i]});
        }
        if (aufschnitt[i] > 0.0) auf.punkte.push_back({note, aufschnitt[i]});
    }
    s.laengeJeNote = laenge;
    s.registerJeNote.clear();
    for (std::size_t i = 0; i < anzahl; ++i) s.registerJeNote.push_back(register_[i]);
    if (s.orgel)
        s.aufschnittJeNote = auf;
    else
    {
        s.druckUntenJeNote = du;
        s.druckObenJeNote = dob;
    }
}

}  // namespace ic::luftstrahl
