/**
 * @file   Kurvenabstand.cpp
 * @brief  Die Rechnung zu `InstrumentMeasure/Kurvenabstand.hpp` (Herleitung dort). Wortgleich aus dem Kopf der
 *         Unit-Tests gezogen (Instrumente_IC_Konzept.md, Teilplan 34e-1): in der .cpp, damit die Schalter nach
 *         E1 a (PRIVATE) gelten.
 */
#include "InstrumentMeasure/Kurvenabstand.hpp"

namespace lumi::messung
{

namespace detail
{

std::vector<double> neuBezogen(const Abklingkurve& k, std::size_t von, std::size_t ende)
{
    std::vector<double> l;
    const std::size_t eigenesEnde = k.ersterPunkt + k.db.size();
    if (von < k.ersterPunkt || ende > eigenesEnde || von >= ende) return l;
    const auto energie = [&](std::size_t punkt) { return punkt >= eigenesEnde ? 0.0 : std::pow(10.0, k.db[punkt - k.ersterPunkt] / 10.0); };
    const double rest = energie(ende);
    const double bezug = energie(von) - rest;
    if (bezug <= 0.0) return l;
    // hoechstens 100 dB tief: unter der Messgrenze (90 dB, US6-1 a) ist nichts mehr gemessen, und tiefer teilt die
    // Rechnung Reste an der Grenze der Genauigkeit (Befund S155 an MIDI 48, festgelegt im Bau)
    for (std::size_t p = von; p < ende; ++p) l.push_back(10.0 * std::log10(std::max((energie(p) - rest) / bezug, 1e-10)));
    return l;
}
}  // namespace detail

KurvenAbstand kurvenAbstand(const TeiltonKurven& m, const TeiltonKurven& s, double randS)
{
    KurvenAbstand a;
    if (!s.erkannt || s.kurve.db.empty()) return a;
    const double raster = s.kurve.rasterS;
    const auto rand = static_cast<std::size_t>(std::lround(randS / raster));
    // Hat das Modell keine Kurve, endet sie, bevor ein Vergleich moeglich ist, oder traegt sie kein Restintegral, fehlt
    // ihm dort Energie: die Kurve zaehlt als sofort bis zur Untergrenze gefallen — 0 dB am ersten Punkt, danach
    // −100 dB wie in `neuBezogen`; die Autokovarianz gegen null (RV1 a, S156). So ist ein fehlender Teilton nie
    // billiger als einer, der zu schnell faellt.
    const auto ohneModell = [&] {
        const std::size_t n = s.kurve.gueltig;
        if (n < 2) return a;
        KurvenAbstand o;
        double q = 0.0;
        for (std::size_t j = 1; j < n; ++j) q += (s.kurve.db[j] + kKurveUntergrenzeDb) * (s.kurve.db[j] + kKurveUntergrenzeDb);
        o.abklingenDb = std::sqrt(q / static_cast<double>(n));
        double c = 0.0;
        for (const double v : s.kovarianz.db2) c += v * v;
        o.schwebungDb = s.kovarianz.db2.empty() ? 0.0 : std::sqrt(std::sqrt(c / static_cast<double>(s.kovarianz.db2.size())));
        o.gewicht = std::min(1.0, -s.kurve.db[n - 1] / 60.0);
        o.punkte = n;
        o.vonPunkt = s.kurve.ersterPunkt;
        o.endePunkt = s.kurve.ersterPunkt + s.kurve.db.size();
        o.gilt = o.gewicht > 0.0;
        return o;
    };
    if (!m.erkannt || m.kurve.db.empty()) return ohneModell();
    const std::size_t von = std::max(m.kurve.ersterPunkt, s.kurve.ersterPunkt);
    const std::size_t ende = std::min(m.kurve.ersterPunkt + m.kurve.db.size(), s.kurve.ersterPunkt + s.kurve.db.size());
    if (ende <= von + rand + 2) return ohneModell();
    const std::vector<double> lm = detail::neuBezogen(m.kurve, von, ende);
    const std::vector<double> ls = detail::neuBezogen(s.kurve, von, ende);
    if (ls.empty()) return a;
    if (lm.empty()) return ohneModell();
    const std::size_t n = ende - rand - von;
    double summe = 0.0;
    double ende20 = 0.0;
    for (std::size_t j = 0; j < n; ++j)
    {
        const double q = (lm[j] - ls[j]) * (lm[j] - ls[j]);
        summe += q;
        if (5 * j >= 4 * n) ende20 += q;
    }
    a.abklingenDb = std::sqrt(summe / static_cast<double>(n));
    a.anteilEnde = summe > 0.0 ? ende20 / summe : 0.0;
    a.gewicht = std::min(1.0, -ls[n - 1] / 60.0);

    const std::size_t lags = std::min(m.kovarianz.db2.size(), s.kovarianz.db2.size());
    if (lags > 0)
    {
        double q = 0.0;
        for (std::size_t j = 0; j < lags; ++j) q += (m.kovarianz.db2[j] - s.kovarianz.db2[j]) * (m.kovarianz.db2[j] - s.kovarianz.db2[j]);
        a.schwebungDb = std::sqrt(std::sqrt(q / static_cast<double>(lags)));
    }
    a.punkte = n;
    a.vonPunkt = von;
    a.endePunkt = ende;
    a.gilt = a.gewicht > 0.0;
    return a;
}

std::optional<double> tastenAbstand(const std::vector<KurvenAbstand>& kurven)
{
    double summe = 0.0;
    double gewichte = 0.0;
    for (const KurvenAbstand& k : kurven)
        if (k.gilt)
        {
            summe += k.gewicht * (k.abklingenDb * k.abklingenDb + k.schwebungDb * k.schwebungDb);
            gewichte += k.gewicht;
        }
    if (gewichte <= 0.0) return std::nullopt;
    return std::sqrt(summe / gewichte);
}

}  // namespace lumi::messung
