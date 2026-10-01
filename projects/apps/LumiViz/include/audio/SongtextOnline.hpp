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
 * **Datenschutz:** eine Anfrage sendet Interpret, Titel, Album und Dauer des
 * laufenden Titels an lrclib.net. Ohne die Einstellung geht nichts hinaus.
 * Die Rechte an den Texten liegen bei Dritten; die Dokumentation des Dienstes
 * sagt dazu nichts — gespeichert wird nur lokal, fuer den eigenen Gebrauch.
 ****************************************************************************************
 */

#pragma once

#include "audio/AudioEvents.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"

#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QString>
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

/**
 * Holt den Songtext eines Titels. Eine Anfrage zur Zeit; eine neue Suche
 * verwirft die laufende. Lebt im GUI-Thread.
 */
class SongtextOnline : public QObject
{
    Q_OBJECT

public:
    explicit SongtextOnline(QObject* parent = nullptr);

    /// Sucht den Text zu `track` (wenn die Einstellung es erlaubt und Titel
    /// und Interpret bekannt sind). Rueckmeldung ueber `gefunden`.
    void suche(const TrackInfo& track);

Q_SIGNALS:
    /// Ein Text wurde gefunden und gespeichert (`pfad` leer = Speichern
    /// misslang, der Text gilt trotzdem fuer diese Sitzung).
    void gefunden(const QString& songPfad,
                  const lumi::multieffect::lyrics::Songtext& text, const QString& pfad);

private:
    void fertig(QNetworkReply* reply, const TrackInfo& track);

    QNetworkAccessManager* m_net = nullptr;
    QNetworkReply* m_laufend = nullptr;
    QDateTime m_pauseBis;  ///< nach 429/503: bis dahin keine Anfrage
};

}  // namespace lumi::audio
