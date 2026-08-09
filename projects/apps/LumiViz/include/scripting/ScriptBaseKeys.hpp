/**
 ****************************************************************************************
 * @file   ScriptBaseKeys.hpp
 * @brief  SSOT der reservierten Namen des einheitlichen Lumi-Skript-Sets, mit
 *         Herkunft — Grundlage der Import-Kollisionsregel (Entscheid D2)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * `Vereinheitlichung_Konzept.md` §4: "Reserviert" wirkt **relativ zum
 * Import-Format**. Ein Name, der im Quellformat selbst Builtin-Bedeutung hat,
 * wird nie umbenannt — er bindet an das Builtin, das IST die Original-Semantik.
 * Umbenannt wird nur, was im Quellformat KEINE Builtin-Bedeutung hat und mit
 * dem Lumi-Set kollidiert.
 *
 * Deshalb traegt jeder Eintrag ein Herkunfts-Feld. Fuer den .avs-Import sind
 * genau die Nicht-`Avs`-Eintraege Umbenennungs-Kandidaten.
 *
 * Belegt an der AVS-Quelle (`ref/vis_avs`, `registerVar`): KEIN AVS-Effekt
 * registriert `bass`, `mid`, `treb`, `treble`, `vol`, `time` oder `dt` — dort
 * sind das gewoehnliche Preset-Variablen. `beat` registriert allein r_list
 * (r_list.cpp:378); die Effect-List-Codes laufen im Translator ueber einen
 * eigenen Pfad und werden deshalb nicht umbenannt.
 *
 * Anlass (S51): "Alien Alloy" fuehrt in `vol` einen Tiefpass
 * (`vol=vol*0.9+getspec(0.5,1,0)`), der Swirl-Staerke und Zeitschritt der
 * Dynamic Movement treibt. Weil die Injektions-Schicht `vol` je Frame setzt,
 * konnte der Akkumulator nie wachsen: der Inhalt wanderte nur halb so weit ins
 * Bild (34184 gegen 59043 gezeichnete Pixel).
 *
 * Umfang: dieser Header beginnt bei den VARIABLEN, weil dort die Kollisionen
 * messbar aufgetreten sind. Funktions- und Konstanten-Kategorien kommen mit dem
 * Vereinheitlichungs-Paket dazu (Konzept §4, Verbraucher: beide Importer, die
 * Injektions-Schicht, der Skript-Editor, Tests).
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <cctype>
#include <string>
#include <string_view>

namespace lumi::scripting {

/// Woher ein reservierter Name stammt — entscheidet die Kollisionsregel.
enum class KeyOrigin
{
    Avs,   ///< Builtin in AVS (r_*.cpp registerVar)
    Milk,  ///< Builtin in MilkDrop
    Lumi   ///< LumiViz-Erweiterung, in keinem der beiden Formate Builtin
};

struct BaseKey
{
    std::string_view name;
    KeyOrigin origin;
};

/**
 * @brief Die Namen, die die Injektions-Schicht auf JEDEM Skript-Modul setzt.
 *
 * `beat` steht als `Avs` drin, weil r_list es registriert — die
 * Effect-List-Semantik ist damit die Original-Semantik und darf nicht
 * umbenannt werden.
 */
inline constexpr std::array<BaseKey, 8> kInjectedKeys = {{
    {"bass", KeyOrigin::Milk},
    {"mid", KeyOrigin::Milk},
    {"treb", KeyOrigin::Milk},
    {"treble", KeyOrigin::Milk},
    {"vol", KeyOrigin::Milk},
    {"beat", KeyOrigin::Avs},
    {"time", KeyOrigin::Milk},
    {"dt", KeyOrigin::Lumi},
}};

/// @brief EEL ist case-insensitiv — Vergleiche laufen ueber ASCII-Kleinschreibung.
[[nodiscard]] inline bool equalsIgnoreCase(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        const auto ca = static_cast<unsigned char>(a[i]);
        const auto cb = static_cast<unsigned char>(b[i]);
        if (std::tolower(ca) != std::tolower(cb)) return false;
    }
    return true;
}

/**
 * @brief Ist @p name beim .avs-Import ein Umbenennungs-Kandidat?
 *
 * Wahr fuer reservierte Namen, die in AVS keine Builtin-Bedeutung haben. Damit
 * bleibt der Read-before-write-Fall exakt: eine umbenannte, nie geschriebene
 * Variable startet weiter uninitialisiert (= 0) statt den Injektionswert zu
 * liefern (Konzept §4, "semantik-erhaltend").
 */
[[nodiscard]] inline bool collidesOnAvsImport(std::string_view name)
{
    for (const BaseKey& key : kInjectedKeys)
    {
        if (key.origin != KeyOrigin::Avs && equalsIgnoreCase(name, key.name))
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Die Registry-Schreibweise eines reservierten Namens (sonst @p name).
 *
 * Umbenennungs-Ziele MUESSEN kanonisch sein: EEL ist case-insensitiv, das
 * transpilierte Lua nicht. Wuerde ein Preset `VOL` in einem und `vol` in einem
 * anderen Slot schreiben, ergaeben `VOL_p` und `vol_p` zwei verschiedene
 * Lua-Variablen — der Zustand waere wieder zerteilt.
 */
[[nodiscard]] inline std::string_view canonicalKey(std::string_view name)
{
    for (const BaseKey& key : kInjectedKeys)
    {
        if (equalsIgnoreCase(name, key.name)) return key.name;
    }
    return name;
}

/// @brief Umbenennungs-Schema D2: `name` → `name_p`, bei erneuter Kollision
///        `name_p2`, `name_p3`, … (@p attempt 0 = `_p`). Immer kanonisch.
[[nodiscard]] inline std::string privateName(std::string_view name, int attempt)
{
    std::string out(canonicalKey(name));
    out += "_p";
    if (attempt > 0) out += std::to_string(attempt + 1);
    return out;
}

}  // namespace lumi::scripting
