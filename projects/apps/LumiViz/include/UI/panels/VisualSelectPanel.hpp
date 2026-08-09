/**
 ****************************************************************************************
 * @file   VisualSelectPanel.hpp
 * @brief  Visualization selection panel
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 *
 * @details
 * ## VisualSelectPanel
 *
 * Provides visualization selection:
 * - List of available visualizers from VisualizerRegistry
 * - Category grouping
 * - Preview and description
 * - Apply to active VisualizerWidget
 *
 * ## Integration
 *
 * ```
 * ┌────────────────────┐
 * │ VisualizerRegistry │
 * └─────────┬──────────┘
 *           │ descriptors()
 *           ▼
 * ┌─────────────────────┐     apply      ┌──────────────────┐
 * │  VisualSelectPanel  │ ──────────────►│ VisualizerWidget │
 * └─────────────────────┘                 └──────────────────┘
 * ```
 ****************************************************************************************
 */

#pragma once

#include "PanelBase.hpp"

#include <vector>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QLabel;
class QPushButton;

/**
 * @class VisualSelectPanel
 * @brief Panel for selecting and configuring visualizations
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
    void onApplyClicked();
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void setupUI();
    void setupConnections();
    void populateVisualizers();
    void applySelectedVisualizer();

    // UI Elements
    QListWidget* m_pVisualizerList = nullptr;
    QLabel* m_pPreviewLabel = nullptr;
    QLabel* m_pDescriptionLabel = nullptr;
    QLabel* m_pCategoryLabel = nullptr;
    QPushButton* m_pApplyButton = nullptr;
    QStackedWidget* m_pSettingsStack = nullptr;
    
    // State
    QString m_selectedVisualizerId;
};
