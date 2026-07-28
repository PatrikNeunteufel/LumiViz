/**
 ****************************************************************************************
 * @file   MultiEffectPanel.cpp
 * @brief  Implementation of the multi-effect chain tree editor (Roadmap 5.7b)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 0.1.0
 ****************************************************************************************
 */

#include "UI/panels/MultiEffectPanel.hpp"

#include "UI/panels/EelScriptEditing.hpp"                 // shared EEL editor toolkit
#include "UI/widgets/GradientPresetDelegate.hpp"          // gradient combo previews
#include "UI/widgets/VisualizerWidget.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"      // figure preset library (SSOT)
#include "visualizers/modules/ColorGradientModule.hpp"   // fractal palette presets
#include "visualizers/multieffect/ChainSerializer.hpp"  // effectTypeKey
#include "visualizers/multieffect/NodePresetStore.hpp"  // Voreinstellungen je Knoten
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"
#include "UI/widgets/PresetTypeIcons.hpp"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QDropEvent>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFont>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QSplitter>
#include <QStandardItemModel>
#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMutex>
#include <QMutexLocker>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <type_traits>
#include <variant>
#include <utility>

using namespace lumi::multieffect;

namespace {

// The palette of effects that "Add" can insert (name -> default params). Entries
// with make == nullptr are non-selectable category headers (grouped in the combo).
//
// Origin marks where a module comes from: Avs = ported from a Nullsoft AVS
// effect (import target), MilkDrop = ported from MilkDrop (Roadmap 6, none
// yet), Native = original LumiViz module (no AVS counterpart, e.g. the Batch-H
// fractals + Debug Bars). Shown as per-origin icon (asset/img/logo/icons);
// falls back to a text marker when the asset folder is not found at runtime.
// Default is Avs, so only the non-AVS entries carry an explicit tag.
enum class Origin
{
    Avs,
    MilkDrop,
    Native
};

/// Inhalts-Elemente eines Milkdrop-Nodes, die das Add-Dropdown anlegen kann
/// (N3.1/N3.3): kein Chain-Effekt — sie landen im PresetState des selektierten
/// Milkdrop-Nodes (Waves/Shapes capped auf 16, MD3-Superset).
enum class MilkElement
{
    None,
    Wave,
    Shape,
    Sprite
};

struct EffectType
{
    const char* name;
    EffectParams (*make)();
    Origin origin = Origin::Avs;
    MilkElement milkElement = MilkElement::None;

    [[nodiscard]] bool isHeader() const
    {
        return make == nullptr && milkElement == MilkElement::None;
    }
};

/// Milkdrop-Anzeige-Kinder (E1: Preset → Code · Waves · Shapes · Shader ·
/// Sprites): Pfad-Segmente >= dieser Basis sind SEKTIONEN des Eltern-Nodes,
/// keine Chain-Indizes — nodeAtPath liefert fuer sie bewusst nullptr
/// (Mutations-Schutz). Auf ein Sektions-Segment darf ein ELEMENT-Index folgen
/// (N3.1/N3.3): [.., kMilkSectionBase+s, i] = Element i der Sektion s.
constexpr int kMilkSectionBase = 1000000;

/// Element-Sentinel unter der Waves-Sektion: die BASIS-Waveform des Presets
/// (nWaveMode/fWaveAlpha/…). Sie existiert immer genau einmal und ist KEIN
/// Index in preset.waves — nicht loesch-/klonbar (Sichttest-Befund S43:
/// sichtbar gerenderte Welle muss im Editor auffindbar sein).
constexpr int kMilkBasisWaveElement = kMilkSectionBase - 1;

/// Baum-Pfad in Chain-Pfad + Milk-Sektion/-Element zerlegen (Sentinels).
struct MilkPathInfo
{
    QList<int> nodePath;  ///< Pfad zum Chain-Node (ohne Sentinels)
    int section = -1;     ///< 0=Code 1=Waves 2=Shapes 3=Shader 4=Sprites
    int element = -1;     ///< Element-Index innerhalb der Sektion (oder -1)
};

MilkPathInfo splitMilkPath(QList<int> p)
{
    MilkPathInfo r;
    if (p.size() >= 2 && p[p.size() - 2] >= kMilkSectionBase)
    {
        r.element = p.takeLast();
        r.section = p.takeLast() - kMilkSectionBase;
    }
    else if (!p.isEmpty() && p.last() >= kMilkSectionBase)
    {
        r.section = p.takeLast() - kMilkSectionBase;
    }
    r.nodePath = p;
    return r;
}

/// Tiefenregel-Helfer (HG1): fuehrt der Pfad DURCH oder AUF eine Host-Gruppe?
bool pathTouchesHostGroup(ChainNode& root, const QList<int>& path)
{
    ChainNode* walk = &root;
    if (walk->isHostGroup()) return true;
    for (int idx : path)
    {
        if (idx < 0 || idx >= static_cast<int>(walk->children.size())) return false;
        walk = &walk->children[static_cast<size_t>(idx)];
        if (walk->isHostGroup()) return true;
    }
    return false;
}

/// Kleinster wavecode_N/shapecode_N-Index, der noch nicht belegt ist.
template <typename Vec>
int nextFreeMilkIndex(const Vec& vec)
{
    for (int idx = 0;; ++idx)
    {
        bool used = false;
        for (const auto& e : vec)
            if (e.index == idx) { used = true; break; }
        if (!used) return idx;
    }
}

// Icon-Aufloesung + Format-Icons: geteilt mit dem Import-Browser
// (UI/widgets/PresetTypeIcons.hpp) — hier nur das Origin-Mapping.
const QIcon& originIcon(Origin origin)
{
    switch (origin)
    {
        case Origin::Avs:
            return lumi::ui::presetTypeIcon(lumi::ui::PresetIconKind::Avs);
        case Origin::MilkDrop:
            return lumi::ui::presetTypeIcon(lumi::ui::PresetIconKind::MilkDrop);
        default:
            return lumi::ui::presetTypeIcon(lumi::ui::PresetIconKind::Native);
    }
}

QString originText(Origin origin)
{
    switch (origin)
    {
        case Origin::Avs: return QStringLiteral("AVS");
        case Origin::MilkDrop: return QStringLiteral("MilkDrop");
        default: return QStringLiteral("LumiViz");
    }
}

QString originToolTip(Origin origin)
{
    switch (origin)
    {
        case Origin::Avs: return QObject::tr("AVS-ported module");
        case Origin::MilkDrop: return QObject::tr("MilkDrop-ported module");
        default: return QObject::tr("Native LumiViz module");
    }
}

const std::vector<EffectType>& effectPalette();

/// Origin of an existing chain node, resolved over the palette (variant index).
Origin originForParams(const EffectParams& params)
{
    static const std::vector<Origin> kByIndex = [] {
        std::vector<Origin> byIndex(std::variant_size_v<EffectParams>, Origin::Avs);
        for (const EffectType& t : effectPalette())
        {
            if (t.make == nullptr) continue;  // headers + Milkdrop-Elemente
            byIndex[t.make().index()] = t.origin;
        }
        return byIndex;
    }();
    return params.index() < kByIndex.size() ? kByIndex[params.index()] : Origin::Avs;
}

const std::vector<EffectType>& effectPalette()
{
    static const std::vector<EffectType> kPalette = {
        {"— Structure & Control —", nullptr},
        {"Effect List", [] { return EffectParams{ListParams{}}; }},
        {"Host Group", [] { return EffectParams{HostGroupParams{}}; }, Origin::Native},
        {"Clear", [] { return EffectParams{ClearParams{}}; }},
        {"Fadeout", [] { return EffectParams{FadeoutParams{}}; }},
        {"OnBeat Clear", [] { return EffectParams{OnBeatClearParams{}}; }},
        {"Buffer Save", [] { return EffectParams{BufferSaveParams{}}; }},
        {"Buffer Blend", [] { return EffectParams{BufferBlendParams{}}; }},
        {"Custom BPM", [] { return EffectParams{CustomBpmParams{}}; }},
        {"Set Render Mode", [] { return EffectParams{SetRenderModeParams{}}; }},
        {"Render Scale", [] { return EffectParams{RenderScaleParams{}}; }, Origin::Native},
        {"Global Variables", [] { return EffectParams{JherikoGlobalParams{}}; }},
        {"Video Delay", [] { return EffectParams{VideoDelayParams{}}; }},
        {"Multi Delay", [] { return EffectParams{MultiDelayParams{}}; }},
        {"Debug Bars", [] { return EffectParams{DebugBarsParams{}}; }, Origin::Native},

        {"— MilkDrop —", nullptr},
        {"Milkdrop (Preset-Pipeline)",
         [] { return EffectParams{MilkdropNodeParams{}}; }, Origin::MilkDrop},

        {"— Scopes & Sources —", nullptr},
        {"SuperScope", [] { return EffectParams{SuperScopeParams{}}; }},
        {"Simple (Scope)", [] { return EffectParams{SimpleScopeParams{}}; }},
        {"Oscilliscope Star", [] { return EffectParams{OscStarParams{}}; }},
        {"Ring", [] { return EffectParams{OscRingParams{}}; }},
        {"Rotating Stars", [] { return EffectParams{RotatingStarsParams{}}; }},
        {"Bass Spin", [] { return EffectParams{BassSpinParams{}}; }},
        {"Moving Particle", [] { return EffectParams{MovingParticleParams{}}; }},
        {"Starfield", [] { return EffectParams{StarfieldParams{}}; }},
        {"FyrewurX", [] { return EffectParams{FyrewurXParams{}}; }},
        {"Metaballs 3D", [] { return EffectParams{Metaballs3DParams{}}; }},
        {"Tentacles 3D", [] { return EffectParams{Tentacles3DParams{}}; }},
        {"Timescope", [] { return EffectParams{TimescopeParams{}}; }},
        {"Dot Grid", [] { return EffectParams{DotGridParams{}}; }},
        {"Dot Plane", [] { return EffectParams{DotPlaneParams{}}; }},
        {"Dot Fountain", [] { return EffectParams{DotFountainParams{}}; }},

        {"— Fractals —", nullptr},
        {"Fractal 2D", [] { return EffectParams{Fractal2DParams{}}; }, Origin::Native},
        {"Fractal 3D", [] { return EffectParams{Fractal3DParams{}}; }, Origin::Native},
        {"Domain Warp", [] { return EffectParams{DomainWarpParams{}}; }, Origin::Native},
        {"Fractal Zoomer", [] { return EffectParams{FractalZoomerParams{}}; }, Origin::Native},
        {"Lyapunov", [] { return EffectParams{LyapunovParams{}}; }, Origin::Native},
        {"Kleinian", [] { return EffectParams{KleinianParams{}}; }, Origin::Native},
        {"Strange Attractor", [] { return EffectParams{StrangeAttractorParams{}}; }, Origin::Native},
        {"Flame", [] { return EffectParams{FlameParams{}}; }, Origin::Native},
        {"Reaction Diffusion", [] { return EffectParams{ReactionDiffusionParams{}}; }, Origin::Native},

        {"— Images —", nullptr},
        {"Picture", [] { return EffectParams{PictureParams{}}; }},
        {"Picture II", [] { return EffectParams{PictureIIParams{}}; }},
        {"Texer", [] { return EffectParams{TexerParams{}}; }},
        {"Texer II", [] { return EffectParams{TexerIIParams{}}; }},
        {"Triangle", [] { return EffectParams{TriangleParams{}}; }},

        {"— Motion & Distortion —", nullptr},
        {"Movement", [] { return EffectParams{MovementParams{}}; }},
        {"Dynamic Movement", [] { return EffectParams{DynamicMovementParams{}}; }},
        {"Dynamic Shift", [] { return EffectParams{DynamicShiftParams{}}; }},
        {"Dynamic Distance Modifier", [] { return EffectParams{DynamicDistanceModifierParams{}}; }},
        {"Blitter Feedback", [] { return EffectParams{BlitterFeedbackParams{}}; }},
        {"Roto Blitter", [] { return EffectParams{RotoBlitterParams{}}; }},
        {"Mosaic", [] { return EffectParams{MosaicParams{}}; }},
        {"Water", [] { return EffectParams{WaterParams{}}; }},
        {"Water Bump", [] { return EffectParams{WaterBumpParams{}}; }},
        {"Bump", [] { return EffectParams{BumpParams{}}; }},
        {"Interferences", [] { return EffectParams{InterferencesParams{}}; }},
        {"Scatter", [] { return EffectParams{ScatterParams{}}; }},
        {"Mirror", [] { return EffectParams{MirrorParams{}}; }},

        {"— Color & Pixel —", nullptr},
        {"Brightness", [] { return EffectParams{BrightnessParams{}}; }},
        {"Fast Brightness", [] { return EffectParams{FastBrightnessParams{}}; }},
        {"Blur", [] { return EffectParams{BlurParams{}}; }},
        {"Invert", [] { return EffectParams{InvertParams{}}; }},
        {"Colorfade", [] { return EffectParams{ColorfadeParams{}}; }},
        {"Color Modifier", [] { return EffectParams{ColorModifierParams{}}; }},
        {"Color Map", [] { return EffectParams{ColorMapParams{}}; }},
        {"Color Clip", [] { return EffectParams{ColorClipParams{}}; }},
        {"Unique Tone", [] { return EffectParams{UniqueToneParams{}}; }},
        {"Interleave", [] { return EffectParams{InterleaveParams{}}; }},
        {"Convolution", [] { return EffectParams{ConvolutionParams{}}; }},
        {"Normalise", [] { return EffectParams{NormaliseParams{}}; }},
        {"MultiFilter", [] { return EffectParams{MultiFilterParams{}}; }},
        {"Channel Shift", [] { return EffectParams{ChannelShiftParams{}}; }},
        {"Color Reduction", [] { return EffectParams{ColorReductionParams{}}; }},
        {"Multiplier", [] { return EffectParams{MultiplierParams{}}; }},
        {"Grain", [] { return EffectParams{GrainParams{}}; }},
        {"Add Borders", [] { return EffectParams{AddBordersParams{}}; }},

        // Lights-Etappe 1 (S48): native Post-Process-Module (kein AVS-Pendant)
        {"— Post —", nullptr},
        {"Bloom", [] { return EffectParams{BloomParams{}}; }, Origin::Native},

        // Lights-Etappe 1+2 (S48): 3D-Module (Kamera-Zustand + Szene)
        {"— 3D —", nullptr},
        {"3D Camera", [] { return EffectParams{Camera3DParams{}}; }, Origin::Native},
        {"SuperScope 3D", [] { return EffectParams{SuperScope3DParams{}}; }, Origin::Native},
        {"Terrain 3D", [] { return EffectParams{Terrain3DParams{}}; }, Origin::Native},
        {"Glow Orbs", [] { return EffectParams{GlowOrbsParams{}}; }, Origin::Native},
        // Milkdrop-Node-Inhalte (N3.1/N3.3): keine Chain-Effekte — "+" legt sie
        // im PresetState des selektierten Milkdrop-Nodes an.
        {"— Milkdrop-Node-Inhalte —", nullptr},
        {"Custom Wave", nullptr, Origin::MilkDrop, MilkElement::Wave},
        {"Custom Shape", nullptr, Origin::MilkDrop, MilkElement::Shape},
        {"Sprite", nullptr, Origin::MilkDrop, MilkElement::Sprite},
    };
    return kPalette;
}

QColor colorFromU32(uint32_t c)
{
    return QColor(static_cast<int>((c >> 16) & 0xFF), static_cast<int>((c >> 8) & 0xFF),
                  static_cast<int>(c & 0xFF));
}
uint32_t u32FromColor(const QColor& c)
{
    return (static_cast<uint32_t>(c.red()) << 16) |
           (static_cast<uint32_t>(c.green()) << 8) | static_cast<uint32_t>(c.blue());
}

/// Startordner des Bild-Dialogs: der in den Einstellungen gesetzte Suchordner
/// (SSOT-Schluessel `import/imageSearchDir`, dort auch vom Import benutzt),
/// sonst der zuletzt benutzte bzw. das Heimverzeichnis.
QString imageStartDir()
{
    QSettings settings;
    const QString dir =
        settings.value(QStringLiteral("import/imageSearchDir")).toString();
    if (!dir.isEmpty() && QFileInfo::exists(dir)) return dir;
    return QDir::homePath();
}

// Die SuperScope-Figuren stehen seit S53 als Dateien in
// `asset/nodepresets/superScope/` und laufen ueber die allgemeine
// Voreinstellungs-Zeile — die Liste und ihr Dropdown sind hier entfallen.

// Vorlagen fuer SuperScope 3D + 3D Camera (S48, Lights-Etappe 1) — kleine
// EEL-Startpunkte im Stil der SuperScope-Figuren. SSOT hier im Panel: anders
// als beim SuperScope steht kein Standalone-Modul mit Preset-Bibliothek
// dahinter. Welt-Konvention: x+ rechts, y+ oben, z+ zum Betrachter;
// Fallback-Kamera bei z=+3,732 sieht y in [-1,1] bei z=0.
struct Scope3DPresetDef
{
    const char* name;
    const char* init;
    const char* frame;
    const char* beat;
    const char* point;
    int pointCount;
};

const std::vector<Scope3DPresetDef>& scope3dPresets()
{
    static const std::vector<Scope3DPresetDef> kList = {
        {"Spiral Galaxy", "n=700", "t=t+0.008", "",
         "d=i*2.5;a=i*25+t;x=sin(a)*d;z=cos(a)*d;y=v*0.3;"
         "size=0.03+0.05*(1-i);red=1;green=0.6+0.4*sin(t+i*6);blue=1-i*0.5",
         700},
        {"Sphere Cloud", "n=500", "t=t+0.005", "",
         "y=(i*2-1)*1.2;rad=sqrt(abs(1.44-y*y));th=i*97+t;"
         "x=rad*cos(th);z=rad*sin(th);size=0.05;"
         "red=0.5+0.5*sin(th);green=0.7;blue=1",
         500},
        {"Wave Ribbon", "n=256", "t=t+0.01", "",
         "x=(i*2-1)*1.6;y=v*0.6;z=sin(i*6.28319+t)*0.5;size=0.04;"
         "red=0.4+0.6*i;green=1-i;blue=1",
         256},
        {"Star Flight", "n=400", "t=t+0.015+bass*0.05", "",
         "sx=sin(i*127.1)*43758.55;sx=sx-floor(sx);"
         "sy=sin(i*269.5)*24634.63;sy=sy-floor(sy);"
         "x=(sx*2-1)*3;y=(sy*2-1)*2;zz=i*8+t;zz=zz-floor(zz/8)*8;z=zz-5;"
         "size=0.05;red=0.8+0.2*sy;green=0.9;blue=1",
         400},
        {"Beat Pulse Orb", "n=350;r=1", "t=t+0.01;r=r+(1-r)*0.1", "r=1.6",
         "y=i*2-1;rad=sqrt(abs(1-y*y));a=i*88+t*2;"
         "x=rad*cos(a)*r;z=rad*sin(a)*r;y=y*r;size=0.04+treb*0.05;"
         "red=1;green=0.4+0.6*(r-1);blue=0.3",
         350},
        {"Lissajous 3D", "n=512", "t=t+0.006", "",
         "a=i*6.28319;x=sin(a*3+t)*1.2;y=sin(a*4+t*1.3)*0.9;"
         "z=sin(a*5+t*0.7)*1.2;size=0.04+v*0.03;"
         "red=0.5+0.5*sin(a+t);green=0.5+0.5*sin(a*2);blue=1",
         512},
        {"Spectrum Ring", "n=180", "t=t+0.004", "",
         "a=i*6.28319;s=getspec(i*0.7,0.05,0);x=cos(a+t)*(1+s);"
         "z=sin(a+t)*(1+s);y=s*1.5-0.4;size=0.05+s*0.1;"
         "red=0.2+s*2;green=0.4+0.6*s;blue=1-s",
         180},
    };
    return kList;
}

struct Camera3DPresetDef
{
    const char* name;
    const char* init;
    const char* frame;
    const char* beat;
};

const std::vector<Camera3DPresetDef>& camera3dPresets()
{
    static const std::vector<Camera3DPresetDef> kList = {
        {"Orbit", "",
         "a=time*0.4;px=sin(a)*4;pz=cos(a)*4;py=1.2;tx=0;ty=0;tz=0", ""},
        {"Beat Zoom", "d=3.732", "d=d+(3.732-d)*0.08;pz=d", "d=2.6"},
        {"Handheld Sway", "",
         "px=sin(time*0.5)*0.4;py=0.3+sin(time*0.33)*0.25;"
         "pz=3.732+sin(time*0.21)*0.3;tx=sin(time*0.27)*0.2", ""},
        {"Fly Forward (Fog)", "fogstart=2;fogend=9",
         "pz=3.732-sin(time*0.2)*3;py=0.5;tz=pz-4;roll=sin(time*0.3)*8", ""},
        {"Bass Shake", "",
         "px=sin(time*13)*bass*0.3;py=cos(time*17)*bass*0.2;pz=3.732", ""},
        {"Top-Down Spin", "",
         "px=0;py=4.5;pz=0;tx=0;ty=0;tz=0;roll=time*40", ""},
    };
    return kList;
}

QVariantList pathToVariant(const QList<int>& path)
{
    QVariantList v;
    for (int i : path) v.append(i);
    return v;
}
QList<int> pathFromVariant(const QVariant& v)
{
    QList<int> path;
    for (const QVariant& e : v.toList()) path.append(e.toInt());
    return path;
}

// =============================================================================
// Script editing: shared toolkit (EelScriptEditing.hpp, extracted Session 40)
// =============================================================================

using lumi::scriptedit::EelHighlighter;
using lumi::scriptedit::builtinsHtml;
using lumi::scriptedit::fnRow;
using lumi::scriptedit::fnTable;
using lumi::scriptedit::highlightLegendHtml;
using lumi::scriptedit::openScriptEditor;
using lumi::scriptedit::refRow;
using lumi::scriptedit::showScriptReference;

// Detect an `initCode` member generically, so nodeInitCode covers every effect
// type that has one without a per-type switch.
template <class T, class = void>
struct HasInit : std::false_type {};
template <class T>
struct HasInit<T, std::void_t<decltype(std::declval<const T&>().initCode)>>
    : std::true_type {};

QString nodeInitCode(const EffectParams& params)
{
    return std::visit(
        [](auto&& p) -> QString {
            using T = std::decay_t<decltype(p)>;
            if constexpr (HasInit<T>::value) return QString::fromStdString(p.initCode);
            else return QString();
        },
        params);
}

// Preset-global names (regNN / qN) written in a node's init code.
void addInitGlobalWrites(const QString& init, QSet<QString>& out)
{
    static const QRegularExpression re("\\b(reg\\d\\d|q\\d+)\\s*=(?![=<>])");
    auto it = re.globalMatch(init);
    while (it.hasNext()) out.insert(it.next().captured(1));
}

// Collect init-declared globals of every node EXCEPT `skip` (recursive).
void collectInitGlobalsExcept(const ChainNode& node, const ChainNode* skip,
                              QSet<QString>& out)
{
    if (&node != skip) addInitGlobalWrites(nodeInitCode(node.params), out);
    for (const ChainNode& child : node.children)
        collectInitGlobalsExcept(child, skip, out);
}


// Module-specific variable table (accurate for the modules wired so far).
QString scriptReferenceHtml(const EffectParams& params)
{
    const char* title = effectTypeName(params);
    QString vars;
    auto table = [](const QString& rows) {
        return QStringLiteral(
                   "<table cellspacing='0' cellpadding='4' "
                   "style='border-collapse:collapse'>"
                   "<tr><th align='left'>Name</th><th align='left'>Type</th>"
                   "<th align='left'>Range</th><th align='left'>Meaning</th></tr>%1</table>")
            .arg(rows);
    };
    // Shared by every Batch-H fractal: audio + time inputs.
    const QString audioIn =
        refRow("bass / mid / treble", "in", "0..~1", "spectrum thirds (low/mid/high energy)") +
        refRow("vol", "in", "0..1", "smoothed overall level") +
        refRow("beat", "in", "0/1", "1 on a detected beat") +
        refRow("time", "in", "seconds", "seconds since start");

    if (std::holds_alternative<SuperScopeParams>(params))
        vars = table(
            refRow("n", "in/out", "8..4096", "point count (frame code may change it)") +
            refRow("i", "in", "0..1", "point index, normalised (per point)") +
            refRow("v", "in", "-1..1", "waveform sample at this point") +
            refRow("b", "in", "0/1", "1 on a beat") +
            refRow("w / h", "in", "px", "surface width / height") +
            refRow("t", "in/out", "any", "your own accumulator (persists)") +
            refRow("x / y", "out", "-1..1", "point position (per point)") +
            refRow("red / green / blue", "in/out", "0..1", "point colour (pre-seeded)") +
            refRow("skip", "out", "0/1", ">0.5 hides this point"));
    else if (std::holds_alternative<Fractal2DParams>(params))
        vars = table(audioIn + refRow("cx / cy", "in/out", "any", "view centre (re/im)") +
                     refRow("zoom", "in/out", ">0", "view scale (higher = closer)") +
                     refRow("rot", "in/out", "rad", "view rotation") +
                     refRow("jx / jy", "in/out", "-2..2", "Julia / Phoenix seed") +
                     refRow("power", "in/out", "1..16", "Multibrot / Nova exponent"));
    else if (std::holds_alternative<Fractal3DParams>(params))
        vars = table(audioIn + refRow("yaw / pitch", "in/out", "rad", "camera orbit") +
                     refRow("dist", "in/out", ">0.1", "camera distance") +
                     refRow("power", "in/out", "1..16", "Mandelbulb exponent") +
                     refRow("scale / fold", "in/out", "any", "Mandelbox / KIFS params"));
    else if (std::holds_alternative<DomainWarpParams>(params))
        vars = table(audioIn + refRow("scale", "in/out", ">0", "base frequency") +
                     refRow("warp", "in/out", "0..8", "domain-warp strength") +
                     refRow("speed", "in/out", "any", "time evolution") +
                     refRow("ox / oy", "in/out", "any", "pan offset"));
    else if (std::holds_alternative<FractalZoomerParams>(params))
        vars = table(audioIn + refRow("cx / cy", "in/out", "any", "zoom target") +
                     refRow("zoomspeed", "in/out", "~1", "per-frame zoom factor") +
                     refRow("rotspeed", "in/out", "rad", "per-frame rotation"));
    else if (std::holds_alternative<LyapunovParams>(params))
        vars = table(audioIn + refRow("amin / amax", "in/out", "0..4", "a-axis view range") +
                     refRow("bmin / bmax", "in/out", "0..4", "b-axis view range"));
    else if (std::holds_alternative<KleinianParams>(params))
        vars = table(audioIn + refRow("morph", "in/out", "any", "tiling morph phase") +
                     refRow("zoom", "in/out", ">0", "view scale") +
                     refRow("rot", "in/out", "rad", "view rotation"));
    else if (std::holds_alternative<StrangeAttractorParams>(params))
        vars = table(audioIn + refRow("a / b / c / d", "in/out", "any", "map coefficients") +
                     refRow("rotation", "in/out", "rad", "extra view rotation"));
    else if (std::holds_alternative<FlameParams>(params))
        vars = table(audioIn + refRow("rotation", "in/out", "rad", "extra view rotation"));
    else if (std::holds_alternative<ReactionDiffusionParams>(params))
        vars = table(audioIn + refRow("feed", "in/out", "0..0.1", "Gray-Scott feed rate") +
                     refRow("kill", "in/out", "0..0.1", "Gray-Scott kill rate"));
    else if (std::holds_alternative<MilkdropNodeParams>(params))
        vars = table(
            audioIn +
            refRow("bass_att / mid_att / treb_att", "in", "~1", "attenuated band loudness") +
            refRow("zoom / rot / warp", "in/out", "any", "warp controls (per_frame)") +
            refRow("cx / cy / dx / dy / sx / sy", "in/out", "any", "warp centre / shift / stretch") +
            refRow("decay / gamma", "in/out", "0..1 / 0..8", "feedback decay, composite gamma") +
            refRow("wave_a/r/g/b, wave_x/y", "in/out", "0..1", "basic wave colour / position") +
            refRow("q1..q32", "in/out", "any", "frame→pixel channel (M2 contract)") +
            refRow("x / y / rad / ang", "in", "0..1 / rad", "per_pixel position (rect + polar)"));
    else if (std::holds_alternative<MovementParams>(params))
        vars = table(refRow("d", "in/out", "0..1", "distance from centre (polar; remap it)") +
                     refRow("r", "in/out", "rad", "angle (polar)") +
                     refRow("x / y", "in/out", "-1..1", "used instead of d/r when Rect coords is on"));
    else if (std::holds_alternative<DynamicMovementParams>(params))
        vars = table(refRow("d / r", "in/out", "0..1 / rad", "polar source coord (point code)") +
                     refRow("x / y", "in/out", "-1..1", "rect source coord when Rect coords is on") +
                     refRow("w / h", "in", "px", "surface size") +
                     refRow("b", "in", "0/1", "beat") +
                     refRow("alpha", "out", "0..1", "per-pixel blend (if used)"));
    else if (std::holds_alternative<DynamicShiftParams>(params))
        vars = table(refRow("x / y", "out", "px", "global image offset") +
                     refRow("w / h", "in", "px", "surface size") +
                     refRow("b", "in", "0/1", "beat") +
                     refRow("alpha", "out", "0..1", "50/50 blend weight"));
    else if (std::holds_alternative<DynamicDistanceModifierParams>(params))
        vars = table(refRow("d", "in/out", "0..1", "ring distance (remap it)") +
                     refRow("b", "in", "0/1", "beat"));
    else if (std::holds_alternative<BumpParams>(params))
        vars = table(refRow("x / y", "out", "0..1", "light position") +
                     refRow("t", "in/out", "any", "your own accumulator"));
    else if (std::holds_alternative<TexerIIParams>(params))
        vars = table(refRow("n", "in/out", "count", "sprite count") +
                     refRow("i", "in", "0..1", "point index (per point)") +
                     refRow("x / y", "out", "-1..1", "sprite centre") +
                     refRow("sizex / sizey", "out", "scale", "sprite size") +
                     refRow("red / green / blue", "out", "0..1", "sprite tint"));
    else if (std::holds_alternative<TriangleParams>(params))
        vars = table(refRow("n", "in/out", "count", "triangle count") +
                     refRow("i", "in", "0..1", "triangle index (per point)") +
                     refRow("x1..y3", "out", "-1..1", "the three vertices") +
                     refRow("red / green / blue", "out", "0..1", "fill colour"));
    else if (std::holds_alternative<ColorModifierParams>(params))
        vars = table(refRow("red / green / blue", "in/out", "0..1",
                            "channel value (level code maps in→out)"));
    else if (std::holds_alternative<JherikoGlobalParams>(params))
        vars = table(refRow("reg00..reg99", "in/out", "any",
                            "preset-global registers (shared across effects)") +
                     refRow("gmegabuf(i)", "in/out", "any", "global scratch buffer"));
    else if (std::holds_alternative<ListParams>(params))
        vars = table(refRow("enabled", "out", "0/1", "render the list this frame?") +
                     refRow("clear", "out", "0/1", "clear the list buffer first?") +
                     refRow("beat", "in/out", "0/1", "beat (may be mutated)") +
                     refRow("alphain / alphaout", "out", "0..255", "blend alphas") +
                     refRow("w / h", "in", "px", "surface size"));
    else
        vars = QStringLiteral(
            "<p><i>No dedicated variable reference for this module yet.</i></p>");

    const QString legend = highlightLegendHtml();
    return QStringLiteral("<h2>%1 — script variables</h2>%2%3%4")
        .arg(QString::fromUtf8(title), vars, builtinsHtml(), legend);
}

/// Sektions-genaue Milkdrop-EEL-Referenz (N3-Nachzug, Befund S42): die Sets
/// folgen dem M2-Skript-Vertrag (state.cpp:267-512 / milkdropfs.cpp) — nur
/// Original-Variablen, damit Presets kompatibel zum echten MilkDrop bleiben.
QString milkSectionReferenceHtml(int section)
{
    auto table = [](const QString& rows) {
        return QStringLiteral(
                   "<table cellspacing='0' cellpadding='4' "
                   "style='border-collapse:collapse'>"
                   "<tr><th align='left'>Name</th><th align='left'>Type</th>"
                   "<th align='left'>Range</th><th align='left'>Meaning</th></tr>%1</table>")
            .arg(rows);
    };
    // Read-only inputs every milk slot sees (Original-Set — NO vol/beat vars!)
    const QString milkIn =
        refRow("time / fps / frame", "in", "s / Hz / n", "run time, frame rate, frame counter") +
        refRow("progress", "in", "0..1", "preset progress (host group / playlist driven)") +
        refRow("bass / mid / treb", "in", "~1", "band loudness relative to its long-term mean") +
        refRow("bass_att / mid_att / treb_att", "in", "~1", "attenuated (smoothed) variants") +
        refRow("meshx / meshy", "in", "grid", "warp mesh resolution") +
        refRow("aspectx / aspecty", "in", "ratio", "inverse aspect factors") +
        refRow("q1..q64", "in/out", "any", "frame→pixel/wave/shape channel (init snapshot contract)");
    const QString milkNote = QStringLiteral(
        "<p><i>MilkDrop dialect: <code>int()</code> = floor, <code>rand(n)</code> "
        "is integer 0..n-1; <code>getosc/getspec</code> are AVS-only — do not "
        "use them if the preset should stay compatible with original "
        "MilkDrop.</i></p>");

    QString title;
    QString vars;
    switch (section)
    {
        default:
        case 0:  // Code: per_frame_init / per_frame / per_pixel
            title = QStringLiteral("Milkdrop — per_frame / per_pixel");
            vars = table(
                milkIn +
                refRow("zoom / rot / warp", "in/out", "any", "warp controls (write in per_frame; per-vertex in per_pixel)") +
                refRow("cx / cy / dx / dy / sx / sy", "in/out", "any", "warp centre / push / stretch") +
                refRow("zoomexp", "in/out", ">0", "zoom exponent") +
                refRow("decay", "in/out", "0..1", "feedback decay") +
                refRow("gamma", "in/out", "0..8", "composite gamma (clamped like the original)") +
                refRow("echo_zoom / echo_alpha / echo_orient", "in/out", "0.001..1000 / 0..1 / 0..3", "video echo layer") +
                refRow("wave_mode", "in/out", "0..7", "basic waveform style") +
                refRow("wave_a / wave_r / wave_g / wave_b", "in/out", "0..1+", "waveform alpha / colour") +
                refRow("wave_x / wave_y / wave_mystery", "in/out", "0..1 / -1..1", "waveform placement / parameter") +
                refRow("wave_usedots / wave_thick / wave_additive / wave_brighten", "in/out", "0/1", "waveform flags") +
                refRow("ob_size, ob_r/g/b/a · ib_size, ib_r/g/b/a", "in/out", "0..1", "outer / inner border") +
                refRow("mv_x / mv_y / mv_dx / mv_dy / mv_l / mv_r / mv_g / mv_b / mv_a", "in/out", "any", "motion vector grid") +
                refRow("darken_center / wrap / invert / brighten / darken / solarize", "in/out", "0/1", "composite flags") +
                refRow("blur1_min..blur3_max", "in/out", "0..1", "blur pyramid ranges") +
                refRow("monitor", "in/out", "any", "persists across frames (debug scratch)") +
                refRow("x / y / rad / ang", "in", "0..1 / rad", "per_pixel only: vertex position (rect + polar)"));
            break;
        case 1:  // Custom waves
            title = QStringLiteral("Milkdrop — custom wave (init / per_frame / per_point)");
            vars = table(
                milkIn +
                refRow("t1..t8", "in/out", "any", "wave-local memory (frozen after init, reset each frame)") +
                refRow("r / g / b / a", "in/out", "0..1", "wave colour (per_point may vary it)") +
                refRow("samples", "in/out", "2..512", "sample count (frame code may change it)") +
                refRow("sample", "in", "0..1", "per_point: position along the wave") +
                refRow("value1 / value2", "in", "-1..1", "per_point: left / right channel sample") +
                refRow("x / y", "out", "0..1", "per_point: point position"));
            break;
        case 2:  // Custom shapes
            title = QStringLiteral("Milkdrop — custom shape (init / per_frame)");
            vars = table(
                milkIn +
                refRow("t1..t8", "in/out", "any", "shape-local memory (frozen after init, reset each frame)") +
                refRow("x / y / rad / ang", "in/out", "0..1 / rad", "shape placement") +
                refRow("sides", "in/out", "3..100", "polygon sides") +
                refRow("instance / num_inst", "in", "0..n-1 / 1..1024", "instance index / count") +
                refRow("r / g / b / a", "in/out", "0..1", "centre colour") +
                refRow("r2 / g2 / b2 / a2", "in/out", "0..1", "edge colour") +
                refRow("border_r / g / b / a", "in/out", "0..1", "border colour") +
                refRow("thickoutline / textured / additive", "in/out", "0/1", "shape flags") +
                refRow("tex_zoom / tex_ang", "in/out", ">0 / rad", "texture mapping (textured=1 samples the previous frame)"));
            break;
        case 4:  // Sprites (per-frame code)
            title = QStringLiteral("Milkdrop — sprite code (per frame)");
            vars = table(
                refRow("time", "in", "s", "sprite-local time (scaled by SpriteSpeed)") +
                refRow("x / y", "in/out", "0..1", "sprite centre (y grows downward like the original)") +
                refRow("sx / sy", "in/out", "any", "scale (negative mirrors)") +
                refRow("rot", "in/out", "rad", "rotation") +
                refRow("flipx / flipy", "in/out", "0/1", "mirror flags") +
                refRow("repeatx / repeaty", "in/out", ">0", "texture repeat") +
                refRow("blendmode", "in/out", "0..4", "alpha / decal / additive / srccolor / colorkey") +
                refRow("r / g / b / a", "in/out", "0..1", "tint / opacity") +
                refRow("burn", "in/out", "0..1", "burn-in into the feedback buffer") +
                refRow("done", "out", "0/1", "1 removes the sprite"));
            break;
    }
    return QStringLiteral("<h2>%1</h2>%2%3%4%5")
        .arg(title, vars, milkNote, builtinsHtml(), highlightLegendHtml());
}

/// HLSL-Referenz fuer die Warp-/Comp-Editoren (Befund S42: fehlte komplett).
/// Spiegelt die GLSL-Praeambel des Hosts = include.fx-Gegenstueck — nur was
/// dort existiert, bleibt zum Original kompatibel.
QString milkShaderReferenceHtml()
{
    auto table = [](const QString& rows) {
        return QStringLiteral(
                   "<table cellspacing='0' cellpadding='4' "
                   "style='border-collapse:collapse'>"
                   "<tr><th align='left'>Name</th><th align='left'>Type</th>"
                   "<th align='left'>Meaning</th></tr>%1</table>")
            .arg(rows);
    };
    auto row = [](const char* n, const char* t, const char* m) {
        return QStringLiteral("<tr><td><code>%1</code></td><td>%2</td><td>%3</td></tr>")
            .arg(QLatin1String(n), QLatin1String(t), QLatin1String(m));
    };
    const QString io = table(
        row("ret", "float3 (out)", "the pixel colour you write (shader_body result)") +
        row("uv / uv_orig", "float2", "0..1 screen position (comp) / mesh UV (warp)") +
        row("rad / ang", "float", "polar position (aspect-corrected)") +
        row("hue_shader", "float3", "fShader colour-wash corners, bilinear (comp; white in warp)"));
    const QString consts = table(
        row("time, fps, frame, progress", "float", "clock inputs") +
        row("bass, mid, treb, vol (+_att)", "float", "band loudness (~1 baseline)") +
        row("q1..q32", "float", "per_frame q values (script contract)") +
        row("rand_preset / rand_frame", "float4", "random per load / per frame") +
        row("roam_cos/sin, slow_roam_cos/sin", "float4", "slow drifting values") +
        row("aspect", "float4", "aspect factors (x, y, 1/x, 1/y)") +
        row("texsize", "float4", "w, h, 1/w, 1/h") +
        row("texsize_noise_lq/_lq_lite/_mq/_hq", "float4", "noise texture sizes") +
        row("blur1_min..blur3_max", "float", "blur range values"));
    const QString samplers = table(
        row("sampler_main (+_fc/_pc/_fw/_pw)", "sampler2D", "current image (filter/wrap variants)") +
        row("sampler_blur1..3", "sampler2D", "blur pyramid (use GetBlur1..3 to un-bias)") +
        row("sampler_noise_lq / _lq_lite / _mq / _hq", "sampler2D", "procedural noise (exact AddNoiseTex port)") +
        row("sampler_XY", "sampler2D", "custom texture XY.jpg/png next to the preset or in textures/"));
    const QString fns = table(
        row("tex2D(s, uv)", "float3/4", "texture fetch") +
        row("GetMain(uv) / GetPixel(uv)", "float3", "current image") +
        row("GetBlur1..3(uv)", "float3", "blur pyramid, range-decompressed") +
        row("lum(c)", "float", "luminance (0.32/0.49/0.29 weights)"));
    return QStringLiteral(
               "<h2>Milkdrop — HLSL shader_body</h2>"
               "<p>Write the <code>shader_body</code> content only (HLSL "
               "ps_2/3 style); it is transpiled to GLSL. Loops, arrays, "
               "tex3D/noise volumes and <code>#if</code> are stage C3 — "
               "presets using them fall back to the MD1 path.</p>"
               "<h3>In / Out</h3>%1<h3>Constants</h3>%2<h3>Samplers</h3>%3"
               "<h3>Functions</h3>%4")
        .arg(io, consts, samplers, fns);
}

} // namespace

void ChainTreeWidget::dropEvent(QDropEvent* event)
{
    QTreeWidgetItem* target = itemAt(event->position().toPoint());
    ChainDrop where = ChainDrop::Viewport;
    switch (dropIndicatorPosition())  // protected enum: only readable in-subclass
    {
        case QAbstractItemView::OnItem:     where = ChainDrop::OnItem; break;
        case QAbstractItemView::AboveItem:  where = ChainDrop::Above;  break;
        case QAbstractItemView::BelowItem:  where = ChainDrop::Below;  break;
        case QAbstractItemView::OnViewport: where = ChainDrop::Viewport; break;
    }
    QTreeWidgetItem* src = currentItem();  // single-selection: dragged == current
    if (onDrop && src != nullptr)
    {
        onDrop(src, target, where);
    }
    // Consume without calling the base: the chain owns ordering and the tree is
    // rebuilt from it, so the view must not move items on its own. IgnoreAction
    // (not MoveAction) stops QAbstractItemView::startDrag from also deleting the
    // source rows after our rebuild already replaced every item.
    event->setDropAction(Qt::IgnoreAction);
    event->accept();
}

void ChainTreeWidget::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) &&
        onDeleteKey)
    {
        onDeleteKey();
        event->accept();
        return;
    }
    QTreeWidget::keyPressEvent(event);
}

MultiEffectPanel::MultiEffectPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "multieffect_chain", tr("Effect Chain"), parent)
{
    setupUI();
    connectToActiveVisualizer();
}

void MultiEffectPanel::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(4, 4, 4, 4);

    m_hint = new QLabel(tr("Select the \"Multi Effect\" visualizer to edit its chain."),
                        this);
    m_hint->setWordWrap(true);
    root->addWidget(m_hint);

    // Toolbar: add-type combo + add / remove / up / down.
    auto* toolbar = new QHBoxLayout();
    m_addTypeCombo = new QComboBox(this);
    auto* comboModel = qobject_cast<QStandardItemModel*>(m_addTypeCombo->model());
    const std::vector<EffectType>& palette = effectPalette();
    const bool haveOriginIcons = !lumi::ui::presetIconDir().isEmpty();
    for (size_t i = 0; i < palette.size(); ++i)
    {
        const EffectType& t = palette[i];
        // Origin marker: per-origin icon (asset/img/logo/icons); text fallback
        // when the icons are not found next to the repo/exe.
        QString label = QString::fromUtf8(t.name);
        if (!t.isHeader() && !haveOriginIcons)
            label += QStringLiteral("   · ") + originText(t.origin);
        if (!t.isHeader() && haveOriginIcons)
            m_addTypeCombo->addItem(originIcon(t.origin), label);
        else
            m_addTypeCombo->addItem(label);
        if (comboModel == nullptr) continue;
        QStandardItem* item = comboModel->item(static_cast<int>(i));
        if (item == nullptr) continue;
        if (t.isHeader())
        {
            item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }
        else
        {
            item->setToolTip(originToolTip(t.origin));
        }
    }
    // Start on the first selectable (non-header) entry, not a category title.
    for (size_t i = 0; i < palette.size(); ++i)
        if (!palette[i].isHeader()) { m_addTypeCombo->setCurrentIndex(static_cast<int>(i)); break; }
    toolbar->addWidget(m_addTypeCombo, 1);

    auto makeButton = [this](const QString& text, const QString& tip) {
        auto* b = new QToolButton(this);
        b->setText(text);
        b->setToolTip(tip);
        return b;
    };
    m_addButton = makeButton("+", tr("Add effect (into the selected list, else root)"));
    m_removeButton = makeButton("-", tr("Remove selected"));
    m_cloneButton = makeButton(QString::fromUtf8("⧉"), tr("Clone selected (with its subtree)"));
    m_upButton = makeButton(QString::fromUtf8("↑"), tr("Move up"));
    m_downButton = makeButton(QString::fromUtf8("↓"), tr("Move down"));
    toolbar->addWidget(m_addButton);
    toolbar->addWidget(m_removeButton);
    toolbar->addWidget(m_cloneButton);
    toolbar->addWidget(m_upButton);
    toolbar->addWidget(m_downButton);
    root->addLayout(toolbar);

    m_tree = new ChainTreeWidget(this);
    m_tree->setHeaderLabels({tr("Name"), QString(), tr("Type"), tr("Description")});
    m_tree->setColumnCount(4);
    m_tree->setColumnWidth(1, 30);  // narrow eye toggle column (col 1)
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_tree->header()->setStretchLastSection(true);
    // Column 0 (Name) is the tree column: show the expand decoration + a clear
    // per-level indent so nested Effect Lists read as nested across levels.
    m_tree->setRootIsDecorated(true);
    m_tree->setIndentation(18);
    m_tree->setEditTriggers(QAbstractItemView::DoubleClicked |
                            QAbstractItemView::EditKeyPressed);
    // Drag & drop re-parents/reorders effects (into / out of groups). We handle
    // the drop ourselves (onDrop) and rebuild from the model.
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);  // Shift/Ctrl multi
    m_tree->setDragEnabled(true);
    m_tree->setAcceptDrops(true);
    m_tree->setDropIndicatorShown(true);
    m_tree->setDragDropMode(QAbstractItemView::InternalMove);
    m_tree->onDrop = [this](QTreeWidgetItem* s, QTreeWidgetItem* t, ChainDrop w) {
        onDropRequested(s, t, w);
    };
    m_tree->onDeleteKey = [this] { onRemove(); };
    root->addWidget(m_tree, 1);

    m_propScroll = new QScrollArea(this);
    m_propScroll->setWidgetResizable(true);
    m_propContainer = new QWidget();
    m_propLayout = new QVBoxLayout(m_propContainer);
    m_propLayout->setAlignment(Qt::AlignTop);
    m_propScroll->setWidget(m_propContainer);
    root->addWidget(m_propScroll, 1);

    connect(m_addButton, &QToolButton::clicked, this, &MultiEffectPanel::onAddEffect);
    connect(m_removeButton, &QToolButton::clicked, this, &MultiEffectPanel::onRemove);
    connect(m_cloneButton, &QToolButton::clicked, this, &MultiEffectPanel::onClone);
    connect(m_upButton, &QToolButton::clicked, this, [this] { onMove(-1); });
    connect(m_downButton, &QToolButton::clicked, this, [this] { onMove(1); });
    connect(m_tree, &QTreeWidget::itemChanged, this, &MultiEffectPanel::onItemChanged);
    connect(m_tree, &QTreeWidget::itemSelectionChanged, this,
            &MultiEffectPanel::onSelectionChanged);
}

void MultiEffectPanel::connectToActiveVisualizer()
{
    // Find the current visualizer through the widget hierarchy (like ConfigPanel).
    if (auto* window = this->window())
    {
        if (auto* viz = window->findChild<VisualizerWidget*>())
        {
            setHost(dynamic_cast<MultiEffectVisualizer*>(viz->visualizer()),
                    &viz->renderMutex());
        }
    }
    if (auto* bus = eventBus())
    {
        m_eventSubscriptions.push_back(bus->subscribeScoped<VisualizerChangedEvent>(
            [this](const VisualizerChangedEvent& e) {
                setHost(dynamic_cast<MultiEffectVisualizer*>(
                            static_cast<IVisualizer*>(e.visualizerPtr)),
                        e.renderMutex);
            }));
        // The chain was replaced externally (AVS import / preset load) — rebuild.
        m_eventSubscriptions.push_back(bus->subscribeScoped<EffectChainChangedEvent>(
            [this](const EffectChainChangedEvent&) {
                rebuildTree();
                clearPropertyEditor();
            }));
    }
}

void MultiEffectPanel::setHost(MultiEffectVisualizer* host, QMutex* mutex)
{
    m_host = host;
    m_mutex = mutex;
    const bool active = m_host != nullptr;
    m_hint->setVisible(!active);
    m_tree->setEnabled(active);
    m_addTypeCombo->setEnabled(active);
    m_addButton->setEnabled(active);
    m_removeButton->setEnabled(active);
    m_cloneButton->setEnabled(active);
    m_upButton->setEnabled(active);
    m_downButton->setEnabled(active);
    rebuildTree();
    clearPropertyEditor();
}

// =============================================================================
// Tree <-> chain
// =============================================================================

void MultiEffectPanel::rebuildTree()
{
    m_updating = true;
    m_tree->clear();
    if (m_host != nullptr && m_mutex != nullptr)
    {
        QMutexLocker lock(m_mutex);
        const ChainNode& root = m_host->chain();
        for (int i = 0; i < static_cast<int>(root.children.size()); ++i)
        {
            addTreeItem(nullptr, root.children[i], {i});
        }
    }
    m_tree->expandAll();
    m_updating = false;
}

void MultiEffectPanel::addTreeItem(QTreeWidgetItem* parentItem, const ChainNode& node,
                                   QList<int> path)
{
    auto* item = new QTreeWidgetItem();
    // Column 0 = name (the tree column: gets the expand arrow + per-level indent,
    // so nesting stays visible); 1 = eye toggle (widget); 2 = type; 3 = desc.
    item->setText(0, QString::fromStdString(
                         node.displayName.empty() ? effectTypeName(node.params)
                                                  : node.displayName));
    item->setText(2, QString::fromStdString(effectTypeName(node.params)));
    // Origin icon on the type column (AVS / MilkDrop / LumiViz).
    if (!lumi::ui::presetIconDir().isEmpty())
    {
        const Origin origin = originForParams(node.params);
        item->setIcon(2, originIcon(origin));
        item->setToolTip(2, originToolTip(origin));
    }
    item->setText(3, QString::fromStdString(node.description));
    item->setFlags((item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled |
                    Qt::ItemIsDropEnabled));
    item->setData(0, Qt::UserRole, pathToVariant(path));

    if (parentItem != nullptr) parentItem->addChild(item);
    else m_tree->addTopLevelItem(item);

    // Eye toggle (hide/show like the stage previews) in its own narrow column so
    // it never fights the tree indentation. Toggling `enabled` is not structural,
    // so no rebuild is needed and the captured path stays valid.
    auto* eye = new QToolButton(m_tree);
    eye->setCheckable(true);
    eye->setAutoRaise(true);
    eye->setChecked(node.enabled);
    eye->setText(node.enabled ? QString::fromUtf8("\xF0\x9F\x91\x81")   // 👁
                              : QString::fromUtf8("\xE2\x80\x94"));      // —
    eye->setToolTip(tr("Show / hide this effect"));
    connect(eye, &QToolButton::toggled, this, [this, eye, path](bool on) {
        eye->setText(on ? QString::fromUtf8("\xF0\x9F\x91\x81")
                        : QString::fromUtf8("\xE2\x80\x94"));
        setNodeEnabled(path, on);
    });
    m_tree->setItemWidget(item, 1, eye);

    if (node.isContainer())  // Listen + Host-Gruppen (HG1)
    {
        for (int i = 0; i < static_cast<int>(node.children.size()); ++i)
        {
            QList<int> childPath = path;
            childPath.append(i);
            addTreeItem(item, node.children[i], childPath);
        }
    }

    // Milkdrop-Meganode (N2, Entscheid E1; N3 Session 42): Anzeige-Kinder
    // Preset → Code · Waves · Shapes · Shader · Sprites. Sektions- und
    // Element-Items sind reine Navigations-Items (Sentinel-Pfade, nicht
    // editier-/drag-/dropbar) — der Editor zeigt die gewaehlte Sektion bzw.
    // das gewaehlte Element; +/-/⧉ der Toolbar wirken auf Elemente (N3.1).
    if (const auto* milk = std::get_if<MilkdropNodeParams>(&node.params))
    {
        int wavesOn = 0;
        int shapesOn = 0;
        for (const auto& w : milk->preset.waves)
            if (w.enabled) ++wavesOn;
        for (const auto& s : milk->preset.shapes)
            if (s.enabled) ++shapesOn;
        const QStringList sections = {
            tr("Code (per_frame / per_pixel)"),
            tr("Waves (Basis + %1 Custom aktiv)").arg(wavesOn),
            tr("Shapes (%1 aktiv)").arg(shapesOn),
            tr("Shader (Warp/Comp)"),
            tr("Sprites (%1)").arg(milk->preset.sprites.size()),
            tr("Parameter (Basiswerte)")};
        const bool haveMilkIcons = !lumi::ui::presetIconDir().isEmpty();
        const QIcon& milkIcon =
            lumi::ui::presetTypeIcon(lumi::ui::PresetIconKind::MilkDrop);

        // Auge-Toggle fuer Wave-/Shape-Elemente (S43, Sichttest-Befund):
        // spiegelt `enabled` wie das Node-Auge — Mutation via mutate()
        // (renderMutex + recompile) + Revision-Bump; Labels werden in-place
        // nachgezogen (enabled-Toggle ist nicht strukturell, kein Rebuild).
        auto addMilkEye = [this, path](QTreeWidgetItem* el, QTreeWidgetItem* sec,
                                       int sectionIdx, int elementIdx,
                                       int displayIndex, bool on) {
            auto* eye = new QToolButton(m_tree);
            eye->setCheckable(true);
            eye->setAutoRaise(true);
            eye->setChecked(on);
            eye->setText(on ? QString::fromUtf8("\xF0\x9F\x91\x81")   // 👁
                            : QString::fromUtf8("\xE2\x80\x94"));      // —
            eye->setToolTip(tr("Show / hide this element"));
            connect(eye, &QToolButton::toggled, this,
                    [this, eye, el, sec, path, sectionIdx, elementIdx,
                     displayIndex](bool v) {
                        eye->setText(v ? QString::fromUtf8("\xF0\x9F\x91\x81")
                                       : QString::fromUtf8("\xE2\x80\x94"));
                        int onCount = 0;
                        mutate(path, [&](ChainNode& n) {
                            auto* m = std::get_if<MilkdropNodeParams>(&n.params);
                            if (m == nullptr) return;
                            auto apply = [&](auto& vec) {
                                if (elementIdx >= 0 &&
                                    elementIdx < static_cast<int>(vec.size()))
                                {
                                    vec[static_cast<std::size_t>(elementIdx)]
                                        .enabled = v;
                                    ++m->revision;
                                }
                                for (const auto& e2 : vec)
                                    if (e2.enabled) ++onCount;
                            };
                            if (sectionIdx == 1) apply(m->preset.waves);
                            else if (sectionIdx == 2) apply(m->preset.shapes);
                        });
                        m_updating = true;
                        el->setText(0, (sectionIdx == 1 ? tr("Wave %1%2")
                                                        : tr("Shape %1%2"))
                                           .arg(displayIndex)
                                           .arg(v ? QString() : tr(" (aus)")));
                        sec->setText(
                            0, sectionIdx == 1
                                   ? tr("Waves (Basis + %1 Custom aktiv)").arg(onCount)
                                   : tr("Shapes (%1 aktiv)").arg(onCount));
                        m_updating = false;
                        // Editor-Checkbox „... aktiv" nachziehen, falls sichtbar
                        if (m_tree->currentItem() == el)
                            buildPropertyEditor(currentPath());
                    });
            m_tree->setItemWidget(el, 1, eye);
        };

        for (int s = 0; s < sections.size(); ++s)
        {
            auto* sec = new QTreeWidgetItem(item);
            sec->setText(0, sections[s]);
            sec->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            if (haveMilkIcons) sec->setIcon(2, milkIcon);  // Typ-Spalte (S43)
            QList<int> secPath = path;
            secPath.append(kMilkSectionBase + s);
            sec->setData(0, Qt::UserRole, pathToVariant(secPath));

            // Element-Kinder (N3.1/N3.3): je Wave/Shape/Sprite ein waehlbares
            // Item — Pfad = [.., Sektions-Sentinel, Element-Index]
            auto addElement = [&](int elementIdx,
                                  const QString& label) -> QTreeWidgetItem* {
                auto* el = new QTreeWidgetItem(sec);
                el->setText(0, label);
                el->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                if (haveMilkIcons) el->setIcon(2, milkIcon);
                QList<int> elPath = secPath;
                elPath.append(elementIdx);
                el->setData(0, Qt::UserRole, pathToVariant(elPath));
                return el;
            };
            if (s == 1)
            {
                // Basis-Waveform zuerst: sie rendert immer (auch bei 0 Custom
                // Waves) und braucht darum einen sichtbaren Editor-Einstieg.
                addElement(kMilkBasisWaveElement, tr("Basis-Waveform"));
                for (int i = 0; i < static_cast<int>(milk->preset.waves.size()); ++i)
                {
                    const auto& w = milk->preset.waves[i];
                    auto* el = addElement(
                        i, tr("Wave %1%2")
                               .arg(w.index)
                               .arg(w.enabled ? QString() : tr(" (aus)")));
                    addMilkEye(el, sec, 1, i, w.index, w.enabled);
                }
            }
            else if (s == 2)
            {
                for (int i = 0; i < static_cast<int>(milk->preset.shapes.size()); ++i)
                {
                    const auto& sh = milk->preset.shapes[i];
                    auto* el = addElement(
                        i, tr("Shape %1%2")
                               .arg(sh.index)
                               .arg(sh.enabled ? QString() : tr(" (aus)")));
                    addMilkEye(el, sec, 2, i, sh.index, sh.enabled);
                }
            }
            else if (s == 4)
            {
                for (int i = 0; i < static_cast<int>(milk->preset.sprites.size()); ++i)
                    addElement(i, tr("Sprite %1 — %2")
                                      .arg(milk->preset.sprites[i].index)
                                      .arg(QString::fromStdString(
                                          milk->preset.sprites[i].imageName)));
            }
        }
    }
}

ChainNode* MultiEffectPanel::nodeAtPath(const QList<int>& path)
{
    if (m_host == nullptr) return nullptr;
    ChainNode* node = &m_host->chain();
    for (int idx : path)
    {
        if (idx < 0 || idx >= static_cast<int>(node->children.size())) return nullptr;
        node = &node->children[static_cast<size_t>(idx)];
    }
    return node;
}

QList<int> MultiEffectPanel::currentPath() const
{
    auto* item = m_tree->currentItem();
    if (item == nullptr) return {};
    return pathFromVariant(item->data(0, Qt::UserRole));
}

// =============================================================================
// Mutations
// =============================================================================

void MultiEffectPanel::mutate(const QList<int>& path,
                              const std::function<void(ChainNode&)>& fn)
{
    if (m_host == nullptr || m_mutex == nullptr) return;
    {
        QMutexLocker lock(m_mutex);
        ChainNode* node = nodeAtPath(path);
        if (node == nullptr) return;
        fn(*node);
        m_host->recompileChain();
    }
    // Refresh the affected item's label (name/clamps may have changed).
    // Milk-Sektions-/Element-Items (Sentinel-Pfade) NICHT umbenennen — sie
    // tragen ihre eigenen Labels, nicht den Node-Namen.
    if (auto* item = m_tree->currentItem())
    {
        const QList<int> itemPath = pathFromVariant(item->data(0, Qt::UserRole));
        for (int seg : itemPath)
            if (seg >= kMilkSectionBase) return;
        QMutexLocker lock(m_mutex);
        if (ChainNode* node = nodeAtPath(path))
        {
            m_updating = true;
            item->setText(0, QString::fromStdString(
                                 node->displayName.empty()
                                     ? effectTypeName(node->params)
                                     : node->displayName));
            m_updating = false;
        }
    }
}

void MultiEffectPanel::mutateStructure(const std::function<void()>& fn)
{
    if (m_host == nullptr || m_mutex == nullptr) return;
    {
        QMutexLocker lock(m_mutex);
        fn();
        m_host->recompileChain();
    }
    rebuildTree();
    clearPropertyEditor();
}

void MultiEffectPanel::onAddEffect()
{
    if (m_host == nullptr) return;
    const int typeIdx = m_addTypeCombo->currentIndex();
    if (typeIdx < 0) return;
    const EffectType& chosen = effectPalette()[static_cast<size_t>(typeIdx)];
    if (chosen.isHeader()) return;  // category title, not an insertable effect

    // Milkdrop-Node-Inhalt (N3.1/N3.3): landet im PresetState des selektierten
    // Milkdrop-Nodes (bzw. seiner Sektion/seines Elements), nicht in der Chain.
    if (chosen.milkElement != MilkElement::None)
    {
        const MilkPathInfo mp = splitMilkPath(currentPath());
        QList<int> newSel;
        mutateStructure([&] {
            ChainNode* node = nodeAtPath(mp.nodePath);
            auto* milk =
                node ? std::get_if<MilkdropNodeParams>(&node->params) : nullptr;
            if (milk == nullptr) return;  // Auswahl ist kein Milkdrop-Node
            auto& preset = milk->preset;
            switch (chosen.milkElement)
            {
                case MilkElement::Wave:
                {
                    if (preset.waves.size() >= 16) return;  // MD3-Superset-Cap
                    lumi::milkdrop::WaveState w;
                    w.index = nextFreeMilkIndex(preset.waves);
                    w.enabled = true;  // frisch angelegt = sichtbar gewollt
                    preset.waves.push_back(std::move(w));
                    ++milk->revision;
                    newSel = mp.nodePath;
                    newSel << (kMilkSectionBase + 1)
                           << static_cast<int>(preset.waves.size()) - 1;
                    break;
                }
                case MilkElement::Shape:
                {
                    if (preset.shapes.size() >= 16) return;
                    lumi::milkdrop::ShapeState s;
                    s.index = nextFreeMilkIndex(preset.shapes);
                    s.enabled = true;
                    preset.shapes.push_back(std::move(s));
                    ++milk->revision;
                    newSel = mp.nodePath;
                    newSel << (kMilkSectionBase + 2)
                           << static_cast<int>(preset.shapes.size()) - 1;
                    break;
                }
                case MilkElement::Sprite:
                {
                    lumi::milkdrop::SpriteState sp;
                    sp.index = nextFreeMilkIndex(preset.sprites);
                    preset.sprites.push_back(std::move(sp));
                    ++milk->revision;
                    newSel = mp.nodePath;
                    newSel << (kMilkSectionBase + 4)
                           << static_cast<int>(preset.sprites.size()) - 1;
                    break;
                }
                default: break;
            }
        });
        if (!newSel.isEmpty()) selectByPath(newSel);
        return;
    }

    ChainNode fresh;
    fresh.params = chosen.make();

    const QList<int> sel = currentPath();
    mutateStructure([&] {
        ChainNode* target = nodeAtPath(sel);
        // Tiefenregel (HG1, Entwurf §5.1): keine Host-Gruppe in eine
        // Host-Gruppe einfuegen — weder direkt noch in eine Liste darunter.
        // Greift nur, wenn der Einfuege-Ort wirklich der selektierte Container
        // ist (sonst landet der Add ohnehin auf der Root-Liste).
        if (fresh.isHostGroup() && target != nullptr && target->isContainer())
        {
            ChainNode* walk = &m_host->chain();
            bool insideGroup = walk->isHostGroup();
            for (int idx : sel)
            {
                if (idx < 0 || idx >= static_cast<int>(walk->children.size())) break;
                walk = &walk->children[static_cast<size_t>(idx)];
                insideGroup = insideGroup || walk->isHostGroup();
            }
            if (insideGroup) return;
        }
        // Add into the selected container (list/host group); else root list.
        if (target != nullptr && target->isContainer())
        {
            target->children.push_back(std::move(fresh));
        }
        else
        {
            m_host->chain().children.push_back(std::move(fresh));
        }
    });
}

void MultiEffectPanel::onRemove()
{
    const QList<QList<int>> paths = selectedPaths();
    if (paths.isEmpty()) return;  // never remove the root

    // Milk-Elemente (N3.1/N3.3): selektierte Waves/Shapes/Sprites entfernen.
    // Sektions-Items selbst sind nicht loeschbar.
    {
        const MilkPathInfo first = splitMilkPath(paths.first());
        if (first.section >= 0 && first.element >= 0)
        {
            QList<int> idxs;
            for (const QList<int>& pth : paths)
            {
                const MilkPathInfo mp = splitMilkPath(pth);
                if (mp.section == first.section && mp.element >= 0 &&
                    mp.element != kMilkBasisWaveElement &&  // Basis bleibt
                    mp.nodePath == first.nodePath)
                {
                    idxs.append(mp.element);
                }
            }
            if (idxs.isEmpty()) return;  // z. B. nur Basis-Waveform selektiert
            std::sort(idxs.begin(), idxs.end(), std::greater<int>());
            mutateStructure([&] {
                ChainNode* node = nodeAtPath(first.nodePath);
                auto* milk =
                    node ? std::get_if<MilkdropNodeParams>(&node->params) : nullptr;
                if (milk == nullptr) return;
                auto eraseAt = [](auto& vec, int idx) {
                    if (idx >= 0 && idx < static_cast<int>(vec.size()))
                    {
                        vec.erase(vec.begin() + idx);
                        return true;
                    }
                    return false;
                };
                bool removed = false;
                for (int idx : idxs)
                {
                    if (first.section == 1)
                        removed |= eraseAt(milk->preset.waves, idx);
                    else if (first.section == 2)
                        removed |= eraseAt(milk->preset.shapes, idx);
                    else if (first.section == 4)
                        removed |= eraseAt(milk->preset.sprites, idx);
                }
                if (removed) ++milk->revision;
            });
            return;
        }
        if (first.section >= 0) return;  // Sektions-Item: nichts entfernen
    }

    QList<int> parentPath = paths.first();
    parentPath.removeLast();
    QList<int> idxs;
    for (const QList<int>& p : paths) idxs.append(p.last());
    std::sort(idxs.begin(), idxs.end(), std::greater<int>());  // erase high -> low
    mutateStructure([&] {
        ChainNode* parent = nodeAtPath(parentPath);
        if (parent == nullptr) return;
        for (int idx : idxs)
            if (idx >= 0 && idx < static_cast<int>(parent->children.size()))
                parent->children.erase(parent->children.begin() + idx);
    });
}

void MultiEffectPanel::onClone()
{
    const QList<int> path = currentPath();
    if (path.isEmpty()) return;  // root is not clonable

    // Milk-Elemente (N3.1/N3.3): Wave/Shape/Sprite duplizieren (Einfuegung
    // direkt dahinter, frischer index). Sektions-Items sind nicht klonbar.
    {
        const MilkPathInfo mp = splitMilkPath(path);
        if (mp.section >= 0)
        {
            if (mp.element < 0) return;
            if (mp.element == kMilkBasisWaveElement) return;  // Basis: 1x fix
            QList<int> newSel;
            mutateStructure([&] {
                ChainNode* node = nodeAtPath(mp.nodePath);
                auto* milk =
                    node ? std::get_if<MilkdropNodeParams>(&node->params) : nullptr;
                if (milk == nullptr) return;
                auto cloneAt = [&](auto& vec, std::size_t cap) {
                    const int e = mp.element;
                    if (e < 0 || e >= static_cast<int>(vec.size())) return false;
                    if (cap > 0 && vec.size() >= cap) return false;
                    auto copy = vec[static_cast<std::size_t>(e)];
                    copy.index = nextFreeMilkIndex(vec);
                    vec.insert(vec.begin() + e + 1, std::move(copy));
                    return true;
                };
                bool ok = false;
                if (mp.section == 1) ok = cloneAt(milk->preset.waves, 16);
                else if (mp.section == 2) ok = cloneAt(milk->preset.shapes, 16);
                else if (mp.section == 4) ok = cloneAt(milk->preset.sprites, 0);
                if (ok)
                {
                    ++milk->revision;
                    newSel = mp.nodePath;
                    newSel << (kMilkSectionBase + mp.section) << (mp.element + 1);
                }
            });
            if (!newSel.isEmpty()) selectByPath(newSel);
            return;
        }
    }

    QList<int> finalPath;
    mutateStructure([&] {
        QList<int> parentPath = path;
        const int idx = parentPath.takeLast();
        ChainNode* parent = nodeAtPath(parentPath);
        if (parent == nullptr || idx < 0 ||
            idx >= static_cast<int>(parent->children.size()))
        {
            return;
        }
        ChainNode copy = parent->children[static_cast<size_t>(idx)];  // deep copy
        // Fresh identities: compileChain only assigns to nodeId==0, so a copied
        // (non-zero) id would collide with the original -> zero the whole subtree.
        std::function<void(ChainNode&)> clearIds = [&](ChainNode& n) {
            n.nodeId = 0;
            for (ChainNode& c : n.children) clearIds(c);
        };
        clearIds(copy);
        parent->children.insert(parent->children.begin() + idx + 1, std::move(copy));
        finalPath = parentPath;
        finalPath.append(idx + 1);
    });
    if (!finalPath.isEmpty()) selectByPath(finalPath);
}

void MultiEffectPanel::onMove(int delta)
{
    const QList<QList<int>> paths = selectedPaths();
    if (paths.isEmpty()) return;
    QList<int> parentPath = paths.first();
    parentPath.removeLast();
    QList<int> idxs;
    for (const QList<int>& p : paths) idxs.append(p.last());
    std::sort(idxs.begin(), idxs.end());
    const int count = static_cast<int>(idxs.size());
    const int minI = idxs.first();
    const bool contiguous = (idxs.last() - minI + 1 == count);

    QList<QList<int>> newSel;
    mutateStructure([&] {
        ChainNode* parent = nodeAtPath(parentPath);
        if (parent == nullptr) return;
        const int n = static_cast<int>(parent->children.size());
        const int newBase = std::clamp(minI + delta, 0, n - count);
        if (newBase == minI && contiguous) return;  // already there, can't move

        const QSet<int> sel(idxs.begin(), idxs.end());
        std::vector<ChainNode> picked, rest;
        for (int i = 0; i < n; ++i)
        {
            if (sel.contains(i))
                picked.push_back(std::move(parent->children[static_cast<size_t>(i)]));
            else
                rest.push_back(std::move(parent->children[static_cast<size_t>(i)]));
        }
        std::vector<ChainNode> result;
        result.reserve(static_cast<size_t>(n));
        for (int i = 0; i < newBase; ++i)
            result.push_back(std::move(rest[static_cast<size_t>(i)]));
        for (ChainNode& node : picked) result.push_back(std::move(node));
        for (int i = newBase; i < static_cast<int>(rest.size()); ++i)
            result.push_back(std::move(rest[static_cast<size_t>(i)]));
        parent->children = std::move(result);

        for (int k = 0; k < count; ++k)
        {
            QList<int> np = parentPath;
            np.append(newBase + k);
            newSel.append(np);
        }
    });
    if (!newSel.isEmpty()) selectPaths(newSel);
}

void MultiEffectPanel::setNodeEnabled(const QList<int>& path, bool enabled)
{
    if (m_host == nullptr || m_mutex == nullptr) return;
    QMutexLocker lock(m_mutex);
    if (ChainNode* node = nodeAtPath(path))
    {
        node->enabled = enabled;
        m_host->recompileChain();  // no structural change -> paths stay valid
    }
}

void MultiEffectPanel::applyScope3DPreset(const QList<int>& path, int presetIndex)
{
    const auto& defs = scope3dPresets();
    if (presetIndex < 0 || presetIndex >= static_cast<int>(defs.size())) return;
    if (m_host == nullptr || m_mutex == nullptr) return;
    const Scope3DPresetDef& def = defs[static_cast<std::size_t>(presetIndex)];

    QMutexLocker lock(m_mutex);
    ChainNode* node = nodeAtPath(path);
    if (node == nullptr) return;
    if (auto* p = std::get_if<SuperScope3DParams>(&node->params))
    {
        p->initCode = def.init;
        p->frameCode = def.frame;
        p->beatCode = def.beat;
        p->pointCode = def.point;
        p->pointCount = def.pointCount;
        m_host->recompileChain();
    }
}

void MultiEffectPanel::applyCamera3DPreset(const QList<int>& path, int presetIndex)
{
    const auto& defs = camera3dPresets();
    if (presetIndex < 0 || presetIndex >= static_cast<int>(defs.size())) return;
    if (m_host == nullptr || m_mutex == nullptr) return;
    const Camera3DPresetDef& def = defs[static_cast<std::size_t>(presetIndex)];

    QMutexLocker lock(m_mutex);
    ChainNode* node = nodeAtPath(path);
    if (node == nullptr) return;
    if (auto* p = std::get_if<Camera3DParams>(&node->params))
    {
        p->initCode = def.init;
        p->frameCode = def.frame;
        p->beatCode = def.beat;
        m_host->recompileChain();
    }
}

// --- Drag & drop re-parenting ------------------------------------------------

void MultiEffectPanel::onDropRequested(QTreeWidgetItem* src, QTreeWidgetItem* target,
                                       ChainDrop where)
{
    if (src == nullptr) return;
    const QList<int> srcPath = pathFromVariant(src->data(0, Qt::UserRole));
    if (srcPath.isEmpty()) return;
    QList<int> targetPath;
    if (target != nullptr) targetPath = pathFromVariant(target->data(0, Qt::UserRole));

    // Move the whole (same-level) selection as a block; if the dragged item is
    // not part of the selection, move just it.
    QList<QList<int>> srcPaths = selectedPaths();
    bool srcInSel = false;
    for (const QList<int>& p : srcPaths) if (p == srcPath) srcInSel = true;
    if (!srcInSel) srcPaths = {srcPath};

    // Defer the actual move: we are inside the tree's own dropEvent, and the
    // rebuild would delete the items the drag machinery still touches. Paths are
    // value copies, so they survive the queued hop.
    QMetaObject::invokeMethod(
        this,
        [this, srcPaths, targetPath, where] {
            QList<QList<int>> finalPaths;
            bool moved = false;
            mutateStructure(
                [&] { moved = moveNodesLocked(srcPaths, targetPath, where, finalPaths); });
            if (moved) selectPaths(finalPaths);
        },
        Qt::QueuedConnection);
}

bool MultiEffectPanel::moveNodesLocked(const QList<QList<int>>& srcPaths,
                                       const QList<int>& targetPath, ChainDrop where,
                                       QList<QList<int>>& finalPaths)
{
    if (m_host == nullptr || srcPaths.isEmpty()) return false;
    if (srcPaths.size() == 1)  // single node: the well-tested path
    {
        QList<int> fp;
        if (!moveNodeLocked(srcPaths.first(), targetPath, where, fp)) return false;
        finalPaths.append(fp);
        return true;
    }

    // All selected share one parent (enforced by the selection constraint).
    QList<int> srcParentPath = srcPaths.first();
    srcParentPath.removeLast();
    QList<int> srcIdxs;
    for (const QList<int>& p : srcPaths) srcIdxs.append(p.last());
    std::sort(srcIdxs.begin(), srcIdxs.end());

    // Resolve the destination parent + index (same rules as moveNodeLocked).
    QList<int> dstParentPath;
    int dstIndex = 0;
    if (targetPath.isEmpty() || where == ChainDrop::Viewport)
    {
        dstIndex = static_cast<int>(m_host->chain().children.size());
    }
    else
    {
        QList<int> targetParentPath = targetPath;
        const int targetIdx = targetParentPath.takeLast();
        if (where == ChainDrop::OnItem)
        {
            ChainNode* tnode = nodeAtPath(targetPath);
            if (tnode != nullptr && tnode->isContainer())
            {
                dstParentPath = targetPath;
                dstIndex = static_cast<int>(tnode->children.size());
            }
            else { dstParentPath = targetParentPath; dstIndex = targetIdx + 1; }
        }
        else if (where == ChainDrop::Above) { dstParentPath = targetParentPath; dstIndex = targetIdx; }
        else { dstParentPath = targetParentPath; dstIndex = targetIdx + 1; }
    }

    // Never drop the block into one of its own subtrees.
    for (const QList<int>& sp : srcPaths)
    {
        const bool intoSub = dstParentPath.size() >= sp.size() &&
                             dstParentPath.mid(0, sp.size()) == sp;
        if (dstParentPath == sp || intoSub) return false;
    }

    // Tiefenregel (HG1): Teilbaeume MIT Host-Gruppe nicht in eine Gruppe ziehen.
    if (pathTouchesHostGroup(m_host->chain(), dstParentPath))
    {
        for (const QList<int>& sp : srcPaths)
        {
            ChainNode* srcNode = nodeAtPath(sp);
            if (srcNode != nullptr && chainHasHostGroup(*srcNode)) return false;
        }
    }

    ChainNode* srcParent = nodeAtPath(srcParentPath);
    if (srcParent == nullptr) return false;
    for (int i : srcIdxs)
        if (i < 0 || i >= static_cast<int>(srcParent->children.size())) return false;

    // Extract (asc) then erase (desc) from the source.
    std::vector<ChainNode> picked;
    for (int i : srcIdxs)
        picked.push_back(std::move(srcParent->children[static_cast<size_t>(i)]));
    for (int j = static_cast<int>(srcIdxs.size()) - 1; j >= 0; --j)
        srcParent->children.erase(srcParent->children.begin() + srcIdxs[j]);

    // Adjust the destination for the removals.
    const int sp = static_cast<int>(srcParentPath.size());
    if (dstParentPath == srcParentPath)
    {
        int before = 0;
        for (int i : srcIdxs) if (i < dstIndex) ++before;
        dstIndex -= before;
    }
    else if (dstParentPath.size() > sp && dstParentPath.mid(0, sp) == srcParentPath)
    {
        int shift = 0;
        for (int i : srcIdxs) if (i < dstParentPath[sp]) ++shift;
        dstParentPath[sp] -= shift;
    }

    ChainNode* dstParent = nodeAtPath(dstParentPath);
    if (dstParent == nullptr)
    {
        dstParent = &m_host->chain();
        dstParentPath = {};
        dstIndex = static_cast<int>(dstParent->children.size());
    }
    dstIndex = std::clamp(dstIndex, 0, static_cast<int>(dstParent->children.size()));
    for (int k = 0; k < static_cast<int>(picked.size()); ++k)
        dstParent->children.insert(dstParent->children.begin() + dstIndex + k,
                                   std::move(picked[static_cast<size_t>(k)]));

    for (int k = 0; k < static_cast<int>(picked.size()); ++k)
    {
        QList<int> np = dstParentPath;
        np.append(dstIndex + k);
        finalPaths.append(np);
    }
    return true;
}

bool MultiEffectPanel::moveNodeLocked(const QList<int>& srcPath,
                                      const QList<int>& targetPath, ChainDrop where,
                                      QList<int>& finalPath)
{
    if (m_host == nullptr || srcPath.isEmpty()) return false;

    QList<int> srcParentPath = srcPath;
    const int srcIdx = srcParentPath.takeLast();

    // --- Resolve the destination (parent path + insertion index) -------------
    QList<int> dstParentPath;
    int dstIndex = 0;
    if (targetPath.isEmpty() || where == ChainDrop::Viewport)
    {
        dstParentPath = {};  // dropped on empty space -> end of root
        dstIndex = static_cast<int>(m_host->chain().children.size());
    }
    else
    {
        QList<int> targetParentPath = targetPath;
        const int targetIdx = targetParentPath.takeLast();
        if (where == ChainDrop::OnItem)
        {
            ChainNode* tnode = nodeAtPath(targetPath);
            if (tnode != nullptr && tnode->isContainer())
            {
                dstParentPath = targetPath;  // drop INTO the group (append)
                dstIndex = static_cast<int>(tnode->children.size());
            }
            else  // onto a leaf -> place right after it
            {
                dstParentPath = targetParentPath;
                dstIndex = targetIdx + 1;
            }
        }
        else if (where == ChainDrop::Above)
        {
            dstParentPath = targetParentPath;
            dstIndex = targetIdx;
        }
        else  // ChainDrop::Below
        {
            dstParentPath = targetParentPath;
            dstIndex = targetIdx + 1;
        }
    }

    // --- Guards: no self-drop / into own subtree, and no no-op ---------------
    const int sp = static_cast<int>(srcParentPath.size());
    const bool intoSubtree = dstParentPath.size() >= srcPath.size() &&
                             dstParentPath.mid(0, srcPath.size()) == srcPath;
    if (dstParentPath == srcPath || intoSubtree) return false;
    if (dstParentPath == srcParentPath && (dstIndex == srcIdx || dstIndex == srcIdx + 1))
    {
        return false;  // dropped back onto its own slot
    }

    // Tiefenregel (HG1): Teilbaum MIT Host-Gruppe nicht in eine Gruppe ziehen.
    if (pathTouchesHostGroup(m_host->chain(), dstParentPath))
    {
        ChainNode* srcNode = nodeAtPath(srcPath);
        if (srcNode != nullptr && chainHasHostGroup(*srcNode)) return false;
    }

    // --- Extract from source, then insert (adjust dst for the removal) -------
    ChainNode* srcParent = nodeAtPath(srcParentPath);
    if (srcParent == nullptr || srcIdx < 0 ||
        srcIdx >= static_cast<int>(srcParent->children.size()))
    {
        return false;
    }
    ChainNode moved = std::move(srcParent->children[static_cast<size_t>(srcIdx)]);
    srcParent->children.erase(srcParent->children.begin() + srcIdx);

    // The erase shifts indices at/after srcIdx within srcParent's vector.
    if (dstParentPath.size() > sp && dstParentPath.mid(0, sp) == srcParentPath &&
        dstParentPath[sp] > srcIdx)
    {
        dstParentPath[sp] -= 1;  // dst lives under a sibling that shifted down
    }
    else if (dstParentPath == srcParentPath && dstIndex > srcIdx)
    {
        dstIndex -= 1;
    }

    ChainNode* dstParent = nodeAtPath(dstParentPath);
    if (dstParent == nullptr)
    {
        // Should not happen after the guards; fall back to root append.
        dstParent = &m_host->chain();
        dstParentPath = {};
        dstIndex = static_cast<int>(dstParent->children.size());
    }
    dstIndex = std::clamp(dstIndex, 0, static_cast<int>(dstParent->children.size()));
    dstParent->children.insert(dstParent->children.begin() + dstIndex, std::move(moved));

    finalPath = dstParentPath;
    finalPath.append(dstIndex);
    return true;
}

QTreeWidgetItem* MultiEffectPanel::itemAtPath(const QList<int>& path) const
{
    if (path.isEmpty()) return nullptr;
    QTreeWidgetItem* item = m_tree->topLevelItem(path[0]);
    for (int i = 1; i < path.size() && item != nullptr; ++i)
    {
        const int seg = path[i];
        const bool inMilk = seg >= kMilkSectionBase || path[i - 1] >= kMilkSectionBase;
        if (inMilk)
        {
            // Milk-Sektionen/-Elemente liegen an beliebiger Kindposition (nach
            // evtl. Chain-Kindern) — ueber den gespeicherten UserRole-Pfad suchen
            QTreeWidgetItem* found = nullptr;
            for (int c = 0; c < item->childCount(); ++c)
            {
                const QList<int> childPath =
                    pathFromVariant(item->child(c)->data(0, Qt::UserRole));
                if (childPath.size() == i + 1 && childPath.last() == seg)
                {
                    found = item->child(c);
                    break;
                }
            }
            item = found;
        }
        else
        {
            item = item->child(seg);
        }
    }
    return item;
}

void MultiEffectPanel::selectByPath(const QList<int>& path)
{
    if (QTreeWidgetItem* item = itemAtPath(path))
    {
        m_tree->setCurrentItem(item);
        m_tree->scrollToItem(item);
    }
}

void MultiEffectPanel::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_updating || item == nullptr) return;
    const QList<int> path = pathFromVariant(item->data(0, Qt::UserRole));
    if (column == 0)
    {
        // Column 0 is the editable name (enable lives on the eye toggle now).
        const std::string name = item->text(0).toStdString();
        mutate(path, [&](ChainNode& n) { n.displayName = name; });
    }
    else if (column == 3)
    {
        const std::string desc = item->text(3).toStdString();
        mutate(path, [&](ChainNode& n) { n.description = desc; });
    }
}

void MultiEffectPanel::onSelectionChanged()
{
    enforceSameLevelSelection();
    buildPropertyEditor(currentPath());
}

QList<QList<int>> MultiEffectPanel::selectedPaths() const
{
    QList<QList<int>> out;
    const QList<QTreeWidgetItem*> items = m_tree->selectedItems();
    for (QTreeWidgetItem* it : items)
    {
        const QList<int> p = pathFromVariant(it->data(0, Qt::UserRole));
        if (!p.isEmpty()) out.append(p);
    }
    if (out.isEmpty())
    {
        const QList<int> cur = currentPath();
        if (!cur.isEmpty()) out.append(cur);
    }
    std::sort(out.begin(), out.end(), [](const QList<int>& a, const QList<int>& b) {
        for (int i = 0; i < a.size() && i < b.size(); ++i)
            if (a[i] != b[i]) return a[i] < b[i];
        return a.size() < b.size();
    });
    return out;
}

void MultiEffectPanel::enforceSameLevelSelection()
{
    if (m_selecting) return;
    QTreeWidgetItem* anchor = m_tree->currentItem();
    if (anchor == nullptr) return;
    QList<int> anchorParent = pathFromVariant(anchor->data(0, Qt::UserRole));
    if (anchorParent.isEmpty()) return;
    anchorParent.removeLast();  // the effect-list level of the anchor

    m_selecting = true;
    for (QTreeWidgetItem* it : m_tree->selectedItems())
    {
        QList<int> p = pathFromVariant(it->data(0, Qt::UserRole));
        QList<int> parent = p;
        if (!parent.isEmpty()) parent.removeLast();
        if (p.isEmpty() || parent != anchorParent) it->setSelected(false);
    }
    m_selecting = false;
}

void MultiEffectPanel::selectPaths(const QList<QList<int>>& paths)
{
    m_selecting = true;
    m_tree->clearSelection();
    QTreeWidgetItem* last = nullptr;
    for (const QList<int>& p : paths)
    {
        if (QTreeWidgetItem* it = itemAtPath(p))
        {
            it->setSelected(true);
            last = it;
        }
    }
    m_selecting = false;
    if (last != nullptr)
    {
        m_tree->setCurrentItem(last);
        m_tree->scrollToItem(last);
    }
}

// =============================================================================
// Property editor
// =============================================================================

void MultiEffectPanel::clearPropertyEditor()
{
    // Delete the whole editor page at once — deleting just layout items left the
    // nested QFormLayout's widgets alive (they stacked up → overlap + lag).
    delete m_propPage;
    m_propPage = nullptr;
}

#if defined(_MSC_VER)
#pragma warning(push)
// The per-type `else if (auto* p = std::get_if<...>)` chain intentionally
// reuses `p` in mutually-exclusive branches — harmless shadowing.
#pragma warning(disable : 4456)
#endif
void MultiEffectPanel::addImageRow(
    QFormLayout* form, const QList<int>& path, const std::string& filename,
    const std::string& imageData,
    std::function<void(ChainNode&, std::string, std::string)> set)
{
    // Bild-Knoten (Picture · Picture II · Texer · Texer II) tragen das Bild
    // base64-eingebettet, damit eine .lvfx-Kette fuer sich steht. Bis S53 zeigte
    // das Panel nur, OB das geklappt hat — auswaehlen liess sich nichts
    // (Vorgabe Patrik aus S50, hier vorgezogen).
    auto* row = new QWidget(m_propContainer);
    auto* hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);

    const bool embedded = !imageData.empty();
    auto* label = new QLabel(row);
    label->setWordWrap(true);
    label->setText(embedded
                       ? tr("✓ %1 (embedded, %2 KB)")
                             .arg(QString::fromStdString(filename).isEmpty()
                                      ? tr("(no name)")
                                      : QFileInfo(QString::fromStdString(filename))
                                            .fileName(),
                                  QString::number(imageData.size() * 3 / 4096))
                       : (filename.empty()
                              ? tr("⚠ no image")
                              : tr("⚠ not found: %1")
                                    .arg(QString::fromStdString(filename))));

    auto* chooseBtn = new QToolButton(row);
    chooseBtn->setText(tr("Choose…"));
    chooseBtn->setToolTip(tr("Pick an image file; it is embedded into the chain "
                             "so the preset stays self-contained."));
    connect(chooseBtn, &QToolButton::clicked, this, [this, path, set]() {
        const QString file = QFileDialog::getOpenFileName(
            this, tr("Choose image"), imageStartDir(),
            tr("Images (*.bmp *.jpg *.jpeg *.png *.gif);;All files (*)"));
        if (file.isEmpty()) return;
        QFile f(file);
        if (!f.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, tr("Image"),
                                 tr("Could not read:\n%1").arg(file));
            return;
        }
        const std::string data = f.readAll().toBase64().toStdString();
        f.close();
        const std::string name = QFileInfo(file).fileName().toStdString();
        mutate(path, [&](ChainNode& n) { set(n, name, data); });
        QMetaObject::invokeMethod(
            this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
    });

    auto* clearBtn = new QToolButton(row);
    clearBtn->setText(tr("Clear"));
    clearBtn->setToolTip(tr("Drop the embedded image (the node then draws nothing)"));
    clearBtn->setEnabled(embedded || !filename.empty());
    connect(clearBtn, &QToolButton::clicked, this, [this, path, set]() {
        mutate(path, [&](ChainNode& n) { set(n, std::string{}, std::string{}); });
        QMetaObject::invokeMethod(
            this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
    });

    hl->addWidget(label, 1);
    hl->addWidget(chooseBtn);
    hl->addWidget(clearBtn);
    form->addRow(tr("Image"), row);
}

void MultiEffectPanel::addKernelGrid(QFormLayout* form, const QList<int>& path,
                                     const std::array<int, 49>& kernel)
{
    // 7x7-Gitter (r_convolution): eine Spinbox je Zelle, Mitte hervorgehoben.
    // Der Kernel war bis S53 gar nicht erreichbar — importierte Presets trugen
    // ihn, verstellen liess er sich nie.
    auto* grid = new QWidget(m_propContainer);
    auto* gl = new QGridLayout(grid);
    gl->setContentsMargins(0, 0, 0, 0);
    gl->setSpacing(2);

    for (int i = 0; i < 49; ++i)
    {
        auto* cell = new QSpinBox(grid);
        cell->setRange(-9999, 9999);
        cell->setValue(kernel[static_cast<std::size_t>(i)]);
        cell->setButtonSymbols(QAbstractSpinBox::NoButtons);
        cell->setAlignment(Qt::AlignCenter);
        cell->setMinimumWidth(38);
        if (i == 24)  // Mitte = der Bildpunkt selbst
        {
            QFont f = cell->font();
            f.setBold(true);
            cell->setFont(f);
            cell->setToolTip(tr("Centre tap — the pixel itself"));
        }
        connect(cell, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [this, path, i](int v) {
                    mutate(path, [&](ChainNode& n) {
                        std::get<ConvolutionParams>(n.params)
                            .kernel[static_cast<std::size_t>(i)] = v;
                    });
                });
        gl->addWidget(cell, i / 7, i % 7);
    }

    auto* resetBtn = new QToolButton(grid);
    resetBtn->setText(tr("Identity"));
    resetBtn->setToolTip(tr("Reset to the neutral kernel (centre 1, rest 0)"));
    connect(resetBtn, &QToolButton::clicked, this, [this, path]() {
        mutate(path, [](ChainNode& n) {
            auto& k = std::get<ConvolutionParams>(n.params).kernel;
            k.fill(0);
            k[24] = 1;
        });
        QMetaObject::invokeMethod(
            this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
    });
    gl->addWidget(resetBtn, 7, 0, 1, 7);

    grid->setToolTip(tr("7×7 convolution kernel (row-major). The weighted sum is "
                        "divided by 'Scale' and shifted by 'Bias'."));
    form->addRow(tr("Kernel"), grid);
}

void MultiEffectPanel::addGradientStops(QFormLayout* form, const QList<int>& path,
                                        const std::vector<int>& stopPos,
                                        const std::vector<uint32_t>& stopColor)
{
    // Stuetzstellen der Color-Map: Position 0..255 + Farbe. Beide Vektoren
    // laufen parallel — jede Aenderung haelt sie gleich lang.
    auto* box = new QWidget(m_propContainer);
    auto* vl = new QVBoxLayout(box);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(2);

    const int count = static_cast<int>(std::min(stopPos.size(), stopColor.size()));
    for (int si = 0; si < count; ++si)
    {
        auto* rowW = new QWidget(box);
        auto* rl = new QHBoxLayout(rowW);
        rl->setContentsMargins(0, 0, 0, 0);

        auto* pos = new QSpinBox(rowW);
        pos->setRange(0, 255);
        pos->setValue(stopPos[static_cast<std::size_t>(si)]);
        pos->setToolTip(tr("Input value this stop sits at (0..255)"));
        connect(pos, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [this, path, si](int v) {
                    mutate(path, [&](ChainNode& n) {
                        auto& ps = std::get<ColorMapParams>(n.params).stopPos;
                        if (si < static_cast<int>(ps.size()))
                            ps[static_cast<std::size_t>(si)] = v;
                    });
                });

        const uint32_t initC = stopColor[static_cast<std::size_t>(si)];
        auto* sw = new QPushButton(rowW);
        sw->setFixedSize(40, 20);
        sw->setStyleSheet(QString("background:%1").arg(colorFromU32(initC).name()));
        connect(sw, &QPushButton::clicked, this, [this, path, si, initC, sw]() {
            const QColor picked = QColorDialog::getColor(colorFromU32(initC), this);
            if (!picked.isValid()) return;
            const uint32_t c = u32FromColor(picked);
            sw->setStyleSheet(QString("background:%1").arg(picked.name()));
            mutate(path, [&](ChainNode& n) {
                auto& cs = std::get<ColorMapParams>(n.params).stopColor;
                if (si < static_cast<int>(cs.size()))
                    cs[static_cast<std::size_t>(si)] = c;
            });
        });

        auto* del = new QToolButton(rowW);
        del->setText(QString::fromUtf8("−"));
        del->setToolTip(tr("Remove this stop"));
        connect(del, &QToolButton::clicked, this, [this, path, si]() {
            mutate(path, [&](ChainNode& n) {
                auto& p = std::get<ColorMapParams>(n.params);
                if (si < static_cast<int>(p.stopPos.size()))
                    p.stopPos.erase(p.stopPos.begin() + si);
                if (si < static_cast<int>(p.stopColor.size()))
                    p.stopColor.erase(p.stopColor.begin() + si);
            });
            QMetaObject::invokeMethod(
                this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
        });

        rl->addWidget(pos);
        rl->addWidget(sw);
        rl->addWidget(del);
        rl->addStretch(1);
        vl->addWidget(rowW);
    }

    auto* addBtn = new QToolButton(box);
    addBtn->setText(tr("+ stop"));
    connect(addBtn, &QToolButton::clicked, this, [this, path]() {
        mutate(path, [](ChainNode& n) {
            auto& p = std::get<ColorMapParams>(n.params);
            // Hinter der letzten Stuetzstelle einhaengen — nur der Lesbarkeit
            // wegen: `buildColorMapLut` sortiert und klemmt ohnehin selbst.
            const int last = p.stopPos.empty() ? -1 : p.stopPos.back();
            p.stopPos.push_back(std::min(255, last + 32));
            p.stopColor.push_back(0xFFFFFF);
        });
        QMetaObject::invokeMethod(
            this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
    });
    vl->addWidget(addBtn);

    box->setToolTip(tr("Gradient stops: input position 0..255 and its colour. The "
                       "host interpolates a 256-entry lookup table between them; "
                       "the order in this list does not matter (stops are sorted "
                       "when the table is built)."));
    form->addRow(tr("Gradient stops"), box);
}

bool MultiEffectPanel::askPresetToSave(const EffectParams& params, QString& outName,
                                       QStringList& outFields)
{
    namespace np = lumi::multieffect::nodepresets;
    const QStringList all = np::fieldNames(params);

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Save preset"));
    auto* lay = new QVBoxLayout(&dlg);

    lay->addWidget(new QLabel(tr("Name:"), &dlg));
    auto* nameEdit = new QLineEdit(&dlg);
    lay->addWidget(nameEdit);

    lay->addWidget(new QLabel(
        tr("Fields to store — unchecked ones stay untouched when the preset is "
           "loaded (that is how a figure keeps the node's colour):"),
        &dlg));

    // Ein Kaestchen je Parameter des Knotens; die Liste kommt generisch aus
    // `nodeToJson`, gilt also fuer jeden Typ ohne Zutun.
    auto* listWidget = new QListWidget(&dlg);
    for (const QString& key : all)
    {
        auto* item = new QListWidgetItem(key, listWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }
    listWidget->setMinimumHeight(180);
    lay->addWidget(listWidget, 1);

    auto* toggleRow = new QHBoxLayout();
    auto* allBtn = new QPushButton(tr("All"), &dlg);
    auto* noneBtn = new QPushButton(tr("None"), &dlg);
    toggleRow->addWidget(allBtn);
    toggleRow->addWidget(noneBtn);
    toggleRow->addStretch(1);
    lay->addLayout(toggleRow);
    const auto setAll = [listWidget](Qt::CheckState state) {
        for (int i = 0; i < listWidget->count(); ++i)
            listWidget->item(i)->setCheckState(state);
    };
    connect(allBtn, &QPushButton::clicked, &dlg,
            [setAll] { setAll(Qt::Checked); });
    connect(noneBtn, &QPushButton::clicked, &dlg,
            [setAll] { setAll(Qt::Unchecked); });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                         &dlg);
    lay->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) return false;
    outName = nameEdit->text().trimmed();
    if (outName.isEmpty()) return false;

    outFields.clear();
    for (int i = 0; i < listWidget->count(); ++i)
        if (listWidget->item(i)->checkState() == Qt::Checked)
            outFields.append(listWidget->item(i)->text());
    if (outFields.isEmpty())
    {
        QMessageBox::warning(this, tr("Save preset"),
                             tr("A preset without a single field would do nothing "
                                "when loaded."));
        return false;
    }
    return true;
}

void MultiEffectPanel::addPresetRow(QFormLayout* form, const QList<int>& path,
                                    const EffectParams& params)
{
    namespace np = lumi::multieffect::nodepresets;
    const QString typeKey = effectTypeKey(params);
    if (typeKey.isEmpty()) return;

    auto* row = new QWidget(m_propContainer);
    auto* hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);

    auto* combo = new QComboBox(row);
    combo->addItem(tr("(unchanged)"));  // Index 0 = kein Preset, nur Beschriftung
    const std::vector<np::Entry> entries = np::list(typeKey);
    for (const np::Entry& e : entries)
        combo->addItem(e.builtin ? e.name : e.name + QStringLiteral(" *"));
    combo->setToolTip(tr("Named parameter sets for this node type (* = your own). "
                         "Loading replaces every parameter including the scripts."));

    auto* saveBtn = new QToolButton(row);
    saveBtn->setText(tr("Save as…"));
    saveBtn->setToolTip(tr("Store the current parameters under a name"));
    auto* delBtn = new QToolButton(row);
    delBtn->setText(QString::fromUtf8("🗑"));
    delBtn->setToolTip(tr("Delete the selected preset (your own only)"));
    delBtn->setEnabled(false);

    hl->addWidget(combo, 1);
    hl->addWidget(saveBtn);
    hl->addWidget(delBtn);

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, path, typeKey, entries, delBtn](int idx) {
                const int i = idx - 1;  // Index 0 ist die Beschriftung
                if (i < 0 || i >= static_cast<int>(entries.size()))
                {
                    delBtn->setEnabled(false);
                    return;
                }
                const np::Entry& e = entries[static_cast<std::size_t>(i)];
                delBtn->setEnabled(!e.builtin);

                // Frisch aus dem Modell: die Datei wird DARUEBER gelegt, ein
                // Teil-Preset (Figur) laesst den Rest der Parameter stehen.
                EffectParams loaded;
                {
                    QMutexLocker lock(m_mutex);
                    const ChainNode* node = nodeAtPath(path);
                    if (node == nullptr) return;
                    loaded = node->params;
                }
                QStringList report;
                if (!np::load(e.path, typeKey, loaded, &report))
                {
                    // Dialog direkt (wie ConfigPanel): Panels sind nur im
                    // Fensterbetrieb bedienbar — die Vollbild-Regel aus S52
                    // (MainWindow::reportProblem) greift hier nicht.
                    QMessageBox::warning(
                        this, tr("Preset"),
                        tr("Preset '%1' could not be loaded.\n%2")
                            .arg(e.name, report.join(QLatin1Char('\n'))));
                    return;
                }
                // Eine Mutation: unter renderMutex + recompileChain, undo-faehig.
                mutate(path, [&](ChainNode& n) { n.params = loaded; });
                // Die Werte haben sich alle geaendert — Editor neu aufbauen (nicht
                // aus dem Signal heraus, sonst stirbt das Widget unter uns).
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); },
                    Qt::QueuedConnection);
            });

    connect(saveBtn, &QToolButton::clicked, this, [this, path]() {
        // Frisch aus dem Modell lesen: der `params`-Schnappschuss des Editors ist
        // vom Aufbau, zwischenzeitliche Reglerbewegungen fehlen darin.
        EffectParams current;
        {
            QMutexLocker lock(m_mutex);
            const ChainNode* node = nodeAtPath(path);
            if (node == nullptr) return;
            current = node->params;
        }

        QString name;
        QStringList fields;
        if (!askPresetToSave(current, name, fields)) return;

        QString written;
        if (!np::save(name, current, fields, &written))
        {
            QMessageBox::warning(this, tr("Preset"),
                                 tr("Preset '%1' could not be saved.").arg(name));
            return;
        }
        QMetaObject::invokeMethod(
            this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
    });

    connect(delBtn, &QToolButton::clicked, this,
            [this, path, entries, combo]() {
                const int i = combo->currentIndex() - 1;
                if (i < 0 || i >= static_cast<int>(entries.size())) return;
                const np::Entry& e = entries[static_cast<std::size_t>(i)];
                if (QMessageBox::question(
                        this, tr("Delete preset"),
                        tr("Delete the preset '%1' for good?").arg(e.name)) !=
                    QMessageBox::Yes)
                    return;
                if (!np::remove(e))
                {
                    QMessageBox::warning(
                        this, tr("Preset"),
                        tr("Preset '%1' could not be deleted.").arg(e.name));
                    return;
                }
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); },
                    Qt::QueuedConnection);
            });

    form->addRow(tr("Preset"), row);
}

void MultiEffectPanel::buildPropertyEditor(const QList<int>& rawPath)
{
    clearPropertyEditor();
    if (m_host == nullptr || rawPath.isEmpty()) return;

    // Milkdrop-Sektion/-Element (Anzeige-Kinder) vom echten Chain-Pfad
    // abtrennen: alle Mutations-Lambdas fangen unten das bereinigte `path` —
    // Sentinels erreichen nodeAtPath nie. milkElem >= 0 = Einzel-Element-Sicht.
    const MilkPathInfo milkPath = splitMilkPath(rawPath);
    const int milkSection = milkPath.section;
    const int milkElem = milkPath.element;
    if (milkPath.nodePath.isEmpty()) return;
    const QList<int> path = milkPath.nodePath;

    EffectParams params;
    std::string nodeName;
    std::string nodeDesc;
    {
        QMutexLocker lock(m_mutex);
        ChainNode* node = nodeAtPath(path);
        if (node == nullptr) return;
        params = node->params;
        nodeName = node->displayName;
        nodeDesc = node->description;
    }

    m_propPage = new QWidget(m_propContainer);
    auto* form = new QFormLayout(m_propPage);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    // Voreinstellungen (Knoten-Parameter-Konzept §3, Etappe 1): eine Zeile fuer
    // JEDEN Knotentyp — der Store arbeitet ueber effectTypeKey + nodeToJson und
    // kennt keinen einzigen Typ beim Namen. Nur in der echten Knotenansicht:
    // Milkdrop-Sektionen/-Elemente sind Navigations-Items ohne eigene params.
    if (milkSection < 0 && milkElem < 0) addPresetRow(form, path, params);

    // Editable name + free-text description (commit on Enter / focus-out).
    auto addLine = [&](const QString& label, const QString& value,
                       std::function<void(ChainNode&, std::string)> set) {
        auto* edit = new QLineEdit(value, m_propPage);
        connect(edit, &QLineEdit::editingFinished, this, [this, path, set, edit]() {
            const std::string t = edit->text().toStdString();
            mutate(path, [&](ChainNode& n) { set(n, t); });
        });
        form->addRow(label, edit);
    };
    addLine(tr("Name"), QString::fromStdString(nodeName),
            [](ChainNode& n, std::string v) { n.displayName = std::move(v); });
    addLine(tr("Description"), QString::fromStdString(nodeDesc),
            [](ChainNode& n, std::string v) { n.description = std::move(v); });

    // --- row helpers (each connects to a typed mutation) ---------------------
    auto addInt = [&](const QString& label, int value, int lo, int hi,
                      std::function<void(ChainNode&, int)> set) {
        auto* spin = new QSpinBox(m_propContainer);
        spin->setRange(lo, hi);
        spin->setValue(value);
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [this, path, set](int v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        form->addRow(label, spin);
    };
    auto addDouble = [&](const QString& label, double value, double lo, double hi,
                         double step, std::function<void(ChainNode&, double)> set) {
        auto* spin = new QDoubleSpinBox(m_propContainer);
        spin->setRange(lo, hi);
        spin->setSingleStep(step);
        spin->setDecimals(3);
        spin->setValue(value);
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this, path, set](double v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        form->addRow(label, spin);
    };
    auto addBool = [&](const QString& label, bool value,
                       std::function<void(ChainNode&, bool)> set) {
        auto* box = new QCheckBox(m_propContainer);
        box->setChecked(value);
        connect(box, &QCheckBox::toggled, this, [this, path, set](bool v) {
            mutate(path, [&](ChainNode& n) { set(n, v); });
        });
        form->addRow(label, box);
    };
    auto addColor = [&](const QString& label, uint32_t value,
                        std::function<void(ChainNode&, uint32_t)> set) {
        auto* btn = new QPushButton(m_propContainer);
        auto paint = [btn](uint32_t c) {
            btn->setText(QString("#%1").arg(c, 6, 16, QChar('0')).toUpper());
            btn->setStyleSheet(QString("background:%1").arg(colorFromU32(c).name()));
        };
        paint(value);
        connect(btn, &QPushButton::clicked, this, [this, path, set, paint, value]() {
            const QColor picked = QColorDialog::getColor(colorFromU32(value), this);
            if (!picked.isValid()) return;
            const uint32_t c = u32FromColor(picked);
            paint(c);
            mutate(path, [&](ChainNode& n) { set(n, c); });
        });
        form->addRow(label, btn);
    };
    // Wie addDouble, aber fuer KLASSE-A-Werte: die Vorgabe ist der Wert des
    // Referenz-Renderers, jede Abweichung entfernt das Bild messbar von AVS.
    // Die Zeile sagt das (Entscheid Patrik §8.4) — sonst sieht man einem Preset
    // spaeter nicht an, warum es in der Kalibrierung ausschert.
    auto addRefDouble = [&](const QString& label, double value, double lo, double hi,
                            double step, double reference, int decimals,
                            std::function<void(ChainNode&, double)> set) {
        auto* spin = new QDoubleSpinBox(m_propContainer);
        spin->setRange(lo, hi);
        spin->setSingleStep(step);
        spin->setDecimals(decimals);
        spin->setValue(value);
        auto* lbl = new QLabel(label, m_propContainer);
        const auto mark = [lbl, label, reference, this](double v) {
            const bool off = std::abs(v - reference) > 1e-6;
            lbl->setText(off ? label + QStringLiteral(" ⚠") : label);
            lbl->setToolTip(off ? tr("Differs from the AVS reference value (%1) — "
                                     "this node no longer matches the original.")
                                      .arg(reference)
                                : tr("AVS reference value"));
        };
        mark(value);
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this, path, set, mark](double v) {
                    mark(v);
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        form->addRow(lbl, spin);
    };
    // Hinweiszeile fuer KLASSE-A-Knoten mit Parameter-Skript. Die ⚠ an den
    // Reglern sieht nur FESTE Abweichungen — was ein Frame-Skript rechnet,
    // kann sie nicht kennen. Deshalb sagt der Knoten es hier von sich aus.
    auto addReferenceNote = [&](bool scripted) {
        auto* note = new QLabel(m_propContainer);
        note->setWordWrap(true);
        note->setText(scripted
                          ? tr("⚠ This node mirrors AVS line by line. Its script "
                               "recomputes the values every frame — the reference "
                               "match no longer holds.")
                          : tr("This node mirrors AVS line by line. A script here "
                               "departs from the reference."));
        form->addRow(note);
    };
    auto addEnum = [&](const QString& label, int index, const QStringList& items,
                       std::function<void(ChainNode&, int)> set) {
        auto* combo = new QComboBox(m_propContainer);
        combo->addItems(items);
        combo->setCurrentIndex(index);
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this, path, set](int v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        form->addRow(label, combo);
    };
    // Farbtafel: ein Farbfeld je Eintrag plus Hinzufuegen/Entfernen. Der Zugriff
    // auf den Vektor kommt als Funktion herein, damit sich JEDER Knoten mit
    // Palette dieselbe Zeile teilt (SuperScope, Metaballs 3D, Tentacles 3D) —
    // Laenge aendern heisst Editor neu bauen, deshalb der verzoegerte Aufruf.
    auto addColorTable = [&](const QString& label, const std::vector<uint32_t>& colors,
                             std::function<std::vector<uint32_t>&(ChainNode&)> access,
                             const QString& toolTip) {
        auto* colorsWidget = new QWidget(m_propContainer);
        auto* row = new QHBoxLayout(colorsWidget);
        row->setContentsMargins(0, 0, 0, 0);
        for (int ci = 0; ci < static_cast<int>(colors.size()); ++ci)
        {
            const uint32_t initC = colors[static_cast<size_t>(ci)];
            auto* sw = new QPushButton(colorsWidget);
            sw->setFixedSize(24, 20);
            sw->setStyleSheet(QString("background:%1").arg(colorFromU32(initC).name()));
            connect(sw, &QPushButton::clicked, this,
                    [this, path, ci, initC, sw, access]() {
                        const QColor picked =
                            QColorDialog::getColor(colorFromU32(initC), this);
                        if (!picked.isValid()) return;
                        const uint32_t c = u32FromColor(picked);
                        sw->setStyleSheet(QString("background:%1").arg(picked.name()));
                        mutate(path, [&](ChainNode& n) {
                            std::vector<uint32_t>& cs = access(n);
                            if (ci < static_cast<int>(cs.size()))
                                cs[static_cast<size_t>(ci)] = c;
                        });
                    });
            row->addWidget(sw);
        }
        auto* addC = new QToolButton(colorsWidget);
        addC->setText("+");
        addC->setToolTip(tr("Add color"));
        connect(addC, &QToolButton::clicked, this, [this, path, access]() {
            mutate(path, [&](ChainNode& n) { access(n).push_back(0xFFFFFF); });
            QMetaObject::invokeMethod(
                this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
        });
        auto* delC = new QToolButton(colorsWidget);
        delC->setText(QString::fromUtf8("−"));
        delC->setToolTip(tr("Remove last color"));
        connect(delC, &QToolButton::clicked, this, [this, path, access]() {
            mutate(path, [&](ChainNode& n) {
                std::vector<uint32_t>& cs = access(n);
                if (!cs.empty()) cs.pop_back();
            });
            QMetaObject::invokeMethod(
                this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
        });
        row->addWidget(addC);
        row->addWidget(delC);
        row->addStretch();
        colorsWidget->setToolTip(toolTip);
        form->addRow(label, colorsWidget);
    };
    // Milkdrop-Sektionen tauschen die Referenz unten gegen ihr Original-Set
    // aus (Befund S42: eine generische Tabelle passte zu keinem Slot richtig)
    QString scriptRef = scriptReferenceHtml(params);

    // Global conflict set: globals this node's init declares that another node's
    // init also declares (Skript_Variablen_Konzept §6.3) — flagged red in init.
    QSet<QString> initConflicts;
    {
        QSet<QString> cur, others;
        addInitGlobalWrites(nodeInitCode(params), cur);
        if (!cur.isEmpty() && m_host != nullptr && m_mutex != nullptr)
        {
            QMutexLocker lock(m_mutex);
            const ChainNode* node = nodeAtPath(path);
            collectInitGlobalsExcept(m_host->chain(), node, others);
        }
        for (const QString& g : cur)
            if (others.contains(g)) initConflicts.insert(g);
    }

    auto addScript = [&](const QString& label, const std::string& value,
                         std::function<void(ChainNode&, std::string)> set) {
        const bool isInit = label.startsWith(QLatin1String("Init"));
        const QSet<QString> conflicts = isInit ? initConflicts : QSet<QString>{};
        auto* edit = new QPlainTextEdit(m_propContainer);
        edit->setPlainText(QString::fromStdString(value));
        edit->setPlaceholderText(tr("EEL expression"));
        edit->setLineWrapMode(QPlainTextEdit::NoWrap);
        edit->setMinimumHeight(54);
        edit->setMaximumHeight(200);  // taller inline; full editing via Expand
        QFont mono(QStringLiteral("Consolas"));
        mono.setStyleHint(QFont::Monospace);
        edit->setFont(mono);
        new EelHighlighter(edit->document(), conflicts);
        connect(edit, &QPlainTextEdit::textChanged, this, [this, path, set, edit]() {
            const std::string text = edit->toPlainText().toStdString();
            mutate(path, [&](ChainNode& n) { set(n, text); });
        });

        // Header: label + reference (ⓘ) and expand-to-large-editor (⤢) buttons.
        auto* header = new QWidget(m_propContainer);
        auto* hl = new QHBoxLayout(header);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->addWidget(new QLabel(label, header));
        hl->addStretch(1);
        auto* varsBtn = new QToolButton(header);
        varsBtn->setText(QString::fromUtf8("ⓘ"));  // ⓘ
        varsBtn->setToolTip(tr("Show this module's script variables"));
        auto* expandBtn = new QToolButton(header);
        expandBtn->setText(QString::fromUtf8("⤢"));  // ⤢
        expandBtn->setToolTip(tr("Expand to a large, resizable editor"));
        hl->addWidget(varsBtn);
        hl->addWidget(expandBtn);
        connect(varsBtn, &QToolButton::clicked, this,
                [this, scriptRef]() { showScriptReference(this, scriptRef); });
        connect(expandBtn, &QToolButton::clicked, this,
                [this, edit, label, scriptRef, conflicts]() {
                    QString out;
                    if (openScriptEditor(this, label, edit->toPlainText(), scriptRef,
                                         out, conflicts))
                    {
                        edit->setPlainText(out);  // triggers textChanged -> mutate
                    }
                });

        auto* box = new QWidget(m_propContainer);
        auto* v = new QVBoxLayout(box);
        v->setContentsMargins(0, 0, 0, 0);
        v->addWidget(header);
        v->addWidget(edit);
        form->addRow(box);
    };
    auto addGradient = [&](const QString& label, const std::string& value,
                           std::function<void(ChainNode&, std::string)> set) {
        auto* combo = new QComboBox(m_propContainer);
        lumi::modules::ColorGradientModule grad;
        for (const std::string& name : grad.presetNames())
            combo->addItem(QString::fromStdString(name));
        combo->setCurrentText(QString::fromStdString(value));
        connect(combo, &QComboBox::currentTextChanged, this,
                [this, path, set](const QString& t) {
                    const std::string name = t.toStdString();
                    mutate(path, [&](ChainNode& n) { set(n, name); });
                });
        form->addRow(label, combo);
    };
    auto addText = [&](const QString& label, const std::string& value,
                       std::function<void(ChainNode&, std::string)> set) {
        auto* edit = new QLineEdit(QString::fromStdString(value), m_propContainer);
        connect(edit, &QLineEdit::editingFinished, this, [this, path, set, edit]() {
            const std::string t = edit->text().toStdString();
            mutate(path, [&](ChainNode& n) { set(n, t); });
        });
        form->addRow(label, edit);
    };

    const QStringList kBlendNames = {"Ignore", "Replace", "50/50", "Maximum",
                                     "Additive", "Sub 1-2", "Sub 2-1", "EveryLine",
                                     "EveryPixel", "XOR", "Adjustable", "Multiply",
                                     "Buffer", "Minimum"};

    // --- per-type fields -----------------------------------------------------
    if (auto* p = std::get_if<ListParams>(&params))
    {
        addBool(tr("Clear each frame"), p->clearEveryFrame,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).clearEveryFrame = v; });
        addEnum(tr("Blend In"), static_cast<int>(p->blendIn), kBlendNames,
                [](ChainNode& n, int v) { std::get<ListParams>(n.params).blendIn = static_cast<BlendMode>(v); });
        addEnum(tr("Blend Out"), static_cast<int>(p->blendOut), kBlendNames,
                [](ChainNode& n, int v) { std::get<ListParams>(n.params).blendOut = static_cast<BlendMode>(v); });
        addInt(tr("In Alpha"), p->inAdjustAlpha, 0, 255,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).inAdjustAlpha = v; });
        addInt(tr("Out Alpha"), p->outAdjustAlpha, 0, 255,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).outAdjustAlpha = v; });
        // Only relevant when the matching blend is "Buffer": pool slot + invert.
        addInt(tr("In Buffer"), p->bufferIn, 0, 7,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).bufferIn = v; });
        addInt(tr("Out Buffer"), p->bufferOut, 0, 7,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).bufferOut = v; });
        addBool(tr("In Buffer invert"), p->bufferInInvert,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).bufferInInvert = v; });
        addBool(tr("Out Buffer invert"), p->bufferOutInvert,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).bufferOutInvert = v; });
        addBool(tr("OnBeat render"), p->onBeatRender,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).onBeatRender = v; });
        addInt(tr("OnBeat frames"), p->onBeatFrames, 1, 200,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).onBeatFrames = v; });
        addBool(tr("Use list code"), p->useCode,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).useCode = v; });
        addScript(tr("Init code"), p->initCode,
                  [](ChainNode& n, std::string v) { std::get<ListParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode,
                  [](ChainNode& n, std::string v) { std::get<ListParams>(n.params).frameCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ClearParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<ClearParams>(n.params).color = v; });
        addBool(tr("Only first frame"), p->onlyFirst, [](ChainNode& n, bool v) { std::get<ClearParams>(n.params).onlyFirst = v; });
        const QStringList kClearBlendNames = {tr("Replace"), tr("Additive"),
                                              tr("50/50"), tr("Line blend")};
        addEnum(tr("Blend"), p->blend, kClearBlendNames, [](ChainNode& n, int v) { std::get<ClearParams>(n.params).blend = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ClearParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ClearParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ClearParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FadeoutParams>(&params))
    {
        addInt(tr("Fade length"), p->fadeLen, 0, 92, [](ChainNode& n, int v) { std::get<FadeoutParams>(n.params).fadeLen = v; });
        addColor(tr("Target color"), p->color, [](ChainNode& n, uint32_t v) { std::get<FadeoutParams>(n.params).color = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<FadeoutParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FadeoutParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FadeoutParams>(n.params).beatCode = std::move(v); });
    }
    else if (std::holds_alternative<InvertParams>(params))
    {
        form->addRow(new QLabel(tr("(no parameters)"), m_propContainer));
    }
    else if (auto* p = std::get_if<BrightnessParams>(&params))
    {
        addInt(tr("Red"), p->red, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).red = v; });
        addInt(tr("Green"), p->green, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).green = v; });
        addInt(tr("Blue"), p->blue, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).blue = v; });
        addBool(tr("Exclude color"), p->exclude, [](ChainNode& n, bool v) { std::get<BrightnessParams>(n.params).exclude = v; });
        addColor(tr("Exclude"), p->color, [](ChainNode& n, uint32_t v) { std::get<BrightnessParams>(n.params).color = v; });
        addInt(tr("Distance"), p->distance, 0, 255, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).distance = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BrightnessParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BrightnessParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BrightnessParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FastBrightnessParams>(&params))
    {
        addEnum(tr("Mode"), p->dir, {"x2", "x0.5", "off"}, [](ChainNode& n, int v) { std::get<FastBrightnessParams>(n.params).dir = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<FastBrightnessParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FastBrightnessParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FastBrightnessParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BlurParams>(&params))
    {
        addEnum(tr("Strength"), p->strength - 1, {"Light", "Normal", "Heavy"},
                [](ChainNode& n, int v) { std::get<BlurParams>(n.params).strength = v + 1; });
        addBool(tr("Round up"), p->roundUp, [](ChainNode& n, bool v) { std::get<BlurParams>(n.params).roundUp = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BlurParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BlurParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BlurParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MirrorParams>(&params))
    {
        auto setModeBit = [](int bit) {
            return [bit](ChainNode& n, bool v) {
                int& m = std::get<MirrorParams>(n.params).mode;
                m = v ? (m | bit) : (m & ~bit);
            };
        };
        addBool(tr("Top -> Bottom"), (p->mode & 1) != 0, setModeBit(1));
        addBool(tr("Bottom -> Top"), (p->mode & 2) != 0, setModeBit(2));
        addBool(tr("Left -> Right"), (p->mode & 4) != 0, setModeBit(4));
        addBool(tr("Right -> Left"), (p->mode & 8) != 0, setModeBit(8));
        addBool(tr("OnBeat random"), p->onBeatRandom, [](ChainNode& n, bool v) { std::get<MirrorParams>(n.params).onBeatRandom = v; });
        addBool(tr("Smooth transition"), p->smooth, [](ChainNode& n, bool v) { std::get<MirrorParams>(n.params).smooth = v; });
        addInt(tr("Slower (frames/step)"), p->slower, 1, 16, [](ChainNode& n, int v) { std::get<MirrorParams>(n.params).slower = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MirrorParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MirrorParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MirrorParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<OnBeatClearParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<OnBeatClearParams>(n.params).color = v; });
        addInt(tr("Every N beats"), p->everyNBeats, 1, 100, [](ChainNode& n, int v) { std::get<OnBeatClearParams>(n.params).everyNBeats = v; });
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<OnBeatClearParams>(n.params).blend = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<OnBeatClearParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<OnBeatClearParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<OnBeatClearParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ColorfadeParams>(&params))
    {
        addInt(tr("Fader R"), p->faderR, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderR = v; });
        addInt(tr("Fader G"), p->faderG, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderG = v; });
        addInt(tr("Fader B"), p->faderB, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderB = v; });
        addInt(tr("Beat Fader R"), p->beatFaderR, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderR = v; });
        addInt(tr("Beat Fader G"), p->beatFaderG, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderG = v; });
        addInt(tr("Beat Fader B"), p->beatFaderB, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderB = v; });
        addInt(tr("Beat frames"), p->onBeatFrames, 1, 200, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).onBeatFrames = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorfadeParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorfadeParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorfadeParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ColorModifierParams>(&params))
    {
        addBool(tr("Recompute/frame"), p->recompute, [](ChainNode& n, bool v) { std::get<ColorModifierParams>(n.params).recompute = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Level"), p->levelCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).levelCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MovementParams>(&params))
    {
        addBool(tr("Rect coords"), p->rectCoords, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).rectCoords = v; });
        addBool(tr("Wrap"), p->wrap, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).wrap = v; });
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).blend = v; });
        addBool(tr("Subpixel (bilinear)"), p->subpixel, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).subpixel = v; });
        const QStringList kSourceMappedNames = {tr("Off"), tr("On (scatter)"),
                                                tr("Off, toggle on beat"),
                                                tr("On, toggle on beat")};
        addEnum(tr("Source mapped"), p->sourceMapped, kSourceMappedNames, [](ChainNode& n, int v) { std::get<MovementParams>(n.params).sourceMapped = v; });
        // r_trans-Builtins OHNE eval_desc: reine Pixel-Index-Umlegungen, für die
        // es keine d/r-Formel gibt. Ist einer gewählt, wird der Point-Code nicht
        // ausgewertet (S35).
        addEnum(tr("Builtin remap"), p->builtinRemap == 7 ? 2 : p->builtinRemap,
                {tr("Off (use code)"), tr("Slight fuzzify"), tr("Blocky partial out")},
                [](ChainNode& n, int v) {
                    std::get<MovementParams>(n.params).builtinRemap =
                        v == 2 ? 7 : v;  // 0 · 1 · 7 sind die echten Werte
                });
        addScript(tr("Point code"), p->code, [](ChainNode& n, std::string v) { std::get<MovementParams>(n.params).code = std::move(v); });
        // Befund Patrik (S53, movement3b.lvfx): eine Beat-Umkehr im Point-Code
        // kann hier NICHT wirken — zweimal nicht. Der Editor sagt es an, sonst
        // laeuft der naechste wieder hinein.
        {
            auto* note = new QLabel(
                tr("Movement builds a STATIC table: the point code runs once per "
                   "size/text change, not per frame (like AVS r_trans). A value "
                   "that changes over time — a beat counter, say — cannot move "
                   "the image here. Use Dynamic Movement for that: it evaluates "
                   "every frame and has Init/Frame/Beat/Point in one script."),
                m_propContainer);
            note->setWordWrap(true);
            form->addRow(note);
        }
    }
    else if (auto* p = std::get_if<DynamicMovementParams>(&params))
    {
        addInt(tr("Grid X"), p->xres, 2, 256, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).xres = v; });
        addInt(tr("Grid Y"), p->yres, 2, 256, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).yres = v; });
        addBool(tr("Rect coords"), p->rectCoords, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).rectCoords = v; });
        addBool(tr("Wrap"), p->wrap, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).wrap = v; });
        addBool(tr("Blend (alpha)"), p->blend, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).blend = v; });
        addBool(tr("No move"), p->nomove, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).nomove = v; });
        addBool(tr("Subpixel (bilinear)"), p->subpixel, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).subpixel = v; });
        addInt(tr("Source buffer (0 = frame)"), p->buffern, 0, 8, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).buffern = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point"), p->pointCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DynamicShiftParams>(&params))
    {
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<DynamicShiftParams>(n.params).blend = v; });
        addBool(tr("Bilinear"), p->bilinear, [](ChainNode& n, bool v) { std::get<DynamicShiftParams>(n.params).bilinear = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame (x,y shift)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DynamicDistanceModifierParams>(&params))
    {
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<DynamicDistanceModifierParams>(n.params).blend = v; });
        addBool(tr("Bilinear"), p->bilinear, [](ChainNode& n, bool v) { std::get<DynamicDistanceModifierParams>(n.params).bilinear = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Pixel (d in/out)"), p->pixelCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).pixelCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MovingParticleParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<MovingParticleParams>(n.params).color = v; });
        addInt(tr("Max distance"), p->maxDistance, 1, 256, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).maxDistance = v; });
        addInt(tr("Size"), p->size, 1, 128, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).size = v; });
        addInt(tr("On-beat size"), p->size2, 1, 128, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).size2 = v; });
        addBool(tr("Resize on beat"), p->onBeatSize, [](ChainNode& n, bool v) { std::get<MovingParticleParams>(n.params).onBeatSize = v; });
        addRefDouble(tr("Spring"), p->spring, 0.0, 0.1, 0.001, 0.004, 4, [](ChainNode& n, double v) { std::get<MovingParticleParams>(n.params).spring = static_cast<float>(v); });
        addRefDouble(tr("Damping"), p->damping, 0.8, 1.0, 0.001, 0.991, 4, [](ChainNode& n, double v) { std::get<MovingParticleParams>(n.params).damping = static_cast<float>(v); });
        addReferenceNote(!p->initCode.empty() || !p->frameCode.empty() || !p->beatCode.empty());
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MovingParticleParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MovingParticleParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MovingParticleParams>(n.params).beatCode = std::move(v); });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50", "Line"}, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<ColorMapParams>(&params))
    {
        addEnum(tr("Input"), p->key, {"Red", "Green", "Blue", "(R+G+B)/2", "Max", "(R+G+B)/3"}, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).key = v; });
        addEnum(tr("Blend"), p->blendMode, {"Replace", "Additive", "Maximum", "Minimum", "50/50", "Sub d-s", "Sub s-d", "Multiply", "XOR", "Adjustable"}, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).blendMode = v; });
        addInt(tr("Adjustable"), p->adjustBlend, 0, 255, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).adjustBlend = v; });
        addGradientStops(form, path, p->stopPos, p->stopColor);
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorMapParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorMapParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorMapParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BufferBlendParams>(&params))
    {
        const QStringList bufs{"Buffer 1", "Buffer 2", "Buffer 3", "Buffer 4", "Buffer 5", "Buffer 6", "Buffer 7", "Buffer 8", "Current"};
        addEnum(tr("Buffer A"), p->bufferA, bufs, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).bufferA = v; });
        addEnum(tr("Buffer B"), p->bufferB, bufs, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).bufferB = v; });
        addEnum(tr("Mode"), p->mode, {"Replace", "Additive", "Maximum", "50/50", "Sub A-B", "Sub B-A", "Multiply", "Adjustable", "XOR", "Minimum", "Abs diff"}, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).mode = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BufferBlendParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BufferBlendParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BufferBlendParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<JherikoGlobalParams>(&params))
    {
        form->addRow(new QLabel(tr("The variable-set module: declare preset-global "
                                   "variables (reg00…reg99, gmegabuf) in Init; other "
                                   "effects can read them. Re-declaring a global that "
                                   "another node's Init already sets is flagged red."),
                                m_propContainer));
        addEnum(tr("Load"), p->loadMode, {"None", "Once", "Code control", "Every frame"}, [](ChainNode& n, int v) { std::get<JherikoGlobalParams>(n.params).loadMode = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<JherikoGlobalParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<JherikoGlobalParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<JherikoGlobalParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ColorClipParams>(&params))
    {
        addEnum(tr("Mode"), p->mode - 1, {"Below", "Above", "Near"}, [](ChainNode& n, int v) { std::get<ColorClipParams>(n.params).mode = v + 1; });
        addColor(tr("Threshold"), p->clipColor, [](ChainNode& n, uint32_t v) { std::get<ColorClipParams>(n.params).clipColor = v; });
        addColor(tr("Replace with"), p->outColor, [](ChainNode& n, uint32_t v) { std::get<ColorClipParams>(n.params).outColor = v; });
        addInt(tr("Distance"), p->distance, 0, 255, [](ChainNode& n, int v) { std::get<ColorClipParams>(n.params).distance = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorClipParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorClipParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorClipParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<UniqueToneParams>(&params))
    {
        addColor(tr("Tone"), p->color, [](ChainNode& n, uint32_t v) { std::get<UniqueToneParams>(n.params).color = v; });
        addBool(tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<UniqueToneParams>(n.params).invert = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<UniqueToneParams>(n.params).blend = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<UniqueToneParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<UniqueToneParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<UniqueToneParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<InterleaveParams>(&params))
    {
        addInt(tr("X spacing"), p->x, 0, 512, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).x = v; });
        addInt(tr("Y spacing"), p->y, 0, 512, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).y = v; });
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<InterleaveParams>(n.params).color = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).blend = v; });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<InterleaveParams>(n.params).onBeat = v; });
        addInt(tr("Beat X"), p->x2, 0, 512, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).x2 = v; });
        addInt(tr("Beat Y"), p->y2, 0, 512, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).y2 = v; });
        addInt(tr("Beat duration"), p->beatDuration, 1, 100, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).beatDuration = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<InterleaveParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<InterleaveParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<InterleaveParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ConvolutionParams>(&params))
    {
        addEnum(tr("Edge"), p->edgeMode, {"Extend", "Wrap"}, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).edgeMode = v; });
        addBool(tr("Absolute"), p->absolute, [](ChainNode& n, bool v) { std::get<ConvolutionParams>(n.params).absolute = v; });
        addBool(tr("Two pass"), p->twoPass, [](ChainNode& n, bool v) { std::get<ConvolutionParams>(n.params).twoPass = v; });
        addInt(tr("Bias"), p->bias, -255, 255, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).bias = v; });
        addInt(tr("Scale"), p->scale, 1, 1000, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).scale = v; });
        addKernelGrid(form, path, p->kernel);
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ConvolutionParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ConvolutionParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ConvolutionParams>(n.params).beatCode = std::move(v); });
    }
    else if (std::holds_alternative<NormaliseParams>(params))
    {
        auto* info = new QLabel(tr("Auto-levels — no parameters"), m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<MultiFilterParams>(&params))
    {
        addEnum(tr("Effect"), p->effect, {"Chrome", "Double chrome", "Triple chrome", "Infinite root"}, [](ChainNode& n, int v) { std::get<MultiFilterParams>(n.params).effect = v; });
        addBool(tr("On beat only"), p->onBeat, [](ChainNode& n, bool v) { std::get<MultiFilterParams>(n.params).onBeat = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MultiFilterParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MultiFilterParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MultiFilterParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<AddBordersParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<AddBordersParams>(n.params).color = v; });
        addInt(tr("Size"), p->size, 0, 200, [](ChainNode& n, int v) { std::get<AddBordersParams>(n.params).size = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<AddBordersParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<AddBordersParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<AddBordersParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<SimpleScopeParams>(&params))
    {
        addEnum(tr("Mode"), p->mode,
                {"Solid analyzer", "Line analyzer", "Line scope", "Solid scope",
                 "Dot analyzer", "Dot scope"},
                [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).mode = v; });
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).channel = v; });
        addEnum(tr("Position"), p->position, {"Top", "Bottom", "Center"}, [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).position = v; });
        addColorTable(
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<SimpleScopeParams>(n.params).colors;
            },
            tr("AVS color table — the scope cycles through these over time."));
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<SimpleScopeParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SimpleScopeParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SimpleScopeParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BassSpinParams>(&params))
    {
        addBool(tr("Left"), p->left, [](ChainNode& n, bool v) { std::get<BassSpinParams>(n.params).left = v; });
        addBool(tr("Right"), p->right, [](ChainNode& n, bool v) { std::get<BassSpinParams>(n.params).right = v; });
        addColor(tr("Left color"), p->colorLeft, [](ChainNode& n, uint32_t v) { std::get<BassSpinParams>(n.params).colorLeft = v; });
        addColor(tr("Right color"), p->colorRight, [](ChainNode& n, uint32_t v) { std::get<BassSpinParams>(n.params).colorRight = v; });
        addEnum(tr("Mode"), p->mode, {"Lines", "Filled"}, [](ChainNode& n, int v) { std::get<BassSpinParams>(n.params).mode = v; });
        addRefDouble(tr("Smoothing"), p->smoothing, 0.0, 1.0, 0.05, 0.7, 3, [](ChainNode& n, double v) { std::get<BassSpinParams>(n.params).smoothing = static_cast<float>(v); });
        addRefDouble(tr("Spin step"), p->spinStep, 0.0, 3.14, 0.01, static_cast<double>(3.14159f / 6.0f), 4, [](ChainNode& n, double v) { std::get<BassSpinParams>(n.params).spinStep = static_cast<float>(v); });
        addReferenceNote(!p->initCode.empty() || !p->frameCode.empty() || !p->beatCode.empty());
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BassSpinParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BassSpinParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BassSpinParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<OscStarParams>(&params))
    {
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).channel = v; });
        addEnum(tr("Position"), p->position, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).position = v; });
        addInt(tr("Size"), p->size, 0, 16, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).size = v; });
        addInt(tr("Rotation"), p->rot, 0, 16, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).rot = v; });
        addInt(tr("Spokes"), p->spokes, 1, 64, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).spokes = v; });
        addDouble(tr("Rotation scale"), p->rotScale, 0.0, 1.0, 0.005, [](ChainNode& n, double v) { std::get<OscStarParams>(n.params).rotScale = static_cast<float>(v); });
        addDouble(tr("Amplitude"), p->amplitude, 0.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<OscStarParams>(n.params).amplitude = static_cast<float>(v); });
        addColorTable(
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<OscStarParams>(n.params).colors;
            },
            tr("AVS color table — cycled over time."));
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<OscStarParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<OscStarParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<OscStarParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<OscRingParams>(&params))
    {
        addEnum(tr("Source"), p->source, {"Waveform", "Spectrum"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).source = v; });
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).channel = v; });
        addEnum(tr("Position"), p->position, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).position = v; });
        addInt(tr("Size"), p->size, 0, 16, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).size = v; });
        addInt(tr("Segments"), p->segments, 3, 1024, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).segments = v; });
        addDouble(tr("Base radius"), p->baseScale, 0.0, 2.0, 0.05, [](ChainNode& n, double v) { std::get<OscRingParams>(n.params).baseScale = static_cast<float>(v); });
        addDouble(tr("Audio radius"), p->audioScale, 0.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<OscRingParams>(n.params).audioScale = static_cast<float>(v); });
        addColorTable(
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<OscRingParams>(n.params).colors;
            },
            tr("AVS color table — cycled over time."));
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<OscRingParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<OscRingParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<OscRingParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<RotatingStarsParams>(&params))
    {
        addInt(tr("Points"), p->points, 2, 64, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).points = v; });
        addInt(tr("Skip (2 = pentagram)"), p->skip, 1, 32, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).skip = v; });
        addInt(tr("Stars"), p->stars, 1, 16, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).stars = v; });
        addDouble(tr("Rotation speed"), p->rotSpeed, -1.0, 1.0, 0.01, [](ChainNode& n, double v) { std::get<RotatingStarsParams>(n.params).rotSpeed = static_cast<float>(v); });
        addDouble(tr("Orbit radius"), p->orbit, 0.0, 2.0, 0.05, [](ChainNode& n, double v) { std::get<RotatingStarsParams>(n.params).orbit = static_cast<float>(v); });
        addDouble(tr("Base size"), p->baseRadius, 0.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<RotatingStarsParams>(n.params).baseRadius = static_cast<float>(v); });
        addDouble(tr("Audio gain"), p->audioGain, 0.0, 8.0, 0.05, [](ChainNode& n, double v) { std::get<RotatingStarsParams>(n.params).audioGain = static_cast<float>(v); });
        addInt(tr("Band from"), p->bandLo, 0, 575, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).bandLo = v; });
        addInt(tr("Band to (exclusive)"), p->bandHi, 1, 576, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).bandHi = v; });
        addColorTable(
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<RotatingStarsParams>(n.params).colors;
            },
            tr("AVS color table — cycled over time."));
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<RotatingStarsParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<RotatingStarsParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<RotatingStarsParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<PictureParams>(&params))
    {
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<PictureParams>(n.params).blend = v; });
        addBool(tr("Keep aspect"), p->keepAspect, [](ChainNode& n, bool v) { std::get<PictureParams>(n.params).keepAspect = v; });
        addImageRow(form, path, p->filename, p->imageData,
                    [](ChainNode& n, std::string f, std::string d) {
                        auto& q = std::get<PictureParams>(n.params);
                        q.filename = std::move(f);
                        q.imageData = std::move(d);
                    });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<PictureParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<PictureParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<PictureParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<PictureIIParams>(&params))
    {
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<PictureIIParams>(n.params).blend = v; });
        addImageRow(form, path, p->filename, p->imageData,
                    [](ChainNode& n, std::string f, std::string d) {
                        auto& q = std::get<PictureIIParams>(n.params);
                        q.filename = std::move(f);
                        q.imageData = std::move(d);
                    });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<PictureIIParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<PictureIIParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<PictureIIParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<TexerParams>(&params))
    {
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive"}, [](ChainNode& n, int v) { std::get<TexerParams>(n.params).blend = v; });
        addInt(tr("Particles"), p->particles, 1, 4096, [](ChainNode& n, int v) { std::get<TexerParams>(n.params).particles = v; });
        addImageRow(form, path, p->filename, p->imageData,
                    [](ChainNode& n, std::string f, std::string d) {
                        auto& q = std::get<TexerParams>(n.params);
                        q.filename = std::move(f);
                        q.imageData = std::move(d);
                    });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TexerParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TexerParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<TexerParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<TexerIIParams>(&params))
    {
        addBool(tr("Color filtering"), p->colorFiltering, [](ChainNode& n, bool v) { std::get<TexerIIParams>(n.params).colorFiltering = v; });
        addBool(tr("Resizing"), p->resizing, [](ChainNode& n, bool v) { std::get<TexerIIParams>(n.params).resizing = v; });
        addBool(tr("Wrap around"), p->wrapAround, [](ChainNode& n, bool v) { std::get<TexerIIParams>(n.params).wrapAround = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point (x,y,sizex,sizey,rgb)"), p->pointCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).pointCode = std::move(v); });
        addImageRow(form, path, p->filename, p->imageData,
                    [](ChainNode& n, std::string f, std::string d) {
                        auto& q = std::get<TexerIIParams>(n.params);
                        q.filename = std::move(f);
                        q.imageData = std::move(d);
                    });
    }
    else if (auto* p = std::get_if<TriangleParams>(&params))
    {
        // Gefuellt ist der Referenzzustand (S51) — Drahtgitter ist eine bewusste
        // Abweichung von AVS und deshalb NICHT die Vorgabe.
        addBool(tr("Filled (AVS)"), p->filled, [](ChainNode& n, bool v) { std::get<TriangleParams>(n.params).filled = v; });
        addDouble(tr("Edge width (wireframe)"), p->lineWidth, 1.0, 20.0, 0.5, [](ChainNode& n, double v) { std::get<TriangleParams>(n.params).lineWidth = static_cast<float>(v); });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame (set n)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point (x1..y3,rgb)"), p->pointCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BlitterFeedbackParams>(&params))
    {
        addInt(tr("Scale (32 = off, <32 in, >32 out)"), p->scale, 0, 256, [](ChainNode& n, int v) { std::get<BlitterFeedbackParams>(n.params).scale = v; });
        addInt(tr("Beat scale"), p->scale2, 0, 256, [](ChainNode& n, int v) { std::get<BlitterFeedbackParams>(n.params).scale2 = v; });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).onBeat = v; });
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).blend = v; });
        addBool(tr("Subpixel (bilinear)"), p->subpixel, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).subpixel = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BlitterFeedbackParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BlitterFeedbackParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BlitterFeedbackParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<RotoBlitterParams>(&params))
    {
        addInt(tr("Zoom scale (31 = off)"), p->zoomScale, 0, 256, [](ChainNode& n, int v) { std::get<RotoBlitterParams>(n.params).zoomScale = v; });
        addInt(tr("Beat zoom scale"), p->zoomScale2, 0, 256, [](ChainNode& n, int v) { std::get<RotoBlitterParams>(n.params).zoomScale2 = v; });
        addInt(tr("Rotation (32 = off, deg/frame +32)"), p->rotDir, 0, 64, [](ChainNode& n, int v) { std::get<RotoBlitterParams>(n.params).rotDir = v; });
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).blend = v; });
        addBool(tr("Beat reverse"), p->beatReverse, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).beatReverse = v; });
        addInt(tr("Reverse speed"), p->beatReverseSpeed, 0, 8, [](ChainNode& n, int v) { std::get<RotoBlitterParams>(n.params).beatReverseSpeed = v; });
        addBool(tr("Beat zoom jump"), p->beatZoomJump, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).beatZoomJump = v; });
        addBool(tr("Subpixel (bilinear)"), p->subpixel, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).subpixel = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<RotoBlitterParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<RotoBlitterParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<RotoBlitterParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BufferSaveParams>(&params))
    {
        addInt(tr("Buffer slot"), p->slot, 0, 7, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).slot = v; });
        const QStringList kDirNames = {tr("Save"), tr("Restore"),
                                       tr("Alternate (save first)"),
                                       tr("Alternate (restore first)")};
        addEnum(tr("Direction"), p->dir, kDirNames, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).dir = v; });
        addEnum(tr("Blend"), static_cast<int>(p->blend), kBlendNames, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).blend = static_cast<BlendMode>(v); });
        addInt(tr("Blend alpha"), p->adjustAlpha, 0, 255, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).adjustAlpha = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BufferSaveParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BufferSaveParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BufferSaveParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<CustomBpmParams>(&params))
    {
        addBool(tr("Arbitrary"), p->arbitrary, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).arbitrary = v; });
        addInt(tr("Interval (ms)"), p->arbitraryMs, 1, 5000, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).arbitraryMs = v; });
        addBool(tr("Skip"), p->skip, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).skip = v; });
        addInt(tr("Skip count"), p->skipCount, 1, 16, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).skipCount = v; });
        addBool(tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).invert = v; });
        // `skipfirst`: die ersten N Beats werden verschluckt, der Skip-Zaehler
        // laeuft dabei NICHT mit (r_bpm.cpp:148).
        addInt(tr("Skip first N beats"), p->skipFirst, 0, 64, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).skipFirst = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<CustomBpmParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<CustomBpmParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<CustomBpmParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<SuperScopeParams>(&params))
    {
        // Das frueher hier stehende „Figure"-Dropdown ist ENTFALLEN (S53): die
        // Figuren liegen jetzt als Dateien in `asset/nodepresets/superScope/`
        // und laufen ueber die allgemeine Zeile „Preset" ganz oben — eine
        // Voreinstellungs-Liste statt zweier nebeneinander (Vorgabe Patrik).
        addInt(tr("Point count"), p->pointCount, 1, 4096, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).pointCount = v; });
        addEnum(tr("Draw mode"), p->renderMode, {"Dots", "Lines", "Thick"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).renderMode = v; });
        addDouble(tr("Line width"), p->lineWidth, 1.0, 20.0, 0.5, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).lineWidth = static_cast<float>(v); });
        addDouble(tr("Dot size"), p->dotSize, 1.0, 50.0, 1.0, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum(tr("Channel"), p->audioChannel, {"Left", "Right", "Mono", "Mid", "Side"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).audioChannel = v; });
        addBool(tr("Spectrum source (v)"), p->spectrumSource,
                [](ChainNode& n, bool v) { std::get<SuperScopeParams>(n.params).spectrumSource = v; });
        addEnum(tr("Line blend"), p->lineBlend,
                {"Replace", "Additive", "Maximum", "50/50", "Sub (fb-c)",
                 "Sub (c-fb)", "Multiply", "Adjustable", "XOR (~Add)", "Minimum"},
                [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).lineBlend = v; });

        // --- Color: gradient (per point) x table (cycled) combined by mode ----
        // The base color pre-seeds red/green/blue before the point code, which
        // may keep, modulate (red=red*v) or override it (AVS r_sscope).
        {
            auto* modeCombo = new QComboBox(m_propContainer);
            modeCombo->addItems({tr("Gradient (per point)"), tr("Color table (cycled)"),
                                 tr("Additive (both)"), tr("Multiply (both)"),
                                 tr("Average (both)")});
            modeCombo->setCurrentIndex(std::clamp(p->colorBlend, 0, 4));
            modeCombo->setToolTip(
                tr("Base color for red/green/blue before the point code runs; the "
                   "code may keep, modulate or override it."));
            connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                    [this, path](int v) {
                        mutate(path, [&](ChainNode& n) {
                            std::get<SuperScopeParams>(n.params).colorBlend = v;
                        });
                    });
            form->addRow(tr("Color mode"), modeCombo);
        }
        {
            const std::vector<std::string> gradNames = m_gradientPreview.presetNames();
            auto* gradCombo = new QComboBox(m_propContainer);
            gradCombo->setMinimumHeight(28);  // room for the preview strip
            int curGrad = 0;
            for (int gi = 0; gi < static_cast<int>(gradNames.size()); ++gi)
            {
                gradCombo->addItem(QString::fromStdString(gradNames[static_cast<size_t>(gi)]));
                if (gradNames[static_cast<size_t>(gi)] == p->gradientPreset) curGrad = gi;
            }
            // Preview delegate: draws each preset's gradient strip beside its name.
            auto* delegate = new lumi::ui::GradientPresetDelegate(gradCombo);
            delegate->setGradientModule(&m_gradientPreview);
            gradCombo->setItemDelegate(delegate);
            gradCombo->setCurrentIndex(curGrad);
            connect(gradCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                    [this, path, gradNames](int gi) {
                        if (gi < 0 || gi >= static_cast<int>(gradNames.size())) return;
                        const std::string name = gradNames[static_cast<size_t>(gi)];
                        mutate(path, [&](ChainNode& n) {
                            std::get<SuperScopeParams>(n.params).gradientPreset = name;
                        });
                    });
            form->addRow(tr("Gradient"), gradCombo);
        }
        addInt(tr("Cycle frames"), p->colorCycleFrames, 1, 600, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).colorCycleFrames = v; });
        // AVS color table: cycled over time in the "Color table"/blend modes.
        addColorTable(
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<SuperScopeParams>(n.params).colors;
            },
            tr("AVS color table — the scope cycles through these over time in "
               "the Color table / blend modes (one step every 'Cycle frames')."));

        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point"), p->pointCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MosaicParams>(&params))
    {
        addInt(tr("Quality"), p->quality, 1, 100, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).quality = v; });
        addInt(tr("OnBeat quality"), p->quality2, 1, 100, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).quality2 = v; });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<MosaicParams>(n.params).onBeat = v; });
        addInt(tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).durationFrames = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).blend = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MosaicParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MosaicParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MosaicParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<GrainParams>(&params))
    {
        addInt(tr("Amount"), p->amount, 0, 100, [](ChainNode& n, int v) { std::get<GrainParams>(n.params).amount = v; });
        addBool(tr("Static"), p->staticGrain, [](ChainNode& n, bool v) { std::get<GrainParams>(n.params).staticGrain = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<GrainParams>(n.params).blend = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<GrainParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<GrainParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<GrainParams>(n.params).beatCode = std::move(v); });
    }
    else if (std::get_if<ScatterParams>(&params) != nullptr)
    {
        auto* info = new QLabel(tr("Per-pixel random displacement (no parameters)."), m_propPage);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (std::get_if<WaterParams>(&params) != nullptr)
    {
        auto* info = new QLabel(tr("Water ripple — neighbour average minus the previous frame (no parameters)."), m_propPage);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<BumpParams>(&params))
    {
        addInt(tr("Depth"), p->depth, 0, 100, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).depth = v; });
        addBool(tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).invert = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).blend = v; });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).onBeat = v; });
        addInt(tr("Beat depth"), p->depth2, 0, 100, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).depth2 = v; });
        addInt(tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).durationFrames = v; });
        addBool(tr("Old-style x,y (0..100)"), p->oldStyle, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).oldStyle = v; });
        addInt(tr("Depth buffer (0 = frame)"), p->buffern, 0, 8, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).buffern = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame (light x,y)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<WaterBumpParams>(&params))
    {
        addInt(tr("Density (damping)"), p->density, 1, 12, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).density = v; });
        addInt(tr("Drop depth"), p->depth, 0, 2000, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).depth = v; });
        addDouble(tr("Refraction"), p->displaceScale, 0.0, 40.0, 0.5, [](ChainNode& n, double v) { std::get<WaterBumpParams>(n.params).displaceScale = static_cast<float>(v); });
        addBool(tr("Random drop"), p->randomDrop, [](ChainNode& n, bool v) { std::get<WaterBumpParams>(n.params).randomDrop = v; });
        addEnum(tr("Drop X"), p->dropX, {"Near", "Mid", "Far"}, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropX = v; });
        addEnum(tr("Drop Y"), p->dropY, {"Near", "Mid", "Far"}, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropY = v; });
        addInt(tr("Drop radius"), p->dropRadius, 1, 200, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropRadius = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<WaterBumpParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<WaterBumpParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<WaterBumpParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<InterferencesParams>(&params))
    {
        addInt(tr("Points"), p->points, 1, 8, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).points = v; });
        addInt(tr("Distance"), p->distance, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).distance = v; });
        addInt(tr("Alpha"), p->alpha, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).alpha = v; });
        addInt(tr("Rotation"), p->rotation, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotation = v; });
        addInt(tr("Rotation/frame"), p->rotationInc, -64, 64, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotationInc = v; });
        addBool(tr("RGB split"), p->rgb, [](ChainNode& n, bool v) { std::get<InterferencesParams>(n.params).rgb = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).blend = v; });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<InterferencesParams>(n.params).onBeat = v; });
        addInt(tr("Beat distance"), p->distance2, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).distance2 = v; });
        addInt(tr("Beat alpha"), p->alpha2, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).alpha2 = v; });
        addInt(tr("Beat rotation/frame"), p->rotationInc2, -64, 64, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotationInc2 = v; });
        addDouble(tr("Beat speed"), p->speed, 0.01, 2.0, 0.01, [](ChainNode& n, double v) { std::get<InterferencesParams>(n.params).speed = static_cast<float>(v); });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<InterferencesParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<InterferencesParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<InterferencesParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FyrewurXParams>(&params))
    {
        addInt(tr("Sparks per burst"), p->sparks, 1, 1024, [](ChainNode& n, int v) { std::get<FyrewurXParams>(n.params).sparks = v; });
        addDouble(tr("Speed"), p->speed, 0.05, 5.0, 0.05, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).speed = static_cast<float>(v); });
        addDouble(tr("Gravity"), p->gravity, 0.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).gravity = static_cast<float>(v); });
        addDouble(tr("Life (s)"), p->lifeSeconds, 0.1, 10.0, 0.1, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).lifeSeconds = static_cast<float>(v); });
        addDouble(tr("Spark size"), p->dotSize, 1.0, 32.0, 0.5, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).dotSize = static_cast<float>(v); });
        addDouble(tr("Hue drift"), p->hueDrift, 0.0, 6.0, 0.1, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).hueDrift = static_cast<float>(v); });
        addDouble(tr("Burst spread"), p->burstSpread, 0.0, 3.0, 0.05, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).burstSpread = static_cast<float>(v); });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<FyrewurXParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FyrewurXParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FyrewurXParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Metaballs3DParams>(&params))
    {
        // Verhaltens-Nachbau (S52): aus dem Preset kommt nur die Farbtafel, die
        // Geometrie ist host-eigen — also sind ALLE Zahlen hier frei verstellbar.
        // Grenzen wie im Renderer/Serialisierer (count 1..16).
        addInt(tr("Balls"), p->count, 1, 16, [](ChainNode& n, int v) { std::get<Metaballs3DParams>(n.params).count = v; });
        addDouble(tr("Radius"), p->radius, 0.01, 1.0, 0.01, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).radius = static_cast<float>(v); });
        addDouble(tr("Speed"), p->speed, 0.0, 5.0, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).speed = static_cast<float>(v); });
        addDouble(tr("Threshold"), p->threshold, 0.05, 8.0, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).threshold = static_cast<float>(v); });
        addEnum(tr("Blend"), p->blend, {tr("Replace"), tr("Additive"), tr("50/50")}, [](ChainNode& n, int v) { std::get<Metaballs3DParams>(n.params).blend = v; });
        addDouble(tr("Orbit spread"), p->spread, 0.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).spread = static_cast<float>(v); });
        addDouble(tr("Depth"), p->depth, 0.2, 6.0, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).depth = static_cast<float>(v); });
        addDouble(tr("Phase per ball"), p->phase, 0.0, 6.3, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).phase = static_cast<float>(v); });
        addColorTable(
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<Metaballs3DParams>(n.params).colors;
            },
            tr("Palette from the preset — ball i takes entry i (white if empty)."));
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<Metaballs3DParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Metaballs3DParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Metaballs3DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Tentacles3DParams>(&params))
    {
        addInt(tr("Tentacles"), p->count, 1, 16, [](ChainNode& n, int v) { std::get<Tentacles3DParams>(n.params).count = v; });
        addInt(tr("Segments"), p->segments, 2, 256, [](ChainNode& n, int v) { std::get<Tentacles3DParams>(n.params).segments = v; });
        addDouble(tr("Length"), p->length, 0.05, 2.0, 0.05, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).length = static_cast<float>(v); });
        addDouble(tr("Thickness (px)"), p->thickness, 1.0, 64.0, 0.5, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).thickness = static_cast<float>(v); });
        addDouble(tr("Speed"), p->speed, 0.0, 5.0, 0.05, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).speed = static_cast<float>(v); });
        addEnum(tr("Blend"), p->blend, {tr("Replace"), tr("Additive"), tr("50/50")}, [](ChainNode& n, int v) { std::get<Tentacles3DParams>(n.params).blend = v; });
        addDouble(tr("Sway"), p->sway, 0.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).sway = static_cast<float>(v); });
        addDouble(tr("Waves"), p->waves, 0.0, 20.0, 0.1, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).waves = static_cast<float>(v); });
        addDouble(tr("Taper (0 = even)"), p->taper, 0.0, 1.0, 0.05, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).taper = static_cast<float>(v); });
        addColorTable(
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<Tentacles3DParams>(n.params).colors;
            },
            tr("Palette from the preset — tentacle i takes entry i (white if empty)."));
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<Tentacles3DParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Tentacles3DParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Tentacles3DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<StarfieldParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<StarfieldParams>(n.params).color = v; });
        addInt(tr("Stars"), p->maxStars, 1, 8192, [](ChainNode& n, int v) { std::get<StarfieldParams>(n.params).maxStars = v; });
        addDouble(tr("Warp speed"), p->warpSpeed, 0.1, 50.0, 0.5, [](ChainNode& n, double v) { std::get<StarfieldParams>(n.params).warpSpeed = static_cast<float>(v); });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<StarfieldParams>(n.params).onBeat = v; });
        addDouble(tr("Beat speed"), p->beatSpeed, 0.1, 50.0, 0.5, [](ChainNode& n, double v) { std::get<StarfieldParams>(n.params).beatSpeed = static_cast<float>(v); });
        addInt(tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<StarfieldParams>(n.params).durationFrames = v; });
        const QStringList kStarBlendNames = {tr("Replace"), tr("Additive"), tr("50/50")};
        addEnum(tr("Blend"), p->blend, kStarBlendNames, [](ChainNode& n, int v) { std::get<StarfieldParams>(n.params).blend = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<StarfieldParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<StarfieldParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<StarfieldParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<TimescopeParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<TimescopeParams>(n.params).color = v; });
        addInt(tr("Bands"), p->bands, 1, 576, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).bands = v; });
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).channel = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50", "Line (SRM)"}, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).blend = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TimescopeParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TimescopeParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<TimescopeParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DotGridParams>(&params))
    {
        addInt(tr("Spacing"), p->spacing, 2, 128, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).spacing = v; });
        addInt(tr("X move"), p->xMove, -1024, 1024, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).xMove = v; });
        addInt(tr("Y move"), p->yMove, -1024, 1024, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).yMove = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50", "Line (SRM)"}, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).blend = v; });
        addColorTable(
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<DotGridParams>(n.params).colors;
            },
            tr("AVS color table — the dots cycle through these over time."));
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DotGridParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DotGridParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DotGridParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DotPlaneParams>(&params))
    {
        addInt(tr("Rotation speed"), p->rotVel, -50, 50, [](ChainNode& n, int v) { std::get<DotPlaneParams>(n.params).rotVel = v; });
        addInt(tr("Angle"), p->angle, -90, 90, [](ChainNode& n, int v) { std::get<DotPlaneParams>(n.params).angle = v; });
        addRefDouble(tr("Camera distance"), p->camDistance, 50.0, 2000.0, 10.0, 400.0, 1, [](ChainNode& n, double v) { std::get<DotPlaneParams>(n.params).camDistance = static_cast<float>(v); });
        addRefDouble(tr("Settle"), p->settle, 0.0, 2.0, 0.01, 0.15, 3, [](ChainNode& n, double v) { std::get<DotPlaneParams>(n.params).settle = static_cast<float>(v); });
        for (int i = 0; i < 5; ++i)
            addColor(tr("Color %1").arg(i + 1), p->colors[i], [i](ChainNode& n, uint32_t v) { std::get<DotPlaneParams>(n.params).colors[i] = v; });
        addReferenceNote(!p->initCode.empty() || !p->frameCode.empty() || !p->beatCode.empty());
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DotPlaneParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DotPlaneParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DotPlaneParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DotFountainParams>(&params))
    {
        addInt(tr("Rotation speed"), p->rotVel, -50, 50, [](ChainNode& n, int v) { std::get<DotFountainParams>(n.params).rotVel = v; });
        addInt(tr("Angle"), p->angle, -90, 90, [](ChainNode& n, int v) { std::get<DotFountainParams>(n.params).angle = v; });
        for (int i = 0; i < 5; ++i)
            addColor(tr("Color %1").arg(i + 1), p->colors[i], [i](ChainNode& n, uint32_t v) { std::get<DotFountainParams>(n.params).colors[i] = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DotFountainParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DotFountainParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DotFountainParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ChannelShiftParams>(&params))
    {
        addEnum(tr("Mode"), p->mode, {"RGB", "RBG", "GBR", "GRB", "BRG", "BGR"}, [](ChainNode& n, int v) { std::get<ChannelShiftParams>(n.params).mode = v; });
        addBool(tr("On beat (random)"), p->onBeat, [](ChainNode& n, bool v) { std::get<ChannelShiftParams>(n.params).onBeat = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ChannelShiftParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ChannelShiftParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ChannelShiftParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ColorReductionParams>(&params))
    {
        addInt(tr("Levels (bits)"), p->levels, 1, 8, [](ChainNode& n, int v) { std::get<ColorReductionParams>(n.params).levels = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorReductionParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorReductionParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorReductionParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MultiplierParams>(&params))
    {
        addEnum(tr("Factor"), p->mode, {"Saturate", "x8", "x4", "x2", "x0.5", "x0.25", "x0.125", "Keep"}, [](ChainNode& n, int v) { std::get<MultiplierParams>(n.params).mode = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MultiplierParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MultiplierParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MultiplierParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<VideoDelayParams>(&params))
    {
        addInt(tr("Delay"), p->delay, 1, 128, [](ChainNode& n, int v) { std::get<VideoDelayParams>(n.params).delay = v; });
        addBool(tr("In beats"), p->useBeats, [](ChainNode& n, bool v) { std::get<VideoDelayParams>(n.params).useBeats = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<VideoDelayParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<VideoDelayParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<VideoDelayParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MultiDelayParams>(&params))
    {
        addEnum(tr("Mode"), p->mode, {"None", "Input (write)", "Output (read)"}, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).mode = v; });
        addInt(tr("Buffer"), p->buffer, 0, 5, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).buffer = v; });
        addInt(tr("Delay"), p->delay, 1, 128, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).delay = v; });
        addBool(tr("In beats"), p->useBeats, [](ChainNode& n, bool v) { std::get<MultiDelayParams>(n.params).useBeats = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MultiDelayParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MultiDelayParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MultiDelayParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Fractal2DParams>(&params))
    {
        addEnum(tr("Type"), p->type,
                {"Mandelbrot", "Julia", "Burning Ship", "Tricorn", "Multibrot",
                 "Newton", "Phoenix", "Magnet", "Nova"},
                [](ChainNode& n, int v) { std::get<Fractal2DParams>(n.params).type = v; });
        addDouble(tr("Center X"), p->centerX, -4.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).centerX = static_cast<float>(v); });
        addDouble(tr("Center Y"), p->centerY, -4.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).centerY = static_cast<float>(v); });
        addDouble(tr("Zoom"), p->zoom, 0.001, 100000.0, 0.1, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).zoom = static_cast<float>(v); });
        addDouble(tr("Rotation"), p->rotation, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).rotation = static_cast<float>(v); });
        addInt(tr("Max iterations"), p->maxIter, 1, 2048, [](ChainNode& n, int v) { std::get<Fractal2DParams>(n.params).maxIter = v; });
        addDouble(tr("Julia X"), p->juliaX, -2.0, 2.0, 0.001, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).juliaX = static_cast<float>(v); });
        addDouble(tr("Julia Y"), p->juliaY, -2.0, 2.0, 0.001, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).juliaY = static_cast<float>(v); });
        addDouble(tr("Power"), p->power, 1.0, 16.0, 0.1, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).power = static_cast<float>(v); });
        addDouble(tr("Escape R^2"), p->escapeR, 1.0, 1024.0, 1.0, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).escapeR = static_cast<float>(v); });
        addBool(tr("Smooth colour"), p->smooth, [](ChainNode& n, bool v) { std::get<Fractal2DParams>(n.params).smooth = v; });
        addDouble(tr("Colour scale"), p->colorScale, 0.001, 4.0, 0.005, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble(tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).colorCycle = static_cast<float>(v); });
        addColor(tr("Inside colour"), p->insideColor, [](ChainNode& n, uint32_t v) { std::get<Fractal2DParams>(n.params).insideColor = v; });
        {
            auto* combo = new QComboBox(m_propContainer);
            lumi::modules::ColorGradientModule grad;
            for (const std::string& name : grad.presetNames())
                combo->addItem(QString::fromStdString(name));
            combo->setCurrentText(QString::fromStdString(p->gradientPreset));
            connect(combo, &QComboBox::currentTextChanged, this,
                    [this, path](const QString& t) {
                        const std::string name = t.toStdString();
                        mutate(path, [&](ChainNode& n) {
                            std::get<Fractal2DParams>(n.params).gradientPreset = name;
                        });
                    });
            form->addRow(tr("Palette"), combo);
        }
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<Fractal2DParams>(n.params).blend = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<Fractal2DParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Fractal2DParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Fractal2DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DomainWarpParams>(&params))
    {
        addInt(tr("Octaves"), p->octaves, 1, 10, [](ChainNode& n, int v) { std::get<DomainWarpParams>(n.params).octaves = v; });
        addDouble(tr("Lacunarity"), p->lacunarity, 1.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).lacunarity = static_cast<float>(v); });
        addDouble(tr("Gain"), p->gain, 0.0, 1.0, 0.02, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).gain = static_cast<float>(v); });
        addDouble(tr("Scale"), p->scale, 0.1, 32.0, 0.1, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).scale = static_cast<float>(v); });
        addDouble(tr("Warp"), p->warp, 0.0, 8.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).warp = static_cast<float>(v); });
        addDouble(tr("Warp scale"), p->warpScale, 0.1, 8.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).warpScale = static_cast<float>(v); });
        addDouble(tr("Speed"), p->speed, -4.0, 4.0, 0.02, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).speed = static_cast<float>(v); });
        addDouble(tr("Offset X"), p->offsetX, -32.0, 32.0, 0.1, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).offsetX = static_cast<float>(v); });
        addDouble(tr("Offset Y"), p->offsetY, -32.0, 32.0, 0.1, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).offsetY = static_cast<float>(v); });
        addDouble(tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble(tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).colorCycle = static_cast<float>(v); });
        {
            auto* combo = new QComboBox(m_propContainer);
            lumi::modules::ColorGradientModule grad;
            for (const std::string& name : grad.presetNames())
                combo->addItem(QString::fromStdString(name));
            combo->setCurrentText(QString::fromStdString(p->gradientPreset));
            connect(combo, &QComboBox::currentTextChanged, this,
                    [this, path](const QString& t) {
                        const std::string name = t.toStdString();
                        mutate(path, [&](ChainNode& n) {
                            std::get<DomainWarpParams>(n.params).gradientPreset = name;
                        });
                    });
            form->addRow(tr("Palette"), combo);
        }
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<DomainWarpParams>(n.params).blend = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<DomainWarpParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DomainWarpParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DomainWarpParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<SetRenderModeParams>(&params))
    {
        form->addRow(new QLabel(tr("Sets line width + blend for the render effects that follow."), m_propContainer));
        addBool(tr("Override blend"), p->enabled, [](ChainNode& n, bool v) { std::get<SetRenderModeParams>(n.params).enabled = v; });
        addInt(tr("Line width"), p->lineWidth, 0, 255, [](ChainNode& n, int v) { std::get<SetRenderModeParams>(n.params).lineWidth = v; });
        addEnum(tr("Line blend"), p->lineBlend,
                {"Replace", "Additive", "Maximum", "50/50", "Sub (fb-c)",
                 "Sub (c-fb)", "Multiply", "Adjustable", "XOR (~Add)", "Minimum"},
                [](ChainNode& n, int v) { std::get<SetRenderModeParams>(n.params).lineBlend = v; });
        addInt(tr("Adjustable alpha"), p->adjustAlpha, 0, 255, [](ChainNode& n, int v) { std::get<SetRenderModeParams>(n.params).adjustAlpha = v; });
    }
    else if (auto* p = std::get_if<RenderScaleParams>(&params))
    {
        form->addRow(new QLabel(tr("Renders the whole chain at window/divisor and "
                                   "upscales — the classic Winamp pixel-doubling "
                                   "look (AVS effects use fixed pixel sizes)."),
                                m_propContainer));
        addInt(tr("Divisor (window / N)"), p->divisor, 1, 8, [](ChainNode& n, int v) { std::get<RenderScaleParams>(n.params).divisor = v; });
        addEnum(tr("Upscale filter"), p->filter, {"Nearest (chunky)", "Linear (smooth)"},
                [](ChainNode& n, int v) { std::get<RenderScaleParams>(n.params).filter = v; });
    }
    else if (auto* p = std::get_if<BloomParams>(&params))
    {
        form->addRow(new QLabel(tr("Additive glow: downsample, separable 25-tap "
                                   "gaussian, add back (the \"Lights\" bloom — "
                                   "threshold 0 = reference). Display-only by "
                                   "default: the glow is mixed in at present "
                                   "time and never feeds back into the chain "
                                   "(uncheck for glow trails — accumulates in "
                                   "feedback chains!)."),
                                m_propContainer));
        addBool(tr("Display only (post)"), p->post, [](ChainNode& n, bool v) { std::get<BloomParams>(n.params).post = v; });
        addInt(tr("Downsample (1/2^n)"), p->downsample, 0, 4, [](ChainNode& n, int v) { std::get<BloomParams>(n.params).downsample = v; });
        addInt(tr("Radius (sigma px)"), p->radius, 1, 32, [](ChainNode& n, int v) { std::get<BloomParams>(n.params).radius = v; });
        addDouble(tr("Intensity"), p->intensity, 0.0, 8.0, 0.1, [](ChainNode& n, double v) { std::get<BloomParams>(n.params).intensity = static_cast<float>(v); });
        addDouble(tr("Threshold"), p->threshold, 0.0, 1.0, 0.05, [](ChainNode& n, double v) { std::get<BloomParams>(n.params).threshold = static_cast<float>(v); });
        addBool(tr("Vignette"), p->vignette, [](ChainNode& n, bool v) { std::get<BloomParams>(n.params).vignette = v; });
        addDouble(tr("Vignette strength"), p->vignetteStrength, 0.0, 1.0, 0.05, [](ChainNode& n, double v) { std::get<BloomParams>(n.params).vignetteStrength = static_cast<float>(v); });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BloomParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BloomParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BloomParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Camera3DParams>(&params))
    {
        form->addRow(new QLabel(tr("Sets the 3D camera for the following 3D "
                                   "modules (per-frame state, like Set Render "
                                   "Mode). Scripts may override px..pz, tx..tz, "
                                   "fov, roll, fogstart, fogend."),
                                m_propContainer));
        // Vorlagen-Dropdown (S48): laedt die EEL-Slots der Vorlage in die
        // Code-Felder unten (Quelle: camera3dPresets). Index 0 ist das Label.
        auto* presetCombo = new QComboBox(m_propContainer);
        presetCombo->addItem(tr("— Load template —"));
        for (const Camera3DPresetDef& def : camera3dPresets())
            presetCombo->addItem(def.name);
        connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this, path](int idx) {
                    if (idx <= 0) return;  // the label row
                    // Defer: applying rebuilds this editor (deletes the combo).
                    QMetaObject::invokeMethod(
                        this,
                        [this, path, idx] {
                            applyCamera3DPreset(path, idx - 1);
                            buildPropertyEditor(path);
                        },
                        Qt::QueuedConnection);
                });
        form->addRow(tr("Template"), presetCombo);

        addDouble(tr("Position X"), p->px, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).px = static_cast<float>(v); });
        addDouble(tr("Position Y"), p->py, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).py = static_cast<float>(v); });
        addDouble(tr("Position Z"), p->pz, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).pz = static_cast<float>(v); });
        addDouble(tr("Target X"), p->tx, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).tx = static_cast<float>(v); });
        addDouble(tr("Target Y"), p->ty, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).ty = static_cast<float>(v); });
        addDouble(tr("Target Z"), p->tz, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).tz = static_cast<float>(v); });
        addDouble(tr("FOV (deg)"), p->fov, 1.0, 179.0, 1.0, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).fov = static_cast<float>(v); });
        addDouble(tr("Roll (deg)"), p->roll, -360.0, 360.0, 1.0, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).roll = static_cast<float>(v); });
        addDouble(tr("Fog start"), p->fogStart, 0.0, 1000.0, 0.5, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).fogStart = static_cast<float>(v); });
        addDouble(tr("Fog end"), p->fogEnd, 0.0, 1000.0, 0.5, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).fogEnd = static_cast<float>(v); });
        addColor(tr("Fog color"), p->fogColor, [](ChainNode& n, uint32_t v) { std::get<Camera3DParams>(n.params).fogColor = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<Camera3DParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Camera3DParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Camera3DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<SuperScope3DParams>(&params))
    {
        form->addRow(new QLabel(tr("Scripted 3D point cloud: the point code "
                                   "writes x, y, z in WORLD units (y+ = up), "
                                   "plus size, red/green/blue, skip. Uses the "
                                   "active 3D camera (or the built-in default), "
                                   "drawn as additive soft sprites."),
                                m_propContainer));
        // Vorlagen-Dropdown (S48): laedt das EEL-Quartett der Vorlage in die
        // Code-Felder unten (Quelle: scope3dPresets). Index 0 ist das Label.
        auto* presetCombo = new QComboBox(m_propContainer);
        presetCombo->addItem(tr("— Load template —"));
        for (const Scope3DPresetDef& def : scope3dPresets())
            presetCombo->addItem(def.name);
        connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this, path](int idx) {
                    if (idx <= 0) return;  // the label row
                    // Defer: applying rebuilds this editor (deletes the combo).
                    QMetaObject::invokeMethod(
                        this,
                        [this, path, idx] {
                            applyScope3DPreset(path, idx - 1);
                            buildPropertyEditor(path);
                        },
                        Qt::QueuedConnection);
                });
        form->addRow(tr("Template"), presetCombo);

        addInt(tr("Points (n)"), p->pointCount, 1, 4096, [](ChainNode& n, int v) { std::get<SuperScope3DParams>(n.params).pointCount = v; });
        addEnum(tr("Draw mode"), p->renderMode, {"Soft sprites", "Lines"},
                [](ChainNode& n, int v) { std::get<SuperScope3DParams>(n.params).renderMode = v; });
        addDouble(tr("Default size (world)"), p->size, 0.0001, 100.0, 0.01, [](ChainNode& n, double v) { std::get<SuperScope3DParams>(n.params).size = static_cast<float>(v); });
        addDouble(tr("Sprite falloff (k)"), p->falloff, 0.5, 32.0, 0.5, [](ChainNode& n, double v) { std::get<SuperScope3DParams>(n.params).falloff = static_cast<float>(v); });
        addEnum(tr("Audio channel (v)"), p->audioChannel, {"Left", "Right", "Mono"},
                [](ChainNode& n, int v) { std::get<SuperScope3DParams>(n.params).audioChannel = v; });
        addBool(tr("v from spectrum"), p->spectrumSource, [](ChainNode& n, bool v) { std::get<SuperScope3DParams>(n.params).spectrumSource = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<SuperScope3DParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SuperScope3DParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SuperScope3DParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point code"), p->pointCode, [](ChainNode& n, std::string v) { std::get<SuperScope3DParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Terrain3DParams>(&params))
    {
        form->addRow(new QLabel(tr("Heightfield after the \"Lights\" recipe: "
                                   "procedural base, spectrum as radial rings, "
                                   "spring relaxation. Scripts see the height "
                                   "grid as megabuf(gy*res+gx); the point code "
                                   "colors grid dots (i, gx, gy, h given)."),
                                m_propContainer));
        addInt(tr("Resolution"), p->resolution, 8, 128, [](ChainNode& n, int v) { std::get<Terrain3DParams>(n.params).resolution = v; });
        addDouble(tr("Extent (world)"), p->extent, 0.1, 100.0, 0.5, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).extent = static_cast<float>(v); });
        addDouble(tr("Base amplitude"), p->baseAmp, 0.0, 10.0, 0.05, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).baseAmp = static_cast<float>(v); });
        addDouble(tr("Y offset"), p->yOffset, -100.0, 100.0, 0.1, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).yOffset = static_cast<float>(v); });
        addDouble(tr("Spectrum rings"), p->ringAmp, 0.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).ringAmp = static_cast<float>(v); });
        addDouble(tr("Relax (spring)"), p->relax, 0.0, 1.0, 0.02, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).relax = static_cast<float>(v); });
        addDouble(tr("Flatten"), p->flatten, 0.0, 1.0, 0.05, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).flatten = static_cast<float>(v); });
        addBool(tr("Draw mesh"), p->drawMesh, [](ChainNode& n, bool v) { std::get<Terrain3DParams>(n.params).drawMesh = v; });
        addColor(tr("Mesh color"), p->meshColor, [](ChainNode& n, uint32_t v) { std::get<Terrain3DParams>(n.params).meshColor = v; });
        addBool(tr("Draw dots"), p->drawDots, [](ChainNode& n, bool v) { std::get<Terrain3DParams>(n.params).drawDots = v; });
        addDouble(tr("Dot size (world)"), p->dotSize, 0.0001, 10.0, 0.005, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).dotSize = static_cast<float>(v); });
        addDouble(tr("Sprite falloff (k)"), p->falloff, 0.5, 32.0, 0.5, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).falloff = static_cast<float>(v); });
        addColor(tr("Color (valley)"), p->colorLow, [](ChainNode& n, uint32_t v) { std::get<Terrain3DParams>(n.params).colorLow = v; });
        addColor(tr("Color (peak)"), p->colorHigh, [](ChainNode& n, uint32_t v) { std::get<Terrain3DParams>(n.params).colorHigh = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<Terrain3DParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Terrain3DParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Terrain3DParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point code"), p->pointCode, [](ChainNode& n, std::string v) { std::get<Terrain3DParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<GlowOrbsParams>(&params))
    {
        form->addRow(new QLabel(tr("Low-poly glow orbs with two-color gradient, "
                                   "beat flash and halo (the \"Lights\" orbs). "
                                   "Point code runs PER ORB (i = orb index) and "
                                   "may set x,y,z, radius, sx,sy,sz, red/green/"
                                   "blue, red2/green2/blue2, flash."),
                                m_propContainer));
        addInt(tr("Orbs (n)"), p->orbCount, 1, 64, [](ChainNode& n, int v) { std::get<GlowOrbsParams>(n.params).orbCount = v; });
        addDouble(tr("Halo scale"), p->haloScale, 1.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<GlowOrbsParams>(n.params).haloScale = static_cast<float>(v); });
        addDouble(tr("Halo intensity"), p->haloIntensity, 0.0, 4.0, 0.1, [](ChainNode& n, double v) { std::get<GlowOrbsParams>(n.params).haloIntensity = static_cast<float>(v); });
        addDouble(tr("Halo falloff (k)"), p->falloff, 0.5, 32.0, 0.5, [](ChainNode& n, double v) { std::get<GlowOrbsParams>(n.params).falloff = static_cast<float>(v); });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<GlowOrbsParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<GlowOrbsParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<GlowOrbsParams>(n.params).beatCode = std::move(v); });
        addScript(tr("Point code"), p->pointCode, [](ChainNode& n, std::string v) { std::get<GlowOrbsParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Fractal3DParams>(&params))
    {
        addEnum(tr("Type"), p->type, {"Mandelbulb", "Mandelbox", "Menger", "Quaternion-Julia", "KIFS"}, [](ChainNode& n, int v) { std::get<Fractal3DParams>(n.params).type = v; });
        addDouble(tr("Yaw"), p->yaw, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).yaw = static_cast<float>(v); });
        addDouble(tr("Pitch"), p->pitch, -1.5, 1.5, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).pitch = static_cast<float>(v); });
        addDouble(tr("Distance"), p->dist, 0.5, 12.0, 0.1, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).dist = static_cast<float>(v); });
        addDouble(tr("FOV"), p->fov, 0.3, 3.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).fov = static_cast<float>(v); });
        addDouble(tr("Power"), p->power, 1.0, 16.0, 0.1, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).power = static_cast<float>(v); });
        addDouble(tr("Scale"), p->scale, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).scale = static_cast<float>(v); });
        addDouble(tr("Fold"), p->fold, 0.1, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).fold = static_cast<float>(v); });
        addInt(tr("Max steps"), p->maxSteps, 8, 512, [](ChainNode& n, int v) { std::get<Fractal3DParams>(n.params).maxSteps = v; });
        addInt(tr("DE iterations"), p->maxIter, 1, 64, [](ChainNode& n, int v) { std::get<Fractal3DParams>(n.params).maxIter = v; });
        addDouble(tr("Julia X"), p->juliaX, -2.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).juliaX = static_cast<float>(v); });
        addDouble(tr("Julia Y"), p->juliaY, -2.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).juliaY = static_cast<float>(v); });
        addDouble(tr("Julia Z"), p->juliaZ, -2.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).juliaZ = static_cast<float>(v); });
        addDouble(tr("Julia W"), p->juliaW, -2.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).juliaW = static_cast<float>(v); });
        addDouble(tr("Light yaw"), p->lightYaw, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).lightYaw = static_cast<float>(v); });
        addDouble(tr("Light pitch"), p->lightPitch, -1.5, 1.5, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).lightPitch = static_cast<float>(v); });
        addDouble(tr("Ambient"), p->ambient, 0.0, 1.0, 0.02, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).ambient = static_cast<float>(v); });
        addBool(tr("AO"), p->ao, [](ChainNode& n, bool v) { std::get<Fractal3DParams>(n.params).ao = v; });
        addDouble(tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble(tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).colorCycle = static_cast<float>(v); });
        addColor(tr("Background"), p->background, [](ChainNode& n, uint32_t v) { std::get<Fractal3DParams>(n.params).background = v; });
        addGradient(tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<Fractal3DParams>(n.params).gradientPreset = std::move(v); });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<Fractal3DParams>(n.params).blend = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<Fractal3DParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Fractal3DParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Fractal3DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<LyapunovParams>(&params))
    {
        addText(tr("Sequence"), p->sequence, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).sequence = std::move(v); });
        addDouble(tr("a min"), p->aMin, 0.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).aMin = static_cast<float>(v); });
        addDouble(tr("a max"), p->aMax, 0.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).aMax = static_cast<float>(v); });
        addDouble(tr("b min"), p->bMin, 0.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).bMin = static_cast<float>(v); });
        addDouble(tr("b max"), p->bMax, 0.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).bMax = static_cast<float>(v); });
        addInt(tr("Warmup"), p->warmup, 0, 2000, [](ChainNode& n, int v) { std::get<LyapunovParams>(n.params).warmup = v; });
        addInt(tr("Iterations"), p->iterations, 1, 4000, [](ChainNode& n, int v) { std::get<LyapunovParams>(n.params).iterations = v; });
        addColor(tr("Ordered colour"), p->negColor, [](ChainNode& n, uint32_t v) { std::get<LyapunovParams>(n.params).negColor = v; });
        addDouble(tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble(tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).colorCycle = static_cast<float>(v); });
        addGradient(tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).gradientPreset = std::move(v); });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<LyapunovParams>(n.params).blend = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<KleinianParams>(&params))
    {
        addInt(tr("p"), p->p, 3, 20, [](ChainNode& n, int v) { std::get<KleinianParams>(n.params).p = v; });
        addInt(tr("q"), p->q, 3, 20, [](ChainNode& n, int v) { std::get<KleinianParams>(n.params).q = v; });
        addInt(tr("Iterations"), p->iterations, 1, 200, [](ChainNode& n, int v) { std::get<KleinianParams>(n.params).iterations = v; });
        addDouble(tr("Morph"), p->morph, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).morph = static_cast<float>(v); });
        addDouble(tr("Zoom"), p->zoom, 0.1, 8.0, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).zoom = static_cast<float>(v); });
        addDouble(tr("Rotation"), p->rotation, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).rotation = static_cast<float>(v); });
        addDouble(tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble(tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).colorCycle = static_cast<float>(v); });
        addGradient(tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<KleinianParams>(n.params).gradientPreset = std::move(v); });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<KleinianParams>(n.params).blend = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<KleinianParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<KleinianParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<KleinianParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FractalZoomerParams>(&params))
    {
        addEnum(tr("Type"), p->type, {"Mandelbrot", "Julia", "Burning Ship"}, [](ChainNode& n, int v) { std::get<FractalZoomerParams>(n.params).type = v; });
        addDouble(tr("Center X"), p->centerX, -2.0, 2.0, 0.0001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).centerX = static_cast<float>(v); });
        addDouble(tr("Center Y"), p->centerY, -2.0, 2.0, 0.0001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).centerY = static_cast<float>(v); });
        addDouble(tr("Julia X"), p->juliaX, -2.0, 2.0, 0.001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).juliaX = static_cast<float>(v); });
        addDouble(tr("Julia Y"), p->juliaY, -2.0, 2.0, 0.001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).juliaY = static_cast<float>(v); });
        addInt(tr("Max iterations"), p->maxIter, 1, 2048, [](ChainNode& n, int v) { std::get<FractalZoomerParams>(n.params).maxIter = v; });
        addDouble(tr("Zoom speed"), p->zoomSpeed, 0.9, 1.2, 0.001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).zoomSpeed = static_cast<float>(v); });
        addDouble(tr("Rotation speed"), p->rotationSpeed, -0.2, 0.2, 0.001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).rotationSpeed = static_cast<float>(v); });
        addDouble(tr("Feedback"), p->feedback, 0.0, 1.0, 0.02, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).feedback = static_cast<float>(v); });
        addDouble(tr("Colour scale"), p->colorScale, 0.001, 4.0, 0.005, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble(tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).colorCycle = static_cast<float>(v); });
        addColor(tr("Inside colour"), p->insideColor, [](ChainNode& n, uint32_t v) { std::get<FractalZoomerParams>(n.params).insideColor = v; });
        addGradient(tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<FractalZoomerParams>(n.params).gradientPreset = std::move(v); });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<FractalZoomerParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FractalZoomerParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FractalZoomerParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<StrangeAttractorParams>(&params))
    {
        addEnum(tr("Type"), p->type, {"Lorenz", "Clifford", "De Jong", "Aizawa"}, [](ChainNode& n, int v) { std::get<StrangeAttractorParams>(n.params).type = v; });
        addDouble(tr("a"), p->a, -3.0, 3.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).a = static_cast<float>(v); });
        addDouble(tr("b"), p->b, -3.0, 3.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).b = static_cast<float>(v); });
        addDouble(tr("c"), p->c, -3.0, 3.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).c = static_cast<float>(v); });
        addDouble(tr("d"), p->d, -3.0, 3.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).d = static_cast<float>(v); });
        addInt(tr("Points"), p->points, 1, 100000, [](ChainNode& n, int v) { std::get<StrangeAttractorParams>(n.params).points = v; });
        addDouble(tr("Scale"), p->scale, 0.01, 4.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).scale = static_cast<float>(v); });
        addDouble(tr("Rotation"), p->rotation, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).rotation = static_cast<float>(v); });
        addDouble(tr("Rotation speed"), p->rotationSpeed, -2.0, 2.0, 0.02, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).rotationSpeed = static_cast<float>(v); });
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<StrangeAttractorParams>(n.params).color = v; });
        addBool(tr("Use gradient"), p->useGradient, [](ChainNode& n, bool v) { std::get<StrangeAttractorParams>(n.params).useGradient = v; });
        addGradient(tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<StrangeAttractorParams>(n.params).gradientPreset = std::move(v); });
        addDouble(tr("Dot size"), p->dotSize, 1.0, 32.0, 0.5, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<StrangeAttractorParams>(n.params).blend = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<StrangeAttractorParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<StrangeAttractorParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<StrangeAttractorParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FlameParams>(&params))
    {
        addEnum(tr("Variation"), p->variation, {"Linear", "Sinusoidal", "Spherical", "Swirl", "Horseshoe"}, [](ChainNode& n, int v) { std::get<FlameParams>(n.params).variation = v; });
        addInt(tr("Functions"), p->functions, 2, 4, [](ChainNode& n, int v) { std::get<FlameParams>(n.params).functions = v; });
        addInt(tr("Points"), p->points, 1, 200000, [](ChainNode& n, int v) { std::get<FlameParams>(n.params).points = v; });
        addDouble(tr("Scale"), p->scale, 0.05, 4.0, 0.02, [](ChainNode& n, double v) { std::get<FlameParams>(n.params).scale = static_cast<float>(v); });
        addDouble(tr("Rotation"), p->rotation, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<FlameParams>(n.params).rotation = static_cast<float>(v); });
        addDouble(tr("Rotation speed"), p->rotationSpeed, -2.0, 2.0, 0.02, [](ChainNode& n, double v) { std::get<FlameParams>(n.params).rotationSpeed = static_cast<float>(v); });
        addGradient(tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<FlameParams>(n.params).gradientPreset = std::move(v); });
        addDouble(tr("Dot size"), p->dotSize, 1.0, 16.0, 0.5, [](ChainNode& n, double v) { std::get<FlameParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<FlameParams>(n.params).blend = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<FlameParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FlameParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FlameParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ReactionDiffusionParams>(&params))
    {
        addDouble(tr("Feed"), p->feed, 0.0, 0.1, 0.001, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).feed = static_cast<float>(v); });
        addDouble(tr("Kill"), p->kill, 0.0, 0.1, 0.001, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).kill = static_cast<float>(v); });
        addDouble(tr("Diffuse A"), p->diffA, 0.0, 2.0, 0.02, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).diffA = static_cast<float>(v); });
        addDouble(tr("Diffuse B"), p->diffB, 0.0, 2.0, 0.02, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).diffB = static_cast<float>(v); });
        addInt(tr("Steps/frame"), p->stepsPerFrame, 1, 64, [](ChainNode& n, int v) { std::get<ReactionDiffusionParams>(n.params).stepsPerFrame = v; });
        addBool(tr("Seed on beat"), p->seedOnBeat, [](ChainNode& n, bool v) { std::get<ReactionDiffusionParams>(n.params).seedOnBeat = v; });
        addDouble(tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble(tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).colorCycle = static_cast<float>(v); });
        addGradient(tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<ReactionDiffusionParams>(n.params).gradientPreset = std::move(v); });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<ReactionDiffusionParams>(n.params).blend = v; });
        addScript(tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<ReactionDiffusionParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ReactionDiffusionParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ReactionDiffusionParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DebugBarsParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<DebugBarsParams>(n.params).color = v; });
        addDouble(tr("Orbit speed"), p->orbitSpeed, -10.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<DebugBarsParams>(n.params).orbitSpeed = static_cast<float>(v); });
    }
    else if (auto* p = std::get_if<HostGroupParams>(&params))
    {
        // HG1 (HostGruppen_Crossfade_Entwurf.md): Level-1-Host — eigener
        // persistenter Buffer (Feedback), eigener Buffer-Pool + ScriptContext.
        auto* info = new QLabel(
            tr("Host-Gruppe: kapselt ein komplettes Visual (eigener "
               "Feedback-Buffer, eigene Buffer-Save-Slots und Skript-Globals). "
               "Mehrere aktive Gruppen stapeln sich ueber Blend Out; der "
               "Crossfade (HG2) blendet beim Wechsel."),
            m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);

        addEnum(tr("Blend Out"), static_cast<int>(p->blendOut), kBlendNames,
                [](ChainNode& n, int v) {
                    std::get<HostGroupParams>(n.params).blendOut =
                        static_cast<BlendMode>(v);
                });
        addInt(tr("Out Alpha"), p->outAdjustAlpha, 0, 255,
               [](ChainNode& n, int v) {
                   std::get<HostGroupParams>(n.params).outAdjustAlpha = v;
               });

        auto* syncNote = new QLabel(
            tr("Hinweis: Die Crossfade-Dauer gilt synchron fuer ALLE "
               "Host-Gruppen — eine Aenderung hier wird auf alle uebertragen "
               "(Entwurf-Regel). Die Kurven sind je Gruppe individuell."),
            m_propContainer);
        syncNote->setWordWrap(true);
        form->addRow(syncNote);
        addDouble(tr("Crossfade-Dauer (s)"), p->crossfadeSeconds, 0.0, 60.0, 0.1,
                  [this](ChainNode& n, double v) {
                      std::get<HostGroupParams>(n.params).crossfadeSeconds = v;
                      // Sync-Regel (Entwurf §2.4) — laeuft unter dem
                      // renderMutex von mutate(): alle Gruppen angleichen
                      if (m_host == nullptr) return;
                      std::function<void(ChainNode&)> sync = [&](ChainNode& c) {
                          if (auto* g = std::get_if<HostGroupParams>(&c.params))
                              g->crossfadeSeconds = v;
                          for (ChainNode& ch : c.children) sync(ch);
                      };
                      sync(m_host->chain());
                  });
        const QStringList curveNames = {tr("Linear"), tr("S-Kurve (smooth)"),
                                        tr("Ease-In"), tr("Ease-Out"),
                                        tr("Exponentiell")};
        addEnum(tr("Eingangskurve"), std::clamp(p->curveIn, 0, 4), curveNames,
                [](ChainNode& n, int v) {
                    std::get<HostGroupParams>(n.params).curveIn = v;
                });
        addEnum(tr("Ausgangskurve"), std::clamp(p->curveOut, 0, 4), curveNames,
                [](ChainNode& n, int v) {
                    std::get<HostGroupParams>(n.params).curveOut = v;
                });

        // Manueller Wechsel-Trigger (Vorstufe der Visual-Playlist E6): diese
        // Gruppe einblenden, alle anderen Host-Gruppen ausblenden lassen.
        auto* switchBtn = new QPushButton(
            tr("Zu dieser Gruppe wechseln (Crossfade)"), m_propContainer);
        connect(switchBtn, &QPushButton::clicked, this, [this, path]() {
            if (m_host == nullptr) return;
            mutateStructure([&] {
                ChainNode* me = nodeAtPath(path);
                if (me == nullptr || !me->isHostGroup()) return;
                std::function<void(ChainNode&)> toggle = [&](ChainNode& n) {
                    if (n.isHostGroup()) n.enabled = (&n == me);
                    for (ChainNode& c : n.children) toggle(c);
                };
                toggle(m_host->chain());
            });
        });
        form->addRow(switchBtn);

        if (!p->sourceFile.empty())
        {
            auto* src = new QLabel(tr("Quelle: %1").arg(QString::fromStdString(
                                       p->sourceFile)),
                                   m_propContainer);
            src->setWordWrap(true);
            form->addRow(src);
        }
        auto* importBtn = new QPushButton(
            tr(".lvfx in diese Gruppe importieren..."), m_propContainer);
        connect(importBtn, &QPushButton::clicked, this, [this, path]() {
            const QString file = QFileDialog::getOpenFileName(
                this, tr("Import .lvfx in Host-Gruppe"), QString(),
                tr("LumiViz Effect Chain (*.lvfx)"));
            if (file.isEmpty()) return;
            ChainNode loaded;
            QStringList report;
            if (!lumi::multieffect::loadChainFromFile(file, loaded, &report))
            {
                QMessageBox::warning(this, tr("Import .lvfx"),
                                     tr("Konnte nicht laden:\n%1").arg(file));
                return;
            }
            if (chainHasHostGroup(loaded))
            {
                QMessageBox::information(
                    this, tr("Import .lvfx"),
                    tr("Die Quelle enthaelt selbst Host-Gruppen — sie werden "
                       "beim Uebernehmen zur Effect List degradiert "
                       "(Tiefenregel)."));
            }
            mutateStructure([&] {
                ChainNode* node = nodeAtPath(path);
                auto* hg = node != nullptr
                               ? std::get_if<HostGroupParams>(&node->params)
                               : nullptr;
                if (hg == nullptr) return;
                // Frische IDs — compileChain vergibt nur an nodeId==0
                std::function<void(ChainNode&)> clearIds = [&](ChainNode& n) {
                    n.nodeId = 0;
                    for (ChainNode& c : n.children) clearIds(c);
                };
                clearIds(loaded);
                node->children = std::move(loaded.children);
                hg->sourceFile = file.toStdString();
            });
        });
        form->addRow(importBtn);
    }
    else if (auto* p = std::get_if<MilkdropNodeParams>(&params))
    {
        // Jede Mutation bumpt die Revision — der Render-Host uebernimmt den
        // Node-Zustand (Skripte/Shader/Texturen) nur bei neuer Revision
        auto milkOf = [](ChainNode& n) -> MilkdropNodeParams& {
            auto& mp = std::get<MilkdropNodeParams>(n.params);
            ++mp.revision;
            return mp;
        };
        const auto className = [](lumi::milk::ShaderClass c) -> QString {
            switch (c)
            {
            case lumi::milk::ShaderClass::None: return QStringLiteral("MD1");
            case lumi::milk::ShaderClass::Md1Default: return QStringLiteral("MD1-Default (baked)");
            case lumi::milk::ShaderClass::Md1Plus: return QStringLiteral("MD1+Blur (baked)");
            case lumi::milk::ShaderClass::Custom: return QStringLiteral("Custom (C1)");
            }
            return QStringLiteral("?");
        };

        // Sektions-genaue ⓘ-Referenz (per_frame/pixel · Wave · Shape · Sprite)
        if (milkSection >= 0) scriptRef = milkSectionReferenceHtml(milkSection);

        if (milkSection < 0)  // Node selbst = Preset-Uebersicht
        {
            auto* info = new QLabel(
                tr("MilkDrop-Preset '%1' — PS%2 · Warp: %3 · Comp: %4 · %5 Sprite(s)\n"
                   "Sektionen (Code / Waves / Shapes / Shader) als Kinder im Baum")
                    .arg(QString::fromStdString(p->preset.name))
                    .arg(p->preset.psVersion)
                    .arg(className(p->preset.warpInfo.shaderClass))
                    .arg(className(p->preset.compInfo.shaderClass))
                    .arg(p->preset.sprites.size()),
                m_propContainer);
            info->setWordWrap(true);
            form->addRow(info);
            addInt(tr("Mesh X"), p->meshX, 8, 96,
                   [milkOf](ChainNode& n, int v) { milkOf(n).meshX = v; });
            addInt(tr("Mesh Y"), p->meshY, 6, 72,
                   [milkOf](ChainNode& n, int v) { milkOf(n).meshY = v; });
            addBool(tr("Kalibrier-Raster"), p->debugGrid,
                    [milkOf](ChainNode& n, bool v) { milkOf(n).debugGrid = v; });
            addText(tr("Textur-Basisordner"), p->presetDir,
                    [milkOf](ChainNode& n, std::string v) {
                        milkOf(n).presetDir = std::move(v);
                    });
        }
        else if (milkSection == 0)  // Code-Slots (EEL, Dialekt Milkdrop)
        {
            addScript(tr("Init (per_frame_init)"), p->preset.perFrameInit,
                      [milkOf](ChainNode& n, std::string v) {
                          milkOf(n).preset.perFrameInit = std::move(v);
                      });
            addScript(tr("Frame (per_frame)"), p->preset.perFrame,
                      [milkOf](ChainNode& n, std::string v) {
                          milkOf(n).preset.perFrame = std::move(v);
                      });
            addScript(tr("Point (per_pixel)"), p->preset.perPixel,
                      [milkOf](ChainNode& n, std::string v) {
                          milkOf(n).preset.perPixel = std::move(v);
                      });
        }
        else if (milkSection == 1 && milkElem == kMilkBasisWaveElement)
        {
            // Basis-Waveform (S43): die immer gerenderte Standard-Welle des
            // Presets — vorher nur unter Parameter (Basiswerte) versteckt.
            using PS = lumi::milkdrop::PresetState;
            const auto setD = [milkOf](double PS::* m) {
                return [milkOf, m](ChainNode& n, double v) {
                    milkOf(n).preset.*m = v;
                };
            };
            const auto setB = [milkOf](bool PS::* m) {
                return [milkOf, m](ChainNode& n, bool v) {
                    milkOf(n).preset.*m = v;
                };
            };
            const auto setI = [milkOf](int PS::* m) {
                return [milkOf, m](ChainNode& n, int v) {
                    milkOf(n).preset.*m = v;
                };
            };
            auto* hint = new QLabel(
                tr("Die Basis-Waveform wird immer gerendert (Alpha 0 = "
                   "unsichtbar). Startwerte — per_frame-Code kann wave_* je "
                   "Frame ueberschreiben."),
                m_propContainer);
            hint->setWordWrap(true);
            form->addRow(hint);
            addInt(tr("Wave-Modus (0-7)"), p->preset.waveMode, 0, 7, setI(&PS::waveMode));
            addBool(tr("Additiv"), p->preset.additiveWaves, setB(&PS::additiveWaves));
            addBool(tr("Punkte statt Linien"), p->preset.waveDots, setB(&PS::waveDots));
            addBool(tr("Dick"), p->preset.waveThick, setB(&PS::waveThick));
            addBool(tr("Alpha nach Lautstaerke"), p->preset.modWaveAlphaByVolume, setB(&PS::modWaveAlphaByVolume));
            addBool(tr("Farbe maximieren"), p->preset.maximizeWaveColor, setB(&PS::maximizeWaveColor));
            addDouble(tr("Alpha"), p->preset.waveAlpha, 0.0, 10.0, 0.01, setD(&PS::waveAlpha));
            addDouble(tr("Skalierung"), p->preset.waveScale, 0.0, 100.0, 0.01, setD(&PS::waveScale));
            addDouble(tr("Glaettung"), p->preset.waveSmoothing, 0.0, 0.95, 0.01, setD(&PS::waveSmoothing));
            addDouble(tr("Mystery (wave_mystery)"), p->preset.waveMystery, -2.0, 2.0, 0.01, setD(&PS::waveMystery));
            addDouble(tr("Mod-Alpha Start"), p->preset.modWaveAlphaStart, 0.0, 2.0, 0.01, setD(&PS::modWaveAlphaStart));
            addDouble(tr("Mod-Alpha Ende"), p->preset.modWaveAlphaEnd, 0.0, 2.0, 0.01, setD(&PS::modWaveAlphaEnd));
            addDouble(tr("Rot"), p->preset.waveR, 0.0, 1.0, 0.01, setD(&PS::waveR));
            addDouble(tr("Gruen"), p->preset.waveG, 0.0, 1.0, 0.01, setD(&PS::waveG));
            addDouble(tr("Blau"), p->preset.waveB, 0.0, 1.0, 0.01, setD(&PS::waveB));
            addDouble(tr("Position X"), p->preset.waveX, 0.0, 1.0, 0.01, setD(&PS::waveX));
            addDouble(tr("Position Y"), p->preset.waveY, 0.0, 1.0, 0.01, setD(&PS::waveY));
        }
        else if (milkSection == 1)  // Custom Waves (Sektion oder Einzel-Element)
        {
            if (p->preset.waves.empty())
                form->addRow(new QLabel(tr("Keine Custom-Waves im Preset"),
                                        m_propContainer));
            if (milkElem < 0)
                form->addRow(new QLabel(
                    tr("Neu anlegen: 'Custom Wave' im Dropdown + '+'. "
                       "Entfernen/Klonen: Wave im Baum markieren."),
                    m_propContainer));
            const std::size_t nWaves = p->preset.waves.size();
            const std::size_t wBegin =
                milkElem >= 0 ? std::min<std::size_t>(milkElem, nWaves) : 0;
            const std::size_t wEnd =
                milkElem >= 0 ? std::min<std::size_t>(milkElem + 1, nWaves)
                              : nWaves;
            for (std::size_t i = wBegin; i < wEnd; ++i)
            {
                const auto& w = p->preset.waves[i];
                addBool(tr("Wave %1 aktiv").arg(w.index), w.enabled,
                        [milkOf, i](ChainNode& n, bool v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.waves.size())
                                mp.preset.waves[i].enabled = v;
                        });
                const auto setWave = [milkOf, i](std::string lumi::milkdrop::WaveState::* member) {
                    return [milkOf, i, member](ChainNode& n, std::string v) {
                        auto& mp = milkOf(n);
                        if (i < mp.preset.waves.size())
                            mp.preset.waves[i].*member = std::move(v);
                    };
                };
                // N3.2: numerische Init-Parameter — nur in der Einzel-Ansicht
                // (die Sektions-Liste bleibt kompakt: enabled + Codes)
                if (milkElem >= 0)
                {
                    using WS = lumi::milkdrop::WaveState;
                    const auto wD = [milkOf, i](double WS::* m) {
                        return [milkOf, i, m](ChainNode& n, double v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.waves.size()) mp.preset.waves[i].*m = v;
                        };
                    };
                    const auto wI = [milkOf, i](int WS::* m) {
                        return [milkOf, i, m](ChainNode& n, int v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.waves.size()) mp.preset.waves[i].*m = v;
                        };
                    };
                    const auto wB = [milkOf, i](bool WS::* m) {
                        return [milkOf, i, m](ChainNode& n, bool v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.waves.size()) mp.preset.waves[i].*m = v;
                        };
                    };
                    addInt(tr("Samples"), w.samples, 2, 512, wI(&WS::samples));
                    addInt(tr("L/R-Versatz (sep)"), w.sep, 0, 256, wI(&WS::sep));
                    addBool(tr("Spektrum statt Waveform"), w.spectrum, wB(&WS::spectrum));
                    addBool(tr("Punkte"), w.useDots, wB(&WS::useDots));
                    addBool(tr("Dick"), w.drawThick, wB(&WS::drawThick));
                    addBool(tr("Additiv"), w.additive, wB(&WS::additive));
                    addDouble(tr("Skalierung"), w.scaling, 0.0, 100.0, 0.01, wD(&WS::scaling));
                    addDouble(tr("Glaettung"), w.smoothing, 0.0, 1.0, 0.01, wD(&WS::smoothing));
                    addDouble(tr("Rot"), w.r, 0.0, 1.0, 0.01, wD(&WS::r));
                    addDouble(tr("Gruen"), w.g, 0.0, 1.0, 0.01, wD(&WS::g));
                    addDouble(tr("Blau"), w.b, 0.0, 1.0, 0.01, wD(&WS::b));
                    addDouble(tr("Alpha"), w.a, 0.0, 1.0, 0.01, wD(&WS::a));
                }
                addScript(tr("Wave %1 · Init").arg(w.index), w.initCode,
                          setWave(&lumi::milkdrop::WaveState::initCode));
                addScript(tr("Wave %1 · Frame").arg(w.index), w.frameCode,
                          setWave(&lumi::milkdrop::WaveState::frameCode));
                addScript(tr("Wave %1 · Point").arg(w.index), w.pointCode,
                          setWave(&lumi::milkdrop::WaveState::pointCode));
            }
        }
        else if (milkSection == 2)  // Custom Shapes (Sektion oder Einzel-Element)
        {
            if (p->preset.shapes.empty())
                form->addRow(new QLabel(tr("Keine Custom-Shapes im Preset"),
                                        m_propContainer));
            if (milkElem < 0)
                form->addRow(new QLabel(
                    tr("Neu anlegen: 'Custom Shape' im Dropdown + '+'. "
                       "Entfernen/Klonen: Shape im Baum markieren."),
                    m_propContainer));
            const std::size_t nShapes = p->preset.shapes.size();
            const std::size_t sBegin =
                milkElem >= 0 ? std::min<std::size_t>(milkElem, nShapes) : 0;
            const std::size_t sEnd =
                milkElem >= 0 ? std::min<std::size_t>(milkElem + 1, nShapes)
                              : nShapes;
            for (std::size_t i = sBegin; i < sEnd; ++i)
            {
                const auto& s = p->preset.shapes[i];
                addBool(tr("Shape %1 aktiv").arg(s.index), s.enabled,
                        [milkOf, i](ChainNode& n, bool v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.shapes.size())
                                mp.preset.shapes[i].enabled = v;
                        });
                const auto setShape = [milkOf, i](std::string lumi::milkdrop::ShapeState::* member) {
                    return [milkOf, i, member](ChainNode& n, std::string v) {
                        auto& mp = milkOf(n);
                        if (i < mp.preset.shapes.size())
                            mp.preset.shapes[i].*member = std::move(v);
                    };
                };
                // N3.2: numerische Init-Parameter — nur in der Einzel-Ansicht
                if (milkElem >= 0)
                {
                    using SS = lumi::milkdrop::ShapeState;
                    const auto sD = [milkOf, i](double SS::* m) {
                        return [milkOf, i, m](ChainNode& n, double v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.shapes.size()) mp.preset.shapes[i].*m = v;
                        };
                    };
                    const auto sI = [milkOf, i](int SS::* m) {
                        return [milkOf, i, m](ChainNode& n, int v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.shapes.size()) mp.preset.shapes[i].*m = v;
                        };
                    };
                    const auto sB = [milkOf, i](bool SS::* m) {
                        return [milkOf, i, m](ChainNode& n, bool v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.shapes.size()) mp.preset.shapes[i].*m = v;
                        };
                    };
                    addInt(tr("Seiten"), s.sides, 3, 100, sI(&SS::sides));
                    addInt(tr("Instanzen (num_inst)"), s.instances, 1, 1024, sI(&SS::instances));
                    addBool(tr("Additiv"), s.additive, sB(&SS::additive));
                    addBool(tr("Dicker Rand"), s.thickOutline, sB(&SS::thickOutline));
                    addBool(tr("Texturiert (Vorframe)"), s.textured, sB(&SS::textured));
                    addDouble(tr("Textur-Zoom"), s.texZoom, 0.01, 10.0, 0.01, sD(&SS::texZoom));
                    addDouble(tr("Textur-Winkel"), s.texAng, -6.3, 6.3, 0.01, sD(&SS::texAng));
                    addDouble(tr("Position X"), s.x, 0.0, 1.0, 0.001, sD(&SS::x));
                    addDouble(tr("Position Y"), s.y, 0.0, 1.0, 0.001, sD(&SS::y));
                    addDouble(tr("Radius"), s.rad, 0.0, 2.0, 0.001, sD(&SS::rad));
                    addDouble(tr("Winkel"), s.ang, -6.3, 6.3, 0.01, sD(&SS::ang));
                    addDouble(tr("Zentrum R"), s.r, 0.0, 1.0, 0.01, sD(&SS::r));
                    addDouble(tr("Zentrum G"), s.g, 0.0, 1.0, 0.01, sD(&SS::g));
                    addDouble(tr("Zentrum B"), s.b, 0.0, 1.0, 0.01, sD(&SS::b));
                    addDouble(tr("Zentrum A"), s.a, 0.0, 1.0, 0.01, sD(&SS::a));
                    addDouble(tr("Rand R (r2)"), s.r2, 0.0, 1.0, 0.01, sD(&SS::r2));
                    addDouble(tr("Rand G (g2)"), s.g2, 0.0, 1.0, 0.01, sD(&SS::g2));
                    addDouble(tr("Rand B (b2)"), s.b2, 0.0, 1.0, 0.01, sD(&SS::b2));
                    addDouble(tr("Rand A (a2)"), s.a2, 0.0, 1.0, 0.01, sD(&SS::a2));
                    addDouble(tr("Border R"), s.borderR, 0.0, 1.0, 0.01, sD(&SS::borderR));
                    addDouble(tr("Border G"), s.borderG, 0.0, 1.0, 0.01, sD(&SS::borderG));
                    addDouble(tr("Border B"), s.borderB, 0.0, 1.0, 0.01, sD(&SS::borderB));
                    addDouble(tr("Border A"), s.borderA, 0.0, 1.0, 0.01, sD(&SS::borderA));
                }
                addScript(tr("Shape %1 · Init").arg(s.index), s.initCode,
                          setShape(&lumi::milkdrop::ShapeState::initCode));
                addScript(tr("Shape %1 · Frame").arg(s.index), s.frameCode,
                          setShape(&lumi::milkdrop::ShapeState::frameCode));
            }
        }
        else if (milkSection == 3)  // Warp/Comp-Shader (HLSL, SSOT — Klassifikation wird neu abgeleitet)
        {
            auto addHlsl = [&](const QString& label, const std::string& value,
                               bool isWarp) {
                auto* edit = new QPlainTextEdit(m_propContainer);
                edit->setPlainText(QString::fromStdString(value));
                edit->setPlaceholderText(tr("HLSL shader_body (leer = MD1-Pfad)"));
                edit->setLineWrapMode(QPlainTextEdit::NoWrap);
                edit->setMinimumHeight(80);
                edit->setMaximumHeight(260);
                QFont mono(QStringLiteral("Consolas"));
                mono.setStyleHint(QFont::Monospace);
                edit->setFont(mono);
                connect(edit, &QPlainTextEdit::textChanged, this,
                        [this, path, isWarp, edit]() {
                            const std::string text = edit->toPlainText().toStdString();
                            mutate(path, [&](ChainNode& n) {
                                auto& mp = std::get<MilkdropNodeParams>(n.params);
                                ++mp.revision;
                                if (isWarp)
                                {
                                    mp.preset.warpShaderText = text;
                                    mp.preset.warpInfo =
                                        lumi::milk::analyzeWarpShader(text);
                                }
                                else
                                {
                                    mp.preset.compShaderText = text;
                                    mp.preset.compInfo =
                                        lumi::milk::analyzeCompShader(text);
                                }
                            });
                        });
                // ⓘ-Referenz (Befund S42: die HLSL-Editoren hatten keine) —
                // Inputs/Konstanten/Sampler/Funktionen der Praeambel
                auto* header = new QWidget(m_propContainer);
                auto* hl = new QHBoxLayout(header);
                hl->setContentsMargins(0, 0, 0, 0);
                hl->addWidget(new QLabel(label, header));
                hl->addStretch(1);
                auto* refBtn = new QToolButton(header);
                refBtn->setText(QString::fromUtf8("ⓘ"));
                refBtn->setToolTip(
                    tr("Shader-Referenz (Inputs, Konstanten, Sampler, Funktionen)"));
                hl->addWidget(refBtn);
                connect(refBtn, &QToolButton::clicked, this, [this]() {
                    showScriptReference(this, milkShaderReferenceHtml());
                });
                form->addRow(header);
                form->addRow(edit);
            };
            addHlsl(tr("Warp-Shader (HLSL)"), p->preset.warpShaderText, true);
            addHlsl(tr("Comp-Shader (HLSL)"), p->preset.compShaderText, false);
        }
        else if (milkSection == 4)  // Sprites (N3.3 — Sektion oder Einzel-Element)
        {
            if (p->preset.sprites.empty())
                form->addRow(new QLabel(tr("Keine Sprites im Preset"),
                                        m_propContainer));
            if (milkElem < 0)
                form->addRow(new QLabel(
                    tr("Neu anlegen: 'Sprite' im Dropdown + '+'. "
                       "Entfernen/Klonen: Sprite im Baum markieren."),
                    m_propContainer));
            const std::size_t nSprites = p->preset.sprites.size();
            const std::size_t bBegin =
                milkElem >= 0 ? std::min<std::size_t>(milkElem, nSprites) : 0;
            const std::size_t bEnd =
                milkElem >= 0 ? std::min<std::size_t>(milkElem + 1, nSprites)
                              : nSprites;
            for (std::size_t i = bBegin; i < bEnd; ++i)
            {
                const auto& sp = p->preset.sprites[i];
                using lumi::milkdrop::SpriteState;
                // Bounds-gesicherte Mutation eines Sprite-Felds (+Revision)
                auto spriteSet = [milkOf, i](auto setter) {
                    return [milkOf, i, setter](ChainNode& n, auto v) {
                        auto& mp = milkOf(n);
                        if (i < mp.preset.sprites.size())
                            setter(mp.preset.sprites[i], v);
                    };
                };
                const QString t = tr("Sprite %1").arg(sp.index);
                addText(t + tr(" · Bild (relativ zum Textur-Ordner)"),
                        sp.imageName, spriteSet([](SpriteState& s, std::string v) {
                            s.imageName = std::move(v);
                        }));
                addColor(t + tr(" · Colorkey"), sp.colorKey,
                         spriteSet([](SpriteState& s, uint32_t v) {
                             s.colorKey = v;
                         }));
                addInt(t + tr(" · Layer"), sp.layer, -100, 100,
                       spriteSet([](SpriteState& s, int v) { s.layer = v; }));
                addInt(t + tr(" · Blend (0=Alpha 1=Decal 2=Add 3=SrcColor 4=Colorkey)"),
                       sp.blendMode, 0, 4,
                       spriteSet([](SpriteState& s, int v) { s.blendMode = v; }));
                addDouble(t + tr(" · Alpha"), sp.alpha, 0.0, 1.0, 0.01,
                          spriteSet([](SpriteState& s, double v) { s.alpha = v; }));
                addDouble(t + tr(" · Burn"), sp.burn, 0.0, 1.0, 0.01,
                          spriteSet([](SpriteState& s, double v) { s.burn = v; }));
                addDouble(t + tr(" · X"), sp.x, -5.0, 5.0, 0.01,
                          spriteSet([](SpriteState& s, double v) { s.x = v; }));
                addDouble(t + tr(" · Y"), sp.y, -5.0, 5.0, 0.01,
                          spriteSet([](SpriteState& s, double v) { s.y = v; }));
                addDouble(t + tr(" · Skalierung X (negativ = gespiegelt)"),
                          sp.sx, -10.0, 10.0, 0.01,
                          spriteSet([](SpriteState& s, double v) { s.sx = v; }));
                addDouble(t + tr(" · Skalierung Y"), sp.sy, -10.0, 10.0, 0.01,
                          spriteSet([](SpriteState& s, double v) { s.sy = v; }));
                addDouble(t + tr(" · Rotation (rad)"), sp.rot, -10.0, 10.0, 0.01,
                          spriteSet([](SpriteState& s, double v) { s.rot = v; }));
                addDouble(t + tr(" · Speed (time-Skalierung)"), sp.speed,
                          0.0, 10.0, 0.1,
                          spriteSet([](SpriteState& s, double v) { s.speed = v; }));
                addDouble(t + tr(" · Repeat X"), sp.repeatX, 0.01, 100.0, 0.1,
                          spriteSet([](SpriteState& s, double v) {
                              s.repeatX = v;
                          }));
                addDouble(t + tr(" · Repeat Y"), sp.repeatY, 0.01, 100.0, 0.1,
                          spriteSet([](SpriteState& s, double v) {
                              s.repeatY = v;
                          }));
                addScript(t + tr(" · Code (per Frame)"), sp.code,
                          spriteSet([](SpriteState& s, std::string v) {
                              s.code = std::move(v);
                          }));
            }
        }
        else if (milkSection == 5)  // Parameter (N3.2 — numerische Basiswerte)
        {
            using PS = lumi::milkdrop::PresetState;
            // Setter ueber Member-Pointer: jede Aenderung laeuft durch milkOf
            // (Revision-Bump), der Render-Thread uebernimmt den PresetState
            const auto setD = [milkOf](double PS::* m) {
                return [milkOf, m](ChainNode& n, double v) {
                    milkOf(n).preset.*m = v;
                };
            };
            const auto setB = [milkOf](bool PS::* m) {
                return [milkOf, m](ChainNode& n, bool v) {
                    milkOf(n).preset.*m = v;
                };
            };
            const auto setI = [milkOf](int PS::* m) {
                return [milkOf, m](ChainNode& n, int v) {
                    milkOf(n).preset.*m = v;
                };
            };
            const auto group = [&](const QString& t2) {
                auto* l = new QLabel(t2, m_propContainer);
                QFont f2 = l->font();
                f2.setBold(true);
                l->setFont(f2);
                form->addRow(l);
            };

            auto* hint = new QLabel(
                tr("Startwerte des Presets — per_frame-Code kann viele davon "
                   "je Frame ueberschreiben (zoom/rot/wave_*/…)."),
                m_propContainer);
            hint->setWordWrap(true);
            form->addRow(hint);
            const bool bakedComp =
                p->preset.compInfo.shaderClass == lumi::milk::ShaderClass::Md1Default ||
                p->preset.compInfo.shaderClass == lumi::milk::ShaderClass::Md1Plus;
            if (bakedComp || p->preset.compInfo.shaderClass == lumi::milk::ShaderClass::Custom)
            {
                auto* baked = new QLabel(
                    tr("Hinweis: Dieses Preset hat einen Comp-Shader — Gamma/"
                       "Echo/Filter/fShader sind dort EINGEBACKEN und wirken "
                       "hier nicht (Baked-Vertrag; Shader leeren, um sie zu "
                       "aktivieren)."),
                    m_propContainer);
                baked->setWordWrap(true);
                form->addRow(baked);
            }

            group(tr("General / Composite"));
            addDouble(tr("Decay"), p->preset.decay, 0.0, 1.0, 0.001, setD(&PS::decay));
            addDouble(tr("Gamma"), p->preset.gammaAdj, 0.0, 8.0, 0.01, setD(&PS::gammaAdj));
            addDouble(tr("Echo-Zoom"), p->preset.videoEchoZoom, 0.001, 1000.0, 0.01, setD(&PS::videoEchoZoom));
            addDouble(tr("Echo-Alpha"), p->preset.videoEchoAlpha, 0.0, 1.0, 0.01, setD(&PS::videoEchoAlpha));
            addEnum(tr("Echo-Orientierung"), p->preset.videoEchoOrientation,
                    {tr("Keine"), tr("H-Spiegel"), tr("V-Spiegel"), tr("Beide")},
                    setI(&PS::videoEchoOrientation));
            addDouble(tr("fShader-Farbwash"), p->preset.shader, 0.0, 1.0, 0.01, setD(&PS::shader));
            addBool(tr("Textur-Wrap"), p->preset.texWrap, setB(&PS::texWrap));
            addBool(tr("Darken Center"), p->preset.darkenCenter, setB(&PS::darkenCenter));
            addBool(tr("Brighten"), p->preset.brighten, setB(&PS::brighten));
            addBool(tr("Darken"), p->preset.darken, setB(&PS::darken));
            addBool(tr("Solarize"), p->preset.solarize, setB(&PS::solarize));
            addBool(tr("Invert"), p->preset.invert, setB(&PS::invert));

            group(tr("Basis-Waveform"));
            {
                // S43: eigene Ansicht im Baum (Waves -> Basis-Waveform) —
                // hier nur der Wegweiser, keine doppelte Regler-Flaeche.
                auto* basisRef = new QLabel(
                    tr("Eigene Ansicht: Sektion \"Waves\" → "
                       "\"Basis-Waveform\" im Baum."),
                    m_propContainer);
                basisRef->setWordWrap(true);
                form->addRow(basisRef);
            }

            group(tr("Motion (Warp-Mesh)"));
            addDouble(tr("Zoom"), p->preset.zoom, 0.01, 10.0, 0.001, setD(&PS::zoom));
            addDouble(tr("Zoom-Exponent"), p->preset.zoomExponent, 0.01, 10.0, 0.01, setD(&PS::zoomExponent));
            addDouble(tr("Rotation"), p->preset.rot, -2.0, 2.0, 0.001, setD(&PS::rot));
            addDouble(tr("Warp"), p->preset.warp, 0.0, 10.0, 0.01, setD(&PS::warp));
            addDouble(tr("Warp-Anim-Speed"), p->preset.warpAnimSpeed, 0.01, 10.0, 0.01, setD(&PS::warpAnimSpeed));
            addDouble(tr("Warp-Scale"), p->preset.warpScale, 0.01, 10.0, 0.01, setD(&PS::warpScale));
            addDouble(tr("Zentrum X (cx)"), p->preset.cx, 0.0, 1.0, 0.001, setD(&PS::cx));
            addDouble(tr("Zentrum Y (cy)"), p->preset.cy, 0.0, 1.0, 0.001, setD(&PS::cy));
            addDouble(tr("Drift X (dx)"), p->preset.dx, -1.0, 1.0, 0.001, setD(&PS::dx));
            addDouble(tr("Drift Y (dy)"), p->preset.dy, -1.0, 1.0, 0.001, setD(&PS::dy));
            addDouble(tr("Stretch X (sx)"), p->preset.sx, 0.01, 10.0, 0.001, setD(&PS::sx));
            addDouble(tr("Stretch Y (sy)"), p->preset.sy, 0.01, 10.0, 0.001, setD(&PS::sy));

            group(tr("Borders"));
            addDouble(tr("Aussen-Groesse"), p->preset.obSize, 0.0, 0.5, 0.001, setD(&PS::obSize));
            addDouble(tr("Aussen R"), p->preset.obR, 0.0, 1.0, 0.01, setD(&PS::obR));
            addDouble(tr("Aussen G"), p->preset.obG, 0.0, 1.0, 0.01, setD(&PS::obG));
            addDouble(tr("Aussen B"), p->preset.obB, 0.0, 1.0, 0.01, setD(&PS::obB));
            addDouble(tr("Aussen A"), p->preset.obA, 0.0, 1.0, 0.01, setD(&PS::obA));
            addDouble(tr("Innen-Groesse"), p->preset.ibSize, 0.0, 0.5, 0.001, setD(&PS::ibSize));
            addDouble(tr("Innen R"), p->preset.ibR, 0.0, 1.0, 0.01, setD(&PS::ibR));
            addDouble(tr("Innen G"), p->preset.ibG, 0.0, 1.0, 0.01, setD(&PS::ibG));
            addDouble(tr("Innen B"), p->preset.ibB, 0.0, 1.0, 0.01, setD(&PS::ibB));
            addDouble(tr("Innen A"), p->preset.ibA, 0.0, 1.0, 0.01, setD(&PS::ibA));

            group(tr("Motion Vectors"));
            addDouble(tr("Raster X (mv_x)"), p->preset.mvX, 0.0, 64.0, 0.1, setD(&PS::mvX));
            addDouble(tr("Raster Y (mv_y)"), p->preset.mvY, 0.0, 48.0, 0.1, setD(&PS::mvY));
            addDouble(tr("Versatz X (mv_dx)"), p->preset.mvDX, -1.0, 1.0, 0.001, setD(&PS::mvDX));
            addDouble(tr("Versatz Y (mv_dy)"), p->preset.mvDY, -1.0, 1.0, 0.001, setD(&PS::mvDY));
            addDouble(tr("Laenge (mv_l)"), p->preset.mvL, 0.0, 10.0, 0.01, setD(&PS::mvL));
            addDouble(tr("Rot (mv_r)"), p->preset.mvR, 0.0, 1.0, 0.01, setD(&PS::mvR));
            addDouble(tr("Gruen (mv_g)"), p->preset.mvG, 0.0, 1.0, 0.01, setD(&PS::mvG));
            addDouble(tr("Blau (mv_b)"), p->preset.mvB, 0.0, 1.0, 0.01, setD(&PS::mvB));
            addDouble(tr("Alpha (mv_a)"), p->preset.mvA, 0.0, 1.0, 0.01, setD(&PS::mvA));

            group(tr("Blur-Pyramide"));
            addDouble(tr("Blur1 Min"), p->preset.blur1Min, 0.0, 1.0, 0.01, setD(&PS::blur1Min));
            addDouble(tr("Blur1 Max"), p->preset.blur1Max, 0.0, 1.0, 0.01, setD(&PS::blur1Max));
            addDouble(tr("Blur2 Min"), p->preset.blur2Min, 0.0, 1.0, 0.01, setD(&PS::blur2Min));
            addDouble(tr("Blur2 Max"), p->preset.blur2Max, 0.0, 1.0, 0.01, setD(&PS::blur2Max));
            addDouble(tr("Blur3 Min"), p->preset.blur3Min, 0.0, 1.0, 0.01, setD(&PS::blur3Min));
            addDouble(tr("Blur3 Max"), p->preset.blur3Max, 0.0, 1.0, 0.01, setD(&PS::blur3Max));
            addDouble(tr("Blur1 Edge-Darken"), p->preset.blur1EdgeDarken, 0.0, 1.0, 0.01, setD(&PS::blur1EdgeDarken));
        }
    }
    else if (auto* p = std::get_if<TextParams>(&params))
    {
        auto* edit = new QPlainTextEdit(m_propContainer);
        edit->setPlainText(QString::fromStdString(p->text));
        edit->setPlaceholderText(tr("Words separated by ;"));
        edit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        edit->setMinimumHeight(60);
        edit->setMaximumHeight(120);
        connect(edit, &QPlainTextEdit::textChanged, this, [this, path, edit]() {
            const std::string text = edit->toPlainText().toStdString();
            mutate(path, [&](ChainNode& n) {
                std::get<TextParams>(n.params).text = text;
            });
        });
        form->addRow(tr("Text (';' trennt)"), edit);
        addText(tr("Font"), p->fontFace,
                [](ChainNode& n, std::string v) { std::get<TextParams>(n.params).fontFace = std::move(v); });
        addInt(tr("Height (px)"), std::abs(p->fontHeight), 4, 400,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).fontHeight = -v; });
        addInt(tr("Weight"), p->fontWeight, 100, 900,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).fontWeight = v; });
        addBool(tr("Italic"), p->italic,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).italic = v; });
        addBool(tr("Underline"), p->underline,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).underline = v; });
        addColor(tr("Color"), p->color,
                 [](ChainNode& n, uint32_t v) { std::get<TextParams>(n.params).color = v; });
        addEnum(tr("Blend"), std::clamp(p->blend, 0, 2),
                {tr("Replace"), tr("Additive"), tr("50/50")},
                [](ChainNode& n, int v) { std::get<TextParams>(n.params).blend = v; });
        addBool(tr("On beat"), p->onBeat,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).onBeat = v; });
        addInt(tr("Beat hold (frames)"), p->onBeatSpeed, 1, 600,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).onBeatSpeed = v; });
        addInt(tr("Word time (frames)"), p->normSpeed, 1, 600,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).normSpeed = v; });
        addBool(tr("Insert blank"), p->insertBlank,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).insertBlank = v; });
        addBool(tr("Random position"), p->randomPos,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).randomPos = v; });
        addBool(tr("Random word"), p->randomWord,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).randomWord = v; });
        addEnum(tr("H-Align"), std::clamp(p->hAlign, 0, 2),
                {tr("Left"), tr("Center"), tr("Right")},
                [](ChainNode& n, int v) { std::get<TextParams>(n.params).hAlign = v; });
        addEnum(tr("V-Align"), std::clamp(p->vAlign, 0, 2),
                {tr("Top"), tr("Center"), tr("Bottom")},
                [](ChainNode& n, int v) { std::get<TextParams>(n.params).vAlign = v; });
        addInt(tr("X-Shift (%)"), p->xShift, -100, 100,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).xShift = v; });
        addInt(tr("Y-Shift (%)"), p->yShift, -100, 100,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).yShift = v; });
        addBool(tr("Outline"), p->outline,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).outline = v; });
        addColor(tr("Outline color"), p->outlineColor,
                 [](ChainNode& n, uint32_t v) { std::get<TextParams>(n.params).outlineColor = v; });
        addInt(tr("Outline size"), p->outlineSize, 1, 32,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).outlineSize = v; });
        addBool(tr("Shadow"), p->shadow,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).shadow = v; });
    }
    else if (auto* p = std::get_if<AviParams>(&params))
    {
        addText(tr("File"), p->filename,
                [](ChainNode& n, std::string v) { std::get<AviParams>(n.params).filename = std::move(v); });
        addText(tr("Resolved path"), p->resolvedPath,
                [](ChainNode& n, std::string v) { std::get<AviParams>(n.params).resolvedPath = std::move(v); });
        addEnum(tr("Blend"), std::clamp(p->blend, 0, 2),
                {tr("Replace"), tr("Additive"), tr("50/50")},
                [](ChainNode& n, int v) { std::get<AviParams>(n.params).blend = v; });
        addBool(tr("Beat adaptive"), p->adapt,
                [](ChainNode& n, bool v) { std::get<AviParams>(n.params).adapt = v; });
        addInt(tr("Persist (frames)"), p->persist, 0, 32,
               [](ChainNode& n, int v) { std::get<AviParams>(n.params).persist = v; });
        addInt(tr("Frame time (ms)"), p->speedMs, 0, 1000,
               [](ChainNode& n, int v) { std::get<AviParams>(n.params).speedMs = v; });
    }
    else if (auto* p = std::get_if<CommentParams>(&params))
    {
        // Eigenes Mehrzeilen-Feld (Entscheid Patrik S44) — bewusst NICHT die
        // description, damit die Tabellen-Ansicht kompakt bleibt.
        auto* edit = new QPlainTextEdit(m_propContainer);
        edit->setPlainText(QString::fromStdString(p->text));
        edit->setPlaceholderText(tr("Comment"));
        edit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        edit->setMinimumHeight(80);
        connect(edit, &QPlainTextEdit::textChanged, this, [this, path, edit]() {
            const std::string text = edit->toPlainText().toStdString();
            mutate(path, [&](ChainNode& n) {
                std::get<CommentParams>(n.params).text = text;
            });
        });
        form->addRow(tr("Comment"), edit);
    }
    else if (auto* p = std::get_if<ImportNotesParams>(&params))
    {
        // Protokoll des Imports (Entscheid Patrik S51): schreibgeschuetzt, weil
        // es einen Vorgang festhaelt und keine Eingabe ist. Wer es nicht
        // braucht, loescht den Knoten.
        auto* edit = new QPlainTextEdit(m_propContainer);
        edit->setPlainText(QString::fromStdString(p->text));
        edit->setReadOnly(true);
        edit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        edit->setMinimumHeight(160);
        form->addRow(tr("Import notes"), edit);
    }
    else if (auto* p = std::get_if<PassthroughParams>(&params))
    {
        form->addRow(new QLabel(
            tr("Conserved effect (id %1): %2")
                .arg(p->sourceId)
                .arg(QString::fromStdString(p->note)),
            m_propContainer));
    }

    m_propLayout->addWidget(m_propPage);
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
