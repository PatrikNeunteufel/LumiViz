/**
 * @file Pegel.hpp
 * @brief Loop: wie laut ein Instrument oder ein Drum-Klang ist — die
 *        Standardprobe und ihre Messung nach EBU R128 (Loop_Konzept §4n,
 *        Entscheide Patrik S139, LL2 a und LL4 a; Stufe 27b)
 *
 * Ohne Engine und ohne Fenster. Ein Instrument wird mit der **Standardprobe**
 * gemessen: eine Tonleiter ueber zwei Oktaven und ein Akkord, gerechnet wie
 * das Programm einen Loop rechnet (`plane` mit den Saiten des Instruments,
 * Klaenge aus `rendereTon`, `rechne`); das Mass ist das **Short-term-
 * Maximum** (3 s). Ein Drum-Klang wird mit **einem Schlag** gemessen, das
 * Mass ist das **Momentary-Maximum** (0,4 s). Sample-Schichten spielen, was
 * zu ihrer Aufnahme gemerkt ist (`sampleDaten`, Sample.hpp) — wer misst,
 * sorgt dafuer, dass es gemerkt ist.
 */
#pragma once

#include "audio/Lautheit.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <vector>

namespace lumi::loop
{

/// Die Standardprobe als Loop: 120 BPM, 4/4, Sechzehntel; eine Dur-Tonleiter
/// ueber zwei Oktaven (15 Toene, je zwei Schritte, Lautstaerke 0,9), zwei
/// Schritte Pause, dann der Akkord Grundton, Terz, Quinte, Oktave ueber einen
/// Takt (0,7); dahinter ein leerer Takt fuer den Ausklang. Grundton C4, fuer
/// die Art `bass` C2. Loop und Muster mit Lautstaerke 1, Muster in der Mitte.
[[nodiscard]] Loop standardProbe(const QString& instrument, const QString& art);

/// Die Standardprobe von `instrument` rechnen — ein Durchlauf, Stereo, in
/// `rate`. Die Planung bekommt die Saiten dieses Instruments (auch wenn es
/// noch keine Datei hat), jeder Ton wird mit ihm gerechnet.
[[nodiscard]] std::vector<float> rechneStandardprobe(const Instrument& instrument, int rate);

/// Die Standardprobe messen; das Mass fuer den Bezug ist `kurzzeitMaxLufs`.
[[nodiscard]] audio::Lautheit messeInstrument(const Instrument& instrument, int rate);

/// Einen Schlag messen: der Klang einmal, dahinter eine Sekunde Stille (damit
/// auch ein kurzer Klang das Fenster von 0,4 s fuellt); das Mass fuer den
/// Bezug ist `momentanMaxLufs`.
[[nodiscard]] audio::Lautheit messeSchlag(const Klang& klang, int rate);

/// Ein Instrument als Drum-Klang: einmal auf C2, `kSpurDauerMs` lang — wie
/// eine Drum-Spur es spielt — und als Schlag gemessen.
[[nodiscard]] audio::Lautheit messeDrumInstrument(const Instrument& instrument, int rate);

}  // namespace lumi::loop
