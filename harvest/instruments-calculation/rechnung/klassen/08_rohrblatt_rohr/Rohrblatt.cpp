/**
 * @file Rohrblatt.cpp
 * @brief Klasse 8: der Ton eines Rohrblattinstruments (Herleitung im Kopf von `Rohrblatt.hpp`)
 */
#include "klassen/08_rohrblatt_rohr/Rohrblatt.hpp"

#include "gemeinsam/Werkzeug.hpp"
#include "messung/Bausteine.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic::rohrblatt
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr double kGueteMin = 5.0;  ///< Spitzen mit kleinerer Guete sind keine Moden (offen)
constexpr int kTeilung = 4;  // das Blatt rechnet viermal feiner (Anschlag an der Bahn)
constexpr double kAusblendenS = 0.005;

double radiusAn(const Profil& p, double x)
{
    if (p.punkte.empty()) return 0.0;
    if (x <= p.punkte.front().first) return p.punkte.front().second;
    for (std::size_t i = 1; i < p.punkte.size(); ++i)
        if (x <= p.punkte[i].first)
        {
            const auto& [x0, r0] = p.punkte[i - 1];
            const auto& [x1, r1] = p.punkte[i];
            return r0 + (r1 - r0) * (x - x0) / std::max(1e-12, x1 - x0);
        }
    return p.punkte.back().second;
}

/// Die Impedanz mit dem Mundstueck parallel: 1/(iωC + 1/Z).
Komplex zMit(const RohrblattSpiel& s, const Bohrung& b, double hz)
{
    const Luft luft;
    const Komplex z = eingangsImpedanz(b, hz, luft);
    if (s.profil.mundstueckM3 <= 0.0) return z;
    const double c = s.profil.mundstueckM3 / (luft.dichte * luft.schall * luft.schall);
    return 1.0 / (Komplex(0.0, 2.0 * kPi * hz * c) + 1.0 / z);
}

/// p_Abstrahlung/p_Eingang bei f: aus der Kettenmatrix, U_Ende = U_Ein/(c·Z_L + d), p = iωρ/(4π)·U_Ende.
Komplex abstrahlJeEingang(const RohrblattSpiel& s, const Bohrung& b, double hz)
{
    const Luft luft;
    const Kettenmatrix k = kettenmatrix(b, hz, luft);
    const Komplex zl = abschlussImpedanz(b, hz, luft);
    const Komplex uEnde = 1.0 / (k.c * zl + k.d);
    const Komplex z = zMit(s, b, hz);
    // ein Teil des Eingangsflusses fliesst in das Mundstueck: U_Bohrung = p/Z_Bohrung
    const Komplex zb = eingangsImpedanz(b, hz, luft);
    const Komplex anteil = z / zb;
    return Komplex(0.0, 2.0 * kPi * hz) * luft.dichte / (4.0 * kPi) * uEnde * anteil / z;
}

}  // namespace

Bohrung bohrungBis(const RohrblattSpiel& s, double laengeM)
{
    Bohrung b;
    const auto& p = s.profil.punkte;
    double x = 0.0;
    for (std::size_t i = 1; i < p.size() && x < laengeM; ++i)
    {
        const double ende = std::min(p[i].first, laengeM);
        if (ende > x) b.abschnitte.push_back({ende - x, radiusAn(s.profil, x), radiusAn(s.profil, ende)});
        x = ende;
    }
    if (x < laengeM && !p.empty()) b.abschnitte.push_back({laengeM - x, p.back().second, p.back().second});
    b.ende = Abschluss::Offen;
    // hinter der wirksamen Laenge die offenen Tonloecher (nicht beim laengsten Griff: dort strahlt der Becher)
    if (!p.empty() && laengeM < 0.98 * p.back().first) b.gitterGrenzeHz = s.gitterGrenzeHz;
    return b;
}

std::vector<ImpedanzMode> grifModen(const RohrblattSpiel& s, double laengeM, bool zweitesRegister)
{
    const Bohrung b = bohrungBis(s, laengeM);
    std::vector<ImpedanzMode> m =
        modenAusBetrag([&](double f) { return std::abs(zMit(s, b, f)); }, 20.0, 9000.0, 2.0, static_cast<std::size_t>(std::max(1, s.moden)));
    if (static_cast<int>(m.size()) > s.moden) m.resize(static_cast<std::size_t>(s.moden));
    // die breiten Spitzen ueber der Grenze des Gitters (Q < kGueteMin) nicht als Moden: ihre Flanken verfaelschten die
    // Summe zwischen den Spitzen (gerade Teiltoene der Klarinette — Befund IC Session 4)
    m.erase(std::remove_if(m.begin() + 1, m.end(), [](const ImpedanzMode& x) { return x.q < kGueteMin; }), m.end());
    if (zweitesRegister && !m.empty()) m.front().spitze *= s.registerDaempfung;
    return m;
}

void rechneRohrblattTon(const RohrblattSpiel& s, const RohrblattAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                        RohrblattMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0 || a.kette.empty() || !s.spielbar) return;
    const double fs = a.rate;
    const double dt = 1.0 / fs;
    const Luft luft;
    // je Note der Kette die Moden und die Gewichte der Abstrahlung
    std::vector<std::vector<ImpedanzMode>> griffe;
    std::vector<std::vector<Komplex>> gewichte;
    for (const KettenNote& k : a.kette)
    {
        const int n = std::clamp(k.note, s.noteMin, s.noteMax);
        const double l = wertAn(s.laengeJeNote, n, Skala::Logarithmisch);
        const bool zweites = n >= s.registerAb;
        griffe.push_back(grifModen(s, l, zweites));
        // die Gewichte der Abstrahlung ohne das Gitter: mit ihm wuchsen die Gewichte der hohen Moden, und deren Flanken
        // neben der Spitze gaben der Klarinette G3 gerade Teiltoene +11 dB ueber H1 (Befund IC Session 4); die Abstrahlung
        // durch die offenen Loecher oberhalb der Grenze ist damit unterschaetzt (offen.md)
        Bohrung b = bohrungBis(s, l);
        b.gitterGrenzeHz = 0.0;
        std::vector<Komplex> g;
        for (const ImpedanzMode& m : griffe.back()) g.push_back(abstrahlJeEingang(s, b, m.hz));
        gewichte.push_back(g);
    }
    RohrKern kern(griffe.front(), fs);
    // der Vokaltrakt fuer den Bend: eine Resonanz unter der Note (nur abwaerts, gekappt)
    const double bend = std::clamp(a.pitchCent, -s.bendGrenzeCent, 0.0);
    RohrKern trakt({{a.kette.front().hz * std::exp2((bend - 50.0) / 1200.0), 10.0, bend < 0.0 ? s.vokalSpitzePa * (-bend / s.bendGrenzeCent) : 0.0}}, fs);
    const int n0 = std::clamp(a.kette.front().note, s.noteMin, s.noteMax);
    const double ps = s.schwelleJeNote.punkte.empty() ? 2000.0 : wertAn(s.schwelleJeNote, n0, Skala::Logarithmisch);
    const double oben = s.obergrenzeJeNote.punkte.empty() ? 1e300 : wertAn(s.obergrenzeJeNote, n0, Skala::Logarithmisch);
    const double von = 1.15 * ps, bis = std::max(von, std::min({s.druckMaxPa, s.weg * ps, oben}));
    const double druck0 = von * std::pow(bis / von, std::clamp(a.expression, 0.0, 1.0));
    const double sr = s.steife * s.ruheSpaltM / s.schliessdruckPa;  // S_r aus p_M = k·H/S_r
    const auto halten = static_cast<std::int64_t>(std::llround(a.haltenS * fs));
    double y = 0.0, v = 0.0;
    std::uint64_t zufall = 0x2545F4914F6CDD1Dull;
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    std::size_t aktiv = 0;
    double phaseV = 0.0, phaseF = 0.0;
    double pMin = 1e300, pMax = -1e300;
    const std::int64_t schwankungAb = std::min(frames, halten) * 4 / 5;
    for (std::int64_t n = 0; n < frames; ++n)
    {
        const double t = static_cast<double>(n) * dt;
        while (aktiv + 1 < a.kette.size() && t >= a.kette[aktiv + 1].abS)
        {
            ++aktiv;
            kern.blendeAuf(griffe[aktiv], static_cast<int>(0.01 * fs));  // Legato: die Moden ueberblendet (Maestre 2018)
        }
        // Druck: Zunge — sofort voll, die Zunge haelt; Atem — Anstieg 30 ms; Absetzen 30 ms
        double env = a.ansatz == Ansatz::Atem ? std::min(1.0, t / 0.03) : std::min(1.0, t / 0.002);
        if (n >= halten) env *= std::max(0.0, 1.0 - (t - a.haltenS) / 0.03);
        phaseV += 2.0 * kPi * a.vibratoHz * dt;
        // Vibrato ueber den Druck: ±(Cent/100)·10 % (die Abbildung Cent → Druck ist offen)
        const double druck = druck0 * env * (1.0 + 0.001 * a.vibratoCent * std::sin(phaseV));
        // die Zunge: haelt das Blatt bis zum Einsatz zu, gibt es in `zungeS` frei (Flatterzunge: periodisch)
        double zunge = -1.0;  // Lage der Zungenflaeche (y-Richtung); < 0: frei
        if (a.ansatz == Ansatz::Zunge && t < s.zungeS) zunge = s.ruheSpaltM * (1.0 - t / s.zungeS);
        if (a.ansatz == Ansatz::Flatterzunge)
        {
            phaseF += 2.0 * kPi * a.flatterHz * dt;
            const double w = 0.5 * (1.0 + std::cos(phaseF));
            zunge = w > 0.6 ? s.ruheSpaltM * (w - 0.6) / 0.4 : -1.0;
        }
        const double pv = kern.vorher();
        const double vv = trakt.vorher();
        const double d = kern.durchgriff() + trakt.durchgriff();
        double u = 0.0;
        for (int sub = 0; sub < kTeilung; ++sub)
        {
            const double h = s.ruheSpaltM - y;
            const double ka = s.breiteM * std::max(0.0, h) * std::sqrt(2.0 / luft.dichte);
            // der Fluss des Blatts S_r·ẏ (y zur Bahn hin: es schiebt Luft ins Mundstueck), mit der Schnelle des Teilschritts
            const double ub = sr * v;
            const double x = druck - vv - pv - d * ub;
            double us = 0.0;
            if (ka > 0.0)
            {
                const double a2 = ka * ka;
                us = x >= 0.0 ? 0.5 * (-a2 * d + std::sqrt(a2 * a2 * d * d + 4.0 * a2 * x))
                              : -0.5 * (-a2 * d + std::sqrt(a2 * a2 * d * d - 4.0 * a2 * x));
            }
            const double dp = x - d * us;  // = P − p_Mundstueck
            double f = sr * dp - s.steife * y - s.masseKg * s.daempfung * v;
            if (h < 0.0) f -= s.bahnSteife * h * h * std::max(0.0, 1.0 + s.bahnDaempfung * v);  // die Bahn
            if (zunge >= 0.0 && s.ruheSpaltM - zunge < h)  // die Zunge drueckt das Blatt zur Bahn
            {
                const double tief = h - (s.ruheSpaltM - zunge);
                f += s.zungeSteife * tief;  // linear (der Exponent der Quelle ist offen)
            }
            v += f / s.masseKg * dt / kTeilung;
            y += v * dt / kTeilung;
            u += (us + ub) / kTeilung;
        }
        if (n >= schwankungAb && n < halten)
        {
            pMin = std::min(pMin, pv + d * u);
            pMax = std::max(pMax, pv + d * u);
        }
        double aus = kern.ausgang(gewichte[aktiv]);
        // Atemgeraeusch: Rauschen mal Spalt (nicht physikalisch, offen)
        zufall ^= zufall >> 12;
        zufall ^= zufall << 25;
        zufall ^= zufall >> 27;
        const double r = static_cast<double>((zufall * 0x2545F4914F6CDD1Dull) >> 11) / 9007199254740992.0 - 0.5;
        aus += s.atemPegel * r * druck * std::max(0.0, s.ruheSpaltM - y) / s.ruheSpaltM;
        kern.nachher(u);
        trakt.nachher(u);
        summe[static_cast<std::size_t>(n)] = aus;
    }
    if (messung != nullptr)
    {
        messung->schwankung = pMax > pMin ? (pMax - pMin) / std::max(1.0, druck0) : 0.0;
        const auto ab = static_cast<std::size_t>(std::max(a.kette.back().abS + 0.1, 0.5 * a.haltenS) * fs);
        const auto ende = static_cast<std::size_t>(std::min<std::int64_t>(frames, halten));
        if (ende > ab + 4800)
        {
            std::vector<double> stueck(summe.begin() + static_cast<std::ptrdiff_t>(ab), summe.begin() + static_cast<std::ptrdiff_t>(ende));
            const messung::Tonhoehe th = messung::tonhoehe(stueck, a.rate, 40.0, 2500.0);
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
        // Anstieg der Huelle (10 ms) im Einsatz
        const auto zehn = static_cast<std::size_t>(0.01 * fs);
        double vorher = -300.0;
        for (std::size_t i = 0; i + zehn < std::min<std::size_t>(summe.size(), static_cast<std::size_t>(0.3 * fs)); i += zehn)
        {
            double e = 0.0;
            for (std::size_t j = 0; j < zehn; ++j) e += summe[i + j] * summe[i + j];
            const double db = 10.0 * std::log10(std::max(1e-300, e / zehn));
            if (vorher > -250.0) messung->anstiegDbJeS = std::max(messung->anstiegDbJeS, (db - vorher) / 0.01);
            vorher = db;
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

void stimme(RohrblattSpiel& s)
{
    if (!s.spielbar) return;
    const int anzahl = s.noteMax - s.noteMin + 1;
    std::vector<double> laengen(static_cast<std::size_t>(anzahl)), unten(laengen.size()), oben(laengen.size());
    const double lMax = s.profil.punkte.back().first;
    // je Note unabhaengig (deterministisch auf Threads: jede Note schreibt nur ihre Werte)
    parallelJe(laengen.size(), [&](std::size_t i) {
        const int note = s.noteMin + static_cast<int>(i);
        const double f = 440.0 * std::exp2((note - 69) / 12.0);
        const bool zweites = note >= s.registerAb;
        // die Spitze `registerMode` (bzw. 1) des Griffs auf f: Halbierung in der Laenge
        double lo = 0.05, hi = lMax;
        for (int k = 0; k < 30; ++k)
        {
            const double l = 0.5 * (lo + hi);
            const Bohrung b = bohrungBis(s, l);
            const auto m = modenAusBetrag([&](double x) { return std::abs(zMit(s, b, x)); }, 20.0, 1.3 * f, std::max(0.2, f * 2e-3));
            const std::size_t idx = static_cast<std::size_t>((zweites ? s.registerMode : 1) - 1);
            const double fm = m.size() > idx ? m[idx].hz : 1e9;
            if (fm > f)
                lo = l;
            else
                hi = l;
        }
        double l = 0.5 * (lo + hi);
        // Das Modell ist mehrstabil (wie das Instrument, Dalmont et al. — nur Bericht): je nach Druck spielt eine andere
        // Mode. Gesucht ist das Fenster des Drucks, in dem die Note ihr Register spielt — der laengste zusammenhaengende
        // Lauf auf einem Raster von 2^(1/4) zwischen 300 Pa und 1,5·p_M; die Spieler bleiben darin (Befund IC Session 4:
        // die Halbierung nach einer Schwelle fand bei den Saxophonen keine, weil ueber ihr wieder die Oktave sprang).
        RohrblattSpiel t = s;
        t.druckMaxPa = 1e9;
        t.obergrenzeJeNote = {};
        const auto spielt = [&](double p, double laenge) {
            t.laengeJeNote = kurveAusPunkten({{note, laenge}});
            t.schwelleJeNote = kurveAusPunkten({{note, p / 1.15}});
            RohrblattAnschlag a;
            a.kette = {{0.0, note, f}};
            a.expression = 0.0;
            a.haltenS = 0.6;  // so lange wie die Messung der Tests (nach 0,4 s sprang manche Note noch)
            RohrblattMessung mm;
            std::vector<float> ton;
            rechneRohrblattTon(t, a, ton, static_cast<std::int64_t>(0.6 * a.rate), &mm);
            return mm;
        };
        const auto fenster = [&](double laenge) {
            std::pair<double, double> best{0.0, 0.0};
            int bestLauf = 0, lauf = 0;
            double anfang = 0.0;
            for (double p = 300.0; p <= 1.5 * s.schliessdruckPa; p *= std::exp2(0.25))
            {
                const RohrblattMessung mm = spielt(p, laenge);
                if (mm.schwankung >= 0.3 && mm.schwingt && std::abs(1200.0 * std::log2(mm.spielHz / f)) < 100.0)
                {
                    if (lauf++ == 0) anfang = p;
                    if (lauf > bestLauf)
                    {
                        bestLauf = lauf;
                        best = {anfang, p};
                    }
                }
                else
                    lauf = 0;
            }
            // weg von den Raendern (dort sprang der Ton bei laengerem Halten noch): je ein Schritt, wenn das Fenster
            // mindestens vier Punkte hat
            const double schritt = std::exp2(0.25);
            if (best.second >= best.first * schritt * schritt * schritt * 0.999)
            {
                best.first *= schritt;
                best.second /= schritt;
            }
            return best;
        };
        std::pair<double, double> w = fenster(l);
        // Feinstimmung der Laenge in der Mitte des Fensters (f ∝ 1/L genaehert), dann das Fenster neu
        if (w.first > 0.0)
        {
            const double mitte = std::sqrt(w.first * w.second);
            for (int runde = 0; runde < 4; ++runde)
            {
                const RohrblattMessung mm = spielt(mitte, l);
                if (!mm.schwingt) break;
                const double fehler = 1200.0 * std::log2(mm.spielHz / f);
                if (std::abs(fehler) < 3.0 || std::abs(fehler) > 400.0) break;
                l = std::clamp(l * mm.spielHz / f, 0.05, lMax);
            }
            const std::pair<double, double> w2 = fenster(l);
            if (w2.first > 0.0) w = w2;
        }
        laengen[i] = l;
        unten[i] = w.first > 0.0 ? w.first / 1.15 : 0.0;
        oben[i] = w.first > 0.0 ? w.second : 0.0;
    });
    Kurve laenge, schwelle, obergrenze;
    laenge.belegkraft = schwelle.belegkraft = obergrenze.belegkraft = Belegkraft::Rechnung;
    laenge.herkunft = "wirksame Laenge je Griff: die Spitze des Registers auf der Note, dann die Spielfrequenz durch das Modell (stimme)";
    schwelle.herkunft = "unterer Rand des Fensters (/1,15), in dem die Note ihr Register spielt: Wechseldruck ≥ 30 % des Blasdrucks, ±100 Cent (stimme)";
    obergrenze.herkunft = "oberer Rand dieses Fensters (stimme)";
    for (std::size_t i = 0; i < laengen.size(); ++i)
    {
        const int note = s.noteMin + static_cast<int>(i);
        laenge.punkte.push_back({note, laengen[i]});
        // ohne Fenster: kein Punkt (die Nachbarn tragen; offen.md)
        if (unten[i] > 0.0)
        {
            schwelle.punkte.push_back({note, unten[i]});
            obergrenze.punkte.push_back({note, oben[i]});
        }
    }
    s.laengeJeNote = laenge;
    s.schwelleJeNote = schwelle;
    s.obergrenzeJeNote = obergrenze;
}

}  // namespace ic::rohrblatt
