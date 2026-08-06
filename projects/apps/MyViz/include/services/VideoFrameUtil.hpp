/**
 ****************************************************************************************
 * @file   VideoFrameUtil.hpp
 * @brief  QVideoFrame -> QImage ueber map()+Rohbytes (toImage-Ersatz)
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * BEFUND S70: `QVideoFrame::toImage()` liefert unter Qt 6.10.1 (FFmpeg-
 * Backend, Windows) SCHWARZE Bilder mit korrekten Zeitstempeln — die
 * Rohdaten sind per `map()` aber vollstaendig da (Sonde: testvideo.avi,
 * Hintergrund 0x20/0x20/0x30 kommt in bits(0) an, toImage = 0/0/0).
 * Dieser Helfer baut das QImage deshalb selbst aus den gemappten Bytes.
 * Verwendet von VideoFrameCache (Frame-Schritt) und LiveVideoFeed
 * (Kamera/Streaming) — EINE Stelle fuer den Workaround (SSOT).
 ****************************************************************************************
 */

#pragma once

#include <QImage>
#include <QVideoFrame>
#include <QVideoFrameFormat>

namespace lumi::services {

/**
 * @brief Videoframe nach RGBX8888 wandeln (top-down), ohne toImage()
 * @return Null-Image, wenn der Frame nicht mappbar/abbildbar ist
 */
inline QImage videoFrameZuBild(const QVideoFrame& frame)
{
    if (!frame.isValid()) return {};
    QVideoFrame kopie = frame;  // map() braucht ein nicht-konstantes Handle
    if (!kopie.map(QVideoFrame::ReadOnly)) return {};
    QImage ergebnis;
    const QImage::Format qfmt =
        QVideoFrameFormat::imageFormatFromPixelFormat(kopie.pixelFormat());
    if (qfmt != QImage::Format_Invalid && kopie.bits(0) != nullptr)
    {
        const QImage huelle(kopie.bits(0), kopie.width(), kopie.height(),
                            kopie.bytesPerLine(0), qfmt);
        // Deep-Copy VOR unmap(): convertToFormat teilt bei gleichem Format
        // den Puffer — der stirbt mit unmap().
        ergebnis = qfmt == QImage::Format_RGBX8888
                       ? huelle.copy()
                       : huelle.convertToFormat(QImage::Format_RGBX8888);
    }
    kopie.unmap();
    if (ergebnis.isNull())
    {
        // Ausweich fuer Nicht-RGB-Formate (z. B. YUV-Planar): toImage kann
        // dort weiterhin richtig liegen — besser als gar kein Bild.
        ergebnis = frame.toImage().convertToFormat(QImage::Format_RGBX8888);
    }
    return ergebnis;
}

} // namespace lumi::services
