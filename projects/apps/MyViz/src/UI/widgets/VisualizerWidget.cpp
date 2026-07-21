/**
 ****************************************************************************************
 * @file   VisualizerWidget.cpp
 * @brief  Facade widget for visualizer rendering on a dedicated render thread
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 3.0.0 - Render thread decoupling (Render_Thread_Entwurf.md)
 ****************************************************************************************
 */

// =============================================================================
// Includes
// =============================================================================

#include "pch.h"
#include "UI/widgets/VisualizerWidget.hpp"
#include "visualizers/IVisualizer.hpp"
#include "services/VisualizerRegistry.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

// Qt
#include <QOpenGLContext>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QString>
#include <QKeyEvent>

// BasicLogger
#include <BasicLogger.h>

// =============================================================================
// Construction / Destruction
// =============================================================================

VisualizerWidget::VisualizerWidget(ServiceContainer& services, QWidget* parent)
    : StandardWidgetBase(services,
                         QStringLiteral("visualizer"),
                         tr("Visualizer"),
                         parent)
{
    BasicLogger::logDebug("VisualizerWidget constructor (render thread facade)");

    // -------------------------------------------------------------------------
    // GL window + context + render thread (Entwurf §1)
    // -------------------------------------------------------------------------

    m_glWindow = new VisualizerGLWindow();

    m_context = std::make_unique<QOpenGLContext>();
    m_context->setFormat(m_glWindow->requestedFormat());
    if (!m_context->create())
    {
        BasicLogger::logError("VisualizerWidget: QOpenGLContext creation FAILED");
    }

    m_thread = std::make_unique<VisualizerRenderThread>(
        *m_glWindow, *m_context, m_renderMutex);
    m_glWindow->attachThread(m_thread.get());

    // The context belongs to the render thread from now on (makeCurrent there)
    m_context->moveToThread(m_thread.get());

    // createWindowContainer takes ownership of the window
    auto* container = QWidget::createWindowContainer(m_glWindow, this);
    container->setMinimumSize(120, 90);

    // Key events (Esc in fullscreen) must reach the embedded GL window:
    // route focus through the container, which forwards it to the window
    container->setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(container);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container);

    // -------------------------------------------------------------------------
    // Signal wiring (all on the GUI thread; fpsMeasured arrives queued)
    // -------------------------------------------------------------------------

    connect(m_thread.get(), &VisualizerRenderThread::fpsMeasured,
            this, &VisualizerWidget::fpsMeasured);

    // Double-click inside the GL area toggles fullscreen for THIS widget
    connect(m_glWindow, &VisualizerGLWindow::doubleClicked, this, [this]() {
        if (auto* bus = eventBus())
        {
            bus->publish(ToggleFullscreenEvent{static_cast<void*>(this)});
            BasicLogger::logDebug("VisualizerWidget: Double-click -> Toggle Fullscreen");
        }
    });

    // Esc leaves fullscreen — only toggle when we actually ARE fullscreen
    // (otherwise Esc in normal mode would ENTER fullscreen)
    connect(m_glWindow, &VisualizerGLWindow::escapePressed, this, [this]() {
        if (window()->isFullScreen())
        {
            if (auto* bus = eventBus())
            {
                bus->publish(ToggleFullscreenEvent{static_cast<void*>(this)});
                BasicLogger::logDebug("VisualizerWidget: Esc -> exit fullscreen");
            }
        }
    });

    // On application quit no event loop is left for deleteLater() — stop the
    // render thread deterministically BEFORE the QApplication teardown.
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        if (m_thread != nullptr)
        {
            m_thread->stopAndWait();
        }
    });

    m_thread->start();

    // Default visualizer (previously loaded lazily in initializeGL): an
    // explicit setVisualizer() afterwards is a cheap no-op if it is the same
    loadDefaultVisualizer();
}

VisualizerWidget::~VisualizerWidget()
{
    BasicLogger::logDebug("VisualizerWidget destructor");

    if (m_thread != nullptr)
    {
        // Joins the thread; the thread GL-cleans the active visualizer and
        // moves the context back to the GUI thread.
        m_thread->stopAndWait();
    }
    if (m_glWindow != nullptr)
    {
        // The window dies with the container in ~QWidget — ignore its events
        m_glWindow->attachThread(nullptr);
    }

    // m_visualizer (GL already cleaned) and m_context die via unique_ptr.
}

// =============================================================================
// WidgetBase Overrides
// =============================================================================

void VisualizerWidget::onStartUpdates()
{
    // Render thread resumes via expose events — nothing to do.
    BasicLogger::logDebug("VisualizerWidget::onStartUpdates()");
}

void VisualizerWidget::onStopUpdates()
{
    // Render thread pauses via expose events — nothing to do.
    BasicLogger::logDebug("VisualizerWidget::onStopUpdates()");
}

// =============================================================================
// Visualizer Management
// =============================================================================

bool VisualizerWidget::setVisualizer(const QString& id)
{
    BasicLogger::logInfo("VisualizerWidget::setVisualizer(\"" + id.toStdString() + "\")");

    // Check if already active
    if (id == m_currentVisualizerId && m_visualizer != nullptr)
    {
        BasicLogger::logDebug("  Visualizer already active");
        return true;
    }

    // Check if registered
    auto& registry = VisualizerRegistry::instance();
    if (!registry.has(id.toStdString()))
    {
        QString error = QStringLiteral("Visualizer not found: ") + id;
        BasicLogger::logWarning("  " + error.toStdString());
        Q_EMIT visualizerError(id, error);
        return false;
    }

    // Create the new instance on the GUI thread (no GL involved — the render
    // thread initializes it before its first frame)
    std::unique_ptr<IVisualizer> next = registry.create(id.toStdString());
    if (next == nullptr)
    {
        QString error = QStringLiteral("Failed to create visualizer: ") + id;
        BasicLogger::logError("  " + error.toStdString());
        Q_EMIT visualizerError(id, error);
        return false;
    }

    // Hand the swap to the render thread: it GL-cleans and deletes the old
    // instance and initializes the new one. The facade keeps ownership of
    // the new instance (UI reads paramDescs etc. — under renderMutex()).
    std::unique_ptr<IVisualizer> retire = std::move(m_visualizer);
    m_visualizer = std::move(next);
    m_currentVisualizerId = id;
    m_thread->setVisualizer(m_visualizer.get(), std::move(retire));

    BasicLogger::logInfo("  Visualizer created: " +
                         m_visualizer->visualizerName().toStdString() +
                         " (GL init on render thread)");

    // Publish event via EventBus — carries the render mutex so subscribers
    // (ConfigPanel, commands) can guard their accesses
    auto* bus = eventBus();
    if (bus != nullptr)
    {
        bus->publish(VisualizerChangedEvent{
            id.toStdString(),
            m_visualizer->visualizerName().toStdString(),
            static_cast<void*>(m_visualizer.get()),
            &m_renderMutex
        });
    }

    Q_EMIT visualizerChanged(id);
    return true;
}

QString VisualizerWidget::currentVisualizerName() const
{
    if (m_visualizer != nullptr)
    {
        // Immutable identity data — safe without the render mutex
        return m_visualizer->visualizerName();
    }
    return QString();
}

void VisualizerWidget::loadDefaultVisualizer()
{
    // Try to load "pulsing" as default
    if (!setVisualizer(QStringLiteral("pulsing")))
    {
        // If pulsing not available, try first registered visualizer
        auto& registry = VisualizerRegistry::instance();
        auto descriptors = registry.descriptors();

        if (!descriptors.empty())
        {
            setVisualizer(QString::fromStdString(descriptors[0].id));
        }
        else
        {
            BasicLogger::logWarning("No visualizers registered!");
        }
    }
}

// =============================================================================
// Frame Pacing
// =============================================================================

void VisualizerWidget::setFrameMode(RenderPacing pacing, int targetFps)
{
    if (m_thread != nullptr)
    {
        m_thread->setPacing(pacing, targetFps);
    }
}

void VisualizerWidget::activateGLWindow()
{
    if (m_glWindow != nullptr)
    {
        m_glWindow->requestActivate();
    }
}

void VisualizerWidget::recreateNativeWindow()
{
    hide();
    destroy();  // drops stale native handles (children included)
    show();     // recreates them along the CURRENT parent chain
}

void VisualizerWidget::keyPressEvent(QKeyEvent* event)
{
    // Fallback for fullscreen exit: if the focus sits on the container
    // widget (not the embedded GL window), the Esc arrives here
    if (event->key() == Qt::Key_Escape && window()->isFullScreen())
    {
        if (auto* bus = eventBus())
        {
            bus->publish(ToggleFullscreenEvent{static_cast<void*>(this)});
            BasicLogger::logDebug("VisualizerWidget: Esc (widget) -> exit fullscreen");
        }
        event->accept();
        return;
    }
    StandardWidgetBase::keyPressEvent(event);
}

// =============================================================================
// Audio Data (snapshot hand-over)
// =============================================================================

void VisualizerWidget::updateSpectrum(const float* spectrum, int count)
{
    if (m_thread != nullptr)
    {
        m_thread->updateAudio(spectrum, count, nullptr, 0);
    }
}

void VisualizerWidget::updateWaveform(const float* waveform, int count)
{
    if (m_thread != nullptr)
    {
        m_thread->updateAudio(nullptr, 0, waveform, count);
    }
}

void VisualizerWidget::updateAudioStereo(const float* specInterleaved, int binsPerCh,
                                         const float* waveInterleaved, int frames,
                                         int channels)
{
    if (m_thread != nullptr)
    {
        m_thread->updateAudioStereo(specInterleaved, binsPerCh, waveInterleaved,
                                    frames, channels);
    }
}
