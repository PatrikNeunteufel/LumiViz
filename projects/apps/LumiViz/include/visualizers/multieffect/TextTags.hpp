/**
 ****************************************************************************************
 * @file   TextTags.hpp
 * @brief  Vorlagen im Text-Knoten: Tags des laufenden Titels einsetzen
 *
 * @details
 * Session 86 (Wunsch Patrik, `Offene_Punkte` „Text-Knoten ausbauen"): der Text
 * eines Text-Knotens darf Platzhalter tragen, die beim Zeichnen durch die Tags
 * des laufenden Titels ersetzt werden.
 *
 * @code
 * [{artist} – ]{title}[ ({year})]
 * @endcode
 *
 * - `{name}` — Platzhalter; Namen ohne Rücksicht auf Groß-/Kleinschreibung:
 *   `title`, `artist`, `album`, `year`, `genre`, `track`, `file`. Ein
 *   unbekannter Name bleibt wörtlich stehen (ein Tippfehler fällt so auf).
 * - `[ … ]` — wahlfreier Abschnitt: er entfällt ganz, wenn ein Platzhalter
 *   darin leer ist. So hinterlässt ein fehlendes Jahr kein „ ()". Ein
 *   Abschnitt ohne Platzhalter bleibt wörtlich stehen, samt Klammern.
 * - Ein `;` in einem Tag-Wert wird zum Komma — das Semikolon trennt die Wörter
 *   des Text-Knotens.
 *
 * Reine Zeichenkettenarbeit, ohne Qt — in Unit-Tests ohne GL prüfbar.
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <cctype>
#include <string>
#include <string_view>

namespace lumi::multieffect::texttags
{

/// Tags des laufenden Titels (UTF-8). Leer = nicht vorhanden.
struct SongTags
{
    std::string title;
    std::string artist;
    std::string album;
    std::string year;
    std::string genre;
    std::string track;
    std::string file;  ///< Dateiname ohne Pfad und Endung

    [[nodiscard]] bool operator==(const SongTags&) const = default;
};

/// Die Namen der Platzhalter (Reihenfolge wie im Editor-Menue).
inline constexpr std::array<std::string_view, 7> kNamen = {
    "title", "artist", "album", "year", "genre", "track", "file"};

/// Wert eines Platzhalters; nullptr = Name unbekannt.
[[nodiscard]] inline const std::string* wert(const SongTags& t, std::string_view name)
{
    std::string klein(name);
    for (char& c : klein) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (klein == "title") return &t.title;
    if (klein == "artist") return &t.artist;
    if (klein == "album") return &t.album;
    if (klein == "year") return &t.year;
    if (klein == "genre") return &t.genre;
    if (klein == "track") return &t.track;
    if (klein == "file") return &t.file;
    return nullptr;
}

/// Traegt die Vorlage ueberhaupt einen Platzhalter? (Schnelltest je Frame)
[[nodiscard]] inline bool hatPlatzhalter(std::string_view vorlage)
{
    return vorlage.find('{') != std::string_view::npos;
}

namespace detail
{
/// Setzt die Platzhalter in `s` ein; `leer` meldet, ob einer leer war,
/// `gefunden`, ob es ueberhaupt einen bekannten gab.
inline std::string einsetzen(std::string_view s, const SongTags& t, bool& leer, bool& gefunden)
{
    std::string out;
    for (std::size_t i = 0; i < s.size();)
    {
        if (s[i] == '{')
        {
            const std::size_t zu = s.find('}', i + 1);
            if (zu != std::string_view::npos)
            {
                if (const std::string* w = wert(t, s.substr(i + 1, zu - i - 1)))
                {
                    gefunden = true;
                    if (w->empty()) leer = true;
                    for (const char c : *w) out += c == ';' ? ',' : c;
                    i = zu + 1;
                    continue;
                }
            }
        }
        out += s[i++];
    }
    return out;
}
}  // namespace detail

/// Vorlage mit den Tags fuellen (siehe Dateikopf).
[[nodiscard]] inline std::string ersetze(std::string_view vorlage, const SongTags& t)
{
    std::string out;
    for (std::size_t i = 0; i < vorlage.size();)
    {
        if (vorlage[i] == '[')
        {
            const std::size_t zu = vorlage.find(']', i + 1);
            if (zu != std::string_view::npos)
            {
                bool leer = false;
                bool gefunden = false;
                const std::string innen =
                    detail::einsetzen(vorlage.substr(i + 1, zu - i - 1), t, leer, gefunden);
                if (!gefunden)
                    out += vorlage.substr(i, zu - i + 1);  // kein Platzhalter: woertlich
                else if (!leer)
                    out += innen;
                i = zu + 1;
                continue;
            }
        }
        // bis zum naechsten '[' (oder Ende) am Stueck einsetzen
        const std::size_t naechste = vorlage.find('[', i + 1);
        const std::size_t ende = naechste == std::string_view::npos ? vorlage.size() : naechste;
        bool leer = false;
        bool gefunden = false;
        out += detail::einsetzen(vorlage.substr(i, ende - i), t, leer, gefunden);
        i = ende;
    }
    return out;
}

}  // namespace lumi::multieffect::texttags
