/**
 ****************************************************************************************
 * @file   VisualSelectPanel.hpp
 * @brief  Visualization selection panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

class QListWidget;
class QStackedWidget;
class QLabel;

/**
 * @class VisualSelectPanel
 * @brief Panel for selecting and configuring visualizations
 *
 * Provides:
 *   - List of available visualizers (Spectrum, Waveform, etc.)
 *   - Preview thumbnails
 *   - Per-visualizer settings
 *
 * Note: Visualizers are NOT panels - they render in the main VisualizerWidget.
 * This panel just controls which visualizer is active.
 */
class VisualSelectPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit VisualSelectPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~VisualSelectPanel() override = default;

    [[nodiscard]] int preferredArea() const override;

Q_SIGNALS:
    /**
     * @brief Emitted when user selects a different visualizer
     * @param visualizerId ID of selected visualizer
     */
    void visualizerSelected(const QString& visualizerId);

protected:
    void onActivate() override;

private Q_SLOTS:
    void onSelectionChanged();

private:
    void setupUI();
    void setupConnections();
    void populateVisualizers();

    // UI Elements
    QListWidget* m_pVisualizerList = nullptr;
    QLabel* m_pPreviewLabel = nullptr;
    QLabel* m_pDescriptionLabel = nullptr;
    QStackedWidget* m_pSettingsStack = nullptr;
};
