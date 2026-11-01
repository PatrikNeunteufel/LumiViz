/**
 * @file Psola.hpp
 * @brief Loop: Dauer und Tonhoehe einstimmigen Gesangs mit TD-PSOLA (docs/audio/Loop_Konzept.md
 *        §4p; Entscheid Patrik WV6 a, Stufe 15h)
 *
 * TD-PSOLA nach Moulines & Charpentier, „Pitch-synchronous waveform processing techniques for
 * text-to-speech synthesis using diphones“, Speech Communication 9 (1990): der Klang wird an
 * **Marken** je Periode in Koerner zerlegt (je Korn zwei Perioden, Hann-Fenster), die Koerner
 * werden im Abstand der neuen Periode wieder aufeinandergelegt. Die Form je Periode bleibt —
 * damit die Formanten; nur der Abstand der Pulse aendert sich.
 *
 * **Marken:** der Tonhoehenverlauf mit pYIN (`loop/PYin`, WV6 a) auf dem Klang selbst; in
 * gesungenen Stellen je Periode eine Marke am Gipfel des tiefpassgefilterten Mono-Signals,
 * gesucht ±20 % der Periode um die erwartete Stelle (in der ersten Periode nach einer Pause
 * ueber die ganze Periode), **parabolisch auf einen Bruchteil verfeinert**; in stummen Stellen
 * eine Marke alle 5 ms, dort bleibt die Tonhoehe.
 *
 * **Bruchteile:** Koerner werden auf die Stelle mit Bruchteil gesetzt und dafuer aus dem Klang
 * mit einem gefensterten Sinc (16 Stuetzen, Kaiser β = 8, 1024 Phasen) gelesen. Ganze
 * Abtastwerte liessen die Pulse um ±0,5 Abtastwerte schwanken — bei 48 kHz am Formanten um
 * 1 kHz ein Nebengeraeusch von rund −28 dB (Pruefstein 15h, erster Lauf).
 *
 * **Koerner:** ein Korn reicht von der vorigen bis zur naechsten Marke (Hann je Haelfte, links
 * und rechts so lang wie der Abstand zur Nachbarmarke) — bei gleicher Dauer und Tonhoehe ergeben
 * die Koerner den Klang genau wieder. Die Synthese setzt Koerner im Abstand Periode / β
 * (β = 2^(Halbtoene / 12)); je Stelle der Ausgabe gilt die Marke, die der Stelle der Eingabe
 * (Ausgabe mal Verhaeltnis der Laengen) am naechsten liegt. Wo Koerner sich mehr als einfach
 * ueberlagern (β > 1), wird durch die Summe der Fenster geteilt; bei β < 1 entstehen Luecken
 * zwischen den Pulsen, jedes Korn traegt dann den Faktor √(1/β), damit die Leistung ungefaehr
 * bleibt (von Claude gesetzt, im Pruefstein gemessen).
 *
 * Reines C++ ohne Qt; derselbe Aufruf gibt dieselben Werte.
 */
#pragma once

#include "loop/Dehner.hpp"
#include "loop/PYin.hpp"

#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// In stummen Stellen eine Marke alle so viele Sekunden (von Claude gesetzt).
inline constexpr double kPsolaStummAbstandS = 0.005;

/// Eine Marke: die Stelle im Klang (in Abtastwerten, mit Bruchteil) und ob dort gesungen wird.
struct PsolaMarke
{
    double stelle = 0.0;
    bool gesungen = false;
    [[nodiscard]] bool operator==(const PsolaMarke&) const = default;
};

/// Die Marken von `mono` in `rate` nach dem Verlauf `verlauf` (aus `messeTonhoehenverlauf` mit
/// derselben Rate); aufsteigend, die erste bei 0 oder am ersten Gipfel.
[[nodiscard]] std::vector<PsolaMarke> psolaMarken(const std::vector<double>& mono, int rate, const Tonhoehenverlauf& verlauf);

/// Der Dehner mit TD-PSOLA (*Voice*, WV6 a); die Marken misst pYIN mit `pyin`.
[[nodiscard]] std::unique_ptr<Dehner> dehnerPsola(const PYinEinstellungen& pyin = {});

}  // namespace lumi::loop
