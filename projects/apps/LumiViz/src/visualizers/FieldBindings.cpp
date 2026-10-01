// ERZEUGT von asset/calibration/fields/harvest_field_docs.py — nicht
// von Hand aendern. Quelle: Inventar (Felder), Panel (Art, Bereich),
// KLASSEN/ALIASSE im Ernter. Block F, Stufe F1
// (docs/visuals/Feldsteuerung_Konzept.md).

#include "visualizers/multieffect/FieldBindings.hpp"

#include <limits>
#include <utility>
#include <variant>

namespace lumi::multieffect::feldtabelle
{
namespace
{
constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

// Zugriff ueber std::get<T>(p).member: ein umbenanntes Feld bricht den Build.
#define LV_FELD(T, member, json, name, alias, skala, art, klasse, lo, hi)      \
    Feld{json, name, alias, skala, Art::art, Klasse::klasse, lo, hi,           \
         [](const EffectParams& p) { return lese(std::get<T>(p).member); },   \
         [](EffectParams& p, double v) { setze(std::get<T>(p).member, v); }}

template <typename T>
std::size_t ix()
{
    return EffectParams(std::in_place_type<T>).index();
}

std::vector<std::pair<std::size_t, Typ>> baue()
{
    std::vector<std::pair<std::size_t, Typ>> out;
    out.push_back({ix<AddBordersParams>(), Typ{"addBorders", Traeger::ParamSkript, {
        LV_FELD(AddBordersParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(AddBordersParams, size, "size", "size", "", 1.0, Ganzzahl, Live, 0.0, 200.0),
    }}});
    out.push_back({ix<AviParams>(), Typ{"avi", Traeger::Keiner, {
        LV_FELD(AviParams, adapt, "adapt", "adapt", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(AviParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(AviParams, persist, "persist", "persist", "", 1.0, Ganzzahl, Live, 0.0, 32.0),
        LV_FELD(AviParams, speedMs, "speedMs", "speedms", "", 1.0, Ganzzahl, Live, 0.0, 1000.0),
    }}});
    out.push_back({ix<BassSpinParams>(), Typ{"bassSpin", Traeger::ParamSkript, {
        LV_FELD(BassSpinParams, colorLeft, "colorLeft", "colorleft", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(BassSpinParams, colorRight, "colorRight", "colorright", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(BassSpinParams, left, "left", "left", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BassSpinParams, mode, "mode", "mode", "", 1.0, Aufzaehlung, Live, 0.0, 1.0),
        LV_FELD(BassSpinParams, right, "right", "right", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BassSpinParams, smoothing, "smoothing", "smoothing", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(BassSpinParams, spinStep, "spinStep", "spinstep", "", 1.0, Kommazahl, Live, 0.0, 3.14),
    }}});
    out.push_back({ix<BlitterFeedbackParams>(), Typ{"blitterFeedback", Traeger::ParamSkript, {
        LV_FELD(BlitterFeedbackParams, blend, "blend", "blend", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BlitterFeedbackParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BlitterFeedbackParams, scale, "scale", "scale", "", 1.0, Ganzzahl, Live, 0.0, 256.0),
        LV_FELD(BlitterFeedbackParams, scale2, "scale2", "scale2", "", 1.0, Ganzzahl, Live, 0.0, 256.0),
        LV_FELD(BlitterFeedbackParams, subpixel, "subpixel", "subpixel", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<BloomParams>(), Typ{"bloom", Traeger::ParamSkript, {
        LV_FELD(BloomParams, downsample, "downsample", "downsample", "", 1.0, Ganzzahl, Struktur, 0.0, 4.0),
        LV_FELD(BloomParams, intensity, "intensity", "intensity", "", 1.0, Kommazahl, Live, 0.0, 8.0),
        LV_FELD(BloomParams, post, "post", "post", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BloomParams, radius, "radius", "radius", "", 1.0, Ganzzahl, Live, 1.0, 32.0),
        LV_FELD(BloomParams, threshold, "threshold", "threshold", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(BloomParams, vignette, "vignette", "vignette", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BloomParams, vignetteStrength, "vignetteStrength", "vignettestrength", "", 1.0, Kommazahl, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<BlurParams>(), Typ{"blur", Traeger::ParamSkript, {
        LV_FELD(BlurParams, roundUp, "roundUp", "roundup", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BlurParams, strength, "strength", "strength", "", 1.0, Aufzaehlung, Live, 1.0, 3.0),
    }}});
    out.push_back({ix<BrightnessParams>(), Typ{"brightness", Traeger::ParamSkript, {
        LV_FELD(BrightnessParams, blue, "blue", "blue", "", 1.0, Ganzzahl, Live, -4096.0, 4096.0),
        LV_FELD(BrightnessParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(BrightnessParams, distance, "distance", "distance", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(BrightnessParams, exclude, "exclude", "exclude", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BrightnessParams, green, "green", "green", "", 1.0, Ganzzahl, Live, -4096.0, 4096.0),
        LV_FELD(BrightnessParams, red, "red", "red", "", 1.0, Ganzzahl, Live, -4096.0, 4096.0),
    }}});
    out.push_back({ix<BufferBlendParams>(), Typ{"bufferBlend", Traeger::ParamSkript, {
        LV_FELD(BufferBlendParams, bufferA, "bufferA", "buffera", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(BufferBlendParams, bufferB, "bufferB", "bufferb", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(BufferBlendParams, mode, "mode", "mode", "", 1.0, Aufzaehlung, Live, 0.0, 10.0),
    }}});
    out.push_back({ix<BufferSaveParams>(), Typ{"bufferSave", Traeger::ParamSkript, {
        LV_FELD(BufferSaveParams, adjustAlpha, "adjustAlpha", "adjustalpha", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(BufferSaveParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(BufferSaveParams, dir, "dir", "dir", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(BufferSaveParams, slot, "slot", "slot", "", 1.0, Ganzzahl, Live, 0.0, 7.0),
    }}});
    out.push_back({ix<BumpParams>(), Typ{"bump", Traeger::EigenerHost, {
        LV_FELD(BumpParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(BumpParams, buffern, "buffern", "buffern", "", 1.0, Ganzzahl, Live, 0.0, 8.0),
        LV_FELD(BumpParams, depth, "depth", "depth", "", 1.0, Ganzzahl, Live, 0.0, 100.0),
        LV_FELD(BumpParams, depth2, "depth2", "depth2", "", 1.0, Ganzzahl, Live, 0.0, 100.0),
        LV_FELD(BumpParams, durationFrames, "durationFrames", "durationframes", "", 1.0, Ganzzahl, Live, 1.0, 200.0),
        LV_FELD(BumpParams, invert, "invert", "invert", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BumpParams, oldStyle, "oldStyle", "oldstyle", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(BumpParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<Camera3DParams>(), Typ{"camera3d", Traeger::EigenerHost, {
        LV_FELD(Camera3DParams, fogColor, "fogColor", "fogcolor", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(Camera3DParams, fogEnd, "fogEnd", "fogend", "", 1.0, Kommazahl, Live, 0.0, 1000.0),
        LV_FELD(Camera3DParams, fogStart, "fogStart", "fogstart", "", 1.0, Kommazahl, Live, 0.0, 1000.0),
        LV_FELD(Camera3DParams, fov, "fov", "fov", "", 1.0, Kommazahl, Live, 1.0, 179.0),
        LV_FELD(Camera3DParams, px, "px", "px", "", 1.0, Kommazahl, Live, -1000.0, 1000.0),
        LV_FELD(Camera3DParams, py, "py", "py", "", 1.0, Kommazahl, Live, -1000.0, 1000.0),
        LV_FELD(Camera3DParams, pz, "pz", "pz", "", 1.0, Kommazahl, Live, -1000.0, 1000.0),
        LV_FELD(Camera3DParams, roll, "roll", "roll", "", 1.0, Kommazahl, Live, -360.0, 360.0),
        LV_FELD(Camera3DParams, tx, "tx", "tx", "", 1.0, Kommazahl, Live, -1000.0, 1000.0),
        LV_FELD(Camera3DParams, ty, "ty", "ty", "", 1.0, Kommazahl, Live, -1000.0, 1000.0),
        LV_FELD(Camera3DParams, tz, "tz", "tz", "", 1.0, Kommazahl, Live, -1000.0, 1000.0),
    }}});
    out.push_back({ix<ChannelShiftParams>(), Typ{"channelShift", Traeger::ParamSkript, {
        LV_FELD(ChannelShiftParams, mode, "mode", "mode", "", 1.0, Aufzaehlung, Zustand, 0.0, 5.0),
        LV_FELD(ChannelShiftParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<ClearParams>(), Typ{"clear", Traeger::ParamSkript, {
        LV_FELD(ClearParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(ClearParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(ClearParams, onlyFirst, "onlyFirst", "onlyfirst", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<ColorClipParams>(), Typ{"colorClip", Traeger::ParamSkript, {
        LV_FELD(ColorClipParams, clipColor, "clipColor", "clipcolor", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(ColorClipParams, distance, "distance", "distance", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(ColorClipParams, mode, "mode", "mode", "", 1.0, Aufzaehlung, Live, 1.0, 3.0),
        LV_FELD(ColorClipParams, outColor, "outColor", "outcolor", "", 1.0, Farbe, Live, kNaN, kNaN),
    }}});
    out.push_back({ix<ColorMapParams>(), Typ{"colorMap", Traeger::ParamSkript, {
        LV_FELD(ColorMapParams, adjustBlend, "adjustBlend", "adjustblend", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(ColorMapParams, blendMode, "blendMode", "blendmode", "", 1.0, Aufzaehlung, Live, 0.0, 9.0),
        LV_FELD(ColorMapParams, key, "key", "key", "", 1.0, Aufzaehlung, Live, 0.0, 5.0),
    }}});
    out.push_back({ix<ColorModifierParams>(), Typ{"colorModifier", Traeger::Keiner, {
        LV_FELD(ColorModifierParams, recompute, "recompute", "recompute", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<ColorReductionParams>(), Typ{"colorReduction", Traeger::ParamSkript, {
        LV_FELD(ColorReductionParams, levels, "levels", "levels", "", 1.0, Ganzzahl, Live, 1.0, 8.0),
    }}});
    out.push_back({ix<ColorfadeParams>(), Typ{"colorfade", Traeger::ParamSkript, {
        LV_FELD(ColorfadeParams, beatFaderB, "beatFaderB", "beatfaderb", "", 1.0, Ganzzahl, Live, -32.0, 32.0),
        LV_FELD(ColorfadeParams, beatFaderG, "beatFaderG", "beatfaderg", "", 1.0, Ganzzahl, Live, -32.0, 32.0),
        LV_FELD(ColorfadeParams, beatFaderR, "beatFaderR", "beatfaderr", "", 1.0, Ganzzahl, Live, -32.0, 32.0),
        LV_FELD(ColorfadeParams, faderB, "faderB", "faderb", "", 1.0, Ganzzahl, Live, -32.0, 32.0),
        LV_FELD(ColorfadeParams, faderG, "faderG", "faderg", "", 1.0, Ganzzahl, Live, -32.0, 32.0),
        LV_FELD(ColorfadeParams, faderR, "faderR", "faderr", "", 1.0, Ganzzahl, Live, -32.0, 32.0),
        LV_FELD(ColorfadeParams, onBeatFrames, "onBeatFrames", "onbeatframes", "", 1.0, Ganzzahl, Live, 1.0, 200.0),
        LV_FELD(ColorfadeParams, onBeatRandom, "onBeatRandom", "onbeatrandom", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(ColorfadeParams, slowFade, "slowFade", "slowfade", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<ConvolutionParams>(), Typ{"convolution", Traeger::ParamSkript, {
        LV_FELD(ConvolutionParams, absolute, "absolute", "absolute", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(ConvolutionParams, bias, "bias", "bias", "", 1.0, Ganzzahl, Live, -255.0, 255.0),
        LV_FELD(ConvolutionParams, edgeMode, "edgeMode", "edgemode", "", 1.0, Aufzaehlung, Live, 0.0, 1.0),
        LV_FELD(ConvolutionParams, scale, "scale", "scale", "", 1.0, Ganzzahl, Live, 1.0, 1000.0),
        LV_FELD(ConvolutionParams, twoPass, "twoPass", "twopass", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<CustomBpmParams>(), Typ{"customBpm", Traeger::ParamSkript, {
        LV_FELD(CustomBpmParams, arbitrary, "arbitrary", "arbitrary", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(CustomBpmParams, arbitraryMs, "arbitraryMs", "arbitraryms", "", 1.0, Ganzzahl, Live, 1.0, 5000.0),
        LV_FELD(CustomBpmParams, invert, "invert", "invert", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(CustomBpmParams, skip, "skip", "skip", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(CustomBpmParams, skipCount, "skipCount", "skipcount", "", 1.0, Ganzzahl, Live, 1.0, 16.0),
        LV_FELD(CustomBpmParams, skipFirst, "skipFirst", "skipfirst", "", 1.0, Ganzzahl, Live, 0.0, 64.0),
    }}});
    out.push_back({ix<DebugBarsParams>(), Typ{"debugBars", Traeger::Keiner, {
        LV_FELD(DebugBarsParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(DebugBarsParams, orbitSpeed, "orbitSpeed", "orbitspeed", "", 1.0, Kommazahl, Live, -10.0, 10.0),
    }}});
    out.push_back({ix<DomainWarpParams>(), Typ{"domainWarp", Traeger::EigenerHost, {
        LV_FELD(DomainWarpParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(DomainWarpParams, colorCycle, "colorCycle", "colorcycle", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(DomainWarpParams, colorScale, "colorScale", "colorscale", "", 1.0, Kommazahl, Live, 0.01, 8.0),
        LV_FELD(DomainWarpParams, gain, "gain", "gain", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(DomainWarpParams, lacunarity, "lacunarity", "lacunarity", "", 1.0, Kommazahl, Live, 1.0, 4.0),
        LV_FELD(DomainWarpParams, octaves, "octaves", "octaves", "", 1.0, Ganzzahl, Live, 1.0, 10.0),
        LV_FELD(DomainWarpParams, offsetX, "offsetX", "offsetx", "ox", 1.0, Kommazahl, Live, -32.0, 32.0),
        LV_FELD(DomainWarpParams, offsetY, "offsetY", "offsety", "oy", 1.0, Kommazahl, Live, -32.0, 32.0),
        LV_FELD(DomainWarpParams, scale, "scale", "scale", "", 1.0, Kommazahl, Live, 0.1, 32.0),
        LV_FELD(DomainWarpParams, speed, "speed", "speed", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(DomainWarpParams, warp, "warp", "warp", "", 1.0, Kommazahl, Live, 0.0, 8.0),
        LV_FELD(DomainWarpParams, warpScale, "warpScale", "warpscale", "", 1.0, Kommazahl, Live, 0.1, 8.0),
    }}});
    out.push_back({ix<DotFountainParams>(), Typ{"dotFountain", Traeger::ParamSkript, {
        LV_FELD(DotFountainParams, angle, "angle", "angle", "", 1.0, Ganzzahl, Live, -90.0, 90.0),
        LV_FELD(DotFountainParams, rotVel, "rotVel", "rotvel", "", 1.0, Ganzzahl, Live, -50.0, 50.0),
        LV_FELD(DotFountainParams, startRotation, "startRotation", "startrotation", "", 1.0, Kommazahl, Zustand, 0.0, 360.0),
    }}});
    out.push_back({ix<DotGridParams>(), Typ{"dotGrid", Traeger::ParamSkript, {
        LV_FELD(DotGridParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 3.0),
        LV_FELD(DotGridParams, spacing, "spacing", "spacing", "", 1.0, Ganzzahl, Live, 2.0, 128.0),
        LV_FELD(DotGridParams, xMove, "xMove", "xmove", "", 1.0, Ganzzahl, Live, -1024.0, 1024.0),
        LV_FELD(DotGridParams, yMove, "yMove", "ymove", "", 1.0, Ganzzahl, Live, -1024.0, 1024.0),
    }}});
    out.push_back({ix<DotPlaneParams>(), Typ{"dotPlane", Traeger::ParamSkript, {
        LV_FELD(DotPlaneParams, angle, "angle", "angle", "", 1.0, Ganzzahl, Live, -90.0, 90.0),
        LV_FELD(DotPlaneParams, camDistance, "camDistance", "camdistance", "", 1.0, Kommazahl, Live, 50.0, 2000.0),
        LV_FELD(DotPlaneParams, rotVel, "rotVel", "rotvel", "", 1.0, Ganzzahl, Live, -50.0, 50.0),
        LV_FELD(DotPlaneParams, settle, "settle", "settle", "", 1.0, Kommazahl, Live, 0.0, 2.0),
        LV_FELD(DotPlaneParams, startRotation, "startRotation", "startrotation", "", 1.0, Kommazahl, Zustand, 0.0, 360.0),
    }}});
    out.push_back({ix<DynamicDistanceModifierParams>(), Typ{"dynamicDistanceModifier", Traeger::EigenerHost, {
        LV_FELD(DynamicDistanceModifierParams, blend, "blend", "blend", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(DynamicDistanceModifierParams, subpixel, "subpixel", "subpixel", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<DynamicMovementParams>(), Typ{"dynamicMovement", Traeger::Keiner, {
        LV_FELD(DynamicMovementParams, blend, "blend", "blend", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(DynamicMovementParams, buffern, "buffern", "buffern", "", 1.0, Ganzzahl, Live, 0.0, 8.0),
        LV_FELD(DynamicMovementParams, nomove, "nomove", "nomove", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(DynamicMovementParams, rectCoords, "rectCoords", "rectcoords", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(DynamicMovementParams, subpixel, "subpixel", "subpixel", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(DynamicMovementParams, wrap, "wrap", "wrap", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(DynamicMovementParams, xres, "xres", "xres", "", 1.0, Ganzzahl, Struktur, 2.0, 256.0),
        LV_FELD(DynamicMovementParams, yres, "yres", "yres", "", 1.0, Ganzzahl, Struktur, 2.0, 256.0),
    }}});
    out.push_back({ix<DynamicShiftParams>(), Typ{"dynamicShift", Traeger::EigenerHost, {
        LV_FELD(DynamicShiftParams, blend, "blend", "blend", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(DynamicShiftParams, subpixel, "subpixel", "subpixel", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<FadeoutParams>(), Typ{"fadeout", Traeger::ParamSkript, {
        LV_FELD(FadeoutParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(FadeoutParams, fadeLen, "fadeLen", "fadelen", "", 1.0, Ganzzahl, Live, 0.0, 92.0),
    }}});
    out.push_back({ix<FastBrightnessParams>(), Typ{"fastBrightness", Traeger::ParamSkript, {
        LV_FELD(FastBrightnessParams, dir, "dir", "dir", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
    }}});
    out.push_back({ix<FlameParams>(), Typ{"flame", Traeger::EigenerHost, {
        LV_FELD(FlameParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(FlameParams, dotSize, "dotSize", "dotsize", "", 1.0, Kommazahl, Live, 1.0, 16.0),
        LV_FELD(FlameParams, functions, "functions", "functions", "", 1.0, Ganzzahl, Live, 2.0, 4.0),
        LV_FELD(FlameParams, points, "points", "points", "", 1.0, Ganzzahl, Live, 1.0, 200000.0),
        LV_FELD(FlameParams, rotation, "rotation", "rotation", "", 1.0, Kommazahl, Live, -6.2832, 6.2832),
        LV_FELD(FlameParams, rotationSpeed, "rotationSpeed", "rotationspeed", "", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(FlameParams, scale, "scale", "scale", "", 1.0, Kommazahl, Live, 0.05, 4.0),
        LV_FELD(FlameParams, variation, "variation", "variation", "", 1.0, Aufzaehlung, Live, 0.0, 4.0),
    }}});
    out.push_back({ix<Fractal2DParams>(), Typ{"fractal2D", Traeger::EigenerHost, {
        LV_FELD(Fractal2DParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(Fractal2DParams, centerX, "centerX", "centerx", "cx", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(Fractal2DParams, centerY, "centerY", "centery", "cy", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(Fractal2DParams, colorCycle, "colorCycle", "colorcycle", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(Fractal2DParams, colorScale, "colorScale", "colorscale", "", 1.0, Kommazahl, Live, 0.001, 4.0),
        LV_FELD(Fractal2DParams, escapeR, "escapeR", "escaper", "", 1.0, Kommazahl, Live, 1.0, 1024.0),
        LV_FELD(Fractal2DParams, type, "ftype", "ftype", "", 1.0, Aufzaehlung, Live, 0.0, 8.0),
        LV_FELD(Fractal2DParams, insideColor, "insideColor", "insidecolor", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(Fractal2DParams, juliaX, "juliaX", "juliax", "jx", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(Fractal2DParams, juliaY, "juliaY", "juliay", "jy", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(Fractal2DParams, maxIter, "maxIter", "maxiter", "", 1.0, Ganzzahl, Live, 1.0, 2048.0),
        LV_FELD(Fractal2DParams, power, "power", "power", "", 1.0, Kommazahl, Live, 1.0, 16.0),
        LV_FELD(Fractal2DParams, rotation, "rotation", "rotation", "rot", 1.0, Kommazahl, Live, -6.2832, 6.2832),
        LV_FELD(Fractal2DParams, smooth, "smooth", "smooth", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(Fractal2DParams, zoom, "zoom", "zoom", "", 1.0, Kommazahl, Live, 0.001, 100000.0),
    }}});
    out.push_back({ix<Fractal3DParams>(), Typ{"fractal3D", Traeger::EigenerHost, {
        LV_FELD(Fractal3DParams, ambient, "ambient", "ambient", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(Fractal3DParams, ao, "ao", "ao", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(Fractal3DParams, background, "background", "background", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(Fractal3DParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(Fractal3DParams, colorCycle, "colorCycle", "colorcycle", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(Fractal3DParams, colorScale, "colorScale", "colorscale", "", 1.0, Kommazahl, Live, 0.01, 8.0),
        LV_FELD(Fractal3DParams, dist, "dist", "dist", "", 1.0, Kommazahl, Live, 0.5, 12.0),
        LV_FELD(Fractal3DParams, fold, "fold", "fold", "", 1.0, Kommazahl, Live, 0.1, 4.0),
        LV_FELD(Fractal3DParams, fov, "fov", "fov", "", 1.0, Kommazahl, Live, 0.3, 3.0),
        LV_FELD(Fractal3DParams, type, "ftype", "ftype", "", 1.0, Aufzaehlung, Live, 0.0, 4.0),
        LV_FELD(Fractal3DParams, juliaW, "juliaW", "juliaw", "", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(Fractal3DParams, juliaX, "juliaX", "juliax", "", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(Fractal3DParams, juliaY, "juliaY", "juliay", "", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(Fractal3DParams, juliaZ, "juliaZ", "juliaz", "", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(Fractal3DParams, lightPitch, "lightPitch", "lightpitch", "", 1.0, Kommazahl, Live, -1.5, 1.5),
        LV_FELD(Fractal3DParams, lightYaw, "lightYaw", "lightyaw", "", 1.0, Kommazahl, Live, -6.2832, 6.2832),
        LV_FELD(Fractal3DParams, maxIter, "maxIter", "maxiter", "", 1.0, Ganzzahl, Live, 1.0, 64.0),
        LV_FELD(Fractal3DParams, maxSteps, "maxSteps", "maxsteps", "", 1.0, Ganzzahl, Live, 8.0, 512.0),
        LV_FELD(Fractal3DParams, pitch, "pitch", "pitch", "", 1.0, Kommazahl, Live, -1.5, 1.5),
        LV_FELD(Fractal3DParams, power, "power", "power", "", 1.0, Kommazahl, Live, 1.0, 16.0),
        LV_FELD(Fractal3DParams, scale, "scale", "scale", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(Fractal3DParams, yaw, "yaw", "yaw", "", 1.0, Kommazahl, Live, -6.2832, 6.2832),
    }}});
    out.push_back({ix<FractalZoomerParams>(), Typ{"fractalZoomer", Traeger::EigenerHost, {
        LV_FELD(FractalZoomerParams, centerX, "centerX", "centerx", "cx", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(FractalZoomerParams, centerY, "centerY", "centery", "cy", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(FractalZoomerParams, colorCycle, "colorCycle", "colorcycle", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(FractalZoomerParams, colorScale, "colorScale", "colorscale", "", 1.0, Kommazahl, Live, 0.001, 4.0),
        LV_FELD(FractalZoomerParams, feedback, "feedback", "feedback", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(FractalZoomerParams, type, "ftype", "ftype", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(FractalZoomerParams, insideColor, "insideColor", "insidecolor", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(FractalZoomerParams, juliaX, "juliaX", "juliax", "", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(FractalZoomerParams, juliaY, "juliaY", "juliay", "", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(FractalZoomerParams, maxIter, "maxIter", "maxiter", "", 1.0, Ganzzahl, Live, 1.0, 2048.0),
        LV_FELD(FractalZoomerParams, rotationSpeed, "rotationSpeed", "rotationspeed", "rotspeed", 1.0, Kommazahl, Live, -0.2, 0.2),
        LV_FELD(FractalZoomerParams, zoomSpeed, "zoomSpeed", "zoomspeed", "", 1.0, Kommazahl, Live, 0.9, 1.2),
    }}});
    out.push_back({ix<FyrewurXParams>(), Typ{"fyrewurx", Traeger::ParamSkript, {
        LV_FELD(FyrewurXParams, burstSpread, "burstSpread", "burstspread", "", 1.0, Kommazahl, Live, 0.0, 3.0),
        LV_FELD(FyrewurXParams, dotSize, "dotSize", "dotsize", "", 1.0, Kommazahl, Live, 1.0, 32.0),
        LV_FELD(FyrewurXParams, gravity, "gravity", "gravity", "", 1.0, Kommazahl, Live, 0.0, 10.0),
        LV_FELD(FyrewurXParams, hueDrift, "hueDrift", "huedrift", "", 1.0, Kommazahl, Live, 0.0, 6.0),
        LV_FELD(FyrewurXParams, lifeSeconds, "lifeSeconds", "lifeseconds", "life", 1.0, Kommazahl, Live, 0.1, 10.0),
        LV_FELD(FyrewurXParams, sparks, "sparks", "sparks", "", 1.0, Ganzzahl, Live, 1.0, 1024.0),
        LV_FELD(FyrewurXParams, speed, "speed", "speed", "", 1.0, Kommazahl, Live, 0.05, 5.0),
    }}});
    out.push_back({ix<GlowOrbsParams>(), Typ{"glowOrbs", Traeger::EigenerHost, {
        LV_FELD(GlowOrbsParams, falloff, "falloff", "falloff", "", 1.0, Kommazahl, Live, 0.5, 32.0),
        LV_FELD(GlowOrbsParams, haloIntensity, "haloIntensity", "halointensity", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(GlowOrbsParams, haloScale, "haloScale", "haloscale", "", 1.0, Kommazahl, Live, 1.0, 10.0),
        LV_FELD(GlowOrbsParams, orbCount, "orbCount", "orbcount", "n", 1.0, Ganzzahl, Live, 1.0, 64.0),
    }}});
    out.push_back({ix<GpuParticlesParams>(), Typ{"gpuParticles", Traeger::ParamSkript, {
        LV_FELD(GpuParticlesParams, additive, "additive", "additive", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(GpuParticlesParams, colorEnd, "colorEnd", "colorend", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(GpuParticlesParams, colorStart, "colorStart", "colorstart", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(GpuParticlesParams, count, "count", "count", "", 1.0, Ganzzahl, Struktur, kNaN, kNaN),
        LV_FELD(GpuParticlesParams, direction, "direction", "direction", "dir", 1.0, Kommazahl, Live, -360.0, 360.0),
        LV_FELD(GpuParticlesParams, drag, "drag", "drag", "", 1.0, Kommazahl, Live, 0.0, 10.0),
        LV_FELD(GpuParticlesParams, fan, "fan", "fan", "", 1.0, Kommazahl, Live, 0.0, 360.0),
        LV_FELD(GpuParticlesParams, gravityX, "gravityX", "gravityx", "gravx", 1.0, Kommazahl, Live, -5.0, 5.0),
        LV_FELD(GpuParticlesParams, gravityY, "gravityY", "gravityy", "gravy", 1.0, Kommazahl, Live, -5.0, 5.0),
        LV_FELD(GpuParticlesParams, lifeJitter, "lifeJitter", "lifejitter", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(GpuParticlesParams, lifeSeconds, "lifeSeconds", "lifeseconds", "life", 1.0, Kommazahl, Live, 0.1, 30.0),
        LV_FELD(GpuParticlesParams, sizeEndFactor, "sizeEndFactor", "sizeendfactor", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(GpuParticlesParams, sizePx, "sizePx", "sizepx", "size", 1.0, Kommazahl, Live, 0.5, 128.0),
        LV_FELD(GpuParticlesParams, spawnSpread, "spawnSpread", "spawnspread", "spread", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(GpuParticlesParams, spawnX, "spawnX", "spawnx", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(GpuParticlesParams, spawnY, "spawnY", "spawny", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(GpuParticlesParams, speed, "speed", "speed", "", 1.0, Kommazahl, Live, 0.0, 5.0),
    }}});
    out.push_back({ix<GrainParams>(), Typ{"grain", Traeger::ParamSkript, {
        LV_FELD(GrainParams, amount, "amount", "amount", "", 1.0, Ganzzahl, Live, 0.0, 100.0),
        LV_FELD(GrainParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(GrainParams, staticGrain, "staticGrain", "staticgrain", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<HostSwitcherParams>(), Typ{"hostSwitcher", Traeger::Keiner, {
        LV_FELD(HostSwitcherParams, intervalSeconds, "intervalSeconds", "intervalseconds", "", 1.0, Kommazahl, Live, 0.0, 3600.0),
        LV_FELD(HostSwitcherParams, nextOnSongChange, "nextOnSongChange", "nextonsongchange", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(HostSwitcherParams, order, "order", "order", "", 1.0, Aufzaehlung, Live, 0.0, 1.0),
        LV_FELD(HostSwitcherParams, startIndex, "startIndex", "startindex", "", 1.0, Ganzzahl, Zustand, kNaN, kNaN),
    }}});
    out.push_back({ix<HostGroupParams>(), Typ{"hostgroup", Traeger::Keiner, {
        LV_FELD(HostGroupParams, blendOut, "blendOut", "blendout", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(HostGroupParams, crossfadeSeconds, "crossfadeSeconds", "crossfadeseconds", "", 1.0, Kommazahl, Live, 0.0, 60.0),
        LV_FELD(HostGroupParams, curveIn, "curveIn", "curvein", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(HostGroupParams, curveOut, "curveOut", "curveout", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(HostGroupParams, outAdjustAlpha, "outAdjustAlpha", "outadjustalpha", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
    }}});
    out.push_back({ix<InterferencesParams>(), Typ{"interferences", Traeger::ParamSkript, {
        LV_FELD(InterferencesParams, alpha, "alpha", "alpha", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(InterferencesParams, alpha2, "alpha2", "alpha2", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(InterferencesParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(InterferencesParams, distance, "distance", "distance", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(InterferencesParams, distance2, "distance2", "distance2", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(InterferencesParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(InterferencesParams, points, "points", "points", "", 1.0, Ganzzahl, Live, 1.0, 8.0),
        LV_FELD(InterferencesParams, rgb, "rgb", "rgb", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(InterferencesParams, rotation, "rotation", "rotation", "", 1.0, Ganzzahl, Zustand, 0.0, 255.0),
        LV_FELD(InterferencesParams, rotationInc, "rotationInc", "rotationinc", "", 1.0, Ganzzahl, Live, -64.0, 64.0),
        LV_FELD(InterferencesParams, rotationInc2, "rotationInc2", "rotationinc2", "", 1.0, Ganzzahl, Live, -64.0, 64.0),
        LV_FELD(InterferencesParams, speed, "speed", "speed", "", 1.0, Kommazahl, Live, 0.01, 2.0),
    }}});
    out.push_back({ix<InterleaveParams>(), Typ{"interleave", Traeger::ParamSkript, {
        LV_FELD(InterleaveParams, beatDuration, "beatDuration", "beatduration", "", 1.0, Ganzzahl, Live, 1.0, 100.0),
        LV_FELD(InterleaveParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(InterleaveParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(InterleaveParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(InterleaveParams, x, "x", "x", "", 1.0, Ganzzahl, Live, 0.0, 512.0),
        LV_FELD(InterleaveParams, x2, "x2", "x2", "", 1.0, Ganzzahl, Live, 0.0, 512.0),
        LV_FELD(InterleaveParams, y, "y", "y", "", 1.0, Ganzzahl, Live, 0.0, 512.0),
        LV_FELD(InterleaveParams, y2, "y2", "y2", "", 1.0, Ganzzahl, Live, 0.0, 512.0),
    }}});
    out.push_back({ix<IsfFilterParams>(), Typ{"isfFilter", Traeger::ParamSkript, {
        LV_FELD(IsfFilterParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(IsfFilterParams, geometrie, "geometrie", "geometrie", "", 1.0, Aufzaehlung, Struktur, 0.0, 2.0),
        LV_FELD(IsfFilterParams, gridX, "gridX", "gridx", "", 1.0, Ganzzahl, Struktur, kNaN, kNaN),
        LV_FELD(IsfFilterParams, gridY, "gridY", "gridy", "", 1.0, Ganzzahl, Struktur, kNaN, kNaN),
        LV_FELD(IsfFilterParams, mixAmount, "mixAmount", "mixamount", "", 1.0, Kommazahl, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<JherikoGlobalParams>(), Typ{"jherikoGlobal", Traeger::EigenerHost, {
        LV_FELD(JherikoGlobalParams, loadMode, "loadMode", "loadmode", "", 1.0, Aufzaehlung, Live, 0.0, 3.0),
    }}});
    out.push_back({ix<KleinianParams>(), Typ{"kleinian", Traeger::EigenerHost, {
        LV_FELD(KleinianParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(KleinianParams, colorCycle, "colorCycle", "colorcycle", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(KleinianParams, colorScale, "colorScale", "colorscale", "", 1.0, Kommazahl, Live, 0.01, 8.0),
        LV_FELD(KleinianParams, iterations, "iterations", "iterations", "", 1.0, Ganzzahl, Live, 1.0, 200.0),
        LV_FELD(KleinianParams, morph, "morph", "morph", "", 1.0, Kommazahl, Live, -6.2832, 6.2832),
        LV_FELD(KleinianParams, p, "p", "p", "", 1.0, Ganzzahl, Live, 3.0, 20.0),
        LV_FELD(KleinianParams, q, "q", "q", "", 1.0, Ganzzahl, Live, 3.0, 20.0),
        LV_FELD(KleinianParams, rotation, "rotation", "rotation", "rot", 1.0, Kommazahl, Live, -6.2832, 6.2832),
        LV_FELD(KleinianParams, zoom, "zoom", "zoom", "", 1.0, Kommazahl, Live, 0.1, 8.0),
    }}});
    out.push_back({ix<ListParams>(), Typ{"list", Traeger::EigenerHost, {
        LV_FELD(ListParams, blendIn, "blendIn", "blendin", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(ListParams, blendOut, "blendOut", "blendout", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(ListParams, bufferIn, "bufferIn", "bufferin", "", 1.0, Ganzzahl, Live, 0.0, 7.0),
        LV_FELD(ListParams, bufferInInvert, "bufferInInvert", "bufferininvert", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(ListParams, bufferOut, "bufferOut", "bufferout", "", 1.0, Ganzzahl, Live, 0.0, 7.0),
        LV_FELD(ListParams, bufferOutInvert, "bufferOutInvert", "bufferoutinvert", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(ListParams, clearEveryFrame, "clearEveryFrame", "cleareveryframe", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(ListParams, inAdjustAlpha, "inAdjustAlpha", "inadjustalpha", "alphain", 0.00392156862745098, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(ListParams, onBeatFrames, "onBeatFrames", "onbeatframes", "", 1.0, Ganzzahl, Live, 1.0, 200.0),
        LV_FELD(ListParams, onBeatRender, "onBeatRender", "onbeatrender", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(ListParams, outAdjustAlpha, "outAdjustAlpha", "outadjustalpha", "alphaout", 0.00392156862745098, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(ListParams, useCode, "useCode", "usecode", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<LyapunovParams>(), Typ{"lyapunov", Traeger::EigenerHost, {
        LV_FELD(LyapunovParams, aMax, "aMax", "amax", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(LyapunovParams, aMin, "aMin", "amin", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(LyapunovParams, bMax, "bMax", "bmax", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(LyapunovParams, bMin, "bMin", "bmin", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(LyapunovParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(LyapunovParams, colorCycle, "colorCycle", "colorcycle", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(LyapunovParams, colorScale, "colorScale", "colorscale", "", 1.0, Kommazahl, Live, 0.01, 8.0),
        LV_FELD(LyapunovParams, iterations, "iterations", "iterations", "", 1.0, Ganzzahl, Live, 1.0, 4000.0),
        LV_FELD(LyapunovParams, negColor, "negColor", "negcolor", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(LyapunovParams, warmup, "warmup", "warmup", "", 1.0, Ganzzahl, Live, 0.0, 2000.0),
    }}});
    out.push_back({ix<MeshWarpParams>(), Typ{"meshWarp", Traeger::ParamSkript, {
        LV_FELD(MeshWarpParams, gridX, "gridX", "gridx", "", 1.0, Ganzzahl, Struktur, kNaN, kNaN),
        LV_FELD(MeshWarpParams, gridY, "gridY", "gridy", "", 1.0, Ganzzahl, Struktur, kNaN, kNaN),
        LV_FELD(MeshWarpParams, mixAmount, "mixAmount", "mixamount", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(MeshWarpParams, wrapUv, "wrapUv", "wrapuv", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<Metaballs3DParams>(), Typ{"metaballs3d", Traeger::ParamSkript, {
        LV_FELD(Metaballs3DParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(Metaballs3DParams, count, "count", "count", "", 1.0, Ganzzahl, Live, 1.0, 16.0),
        LV_FELD(Metaballs3DParams, depth, "depth", "depth", "", 1.0, Kommazahl, Live, 0.2, 6.0),
        LV_FELD(Metaballs3DParams, phase, "phase", "phase", "", 1.0, Kommazahl, Live, 0.0, 6.3),
        LV_FELD(Metaballs3DParams, radius, "radius", "radius", "", 1.0, Kommazahl, Live, 0.01, 1.0),
        LV_FELD(Metaballs3DParams, speed, "speed", "speed", "", 1.0, Kommazahl, Live, 0.0, 5.0),
        LV_FELD(Metaballs3DParams, spread, "spread", "spread", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(Metaballs3DParams, threshold, "threshold", "threshold", "", 1.0, Kommazahl, Live, 0.05, 8.0),
    }}});
    out.push_back({ix<MirrorParams>(), Typ{"mirror", Traeger::ParamSkript, {
        LV_FELD(MirrorParams, mode, "mode", "mode", "", 1.0, Ganzzahl, Live, 0.0, 15.0),
        LV_FELD(MirrorParams, onBeatRandom, "onBeatRandom", "onbeatrandom", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(MirrorParams, slower, "slower", "slower", "", 1.0, Ganzzahl, Live, 1.0, 16.0),
        LV_FELD(MirrorParams, smooth, "smooth", "smooth", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<MosaicParams>(), Typ{"mosaic", Traeger::ParamSkript, {
        LV_FELD(MosaicParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(MosaicParams, durationFrames, "durationFrames", "durationframes", "", 1.0, Ganzzahl, Live, 1.0, 200.0),
        LV_FELD(MosaicParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(MosaicParams, quality, "quality", "quality", "", 1.0, Ganzzahl, Live, 1.0, 100.0),
        LV_FELD(MosaicParams, quality2, "quality2", "quality2", "", 1.0, Ganzzahl, Live, 1.0, 100.0),
    }}});
    out.push_back({ix<MovementParams>(), Typ{"movement", Traeger::Keiner, {
        LV_FELD(MovementParams, blend, "blend", "blend", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(MovementParams, builtinRemap, "builtinRemap", "builtinremap", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(MovementParams, rectCoords, "rectCoords", "rectcoords", "", 1.0, Schalter, Struktur, 0.0, 1.0),
        LV_FELD(MovementParams, sourceMapped, "sourceMapped", "sourcemapped", "", 1.0, Aufzaehlung, Zustand, 0.0, 0.0),
        LV_FELD(MovementParams, subpixel, "subpixel", "subpixel", "", 1.0, Schalter, Struktur, 0.0, 1.0),
        LV_FELD(MovementParams, wrap, "wrap", "wrap", "", 1.0, Schalter, Struktur, 0.0, 1.0),
    }}});
    out.push_back({ix<MovingParticleParams>(), Typ{"movingParticle", Traeger::ParamSkript, {
        LV_FELD(MovingParticleParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 3.0),
        LV_FELD(MovingParticleParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(MovingParticleParams, damping, "damping", "damping", "", 1.0, Kommazahl, Live, 0.8, 1.0),
        LV_FELD(MovingParticleParams, maxDistance, "maxDistance", "maxdistance", "", 1.0, Ganzzahl, Live, 1.0, 256.0),
        LV_FELD(MovingParticleParams, onBeatSize, "onBeatSize", "onbeatsize", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(MovingParticleParams, size, "size", "size", "", 1.0, Ganzzahl, Live, 1.0, 128.0),
        LV_FELD(MovingParticleParams, size2, "size2", "size2", "", 1.0, Ganzzahl, Live, 1.0, 128.0),
        LV_FELD(MovingParticleParams, spring, "spring", "spring", "", 1.0, Kommazahl, Live, 0.0, 0.1),
    }}});
    out.push_back({ix<MultiDelayParams>(), Typ{"multiDelay", Traeger::ParamSkript, {
        LV_FELD(MultiDelayParams, buffer, "buffer", "buffer", "", 1.0, Ganzzahl, Live, 0.0, 5.0),
        LV_FELD(MultiDelayParams, delay, "delay", "delay", "", 1.0, Ganzzahl, Live, 1.0, 128.0),
        LV_FELD(MultiDelayParams, mode, "mode", "mode", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(MultiDelayParams, useBeats, "useBeats", "usebeats", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<MultiFilterParams>(), Typ{"multiFilter", Traeger::ParamSkript, {
        LV_FELD(MultiFilterParams, effect, "effect", "effect", "", 1.0, Aufzaehlung, Live, 0.0, 3.0),
        LV_FELD(MultiFilterParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<MultiplierParams>(), Typ{"multiplier", Traeger::ParamSkript, {
        LV_FELD(MultiplierParams, mode, "mode", "mode", "", 1.0, Aufzaehlung, Live, 0.0, 7.0),
    }}});
    out.push_back({ix<OnBeatClearParams>(), Typ{"onBeatClear", Traeger::ParamSkript, {
        LV_FELD(OnBeatClearParams, blend, "blend", "blend", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(OnBeatClearParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(OnBeatClearParams, everyNBeats, "everyNBeats", "everynbeats", "", 1.0, Ganzzahl, Live, 1.0, 100.0),
    }}});
    out.push_back({ix<OscRingParams>(), Typ{"oscRing", Traeger::ParamSkript, {
        LV_FELD(OscRingParams, audioScale, "audioScale", "audioscale", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(OscRingParams, baseScale, "baseScale", "basescale", "", 1.0, Kommazahl, Live, 0.0, 2.0),
        LV_FELD(OscRingParams, channel, "channel", "channel", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(OscRingParams, position, "position", "position", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(OscRingParams, segments, "segments", "segments", "", 1.0, Ganzzahl, Live, 3.0, 1024.0),
        LV_FELD(OscRingParams, size, "size", "size", "", 1.0, Ganzzahl, Live, 0.0, 16.0),
        LV_FELD(OscRingParams, source, "source", "source", "", 1.0, Aufzaehlung, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<OscStarParams>(), Typ{"oscStar", Traeger::ParamSkript, {
        LV_FELD(OscStarParams, amplitude, "amplitude", "amplitude", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(OscStarParams, channel, "channel", "channel", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(OscStarParams, position, "position", "position", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(OscStarParams, rot, "rot", "rot", "", 1.0, Ganzzahl, Live, 0.0, 16.0),
        LV_FELD(OscStarParams, rotScale, "rotScale", "rotscale", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(OscStarParams, size, "size", "size", "", 1.0, Ganzzahl, Live, 0.0, 16.0),
        LV_FELD(OscStarParams, spokes, "spokes", "spokes", "", 1.0, Ganzzahl, Live, 1.0, 64.0),
    }}});
    out.push_back({ix<PictureParams>(), Typ{"picture", Traeger::ParamSkript, {
        LV_FELD(PictureParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(PictureParams, keepAspect, "keepAspect", "keepaspect", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<PictureIIParams>(), Typ{"pictureII", Traeger::ParamSkript, {
        LV_FELD(PictureIIParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
    }}});
    out.push_back({ix<PixelFilterParams>(), Typ{"pixelFilter", Traeger::ParamSkript, {
        LV_FELD(PixelFilterParams, mixAmount, "mixAmount", "mixamount", "", 1.0, Kommazahl, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<ReactionDiffusionParams>(), Typ{"reactionDiffusion", Traeger::EigenerHost, {
        LV_FELD(ReactionDiffusionParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(ReactionDiffusionParams, colorCycle, "colorCycle", "colorcycle", "", 1.0, Kommazahl, Live, -4.0, 4.0),
        LV_FELD(ReactionDiffusionParams, colorScale, "colorScale", "colorscale", "", 1.0, Kommazahl, Live, 0.01, 8.0),
        LV_FELD(ReactionDiffusionParams, diffA, "diffA", "diffa", "", 1.0, Kommazahl, Live, 0.0, 2.0),
        LV_FELD(ReactionDiffusionParams, diffB, "diffB", "diffb", "", 1.0, Kommazahl, Live, 0.0, 2.0),
        LV_FELD(ReactionDiffusionParams, feed, "feed", "feed", "", 1.0, Kommazahl, Live, 0.0, 0.1),
        LV_FELD(ReactionDiffusionParams, kill, "kill", "kill", "", 1.0, Kommazahl, Live, 0.0, 0.1),
        LV_FELD(ReactionDiffusionParams, seedOnBeat, "seedOnBeat", "seedonbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(ReactionDiffusionParams, stepsPerFrame, "stepsPerFrame", "stepsperframe", "", 1.0, Ganzzahl, Live, 1.0, 64.0),
    }}});
    out.push_back({ix<RenderScaleParams>(), Typ{"renderScale", Traeger::Keiner, {
        LV_FELD(RenderScaleParams, divisor, "divisor", "divisor", "", 1.0, Ganzzahl, Struktur, 1.0, 8.0),
        LV_FELD(RenderScaleParams, filter, "filter", "filter", "", 1.0, Aufzaehlung, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<RotatingStarsParams>(), Typ{"rotatingStars", Traeger::ParamSkript, {
        LV_FELD(RotatingStarsParams, audioGain, "audioGain", "audiogain", "", 1.0, Kommazahl, Live, 0.0, 8.0),
        LV_FELD(RotatingStarsParams, bandHi, "bandHi", "bandhi", "", 1.0, Ganzzahl, Live, 1.0, 576.0),
        LV_FELD(RotatingStarsParams, bandLo, "bandLo", "bandlo", "", 1.0, Ganzzahl, Live, 0.0, 575.0),
        LV_FELD(RotatingStarsParams, baseRadius, "baseRadius", "baseradius", "", 1.0, Kommazahl, Live, 0.0, 2.0),
        LV_FELD(RotatingStarsParams, orbit, "orbit", "orbit", "", 1.0, Kommazahl, Live, 0.0, 2.0),
        LV_FELD(RotatingStarsParams, points, "points", "points", "", 1.0, Ganzzahl, Live, 2.0, 64.0),
        LV_FELD(RotatingStarsParams, rotSpeed, "rotSpeed", "rotspeed", "", 1.0, Kommazahl, Live, -1.0, 1.0),
        LV_FELD(RotatingStarsParams, skip, "skip", "skip", "", 1.0, Ganzzahl, Live, 1.0, 32.0),
        LV_FELD(RotatingStarsParams, stars, "stars", "stars", "", 1.0, Ganzzahl, Live, 1.0, 16.0),
    }}});
    out.push_back({ix<RotoBlitterParams>(), Typ{"rotoBlitter", Traeger::ParamSkript, {
        LV_FELD(RotoBlitterParams, beatReverse, "beatReverse", "beatreverse", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(RotoBlitterParams, beatReverseSpeed, "beatReverseSpeed", "beatreversespeed", "", 1.0, Ganzzahl, Live, 0.0, 8.0),
        LV_FELD(RotoBlitterParams, beatZoomJump, "beatZoomJump", "beatzoomjump", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(RotoBlitterParams, blend, "blend", "blend", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(RotoBlitterParams, rotDir, "rotDir", "rotdir", "", 1.0, Ganzzahl, Live, 0.0, 64.0),
        LV_FELD(RotoBlitterParams, subpixel, "subpixel", "subpixel", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(RotoBlitterParams, zoomScale, "zoomScale", "zoomscale", "", 1.0, Ganzzahl, Live, 0.0, 256.0),
        LV_FELD(RotoBlitterParams, zoomScale2, "zoomScale2", "zoomscale2", "", 1.0, Ganzzahl, Live, 0.0, 256.0),
    }}});
    out.push_back({ix<SetRenderModeParams>(), Typ{"setRenderMode", Traeger::ParamSkript, {
        LV_FELD(SetRenderModeParams, adjustAlpha, "adjustAlpha", "adjustalpha", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(SetRenderModeParams, lineBlend, "lineBlend", "lineblend", "", 1.0, Aufzaehlung, Live, 0.0, 9.0),
        LV_FELD(SetRenderModeParams, lineWidth, "lineWidth", "linewidth", "", 1.0, Ganzzahl, Live, 0.0, 255.0),
        LV_FELD(SetRenderModeParams, enabled, "overrideBlend", "overrideblend", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<ShadertoyParams>(), Typ{"shadertoy", Traeger::ParamSkript, {
        LV_FELD(ShadertoyParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
    }}});
    out.push_back({ix<SimpleScopeParams>(), Typ{"simpleScope", Traeger::ParamSkript, {
        LV_FELD(SimpleScopeParams, channel, "channel", "channel", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(SimpleScopeParams, mode, "mode", "mode", "", 1.0, Aufzaehlung, Live, 0.0, 5.0),
        LV_FELD(SimpleScopeParams, position, "position", "position", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
    }}});
    out.push_back({ix<StarfieldParams>(), Typ{"starfield", Traeger::ParamSkript, {
        LV_FELD(StarfieldParams, beatSpeed, "beatSpeed", "beatspeed", "", 1.0, Kommazahl, Live, 0.1, 50.0),
        LV_FELD(StarfieldParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 0.0),
        LV_FELD(StarfieldParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(StarfieldParams, durationFrames, "durationFrames", "durationframes", "", 1.0, Ganzzahl, Live, 1.0, 200.0),
        LV_FELD(StarfieldParams, maxStars, "maxStars", "maxstars", "", 1.0, Ganzzahl, Struktur, 1.0, 8192.0),
        LV_FELD(StarfieldParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(StarfieldParams, warpSpeed, "warpSpeed", "warpspeed", "", 1.0, Kommazahl, Live, 0.1, 50.0),
    }}});
    out.push_back({ix<StrangeAttractorParams>(), Typ{"strangeAttractor", Traeger::EigenerHost, {
        LV_FELD(StrangeAttractorParams, a, "a", "a", "", 1.0, Kommazahl, Live, -3.0, 3.0),
        LV_FELD(StrangeAttractorParams, b, "b", "b", "", 1.0, Kommazahl, Live, -3.0, 3.0),
        LV_FELD(StrangeAttractorParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(StrangeAttractorParams, c, "c", "c", "", 1.0, Kommazahl, Live, -3.0, 3.0),
        LV_FELD(StrangeAttractorParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(StrangeAttractorParams, d, "d", "d", "", 1.0, Kommazahl, Live, -3.0, 3.0),
        LV_FELD(StrangeAttractorParams, dotSize, "dotSize", "dotsize", "", 1.0, Kommazahl, Live, 1.0, 32.0),
        LV_FELD(StrangeAttractorParams, type, "ftype", "ftype", "", 1.0, Aufzaehlung, Live, 0.0, 3.0),
        LV_FELD(StrangeAttractorParams, points, "points", "points", "", 1.0, Ganzzahl, Live, 1.0, 100000.0),
        LV_FELD(StrangeAttractorParams, rotation, "rotation", "rotation", "", 1.0, Kommazahl, Live, -6.2832, 6.2832),
        LV_FELD(StrangeAttractorParams, rotationSpeed, "rotationSpeed", "rotationspeed", "", 1.0, Kommazahl, Live, -2.0, 2.0),
        LV_FELD(StrangeAttractorParams, scale, "scale", "scale", "", 1.0, Kommazahl, Live, 0.01, 4.0),
        LV_FELD(StrangeAttractorParams, useGradient, "useGradient", "usegradient", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<SuperScopeParams>(), Typ{"superScope", Traeger::EigenerHost, {
        LV_FELD(SuperScopeParams, audioChannel, "audioChannel", "audiochannel", "", 1.0, Aufzaehlung, Live, 0.0, 4.0),
        LV_FELD(SuperScopeParams, colorCycleFrames, "colorCycleFrames", "colorcycleframes", "", 1.0, Ganzzahl, Live, 1.0, 600.0),
        LV_FELD(SuperScopeParams, dotSize, "dotSize", "dotsize", "", 1.0, Kommazahl, Live, 1.0, 50.0),
        LV_FELD(SuperScopeParams, lineBlend, "lineBlend", "lineblend", "", 1.0, Aufzaehlung, Live, 0.0, 9.0),
        LV_FELD(SuperScopeParams, lineWidth, "lineWidth", "linewidth", "", 1.0, Kommazahl, Live, 1.0, 20.0),
        LV_FELD(SuperScopeParams, pointCount, "pointCount", "pointcount", "", 1.0, Ganzzahl, Live, 1.0, 4096.0),
        LV_FELD(SuperScopeParams, renderMode, "renderMode", "rendermode", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(SuperScopeParams, spectrumSource, "spectrumSource", "spectrumsource", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<SuperScope3DParams>(), Typ{"superScope3d", Traeger::EigenerHost, {
        LV_FELD(SuperScope3DParams, audioChannel, "audioChannel", "audiochannel", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(SuperScope3DParams, falloff, "falloff", "falloff", "", 1.0, Kommazahl, Live, 0.5, 32.0),
        LV_FELD(SuperScope3DParams, pointCount, "pointCount", "pointcount", "n", 1.0, Ganzzahl, Live, 1.0, 4096.0),
        LV_FELD(SuperScope3DParams, renderMode, "renderMode", "rendermode", "", 1.0, Aufzaehlung, Live, 0.0, 1.0),
        LV_FELD(SuperScope3DParams, size, "size", "size", "", 1.0, Kommazahl, Live, 0.0001, 100.0),
        LV_FELD(SuperScope3DParams, spectrumSource, "spectrumSource", "spectrumsource", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<Tentacles3DParams>(), Typ{"tentacles3d", Traeger::ParamSkript, {
        LV_FELD(Tentacles3DParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(Tentacles3DParams, count, "count", "count", "", 1.0, Ganzzahl, Live, 1.0, 16.0),
        LV_FELD(Tentacles3DParams, length, "length", "length", "", 1.0, Kommazahl, Live, 0.05, 2.0),
        LV_FELD(Tentacles3DParams, segments, "segments", "segments", "", 1.0, Ganzzahl, Live, 2.0, 256.0),
        LV_FELD(Tentacles3DParams, speed, "speed", "speed", "", 1.0, Kommazahl, Live, 0.0, 5.0),
        LV_FELD(Tentacles3DParams, sway, "sway", "sway", "", 1.0, Kommazahl, Live, 0.0, 4.0),
        LV_FELD(Tentacles3DParams, taper, "taper", "taper", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(Tentacles3DParams, thickness, "thickness", "thickness", "", 1.0, Kommazahl, Live, 1.0, 64.0),
        LV_FELD(Tentacles3DParams, waves, "waves", "waves", "", 1.0, Kommazahl, Live, 0.0, 20.0),
    }}});
    out.push_back({ix<Terrain3DParams>(), Typ{"terrain3d", Traeger::EigenerHost, {
        LV_FELD(Terrain3DParams, baseAmp, "baseAmp", "baseamp", "", 1.0, Kommazahl, Live, 0.0, 10.0),
        LV_FELD(Terrain3DParams, colorHigh, "colorHigh", "colorhigh", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(Terrain3DParams, colorLow, "colorLow", "colorlow", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(Terrain3DParams, dotSize, "dotSize", "dotsize", "", 1.0, Kommazahl, Live, 0.0001, 10.0),
        LV_FELD(Terrain3DParams, drawDots, "drawDots", "drawdots", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(Terrain3DParams, drawMesh, "drawMesh", "drawmesh", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(Terrain3DParams, extent, "extent", "extent", "", 1.0, Kommazahl, Live, 0.1, 100.0),
        LV_FELD(Terrain3DParams, falloff, "falloff", "falloff", "", 1.0, Kommazahl, Live, 0.5, 32.0),
        LV_FELD(Terrain3DParams, flatten, "flatten", "flatten", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(Terrain3DParams, meshColor, "meshColor", "meshcolor", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(Terrain3DParams, relax, "relax", "relax", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(Terrain3DParams, resolution, "resolution", "resolution", "res", 1.0, Ganzzahl, Struktur, 8.0, 128.0),
        LV_FELD(Terrain3DParams, ringAmp, "ringAmp", "ringamp", "rings", 1.0, Kommazahl, Live, 0.0, 10.0),
        LV_FELD(Terrain3DParams, yOffset, "yOffset", "yoffset", "", 1.0, Kommazahl, Live, -100.0, 100.0),
    }}});
    out.push_back({ix<TexerParams>(), Typ{"texer", Traeger::ParamSkript, {
        LV_FELD(TexerParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 1.0),
        LV_FELD(TexerParams, particles, "particles", "particles", "", 1.0, Ganzzahl, Live, 1.0, 4096.0),
    }}});
    out.push_back({ix<TexerIIParams>(), Typ{"texerII", Traeger::EigenerHost, {
        LV_FELD(TexerIIParams, colorFiltering, "colorFiltering", "colorfiltering", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(TexerIIParams, resizing, "resizing", "resizing", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(TexerIIParams, wrapAround, "wrapAround", "wraparound", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<TextParams>(), Typ{"text", Traeger::Keiner, {
        LV_FELD(TextParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(TextParams, color, "color", "color", "", 1.0, Farbe, Struktur, kNaN, kNaN),
        LV_FELD(TextParams, fontHeight, "fontHeight", "fontheight", "", 1.0, Ganzzahl, Struktur, 4.0, 400.0),
        LV_FELD(TextParams, fontWeight, "fontWeight", "fontweight", "", 1.0, Ganzzahl, Struktur, 100.0, 900.0),
        LV_FELD(TextParams, hAlign, "hAlign", "halign", "", 1.0, Aufzaehlung, Struktur, 0.0, 2.0),
        LV_FELD(TextParams, insertBlank, "insertBlank", "insertblank", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(TextParams, italic, "italic", "italic", "", 1.0, Schalter, Struktur, 0.0, 1.0),
        LV_FELD(TextParams, normSpeed, "normSpeed", "normspeed", "", 1.0, Ganzzahl, Live, 1.0, 600.0),
        LV_FELD(TextParams, onBeat, "onBeat", "onbeat", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(TextParams, onBeatSpeed, "onBeatSpeed", "onbeatspeed", "", 1.0, Ganzzahl, Live, 1.0, 600.0),
        LV_FELD(TextParams, outline, "outline", "outline", "", 1.0, Schalter, Struktur, 0.0, 1.0),
        LV_FELD(TextParams, outlineColor, "outlineColor", "outlinecolor", "", 1.0, Farbe, Struktur, kNaN, kNaN),
        LV_FELD(TextParams, outlineSize, "outlineSize", "outlinesize", "", 1.0, Ganzzahl, Struktur, 1.0, 32.0),
        LV_FELD(TextParams, randomPos, "randomPos", "randompos", "", 1.0, Schalter, Struktur, 0.0, 1.0),
        LV_FELD(TextParams, randomWord, "randomWord", "randomword", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(TextParams, shadow, "shadow", "shadow", "", 1.0, Schalter, Struktur, 0.0, 1.0),
        LV_FELD(TextParams, underline, "underline", "underline", "", 1.0, Schalter, Struktur, 0.0, 1.0),
        LV_FELD(TextParams, vAlign, "vAlign", "valign", "", 1.0, Aufzaehlung, Struktur, 0.0, 2.0),
        LV_FELD(TextParams, xShift, "xShift", "xshift", "", 1.0, Ganzzahl, Struktur, -100.0, 100.0),
        LV_FELD(TextParams, yShift, "yShift", "yshift", "", 1.0, Ganzzahl, Struktur, -100.0, 100.0),
    }}});
    out.push_back({ix<TimescopeParams>(), Typ{"timescope", Traeger::ParamSkript, {
        LV_FELD(TimescopeParams, bands, "bands", "bands", "", 1.0, Ganzzahl, Live, 1.0, 576.0),
        LV_FELD(TimescopeParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 3.0),
        LV_FELD(TimescopeParams, channel, "channel", "channel", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(TimescopeParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(TimescopeParams, useChannel, "useChannel", "usechannel", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<TriangleParams>(), Typ{"triangle", Traeger::EigenerHost, {
        LV_FELD(TriangleParams, filled, "filled", "filled", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(TriangleParams, lineWidth, "lineWidth", "linewidth", "", 1.0, Kommazahl, Live, 1.0, 20.0),
    }}});
    out.push_back({ix<UniqueToneParams>(), Typ{"uniqueTone", Traeger::ParamSkript, {
        LV_FELD(UniqueToneParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(UniqueToneParams, color, "color", "color", "", 1.0, Farbe, Live, kNaN, kNaN),
        LV_FELD(UniqueToneParams, invert, "invert", "invert", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    out.push_back({ix<VideoDelayParams>(), Typ{"videoDelay", Traeger::ParamSkript, {
        LV_FELD(VideoDelayParams, delay, "delay", "delay", "", 1.0, Ganzzahl, Struktur, 1.0, 128.0),
        LV_FELD(VideoDelayParams, useBeats, "useBeats", "usebeats", "", 1.0, Schalter, Struktur, 0.0, 1.0),
    }}});
    out.push_back({ix<VideoSourceParams>(), Typ{"videoSource", Traeger::ParamSkript, {
        LV_FELD(VideoSourceParams, blend, "blend", "blend", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(VideoSourceParams, fit, "fit", "fit", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(VideoSourceParams, loop, "loop", "loop", "", 1.0, Schalter, Live, 0.0, 1.0),
        LV_FELD(VideoSourceParams, opacity, "opacity", "opacity", "", 1.0, Kommazahl, Live, 0.0, 1.0),
        LV_FELD(VideoSourceParams, source, "source", "source", "", 1.0, Aufzaehlung, Struktur, 0.0, 2.0),
        LV_FELD(VideoSourceParams, speed, "speed", "speed", "", 1.0, Kommazahl, Live, kNaN, kNaN),
        LV_FELD(VideoSourceParams, streaming, "streaming", "streaming", "", 1.0, Schalter, Struktur, 0.0, 1.0),
    }}});
    out.push_back({ix<WaterBumpParams>(), Typ{"waterBump", Traeger::ParamSkript, {
        LV_FELD(WaterBumpParams, density, "density", "density", "", 1.0, Ganzzahl, Live, 1.0, 12.0),
        LV_FELD(WaterBumpParams, depth, "depth", "depth", "", 1.0, Ganzzahl, Live, 0.0, 2000.0),
        LV_FELD(WaterBumpParams, displaceScale, "displaceScale", "displacescale", "", 1.0, Kommazahl, Live, 0.0, 40.0),
        LV_FELD(WaterBumpParams, dropRadius, "dropRadius", "dropradius", "", 1.0, Ganzzahl, Live, 1.0, 200.0),
        LV_FELD(WaterBumpParams, dropX, "dropX", "dropx", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(WaterBumpParams, dropY, "dropY", "dropy", "", 1.0, Aufzaehlung, Live, 0.0, 2.0),
        LV_FELD(WaterBumpParams, randomDrop, "randomDrop", "randomdrop", "", 1.0, Schalter, Live, 0.0, 1.0),
    }}});
    return out;
}
#undef LV_FELD

struct Tabelle
{
    std::vector<std::pair<std::size_t, Typ>> daten;
    std::vector<const Typ*> nachIndex;
    std::vector<const Typ*> alle;
};

const Tabelle& tabelle()
{
    static const Tabelle t = [] {
        Tabelle x;
        x.daten = baue();
        x.nachIndex.assign(std::variant_size_v<EffectParams>, nullptr);
        for (const auto& [index, typ] : x.daten)
        {
            x.nachIndex[index] = &typ;
            x.alle.push_back(&typ);
        }
        return x;
    }();
    return t;
}
}  // namespace

const Typ* typ(std::size_t index)
{
    const auto& t = tabelle().nachIndex;
    return index < t.size() ? t[index] : nullptr;
}

const std::vector<const Typ*>& alle()
{
    return tabelle().alle;
}

}  // namespace lumi::multieffect::feldtabelle
