/**
 ****************************************************************************************
 * @file   VideoFrameCache.hpp
 * @brief  Frame-genauer Video-Decoder-Cache ueber Qt Multimedia (FFmpeg-Backend)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026 (fps-Feld August 2026, S70)
 * @version 1.1.0
 *
 * @details
 * Stufe 1 des Video-Wegs (Entscheid S59): der AVS-`avi`-Knoten dekodiert
 * weiterhin bevorzugt ueber VfW (bit-treu zur Referenz) — scheitert VfW aber
 * am Codec (64-Bit-Windows kennt keine Alt-Codecs wie Indeo 3.2 mehr),
 * springt dieser Cache ein: er dekodiert die Datei EINMAL vollstaendig in
 * eine Frame-Liste, aus der der Knoten deterministisch NACH INDEX liest —
 * die Frame-Schritt-Pflicht der Sondierung (Offene_Punkte §7): Qt liefert
 * Frames uhrzeitgetrieben, reproduzierbare Laeufe brauchen Index-Zugriff.
 *
 * Threading: `hole()` ist render-thread-tauglich (Mutex + Queued-Invoke);
 * das Dekodieren laeuft auf dem MAIN-Thread (QMediaPlayer braucht dessen
 * Event-Loop). Der Aufrufer pollt `Clip::status` und liest `frames` erst
 * bei FERTIG — danach ist der Clip unveraenderlich.
 ****************************************************************************************
 */

#pragma once

#include <QHash>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QString>

#include <atomic>
#include <memory>
#include <vector>

namespace lumi::services {

/**
 * @class VideoFrameCache
 * @brief Dekodiert Videodateien einmalig in QImage-Listen (ein Clip je Pfad)
 */
class VideoFrameCache : public QObject
{
    // bewusst OHNE Q_OBJECT: keine eigenen Signals/Slots — Queued-Invoke
    // laeuft ueber das Lambda-Overload, Signal-Warten ueber fremde QObjects
public:
    /// Zustand eines Clips (Werte von Clip::status)
    enum Status
    {
        LAEDT = 0,
        FERTIG = 1,
        FEHLGESCHLAGEN = -1
    };

    struct Clip
    {
        /// RGBX8888, top-down; erst lesen, wenn status == FERTIG (danach
        /// unveraenderlich — der Ladevorgang schreibt nur VOR dem Umschalten)
        std::vector<QImage> frames;
        /// Bildrate der Quelle (S70, fuer Original-Tempo-Abspielen); wie
        /// `frames` erst nach FERTIG lesen. 25 = Ausweichwert der Ladung.
        double fps = 25.0;
        std::atomic<int> status{LAEDT};
    };

    static VideoFrameCache& instance();

    /**
     * @brief Laden anstossen (einmal je Pfad) und den Clip liefern
     * @return nie nullptr; Zustand ueber Clip::status pollen
     */
    std::shared_ptr<Clip> hole(const QString& pfad);

private:
    explicit VideoFrameCache() = default;

    /// Laeuft auf dem Main-Thread (Queued-Invoke aus hole())
    void lade(const QString& pfad, std::shared_ptr<Clip> clip);

    QMutex m_mutex;
    QHash<QString, std::shared_ptr<Clip>> m_clips;
};

} // namespace lumi::services
