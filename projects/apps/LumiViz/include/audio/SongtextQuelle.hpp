/**
 ****************************************************************************************
 * @file   SongtextQuelle.hpp
 * @brief  Den Songtext zu einem Titel finden — lokale Quellen in fester Reihenfolge
 *
 * @details
 * Session 86, Entscheid Patrik 2a: **synchron vor unsynchron, nah am Song vor
 * fern.**
 *
 * | # | Quelle | gilt als |
 * |---|--------|----------|
 * | 1 | eingebettet synchron (ID3 SYLT; oder eingebetteter Text MIT Zeitmarken) | synchron |
 * | 2 | `<Song>.lrc` neben dem Song (mit Zeitmarken) | synchron |
 * | 3 | `<Interpret> - <Titel>.lrc` im Benutzerordner (mit Zeitmarken) | synchron |
 * | 4 | eingebettet unsynchron (ID3 USLT, Vorbis LYRICS …) | unsynchron |
 * | 5 | `<Song>.txt` neben dem Song — oder eine `.lrc` dort ohne Zeitmarken | unsynchron |
 * | 6 | `<Interpret> - <Titel>.txt` im Benutzerordner — oder `.lrc` ohne Zeitmarken | unsynchron |
 *
 * Synchron ist, was Zeitmarken TRAEGT — die Endung entscheidet nicht
 * (`LyricsModel.hpp`). Der Online-Abruf (Entscheid 3a, Opt-in) kommt als
 * siebte Stufe dahinter und legt seinen Fund ueber `speichereSongtext()` ab
 * (Entscheid 4a); dieselbe Funktion nimmt ein spaeteres Werkzeug zum
 * Nachsynchronisieren.
 ****************************************************************************************
 */

#pragma once

#include "audio/AudioEvents.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"

#include <QString>

namespace lumi::audio
{

/// Benutzerordner fuer Songtexte: `<AppDataLocation>/lyrics`, unter Windows
/// `%APPDATA%\LumiViz Project\LumiViz\lyrics` (wird hier nicht angelegt).
[[nodiscard]] QString songtextOrdner();

/// Dateiname OHNE Endung im Benutzerordner: `<Interpret> - <Titel>`, fehlt der
/// Interpret nur der Titel; fuer Dateinamen verbotene Zeichen werden zu `_`.
/// Leer, wenn der Titel fehlt.
[[nodiscard]] QString songtextName(const TrackInfo& track);

/// Gilt `text` fuer diesen Titel? Traegt die Datei eine Dauer (`[length:]`),
/// muss sie auf ±2,5 s passen; ohne Angabe (selbst angelegt, aeltere Datei)
/// gilt der Text immer.
[[nodiscard]] bool songtextPasst(const lumi::multieffect::lyrics::Songtext& text,
                                 const TrackInfo& track);

/// Pfad OHNE Endung, unter dem ein Text fuer `track` im Benutzerordner liegt
/// bzw. abgelegt wird: `<Interpret> - <Titel>` — oder, wenn dieser Name von
/// einer anderen Fassung des Songs belegt ist, `<Interpret> - <Titel> [m-ss]`
/// mit der Dauer dieses Titels. Leer, wenn kein Name zu bilden ist.
[[nodiscard]] QString songtextBenutzerBasis(const TrackInfo& track,
                                            const QString& benutzerOrdner = songtextOrdner());

/// Songtext nach der Reihenfolge oben; leer (`leer()`), wenn nichts gefunden.
/// `Songtext::quelle` nennt die Herkunft (englisch, fuer die Anzeige).
[[nodiscard]] lumi::multieffect::lyrics::Songtext findeSongtext(
    const TrackInfo& track, const QString& benutzerOrdner = songtextOrdner());

/// Ergebnis von `weiseSongtextZu`.
struct SongtextZuweisung
{
    bool ok = false;  ///< false = nichts Lesbares in der Datei, kein Name, Schreibfehler
    QString ziel;     ///< die Kopie im Benutzerordner (absoluter Pfad)
    /// Was die Suche fuer den Titel danach liefert — das zeigt der Knoten.
    lumi::multieffect::lyrics::Songtext gilt;
    /// Die Kopie liegt da, aber eine Quelle mit Vorrang gewinnt (eingebetteter
    /// Text, Datei neben dem Song, eine eigene `.lrc` im Benutzerordner).
    bool verdeckt = false;
};

/**
 * S88 („Lyrics file…", Entscheid Patrik: kopieren): eine vorhandene `.lrc`/
 * `.txt` dem Titel zuweisen — sie wird UNVERAENDERT in den Benutzerordner
 * kopiert, unter `<Interpret> - <Titel>` (`songtextBenutzerBasis`), mit `.lrc`,
 * wenn sie Zeitmarken traegt, sonst `.txt`. Eine dort schon liegende Datei
 * gleichen Namens wird ersetzt; die mit der anderen Endung nur entfernt, wenn
 * LumiViz sie geholt hat. Die Suchreihenfolge bleibt, wie sie ist — gewinnt
 * danach eine andere Quelle, meldet `verdeckt` das.
 */
[[nodiscard]] SongtextZuweisung weiseSongtextZu(const TrackInfo& track, const QString& quellPfad,
                                                const QString& benutzerOrdner = songtextOrdner());

/// Schluessel eines Titels fuer Ablagen im Benutzerordner (Merker, Versatz):
/// Interpret, Titel, Album und Dauer in Sekunden, ohne Ruecksicht auf
/// Gross-/Kleinschreibung und Leerraum. Zwei Fassungen eines Songs haben
/// verschiedene Schluessel.
[[nodiscard]] QString songtextSchluessel(const TrackInfo& track);

/// S88 (Entscheid Patrik a): Versatz in ms, der nur fuer `track` gilt
/// (`offsets.json` im Benutzerordner der Songtexte) — positiv = Text frueher,
/// wie `[offset:]`. 0 = keiner. Er wirkt zusaetzlich zum Feld des Knotens und
/// zur Datei, fuer jede Quelle gleich (auch eingebetteten Text); eine
/// Songtext-Datei wird dafuer nie umgeschrieben.
[[nodiscard]] int songtextTitelVersatzMs(const TrackInfo& track,
                                         const QString& benutzerOrdner = songtextOrdner());

/// Versatz fuer `track` festhalten; 0 entfernt den Eintrag. false = Titel ohne
/// Namen oder Schreibfehler.
bool setzeSongtextTitelVersatz(const TrackInfo& track, int ms,
                               const QString& benutzerOrdner = songtextOrdner());

/// Songtext als UTF-8-LRC nach `pfad` schreiben (Ordner wird angelegt).
bool speichereSongtext(const lumi::multieffect::lyrics::Songtext& text, const QString& pfad);

}  // namespace lumi::audio
