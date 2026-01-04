/**
 ****************************************************************************************
 * @file   SuperscopeVisualizer.cpp
 * @brief  Implementation of programmable point/line visualizer
 *
 * @author LumiPulse Team
 * @date   January 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/SuperscopeVisualizer.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <algorithm>
#include <cmath>

// =============================================================================
// Shader Sources
// =============================================================================

static const char* s_pointVertexShader = R"(
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

static const char* s_pointFragmentShader = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;

uniform bool uGlowEnabled;
uniform float uGlowIntensity;

void main()
{
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);
    
    if (dist > 1.0)
        discard;
    
    float alpha = 1.0;
    if (uGlowEnabled)
    {
        // Soft glow falloff
        alpha = exp(-dist * dist * 2.0) * uGlowIntensity + (1.0 - dist) * 0.5;
        alpha = clamp(alpha, 0.0, 1.0);
    }
    else
    {
        // Hard circle
        alpha = 1.0 - smoothstep(0.8, 1.0, dist);
    }
    
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
)";

static const char* s_lineVertexShader = R"(
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

static const char* s_lineFragmentShader = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;

uniform float uAlpha;

void main()
{
    FragColor = vec4(vColor.rgb, vColor.a * uAlpha);
}
)";

// =============================================================================
// Construction / Destruction
// =============================================================================

SuperscopeVisualizer::SuperscopeVisualizer()
    : VisualizerBase("superscope", "Superscope", "Programmable point/line visualizer")
{
    // Set default preset
    m_superscope.setPreset(lumi::modules::SuperscopePreset::Spiral);
}

SuperscopeVisualizer::~SuperscopeVisualizer()
{
    // OpenGL cleanup happens in onCleanup()
}

// =============================================================================
// IVisualizer Parameter Interface
// =============================================================================

std::vector<lumi::modules::ModuleParamDesc> SuperscopeVisualizer::paramDescs() const
{
    using namespace lumi::modules;
    std::vector<ModuleParamDesc> params;

    // =========================================================================
    // 1. Audio Section (AudioSourceModule)
    // =========================================================================
    for (const auto& p : m_audioSource.paramDescs())
    {
        ModuleParamDesc prefixed = p;
        prefixed.id = "audio." + p.id;
        prefixed.group = "1. Audio";
        
        // Prefix dependsOn if set
        if (!prefixed.dependsOn.empty())
        {
            prefixed.dependsOn = "audio." + prefixed.dependsOn;
        }
        
        params.push_back(prefixed);
    }

    // =========================================================================
    // 2. Superscope Parameters (with "scope." prefix)
    // =========================================================================
    auto scopeParams = m_superscope.paramDescs("scope.");
    for (auto& p : scopeParams)
    {
        // Set group to "2. Superscope" if not already set
        if (p.group.empty())
        {
            p.group = "2. Superscope";
        }
        params.push_back(std::move(p));
    }

    return params;
}

bool SuperscopeVisualizer::getParam(const std::string& id, lumi::modules::ParamValue& out) const
{
    // Audio parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.getParam(id.substr(6), out);
    }

    // Superscope module parameters
    if (id.rfind("scope.", 0) == 0)
    {
        std::string moduleId = id.substr(6);  // Remove "scope."
        return m_superscope.getParam(moduleId, out);
    }

    return false;
}

bool SuperscopeVisualizer::setParam(const std::string& id, const lumi::modules::ParamValue& value)
{
    // Audio parameters
    if (id.rfind("audio.", 0) == 0)
    {
        return m_audioSource.setParam(id.substr(6), value);
    }

    // Superscope module parameters
    if (id.rfind("scope.", 0) == 0)
    {
        std::string moduleId = id.substr(6);  // Remove "scope."
        return m_superscope.setParam(moduleId, value);
    }

    return false;
}

// =============================================================================
// IVisualizer Audio Interface
// =============================================================================

void SuperscopeVisualizer::updateWaveform(const float* waveform, int count)
{
    if (!waveform || count <= 0) return;

    // Get gain from AudioSourceModule
    float gain = m_audioSource.gain();

    // Assume interleaved stereo
    int samplesPerChannel = count / 2;
    
    m_waveformLeft.resize(samplesPerChannel);
    m_waveformRight.resize(samplesPerChannel);

    for (int i = 0; i < samplesPerChannel; ++i)
    {
        m_waveformLeft[i] = waveform[i * 2] * gain;
        m_waveformRight[i] = waveform[i * 2 + 1] * gain;
    }

    // Simple beat detection based on energy
    float energy = 0.0f;
    for (int i = 0; i < samplesPerChannel; ++i)
    {
        float sample = (m_waveformLeft[i] + m_waveformRight[i]) * 0.5f;
        energy += sample * sample;
    }
    energy = std::sqrt(energy / samplesPerChannel);

    // Adaptive threshold
    m_beatThreshold = m_beatThreshold * 0.95f + energy * 0.05f;
    m_isBeat = (energy > m_beatThreshold * 1.5f && energy > 0.1f);
    m_beatEnergy = energy;
}

void SuperscopeVisualizer::updateSpectrum(const float* spectrum, int count)
{
    if (!spectrum || count <= 0) return;

    // Get gain from AudioSourceModule
    float gain = m_audioSource.gain();

    // Assume interleaved stereo spectrum
    int binsPerChannel = count / 2;
    
    m_spectrumLeft.resize(binsPerChannel);
    m_spectrumRight.resize(binsPerChannel);

    for (int i = 0; i < binsPerChannel; ++i)
    {
        m_spectrumLeft[i] = spectrum[i * 2] * gain;
        m_spectrumRight[i] = spectrum[i * 2 + 1] * gain;
    }
}

// =============================================================================
// Reset
// =============================================================================

void SuperscopeVisualizer::resetToDefaults()
{
    m_superscope.setPreset(lumi::modules::SuperscopePreset::Spiral);
    m_superscope.resetState();
}

// =============================================================================
// Initialization
// =============================================================================

void SuperscopeVisualizer::onInitialize()
{
    auto* f = QOpenGLContext::currentContext()->functions();
    
    // Enable point sprites
    f->glEnable(GL_PROGRAM_POINT_SIZE);
    
    // Create shaders
    if (!createShaders())
    {
        qWarning("SuperscopeVisualizer: Failed to create shaders");
        return;
    }

    // Create VAO and VBO
    m_vao = std::make_unique<QOpenGLVertexArrayObject>();
    m_vao->create();
    m_vao->bind();

    m_vertexBuffer = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_vertexBuffer->create();
    m_vertexBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);

    m_vao->release();

    m_lastContext = QOpenGLContext::currentContext();
}

bool SuperscopeVisualizer::createShaders()
{
    // Point shader
    m_pointShader = std::make_unique<QOpenGLShaderProgram>();
    if (!m_pointShader->addShaderFromSourceCode(QOpenGLShader::Vertex, s_pointVertexShader))
    {
        qWarning("SuperscopeVisualizer: Point vertex shader compilation failed: %s",
                 qPrintable(m_pointShader->log()));
        return false;
    }
    if (!m_pointShader->addShaderFromSourceCode(QOpenGLShader::Fragment, s_pointFragmentShader))
    {
        qWarning("SuperscopeVisualizer: Point fragment shader compilation failed: %s",
                 qPrintable(m_pointShader->log()));
        return false;
    }
    if (!m_pointShader->link())
    {
        qWarning("SuperscopeVisualizer: Point shader linking failed: %s",
                 qPrintable(m_pointShader->log()));
        return false;
    }

    m_pointUniPointSize = m_pointShader->uniformLocation("uPointSize");
    m_pointUniGlowEnabled = m_pointShader->uniformLocation("uGlowEnabled");
    m_pointUniGlowIntensity = m_pointShader->uniformLocation("uGlowIntensity");

    // Line shader
    m_lineShader = std::make_unique<QOpenGLShaderProgram>();
    if (!m_lineShader->addShaderFromSourceCode(QOpenGLShader::Vertex, s_lineVertexShader))
    {
        qWarning("SuperscopeVisualizer: Line vertex shader compilation failed: %s",
                 qPrintable(m_lineShader->log()));
        return false;
    }
    if (!m_lineShader->addShaderFromSourceCode(QOpenGLShader::Fragment, s_lineFragmentShader))
    {
        qWarning("SuperscopeVisualizer: Line fragment shader compilation failed: %s",
                 qPrintable(m_lineShader->log()));
        return false;
    }
    if (!m_lineShader->link())
    {
        qWarning("SuperscopeVisualizer: Line shader linking failed: %s",
                 qPrintable(m_lineShader->log()));
        return false;
    }

    m_lineUniAlpha = m_lineShader->uniformLocation("uAlpha");

    return true;
}

// =============================================================================
// Rendering
// =============================================================================

void SuperscopeVisualizer::onRender(float deltaTime)
{
    auto* f = QOpenGLContext::currentContext()->functions();
    
    // Check for context change
    if (QOpenGLContext::currentContext() != m_lastContext)
    {
        onCleanup();
        onInitialize();
    }

    if (!m_vao || !m_vertexBuffer || !m_pointShader || !m_lineShader)
        return;

    m_totalTime += deltaTime;

    // Clear background
    f->glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);

    // Setup blending based on mode
    switch (m_superscope.blendMode())
    {
        case lumi::modules::SuperscopeBlendMode::Replace:
            f->glDisable(GL_BLEND);
            break;

        case lumi::modules::SuperscopeBlendMode::Additive:
            f->glEnable(GL_BLEND);
            f->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;

        case lumi::modules::SuperscopeBlendMode::Alpha:
            f->glEnable(GL_BLEND);
            f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
    }

    // Get viewport size
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    // Execute superscope to generate points
    const float* waveL = m_waveformLeft.empty() ? nullptr : m_waveformLeft.data();
    const float* waveR = m_waveformRight.empty() ? nullptr : m_waveformRight.data();
    const float* specL = m_spectrumLeft.empty() ? nullptr : m_spectrumLeft.data();
    const float* specR = m_spectrumRight.empty() ? nullptr : m_spectrumRight.data();
    int sampleCount = static_cast<int>(m_waveformLeft.size());

    auto points = m_superscope.execute(
        waveL, waveR, specL, specR,
        sampleCount, width, height,
        m_isBeat, deltaTime
    );

    // Hold/Fade: Update and render old frames first (behind current)
    if (m_superscope.holdEnabled())
    {
        updateHeldFrames(deltaTime);
        renderHeldFrames();

        // Add current frame to history
        HeldFrame frame;
        frame.points = points;
        frame.age = 0.0f;
        frame.alpha = 1.0f;
        m_heldFrames.push_back(std::move(frame));
    }

    // Render current frame based on mode
    switch (m_superscope.renderMode())
    {
        case lumi::modules::SuperscopeRenderMode::Dots:
            renderPoints(points);
            break;

        case lumi::modules::SuperscopeRenderMode::Lines:
            renderLines(points);
            break;

        case lumi::modules::SuperscopeRenderMode::ThickLines:
            renderThickLines(points);
            break;
    }

    // Reset beat flag
    m_isBeat = false;
}

void SuperscopeVisualizer::renderPoints(const std::vector<lumi::modules::SuperscopePoint>& points)
{
    if (points.empty()) return;

    auto* f = QOpenGLContext::currentContext()->functions();

    // Build vertex data: x, y, r, g, b, a
    std::vector<float> vertices;
    vertices.reserve(points.size() * 6);

    for (const auto& pt : points)
    {
        if (pt.skip) continue;

        vertices.push_back(pt.x);
        vertices.push_back(pt.y);
        vertices.push_back(pt.r);
        vertices.push_back(pt.g);
        vertices.push_back(pt.b);
        vertices.push_back(pt.a);
    }

    if (vertices.empty()) return;

    uploadVertexData(vertices);

    // Draw
    m_pointShader->bind();
    m_vao->bind();

    m_pointShader->setUniformValue(m_pointUniPointSize, m_superscope.dotSize());
    m_pointShader->setUniformValue(m_pointUniGlowEnabled, m_superscope.glowEnabled());
    m_pointShader->setUniformValue(m_pointUniGlowIntensity, m_superscope.glowIntensity());

    int vertexCount = static_cast<int>(vertices.size() / 6);
    f->glDrawArrays(GL_POINTS, 0, vertexCount);

    m_vao->release();
    m_pointShader->release();
}

void SuperscopeVisualizer::renderLines(const std::vector<lumi::modules::SuperscopePoint>& points)
{
    if (points.empty()) return;

    auto* f = QOpenGLContext::currentContext()->functions();

    float lineWidth = m_superscope.lineWidth();

    // Use triangle strips for line width > 1.0 (glLineWidth often limited to 1.0)
    if (lineWidth > 1.0f)
    {
        renderLinesAsTriangleStrip(points, lineWidth);
        return;
    }

    // Thin lines: Use GL_LINE_STRIP for performance
    std::vector<float> vertices;
    vertices.reserve(points.size() * 6);

    std::vector<std::pair<int, int>> segments;  // (start, count)
    int currentStart = 0;
    int currentCount = 0;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto& pt = points[i];

        if (pt.skip)
        {
            // End current segment
            if (currentCount > 0)
            {
                segments.emplace_back(currentStart, currentCount);
            }
            currentStart = static_cast<int>(vertices.size() / 6);
            currentCount = 0;
            continue;
        }

        vertices.push_back(pt.x);
        vertices.push_back(pt.y);
        vertices.push_back(pt.r);
        vertices.push_back(pt.g);
        vertices.push_back(pt.b);
        vertices.push_back(pt.a);
        ++currentCount;
    }

    // Add final segment
    if (currentCount > 0)
    {
        segments.emplace_back(currentStart, currentCount);
    }

    if (vertices.empty()) return;

    uploadVertexData(vertices);

    // Draw
    m_lineShader->bind();
    m_vao->bind();

    m_lineShader->setUniformValue(m_lineUniAlpha, 1.0f);
    f->glLineWidth(1.0f);

    for (const auto& seg : segments)
    {
        if (seg.second >= 2)
        {
            f->glDrawArrays(GL_LINE_STRIP, seg.first, seg.second);
        }
    }

    m_vao->release();
    m_lineShader->release();
}

void SuperscopeVisualizer::renderLinesAsTriangleStrip(const std::vector<lumi::modules::SuperscopePoint>& points,
                                                       float lineWidth)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    // Get pixel height for line width calculation
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    float pixelHeight = viewport[3] > 0 ? 2.0f / static_cast<float>(viewport[3]) : 0.002f;
    float halfWidth = lineWidth * pixelHeight * 0.5f;

    // Build triangle strip vertices
    std::vector<float> vertices;
    vertices.reserve(points.size() * 2 * 6);

    std::vector<std::pair<int, int>> segments;
    int currentSegmentStart = 0;
    int currentVertexCount = 0;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto& pt = points[i];

        if (pt.skip)
        {
            if (currentVertexCount >= 4)
            {
                segments.push_back({currentSegmentStart, currentVertexCount});
            }
            currentSegmentStart = static_cast<int>(vertices.size() / 6);
            currentVertexCount = 0;
            continue;
        }

        // Calculate normal direction
        float nx = 0.0f;
        float ny = 1.0f;

        if (i < points.size() - 1 && !points[i + 1].skip)
        {
            float dx = points[i + 1].x - pt.x;
            float dy = points[i + 1].y - pt.y;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.0001f)
            {
                nx = -dy / len;
                ny = dx / len;
            }
        }
        else if (i > 0 && !points[i - 1].skip)
        {
            float dx = pt.x - points[i - 1].x;
            float dy = pt.y - points[i - 1].y;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.0001f)
            {
                nx = -dy / len;
                ny = dx / len;
            }
        }

        // Top vertex
        vertices.push_back(pt.x + nx * halfWidth);
        vertices.push_back(pt.y + ny * halfWidth);
        vertices.push_back(pt.r);
        vertices.push_back(pt.g);
        vertices.push_back(pt.b);
        vertices.push_back(pt.a);

        // Bottom vertex
        vertices.push_back(pt.x - nx * halfWidth);
        vertices.push_back(pt.y - ny * halfWidth);
        vertices.push_back(pt.r);
        vertices.push_back(pt.g);
        vertices.push_back(pt.b);
        vertices.push_back(pt.a);

        currentVertexCount += 2;
    }

    if (currentVertexCount >= 4)
    {
        segments.push_back({currentSegmentStart, currentVertexCount});
    }

    if (vertices.empty()) return;

    uploadVertexData(vertices);

    m_lineShader->bind();
    m_vao->bind();

    m_lineShader->setUniformValue(m_lineUniAlpha, 1.0f);

    for (const auto& seg : segments)
    {
        if (seg.second >= 4)
        {
            f->glDrawArrays(GL_TRIANGLE_STRIP, seg.first, seg.second);
        }
    }

    m_vao->release();
    m_lineShader->release();
}

void SuperscopeVisualizer::renderThickLines(const std::vector<lumi::modules::SuperscopePoint>& points)
{
    if (points.empty()) return;

    auto* f = QOpenGLContext::currentContext()->functions();

    // Get pixel height for line width calculation
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    float pixelHeight = viewport[3] > 0 ? 2.0f / static_cast<float>(viewport[3]) : 0.002f;
    float halfWidth = m_superscope.lineWidth() * pixelHeight * 0.5f;

    // Build triangle strip vertices with normals for thickness
    // Format: x, y, r, g, b, a
    std::vector<float> vertices;
    vertices.reserve(points.size() * 2 * 6);  // 2 vertices per point

    std::vector<std::pair<int, int>> segments;  // (startVertex, vertexCount) pairs
    int currentSegmentStart = 0;
    int currentVertexCount = 0;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto& pt = points[i];

        // Handle skip flag - start new segment
        if (pt.skip)
        {
            if (currentVertexCount >= 4)  // Need at least 2 line points (4 vertices)
            {
                segments.push_back({currentSegmentStart, currentVertexCount});
            }
            currentSegmentStart = static_cast<int>(vertices.size() / 6);
            currentVertexCount = 0;
            continue;
        }

        // Calculate normal direction for thickness
        float nx = 0.0f;
        float ny = 1.0f;

        if (i < points.size() - 1 && !points[i + 1].skip)
        {
            // Use direction to next point
            float dx = points[i + 1].x - pt.x;
            float dy = points[i + 1].y - pt.y;
            float len = std::sqrt(dx * dx + dy * dy);

            if (len > 0.0001f)
            {
                // Perpendicular to direction
                nx = -dy / len;
                ny = dx / len;
            }
        }
        else if (i > 0 && !points[i - 1].skip)
        {
            // Use direction from previous point
            float dx = pt.x - points[i - 1].x;
            float dy = pt.y - points[i - 1].y;
            float len = std::sqrt(dx * dx + dy * dy);

            if (len > 0.0001f)
            {
                nx = -dy / len;
                ny = dx / len;
            }
        }

        // Top vertex (offset by +normal * halfWidth)
        vertices.push_back(pt.x + nx * halfWidth);
        vertices.push_back(pt.y + ny * halfWidth);
        vertices.push_back(pt.r);
        vertices.push_back(pt.g);
        vertices.push_back(pt.b);
        vertices.push_back(pt.a);

        // Bottom vertex (offset by -normal * halfWidth)
        vertices.push_back(pt.x - nx * halfWidth);
        vertices.push_back(pt.y - ny * halfWidth);
        vertices.push_back(pt.r);
        vertices.push_back(pt.g);
        vertices.push_back(pt.b);
        vertices.push_back(pt.a);

        currentVertexCount += 2;
    }

    // Don't forget the last segment
    if (currentVertexCount >= 4)
    {
        segments.push_back({currentSegmentStart, currentVertexCount});
    }

    if (vertices.empty()) return;

    uploadVertexData(vertices);

    // Draw triangle strips
    m_lineShader->bind();
    m_vao->bind();

    m_lineShader->setUniformValue(m_lineUniAlpha, 1.0f);

    for (const auto& seg : segments)
    {
        if (seg.second >= 4)  // Need at least 4 vertices for a visible triangle strip
        {
            f->glDrawArrays(GL_TRIANGLE_STRIP, seg.first, seg.second);
        }
    }

    m_vao->release();
    m_lineShader->release();

    // Glow pass: Draw points at vertices for soft glow effect
    if (m_superscope.glowEnabled())
    {
        f->glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive for glow
        renderPoints(points);
    }
}

// =============================================================================
// Hold/Fade
// =============================================================================

void SuperscopeVisualizer::updateHeldFrames(float deltaTime)
{
    float fadeTime = m_superscope.fadeTime();
    int maxFrames = m_superscope.maxHoldFrames();

    // Update age and alpha for each held frame
    for (auto& frame : m_heldFrames)
    {
        frame.age += deltaTime;
        frame.alpha = 1.0f - (frame.age / fadeTime);
        frame.alpha = std::max(0.0f, frame.alpha);
    }

    // Remove frames that have fully faded
    while (!m_heldFrames.empty() && m_heldFrames.front().alpha <= 0.0f)
    {
        m_heldFrames.pop_front();
    }

    // Limit to max frames
    while (m_heldFrames.size() > static_cast<size_t>(maxFrames))
    {
        m_heldFrames.pop_front();
    }
}

void SuperscopeVisualizer::renderHeldFrames()
{
    if (m_heldFrames.empty()) return;

    // Render each held frame with reduced alpha
    for (const auto& frame : m_heldFrames)
    {
        if (frame.alpha <= 0.01f) continue;

        // Create a modified copy with adjusted alpha
        std::vector<lumi::modules::SuperscopePoint> fadedPoints = frame.points;
        for (auto& pt : fadedPoints)
        {
            pt.a *= frame.alpha;
        }

        // Render based on current mode
        switch (m_superscope.renderMode())
        {
            case lumi::modules::SuperscopeRenderMode::Dots:
                renderPoints(fadedPoints);
                break;

            case lumi::modules::SuperscopeRenderMode::Lines:
                renderLines(fadedPoints);
                break;

            case lumi::modules::SuperscopeRenderMode::ThickLines:
                // Render lines as triangle strip
                renderLinesAsTriangleStrip(fadedPoints, m_superscope.lineWidth());
                // Also render glow dots if enabled
                if (m_superscope.glowEnabled())
                {
                    renderPoints(fadedPoints);
                }
                break;
        }
    }
}

void SuperscopeVisualizer::uploadVertexData(const std::vector<float>& vertices)
{
    auto* f = QOpenGLContext::currentContext()->functions();

    m_vao->bind();
    m_vertexBuffer->bind();

    m_vertexBuffer->allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    // Position attribute (location 0)
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);

    // Color attribute (location 1)
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                             reinterpret_cast<void*>(2 * sizeof(float)));

    m_vertexBuffer->release();
    m_vao->release();
}

// =============================================================================
// Resize
// =============================================================================

void SuperscopeVisualizer::onResize(const QSize& size)
{
    Q_UNUSED(size);
    // Nothing special needed
}

// =============================================================================
// Cleanup
// =============================================================================

void SuperscopeVisualizer::onCleanup()
{
    m_pointShader.reset();
    m_lineShader.reset();
    m_vertexBuffer.reset();
    m_vao.reset();
    m_lastContext = nullptr;
    m_heldFrames.clear();
}

// =============================================================================
// Stereo Split Helper
// =============================================================================

void SuperscopeVisualizer::splitStereoData(const std::vector<float>& interleaved,
                                            std::vector<float>& left,
                                            std::vector<float>& right)
{
    int samplesPerChannel = static_cast<int>(interleaved.size()) / 2;
    left.resize(samplesPerChannel);
    right.resize(samplesPerChannel);

    for (int i = 0; i < samplesPerChannel; ++i)
    {
        left[i] = interleaved[i * 2];
        right[i] = interleaved[i * 2 + 1];
    }
}
