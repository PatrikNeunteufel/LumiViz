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
    }
    void operator()(const FadeoutParams& p) const
    {
        o["fadeLen"] = p.fadeLen;
        o["color"] = static_cast<double>(p.color);
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
    }
    void operator()(const FastBrightnessParams& p) const { o["dir"] = p.dir; }
    void operator()(const BlurParams& p) const
    {
        o["strength"] = p.strength;
        o["roundUp"] = p.roundUp;
    }
    void operator()(const MirrorParams& p) const
    {
        o["mode"] = p.mode;
        o["onBeatRandom"] = p.onBeatRandom;
        o["smooth"] = p.smooth;
        o["slower"] = p.slower;
    }
    void operator()(const OnBeatClearParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["everyNBeats"] = p.everyNBeats;
        o["blend"] = p.blend;
    }
    void operator()(const ColorfadeParams& p) const
    {
        o["faderR"] = p.faderR;   o["faderG"] = p.faderG;   o["faderB"] = p.faderB;
        o["beatFaderR"] = p.beatFaderR;
        o["beatFaderG"] = p.beatFaderG;
        o["beatFaderB"] = p.beatFaderB;
        o["onBeatFrames"] = p.onBeatFrames;
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
    }
    void operator()(const BufferSaveParams& p) const
    {
        o["slot"] = p.slot;
        o["dir"] = p.dir;
        o["blend"] = static_cast<int>(p.blend);
        o["adjustAlpha"] = p.adjustAlpha;
    }
    void operator()(const CustomBpmParams& p) const
    {
        o["arbitrary"] = p.arbitrary;
        o["arbitraryMs"] = p.arbitraryMs;
        o["skip"] = p.skip;
        o["skipCount"] = p.skipCount;
        o["invert"] = p.invert;
        o["skipFirst"] = p.skipFirst;
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
    }
    void operator()(const GrainParams& p) const
    {
        o["amount"] = p.amount;
        o["staticGrain"] = p.staticGrain;
        o["blend"] = p.blend;
    }
    void operator()(const ScatterParams&) const {}
    void operator()(const WaterParams&) const {}
    void operator()(const DynamicShiftParams& p) const
    {
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["blend"] = p.blend;
        o["bilinear"] = p.bilinear;
    }
    void operator()(const DynamicDistanceModifierParams& p) const
    {
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
        o["beatCode"] = QString::fromStdString(p.beatCode);
        o["pixelCode"] = QString::fromStdString(p.pixelCode);
        o["blend"] = p.blend;
        o["bilinear"] = p.bilinear;
    }
    void operator()(const MovingParticleParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["maxDistance"] = p.maxDistance;
        o["size"] = p.size;
        o["size2"] = p.size2;
        o["onBeatSize"] = p.onBeatSize;
        o["blend"] = p.blend;
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
    }
    void operator()(const BassSpinParams& p) const
    {
        o["left"] = p.left;
        o["right"] = p.right;
        o["colorLeft"] = static_cast<double>(p.colorLeft);
        o["colorRight"] = static_cast<double>(p.colorRight);
        o["mode"] = p.mode;
    }
    void operator()(const OscStarParams& p) const
    {
        o["channel"] = p.channel;
        o["position"] = p.position;
        o["size"] = p.size;
        o["rot"] = p.rot;
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
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
    }
    void operator()(const RotatingStarsParams& p) const
    {
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
    }
    void operator()(const PictureIIParams& p) const
    {
        o["filename"] = QString::fromStdString(p.filename);
        o["imageData"] = QString::fromStdString(p.imageData);
        o["blend"] = p.blend;
    }
    void operator()(const TexerParams& p) const
    {
        o["filename"] = QString::fromStdString(p.filename);
        o["imageData"] = QString::fromStdString(p.imageData);
        o["blend"] = p.blend;
        o["particles"] = p.particles;
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
    }
    void operator()(const NormaliseParams&) const {}
    void operator()(const MultiFilterParams& p) const
    {
        o["effect"] = p.effect;
        o["onBeat"] = p.onBeat;
    }
    void operator()(const AddBordersParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["size"] = p.size;
    }
    void operator()(const ColorClipParams& p) const
    {
        o["mode"] = p.mode;
        o["clipColor"] = static_cast<double>(p.clipColor);
        o["outColor"] = static_cast<double>(p.outColor);
        o["distance"] = p.distance;
    }
    void operator()(const UniqueToneParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["invert"] = p.invert;
        o["blend"] = p.blend;
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
    }
    void operator()(const FyrewurXParams& p) const
    {
        o["sparks"] = p.sparks;
        o["speed"] = p.speed;
        o["gravity"] = p.gravity;
        o["lifeSeconds"] = p.lifeSeconds;
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
    }
    void operator()(const TimescopeParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["blend"] = p.blend;
        o["channel"] = p.channel;
        o["bands"] = p.bands;
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
    }
    void operator()(const DotPlaneParams& p) const
    {
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
        o["rotVel"] = p.rotVel;
        o["angle"] = p.angle;
    }
    void operator()(const DotFountainParams& p) const
    {
        QJsonArray cols;
        for (uint32_t c : p.colors) cols.append(static_cast<double>(c));
        o["colors"] = cols;
        o["rotVel"] = p.rotVel;
        o["angle"] = p.angle;
    }
    void operator()(const ChannelShiftParams& p) const
    {
        o["mode"] = p.mode;
        o["onBeat"] = p.onBeat;
    }
    void operator()(const ColorReductionParams& p) const { o["levels"] = p.levels; }
    void operator()(const MultiplierParams& p) const { o["mode"] = p.mode; }
    void operator()(const VideoDelayParams& p) const
    {
        o["useBeats"] = p.useBeats;
        o["delay"] = p.delay;
    }
    void operator()(const MultiDelayParams& p) const
    {
        o["mode"] = p.mode;
        o["buffer"] = p.buffer;
        o["delay"] = p.delay;
        o["useBeats"] = p.useBeats;
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
        p.clearEveryFrame = getBool(o, "clearEveryFrame", false);
        p.blendIn = static_cast<BlendMode>(getInt(o, "blendIn", 0));
        p.blendOut = static_cast<BlendMode>(getInt(o, "blendOut", 1));
        p.inAdjustAlpha = getInt(o, "inAdjustAlpha", 128);
        p.outAdjustAlpha = getInt(o, "outAdjustAlpha", 128);
        p.bufferIn = getInt(o, "bufferIn", 0);
        p.bufferOut = getInt(o, "bufferOut", 0);
        p.bufferInInvert = getBool(o, "bufferInInvert", false);
        p.bufferOutInvert = getBool(o, "bufferOutInvert", false);
        p.onBeatRender = getBool(o, "onBeatRender", false);
        p.onBeatFrames = getInt(o, "onBeatFrames", 1);
        p.useCode = getBool(o, "useCode", false);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        return p;
    }
    if (type == "hostgroup")  // HG1 — .lvfx2-Kennzeichen
    {
        HostGroupParams p;
        p.blendOut = static_cast<BlendMode>(getInt(o, "blendOut", 1));
        p.outAdjustAlpha = getInt(o, "outAdjustAlpha", 128);
        p.crossfadeSeconds = o.value("crossfadeSeconds").toDouble(2.0);
        p.curveIn = getInt(o, "curveIn", 0);
        p.curveOut = getInt(o, "curveOut", 0);
        p.sourceFile = getStr(o, "sourceFile");
        return p;
    }
    if (type == "clear")
        return ClearParams{getColor(o, "color", 0), getBool(o, "onlyFirst", false),
                           getInt(o, "blend", 0)};
    if (type == "fadeout")
        return FadeoutParams{getInt(o, "fadeLen", 16), getColor(o, "color", 0)};
    if (type == "invert") return InvertParams{};
    if (type == "brightness")
    {
        BrightnessParams p;
        p.red = getInt(o, "red", 0);
        p.green = getInt(o, "green", 0);
        p.blue = getInt(o, "blue", 0);
        p.exclude = getBool(o, "exclude", false);
        p.color = getColor(o, "color", 0);
        p.distance = getInt(o, "distance", 16);
        return p;
    }
    if (type == "fastBrightness") return FastBrightnessParams{getInt(o, "dir", 0)};
    if (type == "blur")
        return BlurParams{getInt(o, "strength", 1), getBool(o, "roundUp", true)};
    if (type == "mirror")
    {
        MirrorParams p;
        // Legacy files carry two bools; current files the r_mirror bit mode.
        const int legacy = (getBool(o, "topToBottom", false) ? 1 : 0) |
                           (getBool(o, "leftToRight", true) ? 4 : 0);
        p.mode = getInt(o, "mode", legacy) & 15;
        p.onBeatRandom = getBool(o, "onBeatRandom", false);
        p.smooth = getBool(o, "smooth", false);
        p.slower = getInt(o, "slower", 4);
        return p;
    }
    if (type == "onBeatClear")
        return OnBeatClearParams{getColor(o, "color", 0), getInt(o, "everyNBeats", 1),
                                 getBool(o, "blend", false)};
    if (type == "colorfade")
    {
        ColorfadeParams p;
        p.faderR = getInt(o, "faderR", 8);
        p.faderG = getInt(o, "faderG", 8);
        p.faderB = getInt(o, "faderB", -8);
        p.beatFaderR = getInt(o, "beatFaderR", 8);
        p.beatFaderG = getInt(o, "beatFaderG", -8);
        p.beatFaderB = getInt(o, "beatFaderB", 8);
        p.onBeatFrames = getInt(o, "onBeatFrames", 1);
        return p;
    }
    if (type == "colorModifier")
    {
        ColorModifierParams p;
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.levelCode = getStr(o, "levelCode");
        p.recompute = getBool(o, "recompute", true);
        return p;
    }
    if (type == "movement")
    {
        MovementParams p;
        p.code = getStr(o, "code");
        p.rectCoords = getBool(o, "rectCoords", false);
        p.wrap = getBool(o, "wrap", false);
        p.blend = getBool(o, "blend", false);
        p.subpixel = getBool(o, "subpixel", true);
        p.sourceMapped = getInt(o, "sourceMapped", 0);
        p.builtinRemap = getInt(o, "builtinRemap", 0);
        return p;
    }
    if (type == "dynamicMovement")
    {
        DynamicMovementParams p;
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.pointCode = getStr(o, "pointCode");
        p.xres = getInt(o, "xres", 16);
        p.yres = getInt(o, "yres", 12);
        p.rectCoords = getBool(o, "rectCoords", false);
        p.wrap = getBool(o, "wrap", false);
        p.blend = getBool(o, "blend", false);
        p.nomove = getBool(o, "nomove", false);
        p.subpixel = getBool(o, "subpixel", true);
        p.buffern = getInt(o, "buffern", 0);
        return p;
    }
    if (type == "blitterFeedback")
    {
        BlitterFeedbackParams p;
        if (o.contains("scale"))
        {
            p.scale = getInt(o, "scale", 30);
            p.scale2 = getInt(o, "scale2", 30);
        }
        else
        {
            // Alt-Dokumente (vor S48): zoom-Float war 1 + scale/1024.
            p.scale = static_cast<int>(
                std::lround((getDouble(o, "zoom", 1.03) - 1.0) * 1024.0));
            p.scale2 = static_cast<int>(
                std::lround((getDouble(o, "beatZoom", 0.9) - 1.0) * 1024.0));
        }
        p.onBeat = getBool(o, "onBeat", false);
        p.blend = getBool(o, "blend", false);
        p.subpixel = getBool(o, "subpixel", true);
        return p;
    }
    if (type == "rotoBlitter")
    {
        RotoBlitterParams p;
        if (o.contains("zoomScale"))
        {
            p.zoomScale = getInt(o, "zoomScale", 31);
            p.zoomScale2 = getInt(o, "zoomScale2", 31);
            p.rotDir = getInt(o, "rotDir", 31);
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
        p.blend = getBool(o, "blend", false);
        p.beatReverse = getBool(o, "beatReverse", false);
        p.beatReverseSpeed = getInt(o, "beatReverseSpeed", 0);
        p.beatZoomJump = getBool(o, "beatZoomJump", false);
        p.subpixel = getBool(o, "subpixel", true);
        return p;
    }
    if (type == "bufferSave")
    {
        BufferSaveParams p;
        p.slot = getInt(o, "slot", 0);
        // Legacy files carry bool "save"; current files carry "dir" 0..3.
        p.dir = getInt(o, "dir", getBool(o, "save", true) ? 0 : 1);
        p.blend = static_cast<BlendMode>(getInt(o, "blend", 1));
        p.adjustAlpha = getInt(o, "adjustAlpha", 128);
        return p;
    }
    if (type == "customBpm")
    {
        CustomBpmParams p;
        p.arbitrary = getBool(o, "arbitrary", false);
        p.arbitraryMs = getInt(o, "arbitraryMs", 500);
        p.skip = getBool(o, "skip", false);
        p.skipCount = getInt(o, "skipCount", 1);
        p.invert = getBool(o, "invert", false);
        p.skipFirst = getInt(o, "skipFirst", 0);
        return p;
    }
    if (type == "superScope")
    {
        SuperScopeParams p;
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.pointCode = getStr(o, "pointCode");
        p.pointCount = getInt(o, "pointCount", 256);
        p.renderMode = getInt(o, "renderMode", 1);
        p.lineWidth = static_cast<float>(getDouble(o, "lineWidth", 2.0));
        p.dotSize = static_cast<float>(getDouble(o, "dotSize", 4.0));
        p.audioChannel = getInt(o, "audioChannel", 2);
        p.spectrumSource = getBool(o, "spectrumSource", false);
        p.lineBlend = getInt(o, "lineBlend", 1);
        p.colorBlend = getInt(o, "colorBlend", 0);
        p.colorCycleFrames = getInt(o, "colorCycleFrames", 60);
        p.gradientPreset = o.contains("gradientPreset") ? getStr(o, "gradientPreset")
                                                         : std::string("Neon");
        for (const QJsonValue& v : o.value("colors").toArray())
            p.colors.push_back(static_cast<uint32_t>(v.toDouble()));
        return p;
    }
    if (type == "mosaic")
    {
        MosaicParams p;
        p.quality = getInt(o, "quality", 50);
        p.quality2 = getInt(o, "quality2", 50);
        p.onBeat = getBool(o, "onBeat", false);
        p.durationFrames = getInt(o, "durationFrames", 16);
        p.blend = getInt(o, "blend", 0);
        return p;
    }
    if (type == "grain")
    {
        GrainParams p;
        p.amount = getInt(o, "amount", 100);
        p.staticGrain = getBool(o, "staticGrain", false);
        p.blend = getInt(o, "blend", 0);
        return p;
    }
    if (type == "scatter")
        return ScatterParams{};
    if (type == "water")
        return WaterParams{};
    if (type == "waterBump")
    {
        WaterBumpParams p;
        p.density = getInt(o, "density", 5);
        p.depth = getInt(o, "depth", 600);
        p.randomDrop = getBool(o, "randomDrop", true);
        p.dropX = getInt(o, "dropX", 1);
        p.dropY = getInt(o, "dropY", 1);
        p.dropRadius = getInt(o, "dropRadius", 40);
        p.displaceScale = static_cast<float>(getDouble(o, "displaceScale", 6.0));
        return p;
    }
    if (type == "dynamicShift")
    {
        DynamicShiftParams p;
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.blend = getBool(o, "blend", false);
        p.bilinear = getBool(o, "bilinear", true);
        return p;
    }
    if (type == "dynamicDistanceModifier")
    {
        DynamicDistanceModifierParams p;
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.pixelCode = getStr(o, "pixelCode");
        p.blend = getBool(o, "blend", false);
        p.bilinear = getBool(o, "bilinear", true);
        return p;
    }
    if (type == "movingParticle")
    {
        MovingParticleParams p;
        p.color = getColor(o, "color", 0xFFFFFF);
        p.maxDistance = getInt(o, "maxDistance", 16);
        p.size = getInt(o, "size", 8);
        p.size2 = getInt(o, "size2", 8);
        p.onBeatSize = getBool(o, "onBeatSize", false);
        p.blend = getInt(o, "blend", 1);
        return p;
    }
    if (type == "colorMap")
    {
        ColorMapParams p;
        p.key = getInt(o, "key", 0);
        p.blendMode = getInt(o, "blendMode", 0);
        p.adjustBlend = getInt(o, "adjustBlend", 128);
        const QJsonArray pos = o.value("stopPos").toArray();
        const QJsonArray col = o.value("stopColor").toArray();
        for (const auto& v : pos) p.stopPos.push_back(v.toInt());
        for (const auto& v : col) p.stopColor.push_back(static_cast<uint32_t>(v.toDouble()));
        return p;
    }
    if (type == "fractal2D")
    {
        Fractal2DParams p;
        p.type = getInt(o, "ftype", 0);
        p.centerX = static_cast<float>(getDouble(o, "centerX", -0.5));
        p.centerY = static_cast<float>(getDouble(o, "centerY", 0.0));
        p.zoom = static_cast<float>(getDouble(o, "zoom", 1.0));
        p.rotation = static_cast<float>(getDouble(o, "rotation", 0.0));
        p.maxIter = getInt(o, "maxIter", 128);
        p.juliaX = static_cast<float>(getDouble(o, "juliaX", -0.8));
        p.juliaY = static_cast<float>(getDouble(o, "juliaY", 0.156));
        p.power = static_cast<float>(getDouble(o, "power", 2.0));
        p.escapeR = static_cast<float>(getDouble(o, "escapeR", 4.0));
        p.smooth = getBool(o, "smooth", true);
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", 0.05));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", 0.0));
        p.insideColor = getColor(o, "insideColor", 0x000000);
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.blend = getInt(o, "blend", 0);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "domainWarp")
    {
        DomainWarpParams p;
        p.octaves = getInt(o, "octaves", 5);
        p.lacunarity = static_cast<float>(getDouble(o, "lacunarity", 2.0));
        p.gain = static_cast<float>(getDouble(o, "gain", 0.5));
        p.scale = static_cast<float>(getDouble(o, "scale", 3.0));
        p.warp = static_cast<float>(getDouble(o, "warp", 0.5));
        p.warpScale = static_cast<float>(getDouble(o, "warpScale", 1.0));
        p.speed = static_cast<float>(getDouble(o, "speed", 0.2));
        p.offsetX = static_cast<float>(getDouble(o, "offsetX", 0.0));
        p.offsetY = static_cast<float>(getDouble(o, "offsetY", 0.0));
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", 1.0));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", 0.0));
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.blend = getInt(o, "blend", 0);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "setRenderMode")
    {
        SetRenderModeParams p;
        // Altbestand: bis S51 stand das Override-Flag unter "enabled" und hat
        // dabei den Knoten-Schalter ueberschrieben — als Rueckfall weiter lesen.
        p.enabled = getBool(o, "overrideBlend", getBool(o, "enabled", true));
        p.lineWidth = getInt(o, "lineWidth", 1);
        p.lineBlend = getInt(o, "lineBlend", 1);
        p.adjustAlpha = getInt(o, "adjustAlpha", 128);
        return p;
    }
    if (type == "fractal3D")
    {
        Fractal3DParams p;
        p.type = getInt(o, "ftype", 0);
        p.yaw = static_cast<float>(getDouble(o, "yaw", 0.6));
        p.pitch = static_cast<float>(getDouble(o, "pitch", 0.3));
        p.dist = static_cast<float>(getDouble(o, "dist", 3.2));
        p.fov = static_cast<float>(getDouble(o, "fov", 1.0));
        p.power = static_cast<float>(getDouble(o, "power", 8.0));
        p.scale = static_cast<float>(getDouble(o, "scale", 2.0));
        p.fold = static_cast<float>(getDouble(o, "fold", 1.0));
        p.maxSteps = getInt(o, "maxSteps", 96);
        p.maxIter = getInt(o, "maxIter", 8);
        p.juliaX = static_cast<float>(getDouble(o, "juliaX", 0.2));
        p.juliaY = static_cast<float>(getDouble(o, "juliaY", 0.3));
        p.juliaZ = static_cast<float>(getDouble(o, "juliaZ", 0.1));
        p.juliaW = static_cast<float>(getDouble(o, "juliaW", 0.0));
        p.lightYaw = static_cast<float>(getDouble(o, "lightYaw", 0.7));
        p.lightPitch = static_cast<float>(getDouble(o, "lightPitch", 0.8));
        p.ambient = static_cast<float>(getDouble(o, "ambient", 0.2));
        p.ao = getBool(o, "ao", true);
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", 1.0));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", 0.0));
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.background = getColor(o, "background", 0x000000);
        p.blend = getInt(o, "blend", 0);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "lyapunov")
    {
        LyapunovParams p;
        p.sequence = getStr(o, "sequence");
        if (p.sequence.empty()) p.sequence = "AB";
        p.aMin = static_cast<float>(getDouble(o, "aMin", 2.5));
        p.aMax = static_cast<float>(getDouble(o, "aMax", 4.0));
        p.bMin = static_cast<float>(getDouble(o, "bMin", 2.5));
        p.bMax = static_cast<float>(getDouble(o, "bMax", 4.0));
        p.warmup = getInt(o, "warmup", 100);
        p.iterations = getInt(o, "iterations", 400);
        p.negColor = getColor(o, "negColor", 0x000030);
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", 1.0));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", 0.0));
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Fire";
        p.blend = getInt(o, "blend", 0);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "kleinian")
    {
        KleinianParams p;
        p.p = getInt(o, "p", 5);
        p.q = getInt(o, "q", 4);
        p.iterations = getInt(o, "iterations", 30);
        p.morph = static_cast<float>(getDouble(o, "morph", 0.0));
        p.zoom = static_cast<float>(getDouble(o, "zoom", 1.0));
        p.rotation = static_cast<float>(getDouble(o, "rotation", 0.0));
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", 1.0));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", 0.0));
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.blend = getInt(o, "blend", 0);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "fractalZoomer")
    {
        FractalZoomerParams p;
        p.type = getInt(o, "ftype", 0);
        p.centerX = static_cast<float>(getDouble(o, "centerX", -0.743643887));
        p.centerY = static_cast<float>(getDouble(o, "centerY", 0.131825904));
        p.juliaX = static_cast<float>(getDouble(o, "juliaX", -0.8));
        p.juliaY = static_cast<float>(getDouble(o, "juliaY", 0.156));
        p.maxIter = getInt(o, "maxIter", 200);
        p.zoomSpeed = static_cast<float>(getDouble(o, "zoomSpeed", 1.02));
        p.rotationSpeed = static_cast<float>(getDouble(o, "rotationSpeed", 0.0));
        p.feedback = static_cast<float>(getDouble(o, "feedback", 0.5));
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", 0.05));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", 0.0));
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.insideColor = getColor(o, "insideColor", 0x000000);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "strangeAttractor")
    {
        StrangeAttractorParams p;
        p.type = getInt(o, "ftype", 0);
        p.a = static_cast<float>(getDouble(o, "a", 1.4));
        p.b = static_cast<float>(getDouble(o, "b", 1.6));
        p.c = static_cast<float>(getDouble(o, "c", 1.0));
        p.d = static_cast<float>(getDouble(o, "d", 0.7));
        p.points = getInt(o, "points", 6000);
        p.scale = static_cast<float>(getDouble(o, "scale", 0.28));
        p.rotation = static_cast<float>(getDouble(o, "rotation", 0.0));
        p.rotationSpeed = static_cast<float>(getDouble(o, "rotationSpeed", 0.08));
        p.color = getColor(o, "color", 0x66CCFF);
        p.useGradient = getBool(o, "useGradient", true);
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.dotSize = static_cast<float>(getDouble(o, "dotSize", 2.0));
        p.blend = getInt(o, "blend", 1);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "flame")
    {
        FlameParams p;
        p.variation = getInt(o, "variation", 0);
        p.functions = getInt(o, "functions", 3);
        p.points = getInt(o, "points", 20000);
        p.scale = static_cast<float>(getDouble(o, "scale", 0.5));
        p.rotation = static_cast<float>(getDouble(o, "rotation", 0.0));
        p.rotationSpeed = static_cast<float>(getDouble(o, "rotationSpeed", 0.04));
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Fire";
        p.dotSize = static_cast<float>(getDouble(o, "dotSize", 1.5));
        p.blend = getInt(o, "blend", 1);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "reactionDiffusion")
    {
        ReactionDiffusionParams p;
        p.feed = static_cast<float>(getDouble(o, "feed", 0.055));
        p.kill = static_cast<float>(getDouble(o, "kill", 0.062));
        p.diffA = static_cast<float>(getDouble(o, "diffA", 1.0));
        p.diffB = static_cast<float>(getDouble(o, "diffB", 0.5));
        p.stepsPerFrame = getInt(o, "stepsPerFrame", 8);
        p.seedOnBeat = getBool(o, "seedOnBeat", true);
        p.colorScale = static_cast<float>(getDouble(o, "colorScale", 1.0));
        p.colorCycle = static_cast<float>(getDouble(o, "colorCycle", 0.0));
        p.gradientPreset = getStr(o, "gradientPreset");
        if (p.gradientPreset.empty()) p.gradientPreset = "Neon";
        p.blend = getInt(o, "blend", 0);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "bufferBlend")
    {
        BufferBlendParams p;
        p.bufferA = getInt(o, "bufferA", 8);
        p.bufferB = getInt(o, "bufferB", 8);
        p.mode = getInt(o, "mode", 0);
        return p;
    }
    if (type == "jherikoGlobal")
    {
        JherikoGlobalParams p;
        p.loadMode = getInt(o, "loadMode", 1);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "simpleScope")
    {
        SimpleScopeParams p;
        if (o.contains("mode"))
        {
            p.mode = std::clamp(getInt(o, "mode", 3), 0, 5);
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
        p.channel = getInt(o, "channel", 2);
        p.position = getInt(o, "position", 2);
        p.colors.clear();
        const QJsonArray cols = o.value("colors").toArray();
        for (const auto& v : cols) p.colors.push_back(static_cast<uint32_t>(v.toDouble()));
        if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
        return p;
    }
    if (type == "bassSpin")
    {
        BassSpinParams p;
        p.left = getBool(o, "left", true);
        p.right = getBool(o, "right", true);
        p.colorLeft = getColor(o, "colorLeft", 0xFFFFFF);
        p.colorRight = getColor(o, "colorRight", 0xFFFFFF);
        p.mode = getInt(o, "mode", 1);
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
            p.channel = getInt(o, "channel", 2);
            p.position = getInt(o, "position", 2);
            p.size = getInt(o, "size", 8);
            p.rot = getInt(o, "rot", 3);
            p.colors = std::move(colors);
            return p;
        }
        OscRingParams p;
        p.source = getInt(o, "source", 0);
        p.channel = getInt(o, "channel", 2);
        p.position = getInt(o, "position", 2);
        p.size = getInt(o, "size", 8);
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
        return p;
    }
    if (type == "picture")
    {
        PictureParams p;
        p.filename = getStr(o, "filename");
        p.imageData = getStr(o, "imageData");
        p.blend = getInt(o, "blend", 2);
        p.keepAspect = getBool(o, "keepAspect", true);
        return p;
    }
    if (type == "pictureII")
    {
        PictureIIParams p;
        p.filename = getStr(o, "filename");
        p.imageData = getStr(o, "imageData");
        p.blend = getInt(o, "blend", 2);
        return p;
    }
    if (type == "texer")
    {
        TexerParams p;
        p.filename = getStr(o, "filename");
        p.imageData = getStr(o, "imageData");
        p.blend = getInt(o, "blend", 1);
        p.particles = getInt(o, "particles", 100);
        return p;
    }
    if (type == "texerII")
    {
        TexerIIParams p;
        p.filename = getStr(o, "filename");
        p.imageData = getStr(o, "imageData");
        p.resizing = getBool(o, "resizing", false);
        p.wrapAround = getBool(o, "wrapAround", false);
        p.colorFiltering = getBool(o, "colorFiltering", true);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.pointCode = getStr(o, "pointCode");
        return p;
    }
    if (type == "triangle")
    {
        TriangleParams p;
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.pointCode = getStr(o, "pointCode");
        return p;
    }
    if (type == "convolution")
    {
        ConvolutionParams p;
        p.edgeMode = getInt(o, "edgeMode", 0);
        p.absolute = getBool(o, "absolute", false);
        p.twoPass = getBool(o, "twoPass", false);
        p.bias = getInt(o, "bias", 0);
        p.scale = getInt(o, "scale", 1);
        const QJsonArray k = o.value("kernel").toArray();
        for (int i = 0; i < 49 && i < k.size(); ++i)
            p.kernel[static_cast<std::size_t>(i)] = k[i].toInt();
        return p;
    }
    if (type == "normalise") return NormaliseParams{};
    if (type == "multiFilter")
    {
        MultiFilterParams p;
        p.effect = getInt(o, "effect", 0);
        p.onBeat = getBool(o, "onBeat", false);
        return p;
    }
    if (type == "addBorders")
    {
        AddBordersParams p;
        p.color = getColor(o, "color", 0xFFFFFF);
        p.size = getInt(o, "size", 2);
        return p;
    }
    if (type == "colorClip")
    {
        ColorClipParams p;
        p.mode = getInt(o, "mode", 1);
        p.clipColor = getColor(o, "clipColor", 0x202020);
        p.outColor = getColor(o, "outColor", 0x202020);
        p.distance = getInt(o, "distance", 10);
        return p;
    }
    if (type == "uniqueTone")
    {
        UniqueToneParams p;
        p.color = getColor(o, "color", 0xFFFFFF);
        p.invert = getBool(o, "invert", false);
        p.blend = getInt(o, "blend", 0);
        return p;
    }
    if (type == "interleave")
    {
        InterleaveParams p;
        p.x = getInt(o, "x", 1);
        p.y = getInt(o, "y", 1);
        p.color = getColor(o, "color", 0);
        p.blend = getInt(o, "blend", 0);
        p.onBeat = getBool(o, "onBeat", false);
        p.x2 = getInt(o, "x2", 1);
        p.y2 = getInt(o, "y2", 1);
        p.beatDuration = getInt(o, "beatDuration", 4);
        return p;
    }
    if (type == "bump")
    {
        BumpParams p;
        p.depth = getInt(o, "depth", 30);
        p.depth2 = getInt(o, "depth2", 100);
        p.onBeat = getBool(o, "onBeat", false);
        p.durationFrames = getInt(o, "durationFrames", 15);
        p.invert = getBool(o, "invert", false);
        p.oldStyle = getBool(o, "oldStyle", false);
        p.blend = getInt(o, "blend", 0);
        p.buffern = getInt(o, "buffern", 0);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "interferences")
    {
        InterferencesParams p;
        p.points = getInt(o, "points", 2);
        p.distance = getInt(o, "distance", 10);
        p.alpha = getInt(o, "alpha", 128);
        p.rotation = getInt(o, "rotation", 0);
        p.rotationInc = getInt(o, "rotationInc", 0);
        p.distance2 = getInt(o, "distance2", 32);
        p.alpha2 = getInt(o, "alpha2", 192);
        p.rotationInc2 = getInt(o, "rotationInc2", 25);
        p.rgb = getBool(o, "rgb", false);
        p.onBeat = getBool(o, "onBeat", false);
        p.speed = static_cast<float>(getDouble(o, "speed", 0.2));
        p.blend = getInt(o, "blend", 0);
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
            p.radius = static_cast<float>(getDouble(o, "radius", 0.20));
            p.speed = static_cast<float>(getDouble(o, "speed", 0.45));
            p.threshold = static_cast<float>(getDouble(o, "threshold", 1.0));
            p.blend = std::clamp(getInt(o, "blend", 0), 0, 2);
            return p;
        }
        Tentacles3DParams p;
        p.colors = colors;
        p.count = std::clamp(getInt(o, "count", 7), 1, 16);
        p.segments = std::clamp(getInt(o, "segments", 28), 2, 256);
        p.length = static_cast<float>(getDouble(o, "length", 0.85));
        p.thickness = static_cast<float>(getDouble(o, "thickness", 9.0));
        p.speed = static_cast<float>(getDouble(o, "speed", 0.7));
        p.blend = std::clamp(getInt(o, "blend", 1), 0, 2);
        return p;
    }
    if (type == "fyrewurx")
    {
        FyrewurXParams p;
        p.sparks = getInt(o, "sparks", 80);
        p.speed = static_cast<float>(getDouble(o, "speed", 0.7));
        p.gravity = static_cast<float>(getDouble(o, "gravity", 0.8));
        p.lifeSeconds = static_cast<float>(getDouble(o, "lifeSeconds", 1.6));
        return p;
    }
    if (type == "text")
    {
        TextParams p;
        p.text = getStr(o, "text");
        p.fontFace = getStr(o, "fontFace");
        p.fontHeight = getInt(o, "fontHeight", -20);
        p.fontWeight = getInt(o, "fontWeight", 400);
        p.italic = getBool(o, "italic", false);
        p.underline = getBool(o, "underline", false);
        p.color = getColor(o, "color", 0xFFFFFF);
        p.blend = getInt(o, "blend", 0);
        p.onBeat = getBool(o, "onBeat", false);
        p.onBeatSpeed = getInt(o, "onBeatSpeed", 15);
        p.normSpeed = getInt(o, "normSpeed", 15);
        p.insertBlank = getBool(o, "insertBlank", false);
        p.randomPos = getBool(o, "randomPos", false);
        p.randomWord = getBool(o, "randomWord", false);
        p.hAlign = getInt(o, "hAlign", 1);
        p.vAlign = getInt(o, "vAlign", 1);
        p.xShift = getInt(o, "xShift", 0);
        p.yShift = getInt(o, "yShift", 0);
        p.outline = getBool(o, "outline", false);
        p.outlineColor = getColor(o, "outlineColor", 0);
        p.outlineSize = getInt(o, "outlineSize", 1);
        p.shadow = getBool(o, "shadow", false);
        return p;
    }
    if (type == "avi")
    {
        AviParams p;
        p.filename = getStr(o, "filename");
        p.resolvedPath = getStr(o, "resolvedPath");
        p.blend = getInt(o, "blend", 0);
        p.adapt = getBool(o, "adapt", false);
        p.persist = getInt(o, "persist", 6);
        p.speedMs = getInt(o, "speedMs", 0);
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
        p.vignette = getBool(o, "vignette", false);
        p.vignetteStrength = static_cast<float>(
            std::clamp(getDouble(o, "vignetteStrength", 0.3), 0.0, 1.0));
        p.post = getBool(o, "post", true);
        return p;
    }
    if (type == "camera3d")
    {
        Camera3DParams p;
        p.px = static_cast<float>(getDouble(o, "px", 0.0));
        p.py = static_cast<float>(getDouble(o, "py", 0.0));
        p.pz = static_cast<float>(getDouble(o, "pz", 3.7320508));
        p.tx = static_cast<float>(getDouble(o, "tx", 0.0));
        p.ty = static_cast<float>(getDouble(o, "ty", 0.0));
        p.tz = static_cast<float>(getDouble(o, "tz", 0.0));
        p.fov = static_cast<float>(std::clamp(getDouble(o, "fov", 30.0), 1.0, 179.0));
        p.roll = static_cast<float>(getDouble(o, "roll", 0.0));
        p.fogStart = static_cast<float>(getDouble(o, "fogStart", 0.0));
        p.fogEnd = static_cast<float>(getDouble(o, "fogEnd", 0.0));
        p.fogColor = getColor(o, "fogColor", 0x000000);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        return p;
    }
    if (type == "superScope3d")
    {
        SuperScope3DParams p;
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.pointCode = getStr(o, "pointCode");
        p.pointCount = std::clamp(getInt(o, "pointCount", 256), 1, 4096);
        p.renderMode = std::clamp(getInt(o, "renderMode", 0), 0, 1);
        p.size = static_cast<float>(
            std::clamp(getDouble(o, "size", 0.05), 0.0001, 100.0));
        p.falloff = static_cast<float>(
            std::clamp(getDouble(o, "falloff", 4.0), 0.5, 32.0));
        p.audioChannel = std::clamp(getInt(o, "audioChannel", 2), 0, 2);
        p.spectrumSource = getBool(o, "spectrumSource", false);
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
        p.drawMesh = getBool(o, "drawMesh", true);
        p.meshColor = getColor(o, "meshColor", 0x101418);
        p.drawDots = getBool(o, "drawDots", true);
        p.dotSize = static_cast<float>(
            std::clamp(getDouble(o, "dotSize", 0.045), 0.0001, 10.0));
        p.falloff = static_cast<float>(
            std::clamp(getDouble(o, "falloff", 4.0), 0.5, 32.0));
        p.colorLow = getColor(o, "colorLow", 0x0A2040);
        p.colorHigh = getColor(o, "colorHigh", 0x40C0FF);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.pointCode = getStr(o, "pointCode");
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
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        p.beatCode = getStr(o, "beatCode");
        p.pointCode = getStr(o, "pointCode");
        return p;
    }
    if (type == "starfield")
    {
        StarfieldParams p;
        p.color = getColor(o, "color", 0xFFFFFF);
        p.warpSpeed = static_cast<float>(getDouble(o, "warpSpeed", 6.0));
        p.maxStars = getInt(o, "maxStars", 350);
        p.onBeat = getBool(o, "onBeat", false);
        p.beatSpeed = static_cast<float>(getDouble(o, "beatSpeed", 4.0));
        p.durationFrames = getInt(o, "durationFrames", 15);
        p.blend = getInt(o, "blend", 1);  // legacy files rendered additively
        return p;
    }
    if (type == "timescope")
    {
        TimescopeParams p;
        p.color = getColor(o, "color", 0xFFFFFF);
        p.blend = getInt(o, "blend", 0);
        p.channel = getInt(o, "channel", 2);
        p.bands = getInt(o, "bands", 576);
        return p;
    }
    if (type == "dotGrid")
    {
        DotGridParams p;
        p.colors.clear();
        for (const QJsonValue& v : o.value("colors").toArray())
            p.colors.push_back(static_cast<uint32_t>(v.toDouble()));
        if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
        p.spacing = getInt(o, "spacing", 8);
        p.xMove = getInt(o, "xMove", 128);
        p.yMove = getInt(o, "yMove", 128);
        p.blend = getInt(o, "blend", 0);
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
            p.rotVel = getInt(o, "rotVel", 16);
            p.angle = getInt(o, "angle", -20);
            return p;
        }
        DotFountainParams p;
        fill(p.colors);
        p.rotVel = getInt(o, "rotVel", 16);
        p.angle = getInt(o, "angle", -20);
        return p;
    }
    if (type == "channelShift")
        return ChannelShiftParams{getInt(o, "mode", 1), getBool(o, "onBeat", false)};
    if (type == "colorReduction")
        return ColorReductionParams{getInt(o, "levels", 8)};
    if (type == "multiplier")
        return MultiplierParams{getInt(o, "mode", 3)};
    if (type == "videoDelay")
        return VideoDelayParams{getBool(o, "useBeats", false), getInt(o, "delay", 10)};
    if (type == "multiDelay")
    {
        MultiDelayParams p;
        p.mode = getInt(o, "mode", 0);
        p.buffer = getInt(o, "buffer", 0);
        p.delay = getInt(o, "delay", 10);
        p.useBeats = getBool(o, "useBeats", false);
        return p;
    }
    if (type == "debugBars")
        return DebugBarsParams{getColor(o, "color", 0xFF80FF),
                               static_cast<float>(getDouble(o, "orbitSpeed", 1.0))};
    if (type == "milkdrop")
    {
        MilkdropNodeParams p;
        p.preset = lumi::milkdrop::presetFromJson(o.value("preset").toObject(), nullptr);
        p.presetDir = getStr(o, "presetDir");
        p.meshX = getInt(o, "meshX", 32);
        p.meshY = getInt(o, "meshY", 24);
        p.debugGrid = getBool(o, "debugGrid", false);
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
