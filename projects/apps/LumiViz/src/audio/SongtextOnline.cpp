/**
 ****************************************************************************************
 * @file   SongtextOnline.cpp
 * @brief  Songtext aus dem Netz holen (LRCLIB) — nur auf ausdrueckliche Einstellung
 ****************************************************************************************
 */

#include "audio/SongtextOnline.hpp"

#include "audio/SongtextQuelle.hpp"

#include <BasicLogger.h>

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrlQuery>

#include <algorithm>
#include <iterator>
#include <mutex>
#include <utility>

namespace lumi::audio
{

using lumi::multieffect::lyrics::Songtext;

bool songtextOnlineErlaubt()
{
    return QSettings().value(QStringLiteral("lyrics/onlineSearch"), false).toBool();
}

SongtextAblage songtextAblage()
{
    return QSettings().value(QStringLiteral("lyrics/saveLocation"), 0).toInt() == 1
               ? SongtextAblage::NebenDemSong
               : SongtextAblage::Benutzerordner;
}

QUrl lrclibUrl(const TrackInfo& track)
{
    const QString titel = track.title.trimmed();
    const QString interpret = track.artist.trimmed();
    if (titel.isEmpty() || interpret.isEmpty()) return QUrl();  // beide sind Pflicht
    // QUrlQuery kodiert `&` und `=` selbst; `%` und `+` muessen vorab kodiert
    // werden (ein `+` gaelte beim Server als Leerzeichen).
    const auto wert = [](QString v) {
        v.replace(QLatin1Char('%'), QStringLiteral("%25"));
        v.replace(QLatin1Char('+'), QStringLiteral("%2B"));
        return v;
    };
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("track_name"), wert(titel));
    q.addQueryItem(QStringLiteral("artist_name"), wert(interpret));
    if (!track.album.trimmed().isEmpty())
        q.addQueryItem(QStringLiteral("album_name"), wert(track.album.trimmed()));
    // Dauer in Sekunden, erlaubt 1..3600 — ausserhalb weglassen statt abgewiesen werden
    const int sekunden = (track.durationMs + 500) / 1000;
    if (sekunden >= 1 && sekunden <= 3600)
        q.addQueryItem(QStringLiteral("duration"), QString::number(sekunden));
    QUrl url(QStringLiteral("https://lrclib.net/api/get"));
    url.setQuery(q);
    return url;
}

Songtext songtextAusLrclib(const QByteArray& json)
{
    const QJsonObject o = QJsonDocument::fromJson(json).object();
    if (o.isEmpty() || o.value(QStringLiteral("instrumental")).toBool()) return {};
    // synchron vor unsynchron — dieselbe Regel wie bei den lokalen Quellen
    for (const char* feld : {"syncedLyrics", "plainLyrics"})
    {
        const QString text = o.value(QLatin1String(feld)).toString();
        if (text.trimmed().isEmpty()) continue;
        Songtext s = lumi::multieffect::lyrics::leseLrc(text.toStdString());
        if (s.leer()) continue;
        s.quelle = "LRCLIB";
        // Herkunft fuer die Datei: Dienst und Kennung des Eintrags; das Datum
        // haengt der Abruf an (fertig)
        const qint64 id = static_cast<qint64>(o.value(QStringLiteral("id")).toDouble());
        s.herkunft = id > 0 ? "LRCLIB id " + std::to_string(id) : std::string("LRCLIB");
        return s;
    }
    return {};
}

QString songtextZiel(const TrackInfo& track, bool synchron, SongtextAblage ablage,
                     const QString& benutzerOrdner, bool* ausgewichen)
{
    if (ausgewichen != nullptr) *ausgewichen = false;
    const QString endung = synchron ? QStringLiteral(".lrc") : QStringLiteral(".txt");
    if (ablage == SongtextAblage::NebenDemSong && !track.filePath.isEmpty())
    {
        const QFileInfo song(track.filePath);
        if (QFileInfo(song.absolutePath()).isWritable())
            return song.dir().absoluteFilePath(song.completeBaseName() + endung);
        if (ausgewichen != nullptr) *ausgewichen = true;
    }
    // S87: ist der gemeinsame Name von einer anderen Fassung belegt, bekommt
    // diese ihren eigenen (`… [m-ss]`)
    const QString basis = songtextBenutzerBasis(track, benutzerOrdner);
    return basis.isEmpty() ? QString() : basis + endung;
}

QUrl lrclibSuchUrl(const QString& titel, const QString& interpret)
{
    if (titel.trimmed().isEmpty()) return QUrl();
    const auto wert = [](QString v) {  // wie lrclibUrl: % und + vorab kodieren
        v.replace(QLatin1Char('%'), QStringLiteral("%25"));
        v.replace(QLatin1Char('+'), QStringLiteral("%2B"));
        return v;
    };
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("track_name"), wert(titel.trimmed()));
    if (!interpret.trimmed().isEmpty())
        q.addQueryItem(QStringLiteral("artist_name"), wert(interpret.trimmed()));
    QUrl url(QStringLiteral("https://lrclib.net/api/search"));
    url.setQuery(q);
    return url;
}

QString songtextTitelOhneZusatz(const QString& titel)
{
    static const QRegularExpression klammern(QStringLiteral("\\s*[\\(\\[][^\\)\\]]*[\\)\\]]"));
    QString kurz = titel;
    kurz.remove(klammern);
    kurz = kurz.simplified();
    return kurz.isEmpty() ? titel.simplified() : kurz;
}

QString songtextInterpretOhneZusatz(const QString& interpret)
{
    static const QRegularExpression gast(
        // kein Komma: „Earth, Wind & Fire" ist EIN Interpret
        QStringLiteral("\\s*(?:;|\\s(?:feat\\.?|ft\\.?|featuring)\\s).*$"),
        QRegularExpression::CaseInsensitiveOption);
    QString kurz = interpret;
    kurz.remove(gast);
    kurz = kurz.simplified();
    return kurz.isEmpty() ? interpret.simplified() : kurz;
}

int waehleKandidat(const std::vector<SongtextKandidat>& kandidaten, const TrackInfo& track,
                   const QString& benutzerOrdner)
{
    const int soll = (track.durationMs + 500) / 1000;
    if (soll <= 0) return -1;  // ohne Dauer laesst sich keine Fassung verantworten
    int bester = -1;
    for (int i = 0; i < static_cast<int>(kandidaten.size()); ++i)
    {
        const SongtextKandidat& k = kandidaten[static_cast<std::size_t>(i)];
        if (k.text.leer() || std::abs(k.dauerSek - soll) > 2) continue;
        if (songtextGesperrt(k.id, benutzerOrdner)) continue;
        if (bester < 0 ||
            (k.text.synchron && !kandidaten[static_cast<std::size_t>(bester)].text.synchron))
            bester = i;
    }
    return bester;
}

std::vector<SongtextKandidat> kandidatenAusLrclib(const QByteArray& json)
{
    std::vector<SongtextKandidat> out;
    const QJsonArray liste = QJsonDocument::fromJson(json).array();
    out.reserve(static_cast<std::size_t>(liste.size()));
    for (const QJsonValue& v : liste)
    {
        const QJsonObject o = v.toObject();
        SongtextKandidat k;
        k.id = static_cast<qint64>(o.value(QStringLiteral("id")).toDouble());
        if (k.id <= 0) continue;
        k.titel = o.value(QStringLiteral("trackName")).toString();
        k.interpret = o.value(QStringLiteral("artistName")).toString();
        k.album = o.value(QStringLiteral("albumName")).toString();
        k.dauerSek = static_cast<int>(o.value(QStringLiteral("duration")).toDouble() + 0.5);
        k.instrumental = o.value(QStringLiteral("instrumental")).toBool();
        k.text = songtextAusLrclib(QJsonDocument(o).toJson(QJsonDocument::Compact));
        out.push_back(std::move(k));
    }
    return out;
}

qint64 lrclibKennung(const std::string& herkunft)
{
    static const QRegularExpression re(QStringLiteral("^LRCLIB id (\\d+)"));
    const auto m = re.match(QString::fromStdString(herkunft));
    return m.hasMatch() ? m.captured(1).toLongLong() : 0;
}

namespace
{
/// Traegt die Datei einen Herkunftsvermerk von LumiViz? (nur dann darf sie weg)
bool vonLumiVizGeholt(const QString& pfad)
{
    QFile f(pfad);
    if (!f.open(QIODevice::ReadOnly)) return false;
    return !lumi::multieffect::lyrics::leseLrc(f.readAll().toStdString()).herkunft.empty();
}

QString sperrDatei(const QString& benutzerOrdner)
{
    return benutzerOrdner + QStringLiteral("/blocked.json");
}

void schreibeSperrliste(const std::vector<SongtextSperre>& liste, const QString& benutzerOrdner)
{
    QJsonArray a;
    for (const SongtextSperre& s : liste)
        a.append(QJsonObject{{QStringLiteral("id"), static_cast<double>(s.id)},
                             {QStringLiteral("title"), s.titel}});
    QDir().mkpath(benutzerOrdner);
    QFile f(sperrDatei(benutzerOrdner));
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        f.write(QJsonDocument(a).toJson(QJsonDocument::Indented));
}
}  // namespace

Songtext uebernehmeSongtext(const TrackInfo& track, Songtext text, const QString& benutzerOrdner)
{
    text.herkunft += ", fetched " + QDate::currentDate().toString(Qt::ISODate).toStdString();
    // fuer WELCHE Fassung der Text gilt: die Dauer dieses Titels (`[length:]`)
    if (track.durationMs > 0) text.laengeMs = track.durationMs;
    bool ausgewichen = false;
    const SongtextAblage ablage = songtextAblage();
    const QString ziel = songtextZiel(track, text.synchron, ablage, benutzerOrdner, &ausgewichen);
    QString gespeichert;
    if (!ziel.isEmpty() && speichereSongtext(text, ziel)) gespeichert = ziel;
    if (ausgewichen)
        BasicLogger::logInfo("Lyrics: folder of the song is not writable — saved to the user "
                             "folder instead");
    // die fruehere Fassung mit der anderen Endung: weg, wenn LumiViz sie geholt hat
    if (!gespeichert.isEmpty())
    {
        const QString andere = songtextZiel(track, !text.synchron, ablage, benutzerOrdner);
        if (!andere.isEmpty() && andere != gespeichert && vonLumiVizGeholt(andere))
            QFile::remove(andere);
    }
    text.quelle = gespeichert.isEmpty()
                      ? "LRCLIB (not saved)"
                      : "LRCLIB, saved: " + QFileInfo(gespeichert).fileName().toStdString();
    text.datei = gespeichert.toStdString();
    return text;
}

std::vector<SongtextSperre> songtextSperrliste(const QString& benutzerOrdner)
{
    std::vector<SongtextSperre> out;
    QFile f(sperrDatei(benutzerOrdner));
    if (!f.open(QIODevice::ReadOnly)) return out;
    for (const QJsonValue& v : QJsonDocument::fromJson(f.readAll()).array())
    {
        const QJsonObject o = v.toObject();
        const auto id = static_cast<qint64>(o.value(QStringLiteral("id")).toDouble());
        if (id > 0) out.push_back({id, o.value(QStringLiteral("title")).toString()});
    }
    return out;
}

bool songtextGesperrt(qint64 id, const QString& benutzerOrdner)
{
    if (id <= 0) return false;
    const auto liste = songtextSperrliste(benutzerOrdner);
    return std::any_of(liste.begin(), liste.end(),
                       [id](const SongtextSperre& s) { return s.id == id; });
}

void sperreSongtext(qint64 id, const QString& titel, const QString& benutzerOrdner)
{
    if (id <= 0 || songtextGesperrt(id, benutzerOrdner)) return;
    auto liste = songtextSperrliste(benutzerOrdner);
    liste.push_back({id, titel});
    schreibeSperrliste(liste, benutzerOrdner);
}

void entsperreSongtext(qint64 id, const QString& benutzerOrdner)
{
    auto liste = songtextSperrliste(benutzerOrdner);
    const auto neuEnde = std::remove_if(liste.begin(), liste.end(),
                                        [id](const SongtextSperre& s) { return s.id == id; });
    if (neuEnde == liste.end()) return;
    liste.erase(neuEnde, liste.end());
    schreibeSperrliste(liste, benutzerOrdner);
}

bool verwerfeSongtext(const TrackInfo& track, const std::string& herkunft,
                      const std::string& datei, const QString& benutzerOrdner)
{
    if (herkunft.empty()) return false;  // selbst angelegt oder eingebettet: nie loeschen
    const QString pfad = QString::fromStdString(datei);
    if (!pfad.isEmpty() && vonLumiVizGeholt(pfad)) QFile::remove(pfad);
    sperreSongtext(lrclibKennung(herkunft), songtextName(track), benutzerOrdner);
    return true;
}

namespace
{
std::mutex g_titelMutex;
TrackInfo g_laufenderTitel;
}  // namespace

void setzeLaufendenTitel(const TrackInfo& track)
{
    const std::lock_guard<std::mutex> lock(g_titelMutex);
    g_laufenderTitel = track;
}

TrackInfo laufenderTitel()
{
    const std::lock_guard<std::mutex> lock(g_titelMutex);
    return g_laufenderTitel;
}

int songtextWartezeitMs(int versuch, int retryAfterSek)
{
    // Lauf Patrik 2026-10-01: 19 von 62 Anfragen kamen mit 503 und
    // `Retry-After: 1` zurueck; dieselbe Anfrage gelang meist 8–17 s spaeter,
    // bei einzelnen Titeln auch nach 80 s nicht. Deshalb wachsende Abstaende
    // und ein Ende.
    static constexpr int kStufenMs[] = {2000, 5000, 10000, 20000, 40000};
    static constexpr int kStufen = static_cast<int>(std::size(kStufenMs));
    if (versuch < 1 || versuch > kStufen) return -1;
    if (retryAfterSek > 120) return -1;  // so lange wartet niemand auf eine Zeile
    return std::max(kStufenMs[versuch - 1], std::max(0, retryAfterSek) * 1000);
}

namespace
{
constexpr int kAtempauseMs = 400;  ///< Abstand zwischen Titelwechsel und Anfrage
}

SongtextOnline::SongtextOnline(QObject* parent) : QObject(parent)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &SongtextOnline::sende);
}

void SongtextOnline::suche(const TrackInfo& track)
{
    // eine Anfrage zur Zeit; der neue Titel zaehlt. Erst austragen, dann
    // abbrechen: abort() meldet `finished` sofort, und `fertig` soll die
    // abgebrochene Antwort als ueberholt erkennen, nicht als Fehler.
    m_timer.stop();
    if (QNetworkReply* alt = std::exchange(m_laufend, nullptr)) alt->abort();
    m_track = track;
    m_versuch = 0;
    m_stufe = 0;
    Q_EMIT zustand(track.filePath, SongtextAbruf::Ruhe);

    if (track.filePath.isEmpty() || !songtextOnlineErlaubt()) return;
    if (lrclibUrl(track).isEmpty()) return;  // Titel oder Interpret fehlt

    // der Dienst hat um Ruhe gebeten: nicht verwerfen, sondern danach fragen
    const qint64 restMs = m_pauseBis.isValid()
                              ? QDateTime::currentDateTimeUtc().msecsTo(m_pauseBis)
                              : 0;
    if (restMs > kAtempauseMs)
    {
        Q_EMIT zustand(track.filePath, SongtextAbruf::Wartet);
        m_timer.start(static_cast<int>(restMs));
        return;
    }
    // Kurze Atempause vor der Anfrage (lrclib.net/docs, „Request Throttling":
    // 200–500 ms zwischen Anfragen): wer durch die Liste springt, loest so
    // nicht je Titel eine Anfrage aus — der naechste Wechsel haelt die Uhr an.
    Q_EMIT zustand(track.filePath, SongtextAbruf::Laedt);
    m_timer.start(kAtempauseMs);
}

void SongtextOnline::sende()
{
    const QUrl url = m_stufe == 1
                         ? lrclibSuchUrl(songtextTitelOhneZusatz(m_track.title),
                                         songtextInterpretOhneZusatz(m_track.artist))
                         : lrclibUrl(m_track);
    if (url.isEmpty() || !songtextOnlineErlaubt()) return;  // inzwischen abgeschaltet

    if (m_net == nullptr) m_net = new QNetworkAccessManager(this);
    QNetworkRequest req(url);
    // verlangt: Programmname, Version und Projektseite (lrclib.net/docs)
    req.setRawHeader("User-Agent",
                     QStringLiteral("LumiViz v%1 (https://github.com/PatrikNeunteufel/LumiViz)")
                         .arg(QCoreApplication::applicationVersion())
                         .toUtf8());
    req.setTransferTimeout(15000);
    QNetworkReply* reply = m_net->get(req);
    m_laufend = reply;
    Q_EMIT zustand(m_track.filePath, SongtextAbruf::Laedt);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { fertig(reply); });
}

void SongtextOnline::fertig(QNetworkReply* reply)
{
    reply->deleteLater();
    if (reply != m_laufend) return;  // ueberholt von einer neueren Suche
    m_laufend = nullptr;
    antwort(m_track, reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
            reply->hasRawHeader("Retry-After") ? reply->rawHeader("Retry-After").toInt() : -1,
            reply->readAll());
}

void SongtextOnline::antwort(const TrackInfo& track, int status, int retryAfterSek,
                             const QByteArray& body)
{
    m_timer.stop();
    m_track = track;
    const std::string name = track.title.toStdString();
    if (status == 429 || status == 503)
    {
        // „gleich noch einmal": Retry-After gilt fuer jede Anfrage, auch die
        // zum naechsten Titel; fuer diesen Titel wird von selbst nachgefragt
        if (retryAfterSek > 0)
            m_pauseBis = QDateTime::currentDateTimeUtc().addSecs(retryAfterSek);
        ++m_versuch;
        const int warteMs = songtextWartezeitMs(m_versuch, retryAfterSek);
        if (warteMs < 0)
        {
            BasicLogger::logInfo("Lyrics: LRCLIB stays busy (HTTP " + std::to_string(status) +
                                 "), giving up — " + name);
            Q_EMIT zustand(track.filePath, SongtextAbruf::Fehlgeschlagen);
            return;
        }
        BasicLogger::logInfo("Lyrics: LRCLIB busy (HTTP " + std::to_string(status) +
                             "), retry " + std::to_string(m_versuch) + " in " +
                             std::to_string(warteMs / 1000) + " s — " + name);
        Q_EMIT zustand(track.filePath, SongtextAbruf::Wartet);
        m_timer.start(warteMs);
        return;
    }
    const QString ordner = m_ordner.isEmpty() ? songtextOrdner() : m_ordner;
    // S87 (Befund Patrik: „Try (Video Version)" von „Schiller feat. Nadia Ali"
    // fand der Dienst nie): zweite Stufe — die Suche ohne Zusaetze in Titel
    // und Interpret. `immer` = auch bei unveraenderten Begriffen (der Eintrag
    // der ersten Stufe ist gesperrt, es kann einen anderen geben).
    const auto zweiteStufe = [&](bool immer) {
        if (m_stufe != 0) return false;
        const bool gekuerzt = songtextTitelOhneZusatz(track.title) != track.title.simplified() ||
                              songtextInterpretOhneZusatz(track.artist) != track.artist.simplified();
        if (!gekuerzt && !immer) return false;
        m_stufe = 1;
        m_versuch = 0;
        BasicLogger::logInfo("Lyrics: searching without additions — " + name);
        Q_EMIT zustand(track.filePath, SongtextAbruf::Laedt);
        m_timer.start(kAtempauseMs);
        return true;
    };
    if (status == 404)  // der Dienst kennt den Titel nicht
    {
        if (zweiteStufe(false)) return;
        BasicLogger::logInfo("Lyrics: not found online — " + name);
        Q_EMIT zustand(track.filePath, SongtextAbruf::NichtGefunden);
        return;
    }
    if (status != 200)  // kein Netz, Zeitueberschreitung, anderer Fehler
    {
        BasicLogger::logInfo("Lyrics: online search failed (HTTP " + std::to_string(status) +
                             ") — " + name);
        Q_EMIT zustand(track.filePath, SongtextAbruf::Fehlgeschlagen);
        return;
    }

    Songtext text;
    if (m_stufe == 1)
    {
        // Suche: nur ein Eintrag, dessen Dauer zum Titel passt — sonst
        // entscheidet der Mensch ueber „Alternatives…"
        const auto kandidaten = kandidatenAusLrclib(body);
        const int wahl = waehleKandidat(kandidaten, track, ordner);
        if (wahl < 0)
        {
            BasicLogger::logInfo("Lyrics: no entry with a matching duration (" +
                                 std::to_string(kandidaten.size()) +
                                 " found — see Alternatives) — " + name);
            Q_EMIT zustand(track.filePath, SongtextAbruf::NichtGefunden);
            return;
        }
        text = kandidaten[static_cast<std::size_t>(wahl)].text;
    }
    else
    {
        text = songtextAusLrclib(body);
        if (text.leer())  // instrumental oder ohne Text eingetragen
        {
            if (zweiteStufe(false)) return;
            BasicLogger::logInfo("Lyrics: entry without text (instrumental?) — " + name);
            Q_EMIT zustand(track.filePath, SongtextAbruf::NichtGefunden);
            return;
        }
        // S87: als falsch verworfen („Wrong lyrics") — nicht wieder holen;
        // die Suche kann eine andere Fassung mit passender Dauer kennen
        if (songtextGesperrt(lrclibKennung(text.herkunft), ordner))
        {
            if (zweiteStufe(true)) return;
            BasicLogger::logInfo("Lyrics: the entry found online is blocked — " + name);
            Q_EMIT zustand(track.filePath, SongtextAbruf::NichtGefunden);
            return;
        }
    }
    text = uebernehmeSongtext(track, std::move(text), ordner);
    BasicLogger::logInfo("Lyrics: fetched from LRCLIB — " + name);
    Q_EMIT gefunden(track.filePath, text, QString::fromStdString(text.datei));
    Q_EMIT zustand(track.filePath, SongtextAbruf::Gefunden);
}

}  // namespace lumi::audio
