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
class QCheckBox;
class QPushButton;

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
    void onTargetFpsChanged(int value);
    void onVSyncChanged(bool checked);
    void onResetImportBrowserDir();

private:
    void setupUI();
    void setupConnections();
    void subscribeToEvents();
    void unsubscribeFromEvents();
    void populateAudioDevices();

    QWidget* createAudioTab();
    QWidget* createPerformanceTab();
    QWidget* createPanelsTab();

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

    // Panels Tab
    QPushButton* m_pResetImportDirButton = nullptr;
    /// AVS-Import: Render-Scale-Divisor des automatisch eingefuegten Knotens
    /// (QSettings "import/avsRenderScaleDivisor", 1 = neutral — S47)
    QSpinBox* m_pAvsRenderScaleSpinBox = nullptr;

    // State
    bool m_isUpdating = false;
};
