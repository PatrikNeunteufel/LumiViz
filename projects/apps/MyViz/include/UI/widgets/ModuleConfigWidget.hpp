/**
 ****************************************************************************************
 * @file   ModuleConfigWidget.hpp
 * @brief  Dynamic UI generator for IModule parameters
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 *
 * @details
 * Automatically generates UI controls from ModuleParamDesc. The module defines
 * its parameters, and this widget creates the appropriate controls.
 *
 * ## Architecture
 *
 * ```
 * IModule::paramDescs() → ModuleParamDesc[] → ModuleConfigWidget → UI
 *                                                    ↓
 *                                          IModule::setParam()
 * ```
 *
 * ## Usage
 *
 * ```cpp
 * auto* widget = new ModuleConfigWidget(myModule, parent);
 * // UI is automatically generated from module's paramDescs()
 * // Changes automatically call module->setParam()
 * ```
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/IModule.hpp"

#include <QWidget>
#include <QMap>
#include <functional>

class QVBoxLayout;
class QSlider;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class CollapsibleGroupBox;

namespace lumi::modules { class IModule; }

/**
 * @class ModuleConfigWidget
 * @brief Generates UI controls from IModule parameter descriptors
 */
class ModuleConfigWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct widget for a module
     * @param module The module to configure (not owned)
     * @param parent Parent widget
     */
    explicit ModuleConfigWidget(lumi::modules::IModule* module, QWidget* parent = nullptr);

    /**
     * @brief Rebuild UI (call if module's params change)
     */
    void rebuild();

    /**
     * @brief Sync UI to current module values
     */
    void syncFromModule();

    /**
     * @brief Get the module being configured
     */
    [[nodiscard]] lumi::modules::IModule* module() const { return m_module; }

Q_SIGNALS:
    /**
     * @brief Emitted when any parameter changes
     * @param paramId The parameter ID
     * @param value The new value
     */
    void parameterChanged(const QString& paramId, const lumi::modules::ParamValue& value);

private:
    void buildUI();
    void clearUI();

    // Widget creators for each param type
    QWidget* createBoolWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createIntWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createFloatWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createEnumWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createStringWidget(const lumi::modules::ModuleParamDesc& desc);
    QWidget* createColorWidget(const lumi::modules::ModuleParamDesc& desc);

    // Get or create group box for a group name
    CollapsibleGroupBox* getOrCreateGroup(const QString& groupName);

    // Update module when UI changes
    void onParamChanged(const std::string& paramId, const lumi::modules::ParamValue& value);

    // Check visibility based on dependencies
    void updateVisibility();

    lumi::modules::IModule* m_module = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;

    // Group management
    QMap<QString, CollapsibleGroupBox*> m_groups;

    // Track widgets for dependency updates
    struct ParamWidgetInfo
    {
        QWidget* widget = nullptr;
        QWidget* labelWidget = nullptr;
        lumi::modules::ModuleParamDesc desc;
    };
    QMap<QString, ParamWidgetInfo> m_paramWidgets;

    bool m_isUpdating = false;
};
