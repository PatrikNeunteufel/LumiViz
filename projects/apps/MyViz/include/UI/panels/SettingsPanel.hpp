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

    // Hotkeys Tab
    QLabel* m_pHotkeyStatusLabel = nullptr;

    // State
    bool m_isUpdating = false;
};
