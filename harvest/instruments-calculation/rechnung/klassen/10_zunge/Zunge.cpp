/**
 * @file Zunge.cpp
 * @brief Klasse 10: der Ton einer durchschlagenden Zunge (Herleitung im Kopf von `Zunge.hpp`)
 */
#include "klassen/10_zunge/Zunge.hpp"

#include "gemeinsam/Werkzeug.hpp"
#include "messung/Bausteine.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace ic::zunge
{

namespace
{

constexpr double kPi = std::numbers::pi;
constexpr double kRho = 1.2041, kC = 343.0;
constexpr int kUeber = 4;

double hzVon(double note)
{
    return 440.0 * std::exp2((note - 69.0) / 12.0);
}

}  // namespace

void rechneEineZunge(const ZungenSpiel& s, double f0, double noteHz, const ZungenAnschlag& a, std::vector<double>& aus, std::int64_t frames,
                     ZungenMessung* messung)
{
    aus.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0);
    if (frames <= 0 || f0 <= 0.0) return;
    const double fs = static_cast<double>(a.rate) * kUeber, dt = 1.0 / fs;
    // die Zunge zu dieser Note: Laenge ∝ 1/√f (bei gleicher Dicke, f ∝ h/L² — Rechnung), Masse ∝ Laenge
    const double laenge = s.laengeC4M * std::sqrt(hzVon(60.0) / noteHz);
    const double masse = s.masseC4Kg * laenge / s.laengeC4M;
    const double w0 = 2.0 * kPi * f0;
    const double steife = masse * w0 * w0, daempfung = 2.0 * s.zeta * masse * w0;
    const double sr = 0.5 * laenge * s.breiteM;  // wirksame Flaeche fuer die Kraft (offen)
    const double e = std::clamp(a.expression, 0.0, 1.0);
    double p0 = s.druckPpPa * std::pow(s.druckFfPa / s.druckPpPa, e);
    if (!s.druckUntenJeNote.punkte.empty())
    {
        const int n0 = std::clamp(static_cast<int>(std::lround(69.0 + 12.0 * std::log2(noteHz / 440.0))), s.noteMin, s.noteMax);
        const double u = wertAn(s.druckUntenJeNote, n0, Skala::Logarithmisch);
        const double o = std::max(u, wertAn(s.druckObenJeNote, n0, Skala::Logarithmisch));
        p0 = u * std::pow(o / u, e);
    }
    if (a.druckPa > 0.0) p0 = a.druckPa;
    const double bend = std::clamp(a.pitchCent, -s.bendGrenzeCent, 0.0);
    // das Ventil: der Bend schliesst es (Adachi: kleinere Oeffnung → tiefer); Abbildung Cent → Oeffnung offen
    // (−100 Cent → 1 % der Oeffnung); was die Physik nicht hergibt, faellt weg (F-2 a: das Modell kappt)
    const double ventil = s.ventilFlaecheM2 * std::pow(10.0, 2.0 * bend / 100.0);
    const double traegheit = kRho * s.kanalLaengeM / s.kanalFlaecheM2;
    const double kKanz = kRho * kC * kC / s.kanzelleM3;
    const auto halten = static_cast<std::int64_t>(std::llround(a.haltenS * a.rate)) * kUeber;
    double y = 0.0, v = 0.0, p1 = 0.0, u = 0.0, uVor = 0.0, phaseV = 0.0;
    double yMax = 0.0;
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    for (std::int64_t n = 0; n < frames * kUeber; ++n)
    {
        const double t = static_cast<double>(n) * dt;
        // der Wind: Anstieg 20 ms, Absetzen 30 ms (offen), Vibrato, Balgwechsel (Einbruch 60 ms, offen)
        double env = std::min(1.0, t / 0.02);
        if (n >= halten) env *= std::max(0.0, 1.0 - (t - a.haltenS) / 0.03);
        if (a.balgwechselS >= 0.0 && t >= a.balgwechselS && t < a.balgwechselS + 0.06)
            env *= 0.2 + 0.8 * std::abs(std::cos(kPi * (t - a.balgwechselS) / 0.06));
        phaseV += 2.0 * kPi * a.vibratoHz * dt;
        const double p = p0 * env * (1.0 + 0.002 * a.vibratoCent * std::sin(phaseV));
        // das Ventil (Bernoulli): Fluss aus dem Balg in die Kanzelle
        const double dpv = p - p1;
        const double uEin = ventil * std::sqrt(2.0 * std::abs(dpv) / kRho) * (dpv >= 0.0 ? 1.0 : -1.0);
        // der Schlitz: Durchflussflaeche und traege Stroemung
        const double su = s.breiteM * std::sqrt((y + s.ruheM) * (y + s.ruheM) + s.seitenspaltM * s.seitenspaltM);
        // der Druck an der Zunge (hinter dem traegen Kanal): der Strahl durch den Schlitz, p₂ = ½ρ(U/S_u)²; U geschlossen
        // geloest (implizit): T·(U′ − U)/Δt = p₁ − c·U′|U′| mit c = ½ρ/S_u² — sonst wurde es bei kleiner Traegheit instabil
        const double c = 0.5 * kRho / (su * su);
        const double rhs = u + dt * p1 / traegheit, g = dt * c / traegheit;
        u = rhs >= 0.0 ? (-1.0 + std::sqrt(1.0 + 4.0 * g * rhs)) / (2.0 * g) : (1.0 - std::sqrt(1.0 - 4.0 * g * rhs)) / (2.0 * g);
        const double p2 = c * u * std::abs(u);
        // die Kanzelle: Masse erhalten; schlaegt die Zunge nach aussen durch (v < 0), waechst das Volumen der Kanzelle
        // um S_r·|v| und ihr Druck faellt (mit dem anderen Vorzeichen stieg die Tonhoehe beim Schliessen des Ventils —
        // Befund IC Session 4; Adachi: die wirksame Steife sinkt)
        p1 += dt * kKanz * (uEin - u + sr * v);
        // die Zunge (symplektisch): y zum Schlitz hin negativ — p₂ drueckt sie hinein
        const double kraft = -sr * p2 - steife * y - daempfung * v;
        v += dt * kraft / masse;
        y += dt * v;
        if (n > halten / 2 && n < halten) yMax = std::max(yMax, std::abs(y));
        if (n % kUeber == kUeber - 1)
        {
            const auto i = static_cast<std::size_t>(n / kUeber);
            summe[i] = kRho / (4.0 * kPi) * (u - uVor) * a.rate;
            uVor = u;
        }
    }
    aus = summe;
    if (messung != nullptr)
    {
        messung->amplitudeRel = yMax / laenge;
        const auto haltenF = static_cast<std::size_t>(std::min<std::int64_t>(frames, halten / kUeber));
        const auto ab = static_cast<std::size_t>(0.5 * a.haltenS * a.rate);
        if (haltenF > ab + 4800)
        {
            std::vector<double> stueck(summe.begin() + static_cast<std::ptrdiff_t>(ab), summe.begin() + static_cast<std::ptrdiff_t>(haltenF));
            const messung::Tonhoehe th = messung::tonhoehe(stueck, a.rate, 30.0, 3000.0);
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
        std::vector<double> alles(summe.begin(), summe.begin() + static_cast<std::ptrdiff_t>(haltenF));
        messung->einschwingS = messung::einschwingen(alles, a.rate);
    }
}

void rechneZungenTon(const ZungenSpiel& s, const ZungenAnschlag& a, std::vector<float>& mono, std::int64_t frames, ZungenMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0) return;
    const bool bass = s.bassAb > 0 && a.note < s.bassAb;
    const std::vector<Chor>& choere = bass ? s.bassChoere : s.choere;
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0), ton;
    bool erster = true;
    for (const Chor& c : choere)
    {
        const double note = std::clamp(a.note + c.halbtoene, s.noteMin - 24, s.noteMax + 24);
        const double ziel = hzVon(note) + c.schwebungA4Hz * std::sqrt(hzVon(note) / 440.0);
        // f₀ aus der Stimmung der Note, um die Schwebung verschoben (offen: f₀ ∝ Spielfrequenz in der Naehe)
        const int n0 = std::clamp(static_cast<int>(note), s.noteMin, s.noteMax);
        const double verh = s.eigenfrequenzJeNote.punkte.empty() ? 1.02 : wertAn(s.eigenfrequenzJeNote, n0, Skala::Logarithmisch) / hzVon(n0);
        ZungenMessung m;
        rechneEineZunge(s, ziel * verh, ziel, a, ton, frames, erster && messung != nullptr ? &m : nullptr);
        if (erster && messung != nullptr) *messung = m;
        erster = false;
        for (std::size_t i = 0; i < summe.size(); ++i) summe[i] += c.pegel * ton[i];
    }
    // Gehaeuse (Harmonium, Cassotto): ein Tiefpass erster Ordnung (offen — DAFx23: 10 Biquads, Werte nicht da)
    if (s.gehaeuseTiefpassHz > 0.0)
    {
        const double k = 1.0 - std::exp(-2.0 * kPi * s.gehaeuseTiefpassHz / a.rate);
        double z = 0.0;
        for (double& x : summe) x = (z += k * (x - z));
    }
    const auto blende = std::min<std::int64_t>(frames, std::max<std::int64_t>(1, std::llround(0.005 * a.rate)));
    for (std::int64_t i = 0; i < frames; ++i)
    {
        double w = summe[static_cast<std::size_t>(i)] / s.ausgangBezug;
        const std::int64_t rest = frames - 1 - i;
        if (rest < blende) w *= static_cast<double>(rest) / static_cast<double>(blende);
        mono[static_cast<std::size_t>(i)] = static_cast<float>(w);
    }
}

void stimme(ZungenSpiel& s)
{
    const std::size_t anzahl = static_cast<std::size_t>(s.noteMax - s.noteMin + 1);
    std::vector<double> f0(anzahl, 0.0), unten(anzahl, 0.0), oben(anzahl, 0.0);
    ZungenSpiel t = s;
    t.druckUntenJeNote = {};
    t.druckObenJeNote = {};
    parallelJe(anzahl, [&](std::size_t i) {
        const int note = s.noteMin + static_cast<int>(i);
        const double f = hzVon(note);
        const auto spielt = [&](double druck, double x) {
            ZungenAnschlag a;
            a.note = note;
            a.haltenS = 0.5;
            a.druckPa = druck;
            ZungenMessung m;
            std::vector<double> ton;
            rechneEineZunge(t, x, f, a, ton, static_cast<std::int64_t>(0.5 * a.rate), &m);
            return m;
        };
        // das Fenster: Raster 2^(1/4) von 100 Pa bis 6 kPa, die Zunge spricht an (Amplitude ≥ 2 %, Tonhoehe ±100 Cent
        // an f₀), der laengste Lauf ohne die Raender
        double x = 1.02 * f;
        const double schritt = std::exp2(0.25);
        std::pair<double, double> best{0.0, 0.0};
        int bestLauf = 0, lauf = 0;
        double anfang = 0.0;
        for (double p = 100.0; p <= 6000.0; p *= schritt)
        {
            const ZungenMessung m = spielt(p, x);
            if (m.schwingt && m.amplitudeRel >= 0.02 && std::abs(1200.0 * std::log2(m.spielHz / x)) < 100.0)
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
        if (best.second >= best.first * schritt * schritt * schritt * 0.999)
        {
            best.first *= schritt;
            best.second /= schritt;
        }
        if (best.first > 0.0)
        {
            const double mitte = std::sqrt(best.first * best.second);
            for (int runde = 0; runde < 6; ++runde)
            {
                const ZungenMessung m = spielt(mitte, x);
                if (!m.schwingt) break;
                const double fehler = 1200.0 * std::log2(m.spielHz / f);
                if (std::abs(fehler) < 1.0 || std::abs(fehler) > 600.0) break;
                x *= f / m.spielHz;
            }
        }
        f0[i] = x;
        unten[i] = best.first;
        oben[i] = best.second;
    });
    Kurve k, ku, ko;
    k.belegkraft = ku.belegkraft = ko.belegkraft = Belegkraft::Rechnung;
    k.herkunft = "f₀ der Zunge je Note: die Spielfrequenz in der Mitte des Fensters auf der Note (stimme)";
    ku.herkunft = "unterer Rand des Drucks, bei dem die Zunge anspricht (Amplitude ≥ 2 %, ohne Rand; stimme)";
    ko.herkunft = "oberer Rand dieses Fensters (stimme)";
    for (std::size_t i = 0; i < anzahl; ++i)
    {
        const int note = s.noteMin + static_cast<int>(i);
        k.punkte.push_back({note, f0[i]});
        if (unten[i] > 0.0)
        {
            ku.punkte.push_back({note, unten[i]});
            ko.punkte.push_back({note, oben[i]});
        }
    }
    s.eigenfrequenzJeNote = k;
    s.druckUntenJeNote = ku;
    s.druckObenJeNote = ko;
}

}  // namespace ic::zunge
