/**
 ****************************************************************************************
 * @file   ScopeRenderer.hpp
 * @brief  Reusable GL renderer for scope point clouds (dots / thin / thick lines)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 *
 * @details
 * The point/line drawing core of the Superscope, extracted so more than one
 * caller can use it (Import Roadmap 5.4d, decision E6). Given a vector of
 * `SuperscopePoint` (position in NDC, RGBA, skip flag) it draws them as GL
 * points, thin `GL_LINE_STRIP`, or thick triangle-strip lines — the exact
 * technique `SuperscopeVisualizer` uses today, in a standalone unit.
 *
 * GL objects live on the render thread (create in ensure(), free in destroy()).
 * Blend state is the caller's responsibility (the renderer draws with whatever
 * is set), matching the Superscope's external blend handling.
 *
 * First user: MultiEffectVisualizer's AVS SuperScope effect. Rewiring
 * SuperscopeVisualizer itself onto this class is the remaining de-dup half of
 * E6(a) — mechanical, tracked separately to keep the working visualizer stable.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/modules/SuperscopeModule.hpp"  // SuperscopePoint, RenderMode

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <memory>
#include <vector>

namespace lumi::render {

/**
 * @class ScopeRenderer
 * @brief Draws SuperscopePoint clouds as dots or (thin/thick) lines.
 */
class ScopeRenderer
{
public:
    struct Params
    {
        lumi::modules::SuperscopeRenderMode mode =
            lumi::modules::SuperscopeRenderMode::Lines;
        float lineWidth = 2.0f;
        float dotSize = 4.0f;
        bool glowEnabled = false;
        float glowIntensity = 0.5f;
    };

    ScopeRenderer() = default;
    ~ScopeRenderer() = default;
    ScopeRenderer(const ScopeRenderer&) = delete;
    ScopeRenderer& operator=(const ScopeRenderer&) = delete;

    /** Create shaders/VAO/VBO. Must run with a current context. */
    bool ensure();
    /** Free all GL objects (current context). */
    void destroy();
    [[nodiscard]] bool ready() const { return m_pointShader != nullptr; }

    /** Draw the points per `p.mode`. No-op until ensure() succeeded. */
    void draw(const std::vector<lumi::modules::SuperscopePoint>& points,
              const Params& params);

private:
    void uploadVertexData(const std::vector<float>& vertices);
    void renderDots(const std::vector<lumi::modules::SuperscopePoint>& points,
                    const Params& params);
    void renderThinLines(const std::vector<lumi::modules::SuperscopePoint>& points);
    void renderThickLines(const std::vector<lumi::modules::SuperscopePoint>& points,
                          float lineWidth);

    std::unique_ptr<QOpenGLShaderProgram> m_pointShader;
    std::unique_ptr<QOpenGLShaderProgram> m_lineShader;
    std::unique_ptr<QOpenGLVertexArrayObject> m_vao;
    std::unique_ptr<QOpenGLBuffer> m_vbo;

    int m_pointUniPointSize = -1;
    int m_pointUniGlowEnabled = -1;
    int m_pointUniGlowIntensity = -1;
    int m_lineUniAlpha = -1;
};

} // namespace lumi::render
