/**
 ****************************************************************************************
 * @file   MilkdropSerializer.cpp
 * @brief  Implementation of the milkdrop preset JSON persistence (Roadmap 6, M6)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#include "visualizers/milkdrop/MilkdropSerializer.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

namespace lumi::milkdrop {

namespace {

constexpr int kFormatVersion = 1;
const QString kDocType = QStringLiteral("milkdrop");

// --- small typed getters with defaults (ChainSerializer pattern) --------------
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
QString qstr(const std::string& s)
{
    return QString::fromStdString(s);
}

QJsonObject waveToJson(const WaveState& w)
{
    QJsonObject o;
    o["index"] = w.index;
    o["enabled"] = w.enabled;
    o["samples"] = w.samples;
    o["sep"] = w.sep;
    o["spectrum"] = w.spectrum;
    o["useDots"] = w.useDots;
    o["drawThick"] = w.drawThick;
    o["additive"] = w.additive;
    o["scaling"] = w.scaling;
    o["smoothing"] = w.smoothing;
    o["r"] = w.r;
    o["g"] = w.g;
    o["b"] = w.b;
    o["a"] = w.a;
    o["initCode"] = qstr(w.initCode);
    o["frameCode"] = qstr(w.frameCode);
    o["pointCode"] = qstr(w.pointCode);
    return o;
}

WaveState waveFromJson(const QJsonObject& o)
{
    WaveState w;
    w.index = getInt(o, "index", w.index);
    w.enabled = getBool(o, "enabled", w.enabled);
    w.samples = getInt(o, "samples", w.samples);
    w.sep = getInt(o, "sep", w.sep);
    w.spectrum = getBool(o, "spectrum", w.spectrum);
    w.useDots = getBool(o, "useDots", w.useDots);
    w.drawThick = getBool(o, "drawThick", w.drawThick);
    w.additive = getBool(o, "additive", w.additive);
    w.scaling = getDouble(o, "scaling", w.scaling);
    w.smoothing = getDouble(o, "smoothing", w.smoothing);
    w.r = getDouble(o, "r", w.r);
    w.g = getDouble(o, "g", w.g);
    w.b = getDouble(o, "b", w.b);
    w.a = getDouble(o, "a", w.a);
    w.initCode = getStr(o, "initCode");
    w.frameCode = getStr(o, "frameCode");
    w.pointCode = getStr(o, "pointCode");
    return w;
}

QJsonObject shapeToJson(const ShapeState& s)
{
    QJsonObject o;
    o["index"] = s.index;
    o["enabled"] = s.enabled;
    o["sides"] = s.sides;
    o["additive"] = s.additive;
    o["thickOutline"] = s.thickOutline;
    o["textured"] = s.textured;
    o["instances"] = s.instances;
    o["texZoom"] = s.texZoom;
    o["texAng"] = s.texAng;
    o["x"] = s.x;
    o["y"] = s.y;
    o["rad"] = s.rad;
    o["ang"] = s.ang;
    o["r"] = s.r;
    o["g"] = s.g;
    o["b"] = s.b;
    o["a"] = s.a;
    o["r2"] = s.r2;
    o["g2"] = s.g2;
    o["b2"] = s.b2;
    o["a2"] = s.a2;
    o["borderR"] = s.borderR;
    o["borderG"] = s.borderG;
    o["borderB"] = s.borderB;
    o["borderA"] = s.borderA;
    o["initCode"] = qstr(s.initCode);
    o["frameCode"] = qstr(s.frameCode);
    return o;
}

ShapeState shapeFromJson(const QJsonObject& o)
{
    ShapeState s;
    s.index = getInt(o, "index", s.index);
    s.enabled = getBool(o, "enabled", s.enabled);
    s.sides = getInt(o, "sides", s.sides);
    s.additive = getBool(o, "additive", s.additive);
    s.thickOutline = getBool(o, "thickOutline", s.thickOutline);
    s.textured = getBool(o, "textured", s.textured);
    s.instances = getInt(o, "instances", s.instances);
    s.texZoom = getDouble(o, "texZoom", s.texZoom);
    s.texAng = getDouble(o, "texAng", s.texAng);
    s.x = getDouble(o, "x", s.x);
    s.y = getDouble(o, "y", s.y);
    s.rad = getDouble(o, "rad", s.rad);
    s.ang = getDouble(o, "ang", s.ang);
    s.r = getDouble(o, "r", s.r);
    s.g = getDouble(o, "g", s.g);
    s.b = getDouble(o, "b", s.b);
    s.a = getDouble(o, "a", s.a);
    s.r2 = getDouble(o, "r2", s.r2);
    s.g2 = getDouble(o, "g2", s.g2);
    s.b2 = getDouble(o, "b2", s.b2);
    s.a2 = getDouble(o, "a2", s.a2);
    s.borderR = getDouble(o, "borderR", s.borderR);
    s.borderG = getDouble(o, "borderG", s.borderG);
    s.borderB = getDouble(o, "borderB", s.borderB);
    s.borderA = getDouble(o, "borderA", s.borderA);
    s.initCode = getStr(o, "initCode");
    s.frameCode = getStr(o, "frameCode");
    return s;
}

} // namespace

QJsonObject presetToJson(const PresetState& state)
{
    QJsonObject p;

    // composite / general
    p["decay"] = state.decay;
    p["gammaAdj"] = state.gammaAdj;
    p["videoEchoZoom"] = state.videoEchoZoom;
    p["videoEchoAlpha"] = state.videoEchoAlpha;
    p["videoEchoOrientation"] = state.videoEchoOrientation;
    p["shader"] = state.shader;
    p["texWrap"] = state.texWrap;
    p["darkenCenter"] = state.darkenCenter;
    p["brighten"] = state.brighten;
    p["darken"] = state.darken;
    p["solarize"] = state.solarize;
    p["invert"] = state.invert;

    // basic waveform
    p["waveMode"] = state.waveMode;
    p["additiveWaves"] = state.additiveWaves;
    p["waveDots"] = state.waveDots;
    p["waveThick"] = state.waveThick;
    p["modWaveAlphaByVolume"] = state.modWaveAlphaByVolume;
    p["maximizeWaveColor"] = state.maximizeWaveColor;
    p["waveAlpha"] = state.waveAlpha;
    p["waveScale"] = state.waveScale;
    p["waveSmoothing"] = state.waveSmoothing;
    p["waveMystery"] = state.waveMystery;
    p["modWaveAlphaStart"] = state.modWaveAlphaStart;
    p["modWaveAlphaEnd"] = state.modWaveAlphaEnd;
    p["waveR"] = state.waveR;
    p["waveG"] = state.waveG;
    p["waveB"] = state.waveB;
    p["waveX"] = state.waveX;
    p["waveY"] = state.waveY;

    // motion
    p["warpAnimSpeed"] = state.warpAnimSpeed;
    p["warpScale"] = state.warpScale;
    p["zoomExponent"] = state.zoomExponent;
    p["zoom"] = state.zoom;
    p["rot"] = state.rot;
    p["cx"] = state.cx;
    p["cy"] = state.cy;
    p["dx"] = state.dx;
    p["dy"] = state.dy;
    p["warp"] = state.warp;
    p["sx"] = state.sx;
    p["sy"] = state.sy;

    // borders
    p["obSize"] = state.obSize;
    p["obR"] = state.obR;
    p["obG"] = state.obG;
    p["obB"] = state.obB;
    p["obA"] = state.obA;
    p["ibSize"] = state.ibSize;
    p["ibR"] = state.ibR;
    p["ibG"] = state.ibG;
    p["ibB"] = state.ibB;
    p["ibA"] = state.ibA;

    // motion vectors
    p["mvX"] = state.mvX;
    p["mvY"] = state.mvY;
    p["mvDX"] = state.mvDX;
    p["mvDY"] = state.mvDY;
    p["mvL"] = state.mvL;
    p["mvR"] = state.mvR;
    p["mvG"] = state.mvG;
    p["mvB"] = state.mvB;
    p["mvA"] = state.mvA;

    // blur pyramid (M5)
    p["blur1Min"] = state.blur1Min;
    p["blur2Min"] = state.blur2Min;
    p["blur3Min"] = state.blur3Min;
    p["blur1Max"] = state.blur1Max;
    p["blur2Max"] = state.blur2Max;
    p["blur3Max"] = state.blur3Max;
    p["blur1EdgeDarken"] = state.blur1EdgeDarken;

    // code + shaders (source text — ShaderInfo is re-derived on load)
    p["perFrameInit"] = qstr(state.perFrameInit);
    p["perFrame"] = qstr(state.perFrame);
    p["perPixel"] = qstr(state.perPixel);
    p["warpShader"] = qstr(state.warpShaderText);
    p["compShader"] = qstr(state.compShaderText);

    QJsonArray waves;
    for (const WaveState& w : state.waves) waves.append(waveToJson(w));
    p["waves"] = waves;
    QJsonArray shapes;
    for (const ShapeState& s : state.shapes) shapes.append(shapeToJson(s));
    p["shapes"] = shapes;

    // meta
    p["generation"] = state.generation;
    p["psVersion"] = state.psVersion;
    p["name"] = qstr(state.name);

    QJsonObject header;
    header["formatVersion"] = kFormatVersion;
    header["generator"] = "LumiViz Milkdrop";
    header["type"] = kDocType;

    QJsonObject doc;
    doc["header"] = header;
    doc["preset"] = p;
    return doc;
}

PresetState presetFromJson(const QJsonObject& doc, QStringList* report)
{
    PresetState s;  // CState defaults
    if (!doc.value("preset").isObject())
    {
        if (report != nullptr)
        {
            report->append(QStringLiteral("Dokument ohne 'preset'-Objekt — Defaults"));
        }
        return s;
    }
    const QJsonObject p = doc.value("preset").toObject();

    s.decay = getDouble(p, "decay", s.decay);
    s.gammaAdj = getDouble(p, "gammaAdj", s.gammaAdj);
    s.videoEchoZoom = getDouble(p, "videoEchoZoom", s.videoEchoZoom);
    s.videoEchoAlpha = getDouble(p, "videoEchoAlpha", s.videoEchoAlpha);
    s.videoEchoOrientation = getInt(p, "videoEchoOrientation", s.videoEchoOrientation);
    s.shader = getDouble(p, "shader", s.shader);
    s.texWrap = getBool(p, "texWrap", s.texWrap);
    s.darkenCenter = getBool(p, "darkenCenter", s.darkenCenter);
    s.brighten = getBool(p, "brighten", s.brighten);
    s.darken = getBool(p, "darken", s.darken);
    s.solarize = getBool(p, "solarize", s.solarize);
    s.invert = getBool(p, "invert", s.invert);

    s.waveMode = getInt(p, "waveMode", s.waveMode);
    s.additiveWaves = getBool(p, "additiveWaves", s.additiveWaves);
    s.waveDots = getBool(p, "waveDots", s.waveDots);
    s.waveThick = getBool(p, "waveThick", s.waveThick);
    s.modWaveAlphaByVolume = getBool(p, "modWaveAlphaByVolume", s.modWaveAlphaByVolume);
    s.maximizeWaveColor = getBool(p, "maximizeWaveColor", s.maximizeWaveColor);
    s.waveAlpha = getDouble(p, "waveAlpha", s.waveAlpha);
    s.waveScale = getDouble(p, "waveScale", s.waveScale);
    s.waveSmoothing = getDouble(p, "waveSmoothing", s.waveSmoothing);
    s.waveMystery = getDouble(p, "waveMystery", s.waveMystery);
    s.modWaveAlphaStart = getDouble(p, "modWaveAlphaStart", s.modWaveAlphaStart);
    s.modWaveAlphaEnd = getDouble(p, "modWaveAlphaEnd", s.modWaveAlphaEnd);
    s.waveR = getDouble(p, "waveR", s.waveR);
    s.waveG = getDouble(p, "waveG", s.waveG);
    s.waveB = getDouble(p, "waveB", s.waveB);
    s.waveX = getDouble(p, "waveX", s.waveX);
    s.waveY = getDouble(p, "waveY", s.waveY);

    s.warpAnimSpeed = getDouble(p, "warpAnimSpeed", s.warpAnimSpeed);
    s.warpScale = getDouble(p, "warpScale", s.warpScale);
    s.zoomExponent = getDouble(p, "zoomExponent", s.zoomExponent);
    s.zoom = getDouble(p, "zoom", s.zoom);
    s.rot = getDouble(p, "rot", s.rot);
    s.cx = getDouble(p, "cx", s.cx);
    s.cy = getDouble(p, "cy", s.cy);
    s.dx = getDouble(p, "dx", s.dx);
    s.dy = getDouble(p, "dy", s.dy);
    s.warp = getDouble(p, "warp", s.warp);
    s.sx = getDouble(p, "sx", s.sx);
    s.sy = getDouble(p, "sy", s.sy);

    s.obSize = getDouble(p, "obSize", s.obSize);
    s.obR = getDouble(p, "obR", s.obR);
    s.obG = getDouble(p, "obG", s.obG);
    s.obB = getDouble(p, "obB", s.obB);
    s.obA = getDouble(p, "obA", s.obA);
    s.ibSize = getDouble(p, "ibSize", s.ibSize);
    s.ibR = getDouble(p, "ibR", s.ibR);
    s.ibG = getDouble(p, "ibG", s.ibG);
    s.ibB = getDouble(p, "ibB", s.ibB);
    s.ibA = getDouble(p, "ibA", s.ibA);

    s.mvX = getDouble(p, "mvX", s.mvX);
    s.mvY = getDouble(p, "mvY", s.mvY);
    s.mvDX = getDouble(p, "mvDX", s.mvDX);
    s.mvDY = getDouble(p, "mvDY", s.mvDY);
    s.mvL = getDouble(p, "mvL", s.mvL);
    s.mvR = getDouble(p, "mvR", s.mvR);
    s.mvG = getDouble(p, "mvG", s.mvG);
    s.mvB = getDouble(p, "mvB", s.mvB);
    s.mvA = getDouble(p, "mvA", s.mvA);

    s.blur1Min = getDouble(p, "blur1Min", s.blur1Min);
    s.blur2Min = getDouble(p, "blur2Min", s.blur2Min);
    s.blur3Min = getDouble(p, "blur3Min", s.blur3Min);
    s.blur1Max = getDouble(p, "blur1Max", s.blur1Max);
    s.blur2Max = getDouble(p, "blur2Max", s.blur2Max);
    s.blur3Max = getDouble(p, "blur3Max", s.blur3Max);
    s.blur1EdgeDarken = getDouble(p, "blur1EdgeDarken", s.blur1EdgeDarken);

    s.perFrameInit = getStr(p, "perFrameInit");
    s.perFrame = getStr(p, "perFrame");
    s.perPixel = getStr(p, "perPixel");
    s.warpShaderText = getStr(p, "warpShader");
    s.compShaderText = getStr(p, "compShader");
    // single source of truth: classification derives from the shader text
    s.warpInfo = lumi::milk::analyzeWarpShader(s.warpShaderText);
    s.compInfo = lumi::milk::analyzeCompShader(s.compShaderText);

    for (const QJsonValue& v : p.value("waves").toArray())
    {
        if (v.isObject()) s.waves.push_back(waveFromJson(v.toObject()));
    }
    for (const QJsonValue& v : p.value("shapes").toArray())
    {
        if (v.isObject()) s.shapes.push_back(shapeFromJson(v.toObject()));
    }

    s.generation = getInt(p, "generation", s.generation);
    s.psVersion = getInt(p, "psVersion", s.psVersion);
    s.name = getStr(p, "name");
    return s;
}

bool savePresetToFile(const PresetState& state, const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    const QJsonDocument doc(presetToJson(state));
    const qint64 written = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return written > 0;
}

bool loadPresetFromFile(const QString& path, PresetState& outState, QStringList* report)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    const QByteArray bytes = file.readAll();
    file.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        if (report != nullptr)
        {
            report->append(QStringLiteral("JSON parse error: %1").arg(err.errorString()));
        }
        return false;
    }
    const QJsonObject obj = doc.object();
    if (obj.value("header").toObject().value("type").toString() != kDocType)
    {
        if (report != nullptr)
        {
            report->append(QStringLiteral("Kein Milkdrop-Dokument (header.type)"));
        }
        return false;
    }
    outState = presetFromJson(obj, report);
    return true;
}

bool isMilkdropFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;
    const QByteArray bytes = file.readAll();
    file.close();
    const QJsonDocument doc = QJsonDocument::fromJson(bytes);
    if (!doc.isObject()) return false;
    return doc.object().value("header").toObject().value("type").toString() == kDocType;
}

} // namespace lumi::milkdrop
