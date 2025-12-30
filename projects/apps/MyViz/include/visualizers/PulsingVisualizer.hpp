/**
 ****************************************************************************************
 * @file   PulsingVisualizer.hpp
 * @brief  Simple pulsing color visualizer
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Erster Visualizer
 *
 * Der PulsingVisualizer zeigt einen animierten Regenbogen-Puls-Effekt.
 * Dies ist der bestehende Code aus VisualizerWidget, extrahiert in
 * die neue Visualizer-Architektur.
 *
 * ### Features
 *
 * - Zeit-basierte Animation (nicht frame-basiert)
 * - Regenbogen-Farbzyklus
 * - Reagiert (noch) nicht auf Audio
 *
 * ### Zukünftige Erweiterung
 *
 * Audio-Reaktivität kann später hinzugefügt werden:
 * ```cpp
 * void onRender(float deltaTime) override {
 *     auto spectrum = getSpectrum();
 *     if (!spectrum.empty()) {
 *         m_pulseIntensity = calculateBassIntensity(spectrum);
 *     }
 *     // Use m_pulseIntensity in color calculation
 * }
 * ```
 ****************************************************************************************
 */

#pragma once

#include "VisualizerBase.hpp"

#include <chrono>

/**
 * @class PulsingVisualizer
 * @brief Simple rainbow pulsing effect
 *
 * Time-based color cycling without audio dependency.
 */
class PulsingVisualizer : public VisualizerBase
{
public:
    PulsingVisualizer();
    ~PulsingVisualizer() override = default;

    // =========================================================================
    // Configuration
    // =========================================================================

    /**
     * @brief Set pulse speed
     * @param speed Cycles per second (default: 2.0)
     */
    void setPulseSpeed(float speed) { m_pulseSpeed = speed; }

    /**
     * @brief Get pulse speed
     */
    [[nodiscard]] float pulseSpeed() const { return m_pulseSpeed; }

protected:
    void onInitialize() override;
    void onRender(float deltaTime) override;
    void onResize(const QSize& size) override;
    void onCleanup() override;

private:
    // Animation state
    std::chrono::steady_clock::time_point m_startTime;
    float m_pulseSpeed = 2.0f;

    // For future audio reactivity
    [[maybe_unused]] float m_audioIntensity = 0.0f;
};
