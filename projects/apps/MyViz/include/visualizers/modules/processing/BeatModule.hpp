/**
 ****************************************************************************************
 * @file   BeatModule.hpp
 * @brief  Shared beat detection module (edge-triggered threshold crossing)
 *
 * Consolidates the per-visualizer ad-hoc beat detectors (Phase 4 Schritt 5.6).
 * First user: PulsingVisualizer (rotation reversal on beat, 5.2); Superscope
 * and the Equalizer GradientDomain::Beat follow with their migrations.
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include <algorithm>

namespace lumi::modules {

/**
 * @class BeatModule
 * @brief Minimal beat detector: fires when the sensitivity-scaled level
 *        crosses the threshold upward (one trigger per crossing)
 */
class BeatModule
{
public:
    [[nodiscard]] static const char* moduleName() { return "Beat"; }
    [[nodiscard]] static const char* moduleDescription()
    {
        return "Edge-triggered beat detection on the processed audio level";
    }

    // =========================================================================
    // Configuration
    // =========================================================================

    /// @brief Trigger threshold on the scaled level [0..1]
    void setThreshold(float threshold) { m_threshold = std::clamp(threshold, 0.0f, 1.0f); }
    [[nodiscard]] float threshold() const { return m_threshold; }

    /// @brief Input scaling before the threshold comparison (>= 0)
    void setSensitivity(float sensitivity) { m_sensitivity = std::max(0.0f, sensitivity); }
    [[nodiscard]] float sensitivity() const { return m_sensitivity; }

    // =========================================================================
    // Detection
    // =========================================================================

    /**
     * @brief Feed the current audio level; returns true exactly on the frame
     *        the scaled level crosses the threshold upward
     * @param level Processed audio level [0..1]
     */
    [[nodiscard]] bool update(float level)
    {
        const float scaled = level * m_sensitivity;
        const bool beat = scaled > m_threshold && m_lastLevel <= m_threshold;
        m_lastLevel = scaled;
        return beat;
    }

    /**
     * @brief Adaptive-threshold detection on a signal-energy value
     *
     * The threshold follows the energy as a slow running average; a beat fires
     * while the energy clearly exceeds it (Superscope-style detector).
     * @param energy RMS energy of the current frame
     */
    [[nodiscard]] bool updateAdaptive(float energy)
    {
        m_adaptiveThreshold = m_adaptiveThreshold * kAdaptiveFollow
                              + energy * (1.0f - kAdaptiveFollow);
        return energy > m_adaptiveThreshold * kAdaptiveRatio && energy > kAdaptiveMinEnergy;
    }

    /**
     * @brief AVS-faithful onset detection on the mean |waveform| level
     *
     * Port of the vis_avs render() detector (ref main.cpp:290-329): a slow peak
     * tracker (peak1) mixed with a fast decaying runner-up (peak2); a beat fires
     * when the level exceeds peak1*34/32 and the noise floor, raises the bar to
     * (level+lastPeak)/2 and guards against a same-frame refire via the counter.
     * Feed once per frame; combine stereo by passing max(meanAbsL, meanAbsR).
     *
     * @param meanAbs Mean of |sample| over the frame's waveform, samples in -1..1
     * @return true exactly on onset frames (discrete events, no bursts)
     */
    [[nodiscard]] bool updateAvsOnset(float meanAbs)
    {
        // Original works on the SUM of 576 abs bytes (0..128 each); replicate
        // that scale so the constants (floor 576*16) keep their meaning.
        const float lt = meanAbs * 128.0f * 576.0f;

        m_avsPeak1 = (m_avsPeak1 * 125.0f + m_avsPeak2 * 3.0f) / 128.0f;
        ++m_avsCnt;

        bool beat = false;
        if (lt >= m_avsPeak1 * (34.0f / 32.0f) && lt > 576.0f * 16.0f)
        {
            if (m_avsCnt > 0)
            {
                m_avsCnt = 0;
                beat = true;
            }
            m_avsPeak1 = (lt + m_avsPeak1Peak) * 0.5f;
            m_avsPeak1Peak = lt;
        }
        else if (lt > m_avsPeak2)
        {
            m_avsPeak2 = lt;
        }
        else
        {
            m_avsPeak2 = m_avsPeak2 * (14.0f / 16.0f);
        }
        return beat;
    }

    /// @brief Reset detection state (config stays)
    void reset()
    {
        m_lastLevel = 0.0f;
        m_adaptiveThreshold = 0.0f;
        m_avsPeak1 = 0.0f;
        m_avsPeak2 = 0.0f;
        m_avsPeak1Peak = 0.0f;
        m_avsCnt = 0;
    }

    /// @brief Reset config AND state to defaults
    void resetToDefaults()
    {
        m_threshold = 0.4f;
        m_sensitivity = 1.0f;
        reset();
    }

private:
    static constexpr float kAdaptiveFollow = 0.95f;     ///< Running-average inertia
    static constexpr float kAdaptiveRatio = 1.5f;       ///< Beat = energy > ratio × threshold
    static constexpr float kAdaptiveMinEnergy = 0.1f;   ///< Noise floor

    float m_threshold = 0.4f;
    float m_sensitivity = 1.0f;
    float m_lastLevel = 0.0f;
    float m_adaptiveThreshold = 0.0f;

    // AVS onset tracker state (ref main.cpp beat_peak1/beat_peak2/beat_cnt)
    float m_avsPeak1 = 0.0f;
    float m_avsPeak2 = 0.0f;
    float m_avsPeak1Peak = 0.0f;
    int m_avsCnt = 0;
};

} // namespace lumi::modules
