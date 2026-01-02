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
#include "core/GpuInfo.hpp"
#include "core/GpuSelector.hpp"
#include "visualizers/modules/ColorGradientModule.hpp"
#include "visualizers/modules/processing/SmoothingModule.hpp"
#include "visualizers/modules/source/AudioSourceModule.hpp"

// Qt includes
#include <QApplication>
#include <QStandardPaths>
#include <QTimer>

// BasicLogger - Logging for GUI Applications
#include <BasicLogger.h>

// Standard Library
#include <chrono>
#include <thread>

// =============================================================================
// GPU Export Flags for Hybrid Graphics (NVIDIA Optimus / AMD PowerXpress)
// =============================================================================
// These symbols are read by GPU drivers BEFORE the application starts.
// They hint the driver to use the dedicated GPU instead of integrated.
//
// Safe to include even if no dedicated GPU exists - flags are just ignored.
// The actual GPU selection is configured via gpu.ini (see GpuSelector).

MYVIZ_ENABLE_HIGH_PERFORMANCE_GPU

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
    QTimer* pFrameTimer{nullptr};  // Owned by QApplication (parent-child)

    // -------------------------------------------------------------------------
    // GPU Selection
    // -------------------------------------------------------------------------
    GpuSelector gpuSelector;
    std::vector<GpuDevice> availableGpus;

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
    
    /**
     * @brief Updates the frame timer interval based on current frame mode.
     */
    void updateTimerInterval()
    {
        if (pFrameTimer == nullptr)
        {
            return;
        }
        
        switch (frameMode)
        {
            case FrameMode::Limited:
            {
                int intervalMs = 1000 / targetFps;
                pFrameTimer->setInterval(intervalMs);
                BasicLogger::logDebug("Timer interval set to " + std::to_string(intervalMs) + "ms (Limited)");
                break;
            }
            
            case FrameMode::Unlimited:
            {
                pFrameTimer->setInterval(0);
                BasicLogger::logDebug("Timer interval set to 0ms (Unlimited)");
                break;
            }
            
            case FrameMode::VSync:
            {
                pFrameTimer->setInterval(1);
                BasicLogger::logDebug("Timer interval set to 1ms (VSync)");
                break;
            }
        }
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
    // GPU Enumeration and Selection
    // -------------------------------------------------------------------------
    // Enumerate GPUs BEFORE creating QApplication (for logging purposes)
    // The actual GPU used depends on driver settings and export flags in main.cpp
    
    BasicLogger::logInfo("Enumerating GPUs...");
    m_impl->availableGpus = GpuInfo::enumerate();
    GpuInfo::logGpuInfo(m_impl->availableGpus);
    
    // Load GPU preferences (create default config if not exists)
    m_impl->gpuSelector.createDefaultConfig("gpu.ini");
    m_impl->gpuSelector.loadConfig("gpu.ini");
    
    // Select preferred GPU (for mismatch checking later)
    const GpuDevice* preferredGpu = m_impl->gpuSelector.selectGpu(m_impl->availableGpus);
    if (preferredGpu != nullptr)
    {
        BasicLogger::logInfo("Preferred GPU: " + preferredGpu->name);
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
    // Initialize User Presets Directory
    // -------------------------------------------------------------------------
    // Must be done AFTER QApplication (for QStandardPaths) but BEFORE MainWindow
    // (which creates visualizers that need to load user presets)
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    std::string gradientPresetsDir = (appData + "/presets/gradients").toStdString();
    lumi::modules::ColorGradientModule::setUserPresetsDirectory(gradientPresetsDir);
    BasicLogger::logInfo("Gradient presets directory: " + gradientPresetsDir);
    
    std::string smoothingPresetsDir = (appData + "/presets/smoothing").toStdString();
    lumi::modules::SmoothingModule::setUserPresetsDirectory(smoothingPresetsDir);
    BasicLogger::logInfo("Smoothing presets directory: " + smoothingPresetsDir);
    
    std::string audioPresetsDir = (appData + "/presets/audio").toStdString();
    lumi::modules::AudioSourceModule::setUserPresetsDirectory(audioPresetsDir);
    BasicLogger::logInfo("Audio presets directory: " + audioPresetsDir);

    // -------------------------------------------------------------------------
    // Create MainWindow (this creates the OpenGL context)
    // -------------------------------------------------------------------------
    // NOTE: Visualizers are registered automatically via lazy-init when
    // VisualizerRegistry::instance() is first accessed (in VisualizerAutoReg.cpp)
    BasicLogger::logDebug("Creating MainWindow...");
    m_impl->pMainWindow = std::make_unique<MainWindow>();
    m_impl->pMainWindow->show();

    // -------------------------------------------------------------------------
    // GPU Mismatch Check
    // -------------------------------------------------------------------------
    // After OpenGL context is created, check if the correct GPU is being used
    // The actual GPU name is logged by VisualizerWidget::initializeGL()
    // We'll check in run() after the first frame when we know the active GPU

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
    // Qt6 Tutorial: Event Loop with exec()
    // -------------------------------------------------------------------------
    // Using Qt's native event loop instead of manual processEvents() loop.
    // This avoids timing issues with update() being asynchronous.
    //
    // For frame-rate control:
    //   - VSync mode: Let swapBuffers handle timing (most efficient)
    //   - Limited mode: Use QTimer for consistent frame rate
    //   - Unlimited: Request continuous updates
    
    // Create a timer for frame updates (stored in Impl for mode changes)
    m_impl->pFrameTimer = new QTimer(m_impl->pQtApp.get());
    m_impl->pFrameTimer->setTimerType(Qt::PreciseTimer);
    
    // Connect timer to render update
    QObject::connect(m_impl->pFrameTimer, &QTimer::timeout, [this, &lastFpsLog]() {
        // Check if window was closed
        if (!m_impl->pMainWindow->isVisible())
        {
            BasicLogger::logInfo("MainWindow closed - exiting loop");
            m_impl->pQtApp->quit();
            return;
        }
        
        // Request rendering
        m_impl->pMainWindow->requestRender();
        
        // Measure FPS
        m_impl->measureFps();
        m_impl->pMainWindow->updateFpsDisplay(m_impl->currentFps);
        
        // Log FPS every 5 seconds
        auto now = std::chrono::steady_clock::now();
        auto sinceLastLog = std::chrono::duration_cast<std::chrono::seconds>(now - lastFpsLog);
        if (sinceLastLog.count() >= 5)
        {
            BasicLogger::logDebug("FPS: " + std::to_string(static_cast<int>(m_impl->currentFps)) +
                                  " | Mode: " + frameModeToString(m_impl->frameMode) +
                                  " | Frame: " + std::to_string(m_impl->frameCount));
            lastFpsLog = now;
        }
    });
    
    // -------------------------------------------------------------------------
    // Connect MainWindow frame mode change signal
    // -------------------------------------------------------------------------
    QObject::connect(m_impl->pMainWindow.get(), &MainWindow::frameModeChangeRequested,
                     [this](int mode) {
        switch (mode)
        {
            case 0: 
                m_impl->frameMode = FrameMode::Limited;
                m_impl->pMainWindow->setVSyncOnAllVisualizers(false);
                break;
            case 1: 
                m_impl->frameMode = FrameMode::Unlimited;
                m_impl->pMainWindow->setVSyncOnAllVisualizers(false);
                break;
            case 2: 
                m_impl->frameMode = FrameMode::VSync;
                m_impl->pMainWindow->setVSyncOnAllVisualizers(true);
                break;
        }
        m_impl->updateTimerInterval();
    });
    
    // Set initial timer interval based on frame mode
    m_impl->updateTimerInterval();
    m_impl->pFrameTimer->start();
    
    // Set initial VSync state based on frame mode
    m_impl->pMainWindow->setVSyncOnAllVisualizers(m_impl->frameMode == FrameMode::VSync);
    
    // -------------------------------------------------------------------------
    // Run Qt Event Loop
    // -------------------------------------------------------------------------
    int result = m_impl->pQtApp->exec();
    
    m_running = false;
    if (m_impl->pFrameTimer != nullptr)
    {
        m_impl->pFrameTimer->stop();
    }

    // -------------------------------------------------------------------------
    // Loop ended
    // -------------------------------------------------------------------------
    BasicLogger::logInfo("Event loop exited");
    BasicLogger::logInfo("  Total frames: " + std::to_string(m_impl->frameCount));
    BasicLogger::logInfo("  Final FPS: " + std::to_string(static_cast<int>(m_impl->currentFps)));

    return result;
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
        m_impl->updateTimerInterval();
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
