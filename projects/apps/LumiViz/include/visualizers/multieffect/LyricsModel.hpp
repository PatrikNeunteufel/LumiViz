/**
 ****************************************************************************************
 * @file   LyricsModel.hpp
 * @brief  Songtext als Datenmodell: Zeilen mit Zeit, Stimme und Wort-Zeiten; LRC lesen/schreiben
 *
 * @details
 * Session 86 (Entscheide Patrik 1a–7a, `docs/Offene_Punkte.md` „Songtexte").
 * Das Modell ist die EINE Schnittstelle zwischen allem, was einen Songtext
 * liefert, und allem, was ihn zeigt:
 *
 * @code
 *   Datei (.lrc/.txt) ─┐
 *   eingebettete Tags ─┤                         ┌─ Lyrics-Knoten (Bild)
 *   Online-Abruf ──────┼─►  Songtext  ──────────►┤
 *   Nachsynchronisieren┘   (dieses Modell)       └─ schreibeLrc() -> .lrc
 *   (von Hand, Skript, KI)
 * @endcode
 *
 * Ob ein Text **synchron** ist, steht im Inhalt, nicht in der Dateiendung: er
 * ist es, sobald Zeilen eine Zeit tragen. Eine `.lrc` ohne Zeitmarken ist ein
 * unsynchroner Text, ein eingebetteter Fliesstext mit `[mm:ss.xx]`-Marken ein
 * synchroner. Ein Werkzeug, das einen unsynchronen Text nachtraeglich mit
 * Zeiten versieht (Mitklopfen, Skript, KI-Ausrichtung), fuellt also nur
 * `Zeile::zeitMs` und ruft `schreibeLrc()` — der Knoten merkt keinen
 * Unterschied.
 *
 * ## Gelesenes LRC
 *
 * - Zeilen-Zeitmarken `[mm:ss]`, `[mm:ss.xx]`, `[mm:ss.xxx]`, `[mm:ss:xx]`,
 *   mehrere je Zeile (Wiederholungen).
 * - Wort-Zeitmarken `<mm:ss.xx>` im Text (erweitertes LRC) -> `Zeile::worte`.
 * - `[source:…]` -> `Songtext::herkunft` (eigene Kopfzeile von LumiViz fuer
 *   aus dem Netz geholte Texte; beim Schreiben steht `[re:LumiViz]` davor).
 * - `[offset:±ms]` -> `Songtext::versatzMs`; ein positiver Wert laesst den Text
 *   FRUEHER erscheinen (wirksame Zeit = Marke − Versatz). Uebrige Kopfzeilen
 *   (`[ar:]`, `[ti:]`, `[al:]`, `[by:]`, `[length:]` …) werden uebergangen.
 * - Stimm-Marken am Zeilenanfang (Duette, Entscheid 7a): `v1:`, `v2:`,
 *   `v1+v2:` sowie `M:`, `F:`, `D:`. Eine Marke gilt bis zur naechsten.
 *
 * Reine Zeichenkettenarbeit, ohne Qt — in Unit-Tests ohne GL pruefbar.
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::multieffect::lyrics
{

/// Wer eine Zeile singt (Duette).
enum class Stimme : std::uint8_t
{
    Keine,  ///< keine Marke im Text
    Eins,
    Zwei,
    Beide
};

/// Ein Wort mit eigener Zeit (erweitertes LRC, SYLT). `text` traegt seine
/// Leerzeichen — die Worte einer Zeile aneinandergereiht ergeben `Zeile::text`.
struct Wort
{
    std::int64_t zeitMs = 0;
    std::string text;

    [[nodiscard]] bool operator==(const Wort&) const = default;
};

struct Zeile
{
    std::int64_t zeitMs = -1;  ///< Beginn in ms; -1 = ohne Zeit (unsynchron)
    std::string text;          ///< leer = Pause/Leerzeile
    Stimme stimme = Stimme::Keine;
    std::vector<Wort> worte;   ///< leer = keine Wort-Zeiten

    [[nodiscard]] bool operator==(const Zeile&) const = default;
};

struct Songtext
{
    std::vector<Zeile> zeilen;
    bool synchron = false;       ///< mindestens eine Zeile traegt eine Zeit
    std::int64_t versatzMs = 0;  ///< `[offset:]` — positiv = Text frueher
    std::string quelle;          ///< Herkunft fuer die Anzeige ("file.lrc", "embedded" …)
    /// Herkunft eines aus dem Netz geholten Texts, wie sie in der Datei steht
    /// (Kopfzeile `[source:…]`, Entscheid Patrik S86): Dienst, Kennung des
    /// Eintrags, Datum — z. B. "LRCLIB id 3396226, fetched 2026-10-01". Leer =
    /// kein Vermerk (selbst angelegt, eingebettet).
    std::string herkunft;
    /// Dauer des Titels, fuer den der Text gilt (LRC-Kopfzeile `[length:mm:ss]`),
    /// 0 = nicht angegeben. S87: zwei Fassungen eines Songs tragen im
    /// Benutzerordner denselben Namen `<Interpret> - <Titel>` — die Dauer
    /// haelt sie auseinander (`audio/SongtextQuelle`).
    std::int64_t laengeMs = 0;
    /// Absoluter Pfad der Datei, aus der der Text stammt; leer = eingebettet
    /// oder ohne Datei. Fuer „Ordner oeffnen" im Editor des Lyrics-Knotens.
    std::string datei;

    [[nodiscard]] bool leer() const { return zeilen.empty(); }
    [[nodiscard]] bool operator==(const Songtext&) const = default;
};

/// Stand eines Online-Abrufs fuer den laufenden Titel (S87). Steht beim
/// Modell, weil ihn der Abruf (`audio/SongtextOnline`) setzt und die
/// Oberflaeche ihn zeigt — beide kennen sonst nur dieses Modell.
enum class Abruf : std::uint8_t
{
    Ruhe,           ///< es wird nicht gefragt (lokal gefunden, abgeschaltet, Angaben fehlen)
    Laedt,          ///< Anfrage unterwegs
    Wartet,         ///< der Dienst bat um Geduld — die Wiederholung ist angesetzt
    Gefunden,       ///< Text da
    NichtGefunden,  ///< der Dienst kennt den Titel nicht oder fuehrt ihn ohne Text
    Fehlgeschlagen  ///< kein Netz, Fehler, oder nach allen Wiederholungen aufgegeben
};

namespace detail
{
inline std::string_view trim(std::string_view s)
{
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '\r'))
        s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r'))
        s.remove_suffix(1);
    return s;
}

/// "mm:ss", "mm:ss.xx", "mm:ss.xxx", "mm:ss:xx" -> ms; -1 = keine Zeit.
inline std::int64_t zeit(std::string_view s)
{
    std::size_t i = 0;
    const auto zahl = [&](std::int64_t& out, int& stellen) {
        out = 0;
        stellen = 0;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
        {
            out = out * 10 + (s[i] - '0');
            ++i;
            ++stellen;
        }
        return stellen > 0;
    };
    std::int64_t min = 0, sek = 0, bruch = 0;
    int n = 0;
    if (!zahl(min, n)) return -1;
    if (i >= s.size() || s[i] != ':') return -1;
    ++i;
    if (!zahl(sek, n) || sek > 59) return -1;
    std::int64_t ms = (min * 60 + sek) * 1000;
    if (i < s.size() && (s[i] == '.' || s[i] == ':'))
    {
        ++i;
        if (!zahl(bruch, n)) return -1;
        for (; n < 3; ++n) bruch *= 10;  // "5" = 500 ms, "05" = 50 ms
        for (; n > 3; --n) bruch /= 10;
        ms += bruch;
    }
    return i == s.size() ? ms : -1;
}

inline std::string zeitText(std::int64_t ms)
{
    if (ms < 0) ms = 0;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02lld:%02lld.%02lld", static_cast<long long>(ms / 60000),
                  static_cast<long long>((ms / 1000) % 60),
                  static_cast<long long>((ms % 1000) / 10));
    return buf;
}

/// Stimm-Marke am Anfang von `s` abtrennen; false = keine.
inline bool stimmMarke(std::string_view& s, Stimme& out)
{
    struct Marke
    {
        std::string_view text;
        Stimme stimme;
    };
    static constexpr Marke kMarken[] = {
        {"v1+v2:", Stimme::Beide}, {"v1:", Stimme::Eins}, {"v2:", Stimme::Zwei},
        {"M:", Stimme::Eins},      {"F:", Stimme::Zwei},  {"D:", Stimme::Beide}};
    for (const Marke& m : kMarken)
    {
        if (s.size() < m.text.size()) continue;
        bool gleich = true;
        for (std::size_t k = 0; k < m.text.size() && gleich; ++k)
            gleich = std::tolower(static_cast<unsigned char>(s[k])) ==
                     std::tolower(static_cast<unsigned char>(m.text[k]));
        if (!gleich) continue;
        s.remove_prefix(m.text.size());
        s = trim(s);
        out = m.stimme;
        return true;
    }
    return false;
}

/// Text mit Wort-Zeitmarken `<mm:ss.xx>` in Klartext + Worte zerlegen.
inline void worte(std::string_view roh, std::int64_t zeilenZeit, Zeile& z)
{
    std::string klar;
    std::vector<Wort> liste;
    std::int64_t laufend = zeilenZeit;
    std::string stueck;
    bool marken = false;
    const auto ablegen = [&] {
        if (stueck.empty()) return;
        liste.push_back({laufend, stueck});
        klar += stueck;
        stueck.clear();
    };
    for (std::size_t i = 0; i < roh.size();)
    {
        if (roh[i] == '<')
        {
            const std::size_t zu = roh.find('>', i + 1);
            if (zu != std::string_view::npos)
            {
                const std::int64_t t = zeit(roh.substr(i + 1, zu - i - 1));
                if (t >= 0)
                {
                    ablegen();
                    laufend = t;
                    marken = true;
                    i = zu + 1;
                    continue;
                }
            }
        }
        stueck += roh[i++];
    }
    ablegen();
    z.text = std::string(trim(klar));
    if (marken) z.worte = std::move(liste);
}
}  // namespace detail

/**
 * LRC oder Fliesstext lesen. Ohne eine einzige Zeilen-Zeitmarke ist das
 * Ergebnis unsynchron: jede Textzeile wird eine `Zeile` ohne Zeit (Leerzeilen
 * bleiben als Strophentrenner erhalten, am Anfang und Ende entfallen sie).
 */
[[nodiscard]] inline Songtext leseLrc(std::string_view text)
{
    Songtext out;
    std::vector<Zeile> ohneZeit;
    Stimme stimme = Stimme::Keine;
    if (text.substr(0, 3) == "\xEF\xBB\xBF") text.remove_prefix(3);  // UTF-8-BOM

    std::size_t pos = 0;
    while (pos <= text.size())
    {
        std::size_t ende = text.find('\n', pos);
        if (ende == std::string_view::npos) ende = text.size();
        std::string_view zeile = detail::trim(text.substr(pos, ende - pos));
        pos = ende + 1;

        // fuehrende [..]-Marken: Zeiten sammeln, Kopfzeilen auswerten
        std::vector<std::int64_t> zeiten;
        bool kopf = false;
        while (!zeile.empty() && zeile.front() == '[')
        {
            const std::size_t zu = zeile.find(']');
            if (zu == std::string_view::npos) break;
            const std::string_view innen = zeile.substr(1, zu - 1);
            const std::int64_t t = detail::zeit(innen);
            if (t >= 0)
            {
                zeiten.push_back(t);
            }
            else
            {
                const std::size_t dp = innen.find(':');
                if (dp == std::string_view::npos) break;  // "[Refrain]" ist Text
                std::string name(innen.substr(0, dp));
                for (char& c : name)
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                bool nurBuchstaben = !name.empty();
                for (const char c : name)
                    nurBuchstaben =
                        nurBuchstaben && std::isalpha(static_cast<unsigned char>(c)) != 0;
                if (!nurBuchstaben) break;
                kopf = true;
                if (name == "offset")
                {
                    const std::string wert(detail::trim(innen.substr(dp + 1)));
                    out.versatzMs = std::atoll(wert.c_str());
                }
                else if (name == "source")
                {
                    out.herkunft = std::string(detail::trim(innen.substr(dp + 1)));
                }
                else if (name == "length")
                {
                    out.laengeMs = std::max<std::int64_t>(
                        0, detail::zeit(detail::trim(innen.substr(dp + 1))));
                }
            }
            zeile = detail::trim(zeile.substr(zu + 1));
        }
        if (kopf && zeiten.empty() && zeile.empty()) continue;  // reine Kopfzeile

        Stimme hier = stimme;
        if (detail::stimmMarke(zeile, hier)) stimme = hier;

        if (zeiten.empty())
        {
            Zeile z;
            z.stimme = stimme;
            detail::worte(zeile, -1, z);
            z.worte.clear();  // Wort-Zeiten ohne Zeilen-Zeit sind wertlos
            ohneZeit.push_back(std::move(z));
            continue;
        }
        for (const std::int64_t t : zeiten)
        {
            Zeile z;
            z.zeitMs = t;
            z.stimme = stimme;
            detail::worte(zeile, t, z);
            out.zeilen.push_back(std::move(z));
        }
    }

    if (!out.zeilen.empty())
    {
        out.synchron = true;
        std::stable_sort(out.zeilen.begin(), out.zeilen.end(),
                         [](const Zeile& a, const Zeile& b) { return a.zeitMs < b.zeitMs; });
        return out;
    }
    // unsynchron: Leerzeilen am Rand weg, innen als Strophentrenner behalten
    while (!ohneZeit.empty() && ohneZeit.front().text.empty()) ohneZeit.erase(ohneZeit.begin());
    while (!ohneZeit.empty() && ohneZeit.back().text.empty()) ohneZeit.pop_back();
    out.zeilen = std::move(ohneZeit);
    return out;
}

/// Als LRC schreiben (UTF-8). Zeilen ohne Zeit gehen als blosser Text hinaus.
[[nodiscard]] inline std::string schreibeLrc(const Songtext& s)
{
    std::string out;
    // Herkunftsvermerk zuerst — Kopfzeilen, die kein Leser als Text zeigt
    if (!s.herkunft.empty()) out += "[re:LumiViz]\n[source:" + s.herkunft + "]\n";
    if (s.laengeMs > 0) out += "[length:" + detail::zeitText(s.laengeMs) + "]\n";
    if (s.versatzMs != 0) out += "[offset:" + std::to_string(s.versatzMs) + "]\n";
    Stimme zuletzt = Stimme::Keine;
    for (const Zeile& z : s.zeilen)
    {
        if (z.zeitMs >= 0) out += "[" + detail::zeitText(z.zeitMs) + "]";
        if (z.stimme != zuletzt && z.stimme != Stimme::Keine)
        {
            out += z.stimme == Stimme::Eins   ? "v1: "
                   : z.stimme == Stimme::Zwei ? "v2: "
                                              : "v1+v2: ";
            zuletzt = z.stimme;
        }
        if (z.worte.empty() || z.zeitMs < 0)
        {
            out += z.text;
        }
        else
        {
            for (std::size_t i = 0; i < z.worte.size(); ++i)
            {
                // das erste Wort traegt die Zeilen-Zeit — keine doppelte Marke
                if (i > 0 || z.worte[i].zeitMs != z.zeitMs)
                    out += "<" + detail::zeitText(z.worte[i].zeitMs) + ">";
                out += z.worte[i].text;
            }
        }
        out += "\n";
    }
    return out;
}

/// Index der laufenden Zeile bei Abspielposition `posMs`; -1 = vor der ersten
/// oder Text unsynchron. Der LRC-Versatz ist eingerechnet.
[[nodiscard]] inline int zeileBei(const Songtext& s, std::int64_t posMs)
{
    if (!s.synchron) return -1;
    const std::int64_t t = posMs + s.versatzMs;
    int idx = -1;
    int lo = 0;
    int hi = static_cast<int>(s.zeilen.size()) - 1;
    while (lo <= hi)
    {
        const int mitte = (lo + hi) / 2;
        if (s.zeilen[static_cast<std::size_t>(mitte)].zeitMs <= t)
        {
            idx = mitte;
            lo = mitte + 1;
        }
        else
        {
            hi = mitte - 1;
        }
    }
    return idx;
}

/// Wie viele Worte der Zeile bei `posMs` schon gesungen sind (0..worte.size()).
[[nodiscard]] inline int worteBei(const Songtext& s, const Zeile& z, std::int64_t posMs)
{
    const std::int64_t t = posMs + s.versatzMs;
    int n = 0;
    for (const Wort& w : z.worte)
    {
        if (w.zeitMs <= t) ++n;
    }
    return n;
}

/// Zahl der Worte einer Zeile (S90): mit Wort-Zeiten die Zahl der Wort-
/// Eintraege, sonst die durch Leerraum getrennten Stuecke des Texts.
[[nodiscard]] inline int wortAnzahl(const Zeile& z)
{
    if (!z.worte.empty()) return static_cast<int>(z.worte.size());
    int n = 0;
    bool imWort = false;
    for (const char c : z.text)
    {
        const bool leer = c == ' ' || c == '\t';
        if (!leer && !imWort) ++n;
        imWort = !leer;
    }
    return n;
}

}  // namespace lumi::multieffect::lyrics
