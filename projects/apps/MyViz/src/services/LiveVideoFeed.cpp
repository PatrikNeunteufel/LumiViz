/**
 ****************************************************************************************
 * @file   LiveVideoFeed.cpp
 * @brief  Implementation der Live-Videoquellen (Datei-Streaming + Kamera)
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "services/LiveVideoFeed.hpp"

#include "services/VideoFrameUtil.hpp"  // toImage-Ersatz (Befund S70)

#include <QCamera>
#include <QCameraDevice>
#include <QCoreApplication>
#include <QDir>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QStandardPaths>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoSink>

#include <cmath>
#include <cstdio>

namespace lumi::services {

/// Ein laufender Feed. Die Qt-Multimedia-Objekte leben auf dem Main-Thread
/// (nur baue*/stopp fassen sie an); `bild`/`nummer` stehen unter m_mutex.
struct LiveVideoFeed::Feed
{
    QString quelle;  ///< "datei:<pfad>|<loop>" bzw. "kamera:<id>" (Wechsel-Erkennung)
    double tempo = 1.0;  ///< zuletzt gesetzte Abspielrate (nur Datei)
    std::unique_ptr<QVideoSink> senke;
    std::unique_ptr<QMediaPlayer> spieler;
    std::unique_ptr<QCamera> kamera;
    std::unique_ptr<QMediaCaptureSession> session;
    QImage bild;                ///< letztes Bild (RGBX8888), unter m_mutex
    std::uint64_t nummer = 0;   ///< laufende Bild-Nummer, unter m_mutex
};

LiveVideoFeed& LiveVideoFeed::instance()
{
    static LiveVideoFeed* s_instanz = [] {
        auto* d = new LiveVideoFeed();
        // starteDatei/-Kamera/stopp duerfen vom Render-Thread kommen — das Objekt (und
        // damit die Queued-Invokes) gehoert auf den Main-Thread mit Event-Loop.
        if (auto* app = QCoreApplication::instance())
        {
            d->moveToThread(app->thread());
            // Shutdown-Haken (S70-Befund): beim App-Ende alle Feeds SYNCHRON
            // stoppen — die Queued-Stopps laufen dann nicht mehr, und eine
            // offene QCamera haelt den Prozess sonst am Leben (MF-Threads).
            QObject::connect(app, &QCoreApplication::aboutToQuit, d,
                             [d] { d->alleStoppen(); });
        }
        return d;
    }();
    return *s_instanz;
}

void LiveVideoFeed::alleStoppen()
{
    std::unordered_map<std::uint64_t, std::shared_ptr<Feed>> feeds;
    std::vector<std::shared_ptr<Feed>> tote;
    {
        QMutexLocker sperre(&m_mutex);
        feeds.swap(m_feeds);
        tote.swap(m_friedhof);  // auch nie zugestellte stopp()-Reste (S70 #2)
    }
    for (auto& [id, feed] : feeds) tote.push_back(feed);
    for (auto& feed : tote)
    {
        if (feed->kamera != nullptr) feed->kamera->stop();
        if (feed->spieler != nullptr) feed->spieler->stop();
        // Objekte hier (Main-Thread) sterben lassen — synchron, kein Invoke.
        feed->session.reset();
        feed->kamera.reset();
        feed->spieler.reset();
        feed->senke.reset();
    }
}

QString LiveVideoFeed::testaufnahmenOrdner()
{
    const QString basis =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString ordner = basis + QStringLiteral("/testaufnahmen");
    QDir().mkpath(ordner);
    return ordner;
}

void LiveVideoFeed::starteDatei(std::uint64_t nodeId, const QString& pfad,
                                bool loop, double tempo)
{
    const QString quelle =
        QStringLiteral("datei:%1|%2").arg(pfad).arg(loop ? 1 : 0);
    {
        QMutexLocker sperre(&m_mutex);
        auto it = m_feeds.find(nodeId);
        if (it != m_feeds.end() && it->second->quelle == quelle)
        {
            // Nur das Tempo hat sich geaendert — Rate nachziehen (queued).
            if (std::fabs(it->second->tempo - tempo) > 1e-6)
            {
                it->second->tempo = tempo;
                auto feed = it->second;
                QMetaObject::invokeMethod(
                    this,
                    [feed, tempo] {
                        if (feed->spieler != nullptr)
                            feed->spieler->setPlaybackRate(tempo);
                    },
                    Qt::QueuedConnection);
            }
            return;
        }
    }
    QMetaObject::invokeMethod(
        this, [this, nodeId, pfad, loop, tempo] { baueDatei(nodeId, pfad, loop, tempo); },
        Qt::QueuedConnection);
}

void LiveVideoFeed::starteKamera(std::uint64_t nodeId, const QString& geraeteId)
{
    if (!kameraErlaubt()) return;  // Kamera-Vertrag: nie ohne Freigabe
    const QString quelle = QStringLiteral("kamera:%1").arg(geraeteId);
    {
        QMutexLocker sperre(&m_mutex);
        auto it = m_feeds.find(nodeId);
        if (it != m_feeds.end() && it->second->quelle == quelle) return;
    }
    QMetaObject::invokeMethod(
        this, [this, nodeId, geraeteId] { baueKamera(nodeId, geraeteId); },
        Qt::QueuedConnection);
}

void LiveVideoFeed::stopp(std::uint64_t nodeId)
{
    // Feed unter Mutex ausklinken; das EIGENTUM bleibt beim Dienst
    // (m_friedhof), NIE im Queued-Lambda: wird das Event beim Shutdown nur
    // noch entsorgt statt zugestellt, wuerde der letzte shared_ptr die
    // QCamera sonst MITTEN im ~QApplication zerstoeren — Haupt-Thread haengt
    // in Application::shutdown (S70-Befund #2, Stack-Beweis Patrik).
    {
        QMutexLocker sperre(&m_mutex);
        auto it = m_feeds.find(nodeId);
        if (it == m_feeds.end()) return;
        m_friedhof.push_back(it->second);
        m_feeds.erase(it);
    }
    QMetaObject::invokeMethod(
        this, [this] { friedhofLeeren(); }, Qt::QueuedConnection);
}

void LiveVideoFeed::friedhofLeeren()
{
    std::vector<std::shared_ptr<Feed>> tote;
    {
        QMutexLocker sperre(&m_mutex);
        tote.swap(m_friedhof);
    }
    for (auto& feed : tote)
    {
        if (feed->kamera != nullptr) feed->kamera->stop();
        if (feed->spieler != nullptr) feed->spieler->stop();
        // Qt-Objekte HIER explizit zerstoeren (nicht erst beim letzten
        // shared_ptr): haelt z. B. ein Tempo-Lambda in der Event-Queue noch
        // eine Referenz, bleibt dort nur eine leere Feed-Huelle zurueck.
        feed->session.reset();
        feed->kamera.reset();
        feed->spieler.reset();
        feed->senke.reset();
    }
}

QImage LiveVideoFeed::letztesBild(std::uint64_t nodeId, std::uint64_t* nummer)
{
    QMutexLocker sperre(&m_mutex);
    auto it = m_feeds.find(nodeId);
    if (it == m_feeds.end())
    {
        if (nummer != nullptr) *nummer = 0;
        return {};
    }
    if (nummer != nullptr) *nummer = it->second->nummer;
    return it->second->bild;  // QImage ist COW — flache Kopie reicht
}

void LiveVideoFeed::verbindeSenke(QVideoSink* senke, QMutex* mutex,
                                  const std::shared_ptr<Feed>& feed)
{
    std::weak_ptr<Feed> schwach = feed;
    QObject::connect(senke, &QVideoSink::videoFrameChanged, senke,
                     [schwach, mutex](const QVideoFrame& frame) {
                         auto feed = schwach.lock();
                         if (feed == nullptr || !frame.isValid()) return;
                         // S70: map()+Rohbytes statt toImage() — liefert
                         // unter Qt 6.10.1 schwarz (VideoFrameUtil.hpp)
                         QImage img = videoFrameZuBild(frame);
                         if (img.isNull()) return;
                         QMutexLocker sperre(mutex);
                         feed->bild = std::move(img);
                         ++feed->nummer;
                     });
}

void LiveVideoFeed::baueDatei(std::uint64_t nodeId, const QString& pfad,
                              bool loop, double tempo)
{
    auto feed = std::make_shared<Feed>();
    feed->quelle = QStringLiteral("datei:%1|%2").arg(pfad).arg(loop ? 1 : 0);
    feed->tempo = tempo;
    feed->senke = std::make_unique<QVideoSink>();
    feed->spieler = std::make_unique<QMediaPlayer>();
    feed->spieler->setVideoSink(feed->senke.get());
    feed->spieler->setLoops(loop ? QMediaPlayer::Infinite : 1);
    feed->spieler->setSource(QUrl::fromLocalFile(pfad));
    feed->spieler->setPlaybackRate(tempo);
    verbindeSenke(feed->senke.get(), &m_mutex, feed);
    feed->spieler->play();

    QMutexLocker sperre(&m_mutex);
    m_feeds[nodeId] = std::move(feed);  // ersetzt einen Alt-Feed (stirbt hier)
}

void LiveVideoFeed::baueKamera(std::uint64_t nodeId, const QString& geraeteId)
{
    // Geraet aufloesen: ID zuerst, Beschreibung als Ausweich (Geraete-IDs
    // koennen sich je Treiber-Update aendern).
    QCameraDevice geraet;
    const auto geraete = QMediaDevices::videoInputs();
    for (const QCameraDevice& d : geraete)
    {
        if (QString::fromUtf8(d.id()) == geraeteId || d.description() == geraeteId)
        {
            geraet = d;
            break;
        }
    }
    auto feed = std::make_shared<Feed>();
    feed->quelle = QStringLiteral("kamera:%1").arg(geraeteId);
    if (geraet.isNull())
    {
        // Kein Geraet — Feed trotzdem eintragen (Wechsel-Erkennung verhindert
        // Retry-Spam); Bild bleibt leer, der Knoten zeichnet nichts.
        std::fprintf(stderr, "[LiveVideoFeed] Kamera nicht gefunden: %s\n",
                     qPrintable(geraeteId));
        QMutexLocker sperre(&m_mutex);
        m_feeds[nodeId] = std::move(feed);
        return;
    }
    feed->senke = std::make_unique<QVideoSink>();
    feed->kamera = std::make_unique<QCamera>(geraet);
    feed->session = std::make_unique<QMediaCaptureSession>();
    feed->session->setCamera(feed->kamera.get());
    feed->session->setVideoSink(feed->senke.get());
    verbindeSenke(feed->senke.get(), &m_mutex, feed);
    feed->kamera->start();

    QMutexLocker sperre(&m_mutex);
    m_feeds[nodeId] = std::move(feed);
}

} // namespace lumi::services
