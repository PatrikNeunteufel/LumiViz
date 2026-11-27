/**
 * @file KoerperAmSteg.hpp
 * @brief Der Koerper am Steg fuer einen Kern: `kern::Steg` aus einem gerechneten Koerper (`.lvinstbody`)
 *        (Instrumente_IC_Konzept.md GZ5 a, GZ5-1 a, GZ7 a, SR1 a; Schnittfolge 34k-7d-4 (b))
 *
 * Ein Kern mit `stegImAuftrag` (die gezupfte Saite ab Rechenfassung 2) rechnet die Moden des Koerpers in seiner
 * Saitenschleife. Die App, der Stimmer und die Tests bauen den `Steg` dafuer aus derselben Funktion, `stegAus`:
 *
 * - **Moden**: die gerechneten bzw. gefitteten Moden der Datei (Frequenz, η, Richtung) — der statistische Teil nicht;
 *   ueber 5 kHz rechnet der Kern den Daempfer aus der Dichte (GZ5-1 a). Wie der Bezug `proben/2026-11-26_S222_bezug_zupf.py`,
 *   der nur die Moden der Datei in die Schleife nimmt.
 * - **Stegpunkte**: alle Stegpunkte der Datei mit dem Formwert je Mode laengs ihrer Richtung und φ_rms des statistischen
 *   Teils senkrecht (`stegStreuung` · cos des Winkels im statistischen Teil).
 * - **Dichte**: n(f) des statistischen Teils (`vonHz`, `bisHz`, `dichte`, `anstieg`); ohne statistischen Teil keine.
 * - **Richtung** (SR1 a): traegt die Datei je Mode eine Richtung (eine Mode mit `bridgeAngle` ≠ 0, Fassung 5 — die Gitarre
 *   nach Elie), gilt sie. Sonst (jeder Koerper aus einem Bauplan) die Annahme nach Scherrer 2013 Abb. A.11 wie
 *   `richtung_tan` des Pruefstands S199: tan θ_n = 0,03 · (f_n / 200 Hz)^1,27 auf 0,03 … 0,4, das Vorzeichen je Mode in der
 *   Reihenfolge der Datei aus SplitMix64 mit Samen 7 (+ bei `gleich() < 0,5`). Der Formwert der Datei ist dann der
 *   senkrechte: der Steg traegt θ = atan(tan θ) und den Formwert φ · √(1 + tan² θ), damit senkrecht φ und parallel
 *   φ · tan θ herauskommen — wie der Bezug. Die Koerperdatei bleibt, wie sie gerechnet ist.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentBody/Koerper.hpp"
#include "InstrumentBody/Statistik.hpp"
#include "InstrumentCore/Steg.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <vector>

namespace lumi::koerper
{

/// Der Samen der Annahme nach Scherrer (SR1 a, wie der Pruefstand S199).
inline constexpr std::uint64_t kRichtungSamen = 7;

/// SR1 a: tan θ je Mode nach Scherrer (siehe Kopf), in der Reihenfolge von `hz`.
[[nodiscard]] inline std::vector<double> richtungNachScherrer(const std::vector<double>& hz, std::uint64_t samen = kRichtungSamen)
{
    Zufall z(samen);
    std::vector<double> tan(hz.size());
    for (std::size_t n = 0; n < hz.size(); ++n)
    {
        const double betrag = std::clamp(0.03 * std::pow(hz[n] / 200.0, 1.27), 0.03, 0.4);
        tan[n] = betrag * (z.gleich() < 0.5 ? 1.0 : -1.0);
    }
    return tan;
}

/// Ob der Koerper je Mode eine eigene Richtung am Steg traegt (eine Mode mit Winkel ≠ 0).
[[nodiscard]] inline bool traegtRichtung(const Koerper& k)
{
    return std::any_of(k.moden.begin(), k.moden.end(), [](const ModeImKoerper& m) { return m.stegWinkelGrad != 0.0; });
}

/// Der Koerper am Steg (siehe Kopf).
[[nodiscard]] inline kern::Steg stegAus(const Koerper& k)
{
    constexpr double kGrad = 180.0 / std::numbers::pi;
    kern::Steg s;
    const bool eigene = traegtRichtung(k);
    std::vector<double> tan;
    if (!eigene)
    {
        std::vector<double> hz;
        hz.reserve(k.moden.size());
        for (const ModeImKoerper& m : k.moden) hz.push_back(m.frequenzHz);
        tan = richtungNachScherrer(hz);
    }
    s.moden.reserve(k.moden.size());
    for (std::size_t n = 0; n < k.moden.size(); ++n)
        s.moden.push_back({k.moden[n].frequenzHz, k.moden[n].verlustfaktor, eigene ? k.moden[n].stegWinkelGrad : std::atan(tan[n]) * kGrad});
    const double cosStatistik = std::cos(k.statistik.stegWinkelGrad / kGrad);
    for (std::size_t p = 0; p < k.stegTasten.size(); ++p)
    {
        kern::Steg::Punkt pk;
        pk.taste = k.stegTasten[p];
        pk.form.reserve(k.moden.size());
        for (std::size_t n = 0; n < k.moden.size(); ++n)
            pk.form.push_back(eigene ? k.moden[n].amStegpunkt[p] : k.moden[n].amStegpunkt[p] * std::sqrt(1.0 + tan[n] * tan[n]));
        pk.streuung = p < k.statistik.stegStreuung.size() ? k.statistik.stegStreuung[p] * cosStatistik : 0.0;
        s.punkte.push_back(std::move(pk));
    }
    if (k.statistik.dichte > 0.0)
        s.dichte = {k.statistik.vonHz, k.statistik.bisHz, k.statistik.dichte, k.statistik.anstieg};
    return s;
}

}  // namespace lumi::koerper
