/**
 * @file Json.hpp
 * @brief Gemeinsam: ein kleiner JSON-Leser und -Schreiber fuer Werte- und Bezugsdateien (ohne Qt, ohne fremde
 *        Bibliothek)
 *
 * Liest Objekte, Listen, Zahlen (als double), Texte (mit \\uXXXX als UTF-8), true/false/null. Objekte behalten
 * die Reihenfolge der Schluessel. Fuer die Bezugsdatei des Fluegels (`werte/hammer_bezug_saite.json`) und die
 * Vorgaben der Klassen; kein allgemeiner Ersatz fuer eine JSON-Bibliothek.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ic
{

struct JsonWert
{
    enum class Art
    {
        Null,
        Wahrheit,
        Zahl,
        Text,
        Liste,
        Objekt
    };
    Art art = Art::Null;
    bool wahrheit = false;
    double zahl = 0.0;
    std::string text;
    std::vector<JsonWert> liste;
    /// Objekt: die Schluessel und ihre Werte in derselben Reihenfolge (zwei Listen — ein Paar mit dem noch
    /// unvollstaendigen Typ nimmt clang nicht an)
    std::vector<std::string> schluessel;
    std::vector<JsonWert> objekt;

    /// Haengt an ein Objekt an.
    void setze(std::string name, JsonWert wert);

    /// Der Wert unter `name`; ein leerer (Null), wenn es ihn nicht gibt.
    [[nodiscard]] const JsonWert& operator[](const std::string& name) const;
    [[nodiscard]] bool hat(const std::string& name) const;
};

/// Liest JSON; false bei einem Fehler (dann steht in `fehler`, wo).
bool leseJson(const std::string& text, JsonWert& wert, std::string* fehler = nullptr);
bool leseJsonDatei(const std::string& pfad, JsonWert& wert, std::string* fehler = nullptr);
/// Schreibt JSON eingerueckt; Zahlen mit 17 gueltigen Stellen (genau zurueck).
[[nodiscard]] std::string alsJson(const JsonWert& wert, int einzug = 0);

}  // namespace ic
