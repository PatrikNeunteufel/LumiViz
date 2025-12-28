/**
 ****************************************************************************************
 * @file   Application.hpp
 * @brief  Application Interface - Qt6 Tutorial
 *         Central application class with configurable frame modes
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This file demonstrates:
 *   - Pimpl pattern for Qt encapsulation
 *   - Configurable frame rate modes (Limited, Unlimited, VSync)
 *   - Real-time FPS measurement
 *
 * ## Qt6 Tutorial: Frame Modes
 *
 * | Mode      | Beschreibung                    | CPU   | Verwendung           |
 * |-----------|---------------------------------|-------|----------------------|
 * | Limited   | Software-Begrenzung auf X FPS   | ~5%   | Batterieschonung     |
 * | Unlimited | Keine Begrenzung, max FPS       | 100%  | Benchmarking         |
 * | VSync     | GPU-synchronisiert              | ~5%   | Visualizer (ideal)   |
 *
 * @see Application.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// =============================================================================
// FrameMode Enumeration
// =============================================================================

/**
 * @enum FrameMode
 * @brief Defines the frame timing mode for the main loop.
 *
 * ## Qt6 Tutorial: Frame Timing Strategien
 *
 * Die Wahl des FrameMode beeinflusst CPU-Auslastung und Framerate:
 *
 * ```
 * Limited:   [Update]--[Sleep]--[Update]--[Sleep]--  → Konstant X FPS
 * Unlimited: [Update][Update][Update][Update][Update] → Max FPS, 100% CPU
 * VSync:     [Update]--[GPU Wait]--[Update]--[GPU]--  → Monitor-Sync
 * ```
 */
enum class FrameMode
{
    /**
     * @brief Software-limitierte Framerate.
     *
     * Der Loop wartet aktiv (sleep) um die Ziel-FPS zu erreichen.
     * Gut für Batterieschonung und wenn VSync nicht verfügbar.
     *
     * Beispiel: targetFps = 60 → ~16.67ms pro Frame
     */
    Limited,

    /**
     * @brief Keine Framerate-Begrenzung.
     *
     * Der Loop läuft so schnell wie möglich.
     * Nützlich für Benchmarking und Performance-Tests.
     *
     * Warnung: 100% CPU-Auslastung!
     */
    Unlimited,

    /**
     * @brief Vertikal-Synchronisation mit GPU.
     *
     * Die Framerate wird durch den Monitor-Refresh begrenzt (60Hz, 144Hz, etc.)
     * Verhindert Screen-Tearing und ist energieeffizient.
     *
     * Hinweis: Erfordert OpenGL Widget mit SwapBuffers.
     * Bis dahin verhält sich VSync wie Limited(60).
     */
    VSync
};

// =============================================================================
// Application Class
// =============================================================================

/**
 * @class Application
 * @brief Central entry point for application logic.
 *
 * This class encapsulates QApplication and provides:
 * - Custom event loop with configurable frame modes
 * - Real-time FPS measurement
 * - Clean lifecycle management (init/run/shutdown)
 *
 * ## Usage Example
 *
 * ```cpp
 * Application app;
 * app.init(argc, argv);
 * app.setFrameMode(FrameMode::VSync);
 * app.run();  // Blocks until window closed
 * app.shutdown();
 * ```
 */
class Application
{
public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    Application();
    ~Application();

    // Non-copyable, non-movable
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    // =========================================================================
    // Lifecycle
    // =========================================================================

    /**
     * @brief Initializes the application.
     *
     * Creates QApplication, MainWindow, and sets up logging.
     * Must be called before run().
     *
     * @param argc Number of command line arguments
     * @param argv Command line arguments
     * @return true on success, false on error
     */
    [[nodiscard]] bool init(int argc, char* argv[]);

    /**
     * @brief Starts the main loop.
     *
     * Runs until the main window is closed or requestQuit() is called.
     * Uses the configured FrameMode for timing.
     *
     * @return Exit code (0 = success)
     */
    [[nodiscard]] int run();

    /**
     * @brief Shuts down the application and releases resources.
     *
     * Destroys MainWindow and QApplication in correct order.
     * Safe to call multiple times.
     */
    void shutdown();

    /**
     * @brief Requests the application to quit.
     *
     * Sets the running flag to false. The main loop will exit
     * at the end of the current frame.
     */
    void requestQuit();

    // =========================================================================
    // Frame Mode Configuration
    // =========================================================================

    /**
     * @brief Sets the frame timing mode.
     *
     * Can be changed at runtime. Takes effect on next frame.
     *
     * @param mode The desired FrameMode
     */
    void setFrameMode(FrameMode mode);

    /**
     * @brief Gets the current frame timing mode.
     * @return Current FrameMode
     */
    [[nodiscard]] FrameMode frameMode() const noexcept;

    /**
     * @brief Sets the target FPS for Limited mode.
     *
     * Only affects FrameMode::Limited. Ignored in other modes.
     * Valid range: 1-1000 FPS.
     *
     * @param fps Target frames per second (default: 60)
     */
    void setTargetFps(int fps);

    /**
     * @brief Gets the target FPS for Limited mode.
     * @return Target FPS
     */
    [[nodiscard]] int targetFps() const noexcept;

    // =========================================================================
    // Frame Statistics
    // =========================================================================

    /**
     * @brief Gets the current measured FPS.
     *
     * Updated once per second. Returns 0 before first measurement.
     *
     * @return Current frames per second
     */
    [[nodiscard]] double currentFps() const noexcept;

    /**
     * @brief Gets the total frame count since run() started.
     * @return Total number of frames rendered
     */
    [[nodiscard]] uint64_t frameCount() const noexcept;

    // =========================================================================
    // Application Info
    // =========================================================================

    /**
     * @brief Gets the application name.
     * @return Application name
     */
    [[nodiscard]] const std::string& name() const noexcept;

    /**
     * @brief Gets the application version.
     * @return Application version
     */
    [[nodiscard]] const std::string& version() const noexcept;

    /**
     * @brief Checks if the application is initialized.
     * @return true if initialized
     */
    [[nodiscard]] bool isInitialized() const noexcept;

    /**
     * @brief Checks if the application is running.
     * @return true if running
     */
    [[nodiscard]] bool isRunning() const noexcept;

private:
    // =========================================================================
    // Private Implementation (Pimpl)
    // =========================================================================

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    bool m_initialized{false};
    bool m_running{false};
};
