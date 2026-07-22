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

#include "UI/widgets/GradientPresetDelegate.hpp"          // gradient combo previews
#include "UI/widgets/VisualizerWidget.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"      // figure preset library (SSOT)
#include "visualizers/modules/ColorGradientModule.hpp"   // fractal palette presets
#include "visualizers/multieffect/ChainSerializer.hpp"  // effectTypeKey
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QColorDialog>
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
#include <QSplitter>
#include <QStandardItemModel>
#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QFormLayout>
#include <QHBoxLayout>
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

struct EffectType
{
    const char* name;
    EffectParams (*make)();
    Origin origin = Origin::Avs;

    [[nodiscard]] bool isHeader() const { return make == nullptr; }
};

// Resolve asset/img/logo/icons at runtime: the exe runs from out/build/…, so
// walk upwards from the application dir until the icons appear (dev layout).
// Empty result = no icons deployed -> callers fall back to text markers.
QString originIconDir()
{
    static const QString kDir = [] {
        QDir dir(QCoreApplication::applicationDirPath());
        for (int i = 0; i < 12; ++i)
        {
            const QString candidate =
                dir.filePath(QStringLiteral("asset/img/logo/icons"));
            if (QFileInfo::exists(candidate + QStringLiteral("/lumiviz.ico")))
                return candidate;
            if (!dir.cdUp()) break;
        }
        return QString();
    }();
    return kDir;
}

const QIcon& originIcon(Origin origin)
{
    static const QIcon kAvs(originIconDir() + QStringLiteral("/avs.ico"));
    static const QIcon kMilkDrop(originIconDir() + QStringLiteral("/milkdrop.ico"));
    static const QIcon kNative(originIconDir() + QStringLiteral("/lumiviz.ico"));
    switch (origin)
    {
        case Origin::Avs: return kAvs;
        case Origin::MilkDrop: return kMilkDrop;
        default: return kNative;
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
            if (t.isHeader()) continue;
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
        {"Clear", [] { return EffectParams{ClearParams{}}; }},
        {"Fadeout", [] { return EffectParams{FadeoutParams{}}; }},
        {"OnBeat Clear", [] { return EffectParams{OnBeatClearParams{}}; }},
        {"Buffer Save", [] { return EffectParams{BufferSaveParams{}}; }},
        {"Buffer Blend", [] { return EffectParams{BufferBlendParams{}}; }},
        {"Custom BPM", [] { return EffectParams{CustomBpmParams{}}; }},
        {"Set Render Mode", [] { return EffectParams{SetRenderModeParams{}}; }},
        {"Global Variables", [] { return EffectParams{JherikoGlobalParams{}}; }},
        {"Video Delay", [] { return EffectParams{VideoDelayParams{}}; }},
        {"Multi Delay", [] { return EffectParams{MultiDelayParams{}}; }},
        {"Debug Bars", [] { return EffectParams{DebugBarsParams{}}; }, Origin::Native},

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

// The SuperScope figure presets offered in the editor dropdown. The code lives
// in SuperscopeModule::loadPresetCode (single source of truth); we only pick the
// shape figures here (Custom = no-op, DNA = native-only/empty code are skipped).
const std::vector<lumi::modules::SuperscopePreset>& superscopeFigures()
{
    using P = lumi::modules::SuperscopePreset;
    static const std::vector<P> kList = {
        P::HorizontalScope, P::VerticalScope,   P::Circle,     P::Spiral,
        P::Lissajous,       P::Flower,          P::Star,       P::Starburst,
        P::Heart,           P::SpectrumBars,    P::CircularSpectrum,
        P::Butterfly,       P::Hypocycloid};
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
// Script editing: EEL syntax highlighter + per-module symbol reference
// =============================================================================

// SSOT variable categories (Skript_Variablen_Konzept §3). Drives highlighting
// (§4) and error marking (§6). Only ReadOnly + Constant are "write = error".
enum class SymCat
{
    None,          // custom-local (unknown identifier) — default colour
    ReadOnly,      // host sets, script must not write (w, h, dt)
    Input,         // host sets per frame/point, script reads (audio, i, v, beat)
    Output,        // script sets, host reads (x, y, red/green/blue, skip)
    InOut,         // both (accumulators, driven params)
    Constant,      // fixed (pi, pi2)
    CustomGlobal   // preset-global user vars (reg00-99, q1-64, gmegabuf)
};

// Global (module-independent) classification. The per-module ⓘ reference gives
// the precise contract; this is the broad map for colouring + error marking. The
// ReadOnly/Constant sets are kept small on purpose (no false-positive errors).
SymCat symbolCategory(const QString& n)
{
    static const QSet<QString> kReadOnly = {"w", "h", "dt"};
    static const QSet<QString> kConstant = {"pi", "pi2", "phi", "e"};
    // MilkDrop-Namen (M2-Skript-Vertrag): read-only Frame-Infos + Point-Inputs
    static const QSet<QString> kInput = {
        "i",    "v",        "b",        "bass",     "mid",      "treb",
        "treble", "vol",    "beat",     "time",
        "bass_att", "mid_att", "treb_att", "fps",   "frame",    "progress",
        "meshx", "meshy",   "pixelsx",  "pixelsy",  "aspectx",  "aspecty",
        "sample", "value1", "value2",   "instance", "num_inst"};
    static const QSet<QString> kOutput = {"x",  "y",     "skip",  "x1",   "y1",
                                          "x2", "y2",    "x3",    "y3",   "sizex",
                                          "sizey"};
    static const QSet<QString> kInOut = {
        "n",     "t",       "red",    "green", "blue",  "cx",    "cy",   "zoom",
        "rot",   "jx",      "jy",     "power", "scale", "warp",  "warpscale",
        "speed", "ox",      "oy",     "morph", "feed",  "kill",  "a",    "c",
        "d",     "r",       "amin",   "amax",  "bmin",  "bmax",  "alpha","dist",
        "yaw",   "pitch",   "fold",   "zoomspeed", "rotspeed", "rotation",
        "enabled", "clear", "alphain", "alphaout",
        // MilkDrop per_frame/per_pixel (M2): Warp-Parameter + Post/Deko-Regler
        "zoomexp", "dx", "dy", "sx", "sy", "decay", "gamma", "monitor",
        "wave_a", "wave_r", "wave_g", "wave_b", "wave_x", "wave_y",
        "wave_mystery", "wave_mode", "wave_usedots", "wave_thick",
        "wave_additive", "wave_brighten", "darken_center", "wrap", "invert",
        "brighten", "darken", "solarize", "echo_zoom", "echo_alpha",
        "echo_orient", "ob_size", "ob_r", "ob_g", "ob_b", "ob_a",
        "ib_size", "ib_r", "ib_g", "ib_b", "ib_a",
        "mv_x", "mv_y", "mv_dx", "mv_dy", "mv_l", "mv_r", "mv_g", "mv_b", "mv_a",
        "blur1_min", "blur2_min", "blur3_min", "blur1_max", "blur2_max",
        "blur3_max", "blur1_edge_darken",
        // MilkDrop Wave-/Shape-Code (M2): g steht dort fuer Gruen (i/o);
        // t1-t8 sind wave-/shape-LOKALE Snapshot-Akkus (nicht preset-global)
        "g", "samples", "rad", "ang", "r2", "g2", "b2", "a2",
        "border_r", "border_g", "border_b", "border_a",
        "sides", "textured", "tex_ang", "tex_zoom", "additive", "thick",
        "t1", "t2", "t3", "t4", "t5", "t6", "t7", "t8"};
    if (kReadOnly.contains(n)) return SymCat::ReadOnly;
    if (kConstant.contains(n)) return SymCat::Constant;
    if (kInput.contains(n)) return SymCat::Input;
    if (kOutput.contains(n)) return SymCat::Output;
    if (kInOut.contains(n)) return SymCat::InOut;
    static const QRegularExpression kGlobal("^(reg\\d\\d|q\\d+|gmegabuf)$");
    if (kGlobal.match(n).hasMatch()) return SymCat::CustomGlobal;
    return SymCat::None;
}

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

// EEL/expression highlighter (no Q_OBJECT: only overrides a virtual). Colours
// functions/numbers/comments + variables by category, and red-underlines writes
// to read-only / constant identifiers and to conflicting (double-declared) globals.
class EelHighlighter : public QSyntaxHighlighter
{
public:
    explicit EelHighlighter(QTextDocument* doc, QSet<QString> conflicts = {})
        : QSyntaxHighlighter(doc), m_conflicts(std::move(conflicts))
    {
        m_func.setForeground(QColor(0x4F, 0xC1, 0xFF));
        m_num.setForeground(QColor(0xB5, 0xCE, 0xA8));
        m_comment.setForeground(QColor(0x6A, 0x99, 0x55));
        m_comment.setFontItalic(true);
        m_cat[static_cast<int>(SymCat::ReadOnly)].setForeground(QColor(0x9C, 0xDC, 0xFE));
        m_cat[static_cast<int>(SymCat::Input)].setForeground(QColor(0x4E, 0xC9, 0xB0));
        m_cat[static_cast<int>(SymCat::Output)].setForeground(QColor(0xDC, 0xDC, 0xAA));
        m_cat[static_cast<int>(SymCat::InOut)].setForeground(QColor(0xC5, 0x86, 0xC0));
        m_cat[static_cast<int>(SymCat::Constant)].setForeground(QColor(0x4F, 0xC1, 0xFF));
        m_cat[static_cast<int>(SymCat::Constant)].setFontItalic(true);
        m_cat[static_cast<int>(SymCat::CustomGlobal)].setForeground(QColor(0xD7, 0xBA, 0x7D));
        m_error.setForeground(QColor(0xF4, 0x47, 0x47));
        m_error.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        m_error.setUnderlineColor(QColor(0xF4, 0x47, 0x47));

        static const QStringList kFns = {
            "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "sqrt", "sqr",
            "invsqrt", "pow", "exp", "log", "abs", "sign", "min", "max", "floor",
            "ceil", "mod", "rand", "sigmoid", "if", "above", "below", "equal",
            "bnot", "band", "bor", "assign", "loop", "while", "megabuf", "gmegabuf",
            "getspec", "getosc", "gettime"};
        for (const QString& f : kFns) m_funcNames.insert(f);
        m_funcRe = QRegularExpression("\\b(" + kFns.join('|') + ")\\b");
        m_numRe = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
        m_identRe = QRegularExpression("\\b[A-Za-z_]\\w*\\b");
        m_assignRe = QRegularExpression("\\b([A-Za-z_]\\w*)\\s*=(?![=<>])");
        m_commentRe = QRegularExpression("//[^\\n]*");
    }

protected:
    void highlightBlock(const QString& text) override
    {
        // 1. Variables by category (functions handled below, skip them here).
        auto ids = m_identRe.globalMatch(text);
        while (ids.hasNext())
        {
            const QRegularExpressionMatch m = ids.next();
            const QString name = m.captured();
            if (m_funcNames.contains(name)) continue;
            const SymCat cat = symbolCategory(name);
            if (cat != SymCat::None)
                setFormat(static_cast<int>(m.capturedStart()),
                          static_cast<int>(m.capturedLength()),
                          m_cat[static_cast<int>(cat)]);
        }
        // 2. Numbers + functions.
        auto apply = [&](const QRegularExpression& re, const QTextCharFormat& fmt) {
            auto it = re.globalMatch(text);
            while (it.hasNext())
            {
                const QRegularExpressionMatch m = it.next();
                setFormat(static_cast<int>(m.capturedStart()),
                          static_cast<int>(m.capturedLength()), fmt);
            }
        };
        apply(m_numRe, m_num);
        apply(m_funcRe, m_func);
        // 3. Error: writing to a read-only / constant identifier, or re-declaring
        //    a global already initialised in another node (conflict set).
        auto as = m_assignRe.globalMatch(text);
        while (as.hasNext())
        {
            const QRegularExpressionMatch m = as.next();
            const QString name = m.captured(1);
            const SymCat cat = symbolCategory(name);
            if (cat == SymCat::ReadOnly || cat == SymCat::Constant ||
                m_conflicts.contains(name))
                setFormat(static_cast<int>(m.capturedStart(1)),
                          static_cast<int>(m.capturedLength(1)), m_error);
        }
        // 4. Comments last — they win over any token inside them.
        apply(m_commentRe, m_comment);
    }

private:
    QTextCharFormat m_func, m_num, m_comment, m_error;
    std::array<QTextCharFormat, 7> m_cat;  // indexed by SymCat
    QSet<QString> m_funcNames;
    QSet<QString> m_conflicts;  // globals double-declared elsewhere (init slot)
    QRegularExpression m_funcRe, m_numRe, m_identRe, m_assignRe, m_commentRe;
};

QString refRow(const char* name, const char* type, const char* range, const char* desc)
{
    return QStringLiteral("<tr><td><b>%1</b></td><td><i>%2</i></td><td>%3</td><td>%4</td></tr>")
        .arg(QString::fromUtf8(name), QString::fromUtf8(type), QString::fromUtf8(range),
             QString::fromUtf8(desc));
}

// One function-reference row: signature (with params) · return · description.
QString fnRow(const char* sig, const char* ret, const char* desc)
{
    return QStringLiteral("<tr><td><code>%1</code></td><td><i>%2</i></td><td>%3</td></tr>")
        .arg(QString::fromUtf8(sig), QString::fromUtf8(ret), QString::fromUtf8(desc));
}
QString fnTable(const QString& rows)
{
    return QStringLiteral(
               "<table cellspacing='0' cellpadding='4' style='border-collapse:collapse'>"
               "<tr><th align='left'>Function</th><th align='left'>Returns</th>"
               "<th align='left'>Description</th></tr>%1</table>")
        .arg(rows);
}

// The EEL built-ins shared by every scripted module (functions + constants).
QString builtinsHtml()
{
    const QString math = fnTable(
        fnRow("sin(x) · cos(x) · tan(x)", "number", "trigonometry, x in radians") +
        fnRow("asin(x) · acos(x) · atan(x)", "radians", "inverse trigonometry") +
        fnRow("atan2(y, x)", "radians [-π,π]", "angle of the vector (x,y), quadrant-correct") +
        fnRow("sqrt(x)", "number", "square root (√x)") +
        fnRow("sqr(x)", "number", "square (x·x)") +
        fnRow("invsqrt(x)", "number", "fast inverse square root (1/√x)") +
        fnRow("pow(base, exp)", "number", "base raised to exp (base^exp)") +
        fnRow("exp(x)", "number", "e^x") +
        fnRow("log(x [, base])", "number", "natural log; log(x,10) for base-10") +
        fnRow("abs(x)", "number", "absolute value |x|") +
        fnRow("sign(x)", "-1 / 0 / 1", "sign of x") +
        fnRow("min(a, b) · max(a, b)", "number", "smaller / larger of a and b") +
        fnRow("mod(a, b)", "number", "floating-point remainder of a/b (fmod)") +
        fnRow("rand(n)", "integer 0..n-1", "pseudo-random integer (deterministic, seedable)") +
        fnRow("sigmoid(x, k)", "≈ 0..1", "smooth ramp; k = steepness"));

    const QString logic = fnTable(
        fnRow("if(cond, a, b)", "a or b", "returns a if cond≠0 else b (both args evaluated)") +
        fnRow("above(a, b)", "0 / 1", "1 if a > b") +
        fnRow("below(a, b)", "0 / 1", "1 if a < b") +
        fnRow("equal(a, b)", "0 / 1", "1 if |a-b| < 1e-5 (epsilon)") +
        fnRow("band(a, b)", "0 / 1", "logical AND (a≠0 and b≠0)") +
        fnRow("bor(a, b)", "0 / 1", "logical OR") +
        fnRow("bnot(a)", "0 / 1", "logical NOT (1 if a=0)"));

    const QString mem = fnTable(
        fnRow("megabuf(i)", "number", "read module-local buffer slot i (write: megabuf(i)=v)") +
        fnRow("gmegabuf(i)", "number", "read preset-global buffer slot i (write: gmegabuf(i)=v)"));

    const QString ctrl = fnTable(
        fnRow("loop(n, expr)", "—", "evaluate expr n times") +
        fnRow("while(cond)", "—", "loop while cond≠0 (with a body block)"));

    const QString audio = fnTable(
        fnRow("getspec(band, width, ch)", "≈ 0..1",
              "spectrum energy. band 0..1 (low→high), width 0..1 (averaging window), "
              "ch 0=both 1=left 2=right") +
        fnRow("getosc(band, width, ch)", "≈ -1..1",
              "waveform (oscilloscope) sample; same band/width/ch as getspec") +
        fnRow("gettime(sc)", "seconds", "seconds since start minus sc (use gettime(0))"));

    return QStringLiteral(
               "<h3>Math</h3>%1"
               "<h3>Logic (EEL, → 0/1)</h3>%2"
               "<h3>Memory</h3>%3"
               "<h3>Control</h3>%4"
               "<h3>Audio (all modules)</h3>"
               "<p><b>Variables:</b> <code>bass</code> <code>mid</code> <code>treb</code> "
               "(≈0..1 band energy) · <code>vol</code> (level) · <code>beat</code> (0/1) · "
               "<code>time</code> (seconds)</p>%5"
               "<h3>Constants</h3>"
               "<p><code>pi</code> ≈ 3.14159 · <code>pi2</code> ≈ 6.28319 (2·pi)</p>"
               "<p style='color:#888'>All values are floating-point. Assignment: "
               "<code>x = expr;</code> — statements separated by <code>;</code>. "
               "Comments: <code>// …</code></p>")
        .arg(math).arg(logic).arg(mem).arg(ctrl).arg(audio);
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

    const QString legend = QStringLiteral(
        "<h3>Highlight colours</h3><p>"
        "<span style='color:#9CDCFE'>read-only</span> &nbsp; "
        "<span style='color:#4EC9B0'>input</span> &nbsp; "
        "<span style='color:#DCDCAA'>output</span> &nbsp; "
        "<span style='color:#C586C0'>in/out</span> &nbsp; "
        "<span style='color:#4FC1FF'><i>constant</i></span> &nbsp; "
        "<span style='color:#D7BA7D'>custom-global</span> &nbsp; "
        "custom-local &nbsp; "
        "<span style='color:#F44747'>error</span> (writing a read-only/constant)</p>");
    return QStringLiteral("<h2>%1 — script variables</h2>%2%3%4")
        .arg(QString::fromUtf8(title), vars, builtinsHtml(), legend);
}

// Read-only reference popup (resizable window).
void showScriptReference(QWidget* parent, const QString& html)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("Script reference"));
    dlg.resize(560, 540);
    auto* lay = new QVBoxLayout(&dlg);
    auto* browser = new QTextBrowser(&dlg);
    browser->setHtml(html);
    lay->addWidget(browser, 1);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(bb);
    dlg.exec();
}

// Full, resizable editor: big code pane + the module's reference side-by-side.
// Returns true and fills `out` when accepted ("expand full" / "collapse full").
bool openScriptEditor(QWidget* parent, const QString& label, const QString& text,
                      const QString& refHtml, QString& out,
                      const QSet<QString>& conflicts = {})
{
    QDialog dlg(parent);
    dlg.setWindowTitle(QObject::tr("Edit script — %1").arg(label));
    dlg.resize(920, 620);
    auto* lay = new QVBoxLayout(&dlg);
    auto* split = new QSplitter(Qt::Horizontal, &dlg);
    auto* editor = new QPlainTextEdit(split);
    editor->setPlainText(text);
    editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    QFont mono(QStringLiteral("Consolas"));
    mono.setStyleHint(QFont::Monospace);
    editor->setFont(mono);
    new EelHighlighter(editor->document(), conflicts);
    auto* ref = new QTextBrowser(split);
    ref->setHtml(refHtml);
    split->addWidget(editor);
    split->addWidget(ref);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 2);
    lay->addWidget(split, 1);
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    lay->addWidget(bb);
    if (dlg.exec() == QDialog::Accepted)
    {
        out = editor->toPlainText();
        return true;
    }
    return false;
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
    const bool haveOriginIcons = !originIconDir().isEmpty();
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
    if (!originIconDir().isEmpty())
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

    if (node.isList())
    {
        for (int i = 0; i < static_cast<int>(node.children.size()); ++i)
        {
            QList<int> childPath = path;
            childPath.append(i);
            addTreeItem(item, node.children[i], childPath);
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
    if (auto* item = m_tree->currentItem())
    {
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
    ChainNode fresh;
    fresh.params = chosen.make();

    const QList<int> sel = currentPath();
    mutateStructure([&] {
        ChainNode* target = nodeAtPath(sel);
        // Add into the selected list; otherwise append to the root list.
        if (target != nullptr && target->isList())
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

void MultiEffectPanel::applySuperScopePreset(const QList<int>& path, int presetIndex)
{
    const auto& figures = superscopeFigures();
    if (presetIndex < 0 || presetIndex >= static_cast<int>(figures.size())) return;
    if (m_host == nullptr || m_mutex == nullptr) return;

    // Reuse the standalone module's preset library (single source of truth).
    lumi::modules::SuperscopeModule tmp;
    tmp.loadPresetCode(figures[static_cast<std::size_t>(presetIndex)]);
    const std::string init = tmp.initCode();
    const std::string frame = tmp.frameCode();
    const std::string beat = tmp.beatCode();
    const std::string point = tmp.pointCode();
    const int count = tmp.pointCount();

    QMutexLocker lock(m_mutex);
    ChainNode* node = nodeAtPath(path);
    if (node == nullptr) return;
    if (auto* p = std::get_if<SuperScopeParams>(&node->params))
    {
        p->initCode = init;
        p->frameCode = frame;
        p->beatCode = beat;
        p->pointCode = point;
        p->pointCount = count;
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
            if (tnode != nullptr && tnode->isList())
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
            if (tnode != nullptr && tnode->isList())
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
        item = item->child(path[i]);
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
void MultiEffectPanel::buildPropertyEditor(const QList<int>& path)
{
    clearPropertyEditor();
    if (m_host == nullptr || path.isEmpty()) return;

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
    const QString scriptRef = scriptReferenceHtml(params);

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
    }
    else if (auto* p = std::get_if<FadeoutParams>(&params))
    {
        addInt(tr("Fade length"), p->fadeLen, 0, 92, [](ChainNode& n, int v) { std::get<FadeoutParams>(n.params).fadeLen = v; });
        addColor(tr("Target color"), p->color, [](ChainNode& n, uint32_t v) { std::get<FadeoutParams>(n.params).color = v; });
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
    }
    else if (auto* p = std::get_if<FastBrightnessParams>(&params))
    {
        addEnum(tr("Mode"), p->dir, {"x2", "x0.5", "off"}, [](ChainNode& n, int v) { std::get<FastBrightnessParams>(n.params).dir = v; });
    }
    else if (auto* p = std::get_if<BlurParams>(&params))
    {
        addEnum(tr("Strength"), p->strength - 1, {"Light", "Normal", "Heavy"},
                [](ChainNode& n, int v) { std::get<BlurParams>(n.params).strength = v + 1; });
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
    }
    else if (auto* p = std::get_if<OnBeatClearParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<OnBeatClearParams>(n.params).color = v; });
        addInt(tr("Every N beats"), p->everyNBeats, 1, 100, [](ChainNode& n, int v) { std::get<OnBeatClearParams>(n.params).everyNBeats = v; });
        addBool(tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<OnBeatClearParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<ColorfadeParams>(&params))
    {
        addInt(tr("Fader R"), p->faderR, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderR = v; });
        addInt(tr("Fader G"), p->faderG, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderG = v; });
        addInt(tr("Fader B"), p->faderB, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderB = v; });
        addInt(tr("Beat Fader R"), p->beatFaderR, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderR = v; });
        addInt(tr("Beat Fader G"), p->beatFaderG, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderG = v; });
        addInt(tr("Beat Fader B"), p->beatFaderB, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderB = v; });
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
        addScript(tr("Point code"), p->code, [](ChainNode& n, std::string v) { std::get<MovementParams>(n.params).code = std::move(v); });
    }
    else if (auto* p = std::get_if<DynamicMovementParams>(&params))
    {
        addInt(tr("Grid X"), p->xres, 2, 96, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).xres = v; });
        addInt(tr("Grid Y"), p->yres, 2, 72, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).yres = v; });
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
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50", "Line"}, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<ColorMapParams>(&params))
    {
        addEnum(tr("Input"), p->key, {"Red", "Green", "Blue", "(R+G+B)/2", "Max", "(R+G+B)/3"}, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).key = v; });
        addEnum(tr("Blend"), p->blendMode, {"Replace", "Additive", "Maximum", "Minimum", "50/50", "Sub d-s", "Sub s-d", "Multiply", "XOR", "Adjustable"}, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).blendMode = v; });
        addInt(tr("Adjustable"), p->adjustBlend, 0, 255, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).adjustBlend = v; });
        auto* info = new QLabel(tr("%1 gradient stops (imported, read-only)").arg(p->stopPos.size()), m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<BufferBlendParams>(&params))
    {
        const QStringList bufs{"Buffer 1", "Buffer 2", "Buffer 3", "Buffer 4", "Buffer 5", "Buffer 6", "Buffer 7", "Buffer 8", "Current"};
        addEnum(tr("Buffer A"), p->bufferA, bufs, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).bufferA = v; });
        addEnum(tr("Buffer B"), p->bufferB, bufs, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).bufferB = v; });
        addEnum(tr("Mode"), p->mode, {"Replace", "Additive", "Maximum", "50/50", "Sub A-B", "Sub B-A", "Multiply", "Adjustable", "XOR", "Minimum", "Abs diff"}, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).mode = v; });
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
    }
    else if (auto* p = std::get_if<UniqueToneParams>(&params))
    {
        addColor(tr("Tone"), p->color, [](ChainNode& n, uint32_t v) { std::get<UniqueToneParams>(n.params).color = v; });
        addBool(tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<UniqueToneParams>(n.params).invert = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<UniqueToneParams>(n.params).blend = v; });
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
    }
    else if (auto* p = std::get_if<ConvolutionParams>(&params))
    {
        addEnum(tr("Edge"), p->edgeMode, {"Extend", "Wrap"}, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).edgeMode = v; });
        addBool(tr("Absolute"), p->absolute, [](ChainNode& n, bool v) { std::get<ConvolutionParams>(n.params).absolute = v; });
        addBool(tr("Two pass"), p->twoPass, [](ChainNode& n, bool v) { std::get<ConvolutionParams>(n.params).twoPass = v; });
        addInt(tr("Bias"), p->bias, -255, 255, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).bias = v; });
        addInt(tr("Scale"), p->scale, 1, 1000, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).scale = v; });
        auto* info = new QLabel(tr("7×7 kernel (imported, read-only)"), m_propContainer);
        form->addRow(info);
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
    }
    else if (auto* p = std::get_if<AddBordersParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<AddBordersParams>(n.params).color = v; });
        addInt(tr("Size"), p->size, 0, 200, [](ChainNode& n, int v) { std::get<AddBordersParams>(n.params).size = v; });
    }
    else if (auto* p = std::get_if<SimpleScopeParams>(&params))
    {
        addEnum(tr("Source"), p->source, {"Spectrum", "Waveform"}, [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).source = v; });
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).channel = v; });
        addEnum(tr("Position"), p->position, {"Top", "Bottom", "Center"}, [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).position = v; });
        addEnum(tr("Draw"), p->drawMode, {"Lines", "Dots"}, [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).drawMode = v; });
        auto* info = new QLabel(tr("%1 cycled colors (imported)").arg(p->colors.size()), m_propContainer);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<BassSpinParams>(&params))
    {
        addBool(tr("Left"), p->left, [](ChainNode& n, bool v) { std::get<BassSpinParams>(n.params).left = v; });
        addBool(tr("Right"), p->right, [](ChainNode& n, bool v) { std::get<BassSpinParams>(n.params).right = v; });
        addColor(tr("Left color"), p->colorLeft, [](ChainNode& n, uint32_t v) { std::get<BassSpinParams>(n.params).colorLeft = v; });
        addColor(tr("Right color"), p->colorRight, [](ChainNode& n, uint32_t v) { std::get<BassSpinParams>(n.params).colorRight = v; });
        addEnum(tr("Mode"), p->mode, {"Lines", "Filled"}, [](ChainNode& n, int v) { std::get<BassSpinParams>(n.params).mode = v; });
    }
    else if (auto* p = std::get_if<OscStarParams>(&params))
    {
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).channel = v; });
        addEnum(tr("Position"), p->position, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).position = v; });
        addInt(tr("Size"), p->size, 0, 16, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).size = v; });
        addInt(tr("Rotation"), p->rot, 0, 16, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).rot = v; });
    }
    else if (auto* p = std::get_if<OscRingParams>(&params))
    {
        addEnum(tr("Source"), p->source, {"Waveform", "Spectrum"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).source = v; });
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).channel = v; });
        addEnum(tr("Position"), p->position, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).position = v; });
        addInt(tr("Size"), p->size, 0, 16, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).size = v; });
    }
    else if (std::holds_alternative<RotatingStarsParams>(params))
    {
        auto* info = new QLabel(tr("Rotating stars — audio-reactive, cycled colors"), m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<PictureParams>(&params))
    {
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<PictureParams>(n.params).blend = v; });
        addBool(tr("Keep aspect"), p->keepAspect, [](ChainNode& n, bool v) { std::get<PictureParams>(n.params).keepAspect = v; });
        const QString status = p->imageData.empty()
            ? tr("⚠ image not embedded: %1").arg(QString::fromStdString(p->filename))
            : tr("✓ image embedded (%1 KB)").arg(static_cast<int>(p->imageData.size() * 3 / 4 / 1024));
        auto* info = new QLabel(status, m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<PictureIIParams>(&params))
    {
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<PictureIIParams>(n.params).blend = v; });
        const QString status = p->imageData.empty()
            ? tr("⚠ image not embedded: %1").arg(QString::fromStdString(p->filename))
            : tr("✓ image embedded (%1 KB)").arg(static_cast<int>(p->imageData.size() * 3 / 4 / 1024));
        auto* info = new QLabel(status, m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<TexerParams>(&params))
    {
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive"}, [](ChainNode& n, int v) { std::get<TexerParams>(n.params).blend = v; });
        addInt(tr("Particles"), p->particles, 1, 4096, [](ChainNode& n, int v) { std::get<TexerParams>(n.params).particles = v; });
        const QString status = p->imageData.empty() ? tr("⚠ image not embedded: %1").arg(QString::fromStdString(p->filename)) : tr("✓ image embedded");
        auto* info = new QLabel(status, m_propContainer); info->setWordWrap(true); form->addRow(info);
    }
    else if (auto* p = std::get_if<TexerIIParams>(&params))
    {
        addBool(tr("Color filtering"), p->colorFiltering, [](ChainNode& n, bool v) { std::get<TexerIIParams>(n.params).colorFiltering = v; });
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Point (x,y,sizex,sizey,rgb)"), p->pointCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).pointCode = std::move(v); });
        const QString status = p->imageData.empty() ? tr("⚠ image not embedded: %1").arg(QString::fromStdString(p->filename)) : tr("✓ image embedded");
        auto* info = new QLabel(status, m_propContainer); info->setWordWrap(true); form->addRow(info);
    }
    else if (auto* p = std::get_if<TriangleParams>(&params))
    {
        addScript(tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).initCode = std::move(v); });
        addScript(tr("Frame (set n)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).frameCode = std::move(v); });
        addScript(tr("Point (x1..y3,rgb)"), p->pointCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BlitterFeedbackParams>(&params))
    {
        addDouble(tr("Zoom"), p->zoom, 0.5, 2.0, 0.01, [](ChainNode& n, double v) { std::get<BlitterFeedbackParams>(n.params).zoom = static_cast<float>(v); });
        addDouble(tr("Beat zoom"), p->beatZoom, 0.5, 2.0, 0.01, [](ChainNode& n, double v) { std::get<BlitterFeedbackParams>(n.params).beatZoom = static_cast<float>(v); });
        addBool(tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).onBeat = v; });
        addBool(tr("Blend"), p->blend, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<RotoBlitterParams>(&params))
    {
        addDouble(tr("Zoom"), p->zoom, 0.5, 2.0, 0.01, [](ChainNode& n, double v) { std::get<RotoBlitterParams>(n.params).zoom = static_cast<float>(v); });
        addDouble(tr("Rotation/frame"), p->rotationSpeed, -10.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<RotoBlitterParams>(n.params).rotationSpeed = static_cast<float>(v); });
        addBool(tr("Blend"), p->blend, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).blend = v; });
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
    }
    else if (auto* p = std::get_if<CustomBpmParams>(&params))
    {
        addBool(tr("Arbitrary"), p->arbitrary, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).arbitrary = v; });
        addInt(tr("Interval (ms)"), p->arbitraryMs, 1, 5000, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).arbitraryMs = v; });
        addBool(tr("Skip"), p->skip, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).skip = v; });
        addInt(tr("Skip count"), p->skipCount, 1, 16, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).skipCount = v; });
        addBool(tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).invert = v; });
    }
    else if (auto* p = std::get_if<SuperScopeParams>(&params))
    {
        // Figure preset dropdown: loads the shape's EEL into the code fields
        // below (source: SuperscopeModule::loadPresetCode). Index 0 is a label.
        auto* presetCombo = new QComboBox(m_propContainer);
        presetCombo->addItem(tr("— Load figure preset —"));
        for (lumi::modules::SuperscopePreset fig : superscopeFigures())
            presetCombo->addItem(lumi::modules::SuperscopeModule::presetName(fig));
        connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this, path](int idx) {
                    if (idx <= 0) return;  // the label row
                    // Defer: applying rebuilds this editor (deletes the combo).
                    QMetaObject::invokeMethod(
                        this,
                        [this, path, idx] {
                            applySuperScopePreset(path, idx - 1);
                            buildPropertyEditor(path);
                        },
                        Qt::QueuedConnection);
                });
        form->addRow(tr("Figure"), presetCombo);

        addInt(tr("Point count"), p->pointCount, 1, 4096, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).pointCount = v; });
        addEnum(tr("Draw mode"), p->renderMode, {"Dots", "Lines", "Thick"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).renderMode = v; });
        addDouble(tr("Line width"), p->lineWidth, 1.0, 20.0, 0.5, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).lineWidth = static_cast<float>(v); });
        addDouble(tr("Dot size"), p->dotSize, 1.0, 50.0, 1.0, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum(tr("Channel"), p->audioChannel, {"Left", "Right", "Mono", "Mid", "Side"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).audioChannel = v; });
        addEnum(tr("Line blend"), p->lineBlend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).lineBlend = v; });

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
        {
            // AVS color table: cycled over time in the "Color table"/blend modes.
            // One swatch per entry + add/remove.
            auto* colorsWidget = new QWidget(m_propContainer);
            auto* row = new QHBoxLayout(colorsWidget);
            row->setContentsMargins(0, 0, 0, 0);
            for (int ci = 0; ci < static_cast<int>(p->colors.size()); ++ci)
            {
                const uint32_t initC = p->colors[static_cast<size_t>(ci)];
                auto* sw = new QPushButton(colorsWidget);
                sw->setFixedSize(24, 20);
                sw->setStyleSheet(QString("background:%1").arg(colorFromU32(initC).name()));
                connect(sw, &QPushButton::clicked, this, [this, path, ci, initC, sw]() {
                    const QColor picked = QColorDialog::getColor(colorFromU32(initC), this);
                    if (!picked.isValid()) return;
                    const uint32_t c = u32FromColor(picked);
                    sw->setStyleSheet(QString("background:%1").arg(picked.name()));
                    mutate(path, [&](ChainNode& n) {
                        auto& cs = std::get<SuperScopeParams>(n.params).colors;
                        if (ci < static_cast<int>(cs.size())) cs[static_cast<size_t>(ci)] = c;
                    });
                });
                row->addWidget(sw);
            }
            auto* addC = new QToolButton(colorsWidget);
            addC->setText("+");
            addC->setToolTip(tr("Add color"));
            connect(addC, &QToolButton::clicked, this, [this, path]() {
                mutate(path, [](ChainNode& n) {
                    std::get<SuperScopeParams>(n.params).colors.push_back(0xFFFFFF);
                });
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
            });
            auto* delC = new QToolButton(colorsWidget);
            delC->setText(QString::fromUtf8("−"));
            delC->setToolTip(tr("Remove last color"));
            connect(delC, &QToolButton::clicked, this, [this, path]() {
                mutate(path, [](ChainNode& n) {
                    auto& cs = std::get<SuperScopeParams>(n.params).colors;
                    if (!cs.empty()) cs.pop_back();
                });
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
            });
            row->addWidget(addC);
            row->addWidget(delC);
            row->addStretch();
            colorsWidget->setToolTip(
                tr("AVS color table — the scope cycles through these over time in "
                   "the Color table / blend modes (one step every 'Cycle frames')."));
            form->addRow(tr("Color table"), colorsWidget);
        }

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
    }
    else if (auto* p = std::get_if<GrainParams>(&params))
    {
        addInt(tr("Amount"), p->amount, 0, 100, [](ChainNode& n, int v) { std::get<GrainParams>(n.params).amount = v; });
        addBool(tr("Static"), p->staticGrain, [](ChainNode& n, bool v) { std::get<GrainParams>(n.params).staticGrain = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<GrainParams>(n.params).blend = v; });
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
    }
    else if (auto* p = std::get_if<FyrewurXParams>(&params))
    {
        addInt(tr("Sparks per burst"), p->sparks, 1, 1024, [](ChainNode& n, int v) { std::get<FyrewurXParams>(n.params).sparks = v; });
        addDouble(tr("Speed"), p->speed, 0.05, 5.0, 0.05, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).speed = static_cast<float>(v); });
        addDouble(tr("Gravity"), p->gravity, 0.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).gravity = static_cast<float>(v); });
        addDouble(tr("Life (s)"), p->lifeSeconds, 0.1, 10.0, 0.1, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).lifeSeconds = static_cast<float>(v); });
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
    }
    else if (auto* p = std::get_if<TimescopeParams>(&params))
    {
        addColor(tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<TimescopeParams>(n.params).color = v; });
        addInt(tr("Bands"), p->bands, 1, 576, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).bands = v; });
        addEnum(tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).channel = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<DotGridParams>(&params))
    {
        addInt(tr("Spacing"), p->spacing, 2, 128, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).spacing = v; });
        addInt(tr("X move"), p->xMove, -1024, 1024, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).xMove = v; });
        addInt(tr("Y move"), p->yMove, -1024, 1024, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).yMove = v; });
        addEnum(tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).blend = v; });
        if (!p->colors.empty())
            addColor(tr("Color 1"), p->colors[0], [](ChainNode& n, uint32_t v) { std::get<DotGridParams>(n.params).colors[0] = v; });
    }
    else if (auto* p = std::get_if<DotPlaneParams>(&params))
    {
        addInt(tr("Rotation speed"), p->rotVel, -50, 50, [](ChainNode& n, int v) { std::get<DotPlaneParams>(n.params).rotVel = v; });
        addInt(tr("Angle"), p->angle, -90, 90, [](ChainNode& n, int v) { std::get<DotPlaneParams>(n.params).angle = v; });
        for (int i = 0; i < 5; ++i)
            addColor(tr("Color %1").arg(i + 1), p->colors[i], [i](ChainNode& n, uint32_t v) { std::get<DotPlaneParams>(n.params).colors[i] = v; });
    }
    else if (auto* p = std::get_if<DotFountainParams>(&params))
    {
        addInt(tr("Rotation speed"), p->rotVel, -50, 50, [](ChainNode& n, int v) { std::get<DotFountainParams>(n.params).rotVel = v; });
        addInt(tr("Angle"), p->angle, -90, 90, [](ChainNode& n, int v) { std::get<DotFountainParams>(n.params).angle = v; });
        for (int i = 0; i < 5; ++i)
            addColor(tr("Color %1").arg(i + 1), p->colors[i], [i](ChainNode& n, uint32_t v) { std::get<DotFountainParams>(n.params).colors[i] = v; });
    }
    else if (auto* p = std::get_if<ChannelShiftParams>(&params))
    {
        addEnum(tr("Mode"), p->mode, {"RGB", "RBG", "GBR", "GRB", "BRG", "BGR"}, [](ChainNode& n, int v) { std::get<ChannelShiftParams>(n.params).mode = v; });
        addBool(tr("On beat (random)"), p->onBeat, [](ChainNode& n, bool v) { std::get<ChannelShiftParams>(n.params).onBeat = v; });
    }
    else if (auto* p = std::get_if<ColorReductionParams>(&params))
    {
        addInt(tr("Levels (bits)"), p->levels, 1, 8, [](ChainNode& n, int v) { std::get<ColorReductionParams>(n.params).levels = v; });
    }
    else if (auto* p = std::get_if<MultiplierParams>(&params))
    {
        addEnum(tr("Factor"), p->mode, {"Saturate", "x8", "x4", "x2", "x0.5", "x0.25", "x0.125", "Keep"}, [](ChainNode& n, int v) { std::get<MultiplierParams>(n.params).mode = v; });
    }
    else if (auto* p = std::get_if<VideoDelayParams>(&params))
    {
        addInt(tr("Delay"), p->delay, 1, 128, [](ChainNode& n, int v) { std::get<VideoDelayParams>(n.params).delay = v; });
        addBool(tr("In beats"), p->useBeats, [](ChainNode& n, bool v) { std::get<VideoDelayParams>(n.params).useBeats = v; });
    }
    else if (auto* p = std::get_if<MultiDelayParams>(&params))
    {
        addEnum(tr("Mode"), p->mode, {"None", "Input (write)", "Output (read)"}, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).mode = v; });
        addInt(tr("Buffer"), p->buffer, 0, 5, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).buffer = v; });
        addInt(tr("Delay"), p->delay, 1, 128, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).delay = v; });
        addBool(tr("In beats"), p->useBeats, [](ChainNode& n, bool v) { std::get<MultiDelayParams>(n.params).useBeats = v; });
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
        addEnum(tr("Line blend"), p->lineBlend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<SetRenderModeParams>(n.params).lineBlend = v; });
        addInt(tr("Adjustable alpha"), p->adjustAlpha, 0, 255, [](ChainNode& n, int v) { std::get<SetRenderModeParams>(n.params).adjustAlpha = v; });
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
