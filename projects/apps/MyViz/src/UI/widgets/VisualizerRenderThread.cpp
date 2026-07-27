/**
 ****************************************************************************************
 * @file   VisualizerRenderThread.cpp
 * @brief  Dedicated OpenGL render thread + GL window for visualizer rendering
 *
 * @author Patrik Neunteufel
 * @date   July 2026
 * @version 1.0.0 - Render-Thread-Entkopplung (Render_Thread_Entwurf.md)
 ****************************************************************************************
 */

#include "pch.h"
#include "UI/widgets/VisualizerRenderThread.hpp"
#include "visualizers/IVisualizer.hpp"

// Qt
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QExposeEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPlatformSurfaceEvent>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QSurfaceFormat>

#include <chrono>

// BasicLogger — GUI-thread call sites only (logger is not thread-safe)!
#include <BasicLogger.h>

// =============================================================================
// Platform-specific runtime swap interval (Windows; other platforms keep the
// initial surface-format interval — this project targets Windows)
// =============================================================================

#if defined(_WIN32)
using PFNWGLSWAPINTERVALEXTPROC = int (*)(int interval);
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#endif

// =============================================================================
// VisualizerGLWindow
// =============================================================================

VisualizerGLWindow::VisualizerGLWindow()
{
    // Same GL profile as the previous QOpenGLWidget setup; swap interval is
    // controlled at runtime by the render thread (RenderPacing)
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0);
    format.setDepthBufferSize(24);
    format.setSamples(4);

    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(format);
}

void VisualizerGLWindow::exposeEvent(QExposeEvent* /*event*/)
{
    if (m_thread != nullptr)
    {
        // glViewport needs PHYSICAL pixels — QWindow::size() is logical
        // (device-independent); QOpenGLWidget used to convert internally
        m_thread->onExposeChanged(isExposed(), size() * devicePixelRatio());
    }
}

void VisualizerGLWindow::resizeEvent(QResizeEvent* event)
{
    if (m_thread != nullptr)
    {
        m_thread->onResize(event->size() * devicePixelRatio());
    }
}

bool VisualizerGLWindow::event(QEvent* event)
{
    if (event->type() == QEvent::PlatformSurface)
    {
        auto* surfaceEvent = static_cast<QPlatformSurfaceEvent*>(event);
        if (surfaceEvent->surfaceEventType() ==
                QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed &&
            m_thread != nullptr)
        {
            // Undock/reparent/destroy: the render thread MUST release the
            // surface before Qt destroys it (Entwurf §4). GL resources
            // survive — the context is ours and keeps living.
            BasicLogger::logDebug("VisualizerGLWindow: SurfaceAboutToBeDestroyed"
                                  " -> releasing surface");
            m_thread->releaseSurfaceBlocking();
        }
    }
    return QWindow::event(event);
}

void VisualizerGLWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT doubleClicked();
        event->accept();
        return;
    }
    QWindow::mouseDoubleClickEvent(event);
}

void VisualizerGLWindow::keyPressEvent(QKeyEvent* event)
{
    // In fullscreen the keyboard focus sits HERE (embedded window), not on
    // the MainWindow — Esc must therefore be caught in the GL window
    if (event->key() == Qt::Key_Escape)
    {
        Q_EMIT escapePressed();
        event->accept();
        return;
    }
    QWindow::keyPressEvent(event);
}

// =============================================================================
// VisualizerRenderThread
// =============================================================================

VisualizerRenderThread::VisualizerRenderThread(VisualizerGLWindow& window,
                                               QOpenGLContext& context,
                                               QMutex& renderMutex,
                                               QObject* parent)
    : QThread(parent)
    , m_window(window)
    , m_context(context)
    , m_renderMutex(renderMutex)
{
}

VisualizerRenderThread::~VisualizerRenderThread() = default;

// -----------------------------------------------------------------------------
// Commands (GUI thread)
// -----------------------------------------------------------------------------

void VisualizerRenderThread::updateAudio(const float* spectrum, int spectrumCount,
                                         const float* waveform, int waveformCount)
{
    QMutexLocker lock(&m_audioMutex);

    if (spectrum != nullptr && spectrumCount > 0)
    {
        m_spectrum.assign(spectrum, spectrum + spectrumCount);
        m_audioDirty = true;
    }
    if (waveform != nullptr && waveformCount > 0)
    {
        m_waveform.assign(waveform, waveform + waveformCount);
        m_audioDirty = true;
    }
}

void VisualizerRenderThread::updateAudioStereo(const float* specI, int binsPerCh,
                                               const float* waveI, int frames,
                                               int channels)
{
    QMutexLocker lock(&m_audioMutex);
    const int ch = channels < 1 ? 1 : channels;
    if (specI != nullptr && binsPerCh > 0)
        m_specI.assign(specI, specI + static_cast<size_t>(binsPerCh) * ch);
    if (waveI != nullptr && frames > 0)
        m_waveI.assign(waveI, waveI + static_cast<size_t>(frames) * ch);
    m_stereoBins = binsPerCh;
    m_stereoFrames = frames;
    m_stereoChannels = ch;
    m_stereoDirty = true;
}

void VisualizerRenderThread::setVisualizer(IVisualizer* next,
                                           std::unique_ptr<IVisualizer> retire)
{
    QMutexLocker lock(&m_mutex);

    // A still-unprocessed retiree must not be overwritten silently: clean it
    // up on the next frame together with the new one (chain the swap).
    if (m_pendingRetire != nullptr && retire != nullptr)
    {
        // Extremely rare (two swaps within one frame): the intermediate
        // visualizer was never GL-initialized, plain delete is safe.
        retire.reset();
    }
    else if (retire != nullptr)
    {
        m_pendingRetire = std::move(retire);
    }

    m_pendingAdopt = next;
    m_hasAdopt = true;
    m_cond.wakeAll();
}

void VisualizerRenderThread::setPacing(RenderPacing pacing, int targetFps)
{
    QMutexLocker lock(&m_mutex);
    m_pacing = pacing;
    m_targetFps = (targetFps > 0) ? targetFps : 60;
    m_pacingDirty = true;
    m_cond.wakeAll();
}

void VisualizerRenderThread::requestCapture()
{
    m_captureRequested.storeRelease(1);
    QMutexLocker lock(&m_mutex);
    m_cond.wakeAll();  // im VSync-Leerlauf sonst erst beim naechsten Frame
}

// -----------------------------------------------------------------------------
// Surface synchronization (GUI thread)
// -----------------------------------------------------------------------------

void VisualizerRenderThread::onExposeChanged(bool exposed, const QSize& size)
{
    QMutexLocker lock(&m_mutex);
    m_exposed = exposed;
    if (exposed)
    {
        // New/valid surface: leave the release state
        m_releaseSurface = false;
        m_surfaceReleased = false;
        m_size = size;
        m_hasResize = true;
    }
    m_cond.wakeAll();
}

void VisualizerRenderThread::onResize(const QSize& size)
{
    QMutexLocker lock(&m_mutex);
    m_size = size;
    m_hasResize = true;
    m_cond.wakeAll();
}

void VisualizerRenderThread::releaseSurfaceBlocking()
{
    QMutexLocker lock(&m_mutex);
    if (!isRunning() || m_exit)
    {
        return;
    }

    m_releaseSurface = true;
    m_cond.wakeAll();

    // Bounded wait (deadlock guard in case the thread is just finishing)
    while (!m_surfaceReleased && isRunning() && !m_exit)
    {
        if (!m_surfaceReleasedCond.wait(&m_mutex, 2000))
        {
            BasicLogger::logWarning(
                "VisualizerRenderThread: releaseSurfaceBlocking timeout!");
            break;
        }
    }
}

void VisualizerRenderThread::stopAndWait()
{
    {
        QMutexLocker lock(&m_mutex);
        m_exit = true;
        m_cond.wakeAll();
        m_surfaceReleasedCond.wakeAll();
    }
    wait();
}

// -----------------------------------------------------------------------------
// Render loop (render thread)
// -----------------------------------------------------------------------------

void VisualizerRenderThread::run()
{
    QElapsedTimer frameTimer;
    frameTimer.start();
    float lastFrameTime = 0.0f;

    QElapsedTimer fpsTimer;
    fpsTimer.start();
    int fpsFrames = 0;

    RenderPacing pacing = RenderPacing::Limited;
    int targetFps = 60;

    // Absolute frame schedule for Limited pacing: over-sleeping one frame
    // (Windows timer granularity) is compensated on the next — the average
    // locks onto the target instead of drifting below it.
    auto nextFrameDue = std::chrono::steady_clock::now();

    for (;;)
    {
        QSize pendingSize;
        bool doResize = false;
        bool doAdopt = false;
        IVisualizer* adopt = nullptr;
        std::unique_ptr<IVisualizer> retire;
        bool pacingDirty = false;

        // ---------------------------------------------------------------------
        // Sync point: wait until exposed, serve surface release, pop commands
        // ---------------------------------------------------------------------
        {
            QMutexLocker lock(&m_mutex);
            for (;;)
            {
                if (m_exit)
                {
                    break;
                }
                if (m_releaseSurface && !m_surfaceReleased)
                {
                    m_context.doneCurrent();
                    m_surfaceReleased = true;
                    m_surfaceReleasedCond.wakeAll();
                }
                if (!m_releaseSurface && m_exposed)
                {
                    break;  // rendering possible
                }
                m_cond.wait(&m_mutex);
            }
            if (m_exit)
            {
                break;
            }

            doResize = m_hasResize;
            pendingSize = m_size;
            m_hasResize = false;

            doAdopt = m_hasAdopt;
            adopt = m_pendingAdopt;
            m_hasAdopt = false;
            retire = std::move(m_pendingRetire);

            pacingDirty = m_pacingDirty;
            pacing = m_pacing;
            targetFps = m_targetFps;
            m_pacingDirty = false;
        }

        // ---------------------------------------------------------------------
        // Frame
        // ---------------------------------------------------------------------
        if (!m_context.makeCurrent(&m_window))
        {
            // Surface not ready (yet) — retreat briefly. Re-queue popped
            // commands so they are not lost.
            QMutexLocker lock(&m_mutex);
            if (doAdopt && !m_hasAdopt)
            {
                m_pendingAdopt = adopt;
                m_hasAdopt = true;
            }
            if (retire != nullptr && m_pendingRetire == nullptr)
            {
                m_pendingRetire = std::move(retire);
            }
            if (pacingDirty)
            {
                m_pacingDirty = true;
            }
            lock.unlock();
            QThread::msleep(20);
            continue;
        }

        if (pacingDirty)
        {
            applySwapInterval(pacing == RenderPacing::VSync ? 1 : 0);
        }

        // Retire the previous visualizer (GL cleanup + delete on THIS thread)
        if (retire != nullptr)
        {
            QMutexLocker render(&m_renderMutex);
            if (retire->isInitialized())
            {
                retire->cleanup();
            }
            retire.reset();
        }

        if (doAdopt)
        {
            QMutexLocker render(&m_renderMutex);
            m_current = adopt;
            if (m_current != nullptr && !m_current->isInitialized())
            {
                m_current->initialize();
                if (pendingSize.isValid())
                {
                    // m_size persists after the first expose (physical px);
                    // before that the expose resize-flag sizes the visualizer
                    m_current->resize(pendingSize);
                }
            }
        }

        if (m_current != nullptr && doResize && pendingSize.isValid())
        {
            QMutexLocker render(&m_renderMutex);
            if (auto* gl = m_context.functions())
            {
                gl->glViewport(0, 0, pendingSize.width(), pendingSize.height());
            }
            m_current->resize(pendingSize);
        }

        const float currentTime = frameTimer.elapsed() / 1000.0f;
        const float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        if (m_current != nullptr && m_current->isInitialized())
        {
            // Audio snapshot + render under the render mutex (UI accessors
            // hold the same mutex for setParam/getParam/gradients/taps)
            QMutexLocker render(&m_renderMutex);
            {
                QMutexLocker audioLock(&m_audioMutex);
                if (m_audioDirty)
                {
                    if (!m_spectrum.empty())
                    {
                        m_current->updateSpectrum(
                            m_spectrum.data(),
                            static_cast<int>(m_spectrum.size()));
                    }
                    if (!m_waveform.empty())
                    {
                        m_current->updateWaveform(
                            m_waveform.data(),
                            static_cast<int>(m_waveform.size()));
                    }
                    m_audioDirty = false;
                }
                if (m_stereoDirty)
                {
                    m_current->updateAudioStereo(
                        m_specI.empty() ? nullptr : m_specI.data(), m_stereoBins,
                        m_waveI.empty() ? nullptr : m_waveI.data(), m_stereoFrames,
                        m_stereoChannels);
                    m_stereoDirty = false;
                }
            }
            m_current->render(deltaTime);
        }
        else
        {
            // No visualizer: neutral dark clear (previous fallback was red)
            if (auto* gl = m_context.functions())
            {
                gl->glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
                gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
        }

        // Screenshot VOR dem Swap: der Standard-Framebuffer traegt hier genau
        // das eben gerenderte Bild. Rezeptur wie im AvsStandalone (S44/S45):
        // PHYSISCHE Pixel (sonst liest man bei DPI-Skalierung nur den linken
        // unteren Ausschnitt), Zeile 0 kommt von UNTEN, und Alpha ist kein
        // Bildinhalt — Alpha-0-Pixel erscheinen im Betrachter sonst weiss.
        if (m_captureRequested.fetchAndStoreOrdered(0) != 0)
        {
            const qreal dpr = m_window.devicePixelRatio();
            const int w = std::max(1, static_cast<int>(m_window.width() * dpr));
            const int h = std::max(1, static_cast<int>(m_window.height() * dpr));
            std::vector<unsigned char> rgba(static_cast<std::size_t>(w) * h * 4);
            if (auto* gl = m_context.functions())
            {
                gl->glBindFramebuffer(GL_FRAMEBUFFER,
                                      m_context.defaultFramebufferObject());
                gl->glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
                const QImage shot =
                    QImage(rgba.data(), w, h, w * 4, QImage::Format_RGBA8888)
                        .flipped(Qt::Vertical)
                        .convertToFormat(QImage::Format_RGB888);
                // copy() loest das Bild vom lokalen Puffer — es reist per
                // queued signal in den GUI-Thread.
                Q_EMIT frameCaptured(shot.copy());
            }
        }

        // swapBuffers OUTSIDE the render mutex: the VSync wait must never
        // block UI accessors (Entwurf §2.1)
        m_context.swapBuffers(&m_window);

        // Pacing (Limited): sleep until the next slot of the absolute schedule
        if (pacing == RenderPacing::Limited)
        {
            const auto frameDuration =
                std::chrono::microseconds(1000000 / targetFps);
            nextFrameDue += frameDuration;

            const auto now = std::chrono::steady_clock::now();
            if (nextFrameDue > now)
            {
                const auto remaining =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        nextFrameDue - now);
                QThread::usleep(static_cast<unsigned long>(remaining.count()));
            }
            else if (now - nextFrameDue > frameDuration * 4)
            {
                // Far behind (pause/stall): re-anchor instead of racing ahead
                nextFrameDue = now;
            }
        }
        else
        {
            nextFrameDue = std::chrono::steady_clock::now();
        }

        ++fpsFrames;
        const qint64 fpsElapsed = fpsTimer.elapsed();
        if (fpsElapsed >= 1000)
        {
            Q_EMIT fpsMeasured(fpsFrames * 1000.0 /
                               static_cast<double>(fpsElapsed));
            fpsFrames = 0;
            fpsTimer.restart();
        }
    }

    // -------------------------------------------------------------------------
    // Shutdown ON the render thread (context lives here)
    // -------------------------------------------------------------------------
    const bool haveSurface = m_context.makeCurrent(&m_window);

    if (m_pendingRetire != nullptr)
    {
        if (haveSurface && m_pendingRetire->isInitialized())
        {
            m_pendingRetire->cleanup();
        }
        m_pendingRetire.reset();
    }

    if (m_current != nullptr)
    {
        if (haveSurface && m_current->isInitialized())
        {
            m_current->cleanup();  // GL only — the facade owns and deletes it
        }
        m_current = nullptr;
    }

    if (haveSurface)
    {
        m_context.doneCurrent();
    }

    // Hand the context back to the GUI thread so the facade may destroy it
    m_context.moveToThread(QGuiApplication::instance()->thread());
}

void VisualizerRenderThread::applySwapInterval(int interval)
{
    // Render thread, context is current.
#if defined(_WIN32)
    auto wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
        m_context.getProcAddress("wglSwapIntervalEXT"));
    if (wglSwapIntervalEXT != nullptr)
    {
        wglSwapIntervalEXT(interval);
    }
#elif defined(__APPLE__)
    CGLContextObj cglContext = CGLGetCurrentContext();
    if (cglContext != nullptr)
    {
        GLint swapInterval = interval;
        CGLSetParameter(cglContext, kCGLCPSwapInterval, &swapInterval);
    }
#else
    // Linux: runtime toggling needs platform plumbing (EGL/GLX display
    // handles); the initial surface-format interval stays in effect.
    Q_UNUSED(interval)
#endif
}
