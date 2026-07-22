/**
 ****************************************************************************************
 * @file   DockManager.hpp
 * @brief  Qt-ADS Docking Manager Wrapper - Qt6 Tutorial
 *         Provides dockable UI with tabs, splitting, and floating windows
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This module wraps Qt-ADS (Advanced Docking System) for MyViz.
 * 
 * ## Features
 *   - Multiple visualizer widgets side-by-side
 *   - Tabbed interface when docked in same area
 *   - Floating windows (drag out)
 *   - Auto-hide sidebars
 *   - Save/restore layouts (perspectives)
 *
 * ## Qt-ADS Documentation
 *   https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System
 *
 * @see DockManager.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include <QObject>
#include <QString>
#include <QByteArray>
#include <memory>
#include <vector>
#include <functional>

// =============================================================================
// Forward Declarations
// =============================================================================

QT_BEGIN_NAMESPACE
class QWidget;
class QMainWindow;
class QMenu;
QT_END_NAMESPACE

// Qt-ADS Forward Declarations
namespace ads
{
    class CDockManager;
    class CDockWidget;
    class CDockAreaWidget;
}

class VisualizerWidget;
class ServiceContainer;

// =============================================================================
// DockPosition Enum
// =============================================================================

/**
 * @enum DockPosition
 * @brief Position where a new dock widget should be placed.
 */
enum class DockPosition
{
    Center,     ///< Center area (creates tabs if occupied)
    Left,       ///< Left side
    Right,      ///< Right side
    Top,        ///< Top side
    Bottom,     ///< Bottom side
    Floating    ///< Floating window
};

// =============================================================================
// DockManager Class
// =============================================================================

/**
 * @class DockManager
 * @brief Manages dockable widgets using Qt-ADS.
 *
 * ## Usage Example
 *
 * ```cpp
 * // In MainWindow constructor:
 * m_pDockManager = std::make_unique<DockManager>(this);
 * 
 * // Create visualizers
 * auto* viz1 = m_pDockManager->createVisualizer("Spectrum");
 * auto* viz2 = m_pDockManager->createVisualizer("Waveform", DockPosition::Right);
 * auto* viz3 = m_pDockManager->createVisualizer("3D Effects", DockPosition::Bottom);
 * 
 * // Save layout
 * QByteArray state = m_pDockManager->saveState();
 * 
 * // Restore layout
 * m_pDockManager->restoreState(state);
 * ```
 */
class DockManager : public QObject
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Constructs the DockManager.
     *
     * @param services ServiceContainer for dependency injection
     * @param pMainWindow Parent main window (required for Qt-ADS)
     */
    explicit DockManager(ServiceContainer& services, QMainWindow* pMainWindow);

    /**
     * @brief Destructor.
     */
    ~DockManager() override;

    // Non-copyable
    DockManager(const DockManager&) = delete;
    DockManager& operator=(const DockManager&) = delete;
    DockManager(DockManager&&) = delete;
    DockManager& operator=(DockManager&&) = delete;

    // =========================================================================
    // Visualizer Creation
    // =========================================================================

    /**
     * @brief Creates a new visualizer dock widget.
     *
     * @param title Display title for the dock tab
     * @param position Where to dock (default: Center creates tabs)
     * @return Pointer to the created VisualizerWidget
     */
    VisualizerWidget* createVisualizer(
        const QString& title,
        DockPosition position = DockPosition::Center);

    /**
     * @brief Creates a visualizer docked relative to an existing one.
     *
     * @param title Display title for the dock tab
     * @param position Position relative to reference
     * @param pReference Reference dock widget (nullptr = relative to center)
     * @return Pointer to the created VisualizerWidget
     */
    VisualizerWidget* createVisualizerRelativeTo(
        const QString& title,
        DockPosition position,
        ads::CDockWidget* pReference);

    // =========================================================================
    // Generic Dock Widget Creation
    // =========================================================================

    /**
     * @brief Creates a dock widget with custom content.
     *
     * @param title Display title
     * @param pContent Content widget (ownership transferred)
     * @param position Where to dock
     * @return The created dock widget
     */
    ads::CDockWidget* createDockWidget(
        const QString& title,
        QWidget* pContent,
        DockPosition position = DockPosition::Center);

    // =========================================================================
    // Dock Widget Access
    // =========================================================================

    /**
     * @brief Gets all visualizer widgets.
     */
    [[nodiscard]] std::vector<VisualizerWidget*> visualizers() const;

    /**
     * @brief Gets a dock widget by title.
     */
    [[nodiscard]] ads::CDockWidget* dockWidget(const QString& title) const;

    /**
     * @brief Gets the number of dock widgets.
     */
    [[nodiscard]] int dockWidgetCount() const;

    // =========================================================================
    // Layout Management
    // =========================================================================

    /**
     * @brief Saves the current layout state.
     *
     * @return Serialized state that can be stored in settings
     */
    [[nodiscard]] QByteArray saveState() const;

    /**
     * @brief Restores a previously saved layout.
     *
     * @param state State from saveState()
     * @return true if restored successfully
     */
    bool restoreState(const QByteArray& state);

    /**
     * @brief Saves a named perspective (layout preset).
     *
     * @param name Perspective name (e.g., "Default", "Compact", "Expanded")
     */
    void savePerspective(const QString& name);

    /**
     * @brief Restores a named perspective.
     *
     * @param name Perspective name
     */
    void loadPerspective(const QString& name);

    /**
     * @brief Gets list of saved perspective names.
     */
    [[nodiscard]] QStringList perspectiveNames() const;

    // =========================================================================
    // Menu Integration
    // =========================================================================

    /**
     * @brief Creates a "View" menu with dock widget toggles.
     *
     * Menu contains:
     *   - Toggle actions for each dock widget
     *   - Perspective submenu
     *   - "Reset Layout" action
     *
     * @param pParent Parent menu bar or widget
     * @return The created menu (ownership: Qt parent-child)
     * 
     * @deprecated Use populatePanelsMenu() and populatePerspectivesMenu() instead
     *             for integration with MenuRegistry system.
     */
    QMenu* createViewMenu(QWidget* pParent);

    /**
     * @brief Populates a Panels submenu with dock widget toggle actions.
     *
     * For use with MenuRegistry system. Adds checkable actions for each
     * registered dock widget (panels, visualizers, etc.)
     *
     * @param pMenu Target menu to populate
     */
    void populatePanelsMenu(QMenu* pMenu);

    /**
     * @brief Populates a Perspectives submenu with layout presets.
     *
     * For use with MenuRegistry system. Adds:
     *   - "Save Current..." action
     *   - Separator
     *   - Action for each saved perspective
     *
     * @param pMenu Target menu to populate
     */
    void populatePerspectivesMenu(QMenu* pMenu);

    // =========================================================================
    // Qt-ADS Access
    // =========================================================================

    /**
     * @brief Gets the underlying Qt-ADS DockManager.
     *
     * Use for advanced operations not exposed by this wrapper.
     */
    [[nodiscard]] ads::CDockManager* adsDockManager() const noexcept;

signals:
    // =========================================================================
    // Signals
    // =========================================================================

    /**
     * @brief Emitted when a visualizer is created.
     */
    void visualizerCreated(VisualizerWidget* pVisualizer);

    /**
     * @brief Emitted when a dock widget is closed.
     */
    void dockWidgetClosed(const QString& title);

    /**
     * @brief Emitted when the layout changes.
     */
    void layoutChanged();

public slots:
    // =========================================================================
    // Slots
    // =========================================================================

    /**
     * @brief Resets to default layout.
     */
    void resetLayout();

    /**
     * @brief Re-docks a visualizer after fullscreen mode.
     * 
     * @param pVisualizer The visualizer to re-dock
     */
    void redockVisualizer(VisualizerWidget* pVisualizer);

    /**
     * @brief Closes all dock widgets.
     */
    void closeAll();
    
    /**
     * @brief Saves current layout as the new default.
     * 
     * Call this after arranging panels to your liking.
     * This layout will be restored on "Reset Layout".
     */
    void saveDefaultLayout();
    
    /**
     * @brief Restore layout from settings or apply defaults.
     * 
     * IMPORTANT: Call this AFTER all widgets (Panels, Visualizers) are created!
     * Qt-ADS can only restore positions for widgets that already exist.
     * 
     * @return true if saved layout was restored, false if defaults were applied
     */
    bool restoreLayout();

private:
    // =========================================================================
    // Private Implementation
    // =========================================================================

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    /**
     * @brief Converts DockPosition to Qt-ADS area.
     */
    static ads::CDockAreaWidget* positionToArea(
        ads::CDockManager* pManager,
        DockPosition position);
    
    /**
     * @brief Subscribe to EventBus events.
     */
    void subscribeToEvents();
    
    /**
     * @brief Unsubscribe from EventBus events.
     */
    void unsubscribeFromEvents();
    
    /**
     * @brief Restore layout from QSettings.
     * @return true if layout was restored, false if no saved layout exists
     */
    bool restoreLayoutFromSettings();

    /**
     * @brief Schedules a debounced native-handle rebuild for all embedded
     *        visualizers after a dock layout change (float/redock).
     *
     * Reparenting docks can leave embedded native GL windows at stale
     * absolute positions (the Session-31 "doubled bar" ghost, previously
     * only fixed for the fullscreen-exit path).
     */
    void scheduleNativeResync();
    
    /**
     * @brief Save current layout to QSettings.
     */
    void saveLayoutToSettings();
};
