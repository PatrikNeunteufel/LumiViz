/**
 ****************************************************************************************
 * @file   AvsChainTranslator.cpp
 * @brief  Implementation of the AVS-tree -> host EffectChain translator (Roadmap 5.5)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 ****************************************************************************************
 */

#include "visualizers/multieffect/AvsChainTranslator.hpp"

#include <algorithm>

namespace lumi::multieffect {

namespace {

using lumi::avs::EffectNode;

// AVS builtin effect ids (registration order == preset id, analysis §5.2).
enum AvsId
{
    kFadeout = 3,
    kBlitterFeedback = 4,
    kOnBeatClear = 5,
    kBlur = 6,
    kRotoBlitter = 9,
    kColorfade = 11,
    kMovement = 15,
    kBufferSave = 18,
    kBrightness = 22,
    kClearScreen = 25,
    kMirror = 26,
    kCustomBpm = 33,
    kSuperScope = 36,
    kInvert = 37,
    kSetRenderMode = 40,
    kDynamicMovement = 43,
    kFastBrightness = 44,
    kColorModifier = 45,
};

/** AVS on-disk COLORREF (0x00BBGGRR) -> host 0x00RRGGBB. */
uint32_t avsColor(int32_t c)
{
    const uint32_t u = static_cast<uint32_t>(c);
    return ((u & 0xFFu) << 16) | (u & 0xFF00u) | ((u >> 16) & 0xFFu);
}

std::string slotStr(const EffectNode& n, const char* name)
{
    const std::string_view s = n.slot(name);
    return std::string(s);
}

/** Mutable state carried across the walk (Set Render Mode unroll, decision E4). */
struct Context
{
    int lineWidth = 0;  ///< current Set-Render-Mode line width (0 = unset)
    std::vector<std::string>& report;
    int effectCount = 0;
    int passthroughCount = 0;
};

ChainNode passthrough(const EffectNode& src, const std::string& path,
                      const std::string& reason, Context& ctx)
{
    ChainNode node;
    PassthroughParams p;
    p.sourceId = src.id;
    p.note = reason;
    node.params = std::move(p);
    ctx.report.push_back(path + ": " + reason + " - passthrough");
    ++ctx.passthroughCount;
    return node;
}

ListParams listParamsFrom(const lumi::avs::ListInfo& info)
{
    ListParams lp;
    lp.clearEveryFrame = info.clearEveryFrame();
    lp.blendIn = static_cast<BlendMode>(std::clamp(info.blendIn(), 0, 13));
    lp.blendOut = static_cast<BlendMode>(std::clamp(info.blendOut(), 0, 13));
    lp.inAdjustAlpha = std::clamp(info.inBlendVal, 0, 255);
    lp.outAdjustAlpha = std::clamp(info.outBlendVal, 0, 255);
    lp.onBeatRender = info.beatRender != 0;
    lp.onBeatFrames = std::max(1, info.beatRenderFrames);
    lp.useCode = info.useCode != 0;
    lp.initCode = info.initCode;
    lp.frameCode = info.frameCode;
    return lp;
}

// Forward decl for recursion.
ChainNode translateNode(const EffectNode& src, const std::string& path, Context& ctx);

/** Map one decoded builtin leaf; returns false if not mappable here. */
bool mapBuiltin(const EffectNode& src, const std::string& path, Context& ctx,
                ChainNode& out)
{
    switch (src.id)
    {
        case kFadeout:
            out.params = FadeoutParams{src.field("fadelen"),
                                       avsColor(src.field("color"))};
            return true;

        case kInvert:
            out.enabled = src.field("enabled") != 0;
            out.params = InvertParams{};
            return true;

        case kBrightness:
        {
            BrightnessParams p;
            p.red = src.field("redp");
            p.green = src.field("greenp");
            p.blue = src.field("bluep");
            p.exclude = src.field("exclude") != 0;
            p.color = avsColor(src.field("color"));
            p.distance = src.field("distance");
            out.enabled = src.field("enabled") != 0;
            out.params = p;
            return true;
        }

        case kFastBrightness:
            out.params = FastBrightnessParams{src.field("dir")};
            return true;

        case kBlur:
        {
            const int mode = src.field("enabled");  // 0=off,1,2,3
            out.enabled = mode != 0;
            out.params = BlurParams{std::max(1, mode), src.field("roundmode") != 0};
            return true;
        }

        case kMirror:
        {
            const int mode = src.field("mode");
            MirrorParams p;
            p.leftToRight = (mode & (4 | 8)) != 0;  // VERTICAL1|VERTICAL2
            p.topToBottom = (mode & (1 | 2)) != 0;  // HORIZONTAL1|HORIZONTAL2
            p.onBeatRandom = src.field("onbeat") != 0;
            out.enabled = src.field("enabled") != 0;
            out.params = p;
            return true;
        }

        case kOnBeatClear:
            out.params = OnBeatClearParams{avsColor(src.field("color")),
                                           std::max(1, src.field("nf")),
                                           src.field("blend") != 0};
            return true;

        case kClearScreen:
            out.enabled = src.field("enabled") != 0;
            out.params = ClearParams{avsColor(src.field("color")),
                                     src.field("onlyfirst") != 0};
            return true;

        case kColorfade:
        {
            ColorfadeParams p;
            p.faderR = src.field("fader_r");
            p.faderG = src.field("fader_g");
            p.faderB = src.field("fader_b");
            p.beatFaderR = src.field("beatfader_r");
            p.beatFaderG = src.field("beatfader_g");
            p.beatFaderB = src.field("beatfader_b");
            out.enabled = src.field("enabled") != 0;
            out.params = p;
            return true;
        }

        case kColorModifier:
        {
            ColorModifierParams p;
            p.initCode = slotStr(src, "init");
            p.frameCode = slotStr(src, "frame");
            p.beatCode = slotStr(src, "beat");
            p.levelCode = slotStr(src, "level");
            p.recompute = src.field("recompute") != 0;
            out.params = std::move(p);
            return true;
        }

        case kMovement:
        {
            const std::string code = slotStr(src, "point");
            if (code.empty()) return false;  // built-in formula -> passthrough
            MovementParams p;
            p.code = code;
            p.rectCoords = src.field("rectangular") != 0;
            p.wrap = src.field("wrap") != 0;
            out.params = std::move(p);
            return true;
        }

        case kDynamicMovement:
        {
            DynamicMovementParams p;
            p.initCode = slotStr(src, "init");
            p.frameCode = slotStr(src, "frame");
            p.beatCode = slotStr(src, "beat");
            p.pointCode = slotStr(src, "point");
            p.xres = src.field("xres") > 0 ? src.field("xres") : 16;
            p.yres = src.field("yres") > 0 ? src.field("yres") : 12;
            p.rectCoords = src.field("rectcoords") != 0;
            p.wrap = src.field("wrap") != 0;
            out.params = std::move(p);
            return true;
        }

        case kBlitterFeedback:
        {
            BlitterFeedbackParams p;
            // scale mapping approximate (tune in sight test); ~1.0 = no zoom.
            p.zoom = 1.0f + static_cast<float>(src.field("scale")) / 1024.0f;
            p.beatZoom = 1.0f + static_cast<float>(src.field("scale2")) / 1024.0f;
            p.onBeat = src.field("beatch") != 0;
            p.blend = src.field("blend") != 0;
            out.params = p;
            return true;
        }

        case kRotoBlitter:
        {
            RotoBlitterParams p;
            p.zoom = 1.0f + static_cast<float>(src.field("zoom_scale")) / 1024.0f;
            p.rotationSpeed = static_cast<float>(src.field("rot_dir")) / 32.0f;
            p.blend = src.field("blend") != 0;
            out.params = p;
            return true;
        }

        case kBufferSave:
        {
            BufferSaveParams p;
            p.slot = std::clamp(src.field("which"), 0, 7);
            p.save = src.field("dir") == 0;  // 0=save, 1=restore
            const int b = src.field("blend");
            p.blend = b == 1 ? BlendMode::Additive
                             : (b == 2 ? BlendMode::FiftyFifty : BlendMode::Replace);
            p.adjustAlpha = std::clamp(src.field("adjblend_val"), 0, 255);
            out.params = p;
            return true;
        }

        case kCustomBpm:
        {
            CustomBpmParams p;
            p.arbitrary = src.field("arbitrary") != 0;
            p.arbitraryMs = std::max(1, src.field("arbval"));
            p.skip = src.field("skip") != 0;
            p.skipCount = std::max(1, src.field("skipval"));
            p.invert = src.field("invert") != 0;
            out.params = p;
            return true;
        }

        case kSuperScope:
        {
            SuperScopeParams p;
            p.initCode = slotStr(src, "init");
            p.frameCode = slotStr(src, "frame");
            p.beatCode = slotStr(src, "beat");
            p.pointCode = slotStr(src, "point");
            p.renderMode = (src.field("drawmode") & 1) ? 1 : 0;  // lines / dots
            p.audioChannel = std::clamp(src.field("which_ch"), 0, 4);
            if (ctx.lineWidth > 0)
            {
                p.lineWidth = static_cast<float>(ctx.lineWidth);  // Set-Render-Mode unroll
                p.renderMode = p.renderMode == 0 ? 0 : 1;
            }
            out.params = std::move(p);
            return true;
        }

        default:
            return false;
    }
}

ChainNode translateNode(const EffectNode& src, const std::string& path, Context& ctx)
{
    if (src.isList)
    {
        ChainNode node;
        node.params = listParamsFrom(src.list);
        node.enabled = src.list.enabled();
        for (size_t i = 0; i < src.children.size(); ++i)
        {
            node.children.push_back(
                translateNode(src.children[i], path + "/" + std::to_string(i), ctx));
        }
        return node;
    }

    // Set Render Mode: unroll into the following scopes (decision E4).
    if (src.id == kSetRenderMode)
    {
        const uint32_t packed = static_cast<uint32_t>(src.field("newmode"));
        const int width = static_cast<int>((packed >> 16) & 0xFF);
        if (width > 0) ctx.lineWidth = width;
        return passthrough(src, path, "Set Render Mode (line width " +
                                          std::to_string(width) + ") unrolled", ctx);
    }

    if (!src.decoded)
    {
        const std::string name = src.name.empty() ? "effect " + std::to_string(src.id)
                                                   : src.name;
        return passthrough(src, path, "\"" + name + "\" not decoded", ctx);
    }

    ChainNode node;
    if (mapBuiltin(src, path, ctx, node))
    {
        ++ctx.effectCount;
        return node;
    }

    const std::string name = src.name.empty() ? "effect " + std::to_string(src.id)
                                              : src.name;
    return passthrough(src, path, "\"" + name + "\" not supported yet", ctx);
}

} // namespace

TranslationResult translateAvsTree(const lumi::avs::ParseResult& parsed)
{
    TranslationResult result;

    if (!parsed.ok)
    {
        result.root.params = ListParams{};
        result.report.push_back("parse failed: " + parsed.error);
        compileChain(result.root);
        return result;
    }

    // Carry the parser's own import warnings into the report.
    for (const std::string& warning : parsed.warnings)
    {
        result.report.push_back("parser: " + warning);
    }

    Context ctx{0, result.report, 0, 0};
    result.root = translateNode(parsed.root, "root", ctx);
    // The root of a preset is always a list; guarantee it even if the parser
    // handed us something odd.
    if (!result.root.isList())
    {
        ChainNode wrapper;
        wrapper.params = ListParams{};
        wrapper.children.push_back(std::move(result.root));
        result.root = std::move(wrapper);
    }

    result.effectCount = ctx.effectCount;
    result.passthroughCount = ctx.passthroughCount;
    compileChain(result.root);
    return result;
}

} // namespace lumi::multieffect
