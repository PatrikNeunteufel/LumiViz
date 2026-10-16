/**
 * @file Farbreihen.hpp
 * @brief Die Farbreihen der Zeilen in den Achsen: eine kühle, eine warme (S100, S128)
 *
 * Entstanden im Composer (Entscheid Patrik S100, K1 b): Bild-Kanäle bekommen
 * kühle Töne, Audio-Kanäle warme — die Art bleibt an der Farbe erkennbar. Die
 * Taktachse des Loop-Panels nimmt dieselben Farben für ihre Muster. Seit S128
 * (Entscheid Patrik N1 b) liegen sie hier, neben der gemeinsamen `Achse`, und
 * der Loop hängt dafür nicht mehr am Composer.
 */
#pragma once

#include <cstdint>

namespace lumi::ui
{

enum class Farbreihe : int
{
    Kuehl = 0,  ///< Blau, Violett, Grün, Türkis, Magenta, Indigo
    Warm = 1,   ///< Orange, Rot, Gelb, Rosa, Rotorange, Ocker
};

/// So viele Farben hat eine Reihe.
inline constexpr int kReihenFarben = 6;

/// Die Farbe `nr` der Reihe als 0xRRGGBB; `nr` läuft um (auch negativ).
[[nodiscard]] std::int32_t reihenFarbe(Farbreihe reihe, int nr);

}  // namespace lumi::ui
