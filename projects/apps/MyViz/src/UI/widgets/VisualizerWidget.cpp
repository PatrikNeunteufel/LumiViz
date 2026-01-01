/**
 ****************************************************************************************
 * @file   VisualizerWidget.cpp
 * @brief  OpenGL Visualization Widget Implementation - Qt6 Tutorial
 *         Hardware-accelerated rendering with VSync support
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.2.0 - Added Config Event Support
 ****************************************************************************************
 */

// =============================================================================
// Includes
// =============================================================================

#include "pch.h"
#include "UI/widgets/VisualizerWidget.hpp"
#include "visualizers/IVisualizer.hpp"
#include "visualizers/PulsingVisualizer.hpp"
#include "services/VisualizerRegistry.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

// Qt
#include <QOpenGLContext>
#include <QGuiApplication>
#include <QWindow>
#include <QString>

// BasicLogger
#include <BasicLogger.h>

// =============================================================================
// Platform-specific VSync Support
// =============================================================================

#if defined(_WIN32)
    // Windows: wglSwapIntervalEXT
    using PFNWGLSWAPINTERVALEXTPROC = int (*)(int interval);
    static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
#elif defined(__linux__) && !defined(__ANDROID__)
    // Linux: Multiple options depending on display server
    #include <QtGui/qpa/qplatformnativeinterface.h>
    
    // GLX (X11)
    using PFNGLXSWAPINTERVALEXTPROC = void (*)(void* display, unsigned long drawable, int interval);
    using PFNGLXSWAPINTERVALMESAPROC = int (*)(int interval);
    static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = nullptr;
    static PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA = nullptr;
    
    // EGL (Wayland, also works on X11 with EGL)
    using PFNEGLSWAPINTERVALPROC = unsigned int (*)(void* display, int interval);
    static PFNEGLSWAPINTERVALPROC eglSwapIntervalFunc = nullptr;
    
    #include <dlfcn.h>
    static void* libEGL = nullptr;
#elif defined(__APPLE__)
    #include <OpenGL/OpenGL.h>
#endif

// =============================================================================
// Construction / Destruction
// =============================================================================

VisualizerWidget::VisualizerWidget(ServiceContainer& services, QWidget* parent)
    : OpenGLWidgetBase(services, 
                       QStringLiteral("visualizer"), 
                       tr("Visualizer"), 
                       parent)
    , QOpenGLFunctions()
{
    BasicLogger::logDebug("VisualizerWidget constructor");

    // -------------------------------------------------------------------------
    // Surface Format Configuration
    // -------------------------------------------------------------------------
    QSurfaceFormat format;
    format.setVersion(3, 3);                              // OpenGL 3.3
    format.setProfile(QSurfaceFormat::CoreProfile);       // Core Profile (modern)
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer); // Double buffering
    format.setSwapInterval(0);                            // VSync OFF - we do software timing
    format.setDepthBufferSize(24);                        // 24-bit depth buffer
    format.setSamples(4);                                 // 4x MSAA

    setFormat(format);

    // Start frame timer
    m_frameTimer.start();

    // Subscribe to config events from ConfigPanel
    subscribeToConfigEvents();

    BasicLogger::logDebug("  Requested OpenGL 3.3 Core Profile");
    BasicLogger::logDebug("  VSync: OFF (using software frame limiting)");
    BasicLogger::logDebug("  MSAA: 4x samples");
}

VisualizerWidget::~VisualizerWidget()
{
    BasicLogger::logDebug("VisualizerWidget destructor");
    BasicLogger::logDebug("  Total frames rendered: " + std::to_string(m_frameCount));

    // Unsubscribe from config events
    unsubscribeFromConfigEvents();

    // Make context current for cleanup
    makeCurrent();

    // Cleanup active visualizer
    cleanupVisualizer();

    doneCurrent();
}

// =============================================================================
// WidgetBase Overrides
// =============================================================================

void VisualizerWidget::onStartUpdates()
{
    BasicLogger::logDebug("VisualizerWidget::onStartUpdates()");
    // Could start a render timer here if needed
    update();
}

void VisualizerWidget::onStopUpdates()
{
    BasicLogger::logDebug("VisualizerWidget::onStopUpdates()");
    // Could stop render timer here
}

// =============================================================================
// Config Event Subscription
// =============================================================================

void VisualizerWidget::subscribeToConfigEvents()
{
    auto* bus = eventBus();
    if (!bus)
    {
        BasicLogger::logWarning("VisualizerWidget: No EventBus available for config events");
        return;
    }
    
    // Color Scheme Event
    m_configSubscriptionIds.push_back(
        bus->subscribe<VisualizerColorSchemeEvent>(
            [this](const VisualizerColorSchemeEvent& evt) {
                if (!m_visualizer)
                {
                    return;
                }
                
                // Try to cast to PulsingVisualizer
                auto* pulsing = dynamic_cast<PulsingVisualizer*>(m_visualizer.get());
                if (pulsing)
                {
                    auto scheme = static_cast<lumi::modules::ColorSchemeType>(evt.schemeIndex);
                    pulsing->setColorScheme(scheme);
                    BasicLogger::logDebug("VisualizerWidget: Applied color scheme " + 
                                          std::to_string(evt.schemeIndex));
                }
            }
        )
    );
    
    // Smoothing Event
    m_configSubscriptionIds.push_back(
        bus->subscribe<VisualizerSmoothingEvent>(
            [this](const VisualizerSmoothingEvent& evt) {
                if (!m_visualizer)
                {
                    return;
                }
                
                auto* pulsing = dynamic_cast<PulsingVisualizer*>(m_visualizer.get());
                if (pulsing)
                {
                    // Convert 0-1 to milliseconds (0-500ms range)
                    float ms = evt.smoothingFactor * 500.0f;
                    pulsing->setSmoothingTime(ms);
                    BasicLogger::logDebug("VisualizerWidget: Applied smoothing " + 
                                          std::to_string(ms) + "ms");
                }
            }
        )
    );
    
    // Peak Hold Event
    m_configSubscriptionIds.push_back(
        bus->subscribe<VisualizerPeakHoldEvent>(
            [this](const VisualizerPeakHoldEvent& evt) {
                if (!m_visualizer)
                {
                    return;
                }
                
                auto* pulsing = dynamic_cast<PulsingVisualizer*>(m_visualizer.get());
                if (pulsing)
                {
                    pulsing->setBeatBrightnessEnabled(evt.enabled);
                    BasicLogger::logDebug("VisualizerWidget: Peak hold " + 
                                          std::string(evt.enabled ? "enabled" : "disabled"));
                }
            }
        )
    );
    
    BasicLogger::logDebug("VisualizerWidget: Subscribed to config events");
}

void VisualizerWidget::unsubscribeFromConfigEvents()
{
    auto* bus = eventBus();
    if (!bus)
    {
        return;
    }
    
    for (int id : m_configSubscriptionIds)
    {
        bus->unsubscribe(id);
    }
    m_configSubscriptionIds.clear();
    
    BasicLogger::logDebug("VisualizerWidget: Unsubscribed from config events");
}

// =============================================================================
// Visualizer Management
// =============================================================================

bool VisualizerWidget::setVisualizer(const QString& id)
{
    BasicLogger::logInfo("VisualizerWidget::setVisualizer(\"" + id.toStdString() + "\")");
    BasicLogger::logDebug("  m_glInitialized=" + std::to_string(m_glInitialized));

    // Check if already active
    if (id == m_currentVisualizerId && m_visualizer != nullptr)
    {
        BasicLogger::logDebug("  Visualizer already active");
        return true;
    }

    // Check if registered
    auto& registry = VisualizerRegistry::instance();
    BasicLogger::logDebug("  Registry has " + std::to_string(registry.descriptors().size()) + " visualizers");
    
    if (!registry.has(id.toStdString()))
    {
        QString error = QStringLiteral("Visualizer not found: ") + id;
        BasicLogger::logWarning("  " + error.toStdString());
        Q_EMIT visualizerError(id, error);
        return false;
    }

    // Make context current for OpenGL operations
    makeCurrent();

    // Cleanup current visualizer
    cleanupVisualizer();

    // Create new visualizer
    BasicLogger::logDebug("  Creating visualizer from registry...");
    m_visualizer = registry.create(id.toStdString());
    if (m_visualizer == nullptr)
    {
        QString error = QStringLiteral("Failed to create visualizer: ") + id;
        BasicLogger::logError("  " + error.toStdString());
        Q_EMIT visualizerError(id, error);
        doneCurrent();
        return false;
    }
    BasicLogger::logDebug("  Visualizer created: " + std::to_string(reinterpret_cast<uintptr_t>(m_visualizer.get())));

    m_currentVisualizerId = id;

    // Initialize if OpenGL is ready
    if (m_glInitialized)
    {
        BasicLogger::logInfo("  Calling visualizer->initialize()...");
        m_visualizer->initialize();
        BasicLogger::logInfo("  isInitialized after initialize(): " + std::to_string(m_visualizer->isInitialized()));
        
        m_visualizer->resize(size());
        BasicLogger::logInfo("  Visualizer initialized: " + 
                             m_visualizer->visualizerName().toStdString());
    }
    else
    {
        BasicLogger::logWarning("  OpenGL NOT initialized yet - visualizer will be initialized in initializeGL()");
    }

    doneCurrent();

    // Publish event via EventBus
    auto* bus = eventBus();
    if (bus != nullptr)
    {
        bus->publish(VisualizerChangedEvent{
            id.toStdString(),
            m_visualizer->visualizerName().toStdString(),
            static_cast<void*>(m_visualizer.get())
        });
    }

    Q_EMIT visualizerChanged(id);
    update(); // Request repaint

    return true;
}

QString VisualizerWidget::currentVisualizerName() const
{
    if (m_visualizer != nullptr)
    {
        return m_visualizer->visualizerName();
    }
    return QString();
}

void VisualizerWidget::loadDefaultVisualizer()
{
    // Try to load "pulsing" as default
    if (!setVisualizer(QStringLiteral("pulsing")))
    {
        // If pulsing not available, try first registered visualizer
        auto& registry = VisualizerRegistry::instance();
        auto descriptors = registry.descriptors();
        
        if (!descriptors.empty())
        {
            setVisualizer(QString::fromStdString(descriptors[0].id));
        }
        else
        {
            BasicLogger::logWarning("No visualizers registered!");
        }
    }
}

void VisualizerWidget::cleanupVisualizer()
{
    if (m_visualizer != nullptr)
    {
        BasicLogger::logDebug("Cleaning up visualizer: " + m_currentVisualizerId.toStdString());
        
        if (m_visualizer->isInitialized())
        {
            m_visualizer->cleanup();
        }
        
        m_visualizer.reset();
        m_currentVisualizerId.clear();
    }
}

// =============================================================================
// Audio Data Pass-Through
// =============================================================================

void VisualizerWidget::updateSpectrum(const float* spectrum, int count)
{
    if (m_visualizer != nullptr)
    {
        m_visualizer->updateSpectrum(spectrum, count);
    }
}

void VisualizerWidget::updateWaveform(const float* waveform, int count)
{
    if (m_visualizer != nullptr)
    {
        m_visualizer->updateWaveform(waveform, count);
    }
}

// =============================================================================
// Public Interface
// =============================================================================

void VisualizerWidget::setClearColor(float r, float g, float b, float a)
{
    m_clearR = r;
    m_clearG = g;
    m_clearB = b;
    m_clearA = a;
    update();
}

void VisualizerWidget::setVSync(bool enabled)
{
    int interval = enabled ? 1 : 0;
    
    makeCurrent();
    
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (ctx == nullptr)
    {
        BasicLogger::logWarning("setVSync: No OpenGL context available");
        doneCurrent();
        return;
    }
    
#if defined(_WIN32)
    if (wglSwapIntervalEXT == nullptr)
    {
        wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
            ctx->getProcAddress("wglSwapIntervalEXT"));
    }
    
    if (wglSwapIntervalEXT != nullptr)
    {
        wglSwapIntervalEXT(interval);
        BasicLogger::logInfo("VSync " + std::string(enabled ? "ENABLED" : "DISABLED") + 
                             " (wglSwapIntervalEXT)");
    }
    else
    {
        BasicLogger::logWarning("wglSwapIntervalEXT not available");
    }
    
#elif defined(__linux__) && !defined(__ANDROID__)
    // Try EGL first (works on both Wayland and X11 with EGL)
    bool success = false;
    QString platform = QGuiApplication::platformName();
    
    if (libEGL == nullptr)
    {
        libEGL = dlopen("libEGL.so.1", RTLD_LAZY);
        if (libEGL != nullptr)
        {
            eglSwapIntervalFunc = reinterpret_cast<PFNEGLSWAPINTERVALPROC>(
                dlsym(libEGL, "eglSwapInterval"));
        }
    }
    
    if (eglSwapIntervalFunc != nullptr)
    {
        QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
        if (native != nullptr)
        {
            void* eglDisplay = native->nativeResourceForWindow("egldisplay", windowHandle());
            if (eglDisplay != nullptr)
            {
                eglSwapIntervalFunc(eglDisplay, interval);
                BasicLogger::logInfo("VSync " + std::string(enabled ? "ENABLED" : "DISABLED") + 
                                     " (eglSwapInterval)");
                success = true;
            }
        }
    }
    
    // Try GLX Mesa extension
    if (!success && glXSwapIntervalMESA == nullptr)
    {
        glXSwapIntervalMESA = reinterpret_cast<PFNGLXSWAPINTERVALMESAPROC>(
            ctx->getProcAddress("glXSwapIntervalMESA"));
    }
    
    if (!success && glXSwapIntervalMESA != nullptr)
    {
        glXSwapIntervalMESA(interval);
        BasicLogger::logInfo("VSync " + std::string(enabled ? "ENABLED" : "DISABLED") + 
                             " (glXSwapIntervalMESA)");
        success = true;
    }
    
    // Try GLX EXT extension
    if (!success && glXSwapIntervalEXT == nullptr)
    {
        glXSwapIntervalEXT = reinterpret_cast<PFNGLXSWAPINTERVALEXTPROC>(
            ctx->getProcAddress("glXSwapIntervalEXT"));
    }
    
    if (!success && glXSwapIntervalEXT != nullptr)
    {
        QPlatformNativeInterface* native = QGuiApplication::platformNativeInterface();
        if (native != nullptr)
        {
            void* display = native->nativeResourceForWindow("display", windowHandle());
            void* drawable = native->nativeResourceForWindow("drawable", windowHandle());
            
            if (display != nullptr && drawable != nullptr)
            {
                auto glxDrawable = *reinterpret_cast<unsigned long*>(drawable);
                glXSwapIntervalEXT(display, glxDrawable, interval);
                BasicLogger::logInfo("VSync " + std::string(enabled ? "ENABLED" : "DISABLED") + 
                                     " (glXSwapIntervalEXT)");
                success = true;
            }
        }
    }
    
    if (!success)
    {
        BasicLogger::logWarning("VSync control not available on " + platform.toStdString());
    }
    
#elif defined(__APPLE__)
    CGLContextObj cglContext = CGLGetCurrentContext();
    if (cglContext != nullptr)
    {
        GLint swapInterval = interval;
        CGLSetParameter(cglContext, kCGLCPSwapInterval, &swapInterval);
        BasicLogger::logInfo("VSync " + std::string(enabled ? "ENABLED" : "DISABLED") + 
                             " (CGLSetParameter)");
    }
    else
    {
        BasicLogger::logWarning("CGLGetCurrentContext returned null");
    }
#endif

    doneCurrent();
}

// =============================================================================
// QOpenGLWidget Virtual Methods
// =============================================================================

void VisualizerWidget::initializeGL()
{
    BasicLogger::logInfo("VisualizerWidget::initializeGL()");

    // Initialize OpenGL functions
    initializeOpenGLFunctions();

    // Log OpenGL information
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glsl = reinterpret_cast<const char*>(
        glGetString(GL_SHADING_LANGUAGE_VERSION));

    BasicLogger::logInfo("  OpenGL Vendor:   " + std::string(vendor ? vendor : "N/A"));
    BasicLogger::logInfo("  OpenGL Renderer: " + std::string(renderer ? renderer : "N/A"));
    BasicLogger::logInfo("  OpenGL Version:  " + std::string(version ? version : "N/A"));
    BasicLogger::logInfo("  GLSL Version:    " + std::string(glsl ? glsl : "N/A"));

    // Set initial OpenGL state
    glClearColor(m_clearR, m_clearG, m_clearB, m_clearA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_glInitialized = true;

    // Initialize visualizer if already set
    BasicLogger::logDebug("  Checking visualizer: ptr=" + 
                          std::to_string(reinterpret_cast<uintptr_t>(m_visualizer.get())) +
                          ", id=" + m_currentVisualizerId.toStdString());
    
    if (m_visualizer != nullptr && !m_visualizer->isInitialized())
    {
        BasicLogger::logInfo("  Initializing existing visualizer: " + m_currentVisualizerId.toStdString());
        m_visualizer->initialize();
        m_visualizer->resize(size());
        BasicLogger::logInfo("  Visualizer initialized, isInitialized=" + 
                             std::to_string(m_visualizer->isInitialized()));
    }
    else if (m_visualizer == nullptr)
    {
        BasicLogger::logDebug("  No visualizer set, loading default");
        // Load default visualizer
        loadDefaultVisualizer();
    }
    else
    {
        BasicLogger::logDebug("  Visualizer already initialized");
    }

    BasicLogger::logInfo("  Initialization complete");
}

void VisualizerWidget::resizeGL(int w, int h)
{
    BasicLogger::logDebug("VisualizerWidget::resizeGL(" +
                          std::to_string(w) + ", " + std::to_string(h) + ")");

    glViewport(0, 0, w, h);

    // Notify visualizer
    if (m_visualizer != nullptr && m_visualizer->isInitialized())
    {
        m_visualizer->resize(QSize(w, h));
    }
}

void VisualizerWidget::paintGL()
{
    // Calculate delta time
    float currentTime = m_frameTimer.elapsed() / 1000.0f;
    float deltaTime = currentTime - m_lastFrameTime;
    m_lastFrameTime = currentTime;

    // DEBUG: Log state for first frames
    static int debugCount = 0;
    debugCount++;
    
    if (debugCount <= 10 || debugCount % 300 == 0)
    {
        BasicLogger::logDebug("paintGL frame " + std::to_string(debugCount) +
                              ": visualizer=" + std::to_string(reinterpret_cast<uintptr_t>(m_visualizer.get())) +
                              ", initialized=" + std::to_string(m_visualizer ? m_visualizer->isInitialized() : -1) +
                              ", id=" + m_currentVisualizerId.toStdString());
    }

    // Delegate rendering to active visualizer
    if (m_visualizer != nullptr && m_visualizer->isInitialized())
    {
        m_visualizer->render(deltaTime);
    }
    else
    {
        // Fallback: clear with RED so we see there's a problem!
        glClearColor(0.8f, 0.0f, 0.0f, 1.0f);  // RED = no visualizer!
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (debugCount <= 10)
        {
            BasicLogger::logWarning("paintGL: Using fallback clear (RED) - visualizer not ready!");
        }
    }

    // Frame counter
    m_frameCount++;

    if ((m_frameCount % 300) == 0)
    {
        BasicLogger::logDebug("VisualizerWidget::paintGL() frame " + 
                              std::to_string(m_frameCount) +
                              " (visualizer: " + m_currentVisualizerId.toStdString() + ")");
    }
}
