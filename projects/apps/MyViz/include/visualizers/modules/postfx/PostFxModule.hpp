/**
 ****************************************************************************************
 * @file   PostFxModule.hpp
 * @brief  Shared post-processing building blocks (Phase 4 Schritt 5.6)
 *
 * Consolidates the per-visualizer frame-fade copies (Waveform/Superscope hold,
 * Oscilloscope phosphor/trigger fade, Equalizer particles use their own physics).
 * First user: WaveformVisualizer (post.hold.*, 5.3); Oscilloscope and Superscope
 * follow with their migrations. Shader-based effects (glow, blur) extend this
 * module later — value source and effect stay decoupled (Konzept §5.6, guard 1).
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <deque>
#include <utility>
#include <vector>

namespace lumi::modules {

/**
 * @brief One held frame of the hold/fade trail
 * @tparam TFrameData Frame payload (sample buffer, point list, …)
 */
template <typename TFrameData>
struct HeldFrameT
{
    TFrameData data;
    float age = 0.0f;    ///< Seconds since capture
    float alpha = 1.0f;  ///< 1 → fresh, 0 → faded out (linear over fadeTime)
};

/**
 * @class HoldFadeEffectT
 * @brief Persistence/trail effect: keeps past frames and fades them linearly
 *
 * Config (enabled/fadeTime/maxFrames) stays with the owning visualizer's
 * parameter schema (post.hold.*); this class owns the frame mechanics that
 * were previously copy-pasted per visualizer.
 *
 * @tparam TFrameData Frame payload — float buffers (Waveform), point lists
 *                    (Superscope), …
 */
template <typename TFrameData>
class HoldFadeEffectT
{
public:
    using Frame = HeldFrameT<TFrameData>;

    [[nodiscard]] static const char* moduleName() { return "HoldFade"; }

    /**
     * @brief Capture a frame at the back of the trail
     * @param data Frame payload (moved in)
     * @param maxFrames Trail length limit (oldest frames are dropped)
     */
    void push(TFrameData data, int maxFrames)
    {
        m_frames.push_back(Frame{std::move(data), 0.0f, 1.0f});
        const auto limit = static_cast<size_t>(std::max(1, maxFrames));
        while (m_frames.size() > limit)
        {
            m_frames.pop_front();
        }
    }

    /**
     * @brief Age all frames and drop fully faded ones
     * @param deltaTime Seconds since last update
     * @param fadeTime Seconds until a frame is fully faded (alpha 0)
     */
    void update(float deltaTime, float fadeTime)
    {
        const float safeFade = std::max(0.001f, fadeTime);
        for (auto& frame : m_frames)
        {
            frame.age += deltaTime;
            frame.alpha = std::max(0.0f, 1.0f - frame.age / safeFade);
        }
        while (!m_frames.empty() && m_frames.front().alpha <= 0.0f)
        {
            m_frames.pop_front();
        }
    }

    /// @brief Drop all held frames
    void clear() { m_frames.clear(); }

    /// @brief Held frames, oldest first
    [[nodiscard]] const std::deque<Frame>& frames() const { return m_frames; }

private:
    std::deque<Frame> m_frames;
};

/// Default instantiation for float sample buffers (Waveform hold/fade)
using HoldFadeEffect = HoldFadeEffectT<std::vector<float>>;
using HeldFrame = HeldFrameT<std::vector<float>>;

} // namespace lumi::modules
