/**
 ****************************************************************************************
 * @file   VideoFrameCache.cpp
 * @brief  Implementation des frame-genauen Video-Decoder-Caches (Qt Multimedia)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026 (fps-Feld August 2026, S70)
 * @version 1.1.0
 ****************************************************************************************
 */

#include "services/VideoFrameCache.hpp"

#include "services/VideoFrameUtil.hpp"  // toImage-Ersatz (Befund S70)

#include <QCoreApplication>
#include <QEventLoop>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QTimer>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoSink>

#include <algorithm>
#include <cstdio>

namespace lumi::services {

namespace {

/// Auf ein Signal warten, hoechstens `ms` Millisekunden; false = Timeout.
/// Verschachtelte Event-Loop auf dem Main-Thread — genau dafuer ist der
/// Ladevorgang per Queued-Invoke DORTHIN gelegt (QMediaPlayer braucht sie).
template <typename Sender, typename Signal>
bool warteAuf(Sender* sender, Signal signal, int ms)
{
    QEventLoop loop;
    bool gefeuert = false;
    QObject::connect(sender, signal, &loop, [&loop, &gefeuert] {
        gefeuert = true;
        loop.quit();
    });
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
    return gefeuert;
}

constexpr int kMaxFrames = 4096;
constexpr qint64 kMaxBytes = 256ll * 1024 * 1024;  ///< dekodierte Frames gesamt

} // namespace

VideoFrameCache& VideoFrameCache::instance()
{
    static VideoFrameCache* s_instanz = [] {
        auto* c = new VideoFrameCache();
        // hole() darf vom Render-Thread kommen — das Objekt (und damit die
        // Queued-Invokes) gehoert auf den Main-Thread mit Event-Loop.
        if (QCoreApplication::instance() != nullptr)
            c->moveToThread(QCoreApplication::instance()->thread());
        return c;
    }();
    return *s_instanz;
}

std::shared_ptr<VideoFrameCache::Clip> VideoFrameCache::hole(const QString& pfad)
{
    QMutexLocker sperre(&m_mutex);
    auto it = m_clips.find(pfad);
    if (it != m_clips.end()) return it.value();

    auto clip = std::make_shared<Clip>();
    m_clips.insert(pfad, clip);
    // Laden auf dem Main-Thread anstossen (dort laeuft die Event-Loop)
    QMetaObject::invokeMethod(
        this, [this, pfad, clip] { lade(pfad, clip); }, Qt::QueuedConnection);
    return clip;
}

void VideoFrameCache::lade(const QString& pfad, std::shared_ptr<Clip> clip)
{
    QMediaPlayer spieler;
    QVideoSink senke;
    spieler.setVideoSink(&senke);
    spieler.setSource(QUrl::fromLocalFile(pfad));

    // Auf die Medien-Analyse warten (FFmpeg oeffnet + parst asynchron)
    const auto geladen = [&spieler] {
        const auto st = spieler.mediaStatus();
        return st == QMediaPlayer::LoadedMedia || st == QMediaPlayer::BufferedMedia ||
               st == QMediaPlayer::InvalidMedia;
    };
    for (int versuch = 0; versuch < 100 && !geladen(); ++versuch)
    {
        if (!warteAuf(&spieler, &QMediaPlayer::mediaStatusChanged, 100)) break;
    }
    if (spieler.mediaStatus() == QMediaPlayer::InvalidMedia ||
        spieler.duration() <= 0)
    {
        std::fprintf(stderr, "[VideoFrameCache] nicht dekodierbar: %s\n",
                     qPrintable(pfad));
        clip->status.store(FEHLGESCHLAGEN);
        return;
    }

    // Frame-Zahl aus Bildrate x Dauer (Frame-Schritt: wir holen jeden Frame
    // per Seek — deterministisch, unabhaengig von der Abspielgeschwindigkeit)
    double fps = spieler.metaData().value(QMediaMetaData::VideoFrameRate).toDouble();
    if (fps <= 0.0 || fps > 240.0) fps = 25.0;
    const qint64 dauerMs = spieler.duration();
    const int n = std::clamp(
        static_cast<int>(dauerMs * fps / 1000.0 + 0.5), 1, kMaxFrames);

    spieler.pause();  // Decoder scharf stellen, Position steuerbar
    qint64 bytes = 0;
    for (int i = 0; i < n; ++i)
    {
        // Frame-Mitte anfahren, damit Rundung nicht auf die Frame-Grenze faellt
        const qint64 ziel = static_cast<qint64>((i + 0.5) * 1000.0 / fps);
        spieler.setPosition(std::min(ziel, dauerMs - 1));
        if (!warteAuf(&senke, &QVideoSink::videoFrameChanged, 3000)) break;
        const QVideoFrame frame = senke.videoFrame();
        if (!frame.isValid()) break;
        // S70: map()+Rohbytes statt toImage() — das liefert unter Qt 6.10.1
        // schwarze Bilder (Befund-Doku: VideoFrameUtil.hpp).
        QImage img = videoFrameZuBild(frame);
        if (img.isNull()) break;
        bytes += static_cast<qint64>(img.width()) * img.height() * 4;
        if (bytes > kMaxBytes)
        {
            std::fprintf(stderr,
                         "[VideoFrameCache] %s: Frame-Cache-Limit erreicht "
                         "(%d/%d Frames)\n",
                         qPrintable(pfad), i, n);
            break;
        }
        clip->frames.push_back(std::move(img));
    }

    if (clip->frames.empty())
    {
        std::fprintf(stderr, "[VideoFrameCache] keine Frames aus: %s\n",
                     qPrintable(pfad));
        clip->status.store(FEHLGESCHLAGEN);
        return;
    }
    std::printf("[VideoFrameCache] %s: %zu Frames @ %.3f fps dekodiert\n",
                qPrintable(pfad), clip->frames.size(), fps);
    clip->fps = fps;  // vor dem Umschalten schreiben (Leser-Vertrag s. Clip)
    clip->status.store(FERTIG);
}

} // namespace lumi::services
