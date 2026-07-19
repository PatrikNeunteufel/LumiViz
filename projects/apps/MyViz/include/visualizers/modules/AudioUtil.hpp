/**
 ****************************************************************************************
 * @file   AudioUtil.hpp
 * @brief  Shared audio buffer helpers (Phase 4 Schritt 5.6)
 *
 * Consolidates the per-visualizer stereo-split/resample copies. The dead
 * duplicates in Oscilloscope/Superscope were removed with their migrations;
 * the Oscilloscope keeps its own inline display resampling (linear + clamp,
 * different semantics).
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <vector>

namespace lumi::modules {

/**
 * @brief Split an interleaved stereo buffer into left/right channels
 * @param interleaved L/R interleaved samples (L0 R0 L1 R1 …)
 * @param left Output left channel (resized)
 * @param right Output right channel (resized)
 */
inline void splitStereoData(const std::vector<float>& interleaved,
                            std::vector<float>& left,
                            std::vector<float>& right)
{
    const size_t samples = interleaved.size() / 2;
    left.resize(samples);
    right.resize(samples);

    for (size_t i = 0; i < samples; ++i)
    {
        left[i] = interleaved[i * 2];
        right[i] = interleaved[i * 2 + 1];
    }
}

/**
 * @brief Resample a buffer to targetSize (nearest neighbor) and apply gain
 * @param source Input samples (empty input leaves target untouched)
 * @param target Output buffer (resized to targetSize)
 * @param targetSize Number of output samples
 * @param gain Amplitude factor applied per sample
 */
inline void resampleNearest(const std::vector<float>& source,
                            std::vector<float>& target,
                            int targetSize,
                            float gain)
{
    if (source.empty() || targetSize <= 0) return;

    target.resize(targetSize);

    for (int i = 0; i < targetSize; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(targetSize - 1);
        int srcIdx = static_cast<int>(t * (source.size() - 1));
        srcIdx = std::clamp(srcIdx, 0, static_cast<int>(source.size()) - 1);

        target[i] = source[srcIdx] * gain;
    }
}

} // namespace lumi::modules
