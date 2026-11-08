/**
 * @file Samples.hpp
 * @note Uebernommen aus IC `rechnung/messung/Samples.hpp` (bb7e6d6, IC Session 5; Instrumente_IC_Konzept.md 34e-3,
 *       Journal J22): der allgemeine Teil. Geaendert: der Name der Umgebungsvariable kommt vom Aufrufer (IC
 *       `IC_SAMPLES`, LumiViz etwa `LUMIVIZ_SALAMANDER`), SHA-256 aus `InstrumentCore/Pruefsumme`.
 * @brief Messkette: der Zugang zu Aufnahmen, die **nur lokal** liegen — die Wurzel aus einer Umgebungsvariable, die
 *        Pruefsummen eines Datensatzes (`SHA256SUMS`)
 *
 * Werkzeuge, die an Aufnahmen messen oder setzen, holen die Wurzel ueber `samplesWurzel` und brechen mit der Meldung
 * ab, wenn sie fehlt oder der Ordner nicht da ist (der Ort kann auf einer externen Festplatte liegen — im Code nie
 * fest). Welche Dateien zu einem Datensatz gehoeren, sagt seine `SHA256SUMS` (Format von `sha256sum`); die Pruefung
 * macht `pruefeDatei`.
 */
#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace lumi::messung
{

/// Die Wurzel aus dem Wert `wert` der Umgebungsvariable `variable` (`wert` = nullptr: nicht gesetzt). Ohne Wert oder
/// ohne Ordner: leer, und `meldung` sagt, was fehlt.
[[nodiscard]] std::optional<std::filesystem::path> samplesWurzelAus(const char* variable, const char* wert, std::string& meldung);
/// Dasselbe aus der Umgebung.
[[nodiscard]] std::optional<std::filesystem::path> samplesWurzel(const char* variable, std::string& meldung);

struct SummenEintrag
{
    std::string sha256;
    std::string pfad;  ///< relativ zum Datensatz, mit „/“
};

/// Die Zeilen einer `SHA256SUMS` (Format von `sha256sum`: Summe, zwei Leerzeichen oder Leerzeichen und `*`, Pfad).
[[nodiscard]] std::vector<SummenEintrag> liesSummen(const std::filesystem::path& datei);

/// SHA-256 einer Datei (hex, klein); leer, wenn sie nicht lesbar ist.
[[nodiscard]] std::string sha256Datei(const std::filesystem::path& datei);

/// Stimmt die Datei mit ihrer Summe?
[[nodiscard]] bool pruefeDatei(const std::filesystem::path& datei, const std::string& sha256);

}  // namespace lumi::messung
