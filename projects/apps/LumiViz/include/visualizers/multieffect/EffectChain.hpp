/**
 ****************************************************************************************
 * @file   EffectChain.hpp
 * @brief  GL-free runtime data model of the multi-effect chain (Import Roadmap 5.1)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 *
 * @details
 * Runtime tree mirroring the topology of an AvsParser::EffectNode tree: nested
 * effect lists (containers) plus concrete effect leaves. The tree is editable
 * (decision E5); after every mutation the owner must run compileChain() — the
 * chain compile pass (validation now; Set-Render-Mode roll-out and blend
 * resolution join in later steps, decision E4).
 *
 * GL-free by contract: this header describes *what* to render, never touches
 * OpenGL. The MultiEffectVisualizer walks the tree on the render thread.
 *
 * Effect coverage grows with the 5.x steps (design doc, decision E2). 5.1 ships
 * the trivial leaves (Clear, Fadeout, Invert), a host-only DebugBars leaf for
 * sight-testing the chain, and Passthrough for conserved unknown effects.
 ****************************************************************************************
 */

#pragma once

#include "visualizers/milkdrop/MilkdropPresetState.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace lumi::multieffect {

// =============================================================================
// Blend modes (AVS r_list.cpp order == mode value)
// =============================================================================

/**
 * The 14 AVS list blend modes (in- and out-blend). Batch 1 (decision E3)
 * implements the common ones; the exotic rest falls back to Replace with a
 * compile warning until its batch lands.
 */
enum class BlendMode : int
{
    Ignore = 0,
    Replace = 1,
    FiftyFifty = 2,
    Maximum = 3,
    Additive = 4,
    Subtractive12 = 5,
    Subtractive21 = 6,
    EveryOtherLine = 7,
    EveryOtherPixel = 8,
    Xor = 9,
    Adjustable = 10,
    Multiply = 11,
    Buffer = 12,
    Minimum = 13,
};

/**
 * True for every mode the blend engine can render. Batch 1 (E3) covered the
 * arithmetic modes; batch 2 (Session 35) added the exotic ones (Subtractive,
 * Every-other-line/-pixel, XOR, Buffer), so the whole set is now implemented.
 */
[[nodiscard]] inline bool isBlendModeImplemented(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Ignore:
        case BlendMode::Replace:
        case BlendMode::FiftyFifty:
        case BlendMode::Maximum:
        case BlendMode::Additive:
        case BlendMode::Subtractive12:
        case BlendMode::Subtractive21:
        case BlendMode::EveryOtherLine:
        case BlendMode::EveryOtherPixel:
        case BlendMode::Xor:
        case BlendMode::Adjustable:
        case BlendMode::Multiply:
        case BlendMode::Buffer:
        case BlendMode::Minimum:
            return true;
    }
    return false;
}

[[nodiscard]] inline const char* blendModeName(BlendMode mode)
{
    switch (mode)
    {
        case BlendMode::Ignore: return "Ignore";
        case BlendMode::Replace: return "Replace";
        case BlendMode::FiftyFifty: return "50/50";
        case BlendMode::Maximum: return "Maximum";
        case BlendMode::Additive: return "Additive";
        case BlendMode::Subtractive12: return "Subtractive 1-2";
        case BlendMode::Subtractive21: return "Subtractive 2-1";
        case BlendMode::EveryOtherLine: return "Every other line";
        case BlendMode::EveryOtherPixel: return "Every other pixel";
        case BlendMode::Xor: return "XOR";
        case BlendMode::Adjustable: return "Adjustable";
        case BlendMode::Multiply: return "Multiply";
        case BlendMode::Buffer: return "Buffer";
        case BlendMode::Minimum: return "Minimum";
    }
    return "?";
}

// =============================================================================
// Herkunft eines Fremd-Imports (Lizenz-Pflicht, S72)
// =============================================================================

/**
 * Woher ein fremder Inhalt stammt und unter welcher Lizenz er steht.
 *
 * **Regel (Vorgabe Patrik S71):** JEDER Fremd-Import schreibt Herkunft, Autor,
 * Quelle und Lizenz in den Knoten — im Loader, nicht nur in der Doku, sonst
 * wandern fremde Shader unbemerkt in weitergegebene `.lvfx`. Shadertoy
 * verpflichtet API-Nutzer ausdrücklich auf die Lizenz JEDES EINZELNEN Shaders
 * (Default meist CC BY-NC-SA), ISF trägt ein `CREDIT`-Feld im JSON-Kopf.
 *
 * Bis S71 hingen die vier Felder direkt an `ShadertoyParams` — für ISF und
 * `pixelFilter` gab es sie deshalb gar nicht. Hier stehen sie EINMAL, und
 * jeder importfähige Knoten trägt sie als `herkunft`.
 *
 * **JSON:** ein eigenes Unterobjekt `herkunft` am Knoten. Bis S71 lagen die
 * vier Schlüssel flach — und `name` kollidierte dort mit dem KNOTENNAMEN
 * (`nodeToJson` schreibt `o["name"] = displayName`, der Params-Visitor lief
 * danach und überschrieb ihn). Ein umbenannter Shadertoy-Knoten verlor so
 * seinen Namen beim Speichern; seit S72 ist das behoben. Alte Dateien liest
 * der Serializer weiter (Fallback auf die flachen Schlüssel).
 * Eine leere Herkunft wird gar nicht geschrieben (schlankes JSON) — deshalb
 * taucht sie auch im Feld-Inventar nicht auf, das einen Vorgabe-Knoten
 * serialisiert.
 */
struct Herkunft
{
    std::string name;     ///< Titel des Originals (Shadertoy-Name, ISF-Dateiname)
    std::string author;   ///< Urheber laut Quelle (Shadertoy-Username, ISF-CREDIT)
    std::string url;      ///< Fundstelle (Shadertoy-URL; bei Dateien der Pfad)
    std::string license;  ///< Lizenztext, so wie die Quelle ihn nennt

    /// Nichts bekannt — dann schreibt der Serializer keinen einzigen Schlüssel.
    [[nodiscard]] bool leer() const
    {
        return name.empty() && author.empty() && url.empty() && license.empty();
    }
};

// =============================================================================
// Generischer Parameter-Baum (S72)
// =============================================================================

/// Was für ein Wert in einem `ParamWert` steckt — bestimmt den Editor im
/// Parameter-Baum (Plan §6: typsichere Editoren je Zeile).
enum class ParamTyp : int
{
    Bool = 0,      ///< Kästchen
    Ganzzahl = 1,  ///< Zähler ohne Nachkommastellen
    Zahl = 2,      ///< Zähler mit Nachkommastellen
    Farbe = 3,     ///< Farbwähler (RGBA in `vektor`)
    Punkt2D = 4,   ///< zwei Felder (XY in `vektor`)
    Text = 5,      ///< Zeile
    Auswahl = 6,   ///< Klartext-Dropdown (`auswahlLabels`/`auswahlWerte`)
};

/**
 * Ein Blatt des Parameter-Baums: ein bedienbarer Wert samt allem, was ein
 * Editor über ihn wissen muss.
 *
 * **Bewusst generisch, kein ISF-Sonderweg** (Vorgabe Patrik S71): dieselbe
 * Ablage trägt später die Parameter JEDES Moduls — sie ist die Infrastruktur,
 * die auch der Backlog-Punkt „dynamische Modulparameter" braucht.
 */
struct ParamWert
{
    std::string key;    ///< Bezeichner, unter dem der Shader/das Skript ihn liest
    std::string label;  ///< Klartext für die Anzeige (leer = `key`)
    ParamTyp typ = ParamTyp::Zahl;

    /// Zahl/Ganzzahl/Auswahl — bei Auswahl steht hier der WERT, nicht der Index.
    double zahl = 0.0;
    bool ja = false;        ///< Bool
    std::string text;       ///< Text
    /// Farbe (r,g,b,a) bzw. Punkt2D (x,y) — die restlichen Stellen sind 0.
    std::array<double, 4> vektor{{0.0, 0.0, 0.0, 0.0}};

    double min = 0.0;        ///< untere Grenze, nur gültig bei `hatBereich`
    double max = 1.0;        ///< obere Grenze, nur gültig bei `hatBereich`
    bool hatBereich = false;  ///< false = der Editor gibt sich einen weiten Bereich

    std::vector<std::string> auswahlLabels;  ///< Klartext je Eintrag (Auswahl)
    std::vector<int> auswahlWerte;           ///< der Wert je Eintrag (Auswahl)
};

/**
 * Ein Knoten des Parameter-Baums: eigene Werte plus Untergruppen.
 *
 * **Verschachtelung von Anfang an** (Entwurf Patrik): ISF-`INPUTS` sind zwar
 * flach, `PASSES`/`IMPORTED` sind es nicht — und der Ausbau auf alle Module
 * braucht sie ohnehin. Nachträglich einzuziehen wäre teurer als sie jetzt
 * mitzuführen.
 */
struct ParamGruppe
{
    std::string key;    ///< Bezeichner der Gruppe (leer = Wurzel)
    std::string label;  ///< Klartext der aufklappbaren Zeile
    std::vector<ParamWert> werte;
    std::vector<ParamGruppe> gruppen;

    /// Kein einziger Wert, auch nicht in einer Untergruppe.
    [[nodiscard]] bool leer() const
    {
        if (!werte.empty()) return false;
        for (const ParamGruppe& g : gruppen)
        {
            if (!g.leer()) return false;
        }
        return true;
    }
};

// =============================================================================
// Per-effect parameter structs (variant alternative == effect type)
// =============================================================================

/**
 * Container node (AVS Effect List): persistent list buffer (thisfb), in-/out-
 * blend, OnBeat activation and the optional EEL list slot pair (init/frame)
 * that drives enabled/clear/beat/alphain/alphaout at runtime.
 */
struct ListParams
{
    bool clearEveryFrame = false;  ///< clear the list buffer at frame start

    BlendMode blendIn = BlendMode::Ignore;    ///< parent -> list buffer
    BlendMode blendOut = BlendMode::Replace;  ///< list buffer -> parent
    int inAdjustAlpha = 128;                  ///< Adjustable in-alpha 0..255
    int outAdjustAlpha = 128;                 ///< Adjustable out-alpha 0..255

    // Buffer blend mode (E3 batch 2): a global buffer's per-pixel depth drives
    // the mix. Index = OffscreenBufferPool slot (0..7, same numbering as Buffer
    // Save); invert flips depth (AVS ininvert/outinvert).
    int bufferIn = 0;             ///< pool slot for Buffer in-blend
    int bufferOut = 0;            ///< pool slot for Buffer out-blend
    bool bufferInInvert = false;  ///< invert in-blend buffer depth
    bool bufferOutInvert = false; ///< invert out-blend buffer depth

    bool onBeatRender = false;  ///< render only for N frames after a beat
    int onBeatFrames = 1;       ///< N (>= 1)

    bool useCode = false;   ///< run the EEL list slots
    std::string initCode;   ///< EEL, once after (re)compile
    std::string frameCode;  ///< EEL, per frame (enabled/clear/beat/alphain/alphaout)
};

/**
 * Host-Gruppe (HG1, HostGruppen_Crossfade_Entwurf.md): kapselt ein komplettes
 * Visual als Level-1-Host. children wie bei ListParams, aber mit EIGENEM
 * Laufzeit-Bestand je Gruppe (persistenter Gruppen-Buffer OHNE clearEveryFrame
 * = Feedback, eigener OffscreenBufferPool, eigener ScriptContext). Tiefenregel:
 * keine Host-Gruppe in einer Host-Gruppe — der Compile-Pass degradiert
 * verschachtelte Gruppen zur Effect List (Warnung, kein Hard-Fail).
 * Mehrere Gruppen duerfen gleichzeitig aktiv sein (§2.6) und stapeln sich
 * ueber blendOut; der Crossfade (HG2) blendet paarweise A→B.
 */
struct HostGroupParams
{
    BlendMode blendOut = BlendMode::Replace;  ///< group buffer -> parent
    int outAdjustAlpha = 128;                 ///< Adjustable out-alpha 0..255

    // Wechsel-Settings (synchron ueber alle Gruppen, Entwurf §2.4) — die
    // Ein-/Ausgangskurven sind dagegen individuell je Gruppe; HG2 wertet aus.
    double crossfadeSeconds = 2.0;  ///< blend duration on group switch
    int curveIn = 0;                ///< 0 = linear (more curves with HG2)
    int curveOut = 0;               ///< 0 = linear
    /// Parameter script (Strang D, Block E S79): only `outadjustalpha` (+
    /// `b`/`w`/`h` + audio set) — the crossfade duration stays a slider because it
    /// applies to all groups in sync (Entwurf §2.4). Runs after the children
    /// in the group's own ScriptContext (sees their reg/gmegabuf of this
    /// frame). Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;

    std::string sourceFile;  ///< source (.avs/.milk/.lvfx; "" = created empty)
    /// Preset reference (S82): the content is loaded from `sourceFile` and is
    /// NOT saved along — the group is a link, not a copy. Off =
    /// copy (content lives in the chain, `sourceFile` only records its origin).
    bool verweis = false;
};

/**
 * Host Switcher (S82): schaltet zwischen Host-Gruppen um — nach Zeit, beim
 * Songwechsel oder per Skript; der Uebergang ist der Crossfade der Gruppen
 * (HG2). Geschaltet wird NUR zur Laufzeit (Entscheid Patrik S82, 2a): das
 * persistierte `enabled` der Gruppen bleibt, wie gespeichert. Die Gruppen
 * werden ueber ihren (eindeutigen) Namen angesprochen; Umbenennen zieht die
 * Liste mit. Index im Skript = Listenposition 1..count.
 */
struct HostSwitcherParams
{
    std::vector<std::string> groups;  ///< switched host groups (names), order = index 1..count
    int startIndex = 1;               ///< group at start (1..count)
    double intervalSeconds = 0.0;     ///< advance after this many seconds; 0 = off
    bool nextOnSongChange = false;    ///< advance on song change
    int order = 0;                    ///< advance: 0 = list order, 1 = random
    /// Keep the state of the listed groups while they rest: on leaving a group
    /// its picture, run time and nested lists stay as they are, on entering it
    /// again it continues from there. Off = a group starts with an empty
    /// picture each time. The script variable `clearbuffers` (set to 1) and the
    /// button Reset buffers make the resting groups start completely fresh.
    bool keepState = false;
    std::string initCode;             ///< EEL, once after loading
    std::string frameCode;            ///< EEL, every frame
    std::string beatCode;             ///< EEL, on every beat
    // Song-Slot: ChainNode::songCode (allgemein fuer alle Skript-Knoten)
};

/** AVS "Render / Clear screen" (ID 25), reduced to the 5.1 core. */
struct ClearParams
{
    uint32_t color = 0x000000;  ///< 0x00RRGGBB
    bool onlyFirst = false;     ///< clear only on the first frame
    /// 0 = replace, 1 = additive, 2 = 50/50, 3 = current line-blend
    /// (r_clear.cpp: blend==1 -> BLEND, blendavg -> BLEND_AVG, blend==2 -> BLEND_LINE)
    int blend = 0;

    /// Parameter script (Strang D): `blend` + `b`/`w`/`h` + audio set. Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Fadeout" (ID 3): per-frame clamped step towards a target color. */
struct FadeoutParams
{
    int fadeLen = 16;           ///< per-frame step 0..92 (AVS range)
    uint32_t color = 0x000000;  ///< target color 0x00RRGGBB

    /// Parameter script (Strang D): `fadelen` + `b`/`w`/`h` + audio set. Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Invert" (ID 37): XOR 0xFFFFFF. */
struct InvertParams
{
};

/**
 * AVS "Trans / Brightness" (ID 22): per-channel scale with an optional
 * exclusion color. Scale values are the AVS slider range -4096..4096
 * (0 = unchanged multiplier 1.0; see r_bright pixel math, filled in step 5.3).
 */
struct BrightnessParams
{
    int red = 0;    ///< -4096..4096
    int green = 0;  ///< -4096..4096
    int blue = 0;   ///< -4096..4096

    bool exclude = false;       ///< leave pixels near `color` untouched
    uint32_t color = 0x000000;  ///< exclusion color 0x00RRGGBB
    int distance = 16;          ///< exclusion radius 0..255

    /// Parameter script (Strang D): `red`, `green`, `blue`, `distance` + `b`/`w`/`h` +
    /// audio set. Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Simple" (ID 0, r_simple.cpp): the classic scope. `mode`
 * deckt das effect-Bitfeld ab (Bit 6 = Dot-Modus, sonst effect&3):
 * 0 solid analyzer · 1 line analyzer · 2 line scope · 3 solid scope ·
 * 4 dot analyzer · 5 dot scope. Analyzer lesen das SPEKTRUM (200 Baender,
 * xs=200/w), Scopes die Waveform (288 Samples, Byte^128); solid = eine
 * vertikale Linie je Bildspalte (S48-Matrix-Befund: der Default effect=0
 * ist solid analyzer — vorher zeichneten wir nur eine Linie).
 */
struct SimpleScopeParams
{
    int mode = 3;      ///< 0 solid ana, 1 line ana, 2 line scope, 3 solid scope,
                       ///< 4 dot analyzer, 5 dot scope
    int channel = 2;   ///< 0 L, 1 R, 2 center (byte halving as in the original)
    int position = 2;  ///< 0 top, 1 bottom, 2 center
    /// Color table (0x00RRGGBB), cycled through over the frames.
    std::vector<uint32_t> colors{0xFFFFFF};

    /// Parameter script (Strang D): `mode`, `channel`, `position` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Oscilliscope Star" (ID 2, r_oscstar.cpp): the waveform drawn along
 * 5 rotating radial spokes. `size`/`rot` 0..16; colour-cycled through `colors`.
 */
struct OscStarParams
{
    int channel = 2;   ///< 0 L, 1 R, 2 center (signed halved sum like the reference)
    int position = 2;  ///< 0 left (w/4), 1 right (3w/4), 2 center
    int size = 8;      ///< spoke length 0..16 (r_oscstar: s = size/32)
    int rot = 3;       ///< rotation speed (r_oscstar: m_r += 0.01*rot; 0 = still)
    /// Color table (0x00RRGGBB), cycled through over the frames.
    std::vector<uint32_t> colors{0xFFFFFF};

    // Freigemachte Host-Konstanten (S53; S60 auf die Referenz geankert —
    // Default = exakter r_oscstar-Rechenweg).
    int spokes = 5;          ///< number of spokes (reference fixed at 5)
    float rotScale = 0.01f;  ///< radians per `rot` unit and frame (reference 0.01)
    float amplitude = 1.0f;  ///< factor on the wave amplitude (reference 1)

    /// Parameter script (Strang D): `size`, `rot`, `spokes`, `rotscale`,
    /// `amplitude` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Ring" (ID 14, r_oscring.cpp): the waveform/spectrum drawn as a
 * closed 80-segment ring whose radius is modulated by the audio. `source`
 * 0 waveform, 1 spectrum. `size` 0..16.
 */
struct OscRingParams
{
    int source = 0;    ///< 0 waveform, 1 spectrum
    int channel = 2;   ///< 0 L, 1 R, 2 center
    int position = 2;  ///< 0 left, 1 right, 2 center
    int size = 8;      ///< base radius 0..16
    /// Color table (0x00RRGGBB), cycled through over the frames.
    std::vector<uint32_t> colors{0xFFFFFF};

    // Freigemachte Host-Konstanten (S53), Vorgaben = bisheriges Verhalten.
    int segments = 80;        ///< vertices of the ring (was fixed at 80)
    float baseScale = 0.1f;   ///< radius base without audio (was fixed at 0.1)
    float audioScale = 0.9f;  ///< audio share of the radius (was fixed at 0.9)

    /// Parameter script (Strang D): `size`, `segments`, `basescale`,
    /// `audioscale` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Rotating Stars" (ID 13, r_rotstar.cpp): two 5-pointed stars
 * orbiting the centre, sized by the peak spectrum energy; colour-cycled.
 *
 * Bis S53 hatte der Knoten AUSSER der Farbtafel keinen einzigen Parameter —
 * Zackenzahl, Bahn und Tempo standen als Literale im Renderer. Die Vorgaben
 * unten sind genau diese Literale, ein Preset aendert also nichts an seinem Bild
 * (Default-Vertrag, Knoten_Parameter_Konzept §2).
 */
struct RotatingStarsParams
{
    /// Color table (0x00RRGGBB), cycled through over the frames.
    std::vector<uint32_t> colors{0xFFFFFF};

    int points = 5;          ///< points per star (was fixed at 5)
    int skip = 2;            ///< step width when drawing; 2 = pentagram
    int stars = 2;           ///< number of stars on opposite orbits
    float rotSpeed = 0.1f;   ///< orbit rotation per frame (r_rotstar: r1 += 0.1)
    float orbit = 0.5f;      ///< distance from the image center
                             ///< (NDC; 0.5 = w/4 like the original)
    /// Star size without audio, per axis in NDC. The reference computes
    /// w/8*(s+9)/88 (r_rotstar:145) = (s+9)/352 in NDC — the defaults 9/352
    /// and 255/352 reproduce exactly this formula at the default (S60).
    float baseRadius = 0.0255682f;
    float audioGain = 0.7244318f;  ///< gain from the spectrum peak (255/352, see baseRadius)
    int bandLo = 3;            ///< first evaluated spectral band (local peak search)
    int bandHi = 14;           ///< first band NO longer evaluated

    /// Parameter script (Strang D): computes the sliders per frame. Readable
    /// and writable names are the field names in lower case
    /// (`points`, `skip`, `stars`, `rotspeed`, `orbit`, `baseradius`,
    /// `audiogain`), plus `b`/`w`/`h` and the audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Bass Spin" (ID 7, r_bspin.cpp): two bass-reactive spinning shapes
 * (left/right channel), drawn in the left/right screen half. `mode` 0 lines,
 * 1 filled (line-approximated here). Spin speed follows the bass energy.
 */
struct BassSpinParams
{
    bool left = true;   ///< draw the left image half (left channel)
    bool right = true;  ///< draw the right image half (right channel)
    uint32_t colorLeft = 0xFFFFFF;   ///< color of the left figure 0x00RRGGBB
    uint32_t colorRight = 0xFFFFFF;  ///< color of the right figure 0x00RRGGBB
    int mode = 1;  ///< 0 lines, 1 filled (approximated as lines)

    // KLASSE A: Vorgaben aus `r_bspin.cpp`, Abweichung wird gekennzeichnet.
    float smoothing = 0.7f;   ///< weight of the new value: `v = s*neu + (1-s)*alt`
    /// Rotation step per unit. The default is EXACTLY the original's expression
    /// (`3.14159f/6.0f`), not the mathematical pi/6 — the two
    /// differ in the 7th digit, and that accumulates in the rotation angle
    /// over the frames.
    float spinStep = 3.14159f / 6.0f;

    /// Parameter script (Strang D): `mode`, `smoothing`, `spinstep`.
    /// Class A — see note at `DotPlaneParams`.
    /// Colors via script (Block B, S79): `colorleft_r`, `colorleft_g`, `colorleft_b`,
    /// `colorright_r`, `colorright_g`, `colorright_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Color Clip" (ID 12, r_contrast.cpp): replace pixels matching a
 * colour threshold with `outColor`. `mode` 1 below (all channels <= clip),
 * 2 above (all >= clip), 3 near (within `distance` of clip). COLORREF-swapped.
 */
struct ColorClipParams
{
    int mode = 1;                    ///< 1 below, 2 above, 3 near
    uint32_t clipColor = 0x202020;   ///< threshold color 0x00RRGGBB
    uint32_t outColor = 0x202020;    ///< replacement color
    int distance = 10;               ///< match radius (near mode)

    /// Parameter script (Strang D): `distance` + `b`/`w`/`h` + audio set. Empty = no script.
    /// Colors via script (Block B, S79): `clipcolor_r`, `clipcolor_g`, `clipcolor_b`,
    /// `outcolor_r`, `outcolor_g`, `outcolor_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Unique Tone" (ID 38, r_onetone.cpp): tint the image to a single
 * hue — output = `color` * luminance(max channel), optionally inverted. `blend`
 * 0 replace, 1 additive, 2 50/50.
 */
struct UniqueToneParams
{
    uint32_t color = 0xFFFFFF;  ///< tone color 0x00RRGGBB
    bool invert = false;        ///< invert the depth ramp
    int blend = 0;              ///< 0 replace, 1 additive, 2 50/50

    /// Parameter script (Strang D): `blend` + `b`/`w`/`h` + audio set. Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Interleave" (ID 23, r_interleave.cpp): overlay a stripe/grid of
 * `color` at spacings `x`/`y` (0 = that axis off). On beat the spacing eases to
 * `x2`/`y2` over `beatDuration`. `blend` 0 replace, 1 additive, 2 50/50.
 */
struct InterleaveParams
{
    int x = 1;                  ///< horizontal stripe spacing (px, 0 = off)
    int y = 1;                  ///< vertical stripe spacing (px, 0 = off)
    uint32_t color = 0x000000;  ///< stripe color 0x00RRGGBB
    int blend = 0;              ///< 0 replace, 1 additive, 2 50/50
    bool onBeat = false;        ///< ease to x2/y2 on beat
    int x2 = 1;                 ///< target horizontal spacing in the beat transition
    int y2 = 1;                 ///< target vertical spacing in the beat transition
    int beatDuration = 4;       ///< ease length

    /// Parameter script (Strang D): `x`, `y`, `x2`, `y2` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Picture" (ID 34, r_picture.cpp): display a bitmap over the frame.
 * The original referenced an external file (`filename`); on import the resolved
 * image bytes are base64-embedded into `imageData` (decision: self-contained
 * .lvfx). `blend` 0 replace, 1 additive, 2 50/50; `keepAspect` letterboxes.
 * Empty `imageData` (file not found) renders as a no-op.
 */
struct PictureParams
{
    std::string filename;    ///< original AVS filename (reference only)
    std::string imageData;   ///< base64 of the raw image file ("" = unresolved)
    int blend = 2;           ///< 0 replace, 1 additive, 2 50/50
    bool keepAspect = true;  ///< preserve the image aspect (letterbox)

    /// Parameter script (Strang D): `blend` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Picture II": display an embedded image stretched to fill, blended per
 * `blend` (community APE). Like Picture but always fills the frame (bilinear).
 */
struct PictureIIParams
{
    /// Origin note from the preset. What is drawn is `imageData`; nothing is
    /// ever loaded from this path.
    std::string filename;
    std::string imageData;   ///< base64 of the raw image ("" = unresolved)
    int blend = 2;           ///< 0 replace, 1 additive, 2 50/50

    /// Parameter script (Strang D): `blend` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Fast Brightness" (ID 44): dir 0 = x2, 1 = x0.5, 2 = off. */
struct FastBrightnessParams
{
    int dir = 0;  ///< 0..2

    /// Parameter script (Strang D): `dir` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Blur" (ID 6): box blur, strength selects the kernel. */
struct BlurParams
{
    int strength = 1;    ///< 1 = light, 2 = medium, 3 = heavy
    /// AVS "round mode": off = every partial term is truncated and the image
    /// fades with every application; on = fixed compensation per kernel (+4/+5/+3).
    /// Default OFF as in the original (`roundmode = 0`, r_blur.cpp:75/90) — until S57
    /// this was `true`, and the field was not read anywhere anyway.
    bool roundUp = false;

    /// Parameter script (Strang D): `strength` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Mirror" (ID 26): reflect one screen half onto the other. */
struct MirrorParams
{
    /// r_mirror direction bits: 1 = top->bottom (HORIZONTAL1),
    /// 2 = bottom->top (HORIZONTAL2), 4 = left->right (VERTICAL1),
    /// 8 = right->left (VERTICAL2)
    int mode = 4;
    bool onBeatRandom = false;  ///< randomize active edges on beat
    bool smooth = false;        ///< gradual transition (BLEND_ADAPT ramp)
    int slower = 4;             ///< frames per ramp step (1..16)

    /// Parameter script (Strang D): `mode`, `slower` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Render / OnBeat Clear" (ID 5): clear every N beats. */
struct OnBeatClearParams
{
    uint32_t color = 0x000000;  ///< clear color 0x00RRGGBB
    int everyNBeats = 1;        ///< N (>= 1)
    bool blend = false;         ///< 50/50 towards color instead of hard clear

    /// Parameter script (Strang D): `everyNBeats` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Colorfade" (ID 11): per-pixel channel-order classification adds
 * fader deltas — a cycling color shift. Fader triples are signed byte deltas;
 * the beat variant is used for `onBeatFrames` frames after a beat.
 */
struct ColorfadeParams
{
    int faderR = 8;   ///< -32..32
    int faderG = 8;   ///< green step per frame, -32..32
    int faderB = -8;  ///< blue step per frame, -32..32
    int beatFaderR = 8;   ///< red step in the beat window (replaces `faderR`)
    int beatFaderG = -8;  ///< green step in the beat window (replaces `faderG`)
    int beatFaderB = 8;   ///< blue step in the beat window (replaces `faderB`)
    /// LumiViz EXTENSION (Patrik's suggestion): how many frames the beat state
    /// is held before `slowFade` pulls it back. The original has no
    /// window — it sets the faders in the beat frame and pulls back from the next
    /// one on. **The default 1 is therefore exactly this behavior**, so an
    /// imported AVS preset does not change; larger values are the
    /// new option. Without `slowFade` the field has no effect — then there is
    /// no beat state that could be held.
    int onBeatFrames = 1;
    /// `enabled` bit 1 of `r_colorfade`: in the beat frame the faders are chosen
    /// RANDOMLY instead of taken from `beatFader*` — red and blue from
    /// `rand()%32 - 6`, green from `rand()%64 - 32`, where values with magnitude
    /// below 16 are pushed out to +-32 (r_colorfade.cpp:157-161).
    bool onBeatRandom = false;
    /// `enabled` bit 2 of `r_colorfade`: the faders move ONE step per
    /// frame toward their target value instead of jumping there at once. Only with this
    /// bit do the beat faders have any effect at all — in the original the beat branch
    /// is otherwise unreachable (`if (!(enabled&4)) … else if (isBeat) …`).
    bool slowFade = false;

    /// Parameter script (Strang D): `faderr`, `faderg`, `faderb`, `beatfaderr`,
    /// `beatfaderg`, `beatfaderb`, `onbeatframes` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Color Modifier" (ID 45): per-channel 256-entry curve, scripted
 * (EEL level/frame/beat/init slots run by a ScriptLutModule). `recompute`
 * rebuilds the table every frame instead of once after compile.
 */
struct ColorModifierParams
{
    /// EEL slots that share their variables with the stage code: init
    /// once at build time, frame per frame, beat per beat.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string levelCode;   ///< runs per LUT entry (red/green/blue in/out)
    /// Recompute the table every frame instead of once after translation. Needed
    /// as soon as the curve depends on `time` or on the audio.
    bool recompute = true;
};

/**
 * AVS "Trans / Movement" (ID 15): coordinate remap via a scripted displacement
 * grid (ScriptGridModule runs the point expression per node). 5.4 covers the
 * user-code path; the 23 built-in formulas map to passthrough for now.
 */
struct MovementParams
{
    std::string code;         ///< AVS point expression (empty = identity)
    bool rectCoords = false;  ///< true = x/y, false (AVS default) = polar d/r
    bool wrap = false;        ///< wrap sampling coordinates instead of clamp
    bool blend = false;       ///< 50/50 blend of moved pixel with original (r_trans)
    bool subpixel = true;     ///< bilinear (on) vs nearest (off) source sampling
    /// r_trans sourcemapped bits: 1 = scatter-write (source pixel is PUSHED to
    /// its target, MAX-blended), 2 = toggle bit 1 on every beat
    int sourceMapped = 0;
    /// r_trans builtins WITHOUT eval_desc (pixel-index remaps, no d/r formula):
    /// 1 = "slight fuzzify", 7 = "blocky partial out"; 0 = code/formula path.
    int builtinRemap = 0;

    // KEIN Parameter-Skript (Entscheid S53): Movement legt eine STATISCHE
    // Tabelle an (`applyMovementTable` cacht ueber den Skripttext), ein je
    // Frame gerechneter Wert koennte das Bild gar nicht bewegen. Fuer
    // zeitabhaengige Verzerrung ist `DynamicMovementParams` der Knoten.
};

/**
 * AVS "Misc / Comment" (ID 21): pure annotation, renders nothing. Own type
 * (not a Passthrough) so the editor gets a dedicated multi-line field and
 * the tree's description column stays free (Entscheid Patrik, S44).
 */
struct CommentParams
{
    /// Free text. The node draws nothing — it keeps a note in the
    /// chain.
    std::string text;
};

/**
 * LumiViz "Misc / Import Notes" (kein AVS-Effekt): das Protokoll des Imports,
 * als Knoten IN der Kette statt als Dialog (Entscheid Patrik, S51).
 *
 * Vorher landete jede Notiz im Meldungsfenster — auch die harmlosen, etwa die
 * `_p`-Umbenennungen der Kollisionsregel (D2). Bei einem Preset mit einem
 * Dutzend Umbenennungen gehen darin die WIRKLICHEN Probleme unter. Jetzt gilt:
 * der Knoten traegt das vollstaendige Protokoll (Zusammenfassung, Hinweise,
 * Probleme), der Dialog erscheint nur noch bei Problemen.
 *
 * Wie Comment ein reiner Anmerkungsknoten — rendert nichts, das Feld ist
 * schreibgeschuetzt, weil es einen Vorgang protokolliert und keine Eingabe ist.
 */
struct ImportNotesParams
{
    /// Importer report (read-only): what could not be carried over from
    /// the preset.
    std::string text;
};

/**
 * LumiViz "Misc / Render Scale" (kein AVS-Effekt): laesst die GESAMTE Chain in
 * einer reduzierten internen Aufloesung rendern (Fenster / divisor) und beim
 * finalen Present hochskalieren. Das ist der Winamp-Look: AVS steckt voller
 * pixel-fester Groessen (SRM-width 255, Bump-Radius 127 px, Blur-Kernel) —
 * Presets wie der Wormhole verhungern bei grossen Flaechen, original wie
 * importiert (Befund S47). Der ERSTE aktivierte Knoten in der Kette gewinnt;
 * gerendert wird er selbst als No-op.
 */
struct RenderScaleParams
{
    int divisor = 2;   ///< internal resolution = window / divisor (1..8)
    int filter = 0;    ///< Upscale: 0 = nearest (authentically coarse), 1 = linear
};

/**
 * LumiViz "Post / Bloom" (kein AVS-Effekt; Lights-Etappe 1): additiver Glow
 * nach dem HelloEnjoy-"Lights"-Rezept — Surface auf ein kleines RT
 * downsamplen (Referenz fix 512^2), separierbarer 25-Tap-Gauss (H+V),
 * Ergebnis additiv dazu (min(a+b,1)). Referenz OHNE Threshold
 * (threshold = 0); Vignette (1 - r^2 * strength) als optionaler Abschluss.
 *
 * post=true (Default, Referenzverhalten): der Glow entsteht erst beim
 * PRESENT — Anzeige-only, die Chain-Surface bleibt unberuehrt. Wie in Lights
 * (Bloom ist Post-Processing NACH dem Szenen-Render); der erste aktivierte
 * post-Knoten gewinnt, gerendert wird er selbst als No-op (wie Render
 * Scale). post=false schreibt den Glow zurueck auf die Surface — in
 * Feedback-Ketten (Fadeout statt Clear) akkumuliert der additive Anteil
 * dann ueber die Frames bis Weiss (S48-Befund): bewusst nur fuer
 * Glow-Schweif-Effekte einsetzen.
 */
struct BloomParams
{
    int downsample = 2;        ///< Glow RT = internal resolution / 2^n (0..4)
    int radius = 8;            ///< Gaussian sigma in glow-RT pixels (25 taps fixed)
    float intensity = 1.0f;    ///< factor of the additive glow component
    float threshold = 0.0f;    ///< brightness threshold before the blur (0 = reference)
    bool vignette = false;     ///< final multiplication 1 - r^2 * strength
    float vignetteStrength = 0.3f;  ///< strength of the edge darkening (only with `vignette`)
    bool post = true;          ///< true: glow only at present time (no feedback)

    /// Parameter script (Strang D): `radius`, `intensity`, `threshold`,
    /// `vignettestrength` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * LumiViz "3D / Camera" (kein AVS-Effekt; Lights-Etappe 1): Frame-Zustand
 * wie Set Render Mode — der Knoten setzt beim Ketten-Walk die 3D-Kamera
 * fuer alle FOLGENDEN 3D-Module (SuperScope 3D, spaeter Terrain/Orbs);
 * Reset je Frame auf die Fallback-Kamera (Position 0/0/+1/tan(fov/2),
 * Blick auf den Ursprung, three.js-Konvention wie die Lights-Referenz:
 * x+ rechts, y+ oben, z+ zum Betrachter — x/y in [-1,1] bei z=0 fuellen
 * das Bild vertikal). Die EEL-Slots init/frame/beat duerfen px..pz,
 * tx..tz, fov, roll, fogstart, fogend ueberschreiben — erster Fall der
 * "dynamischen Modulparameter" (Slot-Ordnung Frame VOR Beat,
 * S47-Konvention). Die Parameterwerte seeden die Skript-Variablen beim
 * (Re-)Compile.
 */
struct Camera3DParams
{
    float px = 0.0f;               ///< camera position (world coordinates)
    float py = 0.0f;               ///< camera position, up axis (y+ = up)
    float pz = 3.7320508f;         ///< +1/tan(15 deg): default fills [-1,1]
    float tx = 0.0f;               ///< look-at target
    float ty = 0.0f;               ///< look-at target, up axis
    float tz = 0.0f;               ///< look-at target, depth (z+ = toward the viewer)
    float fov = 30.0f;             ///< vertical field of view (degrees)
    float roll = 0.0f;             ///< rotation around the view axis (degrees)
    float fogStart = 0.0f;         ///< fog distance (world); start >= end = off
    float fogEnd = 0.0f;           ///< distance at which the fog fully covers
    uint32_t fogColor = 0x000000;  ///< 0x00RRGGBB (sprites are only attenuated)
    /// EEL slots (order: frame BEFORE beat): may override `px`..`pz`, `tx`..`tz`,
    /// `fov`, `roll`, `fogstart`, `fogend`. The field values above
    /// are the initial values at translation time.
    /// Colors via script (Block B, S79): `fogcolor_r`, `fogcolor_g`, `fogcolor_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * LumiViz "3D / SuperScope 3D" (kein AVS-Effekt; Lights-Etappe 1):
 * EEL-Quartett wie SuperScope, aber der Point-Code schreibt x,y,z (WELT-
 * Koordinaten, y+ = oben — die AVS-Raum-Regel gilt am Modulrand NICHT) plus
 * optional size (Welt-Einheiten), red/green/blue, skip. Das Modul
 * transformiert mit der aktiven 3D-Kamera (camera3d-Knoten oder Fallback),
 * clippt am Near-Plane, attenuiert die Groesse ~1/Tiefe und daempft per
 * Kamera-Fog. Rendering: additive Soft-Sprites mit eingebautem radialem
 * exp(-r^2*falloff)-Profil (kein Bild-Asset) oder Linien (3D-projiziert,
 * Zeichnung wie SuperScope ueber den ScopeRenderer).
 */
struct SuperScope3DParams
{
    /// EEL quartet. init/frame/beat run once per build/frame/beat and
    /// set, among other things, `n`; the point code runs per point.
    /// More fields via script (Block D, S79): `falloff`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Runs per point: `i` 0..1 and `v` (audio) come in; `x`,
    /// `y`, `z` (world coordinates), `red`/`green`/`blue` and `size` are read.
    std::string pointCode;
    int pointCount = 256;   ///< default n (script n overrides)
    int renderMode = 0;     ///< 0 = soft sprites (points), 1 = lines
    float size = 0.05f;     ///< default point size (world units)
    float falloff = 4.0f;   ///< sprite profile k in exp(-r^2*k)
    int audioChannel = 2;   ///< v source: 0 = left, 1 = right, 2 = mono
    bool spectrumSource = false;  ///< v reads spectrum instead of waveform
};

/**
 * LumiViz "3D / Terrain" (kein AVS-Effekt; Lights-Etappe 2, Modul 4):
 * res x res-Heightfield nach dem Lights-Terrain-Rezept — prozedurale Basis
 * (fester Seed, KEIN Bild-Asset), Spektrum als radiale Ringe
 * (Hoehe += spectrum[dist(x,y,Zentrum)] * ringAmp), Feder-Relaxation zurueck
 * zur Basis (v = v*drag + (h0-h)*|h|*dt), flatten. Darstellung: dunkles
 * opakes Mesh (gemeinsames Depth-RT, Fog) + additive Soft-Sprites an den
 * Gitterpunkten (Sprite-Shader von SuperScope 3D), Punktfarbe als Palette
 * ueber die Hoehe ODER red/green/blue im Point-Slot (je Gitterpunkt,
 * i = INDEX 0..res*res-1). Die EEL-Slots sehen das Hoehen-Grid als
 * megabuf(gy*res+gx) — frei formbar; Grid liegt zentriert um den Ursprung
 * in der xz-Ebene (x+ rechts, z+ zum Betrachter), Hoehe = y.
 */
struct Terrain3DParams
{
    int resolution = 64;      ///< grid points per axis (8..128)
    float extent = 4.0f;      ///< world width/depth of the grid
    float baseAmp = 0.15f;    ///< amplitude of the procedural base
    float yOffset = -0.8f;    ///< base height (world y)
    float ringAmp = 1.0f;     ///< spectrum ring injection per frame (0 = off)
    float relax = 0.12f;      ///< spring relaxation toward the base (0..1)
    float flatten = 0.0f;     ///< direct pull toward the base per frame (0..1)
    bool drawMesh = true;     ///< dark mesh (opaque, writes depth)
    uint32_t meshColor = 0x101418;  ///< grid color 0x00RRGGBB
    bool drawDots = true;     ///< additive soft sprites at the grid points
    float dotSize = 0.045f;   ///< world units
    float falloff = 4.0f;     ///< sprite profile k
    uint32_t colorLow = 0x0A2040;   ///< palette: valley
    uint32_t colorHigh = 0x40C0FF;  ///< palette: peak
    /// EEL slots for the terrain; they share their variables with the
    /// point code below.
    /// Colors via script (Block B, S79): `meshcolor_r`, `meshcolor_g`, `meshcolor_b`,
    /// `colorlow_r`, `colorlow_g`, `colorlow_b`, `colorhigh_r`, `colorhigh_g`,
    /// `colorhigh_b` (0..1).
    /// More fields via script (Block D, S79): `baseamp`, `yoffset`, `extent`, `dotsize`,
    /// `falloff`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pointCode;    ///< per grid point: color (red/green/blue)
};

/**
 * LumiViz "3D / Glow Orbs" (kein AVS-Effekt; Lights-Etappe 2, Modul 5):
 * n Low-Poly-Ellipsoide (16x12) mit Zwei-Farb-Vertex-Verlauf
 * (mix(color2, color, ny)) + flash (Beat-Blitz, additiver Offset wie das
 * Original-addRGB), Fog, gemeinsames Depth-RT (opak); dahinter je Orb ein
 * additives Halo-Billboard (Radial-Falloff, Depth-Test ohne Write —
 * ergibt den Rim-Glow). Slots init/frame/beat/point; der Point-Slot laeuft
 * JE ORB (i = Orb-INDEX 0..n-1) und schreibt x,y,z, radius, sx,sy,sz
 * (Achsen-Squash -> oval), red/green/blue (+ red2/green2/blue2 unten),
 * flash (0..1). Ohne Point-Code stehen die Orbs in einer Reihe.
 */
struct GlowOrbsParams
{
    int orbCount = 5;            ///< default n (script n overrides)
    float haloScale = 2.2f;      ///< halo radius = radius * haloScale
    float haloIntensity = 0.6f;  ///< halo brightness (0 = off)
    float falloff = 3.0f;        ///< halo profile k in exp(-r^2*k)
    /// EEL quartet. init/frame/beat run once per build/frame/beat and
    /// set, among other things, `n` (number of orbs); the point code runs per orb.
    /// More fields via script (Block D, S79): `haloscale`, `halointensity`, `falloff`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pointCode;       ///< per orb (i = orb index)
};

/**
 * AVS "Render / Text" (ID 28, r_text.cpp): word cycler drawn with the app
 * font engine (GDI original -> QPainter port). `text` holds the words
 * separated by ';'; blend applies to the glyph pixels only.
 */
struct TextParams
{
    std::string text;              ///< words separated by ';'
    std::string fontFace;          ///< LOGFONT lfFaceName ("" = default font)
    int fontHeight = -20;          ///< LOGFONT lfHeight (negative = px char height)
    int fontWeight = 400;          ///< 400 normal, 700 bold
    bool italic = false;           ///< LOGFONT lfItalic — italic
    bool underline = false;        ///< LOGFONT lfUnderline — underlined
    std::uint32_t color = 0xFFFFFF;  ///< font color 0x00RRGGBB
    int blend = 0;                 ///< 0 replace, 1 additive, 2 50/50 (glyphs only)
    bool onBeat = false;           ///< word switches on beat instead of timer
    int onBeatSpeed = 15;          ///< frames a beat-word stays visible
    int normSpeed = 15;            ///< frames per word in timer mode
    bool insertBlank = false;      ///< blank frame between words
    bool randomPos = false;        ///< random position per word
    bool randomWord = false;       ///< random instead of sequential word pick
    int hAlign = 1;                ///< 0 left, 1 center, 2 right
    int vAlign = 1;                ///< 0 top, 1 center, 2 bottom
    int xShift = 0;                ///< percent of width
    int yShift = 0;                ///< percent of height
    bool outline = false;          ///< draw an outline around the glyphs
    std::uint32_t outlineColor = 0;  ///< outline color 0x00RRGGBB
    int outlineSize = 1;           ///< outline width in pixels
    bool shadow = false;           ///< shadow variant (outline wins if both)

    /// Parameter script (S86): every field of this node (`normspeed`,
    /// `onbeatspeed`, `blend`, `onbeat` …) + `b`/`w`/`h` + audio set. Fields
    /// that re-render the glyphs (font, color, position, outline) follow the
    /// script from Init, Song and Beat only, not from Frame. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / AVI" (ID 32, r_avi.cpp): video frames stretched to the frame,
 * decoded via Video for Windows (legacy codec support like the original).
 */
struct AviParams
{
    std::string filename;          ///< as stored in the preset (bare name)
    std::string resolvedPath;      ///< absolute path (import-time upward search)
    int blend = 0;                 ///< 0 replace, 1 additive, 2 50/50
    bool adapt = false;            ///< beat-adaptive: additive on beat window, else 50/50
    int persist = 6;               ///< beat window length in frames
    int speedMs = 0;               ///< min milliseconds between frame advances

    /// Parameter script (S86): every field of this node (`blend`, `adapt`,
    /// `persist`, `speedms`) + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Dynamic Movement" (ID 43): grid-based scripted remap with the
 * full EEL quartet and a configurable grid resolution.
 */
struct DynamicMovementParams
{
    /// EEL quartet. The point code runs per grid node and writes
    /// `x`/`y` (Cartesian) or `d`/`r` (polar) and `alpha`; init/frame/beat
    /// run once per build/frame/beat and share the variables.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Runs per grid node: `x`, `y`, `d`, `r`, `alpha` are read —
    /// where the pixel fetches its color FROM, not where it goes.
    std::string pointCode;
    /// Grid nodes horizontally. Coarser means smoother (values in between are
    /// interpolated) and faster.
    int xres = 16;
    /// Grid nodes vertically.
    int yres = 12;
    /// Point code computes in pixels instead of normalized coordinates.
    bool rectCoords = false;
    /// Whatever points past the edge comes back in on the opposite side
    /// (off instead: the edge is clamped).
    bool wrap = false;
    /// r_dmove flags: `blend` mixes the moved pixel onto the original by the
    /// script's per-cell alpha; `nomove` skips displacement and only alpha-fades.
    bool blend = false;
    /// Do not displace — only the script's `alpha` takes effect (cross-fade without
    /// motion, r_dmove flag).
    bool nomove = false;
    bool subpixel = true;  ///< bilinear (on) vs nearest (off) source sampling
    /// Source image: 0 = current frame, 1..8 = global buffer (r_dmove fbin;
    /// missing buffer -> effect is a passthrough, r_dmove.cpp:290)
    int buffern = 0;
};

/**
 * AVS "Trans / Dynamic Distance Modifier" (ID 35): a scripted RADIAL remap. The
 * pixel EEL runs once per distance ring (`d` normalized 0..1 in/out); the host
 * builds a 1-D distance→distance LUT and a shader resamples each pixel at the
 * remapped distance along its angle (r_ddm.cpp). `blend` = 50/50 with the
 * original; `bilinear` = subpixel sampling. Vars: d (in/out), b, plus init/frame.
 */
struct DynamicDistanceModifierParams
{
    /**
     * Script variables: **only `d`** (+ `b` and the audio set).
     *
     * `d` is a **normalized distance**, not a factor: the pixel code receives
     * the distance of the target point from the image center as 0..1 and writes
     * back the distance to read from. `d=d` is thus the
     * identity, `d=d*0.6` fetches from 60 % of the distance (compression toward
     * the outside), `d=1.4` reads from the same distance for EVERY point.
     * Only the table turns this into the scaling factor
     * (r_ddm.cpp:287-289: `m_tab[x] = d * 256 * max_d / (x+1)`).
     *
     * Unlike Movement and Dynamic Movement, this effect knows **neither
     * angles nor coordinates** — its table runs over the radius and
     * applies equally in all directions. It can therefore only compress and
     * stretch, never rotate or shift.
     */
    // Vorgabe LEER — der Deserialisierer liess diese Slots beim Laden schon
    // immer leer (`getStr` ohne Vorgabewert), und seit S56 ist der Struct die
    // einzige Quelle. Die frueheren Demo-Skripte gehoeren als VOREINSTELLUNG
    // hierher (Konzept §11), nicht als Vorgabe: ein importiertes Preset darf
    // sie nicht erben. Sie stehen in `Offene_Punkte.md §1d`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pixelCode;
    bool blend = false;    ///< 50/50 with the original image
    /**
     * Intermediate values when sampling (AVS `subpixel`, r_ddm.cpp:313 /
     * r_shift.cpp:206).
     *
     * `false` reads the nearest pixel — hard edges. `true`
     * mixes the four neighbors like AVS' BLEND4, i.e. with integer steps
     * and truncation instead of GL filtering (the difference is documented, see
     * MultiEffectVisualizer.cpp:676).
     *
     * Was called `bilinear` until S54 and was therefore the only one of the SIX
     * subpixel fields not wired up — Movement, Dynamic Movement, Blitter
     * Feedback and Roto Blitter kept the original name and have long passed it
     * through. Old `.lvfx` files with `"bilinear"` are still read.
     * Default as in the original: OFF (r_ddm.cpp:210).
     */
    bool subpixel = false;
};

/**
 * AVS "Render / Moving Particle" (ID 8): a single spring-driven particle that
 * bounces around (target re-randomized on beat) and is drawn as a filled circle
 * (r_parts.cpp). `maxDistance` scales the travel radius; `size`/`size2` the
 * radius (size2 on beat when `onBeatSize`); `blend` 0 replace, 1 additive,
 * 2 50/50, 3 line (~additive). Rendered as a sized dot via the ScopeRenderer.
 */
struct MovingParticleParams
{
    uint32_t color = 0xFFFFFF;  ///< particle color 0x00RRGGBB
    int maxDistance = 16;       ///< travel-radius scale (AVS maxdist)
    int size = 8;               ///< particle radius (px)
    int size2 = 8;              ///< on-beat radius
    bool onBeatSize = false;    ///< jump to size2 on beat (AVS enabled bit 1)
    int blend = 1;              ///< 0 replace, 1 additive, 2 50/50, 3 line

    // KLASSE A: Vorgaben aus `r_parts.cpp`, Abweichung wird gekennzeichnet.
    float spring = 0.004f;   ///< pull toward the target per frame (original 0.004)
    float damping = 0.991f;  ///< velocity damping (original 0.991)

    /// Parameter script (Strang D): `size`, `size2`, `maxdistance`, `spring`,
    /// `damping`. Class A — see note at `DotPlaneParams`.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Dynamic Shift" (ID 42): a scripted GLOBAL image translation. The
 * frame/beat EEL sets `x,y` (pixel offset); the whole image is shifted by (x,y)
 * with black fill (decision: uniform offset, not a grid remap — r_shift is
 * image-global affine). `blend` = 50/50 with the original by `alpha`; `bilinear`
 * = subpixel sampling. Variables: x,y (out, pixels), w,h, b, alpha (default 0.5).
 */
struct DynamicShiftParams
{
    /// EEL slots. They set `x`/`y` — the shift of the whole image —
    /// and share their variables.
    // Vorgabe LEER — der Deserialisierer liess diese Slots beim Laden schon
    // immer leer (`getStr` ohne Vorgabewert), und seit S56 ist der Struct die
    // einzige Quelle. Die frueheren Demo-Skripte gehoeren als VOREINSTELLUNG
    // hierher (Konzept §11), nicht als Vorgabe: ein importiertes Preset darf
    // sie nicht erben. Sie stehen in `Offene_Punkte.md §1d`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    bool blend = false;    ///< 50/50 with the original image
    /**
     * Intermediate values when sampling (AVS `subpixel`, r_ddm.cpp:313 /
     * r_shift.cpp:206).
     *
     * `false` reads the nearest pixel — hard edges. `true`
     * mixes the four neighbors like AVS' BLEND4, i.e. with integer steps
     * and truncation instead of GL filtering (the difference is documented, see
     * MultiEffectVisualizer.cpp:676).
     *
     * Was called `bilinear` until S54 and was therefore the only one of the SIX
     * subpixel fields not wired up — Movement, Dynamic Movement, Blitter
     * Feedback and Roto Blitter kept the original name and have long passed it
     * through. Old `.lvfx` files with `"bilinear"` are still read.
     * Default as in the original: ON (r_shift.cpp:127).
     */
    bool subpixel = true;
};

/**
 * AVS "Trans / Blitter Feedback" (ID 4, r_blit.cpp — S48 auf die
 * Original-Felder umgestellt): fpos eased mit +-3/Frame auf `scale`
 * (Beat setzt fpos=scale2 wenn onBeat). f_val < 32 = blitter_normal
 * (zentrischer Zoom-IN, Faktor 64/(f_val+32)); f_val > 32 = blitter_out
 * (das GANZE Bild in ein zentriertes Fenster w/((f_val+96)/128) skaliert,
 * der Rand bleibt stehen); 32 = no-op. `blend` = BLEND_AVG mit dem
 * Original; `subpixel` = bilinear (BLEND4) — ohne kaskadiert der
 * Nearest-Zoom zum Mosaik (Matrix-Befund 04).
 */
struct BlitterFeedbackParams
{
    int scale = 30;         ///< Target value of the fpos ease (0..256; 32 = neutral)
    int scale2 = 30;        ///< Beat jump value (onBeat)
    bool onBeat = false;    ///< Beat sets fpos = scale2
    bool blend = false;     ///< BLEND_AVG with the original instead of replace
    bool subpixel = true;   ///< bilinear sampling (BLEND4)

    /// Parameter script (Strang D): `scale`, `scale2` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Roto Blitter" (ID 9, r_rotblit.cpp — S48 auf die
 * Original-Semantik umgestellt): das Bild wird JEDE Frame um das konstante
 * theta = (rotDir-32)*rot_rev Grad gedreht und um 1+(f_val-31)/31 gezoomt —
 * die sichtbare Rotation akkumuliert uebers FEEDBACK (nicht ueber einen
 * wachsenden Winkel; die alte Doppel-Akkumulation war das Pixel-Rauschen
 * des Matrix-Befunds 09). Sampling WRAPPT (kachelt) wie das Original.
 * beatReverse kehrt die Drehrichtung am Beat um (Ease ueber
 * beatReverseSpeed), beatZoomJump setzt das Zoom-fpos auf zoomScale2.
 */
struct RotoBlitterParams
{
    int zoomScale = 31;          ///< Zoom target value (31 = neutral)
    int zoomScale2 = 31;         ///< Beat zoom value (beatZoomJump)
    int rotDir = 31;             ///< theta = (rotDir-32) degrees per frame
    bool blend = false;          ///< BLEND_AVG with the original
    bool beatReverse = false;    ///< Beat reverses the rotation direction
    int beatReverseSpeed = 0;    ///< Ease of the direction change (0 = hard)
    bool beatZoomJump = false;   ///< Beat sets fpos = zoomScale2
    bool subpixel = true;        ///< bilinear sampling (BLEND4)

    /// Parameter script (Strang D): `zoomscale`, `zoomscale2`, `rotdir` + `b`/`w`/`h` +
    /// audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Misc / Buffer Save" (ID 18): copy the framebuffer to one of 8 global
 * buffers (save) or blend a stored buffer back (restore).
 */
struct BufferSaveParams
{
    int slot = 0;                          ///< global buffer index 0..7
    /// Direction (r_stack.cpp): 0 = save, 1 = restore,
    /// 2 = alternate starting with save, 3 = alternate starting with restore
    int dir = 0;
    BlendMode blend = BlendMode::Replace;  ///< blend mode (applies both directions)
    int adjustAlpha = 128;                 ///< Adjustable blend alpha 0..255

    /// Parameter script (Strang D): `slot`, `dir`, `adjustalpha` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Misc / Custom BPM" (ID 33): mutates the beat signal for the effects
 * that follow it in the chain (arbitrary interval / skip / invert).
 */
/**
 * AVS "Misc / Custom BPM" (ID 33) — ein Beat-FILTER (r_bpm.cpp:137-185).
 *
 * Die drei Betriebsarten schliessen einander aus: das Original prueft
 * `arbitrary`, dann `skip`, dann `invert` und kehrt aus jedem Zweig SOFORT
 * zurueck. Sie hintereinander anzuwenden ergibt Kombinationen, die es dort
 * nicht gibt (Befund S52).
 */
struct CustomBpmParams
{
    bool arbitrary = false;  ///< emit a beat every `arbitraryMs`
    int arbitraryMs = 500;   ///< interval for arbitrary mode
    bool skip = false;       ///< only pass every (skipCount+1)-th beat
    /// Raw value `skipVal` of the reference: every (skipCount+1)-th beat is
    /// passed through, so 0 means "every".
    int skipCount = 1;
    bool invert = false;     ///< invert the (possibly modified) beat
    /// `skipfirst`: the first N beats of the preset are swallowed, and the skip
    /// counter does NOT advance meanwhile (r_bpm.cpp:148 returns before the
    /// skip block).
    int skipFirst = 0;

    /// Parameter script (Strang D): `arbitraryms`, `skipcount`, `skipfirst` + `b`/`w`/`h` +
    /// audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Set Render Mode" (ID 40): a stateful command that sets the line
 * width + blend used by the render effects that FOLLOW it in the chain (it draws
 * nothing itself). Modelled as a live host node — at render time it writes the
 * host's current render mode, which the following scope effects read (the AVS
 * behaviour; previously import-time-unrolled into the next SuperScope). `enabled`
 * = AVS bit 31 (override the blend; when false the blend falls back to additive);
 * `lineWidth` px (0 = leave each effect's own); `lineBlend` = the RAW AVS
 * BLEND_LINE mode (r_defs.h:267-283, S9): 0 replace, 1 additive, 2 maximum,
 * 3 50/50, 4 sub(fb-c), 5 sub(c-fb), 6 multiply, 7 adjustable, 8 xor,
 * 9 minimum; `adjustAlpha` the Adjustable-blend alpha 0..255 (bits 8-15).
 */
struct SetRenderModeParams
{
    bool enabled = true;    ///< override the blend (AVS bit 31)
    int lineWidth = 1;      ///< line width for following scopes (px, 0 = leave)
    int lineBlend = 1;      ///< AVS BLEND_LINE mode 0..9 (S9)
    int adjustAlpha = 128;  ///< Adjustable-blend alpha 0..255
    /// Parameter script (Strang D, Block E S79): `linewidth`, `lineblend`,
    /// `adjustalpha`, `overrideblend` + `b`/`w`/`h` + audio set — e.g. the
    /// line width on the beat. Empty = no script (LumiViz extension, AVS
    /// has no code here).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / SuperScope" (ID 36): a scripted point/line scope. The point
 * script (EEL quartet) is run by a SuperscopeModule; the host draws the points
 * via the shared ScopeRenderer (decision E6). `renderMode` 0=dots 1=lines
 * 2=thick; `audioChannel` 0=L 1=R 2=mono 3=mid 4=side (SuperscopeAudioChannel).
 */
struct SuperScopeParams
{
    /// EEL quartet. init/frame/beat run once per setup/frame/beat and set,
    /// among other things, `n` (point count); the point code runs per point.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Runs per point: `i` 0..1 and `v` (audio) come in; `x`/`y` (-1..1),
    /// `red`/`green`/`blue`, `linesize`, `skip` and `drawmode` are read back.
    std::string pointCode;
    /// Start value for `n` — the point code runs this many times. A script that
    /// sets `n` itself wins.
    int pointCount = 256;
    int renderMode = 1;    ///< 0=dots, 1=lines, 2=thick lines
    /// Stroke width in pixels for `renderMode` 1/2; `linesize` from the
    /// point code wins.
    float lineWidth = 2.0f;
    /// Dot diameter in pixels for `renderMode` 0.
    float dotSize = 4.0f;
    int audioChannel = 2;  ///< 0=L 1=R 2=mono 3=mid 4=side
    /// AVS which_ch bit 4 (r_sscope.cpp:232): v reads SPECTRUM instead of waveform.
    bool spectrumSource = false;
    int lineBlend = 1;     ///< AVS BLEND_LINE mode 0..9 (S9; see SetRenderModeParams)
                           ///< (set by a preceding Set Render Mode; default additive)

    // Base color (point code that sets red/green/blue always overrides this).
    // Two orthogonal sources — a time-cycled AVS color table and a per-point
    // gradient — combined by `colorBlend`. Default (0 + Neon) = the historical
    // look, so existing chains render unchanged.
    int colorBlend = 0;  ///< 0 gradient, 1 table, 2 additive, 3 multiply, 4 average
    std::vector<uint32_t> colors;         ///< AVS color table (0x00RRGGBB), cycled
    int colorCycleFrames = 60;            ///< frames per table step (>= 1)
    std::string gradientPreset = "Neon";  ///< ColorGradientModule preset name
};

/**
 * AVS "Trans / Mosaic" (ID 30): pixelate into `quality`×`quality` blocks. On a
 * beat it can jump to `quality2` and ease linearly back to `quality` over
 * `durationFrames` (r_mosaic.cpp). quality 100 = no pixelation. `blend` mixes
 * the mosaic with the untouched image: 0 replace, 1 additive, 2 50/50.
 */
struct MosaicParams
{
    int quality = 50;         ///< block resolution 1..100 (100 = off)
    int quality2 = 50;        ///< on-beat resolution 1..100
    bool onBeat = false;      ///< jump to quality2 on beat, then ease back
    int durationFrames = 16;  ///< ease-back length in frames (>= 1)
    int blend = 0;            ///< 0 replace, 1 additive, 2 50/50

    /// Parameter script (Strang D): `quality`, `quality2` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Grain" (ID 24): darken a random subset of (non-black) pixels by
 * a random factor (r_grain.cpp). `amount` 0..100 = the gated fraction; `blend`
 * 0 replace, 1 additive, 2 50/50; `staticGrain` freezes the noise pattern.
 */
struct GrainParams
{
    int amount = 100;         ///< gated pixel fraction 0..100 (smax)
    bool staticGrain = false; ///< frozen noise vs. per-frame shimmer
    int blend = 0;            ///< 0 replace, 1 additive, 2 50/50

    /// Parameter script (Strang D): `amount` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Scatter" (ID 16): per-pixel random displacement within a small
 *  (~4 px) window (r_scat.cpp). No parameters. */
struct ScatterParams
{
};

/** AVS "Trans / Water" (ID 20): color-space water ripple — neighbour average of
 *  the current frame minus the previous frame (r_water.cpp). No parameters;
 *  needs a persistent per-node "last frame" buffer. */
struct WaterParams
{
};

/**
 * AVS "Trans / Water Bump" (ID 31): a height-field water simulation — waves
 * propagate on a persistent height buffer and refract the image (r_waterbump).
 * On a beat a drop is added (`randomDrop` = random spot, else `dropX/dropY`
 * position code 0/1/2 = near/mid/far). `density` damps the waves, `depth` sets
 * the drop strength. `displaceScale` tunes the refraction (sight-test).
 */
struct WaterBumpParams
{
    int density = 5;        ///< wave damping (higher = longer-lived waves)
    int depth = 600;        ///< drop amplitude
    bool randomDrop = true; ///< random drop spot vs. fixed dropX/dropY
    int dropX = 1;          ///< 0 near / 1 mid / 2 far (position code)
    int dropY = 1;          ///< same vertically: 0 top / 1 middle / 2 bottom
    int dropRadius = 40;    ///< drop radius (px)
    /// Gain of the displacement. Since the switch to integers (S59) the heights
    /// are in REFERENCE units — 1.0 = exactly r_waterbump; the old value 6.0
    /// came from the normalized era and distorted 6-fold.
    float displaceScale = 1.0f;

    /// Parameter script (Strang D): `density`, `depth`, `dropradius`, `displacescale` +
    /// `b`/`w`/`h` + audio set. Empty = no script.
    /// More fields via script (Block D, S79): `dropx`, `dropy`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Bump" (ID 29): per-pixel bump lighting from the image luminance
 * gradient, lit by a movable light source (r_bump.cpp). The light position
 * `x,y` comes from EEL (init/frame/beat, `x,y` output; `t` script-owned). On a
 * beat `depth` can jump to `depth2` for `durationFrames`. `blend` 0 replace,
 * 1 additive, 2 50/50. `oldStyle` scales script x,y by 1/100 (legacy presets).
 */
struct BumpParams
{
    int depth = 30;            ///< bump strength 0..100
    int depth2 = 100;          ///< on-beat strength
    bool onBeat = false;       ///< jump to depth2 on beat, ease back
    int durationFrames = 15;   ///< ease-back length
    bool invert = false;       ///< invert the depth (luminance)
    bool oldStyle = false;     ///< legacy x,y in 0..100 instead of 0..1
    int blend = 0;             ///< 0 replace, 1 additive, 2 50/50
    int buffern = 0;           ///< depth source: 0 = frame, N = global buffer N-1
    /// EEL slots. They set `x`/`y` — the light source in 0..1 (with `oldStyle`
    /// in 0..100) — and share their variables.
    // Vorgabe LEER — der Deserialisierer liess diese Slots beim Laden schon
    // immer leer (`getStr` ohne Vorgabewert), und seit S56 ist der Struct die
    // einzige Quelle. Die frueheren Demo-Skripte gehoeren als VOREINSTELLUNG
    // hierher (Konzept §11), nicht als Vorgabe: ein importiertes Preset darf
    // sie nicht erben. Sie stehen in `Offene_Punkte.md §1d`.
    /// More fields via script (Block D, S79): `depth`, `depth2`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Trans / Interferences" (ID 41): `points` rotated copies of the image
 * accumulated with `alpha` (r_interf.cpp). The rotation advances by
 * `rotationInc`/frame; on a beat it morphs to the *2 parameter set over
 * `speed`. `rgb` splits copies across R/G/B channels. `blend` 0 replace,
 * 1 additive, 2 50/50 with the original.
 */
struct InterferencesParams
{
    int points = 2;          ///< number of copies 1..8
    int distance = 10;       ///< copy offset radius (px)
    int alpha = 128;         ///< per-copy weight 0..255
    int rotation = 0;        ///< initial rotation 0..255 (of a full turn)
    int rotationInc = 0;     ///< rotation delta per frame
    int distance2 = 32;      ///< on-beat target distance
    int alpha2 = 192;        ///< on-beat target alpha
    int rotationInc2 = 25;   ///< on-beat target rotation delta
    bool rgb = false;        ///< split copies across channels
    bool onBeat = false;     ///< morph to the *2 set on a beat
    float speed = 0.2f;      ///< beat-morph transition speed
    int blend = 0;           ///< 0 replace, 1 additive, 2 50/50

    /// Parameter script (Strang D): `points`, `distance`, `alpha`, `rotation`, `rotationinc`,
    /// `speed` + `b`/`w`/`h` + audio set. Empty = no script.
    /// More fields via script (Block D, S79): `distance2`, `alpha2`, `rotationinc2`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Starfield" (ID 27): a 3D star field flying towards the viewer
 * (r_stars.cpp). Stars move by `warpSpeed` (jumping to `beatSpeed` for
 * `durationFrames` on a beat); brightness rises as they approach. `color`
 * tints them. Drawn additively via the shared ScopeRenderer.
 */
/**
 * Community APE "FunkyFX FyrewurX v1" (closed source) — behavioral REBUILD, no
 * original code: on-beat firework bursts of gravity-bound sparks, drawn as
 * additive dots. The original's config word was never varied in the wild (all
 * 146 corpus instances carry identical bytes), so the parameters here are our
 * own, sight-calibrated ones.
 */
/**
 * APE "Metaballs 3D" (UnConeD) — **Verhaltens-Nachbau** wie FyrewurX (S38).
 *
 * Die APE ist closed-source und ihr Blob traegt nur eine Farbtafel (S52); die
 * Geometrie ist deshalb host-eigen. Nachgebaut wird das, was der Effekt tut:
 * mehrere Kugeln wandern durch einen Raum, ihr summiertes 1/r²-Feld wird
 * geschwellt und aus der Preset-Palette eingefaerbt.
 */
struct Metaballs3DParams
{
    /// Palette (0x00RRGGBB). NOT empty: the reader heals an empty table to
    /// white, so an empty one could not be saved at all (roundtrip finding
    /// S56). The default therefore states it explicitly.
    std::vector<uint32_t> colors{0xFFFFFF};
    int count = 7;                 ///< Number of spheres (1..16)
    /// Radius scale per sphere (NDC) — visually calibrated against the real APE
    /// (S52): 0.30 covered clearly more area than the reference.
    float radius = 0.20f;
    float speed = 0.45f;           ///< Orbit speed
    float threshold = 1.0f;        ///< Iso value of the surface
    /// 0 replace, 1 additive, 2 50/50 — the reference draws OPAQUE
    /// bodies (visual comparison S52), hence replace as the default.
    int blend = 0;

    // Freigemachte Bahn-Konstanten (S53), Vorgaben = bisheriges Verhalten.
    float spread = 1.0f;  ///< Factor on the orbit width (was fixed 1)
    float depth = 1.2f;   ///< Camera distance of the orbit center (was fixed 1.2)
    float phase = 1.7f;   ///< Phase offset per sphere (was fixed 1.7)

    /// Parameter script (Strang D): `count`, `radius`, `speed`, `threshold`,
    /// `spread`, `depth`, `phase` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * APE "Tentacles 3D" (UnConeD) — **Verhaltens-Nachbau**, gleiche Lage wie oben.
 * Mehrere Tentakel schwingen aus der Bildmitte nach aussen; je Tentakel eine
 * Farbe aus der Palette, die Dicke nimmt zur Spitze hin ab.
 */
struct Tentacles3DParams
{
    /// Palette (0x00RRGGBB). NOT empty: the reader heals an empty table to
    /// white, so an empty one could not be saved at all (roundtrip finding
    /// S56). The default therefore states it explicitly.
    std::vector<uint32_t> colors{0xFFFFFF};
    int count = 7;                 ///< Number of tentacles (1..16)
    int segments = 28;             ///< Control points per tentacle
    float length = 0.85f;          ///< Length in NDC
    float thickness = 9.0f;        ///< Line width at the root (pixels)
    float speed = 0.7f;            ///< Sway speed
    int blend = 1;                 ///< 0 replace, 1 additive, 2 50/50

    // Freigemachte Schwing-Konstanten (S53), Vorgaben = bisheriges Verhalten.
    float sway = 0.9f;    ///< Sway amplitude at the tip (was fixed 0.9)
    float waves = 3.1f;   ///< Waves along the length (was fixed 3.1)
    float taper = 1.0f;   ///< Taper toward the tip; 0 = uniform thickness

    /// Parameter script (Strang D): `count`, `segments`, `length`, `thickness`,
    /// `speed`, `sway`, `waves`, `taper` + `b`/`w`/`h` + audio set.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

struct FyrewurXParams
{
    int sparks = 80;           ///< sparks per burst
    float speed = 0.7f;        ///< initial spark speed scale (NDC/s)
    float gravity = 0.8f;      ///< downward pull (NDC/s^2)
    float lifeSeconds = 1.6f;  ///< spark lifetime

    // Freigemachte Konstanten (S53), Vorgaben = bisheriges Verhalten.
    float dotSize = 2.0f;      ///< Spark size in pixels (was fixed 2)
    float hueDrift = 0.6f;     ///< Color spread per spark (was fixed 0.6)
    float burstSpread = 1.0f;  ///< Factor on the spread of the burst center

    /// Parameter script (Strang D): `sparks`, `speed`, `gravity`, `life`,
    /// `dotsize`, `huedrift`, `burstspread` + `b`/`w`/`h` + audio set.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

struct StarfieldParams
{
    uint32_t color = 0xFFFFFF;   ///< tint 0x00RRGGBB
    float warpSpeed = 6.0f;      ///< base fly-through speed
    int maxStars = 350;          ///< star count
    bool onBeat = false;         ///< jump to beatSpeed on a beat
    float beatSpeed = 4.0f;      ///< on-beat speed
    int durationFrames = 15;     ///< ease-back length
    int blend = 0;               ///< 0 replace, 1 additive, 2 50/50 (r_stars)

    /// Parameter script (Strang D): `maxstars`, `warpspeed`, `beatspeed`, `durationframes` +
    /// `b`/`w`/`h` + audio set. Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS "Render / Timescope" (ID 39): a scrolling spectrogram — one spectrum
 * column is drawn per frame at an advancing x, building up over time
 * (r_timescope.cpp). `color` tints it; `bands` sets the vertical resolution;
 * `blend` 0 replace, 1 additive, 2 50/50 with the existing image.
 */
struct TimescopeParams
{
    uint32_t color = 0xFFFFFF;  ///< tint 0x00RRGGBB
    /// 0 replace, 1 additive, 2 50/50, 3 BLEND_LINE (follows the Set Render Mode
    /// state — AVS default "Default Blend", r_timescope.cpp:147-148, S3)
    int blend = 3;
    int channel = 2;           ///< 0 L, 1 R, 2 center
    int bands = 576;           ///< vertical spectrum resolution
    /**
     * Actually apply `channel` (extension, default OFF = faithful to the reference).
     *
     * r_timescope.cpp does compute an `fa_data` from `which_ch`, but the
     * drawing loop then reads a fixed `visdata[0][0]` — the left spectrum
     * channel; `fa_data` is never used (S48 matrix finding 39). So the control
     * sits in the original dialog without doing anything.
     *
     * With `false` it stays exactly that way, so that imported presets (which
     * mostly carry `which_ch=2`) keep looking like the reference. With
     * `true`, `channel` counts: 0 left, 1 right, 2 mean of both.
     *
     * Deliberately placed AFTER `bands`: the aggregate initialization
     * `TimescopeParams{color, blend, channel, bands}` is in use in tests and
     * the translator and should stay valid.
     */
    bool useChannel = false;

    /// Parameter script (Strang D): `bands`, `channel`, `blend` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Render / Dot Grid" (ID 17): a scrolling grid of dots whose colour cycles
 *  through `colors` (r_dotgrid.cpp). `spacing` px, `xMove/yMove` scroll speed. */
struct DotGridParams
{
    /// Color table (0x00RRGGBB). Multiple entries are cycled through across
    /// frames, like the AVS color table.
    std::vector<uint32_t> colors{0xFFFFFF};
    int spacing = 8;   ///< grid spacing (px)
    int xMove = 128;   ///< horizontal scroll (fixed-point /256 per frame)
    int yMove = 128;   ///< vertical scroll
    /// 0 replace, 1 additive, 2 50/50, 3 BLEND_LINE (SRM state,
    /// r_dotgrid.cpp:151-159, S3)
    int blend = 0;

    /// Parameter script (Strang D): `spacing`, `xmove`, `ymove`, `blend` + `b`/`w`/`h` +
    /// audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Render / Dot Plane" (ID 1): a rotating audio-reactive point plane, height
 *  from the spectrum, coloured by a 5-stop gradient (r_dotpln.cpp). 3D projection
 *  scale is host tuning (sight-test). */
struct DotPlaneParams
{
    /// Five color-gradient stops (0x00RRGGBB), mapped over the height of the grid points.
    uint32_t colors[5] = {0x0000FF, 0x00FFFF, 0x00FF00, 0xFFFF00, 0xFF0000};
    int rotVel = 16;   ///< rotation speed (-50..50 in AVS)
    int angle = -20;   ///< viewing tilt angle
    /// Initial rotation angle in degrees — the original SAVES its running rotation in the preset
    /// (`r = rr/32`, r_dotpln load_config) and therefore starts in the saved position on load (Tie
    /// Tunnel: 44.84°).
    float startRotation = 0.0f;

    // KLASSE A (Knoten_Parameter_Konzept §2): die Vorgaben sind die Werte aus
    // `r_dotpln.cpp` — eine Abweichung entfernt das Bild MESSBAR von der
    // Referenz. Der Editor kennzeichnet sie deshalb (Entscheid Patrik §8.4).
    // Das 64x64-Gitter bleibt fest: es ist die Struktur der Portierung, kein
    // Parameter (NUM_WIDTH, steckt in Stack-Arrays).
    float camDistance = 400.0f;  ///< matrixTranslate z (original 400)
    float settle = 0.15f;        ///< Sinking per frame: `vel -= settle*h/255`

    /// Parameter script (Strang D): `rotvel`, `angle`, `camdistance`, `settle`. WARNING class A — a
    /// script here leaves the reference; the editor says so, because the ⚠ markers on the sliders
    /// can only check fixed values.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Render / Dot Fountain" (ID 19): a 3D particle fountain coloured by a
 *  5-stop gradient, rotating (r_dotfnt.cpp). Simplified particle model here;
 *  projection/physics scale is host tuning (sight-test). */
struct DotFountainParams
{
    /// Five color-gradient stops (0x00RRGGBB), mapped over the height of the particles.
    uint32_t colors[5] = {0x0000FF, 0x00FFFF, 0x00FF00, 0xFFFF00, 0xFF0000};
    /// Rotation speed (sign = direction).
    int rotVel = 16;
    /// View tilt in degrees.
    int angle = -20;
    /// Initial rotation angle in degrees — like Dot Plane, the original saves its running rotation
    /// in the preset (`r = rr/32`, r_dotfnt load_config).
    float startRotation = 0.0f;

    /// Parameter script (Strang D): `rotvel`, `angle` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Misc: Buffer blend": combine two buffers (A and B) with a blend mode
 * and output the result (community APE, format per grandchild/AVS-File-Decoder).
 * `bufferA`/`bufferB` select a global pool slot 0..7 or 8 = CURRENT frame; `mode`
 * 0..10 (replace/additive/max/50-50/sub d-s/sub s-d/multiply/adjustable/xor/min/
 * abs-diff). Uses the shared OffscreenBufferPool (same slots as Buffer Save).
 */
struct BufferBlendParams
{
    int bufferA = 8;  ///< 0..7 pool slot, 8 = current frame
    int bufferB = 8;  ///< 0..7 pool slot, 8 = current frame
    int mode = 0;     ///< BufferBlendMode 0..10

    /// Parameter script (Strang D): `buffera`, `bufferb`, `mode` + `b`/`w`/`h` + audio set. Empty =
    /// no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Jheriko: Global": runs EEL that sets preset-global registers/gmegabuf,
 * persisting them across frames for other effects (community APE). Init/frame/beat
 * code shares this preset's ScriptContext. `loadMode` 0 none, 1 once, 2 code, 3
 * every-frame. File I/O + save-ranges are not imported. No visual output.
 */
struct JherikoGlobalParams
{
    int loadMode = 1;  ///< 0 none, 1 once, 2 code-control, 3 every-frame
    /// EEL slots without an image of their own: the node only computes and stores its results in
    /// the shared variables (`reg00`..`reg99`, `gmegabuf`), which the following nodes read.
    /// `loadMode` controls when the init slot runs again.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Holden03: Convolution Filter": a 7x7 convolution kernel over the image
 * (community APE). `edgeMode` 0 extend (clamp), 1 wrap. `absolute` takes |result|.
 * `twoPass` applies the kernel twice. `kernel` is 49 ints (row-major); the sum is
 * divided by `scale` (0 -> 1) and offset by `bias`.
 */
struct ConvolutionParams
{
    /// Take the absolute value of the result — negative sums are mirrored instead of clamped to 0
    /// (makes edge kernels visible on both sides).
    bool absolute = false;
    /// Apply the kernel twice in succession.
    bool twoPass = false;
    int edgeMode = 0;   ///< 0 extend, 1 wrap
    /// Constant added to each channel after the division.
    int bias = 0;
    /// Divisor of the weighted sum (0 is treated as 1). Entering the sum of the kernel values here
    /// preserves the brightness.
    int scale = 1;
    std::array<int, 49> kernel = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  ///< identity (center=1)

    /// Parameter script (Strang D): `bias`, `scale`, `edgemode` + `b`/`w`/`h` + audio set. Empty =
    /// no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Trans: Normalise": auto-levels — stretch the image so the darkest
 * pixel becomes black and the brightest white (community APE). No parameters
 * (enabled maps to the node's enabled flag). Min/max are measured on the GPU
 * frame each render.
 */
struct NormaliseParams
{
};

/**
 * AVS APE "Jheriko : MULTIFILTER" (note the space before the colon): applies one
 * of a few fixed pixel filters (community APE, closed source — the exact math is
 * approximated). `effect` 0 chrome, 1 double, 2 triple, 3 infinite-root. `onBeat`
 * only applies on beat frames.
 */
struct MultiFilterParams
{
    int effect = 0;       ///< 0 chrome, 1 double chrome, 2 triple chrome, 3 root
    bool onBeat = false;  ///< apply only on beat frames

    /// Parameter script (Strang D): `effect` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Virtual Effect: Addborders": draw a solid border of `color` and
 * `size` pixels around the image (community APE).
 */
struct AddBordersParams
{
    uint32_t color = 0xFFFFFF;  ///< border color 0x00RRGGBB
    int size = 2;               ///< border width (px)

    /// Parameter script (Strang D): `size` + `b`/`w`/`h` + audio set. Empty = no script.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Texer": draw an embedded image (sprite) at points along the waveform
 * (community APE, no EEL). `blend` 0 replace, 1 additive, 2 50/50; `particles`
 * = number of sprites. Sprite geometry is sight-test-calibrated.
 */
struct TexerParams
{
    /// Provenance note from the preset. `imageData` is what gets drawn; nothing is ever loaded from
    /// this path.
    std::string filename;
    /// The embedded image (base64). Empty = built-in default sprite.
    std::string imageData;
    /// 0 replace, 1 additive, 2 50/50.
    int blend = 1;
    /// Number of sprites along the waveform.
    int particles = 100;

    /// Parameter script (Strang D): `blend`, `particles` + `b`/`w`/`h` + audio set. Empty = no
    /// script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * AVS APE "Acko.net: Texer II": draw an embedded image at each point of a scoped
 * point loop (community APE). The point EEL sets, per point, `x,y` (-1..1),
 * `sizex,sizey` (scale), `red,green,blue` (tint when colorFiltering); `n` = point
 * count. EEL variable contract + sprite geometry are sight-test-calibrated.
 */
struct TexerIIParams
{
    /// Provenance note from the preset. `imageData` is what gets drawn; nothing is ever loaded from
    /// this path.
    std::string filename;
    /// The embedded image (base64). Empty = built-in default sprite (20x20, radially symmetric —
    /// measured against the real texer2.ape).
    std::string imageData;
    /// Apply `sizex`/`sizey` from the point code (off: fixed size).
    bool resizing = false;
    /// Wrap sprites at the image edge around to the opposite side instead of clipping them.
    bool wrapAround = false;
    /// Apply `red`/`green`/`blue` from the point code as a color filter on the image (off: the
    /// image is drawn unchanged).
    bool colorFiltering = true;
    /// EEL quartet. init/frame/beat run once per setup/frame/beat and set `n` (number of sprites);
    /// the point code runs per sprite.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Runs per sprite: `i` 0..1 comes in; read back are `x`/`y` (-1..1), `sizex`/`sizey` (only
    /// with `resizing`) and `red`/`green`/`blue` (only with `colorFiltering`).
    // Vorgabe LEER — der Deserialisierer liess diese Slots beim Laden schon
    // immer leer (`getStr` ohne Vorgabewert), und seit S56 ist der Struct die
    // einzige Quelle. Die frueheren Demo-Skripte gehoeren als VOREINSTELLUNG
    // hierher (Konzept §11), nicht als Vorgabe: ein importiertes Preset darf
    // sie nicht erben. Sie stehen in `Offene_Punkte.md §1d`.
    std::string pointCode;
};

/**
 * AVS APE "Render: Triangle": draw EEL-scripted filled triangles. The point EEL
 * sets, per triangle, `x1,y1,x2,y2,x3,y3` (-1..1) and `red,green,blue`; `n` =
 * triangle count. EEL variable contract is sight-test-calibrated.
 */
struct TriangleParams
{
    /// EEL quartet. init/frame/beat run once per setup/frame/beat and set `n` (number of
    /// triangles); the point code runs per triangle.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Runs per triangle: read back are `x1`/`y1`, `x2`/`y2`, `x3`/`y3` (-1..1) and
    /// `red`/`green`/`blue`.
    std::string pointCode;

    /// FILLED is the reference state (probe `triangle_n_literal`, S51: the former wireframe
    /// produced 4009 instead of 23424 pixels). `false` is therefore a deliberate deviation from
    /// AVS, not an alternative default.
    bool filled = true;
    float lineWidth = 1.0f;  ///< edge width in wireframe mode
};

/**
 * AVS APE "Color Map": map a per-pixel input value (selected channel) through a
 * gradient LUT, then blend the result onto the image (UnConeD, community APE —
 * format per grandchild/AVS-File-Decoder). `key` picks the input (0 red, 1 green,
 * 2 blue, 3 (r+g+b)/2, 4 max, 5 (r+g+b)/3). `blendMode` 0..9 (replace/additive/
 * max/min/50-50/sub d-s/sub s-d/multiply/xor/adjustable). The gradient is a list
 * of stops (position 0..255 + 0x00RRGGBB); the host interpolates a 256-entry LUT.
 * On-beat map cycling (mapCycleMode) is not imported yet — the first enabled map
 * is used.
 */
struct ColorMapParams
{
    int key = 0;              ///< input selector 0..5
    int blendMode = 0;        ///< 0..9 (see above)
    int adjustBlend = 128;    ///< ADJUSTABLE weight 0..255
    std::vector<int> stopPos;         ///< gradient stop positions 0..255
    std::vector<uint32_t> stopColor;  ///< gradient stop colors 0x00RRGGBB

    /// Parameter script (Strang D): `key`, `blendmode`, `adjustblend` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS APE "Channel Shift": permute the R/G/B channels (r_chanshift). `mode`
 *  0 RGB, 1 RBG, 2 GBR, 3 GRB, 4 BRG, 5 BGR; `onBeat` picks a random one each beat. */
struct ChannelShiftParams
{
    int mode = 1;         ///< channel permutation 0..5
    bool onBeat = false;  ///< random permutation on each beat

    /// Parameter script (Strang D): `mode` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS APE "Color Reduction": quantise each channel to 2^`levels` values
 *  (r_colorreduction). levels 1..8 (8 = unchanged). */
struct ColorReductionParams
{
    int levels = 8;  ///< bit depth per channel 1..8

    /// Parameter script (Strang D): `levels` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS APE "Multiplier": scale pixel values (r_multiplier). `mode` 0 saturate,
 *  1 x8, 2 x4, 3 x2, 4 x0.5, 5 x0.25, 6 x0.125, 7 zero-else-keep. */
struct MultiplierParams
{
    int mode = 3;  ///< 0..7 (see above)

    /// Parameter script (Strang D): `mode` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS APE "Video Delay" (Holden04): output the image from `delay` frames ago
 *  via a per-node frame ring buffer (r_videodelay). `useBeats` measures the
 *  delay in beats instead of frames. */
struct VideoDelayParams
{
    bool useBeats = false;  ///< delay in beats vs frames
    int delay = 10;         ///< delay amount (frames, capped for VRAM)

    /// Parameter script (Strang D): `delay` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS APE "Multi Delay" (Holden05): one of 6 host-shared frame ring buffers
 *  (r_multidelay). `mode` 0 none, 1 input (write this frame), 2 output (read the
 *  delayed frame). `buffer` 0..5 selects the shared ring; `delay`/`useBeats` set
 *  its length (input nodes size the ring). */
struct MultiDelayParams
{
    int mode = 0;           ///< 0 none, 1 input, 2 output
    int buffer = 0;         ///< shared buffer index 0..5
    int delay = 10;         ///< delay length
    bool useBeats = false;  ///< delay in beats vs frames

    /// Parameter script (Strang D): `delay`, `buffer` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Host-only debug leaf (no AVS counterpart): audio-reactive quad orbiting the
 * center. Exists so the 5.1 chain has a visible content source before the real
 * scope renderer arrives (step 5.4, decision E6).
 */
struct DebugBarsParams
{
    uint32_t color = 0xFF80FF;  ///< 0x00RRGGBB
    float orbitSpeed = 1.0f;    ///< revolutions factor (1 = one turn per ~6.3 s)
};

/**
 * Batch H (host-native, no AVS origin): an escape-time 2-D fractal generator.
 * A fullscreen fragment shader evaluates the iteration per pixel and colours it
 * through a gradient LUT (smooth iteration count). `type` selects the formula
 * (Mandelbrot, Julia, Burning Ship, Tricorn, Multibrot, Newton, Phoenix, Magnet,
 * Nova). The optional EEL slots (init/frame/beat) drive the view live: they read
 * audio (bass/mid/treble/vol, beat, time) and write cx,cy,zoom,rot,jx,jy,power —
 * so a preset can zoom, morph the Julia seed, spin or pulse to the beat. `blend`
 * composites the result over the current image (0 replace, 1 additive, 2 50/50).
 */
struct Fractal2DParams
{
    int type = 0;           ///< 0 Mandelbrot 1 Julia 2 Burning Ship 3 Tricorn
                            ///< 4 Multibrot 5 Newton 6 Phoenix 7 Magnet 8 Nova
    float centerX = -0.5f;  ///< view centre (real part)
    float centerY = 0.0f;   ///< view centre (imag part)
    float zoom = 1.0f;      ///< view scale (>1 magnifies)
    float rotation = 0.0f;  ///< view rotation (radians)
    int maxIter = 128;      ///< escape-time iteration cap
    float juliaX = -0.8f;   ///< Julia / Phoenix seed (real)
    float juliaY = 0.156f;  ///< Julia / Phoenix seed (imag)
    float power = 2.0f;     ///< Multibrot / Nova exponent
    float escapeR = 4.0f;   ///< escape radius (|z|^2 threshold)

    bool smooth = true;               ///< continuous (fractional) iteration coloring
    float colorScale = 0.05f;         ///< iteration→LUT position factor
    float colorCycle = 0.0f;          ///< LUT offset advance per second (palette drift)
    uint32_t insideColor = 0x000000;  ///< color of the non-escaping set (0x00RRGGBB)
    std::string gradientPreset = "Neon";  ///< ColorGradientModule palette name

    int blend = 0;  ///< 0 replace, 1 additive, 2 50/50 over the current image

    /// Color palette via script (Block C, S79): `colorscale`, `colorcycle`, `colorphase`,
    /// `insidecolor_r`, `insidecolor_g`, `insidecolor_b`.
    /// More fields via script (Block D, S79): `maxiter`, `escaper`.
    std::string initCode;   ///< EEL, once (seed cx,cy,zoom,…)
    std::string frameCode;  ///< EEL, per frame (audio-reactive view)
    std::string beatCode;   ///< EEL, on beat
};

/**
 * Batch H (host-native): a domain-warped fBm field — the "plasma / nebula" look.
 * A fullscreen shader sums `octaves` of value noise (fBm), warps the sample point
 * by two further fBm fields (`warp` strength), animates it over time (`speed`) and
 * colours the result through the shared gradient LUT. Cheap and very audio-reactive:
 * the optional EEL slots read audio (bass/mid/treble/vol/beat/time) and write
 * scale,warp,speed,ox,oy. `blend` composites over the current image.
 */
struct DomainWarpParams
{
    int octaves = 5;          ///< fBm octave count (1..10)
    float lacunarity = 2.0f;  ///< frequency multiplier per octave
    float gain = 0.5f;        ///< amplitude falloff per octave
    float scale = 3.0f;       ///< base spatial frequency
    float warp = 0.5f;        ///< domain-warp strength
    float warpScale = 1.0f;   ///< warp-field frequency relative to base
    float speed = 0.2f;       ///< time-evolution speed
    float offsetX = 0.0f;     ///< pan (x)
    float offsetY = 0.0f;     ///< pan (y)

    float colorScale = 1.0f;  ///< field→LUT position factor
    float colorCycle = 0.0f;  ///< LUT offset advance per second
    std::string gradientPreset = "Neon";  ///< ColorGradientModule palette

    int blend = 0;  ///< 0 replace, 1 additive, 2 50/50 over the current image

    /// Color palette via script (Block C, S79): `colorscale`, `colorcycle`, `colorphase`.
    /// More fields via script (Block D, S79): `warpscale`, `lacunarity`, `gain`, `octaves`.
    std::string initCode;   ///< EEL, once
    std::string frameCode;  ///< EEL, per frame (audio-reactive field)
    std::string beatCode;   ///< EEL, on beat
};

/**
 * Batch H (host-native): a raymarched 3-D distance-estimator fractal. A fullscreen
 * shader marches a ray per pixel through a signed-distance field and shades the hit
 * with a normal-lit gradient. `type` selects the field (Mandelbulb, Mandelbox,
 * Menger sponge, Quaternion-Julia, KIFS). EEL (init/frame/beat) reads audio and
 * writes yaw,pitch,dist,power,scale,fold for live camera / morph. `blend`
 * composites over the current image.
 */
struct Fractal3DParams
{
    int type = 0;         ///< 0 Mandelbulb 1 Mandelbox 2 Menger 3 Quaternion-Julia 4 KIFS
    float yaw = 0.6f;     ///< camera orbit azimuth (radians)
    float pitch = 0.3f;   ///< camera orbit elevation (radians)
    float dist = 3.2f;    ///< camera distance from origin
    float fov = 1.0f;     ///< field-of-view scale
    float power = 8.0f;   ///< Mandelbulb exponent
    float scale = 2.0f;   ///< Mandelbox / KIFS scale
    float fold = 1.0f;    ///< Mandelbox fold limit
    int maxSteps = 96;    ///< raymarch step cap
    int maxIter = 8;      ///< DE iteration count
    float juliaX = 0.2f;  ///< Quaternion-Julia seed
    /// Quaternion-Julia constant, second component — only with `type = 3`.
    float juliaY = 0.3f;
    /// Quaternion-Julia constant, third component — only with `type = 3`.
    float juliaZ = 0.1f;
    /// Quaternion-Julia constant, fourth component — only with `type = 3`.
    float juliaW = 0.0f;
    float lightYaw = 0.7f;    ///< key-light direction (azimuth)
    float lightPitch = 0.8f;  ///< key-light direction (elevation)
    float ambient = 0.2f;     ///< ambient floor
    bool ao = true;           ///< cheap ambient occlusion

    /// Coloring: palette index = `fract(Trefferwert * colorScale + Phase)`.
    float colorScale = 1.0f;
    /// Color phase advance per second (0 = static colors).
    float colorCycle = 0.0f;
    /// Name of the color gradient used to color the surface.
    std::string gradientPreset = "Neon";
    uint32_t background = 0x000000;  ///< miss color (0x00RRGGBB)

    /// Onto the existing image: 0 replace, 1 additive (clamped), 2 50/50.
    int blend = 0;
    /// Parameter script (Strang D): `yaw`, `pitch`, `dist`, `power`, `scale`, `fold` + audio set —
    /// camera and shape live. Empty = no script.
    /// Color palette via script (Block C, S79): `colorscale`, `colorcycle`, `colorphase`,
    /// `background_r`, `background_g`, `background_b`.
    /// More fields via script (Block D, S79): `fov`, `juliax`, `juliay`, `juliaz`, `juliaw`,
    /// `lightyaw`, `lightpitch`, `ambient`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): the Markus-Lyapunov fractal ("Zircon Zity"). For each
 * point of an (a,b) plane the logistic map is iterated with r alternating between
 * a and b along the `sequence` string (e.g. "AB"), and the Lyapunov exponent is
 * accumulated: negative (stable/ordered) zones get `negColor`, positive (chaotic)
 * zones map through the gradient. EEL writes the view rectangle (aMin/aMax/bMin/bMax).
 */
struct LyapunovParams
{
    std::string sequence = "AB";  ///< a/b pattern (A/B chars; others ignored)
    float aMin = 2.5f;            ///< view rectangle (a axis)
    /// Right edge of the view on the a axis.
    float aMax = 4.0f;
    float bMin = 2.5f;            ///< view rectangle (b axis)
    /// Top edge of the view on the b axis.
    float bMax = 4.0f;
    int warmup = 100;            ///< settle iterations before measuring
    int iterations = 400;        ///< measured iterations
    uint32_t negColor = 0x000030;  ///< ordered-zone color (negative exponent)

    /// Coloring: palette index = `fract(Lyapunov-Exponent * colorScale + Phase)`.
    /// Applies only to the chaotic zones; the ordered ones get `negColor`.
    float colorScale = 1.0f;
    /// Color phase advance per second (0 = static colors).
    float colorCycle = 0.0f;
    /// Name of the color gradient for the chaotic zones.
    std::string gradientPreset = "Fire";

    /// Onto the existing image: 0 replace, 1 additive (clamped), 2 50/50.
    int blend = 0;
    /// Parameter script (Strang D): `aMin`, `aMax`, `bMin`, `bMax` — the view — + audio set. Empty
    /// = no script.
    /// Color palette via script (Block C, S79): `colorscale`, `colorcycle`, `colorphase`,
    /// `negcolor_r`, `negcolor_g`, `negcolor_b`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a hyperbolic {p,q} tiling on the Poincaré disk, rendered
 * by repeated inversion into the fundamental domain. `morph` animates the tiling,
 * `zoom`/`rotation` frame it. Colour comes from the reflection count through the
 * gradient LUT. EEL writes p,q (as floats),morph,zoom,rotation.
 */
struct KleinianParams
{
    int p = 5;            ///< polygon sides
    int q = 4;            ///< polygons per vertex (1/p + 1/q < 1/2 → hyperbolic)
    int iterations = 30;  ///< inversion iteration cap
    float morph = 0.0f;   ///< tiling morph phase
    /// Visible section of the Poincaré disc (larger = closer in).
    float zoom = 1.0f;
    /// Rotation of the tiling in radians.
    float rotation = 0.0f;

    /**
     * Coloring: palette index = `fract(Spiegelungszahl * colorScale + Phase)`.
     *
     * The reflection count is a WHOLE number — an integer `colorScale` therefore
     * makes the expression exactly 0 for EVERY cell, and the node draws a
     * single-colored disc instead of a tiling. Exactly that was the default (1.0)
     * until S56, and it made the whole effect invisible: measured, 1.0 and 4.0
     * give the same image pixel for pixel (MAE 0.0000); only 0.17 shows the
     * {p,q} tiling. Five other fields (`p`, `q`, `morph`, `iterations`,
     * `rotation`) therefore appeared "mute" — they change a structure that
     * could not be seen.
     *
     * The value 0.17 runs the color once through the palette over roughly six
     * reflection levels; that is the depth that occurs in the image.
     * Integer values remain adjustable — here they are simply the
     * "everything in one color" setting.
     */
    float colorScale = 0.17f;
    /// Color phase advance per second (0 = static colors).
    float colorCycle = 0.0f;
    /// Name of the gradient onto which the reflection count is mapped.
    std::string gradientPreset = "Neon";

    /// Onto the existing image: 0 replace, 1 additive (clamped), 2 50/50.
    int blend = 0;
    /// Parameter script (Strang D): `p`, `q` (as numbers), `morph`, `zoom`,
    /// `rot` (= `rotation`) + audio set. Empty = no script.
    /// Color palette via script (Block C, S79): `colorscale`, `colorcycle`,
    /// `colorphase`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): an endless-zoom trip — an escape-time fractal whose zoom
 * (and optional rotation) advances every frame, blended with the previous frame
 * (`feedback`) for motion trails. `type` 0 Mandelbrot 1 Julia 2 Burning Ship. EEL
 * writes centerX,centerY (zoom target),zoomSpeed,rotationSpeed.
 */
struct FractalZoomerParams
{
    int type = 0;            ///< 0 Mandelbrot 1 Julia 2 Burning Ship
    float centerX = -0.743643887f;  ///< zoom target (a classic Misiurewicz point)
    /// Zoom target, imaginary part (counterpart of `centerX`).
    float centerY = 0.131825904f;
    /// Julia constant, real part — only effective with `type = 1`.
    float juliaX = -0.8f;
    /// Julia constant, imaginary part — only effective with `type = 1`.
    float juliaY = 0.156f;
    /// Upper limit of iterations per pixel. Higher means more structure deep in
    /// the zoom, but also more computing time per frame.
    int maxIter = 200;
    float zoomSpeed = 1.02f;      ///< per-frame zoom factor (>1 zooms in)
    float rotationSpeed = 0.0f;   ///< radians per frame
    /// Share of the previous image that stays: 0 = no feedback loop,
    /// 0.5 = 50/50, 1 = still image. A real STRENGTH since S57 — before, the
    /// shader read the field only as a switch (`> 0,01 ? 50/50 : ersetzen`), and
    /// 0.3 and 1.0 gave the same image.
    float feedback = 0.5f;

    /// Coloring: palette index = `fract(iterationen * colorScale + Phase)`.
    /// Larger means tighter color rings.
    float colorScale = 0.05f;
    /// Color phase advance per second (0 = static colors).
    float colorCycle = 0.0f;
    /// Name of the gradient onto which the iteration count is mapped.
    std::string gradientPreset = "Neon";
    /// Color of the points that do not escape (the interior of the set), 0x00RRGGBB.
    uint32_t insideColor = 0x000000;

    /// Parameter script (Strang D): `centerX`, `centerY`, `zoomSpeed`,
    /// `rotationSpeed` + audio set. Empty = no script.
    /// Color palette via script (Block C, S79): `colorscale`, `colorcycle`, `colorphase`,
    /// `insidecolor_r`, `insidecolor_g`, `insidecolor_b`.
    /// More fields via script (Block D, S79): `feedback`, `juliax`, `juliay`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a strange-attractor point cloud. Each frame iterates the
 * chosen map for `points` steps and draws the orbit as additive dots (shared
 * ScopeRenderer). `type` 0 Lorenz 1 Clifford 2 De Jong 3 Aizawa; a,b,c,d are the
 * map coefficients. EEL writes a,b,c,d,rotation for live morphing.
 */
struct StrangeAttractorParams
{
    int type = 0;      ///< 0 Lorenz 1 Clifford 2 De Jong 3 Aizawa
    /// First formula coefficient. Its meaning depends on `type`: Lorenz sigma
    /// (x7.142857), Clifford/De Jong the factor inside the sine, Aizawa the z term.
    float a = 1.4f;
    /// Second coefficient: Lorenz rho (x17.5), Clifford/De Jong the second
    /// sine/cosine factor, Aizawa the z offset.
    float b = 1.6f;
    /// Third coefficient: Lorenz beta (x2.6667), Clifford the cosine part in x,
    /// De Jong the sine factor in y, Aizawa the constant term.
    float c = 1.0f;
    /// Fourth coefficient: Clifford/De Jong the cosine part in y, Aizawa the
    /// rotation in the xy plane. Lorenz does not use it.
    float d = 0.7f;
    /// Points per frame (1..100000). The orbit continues across frames — more
    /// points means drawn denser, not longer.
    int points = 6000;
    /// Size of the orbit in the image (factor on the coordinates, before rotation).
    float scale = 0.28f;
    /// Fixed rotation angle of the view in radians, added to the running rotation.
    float rotation = 0.0f;
    /// Running rotation in radians per second (0 = stands still).
    float rotationSpeed = 0.08f;

    /// Dot color 0x00RRGGBB when `useGradient` is off.
    uint32_t color = 0x66CCFF;
    bool useGradient = true;      ///< color dots along the orbit via the gradient
    /// Name of the gradient for `useGradient` — the orbit is colored over it
    /// from start to end.
    std::string gradientPreset = "Neon";
    /// Dot diameter in pixels (1..32).
    float dotSize = 2.0f;
    int blend = 1;                ///< 0 replace, 1 additive, 2 50/50

    /// Parameter script (Strang D): `a`, `b`, `c`, `d`, `rotation` + audio set.
    /// Empty = no script. Caution: changing the coefficients in the panel resets
    /// the carrier and runs the init slot again.
    /// Colors via script (Block B, S79): `color_r`, `color_g`, `color_b` (0..1).
    /// More fields via script (Block D, S79): `scale`, `rotationspeed`, `dotsize`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a Flame / IFS (chaos-game) renderer. Each frame runs the
 * chaos game for `points` iterations over a small affine function system with a
 * non-linear `variation`, drawing the samples as additive dots. `functions`
 * selects the preset system (2..4 maps). EEL writes variation / scale / rotation.
 */
struct FlameParams
{
    int variation = 0;   ///< 0 linear 1 sinusoidal 2 spherical 3 swirl 4 horseshoe
    int functions = 3;   ///< affine map count (2..4)
    /// Moves of the chaos game per frame — more means a denser image, not a
    /// different one.
    int points = 20000;
    /// Size of the figure in the image (factor on the coordinates, before rotation).
    float scale = 0.5f;
    /// Fixed rotation angle in radians, added to the running rotation.
    float rotation = 0.0f;
    /// Running rotation in radians per second (0 = stands still).
    float rotationSpeed = 0.04f;

    /// Name of the gradient — the points are colored by their number in the
    /// move sequence.
    std::string gradientPreset = "Fire";
    /// Dot diameter in pixels.
    float dotSize = 1.5f;
    /// 0 replace, 1 additive, 2 50/50.
    int blend = 1;

    /// Parameter script (Strang D): `variation` (0..4), `scale` and `rotation` +
    /// audio set. Empty = no script.
    /// More fields via script (Block D, S79): `rotationspeed`, `dotsize`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a Gray-Scott reaction-diffusion simulation on a persistent
 * ping-pong buffer (chemical A in .r, B in .g). `stepsPerFrame` sim iterations run
 * per frame; B's concentration maps through the gradient LUT. On a beat a fresh
 * seed blob is stamped (`seedOnBeat`). EEL writes feed,kill for live pattern shifts.
 */
struct ReactionDiffusionParams
{
    float feed = 0.055f;      ///< feed rate
    float kill = 0.062f;      ///< kill rate
    float diffA = 1.0f;       ///< diffusion rate of A
    float diffB = 0.5f;       ///< diffusion rate of B
    int stepsPerFrame = 8;    ///< sim iterations per rendered frame
    bool seedOnBeat = true;   ///< stamp a new seed blob on a beat

    /// Coloring: palette index = `fract(Konzentration von B * colorScale + Phase)`.
    float colorScale = 1.0f;
    /// Color phase advance per second (0 = static colors).
    float colorCycle = 0.0f;
    /// Name of the gradient onto which the concentration is mapped.
    std::string gradientPreset = "Neon";

    /// Onto the existing image: 0 replace, 1 additive (clamped), 2 50/50.
    int blend = 0;
    /// Parameter script (Strang D): `feed`, `kill` + audio set — this makes
    /// the pattern wander while running. Empty = no script.
    /// Color palette via script (Block C, S79): `colorscale`, `colorcycle`,
    /// `colorphase`.
    /// More fields via script (Block D, S79): `diffa`, `diffb`, `stepsperframe`.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Shadertoy-Node (Strang S, `Regelwerk_und_Neue_Module_Plan.md` §S — S65):
 * EIN Fragment-Pass in Chain-Auflösung, Code in Shadertoy-Konvention
 * (`mainImage(out vec4, in vec2)`, GLSL-ES; Wrapper: ShadertoyWrapper.hpp).
 * Modernes Regelwerk — kein Legacy-Ballast. `audioChannel` bestimmt, welcher
 * iChannel die 512×2-Audio-Textur trägt (Zeile 0 = FFT-Spektrum, Zeile 1 =
 * Waveform, 0..1); zusätzlich stehen bass/mid/treb/vol/beat als Uniforms im
 * Shader bereit (Nachrüstung nicht-audioreaktiver Shader ohne Textur-Zugriff).
 * Metadaten füllt der URL-Import (S3) — Inhalte bleiben lokal
 * (Shadertoy-Default-Lizenz CC BY-NC-SA, Plan §S3).
 */
/// Kanal-Bindungs-Kodierung des Shadertoy-Nodes — SSOT. Was hängt an iChannelN?
/// -1 = nichts (schwarz) · 0..3 = Ausgang von Buffer A..D · 4 = Audio-Textur ·
/// 5 = Ketten-Eingang · 6..13 = AVS-Global-Buffer 1..8 (S72).
inline constexpr int kShadertoyInputNone = -1;
inline constexpr int kShadertoyInputAudio = 4;
/// Das Bild, das der Knoten von der Kette bekommt — damit werden
/// Shadertoy-BILDFILTER mit `iChannel0`-Eingang direkt lauffähig (S72).
inline constexpr int kShadertoyInputChain = 5;
/// Erster AVS-Global-Buffer; `kShadertoyInputAvsBase + n` ist Buffer `n+1`
/// (S72). Es sind die FBOs des `OffscreenBufferPool` — dieselben, die
/// „Buffer Save" beschreibt. Semantik: der Stand BEIM LAUF des Knotens, die
/// Reihenfolge in der Kette zählt also. Host-Gruppen-Scoping kommt über
/// `activePool()` gratis mit.
inline constexpr int kShadertoyInputAvsBase = 6;
inline constexpr int kShadertoyInputAvsCount = 8;
/// Größter gültiger Wert — Leser und Panel klemmen dagegen.
inline constexpr int kShadertoyInputMax =
    kShadertoyInputAvsBase + kShadertoyInputAvsCount - 1;

/**
 * Ein Buffer-Pass des Shadertoy-Nodes (S4 — Buffer A..D): eigener
 * mainImage-Quelltext + 4 Kanal-Bindungen. Buffer rendern je Frame in der
 * Reihenfolge A→D in eigene Ping-Pong-Ziele (RGBA32F, Chain-Auflösung);
 * eine Referenz auf sich selbst oder einen SPÄTEREN Buffer liest das
 * VORFRAME (FeedbackBuffer-Muster), eine auf einen früheren das frische Bild
 * dieses Frames — Original-Semantik.
 */
struct ShadertoyPass
{
    std::string code;                       ///< mainImage-Quelltext dieses Buffers
    std::array<int, 4> input = {kShadertoyInputNone, kShadertoyInputNone,
                                kShadertoyInputNone, kShadertoyInputNone};
};

struct ShadertoyParams
{
    std::string code;      ///< mainImage source of the Image pass (empty = starter)
    /// Channel bindings of the Image pass (encoding see kShadertoyInput* —
    /// SSOT; an early `audioChannel` field is migrated on load).
    std::array<int, 4> imageInput = {kShadertoyInputAudio, kShadertoyInputNone,
                                     kShadertoyInputNone, kShadertoyInputNone};
    int blend = 0;         ///< onto the image: 0 replace, 1 additive, 2 50/50
    /// Buffer A..D (S4, max 4; index = letter). Empty = single-pass shader.
    std::vector<ShadertoyPass> buffers;
    /// Where the shader comes from (S3 import; shown by the panel). Since S72 the
    /// shared `Herkunft` instead of four separate fields — see there.
    Herkunft herkunft;
    /// Parameter script (Strang D, Block E S79): `blend` and every uniform
    /// that the code itself declares (`uniform float wobble;` → `wobble`,
    /// vecN → `_x/_y/_z/_w`; from Image and Buffer code) + `b`/`w`/`h` +
    /// audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/// iChannel-Index der Audio-Textur in einer Bindungs-Liste (-1 = keiner)
[[nodiscard]] inline int shadertoyAudioChannel(const std::array<int, 4>& input)
{
    for (int c = 0; c < 4; ++c)
    {
        if (input[static_cast<std::size_t>(c)] == kShadertoyInputAudio) return c;
    }
    return -1;
}

/**
 * Mesh-Warp-Node (Strang G1, `Regelwerk_und_Neue_Module_Plan.md` §G — S69):
 * die GPU-Antwort auf die Movement-Klasse. Eine Nutzer-GLSL-Funktion
 * `vec2 warp(vec2 uv)` läuft je GITTER-VERTEX im Vertex-Shader und bestimmt,
 * wo das aktuelle Chain-Bild abgetastet wird — zustandslos-parallel,
 * modernes Regelwerk (Legacy-Imports bleiben CPU-Mesh, Sequenz-Vertrag).
 * Wrapper/Gitter: MeshWarpWrapper.hpp; Audio ad-hoc als Uniforms
 * (Shadertoy-Muster — Entscheid Patrik S69: G1 vor Vereinheitlichung V2).
 */
struct MeshWarpParams
{
    /// GLSL: defines `vec2 warp(vec2 uv)` (uv 0..1, return value = source UV).
    /// Uniforms: uTime, uDelta, uFrame, uResolution, bass, mid, treb, vol, beat.
    /// Empty = identity (passthrough)
    std::string code;
    /// Grid quads in X (2..256) — GPU vertex work, high values cost almost nothing;
    /// coarse = angular warp
    int gridX = 96;
    /// Grid quads in Y (2..192)
    int gridY = 72;
    /// Share of the warped image 0..1 (1 = replace; below that, mix with the
    /// unwarped sample)
    double mixAmount = 1.0;
    /// Source UV outside 0..1: on = repeat (fract), off = clamp to edge pixels
    bool wrapUv = true;
    /// Parameter script (Strang D): `gridx`, `gridy`, `mixamount` + `b`/`w`/`h` +
    /// audio set. Empty = no script.
    std::string initCode;
    /// Parameter script (Strang D): `gridx`, `gridy`, `mixamount` + `b`/`w`/`h` +
    /// audio set. Empty = no script.
    std::string frameCode;
    /// Parameter script (Strang D): `gridx`, `gridy`, `mixamount` + `b`/`w`/`h` +
    /// audio set. Empty = no script.
    std::string beatCode;
};

/**
 * GPU-Partikel-Node (Strang G2, `Regelwerk_und_Neue_Module_Plan.md` §G — S69):
 * die moderne Antwort auf die searchlight-Klasse — zehntausende Partikel per
 * Instancing statt 800×16-gmegabuf-Schleifen. Zustand (pos+vel) lebt als ein
 * RGBA32F-Texel je Partikel im Ping-Pong (GpuParticlesWrapper.hpp); Alter/
 * Respawn sind hash-basiert OHNE Speicher (deterministisch mit der Sim-Uhr).
 * RENDER-Modul: zeichnet auf das Chain-Bild (Audio ad-hoc als Uniforms,
 * Shadertoy-Muster — Entscheid Patrik S69: Strang G vor Vereinheitlichung V2).
 */
struct GpuParticlesParams
{
    /// Particle count 1..65536 — on the GPU, high values cost little; a change
    /// rebuilds the state (fresh start)
    int count = 4096;
    /// Spawn center X 0..1 (0 = left)
    double spawnX = 0.5;
    /// Spawn center Y 0..1 (0 = bottom)
    double spawnY = 0.35;
    /// Spawn scatter radius in UV (0 = point source)
    double spawnSpread = 0.03;
    /// Start speed in UV/s (scattered 0.6..1.4x per particle)
    double speed = 0.25;
    /// Start direction in degrees (0 = right, 90 = up)
    double direction = 90.0;
    /// Fan/spread angle in degrees around the direction (360 = radial in all directions)
    double fan = 360.0;
    /// Gravity X in UV/s²
    double gravityX = 0.0;
    /// Gravity Y in UV/s² (negative = falls down)
    double gravityY = -0.15;
    /// Air drag 1/s (0 = none; slows the velocity exponentially)
    double drag = 0.6;
    /// Lifetime in seconds (base; afterwards respawn at the spawn point)
    double lifeSeconds = 2.5;
    /// Lifetime jitter 0..1 (0.35 = ±35 % per particle, hash-stable)
    double lifeJitter = 0.35;
    /// Sprite size in pixels (at spawn)
    double sizePx = 6.0;
    /// Size factor at end of life (1 = constant, <1 shrinks)
    double sizeEndFactor = 0.4;
    /// Color at spawn 0x00RRGGBB
    uint32_t colorStart = 0xFFD080;
    /// Color at end of life 0x00RRGGBB (gradient over age)
    uint32_t colorEnd = 0x4020C0;
    /// Draw additively (glow look) instead of alpha blending
    bool additive = true;
    /// GLSL force field (optional): defines
    /// `vec2 kraft(vec2 pos, vec2 vel, float alter)` — additional acceleration in
    /// UV/s²; uniforms uTime, uDelta, uResolution, bass, mid, treb, vol, beat.
    /// Empty = gravity/drag only
    std::string forceCode;
    /// Parameter script (Strang D): `spawnx`, `spawny`, `spread`, `speed`, `dir`,
    /// `fan`, `gravx`, `gravy`, `drag`, `life`, `size` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    /// Colors via script (Block B, S79): `colorstart_r`, `colorstart_g`,
    /// `colorstart_b`, `colorend_r`, `colorend_g`, `colorend_b` (0..1).
    /// More fields via script (Block D, S79): `sizeendfactor`, `lifejitter`.
    std::string initCode;
    /// Parameter script (Strang D): `spawnx`, `spawny`, `spread`, `speed`, `dir`,
    /// `fan`, `gravx`, `gravy`, `drag`, `life`, `size` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    /// Colors via script (Block B, S79): `colorstart_r`, `colorstart_g`,
    /// `colorstart_b`, `colorend_r`, `colorend_g`, `colorend_b` (0..1).
    /// More fields via script (Block D, S79): `sizeendfactor`, `lifejitter`.
    std::string frameCode;
    /// Parameter script (Strang D): `spawnx`, `spawny`, `spread`, `speed`, `dir`,
    /// `fan`, `gravx`, `gravy`, `drag`, `life`, `size` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    /// Colors via script (Block B, S79): `colorstart_r`, `colorstart_g`,
    /// `colorstart_b`, `colorend_r`, `colorend_g`, `colorend_b` (0..1).
    /// More fields via script (Block D, S79): `sizeendfactor`, `lifejitter`.
    std::string beatCode;
};

/**
 * Pixel-Filter-Node (Stilfilter-Strang, Offene_Punkte §7 — S70; Entscheid
 * Patrik: EIN skriptbares Filtermodul + Werks-Voreinstellungen statt
 * Festmodule). TRANSFORM-Modul: eine Nutzer-GLSL-Funktion
 * `vec4 farbe(vec2 uv, vec4 src)` laeuft je PIXEL im Fragment-Shader und
 * faerbt das Chain-Bild um (Nachbar-Abtastung ueber uTex erlaubt — Kantenzuege);
 * wirkt damit auf JEDE Quelle (Video, Kamera, Scopes, MilkDrop).
 * Wrapper: PixelFilterWrapper.hpp; Audio ad-hoc als Uniforms (Shadertoy-
 * Muster). Ein Knoten = EIN Pass; der Filter-STACK ist die Kette selbst.
 */
struct PixelFilterParams
{
    /// GLSL: defines `vec4 farbe(vec2 uv, vec4 src)` (uv 0..1, src = source pixel; neighbors via
    /// texture(uTex, ...); `filter` is reserved in GLSL). Uniforms: uTime, uDelta, uFrame,
    /// uResolution, bass, mid, treb, vol, beat. Empty = identity (passthrough)
    std::string code;
    /// Share of the filtered image 0..1 (1 = replace; below that, mix with the original)
    double mixAmount = 1.0;
    /// Parameter script (Strang D): `mixamount` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    /// Parameter script (Strang D): `mixamount` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string frameCode;
    /// Parameter script (Strang D): `mixamount` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string beatCode;
};

/// Bindungs-Kodierung einer Bildquelle des ISF-Knotens — SSOT.
/// AVS-Konvention (Entscheid Patrik S72): **nichts gewählt = normale
/// Pipeline**, also das Ketten-Bild. Sonst ein AVS-Global-Buffer.
namespace isffilter {
constexpr int kQuelleKette = -1;    ///< Vorgabe: das Bild, das der Knoten vorfindet
constexpr int kQuelleSchwarz = -2;  ///< bewusst nichts (z. B. Generator-Eingang)
/// ISF-Audio-Eingaenge sind gewoehnliche SAMPLER — und zwar ZWEI mit
/// verschiedener Bedeutung: `audio` traegt die **Waveform** (Zeitsignal),
/// `audioFFT` das **Spektrum**; Breite = Samples/Bins, Hoehe = Kanaele.
///
/// **Befund S72:** zuerst hingen beide an der kombinierten 512×2-Textur des
/// Shadertoy-Knotens (Zeile 0 Spektrum, Zeile 1 Waveform). Ein Shader, der
/// bei `y = 0.5` abtastet, landete mit `GL_LINEAR` genau ZWISCHEN den Zeilen
/// und las eine Mischung aus beidem — es kompilierte und zappelte zur Musik,
/// war aber inhaltlich falsch. Deshalb zwei getrennte Bindungen.
constexpr int kQuelleAudioWave = -3;  ///< Waveform-Textur (ISF `audio`)
constexpr int kQuelleAudioFft = -4;   ///< Spektrum-Textur (ISF `audioFFT`)
constexpr int kQuelleBufferBasis = 0;  ///< 0..7 = AVS-Global-Buffer 1..8
constexpr int kQuelleBufferAnzahl = 8;
constexpr int kQuelleMax = kQuelleBufferBasis + kQuelleBufferAnzahl - 1;
constexpr int kGeometrieMax = 2;  ///< 0 Quad · 1 Einzeldreieck · 2 Gitter
constexpr int kGridMin = 2;
constexpr int kGridMax = 256;
}  // namespace isffilter

/// Ein Bild-Eingang des ISF-Shaders und woher er gespeist wird.
struct IsfBildQuelle
{
    std::string name;  ///< Bezeichner im Shader (`inputImage`, `startImage`, …)
    int bindung = isffilter::kQuelleKette;  ///< s. `isffilter::kQuelle*`
};

/**
 * Ein Durchgang eines Multipass-Shaders (ISF `PASSES`).
 *
 * Der Shader-Text ist bei ALLEN Durchgängen derselbe; `PASSINDEX` sagt ihm,
 * im wievielten er steckt. Ein Durchgang mit `target` rendert in einen
 * benannten Puffer, der danach unter genau diesem Namen als Sampler lesbar
 * ist — auch von späteren Durchgängen. Der letzte Durchgang trägt
 * üblicherweise KEIN Ziel und schreibt damit aufs Kettenbild.
 *
 * Das ist die Klasse von Effekten, die in einem Durchgang grundsätzlich nicht
 * geht: separabler Weichzeichner (erst waagerecht, dann senkrecht), Bloom
 * (trennen → weichzeichnen → addieren), Auto-Levels/Histogramm (Reduktion
 * über das ganze Bild).
 */
struct IsfPassZiel
{
    /// Puffername = Sampler-Name im Shader. Leer = zeichnet aufs Kettenbild.
    std::string target;
    /// Größen-AUSDRÜCKE der ISF-Spec (`$WIDTH/16`, `floor($HEIGHT*0.5)` …).
    /// Leer = volle Bildgröße. Reduktionsstufen hängen genau daran.
    std::string breite;
    std::string hoehe;
    /// `FLOAT`: 32-Bit-Gleitkomma statt 8 Bit — nötig, sobald ein Durchgang
    /// Werte außerhalb 0..1 oder feine Zwischenstände sammelt (Histogramme).
    bool gleitkomma = false;
    /// `PERSISTENT`: der Inhalt überlebt das Frame. DAS ist der Feedback-Fall
    /// — ein Durchgang liest sein eigenes Ziel vom Vorframe.
    bool bestaendig = false;
};

/**
 * ISF-Filter-Knoten (Entscheid Patrik S72): ein importierter oder selbst
 * geschriebener Shader im **Interactive Shader Format** — mit BEIDEN
 * Shader-Stufen in getrennten Feldern.
 *
 * Warum ein eigener Knoten statt eines Ausbaus des `pixelFilter`: ISF kennt
 * neben dem Fragment- auch einen **Vertex-Shader**, und damit Sorten, die ein
 * reiner Pixelfilter grundsätzlich nicht kann. Der `pixelFilter` bleibt, was
 * er ist — ein Pass, ein Feld.
 *
 * **Es gibt keine Kategorien, nur unterschiedlich viele Bildquellen** (Befund
 * S72, am Vidvox-Korpus belegt): ein *Filter* hat eine (`inputImage`), ein
 * *Übergang* zwei (`startImage`/`endImage`), ein *Generator* keine — und
 * jede dieser Sorten darf zusätzlich einen `.vs` haben (`Rotate` ist ein
 * Übergang mit, `Life` ein Generator mit Vertex-Shader). Deshalb wird beim
 * Import nichts mehr abgelehnt; das Panel zeigt schlicht so viele
 * Quell-Auswahlen, wie der Shader Bild-Eingänge deklariert.
 */
struct IsfFilterParams
{
    /// The fragment shader as it appears in the `.fs` (without the JSON header).
    std::string fragCode;
    /// The vertex shader from the `.vs` of the same name. Empty = default: the
    /// geometry is passed through unchanged.
    std::string vertexCode;
    /// Image inputs of the shader in declaration order, each with its source.
    std::vector<IsfBildQuelle> quellen;
    /// Passes of a multipass shader (`PASSES`). Empty = one pass.
    std::vector<IsfPassZiel> passes;
    /// The shader's own sliders (ISF `INPUTS`) — see `ParamGruppe`. Each
    /// slider is a uniform whose name is the `key`; the value is set every
    /// frame (no recompiling while turning a slider).
    ParamGruppe parameter;
    /// Origin/license of the import — see `Herkunft`. Shown in the panel.
    Herkunft herkunft;
    /// Type of the drawn geometry: 0 = quad (ISF-faithful, default) ·
    /// 1 = one oversized triangle (no diagonal ⇒ no kinked
    /// varying interpolation) · 2 = grid (real deformation).
    int geometrie = 0;
    /// Grid quads in X — only when `geometrie == 2`.
    int gridX = 64;
    /// Grid quads in Y — only when `geometrie == 2`.
    int gridY = 48;
    /// Onto the existing image: 0 replace, 1 additive, 2 50/50.
    int blend = 0;
    /// Share of the result 0..1 (1 = replace).
    double mixAmount = 1.0;
    /// Parameter script (Strang D, Block E S79): each slider under its
    /// `key` (2D point as `<key>_x`/`_y`, color as `<key>_r`/`_g`/`_b`/`_a`,
    /// collision with a host name → `in_<key>`), plus `blend`,
    /// `mixamount` + `b`/`w`/`h` + audio set. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/// Klemm-Grenzen des Video-/Kamera-Quellknotens — SSOT fuer Leser, Panel und
/// Renderer (Muster: lumi::meshwarp in MeshWarpWrapper.hpp).
namespace videosource {
constexpr double kMinSpeed = 0.05;  ///< Abspieltempo-Faktor untere Grenze
constexpr double kMaxSpeed = 20.0;  ///< Abspieltempo-Faktor obere Grenze
constexpr int kSourceMax = 2;       ///< Quelle 0..2 (Datei/Kamera/Testaufnahme)
constexpr int kFitMax = 2;          ///< Einpassung 0..2 (strecken/einpassen/fuellen)
constexpr int kBlendMax = 2;        ///< Mischung 0..2 (ersetzen/additiv/50-50)
}  // namespace videosource

/**
 * Video-/Kamera-Quellknoten (LumiViz-eigen; Wunsch Patrik S55, umgesetzt S70 —
 * Sondierung: Offene_Punkte §7). RENDER-Modul: zeichnet Videobilder auf das
 * Chain-Bild. Drei Quellen: Datei (FFmpeg-Backend ueber Qt Multimedia),
 * Kamera (live — das Geraet startet NIE automatisch, nur nach ausdruecklicher
 * Freigabe im Panel je App-Lauf) und Testaufnahme (benutzerlokale Kamera-Clips
 * aus dem Settings-Tab „Kamera" — der deterministische Stellvertreter der
 * Kamera fuer Sonden/Standalone). Der AVS-`avi`-Knoten bleibt unangetastet
 * (an ihm haengt die Kalibrierung).
 */
struct VideoSourceParams
{
    /// Source: 0 = file, 1 = camera (live), 2 = camera clip (clip from the Settings tab "Camera")
    int source = 0;
    /// File source: video path (MP4/MKV/WebM/MOV/WMV/AVI — FFmpeg backend). Empty = draw nothing
    std::string filePath;
    /// Camera source: device ID from the panel selection (empty = none). The device starts only
    /// after camera access is granted in the panel — never automatically when loading a preset
    std::string cameraId;
    /// Camera clip: file name of the clip in the user-local storage (without path)
    std::string recordingName;
    /// Real-time streaming: on = frames driven by the wall clock via QMediaPlayer (long videos,
    /// little RAM — not deterministic), off = frame index deterministic via the sim clock
    /// (VideoFrameCache decodes the file COMPLETELY into memory — short clips/probes)
    bool streaming = false;
    /// Playback speed factor 0.05..20 (frame stepping only; 1 = original speed via the clip
    /// frame rate)
    double speed = 1.0;
    /// Restart from the beginning at the end (off = hold the last frame); file/camera clip only
    bool loop = true;
    /// Fit into the chain image: 0 = stretch, 1 = fit (letterbox), 2 = fill (crop)
    int fit = 0;
    /// Blending onto the chain image: 0 = replace, 1 = additive, 2 = 50/50
    int blend = 0;
    /// Opacity 0..1 (scales the contribution in all blend modes)
    double opacity = 1.0;
    /// Parameter script (Strang D): `speed`, `opacity` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string initCode;
    /// Parameter script (Strang D): `speed`, `opacity` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string frameCode;
    /// Parameter script (Strang D): `speed`, `opacity` + `b`/`w`/`h` + audio set.
    /// Empty = no script.
    std::string beatCode;
};

/**
 * Umgang mit dem GEERBTEN Feedback-Bild beim .milk→.milk-Wechsel im Host
 * (S66). Das Original loescht den Hauptpuffer beim Preset-Wechsel nie —
 * chaotische Feedback-Presets kippen deshalb je nach Vorgaenger in andere
 * Aeste. Behalten = Original-Semantik; Loeschen = wie Kaltstart (frische
 * Rausch-Saat); Fading = EINMALIGER Mix aus Erbe und Saat im Moment des
 * Wechsels (Anteil = pufferFading); Ausblenden = das Erbe stirbt ueber
 * pufferAusblendSek Sekunden weg (Echo je Frame zusaetzlich gedaempft —
 * was das neue Preset frisch zeichnet, bleibt); AppEinstellung folgt dem
 * Default aus dem Settings-Panel.
 */
enum class PufferWechsel
{
    AppEinstellung,
    Behalten,
    Loeschen,
    Fading,
    Ausblenden,
};

/**
 * MilkDrop-Meganode (Import Roadmap 6, N1 — Entscheid E1): renders the whole
 * fixed MilkDrop frame pipeline (warp mesh, waves/shapes, blur pyramid,
 * stage-B/C shaders, composite) into the chain buffer. The TRANSLATED preset
 * travels inside the node (self-contained .lvfx rule); shader classification
 * is re-derived from the raw shader texts (SSOT) whenever the host applies a
 * new revision.
 */
struct MilkdropNodeParams
{
    lumi::milkdrop::PresetState preset;  ///< translated .milk incl. HLSL texts
    /// Texture search base (S43: upward search over textures/ + sprites/).
    std::string presetDir;
    /**
     * Embedded images (decision Patrik S43): key = texture base name or
     * sprite imageName, value = Base64 of the ORIGINAL file bytes. The asset
     * folders remain the source; on SAVE the ChainSerializer embeds exactly the
     * currently referenced images (old entries that are no longer referenced
     * are dropped automatically). The loader prefers files and falls back to
     * these entries when nothing is found — .lvfx thus stays portable.
     * randNN samplers stay folder-random (deliberately not embedded).
     */
    std::map<std::string, std::string> embeddedImages;
    int meshX = 32;   ///< warp mesh (app setting in the original, no preset key)
    /// Warp grid vertical (original default 24; higher = finer warp).
    int meshY = 24;
    bool debugGrid = false;  ///< calibration overlay AFTER the composite
    /// Motion vector overlay AFTER the composite (analysis, S76): draws the
    /// warp flow field over the image, forced to alpha 1, without
    /// touching the feedback loop — the preset motion vectors (mv* values)
    /// are unaffected by this and keep drawing into the loop.
    bool debugMotionVectors = false;
    /**
     * Bumped on every preset/script/shader edit (import sets 1). The render
     * host re-applies the state (script compile, shader transpile, texture
     * reload) only when it sees a new revision — frames never re-parse.
     */
    uint64_t revision = 1;

    /// Buffer behavior on a .milk→.milk switch — node setting, survives
    /// the in-place preset swap (like meshX/meshY). Persisted in the chain doc.
    PufferWechsel pufferWechsel = PufferWechsel::AppEinstellung;
    /// Fading: share of the CARRYOVER 0..1 (0 = like "Clear", 1 = like "Keep");
    /// only effective in "Fading" mode.
    double pufferFading = 0.5;
    /// Fade out: duration in seconds over which the carryover dies away (echo additionally
    /// damped each frame); only effective in "Fade out" mode.
    double pufferAusblendSek = 2.0;
    /**
     * Transient (not persisted): bumped on every in-place preset swap; on a new
     * counter value the render host applies `wechselErbe` to the
     * FeedbackBuffer. `wechselErbe` is the carryover share RESOLVED at the swap
     * (app setting already factored in; 1 = do nothing);
     * `wechselAusblendSek` > 0 additionally starts the timed fade-out.
     */
    uint64_t wechselZaehler = 0;
    double wechselErbe = 1.0;
    double wechselAusblendSek = 0.0;
};

/** Conserved effect the host cannot render yet — passes the buffer through. */
struct PassthroughParams
{
    int32_t sourceId = 0;  ///< original AVS effect id (or 0 if hand-built)
    std::string note;      ///< import report text ("effect X not implemented")
};

/**
 * LumiViz "Lyrics" (S86): shows the lyrics of the playing track. Synchronised
 * lyrics follow the playback position of the player (current line highlighted,
 * the next lines as a preview; word by word where the file carries word
 * times). Unsynchronised lyrics stand as a block and can step on beats. The
 * text itself is not part of the node — it comes from the track (embedded,
 * `.lrc`/`.txt` next to the song or in the user folder).
 */
struct LyricsParams
{
    std::string fontFace;          ///< font family ("" = default font)
    int fontHeight = -36;          ///< character height in pixels (stored negative, like Text)
    int fontWeight = 700;          ///< 400 normal, 700 bold (bold from 600)
    bool italic = false;           ///< italic
    std::uint32_t color = 0xFFFFFF;      ///< current line / voice 1 (0x00RRGGBB)
    std::uint32_t color2 = 0x66CCFF;     ///< voice 2 in duets
    std::uint32_t colorBoth = 0xFFE066;  ///< both voices in duets
    std::uint32_t colorNext = 0x808080;  ///< upcoming lines and words not sung yet
    int previewLines = 1;          ///< upcoming lines shown below the current one (0..4)
    int hAlign = 1;                ///< 0 left, 1 center, 2 right
    int vAlign = 2;                ///< 0 top, 1 center, 2 bottom
    int xShift = 0;                ///< percent of width
    int yShift = -6;               ///< percent of height (negative = up)
    bool outline = true;           ///< draw an outline around the glyphs
    std::uint32_t outlineColor = 0;  ///< outline color 0x00RRGGBB
    int outlineSize = 2;           ///< outline width in pixels
    /// How the glyphs mix with the picture: 0 replace, 1 additive, 2 50/50,
    /// 3 subtract, 4 invert (white text inverts the colour underneath), 5 XOR,
    /// 6 multiply, 7 maximum, 8 minimum. From 3 on only the fill mixes — the
    /// outline is drawn normally and keeps the text readable.
    int blend = 0;
    /// Strength of the blend in percent (0..100): scales the opacity of the
    /// fill — 100 = full, 0 = the fill is gone (the outline stays).
    int blendStrength = 100;
    /// Timing offset in milliseconds for synchronised lyrics: positive = the
    /// lyrics appear earlier, negative = later (same sense as LRC `[offset:]`).
    int offsetMs = 0;
    /// Unsynchronised lyrics: step one line forward every N beats (0 = the
    /// block stands still). Has no effect on synchronised lyrics.
    int beatsPerLine = 0;
    bool showSyncMark = true;      ///< mark unsynchronised lyrics with "(not synced)"

    /// Parameter script (S86): every field of this node + `b`/`w`/`h` + audio
    /// set. Fields that re-render the glyphs follow the script from Init, Song,
    /// Beat and Line only. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Runs once on every line change of the lyrics, before Frame — the place
    /// for fields that re-render the glyphs (position, size, colours). Reads
    /// `line` (number of the running line, 0 = before the first line) and
    /// `linecount`. Empty = no script.
    std::string lineCode;
};

/**
 * LumiViz "Lyrics Trail" (S87): draws the lyrics line that has just ended once
 * more — at this node's place in the chain. Whatever follows it (Fadeout,
 * Blur, Movement, a list with feedback) makes the line fade, drift or
 * dissolve. The node borrows font, alignment and position from a Lyrics node
 * and has no text of its own. Synchronised lyrics only.
 */
struct LyricsTrailParams
{
    /// Name of the Lyrics node to follow ("" = the first Lyrics node of the chain).
    std::string source;
    int holdMs = 300;              ///< how long the ended line is drawn, in milliseconds (0..5000)
    int xShift = 0;                ///< offset from the place of the line, percent of width
    int yShift = -8;               ///< offset from the place of the line, percent of height (negative = up)
    bool useOwnColor = false;      ///< draw in `color` instead of the color the line had
    std::uint32_t color = 0xFFFFFF;  ///< own color 0x00RRGGBB (with `useOwnColor`)
    /// How the glyphs mix with the picture: 0 replace, 1 additive, 2 50/50,
    /// 3 subtract, 4 invert, 5 XOR, 6 multiply, 7 maximum, 8 minimum (as on
    /// the Lyrics node; from 3 on the outline stays normal).
    int blend = 0;
    /// Strength of the blend in percent (0..100): scales the opacity of the
    /// trail — 100 = full, 0 = nothing is drawn.
    int blendStrength = 100;

    /// Parameter script (S87): every field of this node + `b`/`w`/`h` + audio
    /// set. Fields that re-render the glyphs follow the script from Init, Song
    /// and Beat only. Empty = no script.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

using EffectParams =
    std::variant<ListParams, ClearParams, FadeoutParams, InvertParams,
                 BrightnessParams, FastBrightnessParams, BlurParams, MirrorParams,
                 OnBeatClearParams, ColorfadeParams, ColorModifierParams,
                 MovementParams, DynamicMovementParams, DynamicShiftParams,
                 DynamicDistanceModifierParams, MovingParticleParams,
                 BlitterFeedbackParams,
                 RotoBlitterParams, BufferSaveParams, CustomBpmParams,
                 SuperScopeParams, MosaicParams, GrainParams, ScatterParams,
                 InterferencesParams, WaterParams, BumpParams, WaterBumpParams,
                 FyrewurXParams, Metaballs3DParams, Tentacles3DParams,
                 StarfieldParams, TimescopeParams, DotGridParams, DotPlaneParams,
                 DotFountainParams, ColorMapParams, BufferBlendParams,
                 JherikoGlobalParams, ColorClipParams, UniqueToneParams,
                 InterleaveParams, ConvolutionParams, NormaliseParams,
                 MultiFilterParams, AddBordersParams, SimpleScopeParams,
                 BassSpinParams, OscStarParams, OscRingParams, RotatingStarsParams,
                 PictureParams, PictureIIParams, TexerParams, TexerIIParams,
                 TriangleParams, ChannelShiftParams, ColorReductionParams,
                 MultiplierParams, VideoDelayParams, MultiDelayParams,
                 Fractal2DParams, DomainWarpParams, Fractal3DParams,
                 LyapunovParams, KleinianParams, FractalZoomerParams,
                 StrangeAttractorParams, FlameParams, ReactionDiffusionParams,
                 SetRenderModeParams, DebugBarsParams, MilkdropNodeParams,
                 HostGroupParams, TextParams, AviParams, CommentParams,
                 ImportNotesParams, RenderScaleParams, BloomParams, Camera3DParams,
                 SuperScope3DParams, Terrain3DParams, GlowOrbsParams,
                 ShadertoyParams, MeshWarpParams, GpuParticlesParams,
                 VideoSourceParams, PixelFilterParams, IsfFilterParams,
                 HostSwitcherParams, PassthroughParams,
                 LyricsParams,
                 LyricsTrailParams>;  // neue Typen ans ENDE (Indizes bleiben stehen)

/**
 * Die Herkunft eines Knotens — oder `nullptr`, wenn sein Typ keine trägt.
 *
 * SSOT der Frage „welche Knoten sind importfähig?" (S72). Panel, Shader-Export
 * und spätere Importeure fragen hier, statt je Knotentyp einen eigenen Zweig zu
 * schreiben; ein neuer importfähiger Typ wird an EINER Stelle nachgetragen.
 */
[[nodiscard]] inline const Herkunft* herkunftVon(const EffectParams& params)
{
    if (const auto* p = std::get_if<ShadertoyParams>(&params)) return &p->herkunft;
    if (const auto* p = std::get_if<IsfFilterParams>(&params)) return &p->herkunft;
    return nullptr;
}

// =============================================================================
// Modulation (Block F, F4 — Feldsteuerung_Konzept.md)
// =============================================================================

/// Quelle einer Modulation. Audio/Beat einseitig 0..1, LFOs beidseitig -1..1.
enum class ModQuelle : std::uint8_t
{
    Bass,
    Mid,
    Treble,
    Pegel,       ///< Gesamtpegel (`vol`)
    BeatHuelle,  ///< springt beim Beat auf 1, klingt mit `glaettung` ab
    LfoSinus,
    LfoDreieck,
    LfoSaege,
    LfoRechteck,
    LfoZufall,   ///< je Periode ein neuer Zufallswert (Sample & Hold)
};

/// JSON-Schluessel der Quellen (Datei-Vertrag, englisch wie alle Schluessel).
inline constexpr std::array<const char*, 10> kModQuelleKeys = {
    "bass", "mid", "treble", "level", "beatEnvelope",
    "lfoSine", "lfoTriangle", "lfoSaw", "lfoSquare", "lfoRandom"};

[[nodiscard]] inline const char* modQuelleKey(ModQuelle q)
{
    return kModQuelleKeys[static_cast<std::size_t>(q)];
}

/// Schluessel -> Quelle; unbekannt = false (Aufrufer behaelt die Vorgabe).
[[nodiscard]] inline bool modQuelleAusKey(std::string_view key, ModQuelle& out)
{
    for (std::size_t i = 0; i < kModQuelleKeys.size(); ++i)
    {
        if (key == kModQuelleKeys[i])
        {
            out = static_cast<ModQuelle>(i);
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline bool istLfo(ModQuelle q) { return q >= ModQuelle::LfoSinus; }

/// Block F, F4b (S86, Entscheid 1b a): was eine Modulation an einem FARBFELD
/// bewegt. Zahlfelder kennen nur `Wert`.
enum class ModZiel : std::uint8_t
{
    Wert,        ///< Zahlfeld: der Wert selbst
    Helligkeit,  ///< HSV-Helligkeit += tiefe * Quelle (0..1)
    Farbton,     ///< HSV-Farbton += tiefe * Quelle (1 = ein voller Umlauf)
    Rot,         ///< Kanal += tiefe * Quelle (0..1)
    Gruen,
    Blau
};

/// Datei-Schluessel der Ziele (Reihenfolge = ModZiel).
inline constexpr std::array<const char*, 6> kModZielKeys = {
    "value", "brightness", "hue", "red", "green", "blue"};

[[nodiscard]] inline const char* modZielKey(ModZiel z)
{
    return kModZielKeys[static_cast<std::size_t>(z)];
}

/// Schluessel -> Ziel; unbekannt = false (Aufrufer behaelt die Vorgabe).
[[nodiscard]] inline bool modZielAusKey(std::string_view key, ModZiel& out)
{
    for (std::size_t i = 0; i < kModZielKeys.size(); ++i)
    {
        if (key == kModZielKeys[i])
        {
            out = static_cast<ModZiel>(i);
            return true;
        }
    }
    return false;
}

/**
 * Block F, F4 (S85): ein Feld bewegt sich ohne Formel um seinen Reglerwert.
 * Angewendet auf die Frame-Kopie VOR den Skripten (Entscheid E5: Basis ->
 * Modulation -> Skript); der Reglerwert selbst bleibt unberuehrt.
 * Wert = Regler + tiefe * Quelle * Spanne; Spanne = Bereich des Felds
 * (hi - lo), ohne bekannten Bereich der Betrag des Reglerwerts (Entscheid 2a).
 */
struct FeldModulation
{
    std::string feld;               ///< JSON-Feldname (wie in der Datei), z. B. "zoom"
    ModQuelle quelle = ModQuelle::LfoSinus;
    double tiefe = 0.25;            ///< Anteil der Spanne, +-
    double rateHz = 0.5;            ///< LFO-Frequenz (Hz)
    /// F4d (S86): > 0 = LFO im Takt — eine Schwingung dauert so viele Beats
    /// (4 = ein Takt, 0.5 = zwei je Beat) und beginnt auf dem Beat; `rateHz`
    /// ruht dann. 0 = freie Rate in Hz.
    double rateBeats = 0.0;
    double glaettung = 0.0;         ///< s — Glaettung der Audio-Quellen, Abklingzeit der Beat-Huelle
    bool aktiv = true;              ///< eigenes Haekchen je Modulation
    /// F4b (S86): Ziel an einem Farbfeld; an Zahlfeldern ohne Bedeutung. Steht
    /// an einem Farbfeld `Wert`, gilt Helligkeit.
    ModZiel ziel = ModZiel::Wert;
};

// =============================================================================
// Chain node
// =============================================================================

/**
 * One node of the runtime chain. Which variant alternative `params` holds
 * decides the effect type; `children` is only meaningful for ListParams.
 */
struct ChainNode
{
    std::string displayName;         ///< editor label ("" = derive from type)
    std::string description;         ///< free-text note (editor convenience, optional)
    /// Song-Slot (S82): EEL, laeuft einmal je Songwechsel VOR dem Frame-Slot,
    /// in derselben Skript-Engine wie die uebrigen Slots des Knotens (sieht
    /// und setzt deren Variablen). Ein Feld fuer ALLE skriptfaehigen Knoten
    /// statt je Parameter-Struct; wirkt nur, wo der Knoten Skripte ausfuehrt.
    std::string songCode;
    /// Schalter „Script active" (S83, Entscheid Patrik E1a): aus = der Knoten
    /// laeuft, als staenden seine EEL-Slots (samt Song-Slot) leer — der Code
    /// bleibt gespeichert. Ein Feld am Knoten wie `songCode`, nicht je Typ.
    /// Die Effect List hat dafuer weiterhin ihr eigenes `useCode` (AVS-Feld).
    bool scriptsActive = true;
    /// Block F, F4 (S85): Modulationen der Felder dieses Knotens (Datei:
    /// `mods`, nur wenn belegt) und ihr Knotenschalter „Modulation active"
    /// (Datei: `modulationActive`, nur wenn aus) — Entscheid E5.
    std::vector<FeldModulation> mods;
    bool modsActive = true;
    bool enabled = true;             ///< disabled nodes are skipped entirely
    EffectParams params;             ///< effect type + its parameters
    std::vector<ChainNode> children; ///< child effects (lists only)

    /**
     * Stable node identity, assigned by compileChain() (0 = unassigned).
     * The render host keys GL/script resources by this id, so it survives
     * vector reallocation during edits; never reuse ids manually.
     */
    uint64_t nodeId = 0;

    /**
     * Der Knoten stammt aus einem AVS-APE (Plugin-DLL), nicht aus einem
     * Builtin. Nur Import-Information: der Vergleichs-Referenzkern AvsRef
     * laedt bewusst KEINE APE-DLLs (avsref_main.cpp:349-357), solche Knoten
     * sind dort also wirkungslos — `AvsStandalone --no-ape` schaltet sie
     * deshalb ab, damit der Rest des Presets vergleichbar bleibt (S49).
     * Laufzeit-Information, wird nicht serialisiert.
     */
    bool fromApe = false;

    [[nodiscard]] bool isList() const
    {
        return std::holds_alternative<ListParams>(params);
    }

    /// Host-Gruppe (HG1): Container wie eine Liste, aber mit eigenem
    /// Laufzeit-Bestand — children sind auch hier gueltig.
    [[nodiscard]] bool isHostGroup() const
    {
        return std::holds_alternative<HostGroupParams>(params);
    }

    /// Container = darf children tragen (Liste oder Host-Gruppe).
    [[nodiscard]] bool isContainer() const { return isList() || isHostGroup(); }

    /// Presetverweis (S82): Host-Gruppe, deren Inhalt aus einer Datei kommt.
    [[nodiscard]] bool isReference() const
    {
        const auto* hg = std::get_if<HostGroupParams>(&params);
        return hg != nullptr && hg->verweis;
    }

    /// Darf der Editor hier Knoten hineinlegen? Container, aber kein Verweis —
    /// dessen Inhalt ist geliehen und ginge beim Speichern verloren.
    [[nodiscard]] bool acceptsChildren() const { return isContainer() && !isReference(); }
};

// =============================================================================
// Chain compile pass (runs after every mutation, decision E4)
// =============================================================================

/** One structural finding, path-prefixed like the AvsParser import report. */
struct CompileMessage
{
    std::string path;  ///< e.g. "root/2/0"
    std::string text;
};

struct CompileResult
{
    bool ok = true;                        ///< false only on structural damage
    std::vector<CompileMessage> warnings;  ///< never a hard failure (AVS rule)
};

/** Human-readable effect-type name (also the displayName fallback). */
[[nodiscard]] inline const char* effectTypeName(const EffectParams& params)
{
    struct Visitor
    {
        const char* operator()(const ListParams&) const { return "Effect List"; }
        const char* operator()(const HostGroupParams&) const { return "Host Group"; }
        const char* operator()(const HostSwitcherParams&) const { return "Host Switcher"; }
        const char* operator()(const LyricsParams&) const { return "Lyrics"; }
        const char* operator()(const LyricsTrailParams&) const { return "Lyrics Trail"; }
        const char* operator()(const ClearParams&) const { return "Clear"; }
        const char* operator()(const FadeoutParams&) const { return "Fadeout"; }
        const char* operator()(const InvertParams&) const { return "Invert"; }
        const char* operator()(const BrightnessParams&) const { return "Brightness"; }
        const char* operator()(const FastBrightnessParams&) const { return "Fast Brightness"; }
        const char* operator()(const BlurParams&) const { return "Blur"; }
        const char* operator()(const MirrorParams&) const { return "Mirror"; }
        const char* operator()(const OnBeatClearParams&) const { return "OnBeat Clear"; }
        const char* operator()(const ColorfadeParams&) const { return "Colorfade"; }
        const char* operator()(const ColorModifierParams&) const { return "Color Modifier"; }
        const char* operator()(const MovementParams&) const { return "Movement"; }
        const char* operator()(const DynamicMovementParams&) const { return "Dynamic Movement"; }
        const char* operator()(const DynamicShiftParams&) const { return "Dynamic Shift"; }
        const char* operator()(const DynamicDistanceModifierParams&) const { return "Dynamic Distance Modifier"; }
        const char* operator()(const MovingParticleParams&) const { return "Moving Particle"; }
        const char* operator()(const BlitterFeedbackParams&) const { return "Blitter Feedback"; }
        const char* operator()(const RotoBlitterParams&) const { return "Roto Blitter"; }
        const char* operator()(const BufferSaveParams&) const { return "Buffer Save"; }
        const char* operator()(const CustomBpmParams&) const { return "Custom BPM"; }
        const char* operator()(const SuperScopeParams&) const { return "SuperScope"; }
        const char* operator()(const MosaicParams&) const { return "Mosaic"; }
        const char* operator()(const GrainParams&) const { return "Grain"; }
        const char* operator()(const ScatterParams&) const { return "Scatter"; }
        const char* operator()(const InterferencesParams&) const { return "Interferences"; }
        const char* operator()(const WaterParams&) const { return "Water"; }
        const char* operator()(const BumpParams&) const { return "Bump"; }
        const char* operator()(const WaterBumpParams&) const { return "Water Bump"; }
        const char* operator()(const FyrewurXParams&) const { return "FyrewurX"; }
        const char* operator()(const Metaballs3DParams&) const { return "Metaballs 3D"; }
        const char* operator()(const Tentacles3DParams&) const { return "Tentacles 3D"; }
        const char* operator()(const StarfieldParams&) const { return "Starfield"; }
        const char* operator()(const TimescopeParams&) const { return "Timescope"; }
        const char* operator()(const DotGridParams&) const { return "Dot Grid"; }
        const char* operator()(const DotPlaneParams&) const { return "Dot Plane"; }
        const char* operator()(const DotFountainParams&) const { return "Dot Fountain"; }
        const char* operator()(const ColorMapParams&) const { return "Color Map"; }
        const char* operator()(const BufferBlendParams&) const { return "Buffer Blend"; }
        const char* operator()(const JherikoGlobalParams&) const { return "Global Variables"; }
        const char* operator()(const ColorClipParams&) const { return "Color Clip"; }
        const char* operator()(const UniqueToneParams&) const { return "Unique Tone"; }
        const char* operator()(const InterleaveParams&) const { return "Interleave"; }
        const char* operator()(const ConvolutionParams&) const { return "Convolution"; }
        const char* operator()(const NormaliseParams&) const { return "Normalise"; }
        const char* operator()(const MultiFilterParams&) const { return "MultiFilter"; }
        const char* operator()(const AddBordersParams&) const { return "Add Borders"; }
        const char* operator()(const SimpleScopeParams&) const { return "Simple"; }
        const char* operator()(const BassSpinParams&) const { return "Bass Spin"; }
        const char* operator()(const OscStarParams&) const { return "Oscilliscope Star"; }
        const char* operator()(const OscRingParams&) const { return "Ring"; }
        const char* operator()(const RotatingStarsParams&) const { return "Rotating Stars"; }
        const char* operator()(const PictureParams&) const { return "Picture"; }
        const char* operator()(const PictureIIParams&) const { return "Picture II"; }
        const char* operator()(const TexerParams&) const { return "Texer"; }
        const char* operator()(const TexerIIParams&) const { return "Texer II"; }
        const char* operator()(const TriangleParams&) const { return "Triangle"; }
        const char* operator()(const ChannelShiftParams&) const { return "Channel Shift"; }
        const char* operator()(const ColorReductionParams&) const { return "Color Reduction"; }
        const char* operator()(const MultiplierParams&) const { return "Multiplier"; }
        const char* operator()(const VideoDelayParams&) const { return "Video Delay"; }
        const char* operator()(const MultiDelayParams&) const { return "Multi Delay"; }
        const char* operator()(const Fractal2DParams&) const { return "Fractal 2D"; }
        const char* operator()(const DomainWarpParams&) const { return "Domain Warp"; }
        const char* operator()(const Fractal3DParams&) const { return "Fractal 3D"; }
        const char* operator()(const LyapunovParams&) const { return "Lyapunov"; }
        const char* operator()(const KleinianParams&) const { return "Kleinian"; }
        const char* operator()(const FractalZoomerParams&) const { return "Fractal Zoomer"; }
        const char* operator()(const StrangeAttractorParams&) const { return "Strange Attractor"; }
        const char* operator()(const FlameParams&) const { return "Flame"; }
        const char* operator()(const ReactionDiffusionParams&) const { return "Reaction Diffusion"; }
        const char* operator()(const SetRenderModeParams&) const { return "Set Render Mode"; }
        const char* operator()(const DebugBarsParams&) const { return "Debug Bars"; }
        const char* operator()(const MilkdropNodeParams&) const { return "Milkdrop"; }
        const char* operator()(const TextParams&) const { return "Text"; }
        const char* operator()(const AviParams&) const { return "AVI"; }
        const char* operator()(const CommentParams&) const { return "Comment"; }
        const char* operator()(const ImportNotesParams&) const { return "Import Notes"; }
        const char* operator()(const RenderScaleParams&) const { return "Render Scale"; }
        const char* operator()(const BloomParams&) const { return "Bloom"; }
        const char* operator()(const Camera3DParams&) const { return "3D Camera"; }
        const char* operator()(const SuperScope3DParams&) const { return "SuperScope 3D"; }
        const char* operator()(const Terrain3DParams&) const { return "Terrain 3D"; }
        const char* operator()(const GlowOrbsParams&) const { return "Glow Orbs"; }
        const char* operator()(const ShadertoyParams&) const { return "Shadertoy"; }
        const char* operator()(const MeshWarpParams&) const { return "Mesh Warp"; }
        const char* operator()(const GpuParticlesParams&) const { return "GPU Particles"; }
        const char* operator()(const VideoSourceParams&) const { return "Video Source"; }
        const char* operator()(const PixelFilterParams&) const { return "Pixel Filter"; }
        const char* operator()(const IsfFilterParams&) const { return "ISF Filter"; }
        const char* operator()(const PassthroughParams&) const { return "Passthrough"; }
    };
    return std::visit(Visitor{}, params);
}

namespace detail {

inline uint64_t maxNodeId(const ChainNode& node)
{
    uint64_t maxId = node.nodeId;
    for (const ChainNode& child : node.children)
    {
        maxId = std::max(maxId, maxNodeId(child));
    }
    return maxId;
}

inline void warnFallbackBlend(BlendMode mode, const char* which,
                              const std::string& path, CompileResult& result)
{
    if (!isBlendModeImplemented(mode))
    {
        result.warnings.push_back(
            {path, std::string("blend mode \"") + blendModeName(mode) + "\" (" +
                       which + ") not implemented yet - falls back to Replace"});
    }
}

inline void compileNode(ChainNode& node, const std::string& path,
                        CompileResult& result, uint64_t& nextId,
                        bool inHostGroup = false)
{
    if (node.nodeId == 0) node.nodeId = nextId++;

    // Tiefenregel (HG1, Entwurf §5.3): Host-Gruppe in Host-Gruppe wird zur
    // Effect List degradiert (children bleiben, blendOut zieht um) — Warnung
    // statt Hard-Fail, defekte/fremde Dateien laden weiter.
    if (inHostGroup && node.isHostGroup())
    {
        const auto& hg = std::get<HostGroupParams>(node.params);
        ListParams asList;
        asList.blendOut = hg.blendOut;
        asList.outAdjustAlpha = hg.outAdjustAlpha;
        node.params = asList;
        result.warnings.push_back(
            {path, "nested host group is not allowed - degraded to an effect "
                   "list (depth rule)"});
    }

    if (node.displayName.empty()) node.displayName = effectTypeName(node.params);

    if (!node.isContainer() && !node.children.empty())
    {
        result.warnings.push_back(
            {path, std::string(effectTypeName(node.params)) +
                       ": children on a non-list node are ignored"});
    }

    // Clamp effect parameters to their AVS ranges (editor may write anything).
    if (auto* fade = std::get_if<FadeoutParams>(&node.params))
    {
        fade->fadeLen = std::clamp(fade->fadeLen, 0, 92);
    }
    if (auto* bright = std::get_if<BrightnessParams>(&node.params))
    {
        bright->red = std::clamp(bright->red, -4096, 4096);
        bright->green = std::clamp(bright->green, -4096, 4096);
        bright->blue = std::clamp(bright->blue, -4096, 4096);
        bright->distance = std::clamp(bright->distance, 0, 255);
    }
    if (auto* fast = std::get_if<FastBrightnessParams>(&node.params))
    {
        fast->dir = std::clamp(fast->dir, 0, 2);
    }
    if (auto* blur = std::get_if<BlurParams>(&node.params))
    {
        blur->strength = std::clamp(blur->strength, 1, 3);
    }
    if (auto* clearBeat = std::get_if<OnBeatClearParams>(&node.params))
    {
        if (clearBeat->everyNBeats < 1) clearBeat->everyNBeats = 1;
    }
    if (auto* fade = std::get_if<ColorfadeParams>(&node.params))
    {
        auto clampFader = [](int& v) { v = std::clamp(v, -32, 32); };
        clampFader(fade->faderR);
        clampFader(fade->faderG);
        clampFader(fade->faderB);
        clampFader(fade->beatFaderR);
        clampFader(fade->beatFaderG);
        clampFader(fade->beatFaderB);
        if (fade->onBeatFrames < 1) fade->onBeatFrames = 1;
    }
    if (auto* bpm = std::get_if<CustomBpmParams>(&node.params))
    {
        if (bpm->arbitraryMs < 1) bpm->arbitraryMs = 1;
        if (bpm->skipCount < 1) bpm->skipCount = 1;
    }
    if (auto* dmove = std::get_if<DynamicMovementParams>(&node.params))
    {
        dmove->xres = std::clamp(dmove->xres, 2, 256);  // AVS: r_dmove.cpp:235-238
        dmove->yres = std::clamp(dmove->yres, 2, 256);
        dmove->buffern = std::clamp(dmove->buffern, 0, 8);
    }
    if (auto* move = std::get_if<MovementParams>(&node.params))
    {
        move->sourceMapped = std::clamp(move->sourceMapped, 0, 3);
    }
    if (auto* fw = std::get_if<FyrewurXParams>(&node.params))
    {
        fw->sparks = std::clamp(fw->sparks, 1, 1024);
        fw->speed = std::clamp(fw->speed, 0.05f, 5.0f);
        fw->gravity = std::clamp(fw->gravity, 0.0f, 10.0f);
        fw->lifeSeconds = std::clamp(fw->lifeSeconds, 0.1f, 10.0f);
    }
    if (auto* save = std::get_if<BufferSaveParams>(&node.params))
    {
        save->slot = std::clamp(save->slot, 0, 7);
        save->dir = std::clamp(save->dir, 0, 3);
        save->adjustAlpha = std::clamp(save->adjustAlpha, 0, 255);
        if (save->dir != 0) warnFallbackBlend(save->blend, "restore", path, result);
    }
    if (auto* scope = std::get_if<SuperScopeParams>(&node.params))
    {
        scope->pointCount = std::clamp(scope->pointCount, 1, 4096);
        scope->renderMode = std::clamp(scope->renderMode, 0, 2);
        scope->audioChannel = std::clamp(scope->audioChannel, 0, 4);
        scope->lineWidth = std::clamp(scope->lineWidth, 1.0f, 255.0f);
        scope->dotSize = std::clamp(scope->dotSize, 1.0f, 50.0f);
        scope->lineBlend = std::clamp(scope->lineBlend, 0, 9);
    }
    if (auto* frac = std::get_if<Fractal2DParams>(&node.params))
    {
        frac->type = std::clamp(frac->type, 0, 8);
        frac->maxIter = std::clamp(frac->maxIter, 1, 2048);
        frac->zoom = std::max(frac->zoom, 1e-6f);
        frac->power = std::clamp(frac->power, 1.0f, 16.0f);
        frac->escapeR = std::max(frac->escapeR, 1.0f);
        frac->blend = std::clamp(frac->blend, 0, 2);
    }
    if (auto* warp = std::get_if<DomainWarpParams>(&node.params))
    {
        warp->octaves = std::clamp(warp->octaves, 1, 10);
        warp->blend = std::clamp(warp->blend, 0, 2);
    }
    if (auto* f3 = std::get_if<Fractal3DParams>(&node.params))
    {
        f3->type = std::clamp(f3->type, 0, 4);
        f3->maxSteps = std::clamp(f3->maxSteps, 8, 512);
        f3->maxIter = std::clamp(f3->maxIter, 1, 64);
        f3->power = std::clamp(f3->power, 1.0f, 16.0f);
        f3->dist = std::max(f3->dist, 0.1f);
        f3->blend = std::clamp(f3->blend, 0, 2);
    }
    if (auto* ly = std::get_if<LyapunovParams>(&node.params))
    {
        ly->warmup = std::clamp(ly->warmup, 0, 2000);
        ly->iterations = std::clamp(ly->iterations, 1, 4000);
        ly->blend = std::clamp(ly->blend, 0, 2);
    }
    if (auto* kl = std::get_if<KleinianParams>(&node.params))
    {
        kl->p = std::clamp(kl->p, 3, 20);
        kl->q = std::clamp(kl->q, 3, 20);
        kl->iterations = std::clamp(kl->iterations, 1, 200);
        kl->blend = std::clamp(kl->blend, 0, 2);
    }
    if (auto* fz = std::get_if<FractalZoomerParams>(&node.params))
    {
        fz->type = std::clamp(fz->type, 0, 2);
        fz->maxIter = std::clamp(fz->maxIter, 1, 2048);
        fz->feedback = std::clamp(fz->feedback, 0.0f, 1.0f);
    }
    if (auto* milk = std::get_if<MilkdropNodeParams>(&node.params))
    {
        // ranges = MilkdropVisualizer::paramDescs (Default 32x24, Cap 96x72)
        milk->meshX = std::clamp(milk->meshX, 8, 96);
        milk->meshY = std::clamp(milk->meshY, 6, 72);
        if (milk->revision == 0) milk->revision = 1;
    }
    if (auto* sa = std::get_if<StrangeAttractorParams>(&node.params))
    {
        sa->type = std::clamp(sa->type, 0, 3);
        sa->points = std::clamp(sa->points, 1, 100000);
        sa->blend = std::clamp(sa->blend, 0, 2);
    }
    if (auto* fl = std::get_if<FlameParams>(&node.params))
    {
        fl->variation = std::clamp(fl->variation, 0, 4);
        fl->functions = std::clamp(fl->functions, 2, 4);
        fl->points = std::clamp(fl->points, 1, 200000);
        fl->blend = std::clamp(fl->blend, 0, 2);
    }
    if (auto* rd = std::get_if<ReactionDiffusionParams>(&node.params))
    {
        rd->stepsPerFrame = std::clamp(rd->stepsPerFrame, 1, 64);
        rd->blend = std::clamp(rd->blend, 0, 2);
    }
    if (auto* srm = std::get_if<SetRenderModeParams>(&node.params))
    {
        srm->lineWidth = std::clamp(srm->lineWidth, 0, 255);
        srm->lineBlend = std::clamp(srm->lineBlend, 0, 9);
        srm->adjustAlpha = std::clamp(srm->adjustAlpha, 0, 255);
    }
    if (auto* list = std::get_if<ListParams>(&node.params))
    {
        list->inAdjustAlpha = std::clamp(list->inAdjustAlpha, 0, 255);
        list->outAdjustAlpha = std::clamp(list->outAdjustAlpha, 0, 255);
        if (list->onBeatFrames < 1) list->onBeatFrames = 1;
        warnFallbackBlend(list->blendIn, "in", path, result);
        warnFallbackBlend(list->blendOut, "out", path, result);
    }

    if (auto* group = std::get_if<HostGroupParams>(&node.params))
    {
        group->outAdjustAlpha = std::clamp(group->outAdjustAlpha, 0, 255);
        group->crossfadeSeconds = std::clamp(group->crossfadeSeconds, 0.0, 60.0);
        // 0=Linear 1=S-Kurve 2=Ease-In 3=Ease-Out 4=Exponentiell (HG2)
        group->curveIn = std::clamp(group->curveIn, 0, 4);
        group->curveOut = std::clamp(group->curveOut, 0, 4);
        warnFallbackBlend(group->blendOut, "out", path, result);
    }

    if (auto* sw = std::get_if<HostSwitcherParams>(&node.params))
    {
        const int anzahl = static_cast<int>(sw->groups.size());
        sw->startIndex = std::clamp(sw->startIndex, 1, std::max(anzahl, 1));
        sw->intervalSeconds = std::clamp(sw->intervalSeconds, 0.0, 3600.0);
        sw->order = std::clamp(sw->order, 0, 1);
    }

    if (node.isContainer())
    {
        const bool childInGroup = inHostGroup || node.isHostGroup();
        for (size_t i = 0; i < node.children.size(); ++i)
        {
            compileNode(node.children[i], path + "/" + std::to_string(i), result,
                        nextId, childInGroup);
        }
    }
}

/// Alle Host-Gruppen in Baumreihenfolge (nach compileNode: nur noch oberste Ebene).
template <class Node, class Fn>
void jedeHostGruppe(Node& node, Fn&& fn)
{
    if (node.isHostGroup()) fn(node);
    for (auto& child : node.children) jedeHostGruppe(child, fn);
}

/// „Host Group 3" → „Host Group" (Zaehler am Ende weg).
inline std::string nameOhneZaehler(const std::string& name)
{
    size_t ende = name.size();
    while (ende > 0 && name[ende - 1] >= '0' && name[ende - 1] <= '9') --ende;
    if (ende < name.size() && ende > 0 && name[ende - 1] == ' ') return name.substr(0, ende - 1);
    return name;
}

/// Kleinster freier Name „<basis> N" (N >= 2), der in `belegt` fehlt.
inline std::string freierName(const std::string& wunsch, const std::set<std::string>& belegt)
{
    if (belegt.count(wunsch) == 0) return wunsch;
    const std::string basis = nameOhneZaehler(wunsch);
    for (int n = 2;; ++n)
    {
        std::string kandidat = basis + " " + std::to_string(n);
        if (belegt.count(kandidat) == 0) return kandidat;
    }
}

/// Namen der Host-Gruppen eindeutig machen (S82): der Host Switcher spricht
/// Gruppen ueber den Namen an. Die ERSTE Gruppe eines Namens behaelt ihn,
/// spaetere bekommen den naechsten freien Zaehler („Host Group 2") — so
/// entstehen beim Einfuegen, Klonen und Laden alter Dateien eindeutige Namen.
inline void hostGruppenNamenEindeutig(ChainNode& root, CompileResult& result)
{
    std::set<std::string> alle;
    jedeHostGruppe(root, [&](ChainNode& g) { alle.insert(g.displayName); });
    std::set<std::string> gesehen;
    jedeHostGruppe(root, [&](ChainNode& g) {
        if (gesehen.insert(g.displayName).second) return;
        std::set<std::string> belegt = alle;
        belegt.insert(gesehen.begin(), gesehen.end());
        const std::string neu = freierName(g.displayName, belegt);
        result.warnings.push_back({"root", "host group name \"" + g.displayName +
                                               "\" was not unique - renamed to \"" +
                                               neu + "\""});
        g.displayName = neu;
        alle.insert(neu);
        gesehen.insert(neu);
    });
}

} // namespace detail

/**
 * @brief Validate + normalize the chain in place.
 *
 * Fills empty display names, clamps parameter ranges, reports structural
 * oddities as warnings (never hard-fails — AVS philosophy). Later steps add
 * the Set-Render-Mode roll-out and blend resolution here (decision E4:
 * re-propagated on every edit).
 *
 * @param root Root node; must be a list (warning + ok=false otherwise).
 * @param idHochwasser Optional: die naechste noch nie vergebene Id des
 *        Besitzers der Kette. Neue Knoten bekommen Ids ab dort, und der Wert
 *        wird fortgeschrieben — so traegt ein neuer Knoten nie die Id eines
 *        eben entfernten (S87: die Laufzeiten haengen an der Id; ein neuer
 *        Presetverweis zeigte sonst das Preset des entfernten).
 */
inline CompileResult compileChain(ChainNode& root, uint64_t* idHochwasser = nullptr)
{
    CompileResult result;
    if (!root.isList())
    {
        result.ok = false;
        result.warnings.push_back({"root", "root node must be an effect list"});
        return result;
    }
    uint64_t nextId = detail::maxNodeId(root) + 1;
    if (idHochwasser != nullptr) nextId = std::max(nextId, *idHochwasser);
    detail::compileNode(root, "root", result, nextId);
    if (idHochwasser != nullptr) *idHochwasser = nextId;
    detail::hostGruppenNamenEindeutig(root, result);
    return result;
}

/**
 * Freier Name fuer eine Host-Gruppe (S82): `wunsch` selbst, wenn keine ANDERE
 * Gruppe ihn traegt, sonst „<wunsch ohne Zaehler> N". `selbst` = die Gruppe,
 * die umbenannt wird (ihr eigener Name gilt nicht als belegt).
 */
[[nodiscard]] inline std::string freierHostGruppenName(const ChainNode& root,
                                                     const std::string& wunsch,
                                                     const ChainNode* selbst = nullptr)
{
    std::set<std::string> belegt;
    detail::jedeHostGruppe(root, [&](const ChainNode& g) {
        if (&g != selbst) belegt.insert(g.displayName);
    });
    return detail::freierName(wunsch, belegt);
}

/**
 * Umbenennen wie ein IDE-Refactoring (S82): jeder Host Switcher, der `alt`
 * listet, fuehrt danach `neu`. Rueckgabe = Anzahl ersetzter Eintraege.
 */
inline int hostGruppeUmbenennenInSwitchern(ChainNode& node, const std::string& alt,
                                           const std::string& neu)
{
    int ersetzt = 0;
    if (auto* sw = std::get_if<HostSwitcherParams>(&node.params))
    {
        for (std::string& name : sw->groups)
        {
            if (name == alt)
            {
                name = neu;
                ++ersetzt;
            }
        }
    }
    for (ChainNode& child : node.children)
        ersetzt += hostGruppeUmbenennenInSwitchern(child, alt, neu);
    return ersetzt;
}

/** true, wenn irgendwo im Baum eine Host-Gruppe sitzt (Persistenz: .lvfx2). */
[[nodiscard]] inline bool chainHasHostGroup(const ChainNode& node)
{
    if (node.isHostGroup()) return true;
    for (const ChainNode& child : node.children)
    {
        if (chainHasHostGroup(child)) return true;
    }
    return false;
}

/** Number of nodes in the tree, root excluded (AvsParser::effectCount rule). */
[[nodiscard]] inline int nodeCount(const ChainNode& root)
{
    int count = 0;
    for (const ChainNode& child : root.children)
    {
        count += 1 + nodeCount(child);
    }
    return count;
}

} // namespace lumi::multieffect
