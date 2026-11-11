/**
 * @file Zunge.cpp
 * @brief InstrumentModels, Klasse 10: der Ton einer durchschlagenden Zunge (Herleitung und Herkunft im Kopf von
 *        `Zunge.hpp`)
 */
#include "InstrumentModels/zunge/Zunge.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace lumi::kern::zunge
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

double an(const Kurve& k, int note)
{
    return wertAn(k, note, Skala::Logarithmisch);
}

}  // namespace

ZungeAnDerTaste zungeAn(const ZungenSpiel& s, int zungenNote)
{
    ZungeAnDerTaste z;
    z.laenge = an(s.laengeM, zungenNote);
    z.breite = an(s.breiteM, zungenNote);
    // die Masse aus der Masse je wirksamer Flaeche (die Reihenfolge wie im Pruefstand S190)
    z.masse = an(s.flaechenmasse, zungenNote) * 0.5 * z.laenge * z.breite;
    z.zeta = an(s.zeta, zungenNote);
    z.ruhe = an(s.ruheM, zungenNote);
    z.spalt = an(s.seitenspaltM, zungenNote);
    z.kanzelle = an(s.kanzelleM3, zungenNote);
    z.kanalLaenge = an(s.kanalLaengeM, zungenNote);
    z.kanalFlaeche = an(s.kanalFlaecheM2, zungenNote);
    z.ventil = an(s.ventilFlaecheM2, zungenNote);
    return z;
}

double windBei(const ZungenSpiel& s, double expression)
{
    const double e = std::clamp(expression, 0.0, 1.0);
    return s.windMinPa * std::pow(s.windMaxPa / s.windMinPa, e);
}

double windMitte(const ZungenSpiel& s)
{
    return windBei(s, 0.5);
}

double bendStufe(int k)
{
    return std::exp2(-(k + 1) / 4.0);
}

double ventilFuerCent(const ZungenSpiel& s, int note, double cent)
{
    if (!s.bendUeberVentil || cent >= 0.0 || s.bendGrenzeOeffnung.punkte.empty() || s.bendGrenzeCent.punkte.empty() ||
        static_cast<int>(s.bendCentJeStufe.size()) != kBendStufen)
        return 1.0;
    const int n = std::clamp(note, s.noteMin, s.noteMax);
    const double grenzeF = std::clamp(wertAn(s.bendGrenzeOeffnung, n, Skala::Logarithmisch), 1e-6, 1.0);
    const double grenzeC = std::min(0.0, wertAn(s.bendGrenzeCent, n, Skala::Linear));
    const double ziel = std::max(cent, grenzeC);
    if (ziel >= 0.0) return 1.0;
    // die Punkte (Oeffnung, Cent) von offen bis zur Grenze, fallend in der Oeffnung
    double fVor = 1.0, cVor = 0.0;
    for (int k = 0; k <= kBendStufen; ++k)
    {
        const bool letzte = k == kBendStufen || bendStufe(k) <= grenzeF;
        const double f = letzte ? grenzeF : bendStufe(k);
        const double c = letzte ? grenzeC : std::min(0.0, wertAn(s.bendCentJeStufe[static_cast<std::size_t>(k)], n, Skala::Linear));
        if (c <= ziel && c < cVor)
        {
            const double u = (ziel - cVor) / (c - cVor);
            return std::exp(std::log(fVor) + u * (std::log(f) - std::log(fVor)));
        }
        if (letzte) return grenzeF;
        fVor = f;
        cVor = std::min(cVor, c);
    }
    return grenzeF;
}

void rechneEineZunge(const ZungenSpiel& s, double f0, int zungenNote, const ZungenAnschlag& a, std::vector<double>& aus, std::int64_t frames,
                     ZungenMessung* messung)
{
    aus.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0);
    if (messung != nullptr && messung->auslenkung != nullptr) messung->auslenkung->assign(aus.size(), 0.0);
    if (frames <= 0 || f0 <= 0.0) return;
    const double fs = static_cast<double>(a.rate) * kUeber, dt = 1.0 / fs;
    const ZungeAnDerTaste z = zungeAn(s, zungenNote);
    const double laenge = z.laenge, masse = z.masse;
    const double w0 = 2.0 * kPi * f0;
    const double steife = masse * w0 * w0, daempfung = 2.0 * z.zeta * masse * w0;
    const double sr = 0.5 * laenge * z.breite;  // wirksame Flaeche fuer die Kraft (offen)
    const double p0 = a.druckPa > 0.0 ? a.druckPa : windBei(s, a.expression);
    // das Ventil: der Bend schliesst es (Adachi: kleinere Oeffnung → tiefer); was die Physik nicht hergibt, faellt weg
    // (F-2 a: das Modell kappt)
    const double ventil = z.ventil * (a.ventilFaktor > 0.0 ? std::min(1.0, a.ventilFaktor) : 1.0);
    const double traegheit = kRho * z.kanalLaenge / z.kanalFlaeche;
    const double kKanz = kRho * kC * kC / z.kanzelle;
    const auto halten = static_cast<std::int64_t>(std::llround(a.haltenS * a.rate)) * kUeber;
    double y = 0.0, v = 0.0, p1 = 0.0, u = 0.0, uVor = 0.0, phaseV = 0.0;
    double yMax = 0.0;
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0);
    std::vector<double>* spur = messung != nullptr ? messung->auslenkung : nullptr;
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
        const double su = z.breite * std::sqrt((y + z.ruhe) * (y + z.ruhe) + z.spalt * z.spalt);
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
            if (spur != nullptr) (*spur)[i] = y;
        }
    }
    aus = summe;
    if (messung != nullptr) messung->amplitudeRel = yMax / laenge;
}

ZungeDesChors zungeDesChors(const ZungenSpiel& s, const Chor& c, int note)
{
    const double n = std::clamp(note + c.halbtoene, s.noteMin - 24, s.noteMax + 24);
    const double ziel = hzVon(n) + c.schwebungA4Hz * std::sqrt(hzVon(n) / 440.0);
    // f₀ aus der Stimmung der Note, um die Schwebung verschoben (offen: f₀ ∝ Spielfrequenz in der Naehe)
    const int n0 = std::clamp(static_cast<int>(n), s.noteMin, s.noteMax);
    const double verh = s.eigenfrequenzJeNote.punkte.empty() ? 1.02 : wertAn(s.eigenfrequenzJeNote, n0, Skala::Logarithmisch) / hzVon(n0);
    return ZungeDesChors{ziel * verh, ziel, n0};
}

void rechneZungenTon(const ZungenSpiel& s, const ZungenAnschlag& a, std::vector<float>& mono, std::int64_t frames, ZungenMessung* messung)
{
    mono.assign(static_cast<std::size_t>(std::max<std::int64_t>(0, frames)), 0.0f);
    if (frames <= 0 || a.rate <= 0) return;
    const bool bass = s.bassAb > 0 && a.note < s.bassAb;
    const std::vector<Chor>& choere = bass ? s.bassChoere : s.choere;
    std::vector<double> summe(static_cast<std::size_t>(frames), 0.0), ton;
    // das Ventil der Taste oeffnet fuer alle ihre Zungen gleich: der Bend aus der Kurve der gespielten Note (ZV9 a)
    ZungenAnschlag b = a;
    if (b.ventilFaktor <= 0.0) b.ventilFaktor = ventilFuerCent(s, a.note, a.pitchCent);
    bool erster = true;
    for (const Chor& c : choere)
    {
        const ZungeDesChors z = zungeDesChors(s, c, a.note);
        rechneEineZunge(s, z.f0, z.note, b, ton, frames, erster ? messung : nullptr);
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

}  // namespace lumi::kern::zunge
