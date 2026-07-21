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

#include "UI/widgets/GradientPresetDelegate.hpp"          // gradient combo previews
#include "UI/widgets/VisualizerWidget.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"      // figure preset library (SSOT)
#include "visualizers/multieffect/ChainSerializer.hpp"  // effectTypeKey
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QColorDialog>
#include <QDropEvent>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
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

#include <algorithm>
#include <utility>

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
        {"Dynamic Shift", [] { return EffectParams{DynamicShiftParams{}}; }},
        {"Dynamic Distance Modifier", [] { return EffectParams{DynamicDistanceModifierParams{}}; }},
        {"Moving Particle", [] { return EffectParams{MovingParticleParams{}}; }},
        {"Blitter Feedback", [] { return EffectParams{BlitterFeedbackParams{}}; }},
        {"Roto Blitter", [] { return EffectParams{RotoBlitterParams{}}; }},
        {"Buffer Save", [] { return EffectParams{BufferSaveParams{}}; }},
        {"Custom BPM", [] { return EffectParams{CustomBpmParams{}}; }},
        {"Mosaic", [] { return EffectParams{MosaicParams{}}; }},
        {"Grain", [] { return EffectParams{GrainParams{}}; }},
        {"Scatter", [] { return EffectParams{ScatterParams{}}; }},
        {"Water", [] { return EffectParams{WaterParams{}}; }},
        {"Bump", [] { return EffectParams{BumpParams{}}; }},
        {"Water Bump", [] { return EffectParams{WaterBumpParams{}}; }},
        {"Starfield", [] { return EffectParams{StarfieldParams{}}; }},
        {"Timescope", [] { return EffectParams{TimescopeParams{}}; }},
        {"Dot Grid", [] { return EffectParams{DotGridParams{}}; }},
        {"Dot Plane", [] { return EffectParams{DotPlaneParams{}}; }},
        {"Dot Fountain", [] { return EffectParams{DotFountainParams{}}; }},
        {"Channel Shift", [] { return EffectParams{ChannelShiftParams{}}; }},
        {"Color Reduction", [] { return EffectParams{ColorReductionParams{}}; }},
        {"Multiplier", [] { return EffectParams{MultiplierParams{}}; }},
        {"Video Delay", [] { return EffectParams{VideoDelayParams{}}; }},
        {"Multi Delay", [] { return EffectParams{MultiDelayParams{}}; }},
        {"Interferences", [] { return EffectParams{InterferencesParams{}}; }},
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

// The SuperScope figure presets offered in the editor dropdown. The code lives
// in SuperscopeModule::loadPresetCode (single source of truth); we only pick the
// shape figures here (Custom = no-op, DNA = native-only/empty code are skipped).
const std::vector<lumi::modules::SuperscopePreset>& superscopeFigures()
{
    using P = lumi::modules::SuperscopePreset;
    static const std::vector<P> kList = {
        P::HorizontalScope, P::VerticalScope,   P::Circle,     P::Spiral,
        P::Lissajous,       P::Flower,          P::Star,       P::Starburst,
        P::Heart,           P::SpectrumBars,    P::CircularSpectrum,
        P::Butterfly,       P::Hypocycloid};
    return kList;
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

void ChainTreeWidget::dropEvent(QDropEvent* event)
{
    QTreeWidgetItem* target = itemAt(event->position().toPoint());
    ChainDrop where = ChainDrop::Viewport;
    switch (dropIndicatorPosition())  // protected enum: only readable in-subclass
    {
        case QAbstractItemView::OnItem:     where = ChainDrop::OnItem; break;
        case QAbstractItemView::AboveItem:  where = ChainDrop::Above;  break;
        case QAbstractItemView::BelowItem:  where = ChainDrop::Below;  break;
        case QAbstractItemView::OnViewport: where = ChainDrop::Viewport; break;
    }
    QTreeWidgetItem* src = currentItem();  // single-selection: dragged == current
    if (onDrop && src != nullptr)
    {
        onDrop(src, target, where);
    }
    // Consume without calling the base: the chain owns ordering and the tree is
    // rebuilt from it, so the view must not move items on its own. IgnoreAction
    // (not MoveAction) stops QAbstractItemView::startDrag from also deleting the
    // source rows after our rebuild already replaced every item.
    event->setDropAction(Qt::IgnoreAction);
    event->accept();
}

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
    m_cloneButton = makeButton(QString::fromUtf8("⧉"), tr("Clone selected (with its subtree)"));
    m_upButton = makeButton(QString::fromUtf8("↑"), tr("Move up"));
    m_downButton = makeButton(QString::fromUtf8("↓"), tr("Move down"));
    toolbar->addWidget(m_addButton);
    toolbar->addWidget(m_removeButton);
    toolbar->addWidget(m_cloneButton);
    toolbar->addWidget(m_upButton);
    toolbar->addWidget(m_downButton);
    root->addLayout(toolbar);

    m_tree = new ChainTreeWidget(this);
    m_tree->setHeaderLabels({tr("Name"), QString(), tr("Type"), tr("Description")});
    m_tree->setColumnCount(4);
    m_tree->setColumnWidth(1, 30);  // narrow eye toggle column (col 1)
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_tree->header()->setStretchLastSection(true);
    // Column 0 (Name) is the tree column: show the expand decoration + a clear
    // per-level indent so nested Effect Lists read as nested across levels.
    m_tree->setRootIsDecorated(true);
    m_tree->setIndentation(18);
    m_tree->setEditTriggers(QAbstractItemView::DoubleClicked |
                            QAbstractItemView::EditKeyPressed);
    // Drag & drop re-parents/reorders effects (into / out of groups). We handle
    // the drop ourselves (onDrop) and rebuild from the model.
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setDragEnabled(true);
    m_tree->setAcceptDrops(true);
    m_tree->setDropIndicatorShown(true);
    m_tree->setDragDropMode(QAbstractItemView::InternalMove);
    m_tree->onDrop = [this](QTreeWidgetItem* s, QTreeWidgetItem* t, ChainDrop w) {
        onDropRequested(s, t, w);
    };
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
    connect(m_cloneButton, &QToolButton::clicked, this, &MultiEffectPanel::onClone);
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
    m_cloneButton->setEnabled(active);
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
    // Column 0 = name (the tree column: gets the expand arrow + per-level indent,
    // so nesting stays visible); 1 = eye toggle (widget); 2 = type; 3 = desc.
    item->setText(0, QString::fromStdString(
                         node.displayName.empty() ? effectTypeName(node.params)
                                                  : node.displayName));
    item->setText(2, QString::fromStdString(effectTypeName(node.params)));
    item->setText(3, QString::fromStdString(node.description));
    item->setFlags((item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
                    Qt::ItemIsDropEnabled));
    item->setData(0, Qt::UserRole, pathToVariant(path));

    if (parentItem != nullptr) parentItem->addChild(item);
    else m_tree->addTopLevelItem(item);

    // Eye toggle (hide/show like the stage previews) in its own narrow column so
    // it never fights the tree indentation. Toggling `enabled` is not structural,
    // so no rebuild is needed and the captured path stays valid.
    auto* eye = new QToolButton(m_tree);
    eye->setCheckable(true);
    eye->setAutoRaise(true);
    eye->setChecked(node.enabled);
    eye->setText(node.enabled ? QString::fromUtf8("\xF0\x9F\x91\x81")   // 👁
                              : QString::fromUtf8("\xE2\x80\x94"));      // —
    eye->setToolTip(tr("Show / hide this effect"));
    connect(eye, &QToolButton::toggled, this, [this, eye, path](bool on) {
        eye->setText(on ? QString::fromUtf8("\xF0\x9F\x91\x81")
                        : QString::fromUtf8("\xE2\x80\x94"));
        setNodeEnabled(path, on);
    });
    m_tree->setItemWidget(item, 1, eye);

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

void MultiEffectPanel::onClone()
{
    const QList<int> path = currentPath();
    if (path.isEmpty()) return;  // root is not clonable
    QList<int> finalPath;
    mutateStructure([&] {
        QList<int> parentPath = path;
        const int idx = parentPath.takeLast();
        ChainNode* parent = nodeAtPath(parentPath);
        if (parent == nullptr || idx < 0 ||
            idx >= static_cast<int>(parent->children.size()))
        {
            return;
        }
        ChainNode copy = parent->children[static_cast<size_t>(idx)];  // deep copy
        // Fresh identities: compileChain only assigns to nodeId==0, so a copied
        // (non-zero) id would collide with the original -> zero the whole subtree.
        std::function<void(ChainNode&)> clearIds = [&](ChainNode& n) {
            n.nodeId = 0;
            for (ChainNode& c : n.children) clearIds(c);
        };
        clearIds(copy);
        parent->children.insert(parent->children.begin() + idx + 1, std::move(copy));
        finalPath = parentPath;
        finalPath.append(idx + 1);
    });
    if (!finalPath.isEmpty()) selectByPath(finalPath);
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

void MultiEffectPanel::setNodeEnabled(const QList<int>& path, bool enabled)
{
    if (m_host == nullptr || m_mutex == nullptr) return;
    QMutexLocker lock(m_mutex);
    if (ChainNode* node = nodeAtPath(path))
    {
        node->enabled = enabled;
        m_host->recompileChain();  // no structural change -> paths stay valid
    }
}

void MultiEffectPanel::applySuperScopePreset(const QList<int>& path, int presetIndex)
{
    const auto& figures = superscopeFigures();
    if (presetIndex < 0 || presetIndex >= static_cast<int>(figures.size())) return;
    if (m_host == nullptr || m_mutex == nullptr) return;

    // Reuse the standalone module's preset library (single source of truth).
    lumi::modules::SuperscopeModule tmp;
    tmp.loadPresetCode(figures[static_cast<std::size_t>(presetIndex)]);
    const std::string init = tmp.initCode();
    const std::string frame = tmp.frameCode();
    const std::string beat = tmp.beatCode();
    const std::string point = tmp.pointCode();
    const int count = tmp.pointCount();

    QMutexLocker lock(m_mutex);
    ChainNode* node = nodeAtPath(path);
    if (node == nullptr) return;
    if (auto* p = std::get_if<SuperScopeParams>(&node->params))
    {
        p->initCode = init;
        p->frameCode = frame;
        p->beatCode = beat;
        p->pointCode = point;
        p->pointCount = count;
        m_host->recompileChain();
    }
}

// --- Drag & drop re-parenting ------------------------------------------------

void MultiEffectPanel::onDropRequested(QTreeWidgetItem* src, QTreeWidgetItem* target,
                                       ChainDrop where)
{
    if (src == nullptr) return;
    const QList<int> srcPath = pathFromVariant(src->data(0, Qt::UserRole));
    if (srcPath.isEmpty()) return;
    QList<int> targetPath;
    if (target != nullptr) targetPath = pathFromVariant(target->data(0, Qt::UserRole));

    // Defer the actual move: we are inside the tree's own dropEvent, and the
    // rebuild would delete the items the drag machinery still touches. Paths are
    // value copies, so they survive the queued hop.
    QMetaObject::invokeMethod(
        this,
        [this, srcPath, targetPath, where] {
            QList<int> finalPath;
            bool moved = false;
            mutateStructure(
                [&] { moved = moveNodeLocked(srcPath, targetPath, where, finalPath); });
            if (moved) selectByPath(finalPath);
        },
        Qt::QueuedConnection);
}

bool MultiEffectPanel::moveNodeLocked(const QList<int>& srcPath,
                                      const QList<int>& targetPath, ChainDrop where,
                                      QList<int>& finalPath)
{
    if (m_host == nullptr || srcPath.isEmpty()) return false;

    QList<int> srcParentPath = srcPath;
    const int srcIdx = srcParentPath.takeLast();

    // --- Resolve the destination (parent path + insertion index) -------------
    QList<int> dstParentPath;
    int dstIndex = 0;
    if (targetPath.isEmpty() || where == ChainDrop::Viewport)
    {
        dstParentPath = {};  // dropped on empty space -> end of root
        dstIndex = static_cast<int>(m_host->chain().children.size());
    }
    else
    {
        QList<int> targetParentPath = targetPath;
        const int targetIdx = targetParentPath.takeLast();
        if (where == ChainDrop::OnItem)
        {
            ChainNode* tnode = nodeAtPath(targetPath);
            if (tnode != nullptr && tnode->isList())
            {
                dstParentPath = targetPath;  // drop INTO the group (append)
                dstIndex = static_cast<int>(tnode->children.size());
            }
            else  // onto a leaf -> place right after it
            {
                dstParentPath = targetParentPath;
                dstIndex = targetIdx + 1;
            }
        }
        else if (where == ChainDrop::Above)
        {
            dstParentPath = targetParentPath;
            dstIndex = targetIdx;
        }
        else  // ChainDrop::Below
        {
            dstParentPath = targetParentPath;
            dstIndex = targetIdx + 1;
        }
    }

    // --- Guards: no self-drop / into own subtree, and no no-op ---------------
    const int sp = static_cast<int>(srcParentPath.size());
    const bool intoSubtree = dstParentPath.size() >= srcPath.size() &&
                             dstParentPath.mid(0, srcPath.size()) == srcPath;
    if (dstParentPath == srcPath || intoSubtree) return false;
    if (dstParentPath == srcParentPath && (dstIndex == srcIdx || dstIndex == srcIdx + 1))
    {
        return false;  // dropped back onto its own slot
    }

    // --- Extract from source, then insert (adjust dst for the removal) -------
    ChainNode* srcParent = nodeAtPath(srcParentPath);
    if (srcParent == nullptr || srcIdx < 0 ||
        srcIdx >= static_cast<int>(srcParent->children.size()))
    {
        return false;
    }
    ChainNode moved = std::move(srcParent->children[static_cast<size_t>(srcIdx)]);
    srcParent->children.erase(srcParent->children.begin() + srcIdx);

    // The erase shifts indices at/after srcIdx within srcParent's vector.
    if (dstParentPath.size() > sp && dstParentPath.mid(0, sp) == srcParentPath &&
        dstParentPath[sp] > srcIdx)
    {
        dstParentPath[sp] -= 1;  // dst lives under a sibling that shifted down
    }
    else if (dstParentPath == srcParentPath && dstIndex > srcIdx)
    {
        dstIndex -= 1;
    }

    ChainNode* dstParent = nodeAtPath(dstParentPath);
    if (dstParent == nullptr)
    {
        // Should not happen after the guards; fall back to root append.
        dstParent = &m_host->chain();
        dstParentPath = {};
        dstIndex = static_cast<int>(dstParent->children.size());
    }
    dstIndex = std::clamp(dstIndex, 0, static_cast<int>(dstParent->children.size()));
    dstParent->children.insert(dstParent->children.begin() + dstIndex, std::move(moved));

    finalPath = dstParentPath;
    finalPath.append(dstIndex);
    return true;
}

QTreeWidgetItem* MultiEffectPanel::itemAtPath(const QList<int>& path) const
{
    if (path.isEmpty()) return nullptr;
    QTreeWidgetItem* item = m_tree->topLevelItem(path[0]);
    for (int i = 1; i < path.size() && item != nullptr; ++i)
    {
        item = item->child(path[i]);
    }
    return item;
}

void MultiEffectPanel::selectByPath(const QList<int>& path)
{
    if (QTreeWidgetItem* item = itemAtPath(path))
    {
        m_tree->setCurrentItem(item);
        m_tree->scrollToItem(item);
    }
}

void MultiEffectPanel::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_updating || item == nullptr) return;
    const QList<int> path = pathFromVariant(item->data(0, Qt::UserRole));
    if (column == 0)
    {
        // Column 0 is the editable name (enable lives on the eye toggle now).
        const std::string name = item->text(0).toStdString();
        mutate(path, [&](ChainNode& n) { n.displayName = name; });
    }
    else if (column == 3)
    {
        const std::string desc = item->text(3).toStdString();
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
        // Only relevant when the matching blend is "Buffer": pool slot + invert.
        addInt(tr("In Buffer"), p->bufferIn, 0, 7,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).bufferIn = v; });
        addInt(tr("Out Buffer"), p->bufferOut, 0, 7,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).bufferOut = v; });
        addBool(tr("In Buffer invert"), p->bufferInInvert,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).bufferInInvert = v; });
        addBool(tr("Out Buffer invert"), p->bufferOutInvert,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).bufferOutInvert = v; });
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
    else if (auto* p = std::get_if<DynamicShiftParams>(&params))
    {
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<DynamicShiftParams>(n.params).blend = v; });
        addBool(tr("Bilinear"), p->bilinear, [](ChainNode& n, bool v) { std::get<DynamicShiftParams>(n.params).bilinear = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame (x,y shift)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DynamicDistanceModifierParams>(&params))
    {
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<DynamicDistanceModifierParams>(n.params).blend = v; });
        addBool(tr("Bilinear"), p->bilinear, [](ChainNode& n, bool v) { std::get<DynamicDistanceModifierParams>(n.params).bilinear = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Pixel (d in/out)"), p->pixelCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).pixelCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MovingParticleParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<MovingParticleParams>(n.params).color = v; });
        addInt(tr("Max distance"), p->maxDistance, 1, 256, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).maxDistance = v; });
        addInt(tr("Size"), p->size, 1, 128, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).size = v; });
        addInt(tr("On-beat size"), p->size2, 1, 128, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).size2 = v; });
        addBool(tr("Resize on beat"), p->onBeatSize, [](ChainNode& n, bool v) { std::get<MovingParticleParams>(n.params).onBeatSize = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50", "Line"}, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).blend = v; });
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
        // Figure preset dropdown: loads the shape's EEL into the code fields
        // below (source: SuperscopeModule::loadPresetCode). Index 0 is a label.
        auto* presetCombo = new QComboBox(m_propContainer);
        presetCombo->addItem(tr("— Load figure preset —"));
        for (lumi::modules::SuperscopePreset fig : superscopeFigures())
            presetCombo->addItem(lumi::modules::SuperscopeModule::presetName(fig));
        connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this, path](int idx) {
                    if (idx <= 0) return;  // the label row
                    // Defer: applying rebuilds this editor (deletes the combo).
                    QMetaObject::invokeMethod(
                        this,
                        [this, path, idx] {
                            applySuperScopePreset(path, idx - 1);
                            buildPropertyEditor(path);
                        },
                        Qt::QueuedConnection);
                });
        form->addRow(tr("Figure"), presetCombo);

        addInt(tr("Point count"), p->pointCount, 1, 4096, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).pointCount = v; });
        addEnum(tr("Draw mode"), p->renderMode, {"Dots", "Lines", "Thick"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).renderMode = v; });
        addDouble(tr("Line width"), p->lineWidth, 1.0, 20.0, 0.5, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).lineWidth = static_cast<float>(v); });
        addDouble(tr("Dot size"), p->dotSize, 1.0, 50.0, 1.0, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum(tr("Channel"), p->audioChannel, {"Left", "Right", "Mono", "Mid", "Side"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).audioChannel = v; });
        addEnum(tr("Line blend"), p->lineBlend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).lineBlend = v; });

        // --- Color: gradient (per point) x table (cycled) combined by mode ----
        // The base color pre-seeds red/green/blue before the point code, which
        // may keep, modulate (red=red*v) or override it (AVS r_sscope).
        {
            auto* modeCombo = new QComboBox(m_propContainer);
            modeCombo->addItems({tr("Gradient (per point)"), tr("Color table (cycled)"),
                                 tr("Additive (both)"), tr("Multiply (both)"),
                                 tr("Average (both)")});
            modeCombo->setCurrentIndex(std::clamp(p->colorBlend, 0, 4));
            modeCombo->setToolTip(
                tr("Base color for red/green/blue before the point code runs; the "
                   "code may keep, modulate or override it."));
            connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                    [this, path](int v) {
                        mutate(path, [&](ChainNode& n) {
                            std::get<SuperScopeParams>(n.params).colorBlend = v;
                        });
                    });
            form->addRow(tr("Color mode"), modeCombo);
        }
        {
            const std::vector<std::string> gradNames = m_gradientPreview.presetNames();
            auto* gradCombo = new QComboBox(m_propContainer);
            gradCombo->setMinimumHeight(28);  // room for the preview strip
            int curGrad = 0;
            for (int gi = 0; gi < static_cast<int>(gradNames.size()); ++gi)
            {
                gradCombo->addItem(QString::fromStdString(gradNames[static_cast<size_t>(gi)]));
                if (gradNames[static_cast<size_t>(gi)] == p->gradientPreset) curGrad = gi;
            }
            // Preview delegate: draws each preset's gradient strip beside its name.
            auto* delegate = new lumi::ui::GradientPresetDelegate(gradCombo);
            delegate->setGradientModule(&m_gradientPreview);
            gradCombo->setItemDelegate(delegate);
            gradCombo->setCurrentIndex(curGrad);
            connect(gradCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                    [this, path, gradNames](int gi) {
                        if (gi < 0 || gi >= static_cast<int>(gradNames.size())) return;
                        const std::string name = gradNames[static_cast<size_t>(gi)];
                        mutate(path, [&](ChainNode& n) {
                            std::get<SuperScopeParams>(n.params).gradientPreset = name;
                        });
                    });
            form->addRow(tr("Gradient"), gradCombo);
        }
        addInt(tr("Cycle frames"), p->colorCycleFrames, 1, 600, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).colorCycleFrames = v; });
        {
            // AVS color table: cycled over time in the "Color table"/blend modes.
            // One swatch per entry + add/remove.
            auto* colorsWidget = new QWidget(m_propContainer);
            auto* row = new QHBoxLayout(colorsWidget);
            row->setContentsMargins(0, 0, 0, 0);
            for (int ci = 0; ci < static_cast<int>(p->colors.size()); ++ci)
            {
                const uint32_t initC = p->colors[static_cast<size_t>(ci)];
                auto* sw = new QPushButton(colorsWidget);
                sw->setFixedSize(24, 20);
                sw->setStyleSheet(QString("background:%1").arg(colorFromU32(initC).name()));
                connect(sw, &QPushButton::clicked, this, [this, path, ci, initC, sw]() {
                    const QColor picked = QColorDialog::getColor(colorFromU32(initC), this);
                    if (!picked.isValid()) return;
                    const uint32_t c = u32FromColor(picked);
                    sw->setStyleSheet(QString("background:%1").arg(picked.name()));
                    mutate(path, [&](ChainNode& n) {
                        auto& cs = std::get<SuperScopeParams>(n.params).colors;
                        if (ci < static_cast<int>(cs.size())) cs[static_cast<size_t>(ci)] = c;
                    });
                });
                row->addWidget(sw);
            }
            auto* addC = new QToolButton(colorsWidget);
            addC->setText("+");
            addC->setToolTip(tr("Add color"));
            connect(addC, &QToolButton::clicked, this, [this, path]() {
                mutate(path, [](ChainNode& n) {
                    std::get<SuperScopeParams>(n.params).colors.push_back(0xFFFFFF);
                });
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
            });
            auto* delC = new QToolButton(colorsWidget);
            delC->setText(QString::fromUtf8("−"));
            delC->setToolTip(tr("Remove last color"));
            connect(delC, &QToolButton::clicked, this, [this, path]() {
                mutate(path, [](ChainNode& n) {
                    auto& cs = std::get<SuperScopeParams>(n.params).colors;
                    if (!cs.empty()) cs.pop_back();
                });
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
            });
            row->addWidget(addC);
            row->addWidget(delC);
            row->addStretch();
            colorsWidget->setToolTip(
                tr("AVS color table — the scope cycles through these over time in "
                   "the Color table / blend modes (one step every 'Cycle frames')."));
            form->addRow(tr("Color table"), colorsWidget);
        }

        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point"), p->pointCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MosaicParams>(&params))
    {
        addInt(tr("Quality"), p->quality, 1, 100, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).quality = v; });
        addInt(tr("OnBeat quality"), p->quality2, 1, 100, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).quality2 = v; });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<MosaicParams>(n.params).onBeat = v; });
        addInt(tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).durationFrames = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<GrainParams>(&params))
    {
        addInt(tr("Amount"), p->amount, 0, 100, [](ChainNode& n, int v) { std::get<GrainParams>(n.params).amount = v; });
        addBool(tr("Static"), p->staticGrain, [](ChainNode& n, bool v) { std::get<GrainParams>(n.params).staticGrain = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<GrainParams>(n.params).blend = v; });
    }
    else if (std::get_if<ScatterParams>(&params) != nullptr)
    {
        auto* info = new QLabel(tr("Per-pixel random displacement (no parameters)."), m_propPage);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (std::get_if<WaterParams>(&params) != nullptr)
    {
        auto* info = new QLabel(tr("Water ripple — neighbour average minus the previous frame (no parameters)."), m_propPage);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<BumpParams>(&params))
    {
        addInt(tr("Depth"), p->depth, 0, 100, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).depth = v; });
        addBool(tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).invert = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).blend = v; });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).onBeat = v; });
        addInt(tr("Beat depth"), p->depth2, 0, 100, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).depth2 = v; });
        addInt(tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).durationFrames = v; });
        addBool(tr("Old-style x,y (0..100)"), p->oldStyle, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).oldStyle = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame (light x,y)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<WaterBumpParams>(&params))
    {
        addInt(tr("Density (damping)"), p->density, 1, 12, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).density = v; });
        addInt(tr("Drop depth"), p->depth, 0, 2000, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).depth = v; });
        addDouble(tr("Refraction"), p->displaceScale, 0.0, 40.0, 0.5, [](ChainNode& n, double v) { std::get<WaterBumpParams>(n.params).displaceScale = static_cast<float>(v); });
        addBool(tr("Random drop"), p->randomDrop, [](ChainNode& n, bool v) { std::get<WaterBumpParams>(n.params).randomDrop = v; });
        addEnum(tr("Drop X"), p->dropX, {"Near", "Mid", "Far"}, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropX = v; });
        addEnum(tr("Drop Y"), p->dropY, {"Near", "Mid", "Far"}, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropY = v; });
        addInt(tr("Drop radius"), p->dropRadius, 1, 200, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropRadius = v; });
    }
    else if (auto* p = std::get_if<InterferencesParams>(&params))
    {
        addInt(tr("Points"), p->points, 1, 8, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).points = v; });
        addInt(tr("Distance"), p->distance, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).distance = v; });
        addInt(tr("Alpha"), p->alpha, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).alpha = v; });
        addInt(tr("Rotation"), p->rotation, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotation = v; });
        addInt(tr("Rotation/frame"), p->rotationInc, -64, 64, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotationInc = v; });
        addBool(tr("RGB split"), p->rgb, [](ChainNode& n, bool v) { std::get<InterferencesParams>(n.params).rgb = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).blend = v; });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<InterferencesParams>(n.params).onBeat = v; });
        addInt(tr("Beat distance"), p->distance2, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).distance2 = v; });
        addInt(tr("Beat alpha"), p->alpha2, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).alpha2 = v; });
        addInt(tr("Beat rotation/frame"), p->rotationInc2, -64, 64, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotationInc2 = v; });
        addDouble(tr("Beat speed"), p->speed, 0.01, 2.0, 0.01, [](ChainNode& n, double v) { std::get<InterferencesParams>(n.params).speed = static_cast<float>(v); });
    }
    else if (auto* p = std::get_if<StarfieldParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<StarfieldParams>(n.params).color = v; });
        addInt(tr("Stars"), p->maxStars, 1, 8192, [](ChainNode& n, int v) { std::get<StarfieldParams>(n.params).maxStars = v; });
        addDouble(tr("Warp speed"), p->warpSpeed, 0.1, 50.0, 0.5, [](ChainNode& n, double v) { std::get<StarfieldParams>(n.params).warpSpeed = static_cast<float>(v); });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<StarfieldParams>(n.params).onBeat = v; });
        addDouble(tr("Beat speed"), p->beatSpeed, 0.1, 50.0, 0.5, [](ChainNode& n, double v) { std::get<StarfieldParams>(n.params).beatSpeed = static_cast<float>(v); });
        addInt(tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<StarfieldParams>(n.params).durationFrames = v; });
    }
    else if (auto* p = std::get_if<TimescopeParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<TimescopeParams>(n.params).color = v; });
        addInt(tr("Bands"), p->bands, 1, 576, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).bands = v; });
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).channel = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<DotGridParams>(&params))
    {
        addInt(tr("Spacing"), p->spacing, 2, 128, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).spacing = v; });
        addInt(tr("X move"), p->xMove, -1024, 1024, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).xMove = v; });
        addInt(tr("Y move"), p->yMove, -1024, 1024, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).yMove = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).blend = v; });
        if (!p->colors.empty())
            addColor(tr("Color 1"), p->colors[0], [](ChainNode& n, uint32_t v) { std::get<DotGridParams>(n.params).colors[0] = v; });
    }
    else if (auto* p = std::get_if<DotPlaneParams>(&params))
    {
        addInt(tr("Rotation speed"), p->rotVel, -50, 50, [](ChainNode& n, int v) { std::get<DotPlaneParams>(n.params).rotVel = v; });
        addInt(tr("Angle"), p->angle, -90, 90, [](ChainNode& n, int v) { std::get<DotPlaneParams>(n.params).angle = v; });
        for (int i = 0; i < 5; ++i)
            addColor(tr("Color %1").arg(i + 1), p->colors[i], [i](ChainNode& n, uint32_t v) { std::get<DotPlaneParams>(n.params).colors[i] = v; });
    }
    else if (auto* p = std::get_if<DotFountainParams>(&params))
    {
        addInt(tr("Rotation speed"), p->rotVel, -50, 50, [](ChainNode& n, int v) { std::get<DotFountainParams>(n.params).rotVel = v; });
        addInt(tr("Angle"), p->angle, -90, 90, [](ChainNode& n, int v) { std::get<DotFountainParams>(n.params).angle = v; });
        for (int i = 0; i < 5; ++i)
            addColor(tr("Color %1").arg(i + 1), p->colors[i], [i](ChainNode& n, uint32_t v) { std::get<DotFountainParams>(n.params).colors[i] = v; });
    }
    else if (auto* p = std::get_if<ChannelShiftParams>(&params))
    {
        addEnum(tr("Mode"), p->mode, {"RGB", "RBG", "GBR", "GRB", "BRG", "BGR"}, [](ChainNode& n, int v) { std::get<ChannelShiftParams>(n.params).mode = v; });
        addBool(tr("On beat (random)"), p->onBeat, [](ChainNode& n, bool v) { std::get<ChannelShiftParams>(n.params).onBeat = v; });
    }
    else if (auto* p = std::get_if<ColorReductionParams>(&params))
    {
        addInt(tr("Levels (bits)"), p->levels, 1, 8, [](ChainNode& n, int v) { std::get<ColorReductionParams>(n.params).levels = v; });
    }
    else if (auto* p = std::get_if<MultiplierParams>(&params))
    {
        addEnum(tr("Factor"), p->mode, {"Saturate", "x8", "x4", "x2", "x0.5", "x0.25", "x0.125", "Keep"}, [](ChainNode& n, int v) { std::get<MultiplierParams>(n.params).mode = v; });
    }
    else if (auto* p = std::get_if<VideoDelayParams>(&params))
    {
        addInt(tr("Delay"), p->delay, 1, 128, [](ChainNode& n, int v) { std::get<VideoDelayParams>(n.params).delay = v; });
        addBool(tr("In beats"), p->useBeats, [](ChainNode& n, bool v) { std::get<VideoDelayParams>(n.params).useBeats = v; });
    }
    else if (auto* p = std::get_if<MultiDelayParams>(&params))
    {
        addEnum(tr("Mode"), p->mode, {"None", "Input (write)", "Output (read)"}, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).mode = v; });
        addInt(tr("Buffer"), p->buffer, 0, 5, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).buffer = v; });
        addInt(tr("Delay"), p->delay, 1, 128, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).delay = v; });
        addBool(tr("In beats"), p->useBeats, [](ChainNode& n, bool v) { std::get<MultiDelayParams>(n.params).useBeats = v; });
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
