/**
 ****************************************************************************************
 * @file   VisualizerWidget.hpp
 * @brief  OpenGL Visualization Widget - Qt6 Tutorial
 *         Hardware-accelerated rendering with VSync support
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This file demonstrates:
 *   - QOpenGLWidget inheritance
 *   - OpenGL context management
 *   - VSync through SwapBuffers
 *   - Protected virtual methods (initializeGL, paintGL, resizeGL)
 *
 * ## Qt6 Tutorial: QOpenGLWidget
 *
 * QOpenGLWidget is Qt's integration of OpenGL into the widget system.
 * It provides:
 *   - Automatic OpenGL context creation
 *   - Double buffering with VSync
 *   - Integration with Qt's event system
 *   - Resize handling
 *
 * ### Key Virtual Methods
 *
 * ```
 * initializeGL()  → Called once when context is created
 *                   Setup shaders, VAOs, textures here
 *
 * resizeGL(w, h)  → Called when widget size changes
 *                   Update viewport, projection matrix here
 *
 * paintGL()       → Called every frame (or on update())
 *                   Do all rendering here
 * ```
 *
 * @see VisualizerWidget.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Qt Includes
// =============================================================================

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

// =============================================================================
// Forward Declarations
// =============================================================================

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

/**
 * @class VisualizerWidget
 * @brief OpenGL widget for audio visualization rendering.
 *
 * VisualizerWidget provides a hardware-accelerated canvas for rendering
 * audio visualizations. It inherits from QOpenGLWidget and QOpenGLFunctions.
 *
 * ## Qt6 Tutorial: Inheritance
 *
 * We inherit from TWO classes:
 *
 * 1. **QOpenGLWidget** - Provides the widget with OpenGL context
 *    - Handles context creation/destruction
 *    - Manages double buffering
 *    - Integrates with Qt's paint system
 *
 * 2. **QOpenGLFunctions** - Provides OpenGL function pointers
 *    - Cross-platform OpenGL function access
 *    - No need for GLAD/GLEW in simple cases
 *    - Call initializeOpenGLFunctions() in initializeGL()
 *
 * ## Rendering Flow
 *
 * ```
 * Widget created
 *       │
 *       ▼
 * initializeGL() ─────► Setup (once)
 *       │
 *       ▼
 * resizeGL(w,h) ──────► Viewport setup
 *       │
 *       ▼
 * ┌─────────────┐
 * │  paintGL()  │◄───── Called on update() or timer
 * │  (render)   │
 * └─────────────┘
 *       │
 *       ▼
 * [SwapBuffers] ──────► VSync wait happens here
 * ```
 */
class VisualizerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    // =========================================================================
    // Construction / Destruction
    // =========================================================================

    /**
     * @brief Constructs the VisualizerWidget.
     *
     * @param parent Parent widget (typically MainWindow's central widget)
     *
     * ## Qt6 Tutorial: Constructor
     *
     * The constructor should be lightweight. Don't call any OpenGL functions
     * here - the context doesn't exist yet!
     *
     * OpenGL initialization happens in initializeGL(), which is called
     * automatically when the widget is first shown.
     */
    explicit VisualizerWidget(QWidget* parent = nullptr);

    /**
     * @brief Destructor.
     *
     * Cleanup OpenGL resources here. The context is still valid in the
     * destructor, so it's safe to delete shaders, buffers, etc.
     */
    ~VisualizerWidget() override;

    // Non-copyable
    VisualizerWidget(const VisualizerWidget&) = delete;
    VisualizerWidget& operator=(const VisualizerWidget&) = delete;
    VisualizerWidget(VisualizerWidget&&) = delete;
    VisualizerWidget& operator=(VisualizerWidget&&) = delete;

    // =========================================================================
    // Public Interface
    // =========================================================================

    /**
     * @brief Sets the clear color (background).
     *
     * @param r Red component (0.0 - 1.0)
     * @param g Green component (0.0 - 1.0)
     * @param b Blue component (0.0 - 1.0)
     * @param a Alpha component (0.0 - 1.0), default 1.0
     *
     * Takes effect on next paintGL() call.
     */
    void setClearColor(float r, float g, float b, float a = 1.0f);

    /**
     * @brief Enables or disables VSync at runtime.
     *
     * @param enabled true = VSync ON, false = VSync OFF
     *
     * Uses platform-specific APIs:
     *   - Windows: wglSwapIntervalEXT
     *   - Linux: glXSwapIntervalEXT
     *   - macOS: CGLSetParameter
     *
     * @note Must be called after OpenGL context is initialized.
     */
    void setVSync(bool enabled);

    // =========================================================================
    // Future Interface (TODO)
    // =========================================================================

    // void setAudioData(const float* spectrum, int size);
    // void setVisualizationMode(VisualizationMode mode);

protected:
    // =========================================================================
    // QOpenGLWidget Virtual Methods
    // =========================================================================

    /**
     * @brief Called once when the OpenGL context is created.
     *
     * ## Qt6 Tutorial: initializeGL()
     *
     * This is where you set up all OpenGL resources:
     *   - Initialize OpenGL functions (initializeOpenGLFunctions())
     *   - Create and compile shaders
     *   - Create VAOs and VBOs
     *   - Load textures
     *   - Set initial OpenGL state
     *
     * @warning Called ONCE, not every frame. Don't do per-frame work here.
     */
    void initializeGL() override;

    /**
     * @brief Called when the widget is resized.
     *
     * @param w New width in pixels
     * @param h New height in pixels
     *
     * ## Qt6 Tutorial: resizeGL()
     *
     * Update the viewport and projection matrix here:
     *
     * ```cpp
     * void resizeGL(int w, int h)
     * {
     *     glViewport(0, 0, w, h);
     *     // Update projection matrix...
     * }
     * ```
     *
     * Also called once after initializeGL() with initial size.
     */
    void resizeGL(int w, int h) override;

    /**
     * @brief Called every frame to render.
     *
     * ## Qt6 Tutorial: paintGL()
     *
     * This is your render loop. Called when:
     *   - Widget needs repainting (expose, resize)
     *   - You call update() to request a repaint
     *
     * Typical structure:
     *
     * ```cpp
     * void paintGL()
     * {
     *     // 1. Clear
     *     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     *
     *     // 2. Set up transformations
     *     // ...
     *
     *     // 3. Draw
     *     glDrawArrays(...);
     *     // or glDrawElements(...)
     * }
     * ```
     *
     * @note SwapBuffers is called automatically after paintGL().
     *       This is where VSync waiting happens.
     */
    void paintGL() override;

private:
    // =========================================================================
    // Private Members
    // =========================================================================

    // Clear color (background)
    float m_clearR{0.1f};   // Dark blue-gray default
    float m_clearG{0.1f};
    float m_clearB{0.15f};
    float m_clearA{1.0f};

    // Frame counter (for debugging)
    uint64_t m_frameCount{0};

    // TODO: Add when implementing actual visualization
    // std::unique_ptr<QOpenGLShaderProgram> m_pShaderProgram;
    // GLuint m_vao{0};
    // GLuint m_vbo{0};
};
