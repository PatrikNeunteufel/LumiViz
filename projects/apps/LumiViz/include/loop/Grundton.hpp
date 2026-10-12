/**
 * @file Grundton.hpp
 * @brief Loop: den Grundton einer Aufnahme messen — fuer **Detect** im
 *        Instrument-Creator (docs/audio/Loop_Konzept.md §4h, Stufe 11a,
 *        Schritt 3; Skizze gebilligt, Patrik S115)
 *
 * Gemessen wird die Periode: an mehreren Stellen der Aufnahme wird gesucht,
 * um wie viele Abtastwerte verschoben sie sich selbst am aehnlichsten ist
 * (die mittlere quadratische Abweichung, auf ihren laufenden Mittelwert
 * bezogen — das Verfahren „YIN"). Stimmen genug Stellen ueberein, gilt ihr
 * mittlerer Wert; sonst gibt es keinen klaren Ton — ein Schlag, Rauschen,
 * ein Akkord.
 *
 * Derselbe Aufruf ergibt dieselben Werte. Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include <QString>

#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// Was die Messung ergab.
struct Grundton
{
    bool gefunden = false;  ///< false = kein klarer Ton
    double hz = 0.0;        ///< die gemessene Frequenz
    int hoehe = 0;          ///< die naechste Tonhoehe (MIDI-Nummer)
    double cent = 0.0;      ///< so weit liegt der Ton ueber (+) oder unter ihr, -50 bis +50

    [[nodiscard]] bool operator==(const Grundton&) const = default;
};

/// Der tiefste und der hoechste Ton, den die Messung findet.
inline constexpr double kGrundtonVonHz = 40.0;
inline constexpr double kGrundtonBisHz = 2000.0;

/// Den Grundton messen: `stereo` sind Abtastwerte, links und rechts im
/// Wechsel, in `rate`; gemessen wird ab `abFrame`. Zu kurz (unter rund 50 ms),
/// zu leise oder ohne klaren Ton: `gefunden` ist false.
[[nodiscard]] Grundton messeGrundton(const std::vector<float>& stereo, int rate, std::int64_t abFrame = 0);

/// Wie das Ergebnis im Programm steht: „A3 +12 ct", „C4" — leer, wenn nichts gefunden ist.
[[nodiscard]] QString grundtonText(const Grundton& grundton);

}  // namespace lumi::loop
