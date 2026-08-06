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
#include <string>
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
    BlendMode blendOut = BlendMode::Replace;  ///< Gruppen-Buffer -> Parent
    int outAdjustAlpha = 128;                 ///< Adjustable out-alpha 0..255

    // Wechsel-Settings (synchron ueber alle Gruppen, Entwurf §2.4) — die
    // Ein-/Ausgangskurven sind dagegen individuell je Gruppe; HG2 wertet aus.
    double crossfadeSeconds = 2.0;  ///< Blend-Dauer beim Gruppen-Wechsel
    int curveIn = 0;                ///< 0 = linear (weitere Kurven mit HG2)
    int curveOut = 0;               ///< 0 = linear

    std::string sourceFile;  ///< importierte .lvfx-Quelle ("" = leer angelegt)
};

/** AVS "Render / Clear screen" (ID 25), reduced to the 5.1 core. */
struct ClearParams
{
    uint32_t color = 0x000000;  ///< 0x00RRGGBB
    bool onlyFirst = false;     ///< clear only on the first frame
    /// 0 = replace, 1 = additive, 2 = 50/50, 3 = current line-blend
    /// (r_clear.cpp: blend==1 -> BLEND, blendavg -> BLEND_AVG, blend==2 -> BLEND_LINE)
    int blend = 0;

    /// Parameter-Skript (Strang D): `blend` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Fadeout" (ID 3): per-frame clamped step towards a target color. */
struct FadeoutParams
{
    int fadeLen = 16;           ///< per-frame step 0..92 (AVS range)
    uint32_t color = 0x000000;  ///< target color 0x00RRGGBB

    /// Parameter-Skript (Strang D): `fadelen` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `red`, `green`, `blue`, `distance` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    int channel = 2;   ///< 0 L, 1 R, 2 center (Byte-Halbierung wie Original)
    int position = 2;  ///< 0 top, 1 bottom, 2 center
    /// Farbtafel (0x00RRGGBB), ueber die Frames durchgeschaltet.
    std::vector<uint32_t> colors{0xFFFFFF};

    /// Parameter-Skript (Strang D): `mode`, `channel`, `position` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    int channel = 2;   ///< 0 L, 1 R, 2 center (signed-halbierte Summe wie die Referenz)
    int position = 2;  ///< 0 left (w/4), 1 right (3w/4), 2 center
    int size = 8;      ///< spoke length 0..16 (r_oscstar: s = size/32)
    int rot = 3;       ///< rotation speed (r_oscstar: m_r += 0.01*rot; 0 = still)
    /// Farbtafel (0x00RRGGBB), ueber die Frames durchgeschaltet.
    std::vector<uint32_t> colors{0xFFFFFF};

    // Freigemachte Host-Konstanten (S53; S60 auf die Referenz geankert —
    // Default = exakter r_oscstar-Rechenweg).
    int spokes = 5;          ///< Zahl der Speichen (Referenz fest 5)
    float rotScale = 0.01f;  ///< Bogenmass je `rot`-Einheit und Frame (Referenz 0,01)
    float amplitude = 1.0f;  ///< Faktor auf den Wellenausschlag (Referenz 1)

    /// Parameter-Skript (Strang D): `size`, `rot`, `spokes`, `rotscale`,
    /// `amplitude` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Farbtafel (0x00RRGGBB), ueber die Frames durchgeschaltet.
    std::vector<uint32_t> colors{0xFFFFFF};

    // Freigemachte Host-Konstanten (S53), Vorgaben = bisheriges Verhalten.
    int segments = 80;        ///< Stuetzpunkte des Rings (war fest 80)
    float baseScale = 0.1f;   ///< Radius-Sockel ohne Audio (war fest 0,1)
    float audioScale = 0.9f;  ///< Audio-Anteil am Radius (war fest 0,9)

    /// Parameter-Skript (Strang D): `size`, `segments`, `basescale`,
    /// `audioscale` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Farbtafel (0x00RRGGBB), ueber die Frames durchgeschaltet.
    std::vector<uint32_t> colors{0xFFFFFF};

    int points = 5;          ///< Zacken je Stern (war fest 5)
    int skip = 2;            ///< Sprungweite beim Zeichnen; 2 = Pentagramm
    int stars = 2;           ///< Zahl der Sterne auf gegenueberliegenden Bahnen
    float rotSpeed = 0.1f;   ///< Bahndrehung je Frame (r_rotstar: r1 += 0.1)
    float orbit = 0.5f;      ///< Abstand vom Bildmittelpunkt (NDC; 0,5 = w/4 wie das Original)
    /// Sterngroesse ohne Audio, je Achse in NDC. Die Referenz rechnet
    /// w/8*(s+9)/88 (r_rotstar:145) = (s+9)/352 in NDC — die Vorgaben 9/352
    /// und 255/352 stellen bei Default exakt diese Formel her (S60).
    float baseRadius = 0.0255682f;
    float audioGain = 0.7244318f;  ///< Zuwachs aus der Spektrumsspitze (255/352, s. baseRadius)
    int bandLo = 3;            ///< erstes ausgewertetes Spektralband (Lokal-Peak-Suche)
    int bandHi = 14;           ///< erstes NICHT mehr ausgewertetes Band

    /// Parameter-Skript (Strang D): rechnet die Regler je Frame aus. Lesbare
    /// und schreibbare Namen sind die Feldnamen in Kleinschreibung
    /// (`points`, `skip`, `stars`, `rotspeed`, `orbit`, `baseradius`,
    /// `audiogain`), dazu `b`/`w`/`h` und der Audio-Satz. Leer = kein Skript.
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
    bool left = true;   ///< linke Bildhaelfte (linker Kanal) zeichnen
    bool right = true;  ///< rechte Bildhaelfte (rechter Kanal) zeichnen
    uint32_t colorLeft = 0xFFFFFF;   ///< Farbe der linken Figur 0x00RRGGBB
    uint32_t colorRight = 0xFFFFFF;  ///< Farbe der rechten Figur 0x00RRGGBB
    int mode = 1;  ///< 0 lines, 1 filled (approximated as lines)

    // KLASSE A: Vorgaben aus `r_bspin.cpp`, Abweichung wird gekennzeichnet.
    float smoothing = 0.7f;   ///< Gewicht des neuen Werts: `v = s*neu + (1-s)*alt`
    /// Drehschritt je Einheit. Vorgabe ist EXAKT der Ausdruck des Originals
    /// (`3.14159f/6.0f`), nicht das mathematische pi/6 — die beiden
    /// unterscheiden sich in der 7. Stelle, und das summiert sich ueber die
    /// Frames in der Drehlage auf.
    float spinStep = 3.14159f / 6.0f;

    /// Parameter-Skript (Strang D): `mode`, `smoothing`, `spinstep`.
    /// Klasse A — s. Hinweis bei `DotPlaneParams`.
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
    uint32_t clipColor = 0x202020;   ///< threshold colour 0x00RRGGBB
    uint32_t outColor = 0x202020;    ///< replacement colour
    int distance = 10;               ///< match radius (near mode)

    /// Parameter-Skript (Strang D): `distance` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    uint32_t color = 0xFFFFFF;  ///< tone colour 0x00RRGGBB
    bool invert = false;        ///< invert the depth ramp
    int blend = 0;              ///< 0 replace, 1 additive, 2 50/50

    /// Parameter-Skript (Strang D): `blend` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    uint32_t color = 0x000000;  ///< stripe colour 0x00RRGGBB
    int blend = 0;              ///< 0 replace, 1 additive, 2 50/50
    bool onBeat = false;        ///< ease to x2/y2 on beat
    int x2 = 1;                 ///< Ziel-Abstand waagrecht im Beat-Uebergang
    int y2 = 1;                 ///< Ziel-Abstand senkrecht im Beat-Uebergang
    int beatDuration = 4;       ///< ease length

    /// Parameter-Skript (Strang D): `x`, `y`, `x2`, `y2` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `blend` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Herkunftsnotiz aus dem Preset. Gezeichnet wird `imageData`; von diesem
    /// Pfad wird nie geladen.
    std::string filename;
    std::string imageData;   ///< base64 of the raw image ("" = unresolved)
    int blend = 2;           ///< 0 replace, 1 additive, 2 50/50

    /// Parameter-Skript (Strang D): `blend` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Fast Brightness" (ID 44): dir 0 = x2, 1 = x0.5, 2 = off. */
struct FastBrightnessParams
{
    int dir = 0;  ///< 0..2

    /// Parameter-Skript (Strang D): `dir` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Trans / Blur" (ID 6): box blur, strength selects the kernel. */
struct BlurParams
{
    int strength = 1;    ///< 1 = light, 2 = medium, 3 = heavy
    /// AVS "round mode": aus = jeder Teilterm wird abgeschnitten und das Bild
    /// klingt bei jeder Anwendung ab; an = fester Ausgleich je Kernel (+4/+5/+3).
    /// Vorgabe AUS wie im Original (`roundmode = 0`, r_blur.cpp:75/90) — bis S57
    /// stand hier `true`, und gelesen wurde das Feld ohnehin nirgends.
    bool roundUp = false;

    /// Parameter-Skript (Strang D): `strength` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `mode`, `slower` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `everyNBeats` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    int faderG = 8;   ///< Gruen-Schritt je Frame, -32..32
    int faderB = -8;  ///< Blau-Schritt je Frame, -32..32
    int beatFaderR = 8;   ///< Rot-Schritt im Beat-Fenster (ersetzt `faderR`)
    int beatFaderG = -8;  ///< Gruen-Schritt im Beat-Fenster (ersetzt `faderG`)
    int beatFaderB = 8;   ///< Blau-Schritt im Beat-Fenster (ersetzt `faderB`)
    /// LumiViz-ERWEITERUNG (Vorschlag Patrik): wie viele Frames der Beat-Zustand
    /// gehalten wird, bevor `slowFade` ihn zurueckzieht. Das Original kennt kein
    /// Fenster — es setzt die Fader im Beat-Frame und zieht ab dem naechsten
    /// zurueck. **Die Vorgabe 1 ist deshalb genau dieses Verhalten**, ein
    /// importiertes AVS-Preset aendert sich also nicht; groessere Werte sind die
    /// neue Moeglichkeit. Ohne `slowFade` wirkt das Feld nicht — dann gibt es
    /// keinen Beat-Zustand, den man halten koennte.
    int onBeatFrames = 1;
    /// `enabled`-Bit 1 von `r_colorfade`: im Beat-Frame werden die Fader
    /// ZUFAELLIG gewaehlt statt aus `beatFader*` genommen — Rot und Blau aus
    /// `rand()%32 - 6`, Gruen aus `rand()%64 - 32`, wobei Werte mit Betrag
    /// unter 16 auf +-32 aufgerissen werden (r_colorfade.cpp:157-161).
    bool onBeatRandom = false;
    /// `enabled`-Bit 2 von `r_colorfade`: die Fader wandern um EINEN Schritt je
    /// Frame auf ihren Zielwert zu, statt sofort dort zu stehen. Nur mit diesem
    /// Bit wirken die Beat-Fader ueberhaupt — im Original ist der Beat-Zweig
    /// sonst gar nicht erreichbar (`if (!(enabled&4)) … else if (isBeat) …`).
    bool slowFade = false;

    /// Parameter-Skript (Strang D): `faderr`, `faderg`, `faderb`, `beatfaderr`,
    /// `beatfaderg`, `beatfaderb`, `onbeatframes` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// EEL-Slots, die sich ihre Variablen mit dem Stufen-Code teilen: init
    /// einmal beim Aufbau, frame je Frame, beat je Beat.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string levelCode;   ///< runs per LUT entry (red/green/blue in/out)
    /// Tabelle je Frame neu rechnen statt einmal nach dem Uebersetzen. Noetig,
    /// sobald die Kurve von `time` oder vom Audio abhaengt.
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
    /// Freier Text. Der Knoten zeichnet nichts — er haelt eine Notiz in der
    /// Kette fest.
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
    /// Bericht des Importeurs (nur lesen): was am Preset nicht uebernommen
    /// werden konnte.
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
    int divisor = 2;   ///< interne Aufloesung = Fenster / divisor (1..8)
    int filter = 0;    ///< Upscale: 0 = nearest (authentisch grob), 1 = linear
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
    int downsample = 2;        ///< Glow-RT = interne Aufloesung / 2^n (0..4)
    int radius = 8;            ///< Gauss-Sigma in Glow-RT-Pixeln (25 Taps fix)
    float intensity = 1.0f;    ///< Faktor des additiven Glow-Anteils
    float threshold = 0.0f;    ///< Helligkeits-Schwelle vor dem Blur (0 = Referenz)
    bool vignette = false;     ///< Abschluss-Multiplikation 1 - r^2 * strength
    float vignetteStrength = 0.3f;  ///< Staerke der Randabdunklung (nur mit `vignette`)
    bool post = true;          ///< true: Glow nur beim Present (kein Feedback)

    /// Parameter-Skript (Strang D): `radius`, `intensity`, `threshold`, `vignettestrength` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    float px = 0.0f;               ///< Kamera-Position (Weltkoordinaten)
    float py = 0.0f;               ///< Kamera-Position, Hochachse (y+ = oben)
    float pz = 3.7320508f;         ///< +1/tan(15 deg): Default fuellt [-1,1]
    float tx = 0.0f;               ///< Blickziel
    float ty = 0.0f;               ///< Blickziel, Hochachse
    float tz = 0.0f;               ///< Blickziel, Tiefe (z+ = zum Betrachter)
    float fov = 30.0f;             ///< vertikaler Oeffnungswinkel (Grad)
    float roll = 0.0f;             ///< Drehung um die Blickachse (Grad)
    float fogStart = 0.0f;         ///< Fog-Distanz (Welt); start >= end = aus
    float fogEnd = 0.0f;           ///< Distanz, ab der der Nebel voll deckt
    uint32_t fogColor = 0x000000;  ///< 0x00RRGGBB (Sprites daempfen nur)
    /// EEL-Slots (Reihenfolge Frame VOR Beat): duerfen `px`..`pz`, `tx`..`tz`,
    /// `fov`, `roll`, `fogstart`, `fogend` ueberschreiben. Die Feldwerte oben
    /// sind die Startbelegung beim Uebersetzen.
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
    /// EEL-Quartett. init/frame/beat laufen einmal je Aufbau/Frame/Beat und
    /// setzen unter anderem `n`; der Punkt-Code laeuft je Punkt.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Laeuft je Punkt: `i` 0..1 und `v` (Audio) herein, gelesen werden `x`,
    /// `y`, `z` (Weltkoordinaten), `red`/`green`/`blue` und `size`.
    std::string pointCode;
    int pointCount = 256;   ///< Default-n (Skript-n ueberschreibt)
    int renderMode = 0;     ///< 0 = Soft-Sprites (Punkte), 1 = Linien
    float size = 0.05f;     ///< Default-Punktgroesse (Welt-Einheiten)
    float falloff = 4.0f;   ///< Sprite-Profil k in exp(-r^2*k)
    int audioChannel = 2;   ///< v-Quelle: 0 = links, 1 = rechts, 2 = mono
    bool spectrumSource = false;  ///< v liest Spektrum statt Waveform
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
    int resolution = 64;      ///< Gitterpunkte je Achse (8..128)
    float extent = 4.0f;      ///< Weltbreite/-tiefe des Grids
    float baseAmp = 0.15f;    ///< Amplitude der prozeduralen Basis
    float yOffset = -0.8f;    ///< Grundhoehe (Welt-y)
    float ringAmp = 1.0f;     ///< Spektrum-Ring-Injektion je Frame (0 = aus)
    float relax = 0.12f;      ///< Feder-Relaxation zur Basis (0..1)
    float flatten = 0.0f;     ///< direkter Zug Richtung Basis je Frame (0..1)
    bool drawMesh = true;     ///< dunkles Mesh (opak, schreibt Depth)
    uint32_t meshColor = 0x101418;  ///< Farbe des Gitters 0x00RRGGBB
    bool drawDots = true;     ///< additive Soft-Sprites an den Gitterpunkten
    float dotSize = 0.045f;   ///< Welt-Einheiten
    float falloff = 4.0f;     ///< Sprite-Profil k
    uint32_t colorLow = 0x0A2040;   ///< Palette: Tal
    uint32_t colorHigh = 0x40C0FF;  ///< Palette: Gipfel
    /// EEL-Slots fuer das Gelaende; sie teilen sich ihre Variablen mit dem
    /// Punkt-Code darunter.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pointCode;    ///< je Gitterpunkt: Farbe (red/green/blue)
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
    int orbCount = 5;            ///< Default-n (Skript-n ueberschreibt)
    float haloScale = 2.2f;      ///< Halo-Radius = radius * haloScale
    float haloIntensity = 0.6f;  ///< Halo-Helligkeit (0 = aus)
    float falloff = 3.0f;        ///< Halo-Profil k in exp(-r^2*k)
    /// EEL-Quartett. init/frame/beat laufen einmal je Aufbau/Frame/Beat und
    /// setzen unter anderem `n` (Zahl der Orbs); der Punkt-Code laeuft je Orb.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    std::string pointCode;       ///< je Orb (i = Orb-Index)
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
    bool italic = false;           ///< LOGFONT lfItalic — kursiv
    bool underline = false;        ///< LOGFONT lfUnderline — unterstrichen
    std::uint32_t color = 0xFFFFFF;  ///< Schriftfarbe 0x00RRGGBB
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
    bool outline = false;          ///< Kontur um die Zeichen zeichnen
    std::uint32_t outlineColor = 0;  ///< Konturfarbe 0x00RRGGBB
    int outlineSize = 1;           ///< Konturbreite in Pixeln
    bool shadow = false;           ///< shadow variant (outline wins if both)
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
};

/**
 * AVS "Trans / Dynamic Movement" (ID 43): grid-based scripted remap with the
 * full EEL quartet and a configurable grid resolution.
 */
struct DynamicMovementParams
{
    /// EEL-Quartett. Der Punkt-Code laeuft je Gitterknoten und schreibt
    /// `x`/`y` (kartesisch) bzw. `d`/`r` (polar) und `alpha`; init/frame/beat
    /// laufen einmal je Aufbau/Frame/Beat und teilen sich die Variablen.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Laeuft je Gitterknoten: gelesen werden `x`, `y`, `d`, `r`, `alpha` —
    /// wo der Bildpunkt seine Farbe HERholt, nicht wohin er geht.
    std::string pointCode;
    /// Gitterknoten waagrecht. Grober heisst weicher (dazwischen wird
    /// interpoliert) und schneller.
    int xres = 16;
    /// Gitterknoten senkrecht.
    int yres = 12;
    /// Punkt-Code rechnet in Bildpunkten statt in normierten Koordinaten.
    bool rectCoords = false;
    /// Was ueber den Rand hinauszeigt, kommt auf der Gegenseite wieder herein
    /// (aus statt dessen: der Rand wird festgehalten).
    bool wrap = false;
    /// r_dmove flags: `blend` mixes the moved pixel onto the original by the
    /// script's per-cell alpha; `nomove` skips displacement and only alpha-fades.
    bool blend = false;
    /// Nicht verschieben — nur das `alpha` des Skripts wirkt (Ueberblenden ohne
    /// Bewegung, r_dmove-Flag).
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
     * Skript-Variablen: **nur `d`** (+ `b` und der Audio-Satz).
     *
     * `d` ist eine **normierte Distanz**, kein Faktor: der Pixel-Code bekommt
     * die Entfernung des Zielpunkts vom Bildmitte als 0..1 herein und schreibt
     * die Entfernung zurueck, aus der gelesen werden soll. `d=d` ist damit die
     * Identitaet, `d=d*0.6` holt aus 60 % der Entfernung (Stauchung nach
     * aussen), `d=1.4` liest fuer JEDEN Punkt aus derselben Entfernung.
     * Den Skalierungsfaktor bildet erst die Tabelle daraus
     * (r_ddm.cpp:287-289: `m_tab[x] = d * 256 * max_d / (x+1)`).
     *
     * Anders als Movement und Dynamic Movement kennt dieser Effekt **weder
     * Winkel noch Koordinaten** — seine Tabelle laeuft ueber den Radius und
     * gilt fuer alle Richtungen gleich. Er kann deshalb nur stauchen und
     * dehnen, nie drehen oder verschieben.
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
     * Zwischenwerte beim Abtasten (AVS `subpixel`, r_ddm.cpp:313 /
     * r_shift.cpp:206).
     *
     * `false` liest den naechstgelegenen Bildpunkt — harte Kanten. `true`
     * mischt die vier Nachbarn wie AVS' BLEND4, also mit Ganzzahl-Schritten
     * und Abschneiden statt GL-Filterung (der Unterschied ist belegt, s.
     * MultiEffectVisualizer.cpp:676).
     *
     * Hiess bis S54 `bilinear` und war deshalb als einziges der SECHS
     * subpixel-Felder nicht verdrahtet — Movement, Dynamic Movement, Blitter
     * Feedback und Roto Blitter behielten den Originalnamen und reichen ihn
     * laengst durch. Alte `.lvfx` mit `"bilinear"` werden weiter gelesen.
     * Vorgabe wie im Original: AUS (r_ddm.cpp:210).
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
    uint32_t color = 0xFFFFFF;  ///< particle colour 0x00RRGGBB
    int maxDistance = 16;       ///< travel-radius scale (AVS maxdist)
    int size = 8;               ///< particle radius (px)
    int size2 = 8;              ///< on-beat radius
    bool onBeatSize = false;    ///< jump to size2 on beat (AVS enabled bit 1)
    int blend = 1;              ///< 0 replace, 1 additive, 2 50/50, 3 line

    // KLASSE A: Vorgaben aus `r_parts.cpp`, Abweichung wird gekennzeichnet.
    float spring = 0.004f;   ///< Zug zum Ziel je Frame (Original 0,004)
    float damping = 0.991f;  ///< Geschwindigkeitsdaempfung (Original 0,991)

    /// Parameter-Skript (Strang D): `size`, `size2`, `maxdistance`, `spring`,
    /// `damping`. Klasse A — s. Hinweis bei `DotPlaneParams`.
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
    /// EEL-Slots. Sie setzen `x`/`y` — die Verschiebung des ganzen Bildes —
    /// und teilen sich ihre Variablen.
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
     * Zwischenwerte beim Abtasten (AVS `subpixel`, r_ddm.cpp:313 /
     * r_shift.cpp:206).
     *
     * `false` liest den naechstgelegenen Bildpunkt — harte Kanten. `true`
     * mischt die vier Nachbarn wie AVS' BLEND4, also mit Ganzzahl-Schritten
     * und Abschneiden statt GL-Filterung (der Unterschied ist belegt, s.
     * MultiEffectVisualizer.cpp:676).
     *
     * Hiess bis S54 `bilinear` und war deshalb als einziges der SECHS
     * subpixel-Felder nicht verdrahtet — Movement, Dynamic Movement, Blitter
     * Feedback und Roto Blitter behielten den Originalnamen und reichen ihn
     * laengst durch. Alte `.lvfx` mit `"bilinear"` werden weiter gelesen.
     * Vorgabe wie im Original: AN (r_shift.cpp:127).
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
    int scale = 30;         ///< Zielwert des fpos-Ease (0..256; 32 = neutral)
    int scale2 = 30;        ///< Beat-Sprungwert (onBeat)
    bool onBeat = false;    ///< Beat setzt fpos = scale2
    bool blend = false;     ///< BLEND_AVG mit dem Original statt Replace
    bool subpixel = true;   ///< bilineares Sampling (BLEND4)

    /// Parameter-Skript (Strang D): `scale`, `scale2` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    int zoomScale = 31;          ///< Zoom-Zielwert (31 = neutral)
    int zoomScale2 = 31;         ///< Beat-Zoomwert (beatZoomJump)
    int rotDir = 31;             ///< theta = (rotDir-32) Grad je Frame
    bool blend = false;          ///< BLEND_AVG mit dem Original
    bool beatReverse = false;    ///< Beat kehrt die Drehrichtung um
    int beatReverseSpeed = 0;    ///< Ease des Richtungswechsels (0 = hart)
    bool beatZoomJump = false;   ///< Beat setzt fpos = zoomScale2
    bool subpixel = true;        ///< bilineares Sampling (BLEND4)

    /// Parameter-Skript (Strang D): `zoomscale`, `zoomscale2`, `rotdir` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `slot`, `dir`, `adjustalpha` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Rohwert `skipVal` der Referenz: durchgelassen wird jeder
    /// (skipCount+1)-te Beat, 0 heisst also "jeden".
    int skipCount = 1;
    bool invert = false;     ///< invert the (possibly modified) beat
    /// `skipfirst`: die ersten N Beats des Presets werden verschluckt, und der
    /// Skip-Zaehler laeuft waehrenddessen NICHT mit (r_bpm.cpp:148 kehrt vor
    /// dem Skip-Block zurueck).
    int skipFirst = 0;

    /// Parameter-Skript (Strang D): `arbitraryms`, `skipcount`, `skipfirst` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
};

/**
 * AVS "Render / SuperScope" (ID 36): a scripted point/line scope. The point
 * script (EEL quartet) is run by a SuperscopeModule; the host draws the points
 * via the shared ScopeRenderer (decision E6). `renderMode` 0=dots 1=lines
 * 2=thick; `audioChannel` 0=L 1=R 2=mono 3=mid 4=side (SuperscopeAudioChannel).
 */
struct SuperScopeParams
{
    /// EEL-Quartett. init/frame/beat laufen einmal je Aufbau/Frame/Beat und
    /// setzen unter anderem `n` (Punktzahl); der Punkt-Code laeuft je Punkt.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Laeuft je Punkt: `i` 0..1 und `v` (Audio) herein, gelesen werden `x`/`y`
    /// (-1..1), `red`/`green`/`blue`, `linesize`, `skip` und `drawmode`.
    std::string pointCode;
    /// Startwert fuer `n` — der Punkt-Code laeuft so oft. Ein Skript, das `n`
    /// selbst setzt, gewinnt.
    int pointCount = 256;
    int renderMode = 1;    ///< 0=dots, 1=lines, 2=thick lines
    /// Strichstaerke in Pixeln fuer `renderMode` 1/2; `linesize` aus dem
    /// Punkt-Code gewinnt.
    float lineWidth = 2.0f;
    /// Punktdurchmesser in Pixeln fuer `renderMode` 0.
    float dotSize = 4.0f;
    int audioChannel = 2;  ///< 0=L 1=R 2=mono 3=mid 4=side
    /// AVS which_ch bit 4 (r_sscope.cpp:232): v liest SPEKTRUM statt Waveform.
    bool spectrumSource = false;
    int lineBlend = 1;     ///< AVS BLEND_LINE mode 0..9 (S9; s. SetRenderModeParams)
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

    /// Parameter-Skript (Strang D): `quality`, `quality2` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `amount` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    int dropY = 1;          ///< dito senkrecht: 0 oben / 1 Mitte / 2 unten
    int dropRadius = 40;    ///< drop radius (px)
    /// Verstaerkung des Versatzes. Seit der Ganzzahl-Umstellung (S59) stehen
    /// die Hoehen in REFERENZ-Einheiten — 1.0 = exakt r_waterbump; der alte
    /// Wert 6.0 stammte aus der normierten Aera und verzerrte 6-fach.
    float displaceScale = 1.0f;

    /// Parameter-Skript (Strang D): `density`, `depth`, `dropradius`, `displacescale` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// EEL-Slots. Sie setzen `x`/`y` — die Lichtquelle in 0..1 (mit `oldStyle`
    /// in 0..100) — und teilen sich ihre Variablen.
    // Vorgabe LEER — der Deserialisierer liess diese Slots beim Laden schon
    // immer leer (`getStr` ohne Vorgabewert), und seit S56 ist der Struct die
    // einzige Quelle. Die frueheren Demo-Skripte gehoeren als VOREINSTELLUNG
    // hierher (Konzept §11), nicht als Vorgabe: ein importiertes Preset darf
    // sie nicht erben. Sie stehen in `Offene_Punkte.md §1d`.
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

    /// Parameter-Skript (Strang D): `points`, `distance`, `alpha`, `rotation`, `rotationinc`, `speed` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Palette (0x00RRGGBB). NICHT leer: der Leser heilt eine leere Tafel auf
    /// Weiss, eine leere liesse sich also gar nicht speichern (Roundtrip-Befund
    /// S56). Die Vorgabe sagt es deshalb selbst.
    std::vector<uint32_t> colors{0xFFFFFF};
    int count = 7;                 ///< Zahl der Kugeln (1..16)
    /// Radius-Skala je Kugel (NDC) — sichtkalibriert gegen die echte APE
    /// (S52): 0,30 deckte deutlich mehr Flaeche als die Referenz.
    float radius = 0.20f;
    float speed = 0.45f;           ///< Bahngeschwindigkeit
    float threshold = 1.0f;        ///< Isowert der Oberflaeche
    /// 0 replace, 1 additiv, 2 50/50 — die Referenz zeichnet DECKENDE
    /// Koerper (Sichtvergleich S52), deshalb Replace als Vorgabe.
    int blend = 0;

    // Freigemachte Bahn-Konstanten (S53), Vorgaben = bisheriges Verhalten.
    float spread = 1.0f;  ///< Faktor auf die Bahnweite (war fest 1)
    float depth = 1.2f;   ///< Kamera-Abstand der Bahnmitte (war fest 1,2)
    float phase = 1.7f;   ///< Phasenversatz je Kugel (war fest 1,7)

    /// Parameter-Skript (Strang D): `count`, `radius`, `speed`, `threshold`,
    /// `spread`, `depth`, `phase` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Palette (0x00RRGGBB). NICHT leer: der Leser heilt eine leere Tafel auf
    /// Weiss, eine leere liesse sich also gar nicht speichern (Roundtrip-Befund
    /// S56). Die Vorgabe sagt es deshalb selbst.
    std::vector<uint32_t> colors{0xFFFFFF};
    int count = 7;                 ///< Zahl der Tentakel (1..16)
    int segments = 28;             ///< Stuetzpunkte je Tentakel
    float length = 0.85f;          ///< Laenge in NDC
    float thickness = 9.0f;        ///< Linienbreite an der Wurzel (Pixel)
    float speed = 0.7f;            ///< Schwinggeschwindigkeit
    int blend = 1;                 ///< 0 replace, 1 additiv, 2 50/50

    // Freigemachte Schwing-Konstanten (S53), Vorgaben = bisheriges Verhalten.
    float sway = 0.9f;    ///< Schwingweite an der Spitze (war fest 0,9)
    float waves = 3.1f;   ///< Wellen ueber die Laenge (war fest 3,1)
    float taper = 1.0f;   ///< Verjuengung zur Spitze; 0 = gleich dick

    /// Parameter-Skript (Strang D): `count`, `segments`, `length`, `thickness`,
    /// `speed`, `sway`, `waves`, `taper` + `b`/`w`/`h` + Audio-Satz.
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
    float dotSize = 2.0f;      ///< Funkengroesse in Pixeln (war fest 2)
    float hueDrift = 0.6f;     ///< Farbstreuung je Funke (war fest 0,6)
    float burstSpread = 1.0f;  ///< Faktor auf die Streuung der Burst-Mitte

    /// Parameter-Skript (Strang D): `sparks`, `speed`, `gravity`, `life`,
    /// `dotsize`, `huedrift`, `burstspread` + `b`/`w`/`h` + Audio-Satz.
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

    /// Parameter-Skript (Strang D): `maxstars`, `warpspeed`, `beatspeed`, `durationframes` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// 0 replace, 1 additive, 2 50/50, 3 BLEND_LINE (folgt dem Set-Render-Mode-
    /// Zustand — AVS-Default "Default Blend", r_timescope.cpp:147-148, S3)
    int blend = 3;
    int channel = 2;           ///< 0 L, 1 R, 2 center
    int bands = 576;           ///< vertical spectrum resolution
    /**
     * `channel` wirklich anwenden (Erweiterung, Vorgabe AUS = referenztreu).
     *
     * r_timescope.cpp berechnet aus `which_ch` zwar ein `fa_data`, die
     * Zeichenschleife liest dann aber fest `visdata[0][0]` — den linken
     * Spektrumkanal; `fa_data` wird nie benutzt (S48-Matrix-Befund 39). Der
     * Regler steht also im Original-Dialog, ohne etwas zu tun.
     *
     * Bei `false` bleibt es genau dabei, damit importierte Presets (die
     * ueberwiegend `which_ch=2` tragen) weiter wie die Referenz aussehen. Bei
     * `true` zaehlt `channel`: 0 links, 1 rechts, 2 Mittelwert beider.
     *
     * Steht bewusst HINTER `bands`: die Aggregat-Initialisierung
     * `TimescopeParams{color, blend, channel, bands}` ist in Tests und
     * Uebersetzer in Gebrauch und soll gueltig bleiben.
     */
    bool useChannel = false;

    /// Parameter-Skript (Strang D): `bands`, `channel`, `blend` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Render / Dot Grid" (ID 17): a scrolling grid of dots whose colour cycles
 *  through `colors` (r_dotgrid.cpp). `spacing` px, `xMove/yMove` scroll speed. */
struct DotGridParams
{
    /// Farbtafel (0x00RRGGBB). Mehrere Eintraege werden ueber die Frames
    /// durchgeschaltet, wie die AVS-Farbtabelle.
    std::vector<uint32_t> colors{0xFFFFFF};
    int spacing = 8;   ///< grid spacing (px)
    int xMove = 128;   ///< horizontal scroll (fixed-point /256 per frame)
    int yMove = 128;   ///< vertical scroll
    /// 0 replace, 1 additive, 2 50/50, 3 BLEND_LINE (SRM-Zustand,
    /// r_dotgrid.cpp:151-159, S3)
    int blend = 0;

    /// Parameter-Skript (Strang D): `spacing`, `xmove`, `ymove`, `blend` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Render / Dot Plane" (ID 1): a rotating audio-reactive point plane, height
 *  from the spectrum, coloured by a 5-stop gradient (r_dotpln.cpp). 3D projection
 *  scale is host tuning (sight-test). */
struct DotPlaneParams
{
    /// Fuenf Stuetzstellen des Farbverlaufs (0x00RRGGBB), ueber die Hoehe der
    /// Gitterpunkte abgebildet.
    uint32_t colors[5] = {0x0000FF, 0x00FFFF, 0x00FF00, 0xFFFF00, 0xFF0000};
    int rotVel = 16;   ///< rotation speed (-50..50 in AVS)
    int angle = -20;   ///< viewing tilt angle
    /// Start-Drehwinkel in Grad — das Original SPEICHERT seine laufende
    /// Rotation im Preset (`r = rr/32`, r_dotpln load_config) und steht damit
    /// beim Laden sofort in der gespeicherten Stellung (Tie Tunnel: 44,84°).
    float startRotation = 0.0f;

    // KLASSE A (Knoten_Parameter_Konzept §2): die Vorgaben sind die Werte aus
    // `r_dotpln.cpp` — eine Abweichung entfernt das Bild MESSBAR von der
    // Referenz. Der Editor kennzeichnet sie deshalb (Entscheid Patrik §8.4).
    // Das 64x64-Gitter bleibt fest: es ist die Struktur der Portierung, kein
    // Parameter (NUM_WIDTH, steckt in Stack-Arrays).
    float camDistance = 400.0f;  ///< matrixTranslate z (Original 400)
    float settle = 0.15f;        ///< Absinken je Frame: `vel -= settle*h/255`

    /// Parameter-Skript (Strang D): `rotvel`, `angle`, `camdistance`, `settle`.
    /// ACHTUNG Klasse A — ein Skript hier verlaesst die Referenz; der Editor
    /// sagt das an, weil die ⚠ an den Reglern nur feste Werte pruefen kann.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS "Render / Dot Fountain" (ID 19): a 3D particle fountain coloured by a
 *  5-stop gradient, rotating (r_dotfnt.cpp). Simplified particle model here;
 *  projection/physics scale is host tuning (sight-test). */
struct DotFountainParams
{
    /// Fuenf Stuetzstellen des Farbverlaufs (0x00RRGGBB), ueber die Hoehe der
    /// Teilchen abgebildet.
    uint32_t colors[5] = {0x0000FF, 0x00FFFF, 0x00FF00, 0xFFFF00, 0xFF0000};
    /// Drehgeschwindigkeit (Vorzeichen = Richtung).
    int rotVel = 16;
    /// Neigung der Ansicht in Grad.
    int angle = -20;
    /// Start-Drehwinkel in Grad — wie Dot Plane speichert das Original seine
    /// laufende Rotation im Preset (`r = rr/32`, r_dotfnt load_config).
    float startRotation = 0.0f;

    /// Parameter-Skript (Strang D): `rotvel`, `angle` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `buffera`, `bufferb`, `mode` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// EEL-Slots ohne eigenes Bild: der Knoten rechnet nur und legt seine
    /// Ergebnisse in den geteilten Variablen (`reg00`..`reg99`, `gmegabuf`) ab,
    /// aus denen die folgenden Knoten lesen. `loadMode` steuert, wann der
    /// Init-Slot erneut laeuft.
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
    /// Betrag des Ergebnisses nehmen — negative Summen werden gespiegelt statt
    /// auf 0 geklemmt (macht Kanten-Kerne beidseitig sichtbar).
    bool absolute = false;
    /// Den Kern zweimal hintereinander anwenden.
    bool twoPass = false;
    int edgeMode = 0;   ///< 0 extend, 1 wrap
    /// Festwert, der nach der Division auf jeden Kanal addiert wird.
    int bias = 0;
    /// Teiler der gewichteten Summe (0 wird als 1 gerechnet). Die Summe der
    /// Kernwerte hier einzutragen haelt die Helligkeit.
    int scale = 1;
    std::array<int, 49> kernel = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  ///< identity (center=1)

    /// Parameter-Skript (Strang D): `bias`, `scale`, `edgemode` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `effect` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    uint32_t color = 0xFFFFFF;  ///< border colour 0x00RRGGBB
    int size = 2;               ///< border width (px)

    /// Parameter-Skript (Strang D): `size` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Herkunftsnotiz aus dem Preset. Gezeichnet wird `imageData`; von diesem
    /// Pfad wird nie geladen.
    std::string filename;
    /// Das eingebettete Bild (base64). Leer = eingebautes Standard-Sprite.
    std::string imageData;
    /// 0 ersetzen, 1 additiv, 2 50/50.
    int blend = 1;
    /// Anzahl der Sprites entlang der Wellenform.
    int particles = 100;

    /// Parameter-Skript (Strang D): `blend`, `particles` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Herkunftsnotiz aus dem Preset. Gezeichnet wird `imageData`; von diesem
    /// Pfad wird nie geladen.
    std::string filename;
    /// Das eingebettete Bild (base64). Leer = eingebautes Standard-Sprite
    /// (20x20, radialsymmetrisch — gemessen gegen die echte texer2.ape).
    std::string imageData;
    /// `sizex`/`sizey` aus dem Punkt-Code wirken lassen (aus: feste Groesse).
    bool resizing = false;
    /// Sprites am Bildrand auf der Gegenseite fortsetzen statt abzuschneiden.
    bool wrapAround = false;
    /// `red`/`green`/`blue` aus dem Punkt-Code als Farbfilter auf das Bild
    /// legen (aus: das Bild wird unveraendert gezeichnet).
    bool colorFiltering = true;
    /// EEL-Quartett. init/frame/beat laufen einmal je Aufbau/Frame/Beat und
    /// setzen `n` (Anzahl Sprites); der Punkt-Code laeuft je Sprite.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Laeuft je Sprite: `i` 0..1 herein, gelesen werden `x`/`y` (-1..1),
    /// `sizex`/`sizey` (nur mit `resizing`) und `red`/`green`/`blue` (nur mit
    /// `colorFiltering`).
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
    /// EEL-Quartett. init/frame/beat laufen einmal je Aufbau/Frame/Beat und
    /// setzen `n` (Anzahl Dreiecke); der Punkt-Code laeuft je Dreieck.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
    /// Laeuft je Dreieck: gelesen werden `x1`/`y1`, `x2`/`y2`, `x3`/`y3`
    /// (-1..1) und `red`/`green`/`blue`.
    std::string pointCode;

    /// GEFUELLT ist der Referenz-Zustand (Sonde `triangle_n_literal`, S51: das
    /// fruehere Drahtgitter lieferte 4009 statt 23424 Pixel). `false` ist damit
    /// eine bewusste Abweichung von AVS, kein Alternativ-Default.
    bool filled = true;
    float lineWidth = 1.0f;  ///< Kantenbreite im Drahtgitter-Modus
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
    std::vector<uint32_t> stopColor;  ///< gradient stop colours 0x00RRGGBB

    /// Parameter-Skript (Strang D): `key`, `blendmode`, `adjustblend` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `mode` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS APE "Color Reduction": quantise each channel to 2^`levels` values
 *  (r_colorreduction). levels 1..8 (8 = unchanged). */
struct ColorReductionParams
{
    int levels = 8;  ///< bit depth per channel 1..8

    /// Parameter-Skript (Strang D): `levels` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/** AVS APE "Multiplier": scale pixel values (r_multiplier). `mode` 0 saturate,
 *  1 x8, 2 x4, 3 x2, 4 x0.5, 5 x0.25, 6 x0.125, 7 zero-else-keep. */
struct MultiplierParams
{
    int mode = 3;  ///< 0..7 (see above)

    /// Parameter-Skript (Strang D): `mode` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `delay` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    /// Parameter-Skript (Strang D): `delay`, `buffer` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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

    bool smooth = true;               ///< continuous (fractional) iteration colouring
    float colorScale = 0.05f;         ///< iteration→LUT position factor
    float colorCycle = 0.0f;          ///< LUT offset advance per second (palette drift)
    uint32_t insideColor = 0x000000;  ///< colour of the non-escaping set (0x00RRGGBB)
    std::string gradientPreset = "Neon";  ///< ColorGradientModule palette name

    int blend = 0;  ///< 0 replace, 1 additive, 2 50/50 over the current image

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
    /// Quaternion-Julia-Konstante, zweite Komponente — nur bei `type = 3`.
    float juliaY = 0.3f;
    /// Quaternion-Julia-Konstante, dritte Komponente — nur bei `type = 3`.
    float juliaZ = 0.1f;
    /// Quaternion-Julia-Konstante, vierte Komponente — nur bei `type = 3`.
    float juliaW = 0.0f;
    float lightYaw = 0.7f;    ///< key-light direction (azimuth)
    float lightPitch = 0.8f;  ///< key-light direction (elevation)
    float ambient = 0.2f;     ///< ambient floor
    bool ao = true;           ///< cheap ambient occlusion

    /// Faerbung: Farbtafel-Index = `fract(Trefferwert * colorScale + Phase)`.
    float colorScale = 1.0f;
    /// Vorschub der Farbphase je Sekunde (0 = stehende Farben).
    float colorCycle = 0.0f;
    /// Name des Farbverlaufs, ueber den die Oberflaeche eingefaerbt wird.
    std::string gradientPreset = "Neon";
    uint32_t background = 0x000000;  ///< miss colour (0x00RRGGBB)

    /// Auf das bestehende Bild: 0 ersetzen, 1 additiv (geklemmt), 2 50/50.
    int blend = 0;
    /// Parameter-Skript (Strang D): `yaw`, `pitch`, `dist`, `power`, `scale`,
    /// `fold` + Audio-Satz — Kamera und Form live. Leer = kein Skript.
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
    /// Rechte Kante des Ausschnitts auf der a-Achse.
    float aMax = 4.0f;
    float bMin = 2.5f;            ///< view rectangle (b axis)
    /// Obere Kante des Ausschnitts auf der b-Achse.
    float bMax = 4.0f;
    int warmup = 100;            ///< settle iterations before measuring
    int iterations = 400;        ///< measured iterations
    uint32_t negColor = 0x000030;  ///< ordered-zone colour (negative exponent)

    /// Faerbung: Farbtafel-Index = `fract(Lyapunov-Exponent * colorScale + Phase)`.
    /// Gilt nur fuer die chaotischen Zonen; die geordneten bekommen `negColor`.
    float colorScale = 1.0f;
    /// Vorschub der Farbphase je Sekunde (0 = stehende Farben).
    float colorCycle = 0.0f;
    /// Name des Farbverlaufs fuer die chaotischen Zonen.
    std::string gradientPreset = "Fire";

    /// Auf das bestehende Bild: 0 ersetzen, 1 additiv (geklemmt), 2 50/50.
    int blend = 0;
    /// Parameter-Skript (Strang D): `aMin`, `aMax`, `bMin`, `bMax` — der
    /// Ausschnitt — + Audio-Satz. Leer = kein Skript.
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
    /// Ausschnitt der Poincaré-Scheibe (groesser = naeher heran).
    float zoom = 1.0f;
    /// Drehung der Kachelung in Radiant.
    float rotation = 0.0f;

    /**
     * Faerbung: Farbtafel-Index = `fract(Spiegelungszahl * colorScale + Phase)`.
     *
     * Die Spiegelungszahl ist eine GANZE Zahl — ein ganzzahliges `colorScale`
     * macht den Ausdruck damit fuer JEDE Zelle zu exakt 0, und der Knoten
     * zeichnet eine einfarbige Scheibe statt einer Kachelung. Genau das war bis
     * S56 die Vorgabe (1,0), und es hat den ganzen Effekt unsichtbar gemacht:
     * gemessen liefern 1,0 und 4,0 Pixel fuer Pixel dasselbe Bild (MAE 0,0000),
     * erst 0,17 zeigt die {p,q}-Kachelung. Fuenf weitere Felder (`p`, `q`,
     * `morph`, `iterations`, `rotation`) standen deshalb als „stumm" da — sie
     * aendern eine Struktur, die man nicht sehen konnte.
     *
     * Der Wert 0,17 laesst die Farbe ueber rund sechs Spiegelungsstufen einmal
     * durch die Tafel laufen; das ist die Tiefe, die im Bild vorkommt.
     * Ganzzahlige Werte bleiben einstellbar — sie sind hier schlicht die
     * Einstellung „alles in einer Farbe".
     */
    float colorScale = 0.17f;
    /// Vorschub der Farbphase je Sekunde (0 = stehende Farben).
    float colorCycle = 0.0f;
    /// Name des Farbverlaufs, ueber den die Spiegelungszahl abgebildet wird.
    std::string gradientPreset = "Neon";

    /// Auf das bestehende Bild: 0 ersetzen, 1 additiv (geklemmt), 2 50/50.
    int blend = 0;
    /// Parameter-Skript (Strang D): `p`, `q` (als Zahlen), `morph`, `zoom`,
    /// `rotation` + Audio-Satz. Leer = kein Skript.
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
    /// Zoomziel, Imaginaerteil (Gegenstueck zu `centerX`).
    float centerY = 0.131825904f;
    /// Julia-Konstante, Realteil — wirkt nur bei `type = 1`.
    float juliaX = -0.8f;
    /// Julia-Konstante, Imaginaerteil — wirkt nur bei `type = 1`.
    float juliaY = 0.156f;
    /// Obergrenze der Iterationen je Pixel. Hoeher heisst mehr Struktur tief im
    /// Zoom, aber auch mehr Rechenzeit je Frame.
    int maxIter = 200;
    float zoomSpeed = 1.02f;      ///< per-frame zoom factor (>1 zooms in)
    float rotationSpeed = 0.0f;   ///< radians per frame
    /// Anteil des vorigen Bildes, der stehen bleibt: 0 = keine Schleife,
    /// 0,5 = 50/50, 1 = Standbild. Seit S57 eine echte STAERKE — vorher las der
    /// Shader das Feld nur als Schalter (`> 0,01 ? 50/50 : ersetzen`), und 0,3
    /// wie 1,0 ergaben dasselbe Bild.
    float feedback = 0.5f;

    /// Faerbung: Farbtafel-Index = `fract(iterationen * colorScale + Phase)`.
    /// Groesser heisst engere Farbringe.
    float colorScale = 0.05f;
    /// Vorschub der Farbphase je Sekunde (0 = stehende Farben).
    float colorCycle = 0.0f;
    /// Name des Farbverlaufs, ueber den die Iterationszahl abgebildet wird.
    std::string gradientPreset = "Neon";
    /// Farbe der Punkte, die nicht entkommen (das Innere der Menge), 0x00RRGGBB.
    uint32_t insideColor = 0x000000;

    /// Parameter-Skript (Strang D): `centerX`, `centerY`, `zoomSpeed`,
    /// `rotationSpeed` + Audio-Satz. Leer = kein Skript.
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
    /// Erster Formelbeiwert. Was er bedeutet, haengt am `type`: Lorenz sigma
    /// (x7,142857), Clifford/De Jong der Faktor im Sinus, Aizawa der z-Term.
    float a = 1.4f;
    /// Zweiter Beiwert: Lorenz rho (x17,5), Clifford/De Jong der zweite
    /// Sinus-/Kosinus-Faktor, Aizawa die z-Verschiebung.
    float b = 1.6f;
    /// Dritter Beiwert: Lorenz beta (x2,6667), Clifford der Kosinus-Anteil in x,
    /// De Jong der Sinus-Faktor in y, Aizawa das konstante Glied.
    float c = 1.0f;
    /// Vierter Beiwert: Clifford/De Jong der Kosinus-Anteil in y, Aizawa die
    /// Drehung in der xy-Ebene. Lorenz nutzt ihn nicht.
    float d = 0.7f;
    /// Punkte je Frame (1..100000). Die Bahn laeuft ueber Frames weiter — mehr
    /// Punkte heisst dichter gezeichnet, nicht laenger.
    int points = 6000;
    /// Groesse der Bahn im Bild (Faktor auf die Koordinaten, vor der Drehung).
    float scale = 0.28f;
    /// Fester Drehwinkel der Ansicht in Radiant, addiert zur laufenden Drehung.
    float rotation = 0.0f;
    /// Laufende Drehung in Radiant je Sekunde (0 = steht still).
    float rotationSpeed = 0.08f;

    /// Punktfarbe 0x00RRGGBB, wenn `useGradient` aus ist.
    uint32_t color = 0x66CCFF;
    bool useGradient = true;      ///< colour dots along the orbit via the gradient
    /// Name des Farbverlaufs fuer `useGradient` — die Bahn wird von Anfang bis
    /// Ende darueber eingefaerbt.
    std::string gradientPreset = "Neon";
    /// Punktdurchmesser in Pixeln (1..32).
    float dotSize = 2.0f;
    int blend = 1;                ///< 0 replace, 1 additive, 2 50/50

    /// Parameter-Skript (Strang D): `a`, `b`, `c`, `d`, `rotation` + Audio-Satz.
    /// Leer = kein Skript. Achtung: eine Aenderung der Beiwerte im Panel setzt
    /// den Traeger neu auf und laesst den Init-Slot erneut laufen.
    std::string initCode;
    std::string frameCode;
    std::string beatCode;
};

/**
 * Batch H (host-native): a Flame / IFS (chaos-game) renderer. Each frame runs the
 * chaos game for `points` iterations over a small affine function system with a
 * non-linear `variation`, drawing the samples as additive dots. `functions`
 * selects the preset system (2..4 maps). EEL writes variation params / rotation.
 */
struct FlameParams
{
    int variation = 0;   ///< 0 linear 1 sinusoidal 2 spherical 3 swirl 4 horseshoe
    int functions = 3;   ///< affine map count (2..4)
    /// Zuege des Chaosspiels je Frame — mehr heisst ein dichteres Bild, nicht
    /// ein anderes.
    int points = 20000;
    /// Groesse der Figur im Bild (Faktor auf die Koordinaten, vor der Drehung).
    float scale = 0.5f;
    /// Fester Drehwinkel in Radiant, addiert zur laufenden Drehung.
    float rotation = 0.0f;
    /// Laufende Drehung in Radiant je Sekunde (0 = steht still).
    float rotationSpeed = 0.04f;

    /// Name des Farbverlaufs — die Punkte werden ueber ihre Nummer im Zug
    /// eingefaerbt.
    std::string gradientPreset = "Fire";
    /// Punktdurchmesser in Pixeln.
    float dotSize = 1.5f;
    /// 0 ersetzen, 1 additiv, 2 50/50.
    int blend = 1;

    /// Parameter-Skript (Strang D): die Variationsbeiwerte und `rotation` +
    /// Audio-Satz. Leer = kein Skript.
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

    /// Faerbung: Farbtafel-Index = `fract(Konzentration von B * colorScale + Phase)`.
    float colorScale = 1.0f;
    /// Vorschub der Farbphase je Sekunde (0 = stehende Farben).
    float colorCycle = 0.0f;
    /// Name des Farbverlaufs, ueber den die Konzentration abgebildet wird.
    std::string gradientPreset = "Neon";

    /// Auf das bestehende Bild: 0 ersetzen, 1 additiv (geklemmt), 2 50/50.
    int blend = 0;
    /// Parameter-Skript (Strang D): `feed`, `kill` + Audio-Satz — damit
    /// wandert das Muster im Betrieb. Leer = kein Skript.
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
/// Kanal-Bindungs-Kodierung des Shadertoy-Nodes (S4): was hängt an iChannelN?
/// -1 = nichts (schwarz) · 0..3 = Ausgang von Buffer A..D · 4 = Audio-Textur.
inline constexpr int kShadertoyInputNone = -1;
inline constexpr int kShadertoyInputAudio = 4;

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
    std::string code;      ///< mainImage-Quelltext des Image-Passes (leer = Starter)
    /// Kanal-Bindungen des Image-Passes (Kodierung s. kShadertoyInput* —
    /// SSOT; ein frühes `audioChannel`-Feld wird beim Laden migriert).
    std::array<int, 4> imageInput = {kShadertoyInputAudio, kShadertoyInputNone,
                                     kShadertoyInputNone, kShadertoyInputNone};
    int blend = 0;         ///< aufs Bild: 0 ersetzen, 1 additiv, 2 50/50
    /// Buffer A..D (S4, max 4; Index = Buchstabe). Leer = Ein-Pass-Shader.
    std::vector<ShadertoyPass> buffers;
    std::string name;      ///< Metadaten (S3-Import; Panel zeigt sie nur an)
    std::string author;
    std::string url;
    std::string license;
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
    /// GLSL: definiert `vec2 warp(vec2 uv)` (uv 0..1, Rückgabe = Quell-UV). Uniforms: uTime, uDelta, uFrame, uResolution, bass, mid, treb, vol, beat. Leer = Identität (Passthrough)
    std::string code;
    /// Gitter-Quads in X (2..256) — GPU-Vertex-Arbeit, hohe Werte kosten kaum; grob = kantiger Warp
    int gridX = 96;
    /// Gitter-Quads in Y (2..192)
    int gridY = 72;
    /// Anteil des gewarpten Bilds 0..1 (1 = ersetzen; darunter Mix mit der ungewarpten Abtastung)
    double mixAmount = 1.0;
    /// Quell-UV außerhalb 0..1: an = wiederholen (fract), aus = Randpixel klemmen
    bool wrapUv = true;
    /// Parameter-Skript (Strang D): `gridx`, `gridy`, `mixamount` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    /// Parameter-Skript (Strang D): `gridx`, `gridy`, `mixamount` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string frameCode;
    /// Parameter-Skript (Strang D): `gridx`, `gridy`, `mixamount` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Partikel-Anzahl 1..65536 — GPU-seitig, hohe Werte kosten wenig; Änderung baut den Zustand neu auf (frischer Start)
    int count = 4096;
    /// Spawn-Zentrum X 0..1 (0 = links)
    double spawnX = 0.5;
    /// Spawn-Zentrum Y 0..1 (0 = unten)
    double spawnY = 0.35;
    /// Spawn-Streuradius in UV (0 = Punktquelle)
    double spawnSpread = 0.03;
    /// Start-Tempo in UV/s (je Partikel 0.6..1.4-fach gestreut)
    double speed = 0.25;
    /// Start-Richtung in Grad (0 = rechts, 90 = oben)
    double direction = 90.0;
    /// Fächer-/Streuwinkel in Grad um die Richtung (360 = radial in alle Richtungen)
    double fan = 360.0;
    /// Schwerkraft X in UV/s²
    double gravityX = 0.0;
    /// Schwerkraft Y in UV/s² (negativ = fällt nach unten)
    double gravityY = -0.15;
    /// Luftwiderstand 1/s (0 = keiner; bremst die Geschwindigkeit exponentiell)
    double drag = 0.6;
    /// Lebensdauer in Sekunden (Basis; danach Respawn am Spawn-Punkt)
    double lifeSeconds = 2.5;
    /// Lebensdauer-Streuung 0..1 (0.35 = ±35 % je Partikel, hash-fest)
    double lifeJitter = 0.35;
    /// Sprite-Größe in Pixeln (beim Spawn)
    double sizePx = 6.0;
    /// Größen-Faktor am Lebensende (1 = konstant, <1 schrumpft)
    double sizeEndFactor = 0.4;
    /// Farbe beim Spawn 0x00RRGGBB
    uint32_t colorStart = 0xFFD080;
    /// Farbe am Lebensende 0x00RRGGBB (Verlauf übers Alter)
    uint32_t colorEnd = 0x4020C0;
    /// Additiv zeichnen (Glow-Look) statt Alpha-Mischung
    bool additive = true;
    /// GLSL-Kraftfeld (optional): definiert `vec2 kraft(vec2 pos, vec2 vel, float alter)` — Zusatz-Beschleunigung in UV/s²; Uniforms uTime, uDelta, uResolution, bass, mid, treb, vol, beat. Leer = nur Schwerkraft/Drag
    std::string forceCode;
    /// Parameter-Skript (Strang D): `spawnx`, `spawny`, `spread`, `speed`, `dir`, `fan`, `gravx`, `gravy`, `drag`, `life`, `size` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    /// Parameter-Skript (Strang D): `spawnx`, `spawny`, `spread`, `speed`, `dir`, `fan`, `gravx`, `gravy`, `drag`, `life`, `size` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string frameCode;
    /// Parameter-Skript (Strang D): `spawnx`, `spawny`, `spread`, `speed`, `dir`, `fan`, `gravx`, `gravy`, `drag`, `life`, `size` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// GLSL: definiert `vec4 farbe(vec2 uv, vec4 src)` (uv 0..1, src = Quellpixel; Nachbarn via texture(uTex, ...); `filter` ist in GLSL reserviert). Uniforms: uTime, uDelta, uFrame, uResolution, bass, mid, treb, vol, beat. Leer = Identität (Passthrough)
    std::string code;
    /// Anteil des gefilterten Bilds 0..1 (1 = ersetzen; darunter Mix mit dem Original)
    double mixAmount = 1.0;
    /// Parameter-Skript (Strang D): `mixamount` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    /// Parameter-Skript (Strang D): `mixamount` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string frameCode;
    /// Parameter-Skript (Strang D): `mixamount` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
    /// Quelle: 0 = Datei, 1 = Kamera (live), 2 = Testaufnahme (Clip aus dem Settings-Tab „Kamera")
    int source = 0;
    /// Datei-Quelle: Videopfad (MP4/MKV/WebM/MOV/WMV/AVI — FFmpeg-Backend). Leer = nichts zeichnen
    std::string filePath;
    /// Kamera-Quelle: Geraete-ID aus der Panel-Wahl (leer = keine). Das Geraet startet erst nach der Kamera-Freigabe im Panel — nie automatisch beim Preset-Laden
    std::string cameraId;
    /// Testaufnahme: Dateiname des Clips in der benutzerlokalen Ablage (ohne Pfad)
    std::string recordingName;
    /// Echtzeit-Streaming: an = Frames uhrzeitgetrieben via QMediaPlayer (lange Videos, wenig RAM — nicht deterministisch), aus = Frame-Index deterministisch ueber die Sim-Uhr (VideoFrameCache dekodiert die Datei KOMPLETT in den Speicher — kurze Clips/Sonden)
    bool streaming = false;
    /// Abspieltempo-Faktor 0.05..20 (nur Frame-Schritt; 1 = Original-Tempo ueber die Clip-Framerate)
    double speed = 1.0;
    /// Am Ende von vorn beginnen (aus = letztes Bild halten); nur Datei/Testaufnahme
    bool loop = true;
    /// Einpassung ins Chain-Bild: 0 = strecken, 1 = einpassen (Letterbox), 2 = fuellen (beschneiden)
    int fit = 0;
    /// Mischung aufs Chain-Bild: 0 = ersetzen, 1 = additiv, 2 = 50/50
    int blend = 0;
    /// Deckkraft 0..1 (skaliert den Beitrag in allen Blend-Arten)
    double opacity = 1.0;
    /// Parameter-Skript (Strang D): `speed`, `opacity` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string initCode;
    /// Parameter-Skript (Strang D): `speed`, `opacity` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
    std::string frameCode;
    /// Parameter-Skript (Strang D): `speed`, `opacity` + `b`/`w`/`h` + Audio-Satz. Leer = kein Skript.
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
     * Eingebettete Bilder (Entscheid Patrik S43): Key = Textur-Basisname bzw.
     * Sprite-imageName, Value = Base64 der ORIGINAL-Dateibytes. Quelle bleiben
     * die Asset-Ordner; der ChainSerializer bettet beim SPEICHERN genau die
     * aktuell referenzierten Bilder ein (nicht mehr referenzierte Alt-Eintraege
     * entfallen dabei automatisch). Der Loader bevorzugt Dateien, faellt ohne
     * Fundstelle auf diese Eintraege zurueck — .lvfx bleibt damit portabel.
     * randNN-Sampler bleiben Ordner-Zufall (bewusst nicht eingebettet).
     */
    std::map<std::string, std::string> embeddedImages;
    int meshX = 32;   ///< warp mesh (app setting in the original, no preset key)
    /// Warp-Gitter senkrecht (Original-Vorgabe 24; hoeher = feinerer Warp).
    int meshY = 24;
    bool debugGrid = false;  ///< calibration overlay AFTER the composite
    /**
     * Bumped on every preset/script/shader edit (import sets 1). The render
     * host re-applies the state (script compile, shader transpile, texture
     * reload) only when it sees a new revision — frames never re-parse.
     */
    uint64_t revision = 1;

    /// Puffer-Verhalten beim .milk→.milk-Wechsel — Node-Einstellung, ueberlebt
    /// den In-Place-Preset-Tausch (wie meshX/meshY). Persistiert im Chain-Doc.
    PufferWechsel pufferWechsel = PufferWechsel::AppEinstellung;
    /// Fading: Anteil des ERBES 0..1 (0 = wie Loeschen, 1 = wie Behalten);
    /// nur im Modus Fading wirksam.
    double pufferFading = 0.5;
    /// Ausblenden: Dauer in Sekunden, ueber die das Erbe wegstirbt (Echo je
    /// Frame zusaetzlich gedaempft); nur im Modus Ausblenden wirksam.
    double pufferAusblendSek = 2.0;
    /**
     * Transient (nicht persistiert): bumpt je In-Place-Preset-Tausch; der
     * Render-Host wendet bei neuem Zaehlerstand `wechselErbe` auf den
     * FeedbackBuffer an. `wechselErbe` ist der beim Tausch AUFGELOESTE
     * Erbe-Anteil (AppEinstellung bereits eingerechnet; 1 = nichts tun);
     * `wechselAusblendSek` > 0 startet zusaetzlich die Zeit-Ausblendung.
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
                 VideoSourceParams, PixelFilterParams, PassthroughParams>;

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
 */
inline CompileResult compileChain(ChainNode& root)
{
    CompileResult result;
    if (!root.isList())
    {
        result.ok = false;
        result.warnings.push_back({"root", "root node must be an effect list"});
        return result;
    }
    uint64_t nextId = detail::maxNodeId(root) + 1;
    detail::compileNode(root, "root", result, nextId);
    return result;
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
