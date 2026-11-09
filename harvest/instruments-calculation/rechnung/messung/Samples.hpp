/**
 * @file Samples.hpp
 * @brief Messkette: der Zugang zu den Aufnahmen — **nur lokal**, die Wurzel aus `IC_SAMPLES` (AUFTRAG_BAU §0, §8;
 *        CLAUDE.md: „Downloads von Aufnahmen nur lokal … der Ort `samples\` kann auf eine externe Festplatte wandern —
 *        im Code nie fest, sondern über `IC_SAMPLES`“)
 *
 * Werkzeuge, die an Aufnahmen messen oder setzen, holen die Wurzel ueber `samplesWurzel` und brechen mit der Meldung
 * ab, wenn sie fehlt oder der Ordner nicht da ist. Unter der Wurzel liegt `<Klasse n - Name>/<Datensatz>/` (A-1 a);
 * welche Dateien dazugehoeren, sagt `SHA256SUMS` im Repo (`klassen/<Klasse>/aufnahmen|daten/<Datensatz>/`), die
 * Pruefung macht `pruefeDatei`. Kein Pfad zu Aufnahmen steht im Code.
 */
#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ic::messung
{

/// Die Wurzel aus dem Wert von `IC_SAMPLES` (`wert` = nullptr: nicht gesetzt). Ohne Wert oder ohne Ordner: leer, und
/// `meldung` sagt, was fehlt.
[[nodiscard]] std::optional<std::filesystem::path> samplesWurzelAus(const char* wert, std::string& meldung);
/// Dasselbe aus der Umgebung.
[[nodiscard]] std::optional<std::filesystem::path> samplesWurzel(std::string& meldung);

struct SummenEintrag
{
    std::string sha256;
    std::string pfad;  ///< relativ zum Datensatz, mit „/“
};

/// Die Zeilen einer `SHA256SUMS` (Format von `sha256sum`: Summe, zwei Leerzeichen, Pfad).
[[nodiscard]] std::vector<SummenEintrag> liesSummen(const std::filesystem::path& datei);

/// SHA-256 einer Datei (hex); leer, wenn sie nicht lesbar ist.
[[nodiscard]] std::string sha256Datei(const std::filesystem::path& datei);

/// Stimmt die Datei mit ihrer Summe?
[[nodiscard]] bool pruefeDatei(const std::filesystem::path& datei, const std::string& sha256);

}  // namespace ic::messung
