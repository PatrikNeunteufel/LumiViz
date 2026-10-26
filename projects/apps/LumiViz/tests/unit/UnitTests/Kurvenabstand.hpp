/**
 ****************************************************************************************
 * @file   Kurvenabstand.hpp
 * @brief  Stufe 31d-7 B, 7.5.3: der Abstand der Kurven eines Teiltons zwischen Modell und
 *         *Salamander* (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S154/S155:
 *         SM4 a, K2 a, K3 a, K5 a, K6 a, K6-1 a)
 *
 * @details
 * Reines C++ ohne Qt (IK1). Beide Seiten liegen als `TeiltonKurven` vor (`Fluegelmessung.hpp`):
 * die Kurve nach Schroeder bis zum Ende des eigenen Bereichs und die Autokovarianz des Rests.
 *
 * - *Gemeinsamer Bereich* (K5 a, K6 a): vom spaeteren ersten Punkt bis zum frueheren Ende beider
 *   Kurven. Die Kurve, die laenger reicht, wird auf das gemeinsame Ende gekuerzt — aus L allein:
 *   10^(L/10) ist das Restintegral bezogen auf den Anfang, also I(t) − I(T) fuer das kuerzere T.
 *   Beide neu auf den gemeinsamen ersten Punkt bezogen.
 * - *Verglichen* bis 1 s vor dem gemeinsamen Ende (K6-1 a).
 * - *Anteil Abklingen* (K3 a (1)): RMS der Differenz in dB ueber die verglichenen Punkte.
 * - *Anteil Schwebung* (K3 a (2)): Wurzel aus der RMS-Differenz der Autokovarianzen ueber die
 *   gemeinsamen Verzoegerungen, in dB.
 * - *Gewicht* (K3 a): der Fall der Kurve von *Salamander* ueber die verglichenen Punkte durch 60 dB,
 *   hoechstens 1 — die Datei traegt den Boden nicht; der Fall bis 1 s vor dem Ende eines Bereichs, der
 *   10 dB ueber dem Boden endet, steht fuer den Bereich ueber dem Boden (festgelegt im Bau, S155).
 * - *Je Taste* (K2 a): d² = Abklingen² + Schwebung² je Kurve (1 : 1), gewichtet gemittelt ueber
 *   Ebenen und Teiltoene, die Wurzel daraus.
 ****************************************************************************************
 */

#pragma once

#include "Fluegelmessung.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <vector>

namespace lumi::messung
{

/// Der Abstand einer Kurve.
struct KurvenAbstand
{
    bool gilt = false;
    double abklingenDb = 0.0;  ///< K3 a (1)
    double schwebungDb = 0.0;  ///< K3 a (2)
    double gewicht = 0.0;
    std::size_t punkte = 0;    ///< verglichene Punkte der Kurve nach Schroeder
    std::size_t vonPunkt = 0;  ///< absolut (Punkt · Raster = Zeit nach dem Einsatz)
    std::size_t endePunkt = 0; ///< gemeinsames Ende, absolut
};

namespace detail
{

/// L einer Kurve auf [von, ende) (absolute Punkte) neu bezogen: gekuerzt auf `ende`, 0 dB bei `von`.
inline std::vector<double> neuBezogen(const Abklingkurve& k, std::size_t von, std::size_t ende)
{
    std::vector<double> l;
    const std::size_t eigenesEnde = k.ersterPunkt + k.db.size();
    if (von < k.ersterPunkt || ende > eigenesEnde || von >= ende) return l;
    const auto energie = [&](std::size_t punkt) { return punkt >= eigenesEnde ? 0.0 : std::pow(10.0, k.db[punkt - k.ersterPunkt] / 10.0); };
    const double rest = energie(ende);
    const double bezug = energie(von) - rest;
    if (bezug <= 0.0) return l;
    for (std::size_t p = von; p < ende; ++p) l.push_back(10.0 * std::log10(std::max((energie(p) - rest) / bezug, 1e-30)));
    return l;
}

}  // namespace detail

/// Der Abstand einer Kurve des Modells `m` zur Kurve von *Salamander* `s` (K3 a, K6 a, K6-1 a). Beide auf dem Raster
/// nach K5 a. `randS` wie K6-1 a.
inline KurvenAbstand kurvenAbstand(const TeiltonKurven& m, const TeiltonKurven& s, double randS = 1.0)
{
    KurvenAbstand a;
    if (!m.erkannt || !s.erkannt || m.kurve.db.empty() || s.kurve.db.empty()) return a;
    const double raster = s.kurve.rasterS;
    const std::size_t von = std::max(m.kurve.ersterPunkt, s.kurve.ersterPunkt);
    const std::size_t ende = std::min(m.kurve.ersterPunkt + m.kurve.db.size(), s.kurve.ersterPunkt + s.kurve.db.size());
    const auto rand = static_cast<std::size_t>(std::lround(randS / raster));
    if (ende <= von + rand + 2) return a;
    const std::vector<double> lm = detail::neuBezogen(m.kurve, von, ende);
    const std::vector<double> ls = detail::neuBezogen(s.kurve, von, ende);
    if (lm.empty() || ls.empty()) return a;
    const std::size_t n = ende - rand - von;
    double summe = 0.0;
    for (std::size_t j = 0; j < n; ++j) summe += (lm[j] - ls[j]) * (lm[j] - ls[j]);
    a.abklingenDb = std::sqrt(summe / static_cast<double>(n));
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

/// Der Abstand einer Taste: d² = Abklingen² + Schwebung² je Kurve, gewichtet gemittelt, die Wurzel (K2 a, K3 a).
/// Kurven, die nicht gelten, zaehlen nicht; ohne geltende Kurve kein Wert.
inline std::optional<double> tastenAbstand(const std::vector<KurvenAbstand>& kurven)
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
