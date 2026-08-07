/**
 ****************************************************************************************
 * @file   LiveVideoFeed.cpp
 * @brief  Implementation der Live-Videoquellen (Datei-Streaming + Kamera)
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.3.0
 ****************************************************************************************
 */

#include "services/LiveVideoFeed.hpp"

#include "services/VideoFrameUtil.hpp"  // toImage-Ersatz (Befund S70)

#include "BasicLogger.h"  // Feed-Diagnose S71 (stderr ist in der GUI unsichtbar)

#include <QElapsedTimer>

#include <QCamera>
#include <QCameraDevice>
#include <QCameraFormat>
#include <QCoreApplication>
#include <QDir>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QSemaphore>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVideoFrame>
#include <QVideoSink>

#include <cmath>
#include <cstdio>

namespace lumi::services {

/// Ein laufender Feed. Alle Qt-Multimedia-Objekte leben auf dem Main-Thread
/// (nur baue*/stopp fassen sie an); `bild`/`nummer` stehen unter m_mutex.
/// LAG-BEFUND S70 (zwei Anlaeufe): (1) Die RGBX-Wandlung je Kamera-Frame auf
/// dem GUI-Thread fror das UI ein (Drag&Drop tot). (2) Wandlung auf dem
/// RENDER-Thread haengt bei Hardware-Frames (map()/toImage() funktioniert
/// nur im Lieferkontext) — der Render-Thread blockierte, das Fenster-
/// Schliessen wartete auf ihn, die Kamera blieb an. LOESUNG: die
/// Frame-Verbindung wird QUEUED auf einen eigenen WANDLER-Thread zugestellt
/// (Empfaenger-Kontext m_medienKontext); der Handler wandelt DORT, gibt den
/// Pipeline-Puffer sofort zurueck und legt fertige QImages unter m_mutex ab
/// — GUI- und Render-Thread fassen nie ein QVideoFrame an.
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
    /// TOTFLAGGE (Befund S71): ab hier fasst der Wandler kein Frame dieses
    /// Feeds mehr an. MUSS gesetzt sein, BEVOR stop()/Zerstoerung laufen —
    /// sonst mappt der Wandler in eine sterbende Pipeline und blockiert im
    /// Treiber (s. ABSCHALT-VERTRAG im Header).
    std::atomic<bool> tot{false};
    /// DIAGNOSE S71: wie viele Frames hat der Wandler fuer diesen Feed
    /// angefangen (`begonnen`) bzw. fertig gewandelt (`fertig`)? Bleiben die
    /// beiden auseinander, steckt der Wandler im map() dieses Feeds.
    std::atomic<std::uint64_t> begonnen{0};
    std::atomic<std::uint64_t> fertig{0};
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
            // MEDIEN-THREAD (Umbau S71, s. EIN-THREAD-BESITZ im Header):
            // traegt die komplette Qt-Multimedia-Seite — Aufbau, Frames und
            // Abbau. Der Kontext ist der Empfaenger aller Auftraege; weil
            // Senken und Kameras DORT entstehen, werden die Frame-Signale
            // direkt zugestellt (Wandlung im Lieferkontext) und der Abbau
            // kann die Frame-Verarbeitung nicht mehr ueberholen.
            //
            // ENTSCHEIDENDER BEFUND S71 (A/B am Pruefstand, echte Kamera):
            // Ein EIGENER Thread, der Kamera-Frames verarbeitet hat,
            // verhindert das Prozessende — 2/2 Haenger MIT Thread gegen 3/3
            // sauberes Ende OHNE. Das gilt auch dann, wenn er vorher sauber
            // beendet wurde, und ist unabhaengig vom Media-Backend (FFmpeg
            // wie WMF) und davon, ob die Pipeline auf dem Main- oder dem
            // Worker-Thread lebt; ein CoInitializeEx/CoUninitialize-Paar auf
            // dem Thread half ebenfalls nicht. Es ist exakt der Unterschied
            // zum Commit f93c83a, in dem das Beenden nachweislich sauber war
            // (dort gab es KEINEN Zusatz-Thread).
            //
            // DEFAULT ist deshalb der Main-Thread: eine App, die sich nicht
            // schliessen laesst, waere schlimmer als der UI-Lag, gegen den
            // der Thread eingefuehrt wurde (und der seit der 720p/30fps-
            // Klemme deutlich kleiner ausfaellt). `LUMIVIZ_MEDIEN_THREAD=1`
            // schaltet ihn wieder ein — fuer A/B-Messungen und fuer den Fall,
            // dass der Lag doch stoert.
            if (qEnvironmentVariable("LUMIVIZ_MEDIEN_THREAD",
                                     QStringLiteral("0")) == QLatin1String("1"))
            {
                d->m_medien = new QThread();
                d->m_medien->setObjectName(QStringLiteral("LiveVideoMedien"));
                d->m_medien->start();
                d->m_medienKontext = new QObject();
                d->m_medienKontext->moveToThread(d->m_medien);
                std::fprintf(stderr,
                             "[LiveVideoFeed] MEDIEN-THREAD AN — Prozessende "
                             "kann nach Kamera-Nutzung haengen (Befund S71)\n");
            }
            else
            {
                d->m_medienKontext = d;  // Auftraege laufen auf dem Main-Thread
            }
            // Shutdown-Haken (S70, dritter Anlauf): bei aboutToQuit NUR den
            // Riegel setzen (keine Kamera-Neustarts mehr) — der eigentliche
            // Stopp laeuft in herunterfahren() NACH dem Fenster-/GL-Abbau
            // (Application::shutdown), weil eine sterbende MF-/D3D-Pipeline
            // waehrend des GL-Teardowns den GPU-Kanal verklemmt (nvoglv64-
            // Haenger). herunterfahren() ist zusaetzlich als Sicherheitsnetz
            // angebunden, falls shutdown() nie laeuft (Notausgang) —
            // zweifacher Aufruf ist harmlos (idempotent).
            QObject::connect(app, &QCoreApplication::aboutToQuit, d,
                             [d] { d->beendenVorbereiten(); });
        }
        return d;
    }();
    return *s_instanz;
}

bool LiveVideoFeed::aufMedienThread(const std::function<void()>& arbeit,
                                    int maxMs)
{
    if (m_medienKontext == nullptr || m_medien == nullptr ||
        !m_medien->isRunning())
    {
        arbeit();  // kein Medien-Thread (Tests/Notausgang) => hier erledigen
        return true;
    }
    if (QThread::currentThread() == m_medien)
    {
        arbeit();  // schon dort — ein Auftrag an sich selbst waere ein Deadlock
        return true;
    }
    // Fertigmeldung per Semaphore statt BlockingQueuedConnection: ein
    // haengender Medien-Thread darf den Abbau VERZOEGERN, nicht verklemmen.
    // Der shared_ptr haelt die Semaphore am Leben, falls wir aufgeben und das
    // Lambda erst spaeter laeuft.
    auto fertig = std::make_shared<QSemaphore>();
    QMetaObject::invokeMethod(
        m_medienKontext,
        [arbeit, fertig] {
            arbeit();
            fertig->release();
        },
        Qt::QueuedConnection);
    return fertig->tryAcquire(1, maxMs);
}

void LiveVideoFeed::herunterfahren()
{
    // AUFTRAG + ACK (Entscheid Patrik): der Abbau laeuft DORT, wo die
    // Multimedia-Objekte leben — der Main-Thread wartet nur auf die
    // Fertigmeldung und beendet danach den Thread.
    if (!aufMedienThread([this] { alleStoppen(); }, 5000))
    {
        BasicLogger::logWarning(
            "[LiveVideoFeed] Feed-Abbau auf dem Medien-Thread nicht "
            "bestaetigt (5 s) — Thread wird trotzdem beendet");
    }
    if (m_medien != nullptr && m_medien->isRunning())
    {
        QElapsedTimer uhr;
        uhr.start();
        // BEFUND S71 (gemessen): EIN einzelnes `quit()` direkt nach dem
        // Feed-Abbau geht verloren — `wait(5000)` lief dann in 3 von 3
        // Laeufen in den Timeout, waehrend derselbe Aufruf mit einer Pause
        // davor in 3 von 3 Laeufen nach ~400 ms durchkam. Der Wandler
        // verarbeitet unmittelbar nach dem Abbau noch Ereignisse der
        // Multimedia-Pipeline und wird erst danach quit-faehig; ein von
        // innen gepostetes Quit-Event half ebenso wenig (auch 3/3 Timeout).
        // Deshalb wird das Quit WIEDERHOLT, bis der Thread es annimmt —
        // selbstkorrigierend statt mit einer geratenen Wartezeit, und in
        // Summe nie laenger als die bisherigen 5 s.
        bool beendet = false;
        for (int versuch = 0; versuch < 25 && !beendet; ++versuch)
        {
            m_medien->quit();
            beendet = m_medien->wait(200);
        }
        if (beendet)
        {
            BasicLogger::logDebug("[LiveVideoFeed] Wandler-Thread beendet nach " +
                                  std::to_string(uhr.elapsed()) + " ms");
        }
        else
        {
            // Auch 25 Versuche haben nicht gereicht: dann steckt der Wandler
            // wirklich fest (map() einer Pipeline). Kontext NICHT anfassen —
            // der Thread koennte ihn noch benutzen. Die stilllegen-Zeilen
            // oben zeigen, WELCHER Feed ihn festhaelt (begonnen > fertig).
            BasicLogger::logWarning(
                "[LiveVideoFeed] Wandler-Thread endete NICHT (25 Versuche) — "
                "s. stilllegen-Zeilen oben");
            return;
        }
    }
    // Der Kontext stirbt erst NACH dem Thread-Ende und direkt: ein
    // deleteLater() waere ein Event an einen Thread ohne Event-Loop und
    // wuerde nie ausgefuehrt (der Kontext samt anhaengender Frames bliebe
    // liegen). Ohne laufenden Thread gibt es hier kein Rennen.
    // Ausnahme: ohne Medien-Thread IST der Kontext der Dienst selbst.
    if (m_medienKontext != this) delete m_medienKontext;
    m_medienKontext = nullptr;
}

void LiveVideoFeed::wandlerBarriere(int maxMs)
{
    if (m_medienKontext == nullptr || m_medien == nullptr ||
        !m_medien->isRunning())
    {
        return;  // kein laufender Medien-Thread => kein Frame in Flug
    }
    if (QThread::currentThread() == m_medien)
    {
        // Wir SIND der Verarbeiter: der Frame-Handler laeuft auf genau
        // diesem Thread und kann daher nicht parallel laufen. Seit dem
        // Ein-Thread-Besitz ist das der Normalfall — die Barriere kostet
        // hier nichts (und ein Auftrag an sich selbst waere ein Deadlock).
        return;
    }
    // Die Semaphore gehoert einem shared_ptr, NICHT dem Stack: laeuft die
    // Wartezeit ab, liegt das Lambda noch in der Wandler-Queue und wuerde
    // sonst auf einen toten Stack-Rahmen schreiben.
    auto durch = std::make_shared<QSemaphore>();
    // Das Lambda reiht sich HINTER alle bereits zugestellten Frame-Events —
    // laeuft es, sind die abgearbeitet und ihre Pipeline-Puffer zurueck.
    QElapsedTimer uhr;
    uhr.start();
    QMetaObject::invokeMethod(
        m_medienKontext, [durch] { durch->release(); }, Qt::QueuedConnection);
    if (!durch->tryAcquire(1, maxMs))
    {
        BasicLogger::logWarning(
            "[LiveVideoFeed] Wandler-Barriere TIMEOUT nach " +
            std::to_string(maxMs) +
            " ms — der Wandler-Thread arbeitet nicht mehr");
    }
    else
    {
        BasicLogger::logDebug("[LiveVideoFeed] Wandler-Barriere durch nach " +
                              std::to_string(uhr.elapsed()) + " ms");
    }
}

void LiveVideoFeed::feedStilllegen(const std::shared_ptr<Feed>& feed)
{
    if (feed == nullptr) return;
    // Diagnose S71: Steckt der Wandler in genau DIESEM Feed? Dann ist
    // `begonnen` groesser als `fertig`, und die Barriere unten wird
    // zwangslaeufig in den Timeout laufen.
    const auto beg = feed->begonnen.load(std::memory_order_acquire);
    const auto fer = feed->fertig.load(std::memory_order_acquire);
    BasicLogger::logDebug(
        "[LiveVideoFeed] stilllegen: quelle=" + feed->quelle.toStdString() +
        " frames_begonnen=" + std::to_string(beg) +
        " fertig=" + std::to_string(fer) +
        (beg != fer ? "  << WANDLER STECKT IN DIESEM FEED" : ""));
    // --- Der ABSCHALT-VERTRAG (Header). Reihenfolge ist hier alles. ---
    // 1. Totflagge: der Wandler laesst ab jetzt jedes Frame dieses Feeds
    //    liegen, statt es zu mappen.
    feed->tot.store(true, std::memory_order_release);
    // 2. Senke abklemmen: es kommen keine neuen Frames mehr nach.
    if (feed->senke != nullptr) feed->senke->disconnect();
    // 3. Barriere: die schon zugestellten Frames sind durch und ihre
    //    Pipeline-Puffer zurueckgegeben. OHNE diesen Schritt blockiert
    //    Schritt 4 im Treiber — das war der Teardown-Deadlock (S71).
    wandlerBarriere(2000);
    // 4. Erst JETZT stoppen und zerstoeren (Main-Thread, synchron).
    if (feed->kamera != nullptr) feed->kamera->stop();
    if (feed->spieler != nullptr) feed->spieler->stop();
    feed->session.reset();
    feed->kamera.reset();
    feed->spieler.reset();
    feed->senke.reset();
}

void LiveVideoFeed::alleStoppen()
{
    // Shutdown-Riegel ZUERST: ab jetzt startet nichts mehr (auch nicht die
    // schon gequeueten baue*-Events — sie pruefen den Riegel beim Lauf).
    m_beendet.store(true, std::memory_order_release);
    std::unordered_map<std::uint64_t, std::shared_ptr<Feed>> feeds;
    std::vector<std::shared_ptr<Feed>> tote;
    {
        QMutexLocker sperre(&m_mutex);
        feeds.swap(m_feeds);
        tote.swap(m_friedhof);  // auch nie zugestellte stopp()-Reste (S70 #2)
        m_imBau.clear();  // offene Bauauftraege verfallen (Riegel steht)
    }
    BasicLogger::logDebug(
        "[LiveVideoFeed] alleStoppen: aktive Feeds=" +
        std::to_string(feeds.size()) + " Friedhof=" + std::to_string(tote.size()));
    for (auto& [id, feed] : feeds) tote.push_back(feed);
    // Der Mutex ist hier bewusst FREI: die Barriere in feedStilllegen wartet
    // auf den Wandler, und dessen Handler will genau diesen Mutex.
    for (auto& feed : tote) feedStilllegen(feed);
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
    if (m_beendet.load(std::memory_order_acquire)) return;  // Shutdown-Riegel
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
                // Auf dem Medien-Thread: dort lebt der QMediaPlayer.
                QMetaObject::invokeMethod(
                    m_medienKontext,
                    [feed, tempo] {
                        if (feed->spieler != nullptr)
                            feed->spieler->setPlaybackRate(tempo);
                    },
                    Qt::QueuedConnection);
            }
            return;
        }
    }
    // Nur EIN Bauauftrag je Knoten und Quelle (s. m_imBau): der Render-Thread
    // ruft hier in jedem Frame an, bis der Feed steht.
    if (!bauVormerken(nodeId, quelle)) return;
    QMetaObject::invokeMethod(
        m_medienKontext,
        [this, nodeId, pfad, loop, tempo] { baueDatei(nodeId, pfad, loop, tempo); },
        Qt::QueuedConnection);
}

void LiveVideoFeed::starteKamera(std::uint64_t nodeId, const QString& geraeteId)
{
    if (m_beendet.load(std::memory_order_acquire)) return;  // Shutdown-Riegel
    if (!kameraErlaubt()) return;  // Kamera-Vertrag: nie ohne Freigabe
    const QString quelle = QStringLiteral("kamera:%1").arg(geraeteId);
    {
        QMutexLocker sperre(&m_mutex);
        auto it = m_feeds.find(nodeId);
        if (it != m_feeds.end() && it->second->quelle == quelle) return;
    }
    // Nur EIN Bauauftrag je Knoten und Quelle (s. m_imBau) — sonst queued der
    // Render-Thread bis zum Fertigbau in jedem Frame einen weiteren, und jeder
    // raeumt die eben gebaute Kamera wieder ab (App friert ein, S71).
    if (!bauVormerken(nodeId, quelle)) return;
    QMetaObject::invokeMethod(
        m_medienKontext, [this, nodeId, geraeteId] { baueKamera(nodeId, geraeteId); },
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
        BasicLogger::logDebug(
            "[LiveVideoFeed] stopp: node=" + std::to_string(nodeId) +
            " -> Friedhof(" + std::to_string(m_friedhof.size()) +
            ") aktive Feeds=" + std::to_string(m_feeds.size()));
    }
    QMetaObject::invokeMethod(
        m_medienKontext, [this] { friedhofLeeren(); }, Qt::QueuedConnection);
}

bool LiveVideoFeed::bauVormerken(std::uint64_t nodeId, const QString& quelle)
{
    QMutexLocker sperre(&m_mutex);
    auto it = m_imBau.find(nodeId);
    if (it != m_imBau.end() && it->second == quelle) return false;
    m_imBau[nodeId] = quelle;
    return true;
}

void LiveVideoFeed::bauAustragen(std::uint64_t nodeId)
{
    QMutexLocker sperre(&m_mutex);
    m_imBau.erase(nodeId);
}

void LiveVideoFeed::altenFeedRaeumen(std::uint64_t nodeId)
{
    std::shared_ptr<Feed> alt;
    {
        QMutexLocker sperre(&m_mutex);
        auto it = m_feeds.find(nodeId);
        if (it == m_feeds.end()) return;
        alt = it->second;
        m_feeds.erase(it);
    }
    // BEWUSST OHNE MUTEX: feedStilllegen wartet auf den Wandler, und dessen
    // Handler will genau diesen Mutex — unter der Sperre waere das ein
    // sicherer Deadlock (Befund S71).
    BasicLogger::logDebug("[LiveVideoFeed] Alt-Feed ersetzen: node=" +
                          std::to_string(nodeId));
    feedStilllegen(alt);
}

void LiveVideoFeed::friedhofLeeren()
{
    std::vector<std::shared_ptr<Feed>> tote;
    {
        QMutexLocker sperre(&m_mutex);
        tote.swap(m_friedhof);
    }
    // Qt-Objekte werden in feedStilllegen HIER explizit zerstoert (nicht erst
    // beim letzten shared_ptr): haelt z. B. ein Tempo-Lambda in der
    // Event-Queue noch eine Referenz, bleibt dort nur eine leere Huelle.
    for (auto& feed : tote) feedStilllegen(feed);
}

std::uint64_t LiveVideoFeed::bildNummer(std::uint64_t nodeId)
{
    QMutexLocker sperre(&m_mutex);
    auto it = m_feeds.find(nodeId);
    return it != m_feeds.end() ? it->second->nummer : 0;
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
                                  QObject* kontext,
                                  const std::shared_ptr<Feed>& feed)
{
    std::weak_ptr<Feed> schwach = feed;
    // Empfaenger-Kontext lebt auf dem Wandler-Thread -> Qt stellt QUEUED zu:
    // der Handler (und damit die teure Wandlung) laeuft dort — nie auf dem
    // GUI-Thread (fror das UI ein) und nie auf dem Render-Thread (haengt
    // bei Hardware-Frames). Lag-Befund S70.
    QObject::connect(senke, &QVideoSink::videoFrameChanged, kontext,
                     [schwach, mutex](const QVideoFrame& frame) {
                         auto feed = schwach.lock();
                         // TOTFLAGGE VOR dem map(): ein Frame einer bereits
                         // sterbenden MF-/D3D-Pipeline zu mappen blockiert im
                         // Treiber und haelt deren Puffer fuer immer — genau
                         // so starben die nvwgf2umx-Worker nie (Befund S71).
                         // Frueher Return gibt den Puffer sofort zurueck.
                         if (feed == nullptr ||
                             feed->tot.load(std::memory_order_acquire) ||
                             !frame.isValid())
                         {
                             return;
                         }
                         // Diagnose S71: begonnen/fertig klaffen genau dann
                         // auseinander, wenn der Wandler im map() steckt.
                         feed->begonnen.fetch_add(1, std::memory_order_acq_rel);
                         QImage img = videoFrameZuBild(frame);
                         feed->fertig.fetch_add(1, std::memory_order_acq_rel);
                         if (img.isNull()) return;
                         QMutexLocker sperre(mutex);
                         feed->bild = std::move(img);
                         ++feed->nummer;
                     });
}

void LiveVideoFeed::baueDatei(std::uint64_t nodeId, const QString& pfad,
                              bool loop, double tempo)
{
    // Riegel auch HIER: das Event kann nach alleStoppen() zugestellt werden
    if (m_beendet.load(std::memory_order_acquire))
    {
        bauAustragen(nodeId);
        return;
    }
    // Vorgaenger ZUERST vertragsgemaess abbauen (s. altenFeedRaeumen)
    altenFeedRaeumen(nodeId);
    auto feed = std::make_shared<Feed>();
    feed->quelle = QStringLiteral("datei:%1|%2").arg(pfad).arg(loop ? 1 : 0);
    feed->tempo = tempo;
    feed->senke = std::make_unique<QVideoSink>();
    feed->spieler = std::make_unique<QMediaPlayer>();
    feed->spieler->setVideoSink(feed->senke.get());
    feed->spieler->setLoops(loop ? QMediaPlayer::Infinite : 1);
    feed->spieler->setSource(QUrl::fromLocalFile(pfad));
    feed->spieler->setPlaybackRate(tempo);
    verbindeSenke(feed->senke.get(), &m_mutex, m_medienKontext, feed);
    feed->spieler->play();
    // Auch der Datei-Weg ist eine Qt-Multimedia-Pipeline mit Wandler-Frames
    // — der Notausgang in Application::shutdown haengt daran (Befund S71:
    // ein reiner Datei-Lauf hing genauso wie ein Kamera-Lauf).
    m_feedLief.store(true, std::memory_order_release);

    QMutexLocker sperre(&m_mutex);
    m_feeds[nodeId] = std::move(feed);
    m_imBau.erase(nodeId);  // Bauauftrag erledigt (s. m_imBau)
    BasicLogger::logDebug(
        "[LiveVideoFeed] DATEI gebaut: node=" + std::to_string(nodeId) +
        " aktive Feeds=" + std::to_string(m_feeds.size()) + " Friedhof=" +
        std::to_string(m_friedhof.size()));
}

void LiveVideoFeed::baueKamera(std::uint64_t nodeId, const QString& geraeteId)
{
    // Riegel auch HIER: das Event kann nach alleStoppen() zugestellt werden —
    // ohne den Riegel startete mitten im Teardown eine FRISCHE Kamera-
    // Pipeline (Main-Thread hing in nvoglv64; Befund S70).
    if (m_beendet.load(std::memory_order_acquire))
    {
        bauAustragen(nodeId);
        return;
    }
    // Vorgaenger ZUERST vertragsgemaess abbauen (s. altenFeedRaeumen)
    altenFeedRaeumen(nodeId);
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
        m_imBau.erase(nodeId);  // Bauauftrag erledigt (s. m_imBau)
        return;
    }
    feed->senke = std::make_unique<QVideoSink>();
    feed->kamera = std::make_unique<QCamera>(geraet);
    // Kamera-Format stimmen (Lag-Nachstimmung S70): moderate Aufloesung um
    // 720p und <=30 fps statt des Geraete-Maximums — die RGBX-Wandlung je
    // Frame skaliert mit der Pixelzahl (1080p60 kostet >4x soviel wie
    // 720p30 und gewinnt fuer den Visualizer nichts).
    QCameraFormat beste;
    double besteNote = 1e18;
    const auto formate = geraet.videoFormats();
    for (const QCameraFormat& fmt : formate)
    {
        const QSize s = fmt.resolution();
        if (s.isEmpty()) continue;
        const double pix = static_cast<double>(s.width()) * s.height();
        double note = std::fabs(pix - 1280.0 * 720.0);
        if (fmt.maxFrameRate() > 31.0) note += 5e5;  // lieber <= 30 fps
        if (note < besteNote)
        {
            besteNote = note;
            beste = fmt;
        }
    }
    if (!beste.isNull()) feed->kamera->setCameraFormat(beste);
    feed->session = std::make_unique<QMediaCaptureSession>();
    feed->session->setCamera(feed->kamera.get());
    feed->session->setVideoSink(feed->senke.get());
    verbindeSenke(feed->senke.get(), &m_mutex, m_medienKontext, feed);
    feed->kamera->start();
    m_feedLief.store(true, std::memory_order_release);  // s. feedGelaufen()

    QMutexLocker sperre(&m_mutex);
    m_feeds[nodeId] = std::move(feed);
    m_imBau.erase(nodeId);  // Bauauftrag erledigt (s. m_imBau)
    BasicLogger::logDebug(
        "[LiveVideoFeed] KAMERA gebaut: node=" + std::to_string(nodeId) +
        " aktive Feeds=" + std::to_string(m_feeds.size()) + " Friedhof=" +
        std::to_string(m_friedhof.size()));
}

} // namespace lumi::services
