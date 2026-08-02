/**
 ****************************************************************************************
 * @file   MilkdropSerializer.hpp
 * @brief  JSON persistence for translated MilkDrop presets (Import Roadmap 6, M6)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.1.0
 *
 * @details
 * The .lvfx sister format (MilkDrop_Import_Konzept §2.3): the TRANSLATED
 * PresetState round-trips as JSON, following the ChainSerializer pattern
 * (GL-free, Qt JSON only — unit-testable). Same .lvfx extension as the
 * MultiEffect chains; the document header carries `"type": "milkdrop"` so the
 * loader can dispatch (isMilkdropFile peeks without a full parse).
 *
 * Layout: `{ "header": {formatVersion, generator, type}, "preset": {...} }`.
 * Code slots stay EEL source text, shaders stay raw HLSL — the shader
 * classification (ShaderInfo) is NOT persisted but re-derived on load
 * (single source of truth is the shader text). Missing fields fall back to
 * the CState defaults already encoded in PresetState — an older document
 * loads cleanly on a newer build (AVS philosophy, never fail hard).
 *
 * 1.1.0 (Strang R, S65): Regelwerk-Felder (regelwerk, vier Einzelschalter,
 * psWarp/psComp) als optionale Strings/Bools — fehlend ⇒ Legacy + Auto,
 * damit laden alle Bestands-Dokumente unverändert.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/milkdrop/MilkdropPresetState.hpp"

#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace lumi::milkdrop {

/** Whole document: `{header{type:"milkdrop"}, preset}`. */
[[nodiscard]] QJsonObject presetToJson(const PresetState& state);

/** Parse a whole document (missing fields keep the CState defaults). */
[[nodiscard]] PresetState presetFromJson(const QJsonObject& doc, QStringList* report);

/** Write the preset to a .lvfx file (UTF-8 JSON). */
[[nodiscard]] bool savePresetToFile(const PresetState& state, const QString& path);

/** Load a preset document; false on I/O or JSON failure (report says why). */
[[nodiscard]] bool loadPresetFromFile(const QString& path, PresetState& outState,
                                      QStringList* report);

/** Cheap peek: does this .lvfx carry a milkdrop document (header.type)? */
[[nodiscard]] bool isMilkdropFile(const QString& path);

} // namespace lumi::milkdrop
