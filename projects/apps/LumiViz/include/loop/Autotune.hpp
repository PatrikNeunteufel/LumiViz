/**
 * @file Autotune.hpp
 * @brief Loop: die Spur Autotune eines Wave-Musters rechnen — Korrektur aus Zielnoten und
 *        gemessener Tonhoehe, *Detect notes* (docs/audio/Loop_Konzept.md §4p; Entscheide Patrik
 *        WV4 a, WV9 a, Stufe 15i)
 *
 * **Korrektur:** je Rahmen des Tonhoehenverlaufs (pYIN, `loop/PYin`) in einem Abschnitt der
 * Unterschied vom gesungenen Ton zum Zielton in Halbtoenen, mal *Strength*. Ein stummer Rahmen
 * im Abschnitt haelt den letzten Wert, ausserhalb ist sie 0. Der Wert folgt dem mit *Speed*: nach
 * *Speed* ist 95 % des Wegs gegangen (ein Pol, τ = Speed / 3; von Claude gesetzt); Speed 0 springt
 * — die bekannte harte Klangfarbe (WV9 a). Ausserhalb der Abschnitte klingt die Korrektur mit der
 * Speed des letzten Abschnitts aus. Die Spur Pitch kommt beim Rechnen dazu (`WaveKlang`).
 *
 * ***Detect notes*:** je gesungenem Rahmen der naechste Ton der Tonart; ein Ton bleibt, bis ein
 * anderer um mehr als 0,3 Halbtoene naeher liegt (Hysterese, gegen Vibrato an der Grenze); eine
 * stumme Luecke bis 60 ms gehoert zur Note, eine laengere beendet sie; Noten unter 80 ms fallen
 * weg (alle von Claude gesetzt). Dazu ob die Messung **eine Stimme** sieht — dann schlaegt der
 * Wave editor *Voice* vor (WV6 a).
 *
 * Reines C++ ohne Qt; Zeiten in Sekunden der Datei.
 */
#pragma once

#include "loop/PYin.hpp"
#include "loop/WaveMuster.hpp"

#include <vector>

namespace lumi::loop
{

/// Die Hysterese, die kuerzeste Luecke und Note von *Detect notes* (von Claude gesetzt).
inline constexpr double kNotenHysterese = 0.3;
inline constexpr double kNotenLueckeS = 0.06;
inline constexpr double kNotenMinS = 0.08;

/// MIDI-Nummer mit Bruchteil zu einer Frequenz (A4 = 69 = 440 Hz).
[[nodiscard]] double midiVonHz(double hz);
/// Gehoert der Ton (MIDI) zur Tonart?
[[nodiscard]] bool inTonart(int ton, int grundton, Skala skala);
/// Der Ton der Tonart, der `midi` am naechsten liegt.
[[nodiscard]] int naechsterTon(double midi, int grundton, Skala skala);

/// Die Korrektur in Halbtoenen je Rahmen des Verlaufs (`verlauf.hz.size()` Werte).
[[nodiscard]] std::vector<double> autotuneKorrektur(const std::vector<AutotuneAbschnitt>& abschnitte,
                                                    const Tonhoehenverlauf& verlauf);
/// Die Korrektur bei Sekunde `sekunde` der Datei: zwischen den Rahmen linear, ausserhalb 0.
[[nodiscard]] double autotuneBei(const std::vector<double>& korrektur, const Tonhoehenverlauf& verlauf, double sekunde);

/// Was *Detect notes* findet.
struct NotenErkennung
{
    std::vector<AutotuneAbschnitt> abschnitte;  ///< mit Strength und Speed der Vorgabe
    bool eineStimme = false;  ///< mindestens 30 % gesungen, die Summe der Kandidaten im Median ≥ 0,6
};

/// Noten zwischen den Sekunden `von` und `bis` der Datei nach Tonart und Vorgaben von `autotune`.
[[nodiscard]] NotenErkennung erkenneNoten(const Tonhoehenverlauf& verlauf, const Autotune& autotune, double von, double bis);

/// Die Abschnitte zwischen `von` und `bis` durch `neue` ersetzen: was ganz darin liegt, geht; was
/// hineinragt, wird an der Grenze abgeschnitten. Danach gerichtet (`richteAutotune`).
void ersetzeAbschnitte(Autotune& autotune, const std::vector<AutotuneAbschnitt>& neue, double von, double bis);

/// Ein Abschnitt bei `sekunde` in zwei teilen (*Ctrl+Klick*); false = dort liegt keiner, oder zu
/// nah am Rand (unter `kNotenMinS / 2`).
bool teileAbschnitt(Autotune& autotune, double sekunde);
/// Abschnitt `i` mit dem folgenden verbinden (Zielton, Strength, Speed des ersten); false = es gibt
/// keinen folgenden.
bool verbindeAbschnitte(Autotune& autotune, std::size_t i);

}  // namespace lumi::loop
