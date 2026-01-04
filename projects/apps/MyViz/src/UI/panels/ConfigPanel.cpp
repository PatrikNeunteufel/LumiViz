/**
 ****************************************************************************************
 * @file   ConfigPanel.cpp
 * @brief  Dynamic ConfigPanel implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 3.0.0
 ****************************************************************************************
 */

#include "UI/panels/ConfigPanel.hpp"
#include "UI/widgets/CollapsibleGroupBox.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "UI/widgets/GradientPresetDelegate.hpp"
#include "UI/dialogs/GradientEditorDialog.hpp"
#include "visualizers/IVisualizer.hpp"
#include "visualizers/PulsingVisualizer.hpp"
#include "visualizers/WaveformVisualizer.hpp"
#include "visualizers/OscilloscopeVisualizer.hpp"
#include "visualizers/SuperscopeVisualizer.hpp"
#include "visualizers/VisualizerPresetManager.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <set>
#include <map>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QSignalBlocker>
#include <QStandardItemModel>

#include <BasicLogger.h>

#include <algorithm>

using namespace lumi::modules;

// =============================================================================
// Group Icons
// =============================================================================

static QString groupIcon(const QString& groupName)
{
    if (groupName.contains("Audio", Qt::CaseInsensitive))
        return QStringLiteral("🎵 ");
    if (groupName.contains("Color", Qt::CaseInsensitive))
        return QStringLiteral("🎨 ");
    if (groupName.contains("Shape", Qt::CaseInsensitive) ||
        groupName.contains("Pulse", Qt::CaseInsensitive))
        return QStringLiteral("⭕ ");
    if (groupName.contains("Smooth", Qt::CaseInsensitive))
        return QStringLiteral("〰️ ");
    if (groupName.contains("Effect", Qt::CaseInsensitive))
        return QStringLiteral("✨ ");
    return QString();
}

// =============================================================================
// Construction
// =============================================================================

ConfigPanel::ConfigPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, QStringLiteral("config"), tr("Visualizer Config"), parent)
    , m_presetManager(std::make_unique<lumi::VisualizerPresetManager>())
{
    // Scroll area for many controls
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_scrollWidget = new QWidget(m_scrollArea);
    m_contentLayout = new QVBoxLayout(m_scrollWidget);
    m_contentLayout->setContentsMargins(4, 4, 4, 4);
    m_contentLayout->setSpacing(8);

    m_scrollArea->setWidget(m_scrollWidget);

    // Main layout
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Preset controls at top
    setupPresetUI();
    mainLayout->addWidget(m_scrollArea);

    // Placeholder
    auto* placeholder = new QLabel(tr("No visualizer selected"), m_scrollWidget);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color: gray;");
    m_contentLayout->addWidget(placeholder);
    m_contentLayout->addStretch();
}

ConfigPanel::~ConfigPanel() = default;

int ConfigPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

void ConfigPanel::onActivate()
{
    subscribeToEvents();
    
    // Try to find VisualizerWidget through Qt widget hierarchy
    QWidget* mainWindow = window();
    if (mainWindow != nullptr)
    {
        auto* vizWidget = mainWindow->findChild<VisualizerWidget*>();
        if (vizWidget != nullptr && vizWidget->hasVisualizer())
        {
            setVisualizer(vizWidget->visualizer());
        }
    }
}

void ConfigPanel::onDeactivate()
{
    unsubscribeFromEvents();
}

// =============================================================================
// Visualizer Management
// =============================================================================

void ConfigPanel::setVisualizer(IVisualizer* visualizer)
{
    if (m_visualizer == visualizer)
    {
        return;
    }

    m_visualizer = visualizer;
    rebuildUI();
}

void ConfigPanel::rebuildUI()
{
    clearUI();

    if (!m_visualizer)
    {
        auto* placeholder = new QLabel(tr("No visualizer selected"), m_scrollWidget);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setStyleSheet("color: gray;");
        m_contentLayout->addWidget(placeholder);
        m_contentLayout->addStretch();
        return;
    }

    if (!m_visualizer->hasParameterSupport())
    {
        auto* placeholder = new QLabel(
            tr("Visualizer '%1' does not support configuration")
                .arg(m_visualizer->visualizerName()),
            m_scrollWidget);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setStyleSheet("color: gray;");
        placeholder->setWordWrap(true);
        m_contentLayout->addWidget(placeholder);
        m_contentLayout->addStretch();
        return;
    }

    // Build UI from parameters
    auto params = m_visualizer->paramDescs();
    
    BasicLogger::logInfo("ConfigPanel: Building UI for " + 
                         m_visualizer->visualizerName().toStdString() +
                         " with " + std::to_string(params.size()) + " parameters");

    buildUIFromParams(params);
    
    // Refresh preset list for this visualizer
    refreshPresetList();
}

void ConfigPanel::syncFromVisualizer()
{
    if (!m_visualizer || m_isUpdating)
    {
        return;
    }

    m_isUpdating = true;

    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        ParamValue value;
        if (!m_visualizer->getParam(it.key().toStdString(), value))
        {
            continue;
        }

        const auto& desc = it->desc;

        // Update widget based on type
        if (auto* checkbox = qobject_cast<QCheckBox*>(it->control))
        {
            if (auto* b = std::get_if<bool>(&value))
            {
                checkbox->setChecked(*b);
            }
        }
        else if (auto* spinbox = qobject_cast<QSpinBox*>(it->control))
        {
            if (auto* i = std::get_if<int>(&value))
            {
                spinbox->setValue(*i);
            }
        }
        else if (auto* slider = qobject_cast<QSlider*>(it->control))
        {
            if (desc.type == ParamType::Float)
            {
                if (auto* f = std::get_if<float>(&value))
                {
                    float range = desc.maxValue - desc.minValue;
                    int sliderVal = static_cast<int>((*f - desc.minValue) / range * 1000);
                    slider->setValue(sliderVal);
                    
                    // Update value spinbox if present
                    if (auto* dspinbox = qobject_cast<QDoubleSpinBox*>(it->valueLabel))
                    {
                        QSignalBlocker blocker(dspinbox);
                        dspinbox->setValue(*f);
                    }
                }
            }
            else if (auto* i = std::get_if<int>(&value))
            {
                slider->setValue(*i);
                
                // Update value spinbox if present
                if (auto* ispinbox = qobject_cast<QSpinBox*>(it->valueLabel))
                {
                    QSignalBlocker blocker(ispinbox);
                    ispinbox->setValue(*i);
                }
            }
        }
        else if (auto* combo = qobject_cast<QComboBox*>(it->control))
        {
            if (auto* i = std::get_if<int>(&value))
            {
                combo->setCurrentIndex(*i);
            }
        }
        else if (auto* lineEdit = qobject_cast<QLineEdit*>(it->control))
        {
            if (auto* s = std::get_if<std::string>(&value))
            {
                lineEdit->setText(QString::fromStdString(*s));
            }
        }
        else if (auto* colorBtn = qobject_cast<QPushButton*>(it->control))
        {
            // Color button - update background color
            if (desc.type == ParamType::Color && value.index() == 7)
            {
                const auto& c = std::get<7>(value);  // Color4f
                QColor qc = QColor::fromRgbF(c[0], c[1], c[2], c[3]);
                colorBtn->setStyleSheet(
                    QString("background-color: %1; border: 1px solid gray;").arg(qc.name()));
            }
        }
    }

    m_isUpdating = false;
    updateVisibility();
}

// =============================================================================
// Event Subscription
// =============================================================================

void ConfigPanel::subscribeToEvents()
{
    auto* bus = services().tryResolve<IEventBus>();
    if (!bus)
    {
        return;
    }

    m_subscriptionIds.push_back(
        bus->subscribe<VisualizerChangedEvent>(
            [this](const VisualizerChangedEvent& evt) {
                BasicLogger::logDebug("ConfigPanel: Visualizer changed to " + evt.visualizerName);
                if (evt.visualizerPtr != nullptr)
                {
                    auto* viz = static_cast<IVisualizer*>(evt.visualizerPtr);
                    setVisualizer(viz);
                }
            }
        )
    );
}

void ConfigPanel::unsubscribeFromEvents()
{
    auto* bus = services().tryResolve<IEventBus>();
    if (!bus)
    {
        return;
    }

    for (int id : m_subscriptionIds)
    {
        bus->unsubscribe(id);
    }
    m_subscriptionIds.clear();
}

// =============================================================================
// UI Building
// =============================================================================

void ConfigPanel::clearUI()
{
    // Remove all widgets from layout
    QLayoutItem* item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr)
    {
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }

    m_groups.clear();
    m_subGroups.clear();
    m_paramWidgets.clear();
}

void ConfigPanel::buildUIFromParams(const std::vector<ModuleParamDesc>& params)
{
    // Calculate minimum order for each subGroup (for proper sorting)
    std::map<std::string, int> subGroupMinOrder;
    for (const auto& p : params)
    {
        std::string key = p.group + "|" + p.subGroup;
        if (subGroupMinOrder.find(key) == subGroupMinOrder.end())
        {
            subGroupMinOrder[key] = p.order;
        }
        else
        {
            subGroupMinOrder[key] = std::min(subGroupMinOrder[key], p.order);
        }
    }

    // Sort by group, then by subGroup's minimum order, then by individual order
    auto sortedParams = params;
    std::sort(sortedParams.begin(), sortedParams.end(),
              [&subGroupMinOrder](const ModuleParamDesc& a, const ModuleParamDesc& b) {
                  if (a.group != b.group) return a.group < b.group;
                  
                  std::string keyA = a.group + "|" + a.subGroup;
                  std::string keyB = b.group + "|" + b.subGroup;
                  int minOrderA = subGroupMinOrder[keyA];
                  int minOrderB = subGroupMinOrder[keyB];
                  
                  if (minOrderA != minOrderB) return minOrderA < minOrderB;
                  return a.order < b.order;
              });

    // Build widgets
    for (const auto& desc : sortedParams)
    {
        // Skip advanced parameters for now (could add toggle)
        if (desc.advanced)
        {
            continue;
        }
        
        // Skip hidden parameters (used for internal serialization)
        if (desc.hidden)
        {
            continue;
        }

        QWidget* widget = nullptr;

        switch (desc.type)
        {
        case ParamType::Bool:
            widget = createBoolWidget(desc);
            break;
        case ParamType::Int:
            widget = createIntWidget(desc);
            break;
        case ParamType::Float:
            widget = createFloatWidget(desc);
            break;
        case ParamType::Enum:
            widget = createEnumWidget(desc);
            break;
        case ParamType::String:
            widget = createStringWidget(desc);
            break;
        case ParamType::Color:
            widget = createColorWidget(desc);
            break;
        default:
            BasicLogger::logWarning("ConfigPanel: Unsupported param type for " + desc.id);
            continue;
        }

        if (widget)
        {
            QString groupName = QString::fromStdString(desc.group);
            if (groupName.isEmpty())
            {
                groupName = tr("General");
            }

            auto* group = getOrCreateGroup(groupName);
            
            // Check for subGroup - add to framed container
            if (!desc.subGroup.empty())
            {
                QString subGroupKey = groupName + "|" + QString::fromStdString(desc.subGroup);
                QGroupBox* subGroup = nullptr;
                
                if (m_subGroups.contains(subGroupKey))
                {
                    subGroup = m_subGroups[subGroupKey];
                }
                else
                {
                    // Create new subGroup with frame
                    subGroup = new QGroupBox(QString::fromStdString(desc.subGroup), m_scrollWidget);
                    subGroup->setStyleSheet(
                        "QGroupBox { "
                        "  border: 1px solid #555; "
                        "  border-radius: 4px; "
                        "  margin-top: 8px; "
                        "  padding-top: 4px; "
                        "} "
                        "QGroupBox::title { "
                        "  subcontrol-origin: margin; "
                        "  left: 8px; "
                        "  padding: 0 4px; "
                        "}"
                    );
                    auto* subLayout = new QVBoxLayout(subGroup);
                    subLayout->setContentsMargins(8, 12, 8, 8);
                    subLayout->setSpacing(4);
                    
                    m_subGroups[subGroupKey] = subGroup;
                    group->addWidget(subGroup);
                }
                
                subGroup->layout()->addWidget(widget);
            }
            else
            {
                group->addWidget(widget);
            }
        }
    }

    // Add stretch and info label
    m_contentLayout->addStretch();

    auto* infoLabel = new QLabel(
        tr("Visualizer: %1").arg(m_visualizer->visualizerName()),
        m_scrollWidget);
    infoLabel->setStyleSheet("color: gray; font-size: 10px;");
    infoLabel->setAlignment(Qt::AlignCenter);
    m_contentLayout->addWidget(infoLabel);

    // Initial sync and visibility
    syncFromVisualizer();
}

CollapsibleGroupBox* ConfigPanel::getOrCreateGroup(const QString& groupName)
{
    if (m_groups.contains(groupName))
    {
        return m_groups[groupName];
    }

    QString displayName = groupIcon(groupName) + groupName;
    auto* group = new CollapsibleGroupBox(displayName, m_scrollWidget);
    m_groups[groupName] = group;
    m_contentLayout->addWidget(group);

    return group;
}

// =============================================================================
// Widget Creators
// =============================================================================

QWidget* ConfigPanel::createBoolWidget(const ModuleParamDesc& desc)
{
    auto* checkbox = new QCheckBox(QString::fromStdString(desc.displayName), m_scrollWidget);
    checkbox->setToolTip(QString::fromStdString(desc.tooltip));

    // Get current value
    ParamValue value;
    if (m_visualizer->getParam(desc.id, value))
    {
        if (auto* b = std::get_if<bool>(&value))
        {
            checkbox->setChecked(*b);
        }
    }

    connect(checkbox, &QCheckBox::toggled,
            [this, id = desc.id](bool checked) {
                onParamChanged(id, checked);
            });

    // Track widget
    ParamWidgetInfo info;
    info.container = checkbox;
    info.control = checkbox;
    info.desc = desc;
    m_paramWidgets[QString::fromStdString(desc.id)] = info;

    return checkbox;
}

QWidget* ConfigPanel::createIntWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(m_scrollWidget);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* label = new QLabel(QString::fromStdString(desc.displayName), container);
    label->setMinimumWidth(100);
    layout->addWidget(label);

    QWidget* control = nullptr;

    // Use spinbox for large ranges, slider+spinbox otherwise
    int range = static_cast<int>(desc.maxValue - desc.minValue);
    
    if (desc.widget == ParamWidget::Spinbox || range > 100)
    {
        auto* spinbox = new QSpinBox(container);
        spinbox->setRange(static_cast<int>(desc.minValue), static_cast<int>(desc.maxValue));
        spinbox->setSingleStep(desc.step > 0 ? static_cast<int>(desc.step) : 1);
        spinbox->setToolTip(QString::fromStdString(desc.tooltip));
        spinbox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        spinbox->setAccelerated(true);  // Accelerate when holding buttons
        spinbox->setKeyboardTracking(false);
        spinbox->setMinimumWidth(100);
        spinbox->setMaximumWidth(150);
        
        // Ensure buttons are visible
        spinbox->setStyleSheet(
            "QSpinBox { padding-right: 20px; }"
            "QSpinBox::up-button { width: 16px; }"
            "QSpinBox::down-button { width: 16px; }"
        );

        connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                [this, id = desc.id](int value) {
                    onParamChanged(id, value);
                });

        control = spinbox;
        layout->addWidget(spinbox, 1);
    }
    else
    {
        // Slider + SpinBox combination for smaller ranges
        auto* slider = new QSlider(Qt::Horizontal, container);
        slider->setRange(static_cast<int>(desc.minValue), static_cast<int>(desc.maxValue));
        slider->setToolTip(QString::fromStdString(desc.tooltip));

        auto* spinbox = new QSpinBox(container);
        spinbox->setRange(static_cast<int>(desc.minValue), static_cast<int>(desc.maxValue));
        spinbox->setSingleStep(desc.step > 0 ? static_cast<int>(desc.step) : 1);
        spinbox->setMinimumWidth(50);
        spinbox->setMaximumWidth(70);
        if (!desc.unit.empty())
        {
            spinbox->setSuffix(QString::fromStdString(" " + desc.unit));
        }

        // Slider -> SpinBox + param change
        connect(slider, &QSlider::valueChanged,
                [this, id = desc.id, spinbox](int value) {
                    QSignalBlocker blocker(spinbox);
                    spinbox->setValue(value);
                    onParamChanged(id, value);
                });

        // SpinBox -> Slider + param change
        connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                [this, id = desc.id, slider](int value) {
                    QSignalBlocker blocker(slider);
                    slider->setValue(value);
                    onParamChanged(id, value);
                });

        control = slider;
        layout->addWidget(slider, 1);
        layout->addWidget(spinbox);

        // Track widgets
        ParamWidgetInfo info;
        info.container = container;
        info.control = slider;
        info.valueLabel = spinbox;
        info.desc = desc;
        m_paramWidgets[QString::fromStdString(desc.id)] = info;

        return container;
    }

    // Track widget
    ParamWidgetInfo info;
    info.container = container;
    info.control = control;
    info.desc = desc;
    m_paramWidgets[QString::fromStdString(desc.id)] = info;

    return container;
}

QWidget* ConfigPanel::createFloatWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(m_scrollWidget);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* label = new QLabel(QString::fromStdString(desc.displayName), container);
    label->setMinimumWidth(100);
    layout->addWidget(label);

    // Use slider with 1000 steps for smooth control
    auto* slider = new QSlider(Qt::Horizontal, container);
    slider->setRange(0, 1000);
    slider->setToolTip(QString::fromStdString(desc.tooltip));

    // Editable spinbox for direct value input
    auto* spinbox = new QDoubleSpinBox(container);
    spinbox->setRange(desc.minValue, desc.maxValue);
    spinbox->setDecimals(2);
    spinbox->setSingleStep((desc.maxValue - desc.minValue) / 100.0);
    spinbox->setMinimumWidth(70);
    spinbox->setMaximumWidth(90);
    if (!desc.unit.empty())
    {
        spinbox->setSuffix(QString::fromStdString(" " + desc.unit));
    }

    float range = desc.maxValue - desc.minValue;
    float minVal = desc.minValue;

    // Slider -> SpinBox + param change
    connect(slider, &QSlider::valueChanged,
            [this, id = desc.id, spinbox, minVal, range](int sliderVal) {
                float value = minVal + (sliderVal / 1000.0f) * range;
                
                // Block signals to prevent recursion
                QSignalBlocker blocker(spinbox);
                spinbox->setValue(value);
                
                onParamChanged(id, value);
            });

    // SpinBox -> Slider + param change
    connect(spinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this, id = desc.id, slider, minVal, range](double value) {
                int sliderVal = static_cast<int>(((value - minVal) / range) * 1000.0);
                sliderVal = std::clamp(sliderVal, 0, 1000);
                
                // Block signals to prevent recursion
                QSignalBlocker blocker(slider);
                slider->setValue(sliderVal);
                
                onParamChanged(id, static_cast<float>(value));
            });

    layout->addWidget(slider, 1);
    layout->addWidget(spinbox);

    // Track widget
    ParamWidgetInfo info;
    info.container = container;
    info.control = slider;
    info.valueLabel = spinbox;  // Now a spinbox, not a label
    info.desc = desc;
    m_paramWidgets[QString::fromStdString(desc.id)] = info;

    return container;
}

QWidget* ConfigPanel::createEnumWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(m_scrollWidget);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* label = new QLabel(QString::fromStdString(desc.displayName), container);
    label->setMinimumWidth(100);
    layout->addWidget(label);

    auto* combo = new QComboBox(container);
    for (size_t i = 0; i < desc.enumOptions.size(); ++i)
    {
        const auto& option = desc.enumOptions[i];
        combo->addItem(QString::fromStdString(option));
        
        // Disable separator items ("---")
        if (option == "---")
        {
            auto* model = qobject_cast<QStandardItemModel*>(combo->model());
            if (model)
            {
                auto* item = model->item(static_cast<int>(i));
                if (item)
                {
                    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
                }
            }
        }
    }
    combo->setToolTip(QString::fromStdString(desc.tooltip));
    
    // Check if this is a gradient preset dropdown - add preview delegate
    if (desc.id.find("preset") != std::string::npos && 
        (desc.id.find("color") != std::string::npos || desc.id.find("Color") != std::string::npos ||
         desc.id.find("lineColor") != std::string::npos))
    {
        // Try to get the ColorGradientModule for preview from any visualizer
        lumi::modules::ColorGradientModule* gradientModule = nullptr;
        
        // PulsingVisualizer
        if (auto* pulsing = dynamic_cast<PulsingVisualizer*>(m_visualizer))
        {
            gradientModule = pulsing->colorGradient();
        }
        // WaveformVisualizer
        else if (auto* waveform = dynamic_cast<WaveformVisualizer*>(m_visualizer))
        {
            gradientModule = &waveform->waveform()->colorGradient();
        }
        // OscilloscopeVisualizer - extract channel from parameter ID
        // Parameter IDs are like "ch1Color.preset", "ch2Color.preset", etc.
        // or "m1Color.preset", "m2Color.preset" for math channels
        else if (auto* oscilloscope = dynamic_cast<OscilloscopeVisualizer*>(m_visualizer))
        {
            int channelIndex = 0;  // Default to CH1
            
            if (desc.id.find("ch1Color.") != std::string::npos) channelIndex = 0;
            else if (desc.id.find("ch2Color.") != std::string::npos) channelIndex = 1;
            else if (desc.id.find("ch3Color.") != std::string::npos) channelIndex = 2;
            else if (desc.id.find("ch4Color.") != std::string::npos) channelIndex = 3;
            else if (desc.id.find("m1Color.") != std::string::npos) channelIndex = 4;
            else if (desc.id.find("m2Color.") != std::string::npos) channelIndex = 5;
            
            gradientModule = &oscilloscope->oscilloscope()->colorGradient(channelIndex);
        }
        // SuperscopeVisualizer - color gradient is in superscope module
        else if (auto* superscope = dynamic_cast<SuperscopeVisualizer*>(m_visualizer))
        {
            gradientModule = &superscope->superscope()->colorGradient();
        }
        
        if (gradientModule)
        {
            auto* delegate = new lumi::ui::GradientPresetDelegate(combo);
            delegate->setGradientModule(gradientModule);
            combo->setItemDelegate(delegate);
            
            // Make combo taller to show preview
            combo->setMinimumHeight(28);
        }
    }

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, id = desc.id](int index) {
                onParamChanged(id, index);
            });

    layout->addWidget(combo, 1);
    
    // Add Save button for module preset dropdowns (Smoothing, Audio, Gradient)
    bool isModulePreset = desc.id.find("preset") != std::string::npos &&
                          (desc.id.find("smooth") != std::string::npos ||
                           desc.id.find("audio") != std::string::npos ||
                           desc.id.find("color") != std::string::npos);
    
    if (isModulePreset)
    {
        auto* saveBtn = new QPushButton(tr("Save"), container);
        saveBtn->setFixedWidth(50);
        saveBtn->setToolTip(tr("Save current settings as a new preset"));
        
        connect(saveBtn, &QPushButton::clicked,
                [this, id = desc.id]() {
                    onModulePresetSave(id);
                });
        
        layout->addWidget(saveBtn);
    }

    // Track widget
    ParamWidgetInfo info;
    info.container = container;
    info.control = combo;
    info.desc = desc;
    m_paramWidgets[QString::fromStdString(desc.id)] = info;

    return container;
}

QWidget* ConfigPanel::createStringWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(m_scrollWidget);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // Check if this should be a button (displayName ends with "...")
    bool isButton = desc.displayName.find("...") != std::string::npos;
    
    if (isButton)
    {
        // Create a button instead of a text field
        auto* button = new QPushButton(QString::fromStdString(desc.displayName), container);
        button->setToolTip(QString::fromStdString(desc.tooltip));
        
        // Special handling for gradient editor
        if (desc.id.find("editGradient") != std::string::npos)
        {
            connect(button, &QPushButton::clicked,
                    [this, id = desc.id]() {
                        openGradientEditor(id);
                    });
        }
        else
        {
            // Generic button click - just notify param changed
            connect(button, &QPushButton::clicked,
                    [this, id = desc.id]() {
                        onParamChanged(id, std::string("clicked"));
                    });
        }
        
        layout->addWidget(button);
        layout->addStretch();
        
        // Track widget
        ParamWidgetInfo info;
        info.container = container;
        info.control = button;
        info.desc = desc;
        m_paramWidgets[QString::fromStdString(desc.id)] = info;
    }
    else
    {
        // Standard text field
        auto* label = new QLabel(QString::fromStdString(desc.displayName), container);
        label->setMinimumWidth(100);
        layout->addWidget(label);

        auto* lineEdit = new QLineEdit(container);
        lineEdit->setToolTip(QString::fromStdString(desc.tooltip));

        connect(lineEdit, &QLineEdit::textChanged,
                [this, id = desc.id](const QString& text) {
                    onParamChanged(id, text.toStdString());
                });

        layout->addWidget(lineEdit, 1);

        // Track widget
        ParamWidgetInfo info;
        info.container = container;
        info.control = lineEdit;
        info.desc = desc;
        m_paramWidgets[QString::fromStdString(desc.id)] = info;
    }

    return container;
}

QWidget* ConfigPanel::createColorWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(m_scrollWidget);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* label = new QLabel(QString::fromStdString(desc.displayName), container);
    label->setMinimumWidth(100);
    layout->addWidget(label);

    auto* colorBtn = new QPushButton(container);
    colorBtn->setFixedSize(60, 24);
    colorBtn->setToolTip(QString::fromStdString(desc.tooltip));

    auto updateButtonColor = [colorBtn](const QColor& c) {
        colorBtn->setStyleSheet(
            QString("background-color: %1; border: 1px solid gray;").arg(c.name()));
    };
    updateButtonColor(QColor(128, 128, 128));

    connect(colorBtn, &QPushButton::clicked,
            [this, id = desc.id, updateButtonColor]() {
                QColor color = QColorDialog::getColor(Qt::white, this);
                if (color.isValid())
                {
                    updateButtonColor(color);
                    // Create array first, then construct variant
                    Color4f colorArray = {
                        static_cast<float>(color.redF()),
                        static_cast<float>(color.greenF()),
                        static_cast<float>(color.blueF()),
                        static_cast<float>(color.alphaF())
                    };
                    ParamValue colorValue;
                    colorValue.emplace<7>(colorArray);
                    onParamChanged(id, colorValue);
                }
            });

    layout->addWidget(colorBtn);
    layout->addStretch();

    // Track widget
    ParamWidgetInfo info;
    info.container = container;
    info.control = colorBtn;
    info.desc = desc;
    m_paramWidgets[QString::fromStdString(desc.id)] = info;

    return container;
}

// =============================================================================
// Parameter Change Handler
// =============================================================================

void ConfigPanel::onParamChanged(const std::string& paramId, const ParamValue& value)
{
    if (m_isUpdating || !m_visualizer)
    {
        return;
    }

    if (m_visualizer->setParam(paramId, value))
    {
        BasicLogger::logDebug("ConfigPanel: Set " + paramId);
        updateVisibility();
        
        // Any parameter change means the visualizer preset is now modified
        // Set to [Custom] which indicates modified state
        if (m_presetCombo && m_presetCombo->currentIndex() != 0)
        {
            QSignalBlocker blocker(m_presetCombo);
            m_presetCombo->setCurrentIndex(0);  // [Custom] = modified
        }
        
        // If a module preset was loaded, sync all widgets to show the new values
        if (paramId.find("preset") != std::string::npos)
        {
            BasicLogger::logDebug("ConfigPanel: Preset changed, syncing widgets...");
            syncFromVisualizer();
        }
        else
        {
            // A non-preset parameter was changed - update the related module preset widget
            // The module has already set its preset to [Custom], now sync the UI
            updateRelatedPresetWidget(paramId);
        }
    }
    else
    {
        BasicLogger::logWarning("ConfigPanel: Failed to set " + paramId);
    }
}

void ConfigPanel::updateRelatedPresetWidget(const std::string& paramId)
{
    // Determine which module preset widget needs to be updated
    std::string presetId;
    
    // Check for embedded smoothing parameters (audio.smooth.*)
    if (paramId.find("smooth.") != std::string::npos)
    {
        // Smoothing parameter changed -> update smooth.preset
        size_t smoothPos = paramId.find("smooth.");
        presetId = paramId.substr(0, smoothPos) + "smooth.preset";
    }
    // Check for audio parameters (audio.* but not audio.smooth.*)
    else if (paramId.rfind("audio.", 0) == 0)
    {
        // Audio parameter changed -> update audio.preset
        presetId = "audio.preset";
    }
    // Check for color/gradient parameters (shape.color.*)
    else if (paramId.rfind("shape.color.", 0) == 0)
    {
        // Color/Gradient parameter changed -> update shape.color.preset
        presetId = "shape.color.preset";
    }
    
    if (presetId.empty())
    {
        return;
    }
    
    // Find and update the preset widget
    QString key = QString::fromStdString(presetId);
    auto it = m_paramWidgets.find(key);
    if (it == m_paramWidgets.end())
    {
        return;
    }
    
    auto* combo = qobject_cast<QComboBox*>(it->control);
    if (!combo)
    {
        return;
    }
    
    // Get the current preset value from the module
    ParamValue presetValue;
    if (m_visualizer->getParam(presetId, presetValue))
    {
        if (std::holds_alternative<int>(presetValue))
        {
            int idx = std::get<int>(presetValue);
            if (combo->currentIndex() != idx)
            {
                QSignalBlocker blocker(combo);
                combo->setCurrentIndex(idx);
                BasicLogger::logDebug("ConfigPanel: Updated " + presetId + " to index " + std::to_string(idx));
            }
        }
    }
}

void ConfigPanel::updateVisibility()
{
    if (!m_visualizer) return;
    
    // Helper to create consistent keys (same logic as buildUIFromParams)
    auto makeSubGroupKey = [](const ModuleParamDesc& desc) -> QString {
        QString groupName = QString::fromStdString(desc.group);
        if (groupName.isEmpty())
        {
            groupName = "General";
        }
        return groupName + "|" + QString::fromStdString(desc.subGroup);
    };
    
    // Helper to check if dependsOn refers to a channelMode parameter
    auto isChannelModeDependency = [](const std::string& dependsOn) -> bool {
        const std::string suffix = "channelMode";
        if (dependsOn.size() < suffix.size()) return false;
        return dependsOn.compare(dependsOn.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    
    // Helper to check if dependsOn refers to a .visible parameter (for Oscilloscope channels)
    auto isVisibleDependency = [](const std::string& dependsOn) -> bool {
        const std::string suffix = ".visible";
        if (dependsOn.size() < suffix.size()) return false;
        return dependsOn.compare(dependsOn.size() - suffix.size(), suffix.size(), suffix) == 0;
    };

    // Step 1: Find channelMode parameter and get its value
    std::string channelModeParamId;
    int currentChannelMode = 0;
    
    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        const auto& desc = it.value().desc;
        if (isChannelModeDependency(desc.dependsOn))
        {
            channelModeParamId = desc.dependsOn;
            break;
        }
    }
    
    if (!channelModeParamId.empty())
    {
        ParamValue channelModeValue;
        if (m_visualizer->getParam(channelModeParamId, channelModeValue))
        {
            if (auto* v = std::get_if<int>(&channelModeValue))
            {
                currentChannelMode = *v;
            }
        }
    }
    
    // Step 2: Determine which subGroups should be completely hidden
    // A subGroup starting with "Line Color" is hidden if its channelMode or .visible dependency isn't met
    std::set<QString> hiddenSubGroups;
    std::set<QString> processedSubGroups;
    
    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        const auto& desc = it.value().desc;
        if (desc.subGroup.empty()) continue;
        
        QString key = makeSubGroupKey(desc);
        
        // Only process each subGroup once (use first channelMode param we find)
        if (processedSubGroups.count(key) > 0) continue;
        
        // Only hide "Line Color" subGroups completely
        if (desc.subGroup.find("Line Color") == std::string::npos) continue;
        
        // Handle channelMode dependency (int values)
        if (isChannelModeDependency(desc.dependsOn))
        {
            processedSubGroups.insert(key);
            
            // Check if this subGroup should be visible
            bool shouldBeVisible = false;
            for (const auto& reqValue : desc.dependsValues)
            {
                if (auto* intVal = std::get_if<int>(&reqValue))
                {
                    if (*intVal == currentChannelMode)
                    {
                        shouldBeVisible = true;
                        break;
                    }
                }
            }
            
            if (!shouldBeVisible)
            {
                hiddenSubGroups.insert(key);
            }
            continue;
        }
        
        // Handle .visible dependency (bool values) - for Oscilloscope channel visibility
        if (isVisibleDependency(desc.dependsOn))
        {
            processedSubGroups.insert(key);
            
            ParamValue visibleValue;
            bool shouldBeVisible = true;  // Default visible if can't get param
            
            if (m_visualizer->getParam(desc.dependsOn, visibleValue))
            {
                shouldBeVisible = false;
                for (const auto& reqValue : desc.dependsValues)
                {
                    if (visibleValue == reqValue)
                    {
                        shouldBeVisible = true;
                        break;
                    }
                }
            }
            
            if (!shouldBeVisible)
            {
                hiddenSubGroups.insert(key);
            }
            continue;
        }
    }
    
    // Step 3: Apply visibility to all params
    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        const auto& desc = it.value().desc;
        QString subGroupKey;
        if (!desc.subGroup.empty())
        {
            subGroupKey = makeSubGroupKey(desc);
        }
        
        // If this param's subGroup should be hidden, hide the param
        if (!subGroupKey.isEmpty() && hiddenSubGroups.count(subGroupKey) > 0)
        {
            it.value().container->setVisible(false);
            continue;
        }
        
        // No dependency - always visible
        if (desc.dependsOn.empty())
        {
            it.value().container->setVisible(true);
            continue;
        }
        
        // channelMode dependency - check visibility (for params outside hidden subGroups)
        if (isChannelModeDependency(desc.dependsOn))
        {
            bool visible = false;
            for (const auto& reqValue : desc.dependsValues)
            {
                if (auto* intVal = std::get_if<int>(&reqValue))
                {
                    if (*intVal == currentChannelMode)
                    {
                        visible = true;
                        break;
                    }
                }
            }
            it.value().container->setVisible(visible);
            continue;
        }
        
        // Normal dependsOn logic (e.g., mode-based visibility within a color group)
        ParamValue depValue;
        bool visible = true;
        if (m_visualizer->getParam(desc.dependsOn, depValue))
        {
            visible = false;
            for (const auto& reqValue : desc.dependsValues)
            {
                if (depValue == reqValue)
                {
                    visible = true;
                    break;
                }
            }
        }
        it.value().container->setVisible(visible);
    }
    
    // Step 4: Update subGroup box visibility
    for (auto it = m_subGroups.begin(); it != m_subGroups.end(); ++it)
    {
        bool hidden = hiddenSubGroups.count(it.key()) > 0;
        it.value()->setVisible(!hidden);
    }
}

// =============================================================================
// Gradient Editor Dialog
// =============================================================================

void ConfigPanel::openGradientEditor(const std::string& paramId)
{
    if (!m_visualizer)
    {
        BasicLogger::logWarning("ConfigPanel: No visualizer set for gradient editor");
        return;
    }
    
    // Try to get ColorGradientModule from any supported visualizer
    lumi::modules::ColorGradientModule* gradient = nullptr;
    
    // PulsingVisualizer
    if (auto* pulsing = dynamic_cast<PulsingVisualizer*>(m_visualizer))
    {
        gradient = pulsing->colorGradient();
    }
    // WaveformVisualizer
    else if (auto* waveform = dynamic_cast<WaveformVisualizer*>(m_visualizer))
    {
        gradient = &waveform->waveform()->colorGradient();
    }
    // OscilloscopeVisualizer - extract channel from parameter ID
    // Parameter IDs are like "ch1Color.editGradient", "ch2Color.editGradient", etc.
    else if (auto* oscilloscope = dynamic_cast<OscilloscopeVisualizer*>(m_visualizer))
    {
        int channelIndex = 0;  // Default to CH1
        
        if (paramId.find("ch1Color.") != std::string::npos) channelIndex = 0;
        else if (paramId.find("ch2Color.") != std::string::npos) channelIndex = 1;
        else if (paramId.find("ch3Color.") != std::string::npos) channelIndex = 2;
        else if (paramId.find("ch4Color.") != std::string::npos) channelIndex = 3;
        else if (paramId.find("m1Color.") != std::string::npos) channelIndex = 4;
        else if (paramId.find("m2Color.") != std::string::npos) channelIndex = 5;
        
        gradient = &oscilloscope->oscilloscope()->colorGradient(channelIndex);
    }
    
    if (!gradient)
    {
        BasicLogger::logWarning("ConfigPanel: Gradient editor not supported for this visualizer");
        return;
    }
    
    // Create and show dialog
    lumi::ui::GradientEditorDialog dialog(gradient, this);
    
    // When gradient changes, notify the visualizer
    dialog.setChangeCallback([]() {
        // Trigger a repaint or parameter sync if needed
        BasicLogger::logDebug("ConfigPanel: Gradient changed via editor");
    });
    
    dialog.exec();
    
    // Update the gradient preset dropdown with new preset names
    // (in case user saved a new preset in the editor)
    auto newPresetNames = gradient->presetNames();
    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        const QString& key = it.key();
        ParamWidgetInfo& info = it.value();
        
        if (key.contains("preset") && key.contains("color"))
        {
            auto* combo = qobject_cast<QComboBox*>(info.control);
            if (combo)
            {
                // Remember current selection
                int currentIndex = combo->currentIndex();
                QString currentText = combo->currentText();
                
                // Update options
                combo->blockSignals(true);
                combo->clear();
                for (const auto& name : newPresetNames)
                {
                    combo->addItem(QString::fromStdString(name));
                }
                
                // Try to restore selection
                int newIndex = combo->findText(currentText);
                if (newIndex >= 0)
                {
                    combo->setCurrentIndex(newIndex);
                }
                else if (currentIndex < combo->count())
                {
                    combo->setCurrentIndex(currentIndex);
                }
                combo->blockSignals(false);
                
                BasicLogger::logDebug("ConfigPanel: Updated gradient preset dropdown with " + 
                                      std::to_string(newPresetNames.size()) + " presets");
            }
        }
    }
    
    // Sync all widgets to reflect changes made in the editor
    syncFromVisualizer();
    
    BasicLogger::logInfo("ConfigPanel: Gradient editor closed, widgets synced");
}

// =============================================================================
// Preset Management
// =============================================================================

void ConfigPanel::setupPresetUI()
{
    auto* presetGroup = new QGroupBox(tr("Presets"), this);
    auto* layout = new QHBoxLayout(presetGroup);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    
    m_presetCombo = new QComboBox(presetGroup);
    m_presetCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // Placeholder - will be populated by refreshPresetList()
    m_presetCombo->addItem(tr("[Custom]"));
    m_presetCombo->addItem(tr("Default"));
    layout->addWidget(m_presetCombo);
    
    m_savePresetBtn = new QPushButton(tr("Save"), presetGroup);
    m_savePresetBtn->setFixedWidth(60);
    layout->addWidget(m_savePresetBtn);
    
    m_deletePresetBtn = new QPushButton(tr("Delete"), presetGroup);
    m_deletePresetBtn->setFixedWidth(60);
    m_deletePresetBtn->setEnabled(false);
    layout->addWidget(m_deletePresetBtn);
    
    // Insert at top of main layout
    auto* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout)
    {
        mainLayout->insertWidget(0, presetGroup);
    }
    
    // Connect signals
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConfigPanel::onPresetSelected);
    connect(m_savePresetBtn, &QPushButton::clicked,
            this, &ConfigPanel::onSavePresetClicked);
    connect(m_deletePresetBtn, &QPushButton::clicked,
            this, &ConfigPanel::onDeletePresetClicked);
}

void ConfigPanel::refreshPresetList()
{
    if (!m_visualizer || !m_presetCombo)
    {
        return;
    }
    
    m_presetCombo->blockSignals(true);
    m_presetCombo->clear();
    
    // [Custom] - shown when parameters are manually modified
    m_presetCombo->addItem(tr("[Custom]"));
    
    // Default - the hardcoded initial state
    m_presetCombo->addItem(tr("Default"));
    
    QString vizId = m_visualizer->visualizerId();
    QStringList presets = m_presetManager->availablePresets(vizId);
    
    // Add separator if user presets exist
    if (!presets.isEmpty())
    {
        m_presetCombo->addItem(tr("---"));
        
        // Disable separator
        auto* model = qobject_cast<QStandardItemModel*>(m_presetCombo->model());
        if (model)
        {
            int separatorIdx = m_presetCombo->count() - 1;
            auto* item = model->item(separatorIdx);
            if (item)
            {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            }
        }
        
        // Add user presets
        for (const QString& preset : presets)
        {
            m_presetCombo->addItem(preset);
        }
    }
    
    m_presetCombo->blockSignals(false);
    m_deletePresetBtn->setEnabled(false);
    
    // Start with "Default" selected (index 1)
    m_presetCombo->setCurrentIndex(1);
    
    BasicLogger::logDebug("ConfigPanel: Found " + std::to_string(presets.size()) + 
                          " presets for " + vizId.toStdString());
}

void ConfigPanel::onPresetSelected(int index)
{
    if (!m_visualizer || index < 0)
    {
        return;
    }
    
    // Index 0 = [Custom] - do nothing, just indicates modified state
    if (index == 0)
    {
        m_deletePresetBtn->setEnabled(false);
        return;
    }
    
    // Index 1 = Default - reset to hardcoded defaults
    if (index == 1)
    {
        m_visualizer->resetToDefaults();
        syncFromVisualizer();
        m_deletePresetBtn->setEnabled(false);
        BasicLogger::logInfo("ConfigPanel: Reset to defaults");
        return;
    }
    
    // Index 2 = --- Separator - should not be selectable, but handle anyway
    QString presetName = m_presetCombo->currentText();
    if (presetName == "---")
    {
        // Revert to Default
        m_presetCombo->blockSignals(true);
        m_presetCombo->setCurrentIndex(1);
        m_presetCombo->blockSignals(false);
        m_deletePresetBtn->setEnabled(false);
        return;
    }
    
    // Index 3+ = User presets
    QString vizId = m_visualizer->visualizerId();
    
    auto preset = m_presetManager->loadPreset(vizId, presetName);
    if (preset)
    {
        m_presetManager->applyPreset(m_visualizer, *preset);
        syncFromVisualizer();
        m_deletePresetBtn->setEnabled(true);  // User presets can be deleted
        BasicLogger::logInfo("ConfigPanel: Applied preset '" + presetName.toStdString() + "'");
    }
    else
    {
        BasicLogger::logWarning("ConfigPanel: Failed to load preset '" + presetName.toStdString() + "'");
    }
}

void ConfigPanel::onSavePresetClicked()
{
    if (!m_visualizer)
    {
        return;
    }
    
    bool ok;
    QString name = QInputDialog::getText(this, tr("Save Preset"),
                                         tr("Preset name:"), QLineEdit::Normal,
                                         QString(), &ok);
    
    if (!ok || name.isEmpty())
    {
        return;
    }
    
    // Check if preset exists
    QString vizId = m_visualizer->visualizerId();
    if (m_presetManager->presetExists(vizId, name))
    {
        int result = QMessageBox::question(this, tr("Overwrite Preset"),
                                           tr("A preset named '%1' already exists. Overwrite?").arg(name),
                                           QMessageBox::Yes | QMessageBox::No);
        if (result != QMessageBox::Yes)
        {
            return;
        }
    }
    
    // Capture and save
    auto preset = m_presetManager->capturePreset(m_visualizer, name);
    if (m_presetManager->savePreset(preset))
    {
        refreshPresetList();
        
        // Select the new preset
        int index = m_presetCombo->findText(name);
        if (index >= 0)
        {
            m_presetCombo->setCurrentIndex(index);
        }
        
        QMessageBox::information(this, tr("Preset Saved"),
                                 tr("Preset '%1' saved successfully.").arg(name));
    }
    else
    {
        QMessageBox::warning(this, tr("Save Failed"),
                             tr("Failed to save preset '%1'.").arg(name));
    }
}

void ConfigPanel::onDeletePresetClicked()
{
    // Index 0 = [Custom], Index 1 = Default, Index 2 = ---, Index 3+ = User presets
    if (!m_visualizer || m_presetCombo->currentIndex() <= 2)
    {
        return;
    }
    
    QString presetName = m_presetCombo->currentText();
    QString vizId = m_visualizer->visualizerId();
    
    int result = QMessageBox::question(this, tr("Delete Preset"),
                                       tr("Delete preset '%1'?").arg(presetName),
                                       QMessageBox::Yes | QMessageBox::No);
    
    if (result != QMessageBox::Yes)
    {
        return;
    }
    
    if (m_presetManager->deletePreset(vizId, presetName))
    {
        refreshPresetList();
        BasicLogger::logInfo("ConfigPanel: Deleted preset '" + presetName.toStdString() + "'");
    }
    else
    {
        QMessageBox::warning(this, tr("Delete Failed"),
                             tr("Failed to delete preset '%1'.").arg(presetName));
    }
}

// =============================================================================
// Module Preset Save
// =============================================================================

void ConfigPanel::onModulePresetSave(const std::string& paramId)
{
    if (!m_visualizer)
    {
        return;
    }
    
    auto* pulsing = dynamic_cast<PulsingVisualizer*>(m_visualizer);
    if (!pulsing)
    {
        BasicLogger::logWarning("ConfigPanel: Module preset save only supported for PulsingVisualizer");
        return;
    }
    
    // Get preset name from user
    QString name = QInputDialog::getText(this, tr("Save Preset"),
                                          tr("Preset name:"));
    if (name.isEmpty())
    {
        return;
    }
    
    // Validate name (no special characters)
    if (name.contains('/') || name.contains('\\') || name.contains('.'))
    {
        QMessageBox::warning(this, tr("Invalid Name"),
                             tr("Preset name cannot contain /, \\, or ."));
        return;
    }
    
    std::string presetName = name.toStdString();
    
    // Determine which module to save based on paramId
    // Note: "audio.smooth.preset" contains both "audio" and "smooth"
    // Check for "smooth" first (more specific)
    if (paramId.find("smooth") != std::string::npos)
    {
        // Smoothing preset - access via AudioSourceModule's embedded smoothing
        if (auto* audio = pulsing->audioSource())
        {
            audio->smoothing().savePreset(presetName);
            BasicLogger::logInfo("ConfigPanel: Saved smoothing preset '" + presetName + "'");
            
            // Refresh the dropdown
            refreshModulePresetDropdown(paramId, audio->smoothing().presetNames());
            
            QMessageBox::information(this, tr("Preset Saved"),
                                     tr("Smoothing preset '%1' saved successfully.").arg(name));
        }
    }
    else if (paramId.find("audio") != std::string::npos)
    {
        // Audio preset (includes smoothing settings)
        if (auto* audio = pulsing->audioSource())
        {
            audio->savePreset(presetName);
            BasicLogger::logInfo("ConfigPanel: Saved audio preset '" + presetName + "'");
            
            // Refresh the dropdown
            refreshModulePresetDropdown(paramId, audio->presetNames());
            
            QMessageBox::information(this, tr("Preset Saved"),
                                     tr("Audio preset '%1' saved successfully.").arg(name));
        }
    }
    else if (paramId.find("color") != std::string::npos)
    {
        // Gradient preset
        if (auto* gradient = pulsing->colorGradient())
        {
            gradient->savePreset(presetName);
            BasicLogger::logInfo("ConfigPanel: Saved gradient preset '" + presetName + "'");
            
            // Refresh the dropdown
            refreshModulePresetDropdown(paramId, gradient->presetNames());
            
            QMessageBox::information(this, tr("Preset Saved"),
                                     tr("Gradient preset '%1' saved successfully.").arg(name));
        }
    }
}

void ConfigPanel::refreshModulePresetDropdown(const std::string& paramId, 
                                               const std::vector<std::string>& presetNames)
{
    QString key = QString::fromStdString(paramId);
    auto it = m_paramWidgets.find(key);
    if (it == m_paramWidgets.end())
    {
        return;
    }
    
    auto* combo = qobject_cast<QComboBox*>(it->control);
    if (!combo)
    {
        return;
    }
    
    // Block signals while updating
    QSignalBlocker blocker(combo);
    
    // Remember current selection
    QString currentText = combo->currentText();
    
    // Clear and repopulate
    combo->clear();
    for (size_t i = 0; i < presetNames.size(); ++i)
    {
        const auto& name = presetNames[i];
        combo->addItem(QString::fromStdString(name));
        
        // Disable separator items
        if (name == "---")
        {
            auto* model = qobject_cast<QStandardItemModel*>(combo->model());
            if (model)
            {
                auto* item = model->item(static_cast<int>(i));
                if (item)
                {
                    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
                    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
                }
            }
        }
    }
    
    // Try to restore selection
    int idx = combo->findText(currentText);
    if (idx >= 0)
    {
        combo->setCurrentIndex(idx);
    }
}
