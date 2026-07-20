/**
 ****************************************************************************************
 * @file   MultiEffectPanel.cpp
 * @brief  Implementation of the multi-effect chain tree editor (Roadmap 5.7b)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 ****************************************************************************************
 */

#include "UI/panels/MultiEffectPanel.hpp"

#include "UI/widgets/VisualizerWidget.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"  // effectTypeKey
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMutex>
#include <QMutexLocker>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

using namespace lumi::multieffect;

namespace {

// The palette of effects that "Add" can insert (name -> default params).
struct EffectType
{
    const char* name;
    EffectParams (*make)();
};

const std::vector<EffectType>& effectPalette()
{
    static const std::vector<EffectType> kPalette = {
        {"Effect List", [] { return EffectParams{ListParams{}}; }},
        {"SuperScope", [] { return EffectParams{SuperScopeParams{}}; }},
        {"Clear", [] { return EffectParams{ClearParams{}}; }},
        {"Fadeout", [] { return EffectParams{FadeoutParams{}}; }},
        {"Invert", [] { return EffectParams{InvertParams{}}; }},
        {"Brightness", [] { return EffectParams{BrightnessParams{}}; }},
        {"Fast Brightness", [] { return EffectParams{FastBrightnessParams{}}; }},
        {"Blur", [] { return EffectParams{BlurParams{}}; }},
        {"Mirror", [] { return EffectParams{MirrorParams{}}; }},
        {"OnBeat Clear", [] { return EffectParams{OnBeatClearParams{}}; }},
        {"Colorfade", [] { return EffectParams{ColorfadeParams{}}; }},
        {"Color Modifier", [] { return EffectParams{ColorModifierParams{}}; }},
        {"Movement", [] { return EffectParams{MovementParams{}}; }},
        {"Dynamic Movement", [] { return EffectParams{DynamicMovementParams{}}; }},
        {"Blitter Feedback", [] { return EffectParams{BlitterFeedbackParams{}}; }},
        {"Roto Blitter", [] { return EffectParams{RotoBlitterParams{}}; }},
        {"Buffer Save", [] { return EffectParams{BufferSaveParams{}}; }},
        {"Custom BPM", [] { return EffectParams{CustomBpmParams{}}; }},
        {"Debug Bars", [] { return EffectParams{DebugBarsParams{}}; }},
    };
    return kPalette;
}

QColor colorFromU32(uint32_t c)
{
    return QColor(static_cast<int>((c >> 16) & 0xFF), static_cast<int>((c >> 8) & 0xFF),
                  static_cast<int>(c & 0xFF));
}
uint32_t u32FromColor(const QColor& c)
{
    return (static_cast<uint32_t>(c.red()) << 16) |
           (static_cast<uint32_t>(c.green()) << 8) | static_cast<uint32_t>(c.blue());
}

QVariantList pathToVariant(const QList<int>& path)
{
    QVariantList v;
    for (int i : path) v.append(i);
    return v;
}
QList<int> pathFromVariant(const QVariant& v)
{
    QList<int> path;
    for (const QVariant& e : v.toList()) path.append(e.toInt());
    return path;
}

} // namespace

MultiEffectPanel::MultiEffectPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "multieffect_chain", tr("Effect Chain"), parent)
{
    setupUI();
    connectToActiveVisualizer();
}

void MultiEffectPanel::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 4, 4, 4);

    m_hint = new QLabel(tr("Select the \"Multi Effect\" visualizer to edit its chain."),
                        this);
    m_hint->setWordWrap(true);
    root->addWidget(m_hint);

    // Toolbar: add-type combo + add / remove / up / down.
    auto* toolbar = new QHBoxLayout();
    m_addTypeCombo = new QComboBox(this);
    for (const EffectType& t : effectPalette()) m_addTypeCombo->addItem(t.name);
    toolbar->addWidget(m_addTypeCombo, 1);

    auto makeButton = [this](const QString& text, const QString& tip) {
        auto* b = new QToolButton(this);
        b->setText(text);
        b->setToolTip(tip);
        return b;
    };
    m_addButton = makeButton("+", tr("Add effect (into the selected list, else root)"));
    m_removeButton = makeButton("-", tr("Remove selected"));
    m_upButton = makeButton(QString::fromUtf8("↑"), tr("Move up"));
    m_downButton = makeButton(QString::fromUtf8("↓"), tr("Move down"));
    toolbar->addWidget(m_addButton);
    toolbar->addWidget(m_removeButton);
    toolbar->addWidget(m_upButton);
    toolbar->addWidget(m_downButton);
    root->addLayout(toolbar);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderLabels({tr("Name"), tr("Type"), tr("Description")});
    m_tree->setColumnCount(3);
    m_tree->setRootIsDecorated(true);
    m_tree->setEditTriggers(QAbstractItemView::DoubleClicked |
                            QAbstractItemView::EditKeyPressed);
    root->addWidget(m_tree, 1);

    m_propScroll = new QScrollArea(this);
    m_propScroll->setWidgetResizable(true);
    m_propContainer = new QWidget();
    m_propLayout = new QVBoxLayout(m_propContainer);
    m_propLayout->setAlignment(Qt::AlignTop);
    m_propScroll->setWidget(m_propContainer);
    root->addWidget(m_propScroll, 1);

    connect(m_addButton, &QToolButton::clicked, this, &MultiEffectPanel::onAddEffect);
    connect(m_removeButton, &QToolButton::clicked, this, &MultiEffectPanel::onRemove);
    connect(m_upButton, &QToolButton::clicked, this, [this] { onMove(-1); });
    connect(m_downButton, &QToolButton::clicked, this, [this] { onMove(1); });
    connect(m_tree, &QTreeWidget::itemChanged, this, &MultiEffectPanel::onItemChanged);
    connect(m_tree, &QTreeWidget::itemSelectionChanged, this,
            &MultiEffectPanel::onSelectionChanged);
}

void MultiEffectPanel::connectToActiveVisualizer()
{
    // Find the current visualizer through the widget hierarchy (like ConfigPanel).
    if (auto* window = this->window())
    {
        if (auto* viz = window->findChild<VisualizerWidget*>())
        {
            setHost(dynamic_cast<MultiEffectVisualizer*>(viz->visualizer()),
                    &viz->renderMutex());
        }
    }
    if (auto* bus = eventBus())
    {
        m_eventSubscriptions.push_back(bus->subscribeScoped<VisualizerChangedEvent>(
            [this](const VisualizerChangedEvent& e) {
                setHost(dynamic_cast<MultiEffectVisualizer*>(
                            static_cast<IVisualizer*>(e.visualizerPtr)),
                        e.renderMutex);
            }));
        // The chain was replaced externally (AVS import / preset load) — rebuild.
        m_eventSubscriptions.push_back(bus->subscribeScoped<EffectChainChangedEvent>(
            [this](const EffectChainChangedEvent&) {
                rebuildTree();
                clearPropertyEditor();
            }));
    }
}

void MultiEffectPanel::setHost(MultiEffectVisualizer* host, QMutex* mutex)
{
    m_host = host;
    m_mutex = mutex;
    const bool active = m_host != nullptr;
    m_hint->setVisible(!active);
    m_tree->setEnabled(active);
    m_addTypeCombo->setEnabled(active);
    m_addButton->setEnabled(active);
    m_removeButton->setEnabled(active);
    m_upButton->setEnabled(active);
    m_downButton->setEnabled(active);
    rebuildTree();
    clearPropertyEditor();
}

// =============================================================================
// Tree <-> chain
// =============================================================================

void MultiEffectPanel::rebuildTree()
{
    m_updating = true;
    m_tree->clear();
    if (m_host != nullptr && m_mutex != nullptr)
    {
        QMutexLocker lock(m_mutex);
        const ChainNode& root = m_host->chain();
        for (int i = 0; i < static_cast<int>(root.children.size()); ++i)
        {
            addTreeItem(nullptr, root.children[i], {i});
        }
    }
    m_tree->expandAll();
    m_updating = false;
}

void MultiEffectPanel::addTreeItem(QTreeWidgetItem* parentItem, const ChainNode& node,
                                   QList<int> path)
{
    auto* item = new QTreeWidgetItem();
    item->setText(0, QString::fromStdString(
                         node.displayName.empty() ? effectTypeName(node.params)
                                                  : node.displayName));
    item->setText(1, QString::fromStdString(effectTypeName(node.params)));
    item->setText(2, QString::fromStdString(node.description));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
    item->setCheckState(0, node.enabled ? Qt::Checked : Qt::Unchecked);
    item->setData(0, Qt::UserRole, pathToVariant(path));

    if (parentItem != nullptr) parentItem->addChild(item);
    else m_tree->addTopLevelItem(item);

    if (node.isList())
    {
        for (int i = 0; i < static_cast<int>(node.children.size()); ++i)
        {
            QList<int> childPath = path;
            childPath.append(i);
            addTreeItem(item, node.children[i], childPath);
        }
    }
}

ChainNode* MultiEffectPanel::nodeAtPath(const QList<int>& path)
{
    if (m_host == nullptr) return nullptr;
    ChainNode* node = &m_host->chain();
    for (int idx : path)
    {
        if (idx < 0 || idx >= static_cast<int>(node->children.size())) return nullptr;
        node = &node->children[static_cast<size_t>(idx)];
    }
    return node;
}

QList<int> MultiEffectPanel::currentPath() const
{
    auto* item = m_tree->currentItem();
    if (item == nullptr) return {};
    return pathFromVariant(item->data(0, Qt::UserRole));
}

// =============================================================================
// Mutations
// =============================================================================

void MultiEffectPanel::mutate(const QList<int>& path,
                              const std::function<void(ChainNode&)>& fn)
{
    if (m_host == nullptr || m_mutex == nullptr) return;
    {
        QMutexLocker lock(m_mutex);
        ChainNode* node = nodeAtPath(path);
        if (node == nullptr) return;
        fn(*node);
        m_host->recompileChain();
    }
    // Refresh the affected item's label (name/clamps may have changed).
    if (auto* item = m_tree->currentItem())
    {
        QMutexLocker lock(m_mutex);
        if (ChainNode* node = nodeAtPath(path))
        {
            m_updating = true;
            item->setText(0, QString::fromStdString(
                                 node->displayName.empty()
                                     ? effectTypeName(node->params)
                                     : node->displayName));
            m_updating = false;
        }
    }
}

void MultiEffectPanel::mutateStructure(const std::function<void()>& fn)
{
    if (m_host == nullptr || m_mutex == nullptr) return;
    {
        QMutexLocker lock(m_mutex);
        fn();
        m_host->recompileChain();
    }
    rebuildTree();
    clearPropertyEditor();
}

void MultiEffectPanel::onAddEffect()
{
    if (m_host == nullptr) return;
    const int typeIdx = m_addTypeCombo->currentIndex();
    if (typeIdx < 0) return;
    ChainNode fresh;
    fresh.params = effectPalette()[static_cast<size_t>(typeIdx)].make();

    const QList<int> sel = currentPath();
    mutateStructure([&] {
        ChainNode* target = nodeAtPath(sel);
        // Add into the selected list; otherwise append to the root list.
        if (target != nullptr && target->isList())
        {
            target->children.push_back(std::move(fresh));
        }
        else
        {
            m_host->chain().children.push_back(std::move(fresh));
        }
    });
}

void MultiEffectPanel::onRemove()
{
    const QList<int> path = currentPath();
    if (path.isEmpty()) return;  // never remove the root
    mutateStructure([&] {
        QList<int> parentPath = path;
        const int idx = parentPath.takeLast();
        ChainNode* parent = nodeAtPath(parentPath);
        if (parent != nullptr && idx >= 0 &&
            idx < static_cast<int>(parent->children.size()))
        {
            parent->children.erase(parent->children.begin() + idx);
        }
    });
}

void MultiEffectPanel::onMove(int delta)
{
    const QList<int> path = currentPath();
    if (path.isEmpty()) return;
    mutateStructure([&] {
        QList<int> parentPath = path;
        const int idx = parentPath.takeLast();
        ChainNode* parent = nodeAtPath(parentPath);
        if (parent == nullptr) return;
        const int target = idx + delta;
        if (idx < 0 || idx >= static_cast<int>(parent->children.size()) || target < 0 ||
            target >= static_cast<int>(parent->children.size()))
        {
            return;
        }
        std::swap(parent->children[static_cast<size_t>(idx)],
                  parent->children[static_cast<size_t>(target)]);
    });
}

void MultiEffectPanel::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_updating || item == nullptr) return;
    const QList<int> path = pathFromVariant(item->data(0, Qt::UserRole));
    if (column == 0)
    {
        // Column 0 carries both the enable checkbox and the (editable) name.
        const bool enabled = item->checkState(0) == Qt::Checked;
        const std::string name = item->text(0).toStdString();
        mutate(path, [&](ChainNode& n) {
            n.enabled = enabled;
            n.displayName = name;
        });
    }
    else if (column == 2)
    {
        const std::string desc = item->text(2).toStdString();
        mutate(path, [&](ChainNode& n) { n.description = desc; });
    }
}

void MultiEffectPanel::onSelectionChanged()
{
    buildPropertyEditor(currentPath());
}

// =============================================================================
// Property editor
// =============================================================================

void MultiEffectPanel::clearPropertyEditor()
{
    // Delete the whole editor page at once — deleting just layout items left the
    // nested QFormLayout's widgets alive (they stacked up → overlap + lag).
    delete m_propPage;
    m_propPage = nullptr;
}

#if defined(_MSC_VER)
#pragma warning(push)
// The per-type `else if (auto* p = std::get_if<...>)` chain intentionally
// reuses `p` in mutually-exclusive branches — harmless shadowing.
#pragma warning(disable : 4456)
#endif
void MultiEffectPanel::buildPropertyEditor(const QList<int>& path)
{
    clearPropertyEditor();
    if (m_host == nullptr || path.isEmpty()) return;

    EffectParams params;
    std::string nodeName;
    std::string nodeDesc;
    {
        QMutexLocker lock(m_mutex);
        ChainNode* node = nodeAtPath(path);
        if (node == nullptr) return;
        params = node->params;
        nodeName = node->displayName;
        nodeDesc = node->description;
    }

    m_propPage = new QWidget(m_propContainer);
    auto* form = new QFormLayout(m_propPage);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    // Editable name + free-text description (commit on Enter / focus-out).
    auto addLine = [&](const QString& label, const QString& value,
                       std::function<void(ChainNode&, std::string)> set) {
        auto* edit = new QLineEdit(value, m_propPage);
        connect(edit, &QLineEdit::editingFinished, this, [this, path, set, edit]() {
            const std::string t = edit->text().toStdString();
            mutate(path, [&](ChainNode& n) { set(n, t); });
        });
        form->addRow(label, edit);
    };
    addLine(tr("Name"), QString::fromStdString(nodeName),
            [](ChainNode& n, std::string v) { n.displayName = std::move(v); });
    addLine(tr("Description"), QString::fromStdString(nodeDesc),
            [](ChainNode& n, std::string v) { n.description = std::move(v); });

    // --- row helpers (each connects to a typed mutation) ---------------------
    auto addInt = [&](const QString& label, int value, int lo, int hi,
                      std::function<void(ChainNode&, int)> set) {
        auto* spin = new QSpinBox(m_propContainer);
        spin->setRange(lo, hi);
        spin->setValue(value);
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [this, path, set](int v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        form->addRow(label, spin);
    };
    auto addDouble = [&](const QString& label, double value, double lo, double hi,
                         double step, std::function<void(ChainNode&, double)> set) {
        auto* spin = new QDoubleSpinBox(m_propContainer);
        spin->setRange(lo, hi);
        spin->setSingleStep(step);
        spin->setDecimals(3);
        spin->setValue(value);
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this, path, set](double v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        form->addRow(label, spin);
    };
    auto addBool = [&](const QString& label, bool value,
                       std::function<void(ChainNode&, bool)> set) {
        auto* box = new QCheckBox(m_propContainer);
        box->setChecked(value);
        connect(box, &QCheckBox::toggled, this, [this, path, set](bool v) {
            mutate(path, [&](ChainNode& n) { set(n, v); });
        });
        form->addRow(label, box);
    };
    auto addColor = [&](const QString& label, uint32_t value,
                        std::function<void(ChainNode&, uint32_t)> set) {
        auto* btn = new QPushButton(m_propContainer);
        auto paint = [btn](uint32_t c) {
            btn->setText(QString("#%1").arg(c, 6, 16, QChar('0')).toUpper());
            btn->setStyleSheet(QString("background:%1").arg(colorFromU32(c).name()));
        };
        paint(value);
        connect(btn, &QPushButton::clicked, this, [this, path, set, paint, value]() {
            const QColor picked = QColorDialog::getColor(colorFromU32(value), this);
            if (!picked.isValid()) return;
            const uint32_t c = u32FromColor(picked);
            paint(c);
            mutate(path, [&](ChainNode& n) { set(n, c); });
        });
        form->addRow(label, btn);
    };
    auto addEnum = [&](const QString& label, int index, const QStringList& items,
                       std::function<void(ChainNode&, int)> set) {
        auto* combo = new QComboBox(m_propContainer);
        combo->addItems(items);
        combo->setCurrentIndex(index);
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this, path, set](int v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        form->addRow(label, combo);
    };
    auto addScript = [&](const QString& label, const std::string& value,
                         std::function<void(ChainNode&, std::string)> set) {
        auto* edit = new QPlainTextEdit(m_propContainer);
        edit->setPlainText(QString::fromStdString(value));
        edit->setPlaceholderText(tr("EEL expression"));
        edit->setMaximumHeight(70);
        connect(edit, &QPlainTextEdit::textChanged, this, [this, path, set, edit]() {
            const std::string text = edit->toPlainText().toStdString();
            mutate(path, [&](ChainNode& n) { set(n, text); });
        });
        auto* box = new QWidget(m_propContainer);
        auto* v = new QVBoxLayout(box);
        v->setContentsMargins(0, 0, 0, 0);
        v->addWidget(new QLabel(label, box));
        v->addWidget(edit);
        form->addRow(box);
    };

    const QStringList kBlendNames = {"Ignore", "Replace", "50/50", "Maximum",
                                     "Additive", "Sub 1-2", "Sub 2-1", "EveryLine",
                                     "EveryPixel", "XOR", "Adjustable", "Multiply",
                                     "Buffer", "Minimum"};

    // --- per-type fields -----------------------------------------------------
    if (auto* p = std::get_if<ListParams>(&params))
    {
        addBool(tr("Clear each frame"), p->clearEveryFrame,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).clearEveryFrame = v; });
        addEnum(tr("Blend In"), static_cast<int>(p->blendIn), kBlendNames,
                [](ChainNode& n, int v) { std::get<ListParams>(n.params).blendIn = static_cast<BlendMode>(v); });
        addEnum(tr("Blend Out"), static_cast<int>(p->blendOut), kBlendNames,
                [](ChainNode& n, int v) { std::get<ListParams>(n.params).blendOut = static_cast<BlendMode>(v); });
        addInt(tr("In Alpha"), p->inAdjustAlpha, 0, 255,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).inAdjustAlpha = v; });
        addInt(tr("Out Alpha"), p->outAdjustAlpha, 0, 255,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).outAdjustAlpha = v; });
        addBool(tr("OnBeat render"), p->onBeatRender,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).onBeatRender = v; });
        addInt(tr("OnBeat frames"), p->onBeatFrames, 1, 200,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).onBeatFrames = v; });
        addBool(tr("Use list code"), p->useCode,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).useCode = v; });
        addScript(tr("Init code"), p->initCode,
                  [](ChainNode& n, std::string v) { std::get<ListParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode,
                  [](ChainNode& n, std::string v) { std::get<ListParams>(n.params).frameCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ClearParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<ClearParams>(n.params).color = v; });
        addBool(tr("Only first frame"), p->onlyFirst, [](ChainNode& n, bool v) { std::get<ClearParams>(n.params).onlyFirst = v; });
    }
    else if (auto* p = std::get_if<FadeoutParams>(&params))
    {
        addInt(tr("Fade length"), p->fadeLen, 0, 92, [](ChainNode& n, int v) { std::get<FadeoutParams>(n.params).fadeLen = v; });
        addColor(tr("Target color"), p->color, [](ChainNode& n, uint32_t v) { std::get<FadeoutParams>(n.params).color = v; });
    }
    else if (std::holds_alternative<InvertParams>(params))
    {
        form->addRow(new QLabel(tr("(no parameters)"), m_propContainer));
    }
    else if (auto* p = std::get_if<BrightnessParams>(&params))
    {
        addInt(tr("Red"), p->red, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).red = v; });
        addInt(tr("Green"), p->green, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).green = v; });
        addInt(tr("Blue"), p->blue, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).blue = v; });
        addBool(tr("Exclude color"), p->exclude, [](ChainNode& n, bool v) { std::get<BrightnessParams>(n.params).exclude = v; });
        addColor(tr("Exclude"), p->color, [](ChainNode& n, uint32_t v) { std::get<BrightnessParams>(n.params).color = v; });
        addInt(tr("Distance"), p->distance, 0, 255, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).distance = v; });
    }
    else if (auto* p = std::get_if<FastBrightnessParams>(&params))
    {
        addEnum(tr("Mode"), p->dir, {"x2", "x0.5", "off"}, [](ChainNode& n, int v) { std::get<FastBrightnessParams>(n.params).dir = v; });
    }
    else if (auto* p = std::get_if<BlurParams>(&params))
    {
        addEnum(tr("Strength"), p->strength - 1, {"Light", "Normal", "Heavy"},
                [](ChainNode& n, int v) { std::get<BlurParams>(n.params).strength = v + 1; });
    }
    else if (auto* p = std::get_if<MirrorParams>(&params))
    {
        addBool(tr("Left -> Right"), p->leftToRight, [](ChainNode& n, bool v) { std::get<MirrorParams>(n.params).leftToRight = v; });
        addBool(tr("Top -> Bottom"), p->topToBottom, [](ChainNode& n, bool v) { std::get<MirrorParams>(n.params).topToBottom = v; });
        addBool(tr("OnBeat random"), p->onBeatRandom, [](ChainNode& n, bool v) { std::get<MirrorParams>(n.params).onBeatRandom = v; });
    }
    else if (auto* p = std::get_if<OnBeatClearParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<OnBeatClearParams>(n.params).color = v; });
        addInt(tr("Every N beats"), p->everyNBeats, 1, 100, [](ChainNode& n, int v) { std::get<OnBeatClearParams>(n.params).everyNBeats = v; });
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<OnBeatClearParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<ColorfadeParams>(&params))
    {
        addInt(tr("Fader R"), p->faderR, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderR = v; });
        addInt(tr("Fader G"), p->faderG, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderG = v; });
        addInt(tr("Fader B"), p->faderB, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderB = v; });
        addInt(tr("Beat Fader R"), p->beatFaderR, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderR = v; });
        addInt(tr("Beat Fader G"), p->beatFaderG, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderG = v; });
        addInt(tr("Beat Fader B"), p->beatFaderB, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderB = v; });
    }
    else if (auto* p = std::get_if<ColorModifierParams>(&params))
    {
        addBool(tr("Recompute/frame"), p->recompute, [](ChainNode& n, bool v) { std::get<ColorModifierParams>(n.params).recompute = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Level"), p->levelCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).levelCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MovementParams>(&params))
    {
        addBool(tr("Rect coords"), p->rectCoords, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).rectCoords = v; });
        addBool(tr("Wrap"), p->wrap, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).wrap = v; });
        addScript(tr("Point code"), p->code, [](ChainNode& n, std::string v) { std::get<MovementParams>(n.params).code = std::move(v); });
    }
    else if (auto* p = std::get_if<DynamicMovementParams>(&params))
    {
        addInt(tr("Grid X"), p->xres, 2, 96, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).xres = v; });
        addInt(tr("Grid Y"), p->yres, 2, 72, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).yres = v; });
        addBool(tr("Rect coords"), p->rectCoords, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).rectCoords = v; });
        addBool(tr("Wrap"), p->wrap, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).wrap = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Point"), p->pointCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BlitterFeedbackParams>(&params))
    {
        addDouble(tr("Zoom"), p->zoom, 0.5, 2.0, 0.01, [](ChainNode& n, double v) { std::get<BlitterFeedbackParams>(n.params).zoom = static_cast<float>(v); });
        addDouble(tr("Beat zoom"), p->beatZoom, 0.5, 2.0, 0.01, [](ChainNode& n, double v) { std::get<BlitterFeedbackParams>(n.params).beatZoom = static_cast<float>(v); });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).onBeat = v; });
        addBool(tr("Blend"), p->blend, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<RotoBlitterParams>(&params))
    {
        addDouble(tr("Zoom"), p->zoom, 0.5, 2.0, 0.01, [](ChainNode& n, double v) { std::get<RotoBlitterParams>(n.params).zoom = static_cast<float>(v); });
        addDouble(tr("Rotation/frame"), p->rotationSpeed, -10.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<RotoBlitterParams>(n.params).rotationSpeed = static_cast<float>(v); });
        addBool(tr("Blend"), p->blend, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<BufferSaveParams>(&params))
    {
        addInt(tr("Buffer slot"), p->slot, 0, 7, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).slot = v; });
        addBool(tr("Save (else restore)"), p->save, [](ChainNode& n, bool v) { std::get<BufferSaveParams>(n.params).save = v; });
        addEnum(tr("Restore blend"), static_cast<int>(p->blend), kBlendNames, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).blend = static_cast<BlendMode>(v); });
        addInt(tr("Restore alpha"), p->adjustAlpha, 0, 255, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).adjustAlpha = v; });
    }
    else if (auto* p = std::get_if<CustomBpmParams>(&params))
    {
        addBool(tr("Arbitrary"), p->arbitrary, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).arbitrary = v; });
        addInt(tr("Interval (ms)"), p->arbitraryMs, 1, 5000, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).arbitraryMs = v; });
        addBool(tr("Skip"), p->skip, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).skip = v; });
        addInt(tr("Skip count"), p->skipCount, 1, 16, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).skipCount = v; });
        addBool(tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).invert = v; });
    }
    else if (auto* p = std::get_if<SuperScopeParams>(&params))
    {
        addInt(tr("Point count"), p->pointCount, 1, 4096, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).pointCount = v; });
        addEnum(tr("Draw mode"), p->renderMode, {"Dots", "Lines", "Thick"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).renderMode = v; });
        addDouble(tr("Line width"), p->lineWidth, 1.0, 20.0, 0.5, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).lineWidth = static_cast<float>(v); });
        addDouble(tr("Dot size"), p->dotSize, 1.0, 50.0, 1.0, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum(tr("Channel"), p->audioChannel, {"Left", "Right", "Mono", "Mid", "Side"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).audioChannel = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point"), p->pointCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DebugBarsParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<DebugBarsParams>(n.params).color = v; });
        addDouble(tr("Orbit speed"), p->orbitSpeed, -10.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<DebugBarsParams>(n.params).orbitSpeed = static_cast<float>(v); });
    }
    else if (auto* p = std::get_if<PassthroughParams>(&params))
    {
        form->addRow(new QLabel(
            tr("Conserved effect (id %1): %2")
                .arg(p->sourceId)
                .arg(QString::fromStdString(p->note)),
            m_propContainer));
    }

    m_propLayout->addWidget(m_propPage);
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
