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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

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
        o["onBeatRender"] = p.onBeatRender;
        o["onBeatFrames"] = p.onBeatFrames;
        o["useCode"] = p.useCode;
        o["initCode"] = QString::fromStdString(p.initCode);
        o["frameCode"] = QString::fromStdString(p.frameCode);
    }
    void operator()(const ClearParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["onlyFirst"] = p.onlyFirst;
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
        o["leftToRight"] = p.leftToRight;
        o["topToBottom"] = p.topToBottom;
        o["onBeatRandom"] = p.onBeatRandom;
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
    }
    void operator()(const BlitterFeedbackParams& p) const
    {
        o["zoom"] = p.zoom;   o["beatZoom"] = p.beatZoom;
        o["onBeat"] = p.onBeat;   o["blend"] = p.blend;
    }
    void operator()(const RotoBlitterParams& p) const
    {
        o["zoom"] = p.zoom;
        o["rotationSpeed"] = p.rotationSpeed;
        o["blend"] = p.blend;
    }
    void operator()(const BufferSaveParams& p) const
    {
        o["slot"] = p.slot;
        o["save"] = p.save;
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
    }
    void operator()(const DebugBarsParams& p) const
    {
        o["color"] = static_cast<double>(p.color);
        o["orbitSpeed"] = p.orbitSpeed;
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
        p.onBeatRender = getBool(o, "onBeatRender", false);
        p.onBeatFrames = getInt(o, "onBeatFrames", 1);
        p.useCode = getBool(o, "useCode", false);
        p.initCode = getStr(o, "initCode");
        p.frameCode = getStr(o, "frameCode");
        return p;
    }
    if (type == "clear")
        return ClearParams{getColor(o, "color", 0), getBool(o, "onlyFirst", false)};
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
        return MirrorParams{getBool(o, "leftToRight", true),
                            getBool(o, "topToBottom", false),
                            getBool(o, "onBeatRandom", false)};
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
        return p;
    }
    if (type == "blitterFeedback")
    {
        BlitterFeedbackParams p;
        p.zoom = static_cast<float>(getDouble(o, "zoom", 1.03));
        p.beatZoom = static_cast<float>(getDouble(o, "beatZoom", 0.9));
        p.onBeat = getBool(o, "onBeat", false);
        p.blend = getBool(o, "blend", true);
        return p;
    }
    if (type == "rotoBlitter")
    {
        RotoBlitterParams p;
        p.zoom = static_cast<float>(getDouble(o, "zoom", 1.0));
        p.rotationSpeed = static_cast<float>(getDouble(o, "rotationSpeed", 1.0));
        p.blend = getBool(o, "blend", true);
        return p;
    }
    if (type == "bufferSave")
    {
        BufferSaveParams p;
        p.slot = getInt(o, "slot", 0);
        p.save = getBool(o, "save", true);
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
        return p;
    }
    if (type == "debugBars")
        return DebugBarsParams{getColor(o, "color", 0xFF80FF),
                               static_cast<float>(getDouble(o, "orbitSpeed", 1.0))};
    // "passthrough" and any unknown key
    return PassthroughParams{getInt(o, "sourceId", 0), getStr(o, "note")};
}

} // namespace

QString effectTypeKey(const EffectParams& params)
{
    struct Visitor
    {
        QString operator()(const ListParams&) const { return "list"; }
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
        QString operator()(const DebugBarsParams&) const { return "debugBars"; }
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

    if (node.isList())
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

    if (node.isList() && obj.value("children").isArray())
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
