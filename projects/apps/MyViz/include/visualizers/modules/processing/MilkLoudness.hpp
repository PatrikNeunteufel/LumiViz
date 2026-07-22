/**
 ****************************************************************************************
 * @file   MilkLoudness.hpp
 * @brief  MilkDrop band loudness — relative bass/mid/treb + attenuated *_att variants
 *         (port of the sound analysis in ref/MilkDrop3/code/vis_milk2/plugin.cpp
 *         lines 8749-8779 + utility.cpp AdjustRateToFPS; BSD, Nullsoft/MilkDrop3)
 *
 * @author LumiPulse Team (port); original algorithm: Nullsoft, Inc. (BSD)
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * MilkDrop presets read bass/mid/treb as loudness RELATIVE to the long-term
 * average of the same band (imm_rel = imm / long_avg): quiet passage ~= 1.0,
 * a beat spikes to ~1.3+. The *_att variants (avg_rel) are the attenuated
 * (smoothed) versions with asymmetric attack/release. Feed one call per frame
 * with the three raw band energies (any consistent scale — the ratio cancels
 * the unit) and the current fps; read the six preset inputs afterwards.
 *
 * Reference behaviour kept 1:1: attack rate 0.2 / release 0.5 (at 30 fps),
 * long-average rate 0.992 (0.9 during the first 50 warm-up frames), rates
 * fps-corrected via AdjustRateToFPS (rate^(30/fps)), division guard 0.001 -> 1.0.
 *
 * MilkDrop-Import M2 (Skript-Vertrag), MilkDrop_Import_Konzept.md §3.2.
 * Threading: plain object, one thread at a time (render-thread owned).
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <cmath>

namespace lumi::modules {

/**
 * @class MilkLoudness
 * @brief Relative band loudness (bass/mid/treb + *_att) after the MilkDrop model
 */
class MilkLoudness
{
public:
    static constexpr int kBands = 3;            ///< 0=bass, 1=mid, 2=treb
    static constexpr double kReferenceFps = 30.0;
    static constexpr int kWarmupFrames = 50;    ///< long_avg settles fast at first

    MilkLoudness() { reset(); }

    /// @brief Back to the cold-start state (all averages at the first fed value)
    void reset()
    {
        m_frame = 0;
        m_avg = {};
        m_longAvg = {};
        m_immRel = {1.0, 1.0, 1.0};
        m_avgRel = {1.0, 1.0, 1.0};
    }

    /**
     * @brief Feed one frame of raw band energies (plugin.cpp:8749-8779)
     * @param bassImm raw bass-band energy (scale-free — only ratios matter)
     * @param fps     current frames per second (rates are fps-corrected)
     */
    void update(double bassImm, double midImm, double trebImm, double fps)
    {
        const std::array<double, kBands> imm = {bassImm, midImm, trebImm};
        if (fps <= 0.0) fps = kReferenceFps;

        // cold start: seed the averages with the first frame instead of ramping from 0
        if (m_frame == 0)
        {
            m_avg = imm;
            m_longAvg = imm;
        }

        for (int i = 0; i < kBands; ++i)
        {
            // attenuated version: fast attack (0.2), slow release (0.5)
            const double avgRate =
                adjustRateToFps(imm[i] > m_avg[i] ? 0.2 : 0.5, fps);
            m_avg[i] = m_avg[i] * avgRate + imm[i] * (1.0 - avgRate);

            // long-term average: nearly frozen (0.992), faster during warm-up
            const double longRate =
                adjustRateToFps(m_frame < kWarmupFrames ? 0.9 : 0.992, fps);
            m_longAvg[i] = m_longAvg[i] * longRate + imm[i] * (1.0 - longRate);

            // levels relative to the past (guard: silence reads as 1.0, not inf)
            if (std::fabs(m_longAvg[i]) < 0.001)
            {
                m_immRel[i] = 1.0;
                m_avgRel[i] = 1.0;
            }
            else
            {
                m_immRel[i] = imm[i] / m_longAvg[i];
                m_avgRel[i] = m_avg[i] / m_longAvg[i];
            }
        }
        ++m_frame;
    }

    // --- preset inputs (milkdropfs.cpp:483-488) -----------------------------------------
    [[nodiscard]] double bass() const { return m_immRel[0]; }
    [[nodiscard]] double mid() const { return m_immRel[1]; }
    [[nodiscard]] double treb() const { return m_immRel[2]; }
    [[nodiscard]] double bassAtt() const { return m_avgRel[0]; }
    [[nodiscard]] double midAtt() const { return m_avgRel[1]; }
    [[nodiscard]] double trebAtt() const { return m_avgRel[2]; }

    [[nodiscard]] long frame() const { return m_frame; }

private:
    /// utility.cpp:80 — per-frame rate at kReferenceFps converted to the actual fps
    [[nodiscard]] static double adjustRateToFps(double ratePerFrameAtRef, double fps)
    {
        return std::pow(ratePerFrameAtRef, kReferenceFps / fps);
    }

    long m_frame = 0;
    std::array<double, kBands> m_avg{};      ///< attenuated band energy
    std::array<double, kBands> m_longAvg{};  ///< long-term band energy
    std::array<double, kBands> m_immRel{};   ///< bass/mid/treb
    std::array<double, kBands> m_avgRel{};   ///< bass_att/mid_att/treb_att
};

} // namespace lumi::modules
