/**
 * @file Pruefsumme.hpp
 * @brief InstrumentCore: SHA-256 nach FIPS 180-4 ohne Qt — der Abdruck eines Tons als Bitgleich-Waechter und der
 *        Abdruck der Eingaben gestimmter Werte (Instrumente_IC_Konzept.md IK2-2 a, IK2-11 a, KN2 a)
 *
 * Aus IC `rechnung/gemeinsam/Werkzeug.hpp`/`.cpp` (`bb7e6d6`), dort nach FIPS 180-4 geschrieben; nur Namensraum und
 * Ort geaendert. Die Qt-Schicht der App rechnet ihre Abdruecke weiter mit `QCryptographicHash` (Boden, Koerper,
 * Freeze); der Kern braucht einen ohne Qt.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace lumi::kern
{

/// SHA-256 stueckweise (fuer grosse Eingaben).
class Sha256
{
public:
    Sha256();
    void dazu(const std::uint8_t* daten, std::size_t n);
    void dazu(std::string_view text);
    /// Der Abdruck (64 Zeichen hex, klein); danach ist das Objekt verbraucht.
    [[nodiscard]] std::string fertig();

private:
    std::uint32_t m_h[8];
    std::uint8_t m_puffer[64];
    std::size_t m_imPuffer = 0;
    std::uint64_t m_laenge = 0;
};

/// SHA-256 (hex) ueber Bytes.
[[nodiscard]] std::string sha256(std::span<const std::uint8_t> bytes);
/// SHA-256 (hex) ueber die Bytes eines Texts.
[[nodiscard]] std::string sha256(std::string_view text);
/// SHA-256 (hex) ueber die float-Werte eines Tons (je Wert die vier Bytes in Little-Endian).
[[nodiscard]] std::string abdruck(std::span<const float> werte);

}  // namespace lumi::kern
