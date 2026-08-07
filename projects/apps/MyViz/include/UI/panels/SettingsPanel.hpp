/**
 ****************************************************************************************
 * @file   SettingsPanel.hpp
 * @brief  Application settings panel (Audio, Performance)
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## SettingsPanel
 *
 * Provides global application settings:
 * - Audio device selection and configuration
 * - Performance settings (frame mode, VSync)
 *
 * ## Tabs
 *
 * 1. **Audio** - Device, buffer size, sample rate
 * 2. **Performance** - Frame mode, target FPS, VSync
 * 3. **Panels** - panel housekeeping (reset Import Browser start folder)
 * 4. **Editor** - Beautify-Format der Skript-Editoren (QSettings-Block
 *    "editor/...", gelesen von EelScriptEditing::formatOptionsFromSettings, S69)
 * 5. **Kamera** - Testaufnahmen (S70): kurze Kamera-Clips benutzerlokal
 *    aufnehmen — deterministische Kamera-Stellvertreter des videoSource-
 *    Knotens; die Aufnahme ist zugleich die Kamera-Freigabe des App-Laufs
 * 6. **Hotkeys** - Hotkey-Editor
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <vector>

class QTabWidget;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QListWidget;
class QCamera;
class QMediaCaptureSession;
class QMediaRecorder;

/**
 * @class SettingsPanel
 * @brief Panel for global application settings
 */
class SettingsPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit SettingsPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~SettingsPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

protected:
    void onActivate() override;
    void onDeactivate() override;

private Q_SLOTS:
    void onAudioDeviceChanged(int index);
    void onFrameModeChanged(int index);
    /// Schreibt die Windows-GPU-Praeferenz (core/GpuPreference) und startet die
    /// App SOFORT neu — der Eintrag greift erst beim naechsten Prozessstart.
    void onGpuPreferenceChanged(int index);
    void onTargetFpsChanged(int value);
    void onVSyncChanged(bool checked);
    void onResetImportBrowserDir();
    /// Oeffnet den Benutzerdaten-Ordner (AppDataLocation) im Dateimanager.
    void onOpenAppDataDir();
    /// Waehlt den Bilder-Suchordner (`import/imageSearchDir`, S50-Vorgabe).
    void onChooseImageSearchDir();
    /// Beschriftet den Suchordner-Knopf mit dem aktuellen Pfad.
    void updateImageSearchDirButton();

private:
    void setupUI();
    void setupConnections();
    void subscribeToEvents();
    void unsubscribeFromEvents();
    void populateAudioDevices();

    QWidget* createAudioTab();
    QWidget* createPerformanceTab();
    QWidget* createPanelsTab();
    /// Beautify-Format der Skript-Editoren (Offene_Punkte §7, S69):
    /// Einzugsbreite, Operator-Abstaende, Leerzeilen-Klemme ("editor/*").
    QWidget* createEditorTab();
    /// Kamera-Testaufnahmen (S70): Geraetewahl + Aufnahme in den
    /// benutzerlokalen Ordner (LiveVideoFeed::testaufnahmenOrdner) + Liste.
    QWidget* createKameraTab();
    /// Fuellt die Kamera-Geraeteliste neu (das Aufzaehlen oeffnet KEINE Kamera).
    void aktualisiereKamGeraete();
    /// Fuellt die Liste der vorhandenen Testaufnahmen neu.
    void aktualisiereTestaufnahmenListe();
    /// Startet die Testaufnahme — DIE ausdrueckliche Nutzeraktion, die auch
    /// die Kamera-Freigabe dieses App-Laufs erteilt (LiveVideoFeed).
    void starteTestaufnahme();
    /**
     * @brief Beendet die Aufnahme, raeumt die Kamera-Objekte ab, frischt die Liste
     * @param synchron Objekte SOFORT zerstoeren statt per deleteLater
     *
     * Der Normalweg (`synchron = false`) muss deleteLater nutzen: der Aufruf
     * kommt aus einem Signal des Recorders, ein direktes delete waere ein
     * delete des Senders mitten in der Emission. Beim Herunterfahren gibt es
     * aber keine Event-Loop mehr, die deleteLater ausfuehrt — die Objekte
     * stuerben sonst erst in der Event-Queue-Entsorgung des ~QApplication
     * bzw. per Parent-Destruktor mitten im Fenster-Abbau (Zombie-Klasse,
     * Befund S71). Der aboutToQuit-Haken raeumt deshalb `synchron = true`.
     */
    void beendeTestaufnahme(bool synchron = false);
    /// Hotkey-Editor (docs/ui/Hotkey_Konzept.md §6): Tabelle je Kategorie mit
    /// Aufnahmefeld, Kollisions-/Reservierungspruefung und Zuruecksetzen.
    QWidget* createHotkeyTab();

    // UI Elements
    QTabWidget* m_pTabWidget = nullptr;

    // Audio Tab
    QComboBox* m_pAudioDeviceCombo = nullptr;
    QSpinBox* m_pBufferSizeSpinBox = nullptr;
    QSpinBox* m_pSampleRateSpinBox = nullptr;

    // Performance Tab
    QComboBox* m_pFrameModeCombo = nullptr;
    QSpinBox* m_pTargetFpsSpinBox = nullptr;
    QCheckBox* m_pVSyncCheckBox = nullptr;
    /// GPU-Praeferenz (Automatisch/Energiesparen/Hohe Leistung) — Wert lebt in
    /// der Windows-Registrierung (SSOT), Aenderung loest den Neustart aus.
    QComboBox* m_pGpuPreferenceCombo = nullptr;
    QLabel* m_pActiveGpuLabel = nullptr;

    // Panels Tab
    QPushButton* m_pResetImportDirButton = nullptr;
    QPushButton* m_pOpenAppDataButton = nullptr;
    QPushButton* m_pImageSearchDirButton = nullptr;
    /// AVS-Import: Render-Scale-Divisor des automatisch eingefuegten Knotens
    /// (QSettings "import/avsRenderScaleDivisor", 1 = neutral — S47)
    QSpinBox* m_pAvsRenderScaleSpinBox = nullptr;
    /// MilkDrop: App-Default fuer den Puffer-Wechsel beim Preset-Tausch (S66;
    /// QSettings "milkdrop/pufferWechsel" + "milkdrop/pufferFadingProzent" —
    /// gilt fuer Nodes mit Einstellung "App-Einstellung")
    QComboBox* m_pMilkPufferWechselCombo = nullptr;
    QSpinBox* m_pMilkPufferFadingSpinBox = nullptr;
    QDoubleSpinBox* m_pMilkPufferAusblendSpinBox = nullptr;
    QCheckBox* m_pMilkSichtBlendeCheckBox = nullptr;

    // Editor Tab (Beautify-Format, S69)
    QSpinBox* m_pEditorIndentSpinBox = nullptr;
    QCheckBox* m_pEditorOpSpacesCheckBox = nullptr;
    QSpinBox* m_pEditorMaxBlankSpinBox = nullptr;

    // Kamera Tab (Testaufnahmen, S70)
    QComboBox* m_pKamGeraetCombo = nullptr;
    QSpinBox* m_pKamDauerSpin = nullptr;      ///< Aufnahmedauer in Sekunden
    QPushButton* m_pKamAufnahmeButton = nullptr;
    QLabel* m_pKamStatusLabel = nullptr;
    QListWidget* m_pKamListe = nullptr;       ///< vorhandene Testaufnahmen
    /// Aufnahme-Objekte — nur waehrend einer laufenden Aufnahme belegt
    /// (QObject-Kinder des Panels; deleteLater beim Beenden).
    QCamera* m_pKamera = nullptr;
    QMediaCaptureSession* m_pKamSession = nullptr;
    QMediaRecorder* m_pKamRecorder = nullptr;

    // Hotkeys Tab
    QLabel* m_pHotkeyStatusLabel = nullptr;

    // State
    bool m_isUpdating = false;
};
