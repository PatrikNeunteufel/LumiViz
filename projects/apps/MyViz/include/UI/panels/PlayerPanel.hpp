/**
 ****************************************************************************************
 * @file   PlayerPanel.hpp
 * @brief  Audio player controls panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

class QLabel;
class QPushButton;
class QSlider;

/**
 * @class PlayerPanel
 * @brief Panel for audio playback controls
 *
 * Provides:
 *   - Play/Pause/Stop controls
 *   - Volume control
 *   - Progress display
 *   - Current track info
 */
class PlayerPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit PlayerPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~PlayerPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

protected:
    void onActivate() override;
    void onDeactivate() override;

private:
    void setupUI();
    void setupConnections();

    // UI Elements
    QLabel* m_pTrackLabel = nullptr;
    QLabel* m_pTimeLabel = nullptr;
    QPushButton* m_pPlayButton = nullptr;
    QPushButton* m_pStopButton = nullptr;
    QPushButton* m_pPrevButton = nullptr;
    QPushButton* m_pNextButton = nullptr;
    QSlider* m_pVolumeSlider = nullptr;
    QSlider* m_pProgressSlider = nullptr;
};
