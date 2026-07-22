/**
 ****************************************************************************************
 * @file   MilkdropBlur.hpp
 * @brief  Blur-pyramid math (ranges, kernel weights, texture sizes) — M5, pure/no GL
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * 1:1 port of the MilkDrop blur-pass parameter math (ref milkdropfs.cpp:
 * GetSafeBlurMinMax :1468-1499, BlurPasses :1501-1679; texture-size chain
 * plugin.cpp:1990-2008). Three user-visible blur levels, each rendered in two
 * passes (long horizontal + short vertical) over six internal textures of
 * halving resolution. The per-level [min..max] ranges compress the value range
 * progressively (fscale/fbias); sampling un-biases with GetBlurN(uv) =
 * tex*(max-min)+min (include.fx:118-120).
 *
 * PORT: the reference's range clamp sets min == max when the gap is < 0.1
 * (both `avg - 0.05`, identical in winamp_orig AND MilkDrop3 — an obvious
 * typo whose 1/(max-min) becomes +inf; D3D9's unorm clamp turns that into a
 * hard threshold). We keep the collapse but guard the denominator with a tiny
 * epsilon so GL never sees inf/NaN — same extreme-contrast look, deterministic.
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <array>

namespace lumi::milkdrop {

/// Number of internal blur textures (2 per user-visible level)
inline constexpr int kBlurTexCount = 6;

/// Fixed kernel taps (milkdropfs.cpp:1564; "user can specify these" — nobody does)
inline constexpr std::array<float, 8> kBlurWeights = {4.0f, 3.8f, 3.5f, 2.9f,
                                                      1.9f, 1.2f, 0.7f, 0.3f};

/**
 * @brief Safe per-level blur ranges (monotonic, gap-collapsed like the original)
 */
struct BlurRanges
{
    std::array<float, 3> min{};
    std::array<float, 3> max{};
};

/**
 * @brief GetSafeBlurMinMax port: enforce shrinking gaps, collapse tiny gaps
 */
[[nodiscard]] inline BlurRanges computeSafeBlurRanges(const std::array<float, 3>& minIn,
                                                      const std::array<float, 3>& maxIn)
{
    BlurRanges r;
    r.min = minIn;
    r.max = maxIn;

    constexpr float kMinDist = 0.1f;
    const auto collapse = [](float& lo, float& hi) {
        if (hi - lo < kMinDist)
        {
            const float avg = (lo + hi) * 0.5f;
            lo = avg - kMinDist * 0.5f;
            hi = avg - kMinDist * 0.5f;  // reference typo kept (see PORT note above)
        }
    };
    collapse(r.min[0], r.max[0]);
    r.max[1] = std::min(r.max[0], r.max[1]);
    r.min[1] = std::max(r.min[0], r.min[1]);
    collapse(r.min[1], r.max[1]);
    r.max[2] = std::min(r.max[1], r.max[2]);
    r.min[2] = std::max(r.min[1], r.min[2]);
    collapse(r.min[2], r.max[2]);
    return r;
}

/// Denominator guard: the collapsed ranges divide by 0 in the reference (D3D
/// clamps the inf); keep the sign, bound the magnitude.
[[nodiscard]] inline float safeInverse(float d)
{
    constexpr float kEps = 1.0f / 1024.0f;
    if (d >= 0.0f) return 1.0f / std::max(d, kEps);
    return 1.0f / std::min(d, -kEps);
}

/**
 * @brief Progressive scale/bias per level pair (BlurPasses :1569-1584)
 */
struct BlurPassScales
{
    std::array<float, 3> scale{};
    std::array<float, 3> bias{};
};

[[nodiscard]] inline BlurPassScales computeBlurPassScales(const BlurRanges& r)
{
    BlurPassScales s;
    s.scale[0] = safeInverse(r.max[0] - r.min[0]);
    s.bias[0] = -r.min[0] * s.scale[0];
    float tempMin = (r.min[1] - r.min[0]) * safeInverse(r.max[0] - r.min[0]);
    float tempMax = (r.max[1] - r.min[0]) * safeInverse(r.max[0] - r.min[0]);
    s.scale[1] = safeInverse(tempMax - tempMin);
    s.bias[1] = -tempMin * s.scale[1];
    tempMin = (r.min[2] - r.min[1]) * safeInverse(r.max[1] - r.min[1]);
    tempMax = (r.max[2] - r.min[1]) * safeInverse(r.max[1] - r.min[1]);
    s.scale[2] = safeInverse(tempMax - tempMin);
    s.bias[2] = -tempMin * s.scale[2];
    return s;
}

/**
 * @brief Constants of the long horizontal pass (BlurPasses :1612-1634)
 */
struct BlurKernelH
{
    float w1, w2, w3, w4;
    float d1, d2, d3, d4;
    float wDiv;
};

[[nodiscard]] inline BlurKernelH blurKernelH()
{
    const std::array<float, 8>& w = kBlurWeights;
    BlurKernelH k{};
    k.w1 = w[0] + w[1];
    k.w2 = w[2] + w[3];
    k.w3 = w[4] + w[5];
    k.w4 = w[6] + w[7];
    k.d1 = 0.0f + 2.0f * w[1] / k.w1;
    k.d2 = 2.0f + 2.0f * w[3] / k.w2;
    k.d3 = 4.0f + 2.0f * w[5] / k.w3;
    k.d4 = 6.0f + 2.0f * w[7] / k.w4;
    k.wDiv = 0.5f / (k.w1 + k.w2 + k.w3 + k.w4);
    return k;
}

/**
 * @brief Constants of the short vertical pass (BlurPasses :1636-1644)
 */
struct BlurKernelV
{
    float w1, w2;
    float d1, d2;
    float wDiv;
};

[[nodiscard]] inline BlurKernelV blurKernelV()
{
    const std::array<float, 8>& w = kBlurWeights;
    BlurKernelV k{};
    k.w1 = w[0] + w[1] + w[2] + w[3];
    k.w2 = w[4] + w[5] + w[6] + w[7];
    k.d1 = 0.0f + 2.0f * ((w[2] + w[3]) / k.w1);
    k.d2 = 2.0f + 2.0f * ((w[6] + w[7]) / k.w2);
    k.wDiv = 1.0f / ((k.w1 + k.w2) * 2.0f);
    return k;
}

/**
 * @brief Sizes of the 6 internal blur textures for a given source size
 *
 * Halving chain with the reference's rounding (plugin.cpp:1990-2008): every
 * even index and the first pair halve; width rounds up to /16, height to /4.
 */
[[nodiscard]] inline std::array<std::array<int, 2>, kBlurTexCount> blurTextureSizes(
    int sourceW, int sourceH)
{
    std::array<std::array<int, 2>, kBlurTexCount> sizes{};
    int w = sourceW;
    int h = sourceH;
    for (int i = 0; i < kBlurTexCount; ++i)
    {
        if ((i & 1) == 0 || i < 2)
        {
            w = std::max(16, w / 2);
            h = std::max(16, h / 2);
        }
        sizes[static_cast<std::size_t>(i)] = {((w + 3) / 16) * 16, ((h + 3) / 4) * 4};
    }
    return sizes;
}

} // namespace lumi::milkdrop
