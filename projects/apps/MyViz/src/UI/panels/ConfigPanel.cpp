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
#include "UI/widgets/TapPreviewWidget.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "UI/widgets/GradientPresetDelegate.hpp"
#include "UI/dialogs/GradientEditorDialog.hpp"
#include "visualizers/IVisualizer.hpp"
#include "visualizers/modules/ColorGradientModule.hpp"
#include "visualizers/modules/source/AudioSourceModule.hpp"
#include "visualizers/VisualizerPresetManager.hpp"
#include "visualizers/SetParamCommand.hpp"
#include "services/ServiceContainer.hpp"
#include "services/IEventBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/events/UIEvents.hpp"
#include "services/events/CommandEvents.hpp"

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
#include <QMenu>
#include <QSettings>
#include <QMutex>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QTimer>
#include <QToolButton>

#include <BasicLogger.h>

#include <algorithm>

using namespace lumi::modules;

// =============================================================================
// Pipeline Stage Table (Phase 4)
// =============================================================================

namespace
{

/// Display title + icon per pipeline stage — the UI follows the data flow.
/// Single source of truth for stage naming/ordering in the panel; used as
/// soon as a visualizer declares ModuleParamDesc::stage (Schritt 5 migration).
struct StageInfo
{
    QString icon;
    QString title;
};

const StageInfo* stageInfo(PipelineStage stage)
{
    static const std::map<PipelineStage, StageInfo> table = {
        {PipelineStage::AudioSource,  {QStringLiteral("🎵 "), QStringLiteral("1. Audio / Analysis")}},
        {PipelineStage::Mapping,      {QStringLiteral("🔀 "), QStringLiteral("2. Mapping")}},
        {PipelineStage::Color,        {QStringLiteral("🎨 "), QStringLiteral("3. Color")}},
        {PipelineStage::Render,       {QStringLiteral("🖼️ "), QStringLiteral("4. Rendering")}},
        {PipelineStage::PeakParticle, {QStringLiteral("✨ "), QStringLiteral("5. Peak / Particles")}},
        {PipelineStage::Post,         {QStringLiteral("🌟 "), QStringLiteral("6. Post FX")}},
    };
    auto it = table.find(stage);
    return (it != table.end()) ? &it->second : nullptr;
}

/// Sort rank for unmigrated groups: leading digit of the legacy group name
/// ("3. Color" -> 3); groups without a digit prefix sort last (99).
int legacyGroupRank(const std::string& group)
{
    if (!group.empty() && group.front() >= '1' && group.front() <= '9')
    {
        return group.front() - '0';
    }
    return 99;
}

/// Legacy emoji heuristic — only for groups of unmigrated visualizers
/// (declared stages take their icon from the stage table instead).
QString groupIcon(const QString& groupName)
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

/// @brief Does paramId end with the given suffix?
bool endsWith(const std::string& paramId, const std::string& suffix)
{
    return paramId.size() >= suffix.size() &&
           paramId.compare(paramId.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/// @brief Find the gradient handle whose paramPrefix matches paramId (or null)
lumi::modules::ColorGradientModule* gradientForParam(IVisualizer* visualizer,
                                                     const std::string& paramId,
                                                     std::string* prefixOut = nullptr)
{
    if (!visualizer)
    {
        return nullptr;
    }
    for (const auto& handle : visualizer->gradients())
    {
        if (paramId.rfind(handle.paramPrefix, 0) == 0)
        {
            if (prefixOut)
            {
                *prefixOut = handle.paramPrefix;
            }
            return handle.gradient;
        }
    }
    return nullptr;
}

/// @brief Is desc.defaultValue usable for a reset (matches the declared type)?
///
/// A default-constructed ParamValue holds `bool false` — for e.g. Color
/// parameters without an explicit default that would be a wrong reset, so
/// the reset action is disabled instead.
bool hasUsableDefault(const ModuleParamDesc& desc)
{
    switch (desc.type)
    {
    case ParamType::Bool:
        return std::holds_alternative<bool>(desc.defaultValue);
    case ParamType::Int:
    case ParamType::Enum:
        return std::holds_alternative<int>(desc.defaultValue) ||
               std::holds_alternative<float>(desc.defaultValue);
    case ParamType::Float:
        return std::holds_alternative<float>(desc.defaultValue) ||
               std::holds_alternative<int>(desc.defaultValue);
    case ParamType::String:
        return std::holds_alternative<std::string>(desc.defaultValue);
    case ParamType::Color:
        return holdsColor(desc.defaultValue);
    default:
        return false;
    }
}

/// @brief Default value coerced to the declared type (int<->float tolerant)
ParamValue coercedDefault(const ModuleParamDesc& desc)
{
    const ParamValue& def = desc.defaultValue;
    switch (desc.type)
    {
    case ParamType::Int:
    case ParamType::Enum:
        if (auto* f = std::get_if<float>(&def))
        {
            return static_cast<int>(*f);
        }
        break;
    case ParamType::Float:
        if (auto* i = std::get_if<int>(&def))
        {
            return static_cast<float>(*i);
        }
        break;
    default:
        break;
    }
    return def;
}

}  // namespace

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

    // Shared preview timer (Schritt 6): 20 Hz, runs ONLY while a preview
    // is visible (started/stopped in updatePreviewTimer, N7)
    m_previewTimer = new QTimer(this);
    m_previewTimer->setInterval(50);
    connect(m_previewTimer, &QTimer::timeout, this, [this]() { onPreviewTick(); });
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
            // Always hand over the widget's render mutex — passing null here
            // would silently drop the render-thread guards
            setVisualizer(vizWidget->visualizer(), &vizWidget->renderMutex());
        }
    }

    // Resume preview polling if a preview is toggled on (no-op otherwise)
    updatePreviewTimer();
}

void ConfigPanel::onDeactivate()
{
    unsubscribeFromEvents();
    if (m_previewTimer != nullptr)
    {
        m_previewTimer->stop();
    }
}

// =============================================================================
// Visualizer Management
// =============================================================================

void ConfigPanel::setVisualizer(IVisualizer* visualizer, QMutex* renderMutex)
{
    if (m_visualizer == visualizer)
    {
        m_renderMutex = renderMutex;
        return;
    }

    // The undo history holds SetParamCommands referencing the OLD visualizer
    // instance — drop it before switching (dangling-reference safety).
    if (auto* commandBus = services().tryResolve<ICommandBus>())
    {
        commandBus->clear();
    }

    m_visualizer = visualizer;
    m_renderMutex = renderMutex;
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

    // Stage previews (Schritt 6): tap points + gradient strips per stage group
    buildStagePreviews();

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
            if (desc.type == ParamType::Color && holdsColor(value))
            {
                const auto& c = getColor(value);
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

    m_eventSubscriptions.push_back(
        bus->subscribeScoped<VisualizerChangedEvent>(
            [this](const VisualizerChangedEvent& evt) {
                BasicLogger::logDebug("ConfigPanel: Visualizer changed to " + evt.visualizerName);
                if (evt.visualizerPtr != nullptr)
                {
                    auto* viz = static_cast<IVisualizer*>(evt.visualizerPtr);
                    setVisualizer(viz, evt.renderMutex);
                }
            }
        )
    );

    // Undo/Redo changed parameter values behind the widgets' back — re-sync.
    m_eventSubscriptions.push_back(
        bus->subscribeScoped<CommandHistoryChangedEvent>(
            [this](const CommandHistoryChangedEvent& evt) {
                using Cause = CommandHistoryChangedEvent::Cause;
                if (evt.cause == Cause::Undone || evt.cause == Cause::Redone)
                {
                    syncFromVisualizer();
                    updateVisibility();
                }
            }
        )
    );
}

void ConfigPanel::unsubscribeFromEvents()
{
    // RAII handles unsubscribe on destruction; clearing releases them now.
    m_eventSubscriptions.clear();
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

    // Preview widgets/toggles are children of the group boxes (already
    // deleted above) — drop the bookkeeping and stop polling
    m_stagePreviews.clear();
    if (m_previewTimer != nullptr)
    {
        m_previewTimer->stop();
    }
}

void ConfigPanel::buildUIFromParams(const std::vector<ModuleParamDesc>& params)
{
    // Effective stage per group: a declared ModuleParamDesc::stage wins,
    // otherwise the legacy "1. Audio"-style digit prefix ranks the group.
    // This keeps today's order for unmigrated visualizers and switches to
    // real stage ordering automatically once a visualizer is migrated.
    std::map<std::string, PipelineStage> groupDeclaredStage;
    for (const auto& p : params)
    {
        if (p.stage != PipelineStage::None &&
            groupDeclaredStage.find(p.group) == groupDeclaredStage.end())
        {
            groupDeclaredStage[p.group] = p.stage;
        }
    }

    auto groupRank = [&groupDeclaredStage](const std::string& group) -> int {
        auto it = groupDeclaredStage.find(group);
        if (it != groupDeclaredStage.end())
        {
            return static_cast<int>(it->second);
        }
        return legacyGroupRank(group);
    };

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

    // Sort by effective stage, then group, then subGroup's minimum order,
    // then individual order
    auto sortedParams = params;
    std::sort(sortedParams.begin(), sortedParams.end(),
              [&subGroupMinOrder, &groupRank](const ModuleParamDesc& a, const ModuleParamDesc& b) {
                  int rankA = groupRank(a.group);
                  int rankB = groupRank(b.group);
                  if (rankA != rankB) return rankA < rankB;

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
            // Resolve group key + display title: declared stages share one
            // group per stage (title/icon from the stage table); legacy
            // groups keep their name and the emoji keyword heuristic.
            QString groupKey;
            QString groupTitle;
            auto stageIt = groupDeclaredStage.find(desc.group);
            if (stageIt != groupDeclaredStage.end())
            {
                const StageInfo* info = stageInfo(stageIt->second);
                groupKey = QStringLiteral("stage:%1").arg(static_cast<int>(stageIt->second));
                groupTitle = info ? (info->icon + info->title)
                                  : QString::fromStdString(desc.group);
            }
            else
            {
                QString groupName = QString::fromStdString(desc.group);
                if (groupName.isEmpty())
                {
                    groupName = tr("General");
                }
                groupKey = groupName;
                groupTitle = groupIcon(groupName) + groupName;
            }

            auto* group = getOrCreateGroup(groupKey, groupTitle);

            // Check for subGroup - add to framed container
            QString subGroupKey;
            if (!desc.subGroup.empty())
            {
                subGroupKey = groupKey + "|" + QString::fromStdString(desc.subGroup);
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

                    // Right-click: reset all parameters of this sub-group
                    subGroup->setContextMenuPolicy(Qt::CustomContextMenu);
                    connect(subGroup, &QWidget::customContextMenuRequested,
                            this, [this, subGroup, groupKey, subGroupKey](const QPoint& pos) {
                                QMenu menu(this);
                                QAction* resetAct = menu.addAction(tr("Reset group to defaults"));
                                if (menu.exec(subGroup->mapToGlobal(pos)) == resetAct)
                                {
                                    resetGroupToDefaults(groupKey, subGroupKey);
                                }
                            });
                }

                subGroup->layout()->addWidget(widget);
            }
            else
            {
                group->addWidget(widget);
            }

            // Remember resolved keys + attach the per-parameter reset menu
            auto infoIt = m_paramWidgets.find(QString::fromStdString(desc.id));
            if (infoIt != m_paramWidgets.end())
            {
                infoIt->groupKey = groupKey;
                infoIt->subGroupKey = subGroupKey;

                QWidget* container = infoIt->container;
                container->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(container, &QWidget::customContextMenuRequested,
                        this, [this, container, id = desc.id](const QPoint& pos) {
                            showParamContextMenu(container, pos, id);
                        });
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

CollapsibleGroupBox* ConfigPanel::getOrCreateGroup(const QString& groupKey, const QString& displayTitle)
{
    if (m_groups.contains(groupKey))
    {
        return m_groups[groupKey];
    }

    auto* group = new CollapsibleGroupBox(displayTitle, m_scrollWidget);
    m_groups[groupKey] = group;
    m_contentLayout->addWidget(group);

    // Right-click on the group header: reset all parameters of this group
    group->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(group, &QWidget::customContextMenuRequested,
            this, [this, group, groupKey](const QPoint& pos) {
                QMenu menu(this);
                QAction* resetAct = menu.addAction(tr("Reset group to defaults"));
                if (menu.exec(group->mapToGlobal(pos)) == resetAct)
                {
                    resetGroupToDefaults(groupKey, QString());
                }
            });

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
    
    // Gradient preset dropdown? The matching gradient handle provides the
    // ColorGradientModule for the preview delegate — per channel, no casts.
    lumi::modules::ColorGradientModule* gradientModule = nullptr;
    if (endsWith(desc.id, "preset"))
    {
        gradientModule = gradientForParam(m_visualizer, desc.id);
    }

    if (gradientModule)
    {
        auto* delegate = new lumi::ui::GradientPresetDelegate(combo);
        delegate->setGradientModule(gradientModule);
        combo->setItemDelegate(delegate);

        // Make combo taller to show preview
        combo->setMinimumHeight(28);
    }

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, id = desc.id](int index) {
                onParamChanged(id, index);
            });

    layout->addWidget(combo, 1);

    // Add Save button for module preset dropdowns (Smoothing, Audio, Gradient)
    bool isModulePreset = endsWith(desc.id, "preset") &&
                          (gradientModule != nullptr ||
                           desc.id.find("smooth") != std::string::npos ||
                           desc.id.find("audio") != std::string::npos);

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
                    // Create array first, then construct variant (type-checked
                    // via the kParamValueColorIndex helpers)
                    Color4f colorArray = {
                        static_cast<float>(color.redF()),
                        static_cast<float>(color.greenF()),
                        static_cast<float>(color.blueF()),
                        static_cast<float>(color.alphaF())
                    };
                    onParamChanged(id, makeColorValue(colorArray));
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

    // Route through the CommandBus so the change is undo-able (Ctrl+Z).
    // Consecutive changes to the same parameter merge into one undo step
    // (slider drags). Fallback: direct setParam if the bus is missing or the
    // parameter has no readable old value (e.g. trigger buttons).
    bool applied = false;
    ParamValue oldValue;
    auto* commandBus = services().tryResolve<ICommandBus>();
    if (commandBus && m_visualizer->getParam(paramId, oldValue))
    {
        // The command locks the render mutex itself (undo/redo bypasses us)
        applied = commandBus->execute(std::make_unique<SetParamCommand>(
            *m_visualizer, paramId, oldValue, value, m_renderMutex));
    }
    else
    {
        QMutexLocker lock(m_renderMutex);
        applied = m_visualizer->setParam(paramId, value);
    }

    if (applied)
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
    // Check for gradient parameters — the matching handle names the widget
    else
    {
        std::string gradientPrefix;
        if (gradientForParam(m_visualizer, paramId, &gradientPrefix))
        {
            presetId = gradientPrefix + "preset";
        }
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

    // Purely generic dependsOn/dependsValues evaluation — no parameter-name
    // special cases. Visibility is transitive: a parameter is effectively
    // visible only if its own condition holds AND the parameter it depends
    // on is itself effectively visible. Dependency chains like
    // solidColor -> mode -> channelMode thereby collapse whole sub-groups
    // (e.g. the per-channel "Line Color" boxes) without heuristics.
    QMap<QString, bool> visCache;

    std::function<bool(const QString&)> isEffectivelyVisible =
        [this, &visCache, &isEffectivelyVisible](const QString& paramKey) -> bool {
            auto cached = visCache.find(paramKey);
            if (cached != visCache.end())
            {
                return cached.value();
            }
            // Cycle guard: assume visible while resolving this chain
            visCache.insert(paramKey, true);

            auto it = m_paramWidgets.find(paramKey);
            if (it == m_paramWidgets.end())
            {
                // Dependency target has no widget (e.g. hidden param) —
                // treat as visible, only the value condition applies.
                return true;
            }

            const auto& desc = it->desc;
            bool visible = true;
            if (!desc.dependsOn.empty())
            {
                // Value condition (OR over dependsValues); unreadable
                // dependency values leave the parameter visible.
                ParamValue depValue;
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

                // Transitive: the dependency target must be visible itself
                if (visible)
                {
                    visible = isEffectivelyVisible(QString::fromStdString(desc.dependsOn));
                }
            }

            visCache.insert(paramKey, visible);
            return visible;
        };

    // Apply per-parameter visibility; track which (sub-)groups still have
    // at least one visible parameter.
    QMap<QString, bool> subGroupHasVisible;
    QMap<QString, bool> groupHasVisible;

    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        const bool visible = isEffectivelyVisible(it.key());
        it.value().container->setVisible(visible);

        if (!it.value().groupKey.isEmpty())
        {
            groupHasVisible[it.value().groupKey] =
                groupHasVisible.value(it.value().groupKey, false) || visible;
        }
        if (!it.value().subGroupKey.isEmpty())
        {
            subGroupHasVisible[it.value().subGroupKey] =
                subGroupHasVisible.value(it.value().subGroupKey, false) || visible;
        }
    }

    // A sub-group box is hidden when ALL of its parameters are invisible
    for (auto it = m_subGroups.begin(); it != m_subGroups.end(); ++it)
    {
        it.value()->setVisible(subGroupHasVisible.value(it.key(), true));
    }

    // Same rule for top-level groups (empty group -> hidden)
    for (auto it = m_groups.begin(); it != m_groups.end(); ++it)
    {
        it.value()->setVisible(groupHasVisible.value(it.key(), true));
    }

    // Color strips follow their handle's parameter visibility (Schritt 6) —
    // e.g. hiding a channel via channelMode hides its strip too
    updatePreviewVisibility();
}

// =============================================================================
// Default Reset (Phase 4 — per parameter / sub-group / group, undo-able)
// =============================================================================

void ConfigPanel::showParamContextMenu(QWidget* anchor, const QPoint& pos, const std::string& paramId)
{
    auto it = m_paramWidgets.find(QString::fromStdString(paramId));
    if (it == m_paramWidgets.end() || !m_visualizer)
    {
        return;
    }

    const ModuleParamDesc desc = it->desc;

    // Parent the menu to the panel (not the widget) — a UI rebuild during
    // the nested exec() loop must not delete the open menu.
    QMenu menu(this);
    QAction* resetAct = menu.addAction(tr("Reset to default"));
    // Parameters without a typed default (e.g. colors whose defaultValue is
    // an empty variant) cannot be reset safely — disable instead of guessing.
    resetAct->setEnabled(hasUsableDefault(desc));

    if (menu.exec(anchor->mapToGlobal(pos)) == resetAct)
    {
        // Routed through onParamChanged -> CommandBus, so the reset is
        // one undo step like any other edit.
        onParamChanged(desc.id, coercedDefault(desc));
        syncFromVisualizer();
    }
}

void ConfigPanel::resetGroupToDefaults(const QString& groupKey, const QString& subGroupKey)
{
    if (!m_visualizer)
    {
        return;
    }

    // Reset every parameter of the (sub-)group that has a usable default.
    // Each reset is its own command (multiple undo steps — a composite
    // command is a later refinement).
    for (auto it = m_paramWidgets.begin(); it != m_paramWidgets.end(); ++it)
    {
        const auto& info = it.value();
        if (info.groupKey != groupKey)
        {
            continue;
        }
        if (!subGroupKey.isEmpty() && info.subGroupKey != subGroupKey)
        {
            continue;
        }
        if (!hasUsableDefault(info.desc))
        {
            continue;
        }

        onParamChanged(info.desc.id, coercedDefault(info.desc));
    }

    syncFromVisualizer();
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

    // The gradient handle whose paramPrefix matches identifies the module —
    // per channel (Oscilloscope ch1..m2, Waveform mono/left/right), no casts.
    lumi::modules::ColorGradientModule* gradient = gradientForParam(m_visualizer, paramId);

    if (!gradient)
    {
        BasicLogger::logWarning("ConfigPanel: No gradient handle matches " + paramId);
        return;
    }

    // Create and show dialog; gradient mutations inside the editor are
    // guarded against the render thread via the render mutex
    lumi::ui::GradientEditorDialog dialog(gradient, this);
    dialog.setRenderMutex(m_renderMutex);

    // When gradient changes, notify the visualizer
    dialog.setChangeCallback([]() {
        // Trigger a repaint or parameter sync if needed
        BasicLogger::logDebug("ConfigPanel: Gradient changed via editor");
    });

    dialog.exec();

    // Update every gradient preset dropdown with its handle's preset names
    // (in case the user saved a new preset in the editor)
    for (const auto& handle : m_visualizer->gradients())
    {
        refreshModulePresetDropdown(handle.paramPrefix + "preset",
                                    handle.gradient->presetNames());
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
    
    // Start with "Default" selected (index 1) — still under the signal
    // block: this is initial UI state, NOT a user action. Unblocked it fired
    // onPresetSelected(1) -> resetToDefaults() on EVERY rebuild (the visible
    // color jump when opening the panel).
    m_presetCombo->setCurrentIndex(1);

    m_presetCombo->blockSignals(false);
    m_deletePresetBtn->setEnabled(false);
    
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
        {
            QMutexLocker lock(m_renderMutex);
            m_visualizer->resetToDefaults();
        }
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
        {
            QMutexLocker lock(m_renderMutex);
            m_presetManager->applyPreset(m_visualizer, *preset);
        }
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

    // Resolve the target module generically — works for every visualizer:
    // smoothing/audio via the audio-source handle, gradients via prefix match.
    // Note: "audio.smooth.preset" contains both "audio" and "smooth" —
    // check for "smooth" first (more specific).
    const bool isSmoothing = paramId.find("smooth") != std::string::npos;
    const bool isAudio = !isSmoothing && paramId.find("audio") != std::string::npos;

    auto* audio = m_visualizer->audioSourceModule();
    lumi::modules::ColorGradientModule* gradient =
        (isSmoothing || isAudio) ? nullptr : gradientForParam(m_visualizer, paramId);

    if (((isSmoothing || isAudio) && !audio) || (!isSmoothing && !isAudio && !gradient))
    {
        BasicLogger::logWarning("ConfigPanel: No module for preset save of " + paramId);
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

    if (isSmoothing)
    {
        // Smoothing preset - access via AudioSourceModule's embedded smoothing
        audio->smoothing().savePreset(presetName);
        BasicLogger::logInfo("ConfigPanel: Saved smoothing preset '" + presetName + "'");

        refreshModulePresetDropdown(paramId, audio->smoothing().presetNames());

        QMessageBox::information(this, tr("Preset Saved"),
                                 tr("Smoothing preset '%1' saved successfully.").arg(name));
    }
    else if (isAudio)
    {
        // Audio preset (includes smoothing settings)
        audio->savePreset(presetName);
        BasicLogger::logInfo("ConfigPanel: Saved audio preset '" + presetName + "'");

        refreshModulePresetDropdown(paramId, audio->presetNames());

        QMessageBox::information(this, tr("Preset Saved"),
                                 tr("Audio preset '%1' saved successfully.").arg(name));
    }
    else
    {
        // Gradient preset — the handle matched via paramPrefix
        gradient->savePreset(presetName);
        BasicLogger::logInfo("ConfigPanel: Saved gradient preset '" + presetName + "'");

        refreshModulePresetDropdown(paramId, gradient->presetNames());

        QMessageBox::information(this, tr("Preset Saved"),
                                 tr("Gradient preset '%1' saved successfully.").arg(name));
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

// =============================================================================
// Stage Previews (Phase 4 Schritt 6)
// =============================================================================

void ConfigPanel::buildStagePreviews()
{
    if (!m_visualizer)
    {
        return;
    }

    const QString vizId = m_visualizer->visualizerId();
    QSettings settings;

    // Insert previews at the top of their stage group, keeping tap order
    QMap<QString, int> insertIndex;

    auto ensureToggle = [&](const QString& groupKey) -> StagePreviewGroup* {
        auto* group = m_groups.value(groupKey, nullptr);
        if (group == nullptr)
        {
            return nullptr;
        }
        auto it = m_stagePreviews.find(groupKey);
        if (it != m_stagePreviews.end())
        {
            return &it.value();
        }

        auto* toggle = new QToolButton(group);
        toggle->setText(QStringLiteral("👁"));
        toggle->setToolTip(tr("Show/hide stage preview"));
        toggle->setCheckable(true);
        toggle->setAutoRaise(true);
        group->addHeaderWidget(toggle);

        connect(toggle, &QToolButton::toggled, this,
                [this, groupKey](bool checked) { onPreviewToggled(groupKey, checked); });

        return &m_stagePreviews.insert(groupKey, StagePreviewGroup{toggle, {}}).value();
    };

    auto addPreview = [&](const QString& groupKey, TapPreviewWidget* widget,
                          std::function<std::vector<float>()> sample,
                          const QString& visibilityParamId = {}) {
        auto* group = m_groups.value(groupKey, nullptr);
        auto* preview = ensureToggle(groupKey);
        if (group == nullptr || preview == nullptr)
        {
            delete widget;
            return;
        }
        widget->setVisible(false);  // default off; toggle applies persistence below
        group->contentLayout()->insertWidget(insertIndex[groupKey]++, widget);
        preview->entries.push_back(
            PreviewEntry{widget, std::move(sample), visibilityParamId});
    };

    // Tap points (stages 1/2 — bars or curve, declared by the visualizer)
    for (const auto& tap : m_visualizer->tapPoints())
    {
        const QString groupKey = QStringLiteral("stage:%1").arg(static_cast<int>(tap.stage));
        const auto mode = (tap.display == IVisualizer::TapDisplay::Curve)
                              ? TapPreviewWidget::Mode::Curve
                              : TapPreviewWidget::Mode::Bars;
        auto* widget = new TapPreviewWidget(mode);
        widget->setToolTip(QString::fromStdString(tap.displayName));
        addPreview(groupKey, widget, tap.sample);
    }

    // Gradient handles (stage 3 — one color strip per handle). Each strip
    // follows the visibility of its handle's "mode" parameter, so channel
    // strips disappear with their channel (e.g. Waveform channelMode).
    const QString colorKey =
        QStringLiteral("stage:%1").arg(static_cast<int>(PipelineStage::Color));
    for (const auto& handle : m_visualizer->gradients())
    {
        auto* widget = new TapPreviewWidget(TapPreviewWidget::Mode::ColorStrip);
        widget->setGradient(handle.gradient);
        widget->setToolTip(QString::fromStdString(handle.displayName));
        addPreview(colorKey, widget, {},
                   QString::fromStdString(handle.paramPrefix + "mode"));
    }

    // Restore persisted visibility (default off) — toggling fires the handler,
    // which shows the widgets, persists, and starts the timer
    for (auto it = m_stagePreviews.begin(); it != m_stagePreviews.end(); ++it)
    {
        const QString key =
            QStringLiteral("configpanel/preview/%1/%2").arg(vizId, it.key());
        if (settings.value(key, false).toBool())
        {
            it->toggle->setChecked(true);
        }
    }
}

void ConfigPanel::onPreviewToggled(const QString& groupKey, bool visible)
{
    if (!m_stagePreviews.contains(groupKey))
    {
        return;
    }

    updatePreviewVisibility();

    if (m_visualizer != nullptr)
    {
        QSettings settings;
        settings.setValue(QStringLiteral("configpanel/preview/%1/%2")
                              .arg(m_visualizer->visualizerId(), groupKey),
                          visible);
    }

    updatePreviewTimer();
}

void ConfigPanel::updatePreviewVisibility()
{
    for (auto it = m_stagePreviews.begin(); it != m_stagePreviews.end(); ++it)
    {
        const bool toggledOn = it->toggle->isChecked();
        for (auto& entry : it->entries)
        {
            bool paramVisible = true;
            if (!entry.visibilityParamId.isEmpty())
            {
                auto paramIt = m_paramWidgets.find(entry.visibilityParamId);
                if (paramIt != m_paramWidgets.end() && paramIt->container != nullptr)
                {
                    paramVisible = !paramIt->container->isHidden();
                }
            }
            entry.widget->setVisible(toggledOn && paramVisible);
        }
    }
}

void ConfigPanel::onPreviewTick()
{
    for (auto it = m_stagePreviews.begin(); it != m_stagePreviews.end(); ++it)
    {
        if (!it->toggle->isChecked())
        {
            continue;
        }
        for (auto& entry : it->entries)
        {
            if (entry.sample)
            {
                // sample() copies stage working data that the render thread
                // writes every frame — take the copy under the render mutex
                std::vector<float> data;
                {
                    QMutexLocker lock(m_renderMutex);
                    data = entry.sample();
                }
                entry.widget->setData(std::move(data));
            }
            else
            {
                // Color strip: repaints only if the gradient actually changed
                // (gradient stops are written by the GUI thread only)
                entry.widget->refreshGradient();
            }
        }
    }
}

void ConfigPanel::updatePreviewTimer()
{
    bool anyVisible = false;
    for (auto it = m_stagePreviews.begin(); it != m_stagePreviews.end(); ++it)
    {
        if (it->toggle->isChecked())
        {
            anyVisible = true;
            break;
        }
    }

    // Tap nur bei Abonnent aktiv (N7): no visible preview -> no timer
    if (anyVisible && isVisible())
    {
        m_previewTimer->start();
    }
    else
    {
        m_previewTimer->stop();
    }
}
