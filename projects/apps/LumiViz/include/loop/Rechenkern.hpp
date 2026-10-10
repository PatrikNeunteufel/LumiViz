/**
 * @file Rechenkern.hpp
 * @brief Loop: aus einem Loop und einer Stelle werden Abtastwerte
 *        (docs/audio/Loop_Konzept.md, Entscheid L1 a, Stufe 3)
 *
 * Zwei Schritte:
 *
 * - `plane` uebersetzt einen Loop fuer eine Abtastrate in einen `Ablauf`: je
 *   Spur die Schlaege eines Durchlaufs der Abfolge, jeder mit seiner Stelle
 *   in Abtastwerten und seiner Lautstaerke links und rechts.
 * - `rechne` fuellt einen Block: was der Ablauf, endlos wiederholt, an den
 *   Abtastwerten `[von, von + frames)` hoeren laesst.
 *
 * `rechne` hat **kein Gedaechtnis**: derselbe Bereich ergibt dieselben Werte,
 * gleich in welchen Bloecken und in welcher Reihenfolge gerechnet wird. So
 * laesst sich die Stelle frei setzen (die Quelle im Mixer fuehrt sie selbst,
 * Beleg §6), und ein Test zaehlt die Werte ohne Tonausgabe nach. Es holt
 * keinen Speicher und liest keine Datei — es darf im Ton-Thread laufen.
 *
 * Ein neuer Schlag auf einer Spur schneidet den noch klingenden derselben
 * Spur ab, mit einer Blende ueber `kSchnittFrames` (wie eine Drum-Maschine).
 * Vor dem Abtastwert 0 gibt es nichts: der erste Durchlauf beginnt ohne
 * Ausklang eines frueheren.
 *
 * Kein Qt, keine Audio-Engine.
 */
#pragma once

#include "loop/Loop.hpp"

#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// So lange klingt ein abgeschnittener Schlag noch aus, in Abtastwerten.
/// Zwei Schlaege einer Spur liegen immer weiter auseinander (bei 400 BPM und
/// 16 Schritten je Schlag sind es an 44100 Hz noch 413).
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
};

/// Ein Loop, fuer eine Abtastrate uebersetzt. Aendert sich der Loop, entsteht
/// ein neuer Ablauf — dieser bleibt, wie er ist.
struct Ablauf
{
    int rate = 0;
    std::int64_t durchlaufFrames = 0;         ///< Laenge eines Durchlaufs; 0 = nichts zu spielen
    std::vector<std::vector<Schlag>> spuren;  ///< je Spur des Loops, nach Stelle aufsteigend
};

/// Den Loop uebersetzen. Er muss gerichtet sein (`Loop::richte`).
/// Panorama: in der Mitte beide Seiten voll, zur Seite faellt die andere ab.
[[nodiscard]] Ablauf plane(const Loop& loop, int rate);

/// `frames` Abtastwerte ab `von` nach `aus` schreiben (Stereo, `2 * frames`
/// Werte; der Block wird ueberschrieben). `klaenge[i]` gehoert zur Spur `i`;
/// eine Spur ohne Klang schweigt.
void rechne(const Ablauf& ablauf, const std::vector<Klang>& klaenge, std::int64_t von, int frames, float* aus);

}  // namespace lumi::loop
