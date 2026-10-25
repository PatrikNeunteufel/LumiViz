/**
 * @file WavFormat.cpp
 * @brief Eine WAV-Datei lesen, ohne Qt (Stufe 31d-2, WL1 a)
 */

#include "audio/WavFormat.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string_view>

namespace lumi::audio
{

namespace
{

constexpr std::uint16_t kFormatPcm = 1;
constexpr std::uint16_t kFormatFloat = 3;
constexpr std::uint16_t kFormatErweitert = 0xFFFE;

/// Eine Zahl in der Reihenfolge der Datei (little-endian), unabhaengig vom Rechner.
template <typename T>
T lies(const std::byte* stelle)
{
    std::uint64_t wert = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i)
        wert |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(stelle[i])) << (8 * i);
    return static_cast<T>(wert);
}

float liesFloat(const std::byte* stelle)
{
    const auto bits = lies<std::uint32_t>(stelle);
    float wert;
    std::memcpy(&wert, &bits, sizeof(wert));
    return wert;
}

bool istKennung(const std::byte* stelle, std::string_view kennung)
{
    for (std::size_t i = 0; i < kennung.size(); ++i)
        if (std::to_integer<char>(stelle[i]) != kennung[i]) return false;
    return true;
}

}  // namespace

std::optional<WavInhalt> liesWavBytes(std::span<const std::byte> roh)
{
    if (roh.size() < 12 || !istKennung(roh.data(), "RIFF") || !istKennung(roh.data() + 8, "WAVE")) return std::nullopt;
    std::uint16_t format = 0;
    std::uint16_t kanaele = 0;
    std::uint16_t bits = 0;
    WavInhalt inhalt;
    std::size_t stelle = 12;
    while (stelle + 8 <= roh.size())
    {
        const std::byte* kopf = roh.data() + stelle;
        const auto groesse = lies<std::uint32_t>(kopf + 4);
        const std::byte* anfang = kopf + 8;
        const std::size_t verfuegbar = std::min<std::size_t>(groesse, roh.size() - stelle - 8);
        if (istKennung(kopf, "fmt ") && verfuegbar >= 16)
        {
            format = lies<std::uint16_t>(anfang);
            kanaele = lies<std::uint16_t>(anfang + 2);
            inhalt.rate = static_cast<int>(lies<std::uint32_t>(anfang + 4));
            bits = lies<std::uint16_t>(anfang + 14);
            if (format == kFormatErweitert && verfuegbar >= 26) format = lies<std::uint16_t>(anfang + 24);  // die Unterart
        }
        else if (istKennung(kopf, "data"))
        {
            if (kanaele < 1 || kanaele > 2 || inhalt.rate <= 0) return std::nullopt;
            if (format == kFormatFloat && bits == 32)
                inhalt.tiefe = WavTiefe::Gleitkomma32;
            else if (format == kFormatPcm && bits == 24)
                inhalt.tiefe = WavTiefe::Pcm24;
            else if (format == kFormatPcm && bits == 16)
                inhalt.tiefe = WavTiefe::Pcm16;
            else
                return std::nullopt;
            const std::size_t bytes = bits / 8;
            const std::size_t frames = verfuegbar / (bytes * kanaele);
            inhalt.stereo.reserve(2 * frames);
            const auto wert = [&](const std::byte* p) -> float {
                if (inhalt.tiefe == WavTiefe::Gleitkomma32) return liesFloat(p);
                if (inhalt.tiefe == WavTiefe::Pcm16) return static_cast<float>(lies<std::int16_t>(p) / 32767.0);
                // 24 Bit: die drei Bytes nach oben schieben, das Vorzeichen kommt mit
                const auto drei = static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[0])) |
                                  static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[1])) << 8 |
                                  static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[2])) << 16;
                const auto ganz = static_cast<std::int32_t>(drei << 8) >> 8;
                return static_cast<float>(ganz / 8388607.0);
            };
            for (std::size_t f = 0; f < frames; ++f)
            {
                const std::byte* p = anfang + f * bytes * kanaele;
                inhalt.stereo.push_back(wert(p));
                inhalt.stereo.push_back(wert(p + (kanaele - 1) * bytes));
            }
            return inhalt;
        }
        stelle += 8 + static_cast<std::size_t>(groesse) + (groesse % 2);
    }
    return std::nullopt;
}

std::optional<WavInhalt> liesWavDatei(const std::filesystem::path& pfad)
{
    std::ifstream datei(pfad, std::ios::binary | std::ios::ate);
    if (!datei) return std::nullopt;
    const std::streamoff laenge = datei.tellg();
    if (laenge <= 0) return std::nullopt;
    std::vector<std::byte> roh(static_cast<std::size_t>(laenge));
    datei.seekg(0);
    if (!datei.read(reinterpret_cast<char*>(roh.data()), laenge)) return std::nullopt;
    return liesWavBytes(roh);
}

}  // namespace lumi::audio
