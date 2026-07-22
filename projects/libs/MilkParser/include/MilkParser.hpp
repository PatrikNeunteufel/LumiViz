/**
 ****************************************************************************************
 * @file   MilkParser.hpp
 * @brief  Public API: parse a MilkDrop .milk preset file into a structured result
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Header-only library (no Qt, no app dependencies). Import-phase Roadmap 6, M1.
 * Line-based key=value format: scalars are collected in file order, numbered
 * code families (per_frame_N, per_pixel_N, wave_I_per_frameN, warp_N, ...) are
 * concatenated starting at index 1 and stop at the first gap (original ReadCode
 * behaviour). A leading backtick marks shader/code lines that keep their line
 * break; all other code lines are concatenated verbatim (expressions may span
 * lines). MD3 superset: up to 16 waves/shapes, [SPRITEn_BEGIN] sections,
 * PSVERSION[_WARP/_COMP] headers.
 *
 * Error philosophy (Import-Analyse §4.3): ok=false only when the text holds no
 * recognizable preset data at all. Anything odd (gaps with orphan lines,
 * duplicate code lines, unknown sections, lines without '=') lands as a warning
 * in the import report — parsing never throws, never hard-fails.
 *
 * Parser behaviour derived from the reference presets (ref/winamp_orig,
 * asset/Milkdrop3) and the MilkDrop3 source (BSD); the projectM parser was
 * used as a concept reference only (LGPL — no code taken). Concept:
 * projects/apps/MyViz/docs/visuals/MilkDrop_Import_Konzept.md §2.2.
 ****************************************************************************************
 */

#pragma once

#include "MilkParserTypes.hpp"

#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::milk {

namespace detail {

/// Numbered lines of one code family, keyed by their index (1-based in the file)
using CodeLines = std::map<int, std::string>;

/// Collected wave data before assembly
struct WaveAcc
{
    std::vector<KeyValue> params;
    CodeLines init;
    CodeLines frame;
    CodeLines point;

    [[nodiscard]] bool used() const
    {
        return !params.empty() || !init.empty() || !frame.empty() || !point.empty();
    }
};

/// Collected shape data before assembly
struct ShapeAcc
{
    std::vector<KeyValue> params;
    CodeLines init;
    CodeLines frame;

    [[nodiscard]] bool used() const
    {
        return !params.empty() || !init.empty() || !frame.empty();
    }
};

/// Collected sprite data before assembly
struct SpriteAcc
{
    std::vector<KeyValue> params;
    CodeLines code;
};

inline void warn(ParseResult& result, const std::string& text)
{
    result.warnings.push_back(text);
}

/// @brief key == prefix + digits? (at least one digit, nothing after)
[[nodiscard]] inline bool matchIndexSuffix(std::string_view key, std::string_view prefix,
                                           int& indexOut)
{
    if (key.size() <= prefix.size() || key.substr(0, prefix.size()) != prefix) return false;
    const std::string_view digits = key.substr(prefix.size());
    int value = 0;
    for (const char c : digits)
    {
        if (c < '0' || c > '9') return false;
        value = value * 10 + (c - '0');
        if (value > 1'000'000) return false; // structural sanity, not a format limit
    }
    indexOut = value;
    return true;
}

/// @brief key == prefix + digits + '_' + rest? (wavecode_0_r, wave_0_per_frame1)
[[nodiscard]] inline bool matchMidIndex(std::string_view key, std::string_view prefix,
                                        int& indexOut, std::string_view& restOut)
{
    if (key.size() <= prefix.size() || key.substr(0, prefix.size()) != prefix) return false;
    std::size_t i = prefix.size();
    int value = 0;
    bool anyDigit = false;
    while (i < key.size() && key[i] >= '0' && key[i] <= '9')
    {
        value = value * 10 + (key[i] - '0');
        if (value > 1'000'000) return false;
        anyDigit = true;
        ++i;
    }
    if (!anyDigit || i >= key.size() || key[i] != '_') return false;
    indexOut = value;
    restOut = key.substr(i + 1);
    return true;
}

/// @brief Store one numbered code line; duplicates keep the first and warn
inline void storeCodeLine(CodeLines& lines, int index, std::string_view value,
                          std::string_view key, ParseResult& result)
{
    const auto [it, inserted] = lines.emplace(index, std::string(value));
    if (!inserted)
    {
        warn(result, "doppelte Code-Zeile '" + std::string(key) + "' — erste gewinnt");
    }
}

/**
 * @brief Concatenate a code family from index 1, stopping at the first gap
 *
 * Backtick lines (shader style) keep a line break, everything else is joined
 * verbatim — expressions split across per_frame lines stay intact. Orphan
 * lines after a gap and oversized blocks land in the report.
 */
[[nodiscard]] inline std::string assembleCode(const CodeLines& lines,
                                              std::string_view familyName,
                                              ParseResult& result)
{
    std::string out;
    int next = 1;
    for (auto it = lines.find(next); it != lines.end(); it = lines.find(++next))
    {
        std::string_view value = it->second;
        const bool backtick = !value.empty() && value.front() == '`';
        if (backtick) value.remove_prefix(1);
        if (!backtick)
        {
            // Original-Kommentarregel (StripLinefeedCharsAndComments,
            // state.cpp:1525): '//' UND '\\' kommentieren bis zum ORIGINAL-
            // Zeilenende. Da ohne Umbruch konkateniert wird, MUSS das vor dem
            // Join passieren — sonst frisst der Kommentar den restlichen Code.
            const std::size_t cut = std::min(value.find("//"), value.find("\\\\"));
            if (cut != std::string_view::npos) value = value.substr(0, cut);
        }
        if (out.size() + value.size() + 1 > kMaxCodeSize)
        {
            warn(result, std::string(familyName) + ": Code-Block überschreitet " +
                             std::to_string(kMaxCodeSize) + " Bytes — abgeschnitten");
            break;
        }
        out += value;
        if (backtick) out += '\n';
    }
    const int consumed = next - 1;
    if (consumed < static_cast<int>(lines.size()))
    {
        warn(result, std::string(familyName) + ": Lücke bei Index " + std::to_string(next) +
                         " — " + std::to_string(lines.size() - consumed) +
                         " weitere Zeile(n) ignoriert (Original bricht bei Lücken ab)");
    }
    return out;
}

} // namespace detail

/**
 * @brief Parse .milk preset text into a structured result (never throws)
 */
[[nodiscard]] inline ParseResult parse(std::string_view text)
{
    using namespace detail;

    ParseResult result;

    CodeLines perFrameInit;
    CodeLines perFrame;
    CodeLines perPixel;
    CodeLines warpCode;
    CodeLines compCode;
    WaveAcc waves[kMaxWaves];
    ShapeAcc shapes[kMaxShapes];
    std::map<int, SpriteAcc> sprites;
    bool waveIndexWarned = false;
    bool shapeIndexWarned = false;
    int linesWithoutEquals = 0;
    std::string firstLineWithoutEquals;

    enum class Mode
    {
        Body,          ///< header lines + [presetNN] content
        Sprite,        ///< inside [SPRITEn_BEGIN] .. [SPRITEn_END]
        SkipSection    ///< unknown section — content ignored
    };
    Mode mode = Mode::Body;
    int spriteIndex = 0;

    // --- line loop --------------------------------------------------------------------
    std::size_t pos = 0;
    bool firstLine = true;
    while (pos <= text.size())
    {
        const std::size_t nl = text.find('\n', pos);
        std::string_view line = (nl == std::string_view::npos)
                                    ? text.substr(pos)
                                    : text.substr(pos, nl - pos);
        pos = (nl == std::string_view::npos) ? text.size() + 1 : nl + 1;

        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        if (firstLine)
        {
            firstLine = false;
            if (line.size() >= 3 && static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF)
            {
                line.remove_prefix(3); // UTF-8 BOM
            }
        }
        if (line.empty()) continue;

        // --- section header -----------------------------------------------------------
        if (line.front() == '[')
        {
            const std::size_t close = line.find(']');
            if (close == std::string_view::npos)
            {
                warn(result, "Abschnittszeile ohne ']': '" + std::string(line) + "'");
                continue;
            }
            std::string name(line.substr(1, close - 1));
            for (char& c : name) c = asciiLower(c);

            int idx = 0;
            std::string_view rest;
            if (name.rfind("preset", 0) == 0)
            {
                result.hadPresetSection = true;
                mode = Mode::Body;
            }
            else if (matchIndexSuffix(name, "sprite", idx) ||
                     (matchMidIndex(name, "sprite", idx, rest) && rest == "begin"))
            {
                mode = Mode::Sprite;
                spriteIndex = idx;
                sprites.try_emplace(idx); // ensure the sprite exists even if empty
            }
            else if (matchMidIndex(name, "sprite", idx, rest) && rest == "end")
            {
                mode = Mode::Body;
            }
            else
            {
                warn(result, "unbekannter Abschnitt [" + name + "] — Inhalt ignoriert");
                mode = Mode::SkipSection;
            }
            continue;
        }

        if (mode == Mode::SkipSection) continue;

        // --- comment-only lines (hand-written presets) ---------------------------------
        {
            std::string_view t = line;
            while (!t.empty() && (t.front() == ' ' || t.front() == '\t')) t.remove_prefix(1);
            if (t.size() >= 2 && t[0] == '/' && t[1] == '/') continue;
        }

        // --- key=value -----------------------------------------------------------------
        const std::size_t eq = line.find('=');
        if (eq == std::string_view::npos)
        {
            ++linesWithoutEquals;
            if (firstLineWithoutEquals.empty()) firstLineWithoutEquals = std::string(line);
            continue;
        }
        const std::string_view rawKey = line.substr(0, eq);
        const std::string_view value = line.substr(eq + 1);

        std::string key(rawKey);
        for (char& c : key) c = asciiLower(c);

        if (mode == Mode::Sprite)
        {
            SpriteAcc& sprite = sprites[spriteIndex];
            int n = 0;
            if (matchIndexSuffix(key, "code_", n))
            {
                storeCodeLine(sprite.code, n, value, rawKey, result);
            }
            else
            {
                sprite.params.push_back({std::string(rawKey), std::string(value)});
            }
            continue;
        }

        // Body: code families first (order matters — per_frame_init_ before per_frame_)
        int n = 0;
        int idx = 0;
        std::string_view rest;
        if (matchIndexSuffix(key, "per_frame_init_", n))
        {
            storeCodeLine(perFrameInit, n, value, rawKey, result);
        }
        else if (matchIndexSuffix(key, "per_frame_", n))
        {
            storeCodeLine(perFrame, n, value, rawKey, result);
        }
        else if (matchIndexSuffix(key, "per_pixel_", n))
        {
            storeCodeLine(perPixel, n, value, rawKey, result);
        }
        else if (matchIndexSuffix(key, "warp_", n))
        {
            storeCodeLine(warpCode, n, value, rawKey, result);
        }
        else if (matchIndexSuffix(key, "comp_", n))
        {
            storeCodeLine(compCode, n, value, rawKey, result);
        }
        else if (matchMidIndex(key, "wavecode_", idx, rest))
        {
            if (idx >= kMaxWaves)
            {
                if (!waveIndexWarned)
                {
                    warn(result, "Wave-Index " + std::to_string(idx) + " über Superset-Cap " +
                                     std::to_string(kMaxWaves) + " — ignoriert");
                    waveIndexWarned = true;
                }
            }
            else
            {
                waves[idx].params.push_back({std::string(rest), std::string(value)});
            }
        }
        else if (matchMidIndex(key, "shapecode_", idx, rest))
        {
            if (idx >= kMaxShapes)
            {
                if (!shapeIndexWarned)
                {
                    warn(result, "Shape-Index " + std::to_string(idx) + " über Superset-Cap " +
                                     std::to_string(kMaxShapes) + " — ignoriert");
                    shapeIndexWarned = true;
                }
            }
            else
            {
                shapes[idx].params.push_back({std::string(rest), std::string(value)});
            }
        }
        else if (matchMidIndex(key, "wave_", idx, rest) && idx < kMaxWaves &&
                 (matchIndexSuffix(rest, "init", n) || matchIndexSuffix(rest, "per_frame", n) ||
                  matchIndexSuffix(rest, "per_point", n)))
        {
            CodeLines& target = rest.starts_with("init")        ? waves[idx].init
                                : rest.starts_with("per_frame") ? waves[idx].frame
                                                                : waves[idx].point;
            storeCodeLine(target, n, value, rawKey, result);
        }
        else if (matchMidIndex(key, "shape_", idx, rest) && idx < kMaxShapes &&
                 (matchIndexSuffix(rest, "init", n) || matchIndexSuffix(rest, "per_frame", n)))
        {
            CodeLines& target = rest.starts_with("init") ? shapes[idx].init : shapes[idx].frame;
            storeCodeLine(target, n, value, rawKey, result);
        }
        else
        {
            result.params.push_back({std::string(rawKey), std::string(value)});
        }
    }

    if (linesWithoutEquals > 0)
    {
        warn(result, std::to_string(linesWithoutEquals) + " Zeile(n) ohne '=' ignoriert (erste: '" +
                         firstLineWithoutEquals + "')");
    }

    // --- assembly -----------------------------------------------------------------------
    result.perFrameInitCode = assembleCode(perFrameInit, "per_frame_init", result);
    result.perFrameCode = assembleCode(perFrame, "per_frame", result);
    result.perPixelCode = assembleCode(perPixel, "per_pixel", result);
    result.warpShader = assembleCode(warpCode, "warp", result);
    result.compShader = assembleCode(compCode, "comp", result);

    for (int i = 0; i < kMaxWaves; ++i)
    {
        if (!waves[i].used()) continue;
        CustomWave w;
        w.index = i;
        w.params = std::move(waves[i].params);
        const std::string family = "wave_" + std::to_string(i);
        w.initCode = detail::assembleCode(waves[i].init, family + "_init", result);
        w.frameCode = detail::assembleCode(waves[i].frame, family + "_per_frame", result);
        w.pointCode = detail::assembleCode(waves[i].point, family + "_per_point", result);
        result.waves.push_back(std::move(w));
    }
    for (int i = 0; i < kMaxShapes; ++i)
    {
        if (!shapes[i].used()) continue;
        CustomShape s;
        s.index = i;
        s.params = std::move(shapes[i].params);
        const std::string family = "shape_" + std::to_string(i);
        s.initCode = detail::assembleCode(shapes[i].init, family + "_init", result);
        s.frameCode = detail::assembleCode(shapes[i].frame, family + "_per_frame", result);
        result.shapes.push_back(std::move(s));
    }
    for (auto& [idx, acc] : sprites)
    {
        Sprite s;
        s.index = idx;
        s.params = std::move(acc.params);
        s.code = detail::assembleCode(acc.code, "sprite" + std::to_string(idx) + "_code", result);
        result.sprites.push_back(std::move(s));
    }

    // --- version headers + verdict --------------------------------------------------------
    result.presetVersion = result.valueInt("MILKDROP_PRESET_VERSION", 0);
    result.psVersion = result.valueInt("PSVERSION", -1);
    result.psVersionWarp = result.valueInt("PSVERSION_WARP", -1);
    result.psVersionComp = result.valueInt("PSVERSION_COMP", -1);

    const bool anyCode = !result.perFrameInitCode.empty() || !result.perFrameCode.empty() ||
                         !result.perPixelCode.empty() || !result.warpShader.empty() ||
                         !result.compShader.empty() || !result.waves.empty() ||
                         !result.shapes.empty() || !result.sprites.empty();
    if (result.params.empty() && !anyCode)
    {
        result.ok = false;
        result.error = "keine MilkDrop-Preset-Daten erkennbar (keine key=value-Zeilen)";
        return result;
    }
    if (!result.hadPresetSection)
    {
        warn(result, "kein [preset00]-Abschnitt — Datei als Preset-Rumpf interpretiert");
    }
    result.ok = true;
    return result;
}

/**
 * @brief Parse .milk preset text from a raw buffer
 */
[[nodiscard]] inline ParseResult parse(const char* data, std::size_t size)
{
    return parse(std::string_view(data, size));
}

/**
 * @brief Read and parse a .milk file (never throws; read failure -> ok=false)
 */
[[nodiscard]] inline ParseResult parseFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        ParseResult result;
        result.error = "Datei nicht lesbar: " + path.string();
        return result;
    }
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return parse(text);
}

} // namespace lumi::milk
