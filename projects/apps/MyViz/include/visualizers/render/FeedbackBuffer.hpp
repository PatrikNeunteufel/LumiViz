/**
 ****************************************************************************************
 * @file   FeedbackBuffer.hpp
 * @brief  Double-buffered offscreen feedback (previous/current FBO pair with swap)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.1.0
 *
 * @details
 * Import-Fundament-Entwurf §3 (Roadmap 4.3): the MilkDrop/AVS feedback essence
 * as an opt-in render capability. A frame renders into the CURRENT buffer
 * (typically after drawing the PREVIOUS frame's texture dimmed/zoomed first),
 * then blits to the window framebuffer and swaps.
 *
 * Usage inside a visualizer's render (render thread, context current):
 *   m_feedback->ensure(w, h);                  // create/resize (blit-preserve, E1)
 *   m_feedback->beginFrame();                  // bind current FBO
 *   ...clear...
 *   m_feedback->drawPrevious(decay, zoom);     // echo of the last frame
 *   ...draw content...
 *   m_feedback->endFrame(defaultFbo, w, h);    // blit to screen + swap
 *
 * Lifecycle: all GL objects live in the render thread. Call destroy() from
 * onCleanup()/context-change handling (context current); the destructor
 * assumes the objects are already gone or the context still exists.
 * Resize policy (decision E1): the old current image is blitted into the new
 * previous buffer — trails survive window resizes (MilkDrop behavior).
 ****************************************************************************************
 */

#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <utility>

namespace lumi::render {

/**
 * @class FeedbackBuffer
 * @brief Previous/current FBO pair with an internal textured-quad echo pass
 */
class FeedbackBuffer
{
public:
    FeedbackBuffer() = default;
    ~FeedbackBuffer() = default;

    FeedbackBuffer(const FeedbackBuffer&) = delete;
    FeedbackBuffer& operator=(const FeedbackBuffer&) = delete;

    /// @brief Create or resize both buffers (needs a current GL context)
    /// @return false when FBOs/shader could not be created (feedback unusable)
    bool ensure(int width, int height);

    /// @brief Release all GL objects (call with the owning context current)
    void destroy();

    [[nodiscard]] bool ready() const { return m_current != nullptr && m_previous != nullptr; }

    /// @brief Bind the current buffer as render target
    void beginFrame();

    /**
     * @brief Draw the previous frame's texture as a fullscreen echo
     * @param decay Color multiplier 0..1 (1 = no fade)
     * @param zoom  Quad scale (>1 zooms the echo outward, <1 inward)
     */
    void drawPrevious(float decay, float zoom);

    /// @brief Blit current -> target framebuffer, then swap previous/current
    void endFrame(unsigned int targetFbo, int targetWidth, int targetHeight);

    /// @brief GL texture id of the previous frame (0 if not ready)
    [[nodiscard]] unsigned int previousTexture() const
    {
        return m_previous != nullptr ? m_previous->texture() : 0;
    }

    /// @brief GL texture id of the current frame (0 if not ready) — for callers
    ///        that present via their own composite pass instead of endFrame()
    [[nodiscard]] unsigned int currentTexture() const
    {
        return m_current != nullptr ? m_current->texture() : 0;
    }

    /// @brief Swap previous/current WITHOUT blitting to a target (the caller
    ///        presents the image itself, e.g. MilkDrop-style composite pass)
    void swapOnly() { std::swap(m_current, m_previous); }

    /// @brief FBO handle of the previous buffer (0 if not ready) — for callers
    ///        that need to read pixels back (1.1.0: Puffer-Wechsel-Fading, S66)
    [[nodiscard]] unsigned int previousFboHandle() const
    {
        return m_previous != nullptr ? m_previous->handle() : 0;
    }

    /// @brief FBO handle of the current buffer (0 if not ready)
    [[nodiscard]] unsigned int currentFboHandle() const
    {
        return m_current != nullptr ? m_current->handle() : 0;
    }

private:
    bool ensureQuadPipeline();

    std::unique_ptr<QOpenGLFramebufferObject> m_current;
    std::unique_ptr<QOpenGLFramebufferObject> m_previous;
    std::unique_ptr<QOpenGLShaderProgram> m_quadShader;
    std::unique_ptr<QOpenGLVertexArrayObject> m_quadVao;
    std::unique_ptr<QOpenGLBuffer> m_quadVbo;
    int m_width = 0;
    int m_height = 0;
    int m_uniDecay = -1;
    int m_uniZoom = -1;
};

} // namespace lumi::render
