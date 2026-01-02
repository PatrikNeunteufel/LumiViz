/**
 ****************************************************************************************
 * @file   ConfigPanel.hpp
 * @brief  Dynamic visualizer configuration panel
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 3.0.0 - Dynamically generates UI from visualizer's paramDescs()
 *
 * @details
 * The ConfigPanel automatically generates UI controls based on the active
 * visualizer's parameter descriptors. Each module's parameters are grouped
 * in collapsible sections.
 *
 * ## Architecture
 *
 * ```
 * IVisualizer::paramDescs() → ModuleParamDesc[] 
 *                                    ↓
 *                            ConfigPanel::rebuildUI()
 *                                    ↓
 *                         CollapsibleGroupBox per Module
 *                                    ↓
 *                          Widget per Parameter
 *                                    ↓
 *                       IVisualizer::setParam()
 * ```
 *
 * ## Parameter Groups
 *
 * Parameters are organized by their `group` field:
 * - "Audio Source" → 🎵 Audio Source (collapsible)
 * - "Color Scheme" → 🎨 Color Scheme (collapsible)
 * - "Pulse Shape"  → ⭕ Pulse Shape (collapsible)
 ****************************************************************************************
 */

#pragma once

#include "UI/panels/PanelBase.hpp"
#include "visualizers/modules/IModule.hpp"

#include <QWidget>
#include <QMap>
#include <vector>
#include <memory>

// Forward declarations
class QScrollArea;
class QVBoxLayout;
class QSlider;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QGroupBox;
class CollapsibleGroupBox;
class IVisualizer;

namespace lumi {
class VisualizerPresetManager;
}

/**
 * @class ConfigPanel
 * @brief Dynamically generates UI from visualizer parameters
 */
class ConfigPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit ConfigPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~ConfigPanel() override;

    // IPanel
    [[nodiscard]] int preferredArea() const override;

    /**
     * @brief Set the visualizer to configure
     * @param visualizer Pointer to active visualizer (not owned)
     */
    void setVisualizer(IVisualizer* visualizer);

    /**
     * @brief Get current visualizer
     */
    [[nodiscard]] IVisualizer* visualizer() const { return m_visualizer; }

    /**
     * @brief Rebuild UI from current visualizer's parameters
     */
    void rebuildUI();

    /**
     * @brief Sync UI values from visualizer
     */
    void syncFromVisualizer();

protected:
    void onActivate() override;
    void onDeactivate() override;

private:
    // UI Building
    void clearUI();
    void buildUIFromParams(const std::vector<lumi::modules::ModuleParamDesc>& params);
    
    // Widget creators for each param type
    QWidget* createBoolWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createIntWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createFloatWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createEnumWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createStringWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createColorWidget(const lumi::modules::ModuleParamDesc& desc);

    // Get or create collapsible group
    CollapsibleGroupBox* getOrCreateGroup(const QString& groupName);

    // Parameter change handler
    void onParamChanged(const std::string& paramId, const lumi::modules::ParamValue& value);

    // Update widget visibility based on dependencies
    void updateVisibility();
    
    // Open gradient editor dialog
    void openGradientEditor(const std::string& paramId);

    // Event subscription
    void subscribeToEvents();
    void unsubscribeFromEvents();
    
    // Preset management
    void setupPresetUI();
    void refreshPresetList();
    void onPresetSelected(int index);
    void onSavePresetClicked();
    void onDeletePresetClicked();
    
    // Module preset save (for Smoothing, Audio, Gradient presets)
    void onModulePresetSave(const std::string& paramId);
    void refreshModulePresetDropdown(const std::string& paramId, 
                                      const std::vector<std::string>& presetNames);

    // --- Members ---
    IVisualizer* m_visualizer = nullptr;
    
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_scrollWidget = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;

    // Group management
    QMap<QString, CollapsibleGroupBox*> m_groups;
    QMap<QString, QGroupBox*> m_subGroups;  // key = "group|subGroup"

    // Widget tracking for sync and dependencies
    struct ParamWidgetInfo
    {
        QWidget* container = nullptr;
        QWidget* control = nullptr;
        QWidget* valueLabel = nullptr;  // Can be QSpinBox, QDoubleSpinBox, or QLabel
        lumi::modules::ModuleParamDesc desc;
    };
    QMap<QString, ParamWidgetInfo> m_paramWidgets;
    
    // Preset UI
    QComboBox* m_presetCombo = nullptr;
    QPushButton* m_savePresetBtn = nullptr;
    QPushButton* m_deletePresetBtn = nullptr;
    std::unique_ptr<lumi::VisualizerPresetManager> m_presetManager;

    // State
    bool m_isUpdating = false;
    std::vector<int> m_subscriptionIds;
};
