/**
 ****************************************************************************************
 * @file   ChainSerializer.hpp
 * @brief  JSON persistence for the multi-effect chain (Import Roadmap 5.6)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 *
 * @details
 * Serializes a `lumi::multieffect::ChainNode` tree to a nested JSON document and
 * back (the host's own preset format, decision E1 — the flat .lvp schema cannot
 * hold a variable effect tree). GL-free (Qt JSON only), so it round-trips in a
 * unit test.
 *
 * Layout: `{ "header": {formatVersion, generator}, "root": <node> }`, where each
 * node is `{ "type": "<key>", "enabled": bool, "name": "...", <params...>,
 * "children": [<node>...] }`. Unknown type keys deserialize to a Passthrough so
 * a newer preset never fails to load on an older build (AVS philosophy).
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace lumi::multieffect {

/** Stable type key for a node's effect (persistence identity, not display). */
[[nodiscard]] QString effectTypeKey(const EffectParams& params);

/** Serialize one node (recursive). */
[[nodiscard]] QJsonObject nodeToJson(const ChainNode& node);

/** Deserialize one node (recursive). Unknown types → Passthrough. */
[[nodiscard]] ChainNode nodeFromJson(const QJsonObject& obj, QStringList* report);

/** Whole-chain document: `{header, root}`. */
[[nodiscard]] QJsonObject chainToJson(const ChainNode& root);

/** Parse a whole-chain document; returns a compiled tree. */
[[nodiscard]] ChainNode chainFromJson(const QJsonObject& doc, QStringList* report);

/** Write the chain to a .lvfx file (UTF-8 JSON). */
[[nodiscard]] bool saveChainToFile(const ChainNode& root, const QString& path);

/** Load a chain from a .lvfx file; returns a compiled tree, ok=false on I/O fail. */
[[nodiscard]] bool loadChainFromFile(const QString& path, ChainNode& outRoot,
                                     QStringList* report);

} // namespace lumi::multieffect
