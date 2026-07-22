/**
 ****************************************************************************************
 * @file   MilkParserTypes.hpp
 * @brief  Data model for parsed .milk presets (scalars, code blocks, waves/shapes)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Header-only library (no Qt, no app dependencies). The parse result mirrors the
 * on-disk structure of MilkDrop presets: INI-like key=value scalars plus numbered
 * code lines (per_frame_N, per_pixel_N, ...) that are concatenated into code
 * blocks. Code stays source text here (Milk-EEL / HLSL) — transpilation is the
 * EelTranspiler's job, translation into the MilkdropVisualizer happens in M3.
 *
 * The model is the MD3 superset (decision §6.2, 2026-07-22): up to 16 custom
 * waves/shapes, sprite sections, PSVERSION headers. What the renderer cannot do
 * yet still parses cleanly and is visible for the import report.
 *
 * Format reference: ref/MilkDrop3/code (BSD) and ref/winamp_orig (behaviour);
 * concept: projects/apps/MyViz/docs/visuals/MilkDrop_Import_Konzept.md §2.2.
 ****************************************************************************************
 */

#pragma once

#include <charconv>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::milk {

/// MD3 superset caps (MD2: 4 waves / 4 shapes)
inline constexpr int kMaxWaves = 16;
inline constexpr int kMaxShapes = 16;
/// Per code block; matches the EelTranspiler source cap (2^20)
inline constexpr std::size_t kMaxCodeSize = std::size_t{1} << 20;

namespace detail {

[[nodiscard]] inline char asciiLower(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

[[nodiscard]] inline bool iequals(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (asciiLower(a[i]) != asciiLower(b[i])) return false;
    }
    return true;
}

} // namespace detail

/**
 * @brief One scalar key=value pair, in file order (value stays raw text)
 */
struct KeyValue
{
    std::string key;
    std::string value;
};

/// @brief First entry with matching key (case-insensitive), nullptr if absent
[[nodiscard]] inline const std::string* findParam(const std::vector<KeyValue>& params,
                                                  std::string_view name)
{
    for (const KeyValue& kv : params)
    {
        if (detail::iequals(kv.key, name)) return &kv.value;
    }
    return nullptr;
}

/// @brief Param as double (locale-independent via from_chars), default if absent/garbled
[[nodiscard]] inline double paramDouble(const std::vector<KeyValue>& params,
                                        std::string_view name, double defaultValue)
{
    const std::string* v = findParam(params, name);
    if (v == nullptr) return defaultValue;
    double out = defaultValue;
    const char* begin = v->c_str();
    const char* end = begin + v->size();
    while (begin != end && (*begin == ' ' || *begin == '\t')) ++begin;
    const auto [ptr, ec] = std::from_chars(begin, end, out);
    return (ec == std::errc{} && ptr != begin) ? out : defaultValue;
}

/// @brief Param as int; accepts "0x"-prefixed hex (SpriteColorKey style)
[[nodiscard]] inline int paramInt(const std::vector<KeyValue>& params,
                                  std::string_view name, int defaultValue)
{
    const std::string* v = findParam(params, name);
    if (v == nullptr) return defaultValue;
    std::string_view s(*v);
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
    int base = 10;
    if (s.size() > 2 && s[0] == '0' && detail::asciiLower(s[1]) == 'x')
    {
        base = 16;
        s.remove_prefix(2);
    }
    int out = defaultValue;
    const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), out, base);
    return (ec == std::errc{} && ptr != s.data()) ? out : defaultValue;
}

/**
 * @brief One custom wave (wavecode_I_* params + wave_I_* code slots)
 */
struct CustomWave
{
    int index = -1;                     ///< 0..15 (file order not guaranteed)
    std::vector<KeyValue> params;       ///< wavecode_I_<name> with the prefix stripped
    std::string initCode;               ///< wave_I_initN
    std::string frameCode;              ///< wave_I_per_frameN
    std::string pointCode;              ///< wave_I_per_pointN

    [[nodiscard]] double param(std::string_view name, double defaultValue) const
    {
        return paramDouble(params, name, defaultValue);
    }
};

/**
 * @brief One custom shape (shapecode_I_* params + shape_I_* code slots; no per_point)
 */
struct CustomShape
{
    int index = -1;                     ///< 0..15
    std::vector<KeyValue> params;       ///< shapecode_I_<name> with the prefix stripped
    std::string initCode;               ///< shape_I_initN
    std::string frameCode;              ///< shape_I_per_frameN

    [[nodiscard]] double param(std::string_view name, double defaultValue) const
    {
        return paramDouble(params, name, defaultValue);
    }
};

/**
 * @brief One MD3 sprite section ([SPRITEn_BEGIN] .. [SPRITEn_END])
 */
struct Sprite
{
    int index = 0;                      ///< n from the section name
    std::vector<KeyValue> params;       ///< SpriteName, SpriteBlend, ...
    std::string code;                   ///< code_N lines, concatenated
};

/**
 * @brief Result of parsing one .milk file
 *
 * ok=false only when the text contains no recognizable preset data at all
 * (empty / no key=value lines). Everything else is best effort: oddities land
 * in @ref warnings, never a hard failure (Import-Analyse §4.3).
 */
struct ParseResult
{
    bool ok = false;
    std::string error;                  ///< set when !ok

    // --- version headers (absent = MD1-era file) -------------------------------------
    int presetVersion = 0;              ///< MILKDROP_PRESET_VERSION (201=MD2, 300=MD3, 0=none)
    int psVersion = -1;                 ///< PSVERSION (-1 = absent)
    int psVersionWarp = -1;             ///< PSVERSION_WARP
    int psVersionComp = -1;             ///< PSVERSION_COMP
    bool hadPresetSection = false;      ///< [presetNN] seen (hand-written files may lack it)

    // --- preset body ------------------------------------------------------------------
    std::vector<KeyValue> params;       ///< all scalars in file order (header + body)
    std::string perFrameInitCode;       ///< per_frame_init_N
    std::string perFrameCode;           ///< per_frame_N
    std::string perPixelCode;           ///< per_pixel_N ("per pixel" = per warp-mesh vertex)
    std::string warpShader;             ///< warp_N backtick lines (HLSL source)
    std::string compShader;             ///< comp_N backtick lines (HLSL source)
    std::vector<CustomWave> waves;      ///< only waves present in the file, index-sorted
    std::vector<CustomShape> shapes;    ///< only shapes present in the file, index-sorted
    std::vector<Sprite> sprites;        ///< MD3 sprite sections, file order

    std::vector<std::string> warnings;  ///< import report findings

    /// @brief MilkDrop generation: 1 (no version headers), 2 (v201/PSVERSION), 3 (v300+)
    [[nodiscard]] int generation() const
    {
        if (presetVersion >= 300) return 3;
        if (presetVersion >= 200 || psVersion >= 0) return 2;
        return 1;
    }

    /// @brief Scalar lookup, case-insensitive; nullptr if absent
    [[nodiscard]] const std::string* rawValue(std::string_view name) const
    {
        return findParam(params, name);
    }

    /// @brief Scalar as double (fRating, zoom, ...), default if absent
    [[nodiscard]] double value(std::string_view name, double defaultValue) const
    {
        return paramDouble(params, name, defaultValue);
    }

    /// @brief Scalar as int (nWaveMode, bTexWrap, ...), default if absent
    [[nodiscard]] int valueInt(std::string_view name, int defaultValue) const
    {
        return paramInt(params, name, defaultValue);
    }

    /// @brief Wave by index (nullptr if the file defines none for it)
    [[nodiscard]] const CustomWave* wave(int index) const
    {
        for (const CustomWave& w : waves)
        {
            if (w.index == index) return &w;
        }
        return nullptr;
    }

    /// @brief Shape by index (nullptr if the file defines none for it)
    [[nodiscard]] const CustomShape* shape(int index) const
    {
        for (const CustomShape& s : shapes)
        {
            if (s.index == index) return &s;
        }
        return nullptr;
    }
};

} // namespace lumi::milk
