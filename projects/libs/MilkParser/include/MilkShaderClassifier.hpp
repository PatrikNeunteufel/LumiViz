/**
 ****************************************************************************************
 * @file   MilkShaderClassifier.hpp
 * @brief  Classify .milk warp/comp HLSL shaders: MD1-default family vs. custom (M5)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * MilkDrop 2 GENERATES shader text for pre-MD2 presets from the MD1 keys
 * (GenWarpPShaderText / GenCompPShaderText, ref plugin.cpp:8782-8847). Such
 * shaders — and hand-edited variants that only add linear blur mixes or gain
 * lines — are mechanically recognizable and map EXACTLY onto the MD1 composite
 * plus a blur term. Everything else is Custom (renderer falls back, report).
 *
 * Corpus measurement (Session 40, 910 presets): 310 without shaders, ~20/14
 * pure generated defaults (warp/comp), ~60-70 "default + small extras" whose
 * extras are dominated by `ret += GetBlurN(uv)`, `ret = lerp(GetBlurN(uv),
 * GetPixel(uv), k)` and bare `ret *= k` — hence exactly this extra set.
 *
 * Matching is whitespace-insensitive (lines are compared with all blanks
 * removed) so author reformatting does not break recognition; anything that
 * does not match the known statement set makes the shader Custom. The comp
 * result is a sequential affine model  ret = A*base + sum(B[n]*blurN)  with
 * baked echo/gamma/hue/filter constants — in MD2 those constants live in the
 * shader text, so per_frame animation of e.g. `gamma` has NO effect (unlike
 * shaderless MD1 presets, where the live per-frame values drive the composite).
 *
 * Feature flags (blur/noise/textures/...) are collected for every non-empty
 * shader regardless of class — they feed the import report and the stage-C
 * coverage statistics.
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "MilkParserTypes.hpp"

namespace lumi::milk {

/// Classification of one warp/comp shader text
enum class ShaderClass
{
    None,        ///< no shader block in the file (MD1 preset — MD1 path is exact)
    Md1Default,  ///< exactly the generated default (MD1 path with baked constants)
    Md1Plus,     ///< default + recognized linear extras (blur mix / gain) — still exact
    Custom       ///< real hand-written shader (renderer falls back, report)
};

/**
 * @brief Analysis result of one shader text (see ShaderClass)
 */
struct ShaderInfo
{
    ShaderClass shaderClass = ShaderClass::None;
    int codeLines = 0;                  ///< non-boilerplate statement lines

    // --- feature flags (any non-empty shader, independent of class) --------------------
    std::array<bool, 3> usesBlur{};     ///< sampler_blurN / GetBlurN
    bool usesNoise = false;             ///< noise_lq/mq/hq/noisevol (any fc/pc/fw/pw prefix)
    bool usesTexture = false;           ///< custom sampler_XXX textures
    bool usesRand = false;              ///< rand_frame / rand_preset
    bool usesTex3d = false;
    bool usesUvOrig = false;
    std::vector<std::string> textures;  ///< custom texture base names (prefix-stripped)

    // --- warp: baked constants (Md1Default) --------------------------------------------
    double decayMul = -1.0;             ///< `ret *= k` (-1 = line absent)
    double decaySub = 0.0;              ///< `ret -= k` (file-default style darken)
    bool wrapSampler = true;            ///< sampler_main (true) vs sampler_fc_main

    // --- comp: baked constants + affine model (Md1Default / Md1Plus) -------------------
    bool hasBase = false;               ///< base sample / echo lerp statement seen
    double echoAlpha = 0.0;             ///< 0 = no echo block
    double echoZoom = 2.0;
    int echoOrient = 0;                 ///< 0..3 (bit0 = H flip, bit1 = V flip)
    double gain = 1.0;                  ///< A — total multiplier incl. the //gamma line
    std::array<double, 3> blurAdd{};    ///< B[n] — additive blurN coefficients
    double hueMix = 0.0;                ///< 0 = none, k = `ret *= (1-k) + k*hue_shader`
    bool brighten = false;
    bool darken = false;
    bool solarize = false;
    bool invert = false;

    /// @brief Highest blur level the affine model actually uses (0 = none, 1..3)
    [[nodiscard]] int highestBlurLevel() const
    {
        for (int n = 3; n >= 1; --n)
        {
            const double c = blurAdd[static_cast<std::size_t>(n - 1)];
            if (c > 1e-9 || c < -1e-9) return n;
        }
        return 0;
    }
};

namespace detail {

/// @brief Line with all whitespace removed (matching is format-insensitive)
[[nodiscard]] inline std::string squeezeLine(std::string_view line)
{
    std::string out;
    out.reserve(line.size());
    for (char c : line)
    {
        if (c != ' ' && c != '\t' && c != '\r') out.push_back(c);
    }
    return out;
}

/**
 * @brief Cursor over a squeezed line: consume literals / numbers left to right
 */
struct LineCursor
{
    std::string_view s;

    [[nodiscard]] bool lit(std::string_view text)
    {
        if (s.substr(0, text.size()) != text) return false;
        s.remove_prefix(text.size());
        return true;
    }

    [[nodiscard]] bool num(double& out)
    {
        const char* begin = s.data();
        const char* end = begin + s.size();
        const auto [ptr, ec] = std::from_chars(begin, end, out);
        if (ec != std::errc{} || ptr == begin) return false;
        s.remove_prefix(static_cast<std::size_t>(ptr - begin));
        return true;
    }

    /// blur level digit 1..3
    [[nodiscard]] bool blurLevel(int& out)
    {
        if (s.empty() || s.front() < '1' || s.front() > '3') return false;
        out = s.front() - '0';
        s.remove_prefix(1);
        return true;
    }

    /// end of statement: nothing left, or only a trailing comment
    [[nodiscard]] bool tail()
    {
        return s.empty() || s.substr(0, 2) == "//";
    }
};

/// @brief True for lines that carry no statement (blanks, braces, comments, header)
[[nodiscard]] inline bool isBoilerplate(std::string_view squeezed)
{
    if (squeezed.empty()) return true;
    if (squeezed.substr(0, 2) == "//") return true;
    if (squeezed == "{" || squeezed == "}") return true;
    if (squeezed.substr(0, 11) == "shader_body") return true;
    return false;
}

/// @brief Strip a leading "{" / trailing "}" glued onto a statement line
[[nodiscard]] inline std::string_view trimBraces(std::string_view s)
{
    if (!s.empty() && s.front() == '{') s.remove_prefix(1);
    if (!s.empty() && s.back() == '}') s.remove_suffix(1);
    return s;
}

/// @brief Case-sensitive substring test on the raw shader text
[[nodiscard]] inline bool containsToken(std::string_view text, std::string_view token)
{
    return text.find(token) != std::string_view::npos;
}

/// @brief Collect feature flags + custom texture names from the raw text
inline void scanFeatures(std::string_view text, ShaderInfo& info)
{
    for (int n = 1; n <= 3; ++n)
    {
        const std::string samplerName = "sampler_blur" + std::to_string(n);
        const std::string getterName = "GetBlur" + std::to_string(n);
        info.usesBlur[static_cast<std::size_t>(n - 1)] =
            containsToken(text, samplerName) || containsToken(text, getterName);
    }
    info.usesRand = containsToken(text, "rand_frame") || containsToken(text, "rand_preset");
    info.usesTex3d = containsToken(text, "tex3D");
    info.usesUvOrig = containsToken(text, "uv_orig");

    // sampler_<name> occurrences; fc_/pc_/fw_/pw_ prefixes are sampling modes
    constexpr std::string_view kSampler = "sampler_";
    std::size_t pos = 0;
    while ((pos = text.find(kSampler, pos)) != std::string_view::npos)
    {
        pos += kSampler.size();
        std::size_t end = pos;
        while (end < text.size() &&
               (text[end] == '_' || (text[end] >= '0' && text[end] <= '9') ||
                (text[end] >= 'a' && text[end] <= 'z') ||
                (text[end] >= 'A' && text[end] <= 'Z')))
        {
            ++end;
        }
        std::string_view name = text.substr(pos, end - pos);
        for (std::string_view prefix : {"fc_", "pc_", "fw_", "pw_"})
        {
            if (name.substr(0, prefix.size()) == prefix)
            {
                name.remove_prefix(prefix.size());
                break;
            }
        }
        const bool known = name == "main" || name.substr(0, 4) == "blur" ||
                           name.substr(0, 6) == "noise_" || name.substr(0, 9) == "noisevol_";
        if (name.substr(0, 6) == "noise_" || name.substr(0, 9) == "noisevol_")
        {
            info.usesNoise = true;
        }
        if (!known && !name.empty())
        {
            info.usesTexture = true;
            bool seen = false;
            for (const std::string& t : info.textures) seen = seen || t == name;
            if (!seen) info.textures.emplace_back(name);
        }
        pos = end;
    }
}

/// @brief Statement lines of one shader text (squeezed, boilerplate removed)
[[nodiscard]] inline std::vector<std::string> statementLines(std::string_view text)
{
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start <= text.size())
    {
        const std::size_t nl = text.find('\n', start);
        const std::string_view raw =
            text.substr(start, (nl == std::string_view::npos) ? text.size() - start
                                                              : nl - start);
        std::string squeezed = squeezeLine(raw);
        std::string_view body = trimBraces(squeezed);
        if (!isBoilerplate(body)) lines.emplace_back(body);
        if (nl == std::string_view::npos) break;
        start = nl + 1;
    }
    return lines;
}

} // namespace detail

/**
 * @brief Classify a warp shader text (empty → None)
 *
 * Recognized set (generated default family): the base sample (wrap or clamp),
 * `ret *= k` (decay) and `ret -= k` (file-default darken). Anything else makes
 * it Custom; there is no Md1Plus for warp (extras in the corpus are too
 * heterogeneous to be worth exact modelling).
 */
[[nodiscard]] inline ShaderInfo analyzeWarpShader(std::string_view text)
{
    ShaderInfo info;
    if (text.empty()) return info;
    detail::scanFeatures(text, info);

    const std::vector<std::string> lines = detail::statementLines(text);
    info.codeLines = static_cast<int>(lines.size());
    info.shaderClass = ShaderClass::Custom;

    bool baseSeen = false;
    for (const std::string& line : lines)
    {
        detail::LineCursor c{line};
        if (c.lit("ret=tex2D(sampler_main,uv).xyz;") && c.tail())
        {
            baseSeen = true;
            info.wrapSampler = true;
            continue;
        }
        c = {line};
        if (c.lit("ret=tex2D(sampler_fc_main,uv).xyz;") && c.tail())
        {
            baseSeen = true;
            info.wrapSampler = false;
            continue;
        }
        c = {line};
        double k = 0.0;
        if (c.lit("ret*=") && c.num(k) && c.lit(";") && c.tail())
        {
            info.decayMul = (info.decayMul < 0.0) ? k : info.decayMul * k;
            continue;
        }
        c = {line};
        if (c.lit("ret-=") && c.num(k) && c.lit(";") && c.tail())
        {
            info.decaySub += k;
            continue;
        }
        return info;  // unknown statement → Custom
    }
    if (baseSeen) info.shaderClass = ShaderClass::Md1Default;
    return info;
}

/**
 * @brief Classify a comp shader text (empty → None)
 *
 * Sequentially evaluates the statement list into the affine model
 * ret = gain*base + sum(blurAdd[n]*GetBlurN) with baked echo/gamma/hue/filter
 * constants. Recognized extras beyond the generated default: `ret += GetBlurN
 * (uv) [*k]`, `ret = lerp(GetBlurN(uv), GetPixel(uv), k)` (both argument
 * orders), `ret = GetPixel(uv)*a + GetBlurN(uv)*b`, bare `ret *= k`, and the
 * no-op `ret = ret`. Affine statements after a filter/hue line are NOT
 * representable in the fixed composite order → Custom.
 */
[[nodiscard]] inline ShaderInfo analyzeCompShader(std::string_view text)
{
    ShaderInfo info;
    if (text.empty()) return info;
    detail::scanFeatures(text, info);

    const std::vector<std::string> lines = detail::statementLines(text);
    info.codeLines = static_cast<int>(lines.size());
    info.shaderClass = ShaderClass::Custom;

    bool plus = false;           // any recognized extra beyond the generated set
    bool filtersStarted = false; // hue/filter lines are order-fixed at the end
    int echoPending = 0;         // multi-line echo lerp state machine
    int gainLines = 0;

    const auto setBase = [&](double a) {
        info.hasBase = true;
        info.gain = a;
        info.blurAdd = {0.0, 0.0, 0.0};
    };

    for (const std::string& line : lines)
    {
        detail::LineCursor c{line};
        double k = 0.0;
        double k2 = 0.0;
        int level = 0;

        // --- pending echo lerp block (generated across 4 lines) -----------------------
        if (echoPending == 1)
        {
            if (c.lit("tex2D(sampler_main,uv_echo).xyz,") && c.tail())
            {
                echoPending = 2;
                continue;
            }
            return info;
        }
        if (echoPending == 2)
        {
            if (c.num(k) && c.tail())
            {
                info.echoAlpha = k;
                echoPending = 3;
                continue;
            }
            return info;
        }
        if (echoPending == 3)
        {
            if (c.lit(");") && c.tail())
            {
                echoPending = 0;
                setBase(1.0);
                continue;
            }
            return info;
        }

        // --- generated statements -----------------------------------------------------
        if (c.lit("ret=tex2D(sampler_main,uv).xyz;") && c.tail())
        {
            if (filtersStarted) return info;
            setBase(1.0);
            continue;
        }
        c = {line};
        double ox = 0.0;
        double oy = 0.0;
        if (c.lit("float2uv_echo=(uv-0.5)*") && c.num(k) && c.lit("*float2(") && c.num(ox) &&
            c.lit(",") && c.num(oy) && c.lit(")+0.5;") && c.tail())
        {
            info.echoZoom = (k > 1e-9) ? 1.0 / k : 1.0;
            info.echoOrient = ((ox < 0.0) ? 1 : 0) + ((oy < 0.0) ? 2 : 0);
            continue;
        }
        c = {line};
        if (c.lit("ret=lerp(tex2D(sampler_main,uv).xyz,") && c.tail())
        {
            if (filtersStarted) return info;
            echoPending = 1;
            continue;
        }
        c = {line};
        if (c.lit("ret*=hue_shader;") && c.tail())
        {
            info.hueMix = 1.0;
            filtersStarted = true;
            continue;
        }
        c = {line};
        if (c.lit("ret*=") && c.num(k) && c.lit("+") && c.num(k2) && c.lit("*hue_shader;") &&
            c.tail())
        {
            info.hueMix = k2;
            filtersStarted = true;
            continue;
        }
        c = {line};
        if (c.lit("ret=sqrt(ret);") && c.tail())
        {
            info.brighten = true;
            filtersStarted = true;
            continue;
        }
        c = {line};
        if (c.lit("ret*=ret;") && c.tail())
        {
            info.darken = true;
            filtersStarted = true;
            continue;
        }
        c = {line};
        if (c.lit("ret=ret*(1-ret)*4;") && c.tail())
        {
            info.solarize = true;
            filtersStarted = true;
            continue;
        }
        c = {line};
        if (c.lit("ret=1-ret;") && c.tail())
        {
            info.invert = true;
            filtersStarted = true;
            continue;
        }
        c = {line};
        if (c.lit("ret=ret;") && c.tail())
        {
            continue;  // no-op ("ret = ret; //brighten" with the flag off)
        }

        // --- affine statements (gain / blur mixes) ------------------------------------
        c = {line};
        if (c.lit("ret*=") && c.num(k) && c.lit(";") && c.tail())
        {
            if (filtersStarted) return info;
            info.gain *= k;
            for (double& b : info.blurAdd) b *= k;
            ++gainLines;
            continue;
        }
        c = {line};
        if (c.lit("ret+=GetBlur") && c.blurLevel(level) && c.lit("(uv);") && c.tail())
        {
            if (filtersStarted) return info;
            info.blurAdd[static_cast<std::size_t>(level - 1)] += 1.0;
            plus = true;
            continue;
        }
        c = {line};
        if (c.lit("ret+=GetBlur") && c.blurLevel(level) && c.lit("(uv)*") && c.num(k) &&
            c.lit(";") && c.tail())
        {
            if (filtersStarted) return info;
            info.blurAdd[static_cast<std::size_t>(level - 1)] += k;
            plus = true;
            continue;
        }
        c = {line};
        if (c.lit("ret+=") && c.num(k) && c.lit("*GetBlur") && c.blurLevel(level) &&
            c.lit("(uv);") && c.tail())
        {
            if (filtersStarted) return info;
            info.blurAdd[static_cast<std::size_t>(level - 1)] += k;
            plus = true;
            continue;
        }
        c = {line};
        if (c.lit("ret=lerp(GetBlur") && c.blurLevel(level) && c.lit("(uv),GetPixel(uv),") &&
            c.num(k) && c.lit(");") && c.tail())
        {
            if (filtersStarted) return info;
            setBase(k);
            info.blurAdd[static_cast<std::size_t>(level - 1)] = 1.0 - k;
            plus = true;
            continue;
        }
        c = {line};
        if (c.lit("ret=lerp(GetPixel(uv),GetBlur") && c.blurLevel(level) && c.lit("(uv),") &&
            c.num(k) && c.lit(");") && c.tail())
        {
            if (filtersStarted) return info;
            setBase(1.0 - k);
            info.blurAdd[static_cast<std::size_t>(level - 1)] = k;
            plus = true;
            continue;
        }
        c = {line};
        if (c.lit("ret=GetPixel(uv)*") && c.num(k) && c.lit("+GetBlur") && c.blurLevel(level) &&
            c.lit("(uv)*") && c.num(k2) && c.lit(";") && c.tail())
        {
            if (filtersStarted) return info;
            setBase(k);
            info.blurAdd[static_cast<std::size_t>(level - 1)] = k2;
            plus = true;
            continue;
        }

        return info;  // unknown statement → Custom
    }

    if (echoPending != 0 || !info.hasBase) return info;
    // the additive composite cannot render negative terms — classify as Custom
    if (info.gain < 0.0) return info;
    for (double b : info.blurAdd)
    {
        if (b < 0.0) return info;
    }
    info.shaderClass = (plus || gainLines > 1) ? ShaderClass::Md1Plus : ShaderClass::Md1Default;
    return info;
}

} // namespace lumi::milk
