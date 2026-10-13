/**
 ****************************************************************************************
 * @file   ScriptVariables.cpp
 * @brief  Die Variablentabellen je Knotentyp (SSOT, s. ScriptVariables.hpp)
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Texte auf Englisch — sie erscheinen unveraendert in der Referenz des Panels.
 * Die Zeilen folgen den Bindungen in `MultiEffectVisualizer::run*`; die
 * Bereiche stammen aus den Feldkommentaren in `EffectChain.hpp`.
 ****************************************************************************************
 */

#include "visualizers/multieffect/ScriptVariables.hpp"
#include "visualizers/multieffect/FieldBindings.hpp"  // Block F (F5): generierte Zeilen

#include <cmath>
#include <deque>
#include <set>
#include <sstream>

#include <type_traits>
#include <unordered_map>
#include <variant>

namespace lumi::multieffect {

namespace {

/// Index eines Typs in der Variante — zur Uebersetzungszeit.
template <class T, class V>
struct IndexOf;
template <class T, class... Ts>
struct IndexOf<T, std::variant<Ts...>>
{
    static constexpr std::size_t value = [] {
        std::size_t i = 0;
        (void)((std::is_same_v<T, Ts> ? true : (++i, false)) || ...);
        return i;
    }();
};
template <class T>
constexpr std::size_t ix = IndexOf<T, EffectParams>::value;

constexpr ScriptVarBase kNone = ScriptVarBase::None;
constexpr ScriptVarBase kAudio = ScriptVarBase::Audio;
constexpr ScriptVarBase kParam = ScriptVarBase::ParamScript;

// Wiederkehrende Beschreibungen.
constexpr const char* kBlend3 = "0 replace, 1 additive, 2 50/50";
constexpr const char* kBlend4 = "0 replace, 1 additive, 2 50/50, 3 line blend (Set Render Mode)";
// Block B (S79): Farben als drei Kanaele 0..1; geklemmt wird erst beim Zeichnen.

const std::unordered_map<std::size_t, ScriptVarTable>& tables()
{
    static const std::unordered_map<std::size_t, ScriptVarTable> kTables = {
        // ---------------------------------------------------------------- Struktur
        {ix<ListParams>,
         {kAudio,
          {{"enabled", "in/out", "0/1", "render the list this frame?"},
           {"clear", "in/out", "0/1", "clear the list buffer first?"},
           {"beat", "in/out", "0/1", "beat (may be mutated for the children)"},
           {"alphain / alphaout", "in/out", "0..255", "blend alphas (adjustable modes)"},
           {"w / h", "in", "px", "surface size"}}}},
        {ix<HostGroupParams>,
         {kParam,
          {{"outadjustalpha", "in/out", "0..255",
            "output opacity of the group (Blend Out = Adjustable); runs after the "
            "children, sees the group's registers"}}}},
        // Host Switcher (S82): eigener Skriptlauf (runHostSwitchers) — Init /
        // Frame / Beat / Song; switch/nextgroup gelten nur fuer den Frame.
        {ix<HostSwitcherParams>,
         {kParam,
          {{"index", "in", "1..count", "the running group (position in the list)"},
           {"count", "in", ">=0", "number of groups in the list"},
           {"seconds", "in", "s",
            "time since the last switch — 0 right after a switch, so the Frame slot "
            "can tell that a switch just happened"},
           {"beatcount", "in", ">=0",
            "beats since the last switch — 0 right after a switch (e.g. "
            "nextgroup=above(beatcount,31) switches every 32 beats)"},
           {"switch", "out", "1..count", "switch to this group now (0 = no change)"},
           {"nextgroup", "out", "0/1",
            "1 = go to the next group (list order or random, see Order)"}}}},
        {ix<ClearParams>,
         {kParam,
          {{"blend", "in/out", "0..3", kBlend4}, {"color_r / color_g / color_b", "in/out", "0..1", "clear colour"}}}},
        {ix<FadeoutParams>,
         {kParam,
          {{"fadelen", "in/out", "0..92", "fade step per frame"}, {"color_r / color_g / color_b", "in/out", "0..1", "fade target colour"}}}},
        {ix<OnBeatClearParams>,
         {kParam,
          {{"everynbeats", "in/out", ">=1", "clear on every N-th beat"}, {"color_r / color_g / color_b", "in/out", "0..1", "clear colour"}}}},
        {ix<BufferSaveParams>,
         {kParam,
          {{"slot", "in/out", "0..7", "global buffer index"},
           {"dir", "in/out", "0..3",
            "0 save, 1 restore, 2 alternate (save first), 3 alternate (restore first)"},
           {"adjustalpha", "in/out", "0..255", "adjustable blend alpha"}}}},
        {ix<BufferBlendParams>,
         {kParam,
          {{"buffera / bufferb", "in/out", "0..8", "pool slot 0..7, 8 = current frame"},
           {"mode", "in/out", "0..10", "blend mode"}}}},
        {ix<CustomBpmParams>,
         {kParam,
          {{"arbitraryms", "in/out", "ms", "beat interval in arbitrary mode"},
           {"skipcount", "in/out", ">=0", "let every (skipcount+1)-th beat through"},
           {"skipfirst", "in/out", ">=0", "swallow the first N beats"}}}},
        {ix<SetRenderModeParams>,
         {kParam,
          {{"linewidth", "in/out", "0..255", "line width for the following scopes (0 = leave)"},
           {"lineblend", "in/out", "0..9", "line blend mode (Replace … Minimum)"},
           {"adjustalpha", "in/out", "0..255", "alpha of the adjustable blend"},
           {"overrideblend", "in/out", "0/1", "set the line blend at all?"}}}},
        {ix<JherikoGlobalParams>,
         {kAudio,
          {{"reg00..reg99", "in/out", "any", "preset-global registers (shared across effects)",
            false},
           {"gmegabuf(i)", "in/out", "any", "global scratch buffer", false}}}},
        {ix<VideoDelayParams>,
         {kParam, {{"delay", "in/out", "frames", "delay amount (capped for VRAM)"}}}},
        {ix<MultiDelayParams>,
         {kParam,
          {{"delay", "in/out", "frames", "delay length"},
           {"buffer", "in/out", "0..5", "shared buffer index"}}}},

        // ---------------------------------------------------------------- GPU-Module
        {ix<ShadertoyParams>,
         {kParam,
          {{"blend", "in/out", "0..2", kBlend3},
           {"<own uniform>", "in/out", "per type", "every uniform the image or buffer code declares itself (uniform float wobble; -> wobble; vecN: _x/_y/_z/_w; clash: in_<name>)", false}}}},
        {ix<MeshWarpParams>,
         {kParam,
          {{"gridx / gridy", "in/out", "2..256 / 2..192", "grid quads (coarse = edgy warp)"},
           {"mixamount", "in/out", "0..1", "share of the warped image (1 = replace)"},
           {"<own uniform>", "in/out", "per type", "every uniform the code declares itself (uniform float wobble; -> wobble; vecN: _x/_y/_z/_w; clash: in_<name>)", false}}}},
        {ix<GpuParticlesParams>,
         {kParam,
          {{"spawnx / spawny", "in/out", "0..1", "spawn centre (0 = left / bottom)"},
           {"spread", "in/out", "UV", "spawn scatter radius (0 = point source)"},
           {"speed", "in/out", "UV/s", "start speed"},
           {"dir", "in/out", "deg", "start direction (0 = right, 90 = up)"},
           {"fan", "in/out", "deg", "scatter angle around the direction (360 = radial)"},
           {"gravx / gravy", "in/out", "UV/s^2", "gravity"},
           {"drag", "in/out", "1/s", "air drag (0 = none)"},
           {"life", "in/out", "s", "lifetime"},
           {"sizeendfactor", "in/out", ">=0", "size factor at end of life (1 = constant)"},
           {"lifejitter", "in/out", "0..1", "lifetime scatter per particle"},
           {"<own uniform>", "in/out", "per type", "every uniform the force code declares itself (uniform float wobble; -> wobble; vecN: _x/_y/_z/_w; clash: in_<name>)", false},
           {"size", "in/out", "px", "sprite size at spawn"},
           {"colorstart_r / colorstart_g / colorstart_b", "in/out", "0..1", "colour at spawn"},
           {"colorend_r / colorend_g / colorend_b", "in/out", "0..1", "colour at end of life"}}}},
        {ix<PixelFilterParams>,
         {kParam,
          {{"mixamount", "in/out", "0..1", "share of the filtered image (1 = replace)"},
           {"<own uniform>", "in/out", "per type", "every uniform the code declares itself (uniform float wobble; -> wobble; vecN: _x/_y/_z/_w; clash: in_<name>)", false}}}},
        {ix<IsfFilterParams>,
         {kParam,
          {{"<input key>", "in/out", "per input",
            "every ISF input under its key (vec2: _x/_y, colour: _r/_g/_b/_a; "
            "clash with a host name: in_<key>)",
            false},
           {"blend", "in/out", "0..2", kBlend3},
           {"mixamount", "in/out", "0..1", "share of the result (1 = replace)"}}}},
        {ix<VideoSourceParams>,
         {kParam,
          {{"speed", "in/out", "0.05..20", "playback speed factor"},
           {"opacity", "in/out", "0..1", "opacity (scales the contribution in all blend modes)"}}}},

        // ---------------------------------------------------------------- Scopes & Quellen
        {ix<SuperScopeParams>,
         {kNone,
          {{"n", "in/out", "8..4096", "point count (frame code may change it)"},
           {"i", "in", "0..1", "point index, normalised (per point)"},
           {"v", "in", "-1..1", "waveform sample at this point"},
           {"b", "in", "0/1", "1 on a beat"},
           {"w / h", "in", "px", "surface width / height"},
           {"t", "own", "any", "your own accumulator (persists)"},
           {"x / y", "out", "-1..1", "point position (per point)"},
           {"red / green / blue", "in/out", "0..1", "point colour (pre-seeded)"},
           {"linesize", "in/out", "px", "line width"},
           {"drawmode", "in/out", "0/1", "0 dots, 1 lines"},
           {"skip", "out", "0/1", ">0.5 hides this point"}}}},
        {ix<SimpleScopeParams>,
         {kParam,
          {{"mode", "in/out", "0..3",
            "0 solid analyzer, 1 line analyzer, 2 line scope, 3 solid scope"},
           {"channel", "in/out", "0..2", "0 left, 1 right, 2 centre"},
           {"position", "in/out", "0..2", "0 top, 1 bottom, 2 centre"}}}},
        {ix<OscStarParams>,
         {kParam,
          {{"size", "in/out", "0..16", "spoke length"},
           {"rot", "in/out", "any", "rotation speed (0 = still)"},
           {"spokes", "in/out", ">=1", "number of spokes"},
           {"rotscale", "in/out", "rad", "radians per rot unit and frame"},
           {"amplitude", "in/out", "~1", "factor on the wave deflection"}}}},
        {ix<OscRingParams>,
         {kParam,
          {{"size", "in/out", "0..16", "base radius"},
           {"segments", "in/out", ">=3", "ring vertices"},
           {"basescale", "in/out", "~0.1", "radius without audio"},
           {"audioscale", "in/out", "~0.9", "audio share of the radius"}}}},
        {ix<RotatingStarsParams>,
         {kParam,
          {{"points", "in/out", ">=2", "tips per star"},
           {"skip", "in/out", ">=1", "vertex step when drawing (2 = pentagram)"},
           {"stars", "in/out", ">=1", "stars on opposite orbits"},
           {"rotspeed", "in/out", "rad", "orbit rotation per frame"},
           {"orbit", "in/out", "NDC", "distance from the centre"},
           {"baseradius", "in/out", "NDC", "star size without audio"},
           {"audiogain", "in/out", "NDC", "growth from the spectrum peak"}}}},
        {ix<BassSpinParams>,
         {kParam,
          {{"mode", "in/out", "0/1", "0 lines, 1 filled"},
           {"smoothing", "in/out", "0..1", "weight of the new value"},
           {"spinstep", "in/out", "rad", "rotation step per unit"},
           {"colorleft_r / colorleft_g / colorleft_b", "in/out", "0..1", "left shape colour"},
           {"colorright_r / colorright_g / colorright_b", "in/out", "0..1", "right shape colour"}}}},
        {ix<MovingParticleParams>,
         {kParam,
          {{"size / size2", "in/out", "px", "particle radius / on-beat radius"},
           {"maxdistance", "in/out", "any", "travel-radius scale"},
           {"spring", "in/out", "~0.004", "pull towards the target per frame"},
           {"damping", "in/out", "0..1", "velocity damping"},
           {"color_r / color_g / color_b", "in/out", "0..1", "particle colour"}}}},
        {ix<StarfieldParams>,
         {kParam,
          {{"maxstars", "in/out", "count", "star count"},
           {"warpspeed", "in/out", "any", "base fly-through speed"},
           {"beatspeed", "in/out", "any", "on-beat speed"},
           {"durationframes", "in/out", "frames", "ease-back length"},
           {"color_r / color_g / color_b", "in/out", "0..1", "star tint (white = untinted)"}}}},
        {ix<FyrewurXParams>,
         {kParam,
          {{"sparks", "in/out", "count", "sparks per burst"},
           {"speed", "in/out", "NDC/s", "initial spark speed"},
           {"gravity", "in/out", "NDC/s^2", "downward pull"},
           {"life", "in/out", "s", "spark lifetime"},
           {"dotsize", "in/out", "px", "spark size"},
           {"huedrift", "in/out", "0..1", "colour scatter per spark"},
           {"burstspread", "in/out", "~1", "scatter of the burst centre"}}}},
        {ix<Metaballs3DParams>,
         {kParam,
          {{"count", "in/out", "1..16", "number of balls"},
           {"radius", "in/out", "NDC", "radius scale per ball"},
           {"speed", "in/out", "any", "orbit speed"},
           {"threshold", "in/out", "~1", "iso value of the surface"},
           {"spread", "in/out", "~1", "orbit width factor"},
           {"depth", "in/out", "any", "camera distance of the orbit centre"},
           {"phase", "in/out", "rad", "phase offset per ball"}}}},
        {ix<Tentacles3DParams>,
         {kParam,
          {{"count", "in/out", "1..16", "number of tentacles"},
           {"segments", "in/out", ">=2", "vertices per tentacle"},
           {"length", "in/out", "NDC", "length"},
           {"thickness", "in/out", "px", "line width at the root"},
           {"speed", "in/out", "any", "sway speed"},
           {"sway", "in/out", "any", "sway amplitude at the tip"},
           {"waves", "in/out", "any", "waves along the length"},
           {"taper", "in/out", "0..1", "taper towards the tip (0 = even)"}}}},
        {ix<TimescopeParams>,
         {kParam,
          {{"bands", "in/out", "count", "vertical spectrum resolution"},
           {"channel", "in/out", "0..2", "0 left, 1 right, 2 centre"},
           {"blend", "in/out", "0..3", kBlend4},
           {"color_r / color_g / color_b", "in/out", "0..1", "column colour"}}}},
        {ix<DotGridParams>,
         {kParam,
          {{"spacing", "in/out", "px", "grid spacing"},
           {"xmove / ymove", "in/out", "1/256 px", "scroll per frame"},
           {"blend", "in/out", "0..3", kBlend4}}}},
        {ix<DotPlaneParams>,
         {kParam,
          {{"rotvel", "in/out", "-50..50", "rotation speed"},
           {"angle", "in/out", "deg", "viewing tilt"},
           {"camdistance", "in/out", "any", "camera distance"},
           {"settle", "in/out", "any", "sinking per frame"}}}},
        {ix<DotFountainParams>,
         {kParam,
          {{"rotvel", "in/out", "any", "rotation speed (sign = direction)"},
           {"angle", "in/out", "deg", "viewing tilt"}}}},

        // ---------------------------------------------------------------- Fraktale
        {ix<Fractal2DParams>,
         {kAudio,
          {{"cx / cy", "in/out", "any", "view centre (re/im)"},
           {"zoom", "in/out", ">0", "view scale (higher = closer)"},
           {"rot", "in/out", "rad", "view rotation"},
           {"jx / jy", "in/out", "-2..2", "Julia / Phoenix seed"},
           {"power", "in/out", "1..16", "Multibrot / Nova exponent"},
           {"maxiter", "in/out", "1..2048", "escape-time iteration cap"},
           {"escaper", "in/out", ">=1", "escape radius"},
           {"colorscale", "in/out", ">0", "how often the palette repeats"},
           {"colorcycle", "in/out", "1/s", "palette drift per second"},
           {"colorphase", "in/out", "any", "current palette phase (e.g. step it on beats)"},
           {"insidecolor_r / insidecolor_g / insidecolor_b", "in/out", "0..1", "colour of points inside the set"}}}},
        {ix<Fractal3DParams>,
         {kAudio,
          {{"yaw / pitch", "in/out", "rad", "camera orbit"},
           {"dist", "in/out", ">0.1", "camera distance"},
           {"power", "in/out", "1..16", "Mandelbulb exponent"},
           {"scale / fold", "in/out", "any", "Mandelbox / KIFS params"},
           {"fov", "in/out", ">0", "field-of-view scale"},
           {"juliax / juliay / juliaz / juliaw", "in/out", "any", "quaternion Julia seed"},
           {"lightyaw / lightpitch", "in/out", "rad", "key-light direction"},
           {"ambient", "in/out", "0..1", "ambient light floor"},
           {"colorscale", "in/out", ">0", "how often the palette repeats"},
           {"colorcycle", "in/out", "1/s", "palette drift per second"},
           {"colorphase", "in/out", "any", "current palette phase (e.g. step it on beats)"},
           {"background_r / background_g / background_b", "in/out", "0..1", "background colour (ray miss)"}}}},
        {ix<DomainWarpParams>,
         {kAudio,
          {{"scale", "in/out", ">0", "base frequency"},
           {"warp", "in/out", "0..8", "domain-warp strength"},
           {"speed", "in/out", "any", "time evolution"},
           {"ox / oy", "in/out", "any", "pan offset"},
           {"warpscale", "in/out", ">0", "warp-field frequency relative to base"},
           {"lacunarity", "in/out", ">1", "frequency multiplier per octave"},
           {"gain", "in/out", "0..1", "amplitude falloff per octave"},
           {"octaves", "in/out", "1..10", "fBm octave count"},
           {"colorscale", "in/out", ">0", "how often the palette repeats"},
           {"colorcycle", "in/out", "1/s", "palette drift per second"},
           {"colorphase", "in/out", "any", "current palette phase (e.g. step it on beats)"}}}},
        {ix<FractalZoomerParams>,
         {kAudio,
          {{"cx / cy", "in/out", "any", "zoom target"},
           {"zoomspeed", "in/out", "~1", "per-frame zoom factor"},
           {"rotspeed", "in/out", "rad", "per-frame rotation"},
           {"feedback", "in/out", "0..1", "feedback strength (0.5 = 50/50, 1 = freeze)"},
           {"juliax / juliay", "in/out", "any", "Julia constant (type Julia)"},
           {"colorscale", "in/out", ">0", "how often the palette repeats"},
           {"colorcycle", "in/out", "1/s", "palette drift per second"},
           {"colorphase", "in/out", "any", "current palette phase (e.g. step it on beats)"},
           {"insidecolor_r / insidecolor_g / insidecolor_b", "in/out", "0..1", "colour of points inside the set"}}}},
        {ix<LyapunovParams>,
         {kAudio,
          {{"amin / amax", "in/out", "0..4", "a-axis view range"},
           {"bmin / bmax", "in/out", "0..4", "b-axis view range"},
           {"colorscale", "in/out", ">0", "how often the palette repeats"},
           {"colorcycle", "in/out", "1/s", "palette drift per second"},
           {"colorphase", "in/out", "any", "current palette phase (e.g. step it on beats)"},
           {"negcolor_r / negcolor_g / negcolor_b", "in/out", "0..1", "colour of the stable (negative exponent) region"}}}},
        {ix<KleinianParams>,
         {kAudio,
          {{"p / q", "in/out", "3..20", "tiling symmetry {p,q} (rounded)"},
           {"morph", "in/out", "any", "tiling morph phase"},
           {"zoom", "in/out", ">0", "view scale"},
           {"rot", "in/out", "rad", "view rotation"},
           {"colorscale", "in/out", ">0", "how often the palette repeats"},
           {"colorcycle", "in/out", "1/s", "palette drift per second"},
           {"colorphase", "in/out", "any", "current palette phase (e.g. step it on beats)"}}}},
        {ix<StrangeAttractorParams>,
         {kAudio,
          {{"a / b / c / d", "in/out", "any", "map coefficients"},
           {"scale", "in/out", ">0", "size of the orbit in the frame"},
           {"rotationspeed", "in/out", "rad/s", "running rotation"},
           {"dotsize", "in/out", "1..32 px", "dot diameter"},
           {"rotation", "in/out", "rad", "extra view rotation"},
           {"color_r / color_g / color_b", "in/out", "0..1", "dot colour (without gradient)"}}}},
        {ix<FlameParams>,
         {kAudio,
          {{"variation", "in/out", "0..4",
            "0 linear, 1 sinusoidal, 2 spherical, 3 swirl, 4 horseshoe"},
           {"rotationspeed", "in/out", "rad/s", "running rotation"},
           {"dotsize", "in/out", "px", "dot diameter"},
           {"scale", "in/out", ">0", "size of the figure"},
           {"rotation", "in/out", "rad", "extra view rotation"}}}},
        {ix<ReactionDiffusionParams>,
         {kAudio,
          {{"feed", "in/out", "0..0.1", "Gray-Scott feed rate"},
           {"diffa / diffb", "in/out", ">=0", "diffusion rates of A / B"},
           {"stepsperframe", "in/out", "1..", "simulation steps per frame"},
           {"kill", "in/out", "0..0.1", "Gray-Scott kill rate"},
           {"colorscale", "in/out", ">0", "how often the palette repeats"},
           {"colorcycle", "in/out", "1/s", "palette drift per second"},
           {"colorphase", "in/out", "any", "current palette phase (e.g. step it on beats)"}}}},

        // ---------------------------------------------------------------- Bilder
        {ix<PictureParams>, {kParam, {{"blend", "in/out", "0..2", kBlend3}}}},
        {ix<PictureIIParams>, {kParam, {{"blend", "in/out", "0..2", kBlend3}}}},
        {ix<TexerParams>,
         {kParam,
          {{"particles", "in/out", "count", "sprites along the waveform"},
           {"blend", "in/out", "0..2", kBlend3}}}},
        {ix<TexerIIParams>,
         {kAudio,
          {{"n", "in/out", "count", "sprite count"},
           {"i", "in", "0..1", "point index (per point)"},
           {"v", "in", "-1..1", "waveform sample at this point"},
           {"b", "in", "0/1", "beat"},
           {"w / h", "in", "px", "surface size"},
           {"x / y", "out", "-1..1", "sprite centre"},
           {"sizex / sizey", "in/out", "scale", "sprite size"},
           {"red / green / blue", "in/out", "0..1", "sprite tint"},
           {"skip", "out", "0/1", ">0.5 hides this sprite"}}}},
        {ix<TriangleParams>,
         {kAudio,
          {{"n", "in/out", "count", "triangle count"},
           {"i", "in", "0..1", "triangle index (per point)"},
           {"b", "in", "0/1", "beat"},
           {"w / h", "in", "px", "surface size"},
           {"x1 / y1 / x2 / y2 / x3 / y3", "out", "-1..1", "the three vertices"},
           {"red / green / blue", "in/out", "0..1", "fill colour"}}}},

        // ---------------------------------------------------------------- Bewegung
        {ix<MovementParams>,
         {kNone,
          {{"d", "in/out", "0..1", "distance from centre (polar; remap it)", false},
           {"r", "in/out", "rad", "angle (polar)", false},
           {"x / y", "in/out", "-1..1", "used instead of d/r when Rect coords is on", false}}}},
        {ix<DynamicMovementParams>,
         {kNone,
          {{"d / r", "in/out", "0..1 / rad", "polar source coord (point code)"},
           {"x / y", "in/out", "-1..1", "rect source coord when Rect coords is on"},
           {"w / h", "in", "px", "surface size"},
           {"sw / sh", "in", "px", "grid size"},
           {"time / dt", "in", "s", "run time / frame time"},
           {"b", "in", "0/1", "beat"},
           {"alpha", "in/out", "0..1", "per-pixel blend (if used)"}}}},
        {ix<DynamicShiftParams>,
         {kAudio,
          {{"x / y", "in/out", "px", "global image offset"},
           {"w / h", "in", "px", "surface size"},
           {"b", "in", "0/1", "beat"},
           {"alpha", "in/out", "0..1", "50/50 blend weight"}}}},
        {ix<DynamicDistanceModifierParams>,
         {kAudio,
          {{"d", "in/out", "0..1", "ring distance (remap it)"},
           {"b", "in", "0/1", "beat"}}}},
        {ix<BlitterFeedbackParams>,
         {kParam,
          {{"scale", "in/out", "0..256", "zoom target (32 = neutral)"},
           {"scale2", "in/out", "0..256", "on-beat jump value"}}}},
        {ix<RotoBlitterParams>,
         {kParam,
          {{"zoomscale", "in/out", "any", "zoom target (31 = neutral)"},
           {"zoomscale2", "in/out", "any", "on-beat zoom"},
           {"rotdir", "in/out", "any", "rotation: (rotdir-32) degrees per frame"}}}},
        // S86: Text und AVI — alle Namen kommen aus der Feldtabelle
        {ix<TextParams>, {kParam, {}}},
        {ix<AviParams>, {kParam, {}}},
        // S89: der Lyrics-Knoten kennt die laufende Zeile — und einen eigenen
        // Slot „Line", der bei jedem Zeilenwechsel einmal laeuft
        {ix<LyricsParams>,
         {kParam,
          {{"line", "in", "0..n", "number of the running line (0 = before the first line)"},
           {"linecount", "in", "lines", "number of lines of the lyrics"},
           // S90: das laufende Wort — und der Slot „Word" je Wortwechsel
           {"word", "in", "0..n",
            "number of the word being sung in the running line (1 = first; 0 = none, or the "
            "lyrics carry no word times)"},
           {"wordcount", "in", "words", "number of words of the running line"},
           // S90: unsynchronen Text per Skript weiterruecken
           {"linetrigger", "out", "-1/0/1",
            "unsynced lyrics: step one line forward (positive) or back (negative); counts for "
            "the frame it is set in"},
           {"wordtrigger", "out", "-1/0/1",
            "unsynced lyrics: step one word forward or back; past the last word the next line "
            "begins. The first use turns on the word state (sung / current / upcoming)"}}}},
        {ix<LyricsTrailParams>,
         {kParam,
          // S91: Slot Line — einmal je gegangener Zeile
          {{"line", "in", "0..n", "number of the line that has just ended (0 = none yet)"},
           {"linecount", "in", "lines", "number of lines of the lyrics"}}}},
        {ix<MosaicParams>,
         {kParam,
          {{"quality", "in/out", "1..100", "block resolution (100 = off)"},
           {"quality2", "in/out", "1..100", "on-beat resolution"}}}},
        {ix<WaterBumpParams>,
         {kParam,
          {{"density", "in/out", "any", "wave damping (higher = longer-lived waves)"},
           {"depth", "in/out", "any", "drop amplitude"},
           {"dropradius", "in/out", "px", "drop radius"},
           {"dropx", "in/out", "0..2", "drop position: 0 left, 1 centre, 2 right"},
           {"dropy", "in/out", "0..2", "drop position: 0 top, 1 centre, 2 bottom"},
           {"displacescale", "in/out", "~1", "displacement gain"}}}},
        {ix<BumpParams>,
         {kAudio,
          {{"x / y", "in/out", "0..1", "light position"},
           {"bi", "in/out", "0..1", "bump intensity"},
           {"depth", "in/out", "0..100", "bump strength"},
           {"depth2", "in/out", "0..100", "on-beat strength"},
           {"isbeat / islbeat", "in", "-1/+1", "-1 on a beat / inside the on-beat depth window, else +1"},
           {"t", "own", "any", "your own accumulator"}}}},
        {ix<InterferencesParams>,
         {kParam,
          {{"points", "in/out", "1..8", "number of copies"},
           {"distance", "in/out", "px", "copy offset radius"},
           {"alpha", "in/out", "0..255", "per-copy weight"},
           {"rotation", "in/out", "0..255", "rotation (of a full turn)"},
           {"rotationinc", "in/out", "any", "rotation delta per frame"},
           {"distance2", "in/out", "px", "on-beat target distance"},
           {"alpha2", "in/out", "0..255", "on-beat target alpha"},
           {"rotationinc2", "in/out", "any", "on-beat target rotation delta"},
           {"speed", "in/out", "any", "beat-morph transition speed"}}}},
        {ix<MirrorParams>,
         {kParam,
          {{"mode", "in/out", "bits",
            "1 top->bottom, 2 bottom->top, 4 left->right, 8 right->left"},
           {"slower", "in/out", "1..16", "frames per ramp step"}}}},

        // ---------------------------------------------------------------- Farbe & Pixel
        {ix<BrightnessParams>,
         {kParam,
          {{"red / green / blue", "in/out", "-4096..4096", "channel brightness"},
           {"distance", "in/out", "0..255", "exclusion radius"},
           {"color_r / color_g / color_b", "in/out", "0..1", "exclusion colour"}}}},
        {ix<FastBrightnessParams>, {kParam, {{"dir", "in/out", "0..2", "0 brighter, 1 darker, 2 off"}}}},
        {ix<BlurParams>, {kParam, {{"strength", "in/out", "1..3", "1 light, 2 medium, 3 heavy"}}}},
        {ix<ColorfadeParams>,
         {kParam,
          {{"faderr / faderg / faderb", "in/out", "-32..32", "per-frame colour step"},
           {"beatfaderr / beatfaderg / beatfaderb", "in/out", "-32..32",
            "colour step inside the beat window"},
           {"onbeatframes", "in/out", "frames", "how long the beat state holds"}}}},
        {ix<ColorModifierParams>,
         {kNone,
          {{"red / green / blue", "in/out", "0..1",
            "channel value (level code maps in->out)"},
           {"time / dt", "in", "s", "run time / frame time"},
           {"b", "in", "0/1", "beat"}}}},
        {ix<ColorMapParams>,
         {kParam,
          {{"key", "in/out", "0..5", "input selector"},
           {"blendmode", "in/out", "0..9", "blend mode"},
           {"adjustblend", "in/out", "0..255", "adjustable weight"}}}},
        {ix<ColorClipParams>,
         {kParam,
          {{"distance", "in/out", "0..255", "match radius (near mode)"},
           {"clipcolor_r / clipcolor_g / clipcolor_b", "in/out", "0..1", "threshold colour"},
           {"outcolor_r / outcolor_g / outcolor_b", "in/out", "0..1", "replacement colour"}}}},
        {ix<UniqueToneParams>,
         {kParam, {{"blend", "in/out", "0..2", kBlend3}, {"color_r / color_g / color_b", "in/out", "0..1", "tone colour"}}}},
        {ix<InterleaveParams>,
         {kParam,
          {{"x / y", "in/out", "px", "stripe spacing (0 = off)"},
           {"x2 / y2", "in/out", "px", "spacing the beat jumps to"},
           {"color_r / color_g / color_b", "in/out", "0..1", "stripe colour"}}}},
        {ix<ConvolutionParams>,
         {kParam,
          {{"bias", "in/out", "any", "added to every channel after the division"},
           {"scale", "in/out", "any", "divisor of the weighted sum (0 counts as 1)"},
           {"edgemode", "in/out", "0/1", "0 extend, 1 wrap"}}}},
        {ix<MultiFilterParams>,
         {kParam,
          {{"effect", "in/out", "0..3", "0 chrome, 1 double chrome, 2 triple chrome, 3 root"}}}},
        {ix<ChannelShiftParams>, {kParam, {{"mode", "in/out", "0..5", "channel permutation"}}}},
        {ix<ColorReductionParams>, {kParam, {{"levels", "in/out", "1..8", "bit depth per channel"}}}},
        {ix<MultiplierParams>, {kParam, {{"mode", "in/out", "0..7", "multiplier mode"}}}},
        {ix<GrainParams>, {kParam, {{"amount", "in/out", "0..100", "gated pixel fraction"}}}},
        {ix<AddBordersParams>,
         {kParam, {{"size", "in/out", "px", "border width"}, {"color_r / color_g / color_b", "in/out", "0..1", "border colour"}}}},

        // ---------------------------------------------------------------- Post
        {ix<BloomParams>,
         {kParam,
          {{"intensity", "in/out", "any", "glow factor"},
           {"threshold", "in/out", "0..1", "brightness threshold before the blur"},
           {"radius", "in/out", "px", "Gauss sigma (glow pixels)"},
           {"vignettestrength", "in/out", "0..1", "edge darkening (with vignette)"}}}},

        // ---------------------------------------------------------------- 3D
        {ix<Camera3DParams>,
         {kAudio,
          {{"px / py / pz", "in/out", "world", "camera position"},
           {"tx / ty / tz", "in/out", "world", "look-at target"},
           {"fov", "in/out", "deg", "field of view"},
           {"roll", "in/out", "deg", "camera roll"},
           {"fogstart / fogend", "in/out", "world", "fog range"},
           {"fogcolor_r / fogcolor_g / fogcolor_b", "in/out", "0..1", "fog colour"},
           {"b", "in", "0/1", "beat"}}}},
        {ix<SuperScope3DParams>,
         {kAudio,
          {{"n", "in/out", "count", "point count"},
           {"i", "in", "0..1", "point index (per point)"},
           {"v", "in", "-1..1", "waveform sample at this point"},
           {"b", "in", "0/1", "beat"},
           {"w / h", "in", "px", "surface size"},
           {"x / y / z", "out", "world", "point position"},
           {"size", "in/out", "px", "point size"},
           {"falloff", "in/out", ">0", "sprite profile k in exp(-r^2 k)"},
           {"red / green / blue", "in/out", "0..1", "point colour"},
           {"skip", "out", "0/1", ">0.5 hides this point"}}}},
        // Shape (S121): die Regler kommen aus der Feldtabelle dazu; hier steht,
        // was nur das Feld Point kennt
        {ix<ShapeParams>,
         {kAudio,
          {{"i", "in", "0..1", "place on the round of the contour (per point)"},
           {"v", "in", "-1..1", "waveform sample at this place (per point)"},
           {"x / y", "in/out", "-1..1",
            "the point of the shape, before shift, size and rotation; y runs down (per point)"},
           {"contour", "in/out", "id",
            "Init / Frame / Beat: which contour is drawn (0 = all ticked); per point: the id of "
            "the contour being drawn"},
           {"red / green / blue", "in/out", "0..1", "point colour (pre-seeded with the line colour)"},
           {"skip", "out", "0/1", ">0.5 hides the line into this point"},
           {"b", "in", "0/1", "beat"},
           {"w / h", "in", "px", "surface size"}}}},
        // XY Scope (S121): die Regler kommen aus der Feldtabelle dazu
        {ix<XyScopeParams>,
         {kAudio,
          {{"i", "in", "0..1", "place of the sample in the block (per point)"},
           {"x / y", "in/out", "-1..1",
            "left and right sample, after gain and before rotate, size and shift; y runs up (per point)"},
           {"red / green / blue", "in/out", "0..1", "point colour (pre-seeded from table or gradient)"},
           {"skip", "out", "0/1", ">0.5 hides the line into this point"},
           {"b", "in", "0/1", "beat"},
           {"w / h", "in", "px", "surface size"}}}},
        {ix<Terrain3DParams>,
         {kAudio,
          {{"res", "in", "count", "grid resolution per side"},
           {"n", "in", "count", "grid points (res*res)"},
           {"rings", "in/out", "0..10", "ring-wave amplitude"},
           {"baseamp", "in/out", ">=0", "amplitude of the procedural base"},
           {"yoffset", "in/out", "world", "base height"},
           {"extent", "in/out", "world", "width / depth of the grid"},
           {"dotsize", "in/out", "world", "point size"},
           {"falloff", "in/out", ">0", "sprite profile k"},
           {"relax / flatten", "in/out", "0..1", "height relaxation / flattening"},
           {"meshcolor_r / meshcolor_g / meshcolor_b", "in/out", "0..1", "mesh line colour"},
           {"colorlow_r / colorlow_g / colorlow_b", "in/out", "0..1", "colour of low points"},
           {"colorhigh_r / colorhigh_g / colorhigh_b", "in/out", "0..1", "colour of high points"},
           {"b", "in", "0/1", "beat"},
           {"i", "in", "index", "grid index 0..n-1 (per point)"},
           {"gx / gy", "in", "0..1", "grid position (per point)"},
           {"h", "in/out", "any", "height at this point"},
           {"red / green / blue", "in/out", "0..1", "point colour"},
           {"megabuf(i)", "in/out", "any", "the height grid", false}}}},
        {ix<GlowOrbsParams>,
         {kAudio,
          {{"n", "in/out", "count", "number of orbs"},
           {"b", "in", "0/1", "beat"},
           {"i", "in", "0..1", "orb index (per orb)"},
           {"x / y / z", "in/out", "world", "orb position"},
           {"radius", "in/out", "world", "orb radius"},
           {"haloscale", "in/out", ">0", "halo radius = radius * haloscale"},
           {"halointensity", "in/out", "0..", "halo brightness (0 = off)"},
           {"falloff", "in/out", ">0", "halo profile k in exp(-r^2 k)"},
           {"sx / sy / sz", "in/out", "any", "stretch per axis"},
           {"red / green / blue", "in/out", "0..1", "core colour"},
           {"red2 / green2 / blue2", "in/out", "0..1", "halo colour"},
           {"flash", "in/out", "0..1", "flash amount"}}}},

        // ---------------------------------------------------------------- MilkDrop
        {ix<MilkdropNodeParams>,
         {kNone,
          {{"time / fps / frame", "in", "s / Hz / n", "run time, frame rate, frame counter",
            false},
           {"bass / mid / treb", "in", "~1", "band loudness relative to its long-term mean",
            false},
           {"bass_att / mid_att / treb_att", "in", "~1", "attenuated band loudness", false},
           {"zoom / rot / warp", "in/out", "any", "warp controls (per_frame)", false},
           {"cx / cy / dx / dy / sx / sy", "in/out", "any", "warp centre / shift / stretch",
            false},
           {"decay / gamma", "in/out", "0..1 / 0..8", "feedback decay, composite gamma", false},
           {"wave_a/r/g/b, wave_x/y", "in/out", "0..1", "basic wave colour / position", false},
           {"q1..q32", "in/out", "any", "frame->pixel channel (M2 contract)", false},
           {"x / y / rad / ang", "in", "0..1 / rad", "per_pixel position (rect + polar)",
            false}}}},
    };
    return kTables;
}

}  // namespace

namespace
{
namespace ft = lumi::multieffect::feldtabelle;

/// Namen einer Zeile ("a / b / c") als Menge.
void namenVon(const ScriptVar& v, std::set<std::string>& out)
{
    for (const std::string& n : splitScriptVarNames(v.names)) out.insert(n);
}

std::string bereichText(const ft::Feld& f)
{
    const auto zahl = [](double x) {
        std::ostringstream s;
        s << x;
        return s.str();
    };
    switch (f.art)
    {
        case ft::Art::Schalter: return "0/1";
        case ft::Art::Farbe: return "0..1 each";
        default: break;
    }
    if (std::isnan(f.lo) || std::isnan(f.hi)) return "any";
    return zahl(f.lo) + ".." + zahl(f.hi);
}

std::string bedeutungText(const ft::Feld& f)
{
    std::string t = "field `" + std::string(f.feld) + "`";
    switch (f.art)
    {
        case ft::Art::Aufzaehlung: t += " (index)"; break;
        case ft::Art::Ganzzahl: t += " (rounded)"; break;
        default: break;
    }
    if (f.klasse == ft::Klasse::Struktur)
        t += " — takes effect from init/song/beat only (rebuilds)";
    else if (f.klasse == ft::Klasse::Zustand)
        t += " — resets its running state when changed";
    if (!f.alias.empty()) t += "; old name `" + std::string(f.alias) + "` still works";
    return t;
}

/// Block F (F5, S84): Handtabelle + alle Felder der Feldtabelle, die der Knoten
/// ueber das Parameter-Skript bindet (Traeger ParamSkript). Handzeilen gewinnen
/// bei gleichem Namen — ihre Texte sind oft genauer. Einmal je Typ gebaut.
const ScriptVarTable* gemischt(std::size_t index)
{
    static std::unordered_map<std::size_t, ScriptVarTable> cache;
    static std::deque<std::string> texte;  // haelt die const char* der Zeilen
    const auto it = cache.find(index);
    if (it != cache.end()) return &it->second;

    const auto& hand = tables();
    const auto h = hand.find(index);
    const ft::Typ* typ = ft::typ(index);
    // F2b (S85): auch Module mit eigenem Skript-Host binden jedes Tabellenfeld
    const bool generisch = typ != nullptr && (typ->traeger == ft::Traeger::ParamSkript ||
                                              typ->traeger == ft::Traeger::EigenerHost);
    if (h == hand.end() && !generisch) return nullptr;

    ScriptVarTable t = h != hand.end() ? h->second : ScriptVarTable{ScriptVarBase::ParamScript, {}};
    if (generisch)
    {
        std::set<std::string> vorhanden;
        for (const ScriptVar& v : t.vars) namenVon(v, vorhanden);
        const auto merke = [&](std::string s) { return texte.emplace_back(std::move(s)).c_str(); };
        for (const ft::Feld& f : typ->felder)
        {
            if (ft::rendererEigen(typ->typKey, f.name)) continue;  // F2b: kein Feld im Skript
            const std::string n(f.name);
            std::vector<std::string> namen;
            if (f.art == ft::Art::Farbe)
                namen = {n + "_r", n + "_g", n + "_b"};
            else
                namen = {n};
            if (!f.alias.empty()) namen.emplace_back(f.alias);
            bool neu = false;
            for (const std::string& x : namen) neu = neu || vorhanden.count(x) == 0;
            if (!neu) continue;
            std::string liste;
            for (const std::string& x : namen) liste += (liste.empty() ? "" : " / ") + x;
            ScriptVar v{merke(liste), "in/out", merke(bereichText(f)), merke(bedeutungText(f))};
            v.feld = merke(std::string(f.feld));
            t.vars.push_back(v);
        }
    }
    return &cache.emplace(index, std::move(t)).first->second;
}
}  // namespace

const ScriptVarTable* scriptVariables(const EffectParams& params)
{
    return gemischt(params.index());
}

const std::vector<ScriptVar>& scriptVarBaseRows(ScriptVarBase base)
{
    static const std::vector<ScriptVar> kNoRows;
    static const std::vector<ScriptVar> kAudioRows = {
        {"bass / mid / treb / treble", "in", "0..~1", "spectrum thirds (low/mid/high energy)"},
        {"vol", "in", "0..1", "smoothed overall level"},
        {"beat", "in", "0/1", "1 on a detected beat"},
        {"time", "in", "seconds", "seconds since start"},
    };
    static const std::vector<ScriptVar> kParamRows = [] {
        std::vector<ScriptVar> rows = {
            {"b", "in", "0/1", "1 on a beat"},
            {"w / h", "in", "px", "surface width / height"},
        };
        rows.insert(rows.end(), kAudioRows.begin(), kAudioRows.end());
        return rows;
    }();
    switch (base)
    {
        case ScriptVarBase::Audio: return kAudioRows;
        case ScriptVarBase::ParamScript: return kParamRows;
        case ScriptVarBase::None: break;
    }
    return kNoRows;
}

bool hatSongSlot(const EffectParams& params)
{
    return std::visit([](const auto& p) { return requires { p.frameCode; }; }, params);
}

bool hatSkriptSchalter(const EffectParams& params)
{
    if (std::holds_alternative<ListParams>(params)) return false;  // eigenes useCode
    if (std::holds_alternative<MovementParams>(params)) return true;  // `code` = EEL
    return std::visit(
        [](const auto& p) {
            return requires { p.initCode; } || requires { p.frameCode; } ||
                   requires { p.beatCode; } || requires { p.pointCode; } ||
                   requires { p.pixelCode; } || requires { p.levelCode; } ||
                   requires { p.lineCode; } || requires { p.wordCode; };
        },
        params);
}

void leereSkriptSlots(EffectParams& params)
{
    if (auto* m = std::get_if<MovementParams>(&params)) m->code.clear();
    std::visit(
        [](auto& p) {
            if constexpr (requires { p.initCode; }) p.initCode.clear();
            if constexpr (requires { p.frameCode; }) p.frameCode.clear();
            if constexpr (requires { p.beatCode; }) p.beatCode.clear();
            if constexpr (requires { p.pointCode; }) p.pointCode.clear();
            if constexpr (requires { p.pixelCode; }) p.pixelCode.clear();
            if constexpr (requires { p.levelCode; }) p.levelCode.clear();
            if constexpr (requires { p.lineCode; }) p.lineCode.clear();  // S89: Lyrics
            if constexpr (requires { p.wordCode; }) p.wordCode.clear();  // S90: Lyrics
        },
        params);
}

std::vector<std::string> splitScriptVarNames(const char* names)
{
    std::vector<std::string> out;
    const std::string s(names);
    std::size_t pos = 0;
    while (pos <= s.size())
    {
        const std::size_t cut = s.find(" / ", pos);
        const std::string part = s.substr(pos, cut == std::string::npos ? std::string::npos
                                                                        : cut - pos);
        if (!part.empty()) out.push_back(part);
        if (cut == std::string::npos) break;
        pos = cut + 3;
    }
    return out;
}

}  // namespace lumi::multieffect
