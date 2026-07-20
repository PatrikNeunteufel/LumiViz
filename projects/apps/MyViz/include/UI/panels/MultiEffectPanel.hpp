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

#include <QList>

class QTreeWidget;
class QTreeWidgetItem;
class QComboBox;
class QToolButton;
class QVBoxLayout;
class QScrollArea;
class QLabel;
class QMutex;
class MultiEffectVisualizer;

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

    // Mutations (all lock the render mutex + recompile)
    void mutate(const QList<int>& path,
                const std::function<void(lumi::multieffect::ChainNode&)>& fn);
    void mutateStructure(const std::function<void()>& fn);
    void onAddEffect();
    void onRemove();
    void onMove(int delta);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onSelectionChanged();

    // Property editor
    void buildPropertyEditor(const QList<int>& path);
    void clearPropertyEditor();

    MultiEffectVisualizer* m_host = nullptr;
    QMutex* m_mutex = nullptr;

    QTreeWidget* m_tree = nullptr;
    QComboBox* m_addTypeCombo = nullptr;
    QToolButton* m_addButton = nullptr;
    QToolButton* m_removeButton = nullptr;
    QToolButton* m_upButton = nullptr;
    QToolButton* m_downButton = nullptr;
    QScrollArea* m_propScroll = nullptr;
    QWidget* m_propContainer = nullptr;
    QVBoxLayout* m_propLayout = nullptr;
    QWidget* m_propPage = nullptr;  ///< current editor page (deleted as a whole)
    QLabel* m_hint = nullptr;

    bool m_updating = false;  ///< guards item-changed while rebuilding
};
