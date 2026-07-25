/**
 ****************************************************************************************
 * @file   MultiEffectPanel.hpp
 * @brief  Tree editor for the Multi Effect host's effect chain (Import Roadmap 5.7b)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 *
 * @details
 * A dockable panel that edits the `MultiEffectVisualizer`'s effect chain: a tree
 * view of the nested lists/effects (add / remove / reorder / enable) plus a
 * per-node parameter editor (spinners, colors, EEL script fields). All chain
 * mutations run under the widget's renderMutex() and re-run compileChain(), the
 * editable-chain contract from decision E5.
 *
 * The panel follows the active visualizer via VisualizerChangedEvent; it is only
 * active when the "Multi Effect" host is selected (otherwise it shows a hint).
 ****************************************************************************************
 */

#pragma once

#include "UI/panels/PanelBase.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/modules/ColorGradientModule.hpp"  // gradient combo delegate backing

#include <QList>
#include <QTreeWidget>

#include <functional>

class QComboBox;
class QToolButton;
class QVBoxLayout;
class QScrollArea;
class QLabel;
class QMutex;
class MultiEffectVisualizer;

/// Where a drop landed relative to the target item (public mirror of Qt's
/// protected QAbstractItemView::DropIndicatorPosition).
enum class ChainDrop
{
    OnItem,    ///< onto the item (into a list, else after a leaf)
    Above,     ///< as a sibling before the target
    Below,     ///< as a sibling after the target
    Viewport,  ///< onto empty space -> end of root
};

/**
 * @class ChainTreeWidget
 * @brief QTreeWidget that reports internal drops to a callback instead of
 *        moving items itself.
 *
 * The effect chain (not the view) owns node ordering, so the view must never
 * mutate on its own — that would desync the path-in-UserRole model. dropEvent
 * forwards (source, target, where) to @ref onDrop and swallows the event; the
 * panel mutates the chain and rebuilds the tree from it. No Q_OBJECT: only a
 * virtual override, no new signals -> no moc needed.
 */
class ChainTreeWidget : public QTreeWidget
{
public:
    using QTreeWidget::QTreeWidget;

    std::function<void(QTreeWidgetItem* src, QTreeWidgetItem* target, ChainDrop where)>
        onDrop;
    std::function<void()> onDeleteKey;  ///< Delete/Backspace pressed

protected:
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};

/**
 * @class MultiEffectPanel
 * @brief Dockable tree editor for the multi-effect chain.
 */
class MultiEffectPanel : public PanelBase
{
    Q_OBJECT

public:
    explicit MultiEffectPanel(ServiceContainer& services, QWidget* parent = nullptr);
    ~MultiEffectPanel() override = default;

private:
    void setupUI();
    void connectToActiveVisualizer();
    void setHost(MultiEffectVisualizer* host, QMutex* mutex);

    // Tree <-> chain
    void rebuildTree();
    void addTreeItem(QTreeWidgetItem* parentItem,
                     const lumi::multieffect::ChainNode& node, QList<int> path);
    [[nodiscard]] lumi::multieffect::ChainNode* nodeAtPath(const QList<int>& path);
    [[nodiscard]] QList<int> currentPath() const;
    /// All selected item paths (same parent, sorted by index). Falls back to the
    /// current item when the selection is empty.
    [[nodiscard]] QList<QList<int>> selectedPaths() const;
    [[nodiscard]] QTreeWidgetItem* itemAtPath(const QList<int>& path) const;
    void selectByPath(const QList<int>& path);
    void selectPaths(const QList<QList<int>>& paths);
    /// Keep only selected items on the current item's level (same effect list).
    void enforceSameLevelSelection();

    // Mutations (all lock the render mutex + recompile)
    void mutate(const QList<int>& path,
                const std::function<void(lumi::multieffect::ChainNode&)>& fn);
    void mutateStructure(const std::function<void()>& fn);
    void setNodeEnabled(const QList<int>& path, bool enabled);
    void onAddEffect();
    void onRemove();
    void onClone();
    void onMove(int delta);
    /// Load a SuperScope figure preset's EEL code into the node at `path`.
    void applySuperScopePreset(const QList<int>& path, int presetIndex);
    /// Load a SuperScope-3D template's EEL quartet into the node at `path`.
    void applyScope3DPreset(const QList<int>& path, int presetIndex);
    /// Load a 3D-Camera template's EEL slots into the node at `path`.
    void applyCamera3DPreset(const QList<int>& path, int presetIndex);
    void onDropRequested(QTreeWidgetItem* src, QTreeWidgetItem* target,
                         ChainDrop where);
    /// Move src to the drop destination while the render mutex is held (called
    /// from within mutateStructure). Returns true + the new path on success.
    bool moveNodeLocked(const QList<int>& srcPath, const QList<int>& targetPath,
                        ChainDrop where, QList<int>& finalPath);
    /// Move several same-parent nodes as a block to the drop destination.
    bool moveNodesLocked(const QList<QList<int>>& srcPaths, const QList<int>& targetPath,
                         ChainDrop where, QList<QList<int>>& finalPaths);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onSelectionChanged();

    // Property editor
    void buildPropertyEditor(const QList<int>& path);
    void clearPropertyEditor();

    MultiEffectVisualizer* m_host = nullptr;
    QMutex* m_mutex = nullptr;

    ChainTreeWidget* m_tree = nullptr;
    QComboBox* m_addTypeCombo = nullptr;
    QToolButton* m_addButton = nullptr;
    QToolButton* m_removeButton = nullptr;
    QToolButton* m_cloneButton = nullptr;
    QToolButton* m_upButton = nullptr;
    QToolButton* m_downButton = nullptr;
    QScrollArea* m_propScroll = nullptr;
    QWidget* m_propContainer = nullptr;
    QVBoxLayout* m_propLayout = nullptr;
    QWidget* m_propPage = nullptr;  ///< current editor page (deleted as a whole)
    QLabel* m_hint = nullptr;

    bool m_updating = false;   ///< guards item-changed while rebuilding
    bool m_selecting = false;  ///< guards recursive selection enforcement

    /// Non-null backing for the gradient combo's preview delegate (the delegate
    /// renders each preset into its own temp module; it only needs a live ptr).
    lumi::modules::ColorGradientModule m_gradientPreview;
};
