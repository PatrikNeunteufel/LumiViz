/**
 ****************************************************************************************
 * @file   ScreenshotManager.hpp
 * @brief  Screenshots des Visuals in einen Ordner je Programmlauf
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 *
 * @details
 * ## Was hier entsteht
 *
 * ```
 * <Basis>/2026-07-27_11-47-57/          <- ein Ordner je Programmstart
 *     Alien_Alloy_avs.png               <- das Visual, Dateiname = Preset
 *     Alien_Alloy_avs.txt               <- absoluter Pfad des Presets
 *     fehler.log                        <- nur wenn im Vollbild etwas schiefging
 * ```
 *
 * Die Basis kommt aus `QSettings` (`screenshot/baseDir`); ohne Einstellung wird
 * `asset/calibration/screenshot` neben dem Projekt gesucht (Aufwaerts-Suche wie
 * bei den Preset-Icons), sonst landen die Bilder unter *Bilder*.
 *
 * Der Ordner wird **verzoegert** angelegt: ein Lauf ohne Screenshot hinterlaesst
 * keine leeren Verzeichnisse.
 *
 * ## Warum asynchron
 *
 * Aufnehmen kann nur der Render-Thread (`glReadPixels` braucht den GL-Kontext).
 * Eine Anforderung merkt sich deshalb nur, WAS zu dem Bild geschrieben werden
 * soll; geschrieben wird, wenn das Bild eintrifft (`onFrameCaptured`).
 *
 * ## Vollbild-Regel (Vorgabe Patrik, Session 52)
 *
 * Im Vollbild darf kein Meldungsfenster erscheinen — es reisst die Vorfuehrung
 * auseinander und der Dialog landet hinter dem Vollbildfenster. Statt des
 * Dialogs: eine Zeile in `fehler.log` **und** ein automatischer Screenshot, so
 * dass hinterher nachvollziehbar ist, wie das Preset dabei aussah. Im Fenster
 * bleibt der Dialog.
 ****************************************************************************************
 */

#pragma once

#include <QDateTime>
#include <QImage>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QStringList>

#include <functional>

class VisualizerWidget;

/**
 * @class ScreenshotManager
 * @brief Nimmt das Visual auf und legt Bild, Preset-Pfad und Fehler ab
 */
class ScreenshotManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @param startedAt Startzeit des Programms — benennt den Ordner des Laufs
     */
    explicit ScreenshotManager(const QDateTime& startedAt, QObject* parent = nullptr);

    // =========================================================================
    // Reine Regeln (statisch, ohne Fenster pruefbar)
    // =========================================================================

    /// @brief Ordnername eines Laufs: `2026-07-27_11-47-57` (sortiert sich).
    [[nodiscard]] static QString sessionFolderName(const QDateTime& startedAt);

    /**
     * @brief Basisname der Dateien aus dem Preset-Pfad.
     *
     * Die Endung bleibt als `_avs`/`_lvfx` erhalten: `.avs` und sein
     * `.lvfx`-Zwilling teilen den Basisnamen, und ohne Endung ueberschrieb der
     * Zwilling den Screenshot des Originals (Befund Session 45). Ohne Preset
     * (eigene Kette) `visual`.
     */
    [[nodiscard]] static QString shotBaseName(const QString& presetPath);

    /**
     * @brief Erster freier Name der Reihe `base`, `base_2`, `base_3`, …
     * @param exists Pruefung "gibt es schon" — im Test eine Menge, sonst QFile
     *
     * Ein Preset wird beim Kalibrieren mehrfach aufgenommen; ein stilles
     * Ueberschreiben waere genau das Gegenteil von dem, wofuer der Ordner da ist.
     */
    [[nodiscard]] static QString uniqueBaseName(
        const QString& base, const std::function<bool(const QString&)>& exists);

    // =========================================================================
    // Betrieb
    // =========================================================================

    /// @brief An das Visual haengen, dessen Bilder aufgenommen werden.
    void attach(VisualizerWidget* widget);

    /// @brief Preset merken, dessen Name/Pfad in die Ablage geht (leer = eigene Kette).
    void setCurrentPreset(const QString& absolutePath);
    [[nodiscard]] QString currentPreset() const { return m_presetPath; }

    /// @brief Screenshot anfordern (Hotkey). Das Bild kommt einen Frame spaeter.
    void requestShot();

    /**
     * @brief Ein Problem festhalten statt es anzuzeigen (Vollbild).
     * @param context Woher es kam, z. B. "Import AVS Preset"
     * @param problems Die Meldungszeilen
     *
     * Schreibt nach `fehler.log` und loest zusaetzlich einen Screenshot aus.
     */
    void reportProblem(const QString& context, const QStringList& problems);

    /// @brief Ordner dieses Laufs; legt ihn beim ersten Aufruf an (leer = ging nicht).
    [[nodiscard]] QString sessionDir();

Q_SIGNALS:
    /// @brief Abgelegte Datei (fuer die Statuszeile) bzw. Fehlschlag.
    void shotWritten(const QString& filePath);
    void shotFailed(const QString& reason);

private Q_SLOTS:
    void onFrameCaptured(const QImage& image);

private:
    /// Was zu dem angeforderten Bild gehoert.
    struct Pending
    {
        QString presetPath;  ///< Stand zum Zeitpunkt der Anforderung
    };

    [[nodiscard]] static QString resolveBaseDir();

    QDateTime m_startedAt;
    QString m_sessionDir;   ///< leer, solange nichts angelegt wurde
    QString m_presetPath;
    QQueue<Pending> m_pending;
    VisualizerWidget* m_widget = nullptr;
};
