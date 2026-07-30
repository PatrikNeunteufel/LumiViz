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

    if (params.avsPixelDots)
    {
        renderPixelDots(points);
        return;
    }

    std::vector<float> vertices;
    vertices.reserve(points.size() * 6);
    for (const auto& pt : points)
    {
        if (pt.skip) continue;
        // x/y sind DOUBLE (AVS-Genauigkeit, s. SuperscopePoint); der
        // Vertex-Puffer ist float — hier wird bewusst verengt.
        vertices.insert(vertices.end(), {static_cast<float>(pt.x),
                                        static_cast<float>(pt.y),
                                        pt.r, pt.g, pt.b, pt.a});
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

void ScopeRenderer::renderPixelDots(const std::vector<SuperscopePoint>& points)
{
    // r_sscope.cpp:294-305 — je Punkt GENAU EIN Pixel:
    //   x=(int)((var_x+1.0)*w*0.5); y=(int)((var_y+1.0)*h*0.5);
    //   if (in Bild) BLEND_LINE(framebuffer+x+y*w, farbe);
    // Also getrunkierte Ganzzahl, harte Kante, ausserhalb verworfen. Der
    // GL_POINTS-Pfad traf das nicht: Punktgroesse rastert nach eigener
    // Konvention und der Fragment-Shader legt einen weichen Rand darum
    // (Befund S50: Schwerpunkt 14 px daneben, bei wenigen Punkten gar nichts).
    // Deshalb hier ein deckungsgleiches Quad je Zielpixel.
    auto* f = QOpenGLContext::currentContext()->functions();
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    const int w = viewport[2];
    const int h = viewport[3];
    if (w <= 0 || h <= 0) return;

    std::vector<float> vertices;
    vertices.reserve(points.size() * 6 * 6);
    for (const auto& pt : points)
    {
        if (pt.skip) continue;
        // Punkte liegen im GL-Raum (y+ oben), AVS rechnet mit y+ unten.
        const int px = static_cast<int>((static_cast<double>(pt.x) + 1.0) * w * 0.5);
        const int pyTop =
            static_cast<int>((-static_cast<double>(pt.y) + 1.0) * h * 0.5);
        if (px < 0 || px >= w || pyTop < 0 || pyTop >= h) continue;  // AVS verwirft
        const int row = h - 1 - pyTop;  // GL zaehlt von unten
        const float x0 = static_cast<float>(px) * 2.0f / w - 1.0f;
        const float x1 = static_cast<float>(px + 1) * 2.0f / w - 1.0f;
        const float y0 = static_cast<float>(row) * 2.0f / h - 1.0f;
        const float y1 = static_cast<float>(row + 1) * 2.0f / h - 1.0f;
        const float c[4] = {pt.r, pt.g, pt.b, pt.a};
        const float quad[6][2] = {{x0, y0}, {x1, y0}, {x1, y1},
                                  {x0, y0}, {x1, y1}, {x0, y1}};
        for (const auto& v : quad)
        {
            vertices.insert(vertices.end(), {v[0], v[1], c[0], c[1], c[2], c[3]});
        }
    }
    if (vertices.empty()) return;

    uploadVertexData(vertices);
    // Linien-Shader: flache Farbe ohne Punkt-Falloff.
    m_lineShader->bind();
    m_vao->bind();
    m_lineShader->setUniformValue(m_lineUniAlpha, 1.0f);
    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size() / 6));
    m_vao->release();
    m_lineShader->release();
}

namespace
{
/// AVS zieht Linien zwischen GANZZAHLIGEN Pixeln: `x=(int)((var_x+1.0)*w*0.5)`
/// (r_sscope:292-293), dann Bresenham von Pixel zu Pixel. GL rastert dagegen
/// gegen Pixel-MITTEN — eine Stuetzstelle bei Fensterkoordinate 160,0 liegt
/// genau auf der Grenze und faellt eine Spalte nach links. Gemessen am
/// Referenzbild: unsere Diagonale lag durchgehend eine Spalte links von der
/// Referenz (Zeile 120: 159..161 statt 160..162). Wir setzen die Stuetzstelle
/// deshalb auf die Mitte des Pixels, den AVS berechnet (Befund S58).
float aufPixelmitte(double v, int groesse)
{
    if (groesse <= 0) return static_cast<float>(v);
    const double fenster = (v + 1.0) * static_cast<double>(groesse) * 0.5;
    const double pixel = std::floor(fenster);
    return static_cast<float>((pixel + 0.5) * 2.0 / static_cast<double>(groesse) - 1.0);
}
}  // namespace

void ScopeRenderer::renderThinLines(const std::vector<SuperscopePoint>& points)
{
    auto* f = QOpenGLContext::currentContext()->functions();
    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);

    std::vector<float> vertices;
    vertices.reserve(points.size() * 6);
    std::vector<std::pair<int, int>> segments;
    int currentStart = 0;
    int currentCount = 0;

    for (const auto& pt : points)
    {
        // `skip` unterdrueckt in AVS NUR das Zeichnen des Segments, das in
        // diesem Punkt endet — `lx/ly` werden trotzdem gesetzt (r_sscope:295-334:
        // die Zuweisung steht HINTER dem if-Block). Der Punkt bleibt also
        // Ankerpunkt fuer das naechste Segment. Wir haben ihn frueher ganz
        // verworfen; bei einem Skript, das `skip` je Punkt umschaltet (Bright
        // Light District: `ip=bnot(ip); skip=ip`), blieb damit zwischen zwei
        // gezeichneten Punkten immer ein Bruch — es wurde GAR NICHTS gezeichnet,
        // wo die Referenz jedes zweite Segment zieht (Befund S58).
        if (pt.skip)
        {
            if (currentCount > 0) segments.emplace_back(currentStart, currentCount);
            currentStart = static_cast<int>(vertices.size() / 6);
            currentCount = 0;
        }
        // x/y sind DOUBLE (AVS-Genauigkeit, s. SuperscopePoint); der
        // Vertex-Puffer ist float — hier wird bewusst verengt.
        vertices.insert(vertices.end(), {aufPixelmitte(pt.x, viewport[2]),
                                        aufPixelmitte(pt.y, viewport[3]),
                                        pt.r, pt.g, pt.b, pt.a});
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
    const float pixelWidth = viewport[2] > 0 ? 2.0f / static_cast<float>(viewport[2])
                                             : 0.002f;
    const float pixelHeight = viewport[3] > 0 ? 2.0f / static_cast<float>(viewport[3])
                                              : 0.002f;

    std::vector<float> vertices;
    vertices.reserve(points.size() * 2 * 6);
    std::vector<std::pair<int, int>> segments;
    int currentStart = 0;
    int currentCount = 0;

    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto& pt = points[i];
        // s. renderThinLines: `skip` bricht die Linie, verwirft den Punkt aber
        // NICHT — er bleibt Ankerpunkt des naechsten Segments (r_sscope:334).
        if (pt.skip)
        {
            if (currentCount >= 4) segments.push_back({currentStart, currentCount});
            currentStart = static_cast<int>(vertices.size() / 6);
            currentCount = 0;
        }

        // Richtung des Segments, das an diesem Punkt haengt. Gezeichnet wird ein
        // Segment genau dann, wenn sein ZIELpunkt nicht uebersprungen ist —
        // deshalb fragt der Ausgang nach `points[i+1].skip` und der Eingang nach
        // `pt.skip`, NICHT nach dem Vorgaenger. Mit der alten Abfrage bekam der
        // Endpunkt eines Strichs, dessen Vorgaenger uebersprungen war, die
        // Richtung (0,0) und damit die Verbreiterung der falschen Achse: das
        // Viereck war verdreht und deckte statt sechs nur drei Spalten ab
        // (Befund S58, direkt nach der skip-Korrektur).
        float dx = 0.0f;
        float dy = 0.0f;
        if (i + 1 < points.size() && !points[i + 1].skip)
        {
            dx = static_cast<float>(points[i + 1].x - pt.x);
            dy = static_cast<float>(points[i + 1].y - pt.y);
        }
        else if (i > 0 && !pt.skip)
        {
            dx = static_cast<float>(pt.x - points[i - 1].x);
            dy = static_cast<float>(pt.y - points[i - 1].y);
        }

        // AVS-Semantik (linedraw.cpp, Befund S46/Wormhole): dicke Linien werden
        // ACHSENPARALLEL verbreitert — x-major zeichnet je Spalte eine
        // vertikale Saeule von lw Pixeln, y-major je Zeile eine horizontale
        // Reihe (Diagonalen effektiv um cos(theta) schmaler; SRM width=255 =
        // "wall-thick bars"). KEIN senkrechtes Band.
        const float dxPix = dx / pixelWidth;
        const float dyPix = dy / pixelHeight;
        float ox = 0.0f;
        float oy = 0.0f;
        if (std::fabs(dxPix) >= std::fabs(dyPix))
            oy = lineWidth * pixelHeight * 0.5f;  // x-major: vertikale Saeule
        else
            ox = lineWidth * pixelWidth * 0.5f;   // y-major: horizontale Reihe

        const float px = aufPixelmitte(pt.x, viewport[2]);
        const float py = aufPixelmitte(pt.y, viewport[3]);
        vertices.insert(vertices.end(), {px + ox, py + oy,
                                         pt.r, pt.g, pt.b, pt.a});
        vertices.insert(vertices.end(), {px - ox, py - oy,
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
