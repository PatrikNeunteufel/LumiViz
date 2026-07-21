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
#include <cstring>

namespace lumi::multieffect {

namespace {

using lumi::avs::EffectNode;

// AVS builtin effect ids (registration order == preset id, analysis §5.2).
enum AvsId
{
    kMovingParticle = 8,
    kComment = 21,
    kDynamicDistanceModifier = 35,
    kDotPlane = 1,
    kFadeout = 3,
    kBlitterFeedback = 4,
    kDotGrid = 17,
    kDotFountain = 19,
    kOnBeatClear = 5,
    kBlur = 6,
    kRotoBlitter = 9,
    kColorfade = 11,
    kMovement = 15,
    kBufferSave = 18,
    kBrightness = 22,
    kClearScreen = 25,
    kScatter = 16,
    kWater = 20,
    kGrain = 24,
    kMirror = 26,
    kStarfield = 27,
    kBump = 29,
    kMosaic = 30,
    kWaterBump = 31,
    kCustomBpm = 33,
    kSuperScope = 36,
    kInvert = 37,
    kTimescope = 39,
    kSetRenderMode = 40,
    kInterferences = 41,
    kDynamicShift = 42,
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
    int lineBlend = 1;  ///< current Set-Render-Mode line blend (0 replace,1 add,2 50/50)
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
    lp.bufferIn = std::clamp(info.bufferIn, 0, 7);
    lp.bufferOut = std::clamp(info.bufferOut, 0, 7);
    lp.bufferInInvert = info.inInvert != 0;
    lp.bufferOutInvert = info.outInvert != 0;
    lp.onBeatRender = info.beatRender != 0;
    lp.onBeatFrames = std::max(1, info.beatRenderFrames);
    lp.useCode = info.useCode != 0;
    lp.initCode = info.initCode;
    lp.frameCode = info.frameCode;
    return lp;
}

/**
 * The 23 built-in Movement formulas (r_trans.cpp `descriptions[].eval_desc`).
 * These are the exact AVS point-code strings; our ScriptGridModule now shares
 * AVS' polar convention (d normalised to corner=1, r + pi/2), so they render
 * faithfully. Index 0 (none) plus 1 (slight fuzzify) and 7 (blocky partial out)
 * are not coordinate remaps -> nullptr -> passthrough. `rect` mirrors the AVS
 * uses_rect flag; formulas that need it read/write x,y instead of d,r.
 */
struct MovementFormula
{
    const char* code;  ///< nullptr = not a remap (passthrough)
    bool rect;
};
[[nodiscard]] MovementFormula movementBuiltinFormula(int effect)
{
    static const MovementFormula kTable[] = {
        {nullptr, false},                                        // 0 none
        {nullptr, false},                                        // 1 slight fuzzify
        {"x=x+1/32;", true},                                     // 2 shift rotate left
        {"r = r + (0.1 - (0.2 * d));\nd = d * 0.96;", false},    // 3 big swirl out
        {"d = d * (0.99 * (1.0 - sin(r-$PI*0.5) / 32.0));\n"
         "r = r + (0.03 * sin(d * $PI * 4));", false},           // 4 medium swirl
        {"d = d * (0.94 + (cos((r-$PI*0.5) * 32.0) * 0.06));", false},  // 5 sunburster
        {"d = d * (1.01 + (cos((r-$PI*0.5) * 4) * 0.04));\n"
         "r = r + (0.03 * sin(d * $PI * 4));", false},           // 6 swirl to center
        {nullptr, false},                                        // 7 blocky partial out
        {"r = r + (0.1 * sin(d * $PI * 5));", false},            // 8 swirling both ways
        {"t = sin(d * $PI);\n"
         "d = d - (8*t*t*t*t*t)/sqrt((sw*sw+sh*sh)/4);", false}, // 9 bubbling outward
        {"t = sin(d * $PI);\n"
         "d = d - (8*t*t*t*t*t)/sqrt((sw*sw+sh*sh)/4);\n"
         "t=cos(d*$PI/2.0);\nr= r + 0.1*t*t*t;", false},         // 10 bubbling + swirl
        {"d = d * (0.95 + (cos(((r-$PI*0.5) * 5.0) - ($PI / 2.50)) * 0.03));", false},  // 11
        {"r = r + 0.04;\nd = d * (0.96 + cos(d * $PI) * 0.05);", false},  // 12 tunneling
        {"t = cos(d * $PI);\nr = r + (0.07 * t);\n"
         "d = d * (0.98 + t * 0.10);", false},                   // 13 bleedin'
        {"d=sqrt(x*x+y*y); r=atan2(y,x);\n"
         "r=r+0.1-0.2*d; d=d*0.96;\n"
         "x=cos(r)*d + 8/128; y=sin(r)*d;", true},               // 14 shifted big swirl
        {"d = 0.15", false},                                     // 15 psychotic beaming
        {"r = cos(r * 3)", false},                               // 16 cosine radial 3-way
        {"d = d * (1 - ((d - .35) * .5));\nr = r + .1;", false}, // 17 spinny tube
        {"d = d * (1 - (sin((r-$PI*0.5) * 7) * .03));\n"
         "r = r + (cos(d * 12) * .03);", false},                 // 18 radial swirlies
        {"d = d * (1 - (sin((r - $PI*0.5) * 12) * .05));\n"
         "r = r + (cos(d * 18) * .05);\n"
         "d = d * (1-((d - .4) * .03));\n"
         "r = r + ((d - .4) * .13)", false},                     // 19 swill
        {"x = x + (cos(y * 18) * .02);\n"
         "y = y + (sin(x * 14) * .03);", true},                  // 20 gridley
        {"x = x + (cos(abs(y-.5) * 8) * .02);\n"
         "y = y + (sin(abs(x-.5) * 8) * .05);\n"
         "x = x * .95;\ny = y * .95;", true},                    // 21 grapevine
        {"y = y * ( 1 + (sin(r + $PI/2) * .3) );\n"
         "x = x * ( 1 + (cos(r + $PI/2) * .3) );\n"
         "x = x * .995;\ny = y * .995;", true},                  // 22 quadrant
        {"y = (r*6)/($PI); x = d;", true},                       // 23 6-way kaleida
    };
    if (effect < 0 || effect > 23) return {nullptr, false};
    return kTable[effect];
}

// Forward decl for recursion.
ChainNode translateNode(const EffectNode& src, const std::string& path, Context& ctx);

/** Map one decoded builtin leaf; returns false if not mappable here. */
/** Map a compiled-in builtin APE (dispatched by id string). */
bool mapApe(const EffectNode& src, ChainNode& out)
{
    if (src.apeId == "Channel Shift")
    {
        // Stored mode is a Windows IDC id; map it to our 0..5 permutation index.
        ChannelShiftParams p;
        switch (src.field("mode"))
        {
            case 1183: p.mode = 0; break;  // RGB
            case 1020: p.mode = 1; break;  // RBG
            case 1018: p.mode = 2; break;  // GBR
            case 1022: p.mode = 3; break;  // GRB
            case 1019: p.mode = 4; break;  // BRG
            case 1021: p.mode = 5; break;  // BGR
            default: p.mode = std::clamp(src.field("mode"), 0, 5); break;
        }
        p.onBeat = src.field("onbeat") != 0;
        out.params = p;
        return true;
    }
    if (src.apeId == "Color Map")
    {
        ColorMapParams p;
        p.key = std::clamp(src.field("key"), 0, 5);
        p.blendMode = std::clamp(src.field("blendMode"), 0, 9);
        p.adjustBlend = std::clamp(src.field("adjustBlend"), 0, 255);
        const int count = src.field("cmcount");
        for (int k = 0; k < count && k < static_cast<int>(src.colors.size()); ++k)
        {
            p.stopPos.push_back(
                std::clamp(src.field("cmpos" + std::to_string(k)), 0, 255));
            p.stopColor.push_back(src.colors[static_cast<std::size_t>(k)]);  // 0x00RRGGBB
        }
        out.params = std::move(p);
        return true;
    }
    if (src.apeId == "Color Reduction")
    {
        out.params = ColorReductionParams{std::clamp(src.field("levels"), 1, 8)};
        return true;
    }
    if (src.apeId == "Multiplier")
    {
        out.params = MultiplierParams{std::clamp(src.field("ml"), 0, 7)};
        return true;
    }
    if (src.apeId == "Holden04: Video Delay")
    {
        VideoDelayParams p;
        p.useBeats = src.field("usebeats") != 0;
        p.delay = std::clamp(src.field("delay"), 1, 128);
        out.params = p;
        return true;
    }
    if (src.apeId == "Holden05: Multi Delay")
    {
        MultiDelayParams p;
        p.mode = std::clamp(src.field("mode"), 0, 2);
        p.buffer = std::clamp(src.field("activebuffer"), 0, 5);
        const std::string sub = std::to_string(p.buffer);
        p.useBeats = src.field(("ub" + sub).c_str()) != 0;
        p.delay = std::clamp(src.field(("dl" + sub).c_str()), 1, 128);
        out.params = p;
        return true;
    }
    return false;
}

bool mapBuiltin(const EffectNode& src, const std::string& path, Context& ctx,
                ChainNode& out)
{
    if (!src.apeId.empty() && mapApe(src, out)) return true;
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
            MovementParams p;
            const std::string code = slotStr(src, "point");
            if (!code.empty())
            {
                // User expression (AVS effect id 32767).
                p.code = code;
                p.rectCoords = src.field("rectangular") != 0;
            }
            else
            {
                // Built-in formula: look it up by index; nullptr -> passthrough.
                const MovementFormula f = movementBuiltinFormula(src.field("effect"));
                if (f.code == nullptr) return false;
                p.code = f.code;
                p.rectCoords = f.rect;
            }
            p.wrap = src.field("wrap") != 0;
            out.params = std::move(p);
            return true;
        }

        case kDynamicDistanceModifier:
        {
            DynamicDistanceModifierParams p;
            p.initCode = slotStr(src, "init");
            p.frameCode = slotStr(src, "frame");
            p.beatCode = slotStr(src, "beat");
            p.pixelCode = slotStr(src, "point");
            p.blend = src.field("blend") != 0;
            p.bilinear = src.field("subpixel") != 0;
            out.params = std::move(p);
            return true;
        }

        case kMovingParticle:
        {
            MovingParticleParams p;
            p.color = avsColor(src.field("colors"));
            p.maxDistance = src.field("maxdist");
            p.size = src.field("size");
            p.size2 = src.field("size2");
            p.onBeatSize = (src.field("enabled") & 2) != 0;
            p.blend = src.field("blend");
            out.enabled = (src.field("enabled") & 1) != 0;  // AVS enabled bit 0
            out.params = std::move(p);
            return true;
        }

        case kDynamicShift:
        {
            DynamicShiftParams p;
            p.initCode = slotStr(src, "init");
            p.frameCode = slotStr(src, "frame");
            p.beatCode = slotStr(src, "beat");
            p.blend = src.field("blend") != 0;
            p.bilinear = src.field("subpixel") != 0;
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

        case kGrain:
        {
            GrainParams p;
            p.amount = std::clamp(src.field("smax"), 0, 100);
            p.staticGrain = src.field("staticgrain") != 0;
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
            out.params = p;
            return true;
        }

        case kScatter:
            out.params = ScatterParams{};
            return true;

        case kWater:
            out.params = WaterParams{};
            return true;

        case kDotGrid:
        {
            DotGridParams p;
            p.colors.clear();
            for (std::uint32_t c : src.colors)
                p.colors.push_back(avsColor(static_cast<std::int32_t>(c)));
            if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
            p.spacing = std::max(2, src.field("spacing"));
            p.xMove = src.field("x_move");
            p.yMove = src.field("y_move");
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
            out.params = std::move(p);
            return true;
        }

        case kDotPlane:
        {
            DotPlaneParams p;
            for (std::size_t i = 0; i < src.colors.size() && i < 5; ++i)
                p.colors[i] = avsColor(static_cast<std::int32_t>(src.colors[i]));
            p.rotVel = src.field("rotvel");
            p.angle = src.field("angle");
            out.params = p;
            return true;
        }

        case kDotFountain:
        {
            DotFountainParams p;
            for (std::size_t i = 0; i < src.colors.size() && i < 5; ++i)
                p.colors[i] = avsColor(static_cast<std::int32_t>(src.colors[i]));
            p.rotVel = src.field("rotvel");
            p.angle = src.field("angle");
            out.params = p;
            return true;
        }

        case kTimescope:
        {
            TimescopeParams p;
            p.color = avsColor(src.field("color"));
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
            p.channel = std::clamp(src.field("which_ch"), 0, 2);
            p.bands = src.field("nbands") > 0 ? std::clamp(src.field("nbands"), 1, 576) : 576;
            out.params = p;
            return true;
        }

        case kStarfield:
        {
            auto bitsToFloat = [](std::int32_t bits, float def) {
                float v = 0.0f;
                std::memcpy(&v, &bits, sizeof(float));
                return (v > 0.0f && v < 1000.0f) ? v : def;
            };
            StarfieldParams p;
            p.color = avsColor(src.field("color"));
            p.warpSpeed = bitsToFloat(src.field("warpSpeed_bits"), 6.0f);
            p.maxStars = src.field("maxStars") > 0
                             ? std::clamp(src.field("maxStars"), 1, 8192)
                             : 350;
            p.onBeat = src.field("onbeat") != 0;
            p.beatSpeed = bitsToFloat(src.field("beatSpeed_bits"), 4.0f);
            p.durationFrames = std::max(1, src.field("durFrames"));
            out.params = p;
            return true;
        }

        case kWaterBump:
        {
            WaterBumpParams p;
            p.density = std::clamp(src.field("density"), 1, 12);
            p.depth = src.field("depth");
            p.randomDrop = src.field("random_drop") != 0;
            p.dropX = std::clamp(src.field("drop_x"), 0, 2);
            p.dropY = std::clamp(src.field("drop_y"), 0, 2);
            p.dropRadius = std::max(1, src.field("drop_radius"));
            out.params = p;
            return true;
        }

        case kBump:
        {
            BumpParams p;
            p.depth = std::clamp(src.field("depth"), 0, 100);
            p.depth2 = std::clamp(src.field("depth2"), 0, 100);
            p.onBeat = src.field("onbeat") != 0;
            p.durationFrames = std::max(1, src.field("durFrames"));
            p.invert = src.field("invert") != 0;
            p.oldStyle = src.field("oldstyle") != 0;
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
            p.initCode = slotStr(src, "init");
            p.frameCode = slotStr(src, "frame");
            p.beatCode = slotStr(src, "beat");
            out.params = std::move(p);
            return true;
        }

        case kInterferences:
        {
            InterferencesParams p;
            p.points = std::clamp(src.field("nPoints"), 1, 8);
            p.distance = src.field("distance");
            p.alpha = std::clamp(src.field("alpha"), 0, 255);
            p.rotation = src.field("rotation");
            p.rotationInc = src.field("rotationinc");
            p.distance2 = src.field("distance2");
            p.alpha2 = std::clamp(src.field("alpha2"), 0, 255);
            p.rotationInc2 = src.field("rotationinc2");
            p.rgb = src.field("rgb") != 0;
            p.onBeat = src.field("onbeat") != 0;
            const std::int32_t bits = src.field("speed_bits");
            float speed = 0.0f;
            std::memcpy(&speed, &bits, sizeof(float));
            p.speed = (speed > 0.0f && speed < 100.0f) ? speed : 0.2f;  // sanity
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
            out.params = p;
            return true;
        }

        case kMosaic:
        {
            MosaicParams p;
            p.quality = std::clamp(src.field("quality"), 1, 100);
            p.quality2 = std::clamp(src.field("quality2"), 1, 100);
            p.onBeat = src.field("onbeat") != 0;
            p.durationFrames = std::max(1, src.field("durFrames"));
            // AVS stores two flags; host uses one mode (0 replace,1 add,2 50/50).
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
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
            // AVS color table (COLORREF -> host RRGGBB). Point code that sets
            // red/green/blue still overrides it at render time.
            for (std::uint32_t c : src.colors)
                p.colors.push_back(avsColor(static_cast<std::int32_t>(c)));
            p.colorBlend = p.colors.empty() ? 0 : 1;  // table mode iff AVS had colors
            p.lineBlend = ctx.lineBlend;  // from a preceding Set Render Mode
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

    // Set Render Mode: unroll into the following scopes (decision E4). Carries
    // both the line width (bits 16-23) and the line blend mode (bits 0-7); bit 31
    // enables it. The blend maps onto the SuperScope framebuffer blend.
    if (src.id == kSetRenderMode)
    {
        const uint32_t packed = static_cast<uint32_t>(src.field("newmode"));
        const int width = static_cast<int>((packed >> 16) & 0xFF);
        const int blendBits = static_cast<int>(packed & 0xFF);
        const bool enabled = (packed & 0x80000000u) != 0;
        if (width > 0) ctx.lineWidth = width;
        // AVS line blend -> host (0 replace, 1 additive, 2 50/50); default additive.
        if (enabled)
            ctx.lineBlend = blendBits == 0 ? 0 : (blendBits == 3 ? 2 : 1);
        else
            ctx.lineBlend = 1;
        return passthrough(src, path,
                           "Set Render Mode (line width " + std::to_string(width) +
                               ", blend " + std::to_string(ctx.lineBlend) +
                               ") unrolled",
                           ctx);
    }

    // Comment: informational only (holds preset text). Conserve as a silent no-op
    // — no report warning, not counted as an unrendered passthrough.
    if (src.id == kComment)
    {
        ChainNode node;
        PassthroughParams p;
        p.sourceId = src.id;
        p.note = "Comment";
        node.params = std::move(p);
        node.displayName = "Comment";
        return node;
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

    Context ctx{0, 1, result.report, 0, 0};
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
