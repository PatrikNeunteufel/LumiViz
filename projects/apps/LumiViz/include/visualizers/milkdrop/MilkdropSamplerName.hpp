/**
 ****************************************************************************************
 * @file   MilkdropSamplerName.hpp
 * @brief  Zerlegung eines Milkdrop-Sampler-Bezeichners in Datei-Basisname und
 *         Filter/Wrap-Zustand (S52 — Nachbau von `plugin.cpp:2955`)
 *
 * @author LumiPulse Team
 * @date   Juli 2026
 * @version 1.0.0
 *
 * @details
 * SSOT fuer drei Stellen, die frueher jede ihre eigene Regel hatten: das Laden
 * der Texturdateien, die Frage "deklariert die Praeambel den schon?" und die
 * Wahl des Sampler-Objekts beim Binden.
 *
 * Die Regel steht im Quelltext der Referenz
 * (`ref/winamp_orig/…/vis_milk2/plugin.cpp:2955`) und weicht in drei Punkten von
 * dem ab, was wir vorher gemacht haben — jeder Punkt hat Presets gekostet:
 *
 * 1. **`sampler_` wird nur abgeschnitten, wenn es da steht.** Ein unbedingtes
 *    `substr(8)` machte aus `sampler MilkDrop3_001` den Namen `3_001` (Datei
 *    nicht gefunden) und warf bei `sampler tex` — drei Zeichen, in 25 Presets
 *    des Packs — `std::out_of_range`.
 * 2. **Das Filter/Wrap-Praefix ist case-insensitiv** (`FW_` gilt wie `fw_`).
 * 3. **Die umgedrehten Formen `WF_ CF_ WP_ CP_` gelten auch.**
 *
 * Erkannt wird das Praefix an `name[2] == '_'` bei Laenge > 3 — genau zwei
 * Buchstaben, sonst nichts. Ein nicht passendes Zweierpaar bleibt Teil des
 * Namens (`ab_bild` sucht die Datei `ab_bild`).
 ****************************************************************************************
 */

#pragma once

#include <string>

namespace lumi::milkdrop {

/// Zerlegter Sampler-Bezeichner
struct SamplerName
{
    std::string root;      ///< Basisname fuer die Dateisuche
    bool bilinear = true;  ///< false = Punktabtastung (`P?_`)
    bool wrap = true;      ///< false = Klemmen (`?C_`)
};

/// @brief Bezeichner zerlegen; siehe Dateikopf fuer die Regel.
[[nodiscard]] inline SamplerName parseSamplerName(const std::string& name)
{
    SamplerName out;
    out.root = (name.rfind("sampler_", 0) == 0) ? name.substr(8) : name;

    if (out.root.size() > 3 && out.root[2] == '_')
    {
        const auto up = [](char c) {
            return static_cast<char>((c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c);
        };
        const char a = up(out.root[0]);
        const char b = up(out.root[1]);
        bool matched = true;
        if ((a == 'F' && b == 'W') || (a == 'W' && b == 'F'))
        {
            out.bilinear = true;
            out.wrap = true;
        }
        else if ((a == 'F' && b == 'C') || (a == 'C' && b == 'F'))
        {
            out.bilinear = true;
            out.wrap = false;
        }
        else if ((a == 'P' && b == 'W') || (a == 'W' && b == 'P'))
        {
            out.bilinear = false;
            out.wrap = true;
        }
        else if ((a == 'P' && b == 'C') || (a == 'C' && b == 'P'))
        {
            out.bilinear = false;
            out.wrap = false;
        }
        else
        {
            matched = false;
        }
        if (matched) out.root = out.root.substr(3);
    }
    return out;
}

}  // namespace lumi::milkdrop
