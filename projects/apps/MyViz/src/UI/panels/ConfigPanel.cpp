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
#include "visualizers/IVisualizer.hpp"
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
#include <QColorDialog>

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
    mainLayout->addWidget(m_scrollArea);

    // Placeholder
    auto* placeholder = new QLabel(tr("No visualizer selected"), m_scrollWidget);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color: gray;");
    m_contentLayout->addWidget(placeholder);
    m_contentLayout->addStretch();
}

int ConfigPanel::preferredArea() const
{
    return Qt::RightDockWidgetArea;
}

void ConfigPanel::onActivate()
{
    subscribeToEvents();
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
                }
            }
            else if (auto* i = std::get_if<int>(&value))
            {
                slider->setValue(*i);
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
            [](const VisualizerChangedEvent& evt) {
                BasicLogger::logDebug("ConfigPanel: Visualizer changed to " + evt.visualizerName);
                // The VisualizerWidget should call setVisualizer() when this happens
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
    m_paramWidgets.clear();
}

void ConfigPanel::buildUIFromParams(const std::vector<ModuleParamDesc>& params)
{
    // Sort by group, then order
    auto sortedParams = params;
    std::sort(sortedParams.begin(), sortedParams.end(),
              [](const ModuleParamDesc& a, const ModuleParamDesc& b) {
                  if (a.group != b.group) return a.group < b.group;
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
            group->addWidget(widget);
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

    // Use spinbox for large ranges, slider otherwise
    int range = static_cast<int>(desc.maxValue - desc.minValue);
    
    if (desc.widget == ParamWidget::Spinbox || range > 100)
    {
        auto* spinbox = new QSpinBox(container);
        spinbox->setRange(static_cast<int>(desc.minValue), static_cast<int>(desc.maxValue));
        spinbox->setSingleStep(desc.step > 0 ? static_cast<int>(desc.step) : 1);
        spinbox->setToolTip(QString::fromStdString(desc.tooltip));

        connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                [this, id = desc.id](int value) {
                    onParamChanged(id, value);
                });

        control = spinbox;
        layout->addWidget(spinbox, 1);
    }
    else
    {
        auto* slider = new QSlider(Qt::Horizontal, container);
        slider->setRange(static_cast<int>(desc.minValue), static_cast<int>(desc.maxValue));
        slider->setToolTip(QString::fromStdString(desc.tooltip));

        auto* valueLabel = new QLabel(container);
        valueLabel->setMinimumWidth(50);
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QString unit = QString::fromStdString(desc.unit);

        connect(slider, &QSlider::valueChanged,
                [this, id = desc.id, valueLabel, unit](int value) {
                    valueLabel->setText(QString::number(value) + unit);
                    onParamChanged(id, value);
                });

        control = slider;
        layout->addWidget(slider, 1);
        layout->addWidget(valueLabel);

        // Track value label
        ParamWidgetInfo info;
        info.container = container;
        info.control = slider;
        info.valueLabel = valueLabel;
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

    auto* valueLabel = new QLabel(container);
    valueLabel->setMinimumWidth(60);
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QString unit = QString::fromStdString(desc.unit);
    float range = desc.maxValue - desc.minValue;
    float minVal = desc.minValue;

    connect(slider, &QSlider::valueChanged,
            [this, id = desc.id, valueLabel, unit, minVal, range](int sliderVal) {
                float value = minVal + (sliderVal / 1000.0f) * range;
                valueLabel->setText(QString::number(value, 'f', 2) + unit);
                onParamChanged(id, value);
            });

    layout->addWidget(slider, 1);
    layout->addWidget(valueLabel);

    // Track widget
    ParamWidgetInfo info;
    info.container = container;
    info.control = slider;
    info.valueLabel = valueLabel;
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
    for (const auto& option : desc.enumOptions)
    {
        combo->addItem(QString::fromStdString(option));
    }
    combo->setToolTip(QString::fromStdString(desc.tooltip));

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, id = desc.id](int index) {
                onParamChanged(id, index);
            });

    layout->addWidget(combo, 1);

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
    }
    else
    {
        BasicLogger::logWarning("ConfigPanel: Failed to set " + paramId);
    }
}

void ConfigPanel::updateVisibility()
{
    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        const auto& desc = it->desc;

        if (desc.dependsOn.empty())
        {
            continue;
        }

        ParamValue depValue;
        if (m_visualizer->getParam(desc.dependsOn, depValue))
        {
            bool visible = (depValue == desc.dependsValue);
            it->container->setVisible(visible);
        }
    }
}
