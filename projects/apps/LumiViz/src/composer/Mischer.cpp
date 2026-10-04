/**
 * @file Mischer.cpp
 * @brief Composer: Leinwand und Mischer (Stufe C1a)
 */

#include "composer/Mischer.hpp"

#include "visualizers/MultiEffectVisualizer.hpp"

#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <algorithm>

namespace lumi::composer
{

namespace
{

const char* kVertex = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
out vec2 vTex;
void main()
{
    vTex = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Mischart-Nummern == enum Mischart. Die Formeln entsprechen denen des
// Listen-Mischpasses der Kette (Additive, Subtractive, Maximum, Minimum,
// Multiply, XOR) — hier aber nimmt JEDE Art den Pegel.
const char* kFragment = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uUnten;
uniform sampler2D uOben;
uniform int uArt;
uniform float uPegel;
out vec4 fragColor;
void main()
{
    vec3 d = texture(uUnten, vTex).rgb;
    vec3 s = texture(uOben, vTex).rgb;
    vec3 r;
    if (uArt == 1)      r = min(d + s, vec3(1.0));
    else if (uArt == 2) r = max(d - s, vec3(0.0));
    else if (uArt == 3) r = max(d, s);
    else if (uArt == 4) r = min(d, s);
    else if (uArt == 5) r = d * s;
    else if (uArt == 6)
    {
        ivec3 di = ivec3(d * 255.0 + 0.5);
        ivec3 si = ivec3(s * 255.0 + 0.5);
        r = vec3(di ^ si) / 255.0;
    }
    else                r = s;
    fragColor = vec4(mix(d, r, clamp(uPegel, 0.0, 1.0)), 1.0);
}
)";

QOpenGLFunctions* gl()
{
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    return ctx != nullptr ? ctx->functions() : nullptr;
}

std::unique_ptr<QOpenGLFramebufferObject> neuesBild(QSize groesse)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    format.setInternalTextureFormat(GL_RGBA8);
    auto fbo = std::make_unique<QOpenGLFramebufferObject>(groesse, format);
    if (!fbo->isValid()) return nullptr;
    if (QOpenGLFunctions* f = gl())
    {
        // 1:1 gelesen — kein Filtern, kein Wiederholen am Rand
        f->glBindTexture(GL_TEXTURE_2D, fbo->texture());
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        f->glBindTexture(GL_TEXTURE_2D, 0);
    }
    return fbo;
}

}  // namespace

const char* mischartName(Mischart art)
{
    switch (art)
    {
        case Mischart::Normal: return "Normal";
        case Mischart::Additive: return "Additive";
        case Mischart::Subtract: return "Subtract";
        case Mischart::Maximum: return "Maximum";
        case Mischart::Minimum: return "Minimum";
        case Mischart::Multiply: return "Multiply";
        case Mischart::Xor: return "XOR";
    }
    return "Normal";
}

// =============================================================================
// Leinwand
// =============================================================================

Leinwand::Leinwand() = default;
Leinwand::~Leinwand() = default;

void Leinwand::rendere(MultiEffectVisualizer& renderer, float deltaSekunden, QSize groesse)
{
    QOpenGLFunctions* f = gl();
    if (f == nullptr || groesse.isEmpty()) return;
    if (m_fbo == nullptr || m_fbo->size() != groesse) m_fbo = neuesBild(groesse);
    if (m_fbo == nullptr) return;

    // Der Renderer liest die Fenstergroesse aus dem Viewport und legt sein
    // fertiges Bild ins Present-Ziel (Composer-Naht R1).
    f->glViewport(0, 0, groesse.width(), groesse.height());
    renderer.setPresentZiel(m_fbo->handle());
    renderer.render(deltaSekunden);
    renderer.setPresentZiel(0);
}

unsigned int Leinwand::textur() const
{
    return m_fbo != nullptr ? m_fbo->texture() : 0;
}

unsigned int Leinwand::framebuffer() const
{
    return m_fbo != nullptr ? m_fbo->handle() : 0;
}

QSize Leinwand::groesse() const
{
    return m_fbo != nullptr ? m_fbo->size() : QSize();
}

QImage Leinwand::bild() const
{
    return m_fbo != nullptr ? m_fbo->toImage() : QImage();
}

// =============================================================================
// Mischer
// =============================================================================

Mischer::Mischer() = default;
Mischer::~Mischer() = default;

bool Mischer::stelleSicher(QSize groesse)
{
    if (gl() == nullptr || groesse.isEmpty()) return false;
    if (m_shader == nullptr)
    {
        auto shader = std::make_unique<QOpenGLShaderProgram>();
        if (!shader->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertex) ||
            !shader->addShaderFromSourceCode(QOpenGLShader::Fragment, kFragment) || !shader->link())
            return false;
        m_shader = std::move(shader);

        static const float kEcken[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
        m_vao = std::make_unique<QOpenGLVertexArrayObject>();
        m_vao->create();
        m_vao->bind();
        m_vbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
        m_vbo->create();
        m_vbo->bind();
        m_vbo->allocate(kEcken, static_cast<int>(sizeof(kEcken)));
        QOpenGLFunctions* f = gl();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        m_vao->release();
        m_vbo->release();
    }
    for (auto& bild : m_bild)
        if (bild == nullptr || bild->size() != groesse)
        {
            bild = neuesBild(groesse);
            if (bild == nullptr) return false;
        }
    return true;
}

bool Mischer::beginne(QSize groesse)
{
    if (!stelleSicher(groesse)) return false;
    QOpenGLFunctions* f = gl();
    m_aktuell = 0;
    m_bild[0]->bind();
    f->glViewport(0, 0, groesse.width(), groesse.height());
    f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);
    m_bild[0]->release();
    return true;
}

void Mischer::durchgang(unsigned int unten, unsigned int oben, double pegel, int art)
{
    QOpenGLFunctions* f = gl();
    const GLboolean blendWar = f->glIsEnabled(GL_BLEND);
    f->glDisable(GL_BLEND);
    m_shader->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, unten);
    m_shader->setUniformValue("uUnten", 0);
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, oben);
    m_shader->setUniformValue("uOben", 1);
    f->glActiveTexture(GL_TEXTURE0);
    m_shader->setUniformValue("uArt", art);
    m_shader->setUniformValue("uPegel", static_cast<float>(std::clamp(pegel, 0.0, 1.0)));
    m_vao->bind();
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_vao->release();
    m_shader->release();
    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, 0);
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, 0);
    if (blendWar) f->glEnable(GL_BLEND);
}

void Mischer::lege(unsigned int textur, double pegel, Mischart art)
{
    if (m_shader == nullptr || m_bild[0] == nullptr || textur == 0 || pegel <= 0.0) return;
    QOpenGLFunctions* f = gl();
    QOpenGLFramebufferObject& ziel = *m_bild[1 - m_aktuell];
    ziel.bind();
    f->glViewport(0, 0, ziel.width(), ziel.height());
    durchgang(m_bild[m_aktuell]->texture(), textur, pegel, static_cast<int>(art));
    ziel.release();
    m_aktuell = 1 - m_aktuell;
}

unsigned int Mischer::framebuffer() const
{
    return m_bild[m_aktuell] != nullptr ? m_bild[m_aktuell]->handle() : 0;
}

void Mischer::binde()
{
    if (m_bild[m_aktuell] == nullptr) return;
    m_bild[m_aktuell]->bind();
    gl()->glViewport(0, 0, m_bild[m_aktuell]->width(), m_bild[m_aktuell]->height());
}

unsigned int Mischer::textur() const
{
    return m_bild[m_aktuell] != nullptr ? m_bild[m_aktuell]->texture() : 0;
}

QImage Mischer::bild() const
{
    return m_bild[m_aktuell] != nullptr ? m_bild[m_aktuell]->toImage() : QImage();
}

void Mischer::zeichneIn(unsigned int framebuffer, QSize groesse)
{
    if (m_shader == nullptr || m_bild[m_aktuell] == nullptr) return;
    QOpenGLFunctions* f = gl();
    f->glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    f->glViewport(0, 0, groesse.width(), groesse.height());
    // Normal mit Pegel 1 = das Obere allein
    durchgang(m_bild[m_aktuell]->texture(), m_bild[m_aktuell]->texture(), 1.0,
              static_cast<int>(Mischart::Normal));
}

}  // namespace lumi::composer
