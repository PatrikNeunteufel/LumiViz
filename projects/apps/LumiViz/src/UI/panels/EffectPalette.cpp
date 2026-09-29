/**
 ****************************************************************************************
 * @file   EffectPalette.cpp
 * @brief  Die Tabelle der Effekt-Palette (s. EffectPalette.hpp)
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Aus `MultiEffectPanel.cpp` herausgeloest (S79) und um Rolle, Kurzbeschreibung
 * und Originalnamen ergaenzt. Die Fabriken und ihre Begruendungen sind
 * unveraendert uebernommen.
 ****************************************************************************************
 */

#include "UI/panels/EffectPalette.hpp"

#include "UI/widgets/PresetTypeIcons.hpp"
#include "visualizers/multieffect/GpuParticlesWrapper.hpp"
#include "visualizers/multieffect/IsfFilterWrapper.hpp"
#include "visualizers/multieffect/MeshWarpWrapper.hpp"
#include "visualizers/multieffect/PixelFilterWrapper.hpp"
#include "visualizers/multieffect/ShadertoyWrapper.hpp"

#include <QObject>

#include <variant>

namespace lumi::ui {

using namespace lumi::multieffect;

namespace {

/// Fabrik fuer Knoten, die mit ihren Struct-Vorgaben starten.
template <class T>
EffectParams makeDefault()
{
    return EffectParams{T{}};
}

EffectParams makeMilkdrop()
{
    // Blanker Start (Entscheid Patrik S76): die MilkDrop-Defaults (warp=1,
    // decay=0.98, Basis-Wave an) waren als Einstieg undurchschaubar — eine
    // frische Pipeline waberte, ohne dass irgendwo Code oder Shader stand.
    // .milk-Importe und gespeicherte Ketten behalten die MilkDrop-Defaults
    // (SSOT: PresetState-Initialisierer + Serializer-Fallbacks).
    MilkdropNodeParams p;
    p.preset.warp = 0.0;
    p.preset.decay = 1.0;
    p.preset.basicWaveEnabled = false;
    p.preset.mvA = 0.0;              // 12x9-Raster weisser MV-Striche
    p.preset.kaltstartSaat = false;  // schwarzer Start statt Rauschen
    return EffectParams{p};
}

// Strang S (S65): modernes Regelwerk, Code-Einfuegen; URL-Import = S3
EffectParams makeShadertoy()
{
    ShadertoyParams p;
    p.code = lumi::shadertoy::starterShader();
    return EffectParams{p};
}

// Strang G (S69): GPU-Vertex-Arbeit, modernes Regelwerk
EffectParams makeMeshWarp()
{
    MeshWarpParams p;
    p.code = lumi::meshwarp::starterWarp();
    return EffectParams{p};
}

EffectParams makeGpuParticles()
{
    GpuParticlesParams p;
    p.forceCode = lumi::gpuparticles::starterForce();
    return EffectParams{p};
}

// Stilfilter-Strang (S70): skriptbarer Pixel-Filter, Werks-Looks (Take-On-Me-
// Comic & Co.) liegen als Voreinstellungen bei.
EffectParams makePixelFilter()
{
    PixelFilterParams p;
    p.code = lumi::pixelfilter::starterFilter();
    return EffectParams{p};
}

// ISF-Filter (S72): fuehrt Interactive-Shader-Format-Dateien unveraendert aus —
// mit BEIDEN Shader-Stufen. Ein frisch eingefuegter Knoten hat eine Bildquelle
// und ist damit ein Filter.
EffectParams makeIsfFilter()
{
    IsfFilterParams p;
    p.fragCode = lumi::isffilter::starterFragment();
    p.quellen = {{"inputImage", lumi::multieffect::isffilter::kQuelleKette}};
    return EffectParams{p};
}

constexpr Role R = Role::Render;
constexpr Role T = Role::Transform;
constexpr Role C = Role::Color;
constexpr Role K = Role::Control;

constexpr Origin Avs = Origin::Avs;
constexpr Origin Ape = Origin::Ape;
constexpr Origin Milk = Origin::MilkDrop;
constexpr Origin Lumi = Origin::Native;

}  // namespace

const std::vector<EffectType>& effectPalette()
{
    // Spalten: Name · Fabrik · Rolle · Herkunft · Kurzbeschreibung · Originalname
    static const std::vector<EffectType> kPalette = {
        {"— Structure & Control —"},
        {"Effect List", &makeDefault<ListParams>, K, Avs,
         "Container: renders its children into its own buffer and blends the result in; "
         "optional script decides per frame.",
         "Misc / Effect List"},
        {"Host Group", &makeDefault<HostGroupParams>, K, Lumi,
         "Container for a complete visual with its own runtime; crossfades between groups."},
        {"Preset as Group…", &makeDefault<HostGroupParams>, K, Lumi,
         "Loads an .avs, .milk or .lvfx preset as a Host Group: a copy you can edit, "
         "reloadable from its source file.",
         "", MilkElement::None, true},
        {"Host Switcher", &makeDefault<HostSwitcherParams>, K, Lumi,
         "Switches between Host Groups by time, on song change or by script; the "
         "groups crossfade. Does not change the saved chain."},
        {"Clear", &makeDefault<ClearParams>, R, Avs, "Fills the frame with a colour.",
         "Render / Clear screen"},
        {"Fadeout", &makeDefault<FadeoutParams>, C, Avs,
         "Fades the image step by step towards a colour.", "Trans / Fadeout"},
        {"OnBeat Clear", &makeDefault<OnBeatClearParams>, R, Avs,
         "Clears the frame to a colour every N beats.", "Render / OnBeat Clear"},
        {"Buffer Save", &makeDefault<BufferSaveParams>, K, Avs,
         "Saves the frame to, or restores it from, one of eight global buffers.",
         "Misc / Buffer Save"},
        {"Buffer Blend", &makeDefault<BufferBlendParams>, K, Ape,
         "Blends two global buffers (or the current frame) with one of eleven modes."},
        {"Custom BPM", &makeDefault<CustomBpmParams>, K, Avs,
         "Rewrites the beat signal: fixed interval, skip beats, invert.",
         "Misc / Custom BPM"},
        {"Set Render Mode", &makeDefault<SetRenderModeParams>, K, Avs,
         "Sets line width and line blend mode for the effects that follow.",
         "Misc / Set render mode"},
        {"Render Scale", &makeDefault<RenderScaleParams>, K, Lumi,
         "Renders the whole chain at a reduced internal resolution."},
        {"Global Variables", &makeDefault<JherikoGlobalParams>, K, Ape,
         "Script-only node: sets the shared registers reg00..reg99 and gmegabuf.",
         "Jheriko: Global"},
        {"Video Delay", &makeDefault<VideoDelayParams>, T, Ape,
         "Outputs the image from N frames (or beats) ago.", "Holden04: Video Delay"},
        {"Multi Delay", &makeDefault<MultiDelayParams>, T, Ape,
         "Writes to or reads from one of six shared delay buffers.",
         "Holden05: Multi Delay"},
        {"Debug Bars", &makeDefault<DebugBarsParams>, R, Lumi,
         "Audio-reactive test quad — a debugging aid."},

        {"— MilkDrop —"},
        {"Milkdrop (Preset-Pipeline)", &makeMilkdrop, R, Milk,
         "A complete MilkDrop pipeline: per-frame / per-pixel code, warp and composite "
         "shaders, waves, shapes and sprites."},

        {"— Shadertoy —"},
        {"Shadertoy (GLSL)", &makeShadertoy, R, Lumi,
         "Runs a Shadertoy GLSL shader (Image + Buffer A–D), audio-reactive."},

        {"— GPU-Module —"},
        {"Mesh Warp (GLSL)", &makeMeshWarp, T, Lumi,
         "Warps the image with a GLSL function over a vertex grid."},
        {"GPU-Partikel (Instancing)", &makeGpuParticles, R, Lumi,
         "Tens of thousands of GPU particles with a scriptable force field."},
        {"Pixel Filter (GLSL)", &makePixelFilter, C, Lumi,
         "Per-pixel GLSL filter with factory looks (e.g. comic)."},
        {"ISF Filter (Fragment + Vertex)", &makeIsfFilter, T, Lumi,
         "Runs an ISF shader file (fragment + vertex stage) as filter or generator."},

        {"— Scopes & Sources —"},
        {"Video/Kamera (Quelle)", &makeDefault<VideoSourceParams>, R, Lumi,
         "Draws frames from a video file or a camera."},
        {"SuperScope", &makeDefault<SuperScopeParams>, R, Avs,
         "Scripted scope: draws dots or lines from per-point code.", "Render / SuperScope"},
        {"Simple (Scope)", &makeDefault<SimpleScopeParams>, R, Avs,
         "The classic oscilloscope / spectrum analyser.", "Render / Simple"},
        {"Oscilliscope Star", &makeDefault<OscStarParams>, R, Avs,
         "Waveform drawn along rotating radial spokes.", "Render / Oscilliscope Star"},
        {"Ring", &makeDefault<OscRingParams>, R, Avs,
         "Waveform or spectrum drawn as a closed ring.", "Render / Ring"},
        {"Rotating Stars", &makeDefault<RotatingStarsParams>, R, Avs,
         "Stars orbiting the centre, sized by the spectrum peak.", "Render / Rotating Stars"},
        {"Bass Spin", &makeDefault<BassSpinParams>, R, Avs,
         "Two bass-reactive spinning shapes (left / right channel).", "Render / Bass Spin"},
        {"Moving Particle", &makeDefault<MovingParticleParams>, R, Avs,
         "A particle springing around the centre, growing on beats.",
         "Render / Moving Particle"},
        {"Starfield", &makeDefault<StarfieldParams>, R, Avs,
         "Fly-through starfield, faster on beats.", "Render / Starfield"},
        {"FyrewurX", &makeDefault<FyrewurXParams>, R, Ape, "Firework bursts of sparks on beats.",
         "FyrewurX"},
        {"Metaballs 3D", &makeDefault<Metaballs3DParams>, R, Ape,
         "Orbiting metaballs with a smooth 3D surface.", "Metaballs"},
        {"Tentacles 3D", &makeDefault<Tentacles3DParams>, R, Ape, "Swaying 3D tentacles.",
         "Tentacles"},
        {"Timescope", &makeDefault<TimescopeParams>, R, Avs,
         "Spectrum over time, one column per frame.", "Render / Timescope"},
        {"Dot Grid", &makeDefault<DotGridParams>, R, Avs, "A scrolling grid of dots.",
         "Render / Dot Grid"},
        {"Dot Plane", &makeDefault<DotPlaneParams>, R, Avs,
         "Spectrum as a tilted, rotating plane of dots.", "Render / Dot Plane"},
        {"Dot Fountain", &makeDefault<DotFountainParams>, R, Avs,
         "Spectrum as a rotating fountain of dots.", "Render / Dot Fountain"},

        {"— Fractals —"},
        {"Fractal 2D", &makeDefault<Fractal2DParams>, R, Lumi,
         "Escape-time fractals (Mandelbrot, Julia, Phoenix …) through a gradient."},
        {"Fractal 3D", &makeDefault<Fractal3DParams>, R, Lumi,
         "Raymarched 3D fractals (Mandelbulb, Mandelbox, KIFS)."},
        {"Domain Warp", &makeDefault<DomainWarpParams>, R, Lumi,
         "Domain-warped noise field — the plasma / nebula look."},
        {"Fractal Zoomer", &makeDefault<FractalZoomerParams>, R, Lumi,
         "Endless zoom into an escape-time fractal."},
        {"Lyapunov", &makeDefault<LyapunovParams>, R, Lumi, "The Markus-Lyapunov fractal."},
        {"Kleinian", &makeDefault<KleinianParams>, R, Lumi,
         "Hyperbolic {p,q} tiling on the Poincaré disk."},
        {"Strange Attractor", &makeDefault<StrangeAttractorParams>, R, Lumi,
         "Point cloud of a strange attractor."},
        {"Flame", &makeDefault<FlameParams>, R, Lumi, "Flame / IFS chaos-game renderer."},
        {"Reaction Diffusion", &makeDefault<ReactionDiffusionParams>, R, Lumi,
         "Gray-Scott reaction-diffusion simulation."},

        {"— Images —"},
        {"Picture", &makeDefault<PictureParams>, R, Avs, "Draws an image over the frame.",
         "Render / Picture"},
        {"Picture II", &makeDefault<PictureIIParams>, R, Ape,
         "Draws an embedded image stretched to fill the frame.", "Picture II"},
        {"Texer", &makeDefault<TexerParams>, R, Ape,
         "Sprites of an image along the waveform.", "Texer"},
        {"Texer II", &makeDefault<TexerIIParams>, R, Ape,
         "Scripted image sprites, one per point.", "Acko.net: Texer II"},
        {"Triangle", &makeDefault<TriangleParams>, R, Ape, "Scripted filled triangles.",
         "Render: Triangle"},

        {"— Motion & Distortion —"},
        {"Movement", &makeDefault<MovementParams>, T, Avs,
         "Remaps the image with a static formula or one of the built-in movements.",
         "Trans / Movement"},
        {"Dynamic Movement", &makeDefault<DynamicMovementParams>, T, Avs,
         "Scripted per-pixel remap on a grid, every frame.", "Trans / Dynamic Movement"},
        {"Dynamic Shift", &makeDefault<DynamicShiftParams>, T, Avs,
         "Scripted global offset of the image.", "Trans / Dynamic Shift"},
        {"Dynamic Distance Modifier", &makeDefault<DynamicDistanceModifierParams>, T, Avs,
         "Scripted remap of the distance from the centre.",
         "Trans / Dynamic Distance Modifier"},
        {"Blitter Feedback", &makeDefault<BlitterFeedbackParams>, T, Avs,
         "Zoom feedback: scales the image in or out every frame.", "Trans / Blitter Feedback"},
        {"Roto Blitter", &makeDefault<RotoBlitterParams>, T, Avs,
         "Rotating and zooming feedback.", "Trans / Roto Blitter"},
        {"Mosaic", &makeDefault<MosaicParams>, T, Avs, "Pixelates the image into blocks.",
         "Trans / Mosaic"},
        {"Water", &makeDefault<WaterParams>, T, Avs, "Water ripple feedback.", "Trans / Water"},
        {"Water Bump", &makeDefault<WaterBumpParams>, T, Avs,
         "Drops on a water surface displace the image.", "Trans / Water Bump"},
        {"Bump", &makeDefault<BumpParams>, T, Avs, "Bump mapping lit by a movable light.",
         "Trans / Bump"},
        {"Interferences", &makeDefault<InterferencesParams>, T, Avs,
         "Overlays rotated copies of the image.", "Trans / Interferences"},
        {"Scatter", &makeDefault<ScatterParams>, T, Avs, "Scatters pixels randomly.",
         "Trans / Scatter"},
        {"Mirror", &makeDefault<MirrorParams>, T, Avs,
         "Mirrors one half of the image onto the other.", "Trans / Mirror"},

        {"— Color & Pixel —"},
        {"Brightness", &makeDefault<BrightnessParams>, C, Avs,
         "Scales brightness per channel, with an optional exclusion colour.",
         "Trans / Brightness"},
        {"Fast Brightness", &makeDefault<FastBrightnessParams>, C, Avs,
         "Doubles or halves the brightness.", "Trans / Fast Brightness"},
        {"Blur", &makeDefault<BlurParams>, C, Avs, "Box blur in three strengths.",
         "Trans / Blur"},
        {"Invert", &makeDefault<InvertParams>, C, Avs, "Inverts the colours.", "Trans / Invert"},
        {"Colorfade", &makeDefault<ColorfadeParams>, C, Avs,
         "Cycling colour shift by channel order.", "Trans / Colorfade"},
        {"Color Modifier", &makeDefault<ColorModifierParams>, C, Avs,
         "Scripted colour curve per channel.", "Trans / Color Modifier"},
        {"Color Map", &makeDefault<ColorMapParams>, C, Ape,
         "Maps a channel through a gradient and blends the result.", "Color Map"},
        {"Color Clip", &makeDefault<ColorClipParams>, C, Avs,
         "Replaces colours above, below or near a threshold.", "Trans / Color Clip"},
        {"Unique Tone", &makeDefault<UniqueToneParams>, C, Avs,
         "Tints the image to a single hue.", "Trans / Unique Tone"},
        {"Interleave", &makeDefault<InterleaveParams>, C, Avs,
         "Overlays a stripe grid in one colour.", "Trans / Interleave"},
        {"Convolution", &makeDefault<ConvolutionParams>, C, Ape,
         "Applies a 7×7 convolution kernel (sharpen, edges, emboss …).",
         "Convolution Filter"},
        {"Normalise", &makeDefault<NormaliseParams>, C, Ape,
         "Stretches the image to the full brightness range.", "Normalise"},
        {"MultiFilter", &makeDefault<MultiFilterParams>, C, Ape,
         "Chrome and root colour filters.", "MultiFilter"},
        {"Channel Shift", &makeDefault<ChannelShiftParams>, C, Ape,
         "Swaps the R / G / B channels.", "Channel Shift"},
        {"Color Reduction", &makeDefault<ColorReductionParams>, C, Ape,
         "Reduces the colour depth per channel.", "Color Reduction"},
        {"Multiplier", &makeDefault<MultiplierParams>, C, Ape,
         "Multiplies pixel values (×8 … ×0.125).", "Multiplier"},
        {"Grain", &makeDefault<GrainParams>, C, Avs, "Adds noise grain.", "Trans / Grain"},
        {"Add Borders", &makeDefault<AddBordersParams>, C, Ape,
         "Draws a border around the frame.", "Add Borders"},

        // Lights-Etappe 1 (S48): native Post-Process-Module (kein AVS-Pendant)
        {"— Post —"},
        {"Bloom", &makeDefault<BloomParams>, C, Lumi,
         "Additive glow of the bright areas, optional vignette."},

        // Lights-Etappe 1+2 (S48): 3D-Module (Kamera-Zustand + Szene)
        {"— 3D —"},
        {"3D Camera", &makeDefault<Camera3DParams>, K, Lumi,
         "Sets the camera for the 3D modules that follow."},
        {"SuperScope 3D", &makeDefault<SuperScope3DParams>, R, Lumi,
         "Scripted points in 3D world space."},
        {"Terrain 3D", &makeDefault<Terrain3DParams>, R, Lumi, "Audio-driven heightfield terrain."},
        {"Glow Orbs", &makeDefault<GlowOrbsParams>, R, Lumi, "Glowing orbs in 3D space."},

        // Milkdrop-Node-Inhalte (N3.1/N3.3): keine Chain-Effekte — sie landen im
        // PresetState des gewaehlten Milkdrop-Nodes.
        {"— Milkdrop-Node-Inhalte —"},
        {"Custom Wave", nullptr, R, Milk,
         "Adds a custom wave to the selected Milkdrop node.", "", MilkElement::Wave},
        {"Custom Shape", nullptr, R, Milk,
         "Adds a custom shape to the selected Milkdrop node.", "", MilkElement::Shape},
        {"Sprite", nullptr, R, Milk, "Adds a sprite to the selected Milkdrop node.", "",
         MilkElement::Sprite},
    };
    return kPalette;
}

QString paletteCategoryTitle(const EffectType& header)
{
    QString t = QString::fromUtf8(header.name);
    t.remove(QChar(0x2014));  // Gedankenstrich
    return t.trimmed();
}

Origin originForParams(const EffectParams& params)
{
    static const std::vector<Origin> kByIndex = [] {
        std::vector<Origin> byIndex(std::variant_size_v<EffectParams>, Origin::Avs);
        for (const EffectType& t : effectPalette())
        {
            if (t.make == nullptr) continue;  // Koepfe + Milkdrop-Elemente
            byIndex[t.make().index()] = t.origin;
        }
        return byIndex;
    }();
    return params.index() < kByIndex.size() ? kByIndex[params.index()] : Origin::Avs;
}

// Icon-Aufloesung + Format-Icons: geteilt mit dem Import-Browser
// (UI/widgets/PresetTypeIcons.hpp) — hier nur das Origin-Mapping. APEs tragen
// das AVS-Icon: sie sind AVS-Plugins.
const QIcon& originIcon(Origin origin)
{
    switch (origin)
    {
        case Origin::Avs:
        case Origin::Ape: return presetTypeIcon(PresetIconKind::Avs);
        case Origin::MilkDrop: return presetTypeIcon(PresetIconKind::MilkDrop);
        case Origin::Native: break;
    }
    return presetTypeIcon(PresetIconKind::Native);
}

QString originText(Origin origin)
{
    switch (origin)
    {
        case Origin::Avs: return QStringLiteral("AVS");
        case Origin::Ape: return QStringLiteral("APE");
        case Origin::MilkDrop: return QStringLiteral("MilkDrop");
        case Origin::Native: break;
    }
    return QStringLiteral("LumiViz");
}

QString originToolTip(Origin origin)
{
    switch (origin)
    {
        case Origin::Avs: return QObject::tr("AVS-ported module");
        case Origin::Ape: return QObject::tr("AVS community plugin (APE), ported");
        case Origin::MilkDrop: return QObject::tr("MilkDrop-ported module");
        case Origin::Native: break;
    }
    return QObject::tr("Native LumiViz module");
}

QString roleText(Role role)
{
    switch (role)
    {
        case Role::Render: return QObject::tr("Draws");
        case Role::Transform: return QObject::tr("Transforms");
        case Role::Color: return QObject::tr("Colour");
        case Role::Control: break;
    }
    return QObject::tr("Control");
}

}  // namespace lumi::ui
