/**
 ****************************************************************************************
 * @file   ModuleConfigWidget.cpp
 * @brief  ModuleConfigWidget implementation
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "UI/widgets/ModuleConfigWidget.hpp"
#include "UI/widgets/CollapsibleGroupBox.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QColorDialog>
#include <QPushButton>

#include <BasicLogger.h>

#include <algorithm>

using namespace lumi::modules;

// =============================================================================
// Construction
// =============================================================================

ModuleConfigWidget::ModuleConfigWidget(IModule* module, QWidget* parent)
    : QWidget(parent)
    , m_module(module)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(4);

    if (m_module)
    {
        buildUI();
    }
}

// =============================================================================
// Public Methods
// =============================================================================

void ModuleConfigWidget::rebuild()
{
    clearUI();
    if (m_module)
    {
        buildUI();
    }
}

void ModuleConfigWidget::syncFromModule()
{
    if (!m_module)
    {
        return;
    }

    m_isUpdating = true;

    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        ParamValue value;
        if (m_module->getParam(it.key().toStdString(), value))
        {
            // Update widget based on type
            const auto& desc = it->desc;

            if (auto* checkbox = qobject_cast<QCheckBox*>(it->widget))
            {
                if (auto* b = std::get_if<bool>(&value))
                {
                    checkbox->setChecked(*b);
                }
            }
            else if (auto* spinbox = qobject_cast<QSpinBox*>(it->widget))
            {
                if (auto* i = std::get_if<int>(&value))
                {
                    spinbox->setValue(*i);
                }
            }
            else if (auto* slider = qobject_cast<QSlider*>(it->widget))
            {
                if (auto* f = std::get_if<float>(&value))
                {
                    int sliderVal = static_cast<int>((*f - desc.minValue) / 
                                   (desc.maxValue - desc.minValue) * 1000);
                    slider->setValue(sliderVal);
                }
                else if (auto* i = std::get_if<int>(&value))
                {
                    slider->setValue(*i);
                }
            }
            else if (auto* combo = qobject_cast<QComboBox*>(it->widget))
            {
                if (auto* i = std::get_if<int>(&value))
                {
                    combo->setCurrentIndex(*i);
                }
            }
            else if (auto* lineEdit = qobject_cast<QLineEdit*>(it->widget))
            {
                if (auto* s = std::get_if<std::string>(&value))
                {
                    lineEdit->setText(QString::fromStdString(*s));
                }
            }
        }
    }

    m_isUpdating = false;
    updateVisibility();
}

// =============================================================================
// Private - UI Building
// =============================================================================

void ModuleConfigWidget::clearUI()
{
    // Remove all widgets
    QLayoutItem* item;
    while ((item = m_mainLayout->takeAt(0)) != nullptr)
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

void ModuleConfigWidget::buildUI()
{
    if (!m_module)
    {
        return;
    }

    // Get parameter descriptors
    auto params = m_module->paramDescs();

    // Sort by group, then order
    std::sort(params.begin(), params.end(),
              [](const ModuleParamDesc& a, const ModuleParamDesc& b) {
                  if (a.group != b.group) return a.group < b.group;
                  return a.order < b.order;
              });

    // Build widgets for each parameter
    for (const auto& desc : params)
    {
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
            BasicLogger::logWarning("ModuleConfigWidget: Unsupported param type for " + desc.id);
            continue;
        }

        if (widget)
        {
            // Get or create group
            QString groupName = QString::fromStdString(desc.group);
            if (groupName.isEmpty())
            {
                groupName = tr("General");
            }

            auto* group = getOrCreateGroup(groupName);
            group->addWidget(widget);

            // Track for dependency updates
            ParamWidgetInfo info;
            info.widget = widget;
            info.desc = desc;
            m_paramWidgets[QString::fromStdString(desc.id)] = info;
        }
    }

    // Add stretch at the end
    m_mainLayout->addStretch();

    // Initial visibility update
    updateVisibility();
    syncFromModule();
}

CollapsibleGroupBox* ModuleConfigWidget::getOrCreateGroup(const QString& groupName)
{
    if (m_groups.contains(groupName))
    {
        return m_groups[groupName];
    }

    auto* group = new CollapsibleGroupBox(groupName, this);
    m_groups[groupName] = group;
    m_mainLayout->addWidget(group);

    return group;
}

// =============================================================================
// Widget Creators
// =============================================================================

QWidget* ModuleConfigWidget::createBoolWidget(const ModuleParamDesc& desc)
{
    auto* checkbox = new QCheckBox(QString::fromStdString(desc.displayName), this);
    checkbox->setToolTip(QString::fromStdString(desc.tooltip));

    // Get current value
    ParamValue value;
    if (m_module->getParam(desc.id, value))
    {
        if (auto* b = std::get_if<bool>(&value))
        {
            checkbox->setChecked(*b);
        }
    }

    // Connect
    connect(checkbox, &QCheckBox::toggled,
            [this, id = desc.id](bool checked) {
                onParamChanged(id, checked);
            });

    return checkbox;
}

QWidget* ModuleConfigWidget::createIntWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(this);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* label = new QLabel(QString::fromStdString(desc.displayName), container);
    label->setMinimumWidth(100);
    layout->addWidget(label);

    QWidget* control = nullptr;

    if (desc.widget == ParamWidget::Spinbox || 
        (desc.maxValue - desc.minValue) > 100)
    {
        auto* spinbox = new QSpinBox(container);
        spinbox->setRange(static_cast<int>(desc.minValue), static_cast<int>(desc.maxValue));
        spinbox->setSingleStep(static_cast<int>(desc.step > 0 ? desc.step : 1));
        spinbox->setToolTip(QString::fromStdString(desc.tooltip));

        connect(spinbox, QOverload<int>::of(&QSpinBox::valueChanged),
                [this, id = desc.id](int value) {
                    onParamChanged(id, value);
                });

        control = spinbox;
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

        layout->addWidget(slider, 1);
        layout->addWidget(valueLabel);
        return container;
    }

    layout->addWidget(control, 1);
    return container;
}

QWidget* ModuleConfigWidget::createFloatWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(this);
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

    connect(slider, &QSlider::valueChanged,
            [this, id = desc.id, valueLabel, unit, minVal = desc.minValue, range](int sliderVal) {
                float value = minVal + (sliderVal / 1000.0f) * range;
                valueLabel->setText(QString::number(value, 'f', 2) + unit);
                onParamChanged(id, value);
            });

    layout->addWidget(slider, 1);
    layout->addWidget(valueLabel);

    return container;
}

QWidget* ModuleConfigWidget::createEnumWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(this);
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

    return container;
}

QWidget* ModuleConfigWidget::createStringWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(this);
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

    return container;
}

QWidget* ModuleConfigWidget::createColorWidget(const ModuleParamDesc& desc)
{
    auto* container = new QWidget(this);
    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* label = new QLabel(QString::fromStdString(desc.displayName), container);
    label->setMinimumWidth(100);
    layout->addWidget(label);

    auto* colorBtn = new QPushButton(container);
    colorBtn->setFixedSize(60, 24);
    colorBtn->setToolTip(QString::fromStdString(desc.tooltip));

    // Store current color
    QColor currentColor(128, 128, 128);

    // Update button style
    auto updateButtonColor = [colorBtn](const QColor& c) {
        colorBtn->setStyleSheet(QString("background-color: %1; border: 1px solid gray;")
                                    .arg(c.name()));
    };
    updateButtonColor(currentColor);

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

    return container;
}

// =============================================================================
// Private Methods
// =============================================================================

void ModuleConfigWidget::onParamChanged(const std::string& paramId, const ParamValue& value)
{
    if (m_isUpdating || !m_module)
    {
        return;
    }

    // Apply to module
    if (m_module->setParam(paramId, value))
    {
        BasicLogger::logDebug("ModuleConfigWidget: Set " + paramId);
        Q_EMIT parameterChanged(QString::fromStdString(paramId), value);

        // Update dependent parameters visibility
        updateVisibility();
    }
    else
    {
        BasicLogger::logWarning("ModuleConfigWidget: Failed to set " + paramId);
    }
}

void ModuleConfigWidget::updateVisibility()
{
    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        const auto& desc = it->desc;

        if (desc.dependsOn.empty())
        {
            continue;
        }

        // Check if dependency is satisfied
        ParamValue depValue;
        if (m_module->getParam(desc.dependsOn, depValue))
        {
            // Check if current value matches ANY of the required values (OR logic)
            bool visible = false;
            for (const auto& reqValue : desc.dependsValues)
            {
                if (depValue == reqValue)
                {
                    visible = true;
                    break;
                }
            }
            it->widget->setVisible(visible);
        }
    }
}
