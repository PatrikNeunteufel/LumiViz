/**
 * @file WavFormat.hpp
 * @brief Eine WAV-Datei lesen, ohne Qt (docs/audio/Spielweisen_Konzept.md §4.1, Stufe 31d-2;
 *        Entscheide Patrik S152: WL1 a, nach IK1)
 *
 * Der eine Code, der das Format zerlegt: RIFF/WAVE mit `fmt ` und `data`, PCM 16 und 24 Bit,
 * IEEE float 32 Bit, Mono oder Stereo, auch `WAVE_FORMAT_EXTENSIBLE` mit denselben Arten.
 * `audio::liesWav` (WavDatei.hpp) oeffnet die Datei mit Qt und ruft `liesWavBytes`; die Messung
 * (`Teiltonmessung.hpp`) liest mit `liesWavDatei`. Reines C++.
 */
#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace lumi::audio
{

/// Die Art der Abtastwerte in der Datei (WX2 a).
enum class WavTiefe
{
    Gleitkomma32,  ///< 32-bit float — die Vorgabe, verlustfrei
    Pcm24,
    Pcm16,
};

/// Eine gelesene WAV-Datei: immer Stereo, Werte wie geschrieben (16 und 24 Bit auf ±1).
struct WavInhalt
{
    std::vector<float> stereo;
    int rate = 0;
    WavTiefe tiefe = WavTiefe::Gleitkomma32;
};

/// Den Inhalt einer WAV-Datei zerlegen. Mono kommt auf beide Seiten. Leer = nicht zu lesen.
[[nodiscard]] std::optional<WavInhalt> liesWavBytes(std::span<const std::byte> roh);

/// Eine WAV-Datei vom Datentraeger lesen (`liesWavBytes`). Leer = fehlt oder nicht zu lesen.
[[nodiscard]] std::optional<WavInhalt> liesWavDatei(const std::filesystem::path& pfad);

}  // namespace lumi::audio
