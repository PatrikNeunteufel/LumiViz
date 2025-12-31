/**
 ****************************************************************************************
 * @file   PulsingVisualizer.cpp
 * @brief  PulsingVisualizer implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/PulsingVisualizer.hpp"
#include "services/VisualizerRegistry.hpp"

#include <QOpenGLFunctions>
#include <cmath>

// =============================================================================
// Construction
// =============================================================================

PulsingVisualizer::PulsingVisualizer()
    : VisualizerBase(
          QStringLiteral("pulsing"),
          QObject::tr("Pulsing"),
          QObject::tr("Simple rainbow pulsing effect - time-based color cycling"))
    , m_startTime(std::chrono::steady_clock::now())
{
}

// =============================================================================
// VisualizerBase Implementation
// =============================================================================

void PulsingVisualizer::onInitialize()
{
    // Reset start time when (re)initialized
    m_startTime = std::chrono::steady_clock::now();

    // No additional OpenGL resources needed for this simple visualizer
    // Future: Create shaders, VBOs for more complex effects
}

void PulsingVisualizer::onRender(float /*deltaTime*/)
{
    // =========================================================================
    // Get OpenGL Functions
    // =========================================================================
    // We need to get the OpenGL functions from the current context
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (gl == nullptr)
    {
        return;
    }

    // =========================================================================
    // Time-Based Animation
    // =========================================================================
    // Use time (not frames) for consistent animation speed regardless of FPS

    auto now = std::chrono::steady_clock::now();
    float time = std::chrono::duration<float>(now - m_startTime).count();

    // =========================================================================
    // Rainbow Pulse Calculation
    // =========================================================================
    // Create smooth color cycling using sine waves with phase offsets
    //
    // Phase offsets for RGB:
    //   Red:   0°     (0.000 radians)
    //   Green: 120°   (2.094 radians)
    //   Blue:  240°   (4.189 radians)
    //
    // Formula: color = 0.5 + 0.5 * sin(time * speed + phase)
    // This maps sine output (-1 to 1) to color range (0 to 1)

    float phase = time * m_pulseSpeed;

    float r = 0.5f + 0.5f * std::sin(phase);             // Red
    float g = 0.5f + 0.5f * std::sin(phase + 2.094f);    // Green (120°)
    float b = 0.5f + 0.5f * std::sin(phase + 4.189f);    // Blue (240°)

    // =========================================================================
    // Future: Audio Reactivity
    // =========================================================================
    // When audio is integrated, we can modulate the effect:
    //
    // auto spectrum = getSpectrum();
    // if (!spectrum.empty()) {
    //     // Calculate bass intensity (first few bands)
    //     float bass = 0.0f;
    //     int bassBands = std::min(8, static_cast<int>(spectrum.size()));
    //     for (int i = 0; i < bassBands; ++i) {
    //         bass += spectrum[i];
    //     }
    //     bass /= bassBands;
    //
    //     // Smooth the intensity
    //     m_audioIntensity = m_audioIntensity * 0.8f + bass * 0.2f;
    //
    //     // Modulate pulse speed or brightness
    //     float brightness = 0.5f + m_audioIntensity * 0.5f;
    //     r *= brightness;
    //     g *= brightness;
    //     b *= brightness;
    // }

    // =========================================================================
    // Clear with Animated Color
    // =========================================================================
    gl->glClearColor(r, g, b, 1.0f);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PulsingVisualizer::onResize(const QSize& size)
{
    // Get OpenGL functions
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (gl == nullptr)
    {
        return;
    }

    // Update viewport
    gl->glViewport(0, 0, size.width(), size.height());

    // Future: Update projection matrix for more complex visualizations
}

void PulsingVisualizer::onCleanup()
{
    // No OpenGL resources to cleanup for this simple visualizer
    // Future: Delete shaders, VBOs, textures
}

// =============================================================================
// SELF-REGISTRATION
// =============================================================================
// NOTE: Registration is now handled centrally in VisualizerAutoReg.cpp
// to avoid linker issues with static libraries (dead code elimination).
// The REGISTER_VISUALIZER_CATEGORY macro is no longer used here.
