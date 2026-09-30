/**
 ****************************************************************************************
 * @file   AlbumCover.hpp
 * @brief  Albumcover einer Audiodatei finden (S84, Wunsch Patrik)
 *
 * @details
 * Reihenfolge (Entscheid S84): **eingebettetes Cover** → **Cover-Datei im
 * Ordner** → nichts (der Aufrufer nimmt dann sein eigenes Icon).
 *
 * Eingebettet gelesen werden, ohne fremde Bibliothek:
 * - **ID3v2.2/2.3/2.4** (MP3 u. a.): Rahmen `PIC`/`APIC`, samt
 *   Unsynchronisation und erweitertem Kopf;
 * - **FLAC**: Metadatenblock 6 (`PICTURE`), auch hinter einem ID3-Kopf;
 * - **MP4/M4A**: `moov/udta/meta/ilst/covr/data`.
 * Bei mehreren Bildern gewinnt das Frontcover (Bildtyp 3), sonst das erste.
 * Noch nicht: Ogg/Opus (`METADATA_BLOCK_PICTURE` im Kommentar), WMA.
 *
 * Cover-Dateien im Ordner: `cover`, `folder`, `front`, `albumart*` mit
 * `.jpg/.jpeg/.png` (Gross-/Kleinschreibung egal).
 *
 * `CoverLader` laedt Vorschaubilder fuer eine ganze Liste in einem eigenen
 * Thread — ein Musikordner mit hunderten Titeln soll die Oberflaeche nicht
 * anhalten. Kein GL, keine Widgets: voll unit-testbar.
 ****************************************************************************************
 */

#pragma once

#include <QByteArray>
#include <QImage>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>

#include <atomic>

namespace lumi::audio
{

/// Rohdaten (JPEG/PNG …) des eingebetteten Covers, leer = keins gefunden.
[[nodiscard]] QByteArray eingebettetesCover(const QString& pfad);

/// Cover-Datei im Ordner (absoluter Pfad), leer = keine.
[[nodiscard]] QString ordnerCover(const QString& ordner);

/**
 * Vorschaubild einer Audiodatei: eingebettet, sonst Ordner-Cover; auf
 * `kante` × `kante` verkleinert (Seitenverhaeltnis bleibt). Null = keins.
 * Laeuft in jedem Thread (nur QImage, keine Pixmap).
 */
[[nodiscard]] QImage coverVorschau(const QString& pfad, int kante);

/**
 * Laedt Vorschaubilder im eigenen Thread. `lade()` bricht einen laufenden
 * Auftrag ab (Ordnerwechsel); Ergebnisse tragen die Auftragsnummer, damit der
 * Empfaenger veraltete verwirft. Der Destruktor wartet auf den Thread.
 */
class CoverLader : public QObject
{
    Q_OBJECT

public:
    explicit CoverLader(QObject* parent = nullptr);
    ~CoverLader() override;

    CoverLader(const CoverLader&) = delete;
    CoverLader& operator=(const CoverLader&) = delete;

    /// Neuer Auftrag; gibt seine Nummer zurueck.
    int lade(const QStringList& pfade, int kante);

Q_SIGNALS:
    /// Ein Cover gefunden (nur fuer Dateien MIT Cover; Queued in den GUI-Thread).
    void geladen(int auftrag, const QString& pfad, const QImage& bild);
    /// Auftrag fertig oder abgebrochen.
    void fertig(int auftrag);

private:
    QThread m_thread;
    std::atomic<int> m_auftrag{0};
    QObject* m_arbeiter = nullptr;  ///< lebt im Thread, fuehrt die Auftraege aus
};

}  // namespace lumi::audio
