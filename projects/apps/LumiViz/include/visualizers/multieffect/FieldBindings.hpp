/**
 ****************************************************************************************
 * @file   FieldBindings.hpp
 * @brief  Feldtabelle: jedes skalare Knotenfeld mit Skriptname, Art, Bereich, Klasse
 *
 * @details
 * Block F, Stufe F1 (docs/visuals/Feldsteuerung_Konzept.md): die EINE
 * Adressierung der skalaren Felder (Zahl, Schalter, Aufzaehlung, Farbe) aller
 * `…Params`-Structs — Grundlage fuer den generischen Skript-Laeufer (F2), die
 * Modulation ohne Formel (F4), Referenz/Highlighter (F5) und spaeter MIDI/OSC.
 *
 * Die Tabelle wird **erzeugt** (`FieldBindings.cpp`), nicht von Hand gepflegt:
 *
 * @code
 * python asset/calibration/fields/harvest_field_docs.py
 * @endcode
 *
 * Quelle sind Inventar (welche Felder), Panel (Art, Bereich) und eine kleine
 * Klassen-/Alias-Liste im Ernter. Jeder Eintrag greift ueber
 * `std::get<XParams>(p).feld` zu — ein umbenanntes oder geloeschtes Feld ist
 * damit ein **Compilerfehler**, kein stilles Loch.
 *
 * Namen (Entscheid E2a): `name` = JSON-Feldname klein (wie in den
 * Preset-Dateien); Farben als `<name>_r/_g/_b` (0..1, der Laeufer zerlegt).
 * Bisherige abweichende Skriptnamen (`cx`, `rot`, `n`, `life` …) bleiben als
 * `alias` gueltig — mit eigener Skala, wo der alte Name anders skaliert war
 * (`alphain` = `inAdjustAlpha / 255`).
 ****************************************************************************************
 */

#pragma once

#include "visualizers/multieffect/EffectChain.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>

namespace lumi::multieffect::feldtabelle
{

/// Art des Feldes — bestimmt die Typpflege beim Zurueckschreiben (F2).
enum class Art : std::uint8_t
{
    Ganzzahl,     ///< gerundet
    Kommazahl,
    Schalter,     ///< >= 0.5 = an
    Aufzaehlung,  ///< gerundet, geklemmt auf lo..hi
    Farbe         ///< 0x00RRGGBB; im Skript als <name>_r/_g/_b
};

/// Wie teuer eine Aenderung ist (Entscheid E3a, S84 um den Beat-Slot erweitert).
enum class Klasse : std::uint8_t
{
    Live,      ///< jeden Frame lesbar, keine Folgekosten
    Struktur,  ///< Aenderung baut neu auf (Puffer, Mesh, Raster) — nicht aus dem Frame-Slot
    Zustand    ///< Aenderung setzt Laufzeitzustand neu (Startwert) — billig, aber sichtbar
};

/// Wer das Parameter-Skript des Knotens heute faehrt.
enum class Traeger : std::uint8_t
{
    ParamSkript,  ///< `runParamScript` im Renderer (Strang D) — F2 uebernimmt
    EigenerHost,  ///< eigener Skript-Host (Fraktale, 3D …) — Stufe F2b
    Keiner        ///< kein Parameter-Skript
};

struct Feld
{
    std::string_view feld;   ///< JSON-Feldname (wie im Inventar/Preset)
    std::string_view name;   ///< Skriptname (klein)
    std::string_view alias;  ///< bisheriger abweichender Skriptname, "" = keiner
    double aliasSkala;       ///< Skriptwert unter `alias` = Feldwert * aliasSkala
    Art art;
    Klasse klasse;
    double lo;               ///< Bereich aus dem Panel; NaN = keiner bekannt
    double hi;
    double (*lesen)(const EffectParams&);
    void (*schreiben)(EffectParams&, double);
};

struct Typ
{
    std::string_view typKey;  ///< wie `effectTypeKey()`
    Traeger traeger;
    std::vector<Feld> felder;
};

/// Tabelle des Knotentyps mit Varianten-Index `index`; nullptr = kein Eintrag.
[[nodiscard]] const Typ* typ(std::size_t index);

/// Tabelle des Typs, den `p` gerade traegt; nullptr = kein Eintrag.
[[nodiscard]] inline const Typ* typ(const EffectParams& p) { return typ(p.index()); }

/// Alle Typen mit Eintraegen (fuer Tests, Referenz).
[[nodiscard]] const std::vector<const Typ*>& alle();

// --- Typpflege: Feldwert <-> double ------------------------------------------

template <typename F>
[[nodiscard]] double lese(const F& x)
{
    if constexpr (std::is_enum_v<F>)
        return static_cast<double>(static_cast<std::underlying_type_t<F>>(x));
    else
        return static_cast<double>(x);
}

template <typename F>
void setze(F& x, double v)
{
    if constexpr (std::is_same_v<F, bool>)
        x = v >= 0.5;
    else if constexpr (std::is_enum_v<F>)
        x = static_cast<F>(static_cast<std::underlying_type_t<F>>(std::llround(v)));
    else if constexpr (std::is_integral_v<F>)
        x = static_cast<F>(std::llround(v));
    else
        x = static_cast<F>(v);
}

}  // namespace lumi::multieffect::feldtabelle
