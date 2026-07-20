/**
 ****************************************************************************************
 * @file   ScopeRenderer.cpp
 * @brief  Implementation of the reusable scope point/line renderer (Roadmap 5.4d)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 ****************************************************************************************
 */

#include "visualizers/render/ScopeRenderer.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <cmath>
#include <utility>

namespace lumi::render {

using lumi::modules::SuperscopePoint;
using lumi::modules::SuperscopeRenderMode;

namespace {

// Shaders match SuperscopeVisualizer 1:1 (point size + soft glow, thin lines).
const char* kPointVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
out vec4 vColor;
uniform float uPointSize;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    gl_PointSize = uPointSize;
    vColor = aColor;
}
)";

const char* kPointFragmentShader = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
uniform bool uGlowEnabled;
uniform float uGlowIntensity;
void main()
{
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);
    if (dist > 1.0) discard;
    float alpha = 1.0;
    if (uGlowEnabled)
    {
        alpha = exp(-dist * dist * 2.0) * uGlowIntensity + (1.0 - dist) * 0.5;
        alpha = clamp(alpha, 0.0, 1.0);
    }
    else
    {
        alpha = 1.0 - smoothstep(0.8, 1.0, dist);
    }
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
)";

const char* kLineVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
out vec4 vColor;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)";

const char* kLineFragmentShader = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
uniform float uAlpha;
void main()
{
    FragColor = vec4(vColor.rgb, vColor.a * uAlpha);
}
)";

} // namespace

bool ScopeRenderer::ensure()
{
    if (ready()) return true;

    auto makeProgram = [](const char* vs, const char* fs) {
        auto program = std::make_unique<QOpenGLShaderProgram>();
        if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vs) ||
            !program->addShaderFromSourceCode(QOpenGLShader::Fragment, fs) ||
            !program->link())
        {
            return std::unique_ptr<QOpenGLShaderProgram>{};
        }
        return program;
    };

    m_pointShader = makeProgram(kPointVertexShader, kPointFragmentShader);
    m_lineShader = makeProgram(kLineVertexShader, kLineFragmentShader);
    if (m_pointShader == nullptr || m_lineShader == nullptr)
    {
        m_pointShader.reset();
        m_lineShader.reset();
        return false;
    }

    m_pointUniPointSize = m_pointShader->uniformLocation("uPointSize");
    m_pointUniGlowEnabled = m_pointShader->uniformLocation("uGlowEnabled");
    m_pointUniGlowIntensity = m_pointShader->uniformLocation("uGlowIntensity");
    m_lineUniAlpha = m_lineShader->uniformLocation("uAlpha");

    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    m_vao->create();
    m_vbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_vbo->create();
    m_vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    return true;
}

void ScopeRenderer::destroy()
{
    m_pointShader.reset();
    m_lineShader.reset();
    m_vao.reset();
    m_vbo.reset();
    m_pointUniPointSize = -1;
    m_pointUniGlowEnabled = -1;
    m_pointUniGlowIntensity = -1;
    m_lineUniAlpha = -1;
}

void ScopeRenderer::draw(const std::vector<SuperscopePoint>& points,
                         const Params& params)
{
    if (!ready() || points.empty()) return;

    switch (params.mode)
    {
        case SuperscopeRenderMode::Dots:
            renderDots(points, params);
            break;
        case SuperscopeRenderMode::Lines:
            if (params.lineWidth > 1.0f) renderThickLines(points, params.lineWidth);
            else renderThinLines(points);
            break;
        case SuperscopeRenderMode::ThickLines:
            renderThickLines(points, params.lineWidth);
            break;
    }
}

void ScopeRenderer::uploadVertexData(const std::vector<float>& vertices)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    m_vao->bind();
    m_vbo->bind();
    m_vbo->allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                             reinterpret_cast<void*>(2 * sizeof(float)));

    m_vbo->release();
    m_vao->release();
}

void ScopeRenderer::renderDots(const std::vector<SuperscopePoint>& points,
                               const Params& params)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    std::vector<float> vertices;
    vertices.reserve(points.size() * 6);
    for (const auto& pt : points)
    {
        if (pt.skip) continue;
        vertices.insert(vertices.end(), {pt.x, pt.y, pt.r, pt.g, pt.b, pt.a});
    }
    if (vertices.empty()) return;

    uploadVertexData(vertices);

    f->glEnable(GL_PROGRAM_POINT_SIZE);
    m_pointShader->bind();
    m_vao->bind();
    m_pointShader->setUniformValue(m_pointUniPointSize, params.dotSize);
    m_pointShader->setUniformValue(m_pointUniGlowEnabled, params.glowEnabled);
    m_pointShader->setUniformValue(m_pointUniGlowIntensity, params.glowIntensity);
    f->glDrawArrays(GL_POINTS, 0, static_cast<int>(vertices.size() / 6));
    m_vao->release();
    m_pointShader->release();
}

void ScopeRenderer::renderThinLines(const std::vector<SuperscopePoint>& points)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    std::vector<float> vertices;
    vertices.reserve(points.size() * 6);
    std::vector<std::pair<int, int>> segments;
    int currentStart = 0;
    int currentCount = 0;

    for (const auto& pt : points)
    {
        if (pt.skip)
        {
            if (currentCount > 0) segments.emplace_back(currentStart, currentCount);
            currentStart = static_cast<int>(vertices.size() / 6);
            currentCount = 0;
            continue;
        }
        vertices.insert(vertices.end(), {pt.x, pt.y, pt.r, pt.g, pt.b, pt.a});
        ++currentCount;
    }
    if (currentCount > 0) segments.emplace_back(currentStart, currentCount);
    if (vertices.empty()) return;

    uploadVertexData(vertices);

    m_lineShader->bind();
    m_vao->bind();
    m_lineShader->setUniformValue(m_lineUniAlpha, 1.0f);
    f->glLineWidth(1.0f);
    for (const auto& seg : segments)
    {
        if (seg.second >= 2) f->glDrawArrays(GL_LINE_STRIP, seg.first, seg.second);
    }
    m_vao->release();
    m_lineShader->release();
}

void ScopeRenderer::renderThickLines(const std::vector<SuperscopePoint>& points,
                                     float lineWidth)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    const float pixelHeight = viewport[3] > 0 ? 2.0f / static_cast<float>(viewport[3])
                                              : 0.002f;
    const float halfWidth = lineWidth * pixelHeight * 0.5f;

    std::vector<float> vertices;
    vertices.reserve(points.size() * 2 * 6);
    std::vector<std::pair<int, int>> segments;
    int currentStart = 0;
    int currentCount = 0;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto& pt = points[i];
        if (pt.skip)
        {
            if (currentCount >= 4) segments.push_back({currentStart, currentCount});
            currentStart = static_cast<int>(vertices.size() / 6);
            currentCount = 0;
            continue;
        }

        // Segment normal (from the next non-skip point, else the previous one).
        float nx = 0.0f;
        float ny = 1.0f;
        if (i < points.size() - 1 && !points[i + 1].skip)
        {
            const float dx = points[i + 1].x - pt.x;
            const float dy = points[i + 1].y - pt.y;
            const float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.0001f) { nx = -dy / len; ny = dx / len; }
        }
        else if (i > 0 && !points[i - 1].skip)
        {
            const float dx = pt.x - points[i - 1].x;
            const float dy = pt.y - points[i - 1].y;
            const float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.0001f) { nx = -dy / len; ny = dx / len; }
        }

        vertices.insert(vertices.end(), {pt.x + nx * halfWidth, pt.y + ny * halfWidth,
                                         pt.r, pt.g, pt.b, pt.a});
        vertices.insert(vertices.end(), {pt.x - nx * halfWidth, pt.y - ny * halfWidth,
                                         pt.r, pt.g, pt.b, pt.a});
        currentCount += 2;
    }
    if (currentCount >= 4) segments.push_back({currentStart, currentCount});
    if (vertices.empty()) return;

    uploadVertexData(vertices);

    m_lineShader->bind();
    m_vao->bind();
    m_lineShader->setUniformValue(m_lineUniAlpha, 1.0f);
    for (const auto& seg : segments)
    {
        if (seg.second >= 4) f->glDrawArrays(GL_TRIANGLE_STRIP, seg.first, seg.second);
    }
    m_vao->release();
    m_lineShader->release();
}

} // namespace lumi::render
