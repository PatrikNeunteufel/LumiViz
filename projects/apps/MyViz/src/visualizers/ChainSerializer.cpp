/**
 ****************************************************************************************
 * @file   ChainSerializer.cpp
 * @brief  Implementation of the multi-effect chain JSON persistence (Roadmap 5.6)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 ****************************************************************************************
 */

#include "visualizers/multieffect/ChainSerializer.hpp"

#include "visualizers/milkdrop/MilkdropSerializer.hpp"
#include "visualizers/milkdrop/MilkdropTextureResolve.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>

#include <cmath>

namespace lumi::multieffect {

namespace {

constexpr int kFormatVersion = 1;

// --- small typed getters with defaults ---------------------------------------
int getInt(const QJsonObject& o, const char* key, int def)
{
    return o.contains(key) ? o.value(key).toInt(def) : def;
}
double getDouble(const QJsonObject& o, const char* key, double def)
{
    return o.contains(key) ? o.value(key).toDouble(def) : def;
}
bool getBool(const QJsonObject& o, const char* key, bool def)
{
    return o.contains(key) ? o.value(key).toBool(def) : def;
}
std::string getStr(const QJsonObject& o, const char* key)
{
    return o.value(key).toString().toStdString();
}
// Mit Vorgabe: der Aufrufer reicht sein FELD herein, damit die Vorgabe
// nur im Struct steht (S56). Ein fehlender Schluessel laesst das Feld
// unangetastet — anders als die Fassung darueber, die "" liefert.
std::string getStr(const QJsonObject& o, const char* key,
                   const std::string& def)
{
    return o.contains(key) ? o.value(key).toString().toStdString() : def;
}
uint32_t getColor(const QJsonObject& o, const char* key, uint32_t def)
{
    return o.contains(key) ? static_cast<uint32_t>(o.value(key).toDouble(def)) : def;
}

// --- param -> JSON (writes fields into `o`, type key set by caller) ----------
struct WriteVisitor
{
    QJsonObject& o;

    void operator()(const ListParams& p) const
    {
        o["clearEveryFrame"] = p.clearEveryFrame;
        o["blendIn"] = static_cast<int>(p.blendIn);
        o["blendOut"] = static_cast<int>(p.blendOut);
        o["inAdjustAlpha"] = p.inAdjustAlpha;
        o["outAdjustAlpha"] = p.outAdjustAlpha;
        o["bufferIn"] = p.bufferIn;
        o["bufferOut"] = p.bufferOut;
        o["bufferInInvert"] = p.bufferInInvert;
        o["bufferOutInvert"] = p.bufferOutInvert;
        o["onBeatRender"] = p.onBeatRender;
        o["onBeatFrames"] = p.onBeatFrames;
        o["useCode"] = p.useCode;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
    }
    void operator()(const HostGroupParams& p) const
    {
        o["blendOut"] = static_cast<int>(p.blendOut);
        o["outAdjustAlpha"] = p.outAdjustAlpha;
        o["crossfadeSeconds"] = p.crossfadeSeconds;
        o["curveIn"] = p.curveIn;
        o["curveOut"] = p.curveOut;
        if (!p.sourceFile.empty())
            o["sourceFile"] = QString::fromStdString(p.sourceFile);
    }
    void operator()(const ClearParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["onlyFirst"] = p.onlyFirst;
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const FadeoutParams& p) const
    {
        o["fadeLen"] = p.fadeLen;
        o["color"] = static_cast<double>(p.color);
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const InvertParams&) const {}
    void operator()(const BrightnessParams& p) const
    {
        o["red"] = p.red;
        o["green"] = p.green;
        o["blue"] = p.blue;
        o["exclude"] = p.exclude;
        o["color"] = static_cast<double>(p.color);
        o["distance"] = p.distance;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const FastBrightnessParams& p) const
    {
        o["dir"] = p.dir;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const BlurParams& p) const
    {
        o["strength"] = p.strength;
        o["roundUp"] = p.roundUp;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const MirrorParams& p) const
    {
        o["mode"] = p.mode;
        o["onBeatRandom"] = p.onBeatRandom;
        o["smooth"] = p.smooth;
        o["slower"] = p.slower;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const OnBeatClearParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["everyNBeats"] = p.everyNBeats;
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const ColorfadeParams& p) const
    {
        o["faderR"] = p.faderR;   o["faderG"] = p.faderG;   o["faderB"] = p.faderB;
        o["beatFaderR"] = p.beatFaderR;
        o["beatFaderG"] = p.beatFaderG;
        o["beatFaderB"] = p.beatFaderB;
        o["onBeatFrames"] = p.onBeatFrames;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const ColorModifierParams& p) const
    {
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["levelCode"] = QString::fromStdString(p.levelCode);
        o["recompute"] = p.recompute;
    }
    void operator()(const MovementParams& p) const
    {
        o["code"] = QString::fromStdString(p.code);
        o["rectCoords"] = p.rectCoords;
        o["wrap"] = p.wrap;
        o["blend"] = p.blend;
        o["subpixel"] = p.subpixel;
        o["sourceMapped"] = p.sourceMapped;
        o["builtinRemap"] = p.builtinRemap;
    }
    void operator()(const DynamicMovementParams& p) const
    {
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pointCode"] = QString::fromStdString(p.pointCode);
        o["xres"] = p.xres;   o["yres"] = p.yres;
        o["rectCoords"] = p.rectCoords;
        o["wrap"] = p.wrap;
        o["blend"] = p.blend;
        o["nomove"] = p.nomove;
        o["subpixel"] = p.subpixel;
        o["buffern"] = p.buffern;
    }
    void operator()(const BlitterFeedbackParams& p) const
    {
        o["scale"] = p.scale;
        o["scale2"] = p.scale2;
        o["onBeat"] = p.onBeat;
        o["blend"] = p.blend;
        o["subpixel"] = p.subpixel;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const RotoBlitterParams& p) const
    {
        o["zoomScale"] = p.zoomScale;
        o["zoomScale2"] = p.zoomScale2;
        o["rotDir"] = p.rotDir;
        o["blend"] = p.blend;
        o["beatReverse"] = p.beatReverse;
        o["beatReverseSpeed"] = p.beatReverseSpeed;
        o["beatZoomJump"] = p.beatZoomJump;
        o["subpixel"] = p.subpixel;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const BufferSaveParams& p) const
    {
        o["slot"] = p.slot;
        o["dir"] = p.dir;
        o["blend"] = static_cast<int>(p.blend);
        o["adjustAlpha"] = p.adjustAlpha;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const CustomBpmParams& p) const
    {
        o["arbitrary"] = p.arbitrary;
        o["arbitraryMs"] = p.arbitraryMs;
        o["skip"] = p.skip;
        o["skipCount"] = p.skipCount;
        o["invert"] = p.invert;
        o["skipFirst"] = p.skipFirst;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const SuperScopeParams& p) const
    {
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pointCode"] = QString::fromStdString(p.pointCode);
        o["pointCount"] = p.pointCount;
        o["renderMode"] = p.renderMode;
        o["lineWidth"] = p.lineWidth;
        o["dotSize"] = p.dotSize;
        o["audioChannel"] = p.audioChannel;
        o["spectrumSource"] = p.spectrumSource;
        o["lineBlend"] = p.lineBlend;
        o["colorBlend"] = p.colorBlend;
        o["colorCycleFrames"] = p.colorCycleFrames;
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
    }
    void operator()(const MosaicParams& p) const
    {
        o["quality"] = p.quality;
        o["quality2"] = p.quality2;
        o["onBeat"] = p.onBeat;
        o["durationFrames"] = p.durationFrames;
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const GrainParams& p) const
    {
        o["amount"] = p.amount;
        o["staticGrain"] = p.staticGrain;
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const ScatterParams&) const {}
    void operator()(const WaterParams&) const {}
    void operator()(const DynamicShiftParams& p) const
    {
        o["blend"] = p.blend;
        o["subpixel"] = p.subpixel;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const DynamicDistanceModifierParams& p) const
    {
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pixelCode"] = QString::fromStdString(p.pixelCode);
        o["blend"] = p.blend;
        o["subpixel"] = p.subpixel;
    }
    void operator()(const MovingParticleParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["maxDistance"] = p.maxDistance;
        o["size"] = p.size;
        o["size2"] = p.size2;
        o["onBeatSize"] = p.onBeatSize;
        o["blend"] = p.blend;
        o["spring"] = p.spring;
        o["damping"] = p.damping;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const ColorMapParams& p) const
    {
        o["key"] = p.key;
        o["blendMode"] = p.blendMode;
        o["adjustBlend"] = p.adjustBlend;
        QJsonArray pos;
        for (int v : p.stopPos) pos.append(v);
        QJsonArray col;
        for (uint32_t c : p.stopColor) col.append(static_cast<double>(c));
        o["stopPos"] = pos;
        o["stopColor"] = col;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const Fractal2DParams& p) const
    {
        o["ftype"] = p.type;
        o["centerX"] = static_cast<double>(p.centerX);
        o["centerY"] = static_cast<double>(p.centerY);
        o["zoom"] = static_cast<double>(p.zoom);
        o["rotation"] = static_cast<double>(p.rotation);
        o["maxIter"] = p.maxIter;
        o["juliaX"] = static_cast<double>(p.juliaX);
        o["juliaY"] = static_cast<double>(p.juliaY);
        o["power"] = static_cast<double>(p.power);
        o["escapeR"] = static_cast<double>(p.escapeR);
        o["smooth"] = p.smooth;
        o["colorScale"] = static_cast<double>(p.colorScale);
        o["colorCycle"] = static_cast<double>(p.colorCycle);
        o["insideColor"] = static_cast<double>(p.insideColor);
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const DomainWarpParams& p) const
    {
        o["octaves"] = p.octaves;
        o["lacunarity"] = static_cast<double>(p.lacunarity);
        o["gain"] = static_cast<double>(p.gain);
        o["scale"] = static_cast<double>(p.scale);
        o["warp"] = static_cast<double>(p.warp);
        o["warpScale"] = static_cast<double>(p.warpScale);
        o["speed"] = static_cast<double>(p.speed);
        o["offsetX"] = static_cast<double>(p.offsetX);
        o["offsetY"] = static_cast<double>(p.offsetY);
        o["colorScale"] = static_cast<double>(p.colorScale);
        o["colorCycle"] = static_cast<double>(p.colorCycle);
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const SetRenderModeParams& p) const
    {
        // NICHT "enabled": diesen Schluessel belegt schon der Knoten selbst
        // (nodeToJson schreibt ihn VOR dem Visitor, der ihn dann ueberschrieb).
        // Damit ging der Auge-Zustand eines Set-Render-Mode-Knotens beim
        // Speichern verloren und beide Flags lasen beim Laden denselben Wert
        // (Befund S51, aufgefallen im .lvfx-Roundtrip).
        o["overrideBlend"] = p.enabled;
        o["lineWidth"] = p.lineWidth;
        o["lineBlend"] = p.lineBlend;
        o["adjustAlpha"] = p.adjustAlpha;
    }
    void operator()(const Fractal3DParams& p) const
    {
        o["ftype"] = p.type;
        o["yaw"] = static_cast<double>(p.yaw);
        o["pitch"] = static_cast<double>(p.pitch);
        o["dist"] = static_cast<double>(p.dist);
        o["fov"] = static_cast<double>(p.fov);
        o["power"] = static_cast<double>(p.power);
        o["scale"] = static_cast<double>(p.scale);
        o["fold"] = static_cast<double>(p.fold);
        o["maxSteps"] = p.maxSteps;
        o["maxIter"] = p.maxIter;
        o["juliaX"] = static_cast<double>(p.juliaX);
        o["juliaY"] = static_cast<double>(p.juliaY);
        o["juliaZ"] = static_cast<double>(p.juliaZ);
        o["juliaW"] = static_cast<double>(p.juliaW);
        o["lightYaw"] = static_cast<double>(p.lightYaw);
        o["lightPitch"] = static_cast<double>(p.lightPitch);
        o["ambient"] = static_cast<double>(p.ambient);
        o["ao"] = p.ao;
        o["colorScale"] = static_cast<double>(p.colorScale);
        o["colorCycle"] = static_cast<double>(p.colorCycle);
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["background"] = static_cast<double>(p.background);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const LyapunovParams& p) const
    {
        o["sequence"] = QString::fromStdString(p.sequence);
        o["aMin"] = static_cast<double>(p.aMin);
        o["aMax"] = static_cast<double>(p.aMax);
        o["bMin"] = static_cast<double>(p.bMin);
        o["bMax"] = static_cast<double>(p.bMax);
        o["warmup"] = p.warmup;
        o["iterations"] = p.iterations;
        o["negColor"] = static_cast<double>(p.negColor);
        o["colorScale"] = static_cast<double>(p.colorScale);
        o["colorCycle"] = static_cast<double>(p.colorCycle);
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const KleinianParams& p) const
    {
        o["p"] = p.p;
        o["q"] = p.q;
        o["iterations"] = p.iterations;
        o["morph"] = static_cast<double>(p.morph);
        o["zoom"] = static_cast<double>(p.zoom);
        o["rotation"] = static_cast<double>(p.rotation);
        o["colorScale"] = static_cast<double>(p.colorScale);
        o["colorCycle"] = static_cast<double>(p.colorCycle);
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const FractalZoomerParams& p) const
    {
        o["ftype"] = p.type;
        o["centerX"] = static_cast<double>(p.centerX);
        o["centerY"] = static_cast<double>(p.centerY);
        o["juliaX"] = static_cast<double>(p.juliaX);
        o["juliaY"] = static_cast<double>(p.juliaY);
        o["maxIter"] = p.maxIter;
        o["zoomSpeed"] = static_cast<double>(p.zoomSpeed);
        o["rotationSpeed"] = static_cast<double>(p.rotationSpeed);
        o["feedback"] = static_cast<double>(p.feedback);
        o["colorScale"] = static_cast<double>(p.colorScale);
        o["colorCycle"] = static_cast<double>(p.colorCycle);
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["insideColor"] = static_cast<double>(p.insideColor);
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const StrangeAttractorParams& p) const
    {
        o["ftype"] = p.type;
        o["a"] = static_cast<double>(p.a);
        o["b"] = static_cast<double>(p.b);
        o["c"] = static_cast<double>(p.c);
        o["d"] = static_cast<double>(p.d);
        o["points"] = p.points;
        o["scale"] = static_cast<double>(p.scale);
        o["rotation"] = static_cast<double>(p.rotation);
        o["rotationSpeed"] = static_cast<double>(p.rotationSpeed);
        o["color"] = static_cast<double>(p.color);
        o["useGradient"] = p.useGradient;
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["dotSize"] = static_cast<double>(p.dotSize);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const FlameParams& p) const
    {
        o["variation"] = p.variation;
        o["functions"] = p.functions;
        o["points"] = p.points;
        o["scale"] = static_cast<double>(p.scale);
        o["rotation"] = static_cast<double>(p.rotation);
        o["rotationSpeed"] = static_cast<double>(p.rotationSpeed);
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["dotSize"] = static_cast<double>(p.dotSize);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const ReactionDiffusionParams& p) const
    {
        o["feed"] = static_cast<double>(p.feed);
        o["kill"] = static_cast<double>(p.kill);
        o["diffA"] = static_cast<double>(p.diffA);
        o["diffB"] = static_cast<double>(p.diffB);
        o["stepsPerFrame"] = p.stepsPerFrame;
        o["seedOnBeat"] = p.seedOnBeat;
        o["colorScale"] = static_cast<double>(p.colorScale);
        o["colorCycle"] = static_cast<double>(p.colorCycle);
        o["gradientPreset"] = QString::fromStdString(p.gradientPreset);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const BufferBlendParams& p) const
    {
        o["bufferA"] = p.bufferA;
        o["bufferB"] = p.bufferB;
        o["mode"] = p.mode;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const JherikoGlobalParams& p) const
    {
        o["loadMode"] = p.loadMode;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const SimpleScopeParams& p) const
    {
        o["mode"] = p.mode;
        o["channel"] = p.channel;
        o["position"] = p.position;
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const BassSpinParams& p) const
    {
        o["left"] = p.left;
        o["right"] = p.right;
        o["colorLeft"] = static_cast<double>(p.colorLeft);
        o["colorRight"] = static_cast<double>(p.colorRight);
        o["mode"] = p.mode;
        o["smoothing"] = p.smoothing;
        o["spinStep"] = p.spinStep;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const OscStarParams& p) const
    {
        o["channel"] = p.channel;
        o["position"] = p.position;
        o["size"] = p.size;
        o["rot"] = p.rot;
        o["spokes"] = p.spokes;
        o["rotScale"] = p.rotScale;
        o["amplitude"] = p.amplitude;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
    }
    void operator()(const OscRingParams& p) const
    {
        o["source"] = p.source;
        o["channel"] = p.channel;
        o["position"] = p.position;
        o["size"] = p.size;
        o["segments"] = p.segments;
        o["baseScale"] = p.baseScale;
        o["audioScale"] = p.audioScale;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
    }
    void operator()(const RotatingStarsParams& p) const
    {
        o["points"] = p.points;
        o["skip"] = p.skip;
        o["stars"] = p.stars;
        o["rotSpeed"] = p.rotSpeed;
        o["orbit"] = p.orbit;
        o["baseRadius"] = p.baseRadius;
        o["audioGain"] = p.audioGain;
        o["bandLo"] = p.bandLo;
        o["bandHi"] = p.bandHi;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
    }
    void operator()(const PictureParams& p) const
    {
        o["filename"] = QString::fromStdString(p.filename);
        o["imageData"] = QString::fromStdString(p.imageData);
        o["blend"] = p.blend;
        o["keepAspect"] = p.keepAspect;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const PictureIIParams& p) const
    {
        o["filename"] = QString::fromStdString(p.filename);
        o["imageData"] = QString::fromStdString(p.imageData);
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const TexerParams& p) const
    {
        o["filename"] = QString::fromStdString(p.filename);
        o["imageData"] = QString::fromStdString(p.imageData);
        o["blend"] = p.blend;
        o["particles"] = p.particles;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const TexerIIParams& p) const
    {
        o["filename"] = QString::fromStdString(p.filename);
        o["imageData"] = QString::fromStdString(p.imageData);
        o["resizing"] = p.resizing;
        o["wrapAround"] = p.wrapAround;
        o["colorFiltering"] = p.colorFiltering;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pointCode"] = QString::fromStdString(p.pointCode);
    }
    void operator()(const TriangleParams& p) const
    {
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pointCode"] = QString::fromStdString(p.pointCode);
        o["filled"] = p.filled;
        o["lineWidth"] = p.lineWidth;
    }
    void operator()(const ConvolutionParams& p) const
    {
        o["edgeMode"] = p.edgeMode;
        o["absolute"] = p.absolute;
        o["twoPass"] = p.twoPass;
        o["bias"] = p.bias;
        o["scale"] = p.scale;
        QJsonArray k;
        for (int v : p.kernel) k.append(v);
        o["kernel"] = k;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const NormaliseParams&) const {}
    void operator()(const MultiFilterParams& p) const
    {
        o["effect"] = p.effect;
        o["onBeat"] = p.onBeat;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const AddBordersParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["size"] = p.size;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const ColorClipParams& p) const
    {
        o["mode"] = p.mode;
        o["clipColor"] = static_cast<double>(p.clipColor);
        o["outColor"] = static_cast<double>(p.outColor);
        o["distance"] = p.distance;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const UniqueToneParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["invert"] = p.invert;
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const InterleaveParams& p) const
    {
        o["x"] = p.x;
        o["y"] = p.y;
        o["color"] = static_cast<double>(p.color);
        o["blend"] = p.blend;
        o["onBeat"] = p.onBeat;
        o["x2"] = p.x2;
        o["y2"] = p.y2;
        o["beatDuration"] = p.beatDuration;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const BumpParams& p) const
    {
        o["depth"] = p.depth;
        o["depth2"] = p.depth2;
        o["onBeat"] = p.onBeat;
        o["durationFrames"] = p.durationFrames;
        o["invert"] = p.invert;
        o["oldStyle"] = p.oldStyle;
        o["blend"] = p.blend;
        o["buffern"] = p.buffern;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const WaterBumpParams& p) const
    {
        o["density"] = p.density;
        o["depth"] = p.depth;
        o["randomDrop"] = p.randomDrop;
        o["dropX"] = p.dropX;
        o["dropY"] = p.dropY;
        o["dropRadius"] = p.dropRadius;
        o["displaceScale"] = p.displaceScale;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const InterferencesParams& p) const
    {
        o["points"] = p.points;
        o["distance"] = p.distance;
        o["alpha"] = p.alpha;
        o["rotation"] = p.rotation;
        o["rotationInc"] = p.rotationInc;
        o["distance2"] = p.distance2;
        o["alpha2"] = p.alpha2;
        o["rotationInc2"] = p.rotationInc2;
        o["rgb"] = p.rgb;
        o["onBeat"] = p.onBeat;
        o["speed"] = p.speed;
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const Metaballs3DParams& p) const
    {
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
        o["count"] = p.count;
        o["radius"] = p.radius;
        o["speed"] = p.speed;
        o["threshold"] = p.threshold;
        o["blend"] = p.blend;
        o["spread"] = p.spread;
        o["depth"] = p.depth;
        o["phase"] = p.phase;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const Tentacles3DParams& p) const
    {
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
        o["count"] = p.count;
        o["segments"] = p.segments;
        o["length"] = p.length;
        o["thickness"] = p.thickness;
        o["speed"] = p.speed;
        o["blend"] = p.blend;
        o["sway"] = p.sway;
        o["waves"] = p.waves;
        o["taper"] = p.taper;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const FyrewurXParams& p) const
    {
        o["sparks"] = p.sparks;
        o["speed"] = p.speed;
        o["gravity"] = p.gravity;
        o["lifeSeconds"] = p.lifeSeconds;
        o["dotSize"] = p.dotSize;
        o["hueDrift"] = p.hueDrift;
        o["burstSpread"] = p.burstSpread;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const TextParams& p) const
    {
        o["text"] = QString::fromStdString(p.text);
        o["fontFace"] = QString::fromStdString(p.fontFace);
        o["fontHeight"] = p.fontHeight;
        o["fontWeight"] = p.fontWeight;
        o["italic"] = p.italic;
        o["underline"] = p.underline;
        o["color"] = static_cast<double>(p.color);
        o["blend"] = p.blend;
        o["onBeat"] = p.onBeat;
        o["onBeatSpeed"] = p.onBeatSpeed;
        o["normSpeed"] = p.normSpeed;
        o["insertBlank"] = p.insertBlank;
        o["randomPos"] = p.randomPos;
        o["randomWord"] = p.randomWord;
        o["hAlign"] = p.hAlign;
        o["vAlign"] = p.vAlign;
        o["xShift"] = p.xShift;
        o["yShift"] = p.yShift;
        o["outline"] = p.outline;
        o["outlineColor"] = static_cast<double>(p.outlineColor);
        o["outlineSize"] = p.outlineSize;
        o["shadow"] = p.shadow;
    }
    void operator()(const AviParams& p) const
    {
        o["filename"] = QString::fromStdString(p.filename);
        o["resolvedPath"] = QString::fromStdString(p.resolvedPath);
        o["blend"] = p.blend;
        o["adapt"] = p.adapt;
        o["persist"] = p.persist;
        o["speedMs"] = p.speedMs;
    }
    void operator()(const CommentParams& p) const
    {
        o["text"] = QString::fromStdString(p.text);
    }
    void operator()(const ImportNotesParams& p) const
    {
        o["text"] = QString::fromStdString(p.text);
    }
    void operator()(const RenderScaleParams& p) const
    {
        o["divisor"] = p.divisor;
        o["filter"] = p.filter;
    }
    void operator()(const BloomParams& p) const
    {
        o["downsample"] = p.downsample;
        o["radius"] = p.radius;
        o["intensity"] = p.intensity;
        o["threshold"] = p.threshold;
        o["vignette"] = p.vignette;
        o["vignetteStrength"] = p.vignetteStrength;
        o["post"] = p.post;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const Camera3DParams& p) const
    {
        o["px"] = p.px;
        o["py"] = p.py;
        o["pz"] = p.pz;
        o["tx"] = p.tx;
        o["ty"] = p.ty;
        o["tz"] = p.tz;
        o["fov"] = p.fov;
        o["roll"] = p.roll;
        o["fogStart"] = p.fogStart;
        o["fogEnd"] = p.fogEnd;
        o["fogColor"] = static_cast<double>(p.fogColor);
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const SuperScope3DParams& p) const
    {
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pointCode"] = QString::fromStdString(p.pointCode);
        o["pointCount"] = p.pointCount;
        o["renderMode"] = p.renderMode;
        o["size"] = p.size;
        o["falloff"] = p.falloff;
        o["audioChannel"] = p.audioChannel;
        o["spectrumSource"] = p.spectrumSource;
    }
    void operator()(const Terrain3DParams& p) const
    {
        o["resolution"] = p.resolution;
        o["extent"] = p.extent;
        o["baseAmp"] = p.baseAmp;
        o["yOffset"] = p.yOffset;
        o["ringAmp"] = p.ringAmp;
        o["relax"] = p.relax;
        o["flatten"] = p.flatten;
        o["drawMesh"] = p.drawMesh;
        o["meshColor"] = static_cast<double>(p.meshColor);
        o["drawDots"] = p.drawDots;
        o["dotSize"] = p.dotSize;
        o["falloff"] = p.falloff;
        o["colorLow"] = static_cast<double>(p.colorLow);
        o["colorHigh"] = static_cast<double>(p.colorHigh);
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pointCode"] = QString::fromStdString(p.pointCode);
    }
    void operator()(const GlowOrbsParams& p) const
    {
        o["orbCount"] = p.orbCount;
        o["haloScale"] = p.haloScale;
        o["haloIntensity"] = p.haloIntensity;
        o["falloff"] = p.falloff;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pointCode"] = QString::fromStdString(p.pointCode);
    }
    void operator()(const StarfieldParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["warpSpeed"] = p.warpSpeed;
        o["maxStars"] = p.maxStars;
        o["onBeat"] = p.onBeat;
        o["beatSpeed"] = p.beatSpeed;
        o["durationFrames"] = p.durationFrames;
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const TimescopeParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["blend"] = p.blend;
        o["channel"] = p.channel;
        o["useChannel"] = p.useChannel;
        o["bands"] = p.bands;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const DotGridParams& p) const
    {
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
        o["spacing"] = p.spacing;
        o["xMove"] = p.xMove;
        o["yMove"] = p.yMove;
        o["blend"] = p.blend;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const DotPlaneParams& p) const
    {
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
        o["rotVel"] = p.rotVel;
        o["angle"] = p.angle;
        o["camDistance"] = p.camDistance;
        o["settle"] = p.settle;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const DotFountainParams& p) const
    {
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
        o["rotVel"] = p.rotVel;
        o["angle"] = p.angle;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const ChannelShiftParams& p) const
    {
        o["mode"] = p.mode;
        o["onBeat"] = p.onBeat;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const ColorReductionParams& p) const
    {
        o["levels"] = p.levels;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const MultiplierParams& p) const
    {
        o["mode"] = p.mode;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const VideoDelayParams& p) const
    {
        o["useBeats"] = p.useBeats;
        o["delay"] = p.delay;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const MultiDelayParams& p) const
    {
        o["mode"] = p.mode;
        o["buffer"] = p.buffer;
        o["delay"] = p.delay;
        o["useBeats"] = p.useBeats;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
    }
    void operator()(const DebugBarsParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["orbitSpeed"] = p.orbitSpeed;
    }
    void operator()(const MilkdropNodeParams& p) const
    {
        // eingebettetes Milkdrop-Schwester-Dokument (MilkdropSerializer, M6.1):
        // Shader-/Skript-Texte bleiben SSOT, die Klassifikation wird beim
        // Laden neu abgeleitet. presetDir = Textur-Suchbasis.
        o["preset"] = lumi::milkdrop::presetToJson(p.preset);
        o["presetDir"] = QString::fromStdString(p.presetDir);
        o["meshX"] = p.meshX;
        o["meshY"] = p.meshY;
        o["debugGrid"] = p.debugGrid;

        // Bild-Einbettung (Entscheid Patrik S43): beim SPEICHERN genau die
        // aktuell referenzierten Bilder einbetten — Datei-Bytes bevorzugt
        // (Quelle bleiben die Asset-Ordner), sonst die vorhandene Einbettung
        // uebernehmen. Nicht mehr referenzierte Alt-Eintraege entfallen
        // automatisch. randNN-Sampler bleiben Ordner-Zufall.
        QJsonObject images;
        const QString dir = QString::fromStdString(p.presetDir);
        const auto embed = [&](const std::string& key, const QString& filePath) {
            const QString keyQ = QString::fromStdString(key);
            if (key.empty() || images.contains(keyQ)) return;
            if (!filePath.isEmpty())
            {
                QFile f(filePath);
                if (f.open(QIODevice::ReadOnly))
                {
                    images[keyQ] = QString::fromLatin1(f.readAll().toBase64());
                    return;
                }
            }
            const auto it = p.embeddedImages.find(key);
            if (it != p.embeddedImages.end())
                images[keyQ] = QString::fromStdString(it->second);
        };
        for (const auto& s : p.preset.sprites)
        {
            if (s.imageName.empty()) continue;
            embed(s.imageName, lumi::milkdrop::resolveSpriteFile(
                                   dir, QString::fromStdString(s.imageName)));
        }
        // Custom-Sampler aus den Shader-Texten (Deklarationszeilen)
        static const QRegularExpression kSamplerDecl(
            QStringLiteral("\\bsampler(?:2D|3D)?\\s+sampler_([A-Za-z0-9_]+)"));
        static const QRegularExpression kRand(QStringLiteral("^rand\\d\\d(_.+)?$"));
        const auto scanShader = [&](const std::string& text) {
            QRegularExpressionMatchIterator it =
                kSamplerDecl.globalMatch(QString::fromStdString(text));
            while (it.hasNext())
            {
                QString base = it.next().captured(1);
                for (const char* prefix : {"fc_", "pc_", "fw_", "pw_"})
                {
                    if (base.startsWith(QLatin1String(prefix)))
                    {
                        base = base.mid(3);
                        break;
                    }
                }
                if (base == QLatin1String("main") ||
                    base.startsWith(QLatin1String("blur")) ||
                    base.startsWith(QLatin1String("noise")) ||
                    kRand.match(base).hasMatch())
                {
                    continue;  // Builtins/Zufall — nicht einbetten
                }
                embed(base.toStdString(),
                      lumi::milkdrop::resolveTextureFile(dir, base));
            }
        };
        scanShader(p.preset.warpShaderText);
        scanShader(p.preset.compShaderText);
        if (!images.isEmpty()) o["images"] = images;
    }
    void operator()(const PassthroughParams& p) const
    {
        o["sourceId"] = p.sourceId;
        o["note"] = QString::fromStdString(p.note);
    }
};

// --- JSON -> param (by type key) ---------------------------------------------
EffectParams readParams(const QString& type, const QJsonObject& o)
{
    if (type == "list")
    {
        ListParams p;
        p.clearEveryFrame = getBool(o, "clearEveryFrame", p.clearEveryFrame);
        p.blendIn = static_cast<BlendMode>(getInt(o, "blendIn", static_cast<int>(p.blendIn)));
        p.blendOut = static_cast<BlendMode>(getInt(o, "blendOut", static_cast<int>(p.blendOut)));
        p.inAdjustAlpha = getInt(o, "inAdjustAlpha", p.inAdjustAlpha);
        p.outAdjustAlpha = getInt(o, "outAdjustAlpha", p.outAdjustAlpha);
        p.bufferIn = getInt(o, "bufferIn", p.bufferIn);
        p.bufferOut = getInt(o, "bufferOut", p.bufferOut);
        p.bufferInInvert = getBool(o, "bufferInInvert", p.bufferInInvert);
        p.bufferOutInvert = getBool(o, "bufferOutInvert", p.bufferOutInvert);
        p.onBeatRender = getBool(o, "onBeatRender", p.onBeatRender);
        p.onBeatFrames = getInt(o, "onBeatFrames", p.onBeatFrames);
        p.useCode = getBool(o, "useCode", p.useCode);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        return p;
    }
    if (type == "hostgroup")  // HG1 — .lvfx2-Kennzeichen
    {
        HostGroupParams p;
        p.blendOut = static_cast<BlendMode>(getInt(o, "blendOut", static_cast<int>(p.blendOut)));
        p.outAdjustAlpha = getInt(o, "outAdjustAlpha", p.outAdjustAlpha);
        p.crossfadeSeconds = o.value("crossfadeSeconds").toDouble(2.0);
        p.curveIn = getInt(o, "curveIn", p.curveIn);
        p.curveOut = getInt(o, "curveOut", p.curveOut);
        p.sourceFile = getStr(o, "sourceFile", p.sourceFile);
        return p;
    }
    if (type == "clear")
    {
        ClearParams p;
        p.color = getColor(o, "color", p.color);
        p.onlyFirst = getBool(o, "onlyFirst", p.onlyFirst);
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "fadeout")
    {
        FadeoutParams p;
        p.fadeLen = getInt(o, "fadeLen", p.fadeLen);
        p.color = getColor(o, "color", p.color);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "invert") return InvertParams{};
    if (type == "brightness")
    {
        BrightnessParams p;
        p.red = getInt(o, "red", p.red);
        p.green = getInt(o, "green", p.green);
        p.blue = getInt(o, "blue", p.blue);
        p.exclude = getBool(o, "exclude", p.exclude);
        p.color = getColor(o, "color", p.color);
        p.distance = getInt(o, "distance", p.distance);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "fastBrightness")
    {
        FastBrightnessParams p;
        p.dir = getInt(o, "dir", p.dir);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "blur")
    {
        BlurParams p;
        p.strength = getInt(o, "strength", p.strength);
        p.roundUp = getBool(o, "roundUp", p.roundUp);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "mirror")
    {
        MirrorParams p;
        // Legacy files carry two bools; current files the r_mirror bit mode.
        const int legacy = (getBool(o, "topToBottom", false) ? 1 : 0) |
                           (getBool(o, "leftToRight", true) ? 4 : 0);
        p.mode = getInt(o, "mode", p.mode) & 15;
        p.onBeatRandom = getBool(o, "onBeatRandom", p.onBeatRandom);
        p.smooth = getBool(o, "smooth", p.smooth);
        p.slower = getInt(o, "slower", p.slower);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "onBeatClear")
    {
        OnBeatClearParams p;
        p.color = getColor(o, "color", p.color);
        p.everyNBeats = getInt(o, "everyNBeats", p.everyNBeats);
        p.blend = getBool(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "colorfade")
    {
        ColorfadeParams p;
        p.faderR = getInt(o, "faderR", p.faderR);
        p.faderG = getInt(o, "faderG", p.faderG);
        p.faderB = getInt(o, "faderB", p.faderB);
        p.beatFaderR = getInt(o, "beatFaderR", p.beatFaderR);
        p.beatFaderG = getInt(o, "beatFaderG", p.beatFaderG);
        p.beatFaderB = getInt(o, "beatFaderB", p.beatFaderB);
        p.onBeatFrames = getInt(o, "onBeatFrames", p.onBeatFrames);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "colorModifier")
    {
        ColorModifierParams p;
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.levelCode = getStr(o, "levelCode", p.levelCode);
        p.recompute = getBool(o, "recompute", p.recompute);
        return p;
    }
    if (type == "movement")
    {
        MovementParams p;
        p.code = getStr(o, "code", p.code);
        p.rectCoords = getBool(o, "rectCoords", p.rectCoords);
        p.wrap = getBool(o, "wrap", p.wrap);
        p.blend = getBool(o, "blend", p.blend);
        p.subpixel = getBool(o, "subpixel", p.subpixel);
        p.sourceMapped = getInt(o, "sourceMapped", p.sourceMapped);
        p.builtinRemap = getInt(o, "builtinRemap", p.builtinRemap);
        return p;
    }
    if (type == "dynamicMovement")
    {
        DynamicMovementParams p;
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.pointCode = getStr(o, "pointCode", p.pointCode);
        p.xres = getInt(o, "xres", p.xres);
        p.yres = getInt(o, "yres", p.yres);
        p.rectCoords = getBool(o, "rectCoords", p.rectCoords);
        p.wrap = getBool(o, "wrap", p.wrap);
        p.blend = getBool(o, "blend", p.blend);
        p.nomove = getBool(o, "nomove", p.nomove);
        p.subpixel = getBool(o, "subpixel", p.subpixel);
        p.buffern = getInt(o, "buffern", p.buffern);
        return p;
    }
    if (type == "blitterFeedback")
    {
        BlitterFeedbackParams p;
        // Alt-Dokument? Das erkennt man am ALTEN Feld. Fehlt beides,
        // gelten die Vorgaben des Structs (S56).
        if (o.contains("scale") ||
            (!o.contains("zoom") && !o.contains("beatZoom")))
        {
            p.scale = getInt(o, "scale", p.scale);
            p.scale2 = getInt(o, "scale2", p.scale2);
        }
        else
        {
            // Alt-Dokumente (vor S48): zoom-Float war 1 + scale/1024.
            p.scale = static_cast<int>(
                std::lround((getDouble(o, "zoom", 1.03) - 1.0) * 1024.0));
            p.scale2 = static_cast<int>(
                std::lround((getDouble(o, "beatZoom", 0.9) - 1.0) * 1024.0));
        }
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.blend = getBool(o, "blend", p.blend);
        p.subpixel = getBool(o, "subpixel", p.subpixel);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "rotoBlitter")
    {
        RotoBlitterParams p;
        // s. Blitter Feedback: die Weiche haengt am ALTEN Feld. Ohne das
        // hier zoomte ein Preset ohne `zoomScale` auf 0 statt auf den
        // neutralen 31 — das Bild saettigte, und alle fuenf Felder des
        // Knotens galten als stumm (Befund S56).
        if (o.contains("zoomScale") ||
            (!o.contains("zoom") && !o.contains("rotationSpeed")))
        {
            p.zoomScale = getInt(o, "zoomScale", p.zoomScale);
            p.zoomScale2 = getInt(o, "zoomScale2", p.zoomScale2);
            p.rotDir = getInt(o, "rotDir", p.rotDir);
        }
        else
        {
            // Alt-Dokumente (vor S48): zoom = 1 + zoom_scale/1024,
            // rotationSpeed = rot_dir/32 (Grad/Frame ~ rotDir-32).
            p.zoomScale = static_cast<int>(
                std::lround((getDouble(o, "zoom", 1.0) - 1.0) * 1024.0));
            p.rotDir = 32 + static_cast<int>(
                                std::lround(getDouble(o, "rotationSpeed", 1.0)));
            p.zoomScale2 = p.zoomScale;
        }
        p.blend = getBool(o, "blend", p.blend);
        p.beatReverse = getBool(o, "beatReverse", p.beatReverse);
        p.beatReverseSpeed = getInt(o, "beatReverseSpeed", p.beatReverseSpeed);
        p.beatZoomJump = getBool(o, "beatZoomJump", p.beatZoomJump);
        p.subpixel = getBool(o, "subpixel", p.subpixel);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "bufferSave")
    {
        BufferSaveParams p;
        p.slot = getInt(o, "slot", p.slot);
        // Legacy files carry bool "save"; current files carry "dir" 0..3.
        p.dir = getInt(o, "dir", getBool(o, "save", true) ? 0 : 1);
        p.blend = static_cast<BlendMode>(getInt(o, "blend", static_cast<int>(p.blend)));
        p.adjustAlpha = getInt(o, "adjustAlpha", p.adjustAlpha);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "customBpm")
    {
        CustomBpmParams p;
        p.arbitrary = getBool(o, "arbitrary", p.arbitrary);
        p.arbitraryMs = getInt(o, "arbitraryMs", p.arbitraryMs);
        p.skip = getBool(o, "skip", p.skip);
        p.skipCount = getInt(o, "skipCount", p.skipCount);
        p.invert = getBool(o, "invert", p.invert);
        p.skipFirst = getInt(o, "skipFirst", p.skipFirst);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "superScope")
    {
        SuperScopeParams p;
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.pointCode = getStr(o, "pointCode", p.pointCode);
        p.pointCount = getInt(o, "pointCount", p.pointCount);
        p.renderMode = getInt(o, "renderMode", p.renderMode);
        p.lineWidth = static_cast<float>(getDouble(o, "lineWidth", p.lineWidth));
        p.dotSize = static_cast<float>(getDouble(o, "dotSize", p.dotSize));
        p.audioChannel = getInt(o, "audioChannel", p.audioChannel);
        p.spectrumSource = getBool(o, "spectrumSource", p.spectrumSource);
        p.lineBlend = getInt(o, "lineBlend", p.lineBlend);
        p.colorBlend = getInt(o, "colorBlend", p.colorBlend);
        p.colorCycleFrames = getInt(o, "colorCycleFrames", p.colorCycleFrames);
        p.gradientPreset = o.contains("gradientPreset") ? getStr(o, "gradientPreset")
                                                         : std::string("Neon");
        for (const QJsonValue& v : o.value("colors").toArray())
            p.colors.push_back(static_cast<uint32_t>(v.toDouble()));
        return p;
    }
    if (type == "mosaic")
    {
        MosaicParams p;
        p.quality = getInt(o, "quality", p.quality);
        p.quality2 = getInt(o, "quality2", p.quality2);
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.durationFrames = getInt(o, "durationFrames", p.durationFrames);
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "grain")
    {
        GrainParams p;
        p.amount = getInt(o, "amount", p.amount);
        p.staticGrain = getBool(o, "staticGrain", p.staticGrain);
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "scatter")
        return ScatterParams{};
    if (type == "water")
        return WaterParams{};
    if (type == "waterBump")
    {
        WaterBumpParams p;
        p.density = getInt(o, "density", p.density);
        p.depth = getInt(o, "depth", p.depth);
        p.randomDrop = getBool(o, "randomDrop", p.randomDrop);
        p.dropX = getInt(o, "dropX", p.dropX);
        p.dropY = getInt(o, "dropY", p.dropY);
        p.dropRadius = getInt(o, "dropRadius", p.dropRadius);
        p.displaceScale = static_cast<float>(getDouble(o, "displaceScale", p.displaceScale));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "dynamicShift")
    {
        DynamicShiftParams p;
        p.blend = getBool(o, "blend", p.blend);
        // `bilinear` ist der Altname (bis S54) — vorhandene .lvfx lesen
        // sich damit weiter; der Vorgabewert folgt dem Original.
        p.subpixel = getBool(o, "subpixel", getBool(o, "bilinear", true));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "dynamicDistanceModifier")
    {
        DynamicDistanceModifierParams p;
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.pixelCode = getStr(o, "pixelCode", p.pixelCode);
        p.blend = getBool(o, "blend", p.blend);
        // `bilinear` ist der Altname (bis S54) — vorhandene .lvfx lesen
        // sich damit weiter; der Vorgabewert folgt dem Original.
        p.subpixel = getBool(o, "subpixel", getBool(o, "bilinear", false));
        return p;
    }
    if (type == "movingParticle")
    {
        MovingParticleParams p;
        p.color = getColor(o, "color", p.color);
        p.maxDistance = getInt(o, "maxDistance", p.maxDistance);
        p.size = getInt(o, "size", p.size);
        p.size2 = getInt(o, "size2", p.size2);
        p.onBeatSize = getBool(o, "onBeatSize", p.onBeatSize);
        p.blend = getInt(o, "blend", p.blend);
        p.spring = static_cast<float>(getDouble(o, "spring", p.spring));
        p.damping = static_cast<float>(getDouble(o, "damping", p.damping));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "colorMap")
    {
        ColorMapParams p;
        p.key = getInt(o, "key", p.key);
        p.blendMode = getInt(o, "blendMode", p.blendMode);
        p.adjustBlend = getInt(o, "adjustBlend", p.adjustBlend);
        const QJsonArray pos = o.value("stopPos").toArray();
        const QJsonArray col = o.value("stopColor").toArray();
        for (const auto& v : pos) p.stopPos.push_back(v.toInt());
        for (const auto& v : col) p.stopColor.push_back(static_cast<uint32_t>(v.toDouble()));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "fractal2D")
    {
        Fractal2DParams p;
        p.type = getInt(o, "ftype", p.type);
        p.centerX = static_cast<float>(getDouble(o, "centerX", p.centerX));
        p.centerY = static_cast<float>(getDouble(o, "centerY", p.centerY));
        p.zoom = static_cast<float>(getDouble(o, "zoom", p.zoom));
        p.rotation = static_cast<float>(getDouble(o, "rotation", p.rotation));
        p.maxIter = getInt(o, "maxIter", p.maxIter);
        p.juliaX = static_cast<float>(getDouble(o, "juliaX", p.juliaX));
        p.juliaY = static_cast<float>(getDouble(o, "juliaY", p.juliaY));
        p.power = static_cast<float>(getDouble(o, "power", p.power));
        p.escapeR = static_cast<float>(getDouble(o, "escapeR", p.escapeR));
        p.smooth = getBool(o, "smooth", p.smooth);
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", p.colorScale));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", p.colorCycle));
        p.insideColor = getColor(o, "insideColor", p.insideColor);
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "domainWarp")
    {
        DomainWarpParams p;
        p.octaves = getInt(o, "octaves", p.octaves);
        p.lacunarity = static_cast<float>(getDouble(o, "lacunarity", p.lacunarity));
        p.gain = static_cast<float>(getDouble(o, "gain", p.gain));
        p.scale = static_cast<float>(getDouble(o, "scale", p.scale));
        p.warp = static_cast<float>(getDouble(o, "warp", p.warp));
        p.warpScale = static_cast<float>(getDouble(o, "warpScale", p.warpScale));
        p.speed = static_cast<float>(getDouble(o, "speed", p.speed));
        p.offsetX = static_cast<float>(getDouble(o, "offsetX", p.offsetX));
        p.offsetY = static_cast<float>(getDouble(o, "offsetY", p.offsetY));
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", p.colorScale));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", p.colorCycle));
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "setRenderMode")
    {
        SetRenderModeParams p;
        // Altbestand: bis S51 stand das Override-Flag unter "enabled" und hat
        // dabei den Knoten-Schalter ueberschrieben — als Rueckfall weiter lesen.
        p.enabled = getBool(o, "overrideBlend", getBool(o, "enabled", true));
        p.lineWidth = getInt(o, "lineWidth", p.lineWidth);
        p.lineBlend = getInt(o, "lineBlend", p.lineBlend);
        p.adjustAlpha = getInt(o, "adjustAlpha", p.adjustAlpha);
        return p;
    }
    if (type == "fractal3D")
    {
        Fractal3DParams p;
        p.type = getInt(o, "ftype", p.type);
        p.yaw = static_cast<float>(getDouble(o, "yaw", p.yaw));
        p.pitch = static_cast<float>(getDouble(o, "pitch", p.pitch));
        p.dist = static_cast<float>(getDouble(o, "dist", p.dist));
        p.fov = static_cast<float>(getDouble(o, "fov", p.fov));
        p.power = static_cast<float>(getDouble(o, "power", p.power));
        p.scale = static_cast<float>(getDouble(o, "scale", p.scale));
        p.fold = static_cast<float>(getDouble(o, "fold", p.fold));
        p.maxSteps = getInt(o, "maxSteps", p.maxSteps);
        p.maxIter = getInt(o, "maxIter", p.maxIter);
        p.juliaX = static_cast<float>(getDouble(o, "juliaX", p.juliaX));
        p.juliaY = static_cast<float>(getDouble(o, "juliaY", p.juliaY));
        p.juliaZ = static_cast<float>(getDouble(o, "juliaZ", p.juliaZ));
        p.juliaW = static_cast<float>(getDouble(o, "juliaW", p.juliaW));
        p.lightYaw = static_cast<float>(getDouble(o, "lightYaw", p.lightYaw));
        p.lightPitch = static_cast<float>(getDouble(o, "lightPitch", p.lightPitch));
        p.ambient = static_cast<float>(getDouble(o, "ambient", p.ambient));
        p.ao = getBool(o, "ao", p.ao);
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", p.colorScale));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", p.colorCycle));
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.background = getColor(o, "background", p.background);
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "lyapunov")
    {
        LyapunovParams p;
        p.sequence = getStr(o, "sequence", p.sequence);
        if (p.sequence.empty()) p.sequence = "AB";
        p.aMin = static_cast<float>(getDouble(o, "aMin", p.aMin));
        p.aMax = static_cast<float>(getDouble(o, "aMax", p.aMax));
        p.bMin = static_cast<float>(getDouble(o, "bMin", p.bMin));
        p.bMax = static_cast<float>(getDouble(o, "bMax", p.bMax));
        p.warmup = getInt(o, "warmup", p.warmup);
        p.iterations = getInt(o, "iterations", p.iterations);
        p.negColor = getColor(o, "negColor", p.negColor);
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", p.colorScale));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", p.colorCycle));
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Fire";
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "kleinian")
    {
        KleinianParams p;
        p.p = getInt(o, "p", p.p);
        p.q = getInt(o, "q", p.q);
        p.iterations = getInt(o, "iterations", p.iterations);
        p.morph = static_cast<float>(getDouble(o, "morph", p.morph));
        p.zoom = static_cast<float>(getDouble(o, "zoom", p.zoom));
        p.rotation = static_cast<float>(getDouble(o, "rotation", p.rotation));
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", p.colorScale));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", p.colorCycle));
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "fractalZoomer")
    {
        FractalZoomerParams p;
        p.type = getInt(o, "ftype", p.type);
        p.centerX = static_cast<float>(getDouble(o, "centerX", p.centerX));
        p.centerY = static_cast<float>(getDouble(o, "centerY", p.centerY));
        p.juliaX = static_cast<float>(getDouble(o, "juliaX", p.juliaX));
        p.juliaY = static_cast<float>(getDouble(o, "juliaY", p.juliaY));
        p.maxIter = getInt(o, "maxIter", p.maxIter);
        p.zoomSpeed = static_cast<float>(getDouble(o, "zoomSpeed", p.zoomSpeed));
        p.rotationSpeed = static_cast<float>(getDouble(o, "rotationSpeed", p.rotationSpeed));
        p.feedback = static_cast<float>(getDouble(o, "feedback", p.feedback));
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", p.colorScale));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", p.colorCycle));
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.insideColor = getColor(o, "insideColor", p.insideColor);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "strangeAttractor")
    {
        StrangeAttractorParams p;
        p.type = getInt(o, "ftype", p.type);
        p.a = static_cast<float>(getDouble(o, "a", p.a));
        p.b = static_cast<float>(getDouble(o, "b", p.b));
        p.c = static_cast<float>(getDouble(o, "c", p.c));
        p.d = static_cast<float>(getDouble(o, "d", p.d));
        p.points = getInt(o, "points", p.points);
        p.scale = static_cast<float>(getDouble(o, "scale", p.scale));
        p.rotation = static_cast<float>(getDouble(o, "rotation", p.rotation));
        p.rotationSpeed = static_cast<float>(getDouble(o, "rotationSpeed", p.rotationSpeed));
        p.color = getColor(o, "color", p.color);
        p.useGradient = getBool(o, "useGradient", p.useGradient);
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.dotSize = static_cast<float>(getDouble(o, "dotSize", p.dotSize));
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "flame")
    {
        FlameParams p;
        p.variation = getInt(o, "variation", p.variation);
        p.functions = getInt(o, "functions", p.functions);
        p.points = getInt(o, "points", p.points);
        p.scale = static_cast<float>(getDouble(o, "scale", p.scale));
        p.rotation = static_cast<float>(getDouble(o, "rotation", p.rotation));
        p.rotationSpeed = static_cast<float>(getDouble(o, "rotationSpeed", p.rotationSpeed));
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Fire";
        p.dotSize = static_cast<float>(getDouble(o, "dotSize", p.dotSize));
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "reactionDiffusion")
    {
        ReactionDiffusionParams p;
        p.feed = static_cast<float>(getDouble(o, "feed", p.feed));
        p.kill = static_cast<float>(getDouble(o, "kill", p.kill));
        p.diffA = static_cast<float>(getDouble(o, "diffA", p.diffA));
        p.diffB = static_cast<float>(getDouble(o, "diffB", p.diffB));
        p.stepsPerFrame = getInt(o, "stepsPerFrame", p.stepsPerFrame);
        p.seedOnBeat = getBool(o, "seedOnBeat", p.seedOnBeat);
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", p.colorScale));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", p.colorCycle));
        p.gradientPreset = getStr(o, "gradientPreset", p.gradientPreset);
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "bufferBlend")
    {
        BufferBlendParams p;
        p.bufferA = getInt(o, "bufferA", p.bufferA);
        p.bufferB = getInt(o, "bufferB", p.bufferB);
        p.mode = getInt(o, "mode", p.mode);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "jherikoGlobal")
    {
        JherikoGlobalParams p;
        p.loadMode = getInt(o, "loadMode", p.loadMode);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "simpleScope")
    {
        SimpleScopeParams p;
        // s. Blitter Feedback: die Weiche haengt am ALTEN Feld.
        if (o.contains("mode") ||
            (!o.contains("source") && !o.contains("drawMode")))
        {
            p.mode = std::clamp(getInt(o, "mode", p.mode), 0, 5);
        }
        else
        {
            // Alt-Dokumente (vor S48): source (0 Spektrum / 1 Waveform) +
            // drawMode (0 Linien / 1 Punkte) — solid gab es noch nicht.
            const int source = getInt(o, "source", 1);
            const int drawMode = getInt(o, "drawMode", 0);
            if (drawMode == 1) p.mode = source == 1 ? 5 : 4;
            else p.mode = source == 1 ? 2 : 1;
        }
        p.channel = getInt(o, "channel", p.channel);
        p.position = getInt(o, "position", p.position);
        p.colors.clear();
        const QJsonArray cols = o.value("colors").toArray();
        for (const auto& v : cols) p.colors.push_back(static_cast<uint32_t>(v.toDouble()));
        if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "bassSpin")
    {
        BassSpinParams p;
        p.left = getBool(o, "left", p.left);
        p.right = getBool(o, "right", p.right);
        p.colorLeft = getColor(o, "colorLeft", p.colorLeft);
        p.colorRight = getColor(o, "colorRight", p.colorRight);
        p.mode = getInt(o, "mode", p.mode);
        p.smoothing = static_cast<float>(getDouble(o, "smoothing", p.smoothing));
        // Vorgabe = der Original-Ausdruck `3.14159f/6.0f`, nicht pi/6 (s. Struct).
        p.spinStep = static_cast<float>(
            getDouble(o, "spinStep", static_cast<double>(p.spinStep)));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "oscStar" || type == "oscRing")
    {
        const QJsonArray cols = o.value("colors").toArray();
        std::vector<uint32_t> colors;
        for (const auto& v : cols) colors.push_back(static_cast<uint32_t>(v.toDouble()));
        if (colors.empty()) colors.push_back(0xFFFFFF);
        if (type == "oscStar")
        {
            OscStarParams p;
            p.channel = getInt(o, "channel", p.channel);
            p.position = getInt(o, "position", p.position);
            p.size = getInt(o, "size", p.size);
            p.rot = getInt(o, "rot", p.rot);
            // S53 freigemacht — Vorgabe = das bisherige feste Verhalten, ein
            // alteres Preset ohne diese Schluessel bleibt also unveraendert.
            p.spokes = getInt(o, "spokes", p.spokes);
            p.rotScale = static_cast<float>(getDouble(o, "rotScale", p.rotScale));
            p.amplitude = static_cast<float>(getDouble(o, "amplitude", p.amplitude));
            p.initCode = getStr(o, "initCode", p.initCode);
            p.frameCode = getStr(o, "frameCode", p.frameCode);
            p.beatCode = getStr(o, "beatCode", p.beatCode);
            p.colors = std::move(colors);
            return p;
        }
        OscRingParams p;
        p.source = getInt(o, "source", p.source);
        p.channel = getInt(o, "channel", p.channel);
        p.position = getInt(o, "position", p.position);
        p.size = getInt(o, "size", p.size);
        p.segments = getInt(o, "segments", p.segments);
        p.baseScale = static_cast<float>(getDouble(o, "baseScale", p.baseScale));
        p.audioScale = static_cast<float>(getDouble(o, "audioScale", p.audioScale));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.colors = std::move(colors);
        return p;
    }
    if (type == "rotatingStars")
    {
        RotatingStarsParams p;
        p.colors.clear();
        const QJsonArray cols = o.value("colors").toArray();
        for (const auto& v : cols) p.colors.push_back(static_cast<uint32_t>(v.toDouble()));
        if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
        p.points = getInt(o, "points", p.points);
        p.skip = getInt(o, "skip", p.skip);
        p.stars = getInt(o, "stars", p.stars);
        p.rotSpeed = static_cast<float>(getDouble(o, "rotSpeed", p.rotSpeed));
        p.orbit = static_cast<float>(getDouble(o, "orbit", p.orbit));
        p.baseRadius = static_cast<float>(getDouble(o, "baseRadius", p.baseRadius));
        p.audioGain = static_cast<float>(getDouble(o, "audioGain", p.audioGain));
        p.bandLo = getInt(o, "bandLo", p.bandLo);
        p.bandHi = getInt(o, "bandHi", p.bandHi);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "picture")
    {
        PictureParams p;
        p.filename = getStr(o, "filename", p.filename);
        p.imageData = getStr(o, "imageData", p.imageData);
        p.blend = getInt(o, "blend", p.blend);
        p.keepAspect = getBool(o, "keepAspect", p.keepAspect);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "pictureII")
    {
        PictureIIParams p;
        p.filename = getStr(o, "filename", p.filename);
        p.imageData = getStr(o, "imageData", p.imageData);
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "texer")
    {
        TexerParams p;
        p.filename = getStr(o, "filename", p.filename);
        p.imageData = getStr(o, "imageData", p.imageData);
        p.blend = getInt(o, "blend", p.blend);
        p.particles = getInt(o, "particles", p.particles);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "texerII")
    {
        TexerIIParams p;
        p.filename = getStr(o, "filename", p.filename);
        p.imageData = getStr(o, "imageData", p.imageData);
        p.resizing = getBool(o, "resizing", p.resizing);
        p.wrapAround = getBool(o, "wrapAround", p.wrapAround);
        p.colorFiltering = getBool(o, "colorFiltering", p.colorFiltering);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.pointCode = getStr(o, "pointCode", p.pointCode);
        return p;
    }
    if (type == "triangle")
    {
        TriangleParams p;
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.pointCode = getStr(o, "pointCode", p.pointCode);
        // Vorgabe GEFUELLT = Referenzverhalten (S51); alte Dateien ohne den
        // Schluessel bleiben damit unveraendert.
        p.filled = getBool(o, "filled", p.filled);
        p.lineWidth = static_cast<float>(getDouble(o, "lineWidth", p.lineWidth));
        return p;
    }
    if (type == "convolution")
    {
        ConvolutionParams p;
        p.edgeMode = getInt(o, "edgeMode", p.edgeMode);
        p.absolute = getBool(o, "absolute", p.absolute);
        p.twoPass = getBool(o, "twoPass", p.twoPass);
        p.bias = getInt(o, "bias", p.bias);
        p.scale = getInt(o, "scale", p.scale);
        const QJsonArray k = o.value("kernel").toArray();
        for (int i = 0; i < 49 && i < k.size(); ++i)
            p.kernel[static_cast<std::size_t>(i)] = k[i].toInt();
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "normalise") return NormaliseParams{};
    if (type == "multiFilter")
    {
        MultiFilterParams p;
        p.effect = getInt(o, "effect", p.effect);
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "addBorders")
    {
        AddBordersParams p;
        p.color = getColor(o, "color", p.color);
        p.size = getInt(o, "size", p.size);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "colorClip")
    {
        ColorClipParams p;
        p.mode = getInt(o, "mode", p.mode);
        p.clipColor = getColor(o, "clipColor", p.clipColor);
        p.outColor = getColor(o, "outColor", p.outColor);
        p.distance = getInt(o, "distance", p.distance);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "uniqueTone")
    {
        UniqueToneParams p;
        p.color = getColor(o, "color", p.color);
        p.invert = getBool(o, "invert", p.invert);
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "interleave")
    {
        InterleaveParams p;
        p.x = getInt(o, "x", p.x);
        p.y = getInt(o, "y", p.y);
        p.color = getColor(o, "color", p.color);
        p.blend = getInt(o, "blend", p.blend);
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.x2 = getInt(o, "x2", p.x2);
        p.y2 = getInt(o, "y2", p.y2);
        p.beatDuration = getInt(o, "beatDuration", p.beatDuration);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "bump")
    {
        BumpParams p;
        p.depth = getInt(o, "depth", p.depth);
        p.depth2 = getInt(o, "depth2", p.depth2);
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.durationFrames = getInt(o, "durationFrames", p.durationFrames);
        p.invert = getBool(o, "invert", p.invert);
        p.oldStyle = getBool(o, "oldStyle", p.oldStyle);
        p.blend = getInt(o, "blend", p.blend);
        p.buffern = getInt(o, "buffern", p.buffern);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "interferences")
    {
        InterferencesParams p;
        p.points = getInt(o, "points", p.points);
        p.distance = getInt(o, "distance", p.distance);
        p.alpha = getInt(o, "alpha", p.alpha);
        p.rotation = getInt(o, "rotation", p.rotation);
        p.rotationInc = getInt(o, "rotationInc", p.rotationInc);
        p.distance2 = getInt(o, "distance2", p.distance2);
        p.alpha2 = getInt(o, "alpha2", p.alpha2);
        p.rotationInc2 = getInt(o, "rotationInc2", p.rotationInc2);
        p.rgb = getBool(o, "rgb", p.rgb);
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.speed = static_cast<float>(getDouble(o, "speed", p.speed));
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "metaballs3d" || type == "tentacles3d")
    {
        std::vector<uint32_t> colors;
        for (const auto& v : o.value("colors").toArray())
            colors.push_back(static_cast<uint32_t>(v.toDouble()));
        if (colors.empty()) colors.push_back(0xFFFFFF);
        if (type == "metaballs3d")
        {
            Metaballs3DParams p;
            p.colors = colors;
            p.count = std::clamp(getInt(o, "count", 7), 1, 16);
            p.radius = static_cast<float>(getDouble(o, "radius", p.radius));
            p.speed = static_cast<float>(getDouble(o, "speed", p.speed));
            p.threshold = static_cast<float>(getDouble(o, "threshold", p.threshold));
            p.blend = std::clamp(getInt(o, "blend", 0), 0, 2);
            p.spread = static_cast<float>(getDouble(o, "spread", p.spread));
            p.depth = static_cast<float>(getDouble(o, "depth", p.depth));
            p.phase = static_cast<float>(getDouble(o, "phase", p.phase));
            p.initCode = getStr(o, "initCode", p.initCode);
            p.frameCode = getStr(o, "frameCode", p.frameCode);
            p.beatCode = getStr(o, "beatCode", p.beatCode);
            return p;
        }
        Tentacles3DParams p;
        p.colors = colors;
        p.count = std::clamp(getInt(o, "count", 7), 1, 16);
        p.segments = std::clamp(getInt(o, "segments", 28), 2, 256);
        p.length = static_cast<float>(getDouble(o, "length", p.length));
        p.thickness = static_cast<float>(getDouble(o, "thickness", p.thickness));
        p.speed = static_cast<float>(getDouble(o, "speed", p.speed));
        p.blend = std::clamp(getInt(o, "blend", 1), 0, 2);
        p.sway = static_cast<float>(getDouble(o, "sway", p.sway));
        p.waves = static_cast<float>(getDouble(o, "waves", p.waves));
        p.taper = static_cast<float>(getDouble(o, "taper", p.taper));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "fyrewurx")
    {
        FyrewurXParams p;
        p.sparks = getInt(o, "sparks", p.sparks);
        p.speed = static_cast<float>(getDouble(o, "speed", p.speed));
        p.gravity = static_cast<float>(getDouble(o, "gravity", p.gravity));
        p.lifeSeconds = static_cast<float>(getDouble(o, "lifeSeconds", p.lifeSeconds));
        p.dotSize = static_cast<float>(getDouble(o, "dotSize", p.dotSize));
        p.hueDrift = static_cast<float>(getDouble(o, "hueDrift", p.hueDrift));
        p.burstSpread = static_cast<float>(getDouble(o, "burstSpread", p.burstSpread));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "text")
    {
        TextParams p;
        p.text = getStr(o, "text", p.text);
        p.fontFace = getStr(o, "fontFace", p.fontFace);
        p.fontHeight = getInt(o, "fontHeight", p.fontHeight);
        p.fontWeight = getInt(o, "fontWeight", p.fontWeight);
        p.italic = getBool(o, "italic", p.italic);
        p.underline = getBool(o, "underline", p.underline);
        p.color = getColor(o, "color", p.color);
        p.blend = getInt(o, "blend", p.blend);
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.onBeatSpeed = getInt(o, "onBeatSpeed", p.onBeatSpeed);
        p.normSpeed = getInt(o, "normSpeed", p.normSpeed);
        p.insertBlank = getBool(o, "insertBlank", p.insertBlank);
        p.randomPos = getBool(o, "randomPos", p.randomPos);
        p.randomWord = getBool(o, "randomWord", p.randomWord);
        p.hAlign = getInt(o, "hAlign", p.hAlign);
        p.vAlign = getInt(o, "vAlign", p.vAlign);
        p.xShift = getInt(o, "xShift", p.xShift);
        p.yShift = getInt(o, "yShift", p.yShift);
        p.outline = getBool(o, "outline", p.outline);
        p.outlineColor = getColor(o, "outlineColor", p.outlineColor);
        p.outlineSize = getInt(o, "outlineSize", p.outlineSize);
        p.shadow = getBool(o, "shadow", p.shadow);
        return p;
    }
    if (type == "avi")
    {
        AviParams p;
        p.filename = getStr(o, "filename", p.filename);
        p.resolvedPath = getStr(o, "resolvedPath", p.resolvedPath);
        p.blend = getInt(o, "blend", p.blend);
        p.adapt = getBool(o, "adapt", p.adapt);
        p.persist = getInt(o, "persist", p.persist);
        p.speedMs = getInt(o, "speedMs", p.speedMs);
        return p;
    }
    if (type == "comment")
    {
        return CommentParams{getStr(o, "text")};
    }
    if (type == "importNotes")
    {
        return ImportNotesParams{getStr(o, "text")};
    }
    if (type == "renderScale")
    {
        RenderScaleParams p;
        p.divisor = std::clamp(getInt(o, "divisor", 2), 1, 8);
        p.filter = std::clamp(getInt(o, "filter", 0), 0, 1);
        return p;
    }
    if (type == "bloom")
    {
        BloomParams p;
        p.downsample = std::clamp(getInt(o, "downsample", 2), 0, 4);
        p.radius = std::clamp(getInt(o, "radius", 8), 1, 32);
        p.intensity = static_cast<float>(
            std::clamp(getDouble(o, "intensity", 1.0), 0.0, 8.0));
        p.threshold = static_cast<float>(
            std::clamp(getDouble(o, "threshold", 0.0), 0.0, 1.0));
        p.vignette = getBool(o, "vignette", p.vignette);
        p.vignetteStrength = static_cast<float>(
            std::clamp(getDouble(o, "vignetteStrength", 0.3), 0.0, 1.0));
        p.post = getBool(o, "post", p.post);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "camera3d")
    {
        Camera3DParams p;
        p.px = static_cast<float>(getDouble(o, "px", p.px));
        p.py = static_cast<float>(getDouble(o, "py", p.py));
        p.pz = static_cast<float>(getDouble(o, "pz", p.pz));
        p.tx = static_cast<float>(getDouble(o, "tx", p.tx));
        p.ty = static_cast<float>(getDouble(o, "ty", p.ty));
        p.tz = static_cast<float>(getDouble(o, "tz", p.tz));
        p.fov = static_cast<float>(std::clamp(getDouble(o, "fov", 30.0), 1.0, 179.0));
        p.roll = static_cast<float>(getDouble(o, "roll", p.roll));
        p.fogStart = static_cast<float>(getDouble(o, "fogStart", p.fogStart));
        p.fogEnd = static_cast<float>(getDouble(o, "fogEnd", p.fogEnd));
        p.fogColor = getColor(o, "fogColor", p.fogColor);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "superScope3d")
    {
        SuperScope3DParams p;
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.pointCode = getStr(o, "pointCode", p.pointCode);
        p.pointCount = std::clamp(getInt(o, "pointCount", 256), 1, 4096);
        p.renderMode = std::clamp(getInt(o, "renderMode", 0), 0, 1);
        p.size = static_cast<float>(
            std::clamp(getDouble(o, "size", 0.05), 0.0001, 100.0));
        p.falloff = static_cast<float>(
            std::clamp(getDouble(o, "falloff", 4.0), 0.5, 32.0));
        p.audioChannel = std::clamp(getInt(o, "audioChannel", 2), 0, 2);
        p.spectrumSource = getBool(o, "spectrumSource", p.spectrumSource);
        return p;
    }
    if (type == "terrain3d")
    {
        Terrain3DParams p;
        p.resolution = std::clamp(getInt(o, "resolution", 64), 8, 128);
        p.extent = static_cast<float>(
            std::clamp(getDouble(o, "extent", 4.0), 0.1, 100.0));
        p.baseAmp = static_cast<float>(
            std::clamp(getDouble(o, "baseAmp", 0.15), 0.0, 10.0));
        p.yOffset = static_cast<float>(
            std::clamp(getDouble(o, "yOffset", -0.8), -100.0, 100.0));
        p.ringAmp = static_cast<float>(
            std::clamp(getDouble(o, "ringAmp", 1.0), 0.0, 10.0));
        p.relax = static_cast<float>(
            std::clamp(getDouble(o, "relax", 0.12), 0.0, 1.0));
        p.flatten = static_cast<float>(
            std::clamp(getDouble(o, "flatten", 0.0), 0.0, 1.0));
        p.drawMesh = getBool(o, "drawMesh", p.drawMesh);
        p.meshColor = getColor(o, "meshColor", p.meshColor);
        p.drawDots = getBool(o, "drawDots", p.drawDots);
        p.dotSize = static_cast<float>(
            std::clamp(getDouble(o, "dotSize", 0.045), 0.0001, 10.0));
        p.falloff = static_cast<float>(
            std::clamp(getDouble(o, "falloff", 4.0), 0.5, 32.0));
        p.colorLow = getColor(o, "colorLow", p.colorLow);
        p.colorHigh = getColor(o, "colorHigh", p.colorHigh);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.pointCode = getStr(o, "pointCode", p.pointCode);
        return p;
    }
    if (type == "glowOrbs")
    {
        GlowOrbsParams p;
        p.orbCount = std::clamp(getInt(o, "orbCount", 5), 1, 64);
        p.haloScale = static_cast<float>(
            std::clamp(getDouble(o, "haloScale", 2.2), 1.0, 10.0));
        p.haloIntensity = static_cast<float>(
            std::clamp(getDouble(o, "haloIntensity", 0.6), 0.0, 4.0));
        p.falloff = static_cast<float>(
            std::clamp(getDouble(o, "falloff", 3.0), 0.5, 32.0));
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        p.pointCode = getStr(o, "pointCode", p.pointCode);
        return p;
    }
    if (type == "starfield")
    {
        StarfieldParams p;
        p.color = getColor(o, "color", p.color);
        p.warpSpeed = static_cast<float>(getDouble(o, "warpSpeed", p.warpSpeed));
        p.maxStars = getInt(o, "maxStars", p.maxStars);
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.beatSpeed = static_cast<float>(getDouble(o, "beatSpeed", p.beatSpeed));
        p.durationFrames = getInt(o, "durationFrames", p.durationFrames);
        p.blend = getInt(o, "blend", 1);  // legacy files rendered additively
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "timescope")
    {
        TimescopeParams p;
        p.color = getColor(o, "color", p.color);
        p.blend = getInt(o, "blend", p.blend);
        p.channel = getInt(o, "channel", p.channel);
        // Vorgabe false: ein importiertes Preset traegt `which_ch`, das im
        // Original nichts bewirkt — es soll auch bei uns nichts bewirken.
        p.useChannel = getBool(o, "useChannel", p.useChannel);
        p.bands = getInt(o, "bands", p.bands);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "dotGrid")
    {
        DotGridParams p;
        p.colors.clear();
        for (const QJsonValue& v : o.value("colors").toArray())
            p.colors.push_back(static_cast<uint32_t>(v.toDouble()));
        if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
        p.spacing = getInt(o, "spacing", p.spacing);
        p.xMove = getInt(o, "xMove", p.xMove);
        p.yMove = getInt(o, "yMove", p.yMove);
        p.blend = getInt(o, "blend", p.blend);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "dotPlane" || type == "dotFountain")
    {
        const QJsonArray cols = o.value("colors").toArray();
        auto fill = [&](uint32_t (&dst)[5]) {
            for (int i = 0; i < 5 && i < cols.size(); ++i)
                dst[i] = static_cast<uint32_t>(cols[i].toDouble());
        };
        if (type == "dotPlane")
        {
            DotPlaneParams p;
            fill(p.colors);
            p.rotVel = getInt(o, "rotVel", p.rotVel);
            p.angle = getInt(o, "angle", p.angle);
            p.camDistance = static_cast<float>(getDouble(o, "camDistance", p.camDistance));
            p.settle = static_cast<float>(getDouble(o, "settle", p.settle));
            p.initCode = getStr(o, "initCode", p.initCode);
            p.frameCode = getStr(o, "frameCode", p.frameCode);
            p.beatCode = getStr(o, "beatCode", p.beatCode);
            return p;
        }
        DotFountainParams p;
        fill(p.colors);
        p.rotVel = getInt(o, "rotVel", p.rotVel);
        p.angle = getInt(o, "angle", p.angle);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "channelShift")
    {
        ChannelShiftParams p;
        p.mode = getInt(o, "mode", p.mode);
        p.onBeat = getBool(o, "onBeat", p.onBeat);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "colorReduction")
    {
        ColorReductionParams p;
        p.levels = getInt(o, "levels", p.levels);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "multiplier")
    {
        MultiplierParams p;
        p.mode = getInt(o, "mode", p.mode);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "videoDelay")
    {
        VideoDelayParams p;
        p.useBeats = getBool(o, "useBeats", p.useBeats);
        p.delay = getInt(o, "delay", p.delay);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "multiDelay")
    {
        MultiDelayParams p;
        p.mode = getInt(o, "mode", p.mode);
        p.buffer = getInt(o, "buffer", p.buffer);
        p.delay = getInt(o, "delay", p.delay);
        p.useBeats = getBool(o, "useBeats", p.useBeats);
        p.initCode = getStr(o, "initCode", p.initCode);
        p.frameCode = getStr(o, "frameCode", p.frameCode);
        p.beatCode = getStr(o, "beatCode", p.beatCode);
        return p;
    }
    if (type == "debugBars")
        return DebugBarsParams{getColor(o, "color", 0xFF80FF),
                               static_cast<float>(getDouble(o, "orbitSpeed", 1.0))};
    if (type == "milkdrop")
    {
        MilkdropNodeParams p;
        p.preset = lumi::milkdrop::presetFromJson(o.value("preset").toObject(), nullptr);
        p.presetDir = getStr(o, "presetDir", p.presetDir);
        p.meshX = getInt(o, "meshX", p.meshX);
        p.meshY = getInt(o, "meshY", p.meshY);
        p.debugGrid = getBool(o, "debugGrid", p.debugGrid);
        // Bild-Einbettung (S43) — fehlt der Block, bleibt die Map leer
        const QJsonObject images = o.value("images").toObject();
        for (auto it = images.begin(); it != images.end(); ++it)
        {
            p.embeddedImages[it.key().toStdString()] =
                it.value().toString().toStdString();
        }
        p.revision = 1;  // frisch geladen = erste Revision fuer den Render-Host
        return p;
    }
    // "passthrough" and any unknown key
    return PassthroughParams{getInt(o, "sourceId", 0), getStr(o, "note")};
}

} // namespace

QString effectTypeKey(const EffectParams& params)
{
    struct Visitor
    {
        QString operator()(const ListParams&) const { return "list"; }
        QString operator()(const HostGroupParams&) const { return "hostgroup"; }
        QString operator()(const ClearParams&) const { return "clear"; }
        QString operator()(const FadeoutParams&) const { return "fadeout"; }
        QString operator()(const InvertParams&) const { return "invert"; }
        QString operator()(const BrightnessParams&) const { return "brightness"; }
        QString operator()(const FastBrightnessParams&) const { return "fastBrightness"; }
        QString operator()(const BlurParams&) const { return "blur"; }
        QString operator()(const MirrorParams&) const { return "mirror"; }
        QString operator()(const OnBeatClearParams&) const { return "onBeatClear"; }
        QString operator()(const ColorfadeParams&) const { return "colorfade"; }
        QString operator()(const ColorModifierParams&) const { return "colorModifier"; }
        QString operator()(const MovementParams&) const { return "movement"; }
        QString operator()(const DynamicMovementParams&) const { return "dynamicMovement"; }
        QString operator()(const BlitterFeedbackParams&) const { return "blitterFeedback"; }
        QString operator()(const RotoBlitterParams&) const { return "rotoBlitter"; }
        QString operator()(const BufferSaveParams&) const { return "bufferSave"; }
        QString operator()(const CustomBpmParams&) const { return "customBpm"; }
        QString operator()(const SuperScopeParams&) const { return "superScope"; }
        QString operator()(const MosaicParams&) const { return "mosaic"; }
        QString operator()(const GrainParams&) const { return "grain"; }
        QString operator()(const ScatterParams&) const { return "scatter"; }
        QString operator()(const WaterParams&) const { return "water"; }
        QString operator()(const DynamicShiftParams&) const { return "dynamicShift"; }
        QString operator()(const DynamicDistanceModifierParams&) const { return "dynamicDistanceModifier"; }
        QString operator()(const MovingParticleParams&) const { return "movingParticle"; }
        QString operator()(const ColorMapParams&) const { return "colorMap"; }
        QString operator()(const BufferBlendParams&) const { return "bufferBlend"; }
        QString operator()(const JherikoGlobalParams&) const { return "jherikoGlobal"; }
        QString operator()(const SimpleScopeParams&) const { return "simpleScope"; }
        QString operator()(const BassSpinParams&) const { return "bassSpin"; }
        QString operator()(const OscStarParams&) const { return "oscStar"; }
        QString operator()(const OscRingParams&) const { return "oscRing"; }
        QString operator()(const RotatingStarsParams&) const { return "rotatingStars"; }
        QString operator()(const PictureParams&) const { return "picture"; }
        QString operator()(const PictureIIParams&) const { return "pictureII"; }
        QString operator()(const TexerParams&) const { return "texer"; }
        QString operator()(const TexerIIParams&) const { return "texerII"; }
        QString operator()(const TriangleParams&) const { return "triangle"; }
        QString operator()(const ConvolutionParams&) const { return "convolution"; }
        QString operator()(const NormaliseParams&) const { return "normalise"; }
        QString operator()(const MultiFilterParams&) const { return "multiFilter"; }
        QString operator()(const AddBordersParams&) const { return "addBorders"; }
        QString operator()(const ColorClipParams&) const { return "colorClip"; }
        QString operator()(const UniqueToneParams&) const { return "uniqueTone"; }
        QString operator()(const InterleaveParams&) const { return "interleave"; }
        QString operator()(const BumpParams&) const { return "bump"; }
        QString operator()(const WaterBumpParams&) const { return "waterBump"; }
        QString operator()(const InterferencesParams&) const { return "interferences"; }
        QString operator()(const FyrewurXParams&) const { return "fyrewurx"; }
        QString operator()(const Metaballs3DParams&) const { return "metaballs3d"; }
        QString operator()(const Tentacles3DParams&) const { return "tentacles3d"; }
        QString operator()(const TextParams&) const { return "text"; }
        QString operator()(const AviParams&) const { return "avi"; }
        QString operator()(const CommentParams&) const { return "comment"; }
        QString operator()(const ImportNotesParams&) const { return "importNotes"; }
        QString operator()(const RenderScaleParams&) const { return "renderScale"; }
        QString operator()(const BloomParams&) const { return "bloom"; }
        QString operator()(const Camera3DParams&) const { return "camera3d"; }
        QString operator()(const SuperScope3DParams&) const { return "superScope3d"; }
        QString operator()(const Terrain3DParams&) const { return "terrain3d"; }
        QString operator()(const GlowOrbsParams&) const { return "glowOrbs"; }
        QString operator()(const StarfieldParams&) const { return "starfield"; }
        QString operator()(const TimescopeParams&) const { return "timescope"; }
        QString operator()(const DotGridParams&) const { return "dotGrid"; }
        QString operator()(const DotPlaneParams&) const { return "dotPlane"; }
        QString operator()(const DotFountainParams&) const { return "dotFountain"; }
        QString operator()(const ChannelShiftParams&) const { return "channelShift"; }
        QString operator()(const ColorReductionParams&) const { return "colorReduction"; }
        QString operator()(const MultiplierParams&) const { return "multiplier"; }
        QString operator()(const VideoDelayParams&) const { return "videoDelay"; }
        QString operator()(const MultiDelayParams&) const { return "multiDelay"; }
        QString operator()(const Fractal2DParams&) const { return "fractal2D"; }
        QString operator()(const DomainWarpParams&) const { return "domainWarp"; }
        QString operator()(const Fractal3DParams&) const { return "fractal3D"; }
        QString operator()(const LyapunovParams&) const { return "lyapunov"; }
        QString operator()(const KleinianParams&) const { return "kleinian"; }
        QString operator()(const FractalZoomerParams&) const { return "fractalZoomer"; }
        QString operator()(const StrangeAttractorParams&) const { return "strangeAttractor"; }
        QString operator()(const FlameParams&) const { return "flame"; }
        QString operator()(const ReactionDiffusionParams&) const { return "reactionDiffusion"; }
        QString operator()(const SetRenderModeParams&) const { return "setRenderMode"; }
        QString operator()(const DebugBarsParams&) const { return "debugBars"; }
        QString operator()(const MilkdropNodeParams&) const { return "milkdrop"; }
        QString operator()(const PassthroughParams&) const { return "passthrough"; }
    };
    return std::visit(Visitor{}, params);
}

QJsonObject nodeToJson(const ChainNode& node)
{
    QJsonObject o;
    o["type"] = effectTypeKey(node.params);
    o["enabled"] = node.enabled;
    o["name"] = QString::fromStdString(node.displayName);
    if (!node.description.empty())
        o["description"] = QString::fromStdString(node.description);
    std::visit(WriteVisitor{o}, node.params);

    if (node.isContainer())  // Listen + Host-Gruppen (HG1) tragen children
    {
        QJsonArray children;
        for (const ChainNode& child : node.children)
        {
            children.append(nodeToJson(child));
        }
        o["children"] = children;
    }
    return o;
}

ChainNode nodeFromJson(const QJsonObject& obj, QStringList* report)
{
    ChainNode node;
    const QString type = obj.value("type").toString("passthrough");
    node.params = readParams(type, obj);
    node.enabled = obj.value("enabled").toBool(true);
    node.displayName = obj.value("name").toString().toStdString();
    node.description = obj.value("description").toString().toStdString();

    if (report != nullptr && !obj.contains("type"))
    {
        report->append("node without a type - loaded as passthrough");
    }

    if (node.isContainer() && obj.value("children").isArray())
    {
        const QJsonArray children = obj.value("children").toArray();
        for (const QJsonValue& child : children)
        {
            if (child.isObject())
            {
                node.children.push_back(nodeFromJson(child.toObject(), report));
            }
        }
    }
    return node;
}

QJsonObject chainToJson(const ChainNode& root)
{
    QJsonObject header;
    header["formatVersion"] = kFormatVersion;
    header["generator"] = "LumiViz MultiEffect";

    QJsonObject doc;
    doc["header"] = header;
    doc["root"] = nodeToJson(root);
    return doc;
}

ChainNode chainFromJson(const QJsonObject& doc, QStringList* report)
{
    ChainNode root;
    if (doc.value("root").isObject())
    {
        root = nodeFromJson(doc.value("root").toObject(), report);
    }
    else
    {
        root.params = ListParams{};
        if (report != nullptr) report->append("document has no root node");
    }
    // A preset root must be a list.
    if (!root.isList())
    {
        ChainNode wrapper;
        wrapper.params = ListParams{};
        wrapper.children.push_back(std::move(root));
        root = std::move(wrapper);
    }
    compileChain(root);
    return root;
}

bool saveChainToFile(const ChainNode& root, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    const QJsonDocument doc(chainToJson(root));
    const qint64 written = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return written > 0;
}

bool loadChainFromFile(const QString& path, ChainNode& outRoot, QStringList* report)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    const QByteArray bytes = file.readAll();
    file.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        if (report != nullptr) report->append("JSON parse error: " + err.errorString());
        return false;
    }
    outRoot = chainFromJson(doc.object(), report);
    return true;
}

} // namespace lumi::multieffect
