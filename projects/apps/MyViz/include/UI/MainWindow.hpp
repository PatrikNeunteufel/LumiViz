/**
 ****************************************************************************************
 * @file   MainWindow.hpp
 * @brief  Main Application Window - Qt6 Tutorial
 *         Uses Qt-ADS for dockable visualizer panels
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This file is part of the Qt6 Tutorial series for MyViz.
 * It demonstrates:
 *   - QMainWindow with Qt-ADS docking
 *   - Multiple visualizer widgets
 *   - Tabbed and split layouts
 *   - Menu and status bar integration
 *
 * ## Qt-ADS Docking Layout
 *
 * ```
 * +------------------------------------------+
 * |              Menu Bar                    |
 * +------------------------------------------+
 * | ┌──────────────┬───────────────────────┐ |
 * | │ Spectrum     │ Waveform   │ 3D       │ | ◄─ Tabs
 * | ├──────────────┴───────────────────────┤ |
 * | │                                      │ |
 * | │        [Active Visualizer]           │ |
 * | │                                      │ |
 * | └──────────────────────────────────────┘ |
 * +------------------------------------------+
 * |              Status Bar (FPS)            |
 * +------------------------------------------+
 * ```
 *
 * @see MainWindow.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Qt Includes
// =============================================================================

#include <QMainWindow>
#include <memory>
#include <vector>

// =============================================================================
// Forward Declarations
// =============================================================================

QT_BEGIN_NAMESPACE
class QWidget;
class QMenu;
class QLabel;
QT_END_NAMESPACE

class DockManager;
class MenuManager;
class VisualizerWidget;
class ServiceContainer;

/**
 * @class MainWindow
 * @brief Main application window with dockable visualizer panels.
 *
 * MainWindow uses Qt-ADS (Advanced Docking System) for a flexible
 * multi-visualizer layout. Users can:
 *   - Create multiple visualizer panels
 *   - Arrange them as tabs, splits, or floating windows
 *   - Save and restore layouts (perspectives)
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Constructs the MainWindow.
     *
     * @param parent Optional parent widget (nullptr for top-level window).
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~MainWindow() override;

    // Non-copyable
    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

    // =========================================================================
    // Dock Manager Access
    // =========================================================================

    /**
     * @brief Gets the dock manager.
     *
     * Use to create visualizers, manage layout, etc.
     */
    [[nodiscard]] DockManager* dockManager() const noexcept;

    // =========================================================================
    // Visualizer Access
    // =========================================================================

    /**
     * @brief Gets all visualizer widgets.
     */
    [[nodiscard]] std::vector<VisualizerWidget*> visualizers() const;

    /**
     * @brief Gets the first (primary) visualizer.
     *
     * @return Primary visualizer or nullptr if none exist
     */
    [[nodiscard]] VisualizerWidget* primaryVisualizer() const;

    // =========================================================================
    // Render Control
    // =========================================================================

    /**
     * @brief Requests a repaint of all visualizers.
     *
     * Call this from the main loop to trigger rendering on all panels.
     */
    void requestRender();

public slots:
    // =========================================================================
    // Public Slots
    // =========================================================================

    /**
     * @brief Updates the FPS display in the status bar.
     */
    void updateFpsDisplay(double fps);

    /**
     * @brief Creates a new visualizer panel.
     */
    void onNewVisualizer();
    
    /**
     * @brief Sets VSync on all visualizers.
     *
     * @param enabled true = VSync ON, false = VSync OFF
     */
    void setVSyncOnAllVisualizers(bool enabled);

signals:
    // =========================================================================
    // Signals
    // =========================================================================

    /**
     * @brief Emitted when user changes frame mode via menu.
     *
     * @param mode 0=Limited, 1=Unlimited, 2=VSync
     */
    void frameModeChangeRequested(int mode);

private slots:
    // =========================================================================
    // Private Slots
    // =========================================================================

    void onVisualizerCreated(VisualizerWidget* pVisualizer);

private:
    // =========================================================================
    // Private Methods
    // =========================================================================

    /**
     * @brief Sets up the user interface.
     */
    void setupUi();

    /**
     * @brief Creates the menu bar.
     */
    void setupMenuBar();

    /**
     * @brief Creates the status bar.
     */
    void setupStatusBar();

    /**
     * @brief Creates the default visualizer layout.
     */
    void setupDefaultLayout();

    /**
     * @brief Sets up event handlers for menu actions.
     */
    void setupEventHandlers();

    // =========================================================================
    // Private Members
    // =========================================================================

    std::unique_ptr<ServiceContainer> m_pServices;
    std::unique_ptr<DockManager> m_pDockManager;
    std::unique_ptr<MenuManager> m_pMenuManager;
    QLabel* m_pFpsLabel{nullptr};  // Owned by status bar (Qt parent-child)
};
