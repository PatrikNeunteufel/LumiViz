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

#include <algorithm>
#include <cmath>
#include <cstdlib>
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

// AVS faerbt ein Liniensegment EINFARBIG: `line(framebuffer, lx,ly, x,y, w,h,
// thiscolor, linesize)` bekommt die Farbe des AKTUELLEN Punktes und malt damit
// das ganze Segment vom Vorgaenger her (r_sscope:297/325) — es gibt keine
// Interpolation entlang der Strecke. Wir haben die Vertexfarben interpolieren
// lassen; bei wenigen Stuetzstellen mit starkem Verlauf ist das ein sichtbarer
// Unterschied ("Lost Cause" zieht eine Linie ueber FUENF Punkte von Weiss nach
// Blau). `flat` mit der GL-Vorgabe LAST_VERTEX trifft genau die Referenz: das
// Segment bekommt die Farbe seines Endpunkts (Befund S58).
const char* kLineVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
flat out vec4 vColor;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)";

const char* kLineFragmentShader = R"(
#version 330 core
flat in vec4 vColor;
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
float aufPixelmitteX(double v, int breite)
{
    if (breite <= 0) return static_cast<float>(v);
    // `(int)` schneidet Richtung NULL ab, nicht nach unten — bei knapp
    // negativen Werten (getosc liefert sie) ist das eine ganze Zeile
    // Unterschied: die Referenz zeichnet noch, wir hatten weggeschnitten.
    const double pixel = std::trunc((v + 1.0) * static_cast<double>(breite) * 0.5);
    return static_cast<float>((pixel + 0.5) * 2.0 / static_cast<double>(breite) - 1.0);
}

/// y ebenso, aber ueber die AVS-ZEILE gerechnet. Der Punkt liegt im GL-Raum
/// (y+ oben), AVS zaehlt von oben: `r = (int)((var_y+1)*h*0.5)`. Direkt auf dem
/// gespiegelten Wert zu runden ist NICHT dasselbe — liegt der Wert exakt auf
/// einer Pixelgrenze (var_y = -1, 0, +1 …), landet `floor(h - R)` eine Zeile
/// daneben. Aufgefallen an einer Sonde mit `y = getspec(...)*2-1`, wo der Wert
/// null ist: die Referenz zeichnet in Zeile 0, wir in Zeile 239 (Befund S58).
float aufPixelmitteY(double v, int hoehe)
{
    if (hoehe <= 0) return static_cast<float>(v);
    const double zeile = std::trunc((-v + 1.0) * static_cast<double>(hoehe) * 0.5);
    const double glZeile = static_cast<double>(hoehe) - 1.0 - zeile;
    return static_cast<float>((glZeile + 0.5) * 2.0 / static_cast<double>(hoehe) - 1.0);
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
        // breakStrip: harter Trenner ohne Ankerfunktion — Punkt wird nicht
        // einmal in den Vertex-Puffer uebernommen (s. SuperscopePoint)
        if (pt.breakStrip)
        {
            if (currentCount > 0) segments.emplace_back(currentStart, currentCount);
            currentStart = static_cast<int>(vertices.size() / 6);
            currentCount = 0;
            continue;
        }
        if (pt.skip)
        {
            if (currentCount > 0) segments.emplace_back(currentStart, currentCount);
            currentStart = static_cast<int>(vertices.size() / 6);
            currentCount = 0;
        }
        // x/y sind DOUBLE (AVS-Genauigkeit, s. SuperscopePoint); der
        // Vertex-Puffer ist float — hier wird bewusst verengt.
        vertices.insert(vertices.end(), {aufPixelmitteX(pt.x, viewport[2]),
                                        aufPixelmitteY(pt.y, viewport[3]),
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

namespace
{
/// AVS-Pixel eines Punkts: `(int)((var+1)*dim*0.5)` — trunc Richtung Null,
/// auch ausserhalb des Bildes (linedraw clippt selbst). Grob begrenzt, damit
/// wilde Skriptwerte keinen int-Ueberlauf produzieren.
int avsSpalte(double v, int breite)
{
    const double p = std::trunc((v + 1.0) * static_cast<double>(breite) * 0.5);
    return static_cast<int>(std::clamp(p, -100000.0, 100000.0));
}
int avsZeile(double v, int hoehe)
{
    // Punkte liegen im GL-Raum (y+ oben), AVS zaehlt Zeilen von oben.
    const double p = std::trunc((-v + 1.0) * static_cast<double>(hoehe) * 0.5);
    return static_cast<int>(std::clamp(p, -100000.0, 100000.0));
}
}  // namespace

void ScopeRenderer::renderThickLines(const std::vector<SuperscopePoint>& points,
                                     float lineWidth)
{
    // AVS-Semantik (linedraw.cpp), je SEGMENT ein Quad statt eines geteilten
    // Triangle-Strips — nur so lassen sich zwei Eigenheiten der Referenz
    // treffen (Befund S59, splendora-Ringe):
    //  1. Auf der MAJOR-Achse ist das Ende mit der GROESSEREN Koordinate
    //     EXKLUSIV: die Vertikal-Schleife `while (d++ < ye)` malt die Endzeile
    //     nie (alle vier Zweige gleich). Innen-Stuetzstellen schliesst das
    //     Folgesegment (Start inklusiv) — sichtbar wird es am Kettenende, an
    //     skip-Bruechen und an der Bildkante.
    //  2. Die Fast-Paths klemmen das Ende VOR der Exklusivitaet auf die letzte
    //     Bildzeile/-spalte: `ye=min(max(y1,y2),h-1)` — ein Segment, das nach
    //     y=240 laeuft, malt bis Zeile 238, NICHT 239. Das Movement-Clamp
    //     eines Ring-Presets sampelt genau diese unterste Zeile.
    // Verbreiterung bleibt ACHSENPARALLEL (x-major: vertikale Saeule je
    // Spalte, y-major: horizontale Reihe je Zeile; S46/Wormhole), aber mit dem
    // asymmetrischen Anker der Referenz: Start bei `koord - lw/2`, lw Pixel.
    auto* f = QOpenGLContext::currentContext()->functions();

    GLint viewport[4];
    f->glGetIntegerv(GL_VIEWPORT, viewport);
    const int w = viewport[2];
    const int h = viewport[3];
    if (w <= 0 || h <= 0) return;

    // r_sscope ruft `line(..., (int)(*var_linesize+0.5))` — RUNDEN, nicht
    // abschneiden: linesize=1.51 zeichnet 2 Pixel breit (Keil-Sonde, S59).
    const int lw = std::clamp(static_cast<int>(lineWidth + 0.5f), 1, 255);
    const int lw2 = lw / 2;
    // Rand INNERHALB des Pixels (<0.5), damit die Rasterung gegen Pixelmitten
    // die gewollten Pixel sicher trifft und die Nachbarn sicher nicht.
    constexpr double kRand = 0.3;

    // Spalten-/Zeilenspannen [a..b] (inklusive, AVS-Pixel) -> GL-Clipraum.
    const auto glX = [&](double p) { return static_cast<float>(p * 2.0 / w - 1.0); };
    const auto glY = [&](double p) { return static_cast<float>(1.0 - p * 2.0 / h); };

    std::vector<float> vertices;
    vertices.reserve(points.size() * 6 * 6);
    const auto quad = [&](double c0, double c1, double rT0, double rB0,
                          double rT1, double rB1, const SuperscopePoint& farbe)
    {
        // c0/c1: linke/rechte Spaltengrenze (Pixelmitten +- Rand), rT/rB die
        // Zeilengrenzen an der linken (0) bzw. rechten (1) Kante.
        const float xL = glX(c0);
        const float xR = glX(c1);
        const float v[6][2] = {{xL, glY(rT0)}, {xR, glY(rT1)}, {xR, glY(rB1)},
                               {xL, glY(rT0)}, {xR, glY(rB1)}, {xL, glY(rB0)}};
        for (const auto& p : v)
        {
            vertices.insert(vertices.end(),
                            {p[0], p[1], farbe.r, farbe.g, farbe.b, farbe.a});
        }
    };

    for (size_t i = 0; i + 1 < points.size(); ++i)
    {
        const auto& p0 = points[i];
        const auto& p1 = points[i + 1];
        // `skip` unterdrueckt NUR das Segment, das im uebersprungenen Punkt
        // ENDET — der Punkt bleibt Ankerpunkt des naechsten (r_sscope:334).
        // breakStrip-Trenner ankern dagegen NICHT (beide Nachbarsegmente weg).
        if (p0.breakStrip || p1.breakStrip) continue;
        if (p1.skip) continue;

        const int ax0 = avsSpalte(p0.x, w);
        const int ay0 = avsZeile(p0.y, h);
        const int ax1 = avsSpalte(p1.x, w);
        const int ay1 = avsZeile(p1.y, h);
        const int dx = std::abs(ax1 - ax0);
        const int dy = std::abs(ay1 - ay0);

        if (dx == 0)
        {
            // Vertikaler Fast-Path: Spalten [ax-lw2 .. +lw-1], Zeilen
            // [min .. min(max,h-1)-1] — Endzeile exklusiv NACH dem Klemmen.
            const int d = std::max(std::min(ay0, ay1), 0);
            const int ye = std::min(std::max(ay0, ay1), h - 1);
            if (d >= ye) continue;
            const int xs = ax0 - lw2;
            quad(xs + 0.5 - kRand, xs + lw - 1 + 0.5 + kRand,
                 d + 0.5 - kRand, ye - 1 + 0.5 + kRand,
                 d + 0.5 - kRand, ye - 1 + 0.5 + kRand, p1);
        }
        else if (dy == 0)
        {
            // Horizontaler Fast-Path — spiegelbildlich.
            const int d = std::max(std::min(ax0, ax1), 0);
            const int xe = std::min(std::max(ax0, ax1), w - 1);
            if (d >= xe) continue;
            const int ys = ay0 - lw2;
            quad(d + 0.5 - kRand, xe - 1 + 0.5 + kRand,
                 ys + 0.5 - kRand, ys + lw - 1 + 0.5 + kRand,
                 ys + 0.5 - kRand, ys + lw - 1 + 0.5 + kRand, p1);
        }
        else if (dy <= dx)
        {
            // x-major: Bresenham malt je Spalte lw VOLLE Zeilen ab der
            // gesteppten Zeile minus lw2. Ein kontinuierliches Band trifft
            // dessen Tie-Spalten nie zuverlaessig (Befund S59: die Keil-Sonde
            // verlor an jeder Rundungsgrenze eine Zeile) — deshalb laeuft hier
            // der INTEGER-Bresenham der Referenz mit, und je y-Lauf entsteht
            // ein Rechteck. Ablauf und Klemm-Reihenfolge exakt wie
            // linedraw.cpp:145-199 (inkl. der Eigenheit, dass `d` beim
            // Links-Clip NICHT mitlaeuft); Spaltenende exklusiv, x2 nur auf w
            // geklemmt — die letzte Spalte w-1 bleibt erreichbar.
            int x1 = ax0;
            int y1 = ay0;
            int x2 = ax1;
            int y2 = ay1;
            if (x2 < x1)
            {
                std::swap(x1, x2);
                std::swap(y1, y2);
            }
            const int yincr = y2 > y1 ? 1 : -1;
            y1 -= lw2;
            int d = dy + dy - dx;
            const int eIncr = dy + dy;
            const int neIncr = d - dx;
            if (x2 < 0 || x1 >= w) continue;
            if (x1 < 0)
            {
                int v = yincr * -x1;
                if (dx != 0) v = (v * dy) / dx;
                y1 += v;
                x1 = 0;
            }
            if (x2 > w) x2 = w;
            int runStart = x1;
            while (x1 < x2)
            {
                const bool step = d >= 0;
                d += step ? neIncr : eIncr;
                ++x1;
                if (step || x1 >= x2)
                {
                    // Lauf [runStart..x1-1], Zeilen [y1, y1+lw) geklemmt
                    const int rT = std::max(y1, 0);
                    const int rB = std::min(y1 + lw, h) - 1;
                    if (rB >= rT)
                    {
                        quad(runStart + 0.5 - kRand, x1 - 1 + 0.5 + kRand,
                             rT + 0.5 - kRand, rB + 0.5 + kRand,
                             rT + 0.5 - kRand, rB + 0.5 + kRand, p1);
                    }
                    runStart = x1;
                    if (step) y1 += yincr;
                }
            }
        }
        else
        {
            // y-major — spiegelbildlich zu x-major (Zeilenende exklusiv,
            // y2 nur auf h geklemmt, je x-Lauf ein Rechteck).
            int x1 = ax0;
            int y1 = ay0;
            int x2 = ax1;
            int y2 = ay1;
            if (y2 < y1)
            {
                std::swap(x1, x2);
                std::swap(y1, y2);
            }
            const int xincr = x2 > x1 ? 1 : -1;
            x1 -= lw2;
            int d = dx + dx - dy;
            const int eIncr = dx + dx;
            const int neIncr = d - dy;
            if (y2 < 0 || y1 >= h) continue;
            if (y1 < 0)
            {
                int v = xincr * -y1;
                if (dy != 0) v = (v * dx) / dy;
                x1 += v;
                y1 = 0;
            }
            if (y2 > h) y2 = h;
            int runStart = y1;
            while (y1 < y2)
            {
                const bool step = d >= 0;
                d += step ? neIncr : eIncr;
                ++y1;
                if (step || y1 >= y2)
                {
                    const int cL = std::max(x1, 0);
                    const int cR = std::min(x1 + lw, w) - 1;
                    if (cR >= cL)
                    {
                        quad(cL + 0.5 - kRand, cR + 0.5 + kRand,
                             runStart + 0.5 - kRand, y1 - 1 + 0.5 + kRand,
                             runStart + 0.5 - kRand, y1 - 1 + 0.5 + kRand, p1);
                    }
                    runStart = y1;
                    if (step) x1 += xincr;
                }
            }
        }
    }
    if (vertices.empty()) return;

    uploadVertexData(vertices);

    m_lineShader->bind();
    m_vao->bind();
    m_lineShader->setUniformValue(m_lineUniAlpha, 1.0f);
    f->glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size() / 6));
    m_vao->release();
    m_lineShader->release();
}

} // namespace lumi::render
