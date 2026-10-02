/**
 ****************************************************************************************
 * @file   SongtextOnline.hpp
 * @brief  Songtext aus dem Netz holen (LRCLIB) — nur auf ausdrueckliche Einstellung
 *
 * @details
 * Session 86, Entscheide Patrik 3a/4a: die Online-Suche ist ein **Opt-in**
 * (`lyrics/onlineSearch`, Vorgabe aus), laeuft erst, wenn keine lokale Quelle
 * einen Text liefert, und **speichert** ihren Fund — je nach Einstellung
 * `lyrics/saveLocation` im Benutzerordner (Vorgabe) oder neben dem Song.
 *
 * Dienst: **LRCLIB**, `GET https://lrclib.net/api/get` (Dokumentation
 * https://lrclib.net/docs, gelesen am 2026-10-01):
 * - kein API-Schluessel; verlangt wird ein `User-Agent` mit Programmname,
 *   Version und Projektseite;
 * - Pflicht: `track_name` und `artist_name`; empfohlen `album_name` und
 *   `duration` in Sekunden (1..3600) — ein Treffer verlangt die Dauer auf
 *   ±2 s;
 * - Antwort: `syncedLyrics` (LRC mit Zeilen-Zeitmarken) und `plainLyrics`,
 *   beide `null` bei `instrumental: true`; kein Treffer = 404;
 * - 429 mit `Retry-After` muss befolgt werden — bis dahin wird nicht gefragt.
 *
 * Session 87, Befund aus dem Lauf vom 2026-10-01 (62 Anfragen): 19-mal kam
 * **503 mit `Retry-After: 1`** — der Dienst ist beschaeftigt, dieselbe Anfrage
 * gelingt meist Sekunden spaeter. Die Dokumentation (am 2026-10-01 erneut
 * gelesen) nennt das selbst: „503 ServerOverloaded — The server is temporarily
 * busy; retry after the Retry-After header (usually 1 second)", und zu 404:
 * fehlende Titel koenne ein Hintergrunddienst nachholen, sie seien dann bei
 * einer spaeteren Anfrage da. Dazu „Request Throttling": Anfragen nacheinander
 * und mit 200–500 ms Abstand — deshalb die Atempause vor der ersten Anfrage.
 * `/api/search`: `q` ODER `track_name` ist Pflicht, `artist_name` wahlfrei,
 * hoechstens 20 Eintraege, Antwort ein JSON-Array derselben Datensaetze. Ohne Wiederholung erschien der Text erst
 * beim naechsten Abspielen. Deshalb fragt `SongtextOnline` fuer den laufenden
 * Titel von selbst wieder (`songtextWartezeitMs`) und meldet den Stand
 * (`SongtextAbruf`).
 *
 * **Datenschutz:** eine Anfrage sendet Interpret, Titel, Album und Dauer des
 * laufenden Titels an lrclib.net. Ohne die Einstellung geht nichts hinaus.
 * Die Rechte an den Texten liegen bei Dritten; die Dokumentation des Dienstes
 * sagt dazu nichts — gespeichert wird nur lokal, fuer den eigenen Gebrauch.
 ****************************************************************************************
 */

#pragma once

#include "audio/AudioEvents.hpp"
#include "audio/SongtextQuelle.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"

#include <vector>

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace lumi::audio
{

/// Wohin ein Online-Fund gespeichert wird (Entscheid 4a). Werte = QSettings.
enum class SongtextAblage
{
    Benutzerordner = 0,  ///< `songtextOrdner()/<Interpret> - <Titel>` (Vorgabe)
    NebenDemSong = 1     ///< `<Song>` mit `.lrc`/`.txt`
};

/// Einstellungen (QSettings): Opt-in und Ablage.
[[nodiscard]] bool songtextOnlineErlaubt();
[[nodiscard]] SongtextAblage songtextAblage();

/// Anfrage-URL fuer `track`; leer, wenn Titel oder Interpret fehlen (beide
/// sind Pflicht) — dann wird gar nicht gefragt.
[[nodiscard]] QUrl lrclibUrl(const TrackInfo& track);

/// Antwort von `/api/get` lesen: synchron vor unsynchron. Leer bei
/// instrumental, fehlenden Feldern oder kaputtem JSON.
[[nodiscard]] lumi::multieffect::lyrics::Songtext songtextAusLrclib(const QByteArray& json);

/**
 * Zielpfad eines Funds (ohne ihn zu schreiben): synchron `.lrc`, unsynchron
 * `.txt`. Neben dem Song nur, wenn der Ordner beschreibbar ist — sonst der
 * Benutzerordner (`ausgewichen` meldet das). Leer, wenn kein Name zu bilden ist.
 */
[[nodiscard]] QString songtextZiel(const TrackInfo& track, bool synchron, SongtextAblage ablage,
                                   const QString& benutzerOrdner, bool* ausgewichen = nullptr);

// --- S87: Alternativen, Uebernehmen, Sperrliste ------------------------------
//
// Wunsch Patrik: ein falscher Text soll sich ersetzen lassen (zwei Fassungen
// eines Songs, Titel mit Zusaetzen wie „(Feat. …)"). `/api/search` liefert
// mehrere Eintraege; die Suchbegriffe sind frei waehlbar. „Wrong lyrics"
// verwirft den gezeigten Text UND sperrt seine Kennung, „eine bessere
// nehmen" ersetzt ihn ohne Sperre.

/// Such-URL (`/api/search`) zu frei waehlbarem Titel und Interpret; leer ohne
/// Titel. Der Interpret ist hier wahlfrei.
[[nodiscard]] QUrl lrclibSuchUrl(const QString& titel, const QString& interpret);

/// Titel ohne Zusaetze in Klammern: „Try (Video Version)" -> „Try",
/// „Creep [live]" -> „Creep". Bleibt nichts uebrig, kommt der Titel
/// unveraendert zurueck. (S87, Befund Patrik: solche Titel fand der Dienst
/// nie, ohne den Zusatz schon.)
[[nodiscard]] QString songtextTitelOhneZusatz(const QString& titel);

/// Interpret ohne Gaeste: „Schiller feat. Nadia Ali" -> „Schiller",
/// „Republica;Jonny Male" -> „Republica".
[[nodiscard]] QString songtextInterpretOhneZusatz(const QString& interpret);

/// Aus den Eintraegen einer Suche den fuer `track` waehlen: mit Text, nicht
/// gesperrt, Dauer auf ±2 s wie beim Dienst selbst; synchron vor unsynchron.
/// -1 = keiner — dann entscheidet der Mensch („Alternatives…").
[[nodiscard]] int waehleKandidat(const std::vector<struct SongtextKandidat>& kandidaten,
                                 const TrackInfo& track,
                                 const QString& benutzerOrdner = songtextOrdner());

/// Ein Eintrag aus der Suche.
struct SongtextKandidat
{
    qint64 id = 0;
    QString titel;
    QString interpret;
    QString album;
    int dauerSek = 0;
    bool instrumental = false;
    lumi::multieffect::lyrics::Songtext text;  ///< leer bei instrumental / ohne Text
};

/// Antwort von `/api/search` lesen (ein JSON-Array); kaputtes JSON = leer.
[[nodiscard]] std::vector<SongtextKandidat> kandidatenAusLrclib(const QByteArray& json);

/// Kennung aus einem Herkunftsvermerk ("LRCLIB id 7, fetched …"); 0 = keine.
[[nodiscard]] qint64 lrclibKennung(const std::string& herkunft);

/**
 * Einen Fund fuer `track` uebernehmen: Datum an den Herkunftsvermerk, nach der
 * Einstellung speichern, `quelle`/`datei` setzen. Eine fruehere, von LumiViz
 * geholte Datei desselben Titels mit der ANDEREN Endung wird entfernt — sonst
 * gewoenne eine alte `.lrc` gegen eine neu gewaehlte `.txt`. Dateien ohne
 * Herkunftsvermerk (selbst angelegt) bleiben unberuehrt.
 */
[[nodiscard]] lumi::multieffect::lyrics::Songtext uebernehmeSongtext(
    const TrackInfo& track, lumi::multieffect::lyrics::Songtext text,
    const QString& benutzerOrdner = songtextOrdner());

/// Ein gesperrter Eintrag: der Abruf nimmt ihn nicht mehr.
struct SongtextSperre
{
    qint64 id = 0;
    QString titel;  ///< `<Interpret> - <Titel>` zur Anzeige
};

/// Sperrliste (`blocked.json` im Benutzerordner der Songtexte).
[[nodiscard]] std::vector<SongtextSperre> songtextSperrliste(
    const QString& benutzerOrdner = songtextOrdner());
void sperreSongtext(qint64 id, const QString& titel,
                    const QString& benutzerOrdner = songtextOrdner());
void entsperreSongtext(qint64 id, const QString& benutzerOrdner = songtextOrdner());
[[nodiscard]] bool songtextGesperrt(qint64 id, const QString& benutzerOrdner = songtextOrdner());

/**
 * „Wrong lyrics": den gezeigten, aus dem Netz geholten Text verwerfen — die
 * Datei loeschen und die Kennung sperren. Nur fuer Texte MIT Herkunftsvermerk
 * (eine selbst angelegte Datei wird nie geloescht); sonst `false`.
 */
bool verwerfeSongtext(const TrackInfo& track, const std::string& herkunft,
                      const std::string& datei,
                      const QString& benutzerOrdner = songtextOrdner());

// --- S88: Merker „online nichts gefunden" ------------------------------------
//
// Bis S87 loeste jeder Titel ohne Text bei JEDEM Abspielen eine Anfrage aus
// (404, instrumental, keine Fassung mit passender Dauer). Der Merker haelt das
// Ergebnis fest; solange er gilt, geht fuer diesen Titel nichts hinaus. Er
// laeuft ab, weil der Dienst fehlende Titel nachtragen kann (lrclib.net/docs
// zu 404, gelesen 2026-10-01). Eine Freigabe in der Sperrliste soll sofort
// wirken: `entsperreSongtext` vergisst deshalb alle Merker (der gesperrte
// Eintrag kann der Grund fuer „nichts Passendes" gewesen sein).
// „Alternatives…" sucht unabhaengig vom Merker.

/// Wie lange ein Merker gilt (Tage): „nicht gefunden" kurz, „instrumental"
/// laenger — das ist eine Aussage des Dienstes, kein Fehlen.
inline constexpr int kSongtextMerkerTageNichtGefunden = 7;
inline constexpr int kSongtextMerkerTageInstrumental = 30;

/// Gilt fuer `track` ein Merker (`notfound.json` im Benutzerordner der
/// Songtexte)? Schluessel: Interpret, Titel, Album und Dauer in Sekunden —
/// dieselben Angaben, die die Anfrage traegt.
[[nodiscard]] bool songtextFehlanzeige(const TrackInfo& track,
                                       const QString& benutzerOrdner = songtextOrdner(),
                                       const QDate& heute = QDate::currentDate());

/// Merker fuer `track` setzen (ersetzt einen frueheren); abgelaufene Eintraege
/// fallen dabei aus der Datei.
void merkeSongtextFehlanzeige(const TrackInfo& track, bool instrumental,
                              const QString& benutzerOrdner = songtextOrdner(),
                              const QDate& heute = QDate::currentDate());

/// Alle Merker vergessen (Settings → Audio); liefert, wie viele noch galten.
int vergissSongtextFehlanzeigen(const QString& benutzerOrdner = songtextOrdner(),
                                const QDate& heute = QDate::currentDate());

/// Der laufende Titel, wie ihn der Player zuletzt gemeldet hat — fuer
/// Werkzeuge der Oberflaeche (Alternativen, Nachsynchronisieren). Thread-sicher.
void setzeLaufendenTitel(const TrackInfo& track);
[[nodiscard]] TrackInfo laufenderTitel();

/**
 * Wartezeit in ms vor der Wiederholung Nr. `versuch` (1 = erste) nach 429/503;
 * -1 = aufgeben. Die Abstaende wachsen (2, 5, 10, 20, 40 s); ein laengeres
 * `Retry-After` (`retryAfterSek`, <= 0 = nicht angegeben) geht vor, eines ueber
 * zwei Minuten beendet die Suche.
 */
[[nodiscard]] int songtextWartezeitMs(int versuch, int retryAfterSek);

/// Stand des Online-Abrufs fuer den laufenden Titel (das Modell traegt ihn,
/// die Oberflaeche zeigt ihn). `Gefunden` kommt unmittelbar nach `gefunden`.
using SongtextAbruf = lumi::multieffect::lyrics::Abruf;

/**
 * Holt den Songtext eines Titels. Eine Anfrage zur Zeit; eine neue Suche
 * verwirft die laufende. Antwortet der Dienst mit 429/503, wird fuer denselben
 * Titel von selbst wieder gefragt (`songtextWartezeitMs`). Kennt der Dienst
 * den Titel nicht (404) und traegt der Zusaetze („(Video Version)",
 * „feat. …"), folgt eine Suche ohne sie; uebernommen wird daraus nur ein
 * Eintrag, dessen Dauer passt (`waehleKandidat`). Ein Titel, zu dem der Dienst
 * nichts hatte, wird gemerkt und fuer eine Weile nicht wieder gefragt
 * (`songtextFehlanzeige`, S88). Lebt im GUI-Thread.
 */
class SongtextOnline : public QObject
{
    Q_OBJECT

public:
    explicit SongtextOnline(QObject* parent = nullptr);

    /// Sucht den Text zu `track` (wenn die Einstellung es erlaubt und Titel
    /// und Interpret bekannt sind). Rueckmeldung ueber `gefunden` und `zustand`.
    /// Ein leerer `track` bricht nur ab.
    void suche(const TrackInfo& track);

    /// Wertet eine Antwort des Dienstes aus (`retryAfterSek` < 0 = Kopfzeile
    /// fehlt). Von der Netzantwort gerufen; oeffentlich, damit Tests den Weg
    /// ohne Netz gehen koennen.
    void antwort(const TrackInfo& track, int status, int retryAfterSek, const QByteArray& body);

    /// Benutzerordner fuer Funde ersetzen (Tests — der echte bleibt unberuehrt).
    void setBenutzerOrdner(const QString& ordner) { m_ordner = ordner; }

Q_SIGNALS:
    /// Ein Text wurde gefunden und gespeichert (`pfad` leer = Speichern
    /// misslang, der Text gilt trotzdem fuer diese Sitzung).
    void gefunden(const QString& songPfad,
                  const lumi::multieffect::lyrics::Songtext& text, const QString& pfad);

    /// Der Stand des Abrufs fuer `songPfad` hat sich geaendert.
    void zustand(const QString& songPfad, lumi::audio::SongtextAbruf stand);

private:
    void sende();
    void fertig(QNetworkReply* reply);

    QNetworkAccessManager* m_net = nullptr;
    QNetworkReply* m_laufend = nullptr;
    QTimer m_timer;        ///< angesetzte Wiederholung bzw. Ende der erbetenen Ruhe
    TrackInfo m_track;     ///< der Titel, um den es gerade geht
    int m_versuch = 0;     ///< Wiederholungen fuer `m_track`
    /// 0 = `/api/get` mit den Tags; 1 = `/api/search` ohne Zusaetze in Titel
    /// und Interpret, nachdem der Dienst den vollen Titel nicht kannte (S87)
    int m_stufe = 0;
    QString m_ordner;      ///< leer = `songtextOrdner()`
    QDateTime m_pauseBis;  ///< `Retry-After` des Dienstes: bis dahin keine Anfrage
};

}  // namespace lumi::audio
