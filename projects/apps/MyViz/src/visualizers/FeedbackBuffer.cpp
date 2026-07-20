/**
 ****************************************************************************************
 * @file   FeedbackBuffer.cpp
 * @brief  Implementation of the double-buffered offscreen feedback
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/render/FeedbackBuffer.hpp"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>

namespace lumi::render {

namespace {

const char* kQuadVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform float uZoom;
out vec2 vTex;
void main()
{
    vTex = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos * uZoom, 0.0, 1.0);
}
)";

const char* kQuadFragmentShader = R"(
#version 330 core
in vec2 vTex;
uniform sampler2D uTex;
uniform float uDecay;
out vec4 fragColor;
void main()
{
    fragColor = vec4(texture(uTex, vTex).rgb * uDecay, 1.0);
}
)";

} // namespace

bool FeedbackBuffer::ensure(int width, int height)
{
    if (width <= 0 || height <= 0) return false;
    if (ready() && width == m_width && height == m_height) return true;

    // Resize (decision E1): keep the latest image — it becomes the new
    // "previous" so trails survive the resize.
    std::unique_ptr<QOpenGLFramebufferObject> keep = std::move(m_current);
    m_previous.reset();

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    m_current = std::make_unique<QOpenGLFramebufferObject>(width, height, format);
    m_previous = std::make_unique<QOpenGLFramebufferObject>(width, height, format);
    if (!m_current->isValid() || !m_previous->isValid())
    {
        m_current.reset();
        m_previous.reset();
        return false;
    }

    if (keep != nullptr && QOpenGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        QOpenGLFramebufferObject::blitFramebuffer(
            m_previous.get(), QRect(0, 0, width, height), keep.get(),
            QRect(0, 0, m_width, m_height), GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
    keep.reset();

    m_width = width;
    m_height = height;
    return ensureQuadPipeline();
}

void FeedbackBuffer::destroy()
{
    m_current.reset();
    m_previous.reset();
    m_quadShader.reset();
    m_quadVao.reset();
    m_quadVbo.reset();
    m_width = 0;
    m_height = 0;
    m_uniDecay = -1;
    m_uniZoom = -1;
}

void FeedbackBuffer::beginFrame()
{
    if (ready()) m_current->bind();
}

void FeedbackBuffer::drawPrevious(float decay, float zoom)
{
    if (!ready() || m_quadShader == nullptr) return;

    auto* f = QOpenGLContext::currentContext()->functions();

    const GLboolean blendWasEnabled = f->glIsEnabled(GL_BLEND);
    f->glDisable(GL_BLEND);

    m_quadShader->bind();
    m_quadVao->bind();
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, m_previous->texture());
    m_quadShader->setUniformValue("uTex", 0);
    m_quadShader->setUniformValue(m_uniDecay, decay);
    m_quadShader->setUniformValue(m_uniZoom, zoom);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_quadVao->release();
    m_quadShader->release();

    if (blendWasEnabled == GL_TRUE) f->glEnable(GL_BLEND);
}

void FeedbackBuffer::endFrame(unsigned int targetFbo, int targetWidth, int targetHeight)
{
    if (!ready()) return;

    auto* f = QOpenGLContext::currentContext()->functions();
    m_current->release();   // back to the previously bound framebuffer

    if (QOpenGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        // Qt's static blit helper only targets FBO objects/default(0); bind
        // explicitly so custom targets (nested hosts later) work as well.
        auto* extra = QOpenGLContext::currentContext()->extraFunctions();
        extra->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_current->handle());
        extra->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFbo);
        extra->glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, targetWidth,
                                 targetHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        extra->glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
    }
    else
    {
        // No blit support: draw the current texture as a plain quad instead
        f->glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
        std::swap(m_current, m_previous);   // drawPrevious samples the new frame
        drawPrevious(1.0f, 1.0f);
        std::swap(m_current, m_previous);
    }

    std::swap(m_current, m_previous);
}

bool FeedbackBuffer::ensureQuadPipeline()
{
    if (m_quadShader != nullptr) return true;

    m_quadShader = std::make_unique<QOpenGLShaderProgram>();
    if (!m_quadShader->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                               kQuadVertexShader) ||
        !m_quadShader->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                               kQuadFragmentShader) ||
        !m_quadShader->link())
    {
        m_quadShader.reset();
        return false;
    }
    m_uniDecay = m_quadShader->uniformLocation("uDecay");
    m_uniZoom = m_quadShader->uniformLocation("uZoom");

    static constexpr float kQuad[] = {-1.0f, -1.0f, 1.0f, -1.0f,
                                      -1.0f, 1.0f,  1.0f, 1.0f};
    m_quadVao = std::make_unique<QOpenGLVertexArrayObject>();
    m_quadVao->create();
    m_quadVao->bind();
    m_quadVbo = std::make_unique<QOpenGLBuffer>(QOpenGLBuffer::VertexBuffer);
    m_quadVbo->create();
    m_quadVbo->bind();
    m_quadVbo->allocate(kQuad, sizeof(kQuad));
    auto* f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    m_quadVao->release();
    m_quadVbo->release();
    return true;
}

} // namespace lumi::render
