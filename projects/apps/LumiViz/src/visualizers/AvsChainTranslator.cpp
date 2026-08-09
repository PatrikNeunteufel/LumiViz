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

#include "scripting/ScriptBaseKeys.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>

namespace lumi::multieffect {

namespace {

using lumi::avs::EffectNode;

// AVS builtin effect ids (registration order == preset id, analysis §5.2).
enum AvsId
{
    kSimple = 0,
    kOscStar = 2,
    kText = 28,
    kBassSpin = 7,
    kRotatingStars = 13,
    kOscRing = 14,
    kPicture = 34,
    kMovingParticle = 8,
    kColorClip = 12,
    kInterleave = 23,
    kUniqueTone = 38,
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
    kAvi = 32,
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

/** AVS-Preset-Farben sind bereits Framebuffer-Format 0x00RRGGBB — KEIN
 *  COLORREF: GR_SelectColor (ref util.cpp:61-75) tauscht den Dialog-COLORREF
 *  bei Ein- UND Ausgang, gespeichert wird der FB-Wert. Beweis S46 (AvsRef):
 *  0x00FF80 rendert als RGB (0,255,128). Der fruehere R/B-Swap hier faerbte
 *  alle Importe um (Wormhole gelb -> gruen, Befund C). */
uint32_t avsColor(int32_t c)
{
    return static_cast<uint32_t>(c) & 0xFFFFFFu;
}

/** Mutable state carried across the walk (Set Render Mode unroll, decision E4). */
struct Context
{
    std::vector<std::string>& report;  ///< Probleme — rechtfertigen einen Dialog
    std::vector<std::string>& notes;   ///< planmaessige Hinweise (S51)
    int effectCount = 0;
    int passthroughCount = 0;
};

bool isIdentChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

/// Traegt der Knoten diesen Namen irgendwo in einem seiner Code-Slots?
bool nodeMentions(const EffectNode& n, std::string_view word)
{
    for (const char* slot : {"init", "frame", "beat", "point", "level"})
    {
        const std::string_view src = n.slot(slot);
        for (std::size_t i = 0; i + word.size() <= src.size(); ++i)
        {
            if (i > 0 && isIdentChar(src[i - 1])) continue;
            if (lumi::scripting::equalsIgnoreCase(src.substr(i, word.size()), word) &&
                (i + word.size() >= src.size() || !isIdentChar(src[i + word.size()])))
            {
                return true;
            }
        }
    }
    return false;
}

/// Zielname nach Schema D2: `_p`, bei Kollision mit einem im Preset schon
/// vorhandenen Namen `_p2`, `_p3` — sonst wuerden zwei Variablen verschmelzen.
std::string collisionTarget(const EffectNode& n, std::string_view word)
{
    for (int attempt = 0;; ++attempt)
    {
        std::string target = lumi::scripting::privateName(word, attempt);
        if (!nodeMentions(n, target)) return target;
    }
}

/**
 * @brief Import-Kollisionsregel D2 auf EINEN Code-Slot anwenden.
 *
 * Namen des Lumi-Sets ohne AVS-Builtin-Bedeutung (`vol`, `bass`, `time`, …) sind
 * in einem AVS-Preset gewoehnliche Preset-Variablen. Weil die Injektions-Schicht
 * sie je Frame setzt, wuerde ein Preset-eigener Zustand darin jeden Frame
 * verloren gehen — "Alien Alloy" fuehrt in `vol` einen Tiefpass, der die
 * Swirl-Staerke treibt (Befund S51). Sie werden deshalb beim Import auf `_p`
 * umbenannt, einheitlich ueber ALLE Slots der Komponente.
 *
 * Das Ziel haengt nur am vollstaendigen Slot-Satz des Knotens, nicht an der
 * Aufrufreihenfolge — jeder Slot bekommt damit dieselbe Zuordnung.
 */
std::string applyKeyCollisionRule(const EffectNode& n, std::string_view code)
{
    std::string out;
    out.reserve(code.size());
    for (std::size_t i = 0; i < code.size();)
    {
        if (!isIdentChar(code[i]) || (i > 0 && isIdentChar(code[i - 1])) ||
            std::isdigit(static_cast<unsigned char>(code[i])) != 0)
        {
            out += code[i++];
            continue;
        }
        std::size_t end = i;
        while (end < code.size() && isIdentChar(code[end])) ++end;
        const std::string_view word = code.substr(i, end - i);
        if (!lumi::scripting::collidesOnAvsImport(word))
        {
            out += word;
            i = end;
            continue;
        }
        // Freies Ziel suchen: `_p`, dann `_p2`, `_p3` — der Name darf im Preset
        // nicht schon vorkommen, sonst wuerden zwei Variablen verschmelzen.
        out += collisionTarget(n, word);
        i = end;
    }
    return out;
}

/// Eine ℹ-Zeile je umbenanntem Namen (Konzept §4: "Sichtbar"). Getrennt vom
/// Umbenennen, damit alle 42 slotStr-Aufrufstellen ohne Kontext auskommen.
void reportKeyCollisions(const EffectNode& n, const std::string& path, Context& ctx)
{
    for (const lumi::scripting::BaseKey& key : lumi::scripting::kInjectedKeys)
    {
        if (key.origin == lumi::scripting::KeyOrigin::Avs) continue;
        if (!nodeMentions(n, key.name)) continue;
        ctx.notes.push_back(path + ": Skript-Variable '" + std::string(key.name) +
                             "' -> '" + collisionTarget(n, key.name) +
                             "' (in AVS kein Builtin, Kollision mit dem Lumi-Set;"
                             " Entscheid D2)");
    }
}

/// EEL-Slots laufen durch die Kollisionsregel, Nicht-Code-Felder (`filename`)
/// nicht — dort wuerde eine Umbenennung den Pfad zerstoeren.
bool isCodeSlot(const char* name)
{
    for (const char* code : {"init", "frame", "beat", "point", "level"})
    {
        if (std::strcmp(name, code) == 0) return true;
    }
    return false;
}

std::string slotStr(const EffectNode& n, const char* name)
{
    const std::string_view s = n.slot(name);
    if (!isCodeSlot(name)) return std::string(s);
    return applyKeyCollisionRule(n, s);
}

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
 * are not coordinate remaps -> nullptr; 1 and 7 map to `builtinRemap`
 * (per-pixel index remaps, dedicated shader — S44). `rect` mirrors the AVS
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
        // Two id sets exist for RGB (= identity): presets saved with the
        // ORIGINAL channelshift.ape store 1023 (its dialog resource), the
        // vis_avs builtin port stores 1183 (1023 was taken in its resource.h).
        // 1018-1022 are identical in both. The builtin's switch treats every
        // unknown id as RGB — so unknowns must fall back to 0, not get clamped.
        ChannelShiftParams p;
        switch (src.field("mode"))
        {
            case 1023:
            case 1183: p.mode = 0; break;  // RGB
            case 1020: p.mode = 1; break;  // RBG
            case 1018: p.mode = 2; break;  // GBR
            case 1022: p.mode = 3; break;  // GRB
            case 1019: p.mode = 4; break;  // BRG
            case 1021: p.mode = 5; break;  // BGR
            default:
                p.mode = (src.field("mode") >= 1000)
                             ? 0
                             : std::clamp(src.field("mode"), 0, 5);
                break;
        }
        p.onBeat = src.field("onbeat") != 0;
        out.params = p;
        return true;
    }
    if (src.apeId == "Holden03: Convolution Filter")
    {
        ConvolutionParams p;
        p.edgeMode = std::clamp(src.field("edgeMode"), 0, 1);
        p.absolute = src.field("absolute") != 0;
        p.twoPass = src.field("twoPass") != 0;
        p.bias = src.field("bias");
        p.scale = src.field("scale");
        for (int i = 0; i < 49; ++i)
            p.kernel[static_cast<std::size_t>(i)] = src.field("k" + std::to_string(i));
        out.enabled = src.field("enabled") != 0;
        out.params = std::move(p);
        return true;
    }
    if (src.apeId == "FunkyFX FyrewurX v1")
    {
        // Behavioral rebuild (closed source) with host-own parameters; every
        // known preset carries identical config bytes anyway.
        out.enabled = src.field("enabled") != 0;
        out.params = FyrewurXParams{};
        return true;
    }
    if (src.apeId == "Metaballs 3D" || src.apeId == "Tentacles 3D")
    {
        // Verhaltens-Nachbau (closed source, S52): aus dem Preset kommt NUR die
        // Farbtafel, die Geometrie ist host-eigen — wie bei FyrewurX. Die
        // Farbzahl beschneidet die 16 gelesenen Slots; ist sie unbrauchbar,
        // gilt die volle Tafel.
        out.enabled = true;
        std::vector<uint32_t> colors(src.colors.begin(), src.colors.end());
        const int used = src.field("numcolors");
        if (used > 0 && used < static_cast<int>(colors.size()))
        {
            colors.resize(static_cast<std::size_t>(used));
        }
        if (colors.empty()) colors.push_back(0xFFFFFFu);
        if (src.apeId == "Metaballs 3D")
        {
            Metaballs3DParams p;
            p.colors = colors;
            p.count = std::clamp(static_cast<int>(colors.size()), 1, 16);
            out.params = std::move(p);
        }
        else
        {
            Tentacles3DParams p;
            p.colors = colors;
            p.count = std::clamp(static_cast<int>(colors.size()), 1, 16);
            out.params = std::move(p);
        }
        return true;
    }
    if (src.apeId == "Trans: Normalise")
    {
        out.enabled = src.field("enabled") != 0;
        out.params = NormaliseParams{};
        return true;
    }
    if (src.apeId == "Jheriko : MULTIFILTER")
    {
        MultiFilterParams p;
        p.effect = std::clamp(src.field("effect"), 0, 3);
        p.onBeat = src.field("onbeat") != 0;
        out.enabled = src.field("enabled") != 0;
        out.params = p;
        return true;
    }
    if (src.apeId == "Virtual Effect: Addborders")
    {
        AddBordersParams p;
        p.color = avsColor(src.field("color"));
        p.size = std::max(0, src.field("size"));
        out.enabled = src.field("enabled") != 0;
        out.params = p;
        return true;
    }
    if (src.apeId == "Misc: Buffer blend")
    {
        BufferBlendParams p;
        p.bufferA = std::clamp(src.field("bufferA"), 0, 8);
        p.bufferB = std::clamp(src.field("bufferB"), 0, 8);
        p.mode = std::clamp(src.field("mode"), 0, 10);
        out.enabled = src.field("enabled") != 0;
        out.params = p;
        return true;
    }
    if (src.apeId == "Jheriko: Global")
    {
        JherikoGlobalParams p;
        p.loadMode = std::clamp(src.field("load"), 0, 3);
        p.initCode = slotStr(src, "init");
        p.frameCode = slotStr(src, "frame");
        p.beatCode = slotStr(src, "beat");
        out.params = std::move(p);
        return true;
    }
    if (src.apeId == "Texer")
    {
        TexerParams p;
        p.filename = slotStr(src, "filename");
        const int flags = src.field("flags");
        p.blend = (flags & 4) != 0 ? 1 : 0;  // bit 2 = additive-ish (sight-test)
        p.particles = src.field("particles") > 0 ? src.field("particles") : 100;
        out.params = std::move(p);
        return true;
    }
    if (src.apeId == "Acko.net: Texer II")
    {
        TexerIIParams p;
        p.filename = slotStr(src, "filename");
        p.resizing = src.field("resizing") != 0;
        p.wrapAround = src.field("wrapAround") != 0;
        p.colorFiltering = src.field("colorFiltering") != 0;
        p.initCode = slotStr(src, "init");
        p.frameCode = slotStr(src, "frame");
        p.beatCode = slotStr(src, "beat");
        p.pointCode = slotStr(src, "point");
        out.params = std::move(p);
        return true;
    }
    if (src.apeId == "Render: Triangle")
    {
        TriangleParams p;
        p.initCode = slotStr(src, "init");
        p.frameCode = slotStr(src, "frame");
        p.beatCode = slotStr(src, "beat");
        p.pointCode = slotStr(src, "point");
        out.params = std::move(p);
        return true;
    }
    if (src.apeId == "Picture II")
    {
        PictureIIParams p;
        p.filename = slotStr(src, "filename");
        // Die APE hat SECHS Betriebsarten, an der Referenz gemessen (S58):
        // 0 ersetzen · 1 additiv · 2 Maximum · 3 Minimum · 4 50/50 ·
        // 5 Subtraktion (Framebuffer minus Bild). Der Bildshader nummeriert
        // anders, weil 0..2 dort seit jeher `Picture` (ID 34) gehoeren.
        // Vorher fiel alles ab 2 pauschal auf 50/50 — bei „The Real
        // Impressionist" wurde aus einem Maximum ein Mittelwert.
        static constexpr int kBlendToShader[6] = {0, 1, 3, 4, 2, 5};
        const int bm = src.field("blendMode");
        p.blend = (bm >= 0 && bm < 6) ? kBlendToShader[bm] : 0;
        out.params = std::move(p);
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

    // Der `enabled`-Schalter im Effekt-Blob ist ZENTRAL, nicht je Zweig (S74).
    // In AVS liest fast jedes `load_config` ihn als erstes Feld, und `render`
    // beginnt mit `if (!enabled) return;`. Ihn je Effekt einzeln nachzutragen
    // ging schief: `Water` (Id 20) und `Scatter` (Id 22) lasen ihn nie, und wir
    // haben Effekte ausgefuehrt, die das Original ueberspringt. Gemessen an
    // `02_color extasy`, wo `Water` mit enabled=0 gespeichert ist: die Referenz
    // laesst das Bild ueber alle Frames unveraendert (Mittelwert konstant
    // 0,0997), wir verdoppelten die Helligkeit schon im ersten Frame — MAE
    // 0,332 gegen das Kalibrier-Raster.
    //
    // `hasField` statt `field`, weil ein fehlendes Feld sonst 0 liefert und
    // JEDEN Effekt ohne diesen Schalter abschalten wuerde. Zweige, die den
    // Wert als Bitfeld oder Modus lesen (Starfield, Text, ...), ueberschreiben
    // ihn danach — die Reihenfolge stimmt.
    if (src.hasField("enabled")) out.enabled = src.field("enabled") != 0;

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
            MirrorParams p;
            p.mode = src.field("mode") & 15;  // 4 directed bits, 1:1 (r_mirror)
            p.onBeatRandom = src.field("onbeat") != 0;
            p.smooth = src.field("smooth") != 0;
            p.slower = std::clamp(src.field("slower"), 1, 16);
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
        {
            out.enabled = src.field("enabled") != 0;
            // r_clear precedence: blend==2 -> line blend, blend==1 -> additive,
            // blendavg -> 50/50, else replace.
            const int b = src.field("blend");
            const int mode = b == 2 ? 3
                                    : (b == 1 ? 1 : (src.field("blendavg") != 0 ? 2 : 0));
            out.params = ClearParams{avsColor(src.field("color")),
                                     src.field("onlyfirst") != 0, mode};
            return true;
        }

        case kColorfade:
        {
            ColorfadeParams p;
            p.faderR = src.field("fader_r");
            p.faderG = src.field("fader_g");
            p.faderB = src.field("fader_b");
            p.beatFaderR = src.field("beatfader_r");
            p.beatFaderG = src.field("beatfader_g");
            p.beatFaderB = src.field("beatfader_b");
            // `enabled` ist ein BITFELD (r_colorfade): 1 = an, 2 = die Fader im
            // Beat zufaellig waehlen, 4 = langsam nachziehen. Bis S57 wurde
            // alles ausser Bit 0 verworfen — zwei Betriebsarten fielen beim
            // Import lautlos weg.
            const int bits = src.field("enabled");
            out.enabled = (bits & 1) != 0;
            p.onBeatRandom = (bits & 2) != 0;
            p.slowFade = (bits & 4) != 0;
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
                // Built-in formula: look it up by index; 1/7 are per-pixel
                // remaps (builtinRemap), only 0 "none" stays a passthrough.
                const int effect = src.field("effect");
                const MovementFormula f = movementBuiltinFormula(effect);
                if (f.code != nullptr)
                {
                    p.code = f.code;
                    p.rectCoords = f.rect;
                }
                else if (effect == 1 || effect == 7)
                {
                    p.builtinRemap = effect;
                }
                else
                {
                    return false;
                }
            }
            p.wrap = src.field("wrap") != 0;
            p.blend = src.field("blend") != 0;
            // r_trans.cpp:306-309 excludes effects 1/2/7 from subpixel tables.
            const int fxIdx = p.code.empty() ? p.builtinRemap : src.field("effect");
            p.subpixel = src.field("subpixel") != 0 && fxIdx != 1 && fxIdx != 2 &&
                         fxIdx != 7;
            p.sourceMapped = src.field("sourcemapped") & 3;
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
            p.subpixel = src.field("subpixel") != 0;
            out.params = std::move(p);
            return true;
        }

        case kSimple:
        {
            SimpleScopeParams p;
            const int effect = src.field("effect");
            // r_simple: Bit 6 = Dot-Modus (Bit 1 waehlt Scope/Analyzer),
            // sonst effect&3 = solid ana / line ana / line scope / solid scope.
            if ((effect & (1 << 6)) != 0)
                p.mode = (effect & 2) != 0 ? 5 : 4;
            else
                p.mode = effect & 3;
            p.channel = (effect >> 2) & 3;
            p.position = (effect >> 4) & 3;
            p.colors.clear();
            for (std::uint32_t c : src.colors)
                p.colors.push_back(avsColor(static_cast<std::int32_t>(c)));
            if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
            out.params = std::move(p);
            return true;
        }

        case kOscStar:
        {
            OscStarParams p;
            const int effect = src.field("effect");
            p.channel = (effect >> 2) & 3;
            p.position = (effect >> 4) & 3;
            p.size = src.field("size");
            p.rot = src.field("rot");
            p.colors.clear();
            for (std::uint32_t c : src.colors)
                p.colors.push_back(avsColor(static_cast<std::int32_t>(c)));
            if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
            out.params = std::move(p);
            return true;
        }

        case kPicture:
        {
            PictureParams p;
            p.filename = slotStr(src, "filename");
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
            p.keepAspect = src.field("ratio") != 0;
            // imageData is filled by the app-side embed pass (needs the .avs dir).
            out.enabled = src.field("enabled") != 0;
            out.params = std::move(p);
            return true;
        }

        case kOscRing:
        {
            OscRingParams p;
            const int effect = src.field("effect");
            p.channel = (effect >> 2) & 3;
            p.position = (effect >> 4) & 3;
            p.size = src.field("size");
            p.source = src.field("source") != 0 ? 1 : 0;
            p.colors.clear();
            for (std::uint32_t c : src.colors)
                p.colors.push_back(avsColor(static_cast<std::int32_t>(c)));
            if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
            out.params = std::move(p);
            return true;
        }

        case kRotatingStars:
        {
            RotatingStarsParams p;
            p.colors.clear();
            for (std::uint32_t c : src.colors)
                p.colors.push_back(avsColor(static_cast<std::int32_t>(c)));
            if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
            out.params = std::move(p);
            return true;
        }

        case kBassSpin:
        {
            BassSpinParams p;
            const int en = src.field("enabled");
            p.left = (en & 1) != 0;
            p.right = (en & 2) != 0;
            p.colorLeft = avsColor(src.field("color0"));
            p.colorRight = avsColor(src.field("color1"));
            p.mode = src.field("mode") != 0 ? 1 : 0;
            out.enabled = en != 0;
            out.params = p;
            return true;
        }

        case kColorClip:
        {
            ColorClipParams p;
            const int en = src.field("enabled");
            p.mode = en == 0 ? 1 : std::clamp(en, 1, 3);
            p.clipColor = avsColor(src.field("color_clip"));
            p.outColor = avsColor(src.field("color_clip_out"));
            p.distance = src.field("color_dist");
            out.enabled = en != 0;
            out.params = p;
            return true;
        }

        case kUniqueTone:
        {
            UniqueToneParams p;
            p.color = avsColor(src.field("color"));
            p.invert = src.field("invert") != 0;
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
            out.enabled = src.field("enabled") != 0;
            out.params = p;
            return true;
        }

        case kInterleave:
        {
            InterleaveParams p;
            p.x = std::max(0, src.field("x"));
            p.y = std::max(0, src.field("y"));
            p.color = avsColor(src.field("color"));
            p.blend = src.field("blend") != 0 ? 1 : (src.field("blendavg") != 0 ? 2 : 0);
            p.onBeat = src.field("onbeat") != 0;
            p.x2 = std::max(0, src.field("x2"));
            p.y2 = std::max(0, src.field("y2"));
            p.beatDuration = std::max(1, src.field("beatdur"));
            out.enabled = src.field("enabled") != 0;
            out.params = p;
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
            p.subpixel = src.field("subpixel") != 0;
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
            // r_dmove evaluates xres+1 grid points, min 2 (r_dmove.cpp:232-238):
            // a stored 0 legitimately means a 2-point (near-linear) grid.
            p.xres = std::clamp(src.field("xres") + 1, 2, 256);
            p.yres = std::clamp(src.field("yres") + 1, 2, 256);
            p.rectCoords = src.field("rectcoords") != 0;
            p.wrap = src.field("wrap") != 0;
            p.blend = src.field("blend") != 0;
            p.nomove = src.field("nomove") != 0;
            p.subpixel = src.field("subpixel") != 0;
            p.buffern = std::clamp(src.field("buffern"), 0, 8);
            out.params = std::move(p);
            return true;
        }

        case kBlitterFeedback:
        {
            BlitterFeedbackParams p;
            // r_blit-Felder 1:1 (S48): fpos-Ease/Formeln macht der Renderer.
            p.scale = src.field("scale");
            p.scale2 = src.field("scale2");
            p.onBeat = src.field("beatch") != 0;
            p.blend = src.field("blend") != 0;
            p.subpixel = src.field("subpixel") != 0;
            out.params = p;
            return true;
        }

        case kRotoBlitter:
        {
            RotoBlitterParams p;
            // r_rotblit-Felder 1:1 (S48): theta/zoom rechnet der Renderer.
            p.zoomScale = src.field("zoom_scale");
            p.zoomScale2 = src.field("zoom_scale2");
            p.rotDir = src.field("rot_dir");
            p.blend = src.field("blend") != 0;
            p.beatReverse = src.field("beatch") != 0;
            p.beatReverseSpeed = src.field("beatch_speed");
            p.beatZoomJump = src.field("beatch_scale") != 0;
            p.subpixel = src.field("subpixel") != 0;
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
            // r_dotgrid hat EIN blend-Feld 0..3 (3 = BLEND_LINE/SRM, S3) und
            // kein blendavg — Wert 1:1 durchreichen.
            p.blend = std::clamp(src.field("blend"), 0, 3);
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
            // Das Original speichert seine laufende Rotation mit (r = rr/32,
            // r_dotpln load_config) — ohne sie steht die Ebene dauerhaft
            // verdreht (Tie Tunnel: 1435/32 = 44,84 Grad).
            p.startRotation = static_cast<float>(src.field("r_raw")) / 32.0f;
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
            p.startRotation = static_cast<float>(src.field("r_raw")) / 32.0f;
            out.params = p;
            return true;
        }

        case kTimescope:
        {
            TimescopeParams p;
            p.color = avsColor(src.field("color"));
            // r_timescope.cpp:147-151: blend==2 ist "Default Blend" = BLEND_LINE
            // (folgt SRM, S3), blend==1 additiv, sonst blendavg -> 50/50, sonst
            // replace.
            p.blend = src.field("blend") == 2   ? 3
                      : src.field("blend") != 0 ? 1
                      : src.field("blendavg") != 0 ? 2 : 0;
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
            p.blend = src.field("blend") != 0
                          ? 1
                          : (src.field("blendavg") != 0 ? 2 : 0);
            out.enabled = src.field("enabled") != 0;
            out.params = p;
            return true;
        }

        case kText:
        {
            TextParams p;
            p.text = slotStr(src, "text");
            p.fontFace = slotStr(src, "face");
            p.fontHeight = src.field("fontHeight");
            if (p.fontHeight == 0)  // fall back to the CHOOSEFONT point size
                p.fontHeight = -std::max(8, src.field("pointSize10") * 4 / 30);
            p.fontWeight = src.field("fontWeight") > 0 ? src.field("fontWeight") : 400;
            p.italic = src.field("fontItalic") != 0;
            p.underline = src.field("fontUnderline") != 0;
            p.color = avsColor(src.field("color"));
            p.blend = src.field("blend") != 0 ? 1
                                              : (src.field("blendavg") != 0 ? 2 : 0);
            p.onBeat = src.field("onbeat") != 0;
            p.onBeatSpeed = std::max(1, src.field("onbeatspeed"));
            p.normSpeed = std::max(1, src.field("normspeed"));
            p.insertBlank = src.field("insertblank") != 0;
            p.randomPos = src.field("randompos") != 0;
            p.randomWord = src.field("randomword") != 0;
            // halign is the raw DT_* value (LEFT 0 / CENTER 1 / RIGHT 2);
            // valign uses DT_TOP 0 / DT_VCENTER 4 / DT_BOTTOM 8.
            p.hAlign = std::clamp(src.field("halign"), 0, 2);
            const int va = src.field("valign");
            p.vAlign = va == 4 ? 1 : (va == 8 ? 2 : 0);
            p.xShift = src.field("xshift");
            p.yShift = src.field("yshift");
            p.outline = src.field("outline") != 0;
            p.outlineColor = avsColor(src.field("outlinecolor"));
            p.outlineSize = std::max(1, src.field("outlinesize"));
            p.shadow = src.field("shadow") != 0;
            out.enabled = src.field("enabled") != 0;
            out.params = std::move(p);
            return true;
        }

        case kAvi:
        {
            AviParams p;
            p.filename = slotStr(src, "filename");
            p.blend = src.field("blend") != 0 ? 1
                                              : (src.field("blendavg") != 0 ? 2 : 0);
            p.adapt = src.field("adapt") != 0;
            p.persist = std::clamp(src.field("persist"), 0, 32);
            p.speedMs = std::max(0, src.field("speed"));
            out.enabled = src.field("enabled") != 0;
            out.params = std::move(p);
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
            p.buffern = std::clamp(src.field("buffern"), 0, 8);
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
            p.dir = std::clamp(src.field("dir"), 0, 3);
            // r_stack.cpp:128-231 blend codes -> host BlendMode (1=50/50! 2=add!)
            switch (src.field("blend"))
            {
                case 1: p.blend = BlendMode::FiftyFifty; break;
                case 2: p.blend = BlendMode::Additive; break;
                case 3: p.blend = BlendMode::EveryOtherPixel; break;
                case 4: p.blend = BlendMode::Subtractive12; break;
                case 5: p.blend = BlendMode::EveryOtherLine; break;
                case 6: p.blend = BlendMode::Xor; break;
                case 7: p.blend = BlendMode::Maximum; break;
                case 8: p.blend = BlendMode::Minimum; break;
                case 9: p.blend = BlendMode::Subtractive21; break;
                case 10: p.blend = BlendMode::Multiply; break;
                case 11: p.blend = BlendMode::Adjustable; break;
                default: p.blend = BlendMode::Replace; break;
            }
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
            // Rohwert durchreichen: `skipval` 0 heisst in der Referenz "jeden
            // Beat" (`++skipCount >= 0+1`). Die alte Untergrenze 1 machte
            // daraus "jeden zweiten".
            p.skipCount = std::max(0, src.field("skipval"));
            p.invert = src.field("invert") != 0;
            p.skipFirst = std::max(0, src.field("skipfirst"));
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
            p.pointCount = 100;  // AVS default *var_n=100 (init code overrides)
            p.renderMode = (src.field("drawmode") & 1) ? 1 : 0;  // lines / dots
            // r_sscope.cpp:232-240: Bit 4 = Spektrum-Quelle, Bits 0-1 = Kanal
            // (0 L, 1 R, >=2 Center) — vorher wurde which_ch=4 faelschlich als
            // LumiViz-Kanal "Side" gelesen (S44, Befund "Spektrum fehlt").
            const int whichCh = src.field("which_ch");
            const int ch = whichCh & 3;
            p.audioChannel = ch >= 2 ? 2 : ch;
            p.spectrumSource = (whichCh & 4) != 0;
            // AVS color table (COLORREF -> host RRGGBB). Point code that sets
            // red/green/blue still overrides it at render time. A preset with
            // NO colors gets AVS' default white (r_sscope ctor: 1x 0xFFFFFF) —
            // channels the script leaves untouched must start at 1.0.
            for (std::uint32_t c : src.colors)
                p.colors.push_back(avsColor(static_cast<std::int32_t>(c)));
            if (p.colors.empty()) p.colors.push_back(0xFFFFFF);
            p.colorBlend = 1;  // table mode (frame-constant base, AVS-faithful)
            // AVS default line blend is REPLACE (g_line_blend_mode starts 0) —
            // additive only when a Set Render Mode node says so.
            p.lineBlend = 0;
            // Line width/blend now come from a live Set Render Mode node at render
            // time (host render mode), not baked here.
            // Befund B (S46): r_sscope zeichnet IMMER 1-px-Bresenham (linedraw)
            // — der Chain-Default 2.0 lief ins Dreiecks-Band (~2.5x Pixel-
            // deckung, Feedback-Trails 3x zu hell). width=1 -> GL_LINE_STRIP.
            p.lineWidth = 1.0f;
            p.dotSize = 1.0f;  // AVS-Dots sind 1 px (linedraw), Chain-Default 4.0
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
        // Listen-Codes laufen ueber ListInfo, nicht ueber slotStr — dort bleibt
        // `beat` unangetastet, weil r_list es als Builtin registriert (D2).
        node.params = listParamsFrom(src.list);
        node.enabled = src.list.enabled();
        for (size_t i = 0; i < src.children.size(); ++i)
        {
            node.children.push_back(
                translateNode(src.children[i], path + "/" + std::to_string(i), ctx));
        }
        return node;
    }

    reportKeyCollisions(src, path, ctx);

    // Set Render Mode: a live state-setter node (like Custom BPM). It carries the
    // line width (bits 16-23), line blend (bits 0-7), Adjustable alpha (bits 8-15)
    // and the enable flag (bit 31); the host applies them to the render effects
    // that follow it at render time (no import-time unroll, no passthrough).
    if (src.id == kSetRenderMode)
    {
        const uint32_t packed = static_cast<uint32_t>(src.field("newmode"));
        const int blendBits = static_cast<int>(packed & 0xFF);
        ChainNode node;
        SetRenderModeParams p;
        p.enabled = (packed & 0x80000000u) != 0;
        p.lineWidth = static_cast<int>((packed >> 16) & 0xFF);
        p.adjustAlpha = static_cast<int>((packed >> 8) & 0xFF);
        // RAW BLEND_LINE-Modus (r_defs.h:267-283): 0 replace, 1 add, 2 max,
        // 3 avg, 4 sub(fb-c), 5 sub(c-fb), 6 mul, 7 adjustable, 8 xor, 9 min.
        // S9 (Session 44): nicht mehr auf 3 Host-Modi kollabieren — MAX & Co.
        // sind der Anti-Weiss-Deckel vieler Presets (Beleg ZeroG).
        p.lineBlend = std::clamp(blendBits, 0, 9);
        node.params = p;
        node.displayName = "Set Render Mode";
        return node;
    }

    // Framerate Limiter: the host owns frame pacing — import as a no-op with a
    // note (decision: no-op + Notiz), not an "unsupported" passthrough.
    if (src.apeId == "VFX FRAMERATE LIMITER")
    {
        ChainNode node;
        PassthroughParams p;
        p.sourceId = src.id;
        p.note = "Framerate Limiter";
        node.params = std::move(p);
        node.displayName = "Framerate Limiter";
        ctx.notes.push_back(path + ": Framerate Limiter ignored (host controls pacing)");
        return node;
    }

    // Comment: informational only (holds preset text). Conserve as a silent no-op
    // — no report warning, not counted as an unrendered passthrough.
    if (src.id == kComment)
    {
        // Eigener Typ mit eigenem Textfeld (Befund + Entscheid Patrik S44) —
        // NICHT in description, das zerstoert die Tabellen-Ansicht.
        ChainNode node;
        node.params = CommentParams{slotStr(src, "text")};
        node.displayName = "Comment";
        return node;
    }

    // Movement mit Effekt 0 ("none"): im Original ein NICHTS-TUN, kein fehlendes
    // Feature — `r_trans.cpp` kehrt bei `!effect` sofort zurueck. Wie beim
    // Kommentar ein stiller Durchreicher: kein Bericht, nicht als ungerenderter
    // Passthrough gezaehlt. `splendora.avs` meldete beim Import sonst einen
    // Fehler fuer einen Knoten, der gar nichts tun soll (Befund Patrik S58).
    if (src.id == kMovement && slotStr(src, "point").empty() &&
        src.field("effect") == 0)
    {
        ChainNode node;
        PassthroughParams p;
        p.sourceId = src.id;
        p.note = "Movement \"none\" — im Original ein Nichts-Tun";
        node.params = std::move(p);
        node.displayName = "Movement (none)";
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
        node.fromApe = !src.apeId.empty();
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

    Context ctx{result.report, result.notes, 0, 0};
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
