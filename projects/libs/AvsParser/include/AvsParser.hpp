/**
 ****************************************************************************************
 * @file   AvsParser.hpp
 * @brief  Public API: parse a "Nullsoft AVS Preset 0.1/0.2" file into an effect tree
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Header-only library (no Qt, no app dependencies). Import-phase Roadmap 3:
 * recursive TLV container + core-set blob decoders + import-report scaffolding.
 * The result carries EEL source text; transpilation (EelTranspiler) and the
 * translation into LumiViz presets are separate, later steps.
 *
 * Error philosophy (Import-Analyse §4.3): only a missing/invalid signature makes
 * ok=false. Any structural damage after that truncates parsing at the damaged
 * spot and is reported as a warning — parsing never throws, never hard-fails.
 *
 * Format reference: ref/vis_avs/avs/vis_avs/r_list.cpp (BSD-3, Nullsoft 2005);
 * layout notes in projects/apps/MyViz/docs/visuals/Import_Analyse_AVS_MilkDrop.md §5.3.
 ****************************************************************************************
 */

#pragma once

#include "AvsParserEffects.hpp"
#include "AvsParserReader.hpp"
#include "AvsParserTypes.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::avs {

namespace detail {

/// Signature: 22-char prefix + version digit + 0x1a  (24 bytes total)
inline constexpr std::string_view kSignaturePrefix = "Nullsoft AVS Preset 0.";
inline constexpr std::size_t kSignatureSize = 24;

/// Recursion cap for nested effect lists (structural sanity, not a format limit)
inline constexpr int kMaxListDepth = 100;

inline void warn(ParseResult& result, const std::string& path, const std::string& text)
{
    result.warnings.push_back(path + ": " + text);
}

/**
 * @brief Parse one effect-list config blob (root preset body or nested list)
 *
 * Mirrors C_RenderListClass::load_config: mode byte (0x80 -> full int32 mode),
 * optional extended data block, then child entries [id][len][blob] until the
 * data runs out. The list's EEL code travels as a pseudo entry with APE ID
 * "AVS 2.8+ Effect List Config" (only present when extended data exists).
 */
inline void parseListBody(Reader& r, EffectNode& node, ParseResult& result,
                          const std::string& path, int depth)
{
    node.isList = true;

    // --- mode byte, optionally extended to full 32 bit -------------------------------
    std::int32_t mode = 0;
    if (!r.atEnd())
    {
        mode = r.peekByte();
        r.skip(1);
        if ((mode & 0x80) != 0)
        {
            mode &= ~0x80;
            mode |= r.i32();
        }
    }
    node.list.mode = mode;

    // --- extended data (size travels in mode bits 24-31) -----------------------------
    const std::size_t ext =
        ((static_cast<std::uint32_t>(mode) >> 24) & 0xFF) + 5;
    if (ext > 5)
    {
        if (r.pos() < ext) node.list.inBlendVal = r.i32();
        if (r.pos() < ext) node.list.outBlendVal = r.i32();
        if (r.pos() < ext) node.list.bufferIn = r.i32();
        if (r.pos() < ext) node.list.bufferOut = r.i32();
        if (r.pos() < ext) node.list.inInvert = r.i32();
        if (r.pos() < ext) node.list.outInvert = r.i32();
        if (r.pos() < ext - 4) node.list.beatRender = r.i32();
        if (r.pos() < ext - 4) node.list.beatRenderFrames = r.i32();
    }

    // --- child entries: [int32 id][32-byte APE id?][int32 len][blob] -----------------
    while (!r.atEnd())
    {
        if (r.remaining() < 4)
        {
            warn(result, path, "abgeschnittene Daten am Listenende (" +
                                   std::to_string(r.remaining()) + " Restbytes)");
            break;
        }
        const std::int32_t id = r.i32();

        std::string apeId;
        if (id >= kApeIdBase)
        {
            const std::uint8_t* idBlock = nullptr;
            if (!r.tryBytes(32, idBlock))
            {
                warn(result, path, "abgeschnittener APE-ID-Block");
                break;
            }
            apeId = r.fixedString(idBlock, 32);
        }

        std::int32_t blobLen = 0;
        if (!r.tryI32(blobLen))
        {
            warn(result, path, "abgeschnittener Effekt-Eintrag (Laenge fehlt)");
            break;
        }
        if (blobLen < 0 || r.remaining() < static_cast<std::size_t>(blobLen))
        {
            warn(result, path,
                 "abgeschnittener Config-Blob (deklariert " + std::to_string(blobLen) +
                     ", vorhanden " + std::to_string(r.remaining()) + ")");
            break;
        }
        const std::uint8_t* blob = nullptr;
        r.tryBytes(static_cast<std::size_t>(blobLen), blob);

        // list code pseudo entry (belongs to THIS list, not a child)
        if (ext > 5 && id >= kApeIdBase && apeId == kListCodeApeId)
        {
            Reader code(blob, static_cast<std::size_t>(blobLen));
            node.list.useCode = code.i32();
            node.list.initCode = code.loadString();
            node.list.frameCode = code.loadString();
            continue;
        }

        EffectNode child;
        child.id = id;
        child.apeId = apeId;
        child.rawConfig.assign(blob, blob + blobLen);
        const std::string childPath =
            path + "/" + std::to_string(node.children.size());

        Reader config(blob, static_cast<std::size_t>(blobLen));
        if (id == kListId)
        {
            child.name = "Effect List";
            if (depth >= kMaxListDepth)
            {
                warn(result, childPath,
                     "Verschachtelungstiefe ueberschritten — Liste als Roh-Blob "
                     "konserviert");
            }
            else
            {
                parseListBody(config, child, result,
                              childPath + " " + child.name, depth + 1);
                child.decoded = true;
            }
        }
        else if (id >= 0 && id < kBuiltinCount)
        {
            child.name = kBuiltinNames[static_cast<std::size_t>(id)];
            decodeBuiltin(id, config, child);
        }
        else if (id >= kApeIdBase)
        {
            const std::int32_t aliasIndex = apeAliasToBuiltin(apeId);
            if (aliasIndex >= 0)
            {
                child.name = kBuiltinNames[static_cast<std::size_t>(aliasIndex)];
                decodeBuiltin(aliasIndex, config, child);
            }
            else if (isBuiltinApe(apeId))
            {
                child.name = apeId;
                decodeApe(apeId, config, child);   // core-set APE fields (else raw blob)
            }
            else
            {
                child.name = apeId;
                warn(result, childPath + " " + child.name,
                     "unbekannter APE-Effekt — Roh-Blob konserviert");
            }
        }
        else
        {
            warn(result, childPath,
                 "unbekannte Effekt-ID " + std::to_string(id) +
                     " — Roh-Blob konserviert");
        }

        node.children.push_back(std::move(child));
    }
}

} // namespace detail

/**
 * @brief Parse a .avs preset from memory
 * @param data  file bytes
 * @param size  byte count
 */
inline ParseResult parse(const std::uint8_t* data, std::size_t size)
{
    ParseResult result;

    // Signature check mirrors C_RenderListClass::__LoadPreset: prefix,
    // version digit '1'..'2', 0x1a terminator; anything else is not an .avs.
    // (More lenient than the original minimum length: signature alone parses
    // as an empty preset — import tolerance, Import-Analyse §4.3.)
    if (data == nullptr || size < detail::kSignatureSize)
    {
        result.error = "Datei zu kurz fuer ein AVS-Preset";
        return result;
    }
    if (std::string_view(reinterpret_cast<const char*>(data),
                         detail::kSignaturePrefix.size()) != detail::kSignaturePrefix ||
        data[22] < '1' || data[22] > '2' || data[23] != 0x1a)
    {
        result.error = "keine gueltige 'Nullsoft AVS Preset 0.1/0.2'-Signatur";
        return result;
    }

    result.formatVersion = data[22] - '0';
    result.root.isList = true;
    result.root.id = kListId;
    result.root.name = "Main";

    detail::Reader body(data + detail::kSignatureSize,
                        size - detail::kSignatureSize);
    detail::parseListBody(body, result.root, result, "Main", 0);
    result.ok = true;
    return result;
}

/// @brief Parse a .avs preset from a byte vector
inline ParseResult parse(const std::vector<std::uint8_t>& bytes)
{
    return parse(bytes.data(), bytes.size());
}

/**
 * @brief Parse a .avs preset from a file
 *
 * IO errors yield ok=false with a message; parsing itself follows parse().
 */
inline ParseResult parseFile(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        ParseResult result;
        result.error = "Datei nicht lesbar: " + path.string();
        return result;
    }
    std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(in),
                                    std::istreambuf_iterator<char>()};
    return parse(bytes);
}

} // namespace lumi::avs
