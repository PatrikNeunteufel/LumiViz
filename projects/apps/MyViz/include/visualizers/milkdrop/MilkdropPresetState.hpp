/**
 ****************************************************************************************
 * @file   MilkdropPresetState.hpp
 * @brief  Translated .milk preset state for the MilkdropVisualizer (MD1 core set)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * MilkDrop-Import M3: the render-facing state of one preset — every scalar the
 * MD1 pipeline reads, pre-filled with the ORIGINAL defaults from CState::Default
 * (ref/MilkDrop3/code/vis_milk2/state.cpp:536-680, BSD) so that presets which
 * omit keys behave exactly like in MilkDrop. translate() maps a MilkParser
 * ParseResult onto this struct; code stays EEL source text (the visualizer
 * transpiles via ScriptSlotHost, Dialect::Milkdrop).
 *
 * M4: custom waves/shapes are translated too (defaults from CState::Default,
 * state.cpp:593-635). Sprites stay parse-only for now. No Qt, no GL: fully
 * unit-testable.
 *
 * M5: blur controls (b1n/b1x/../b1ed, state.cpp:1344-1350) and the warp/comp
 * shader classification (MilkShaderClassifier) — Md1Default/Md1Plus shaders
 * render exactly via baked constants, Custom falls back to the live MD1 path.
 ****************************************************************************************
 */

#pragma once

#include <MilkParser.hpp>
#include <MilkShaderClassifier.hpp>

#include <string>

namespace lumi::milkdrop {

/**
 * @brief One custom wave (defaults = CState::Default, state.cpp:593-608)
 */
struct WaveState
{
    int index = -1;
    bool enabled = false;
    int samples = 512;
    int sep = 0;
    bool spectrum = false;
    bool useDots = false;
    bool drawThick = false;
    bool additive = false;
    double scaling = 1.0;
    double smoothing = 0.5;
    double r = 1.0, g = 1.0, b = 1.0, a = 1.0;
    std::string initCode;
    std::string frameCode;
    std::string pointCode;
};

/**
 * @brief One custom shape (defaults = CState::Default, state.cpp:609-635)
 */
struct ShapeState
{
    int index = -1;
    bool enabled = false;
    int sides = 4;
    bool additive = false;
    bool thickOutline = false;
    bool textured = false;
    int instances = 1;                  ///< shapecode_N_num_inst
    double texZoom = 1.0;
    double texAng = 0.0;
    double x = 0.5, y = 0.5;
    double rad = 0.1, ang = 0.0;
    double r = 1.0, g = 0.0, b = 0.0, a = 1.0;      ///< center colour
    double r2 = 0.0, g2 = 1.0, b2 = 0.0, a2 = 0.0;  ///< edge colour
    double borderR = 1.0, borderG = 1.0, borderB = 1.0, borderA = 0.1;
    std::string initCode;
    std::string frameCode;
};

/**
 * @brief MD1 render state of one preset (defaults = CState::Default)
 */
struct PresetState
{
    // --- general / composite ----------------------------------------------------------
    double decay = 0.98;
    double gammaAdj = 2.0;
    double videoEchoZoom = 2.0;
    double videoEchoAlpha = 0.0;
    int videoEchoOrientation = 0;   ///< 0..3: none / H-flip / V-flip / both
    double shader = 0.0;            ///< fShader colour-wash amount (MD1)
    bool texWrap = true;
    bool darkenCenter = false;
    bool brighten = false;
    bool darken = false;
    bool solarize = false;
    bool invert = false;

    // --- basic waveform ----------------------------------------------------------------
    int waveMode = 0;               ///< 0..7 (renderer applies % 8)
    bool additiveWaves = false;
    bool waveDots = false;
    bool waveThick = false;
    bool modWaveAlphaByVolume = false;
    bool maximizeWaveColor = true;
    double waveAlpha = 0.8;
    double waveScale = 1.0;
    double waveSmoothing = 0.75;
    double waveMystery = 0.0;       ///< fWaveParam / wave_mystery
    double modWaveAlphaStart = 0.75;
    double modWaveAlphaEnd = 0.95;
    double waveR = 1.0;
    double waveG = 1.0;
    double waveB = 1.0;
    double waveX = 0.5;
    double waveY = 0.5;

    // --- motion (warp mesh inputs) ------------------------------------------------------
    double warpAnimSpeed = 1.0;
    double warpScale = 1.0;
    double zoomExponent = 1.0;
    double zoom = 1.0;
    double rot = 0.0;
    double cx = 0.5;
    double cy = 0.5;
    double dx = 0.0;
    double dy = 0.0;
    double warp = 1.0;
    double sx = 1.0;
    double sy = 1.0;

    // --- borders -------------------------------------------------------------------------
    double obSize = 0.01;
    double obR = 0.0, obG = 0.0, obB = 0.0, obA = 0.0;
    double ibSize = 0.01;
    double ibR = 0.25, ibG = 0.25, ibB = 0.25, ibA = 0.0;

    // --- motion vectors (parsed now, rendered in M4) --------------------------------------
    double mvX = 12.0;
    double mvY = 9.0;
    double mvDX = 0.0, mvDY = 0.0;
    double mvL = 0.9;
    double mvR = 1.0, mvG = 1.0, mvB = 1.0, mvA = 1.0;

    // --- blur pyramid controls (M5; defaults = CState::Default, state.cpp:552-558) --------
    double blur1Min = 0.0;
    double blur2Min = 0.0;
    double blur3Min = 0.0;
    double blur1Max = 1.0;
    double blur2Max = 1.0;
    double blur3Max = 1.0;
    double blur1EdgeDarken = 0.25;

    // --- shader classification (M5, stage B) ----------------------------------------------
    lumi::milk::ShaderInfo warpInfo;    ///< None = MD1 preset (live path is exact)
    lumi::milk::ShaderInfo compInfo;
    std::string warpShaderText;         ///< raw HLSL (persistence/panel; infos derive from it)
    std::string compShaderText;

    // --- code (EEL source, Dialect::Milkdrop) ---------------------------------------------
    std::string perFrameInit;
    std::string perFrame;
    std::string perPixel;

    // --- custom waves/shapes (M4; only entries present in the file) ------------------------
    std::vector<WaveState> waves;
    std::vector<ShapeState> shapes;

    // --- meta ------------------------------------------------------------------------------
    int generation = 1;             ///< 1/2/3 (MilkParser)
    int psVersion = -1;             ///< -1 = shaderless (MD1 path is exact, not a fallback)
    std::string name;               ///< display name (file stem, set by the loader)
};

/**
 * @brief Map a parsed .milk onto the render state (missing keys keep defaults)
 */
[[nodiscard]] inline PresetState translate(const lumi::milk::ParseResult& parsed)
{
    PresetState s;

    const auto num = [&](const char* key, double defaultValue) {
        return parsed.value(key, defaultValue);
    };
    const auto boolean = [&](const char* key, bool defaultValue) {
        return parsed.valueInt(key, defaultValue ? 1 : 0) != 0;
    };

    s.decay = num("fDecay", s.decay);
    s.gammaAdj = num("fGammaAdj", s.gammaAdj);
    s.videoEchoZoom = num("fVideoEchoZoom", s.videoEchoZoom);
    s.videoEchoAlpha = num("fVideoEchoAlpha", s.videoEchoAlpha);
    s.videoEchoOrientation = parsed.valueInt("nVideoEchoOrientation", s.videoEchoOrientation);
    s.shader = num("fShader", s.shader);
    s.texWrap = boolean("bTexWrap", s.texWrap);
    s.darkenCenter = boolean("bDarkenCenter", s.darkenCenter);
    s.brighten = boolean("bBrighten", s.brighten);
    s.darken = boolean("bDarken", s.darken);
    s.solarize = boolean("bSolarize", s.solarize);
    s.invert = boolean("bInvert", s.invert);

    s.waveMode = parsed.valueInt("nWaveMode", s.waveMode);
    s.additiveWaves = boolean("bAdditiveWaves", s.additiveWaves);
    s.waveDots = boolean("bWaveDots", s.waveDots);
    s.waveThick = boolean("bWaveThick", s.waveThick);
    s.modWaveAlphaByVolume = boolean("bModWaveAlphaByVolume", s.modWaveAlphaByVolume);
    s.maximizeWaveColor = boolean("bMaximizeWaveColor", s.maximizeWaveColor);
    s.waveAlpha = num("fWaveAlpha", s.waveAlpha);
    s.waveScale = num("fWaveScale", s.waveScale);
    s.waveSmoothing = num("fWaveSmoothing", s.waveSmoothing);
    s.waveMystery = num("fWaveParam", s.waveMystery);
    s.modWaveAlphaStart = num("fModWaveAlphaStart", s.modWaveAlphaStart);
    s.modWaveAlphaEnd = num("fModWaveAlphaEnd", s.modWaveAlphaEnd);
    s.waveR = num("wave_r", s.waveR);
    s.waveG = num("wave_g", s.waveG);
    s.waveB = num("wave_b", s.waveB);
    s.waveX = num("wave_x", s.waveX);
    s.waveY = num("wave_y", s.waveY);

    s.warpAnimSpeed = num("fWarpAnimSpeed", s.warpAnimSpeed);
    s.warpScale = num("fWarpScale", s.warpScale);
    s.zoomExponent = num("fZoomExponent", s.zoomExponent);
    s.zoom = num("zoom", s.zoom);
    s.rot = num("rot", s.rot);
    s.cx = num("cx", s.cx);
    s.cy = num("cy", s.cy);
    s.dx = num("dx", s.dx);
    s.dy = num("dy", s.dy);
    s.warp = num("warp", s.warp);
    s.sx = num("sx", s.sx);
    s.sy = num("sy", s.sy);

    s.obSize = num("ob_size", s.obSize);
    s.obR = num("ob_r", s.obR);
    s.obG = num("ob_g", s.obG);
    s.obB = num("ob_b", s.obB);
    s.obA = num("ob_a", s.obA);
    s.ibSize = num("ib_size", s.ibSize);
    s.ibR = num("ib_r", s.ibR);
    s.ibG = num("ib_g", s.ibG);
    s.ibB = num("ib_b", s.ibB);
    s.ibA = num("ib_a", s.ibA);

    s.mvX = num("nMotionVectorsX", s.mvX);
    s.mvY = num("nMotionVectorsY", s.mvY);
    s.mvDX = num("mv_dx", s.mvDX);
    s.mvDY = num("mv_dy", s.mvDY);
    s.mvL = num("mv_l", s.mvL);
    s.mvR = num("mv_r", s.mvR);
    s.mvG = num("mv_g", s.mvG);
    s.mvB = num("mv_b", s.mvB);
    s.mvA = num("mv_a", s.mvA);

    s.blur1Min = num("b1n", s.blur1Min);
    s.blur2Min = num("b2n", s.blur2Min);
    s.blur3Min = num("b3n", s.blur3Min);
    s.blur1Max = num("b1x", s.blur1Max);
    s.blur2Max = num("b2x", s.blur2Max);
    s.blur3Max = num("b3x", s.blur3Max);
    s.blur1EdgeDarken = num("b1ed", s.blur1EdgeDarken);

    s.warpShaderText = parsed.warpShader;
    s.compShaderText = parsed.compShader;
    s.warpInfo = lumi::milk::analyzeWarpShader(s.warpShaderText);
    s.compInfo = lumi::milk::analyzeCompShader(s.compShaderText);

    s.perFrameInit = parsed.perFrameInitCode;
    s.perFrame = parsed.perFrameCode;
    s.perPixel = parsed.perPixelCode;

    for (const lumi::milk::CustomWave& w : parsed.waves)
    {
        WaveState ws;
        ws.index = w.index;
        ws.enabled = w.param("enabled", 0.0) != 0.0;
        ws.samples = static_cast<int>(w.param("samples", ws.samples));
        ws.sep = static_cast<int>(w.param("sep", ws.sep));
        ws.spectrum = w.param("bSpectrum", 0.0) != 0.0;
        ws.useDots = w.param("bUseDots", 0.0) != 0.0;
        ws.drawThick = w.param("bDrawThick", 0.0) != 0.0;
        ws.additive = w.param("bAdditive", 0.0) != 0.0;
        ws.scaling = w.param("scaling", ws.scaling);
        ws.smoothing = w.param("smoothing", ws.smoothing);
        ws.r = w.param("r", ws.r);
        ws.g = w.param("g", ws.g);
        ws.b = w.param("b", ws.b);
        ws.a = w.param("a", ws.a);
        ws.initCode = w.initCode;
        ws.frameCode = w.frameCode;
        ws.pointCode = w.pointCode;
        s.waves.push_back(std::move(ws));
    }
    for (const lumi::milk::CustomShape& sh : parsed.shapes)
    {
        ShapeState ss;
        ss.index = sh.index;
        ss.enabled = sh.param("enabled", 0.0) != 0.0;
        ss.sides = static_cast<int>(sh.param("sides", ss.sides));
        ss.additive = sh.param("additive", 0.0) != 0.0;
        ss.thickOutline = sh.param("thickOutline", 0.0) != 0.0;
        ss.textured = sh.param("textured", 0.0) != 0.0;
        ss.instances = static_cast<int>(sh.param("num_inst", ss.instances));
        ss.texZoom = sh.param("tex_zoom", ss.texZoom);
        ss.texAng = sh.param("tex_ang", ss.texAng);
        ss.x = sh.param("x", ss.x);
        ss.y = sh.param("y", ss.y);
        ss.rad = sh.param("rad", ss.rad);
        ss.ang = sh.param("ang", ss.ang);
        ss.r = sh.param("r", ss.r);
        ss.g = sh.param("g", ss.g);
        ss.b = sh.param("b", ss.b);
        ss.a = sh.param("a", ss.a);
        ss.r2 = sh.param("r2", ss.r2);
        ss.g2 = sh.param("g2", ss.g2);
        ss.b2 = sh.param("b2", ss.b2);
        ss.a2 = sh.param("a2", ss.a2);
        ss.borderR = sh.param("border_r", ss.borderR);
        ss.borderG = sh.param("border_g", ss.borderG);
        ss.borderB = sh.param("border_b", ss.borderB);
        ss.borderA = sh.param("border_a", ss.borderA);
        ss.initCode = sh.initCode;
        ss.frameCode = sh.frameCode;
        s.shapes.push_back(std::move(ss));
    }

    s.generation = parsed.generation();
    s.psVersion = parsed.psVersion;
    return s;
}

} // namespace lumi::milkdrop
