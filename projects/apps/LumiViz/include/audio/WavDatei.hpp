/**
 * @file WavDatei.hpp
 * @brief Eine WAV-Datei schreiben und lesen: Stereo, 32-bit float, 24 oder
 *        16 Bit (docs/audio/Spielweisen_Konzept.md §4.4, Stufe 30b; Entscheide
 *        WX2 a, F5 a)
 *
 * Fuer *Export audio…* und Freeze. Ohne Engine. Geschrieben wird RIFF/WAVE mit
 * dem Abschnitt `fmt ` — PCM (Format 1) fuer 16 und 24 Bit, IEEE float
 * (Format 3, mit dem Abschnitt `fact`) fuer 32-bit float —, das liest jedes
 * Programm. Gelesen wird, was hier geschrieben wird, dazu Mono und
 * `WAVE_FORMAT_EXTENSIBLE` mit denselben Arten.
 *
 * 32-bit float behaelt jeden Wert, auch ueber 0 dB. 24 und 16 Bit kappen bei
 * ±1 und zaehlen, wie viele Werte das traf; 16 Bit wird auf Wunsch mit Dither
 * geschrieben (TPDF, ±1 LSB, aus einem festen Startwert — dieselben Werte
 * ergeben dieselbe Datei).
 *
 * Das Zerlegen beim Lesen steht ohne Qt in `WavFormat.hpp` (WL1 a); hier nur das Oeffnen.
 */
#pragma once

#include "audio/WavFormat.hpp"

#include <QString>

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace lumi::audio
{

/// Was beim Schreiben geschah.
struct WavErgebnis
{
    bool geschrieben = false;
    /// So viele Werte lagen ueber ±1 und wurden gekappt (bei 32-bit float immer 0).
    std::int64_t gekappt = 0;
    /// Der groesste Betrag vor dem Schreiben.
    float spitze = 0.0f;
};

/// `stereo` (links und rechts im Wechsel) nach `pfad` schreiben. `dither` gilt nur fuer
/// 16 Bit. Die Datei entsteht erst, wenn alles geschrieben ist (`QSaveFile`); was ueber
/// die Grenze von RIFF (4 GB) ginge, wird nicht geschrieben.
[[nodiscard]] WavErgebnis schreibeWav(const QString& pfad, std::span<const float> stereo, int rate, WavTiefe tiefe,
                                      bool dither = false);

/// Lesen, was `schreibeWav` schreibt (auch Mono), ueber `liesWavBytes`. Leer = nicht zu lesen.
[[nodiscard]] std::optional<WavInhalt> liesWav(const QString& pfad);

}  // namespace lumi::audio
