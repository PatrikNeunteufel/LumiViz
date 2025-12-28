/**
 ****************************************************************************************
 * @file   VisualizerWidget.cpp
 * @brief  OpenGL Visualization Widget Implementation - Qt6 Tutorial
 *         Hardware-accelerated rendering with VSync support
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// =============================================================================
// Includes
// =============================================================================

#include "pch.h"
#include "UI/widget/VisualizerWidget.hpp"

// BasicLogger
#include <BasicLogger.h>

// =============================================================================
// Construction / Destruction
// =============================================================================

VisualizerWidget::VisualizerWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , QOpenGLFunctions()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Constructor
    // -------------------------------------------------------------------------
    // Keep the constructor lightweight!
    //
    // DON'T do any OpenGL calls here - the context doesn't exist yet.
    // OpenGL setup happens in initializeGL().
    //
    // What we CAN do here:
    //   - Set widget attributes
    //   - Connect signals/slots
    //   - Initialize non-OpenGL members

    BasicLogger::logDebug("VisualizerWidget constructor");

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Surface Format
    // -------------------------------------------------------------------------
    // QSurfaceFormat controls the OpenGL context properties.
    // We can request specific features here.
    //
    // Common settings:
    //   - setVersion(major, minor) - OpenGL version
    //   - setProfile(CoreProfile)  - Modern OpenGL (no deprecated functions)
    //   - setSwapBehavior(...)     - Single/Double/Triple buffering
    //   - setSwapInterval(1)       - VSync (1 = on, 0 = off)
    //   - setSamples(4)            - MSAA anti-aliasing

    QSurfaceFormat format;
    format.setVersion(3, 3);                              // OpenGL 3.3
    format.setProfile(QSurfaceFormat::CoreProfile);       // Core Profile (modern)
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer); // Double buffering
    format.setSwapInterval(1);                            // VSync ON (1 = wait for vsync)
    format.setDepthBufferSize(24);                        // 24-bit depth buffer
    format.setSamples(4);                                 // 4x MSAA

    setFormat(format);

    BasicLogger::logDebug("  Requested OpenGL 3.3 Core Profile");
    BasicLogger::logDebug("  VSync: ON (SwapInterval = 1)");
    BasicLogger::logDebug("  MSAA: 4x samples");
}

VisualizerWidget::~VisualizerWidget()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Destructor Cleanup
    // -------------------------------------------------------------------------
    // The OpenGL context is still valid here, so we can cleanup OpenGL resources.
    //
    // IMPORTANT: Call makeCurrent() before deleting OpenGL objects!
    // This ensures we're working with the correct context.

    BasicLogger::logDebug("VisualizerWidget destructor");
    BasicLogger::logDebug("  Total frames rendered: " + std::to_string(m_frameCount));

    // Make our context current for cleanup
    makeCurrent();

    // TODO: Delete OpenGL resources here
    // if (m_vao != 0)
    // {
    //     glDeleteVertexArrays(1, &m_vao);
    // }
    // if (m_vbo != 0)
    // {
    //     glDeleteBuffers(1, &m_vbo);
    // }
    // m_pShaderProgram.reset();

    // Release context
    doneCurrent();
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

    // Request repaint to show new color
    update();
}

// =============================================================================
// QOpenGLWidget Virtual Methods
// =============================================================================

void VisualizerWidget::initializeGL()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: initializeGL()
    // -------------------------------------------------------------------------
    // Called ONCE when the OpenGL context is first created.
    // This happens when the widget is first shown.
    //
    // MUST call initializeOpenGLFunctions() first!
    // This sets up the function pointers for OpenGL calls.

    BasicLogger::logInfo("VisualizerWidget::initializeGL()");

    // -------------------------------------------------------------------------
    // Step 1: Initialize OpenGL Functions
    // -------------------------------------------------------------------------
    // This is REQUIRED before any gl* calls!
    // QOpenGLFunctions provides cross-platform function loading.

    initializeOpenGLFunctions();

    // -------------------------------------------------------------------------
    // Log OpenGL Information
    // -------------------------------------------------------------------------
    // Useful for debugging and verifying the correct context was created.

    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glsl = reinterpret_cast<const char*>(
        glGetString(GL_SHADING_LANGUAGE_VERSION));

    BasicLogger::logInfo("  OpenGL Vendor:   " + std::string(vendor ? vendor : "N/A"));
    BasicLogger::logInfo("  OpenGL Renderer: " + std::string(renderer ? renderer : "N/A"));
    BasicLogger::logInfo("  OpenGL Version:  " + std::string(version ? version : "N/A"));
    BasicLogger::logInfo("  GLSL Version:    " + std::string(glsl ? glsl : "N/A"));

    // -------------------------------------------------------------------------
    // Step 2: Set Initial OpenGL State
    // -------------------------------------------------------------------------
    // Configure OpenGL defaults that won't change during rendering.

    // Set clear color (background)
    glClearColor(m_clearR, m_clearG, m_clearB, m_clearA);

    // Enable depth testing (for 3D rendering later)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable blending (for transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BasicLogger::logDebug("  Depth test: ENABLED");
    BasicLogger::logDebug("  Blending: ENABLED (SRC_ALPHA, ONE_MINUS_SRC_ALPHA)");

    // -------------------------------------------------------------------------
    // Step 3: Create Shaders (TODO)
    // -------------------------------------------------------------------------
    // Here we would compile and link our shader programs.
    //
    // Example:
    // m_pShaderProgram = std::make_unique<QOpenGLShaderProgram>();
    // m_pShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSrc);
    // m_pShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc);
    // m_pShaderProgram->link();

    // -------------------------------------------------------------------------
    // Step 4: Create Geometry (TODO)
    // -------------------------------------------------------------------------
    // Here we would create VAOs and VBOs for our visualization geometry.
    //
    // Example:
    // glGenVertexArrays(1, &m_vao);
    // glGenBuffers(1, &m_vbo);
    // ...

    BasicLogger::logInfo("  Initialization complete");
}

void VisualizerWidget::resizeGL(int w, int h)
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: resizeGL()
    // -------------------------------------------------------------------------
    // Called whenever the widget is resized, and once after initializeGL().
    //
    // Update the viewport to match the new size.
    // Also update projection matrices if needed.

    BasicLogger::logDebug("VisualizerWidget::resizeGL(" +
                          std::to_string(w) + ", " + std::to_string(h) + ")");

    // -------------------------------------------------------------------------
    // Update Viewport
    // -------------------------------------------------------------------------
    // glViewport tells OpenGL which portion of the window to render to.
    // (0, 0) is bottom-left corner.

    glViewport(0, 0, w, h);

    // -------------------------------------------------------------------------
    // Update Projection Matrix (TODO)
    // -------------------------------------------------------------------------
    // For 2D visualizations (spectrum bars, waveform):
    //   Use orthographic projection
    //
    // For 3D visualizations (MilkDrop-style):
    //   Use perspective projection
    //
    // Example orthographic (2D):
    // m_projection = glm::ortho(0.0f, (float)w, 0.0f, (float)h);
    //
    // Example perspective (3D):
    // float aspect = (float)w / (float)h;
    // m_projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

void VisualizerWidget::paintGL()
{
    // -------------------------------------------------------------------------
    // Qt6 Tutorial: paintGL()
    // -------------------------------------------------------------------------
    // Called every time the widget needs to be repainted.
    // After paintGL() returns, Qt calls swapBuffers() automatically.

    // -------------------------------------------------------------------------
    // Demo Animation: Pulsing Rainbow
    // -------------------------------------------------------------------------
    // Use time-based animation (not frame-based) for consistent speed.
    // This creates a visible color cycle to verify rendering works.

    // Get time in seconds since start
    static auto startTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    float time = std::chrono::duration<float>(now - startTime).count();

    // Create rainbow pulse effect (cycle through colors)
    float r = 0.5f + 0.5f * std::sin(time * 2.0f);           // Red
    float g = 0.5f + 0.5f * std::sin(time * 2.0f + 2.094f);  // Green (120° offset)
    float b = 0.5f + 0.5f * std::sin(time * 2.0f + 4.189f);  // Blue (240° offset)

    // Clear with animated color
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // -------------------------------------------------------------------------
    // Frame Counter (for statistics)
    // -------------------------------------------------------------------------
    m_frameCount++;

    // Log every 300 frames to verify paintGL is being called
    if ((m_frameCount % 300) == 0)
    {
        BasicLogger::logDebug("VisualizerWidget::paintGL() frame " + 
                              std::to_string(m_frameCount));
    }
}
