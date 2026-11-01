/**
 * @file WaveKlang.hpp
 * @brief Loop: der Klang eines Wave-Musters — Ausschnitt, Lage, Gain und Blenden, ohne
 *        Dehnen und Tonhoehe (docs/audio/Loop_Konzept.md §4p, Stufe 15c; Entscheide
 *        Patrik WV3 a, WV8 a)
 *
 * Ein Wave-Muster wird **vorab gerechnet** wie ein Ton (WV8 a): je Muster und wirksamem
 * Stand ein Klang, der eine Wiederholung lang ist — ab dem Anfang des Musters, mit Stille
 * vor einer positiven Lage. Der Rechenkern spielt ihn je Wiederholung ab dem Anfang des
 * Musters (`plane`); Expression und die Kurve am Clip wirken dort beim Abspielen.
 *
 * Wie bei einem Ton beschreibt der **Name** den Klang ganz (`wave:<werte>|<datei>`), in
 * Einheiten, die nicht an der Abtastrate haengen; der Schluessel im Tonspeicher nimmt den
 * Stand der Datei dazu (`klangSchluessel`). So bleibt der Rechenkern frei von Dateien und
 * der Name gleich, solange das Muster gleich klingt.
 *
 * Kein Qt-Widget, keine Audio-Engine — dekodiert wird ueber den `Dekodierer` des
 * Tonauftrags.
 */
#pragma once

#include "loop/Loop.hpp"
#include "loop/PYin.hpp"
#include "loop/Rechenkern.hpp"

#include <QString>

#include <vector>

namespace lumi::loop
{

/// Was den Klang eines Wave-Musters ausmacht (ohne Dehnen und Tonhoehe, Stufe 15c).
struct WaveKlang
{
    QString datei;           ///< die Nennung wie am Muster (`user:`, `lib:`, ein Pfad)
    double startS = 0.0;     ///< Ausschnitt: Anfang in Sekunden der Datei
    double endeS = 0.0;      ///< Ausschnitt: Ende in Sekunden der Datei; 0 = bis zum Ende
    double versatzMs = 0.0;  ///< die Lage ab dem Anfang des Musters, auch negativ
    double laengeMs = 0.0;   ///< eine Wiederholung des Musters
    double gainDb = 0.0;
    double fadeInMs = 0.0;
    double fadeOutMs = 0.0;
    /// Stufe 15f: die Spur Pitch — Punkte in Millisekunden ab dem Anfang des Musters, Werte in
    /// Halbtoenen, dazwischen linear (`kurveBei`). Leer, wenn kein Punkt die Tonhoehe aendert:
    /// dann klingt der Klang Bit fuer Bit wie ohne Spur.
    std::vector<SpurPunkt> tonhoehe;
    /// Stufe 15i: die Abschnitte der Spur Autotune (Sekunden der Datei); leer = ohne.
    std::vector<AutotuneAbschnitt> autotune;
    /// Stufe 15j: *Follow tempo* — so viel schneller spielt die Datei (`waveTempoFaktor`); 1 = im Original.
    double tempo = 1.0;
    /// Stufe 15h: *Pitch method* Voice (TD-PSOLA, `dehnerStimme()`) statt General. Wirkt nur
    /// mit Spur Pitch, Autotune oder Follow tempo; ohne alle immer false, damit der Name gleich bleibt.
    bool stimme = false;

    [[nodiscard]] bool operator==(const WaveKlang&) const = default;
};

/// Beginnt der Name mit `wave:`?
[[nodiscard]] bool istWaveKlang(const QString& name);
/// Der Name: `wave:<start>,<ende>,<versatz>,<laenge>,<gain>,<fadeIn>,<fadeOut>|<datei>`, mit
/// Spur Pitch je Punkt `;<ms>:<halbtoene>` vor dem `|` — die Zahlen so, dass sie genau
/// zurueckgelesen werden, je Abschnitt Autotune `;at:<anfang>:<ende>:<ton>:<cent>:<strength>:<speed>`
/// (Stufe 15i), mit *Follow tempo* `;tempo:<faktor>` (Stufe 15j), mit *Voice* danach `;voice`
/// (Stufe 15h). Ohne Spuren und im Original derselbe Name wie seit 15c.
[[nodiscard]] QString waveKlangName(const WaveKlang& klang);
/// Den Namen wieder zerlegen. false = kein Klang eines Wave-Musters.
bool zerlegeWaveKlang(const QString& name, WaveKlang& klang);

/// Der Klang des Wave-Musters `muster` im Loop: die Felder des Musters, Lage und Laenge
/// beim Tempo und Raster des Loops. Das Muster muss ein Wave-Muster sein.
[[nodiscard]] WaveKlang waveKlangVon(const Loop& loop, const Muster& muster);

/// So viele Sekunden der Datei braucht der Klang hoechstens — so weit wird dekodiert.
[[nodiscard]] double waveBedarfSekunden(const WaveKlang& klang);

/// Gain als Faktor: 0 dB = genau 1; an der unteren Grenze (`kWaveGainMinDb`) schweigt er.
[[nodiscard]] double waveGainFaktor(double gainDb);

/// Den Klang aus der dekodierten Datei rechnen (`datei`: Stereo in `rate`, links und
/// rechts im Wechsel). Abtastwert k des Klangs (ab dem Anfang des Musters) ist Abtastwert
/// k + d der Datei, d = round((start − versatz) · rate). Hoerbar ist er von a bis b
/// (ausschliesslich): a = max(0, round(start · rate) − d), b = min(n, E − d) mit n =
/// ceil(laenge · rate) und E = dem Ende des Ausschnitts in Abtastwerten der Datei (bei
/// `endeS` 0 ihr Ende). Je Abtastwert mal Gain, mal min(1, (k − a) / fadeIn) und
/// min(1, (b − k) / fadeOut) — beide linear, in Abtastwerten, ohne Blende 1. Der Klang
/// ist b Abtastwerte lang (vor a still); ist nichts zu hoeren, ist er leer.
/// Mit Spur Pitch (Stufe 15f) geht das hoerbare Stueck a … b vor Gain und Blenden durch den
/// `Dehner` (`dehner()`, *General*, WV6 a; mit `stimme` `dehnerStimme()`, Stufe 15h), gleich
/// lang, die Tonhoehe bei Abtastwert k nach der Spur an der Stelle k / rate. Mit Abschnitten
/// Autotune und dem Verlauf der Datei (`verlauf`, Stufe 15i) kommt die Korrektur an der Stelle
/// (k + d) / rate der Datei dazu (`autotuneKorrektur`); ohne Verlauf wirkt Autotune nicht.
/// Mit *Follow tempo* (`tempo` r ≠ 1, Stufe 15j) liegt Abtastwert k bei start · rate + (k − v) · r
/// der Datei (v = Lage in Abtastwerten); hoerbar von max(0, v) bis v + (E − start · rate) / r,
/// das Stueck geht immer durch den Dehner, die Tonhoehe bleibt.
[[nodiscard]] Klang rechneWave(const WaveKlang& klang, const std::vector<float>& datei, int rate,
                               const Tonhoehenverlauf* verlauf = nullptr);

}  // namespace lumi::loop
