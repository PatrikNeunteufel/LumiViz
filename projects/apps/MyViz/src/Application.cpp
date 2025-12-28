/**
 ****************************************************************************************
 * @file   Application.cpp
 * @brief  Application Implementation - Qt6 Tutorial
 *         Custom event loop with configurable frame modes
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This file demonstrates:
 *   - Pimpl pattern for Qt encapsulation
 *   - Custom main loop with processEvents()
 *   - Three frame timing modes (Limited, Unlimited, VSync)
 *   - Real-time FPS measurement
 *   - BasicLogger integration
 *
 * @see Application.hpp for public interface
 * @see Application.md for detailed documentation
 ****************************************************************************************
 */

// =============================================================================
// Includes
// =============================================================================

#include "pch.h"
#include "Application.hpp"
#include "UI/MainWindow.hpp"

// Qt includes
#include <QApplication>

// BasicLogger - Logging for GUI Applications
#include <BasicLogger.h>

// Standard Library
#include <chrono>
#include <thread>

// =============================================================================
// Helper: FrameMode to String
// =============================================================================

namespace
{

/**
 * @brief Converts FrameMode enum to readable string.
 * @param mode The FrameMode to convert
 * @return String representation
 */
const char* frameModeToString(FrameMode mode)
{
    switch (mode)
    {
        case FrameMode::Limited:   return "Limited";
        case FrameMode::Unlimited: return "Unlimited";
        case FrameMode::VSync:     return "VSync";
        default:                   return "Unknown";
    }
}

} // anonymous namespace

// =============================================================================
// Private Implementation (Pimpl Pattern)
// =============================================================================

struct Application::Impl
{
    // -------------------------------------------------------------------------
    // Application Metadata
    // -------------------------------------------------------------------------
    std::string name{"MyViz"};
    std::string version{"0.1.0"};
    std::vector<std::string> args;

    // -------------------------------------------------------------------------
    // Qt Objects
    // -------------------------------------------------------------------------
    std::unique_ptr<QApplication> pQtApp;
    std::unique_ptr<MainWindow> pMainWindow;

    // -------------------------------------------------------------------------
    // Frame Mode Configuration
    // -------------------------------------------------------------------------
    FrameMode frameMode{FrameMode::Limited};  // Default: Limited at 60 FPS
    int targetFps{60};                         // Target FPS for Limited mode

    // Calculated frame duration for Limited mode
    std::chrono::microseconds frameDuration{16667};  // 1000000 / 60

    // -------------------------------------------------------------------------
    // Frame Statistics
    // -------------------------------------------------------------------------
    uint64_t frameCount{0};          // Total frames since run()
    double currentFps{0.0};          // Measured FPS (updated every second)

    // FPS measurement helper
    uint64_t fpsFrameCount{0};       // Frames in current second
    std::chrono::steady_clock::time_point fpsStartTime;

    // -------------------------------------------------------------------------
    // Helper Methods
    // -------------------------------------------------------------------------

    /**
     * @brief Updates the frame duration based on target FPS.
     */
    void updateFrameDuration()
    {
        if (targetFps > 0)
        {
            frameDuration = std::chrono::microseconds(1000000 / targetFps);
        }
    }

    /**
     * @brief Measures and updates FPS statistics.
     *
     * Call this once per frame. Updates currentFps every second.
     */
    void measureFps()
    {
        frameCount++;
        fpsFrameCount++;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - fpsStartTime);

        // Update FPS every second
        if (elapsed.count() >= 1000)
        {
            currentFps = static_cast<double>(fpsFrameCount) * 1000.0 /
                         static_cast<double>(elapsed.count());

            // Reset for next measurement
            fpsStartTime = now;
            fpsFrameCount = 0;
        }
    }

    /**
     * @brief Resets FPS measurement counters.
     */
    void resetFpsMeasurement()
    {
        frameCount = 0;
        currentFps = 0.0;
        fpsFrameCount = 0;
        fpsStartTime = std::chrono::steady_clock::now();
    }
};

// =============================================================================
// Construction / Destruction
// =============================================================================

Application::Application()
    : m_impl{std::make_unique<Impl>()}
{
    // -------------------------------------------------------------------------
    // Initialize Logging FIRST
    // -------------------------------------------------------------------------
    BasicLogger::setLogFile("MyViz.log");
    BasicLogger::setLogLevel(BasicLogger::Level::Debug);

    BasicLogger::logInfo("===========================================");
    BasicLogger::logInfo("Application constructor");
    BasicLogger::logInfo("Default FrameMode: " + 
                         std::string(frameModeToString(m_impl->frameMode)));
    BasicLogger::logInfo("Default Target FPS: " + std::to_string(m_impl->targetFps));
}

Application::~Application()
{
    // Ensure proper shutdown
    if (m_initialized)
    {
        shutdown();
    }

    BasicLogger::logInfo("Application destructor complete");
    BasicLogger::logInfo("Total frames: " + std::to_string(m_impl->frameCount));
    BasicLogger::logInfo("===========================================");

    BasicLogger::closeLogFile();
}

// =============================================================================
// Lifecycle
// =============================================================================

bool Application::init(int argc, char* argv[])
{
    // -------------------------------------------------------------------------
    // Guard: Prevent double initialization
    // -------------------------------------------------------------------------
    if (m_initialized)
    {
        BasicLogger::logWarning("Application::init() - Already initialized!");
        return false;
    }

    BasicLogger::logInfo("Application::init() starting...");

    // -------------------------------------------------------------------------
    // Store command line arguments
    // -------------------------------------------------------------------------
    m_impl->args.clear();
    m_impl->args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i)
    {
        m_impl->args.emplace_back(argv[i]);
        BasicLogger::logDebug("  argv[" + std::to_string(i) + "] = " + argv[i]);
    }

    // -------------------------------------------------------------------------
    // Create QApplication
    // -------------------------------------------------------------------------
    BasicLogger::logDebug("Creating QApplication...");
    m_impl->pQtApp = std::make_unique<QApplication>(argc, argv);

    m_impl->pQtApp->setApplicationName(QString::fromStdString(m_impl->name));
    m_impl->pQtApp->setApplicationVersion(QString::fromStdString(m_impl->version));
    m_impl->pQtApp->setOrganizationName(QStringLiteral("MyViz Project"));

    // -------------------------------------------------------------------------
    // Create MainWindow
    // -------------------------------------------------------------------------
    BasicLogger::logDebug("Creating MainWindow...");
    m_impl->pMainWindow = std::make_unique<MainWindow>();
    m_impl->pMainWindow->show();

    // -------------------------------------------------------------------------
    // Initialize frame timing
    // -------------------------------------------------------------------------
    m_impl->updateFrameDuration();
    m_impl->resetFpsMeasurement();

    // -------------------------------------------------------------------------
    // Done
    // -------------------------------------------------------------------------
    m_initialized = true;
    BasicLogger::logInfo(m_impl->name + " v" + m_impl->version + " initialized");

    return true;
}

int Application::run()
{
    // -------------------------------------------------------------------------
    // Guards
    // -------------------------------------------------------------------------
    if (!m_initialized)
    {
        BasicLogger::logError("Application::run() - Not initialized!");
        return 1;
    }

    if (m_running)
    {
        BasicLogger::logWarning("Application::run() - Already running!");
        return 1;
    }

    m_running = true;
    BasicLogger::logInfo("Application::run() - Entering event loop");
    BasicLogger::logInfo("  FrameMode: " + std::string(frameModeToString(m_impl->frameMode)));
    if (m_impl->frameMode == FrameMode::Limited)
    {
        BasicLogger::logInfo("  Target FPS: " + std::to_string(m_impl->targetFps));
    }

    // Reset FPS measurement
    m_impl->resetFpsMeasurement();

    // For FPS logging interval
    auto lastFpsLog = std::chrono::steady_clock::now();

    // -------------------------------------------------------------------------
    // Main Loop
    // -------------------------------------------------------------------------
    while (m_running)
    {
        auto frameStart = std::chrono::steady_clock::now();

        // ---------------------------------------------------------------------
        // 1. Process Qt Events
        // ---------------------------------------------------------------------
        m_impl->pQtApp->processEvents(QEventLoop::AllEvents);

        // ---------------------------------------------------------------------
        // 2. Check if window was closed
        // ---------------------------------------------------------------------
        if (!m_impl->pMainWindow->isVisible())
        {
            BasicLogger::logInfo("MainWindow closed - exiting loop");
            m_running = false;
            break;
        }

        // ---------------------------------------------------------------------
        // 3. Update Logic (TODO: Audio, Visualization)
        // ---------------------------------------------------------------------
        // m_impl->pAudioEngine->update();
        
        // Request rendering - triggers paintGL() in VisualizerWidget
        m_impl->pMainWindow->requestRender();

        // ---------------------------------------------------------------------
        // 4. Measure FPS
        // ---------------------------------------------------------------------
        m_impl->measureFps();

        // Log FPS every 5 seconds
        auto now = std::chrono::steady_clock::now();
        auto sinceLastLog = std::chrono::duration_cast<std::chrono::seconds>(
            now - lastFpsLog);

        if (sinceLastLog.count() >= 5)
        {
            BasicLogger::logDebug("FPS: " + std::to_string(static_cast<int>(m_impl->currentFps)) +
                                  " | Mode: " + frameModeToString(m_impl->frameMode) +
                                  " | Frame: " + std::to_string(m_impl->frameCount));
            lastFpsLog = now;
        }

        // ---------------------------------------------------------------------
        // 5. Frame Timing (depends on FrameMode)
        // ---------------------------------------------------------------------
        switch (m_impl->frameMode)
        {
            case FrameMode::Limited:
            {
                // Software frame limiting with sleep
                auto frameEnd = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                    frameEnd - frameStart);

                if (elapsed < m_impl->frameDuration)
                {
                    std::this_thread::sleep_for(m_impl->frameDuration - elapsed);
                }
                break;
            }

            case FrameMode::Unlimited:
            {
                // No waiting - run as fast as possible
                // (100% CPU usage!)
                break;
            }

            case FrameMode::VSync:
            {
                // TODO: When OpenGL Widget is implemented, SwapBuffers handles VSync
                // For now, fallback to 60 FPS limited
                auto frameEnd = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                    frameEnd - frameStart);

                constexpr auto vsyncDuration = std::chrono::microseconds(16667);  // ~60 FPS
                if (elapsed < vsyncDuration)
                {
                    std::this_thread::sleep_for(vsyncDuration - elapsed);
                }
                break;
            }
        }
    }

    // -------------------------------------------------------------------------
    // Loop ended
    // -------------------------------------------------------------------------
    BasicLogger::logInfo("Event loop exited");
    BasicLogger::logInfo("  Total frames: " + std::to_string(m_impl->frameCount));
    BasicLogger::logInfo("  Final FPS: " + std::to_string(static_cast<int>(m_impl->currentFps)));

    return 0;
}

void Application::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    BasicLogger::logInfo("Application::shutdown() starting...");

    m_running = false;

    // Cleanup in reverse order
    BasicLogger::logDebug("Destroying MainWindow...");
    m_impl->pMainWindow.reset();

    BasicLogger::logDebug("Destroying QApplication...");
    m_impl->pQtApp.reset();

    m_initialized = false;
    BasicLogger::logInfo("Application::shutdown() complete");
}

void Application::requestQuit()
{
    BasicLogger::logInfo("Application::requestQuit() called");
    m_running = false;
}

// =============================================================================
// Frame Mode Configuration
// =============================================================================

void Application::setFrameMode(FrameMode mode)
{
    if (m_impl->frameMode != mode)
    {
        BasicLogger::logInfo("FrameMode changed: " + 
                             std::string(frameModeToString(m_impl->frameMode)) +
                             " -> " + frameModeToString(mode));
        m_impl->frameMode = mode;
    }
}

FrameMode Application::frameMode() const noexcept
{
    return m_impl->frameMode;
}

void Application::setTargetFps(int fps)
{
    // Clamp to valid range
    if (fps < 1)
    {
        fps = 1;
    }
    else if (fps > 1000)
    {
        fps = 1000;
    }

    if (m_impl->targetFps != fps)
    {
        BasicLogger::logInfo("Target FPS changed: " + 
                             std::to_string(m_impl->targetFps) +
                             " -> " + std::to_string(fps));
        m_impl->targetFps = fps;
        m_impl->updateFrameDuration();
    }
}

int Application::targetFps() const noexcept
{
    return m_impl->targetFps;
}

// =============================================================================
// Frame Statistics
// =============================================================================

double Application::currentFps() const noexcept
{
    return m_impl->currentFps;
}

uint64_t Application::frameCount() const noexcept
{
    return m_impl->frameCount;
}

// =============================================================================
// Application Info
// =============================================================================

const std::string& Application::name() const noexcept
{
    return m_impl->name;
}

const std::string& Application::version() const noexcept
{
    return m_impl->version;
}

bool Application::isInitialized() const noexcept
{
    return m_initialized;
}

bool Application::isRunning() const noexcept
{
    return m_running;
}
