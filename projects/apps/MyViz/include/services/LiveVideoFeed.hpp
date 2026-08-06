/**
 ****************************************************************************************
 * @file   LiveVideoFeed.hpp
 * @brief  Live-Videoquellen je Chain-Node: Datei-Streaming + Kamera (Qt Multimedia)
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Gegenstueck zum VideoFrameCache (der dekodiert deterministisch KOMPLETT in den
 * Speicher): dieser Dienst liefert UHRZEITGETRIEBENE Frames — Datei-Streaming
 * ueber QMediaPlayer (lange Videos ohne RAM-Kosten) und Kamera ueber
 * QCamera/QMediaCaptureSession. Beide muenden in einen QVideoSink je Feed; das
 * jeweils letzte Bild liegt als RGBX8888-QImage unter Mutex bereit
 * (`letztesBild`), der Render-Thread holt es je Frame ab. Live heisst nicht
 * reproduzierbar — Sonden/Standalone nutzen den VideoFrameCache-Weg des
 * videoSource-Knotens (streaming aus).
 *
 * KAMERA-VERTRAG (Sondierung Offene_Punkte §7): ein Kamerageraet startet NIE
 * automatisch — `starteKamera()` ist ein No-op, solange nicht `erlaubeKamera()`
 * in DIESEM App-Lauf durch eine ausdrueckliche Nutzeraktion (Panel-Knopf,
 * Settings-Testaufnahme) gesetzt wurde. Ein Preset-Laden alleine oeffnet damit
 * nie den Windows-Berechtigungsdialog.
 *
 * Threading: starteDatei/starteKamera/stopp sind render-thread-tauglich (Queued-
 * Invoke auf den Main-Thread, dessen Event-Loop Qt Multimedia braucht —
 * dasselbe Muster wie VideoFrameCache). `letztesBild` kopiert unter Mutex.
 ****************************************************************************************
 */

#pragma once

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QString>

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

class QVideoSink;

namespace lumi::services {

/**
 * @class LiveVideoFeed
 * @brief Haelt je Chain-Node einen laufenden Video-/Kamera-Feed (Main-Thread)
 */
class LiveVideoFeed : public QObject
{
    // bewusst OHNE Q_OBJECT: keine eigenen Signals/Slots — Queued-Invoke
    // laeuft ueber das Lambda-Overload (Muster VideoFrameCache)
public:
    static LiveVideoFeed& instance();

    /// Kamera-Freigabe fuer diesen App-Lauf — nur aus einer ausdruecklichen
    /// Nutzeraktion heraus setzen (Panel-Knopf/Settings), nie beim Laden.
    void erlaubeKamera() { m_kameraErlaubt.store(true, std::memory_order_release); }
    [[nodiscard]] bool kameraErlaubt() const
    {
        return m_kameraErlaubt.load(std::memory_order_acquire);
    }

    /// Benutzerlokaler Ablageordner der Kamera-Testaufnahmen (wird angelegt);
    /// bewusst AUSSERHALB des Repos (Kameramaterial ist persoenlich).
    static QString testaufnahmenOrdner();

    /// Datei-Streaming starten bzw. weiterlaufen lassen (idempotent je Frame
    /// aufrufbar; nur ein Quellen-Wechsel baut den Feed neu). `tempo` wirkt
    /// als Abspielrate und darf sich laufend aendern.
    void starteDatei(std::uint64_t nodeId, const QString& pfad, bool loop,
                     double tempo);

    /// Kamera starten (idempotent). NO-OP ohne `erlaubeKamera()` in diesem
    /// App-Lauf. `geraeteId` = QCameraDevice::id() als UTF-8-String; eine
    /// unbekannte ID startet nichts (kein Retry-Spam bis zum ID-Wechsel).
    void starteKamera(std::uint64_t nodeId, const QString& geraeteId);

    /// Feed des Knotens beenden und freigeben (idempotent).
    void stopp(std::uint64_t nodeId);

    /// ALLE Feeds SYNCHRON beenden — läuft auf dem Main-Thread, angebunden an
    /// QCoreApplication::aboutToQuit (S70-Befund: die Queued-Stopps kommen
    /// beim Herunterfahren nicht mehr an, eine laufende QCamera hält den
    /// Prozess dann über ihre Capture-Threads am Leben).
    void alleStoppen();

    /**
     * @brief Letztes Bild des Feeds (RGBX8888, top-down)
     * @param nummer laufende Bild-Nummer (0 = noch keins) — Upload-Ersparnis
     * @return Null-Image, solange noch kein Frame angekommen ist
     */
    QImage letztesBild(std::uint64_t nodeId, std::uint64_t* nummer = nullptr);

private:
    explicit LiveVideoFeed() = default;

    struct Feed;

    /// Laufen auf dem Main-Thread (Queued-Invoke aus starteDatei/-Kamera/stopp)
    void baueDatei(std::uint64_t nodeId, const QString& pfad, bool loop,
                   double tempo);
    void baueKamera(std::uint64_t nodeId, const QString& geraeteId);
    /// Entsorgt gestoppte Feeds (m_friedhof) auf dem Main-Thread. Das
    /// Queued-Lambda von stopp() traegt bewusst KEIN Eigentum — landet das
    /// Event nie (Shutdown), raeumt alleStoppen() synchron ab. Sonst stirbt
    /// die QCamera beim Entsorgen der Event-Queue MITTEN im ~QApplication
    /// (S70-Befund #2: Haupt-Thread haengt in Application::shutdown).
    void friedhofLeeren();
    /// Sink-Anschluss: jedes Bild nach RGBX8888 wandeln und unter `mutex`
    /// in den Feed legen (laeuft auf dem Main-Thread, der Sink lebt dort).
    static void verbindeSenke(QVideoSink* senke, QMutex* mutex,
                              const std::shared_ptr<Feed>& feed);

    QMutex m_mutex;
    std::unordered_map<std::uint64_t, std::shared_ptr<Feed>> m_feeds;
    /// Gestoppte Feeds bis zur Main-Thread-Entsorgung (s. friedhofLeeren)
    std::vector<std::shared_ptr<Feed>> m_friedhof;
    std::atomic<bool> m_kameraErlaubt{false};
};

} // namespace lumi::services
