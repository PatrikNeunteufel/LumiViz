/**
 * @file Rechenkern.hpp
 * @brief Loop: aus einem Loop und einer Stelle werden Abtastwerte
 *        (docs/audio/Loop_Konzept.md, Entscheid L1 a)
 *
 * Zwei Schritte:
 *
 * - `plane` uebersetzt einen Loop fuer eine Abtastrate in einen `Ablauf`: je
 *   **Stimme** die Schlaege eines Durchlaufs, jeder mit seiner Stelle in
 *   Abtastwerten und seiner Lautstaerke links und rechts. Eine Stimme ist
 *   eine Spur eines Musters in einem Kanal — oder, bei einem
 *   Instrument-Muster, ein Ton (Tonhoehe und Dauer): seine Noten sind die
 *   Schlaege, sein Klang ist vorab gerechnet (`loop/Instrument.hpp`).
 * - `rechne` fuellt einen Block: was der Ablauf, endlos wiederholt, an den
 *   Abtastwerten `[von, von + frames)` hoeren laesst.
 *
 * `rechne` hat **kein Gedaechtnis**: derselbe Bereich ergibt dieselben Werte,
 * gleich in welchen Bloecken und in welcher Reihenfolge gerechnet wird. So
 * laesst sich die Stelle frei setzen (die Quelle im Mixer fuehrt sie selbst,
 * Beleg §6), und ein Test zaehlt die Werte ohne Tonausgabe nach. Es holt
 * keinen Speicher und liest keine Datei — es darf im Ton-Thread laufen.
 *
 * Ein neuer Schlag einer Stimme schneidet ihren noch klingenden ab, mit einer
 * Blende ueber `kSchnittFrames` (wie eine Drum-Maschine) — auch ueber die
 * Wiederholungen eines Clips und ueber Clips desselben Musters im Kanal
 * hinweg. Verschiedene Stimmen schneiden einander nicht. Vor dem Abtastwert 0
 * gibt es nichts: der erste Durchlauf beginnt ohne Ausklang eines frueheren.
 *
 * Keine Audio-Engine.
 */
#pragma once

#include "loop/Loop.hpp"

#include <QStringList>

#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// So lange klingt ein abgeschnittener Schlag noch aus, in Abtastwerten.
/// Zwei Schlaege einer Stimme liegen immer weiter auseinander (bei 400 BPM
/// und 16 Schritten je Schlag sind es an 44100 Hz noch 413).
inline constexpr int kSchnittFrames = 64;

/// Ein Klang in der Abtastrate des Ablaufs: Stereo, links und rechts im Wechsel.
struct Klang
{
    std::vector<float> daten;
    [[nodiscard]] std::int64_t frames() const { return static_cast<std::int64_t>(daten.size() / 2); }
};

/// Ein Schlag im Durchlauf.
struct Schlag
{
    std::int64_t frame = 0;  ///< Stelle im Durchlauf
    float links = 0.0f;      ///< Lautstaerke: Schritt mal Spur mal Loop, mit Panorama
    float rechts = 0.0f;
    /// Index in `Ablauf::busse`, wenn der Clip des Schlags Effekte hat; -1 =
    /// der Schlag geht unbearbeitet in die Summe.
    int bus = -1;
};

/// Ein Clip mit eingeschalteten Effekten (Stufe 12, G1 b): seine Schlaege
/// kommen getrennt von der Summe heraus, damit seine Effekte nur ihn treffen.
struct Bus
{
    int kanal = 0;      ///< Index des Kanals im Loop
    int startTakt = 0;  ///< zusammen mit dem Kanal: welcher Clip
    ClipEffekte effekte;
};

/// Eine Spur eines Musters in einem Kanal, mit allen ihren Schlaegen.
struct Stimme
{
    int klang = -1;                 ///< Index in `Ablauf::klangNamen`; -1 = kein Klang
    std::vector<Schlag> schlaege;   ///< nach Stelle aufsteigend
};

/// Ein Loop, fuer eine Abtastrate uebersetzt. Aendert sich der Loop, entsteht
/// ein neuer Ablauf — dieser bleibt, wie er ist.
struct Ablauf
{
    int rate = 0;
    double schrittFrames = 0.0;         ///< Laenge eines Schritts in Abtastwerten (nicht ganzzahlig)
    std::int64_t durchlaufFrames = 0;   ///< Laenge eines Durchlaufs; 0 = nichts zu spielen
    /// Die Klaenge, die der Ablauf braucht (`Loop::klangNamen`): wer `rechne`
    /// ruft, reicht sie in dieser Reihenfolge herein.
    QStringList klangNamen;
    std::vector<Stimme> stimmen;        ///< nur Stimmen mit Schlaegen
    double bpm = 0.0;                   ///< das Tempo des Loops — die Echo-Zeit haengt daran
    /// Je Clip mit eingeschalteten Effekten in einem Kanal, der an ist, einer
    /// — auch wenn der Clip gerade keinen Schlag hat: sein Nachklang soll
    /// nicht abreissen, wenn jemand den letzten Schritt loescht.
    std::vector<Bus> busse;
};

/// Den Loop uebersetzen. Er muss gerichtet sein (`Loop::richte`). Kanaele,
/// die aus sind, und stumme Spuren haben keine Stimme; die Laenge des
/// Durchlaufs aendern sie nicht.
/// Panorama: in der Mitte beide Seiten voll, zur Seite faellt die andere ab.
/// Mit `saiten` (Stufe 23b, Entscheid S-A1 a) spielt ein Muster, dessen
/// Instrument Saiten traegt, je Vorkommen den Ton aus `toeneImKanal`
/// (Saitenplan.hpp) — mit Saite und Ende; die Namen stimmen mit
/// `loop.klangNamen(saiten)` ueberein. Ohne `saiten` wie bis Stufe 22.
[[nodiscard]] Ablauf plane(const Loop& loop, int rate, const SaitenVon& saiten = {});

/// `frames` Abtastwerte ab `von` nach `aus` schreiben (Stereo, `2 * frames`
/// Werte; der Block wird ueberschrieben). `klaenge[i]` ist der Klang zu
/// `ablauf.klangNamen[i]`; fehlt er oder ist er leer, schweigt die Stimme.
///
/// `busAus`: je Eintrag in `ablauf.busse` ein eigener Block derselben Groesse
/// (auch er wird ueberschrieben) — die Schlaege eines Clips mit Effekten
/// landen dann dort statt in `aus`, unbearbeitet; wer ruft, rechnet die
/// Effekte und mischt dazu. Ohne `busAus` geht alles unbearbeitet nach `aus`.
void rechne(const Ablauf& ablauf, const std::vector<Klang>& klaenge, std::int64_t von, int frames, float* aus,
            float* const* busAus = nullptr);

}  // namespace lumi::loop
