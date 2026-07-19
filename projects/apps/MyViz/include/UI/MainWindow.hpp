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
#include <QRect>
#include <memory>
#include <vector>

// =============================================================================
// Forward Declarations
// =============================================================================

QT_BEGIN_NAMESPACE
class QWidget;
class QMenu;
class QLabel;
class QTimer;
class QKeyEvent;
QT_END_NAMESPACE

// Qt-ADS Forward Declaration
namespace ads { class CDockWidget; }

class DockManager;
class MenuManager;
class DialogManager;
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

    // Note: The former requestRender() main-loop hook is gone — every
    // VisualizerWidget renders on its own thread (Render_Thread_Entwurf.md).

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
     * @brief Applies the frame mode to all visualizers' render threads.
     *
     * @param mode 0=Limited, 1=Unlimited, 2=VSync (menu index)
     * @param targetFps Target FPS for Limited mode
     */
    void setFrameModeOnAllVisualizers(int mode, int targetFps);

    /**
     * @brief Toggles fullscreen mode.
     */
    void toggleFullscreen();

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

protected:
    // =========================================================================
    // QMainWindow Overrides
    // =========================================================================

    /**
     * @brief Handle key press events (Esc to exit fullscreen)
     */
    void keyPressEvent(QKeyEvent* event) override;

    /**
     * @brief Closing the main window quits the application.
     *
     * Replaces the former frame-timer visibility poll: floating dock windows
     * must not keep the app alive after the main window is gone.
     */
    void closeEvent(QCloseEvent* event) override;

private slots:
    // =========================================================================
    // Private Slots
    // =========================================================================

    void onVisualizerCreated(VisualizerWidget* pVisualizer);
    
    /**
     * @brief Called periodically to update audio playback state.
     */
    void onAudioUpdate();

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

    /**
     * @brief Sets up audio services (Engine, Player, Playlist).
     */
    void setupAudioServices();

    /**
     * @brief Saves the current playlist as session playlist (on quit).
     *
     * Writes the playlist to AppDataLocation/session.m3u8 and remembers the
     * current track index in QSettings. An empty playlist removes the session
     * file so the next start does not restore stale data.
     */
    void saveSessionPlaylist();

    /**
     * @brief Restores the session playlist saved by saveSessionPlaylist().
     *
     * Called once at startup (after audio services and panels exist, so the
     * Loaded event reaches the PlaylistPanel). Does not start playback.
     */
    void restoreSessionPlaylist();

    /**
     * @brief Enter fullscreen mode for a visualizer
     * @param requested Visualizer that requested fullscreen (double-click/
     *        Esc source); nullptr (menu/F11) uses the primary visualizer
     */
    void enterFullscreen(VisualizerWidget* requested = nullptr);

    /**
     * @brief Exit fullscreen mode
     */
    void exitFullscreen();

    // =========================================================================
    // Private Members
    // =========================================================================

    std::unique_ptr<ServiceContainer> m_pServices;
    std::unique_ptr<DockManager> m_pDockManager;
    std::unique_ptr<MenuManager> m_pMenuManager;
    std::unique_ptr<DialogManager> m_pDialogManager;
    QLabel* m_pFpsLabel{nullptr};  // Owned by status bar (Qt parent-child)
    QTimer* m_pAudioUpdateTimer{nullptr};  // Owned by this (Qt parent-child)
    
    // Fullscreen support: the visualizer is taken OUT of its dock and shown
    // as a borderless top-level window (true fullscreen, no docking chrome)
    bool m_isFullscreen{false};                          // Current fullscreen state
    VisualizerWidget* m_pFullscreenVisualizer{nullptr};  // Active visualizer in fullscreen
    ads::CDockWidget* m_pFullscreenDock{nullptr};        // Dock to re-embed into on exit
    
    // Note: Audio services (BassEngine, AudioPlayer, Playlist) are managed by
    // ServiceContainer via registerSingleton factories, not stored here.
};
