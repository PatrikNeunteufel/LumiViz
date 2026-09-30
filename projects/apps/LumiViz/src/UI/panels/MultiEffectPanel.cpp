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
#include "UI/panels/EffectPalette.hpp"                    // Palettentabelle (S79)
#include "UI/panels/EffectPaletteWindow.hpp"              // Palettenfenster (S79)
#include "UI/panels/FieldDocs.hpp"                       // Tooltip je Feld (§10)
#include "UI/panels/ParameterBaum.hpp"                   // generischer Parameter-Baum (S72)
#include "UI/widgets/DateiDrag.hpp"                       // Preset-Dateien im Drag (S84)
#include "UI/widgets/GradientPresetDelegate.hpp"          // gradient combo previews
#include "UI/widgets/VisualizerWidget.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/modules/SuperscopeModule.hpp"      // figure preset library (SSOT)
#include "visualizers/modules/ColorGradientModule.hpp"   // fractal palette presets
#include "visualizers/multieffect/ChainSerializer.hpp"  // effectTypeKey
#include "visualizers/milkdrop/MilkdropSerializer.hpp"    // Element-Presets (S83)
#include "visualizers/multieffect/NodePresetStore.hpp"  // Voreinstellungen je Knoten
#include "visualizers/multieffect/ShadertoyImport.hpp"  // URL-/ID-Import (Strang S3)
#include "visualizers/multieffect/ShadertoyWrapper.hpp" // Starter-Shader (Strang S)
#include "visualizers/multieffect/MeshWarpWrapper.hpp"      // Starter-Warp + Klemmen (G1)
#include "visualizers/multieffect/GpuParticlesWrapper.hpp"  // Starter-Kraftfeld + Klemmen (G2)
#include "visualizers/multieffect/PixelFilterWrapper.hpp"   // Starter-Filter (Stilfilter, S70)
#include "visualizers/multieffect/IsfImport.hpp"            // ISF-Filter-Import (S72)
#include "visualizers/multieffect/IsfFilterWrapper.hpp"     // Starter-Shader ISF (S72)
#include "visualizers/multieffect/ScriptVariables.hpp"      // Variablenreferenz (SSOT, S79)
#include "scripting/ScriptFormatter.hpp"                // Beautify-Kerne (S69)
#include <EelTranspiler.hpp>                            // Apply-Syntaxprobe EEL (S69)
#include <HlslTranspiler.hpp>                           // Apply-Syntaxprobe HLSL (S69)
#include "services/IEventBus.hpp"
#include "services/LiveVideoFeed.hpp"                   // videoSource: Kamera/Testaufnahmen (S70)
#include "services/events/UIEvents.hpp"
#include "UI/widgets/PresetTypeIcons.hpp"

#include <QCameraDevice>                                // videoSource: Geraeteliste (S70)
#include <QMediaDevices>

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QDropEvent>
#include <QMimeData>
#include <QComboBox>
#include <QSlider>
#include <QDesktopServices>
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
#include <QSignalBlocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QPushButton>
#include <BasicLogger.h>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <array>
#include <type_traits>
#include <variant>
#include <utility>

using namespace lumi::multieffect;
using lumi::ui::EffectType;
using lumi::ui::MilkElement;
using lumi::ui::Origin;
using lumi::ui::effectPalette;
using lumi::ui::originForParams;
using lumi::ui::originIcon;
using lumi::ui::originToolTip;

namespace {

// --- Breiten-Vertrag des Panels (S78, Vorgabe Patrik) ---------------------
// Jedes Feld hat mindestens kFeldMinBreite, keines wird breiter als das Panel,
// und zum Panelrand bleiben mindestens kPanelRand. Doku: MultiEffectPanel.md.
constexpr int kFeldMinBreite = 120;   ///< Mindestbreite jedes Editor-Felds
constexpr int kPanelRand = 5;         ///< Abstand Inhalt <-> Panelrand
/// Schmalste Dock-Stellung: Inhaltsbreite ohne Raender/Balken (etwa doppelte
/// Feldbreite, Vorgabe Patrik S78 — Beschriftung + Feld nebeneinander).
constexpr int kPanelMinInhalt = 2 * kFeldMinBreite;
constexpr int kSeitenInnenrand = 4;   ///< Luft zwischen Scroll-Rahmen und Feldern
constexpr int kComboZeichen = 3;      ///< Combo-Mindestbreite in Zeichen (Zeilen aus
                                      ///< Combo + Knoepfen muessen auf 120 px passen)
constexpr int kBaumNameMin = 120;     ///< Effektliste: Spalte Name
constexpr int kBaumTypMin = 90;       ///< Effektliste: Spalte Typ
constexpr int kBaumBeschreibungMin = 80;  ///< Effektliste: Spalte Beschreibung

/// Combo nicht nach ihrem laengsten Eintrag bemessen (Qt-Vorgabe), sondern
/// nach kComboZeichen — Dateinamen, Geraete-IDs oder Preset-Namen trieben die
/// Seite sonst beliebig breit. Die aufgeklappte Liste zeigt die Eintraege
/// trotzdem voll: sie darf breiter als die Combo sein.
void begrenzeCombo(QComboBox* combo)
{
    combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    combo->setMinimumContentsLength(kComboZeichen);
    QAbstractItemView* view = combo->view();
    view->setMinimumWidth(view->sizeHintForColumn(0) +
                          view->verticalScrollBar()->sizeHint().width());
}


/// Milkdrop-Anzeige-Kinder (E1: Preset → Code · Waves · Shapes · Shader ·
/// Sprites): Pfad-Segmente >= dieser Basis sind SEKTIONEN des Eltern-Nodes,
/// keine Chain-Indizes — nodeAtPath liefert fuer sie bewusst nullptr
/// (Mutations-Schutz). Auf ein Sektions-Segment darf ein ELEMENT-Index folgen
/// (N3.1/N3.3): [.., kMilkSectionBase+s, i] = Element i der Sektion s.
constexpr int kMilkSectionBase = 1000000;

/// Baum-Rolle mit der nodeId eines Ketten-Items (S82) — Schluessel fuer den
/// Aufklapp-Zustand ueber einen Neuaufbau hinweg (Pfade verschieben sich).
constexpr int kNodeIdRole = Qt::UserRole + 20;

/// Element-Sentinel unter der Waves-Sektion: die BASIS-Waveform des Presets
/// (nWaveMode/fWaveAlpha/…). Sie existiert immer genau einmal und ist KEIN
/// Index in preset.waves — nicht loesch-/klonbar (Sichttest-Befund S43:
/// sichtbar gerenderte Welle muss im Editor auffindbar sein).
constexpr int kMilkBasisWaveElement = kMilkSectionBase - 1;

/// Baum-Pfad in Chain-Pfad + Milk-Sektion/-Element zerlegen (Sentinels).
struct MilkPathInfo
{
    QList<int> nodePath;  ///< Pfad zum Chain-Node (ohne Sentinels)
    /// 0=Code 1=Waves 2=Shapes 3=Shader-Warp 4=Sprites 5=Parameter
    /// 6=Shader-Comp (S66: Warp/Comp getrennt; Baum zeigt Pipeline-Folge)
    int section = -1;
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

// Ebenso die Vorlagen fuer SuperScope 3D + 3D Camera (S48): seit S83 in
// `asset/nodepresets/superScope3d/` und `.../camera3d/` als Teil-Voreinstellungen
// (nur die Skripte, beim Scope dazu `pointCount`; Entscheid Patrik E3a).

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
using lumi::scriptedit::slotScopeHtml;

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


/// Kategorien der Skriptvariablen DIESES Moduls fuer den Highlighter (Block F,
/// F5, S84) — aus derselben Referenz wie die ⓘ-Seite. Milkdrop bleibt global:
/// dort hat jede Sektion ihren eigenen Vertrag (b = Blau im Wave-Code).
QHash<QString, lumi::scriptedit::SymCat> modulKategorien(const EffectParams& params)
{
    using lumi::scriptedit::SymCat;
    QHash<QString, SymCat> out;
    if (std::holds_alternative<MilkdropNodeParams>(params)) return out;
    const auto* table = lumi::multieffect::scriptVariables(params);
    if (table == nullptr) return out;
    const auto aufnehmen = [&out](const std::vector<lumi::multieffect::ScriptVar>& rows) {
        for (const auto& v : rows)
        {
            const std::string dir(v.dir);
            const SymCat cat = dir == "in"       ? SymCat::Input
                               : dir == "out"    ? SymCat::Output
                               : dir == "in/out" ? SymCat::InOut
                                                 : SymCat::None;
            if (cat == SymCat::None) continue;  // "own": gehoert dem Skript
            for (const std::string& n : lumi::multieffect::splitScriptVarNames(v.names))
            {
                const QString name = QString::fromStdString(n);
                // Schreibschutz der globalen Tabelle (w, h, dt) bleibt bestehen
                if (lumi::scriptedit::symbolCategory(name) == SymCat::ReadOnly) continue;
                out.insert(name, cat);
            }
        }
    };
    aufnehmen(table->vars);
    aufnehmen(lumi::multieffect::scriptVarBaseRows(table->base));
    return out;
}

// Module-specific variable table — rendered from the SSOT in
// ScriptVariables.hpp (S79; vorher eine handgeschriebene Kette, die fuer rund
// vierzig Knoten "No dedicated variable reference" zeigte).
QString scriptReferenceHtml(const EffectParams& params)
{
    const char* title = effectTypeName(params);
    QString vars;
    const QString typeKey = lumi::multieffect::effectTypeKey(params);
    auto rowsOf = [&typeKey](const std::vector<lumi::multieffect::ScriptVar>& list) {
        QString rows;
        for (const auto& v : list)
        {
            if (v.feld == nullptr)
            {
                rows += refRow(v.names, v.dir, v.range, v.meaning);
                continue;
            }
            // Block F (F5, S84): aus der Feldtabelle erzeugte Zeile — Text aus
            // dem Feld-Tooltip (erster Satz), Backticks als Code gesetzt.
            static const QRegularExpression kBacktick(QStringLiteral("`([^`]*)`"));
            QString text = QString::fromUtf8(v.meaning).toHtmlEscaped();
            text.replace(kBacktick, QStringLiteral("<code>\\1</code>"));
            QString tip = lumi::multieffect::fielddocs::tooltip(typeKey, QString::fromUtf8(v.feld));
            const int ende = tip.indexOf(QStringLiteral(". "));
            if (ende > 0) tip = tip.left(ende + 1);
            if (!tip.isEmpty())
                text = tip.toHtmlEscaped().replace(kBacktick, QStringLiteral("<code>\\1</code>")) +
                       QStringLiteral("<br><small>") + text + QStringLiteral("</small>");
            rows += QStringLiteral("<tr><td><b>%1</b></td><td><i>%2</i></td><td>%3</td><td>%4</td></tr>")
                        .arg(QString::fromUtf8(v.names), QString::fromUtf8(v.dir),
                             QString::fromUtf8(v.range), text);
        }
        return rows;
    };
    if (const auto* table = lumi::multieffect::scriptVariables(params))
    {
        vars = QStringLiteral(
                   "<table cellspacing='0' cellpadding='4' "
                   "style='border-collapse:collapse'>"
                   "<tr><th align='left'>Name</th><th align='left'>Type</th>"
                   "<th align='left'>Range</th><th align='left'>Meaning</th></tr>%1</table>")
                   .arg(rowsOf(table->vars) +
                        rowsOf(lumi::multieffect::scriptVarBaseRows(table->base)));
    }
    else
    {
        vars = QStringLiteral("<p><i>This module has no script slots.</i></p>");
    }

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
                refRow("drift_lin / drift_dir", "in/out", "px/frame / rad",
                       "LumiViz extra (Styling): linear drift strength / direction "
                       "(0 = right, π/2 = up). Acts only while its switch is on") +
                refRow("drift_rad / drift_ang / drift_cx / drift_cy", "in/out",
                       "px/frame / rad / 0..1", "LumiViz extra: radial drift strength, "
                       "angle (0 = out, π/2 = swirl ↺, π = in), centre (y: 0 = bottom)") +
                refRow("weich", "in/out", "0..1", "LumiViz extra: per-frame softening") +
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

/// Referenz des Shadertoy-Nodes (Strang S): Uniform-Satz, Audio-Textur-Layout,
/// Buffer-Semantik, LumiViz-Extras — die ⓘ-Seite der GLSL-Editoren.
QString shadertoyReferenceHtml()
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
    const QString uniforms = table(
        row("iResolution", "vec3", "resolution in pixels (x, y, 1)") +
        row("iTime", "float", "seconds since start (deterministic sim clock)") +
        row("iTimeDelta", "float", "seconds since the last frame") +
        row("iFrame", "int", "frames since (re)compilation") +
        row("iFrameRate", "float", "current frame rate") +
        row("iMouse", "vec4", "always 0 (no mouse in the node)") +
        row("iDate", "vec4", "deterministic: (2026, 1, 1, sim seconds)") +
        row("iSampleRate", "float", "44100") +
        row("iChannel0..3", "sampler2D", "channel bindings as set in the combos (see below)") +
        row("iChannelResolution[4]", "vec3", "resolution per channel (audio: 512x2)") +
        row("iChannelTime[4]", "float", "always 0 (video inputs come later)"));
    const QString audio = table(
        row("texture(iChannelN, vec2(f, 0.25)).x", "float 0..1",
            "FFT spectrum: f 0..1 = low..high (row 0 of the 512x2 texture)") +
        row("texture(iChannelN, vec2(t, 0.75)).x", "float 0..1",
            "waveform: 0.5 = zero line (row 1); t = sample position") +
        row("bass, mid, treb", "float", "LumiViz extra: band loudness (~0..1)") +
        row("vol, beat", "float", "LumiViz extra: level · beat pulse (0/1)"));
    return QStringLiteral(
               "<h2>Shadertoy — mainImage(out vec4, in vec2)</h2>"
               "<p>GLSL (ES 3.0 compatible). <code>fragCoord</code> = pixel from "
               "bottom left; <code>fragCoord / iResolution.xy</code> = 0..1 UV. "
               "The alpha value is ignored in the image pass (the node's blend "
               "control applies); in buffer passes it is kept (RGBA32F).</p>"
               "<h3>Uniforms</h3>%1"
               "<h3>Audio (512×2 texture on the selected channel)</h3>%2"
               "<h3>Buffer A–D (multipass)</h3>"
               "<p>Buffers are the node's own render targets (RGBA32F, "
               "chain resolution), rendered every frame BEFORE the image pass in "
               "the order A→D. A channel pointing at its OWN or a LATER "
               "buffer reads the image of the PREVIOUS frame (feedback: "
               "trails, simulations, histories); a channel pointing at an "
               "EARLIER buffer reads that buffer's fresh image of this frame. "
               "Buffers that do not exist deliver black. When publishing "
               "on Shadertoy this matches the buffer tabs exactly.</p>"
               "<p style='color:#888'>LumiViz extras (bass/mid/treb/vol/beat) "
               "do NOT run on shadertoy.com — for portable shaders use only the "
               "audio texture.</p>")
        .arg(uniforms, audio);
}

/// Referenz des GPU-Partikel-Nodes (Strang G2, S69): kraft()-Vertrag +
/// Uniforms + Determinismus-Notiz — die ⓘ-Seite des Kraftfeld-Editors.
QString gpuParticlesReferenceHtml()
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
    const QString uniforms = table(
        row("uResolution", "vec2", "chain resolution in pixels") +
        row("uTime", "float", "seconds since start (deterministic sim clock)") +
        row("uDelta", "float", "seconds since the last frame") +
        row("bass, mid, treb", "float", "band loudness (~0..1)") +
        row("vol", "float", "overall level") +
        row("beat", "float", "beat pulse (0/1)"));
    return QStringLiteral(
               "<h2>GPU Particles — vec2 kraft(vec2 pos, vec2 vel, float alter)</h2>"
               "<p>RENDER module (draws onto the chain image): tens of thousands "
               "of particles via instancing — the modern answer to the "
               "searchlight class. The optional GLSL force field returns an "
               "ADDITIONAL acceleration in UV/s² (on top of gravity and drag); "
               "<code>pos</code>/<code>vel</code> in UV and UV/s respectively, "
               "<code>alter</code> (age) runs 0..1 over the lifetime. Empty = "
               "gravity/drag only.</p>"
               "<h3>Uniforms</h3>%1"
               "<h3>Life cycle</h3>"
               "<p>Age and respawn are HASH-based without memory: every "
               "particle has a fixed phase and lifetime (±spread); at the end "
               "of its cycle it starts fresh at the spawn point — with a new "
               "random direction from the hash of the cycle. With the sim clock "
               "the run is fully reproducible (no rand()).</p>"
               "<h3>Controls</h3>"
               "<p><b>Count</b> rebuilds the state (fresh start). "
               "<b>Direction/Fan</b>: 0° = right, 90° = up; fan 360° "
               "= radial. <b>Drag</b> brakes exponentially. The "
               "parameter scripts (Init/Frame/Beat) can compute all motion "
               "controls per frame (<code>spawnx</code>, "
               "<code>speed</code>, <code>dir</code>, …).</p>")
        .arg(uniforms);
}

/// Referenz des Mesh-Warp-Nodes (Strang G1, S69): warp()-Vertrag + Uniforms —
/// die ⓘ-Seite des GLSL-Editors.
QString meshWarpReferenceHtml()
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
    const QString uniforms = table(
        row("uResolution", "vec2", "chain resolution in pixels") +
        row("uTime", "float", "seconds since start (deterministic sim clock)") +
        row("uDelta", "float", "seconds since the last frame") +
        row("uFrame", "int", "frames since (re)compilation") +
        row("bass, mid, treb", "float", "band loudness (~0..1)") +
        row("vol", "float", "overall level") +
        row("beat", "float", "beat pulse (0/1)"));
    return QStringLiteral(
               "<h2>Mesh Warp — vec2 warp(vec2 uv)</h2>"
               "<p>GLSL function that runs per GRID VERTEX in the vertex shader "
               "(the GPU answer to the Movement class — modern rule set). "
               "<code>uv</code> is the grid position 0..1; the return value is "
               "the SOURCE UV at which the current chain image is sampled "
               "(MilkDrop warp semantics: the grid stays, the source "
               "moves). Identity <code>return uv;</code> = passthrough.</p>"
               "<h3>Uniforms</h3>%1"
               "<h3>Controls</h3>"
               "<p><b>Grid X/Y</b>: resolution of the warp grid — between "
               "the vertices values are interpolated linearly; coarse grids make "
               "the warp angular (usable as a stylistic device). <b>Mix</b>: share "
               "of the warped image (1 = replace). <b>Wrap</b>: repeat source UVs "
               "outside 0..1 instead of clamping.</p>"
               "<p style='color:#888'>Without Clear/Decay in the chain the warp "
               "feeds back on its own previous image (feedback) — intended for trails, "
               "otherwise put a Clear/Fadeout node in front.</p>")
        .arg(uniforms);
}

/// Referenz des Pixel-Filter-Nodes (Stilfilter-Strang, S70): filter()-Vertrag
/// + Uniforms — die ⓘ-Seite des GLSL-Editors.
QString pixelFilterReferenceHtml()
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
    const QString uniforms = table(
        row("uTex", "sampler2D", "the current chain image (for neighbour sampling)") +
        row("uResolution", "vec2", "chain resolution in pixels") +
        row("uTime", "float", "seconds since start (deterministic sim clock)") +
        row("uDelta", "float", "seconds since the last frame") +
        row("uFrame", "int", "frames since (re)compilation") +
        row("bass, mid, treb", "float", "band loudness (~0..1)") +
        row("vol", "float", "overall level") +
        row("beat", "float", "beat pulse (0/1)"));
    return QStringLiteral(
               "<h2>Pixel Filter — vec4 farbe(vec2 uv, vec4 src)</h2>"
               "<p>GLSL function that runs per PIXEL in the fragment shader — the "
               "scriptable style filter module (comic, posterize, halftone, "
               "CRT/VHS, Kuwahara …). <code>uv</code> is the pixel position "
               "0..1, <code>src</code> the source pixel; neighbours are fetched with "
               "<code>texture(uTex, uv + …)</code> (edge strokes!). Identity "
               "<code>return src;</code> = passthrough. The filter acts on "
               "the WHOLE chain image below it — video, camera, scopes, "
               "MilkDrop alike. (<code>filter</code> itself is a reserved "
               "word in GLSL — hence <code>farbe</code>, German for colour.)</p>"
               "<h3>Uniforms</h3>%1"
               "<h3>Controls</h3>"
               "<p><b>Mix</b>: share of the filtered image (1 = replace); "
               "audio-reactive too via the parameter script <code>mixamount</code>."
               "</p>"
               "<p style='color:#888'>One node = ONE pass. Stack several filters "
               "as several nodes in the chain (reorderable); "
               "true multipass looks (blur pyramids etc.) run through the "
               "Shadertoy node with Buffer A–D.</p>")
        .arg(uniforms);
}

/// Referenz des ISF-Filter-Nodes (S83). Vorher zeigten beide ISF-Editoren die
/// Pixel-Filter-Seite — mit `uTex` und `farbe()`, die es in ISF nicht gibt.
QString isfReferenceHtml()
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
            .arg(QString::fromUtf8(n), QString::fromUtf8(t), QString::fromUtf8(m));
    };
    const QString sprache = table(
        row("RENDERSIZE", "vec2", "size of the target in pixels") +
        row("TIME, TIMEDELTA", "float", "seconds since start / since the last frame") +
        row("FRAMEINDEX", "int", "frames since (re)compilation") +
        row("PASSINDEX", "int", "number of the pass (multipass)") +
        row("isf_FragNormCoord", "vec2", "pixel position 0..1") +
        row("IMG_THIS_PIXEL(img)", "vec4", "image input at this position") +
        row("IMG_NORM_PIXEL(img, uv)", "vec4", "image input at uv (0..1)") +
        row("IMG_PIXEL(img, px)", "vec4", "image input at pixel coordinate") +
        row("IMG_SIZE(img)", "vec2", "size of an image input"));
    const QString extras = table(
        row("getosc(band, breite, kanal) / getspec(…)", "float", "LumiViz: waveform / spectrum as in AVS (band 0..1; breite = width, kanal = channel)") +
        row("bass, mid, treb, vol, beat", "float", "LumiViz: band level · beat pulse"));
    return QStringLiteral(
               "<h2>ISF Filter — void main()</h2>"
               "<p>Interactive Shader Format: the code runs unchanged. "
               "Image inputs and controls are declared in the JSON header of the <code>.fs</code> and "
               "reach the node via <b>Import ISF file…</b> — the code "
               "here uses them by their name (<code>inputImage</code> …). A "
               "fresh node has no image input.</p>"
               "<h3>ISF language</h3>%1<h3>LumiViz extras</h3>%2"
               "<h3>Vertex (.vs)</h3>"
               "<p>Optional. <code>isf_vertShaderInit()</code> sets position and "
               "coordinate; after that the shader may precompute its own varyings "
               "(neighbour coordinates). Empty = default.</p>")
        .arg(sprache, extras);
}

} // namespace

QStringList ChainTreeWidget::mimeTypes() const
{
    QStringList types = QTreeWidget::mimeTypes();
    types << QString::fromUtf8(lumi::ui::kPaletteMime) << QStringLiteral("text/uri-list");
    return types;
}

namespace {
/// Pfad des LETZTEN Milkdrop-Knotens in Baumreihenfolge (tiefe zuerst), leer =
/// keiner. Ziel eines Milkdrop-Elements, das auf die Leerflaeche faellt (S80).
QList<int> letzterMilkdropPfad(const ChainNode& node, const QList<int>& pfad = {})
{
    QList<int> treffer;
    if (!pfad.isEmpty() && std::holds_alternative<MilkdropNodeParams>(node.params))
        treffer = pfad;
    for (size_t i = 0; i < node.children.size(); ++i)
    {
        const QList<int> kind =
            letzterMilkdropPfad(node.children[i], pfad + QList<int>{static_cast<int>(i)});
        if (!kind.isEmpty()) treffer = kind;
    }
    return treffer;
}

/// Eigene Zeilen, ein Paletteneintrag oder Preset-Dateien (S84) — sonst
/// (Zeilen fremder Baeume, andere Dateien) nimmt der Kettenbaum nichts an.
bool chainAccepts(const QDropEvent* event, const QObject* self)
{
    return event->source() == self ||
           event->mimeData()->hasFormat(QString::fromUtf8(lumi::ui::kPaletteMime)) ||
           !lumi::ui::presetDateien(event->mimeData()).isEmpty();
}
}  // namespace

void ChainTreeWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (!chainAccepts(event, this)) { event->ignore(); return; }
    QTreeWidget::dragEnterEvent(event);
}

void ChainTreeWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (!chainAccepts(event, this)) { event->ignore(); return; }
    QTreeWidget::dragMoveEvent(event);  // setzt den Einfuege-Indikator
    // Aus der Palette wird immer kopiert, eigene Zeilen immer verschoben.
    if (event->isAccepted())
        event->setDropAction(event->source() == this ? Qt::MoveAction : Qt::CopyAction);
}

void ChainTreeWidget::dropEvent(QDropEvent* event)
{
    if (!chainAccepts(event, this)) { event->ignore(); return; }
    QTreeWidgetItem* target = itemAt(event->position().toPoint());
    ChainDrop where = ChainDrop::Viewport;
    switch (dropIndicatorPosition())  // protected enum: only readable in-subclass
    {
        case QAbstractItemView::OnItem:     where = ChainDrop::OnItem; break;
        case QAbstractItemView::AboveItem:  where = ChainDrop::Above;  break;
        case QAbstractItemView::BelowItem:  where = ChainDrop::Below;  break;
        case QAbstractItemView::OnViewport: where = ChainDrop::Viewport; break;
    }
    const QString paletteMime = QString::fromUtf8(lumi::ui::kPaletteMime);
    if (event->mimeData()->hasFormat(paletteMime))
    {
        bool ok = false;
        const int index = event->mimeData()->data(paletteMime).toInt(&ok);
        if (ok && onPaletteDrop) onPaletteDrop(index, target, where);
    }
    else if (event->source() != this)
    {
        const QStringList dateien = lumi::ui::presetDateien(event->mimeData());
        if (!dateien.isEmpty() && onFileDrop) onFileDrop(dateien, target, where);
    }
    else
    {
        QTreeWidgetItem* src = currentItem();  // single-selection: dragged == current
        if (onDrop && src != nullptr) onDrop(src, target, where);
    }
    // Consume without calling the base: the chain owns ordering and the tree is
    // rebuilt from it, so the view must not move items on its own. IgnoreAction
    // (not MoveAction) stops QAbstractItemView::startDrag from also deleting the
    // source rows after our rebuild already replaced every item.
    event->setDropAction(Qt::IgnoreAction);
    event->accept();
}

void ChainTreeWidget::configureDragDrop()
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);  // Shift/Ctrl multi
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    // DragDrop statt InternalMove (S79): nur so nimmt der Baum auch Eintraege
    // aus dem Palettenfenster an. Eigene Zeilen werden weiterhin verschoben
    // (dropEvent), nie kopiert.
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
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

void ChainTreeWidget::setColumnMinimums(const QList<int>& minimums)
{
    m_columnMinimums = minimums;
    // Die letzte Spalte dehnt fitColumns selbst — stretchLastSection wuerde sie
    // bis auf minimumSectionSize stauchen und die Mindestbreite unterlaufen.
    header()->setStretchLastSection(false);
    connect(header(), &QHeaderView::sectionResized, this,
            [this](int, int, int) { fitColumns(); });
    fitColumns();
}

void ChainTreeWidget::resizeEvent(QResizeEvent* event)
{
    QTreeWidget::resizeEvent(event);
    fitColumns();
}

void ChainTreeWidget::fitColumns()
{
    const int count = std::min(static_cast<int>(m_columnMinimums.size()), columnCount());
    if (m_fitting || count == 0) return;
    m_fitting = true;
    QHeaderView* h = header();
    const int last = count - 1;
    int belegt = 0;
    for (int c = 0; c < last; ++c)
    {
        if (h->sectionSize(c) < m_columnMinimums[c])
            h->resizeSection(c, m_columnMinimums[c]);
        belegt += h->sectionSize(c);
    }
    // Die letzte Spalte fuellt den Rest; reicht der nicht fuer ihr Minimum,
    // wird der Baum breiter als sein Viewport — dann scrollt er (eigener Balken).
    const int rest = viewport()->width() - belegt;
    const int ziel = std::max(m_columnMinimums[last], rest);
    if (h->sectionSize(last) != ziel) h->resizeSection(last, ziel);
    m_fitting = false;
}

MultiEffectPanel::MultiEffectPanel(ServiceContainer& services, QWidget* parent)
    : PanelBase(services, "multieffect_chain", tr("Effect Chain"), parent)
{
    setupUI();
    connectToActiveVisualizer();
}

void MultiEffectPanel::setupUI()
{
    // Aeussere Scrollarea (S78, Vorgabe Patrik): die Felder schrumpfen mit dem
    // Dock bis auf kFeldMinBreite; wird das Dock noch schmaler, scrollt der
    // ganze Inhalt seitlich — von den Beschriftungen zu den Feldern. Das Dock
    // selbst geht bis kFeldMinBreite herunter. Senkrecht scrollen Baum und
    // Editor je fuer sich, deshalb hier nur waagrecht.
    auto* aussen = new QVBoxLayout(this);
    aussen->setContentsMargins(0, 0, 0, 0);
    auto* aussenScroll = new QScrollArea(this);
    aussenScroll->setFrameShape(QFrame::NoFrame);
    aussenScroll->setWidgetResizable(true);
    aussenScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    aussenScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    auto* inhalt = new QWidget();
    aussenScroll->setWidget(inhalt);
    aussen->addWidget(aussenScroll);
    // Die Mindestbreite des Panels (ein Feld + Raender) setzt
    // aktualisiereMindestbreite, sobald die Rahmenmasse bekannt sind.

    auto* root = new QVBoxLayout(inhalt);
    // Rand zum Panel: Baum, Toolbar und Editor halten mindestens kPanelRand
    // Abstand — nirgends sonst ein Aussenrand.
    root->setContentsMargins(kPanelRand, kPanelRand, kPanelRand, kPanelRand);

    m_hint = new QLabel(tr("Select the \"Multi Effect\" visualizer to edit its chain."),
                        this);
    m_hint->setWordWrap(true);
    root->addWidget(m_hint);

    // Toolbar: Palette (+) / entfernen / klonen / hoch / runter. Die Add-Combo
    // ist seit S79 ein eigenes Fenster (EffectPaletteWindow).
    auto* toolbar = new QHBoxLayout();

    auto makeButton = [this](const QString& text, const QString& tip) {
        auto* b = new QToolButton(this);
        b->setText(text);
        b->setToolTip(tip);
        return b;
    };
    m_addButton = makeButton("+", tr("Open the effect palette (search, filter, add or "
                                     "drag effects into the chain)"));
    m_removeButton = makeButton("-", tr("Remove selected"));
    m_cloneButton = makeButton(QString::fromUtf8("⧉"), tr("Clone selected (with its subtree)"));
    m_upButton = makeButton(QString::fromUtf8("↑"),
                            tr("Move up (at the top edge: out of the list, "
                               "placed before it)"));
    m_downButton = makeButton(QString::fromUtf8("↓"),
                              tr("Move down (at the bottom edge: out of the "
                                 "list, placed after it)"));
    toolbar->addWidget(m_addButton);
    toolbar->addWidget(m_removeButton);
    toolbar->addWidget(m_cloneButton);
    toolbar->addWidget(m_upButton);
    toolbar->addWidget(m_downButton);
    toolbar->addStretch();
    root->addLayout(toolbar);

    m_tree = new ChainTreeWidget(this);
    m_tree->setHeaderLabels({tr("Name"), QString(), tr("Type"), tr("Description")});
    m_tree->setColumnCount(4);
    m_tree->setColumnWidth(0, 160);
    m_tree->setColumnWidth(1, 30);  // narrow eye toggle column (col 1)
    m_tree->setColumnWidth(2, 110);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    // Entscheid 2a (S78): Mindestbreiten je Spalte; reicht der Platz nicht,
    // scrollt der Baum mit eigenem Balken, das Panel bleibt, wie es ist.
    m_tree->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_tree->setColumnMinimums({kBaumNameMin, 30, kBaumTypMin, kBaumBeschreibungMin});
    // Column 0 (Name) is the tree column: show the expand decoration + a clear
    // per-level indent so nested Effect Lists read as nested across levels.
    m_tree->setRootIsDecorated(true);
    m_tree->setIndentation(18);
    m_tree->setEditTriggers(QAbstractItemView::DoubleClicked |
                            QAbstractItemView::EditKeyPressed);
    // Drag & drop re-parents/reorders effects (into / out of groups). We handle
    // the drop ourselves (onDrop) and rebuild from the model.
    m_tree->configureDragDrop();
    m_tree->onDrop = [this](QTreeWidgetItem* s, QTreeWidgetItem* t, ChainDrop w) {
        onDropRequested(s, t, w);
    };
    m_tree->onPaletteDrop = [this](int idx, QTreeWidgetItem* t, ChainDrop w) {
        onPaletteDrop(idx, t, w);
    };
    m_tree->onFileDrop = [this](const QStringList& files, QTreeWidgetItem* t, ChainDrop w) {
        onPresetFileDrop(files, t, w);
    };
    m_tree->onDeleteKey = [this] { onRemove(); };

    m_propScroll = new QScrollArea(this);
    m_propScroll->setWidgetResizable(true);
    // Hier nie seitlich: die Seite folgt der Viewport-Breite, und
    // aktualisiereMindestbreite haelt den Viewport mindestens so breit wie die
    // Seite. Seitlich scrollt die aeussere Scrollarea — EIN Balken fuers Panel.
    m_propScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_propContainer = new QWidget();
    m_propLayout = new QVBoxLayout(m_propContainer);
    m_propLayout->setContentsMargins(kSeitenInnenrand, kSeitenInnenrand,
                                     kSeitenInnenrand, kSeitenInnenrand);
    m_propLayout->setAlignment(Qt::AlignTop);
    m_propScroll->setWidget(m_propContainer);

    // Aufteilung Baum/Eigenschafts-Editor verstellbar (S66, Wunsch Patrik):
    // vertikaler Splitter statt fester 50:50-Stretches; die Stellung wird in
    // QSettings gemerkt und beim naechsten Aufbau wiederhergestellt.
    auto* split = new QSplitter(Qt::Vertical, this);
    split->setChildrenCollapsible(false);
    split->addWidget(m_tree);
    split->addWidget(m_propScroll);
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 1);
    {
        const QByteArray state =
            QSettings().value(QStringLiteral("multieffect/chainSplitter")).toByteArray();
        if (!state.isEmpty()) split->restoreState(state);
    }
    connect(split, &QSplitter::splitterMoved, this, [split]() {
        QSettings().setValue(QStringLiteral("multieffect/chainSplitter"),
                             split->saveState());
    });
    root->addWidget(split, 1);

    // Schmalste Dock-Stellung = kPanelMinInhalt samt Raendern (Vorgabe Patrik
    // S78: etwa doppelte Feldbreite — Platz fuer Beschriftung + Feld); was
    // breiter ist, erreicht man per Seitwaerts-Scrollen. JETZT setzen, nicht
    // queued: Qt-ADS liest das Minimum beim Einhaengen ins Dock und bekommt
    // spaetere Aenderungen nicht mit (Sichttest S78: sonst Dock bis 0 px).
    setMinimumWidth(kPanelMinInhalt + editorRahmen() + 2 * kPanelRand);

    // Keine feste Inhaltsbreite mehr (bis S77: 420 am Panel). Die Breite, ab der
    // seitlich gescrollt wird, ergibt sich aus Toolbar und Editor-Seite (S78).
    aktualisiereMindestbreite();

    connect(m_addButton, &QToolButton::clicked, this, &MultiEffectPanel::openPalette);
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
                m_kameraDialogGezeigt = false;  // neue Kette darf neu fragen (S70)
                rebuildTree();
                clearPropertyEditor();
            }));
    }
}

void MultiEffectPanel::setHost(MultiEffectVisualizer* host, QMutex* mutex)
{
    m_host = host;
    m_mutex = mutex;
    m_kameraDialogGezeigt = false;  // neuer Host = neue Kette, neu fragen (S70)
    const bool active = m_host != nullptr;
    m_hint->setVisible(!active);
    m_tree->setEnabled(active);
    // „+" bleibt bedienbar: die Palette laesst sich auch ohne Kette durchsehen,
    // nur das Einfuegen wartet auf den Visualizer.
    m_active = active;
    if (m_palette != nullptr) m_palette->setInsertEnabled(active);
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

namespace
{
/// Aufklapp-Schluessel eines Baum-Items (S82): die nodeId des Knotens (ueber
/// Neukompilieren stabil, anders als der Pfad) bzw. fuer Milkdrop-Sektionen
/// Eltern-Schluessel + Sektion. Leer = kein merkbares Item.
QString aufklappSchluessel(const QTreeWidgetItem* item)
{
    const QVariant id = item->data(0, kNodeIdRole);
    if (id.isValid()) return QStringLiteral("n%1").arg(id.toULongLong());
    if (item->parent() == nullptr) return QString();
    const QString eltern = aufklappSchluessel(item->parent());
    if (eltern.isEmpty()) return QString();
    return eltern + QStringLiteral("/") + item->text(0);
}

void eingeklappteSammeln(const QTreeWidgetItem* item, QSet<QString>& zu)
{
    if (item->childCount() > 0 && !item->isExpanded())
    {
        const QString k = aufklappSchluessel(item);
        if (!k.isEmpty()) zu.insert(k);
    }
    for (int i = 0; i < item->childCount(); ++i) eingeklappteSammeln(item->child(i), zu);
}

void eingeklappteAnwenden(QTreeWidgetItem* item, const QSet<QString>& zu)
{
    if (item->childCount() > 0 && zu.contains(aufklappSchluessel(item)))
        item->setExpanded(false);
    for (int i = 0; i < item->childCount(); ++i) eingeklappteAnwenden(item->child(i), zu);
}
}  // namespace

void MultiEffectPanel::rebuildTree(bool aufklappZustandBehalten)
{
    // Eingeklappte Container ueber den Neuaufbau retten (Befund Patrik S82:
    // „Zu dieser Gruppe wechseln" klappte die ganze Kette wieder auf). Neue
    // Knoten und eine frisch geladene Kette kommen weiterhin aufgeklappt.
    QSet<QString> eingeklappt;
    if (aufklappZustandBehalten)
    {
        for (int i = 0; i < m_tree->topLevelItemCount(); ++i)
            eingeklappteSammeln(m_tree->topLevelItem(i), eingeklappt);
    }

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
    if (!eingeklappt.isEmpty())
    {
        for (int i = 0; i < m_tree->topLevelItemCount(); ++i)
            eingeklappteAnwenden(m_tree->topLevelItem(i), eingeklappt);
    }
    m_updating = false;
    // Kamera-Freigabe-Dialog queued NACH dem Aufbau (kein Modal-Dialog mitten
    // im Tree-Umbau) — fragt nur, wenn die Kette eine Kamera-Quelle traegt.
    QMetaObject::invokeMethod(
        this, [this] { pruefeKameraFreigabe(); }, Qt::QueuedConnection);
}

namespace
{
/// Enthaelt die Kette (rekursiv) eine Kamera-Quelle mit gewaehltem Geraet?
bool chainHatKameraQuelle(const ChainNode& node)
{
    if (const auto* p = std::get_if<VideoSourceParams>(&node.params))
    {
        if (node.enabled && p->source == 1 && !p->cameraId.empty()) return true;
    }
    for (const ChainNode& child : node.children)
    {
        if (chainHatKameraQuelle(child)) return true;
    }
    return false;
}
}  // namespace

void MultiEffectPanel::pruefeKameraFreigabe()
{
    auto& feed = lumi::services::LiveVideoFeed::instance();
    if (m_host == nullptr || m_mutex == nullptr) return;
    if (feed.kameraErlaubt() || m_kameraDialogGezeigt) return;
    bool hatKamera = false;
    {
        QMutexLocker lock(m_mutex);
        hatKamera = chainHatKameraQuelle(m_host->chain());
    }
    if (!hatKamera) return;
    m_kameraDialogGezeigt = true;  // je geladener Kette nur einmal fragen
    // EINZIGE Vollbild-Ausnahme (Entscheid Patrik S70): der Dialog erscheint
    // auch ueber dem Vollbild — Eltern ist das AKTIVE Fenster (das Panel kann
    // dort versteckt sein) und der Dialog liegt immer obenauf.
    QWidget* eltern = QApplication::activeWindow() != nullptr
                          ? QApplication::activeWindow()
                          : static_cast<QWidget*>(this);
    QMessageBox frage(QMessageBox::Question, tr("Camera source in preset"),
                      tr("This preset contains a camera source. Cameras "
                         "never start automatically — allow it for this app run "
                         "now?\n\n(Windows may then ask for camera "
                         "permission. Without permission the "
                         "video node stays black.)"),
                      QMessageBox::Yes | QMessageBox::No, eltern);
    frage.setWindowFlag(Qt::WindowStaysOnTopHint, true);
    if (frage.exec() == QMessageBox::Yes) feed.erlaubeKamera();
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
    if (node.nodeId != 0) item->setData(0, kNodeIdRole, QVariant::fromValue(node.nodeId));

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

    // Presetverweis (S82): Inhalt NICHT zeigen — er ist geliehen, jede
    // Aenderung daran ginge beim Speichern verloren. Statt dessen der Pfad.
    if (node.isReference())
    {
        const QString quelle =
            QString::fromStdString(std::get<HostGroupParams>(node.params).sourceFile);
        item->setText(2, tr("Preset Reference"));
        item->setToolTip(0, tr("Preset reference to %1 — %2 nodes, read fresh from the "
                               "file on load (not saved)")
                                .arg(quelle)
                                .arg(node.children.size()));
        item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
    }
    else if (node.isContainer())  // Listen + Host-Gruppen (HG1)
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
        int spritesOn = 0;
        for (const auto& w : milk->preset.waves)
            if (w.enabled) ++wavesOn;
        for (const auto& s : milk->preset.shapes)
            if (s.enabled) ++shapesOn;
        for (const auto& sp : milk->preset.sprites)
            if (sp.enabled) ++spritesOn;
        // Anzeige-Reihenfolge = Render-Pipeline (S66, Entscheid Patrik):
        // Parameter (Rahmen) zuoberst, dann die Stufen in Frame-Reihenfolge —
        // Code → Warp → Shapes → Waves → Comp → Sprites (Overlay zuletzt).
        // Die SEKTIONS-ID (zweites Feld) bleibt stabil: 0=Code 1=Waves
        // 2=Shapes 3=Shader-Warp 4=Sprites 5=Parameter 6=Shader-Comp — Pfade,
        // Editor-Dispatch und Toolbar-Logik hängen an der Id, nicht an der
        // Baum-Position. Reine Anzeige: die Pipeline selbst ist fest.
        // Waves-Sektionslabel: die Basis-Welle zaehlt nur, wenn sie an ist
        // (seit S76 schaltbar — die blanke Pipeline startet ohne sie).
        const auto wavesLabel = [this](bool basisOn, int custom) {
            return basisOn ? tr("Waves (base + %1 custom active)").arg(custom)
                           : tr("Waves (%1 custom active)").arg(custom);
        };
        const QList<QPair<int, QString>> sections = {
            {5, tr("Parameters (base values)")},
            {0, tr("Code (per_frame / per_pixel)")},
            {3, tr("Shader Warp (HLSL)")},
            {2, tr("Shapes (%1 active)").arg(shapesOn)},
            {1, wavesLabel(milk->preset.basicWaveEnabled, wavesOn)},
            {6, tr("Shader Comp (HLSL)")},
            {4, tr("Sprites (%1 active)").arg(spritesOn)}};
        const bool haveMilkIcons = !lumi::ui::presetIconDir().isEmpty();
        const QIcon& milkIcon =
            lumi::ui::presetTypeIcon(lumi::ui::PresetIconKind::MilkDrop);

        // Auge-Toggle fuer Wave-/Shape-/Sprite-Elemente + Basis-Waveform (S43,
        // Sichttest-Befund; S76 verallgemeinert): spiegelt `enabled` wie das
        // Node-Auge — Mutation via mutate() (renderMutex + recompile) +
        // Revision-Bump. Labels/Zaehler/Auge-Text zieht syncMilkEyes nach —
        // EINE Logik fuer beide Richtungen (Auge und Panel-Checkbox, SSOT);
        // enabled-Toggle ist nicht strukturell, kein Rebuild.
        auto addMilkEye = [this, path](QTreeWidgetItem* el, int sectionIdx,
                                       int elementIdx, bool on) {
            auto* eye = new QToolButton(m_tree);
            eye->setCheckable(true);
            eye->setAutoRaise(true);
            eye->setChecked(on);
            eye->setText(on ? QString::fromUtf8("\xF0\x9F\x91\x81")   // 👁
                            : QString::fromUtf8("\xE2\x80\x94"));      // —
            eye->setToolTip(tr("Show / hide this element"));
            connect(eye, &QToolButton::toggled, this,
                    [this, el, path, sectionIdx, elementIdx](bool v) {
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
                            };
                            if (sectionIdx == 1)
                            {
                                if (elementIdx == kMilkBasisWaveElement)
                                {
                                    m->preset.basicWaveEnabled = v;
                                    ++m->revision;
                                }
                                else apply(m->preset.waves);
                            }
                            else if (sectionIdx == 2) apply(m->preset.shapes);
                            else if (sectionIdx == 4) apply(m->preset.sprites);
                        });
                        syncMilkEyes(path);
                        // Editor-Checkbox „... aktiv" nachziehen, falls sichtbar
                        if (m_tree->currentItem() == el)
                            buildPropertyEditor(currentPath());
                    });
            m_tree->setItemWidget(el, 1, eye);
        };

        for (const auto& [s, label] : sections)
        {
            auto* sec = new QTreeWidgetItem(item);
            sec->setText(0, label);
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
                // Basis-Waveform zuerst: sie existiert immer genau einmal und
                // braucht einen sichtbaren Editor-Einstieg — seit S76 mit
                // eigenem Auge (basicWaveEnabled) statt Immer-Rendern.
                const bool basisOn = milk->preset.basicWaveEnabled;
                auto* basisEl = addElement(
                    kMilkBasisWaveElement,
                    tr("Base waveform") +
                        (basisOn ? QString() : tr(" (off)")));
                addMilkEye(basisEl, 1, kMilkBasisWaveElement, basisOn);
                for (int i = 0; i < static_cast<int>(milk->preset.waves.size()); ++i)
                {
                    const auto& w = milk->preset.waves[i];
                    auto* el = addElement(
                        i, tr("Wave %1%2")
                               .arg(w.index)
                               .arg(w.enabled ? QString() : tr(" (off)")));
                    addMilkEye(el, 1, i, w.enabled);
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
                               .arg(sh.enabled ? QString() : tr(" (off)")));
                    addMilkEye(el, 2, i, sh.enabled);
                }
            }
            else if (s == 4)
            {
                for (int i = 0; i < static_cast<int>(milk->preset.sprites.size()); ++i)
                {
                    const auto& sp = milk->preset.sprites[i];
                    const QString base =
                        tr("Sprite %1 — %2")
                            .arg(sp.index)
                            .arg(QString::fromStdString(sp.imageName));
                    auto* el = addElement(
                        i, base + (sp.enabled ? QString() : tr(" (off)")));
                    addMilkEye(el, 4, i, sp.enabled);
                }
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

QString MultiEffectPanel::herkunftKopfFuer(const QList<int>& path)
{
    Herkunft h;
    {
        QMutexLocker lock(m_mutex);
        const ChainNode* n = nodeAtPath(path);
        if (n == nullptr) return {};
        const Herkunft* q = herkunftVon(n->params);
        if (q == nullptr) return {};
        h = *q;  // Kopie: der Kopf wird ausserhalb des Locks gebaut
    }
    return lumi::scriptedit::herkunftKopf(QString::fromStdString(h.name),
                                          QString::fromStdString(h.author),
                                          QString::fromStdString(h.url),
                                          QString::fromStdString(h.license));
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

namespace
{
// Sicht einer Preset-Zeile — Definition bei addPresetRow (S83).
QJsonObject sichtLesen(const ChainNode& n, int section, int element);
}  // namespace

void MultiEffectPanel::mutate(const QList<int>& path,
                              const std::function<void(ChainNode&)>& fn,
                              bool aendertVoreinstellung)
{
    if (m_host == nullptr || m_mutex == nullptr) return;
    QStringList geaendert;
    {
        QMutexLocker lock(m_mutex);
        ChainNode* node = nodeAtPath(path);
        if (node == nullptr) return;
        // Nur saubere Preset-Marken dieses Knotens (Knoten + Milkdrop-Elemente)
        // vorher/nachher vergleichen: Felder melden editingFinished auch ohne
        // Aenderung (Fokuswechsel). Ist eine Marke „(unsaved)", entfaellt ihr
        // Vergleich — beim Reglerziehen kostet das also nichts.
        std::vector<std::pair<QString, QJsonObject>> vorher;
        if (aendertVoreinstellung)
            for (auto it = m_presetMarken.cbegin(); it != m_presetMarken.cend(); ++it)
                if (it->nodeId == node->nodeId && !it->geaendert)
                    vorher.emplace_back(it.key(),
                                        sichtLesen(*node, it->section, it->element));
        fn(*node);
        for (const auto& [schluessel, json] : vorher)
        {
            const PresetMarke& m = m_presetMarken[schluessel];
            if (sichtLesen(*node, m.section, m.element) != json) geaendert.append(schluessel);
        }
        m_host->recompileChain();
    }
    for (const QString& schluessel : geaendert) markierePresetGeaendert(schluessel);
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
    rebuildTree(/*aufklappZustandBehalten=*/true);
    clearPropertyEditor();
}

void MultiEffectPanel::openPalette()
{
    if (m_palette == nullptr)
    {
        m_palette = new lumi::ui::EffectPaletteWindow(this);
        connect(m_palette, &lumi::ui::EffectPaletteWindow::addRequested, this,
                &MultiEffectPanel::onAddEffect);
    }
    m_palette->setInsertEnabled(m_active);
    m_palette->present();
}

void MultiEffectPanel::onAddEffect(int paletteIndex)
{
    if (m_host == nullptr) return;
    const auto& palette = effectPalette();
    if (paletteIndex < 0 || paletteIndex >= static_cast<int>(palette.size())) return;
    const EffectType& chosen = palette[static_cast<size_t>(paletteIndex)];
    if (chosen.isHeader()) return;  // category title, not an insertable effect

    const QList<int> sel = currentPath();
    bool ok = false;
    if (chosen.milkElement != MilkElement::None)
    {
        ok = insertMilkElement(paletteIndex, sel);
    }
    else
    {
        // Wohin der neue Knoten kommt (Vorgabe Patrik S55):
        //   markierte Liste / Host-Gruppe -> als LETZTES Element hinein
        //   markierter Effekt             -> direkt DARUNTER, in dessen Liste
        //   nichts markiert               -> ans Ende der Root-Liste
        // Vorher landete er in den letzten beiden Faellen immer ganz unten in
        // der Root — bei einer verschachtelten Kette also weit weg von der
        // Stelle, an der man gerade arbeitet.
        QList<int> elternPfad;
        int pos = -1;  // -1 = anhaengen
        {
            QMutexLocker lock(m_mutex);
            const ChainNode* target = nodeAtPath(sel);
            if (target != nullptr && target->acceptsChildren())  // kein Verweis (S82)
            {
                elternPfad = sel;
            }
            else if (!sel.isEmpty())
            {
                elternPfad = sel;
                pos = elternPfad.takeLast() + 1;
            }
        }
        ok = insertPaletteNode(paletteIndex, elternPfad, pos);
    }
    if (ok && m_palette != nullptr) m_palette->noteUsed(paletteIndex);
}

void MultiEffectPanel::onPaletteDrop(int paletteIndex, QTreeWidgetItem* target,
                                     ChainDrop where)
{
    if (m_host == nullptr) return;
    const auto& palette = effectPalette();
    if (paletteIndex < 0 || paletteIndex >= static_cast<int>(palette.size())) return;
    QList<int> targetPath;
    if (target != nullptr) targetPath = pathFromVariant(target->data(0, Qt::UserRole));

    // Wie onDropRequested: erst nach dem dropEvent des Baums einfuegen — der
    // Neuaufbau loeschte sonst Items, die die Drag-Maschinerie noch anfasst.
    QMetaObject::invokeMethod(
        this,
        [this, paletteIndex, targetPath, where] {
            const EffectType& chosen = effectPalette()[static_cast<size_t>(paletteIndex)];
            bool ok = false;
            if (chosen.milkElement != MilkElement::None)
            {
                // Wave/Shape/Sprite: das Ziel muss ein Milkdrop-Node sein (oder
                // eine seiner Sektionen/Elemente) — sonst passiert nichts.
                // Leerflaeche (Befund S80): gemeint ist der Milkdrop-Knoten
                // direkt darueber, also der letzte in Baumreihenfolge. Vorher
                // ging das Element dort still verloren; nur „direkt unter der
                // letzten Zeile" traf zufaellig dessen Sprites-Sektion.
                QList<int> anker = targetPath;
                if (anker.isEmpty() || where == ChainDrop::Viewport)
                {
                    QMutexLocker lock(m_mutex);
                    anker = letzterMilkdropPfad(m_host->chain());
                }
                ok = insertMilkElement(paletteIndex, anker);
            }
            else
            {
                QList<int> elternPfad;
                int pos = -1;
                dropOrt(targetPath, where, elternPfad, pos);
                ok = insertPaletteNode(paletteIndex, elternPfad, pos);
            }
            if (ok && m_palette != nullptr) m_palette->noteUsed(paletteIndex);
        },
        Qt::QueuedConnection);
}

void MultiEffectPanel::dropOrt(const QList<int>& targetPath, ChainDrop where,
                               QList<int>& elternPfad, int& pos)
{
    // Ziel auf eine Milkdrop-Sektion/-Element: gemeint ist der Node.
    const MilkPathInfo mp = splitMilkPath(targetPath);
    const QList<int> ziel = mp.nodePath;
    const ChainDrop wo = mp.section >= 0 ? ChainDrop::Below : where;

    // Dieselben Regeln wie beim Verschieben (moveNodesLocked).
    elternPfad.clear();
    pos = -1;
    if (ziel.isEmpty() || wo == ChainDrop::Viewport) return;
    QMutexLocker lock(m_mutex);
    const ChainNode* tnode = nodeAtPath(ziel);
    elternPfad = ziel;
    const int idx = elternPfad.takeLast();
    if (wo == ChainDrop::OnItem && tnode != nullptr && tnode->acceptsChildren())
        elternPfad = ziel;  // hinein, ans Ende
    else if (wo == ChainDrop::Above)
        pos = idx;
    else
        pos = idx + 1;
}

void MultiEffectPanel::onPresetFileDrop(const QStringList& files, QTreeWidgetItem* target,
                                        ChainDrop where)
{
    if (m_host == nullptr || files.isEmpty()) return;
    // Der Paletteneintrag „Preset Reference…" ist der eine Weg, einen Verweis
    // anzulegen — der Drop gibt ihm nur die Datei vor (kein Dateidialog).
    const auto& palette = effectPalette();
    int verweisIndex = -1;
    for (size_t i = 0; i < palette.size() && verweisIndex < 0; ++i)
        if (palette[i].fromFile && palette[i].asReference) verweisIndex = static_cast<int>(i);
    if (verweisIndex < 0) return;
    QList<int> targetPath;
    if (target != nullptr) targetPath = pathFromVariant(target->data(0, Qt::UserRole));

    // Wie onPaletteDrop: erst nach dem dropEvent des Baums einfuegen.
    QMetaObject::invokeMethod(
        this,
        [this, files, verweisIndex, targetPath, where] {
            QList<int> elternPfad;
            int pos = -1;
            dropOrt(targetPath, where, elternPfad, pos);
            for (const QString& datei : files)
            {
                // Erster Fehlschlag (Tiefenregel, unlesbare Datei) beendet den
                // Zug — sonst kaeme dieselbe Meldung einmal je Datei.
                if (!insertPaletteNode(verweisIndex, elternPfad, pos, datei)) break;
                if (pos >= 0) ++pos;
            }
        },
        Qt::QueuedConnection);
}

bool MultiEffectPanel::insertPaletteNode(int paletteIndex, const QList<int>& parentPath,
                                         int pos, const QString& datei)
{
    const EffectType& chosen = effectPalette()[static_cast<size_t>(paletteIndex)];
    if (chosen.make == nullptr) return false;
    ChainNode fresh;
    fresh.params = chosen.make();

    if (chosen.fromFile)
    {
        // Tiefenregel VOR dem Dateidialog — sonst waehlt man umsonst eine Datei
        {
            QMutexLocker lock(m_mutex);
            if (pathTouchesHostGroup(m_host->chain(), parentPath))
            {
                QMessageBox::information(
                    this, tr("Preset as Group"),
                    tr("A host group cannot sit inside a host group "
                       "(depth rule) — please select a position outside every group."));
                return false;
            }
        }
        QString quelle = datei;  // leer = Dateidialog
        std::vector<ChainNode> inhalt;
        if (!presetGruppeLaden(quelle, inhalt)) return false;
        fresh.children = std::move(inhalt);
        fresh.displayName = QFileInfo(quelle).completeBaseName().toStdString();
        auto& hg = std::get<HostGroupParams>(fresh.params);
        hg.sourceFile = quelle.toStdString();
        hg.verweis = chosen.asReference;  // Presetverweis: nur der Pfad wird gespeichert
    }

    QList<int> neuerPfad;
    mutateStructure([&] {
        ChainNode* eltern = nodeAtPath(parentPath);
        if (eltern == nullptr) return;

        // Tiefenregel (HG1, Entwurf §5.1): keine Host-Gruppe in eine
        // Host-Gruppe einfuegen — weder direkt noch in eine Liste darunter.
        // Geprueft wird der EINFUEGE-ORT, nicht die Auswahl: seit der neue
        // Knoten neben einem markierten Effekt landen kann, sind das zwei
        // verschiedene Dinge.
        if (fresh.isHostGroup() && pathTouchesHostGroup(m_host->chain(), parentPath)) return;

        const int anzahl = static_cast<int>(eltern->children.size());
        const int at = (pos < 0 || pos > anzahl) ? anzahl : pos;
        eltern->children.insert(eltern->children.begin() + at, std::move(fresh));
        neuerPfad = parentPath;
        neuerPfad << at;
    });

    // Den neuen Knoten markieren UND seinen Editor aufbauen. Ohne das blieb die
    // Auswahl auf dem alten Knoten stehen, und der neue liess sich erst
    // bearbeiten, wenn man einmal weg- und wieder hinklickte (Befund Patrik
    // S55). `buildPropertyEditor` zusaetzlich, weil `mutateStructure` den Baum
    // neu aufbaut und dabei `clearPropertyEditor()` ruft — ob `setCurrentItem`
    // danach ein Auswahl-Signal ausloest, haengt daran, ob der Baum die alte
    // Auswahl schon wiederhergestellt hat. Der Aufruf ist idempotent.
    if (neuerPfad.isEmpty()) return false;
    selectByPath(neuerPfad);
    buildPropertyEditor(neuerPfad);
    return true;
}

bool MultiEffectPanel::presetGruppeLaden(QString& datei, std::vector<ChainNode>& outInhalt)
{
    const QString titel = tr("Preset as Group");
    if (datei.isEmpty())
    {
        QSettings settings;
        const QString schluessel = QStringLiteral("effectPalette/presetGroupDir");
        datei = QFileDialog::getOpenFileName(
            this, titel, settings.value(schluessel).toString(),
            tr("Presets (*.avs *.milk *.lvfx *.lvfx2);;AVS Presets (*.avs);;"
               "MilkDrop Presets (*.milk);;LumiViz Effect Chains (*.lvfx *.lvfx2)"));
        if (datei.isEmpty()) return false;
        settings.setValue(schluessel, QFileInfo(datei).absolutePath());
    }

    ChainNode baum;
    QStringList report;
    if (!MultiEffectVisualizer::buildPresetTree(datei, baum, &report))
    {
        QString text = tr("Could not load:\n%1").arg(datei);
        if (!report.isEmpty()) text += QStringLiteral("\n\n") + report.join('\n');
        QMessageBox::warning(this, titel, text);
        return false;
    }
    // Tiefenregel: die Quelle darf selbst Gruppen haben — sie werden beim
    // Kompilieren zur Effect List degradiert (wie beim .lvfx-Import)
    if (chainHasHostGroup(baum))
    {
        QMessageBox::information(
            this, titel,
            tr("The source itself contains host groups — inside this "
               "group they run as Effect Lists (depth rule)."));
    }
    // "ℹ"-Zeilen sind reine Bestaetigungen — nur echte Probleme melden
    QStringList probleme;
    for (const QString& zeile : report)
    {
        if (!zeile.startsWith(QStringLiteral("ℹ"))) probleme.append(zeile);
    }
    if (!probleme.isEmpty())
    {
        QMessageBox::information(this, titel,
                                 tr("Loaded, with notes:\n\n%1").arg(probleme.join('\n')));
    }
    outInhalt = MultiEffectVisualizer::presetGroupChildren(std::move(baum));
    return true;
}

bool MultiEffectPanel::insertMilkElement(int paletteIndex, const QList<int>& anchorPath)
{
    const EffectType& chosen = effectPalette()[static_cast<size_t>(paletteIndex)];
    // Milkdrop-Node-Inhalt (N3.1/N3.3): landet im PresetState des Milkdrop-Nodes
    // (bzw. seiner Sektion/seines Elements), nicht in der Chain.
    const MilkPathInfo mp = splitMilkPath(anchorPath);
    QList<int> newSel;
    mutateStructure([&] {
        ChainNode* node = nodeAtPath(mp.nodePath);
        auto* milk = node ? std::get_if<MilkdropNodeParams>(&node->params) : nullptr;
        if (milk == nullptr) return;  // Ziel ist kein Milkdrop-Node
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
                newSel << (kMilkSectionBase + 1) << static_cast<int>(preset.waves.size()) - 1;
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
                newSel << (kMilkSectionBase + 2) << static_cast<int>(preset.shapes.size()) - 1;
                break;
            }
            case MilkElement::Sprite:
            {
                lumi::milkdrop::SpriteState sp;
                sp.index = nextFreeMilkIndex(preset.sprites);
                preset.sprites.push_back(std::move(sp));
                ++milk->revision;
                newSel = mp.nodePath;
                newSel << (kMilkSectionBase + 4) << static_cast<int>(preset.sprites.size()) - 1;
                break;
            }
            default: break;
        }
    });
    if (newSel.isEmpty()) return false;
    selectByPath(newSel);
    return true;
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

        // Container-Ausstieg (S66, Befund Patrik: steht die Effect List
        // zuunterst, ist "dahinter auf Eltern-Ebene" per Drop kaum treffbar):
        // Am RAND der Geschwister schiebt ↑/↓ den Block eine Ebene HINAUS —
        // vor bzw. hinter den Eltern-Container. Nur ganze Ebenen, nie aus der
        // Wurzel; Hinein geht weiterhin ueber Drop AUF den Container.
        const bool atEdge = contiguous && !parentPath.isEmpty() &&
                            ((delta < 0 && minI == 0) ||
                             (delta > 0 && idxs.last() == n - 1));
        if (atEdge)
        {
            QList<int> grandPath = parentPath;
            const int parentIdx = grandPath.takeLast();
            ChainNode* grand = nodeAtPath(grandPath);
            if (grand == nullptr) return;
            std::vector<ChainNode> picked;
            for (int i : idxs)
                picked.push_back(std::move(parent->children[static_cast<size_t>(i)]));
            for (int j = count - 1; j >= 0; --j)
                parent->children.erase(parent->children.begin() + idxs[j]);
            const int insertAt = delta < 0 ? parentIdx : parentIdx + 1;
            for (int k = 0; k < count; ++k)
                grand->children.insert(grand->children.begin() + insertAt + k,
                                       std::move(picked[static_cast<size_t>(k)]));
            for (int k = 0; k < count; ++k)
            {
                QList<int> np = grandPath;
                np.append(insertAt + k);
                newSel.append(np);
            }
            return;
        }

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
            // Nach dem Verschieben denselben Doppelschritt wie beim Einfuegen
            // (s. onAdd, Befund Patrik S55): markieren UND den Editor aufbauen.
            // `selectPaths` setzt die Auswahl unter `m_selecting`, das
            // Auswahl-Signal ist dabei stillgelegt — das folgende
            // `setCurrentItem` aendert dann nur noch das aktuelle Element und
            // loest keines mehr aus. Der verschobene Knoten liess sich deshalb
            // erst bearbeiten, nachdem man einmal weg- und wieder hinklickte
            // (Befund Patrik S56, dieselbe Ursache wie damals beim Hinzufuegen).
            if (moved)
            {
                selectPaths(finalPaths);
                buildPropertyEditor(currentPath());
            }
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
            if (tnode != nullptr && tnode->acceptsChildren())
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
            if (tnode != nullptr && tnode->acceptsChildren())
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

void MultiEffectPanel::syncMilkEyes(const QList<int>& nodePath)
{
    // Zustand unter dem Mutex kopieren — die UI-Arbeit passiert danach.
    bool basisOn = true;
    struct Elem
    {
        int index;
        bool enabled;
        QString name;  // nur Sprites (Bildname im Label)
    };
    std::vector<Elem> waves;
    std::vector<Elem> shapes;
    std::vector<Elem> sprites;
    {
        QMutexLocker lock(m_mutex);
        const ChainNode* n = nodeAtPath(nodePath);
        const auto* m = n ? std::get_if<MilkdropNodeParams>(&n->params) : nullptr;
        if (m == nullptr) return;
        basisOn = m->preset.basicWaveEnabled;
        for (const auto& w : m->preset.waves)
            waves.push_back({w.index, w.enabled, {}});
        for (const auto& s : m->preset.shapes)
            shapes.push_back({s.index, s.enabled, {}});
        for (const auto& sp : m->preset.sprites)
            sprites.push_back({sp.index, sp.enabled,
                               QString::fromStdString(sp.imageName)});
    }

    const auto setEye = [this](QTreeWidgetItem* el, bool on) {
        auto* eye = qobject_cast<QToolButton*>(m_tree->itemWidget(el, 1));
        if (eye == nullptr) return;
        const QSignalBlocker blocker(eye);  // kein Rueck-Mutieren
        eye->setChecked(on);
        eye->setText(on ? QString::fromUtf8("\xF0\x9F\x91\x81")   // 👁
                        : QString::fromUtf8("\xE2\x80\x94"));      // —
    };
    const auto elemItem = [this, &nodePath](int section,
                                            int element) -> QTreeWidgetItem* {
        QList<int> p = nodePath;
        p << (kMilkSectionBase + section) << element;
        return itemAtPath(p);
    };
    const auto sectionItem = [this, &nodePath](int section) -> QTreeWidgetItem* {
        QList<int> p = nodePath;
        p << (kMilkSectionBase + section);
        return itemAtPath(p);
    };
    const auto aus = [this](bool on) {
        return on ? QString() : tr(" (off)");
    };

    m_updating = true;
    int wavesOn = 0;
    if (QTreeWidgetItem* el = elemItem(1, kMilkBasisWaveElement))
    {
        el->setText(0, tr("Base waveform") + aus(basisOn));
        setEye(el, basisOn);
    }
    for (int i = 0; i < static_cast<int>(waves.size()); ++i)
    {
        if (waves[static_cast<std::size_t>(i)].enabled) ++wavesOn;
        if (QTreeWidgetItem* el = elemItem(1, i))
        {
            const Elem& w = waves[static_cast<std::size_t>(i)];
            el->setText(0, tr("Wave %1").arg(w.index) + aus(w.enabled));
            setEye(el, w.enabled);
        }
    }
    if (QTreeWidgetItem* sec = sectionItem(1))
        sec->setText(0, basisOn
                            ? tr("Waves (base + %1 custom active)").arg(wavesOn)
                            : tr("Waves (%1 custom active)").arg(wavesOn));

    int shapesOn = 0;
    for (int i = 0; i < static_cast<int>(shapes.size()); ++i)
    {
        if (shapes[static_cast<std::size_t>(i)].enabled) ++shapesOn;
        if (QTreeWidgetItem* el = elemItem(2, i))
        {
            const Elem& s = shapes[static_cast<std::size_t>(i)];
            el->setText(0, tr("Shape %1").arg(s.index) + aus(s.enabled));
            setEye(el, s.enabled);
        }
    }
    if (QTreeWidgetItem* sec = sectionItem(2))
        sec->setText(0, tr("Shapes (%1 active)").arg(shapesOn));

    int spritesOn = 0;
    for (int i = 0; i < static_cast<int>(sprites.size()); ++i)
    {
        if (sprites[static_cast<std::size_t>(i)].enabled) ++spritesOn;
        if (QTreeWidgetItem* el = elemItem(4, i))
        {
            const Elem& sp = sprites[static_cast<std::size_t>(i)];
            el->setText(0, tr("Sprite %1 — %2").arg(sp.index).arg(sp.name) +
                               aus(sp.enabled));
            setEye(el, sp.enabled);
        }
    }
    if (QTreeWidgetItem* sec = sectionItem(4))
        sec->setText(0, tr("Sprites (%1 active)").arg(spritesOn));
    m_updating = false;
}

void MultiEffectPanel::umbenennenLocked(ChainNode& n, std::string neu)
{
    if (!n.isHostGroup() || m_host == nullptr)
    {
        n.displayName = std::move(neu);
        return;
    }
    // Host-Gruppe (S82): Name ist die ID fuer den Host Switcher — eindeutig
    // halten (belegt → naechster freier Zaehler) und wie ein IDE-Refactoring
    // in jede Switcher-Liste nachtragen
    if (neu.empty()) neu = effectTypeName(n.params);
    const std::string alt = n.displayName;
    const std::string frei = freierHostGruppenName(m_host->chain(), neu, &n);
    n.displayName = frei;
    if (alt != frei) (void)hostGruppeUmbenennenInSwitchern(m_host->chain(), alt, frei);
}

void MultiEffectPanel::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (m_updating || item == nullptr) return;
    const QList<int> path = pathFromVariant(item->data(0, Qt::UserRole));
    if (column == 0)
    {
        // Column 0 is the editable name (enable lives on the eye toggle now).
        const std::string name = item->text(0).toStdString();
        mutate(path, [&](ChainNode& n) { umbenennenLocked(n, name); },
               /*aendertVoreinstellung=*/false);
    }
    else if (column == 3)
    {
        const std::string desc = item->text(3).toStdString();
        mutate(path, [&](ChainNode& n) { n.description = desc; },
               /*aendertVoreinstellung=*/false);
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
    aktualisiereMindestbreite();
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

bool MultiEffectPanel::askPresetToSave(const QStringList& all, QString& outName,
                                       QStringList& outFields)
{
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

    // Ein Kaestchen je Feld; die Liste kommt generisch aus dem Serializer
    // (Knoten: `nodeToJson`, Milkdrop-Element: `waveToJson` …).
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

namespace
{
/// Zustandszeile der Preset-Combo (Index 0): ohne Namen „(none)", sonst die
/// bearbeitete Voreinstellung.
QString presetStatusText(const QString& geaenderteVoreinstellung)
{
    return geaenderteVoreinstellung.isEmpty()
               ? MultiEffectPanel::tr("(none)")
               : MultiEffectPanel::tr("(unsaved) – %1").arg(geaenderteVoreinstellung);
}

// --- Sicht einer Preset-Zeile (S83) -------------------------------------------
//
// Eine Preset-Zeile gilt dem ganzen Knoten (section < 0) oder einem Teil
// eines Milkdrop-Nodes: Sektion 1 = Custom Wave, 2 = Custom Shape, 4 = Sprite
// (`element` = Index im jeweiligen Vektor, Zaehlung wie splitMilkPath) oder
// die Sektionen 0 = Code und 5 = Parameter (`element` = -1).

/// Typ-Schluessel der Sicht (leer = kein Preset-Pfad).
QString sichtTypKey(const EffectParams& params, int section)
{
    namespace np = lumi::multieffect::nodepresets;
    switch (section)
    {
    case -1: return effectTypeKey(params);
    case 1: return QString::fromLatin1(np::kMilkWave);
    case 2: return QString::fromLatin1(np::kMilkShape);
    case 4: return QString::fromLatin1(np::kMilkSprite);
    case 0: return QString::fromLatin1(np::kMilkCode);
    case 5: return QString::fromLatin1(np::kMilkParams);
    default: return {};
    }
}

/// Felder der Milkdrop-Sektionen Code (0) und Parameter (5) aus dem
/// Preset-Dokument. Parameter = alle skalaren Felder AUSSER Code, Shadern,
/// Element-Listen und Meta — also Basiswerte, Basis-Waveform, Gestaltung,
/// „bisher"-Schalter, Regelwerk.
QJsonObject milkSektionLesen(const lumi::milkdrop::PresetState& preset, int section)
{
    const QJsonObject alles =
        lumi::milkdrop::presetToJson(preset).value(QStringLiteral("preset")).toObject();
    static const QStringList kCode = {QStringLiteral("perFrameInit"),
                                      QStringLiteral("perFrame"), QStringLiteral("perPixel")};
    QJsonObject o;
    if (section == 0)
    {
        for (const QString& k : kCode) o[k] = alles.value(k);
        return o;
    }
    static const QStringList kKeineParameter = {
        QStringLiteral("warpShader"), QStringLiteral("compShader"), QStringLiteral("waves"),
        QStringLiteral("shapes"),     QStringLiteral("sprites"),    QStringLiteral("name"),
        QStringLiteral("generation"), QStringLiteral("psVersion")};
    o = alles;
    for (const QString& k : kCode + kKeineParameter) o.remove(k);
    return o;
}

/// Die Felder der Sicht, OHNE Rahmen (Knoten: type/name/description/enabled/
/// children, Element: index/enabled) — genau das, was eine Datei traegt.
/// Leer = das Element gibt es (nicht mehr).
QJsonObject sichtLesen(const ChainNode& n, int section, int element)
{
    namespace md = lumi::milkdrop;
    QJsonObject o;
    if (section < 0)
    {
        o = nodeToJson(n);
        for (const QString& rahmen : lumi::multieffect::nodepresets::knotenRahmen())
            o.remove(rahmen);
        return o;
    }
    const auto* mp = std::get_if<MilkdropNodeParams>(&n.params);
    if (mp == nullptr) return {};
    if (section == 0 || section == 5) return milkSektionLesen(mp->preset, section);
    if (element < 0) return {};
    const auto e = static_cast<std::size_t>(element);
    if (section == 1 && e < mp->preset.waves.size()) o = md::waveToJson(mp->preset.waves[e]);
    else if (section == 2 && e < mp->preset.shapes.size())
        o = md::shapeToJson(mp->preset.shapes[e]);
    else if (section == 4 && e < mp->preset.sprites.size())
        o = md::spriteToJson(mp->preset.sprites[e]);
    else return {};
    o.remove(QStringLiteral("index"));
    o.remove(QStringLiteral("enabled"));
    return o;
}

/// Schreibt `felder` in die Sicht zurueck; der Rahmen bleibt, wie er ist.
void sichtSchreiben(ChainNode& n, int section, int element, const QJsonObject& felder,
                    QStringList* report)
{
    namespace md = lumi::milkdrop;
    if (section < 0)
    {
        QJsonObject o = felder;
        o[QStringLiteral("type")] = effectTypeKey(n.params);
        n.params = nodeFromJson(o, report).params;
        return;
    }
    auto* mp = std::get_if<MilkdropNodeParams>(&n.params);
    if (mp == nullptr) return;
    if (section == 0 || section == 5)
    {
        // Ueber das ganze Preset-Dokument: die Felder der Sektion darueber,
        // alles andere (Waves, Shader, Name …) reist unveraendert mit.
        QJsonObject doc = md::presetToJson(mp->preset);
        QJsonObject pr = doc.value(QStringLiteral("preset")).toObject();
        for (auto it = felder.constBegin(); it != felder.constEnd(); ++it)
            pr[it.key()] = it.value();
        doc[QStringLiteral("preset")] = pr;
        mp->preset = md::presetFromJson(doc, report);
        ++mp->revision;
        return;
    }
    if (element < 0) return;
    const auto e = static_cast<std::size_t>(element);
    const auto ueber = [&felder](QJsonObject o) {
        for (auto it = felder.constBegin(); it != felder.constEnd(); ++it)
            o[it.key()] = it.value();
        return o;
    };
    if (section == 1 && e < mp->preset.waves.size())
        mp->preset.waves[e] = md::waveFromJson(ueber(md::waveToJson(mp->preset.waves[e])));
    else if (section == 2 && e < mp->preset.shapes.size())
        mp->preset.shapes[e] = md::shapeFromJson(ueber(md::shapeToJson(mp->preset.shapes[e])));
    else if (section == 4 && e < mp->preset.sprites.size())
        mp->preset.sprites[e] =
            md::spriteFromJson(ueber(md::spriteToJson(mp->preset.sprites[e])));
    else return;
    ++mp->revision;  // wie milkOf: der Renderer uebernimmt nur neue Revisionen
}

QString markenSchluessel(std::uint64_t nodeId, int section, int element)
{
    return QStringLiteral("%1/%2/%3").arg(nodeId).arg(section).arg(element);
}
}  // namespace

void MultiEffectPanel::markierePresetGeaendert(const QString& schluessel)
{
    auto it = m_presetMarken.find(schluessel);
    if (it == m_presetMarken.end() || it->geaendert) return;
    it->geaendert = true;
    if (m_presetCombo == nullptr || m_presetComboSchluessel != schluessel) return;
    const QSignalBlocker blocker(m_presetCombo.data());
    m_presetCombo->setItemText(0, presetStatusText(it->name));
    m_presetCombo->setCurrentIndex(0);
}

void MultiEffectPanel::addPresetRow(QFormLayout* form, const QList<int>& path,
                                    const QList<int>& rawPath, const EffectParams& params,
                                    int section, int element)
{
    namespace np = lumi::multieffect::nodepresets;
    const QString typeKey = sichtTypKey(params, section);
    if (typeKey.isEmpty()) return;

    std::uint64_t nodeId = 0;
    {
        QMutexLocker lock(m_mutex);
        const ChainNode* node = nodeAtPath(path);
        if (node == nullptr) return;
        // Basis-Welle & Co. sind keine Elemente mit eigenem Feldsatz.
        if (section >= 0 && sichtLesen(*node, section, element).isEmpty()) return;
        nodeId = node->nodeId;
    }
    const QString schluessel = markenSchluessel(nodeId, section, element);
    // Neu aufbauen ueber den VOLLEN Baum-Pfad — sonst verliesse der Editor
    // nach dem Laden die Element-Ansicht.
    const auto neuAufbauen = [this, rawPath] {
        QMetaObject::invokeMethod(
            this, [this, rawPath] { buildPropertyEditor(rawPath); }, Qt::QueuedConnection);
    };

    auto* row = new QWidget(m_propContainer);
    auto* hl = new QHBoxLayout(row);
    hl->setContentsMargins(0, 0, 0, 0);

    const std::vector<np::Entry> entries = np::list(typeKey);
    const auto eintragNamens = [entries](const QString& name) {
        if (name.isEmpty()) return -1;
        for (std::size_t i = 0; i < entries.size(); ++i)
            if (entries[i].name == name) return static_cast<int>(i);
        return -1;
    };
    // Marke der Sicht (S83): eine inzwischen geloeschte Voreinstellung
    // vergessen, sonst zeigte die Combo einen Namen ohne Datei.
    PresetMarke marke = m_presetMarken.value(schluessel);
    const int markeIdx = eintragNamens(marke.name);
    if (markeIdx < 0 && !marke.name.isEmpty())
    {
        m_presetMarken.remove(schluessel);
        marke = {};
    }

    auto* combo = new QComboBox(row);
    combo->setObjectName(QStringLiteral("knotenPreset"));
    // Index 0 = Zustand ohne passenden Eintrag: „(none)" oder, sobald eine
    // geladene Voreinstellung bearbeitet wurde, „(unsaved) – Name".
    combo->addItem(presetStatusText(marke.geaendert ? marke.name : QString()));
    for (const np::Entry& e : entries)
        combo->addItem(e.builtin ? e.name : e.name + QStringLiteral(" *"));
    {
        const QSignalBlocker blocker(combo);
        combo->setCurrentIndex(markeIdx >= 0 && !marke.geaendert ? markeIdx + 1 : 0);
    }
    m_presetCombo = combo;
    m_presetComboSchluessel = schluessel;
    combo->setToolTip(section < 0
                          ? tr("Named parameter sets for this node type (* = your own). "
                               "Loading replaces every stored field including the scripts.")
                          : tr("Named parameter sets for this element type (* = your own). "
                               "Loading replaces every stored field including the scripts."));

    auto* saveBtn = new QToolButton(row);
    // Symbol statt „Save as…" (S78): die Zeile muss auf kFeldMinBreite
    // schrumpfen koennen — Combo + zwei Symbolknoepfe passen, Text nicht.
    saveBtn->setText(QString::fromUtf8("💾"));
    saveBtn->setToolTip(tr("Save as… — store the current parameters under a name"));
    auto* delBtn = new QToolButton(row);
    delBtn->setObjectName(QStringLiteral("knotenPresetLoeschen"));
    delBtn->setText(QString::fromUtf8("🗑"));
    delBtn->setToolTip(tr("Delete the selected preset (your own only)"));
    // Ziel des Loeschens: der gewaehlte Eintrag, bei „(unsaved) – Name" die
    // Voreinstellung, von der der Stand stammt.
    const auto zielEintrag = [this, combo, schluessel, eintragNamens]() {
        const int i = combo->currentIndex() - 1;
        return i >= 0 ? i : eintragNamens(m_presetMarken.value(schluessel).name);
    };
    const auto loeschbar = [entries](int i) {
        return i >= 0 && i < static_cast<int>(entries.size()) &&
               !entries[static_cast<std::size_t>(i)].builtin;
    };
    delBtn->setEnabled(loeschbar(zielEintrag()));

    hl->addWidget(combo, 1);
    hl->addWidget(saveBtn);
    hl->addWidget(delBtn);

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this, path, section, element, nodeId, schluessel, typeKey, entries, delBtn,
             zielEintrag, loeschbar, neuAufbauen](int idx) {
                delBtn->setEnabled(loeschbar(zielEintrag()));
                const int i = idx - 1;  // Index 0 ist die Zustandszeile
                if (i < 0 || i >= static_cast<int>(entries.size())) return;
                const np::Entry& e = entries[static_cast<std::size_t>(i)];

                // Frisch aus dem Modell: die Datei wird DARUEBER gelegt, ein
                // Teil-Preset (Figur) laesst den Rest der Felder stehen.
                QJsonObject felder;
                {
                    QMutexLocker lock(m_mutex);
                    const ChainNode* node = nodeAtPath(path);
                    if (node == nullptr) return;
                    felder = sichtLesen(*node, section, element);
                }
                QStringList report;
                if (!np::loadJson(e.path, typeKey, felder, &report))
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
                mutate(path, [&](ChainNode& n) {
                    sichtSchreiben(n, section, element, felder, &report);
                });
                // Nach mutate: das hat die Marke eben als geaendert gesetzt.
                m_presetMarken[schluessel] = PresetMarke{e.name, false, nodeId, section, element};
                // Die Werte haben sich alle geaendert — Editor neu aufbauen (nicht
                // aus dem Signal heraus, sonst stirbt das Widget unter uns).
                neuAufbauen();
            });

    connect(saveBtn, &QToolButton::clicked, this,
            [this, path, section, element, nodeId, schluessel, typeKey, neuAufbauen]() {
                // Frisch aus dem Modell lesen: der Schnappschuss des Editors ist
                // vom Aufbau, zwischenzeitliche Reglerbewegungen fehlen darin.
                QJsonObject felder;
                {
                    QMutexLocker lock(m_mutex);
                    const ChainNode* node = nodeAtPath(path);
                    if (node == nullptr) return;
                    felder = sichtLesen(*node, section, element);
                }
                QStringList alle = felder.keys();
                alle.sort(Qt::CaseInsensitive);

                QString name;
                QStringList fields;
                if (!askPresetToSave(alle, name, fields)) return;

                QString written;
                if (!np::saveJson(name, typeKey, felder, fields, &written))
                {
                    QMessageBox::warning(this, tr("Preset"),
                                         tr("Preset '%1' could not be saved.").arg(name));
                    return;
                }
                // Der Dateiname ist bereinigt (sanitize) — die Combo listet ihn so.
                m_presetMarken[schluessel] = PresetMarke{
                    QFileInfo(written).completeBaseName(), false, nodeId, section, element};
                neuAufbauen();
            });

    connect(delBtn, &QToolButton::clicked, this,
            [this, schluessel, entries, zielEintrag, loeschbar, neuAufbauen]() {
                const int i = zielEintrag();
                if (!loeschbar(i)) return;
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
                if (m_presetMarken.value(schluessel).name == e.name)
                    m_presetMarken.remove(schluessel);
                neuAufbauen();
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
    std::string nodeSong;
    bool nodeScriptsActive = true;
    {
        QMutexLocker lock(m_mutex);
        ChainNode* node = nodeAtPath(path);
        if (node == nullptr) return;
        params = node->params;
        nodeName = node->displayName;
        nodeDesc = node->description;
        nodeSong = node->songCode;
        nodeScriptsActive = node->scriptsActive;
    }

    m_propPage = new QWidget(m_propContainer);
    auto* form = new QFormLayout(m_propPage);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    // Beschriftung und Feld bleiben nebeneinander; ist das Dock zu schmal,
    // scrollt man von den Beschriftungen zu den Feldern (S78, Vorgabe Patrik).
    // Aussenrand kommt von m_propLayout, nicht von hier.
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setContentsMargins(0, 0, 0, 0);

    // Voreinstellungen (Knoten-Parameter-Konzept §3, Etappe 1): eine Zeile fuer
    // JEDEN Knotentyp — der Store arbeitet ueber effectTypeKey + nodeToJson und
    // kennt keinen einzigen Typ beim Namen. Nur in der echten Knotenansicht:
    // Milkdrop-Sektionen/-Elemente sind Navigations-Items ohne eigene params.
    if (milkSection < 0 && milkElem < 0) addPresetRow(form, path, rawPath, params);
    // Milkdrop-Elemente (S83, Entscheid Patrik E3a): eigene Typ-Schluessel
    // milkWave/milkShape/milkSprite — nur in der Einzel-Ansicht.
    else if (milkElem >= 0) addPresetRow(form, path, rawPath, params, milkSection, milkElem);
    // Milkdrop-Sektionen Code und Parameter (S83, Wunsch Patrik): per_frame-/
    // per_pixel-Vorlagen und Stellungen wie Gestaltung/„bisher"-Schalter.
    else if (milkSection == 0 || milkSection == 5)
        addPresetRow(form, path, rawPath, params, milkSection, -1);

    // Editable name + free-text description (commit on Enter / focus-out).
    auto addLine = [&](const QString& label, const QString& value,
                       std::function<void(ChainNode&, std::string)> set) {
        auto* edit = new QLineEdit(value, m_propPage);
        connect(edit, &QLineEdit::editingFinished, this, [this, path, set, edit]() {
            const std::string t = edit->text().toStdString();
            // Name/Beschreibung: nicht Teil einer Knoten-Voreinstellung
            mutate(path, [&](ChainNode& n) { set(n, t); },
                   /*aendertVoreinstellung=*/false);
        });
        form->addRow(label, edit);
    };
    addLine(tr("Name"), QString::fromStdString(nodeName),
            [this](ChainNode& n, std::string v) { umbenennenLocked(n, v); });
    addLine(tr("Description"), QString::fromStdString(nodeDesc),
            [](ChainNode& n, std::string v) { n.description = std::move(v); });

    // Schalter „Script active" (S83, Entscheid Patrik E1a): EIN Feld am Knoten
    // fuer alle Typen mit EEL-Slots. Die Effect List behaelt ihr „Use list
    // code" (AVS-Feld), Milkdrop hat seine Skripte im Preset.
    if (milkSection < 0 && milkElem < 0 && hatSkriptSchalter(params))
    {
        auto* box = new QCheckBox(m_propContainer);
        box->setObjectName(QStringLiteral("scriptsActive"));
        box->setChecked(nodeScriptsActive);
        const QString tip =
            tr("Off: the node runs as if its script slots (Init/Frame/Beat/Point, "
               "Song) were empty. The code stays stored; shaders are not affected.");
        box->setToolTip(tip);
        connect(box, &QCheckBox::toggled, this, [this, path](bool v) {
            // Rahmen des Knotens, nicht Teil einer Voreinstellung
            mutate(path, [v](ChainNode& n) { n.scriptsActive = v; },
                   /*aendertVoreinstellung=*/false);
        });
        form->addRow(tr("Script active"), box);
        if (QWidget* l = form->labelForField(box)) l->setToolTip(tip);
    }

    // --- row helpers (each connects to a typed mutation) ---------------------
    //
    // Jede Feldzeile nennt ihren FELDNAMEN als erstes Argument (§10). Er ist
    // der Schluessel in die erzeugte Tabelle `FieldDocs.cpp`, deren Texte aus
    // den Doxygen-Kommentaren an den `…Params`-Feldern stammen — der Text steht
    // damit an genau einer Stelle, und ein Feld ohne Erklaerung faellt dem
    // Waechter auf, nicht dem Benutzer. Der Name ist bewusst nicht das Label:
    // Labels sind uebersetzt und frei formuliert.
    const QString fieldTypeKey = effectTypeKey(params);
    auto addRow = [&](const char* field, const QString& label, QWidget* widget,
                      const QString& explicitTip = QString()) {
        const QString tip =
            explicitTip.isEmpty()
                ? fielddocs::tooltip(fieldTypeKey, QLatin1String(field))
                : explicitTip;
        if (!tip.isEmpty()) widget->setToolTip(tip);
        // Feldname als objectName (S83): Tests und Sonden finden das Feld
        // ueber den Vertragsnamen statt ueber die Position im Formular.
        if (widget->objectName().isEmpty()) widget->setObjectName(QLatin1String(field));
        form->addRow(label, widget);
        // Der Hinweis gehoert auch an die Beschriftung — wer den Namen nicht
        // versteht, faehrt zuerst dorthin, nicht auf das Bedienelement.
        if (!tip.isEmpty())
            if (QWidget* l = form->labelForField(widget)) l->setToolTip(tip);
    };
    auto addInt = [&](const char* field, const QString& label, int value, int lo,
                      int hi, std::function<void(ChainNode&, int)> set,
                      const QString& tip = QString()) {
        auto* spin = new QSpinBox(m_propContainer);
        spin->setRange(lo, hi);
        spin->setValue(value);
        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
                [this, path, set](int v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        addRow(field, label, spin, tip);
    };
    auto addDouble = [&](const char* field, const QString& label, double value,
                         double lo, double hi, double step,
                         std::function<void(ChainNode&, double)> set,
                         const QString& tip = QString()) {
        auto* spin = new QDoubleSpinBox(m_propContainer);
        spin->setRange(lo, hi);
        spin->setSingleStep(step);
        spin->setDecimals(3);
        spin->setValue(value);
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this, path, set](double v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        addRow(field, label, spin, tip);
    };
    // `after` laeuft NACH der Mutation ausserhalb des Locks — fuer UI-Nachzug
    // wie syncMilkEyes (SSOT: Baum-Auge folgt der Panel-Checkbox, S76).
    auto addBool = [&](const char* field, const QString& label, bool value,
                       std::function<void(ChainNode&, bool)> set,
                       const QString& tip = QString(),
                       std::function<void()> after = {}) {
        auto* box = new QCheckBox(m_propContainer);
        box->setChecked(value);
        connect(box, &QCheckBox::toggled, this, [this, path, set, after](bool v) {
            mutate(path, [&](ChainNode& n) { set(n, v); });
            if (after) after();
        });
        addRow(field, label, box, tip);
    };
    auto addColor = [&](const char* field, const QString& label, uint32_t value,
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
        addRow(field, label, btn);
    };
    // Wie addDouble, aber fuer KLASSE-A-Werte: die Vorgabe ist der Wert des
    // Referenz-Renderers, jede Abweichung entfernt das Bild messbar von AVS.
    // Die Zeile sagt das (Entscheid Patrik §8.4) — sonst sieht man einem Preset
    // spaeter nicht an, warum es in der Kalibrierung ausschert.
    auto addRefDouble = [&](const char* field, const QString& label, double value,
                            double lo, double hi, double step, double reference,
                            int decimals,
                            std::function<void(ChainNode&, double)> set) {
        auto* spin = new QDoubleSpinBox(m_propContainer);
        spin->setRange(lo, hi);
        spin->setSingleStep(step);
        spin->setDecimals(decimals);
        spin->setValue(value);
        auto* lbl = new QLabel(label, m_propContainer);
        const auto mark = [lbl, label, reference](double v) {
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
        // Die Beschriftung traegt hier den Referenz-Hinweis, der sich mit dem
        // Wert aendert (⚠) — der §10-Text kommt deshalb nur an den Regler.
        const QString tip = fielddocs::tooltip(fieldTypeKey, QLatin1String(field));
        if (!tip.isEmpty()) spin->setToolTip(tip);
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
    auto addEnum = [&](const char* field, const QString& label, int index,
                       const QStringList& items,
                       std::function<void(ChainNode&, int)> set) {
        auto* combo = new QComboBox(m_propContainer);
        combo->addItems(items);
        combo->setCurrentIndex(index);
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this, path, set](int v) {
                    mutate(path, [&](ChainNode& n) { set(n, v); });
                });
        addRow(field, label, combo);
    };
    // Farbtafel: ein Farbfeld je Eintrag plus Hinzufuegen/Entfernen. Der Zugriff
    // auf den Vektor kommt als Funktion herein, damit sich JEDER Knoten mit
    // Palette dieselbe Zeile teilt (SuperScope, Metaballs 3D, Tentacles 3D) —
    // Laenge aendern heisst Editor neu bauen, deshalb der verzoegerte Aufruf.
    auto addColorTable = [&](const char* field, const QString& label,
                             const std::vector<uint32_t>& colors,
                             std::function<std::vector<uint32_t>&(ChainNode&)> access,
                             const QString& toolTip) {
        auto* colorsWidget = new QWidget(m_propContainer);
        // Raster statt Zeile (S78): N Farbfelder in EINER Zeile machten die
        // Seite mit jeder Farbe breiter. Je kFarbenJeZeile ein Umbruch, die
        // +/−-Knoepfe in einer eigenen Zeile darunter.
        constexpr int kFarbenJeZeile = 4;  // 4x24 + 3x3 < kFeldMinBreite
        auto* grid = new QGridLayout(colorsWidget);
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(3);
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
            grid->addWidget(sw, ci / kFarbenJeZeile, ci % kFarbenJeZeile);
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
        auto* knoepfe = new QHBoxLayout();
        knoepfe->addWidget(addC);
        knoepfe->addWidget(delC);
        knoepfe->addStretch();
        const int zeilen = (static_cast<int>(colors.size()) + kFarbenJeZeile - 1) /
                           kFarbenJeZeile;
        grid->addLayout(knoepfe, zeilen, 0, 1, kFarbenJeZeile);
        grid->setColumnStretch(kFarbenJeZeile, 1);  // Felder links buendig
        addRow(field, label, colorsWidget, toolTip);
    };
    // Milkdrop-Sektionen tauschen die Referenz unten gegen ihr Original-Set
    // aus (Befund S42: eine generische Tabelle passte zu keinem Slot richtig)
    QString scriptRef = scriptReferenceHtml(params);
    const QHash<QString, lumi::scriptedit::SymCat> modulKat = modulKategorien(params);

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

    // Herkunft eines Fremd-Imports (S72) — EINE Zeile fuer JEDEN importfaehigen
    // Knoten, nicht mehr nur fuer Shadertoy. Reine Anzeige: die Felder gehoeren
    // dem Import, von Hand aendert man sie nicht. Ohne sichtbare Lizenz wandern
    // fremde Shader unbemerkt in weitergegebene `.lvfx` (Vorgabe Patrik S71).
    auto addHerkunft = [&](const Herkunft& h) {
        if (h.leer()) return;
        const QString name = h.name.empty() ? tr("(untitled)")
                                            : QString::fromStdString(h.name);
        const QString autor = h.author.empty() ? tr("unknown")
                                               : QString::fromStdString(h.author);
        const QString lizenz = h.license.empty()
                                   ? tr("unknown")
                                   : QString::fromStdString(h.license);
        QString text = tr("%1 by %2 — licence: %3").arg(name, autor, lizenz);
        if (!h.url.empty())
            text += QStringLiteral("\n") + QString::fromStdString(h.url);
        auto* meta = new QLabel(text, m_propContainer);
        meta->setWordWrap(true);
        meta->setTextInteractionFlags(Qt::TextBrowserInteraction);
        meta->setToolTip(tr("Origin of the imported content. This licence applies "
                            "when passing the preset on."));
        form->addRow(meta);
    };

    auto addScript = [&](const char* field, const QString& label,
                         const std::string& value,
                         std::function<void(ChainNode&, std::string)> set) {
        const bool isInit = label.startsWith(QLatin1String("Init"));
        const QSet<QString> conflicts = isInit ? initConflicts : QSet<QString>{};
        // S85: die ⓘ-Seite beginnt mit dem Takt DIESES Slots
        const QString slotRef =
            slotScopeHtml(fieldTypeKey, QLatin1String(field), label) + scriptRef;
        auto* edit = new QPlainTextEdit(m_propContainer);
        edit->setPlainText(QString::fromStdString(value));
        edit->setPlaceholderText(tr("EEL expression"));
        edit->setLineWrapMode(QPlainTextEdit::NoWrap);
        edit->setMinimumHeight(54);
        edit->setMaximumHeight(200);  // taller inline; full editing via Expand
        QFont mono(QStringLiteral("Consolas"));
        mono.setStyleHint(QFont::Monospace);
        edit->setFont(mono);
        new EelHighlighter(edit->document(), conflicts, modulKat);
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
                [this, slotRef]() { showScriptReference(this, slotRef); });
        // Groß-Editor mit Apply/Beautify (S69): Apply schreibt ins Inline-Feld
        // (textChanged -> mutate -> recompileChain) und probt die EEL-Syntax
        // synchron im Dialekt des Knotens (Milkdrop vs. AVS — eel.div-Grenze).
        const bool milkDialekt = fieldTypeKey == QLatin1String("milkdrop");
        connect(expandBtn, &QToolButton::clicked, this,
                [this, edit, label, slotRef, conflicts, milkDialekt, modulKat]() {
                    lumi::scriptedit::ScriptEditorHooks hooks;
                    hooks.modulKategorien = modulKat;
                    hooks.apply = [edit, milkDialekt](const QString& t) -> QString {
                        edit->setPlainText(t);  // textChanged -> mutate
                        const auto probe = lumi::eel::transpile(
                            t.toStdString(), milkDialekt
                                                 ? lumi::eel::Dialect::Milkdrop
                                                 : lumi::eel::Dialect::Avs);
                        return probe.ok ? QString()
                                        : QString::fromStdString(probe.error);
                    };
                    hooks.beautify = [](const QString& t) {
                        return QString::fromStdString(lumi::scripting::beautifyEel(
                            t.toStdString(),
                            lumi::scriptedit::formatOptionsFromSettings()));
                    };
                    QString out;
                    if (openScriptEditor(this, label, edit->toPlainText(), slotRef,
                                         out, conflicts, /*eelHighlight=*/true,
                                         hooks))
                    {
                        edit->setPlainText(out);  // triggers textChanged -> mutate
                    }
                });

        auto* box = new QWidget(m_propContainer);
        auto* v = new QVBoxLayout(box);
        v->setContentsMargins(0, 0, 0, 0);
        v->addWidget(header);
        v->addWidget(edit);
        // Eine Skriptzeile baut ihre Beschriftung selbst (mit ⓘ/⤢) und belegt
        // die ganze Breite — es gibt hier keine Label-Spalte, an die `addRow`
        // den Hinweis haengen koennte. Also direkt auf den Kasten: Feld und
        // Ueberschrift zeigen ihn dann beide.
        const QString tip = fielddocs::tooltip(fieldTypeKey, QLatin1String(field));
        if (!tip.isEmpty()) box->setToolTip(tip);
        form->addRow(box);
    };
    auto addGradient = [&](const char* field, const QString& label,
                           const std::string& value,
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
        addRow(field, label, combo);
    };
    auto addText = [&](const char* field, const QString& label,
                       const std::string& value,
                       std::function<void(ChainNode&, std::string)> set,
                       const QString& tip = QString()) {
        auto* edit = new QLineEdit(QString::fromStdString(value), m_propContainer);
        connect(edit, &QLineEdit::editingFinished, this, [this, path, set, edit]() {
            const std::string t = edit->text().toStdString();
            mutate(path, [&](ChainNode& n) { set(n, t); });
        });
        addRow(field, label, edit, tip);
    };
    // Fremdsprachen-Code-Editor (GLSL, Strang S): wie addScript mit ⓘ-Referenz
    // und ⤢-Groß-Editor, aber OHNE EEL-Highlighter (falsche Einfärbung) und
    // mit frei wählbarer Referenzseite + Tooltip. `exportName` (S69) =
    // Dateinamens-Vorschlag `preset_name.modul.glsl` für Import…/Export…
    // `vorlagenKey` (S83) = `<vertrag>[/<stufe>]`: der Teil vor dem `/` ist der
    // Shader-Vertrag, das Ganze der Schluessel der Shader-Vorlagen.
    auto addCodeEditor = [&](const QString& label, const std::string& value,
                             const QString& placeholder, const QString& refHtml,
                             const QString& toolTip, int minHeight,
                             const QString& exportName,
                             const QString& vorlagenKey,
                             std::function<void(ChainNode&, std::string)> set) {
        auto* edit = new QPlainTextEdit(m_propContainer);
        edit->setPlainText(QString::fromStdString(value));
        edit->setPlaceholderText(placeholder);
        edit->setLineWrapMode(QPlainTextEdit::NoWrap);
        edit->setMinimumHeight(minHeight);
        edit->setMaximumHeight(minHeight * 3);
        QFont mono(QStringLiteral("Consolas"));
        mono.setStyleHint(QFont::Monospace);
        edit->setFont(mono);
        connect(edit, &QPlainTextEdit::textChanged, this, [this, path, set, edit]() {
            const std::string text = edit->toPlainText().toStdString();
            mutate(path, [&](ChainNode& n) { set(n, text); });
        });
        auto* header = new QWidget(m_propContainer);
        auto* hl = new QHBoxLayout(header);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->addWidget(new QLabel(label, header));
        hl->addStretch(1);
        auto* refBtn = new QToolButton(header);
        refBtn->setText(QString::fromUtf8("ⓘ"));
        refBtn->setToolTip(tr("Show reference (uniforms, audio, buffers)"));
        auto* expandBtn = new QToolButton(header);
        expandBtn->setText(QString::fromUtf8("⤢"));
        expandBtn->setToolTip(tr("Open in large, resizable editor"));
        hl->addWidget(refBtn);
        hl->addWidget(expandBtn);
        connect(refBtn, &QToolButton::clicked, this,
                [this, refHtml]() { showScriptReference(this, refHtml); });
        // Groß-Editor mit Apply/Beautify (S69): GLSL kompiliert erst im
        // Render-Thread — der Dialog pollt den Treiber-Fehler des Knotens
        // nach (#line 1: Zeilennummern des Nutzer-Codes, wie im Panel).
        connect(expandBtn, &QToolButton::clicked, this,
                [this, edit, label, refHtml, path, exportName, vorlagenKey]() {
                    lumi::scriptedit::ScriptEditorHooks hooks;
                    hooks.exportFileName = exportName;
                    hooks.vertragKey = vorlagenKey.section(QLatin1Char('/'), 0, 0);
                    hooks.vorlagenKey = vorlagenKey;
                    // Herkunft JETZT holen, nicht beim Aufbau der Zeile: ein
                    // zwischenzeitlicher Import soll im Export stehen (S72).
                    hooks.herkunftKopf = herkunftKopfFuer(path);
                    hooks.apply = [edit](const QString& t) -> QString {
                        edit->setPlainText(t);  // textChanged -> mutate
                        return {};
                    };
                    hooks.pollError = [this, path]() -> QString {
                        if (m_host == nullptr || m_mutex == nullptr) return {};
                        uint64_t nodeId = 0;
                        {
                            QMutexLocker lock(m_mutex);
                            if (ChainNode* n = nodeAtPath(path)) nodeId = n->nodeId;
                        }
                        return QString::fromStdString(m_host->shadertoyError(nodeId))
                            .left(600);
                    };
                    hooks.beautify = [](const QString& t) {
                        return QString::fromStdString(lumi::scripting::beautifyGlsl(
                            t.toStdString(),
                            lumi::scriptedit::formatOptionsFromSettings()));
                    };
                    QString out;
                    if (openScriptEditor(this, label, edit->toPlainText(), refHtml,
                                         out, {}, /*eelHighlight=*/false, hooks))
                    {
                        edit->setPlainText(out);  // triggers textChanged -> mutate
                    }
                });
        auto* box = new QWidget(m_propContainer);
        auto* v = new QVBoxLayout(box);
        v->setContentsMargins(0, 0, 0, 0);
        v->addWidget(header);
        v->addWidget(edit);
        if (!toolTip.isEmpty()) box->setToolTip(toolTip);
        form->addRow(box);
    };

    const QStringList kBlendNames = {"Ignore", "Replace", "50/50", "Maximum",
                                     "Additive", "Sub 1-2", "Sub 2-1", "EveryLine",
                                     "EveryPixel", "XOR", "Adjustable", "Multiply",
                                     "Buffer", "Minimum"};

    // --- per-type fields -----------------------------------------------------
    if (auto* p = std::get_if<ListParams>(&params))
    {
        addBool("clearEveryFrame", tr("Clear each frame"), p->clearEveryFrame,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).clearEveryFrame = v; });
        addEnum("blendIn", tr("Blend In"), static_cast<int>(p->blendIn), kBlendNames,
                [](ChainNode& n, int v) { std::get<ListParams>(n.params).blendIn = static_cast<BlendMode>(v); });
        addEnum("blendOut", tr("Blend Out"), static_cast<int>(p->blendOut), kBlendNames,
                [](ChainNode& n, int v) { std::get<ListParams>(n.params).blendOut = static_cast<BlendMode>(v); });
        addInt("inAdjustAlpha", tr("In Alpha"), p->inAdjustAlpha, 0, 255,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).inAdjustAlpha = v; });
        addInt("outAdjustAlpha", tr("Out Alpha"), p->outAdjustAlpha, 0, 255,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).outAdjustAlpha = v; });
        // Only relevant when the matching blend is "Buffer": pool slot + invert.
        addInt("bufferIn", tr("In Buffer"), p->bufferIn, 0, 7,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).bufferIn = v; });
        addInt("bufferOut", tr("Out Buffer"), p->bufferOut, 0, 7,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).bufferOut = v; });
        addBool("bufferInInvert", tr("In Buffer invert"), p->bufferInInvert,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).bufferInInvert = v; });
        addBool("bufferOutInvert", tr("Out Buffer invert"), p->bufferOutInvert,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).bufferOutInvert = v; });
        addBool("onBeatRender", tr("OnBeat render"), p->onBeatRender,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).onBeatRender = v; });
        addInt("onBeatFrames", tr("OnBeat frames"), p->onBeatFrames, 1, 200,
               [](ChainNode& n, int v) { std::get<ListParams>(n.params).onBeatFrames = v; });
        addBool("useCode", tr("Use list code"), p->useCode,
                [](ChainNode& n, bool v) { std::get<ListParams>(n.params).useCode = v; });
        addScript("initCode", tr("Init code"), p->initCode,
                  [](ChainNode& n, std::string v) { std::get<ListParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode,
                  [](ChainNode& n, std::string v) { std::get<ListParams>(n.params).frameCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ClearParams>(&params))
    {
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<ClearParams>(n.params).color = v; });
        addBool("onlyFirst", tr("Only first frame"), p->onlyFirst, [](ChainNode& n, bool v) { std::get<ClearParams>(n.params).onlyFirst = v; });
        const QStringList kClearBlendNames = {tr("Replace"), tr("Additive"),
                                              tr("50/50"), tr("Line blend")};
        addEnum("blend", tr("Blend"), p->blend, kClearBlendNames, [](ChainNode& n, int v) { std::get<ClearParams>(n.params).blend = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ClearParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ClearParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ClearParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FadeoutParams>(&params))
    {
        addInt("fadeLen", tr("Fade length"), p->fadeLen, 0, 92, [](ChainNode& n, int v) { std::get<FadeoutParams>(n.params).fadeLen = v; });
        addColor("color", tr("Target color"), p->color, [](ChainNode& n, uint32_t v) { std::get<FadeoutParams>(n.params).color = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<FadeoutParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FadeoutParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FadeoutParams>(n.params).beatCode = std::move(v); });
    }
    else if (std::holds_alternative<InvertParams>(params))
    {
        form->addRow(new QLabel(tr("(no parameters)"), m_propContainer));
    }
    else if (auto* p = std::get_if<BrightnessParams>(&params))
    {
        addInt("red", tr("Red"), p->red, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).red = v; });
        addInt("green", tr("Green"), p->green, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).green = v; });
        addInt("blue", tr("Blue"), p->blue, -4096, 4096, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).blue = v; });
        addBool("exclude", tr("Exclude color"), p->exclude, [](ChainNode& n, bool v) { std::get<BrightnessParams>(n.params).exclude = v; });
        addColor("color", tr("Exclude"), p->color, [](ChainNode& n, uint32_t v) { std::get<BrightnessParams>(n.params).color = v; });
        addInt("distance", tr("Distance"), p->distance, 0, 255, [](ChainNode& n, int v) { std::get<BrightnessParams>(n.params).distance = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BrightnessParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BrightnessParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BrightnessParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FastBrightnessParams>(&params))
    {
        addEnum("dir", tr("Mode"), p->dir, {"x2", "x0.5", "off"}, [](ChainNode& n, int v) { std::get<FastBrightnessParams>(n.params).dir = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<FastBrightnessParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FastBrightnessParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FastBrightnessParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BlurParams>(&params))
    {
        addEnum("strength", tr("Strength"), p->strength - 1, {"Light", "Normal", "Heavy"},
                [](ChainNode& n, int v) { std::get<BlurParams>(n.params).strength = v + 1; });
        addBool("roundUp", tr("Round up"), p->roundUp, [](ChainNode& n, bool v) { std::get<BlurParams>(n.params).roundUp = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BlurParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BlurParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BlurParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MirrorParams>(&params))
    {
        auto setModeBit = [](int bit) {
            return [bit](ChainNode& n, bool v) {
                int& m = std::get<MirrorParams>(n.params).mode;
                m = v ? (m | bit) : (m & ~bit);
            };
        };
        addBool("mode", tr("Top -> Bottom"), (p->mode & 1) != 0, setModeBit(1));
        addBool("mode", tr("Bottom -> Top"), (p->mode & 2) != 0, setModeBit(2));
        addBool("mode", tr("Left -> Right"), (p->mode & 4) != 0, setModeBit(4));
        addBool("mode", tr("Right -> Left"), (p->mode & 8) != 0, setModeBit(8));
        addBool("onBeatRandom", tr("OnBeat random"), p->onBeatRandom, [](ChainNode& n, bool v) { std::get<MirrorParams>(n.params).onBeatRandom = v; });
        addBool("smooth", tr("Smooth transition"), p->smooth, [](ChainNode& n, bool v) { std::get<MirrorParams>(n.params).smooth = v; });
        addInt("slower", tr("Slower (frames/step)"), p->slower, 1, 16, [](ChainNode& n, int v) { std::get<MirrorParams>(n.params).slower = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MirrorParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MirrorParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MirrorParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<OnBeatClearParams>(&params))
    {
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<OnBeatClearParams>(n.params).color = v; });
        addInt("everyNBeats", tr("Every N beats"), p->everyNBeats, 1, 100, [](ChainNode& n, int v) { std::get<OnBeatClearParams>(n.params).everyNBeats = v; });
        addBool("blend", tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<OnBeatClearParams>(n.params).blend = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<OnBeatClearParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<OnBeatClearParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<OnBeatClearParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ColorfadeParams>(&params))
    {
        addInt("faderR", tr("Fader R"), p->faderR, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderR = v; });
        addInt("faderG", tr("Fader G"), p->faderG, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderG = v; });
        addInt("faderB", tr("Fader B"), p->faderB, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).faderB = v; });
        addInt("beatFaderR", tr("Beat Fader R"), p->beatFaderR, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderR = v; });
        addInt("beatFaderG", tr("Beat Fader G"), p->beatFaderG, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderG = v; });
        addInt("beatFaderB", tr("Beat Fader B"), p->beatFaderB, -32, 32, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).beatFaderB = v; });
        addInt("onBeatFrames", tr("Beat frames"), p->onBeatFrames, 1, 200, [](ChainNode& n, int v) { std::get<ColorfadeParams>(n.params).onBeatFrames = v; });
        addBool("slowFade", tr("Slow fade"), p->slowFade, [](ChainNode& n, bool v) { std::get<ColorfadeParams>(n.params).slowFade = v; });
        addBool("onBeatRandom", tr("Random on beat"), p->onBeatRandom, [](ChainNode& n, bool v) { std::get<ColorfadeParams>(n.params).onBeatRandom = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorfadeParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorfadeParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorfadeParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ColorModifierParams>(&params))
    {
        addBool("recompute", tr("Recompute/frame"), p->recompute, [](ChainNode& n, bool v) { std::get<ColorModifierParams>(n.params).recompute = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).beatCode = std::move(v); });
        addScript("levelCode", tr("Level"), p->levelCode, [](ChainNode& n, std::string v) { std::get<ColorModifierParams>(n.params).levelCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MovementParams>(&params))
    {
        addBool("rectCoords", tr("Rect coords"), p->rectCoords, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).rectCoords = v; });
        addBool("wrap", tr("Wrap"), p->wrap, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).wrap = v; });
        addBool("blend", tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).blend = v; });
        addBool("subpixel", tr("Bilinear filtering"), p->subpixel,
                [](ChainNode& n, bool v) { std::get<MovementParams>(n.params).subpixel = v; },
                tr("Intermediate values when sampling — in the preset the field is "
                   "called `subpixel`, the AVS dialog shows the same label. "
                   "Off: the nearest pixel, hard edges. On: the "
                   "four neighbours blended like AVS BLEND4, smooth transitions — "
                   "visible mainly when stretching and squeezing."));
        const QStringList kSourceMappedNames = {tr("Off"), tr("On (scatter)"),
                                                tr("Off, toggle on beat"),
                                                tr("On, toggle on beat")};
        addEnum("sourceMapped", tr("Source mapped"), p->sourceMapped, kSourceMappedNames, [](ChainNode& n, int v) { std::get<MovementParams>(n.params).sourceMapped = v; });
        // r_trans-Builtins OHNE eval_desc: reine Pixel-Index-Umlegungen, für die
        // es keine d/r-Formel gibt. Ist einer gewählt, wird der Point-Code nicht
        // ausgewertet (S35).
        addEnum("builtinRemap", tr("Builtin remap"), p->builtinRemap == 7 ? 2 : p->builtinRemap,
                {tr("Off (use code)"), tr("Slight fuzzify"), tr("Blocky partial out")},
                [](ChainNode& n, int v) {
                    std::get<MovementParams>(n.params).builtinRemap =
                        v == 2 ? 7 : v;  // 0 · 1 · 7 sind die echten Werte
                });
        addScript("code", tr("Point code"), p->code, [](ChainNode& n, std::string v) { std::get<MovementParams>(n.params).code = std::move(v); });
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
        addInt("xres", tr("Grid X"), p->xres, 2, 256, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).xres = v; });
        addInt("yres", tr("Grid Y"), p->yres, 2, 256, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).yres = v; });
        addBool("rectCoords", tr("Rect coords"), p->rectCoords, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).rectCoords = v; });
        addBool("wrap", tr("Wrap"), p->wrap, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).wrap = v; });
        addBool("blend", tr("Blend (alpha)"), p->blend, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).blend = v; });
        addBool("nomove", tr("No move"), p->nomove, [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).nomove = v; });
        addBool("subpixel", tr("Bilinear filtering"), p->subpixel,
                [](ChainNode& n, bool v) { std::get<DynamicMovementParams>(n.params).subpixel = v; },
                tr("Intermediate values when sampling — in the preset the field is "
                   "called `subpixel`, the AVS dialog shows the same label. "
                   "Off: the nearest pixel, hard edges. On: the "
                   "four neighbours blended like AVS BLEND4, smooth transitions — "
                   "visible mainly when stretching and squeezing."));
        addInt("buffern", tr("Source buffer (0 = frame)"), p->buffern, 0, 8, [](ChainNode& n, int v) { std::get<DynamicMovementParams>(n.params).buffern = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).beatCode = std::move(v); });
        addScript("pointCode", tr("Point"), p->pointCode, [](ChainNode& n, std::string v) { std::get<DynamicMovementParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DynamicShiftParams>(&params))
    {
        addBool("blend", tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<DynamicShiftParams>(n.params).blend = v; });
        addBool("subpixel", tr("Bilinear filtering"), p->subpixel,
                [](ChainNode& n, bool v) { std::get<DynamicShiftParams>(n.params).subpixel = v; },
                tr("Intermediate values when sampling — in the preset the field is "
                   "called `subpixel`, the AVS dialog shows the same label. "
                   "Off: the nearest pixel, hard edges. On: the "
                   "four neighbours blended like AVS BLEND4, smooth transitions — "
                   "visible mainly when stretching and squeezing."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame (x,y shift)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DynamicShiftParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DynamicDistanceModifierParams>(&params))
    {
        addBool("blend", tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<DynamicDistanceModifierParams>(n.params).blend = v; });
        addBool("subpixel", tr("Bilinear filtering"), p->subpixel,
                [](ChainNode& n, bool v) { std::get<DynamicDistanceModifierParams>(n.params).subpixel = v; },
                tr("Intermediate values when sampling — in the preset the field is "
                   "called `subpixel`, the AVS dialog shows the same label. "
                   "Off: the nearest pixel, hard edges. On: the "
                   "four neighbours blended like AVS BLEND4, smooth transitions — "
                   "visible mainly when stretching and squeezing."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).beatCode = std::move(v); });
        addScript("pixelCode", tr("Pixel (d in/out)"), p->pixelCode, [](ChainNode& n, std::string v) { std::get<DynamicDistanceModifierParams>(n.params).pixelCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MovingParticleParams>(&params))
    {
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<MovingParticleParams>(n.params).color = v; });
        addInt("maxDistance", tr("Max distance"), p->maxDistance, 1, 256, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).maxDistance = v; });
        addInt("size", tr("Size"), p->size, 1, 128, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).size = v; });
        addInt("size2", tr("On-beat size"), p->size2, 1, 128, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).size2 = v; });
        addBool("onBeatSize", tr("Resize on beat"), p->onBeatSize, [](ChainNode& n, bool v) { std::get<MovingParticleParams>(n.params).onBeatSize = v; });
        addRefDouble("spring", tr("Spring"), p->spring, 0.0, 0.1, 0.001, 0.004, 4, [](ChainNode& n, double v) { std::get<MovingParticleParams>(n.params).spring = static_cast<float>(v); });
        addRefDouble("damping", tr("Damping"), p->damping, 0.8, 1.0, 0.001, 0.991, 4, [](ChainNode& n, double v) { std::get<MovingParticleParams>(n.params).damping = static_cast<float>(v); });
        addReferenceNote(!p->initCode.empty() || !p->frameCode.empty() || !p->beatCode.empty());
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MovingParticleParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MovingParticleParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MovingParticleParams>(n.params).beatCode = std::move(v); });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50", "Line"}, [](ChainNode& n, int v) { std::get<MovingParticleParams>(n.params).blend = v; });
    }
    else if (auto* p = std::get_if<ColorMapParams>(&params))
    {
        addEnum("key", tr("Input"), p->key, {"Red", "Green", "Blue", "(R+G+B)/2", "Max", "(R+G+B)/3"}, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).key = v; });
        addEnum("blendMode", tr("Blend"), p->blendMode, {"Replace", "Additive", "Maximum", "Minimum", "50/50", "Sub d-s", "Sub s-d", "Multiply", "XOR", "Adjustable"}, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).blendMode = v; });
        addInt("adjustBlend", tr("Adjustable"), p->adjustBlend, 0, 255, [](ChainNode& n, int v) { std::get<ColorMapParams>(n.params).adjustBlend = v; });
        addGradientStops(form, path, p->stopPos, p->stopColor);
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorMapParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorMapParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorMapParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BufferBlendParams>(&params))
    {
        const QStringList bufs{"Buffer 1", "Buffer 2", "Buffer 3", "Buffer 4", "Buffer 5", "Buffer 6", "Buffer 7", "Buffer 8", "Current"};
        addEnum("bufferA", tr("Buffer A"), p->bufferA, bufs, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).bufferA = v; });
        addEnum("bufferB", tr("Buffer B"), p->bufferB, bufs, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).bufferB = v; });
        addEnum("mode", tr("Mode"), p->mode, {"Replace", "Additive", "Maximum", "50/50", "Sub A-B", "Sub B-A", "Multiply", "Adjustable", "XOR", "Minimum", "Abs diff"}, [](ChainNode& n, int v) { std::get<BufferBlendParams>(n.params).mode = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BufferBlendParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BufferBlendParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BufferBlendParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<JherikoGlobalParams>(&params))
    {
        form->addRow(new QLabel(tr("The variable-set module: declare preset-global "
                                   "variables (reg00…reg99, gmegabuf) in Init; other "
                                   "effects can read them. Re-declaring a global that "
                                   "another node's Init already sets is flagged red."),
                                m_propContainer));
        addEnum("loadMode", tr("Load"), p->loadMode, {"None", "Once", "Code control", "Every frame"}, [](ChainNode& n, int v) { std::get<JherikoGlobalParams>(n.params).loadMode = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<JherikoGlobalParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<JherikoGlobalParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<JherikoGlobalParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ColorClipParams>(&params))
    {
        addEnum("mode", tr("Mode"), p->mode - 1, {"Below", "Above", "Near"}, [](ChainNode& n, int v) { std::get<ColorClipParams>(n.params).mode = v + 1; });
        addColor("clipColor", tr("Threshold"), p->clipColor, [](ChainNode& n, uint32_t v) { std::get<ColorClipParams>(n.params).clipColor = v; });
        addColor("outColor", tr("Replace with"), p->outColor, [](ChainNode& n, uint32_t v) { std::get<ColorClipParams>(n.params).outColor = v; });
        addInt("distance", tr("Distance"), p->distance, 0, 255, [](ChainNode& n, int v) { std::get<ColorClipParams>(n.params).distance = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorClipParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorClipParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorClipParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<UniqueToneParams>(&params))
    {
        addColor("color", tr("Tone"), p->color, [](ChainNode& n, uint32_t v) { std::get<UniqueToneParams>(n.params).color = v; });
        addBool("invert", tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<UniqueToneParams>(n.params).invert = v; });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<UniqueToneParams>(n.params).blend = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<UniqueToneParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<UniqueToneParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<UniqueToneParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<InterleaveParams>(&params))
    {
        addInt("x", tr("X spacing"), p->x, 0, 512, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).x = v; });
        addInt("y", tr("Y spacing"), p->y, 0, 512, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).y = v; });
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<InterleaveParams>(n.params).color = v; });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).blend = v; });
        addBool("onBeat", tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<InterleaveParams>(n.params).onBeat = v; });
        addInt("x2", tr("Beat X"), p->x2, 0, 512, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).x2 = v; });
        addInt("y2", tr("Beat Y"), p->y2, 0, 512, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).y2 = v; });
        addInt("beatDuration", tr("Beat duration"), p->beatDuration, 1, 100, [](ChainNode& n, int v) { std::get<InterleaveParams>(n.params).beatDuration = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<InterleaveParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<InterleaveParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<InterleaveParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ConvolutionParams>(&params))
    {
        addEnum("edgeMode", tr("Edge"), p->edgeMode, {"Extend", "Wrap"}, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).edgeMode = v; });
        addBool("absolute", tr("Absolute"), p->absolute, [](ChainNode& n, bool v) { std::get<ConvolutionParams>(n.params).absolute = v; });
        addBool("twoPass", tr("Two pass"), p->twoPass, [](ChainNode& n, bool v) { std::get<ConvolutionParams>(n.params).twoPass = v; });
        addInt("bias", tr("Bias"), p->bias, -255, 255, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).bias = v; });
        addInt("scale", tr("Scale"), p->scale, 1, 1000, [](ChainNode& n, int v) { std::get<ConvolutionParams>(n.params).scale = v; });
        addKernelGrid(form, path, p->kernel);
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ConvolutionParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ConvolutionParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ConvolutionParams>(n.params).beatCode = std::move(v); });
    }
    else if (std::holds_alternative<NormaliseParams>(params))
    {
        auto* info = new QLabel(tr("Auto-levels — no parameters"), m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);
    }
    else if (auto* p = std::get_if<MultiFilterParams>(&params))
    {
        addEnum("effect", tr("Effect"), p->effect, {"Chrome", "Double chrome", "Triple chrome", "Infinite root"}, [](ChainNode& n, int v) { std::get<MultiFilterParams>(n.params).effect = v; });
        addBool("onBeat", tr("On beat only"), p->onBeat, [](ChainNode& n, bool v) { std::get<MultiFilterParams>(n.params).onBeat = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MultiFilterParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MultiFilterParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MultiFilterParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<AddBordersParams>(&params))
    {
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<AddBordersParams>(n.params).color = v; });
        addInt("size", tr("Size"), p->size, 0, 200, [](ChainNode& n, int v) { std::get<AddBordersParams>(n.params).size = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<AddBordersParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<AddBordersParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<AddBordersParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<SimpleScopeParams>(&params))
    {
        addEnum("mode", tr("Mode"), p->mode,
                {"Solid analyzer", "Line analyzer", "Line scope", "Solid scope",
                 "Dot analyzer", "Dot scope"},
                [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).mode = v; });
        addEnum("channel", tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).channel = v; });
        addEnum("position", tr("Position"), p->position, {"Top", "Bottom", "Center"}, [](ChainNode& n, int v) { std::get<SimpleScopeParams>(n.params).position = v; });
        addColorTable("colors", 
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<SimpleScopeParams>(n.params).colors;
            },
            tr("AVS color table — the scope cycles through these over time."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<SimpleScopeParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SimpleScopeParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SimpleScopeParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BassSpinParams>(&params))
    {
        addBool("left", tr("Left"), p->left, [](ChainNode& n, bool v) { std::get<BassSpinParams>(n.params).left = v; });
        addBool("right", tr("Right"), p->right, [](ChainNode& n, bool v) { std::get<BassSpinParams>(n.params).right = v; });
        addColor("colorLeft", tr("Left color"), p->colorLeft, [](ChainNode& n, uint32_t v) { std::get<BassSpinParams>(n.params).colorLeft = v; });
        addColor("colorRight", tr("Right color"), p->colorRight, [](ChainNode& n, uint32_t v) { std::get<BassSpinParams>(n.params).colorRight = v; });
        addEnum("mode", tr("Mode"), p->mode, {"Lines", "Filled"}, [](ChainNode& n, int v) { std::get<BassSpinParams>(n.params).mode = v; });
        addRefDouble("smoothing", tr("Smoothing"), p->smoothing, 0.0, 1.0, 0.05, 0.7, 3, [](ChainNode& n, double v) { std::get<BassSpinParams>(n.params).smoothing = static_cast<float>(v); });
        addRefDouble("spinStep", tr("Spin step"), p->spinStep, 0.0, 3.14, 0.01, static_cast<double>(3.14159f / 6.0f), 4, [](ChainNode& n, double v) { std::get<BassSpinParams>(n.params).spinStep = static_cast<float>(v); });
        addReferenceNote(!p->initCode.empty() || !p->frameCode.empty() || !p->beatCode.empty());
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BassSpinParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BassSpinParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BassSpinParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<OscStarParams>(&params))
    {
        addEnum("channel", tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).channel = v; });
        addEnum("position", tr("Position"), p->position, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).position = v; });
        addInt("size", tr("Size"), p->size, 0, 16, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).size = v; });
        addInt("rot", tr("Rotation"), p->rot, 0, 16, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).rot = v; });
        addInt("spokes", tr("Spokes"), p->spokes, 1, 64, [](ChainNode& n, int v) { std::get<OscStarParams>(n.params).spokes = v; });
        addDouble("rotScale", tr("Rotation scale"), p->rotScale, 0.0, 1.0, 0.005, [](ChainNode& n, double v) { std::get<OscStarParams>(n.params).rotScale = static_cast<float>(v); });
        addDouble("amplitude", tr("Amplitude"), p->amplitude, 0.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<OscStarParams>(n.params).amplitude = static_cast<float>(v); });
        addColorTable("colors", 
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<OscStarParams>(n.params).colors;
            },
            tr("AVS color table — cycled over time."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<OscStarParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<OscStarParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<OscStarParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<OscRingParams>(&params))
    {
        addEnum("source", tr("Source"), p->source, {"Waveform", "Spectrum"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).source = v; });
        addEnum("channel", tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).channel = v; });
        addEnum("position", tr("Position"), p->position, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).position = v; });
        addInt("size", tr("Size"), p->size, 0, 16, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).size = v; });
        addInt("segments", tr("Segments"), p->segments, 3, 1024, [](ChainNode& n, int v) { std::get<OscRingParams>(n.params).segments = v; });
        addDouble("baseScale", tr("Base radius"), p->baseScale, 0.0, 2.0, 0.05, [](ChainNode& n, double v) { std::get<OscRingParams>(n.params).baseScale = static_cast<float>(v); });
        addDouble("audioScale", tr("Audio radius"), p->audioScale, 0.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<OscRingParams>(n.params).audioScale = static_cast<float>(v); });
        addColorTable("colors", 
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<OscRingParams>(n.params).colors;
            },
            tr("AVS color table — cycled over time."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<OscRingParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<OscRingParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<OscRingParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<RotatingStarsParams>(&params))
    {
        addInt("points", tr("Points"), p->points, 2, 64, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).points = v; });
        addInt("skip", tr("Skip (2 = pentagram)"), p->skip, 1, 32, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).skip = v; });
        addInt("stars", tr("Stars"), p->stars, 1, 16, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).stars = v; });
        addDouble("rotSpeed", tr("Rotation speed"), p->rotSpeed, -1.0, 1.0, 0.01, [](ChainNode& n, double v) { std::get<RotatingStarsParams>(n.params).rotSpeed = static_cast<float>(v); });
        addDouble("orbit", tr("Orbit radius"), p->orbit, 0.0, 2.0, 0.05, [](ChainNode& n, double v) { std::get<RotatingStarsParams>(n.params).orbit = static_cast<float>(v); });
        addDouble("baseRadius", tr("Base size"), p->baseRadius, 0.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<RotatingStarsParams>(n.params).baseRadius = static_cast<float>(v); });
        addDouble("audioGain", tr("Audio gain"), p->audioGain, 0.0, 8.0, 0.05, [](ChainNode& n, double v) { std::get<RotatingStarsParams>(n.params).audioGain = static_cast<float>(v); });
        addInt("bandLo", tr("Band from"), p->bandLo, 0, 575, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).bandLo = v; });
        addInt("bandHi", tr("Band to (exclusive)"), p->bandHi, 1, 576, [](ChainNode& n, int v) { std::get<RotatingStarsParams>(n.params).bandHi = v; });
        addColorTable("colors", 
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<RotatingStarsParams>(n.params).colors;
            },
            tr("AVS color table — cycled over time."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<RotatingStarsParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<RotatingStarsParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<RotatingStarsParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<PictureParams>(&params))
    {
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<PictureParams>(n.params).blend = v; });
        addBool("keepAspect", tr("Keep aspect"), p->keepAspect, [](ChainNode& n, bool v) { std::get<PictureParams>(n.params).keepAspect = v; });
        addImageRow(form, path, p->filename, p->imageData,
                    [](ChainNode& n, std::string f, std::string d) {
                        auto& q = std::get<PictureParams>(n.params);
                        q.filename = std::move(f);
                        q.imageData = std::move(d);
                    });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<PictureParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<PictureParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<PictureParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<PictureIIParams>(&params))
    {
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<PictureIIParams>(n.params).blend = v; });
        addImageRow(form, path, p->filename, p->imageData,
                    [](ChainNode& n, std::string f, std::string d) {
                        auto& q = std::get<PictureIIParams>(n.params);
                        q.filename = std::move(f);
                        q.imageData = std::move(d);
                    });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<PictureIIParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<PictureIIParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<PictureIIParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<TexerParams>(&params))
    {
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive"}, [](ChainNode& n, int v) { std::get<TexerParams>(n.params).blend = v; });
        addInt("particles", tr("Particles"), p->particles, 1, 4096, [](ChainNode& n, int v) { std::get<TexerParams>(n.params).particles = v; });
        addImageRow(form, path, p->filename, p->imageData,
                    [](ChainNode& n, std::string f, std::string d) {
                        auto& q = std::get<TexerParams>(n.params);
                        q.filename = std::move(f);
                        q.imageData = std::move(d);
                    });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TexerParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TexerParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<TexerParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<TexerIIParams>(&params))
    {
        addBool("colorFiltering", tr("Color filtering"), p->colorFiltering, [](ChainNode& n, bool v) { std::get<TexerIIParams>(n.params).colorFiltering = v; });
        addBool("resizing", tr("Resizing"), p->resizing, [](ChainNode& n, bool v) { std::get<TexerIIParams>(n.params).resizing = v; });
        addBool("wrapAround", tr("Wrap around"), p->wrapAround, [](ChainNode& n, bool v) { std::get<TexerIIParams>(n.params).wrapAround = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).beatCode = std::move(v); });
        addScript("pointCode", tr("Point (x,y,sizex,sizey,rgb)"), p->pointCode, [](ChainNode& n, std::string v) { std::get<TexerIIParams>(n.params).pointCode = std::move(v); });
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
        addBool("filled", tr("Filled (AVS)"), p->filled, [](ChainNode& n, bool v) { std::get<TriangleParams>(n.params).filled = v; });
        addDouble("lineWidth", tr("Edge width (wireframe)"), p->lineWidth, 1.0, 20.0, 0.5, [](ChainNode& n, double v) { std::get<TriangleParams>(n.params).lineWidth = static_cast<float>(v); });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame (set n)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).beatCode = std::move(v); });
        addScript("pointCode", tr("Point (x1..y3,rgb)"), p->pointCode, [](ChainNode& n, std::string v) { std::get<TriangleParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BlitterFeedbackParams>(&params))
    {
        addInt("scale", tr("Scale (32 = off, <32 in, >32 out)"), p->scale, 0, 256, [](ChainNode& n, int v) { std::get<BlitterFeedbackParams>(n.params).scale = v; });
        addInt("scale2", tr("Beat scale"), p->scale2, 0, 256, [](ChainNode& n, int v) { std::get<BlitterFeedbackParams>(n.params).scale2 = v; });
        addBool("onBeat", tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).onBeat = v; });
        addBool("blend", tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).blend = v; });
        addBool("subpixel", tr("Bilinear filtering"), p->subpixel,
                [](ChainNode& n, bool v) { std::get<BlitterFeedbackParams>(n.params).subpixel = v; },
                tr("Intermediate values when sampling — in the preset the field is "
                   "called `subpixel`, the AVS dialog shows the same label. "
                   "Off: the nearest pixel, hard edges. On: the "
                   "four neighbours blended like AVS BLEND4, smooth transitions — "
                   "visible mainly when stretching and squeezing."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BlitterFeedbackParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BlitterFeedbackParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BlitterFeedbackParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<RotoBlitterParams>(&params))
    {
        addInt("zoomScale", tr("Zoom scale (31 = off)"), p->zoomScale, 0, 256, [](ChainNode& n, int v) { std::get<RotoBlitterParams>(n.params).zoomScale = v; });
        addInt("zoomScale2", tr("Beat zoom scale"), p->zoomScale2, 0, 256, [](ChainNode& n, int v) { std::get<RotoBlitterParams>(n.params).zoomScale2 = v; });
        addInt("rotDir", tr("Rotation (32 = off, deg/frame +32)"), p->rotDir, 0, 64, [](ChainNode& n, int v) { std::get<RotoBlitterParams>(n.params).rotDir = v; });
        addBool("blend", tr("Blend (50/50)"), p->blend, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).blend = v; });
        addBool("beatReverse", tr("Beat reverse"), p->beatReverse, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).beatReverse = v; });
        addInt("beatReverseSpeed", tr("Reverse speed"), p->beatReverseSpeed, 0, 8, [](ChainNode& n, int v) { std::get<RotoBlitterParams>(n.params).beatReverseSpeed = v; });
        addBool("beatZoomJump", tr("Beat zoom jump"), p->beatZoomJump, [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).beatZoomJump = v; });
        addBool("subpixel", tr("Bilinear filtering"), p->subpixel,
                [](ChainNode& n, bool v) { std::get<RotoBlitterParams>(n.params).subpixel = v; },
                tr("Intermediate values when sampling — in the preset the field is "
                   "called `subpixel`, the AVS dialog shows the same label. "
                   "Off: the nearest pixel, hard edges. On: the "
                   "four neighbours blended like AVS BLEND4, smooth transitions — "
                   "visible mainly when stretching and squeezing."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<RotoBlitterParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<RotoBlitterParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<RotoBlitterParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<BufferSaveParams>(&params))
    {
        addInt("slot", tr("Buffer slot"), p->slot, 0, 7, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).slot = v; });
        const QStringList kDirNames = {tr("Save"), tr("Restore"),
                                       tr("Alternate (save first)"),
                                       tr("Alternate (restore first)")};
        addEnum("dir", tr("Direction"), p->dir, kDirNames, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).dir = v; });
        addEnum("blend", tr("Blend"), static_cast<int>(p->blend), kBlendNames, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).blend = static_cast<BlendMode>(v); });
        addInt("adjustAlpha", tr("Blend alpha"), p->adjustAlpha, 0, 255, [](ChainNode& n, int v) { std::get<BufferSaveParams>(n.params).adjustAlpha = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BufferSaveParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BufferSaveParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BufferSaveParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<CustomBpmParams>(&params))
    {
        addBool("arbitrary", tr("Arbitrary"), p->arbitrary, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).arbitrary = v; });
        addInt("arbitraryMs", tr("Interval (ms)"), p->arbitraryMs, 1, 5000, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).arbitraryMs = v; });
        addBool("skip", tr("Skip"), p->skip, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).skip = v; });
        addInt("skipCount", tr("Skip count"), p->skipCount, 1, 16, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).skipCount = v; });
        addBool("invert", tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<CustomBpmParams>(n.params).invert = v; });
        // `skipfirst`: die ersten N Beats werden verschluckt, der Skip-Zaehler
        // laeuft dabei NICHT mit (r_bpm.cpp:148).
        addInt("skipFirst", tr("Skip first N beats"), p->skipFirst, 0, 64, [](ChainNode& n, int v) { std::get<CustomBpmParams>(n.params).skipFirst = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<CustomBpmParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<CustomBpmParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<CustomBpmParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<SuperScopeParams>(&params))
    {
        // Das frueher hier stehende „Figure"-Dropdown ist ENTFALLEN (S53): die
        // Figuren liegen jetzt als Dateien in `asset/nodepresets/superScope/`
        // und laufen ueber die allgemeine Zeile „Preset" ganz oben — eine
        // Voreinstellungs-Liste statt zweier nebeneinander (Vorgabe Patrik).
        addInt("pointCount", tr("Point count"), p->pointCount, 1, 4096, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).pointCount = v; });
        addEnum("renderMode", tr("Draw mode"), p->renderMode, {"Dots", "Lines", "Thick"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).renderMode = v; });
        addDouble("lineWidth", tr("Line width"), p->lineWidth, 1.0, 20.0, 0.5, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).lineWidth = static_cast<float>(v); });
        addDouble("dotSize", tr("Dot size"), p->dotSize, 1.0, 50.0, 1.0, [](ChainNode& n, double v) { std::get<SuperScopeParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum("audioChannel", tr("Channel"), p->audioChannel, {"Left", "Right", "Mono", "Mid", "Side"}, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).audioChannel = v; });
        addBool("spectrumSource", tr("Spectrum source (v)"), p->spectrumSource,
                [](ChainNode& n, bool v) { std::get<SuperScopeParams>(n.params).spectrumSource = v; });
        addEnum("lineBlend", tr("Line blend"), p->lineBlend,
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
        addInt("colorCycleFrames", tr("Cycle frames"), p->colorCycleFrames, 1, 600, [](ChainNode& n, int v) { std::get<SuperScopeParams>(n.params).colorCycleFrames = v; });
        // AVS color table: cycled over time in the "Color table"/blend modes.
        addColorTable("colors", 
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<SuperScopeParams>(n.params).colors;
            },
            tr("AVS color table — the scope cycles through these over time in "
               "the Color table / blend modes (one step every 'Cycle frames')."));

        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).beatCode = std::move(v); });
        addScript("pointCode", tr("Point"), p->pointCode, [](ChainNode& n, std::string v) { std::get<SuperScopeParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MosaicParams>(&params))
    {
        addInt("quality", tr("Quality"), p->quality, 1, 100, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).quality = v; });
        addInt("quality2", tr("OnBeat quality"), p->quality2, 1, 100, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).quality2 = v; });
        addBool("onBeat", tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<MosaicParams>(n.params).onBeat = v; });
        addInt("durationFrames", tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).durationFrames = v; });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<MosaicParams>(n.params).blend = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MosaicParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MosaicParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MosaicParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<GrainParams>(&params))
    {
        addInt("amount", tr("Amount"), p->amount, 0, 100, [](ChainNode& n, int v) { std::get<GrainParams>(n.params).amount = v; });
        addBool("staticGrain", tr("Static"), p->staticGrain, [](ChainNode& n, bool v) { std::get<GrainParams>(n.params).staticGrain = v; });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<GrainParams>(n.params).blend = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<GrainParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<GrainParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<GrainParams>(n.params).beatCode = std::move(v); });
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
        addInt("depth", tr("Depth"), p->depth, 0, 100, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).depth = v; });
        addBool("invert", tr("Invert"), p->invert, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).invert = v; });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).blend = v; });
        addBool("onBeat", tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).onBeat = v; });
        addInt("depth2", tr("Beat depth"), p->depth2, 0, 100, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).depth2 = v; });
        addInt("durationFrames", tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).durationFrames = v; });
        addBool("oldStyle", tr("Old-style x,y (0..100)"), p->oldStyle, [](ChainNode& n, bool v) { std::get<BumpParams>(n.params).oldStyle = v; });
        addInt("buffern", tr("Depth buffer (0 = frame)"), p->buffern, 0, 8, [](ChainNode& n, int v) { std::get<BumpParams>(n.params).buffern = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame (light x,y)"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BumpParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<WaterBumpParams>(&params))
    {
        addInt("density", tr("Density (damping)"), p->density, 1, 12, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).density = v; });
        addInt("depth", tr("Drop depth"), p->depth, 0, 2000, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).depth = v; });
        addDouble("displaceScale", tr("Refraction"), p->displaceScale, 0.0, 40.0, 0.5, [](ChainNode& n, double v) { std::get<WaterBumpParams>(n.params).displaceScale = static_cast<float>(v); });
        addBool("randomDrop", tr("Random drop"), p->randomDrop, [](ChainNode& n, bool v) { std::get<WaterBumpParams>(n.params).randomDrop = v; });
        addEnum("dropX", tr("Drop X"), p->dropX, {"Near", "Mid", "Far"}, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropX = v; });
        addEnum("dropY", tr("Drop Y"), p->dropY, {"Near", "Mid", "Far"}, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropY = v; });
        addInt("dropRadius", tr("Drop radius"), p->dropRadius, 1, 200, [](ChainNode& n, int v) { std::get<WaterBumpParams>(n.params).dropRadius = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<WaterBumpParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<WaterBumpParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<WaterBumpParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<InterferencesParams>(&params))
    {
        addInt("points", tr("Points"), p->points, 1, 8, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).points = v; });
        addInt("distance", tr("Distance"), p->distance, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).distance = v; });
        addInt("alpha", tr("Alpha"), p->alpha, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).alpha = v; });
        addInt("rotation", tr("Rotation"), p->rotation, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotation = v; });
        addInt("rotationInc", tr("Rotation/frame"), p->rotationInc, -64, 64, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotationInc = v; });
        addBool("rgb", tr("RGB split"), p->rgb, [](ChainNode& n, bool v) { std::get<InterferencesParams>(n.params).rgb = v; });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).blend = v; });
        addBool("onBeat", tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<InterferencesParams>(n.params).onBeat = v; });
        addInt("distance2", tr("Beat distance"), p->distance2, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).distance2 = v; });
        addInt("alpha2", tr("Beat alpha"), p->alpha2, 0, 255, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).alpha2 = v; });
        addInt("rotationInc2", tr("Beat rotation/frame"), p->rotationInc2, -64, 64, [](ChainNode& n, int v) { std::get<InterferencesParams>(n.params).rotationInc2 = v; });
        addDouble("speed", tr("Beat speed"), p->speed, 0.01, 2.0, 0.01, [](ChainNode& n, double v) { std::get<InterferencesParams>(n.params).speed = static_cast<float>(v); });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<InterferencesParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<InterferencesParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<InterferencesParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FyrewurXParams>(&params))
    {
        addInt("sparks", tr("Sparks per burst"), p->sparks, 1, 1024, [](ChainNode& n, int v) { std::get<FyrewurXParams>(n.params).sparks = v; });
        addDouble("speed", tr("Speed"), p->speed, 0.05, 5.0, 0.05, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).speed = static_cast<float>(v); });
        addDouble("gravity", tr("Gravity"), p->gravity, 0.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).gravity = static_cast<float>(v); });
        addDouble("lifeSeconds", tr("Life (s)"), p->lifeSeconds, 0.1, 10.0, 0.1, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).lifeSeconds = static_cast<float>(v); });
        addDouble("dotSize", tr("Spark size"), p->dotSize, 1.0, 32.0, 0.5, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).dotSize = static_cast<float>(v); });
        addDouble("hueDrift", tr("Hue drift"), p->hueDrift, 0.0, 6.0, 0.1, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).hueDrift = static_cast<float>(v); });
        addDouble("burstSpread", tr("Burst spread"), p->burstSpread, 0.0, 3.0, 0.05, [](ChainNode& n, double v) { std::get<FyrewurXParams>(n.params).burstSpread = static_cast<float>(v); });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<FyrewurXParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FyrewurXParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FyrewurXParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Metaballs3DParams>(&params))
    {
        // Verhaltens-Nachbau (S52): aus dem Preset kommt nur die Farbtafel, die
        // Geometrie ist host-eigen — also sind ALLE Zahlen hier frei verstellbar.
        // Grenzen wie im Renderer/Serialisierer (count 1..16).
        addInt("count", tr("Balls"), p->count, 1, 16, [](ChainNode& n, int v) { std::get<Metaballs3DParams>(n.params).count = v; });
        addDouble("radius", tr("Radius"), p->radius, 0.01, 1.0, 0.01, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).radius = static_cast<float>(v); });
        addDouble("speed", tr("Speed"), p->speed, 0.0, 5.0, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).speed = static_cast<float>(v); });
        addDouble("threshold", tr("Threshold"), p->threshold, 0.05, 8.0, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).threshold = static_cast<float>(v); });
        addEnum("blend", tr("Blend"), p->blend, {tr("Replace"), tr("Additive"), tr("50/50")}, [](ChainNode& n, int v) { std::get<Metaballs3DParams>(n.params).blend = v; });
        addDouble("spread", tr("Orbit spread"), p->spread, 0.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).spread = static_cast<float>(v); });
        addDouble("depth", tr("Depth"), p->depth, 0.2, 6.0, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).depth = static_cast<float>(v); });
        addDouble("phase", tr("Phase per ball"), p->phase, 0.0, 6.3, 0.05, [](ChainNode& n, double v) { std::get<Metaballs3DParams>(n.params).phase = static_cast<float>(v); });
        addColorTable("colors", 
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<Metaballs3DParams>(n.params).colors;
            },
            tr("Palette from the preset — ball i takes entry i (white if empty)."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<Metaballs3DParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Metaballs3DParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Metaballs3DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Tentacles3DParams>(&params))
    {
        addInt("count", tr("Tentacles"), p->count, 1, 16, [](ChainNode& n, int v) { std::get<Tentacles3DParams>(n.params).count = v; });
        addInt("segments", tr("Segments"), p->segments, 2, 256, [](ChainNode& n, int v) { std::get<Tentacles3DParams>(n.params).segments = v; });
        addDouble("length", tr("Length"), p->length, 0.05, 2.0, 0.05, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).length = static_cast<float>(v); });
        addDouble("thickness", tr("Thickness (px)"), p->thickness, 1.0, 64.0, 0.5, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).thickness = static_cast<float>(v); });
        addDouble("speed", tr("Speed"), p->speed, 0.0, 5.0, 0.05, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).speed = static_cast<float>(v); });
        addEnum("blend", tr("Blend"), p->blend, {tr("Replace"), tr("Additive"), tr("50/50")}, [](ChainNode& n, int v) { std::get<Tentacles3DParams>(n.params).blend = v; });
        addDouble("sway", tr("Sway"), p->sway, 0.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).sway = static_cast<float>(v); });
        addDouble("waves", tr("Waves"), p->waves, 0.0, 20.0, 0.1, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).waves = static_cast<float>(v); });
        addDouble("taper", tr("Taper (0 = even)"), p->taper, 0.0, 1.0, 0.05, [](ChainNode& n, double v) { std::get<Tentacles3DParams>(n.params).taper = static_cast<float>(v); });
        addColorTable("colors", 
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<Tentacles3DParams>(n.params).colors;
            },
            tr("Palette from the preset — tentacle i takes entry i (white if empty)."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<Tentacles3DParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Tentacles3DParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Tentacles3DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<StarfieldParams>(&params))
    {
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<StarfieldParams>(n.params).color = v; });
        addInt("maxStars", tr("Stars"), p->maxStars, 1, 8192, [](ChainNode& n, int v) { std::get<StarfieldParams>(n.params).maxStars = v; });
        addDouble("warpSpeed", tr("Warp speed"), p->warpSpeed, 0.1, 50.0, 0.5, [](ChainNode& n, double v) { std::get<StarfieldParams>(n.params).warpSpeed = static_cast<float>(v); });
        addBool("onBeat", tr("On beat"), p->onBeat, [](ChainNode& n, bool v) { std::get<StarfieldParams>(n.params).onBeat = v; });
        addDouble("beatSpeed", tr("Beat speed"), p->beatSpeed, 0.1, 50.0, 0.5, [](ChainNode& n, double v) { std::get<StarfieldParams>(n.params).beatSpeed = static_cast<float>(v); });
        addInt("durationFrames", tr("Duration (frames)"), p->durationFrames, 1, 200, [](ChainNode& n, int v) { std::get<StarfieldParams>(n.params).durationFrames = v; });
        const QStringList kStarBlendNames = {tr("Replace"), tr("Additive"), tr("50/50")};
        addEnum("blend", tr("Blend"), p->blend, kStarBlendNames, [](ChainNode& n, int v) { std::get<StarfieldParams>(n.params).blend = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<StarfieldParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<StarfieldParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<StarfieldParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<TimescopeParams>(&params))
    {
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<TimescopeParams>(n.params).color = v; });
        addInt("bands", tr("Bands"), p->bands, 1, 576, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).bands = v; });
        addEnum("channel", tr("Channel"), p->channel, {"Left", "Right", "Center"}, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).channel = v; });
        addBool("useChannel", tr("Apply channel"), p->useChannel, [](ChainNode& n, bool v) { std::get<TimescopeParams>(n.params).useChannel = v; });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50", "Line (SRM)"}, [](ChainNode& n, int v) { std::get<TimescopeParams>(n.params).blend = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<TimescopeParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<TimescopeParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<TimescopeParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DotGridParams>(&params))
    {
        addInt("spacing", tr("Spacing"), p->spacing, 2, 128, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).spacing = v; });
        addInt("xMove", tr("X move"), p->xMove, -1024, 1024, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).xMove = v; });
        addInt("yMove", tr("Y move"), p->yMove, -1024, 1024, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).yMove = v; });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50", "Line (SRM)"}, [](ChainNode& n, int v) { std::get<DotGridParams>(n.params).blend = v; });
        addColorTable("colors", 
            tr("Color table"), p->colors,
            [](ChainNode& n) -> std::vector<uint32_t>& {
                return std::get<DotGridParams>(n.params).colors;
            },
            tr("AVS color table — the dots cycle through these over time."));
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DotGridParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DotGridParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DotGridParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DotPlaneParams>(&params))
    {
        addInt("rotVel", tr("Rotation speed"), p->rotVel, -50, 50, [](ChainNode& n, int v) { std::get<DotPlaneParams>(n.params).rotVel = v; });
        addInt("angle", tr("Angle"), p->angle, -90, 90, [](ChainNode& n, int v) { std::get<DotPlaneParams>(n.params).angle = v; });
        addDouble("startRotation", tr("Start rotation"), p->startRotation, 0.0, 360.0, 1.0, [](ChainNode& n, double v) { std::get<DotPlaneParams>(n.params).startRotation = static_cast<float>(v); });
        addRefDouble("camDistance", tr("Camera distance"), p->camDistance, 50.0, 2000.0, 10.0, 400.0, 1, [](ChainNode& n, double v) { std::get<DotPlaneParams>(n.params).camDistance = static_cast<float>(v); });
        addRefDouble("settle", tr("Settle"), p->settle, 0.0, 2.0, 0.01, 0.15, 3, [](ChainNode& n, double v) { std::get<DotPlaneParams>(n.params).settle = static_cast<float>(v); });
        for (int i = 0; i < 5; ++i)
            addColor("colors", tr("Color %1").arg(i + 1), p->colors[i], [i](ChainNode& n, uint32_t v) { std::get<DotPlaneParams>(n.params).colors[i] = v; });
        addReferenceNote(!p->initCode.empty() || !p->frameCode.empty() || !p->beatCode.empty());
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DotPlaneParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DotPlaneParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DotPlaneParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DotFountainParams>(&params))
    {
        addInt("rotVel", tr("Rotation speed"), p->rotVel, -50, 50, [](ChainNode& n, int v) { std::get<DotFountainParams>(n.params).rotVel = v; });
        addInt("angle", tr("Angle"), p->angle, -90, 90, [](ChainNode& n, int v) { std::get<DotFountainParams>(n.params).angle = v; });
        addDouble("startRotation", tr("Start rotation"), p->startRotation, 0.0, 360.0, 1.0, [](ChainNode& n, double v) { std::get<DotFountainParams>(n.params).startRotation = static_cast<float>(v); });
        for (int i = 0; i < 5; ++i)
            addColor("colors", tr("Color %1").arg(i + 1), p->colors[i], [i](ChainNode& n, uint32_t v) { std::get<DotFountainParams>(n.params).colors[i] = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<DotFountainParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DotFountainParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DotFountainParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ChannelShiftParams>(&params))
    {
        addEnum("mode", tr("Mode"), p->mode, {"RGB", "RBG", "GBR", "GRB", "BRG", "BGR"}, [](ChainNode& n, int v) { std::get<ChannelShiftParams>(n.params).mode = v; });
        addBool("onBeat", tr("On beat (random)"), p->onBeat, [](ChainNode& n, bool v) { std::get<ChannelShiftParams>(n.params).onBeat = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ChannelShiftParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ChannelShiftParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ChannelShiftParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ColorReductionParams>(&params))
    {
        addInt("levels", tr("Levels (bits)"), p->levels, 1, 8, [](ChainNode& n, int v) { std::get<ColorReductionParams>(n.params).levels = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ColorReductionParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ColorReductionParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ColorReductionParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MultiplierParams>(&params))
    {
        addEnum("mode", tr("Factor"), p->mode, {"Saturate", "x8", "x4", "x2", "x0.5", "x0.25", "x0.125", "Keep"}, [](ChainNode& n, int v) { std::get<MultiplierParams>(n.params).mode = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MultiplierParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MultiplierParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MultiplierParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<VideoDelayParams>(&params))
    {
        addInt("delay", tr("Delay"), p->delay, 1, 128, [](ChainNode& n, int v) { std::get<VideoDelayParams>(n.params).delay = v; });
        addBool("useBeats", tr("In beats"), p->useBeats, [](ChainNode& n, bool v) { std::get<VideoDelayParams>(n.params).useBeats = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<VideoDelayParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<VideoDelayParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<VideoDelayParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<MultiDelayParams>(&params))
    {
        addEnum("mode", tr("Mode"), p->mode, {"None", "Input (write)", "Output (read)"}, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).mode = v; });
        addInt("buffer", tr("Buffer"), p->buffer, 0, 5, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).buffer = v; });
        addInt("delay", tr("Delay"), p->delay, 1, 128, [](ChainNode& n, int v) { std::get<MultiDelayParams>(n.params).delay = v; });
        addBool("useBeats", tr("In beats"), p->useBeats, [](ChainNode& n, bool v) { std::get<MultiDelayParams>(n.params).useBeats = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<MultiDelayParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<MultiDelayParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<MultiDelayParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Fractal2DParams>(&params))
    {
        addEnum("ftype", tr("Type"), p->type,
                {"Mandelbrot", "Julia", "Burning Ship", "Tricorn", "Multibrot",
                 "Newton", "Phoenix", "Magnet", "Nova"},
                [](ChainNode& n, int v) { std::get<Fractal2DParams>(n.params).type = v; });
        addDouble("centerX", tr("Center X"), p->centerX, -4.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).centerX = static_cast<float>(v); });
        addDouble("centerY", tr("Center Y"), p->centerY, -4.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).centerY = static_cast<float>(v); });
        addDouble("zoom", tr("Zoom"), p->zoom, 0.001, 100000.0, 0.1, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).zoom = static_cast<float>(v); });
        addDouble("rotation", tr("Rotation"), p->rotation, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).rotation = static_cast<float>(v); });
        addInt("maxIter", tr("Max iterations"), p->maxIter, 1, 2048, [](ChainNode& n, int v) { std::get<Fractal2DParams>(n.params).maxIter = v; });
        addDouble("juliaX", tr("Julia X"), p->juliaX, -2.0, 2.0, 0.001, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).juliaX = static_cast<float>(v); });
        addDouble("juliaY", tr("Julia Y"), p->juliaY, -2.0, 2.0, 0.001, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).juliaY = static_cast<float>(v); });
        addDouble("power", tr("Power"), p->power, 1.0, 16.0, 0.1, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).power = static_cast<float>(v); });
        addDouble("escapeR", tr("Escape R^2"), p->escapeR, 1.0, 1024.0, 1.0, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).escapeR = static_cast<float>(v); });
        addBool("smooth", tr("Smooth colour"), p->smooth, [](ChainNode& n, bool v) { std::get<Fractal2DParams>(n.params).smooth = v; });
        addDouble("colorScale", tr("Colour scale"), p->colorScale, 0.001, 4.0, 0.005, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble("colorCycle", tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal2DParams>(n.params).colorCycle = static_cast<float>(v); });
        addColor("insideColor", tr("Inside colour"), p->insideColor, [](ChainNode& n, uint32_t v) { std::get<Fractal2DParams>(n.params).insideColor = v; });
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
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<Fractal2DParams>(n.params).blend = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<Fractal2DParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Fractal2DParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Fractal2DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<DomainWarpParams>(&params))
    {
        addInt("octaves", tr("Octaves"), p->octaves, 1, 10, [](ChainNode& n, int v) { std::get<DomainWarpParams>(n.params).octaves = v; });
        addDouble("lacunarity", tr("Lacunarity"), p->lacunarity, 1.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).lacunarity = static_cast<float>(v); });
        addDouble("gain", tr("Gain"), p->gain, 0.0, 1.0, 0.02, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).gain = static_cast<float>(v); });
        addDouble("scale", tr("Scale"), p->scale, 0.1, 32.0, 0.1, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).scale = static_cast<float>(v); });
        addDouble("warp", tr("Warp"), p->warp, 0.0, 8.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).warp = static_cast<float>(v); });
        addDouble("warpScale", tr("Warp scale"), p->warpScale, 0.1, 8.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).warpScale = static_cast<float>(v); });
        addDouble("speed", tr("Speed"), p->speed, -4.0, 4.0, 0.02, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).speed = static_cast<float>(v); });
        addDouble("offsetX", tr("Offset X"), p->offsetX, -32.0, 32.0, 0.1, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).offsetX = static_cast<float>(v); });
        addDouble("offsetY", tr("Offset Y"), p->offsetY, -32.0, 32.0, 0.1, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).offsetY = static_cast<float>(v); });
        addDouble("colorScale", tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble("colorCycle", tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<DomainWarpParams>(n.params).colorCycle = static_cast<float>(v); });
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
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<DomainWarpParams>(n.params).blend = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<DomainWarpParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<DomainWarpParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<DomainWarpParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<SetRenderModeParams>(&params))
    {
        form->addRow(new QLabel(tr("Sets line width + blend for the render effects that follow."), m_propContainer));
        addBool("overrideBlend", tr("Override blend"), p->enabled, [](ChainNode& n, bool v) { std::get<SetRenderModeParams>(n.params).enabled = v; });
        addInt("lineWidth", tr("Line width"), p->lineWidth, 0, 255, [](ChainNode& n, int v) { std::get<SetRenderModeParams>(n.params).lineWidth = v; });
        addEnum("lineBlend", tr("Line blend"), p->lineBlend,
                {"Replace", "Additive", "Maximum", "50/50", "Sub (fb-c)",
                 "Sub (c-fb)", "Multiply", "Adjustable", "XOR (~Add)", "Minimum"},
                [](ChainNode& n, int v) { std::get<SetRenderModeParams>(n.params).lineBlend = v; });
        addInt("adjustAlpha", tr("Adjustable alpha"), p->adjustAlpha, 0, 255, [](ChainNode& n, int v) { std::get<SetRenderModeParams>(n.params).adjustAlpha = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<SetRenderModeParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SetRenderModeParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SetRenderModeParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<RenderScaleParams>(&params))
    {
        form->addRow(new QLabel(tr("Renders the whole chain at window/divisor and "
                                   "upscales — the classic Winamp pixel-doubling "
                                   "look (AVS effects use fixed pixel sizes)."),
                                m_propContainer));
        addInt("divisor", tr("Divisor (window / N)"), p->divisor, 1, 8, [](ChainNode& n, int v) { std::get<RenderScaleParams>(n.params).divisor = v; });
        addEnum("filter", tr("Upscale filter"), p->filter, {"Nearest (chunky)", "Linear (smooth)"},
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
        addBool("post", tr("Display only (post)"), p->post, [](ChainNode& n, bool v) { std::get<BloomParams>(n.params).post = v; });
        addInt("downsample", tr("Downsample (1/2^n)"), p->downsample, 0, 4, [](ChainNode& n, int v) { std::get<BloomParams>(n.params).downsample = v; });
        addInt("radius", tr("Radius (sigma px)"), p->radius, 1, 32, [](ChainNode& n, int v) { std::get<BloomParams>(n.params).radius = v; });
        addDouble("intensity", tr("Intensity"), p->intensity, 0.0, 8.0, 0.1, [](ChainNode& n, double v) { std::get<BloomParams>(n.params).intensity = static_cast<float>(v); });
        addDouble("threshold", tr("Threshold"), p->threshold, 0.0, 1.0, 0.05, [](ChainNode& n, double v) { std::get<BloomParams>(n.params).threshold = static_cast<float>(v); });
        addBool("vignette", tr("Vignette"), p->vignette, [](ChainNode& n, bool v) { std::get<BloomParams>(n.params).vignette = v; });
        addDouble("vignetteStrength", tr("Vignette strength"), p->vignetteStrength, 0.0, 1.0, 0.05, [](ChainNode& n, double v) { std::get<BloomParams>(n.params).vignetteStrength = static_cast<float>(v); });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<BloomParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<BloomParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<BloomParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Camera3DParams>(&params))
    {
        form->addRow(new QLabel(tr("Sets the 3D camera for the following 3D "
                                   "modules (per-frame state, like Set Render "
                                   "Mode). Scripts may override px..pz, tx..tz, "
                                   "fov, roll, fogstart, fogend."),
                                m_propContainer));

        addDouble("px", tr("Position X"), p->px, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).px = static_cast<float>(v); });
        addDouble("py", tr("Position Y"), p->py, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).py = static_cast<float>(v); });
        addDouble("pz", tr("Position Z"), p->pz, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).pz = static_cast<float>(v); });
        addDouble("tx", tr("Target X"), p->tx, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).tx = static_cast<float>(v); });
        addDouble("ty", tr("Target Y"), p->ty, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).ty = static_cast<float>(v); });
        addDouble("tz", tr("Target Z"), p->tz, -1000.0, 1000.0, 0.1, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).tz = static_cast<float>(v); });
        addDouble("fov", tr("FOV (deg)"), p->fov, 1.0, 179.0, 1.0, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).fov = static_cast<float>(v); });
        addDouble("roll", tr("Roll (deg)"), p->roll, -360.0, 360.0, 1.0, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).roll = static_cast<float>(v); });
        addDouble("fogStart", tr("Fog start"), p->fogStart, 0.0, 1000.0, 0.5, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).fogStart = static_cast<float>(v); });
        addDouble("fogEnd", tr("Fog end"), p->fogEnd, 0.0, 1000.0, 0.5, [](ChainNode& n, double v) { std::get<Camera3DParams>(n.params).fogEnd = static_cast<float>(v); });
        addColor("fogColor", tr("Fog color"), p->fogColor, [](ChainNode& n, uint32_t v) { std::get<Camera3DParams>(n.params).fogColor = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<Camera3DParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Camera3DParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Camera3DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<SuperScope3DParams>(&params))
    {
        form->addRow(new QLabel(tr("Scripted 3D point cloud: the point code "
                                   "writes x, y, z in WORLD units (y+ = up), "
                                   "plus size, red/green/blue, skip. Uses the "
                                   "active 3D camera (or the built-in default), "
                                   "drawn as additive soft sprites."),
                                m_propContainer));

        addInt("pointCount", tr("Points (n)"), p->pointCount, 1, 4096, [](ChainNode& n, int v) { std::get<SuperScope3DParams>(n.params).pointCount = v; });
        addEnum("renderMode", tr("Draw mode"), p->renderMode, {"Soft sprites", "Lines"},
                [](ChainNode& n, int v) { std::get<SuperScope3DParams>(n.params).renderMode = v; });
        addDouble("size", tr("Default size (world)"), p->size, 0.0001, 100.0, 0.01, [](ChainNode& n, double v) { std::get<SuperScope3DParams>(n.params).size = static_cast<float>(v); });
        addDouble("falloff", tr("Sprite falloff (k)"), p->falloff, 0.5, 32.0, 0.5, [](ChainNode& n, double v) { std::get<SuperScope3DParams>(n.params).falloff = static_cast<float>(v); });
        addEnum("audioChannel", tr("Audio channel (v)"), p->audioChannel, {"Left", "Right", "Mono"},
                [](ChainNode& n, int v) { std::get<SuperScope3DParams>(n.params).audioChannel = v; });
        addBool("spectrumSource", tr("v from spectrum"), p->spectrumSource, [](ChainNode& n, bool v) { std::get<SuperScope3DParams>(n.params).spectrumSource = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<SuperScope3DParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<SuperScope3DParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<SuperScope3DParams>(n.params).beatCode = std::move(v); });
        addScript("pointCode", tr("Point code"), p->pointCode, [](ChainNode& n, std::string v) { std::get<SuperScope3DParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Terrain3DParams>(&params))
    {
        form->addRow(new QLabel(tr("Heightfield after the \"Lights\" recipe: "
                                   "procedural base, spectrum as radial rings, "
                                   "spring relaxation. Scripts see the height "
                                   "grid as megabuf(gy*res+gx); the point code "
                                   "colors grid dots (i, gx, gy, h given)."),
                                m_propContainer));
        addInt("resolution", tr("Resolution"), p->resolution, 8, 128, [](ChainNode& n, int v) { std::get<Terrain3DParams>(n.params).resolution = v; });
        addDouble("extent", tr("Extent (world)"), p->extent, 0.1, 100.0, 0.5, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).extent = static_cast<float>(v); });
        addDouble("baseAmp", tr("Base amplitude"), p->baseAmp, 0.0, 10.0, 0.05, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).baseAmp = static_cast<float>(v); });
        addDouble("yOffset", tr("Y offset"), p->yOffset, -100.0, 100.0, 0.1, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).yOffset = static_cast<float>(v); });
        addDouble("ringAmp", tr("Spectrum rings"), p->ringAmp, 0.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).ringAmp = static_cast<float>(v); });
        addDouble("relax", tr("Relax (spring)"), p->relax, 0.0, 1.0, 0.02, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).relax = static_cast<float>(v); });
        addDouble("flatten", tr("Flatten"), p->flatten, 0.0, 1.0, 0.05, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).flatten = static_cast<float>(v); });
        addBool("drawMesh", tr("Draw mesh"), p->drawMesh, [](ChainNode& n, bool v) { std::get<Terrain3DParams>(n.params).drawMesh = v; });
        addColor("meshColor", tr("Mesh color"), p->meshColor, [](ChainNode& n, uint32_t v) { std::get<Terrain3DParams>(n.params).meshColor = v; });
        addBool("drawDots", tr("Draw dots"), p->drawDots, [](ChainNode& n, bool v) { std::get<Terrain3DParams>(n.params).drawDots = v; });
        addDouble("dotSize", tr("Dot size (world)"), p->dotSize, 0.0001, 10.0, 0.005, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).dotSize = static_cast<float>(v); });
        addDouble("falloff", tr("Sprite falloff (k)"), p->falloff, 0.5, 32.0, 0.5, [](ChainNode& n, double v) { std::get<Terrain3DParams>(n.params).falloff = static_cast<float>(v); });
        addColor("colorLow", tr("Color (valley)"), p->colorLow, [](ChainNode& n, uint32_t v) { std::get<Terrain3DParams>(n.params).colorLow = v; });
        addColor("colorHigh", tr("Color (peak)"), p->colorHigh, [](ChainNode& n, uint32_t v) { std::get<Terrain3DParams>(n.params).colorHigh = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<Terrain3DParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Terrain3DParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Terrain3DParams>(n.params).beatCode = std::move(v); });
        addScript("pointCode", tr("Point code"), p->pointCode, [](ChainNode& n, std::string v) { std::get<Terrain3DParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<GlowOrbsParams>(&params))
    {
        form->addRow(new QLabel(tr("Low-poly glow orbs with two-color gradient, "
                                   "beat flash and halo (the \"Lights\" orbs). "
                                   "Point code runs PER ORB (i = orb index) and "
                                   "may set x,y,z, radius, sx,sy,sz, red/green/"
                                   "blue, red2/green2/blue2, flash."),
                                m_propContainer));
        addInt("orbCount", tr("Orbs (n)"), p->orbCount, 1, 64, [](ChainNode& n, int v) { std::get<GlowOrbsParams>(n.params).orbCount = v; });
        addDouble("haloScale", tr("Halo scale"), p->haloScale, 1.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<GlowOrbsParams>(n.params).haloScale = static_cast<float>(v); });
        addDouble("haloIntensity", tr("Halo intensity"), p->haloIntensity, 0.0, 4.0, 0.1, [](ChainNode& n, double v) { std::get<GlowOrbsParams>(n.params).haloIntensity = static_cast<float>(v); });
        addDouble("falloff", tr("Halo falloff (k)"), p->falloff, 0.5, 32.0, 0.5, [](ChainNode& n, double v) { std::get<GlowOrbsParams>(n.params).falloff = static_cast<float>(v); });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<GlowOrbsParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<GlowOrbsParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<GlowOrbsParams>(n.params).beatCode = std::move(v); });
        addScript("pointCode", tr("Point code"), p->pointCode, [](ChainNode& n, std::string v) { std::get<GlowOrbsParams>(n.params).pointCode = std::move(v); });
    }
    else if (auto* p = std::get_if<Fractal3DParams>(&params))
    {
        addEnum("ftype", tr("Type"), p->type, {"Mandelbulb", "Mandelbox", "Menger", "Quaternion-Julia", "KIFS"}, [](ChainNode& n, int v) { std::get<Fractal3DParams>(n.params).type = v; });
        addDouble("yaw", tr("Yaw"), p->yaw, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).yaw = static_cast<float>(v); });
        addDouble("pitch", tr("Pitch"), p->pitch, -1.5, 1.5, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).pitch = static_cast<float>(v); });
        addDouble("dist", tr("Distance"), p->dist, 0.5, 12.0, 0.1, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).dist = static_cast<float>(v); });
        addDouble("fov", tr("FOV"), p->fov, 0.3, 3.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).fov = static_cast<float>(v); });
        addDouble("power", tr("Power"), p->power, 1.0, 16.0, 0.1, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).power = static_cast<float>(v); });
        addDouble("scale", tr("Scale"), p->scale, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).scale = static_cast<float>(v); });
        addDouble("fold", tr("Fold"), p->fold, 0.1, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).fold = static_cast<float>(v); });
        addInt("maxSteps", tr("Max steps"), p->maxSteps, 8, 512, [](ChainNode& n, int v) { std::get<Fractal3DParams>(n.params).maxSteps = v; });
        addInt("maxIter", tr("DE iterations"), p->maxIter, 1, 64, [](ChainNode& n, int v) { std::get<Fractal3DParams>(n.params).maxIter = v; });
        addDouble("juliaX", tr("Julia X"), p->juliaX, -2.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).juliaX = static_cast<float>(v); });
        addDouble("juliaY", tr("Julia Y"), p->juliaY, -2.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).juliaY = static_cast<float>(v); });
        addDouble("juliaZ", tr("Julia Z"), p->juliaZ, -2.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).juliaZ = static_cast<float>(v); });
        addDouble("juliaW", tr("Julia W"), p->juliaW, -2.0, 2.0, 0.01, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).juliaW = static_cast<float>(v); });
        addDouble("lightYaw", tr("Light yaw"), p->lightYaw, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).lightYaw = static_cast<float>(v); });
        addDouble("lightPitch", tr("Light pitch"), p->lightPitch, -1.5, 1.5, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).lightPitch = static_cast<float>(v); });
        addDouble("ambient", tr("Ambient"), p->ambient, 0.0, 1.0, 0.02, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).ambient = static_cast<float>(v); });
        addBool("ao", tr("AO"), p->ao, [](ChainNode& n, bool v) { std::get<Fractal3DParams>(n.params).ao = v; });
        addDouble("colorScale", tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble("colorCycle", tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<Fractal3DParams>(n.params).colorCycle = static_cast<float>(v); });
        addColor("background", tr("Background"), p->background, [](ChainNode& n, uint32_t v) { std::get<Fractal3DParams>(n.params).background = v; });
        addGradient("gradientPreset", tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<Fractal3DParams>(n.params).gradientPreset = std::move(v); });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<Fractal3DParams>(n.params).blend = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<Fractal3DParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<Fractal3DParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<Fractal3DParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<LyapunovParams>(&params))
    {
        addText("sequence", tr("Sequence"), p->sequence, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).sequence = std::move(v); });
        addDouble("aMin", tr("a min"), p->aMin, 0.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).aMin = static_cast<float>(v); });
        addDouble("aMax", tr("a max"), p->aMax, 0.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).aMax = static_cast<float>(v); });
        addDouble("bMin", tr("b min"), p->bMin, 0.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).bMin = static_cast<float>(v); });
        addDouble("bMax", tr("b max"), p->bMax, 0.0, 4.0, 0.01, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).bMax = static_cast<float>(v); });
        addInt("warmup", tr("Warmup"), p->warmup, 0, 2000, [](ChainNode& n, int v) { std::get<LyapunovParams>(n.params).warmup = v; });
        addInt("iterations", tr("Iterations"), p->iterations, 1, 4000, [](ChainNode& n, int v) { std::get<LyapunovParams>(n.params).iterations = v; });
        addColor("negColor", tr("Ordered colour"), p->negColor, [](ChainNode& n, uint32_t v) { std::get<LyapunovParams>(n.params).negColor = v; });
        addDouble("colorScale", tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble("colorCycle", tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<LyapunovParams>(n.params).colorCycle = static_cast<float>(v); });
        addGradient("gradientPreset", tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).gradientPreset = std::move(v); });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<LyapunovParams>(n.params).blend = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<LyapunovParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<KleinianParams>(&params))
    {
        addInt("p", tr("p"), p->p, 3, 20, [](ChainNode& n, int v) { std::get<KleinianParams>(n.params).p = v; });
        addInt("q", tr("q"), p->q, 3, 20, [](ChainNode& n, int v) { std::get<KleinianParams>(n.params).q = v; });
        addInt("iterations", tr("Iterations"), p->iterations, 1, 200, [](ChainNode& n, int v) { std::get<KleinianParams>(n.params).iterations = v; });
        addDouble("morph", tr("Morph"), p->morph, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).morph = static_cast<float>(v); });
        addDouble("zoom", tr("Zoom"), p->zoom, 0.1, 8.0, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).zoom = static_cast<float>(v); });
        addDouble("rotation", tr("Rotation"), p->rotation, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).rotation = static_cast<float>(v); });
        addDouble("colorScale", tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble("colorCycle", tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<KleinianParams>(n.params).colorCycle = static_cast<float>(v); });
        addGradient("gradientPreset", tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<KleinianParams>(n.params).gradientPreset = std::move(v); });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<KleinianParams>(n.params).blend = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<KleinianParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<KleinianParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<KleinianParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FractalZoomerParams>(&params))
    {
        addEnum("ftype", tr("Type"), p->type, {"Mandelbrot", "Julia", "Burning Ship"}, [](ChainNode& n, int v) { std::get<FractalZoomerParams>(n.params).type = v; });
        addDouble("centerX", tr("Center X"), p->centerX, -2.0, 2.0, 0.0001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).centerX = static_cast<float>(v); });
        addDouble("centerY", tr("Center Y"), p->centerY, -2.0, 2.0, 0.0001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).centerY = static_cast<float>(v); });
        addDouble("juliaX", tr("Julia X"), p->juliaX, -2.0, 2.0, 0.001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).juliaX = static_cast<float>(v); });
        addDouble("juliaY", tr("Julia Y"), p->juliaY, -2.0, 2.0, 0.001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).juliaY = static_cast<float>(v); });
        addInt("maxIter", tr("Max iterations"), p->maxIter, 1, 2048, [](ChainNode& n, int v) { std::get<FractalZoomerParams>(n.params).maxIter = v; });
        addDouble("zoomSpeed", tr("Zoom speed"), p->zoomSpeed, 0.9, 1.2, 0.001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).zoomSpeed = static_cast<float>(v); });
        addDouble("rotationSpeed", tr("Rotation speed"), p->rotationSpeed, -0.2, 0.2, 0.001, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).rotationSpeed = static_cast<float>(v); });
        addDouble("feedback", tr("Feedback"), p->feedback, 0.0, 1.0, 0.02, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).feedback = static_cast<float>(v); });
        addDouble("colorScale", tr("Colour scale"), p->colorScale, 0.001, 4.0, 0.005, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble("colorCycle", tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<FractalZoomerParams>(n.params).colorCycle = static_cast<float>(v); });
        addColor("insideColor", tr("Inside colour"), p->insideColor, [](ChainNode& n, uint32_t v) { std::get<FractalZoomerParams>(n.params).insideColor = v; });
        addGradient("gradientPreset", tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<FractalZoomerParams>(n.params).gradientPreset = std::move(v); });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<FractalZoomerParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FractalZoomerParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FractalZoomerParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<StrangeAttractorParams>(&params))
    {
        addEnum("ftype", tr("Type"), p->type, {"Lorenz", "Clifford", "De Jong", "Aizawa"}, [](ChainNode& n, int v) { std::get<StrangeAttractorParams>(n.params).type = v; });
        addDouble("a", tr("a"), p->a, -3.0, 3.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).a = static_cast<float>(v); });
        addDouble("b", tr("b"), p->b, -3.0, 3.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).b = static_cast<float>(v); });
        addDouble("c", tr("c"), p->c, -3.0, 3.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).c = static_cast<float>(v); });
        addDouble("d", tr("d"), p->d, -3.0, 3.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).d = static_cast<float>(v); });
        addInt("points", tr("Points"), p->points, 1, 100000, [](ChainNode& n, int v) { std::get<StrangeAttractorParams>(n.params).points = v; });
        addDouble("scale", tr("Scale"), p->scale, 0.01, 4.0, 0.01, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).scale = static_cast<float>(v); });
        addDouble("rotation", tr("Rotation"), p->rotation, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).rotation = static_cast<float>(v); });
        addDouble("rotationSpeed", tr("Rotation speed"), p->rotationSpeed, -2.0, 2.0, 0.02, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).rotationSpeed = static_cast<float>(v); });
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<StrangeAttractorParams>(n.params).color = v; });
        addBool("useGradient", tr("Use gradient"), p->useGradient, [](ChainNode& n, bool v) { std::get<StrangeAttractorParams>(n.params).useGradient = v; });
        addGradient("gradientPreset", tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<StrangeAttractorParams>(n.params).gradientPreset = std::move(v); });
        addDouble("dotSize", tr("Dot size"), p->dotSize, 1.0, 32.0, 0.5, [](ChainNode& n, double v) { std::get<StrangeAttractorParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<StrangeAttractorParams>(n.params).blend = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<StrangeAttractorParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<StrangeAttractorParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<StrangeAttractorParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<FlameParams>(&params))
    {
        addEnum("variation", tr("Variation"), p->variation, {"Linear", "Sinusoidal", "Spherical", "Swirl", "Horseshoe"}, [](ChainNode& n, int v) { std::get<FlameParams>(n.params).variation = v; });
        addInt("functions", tr("Functions"), p->functions, 2, 4, [](ChainNode& n, int v) { std::get<FlameParams>(n.params).functions = v; });
        addInt("points", tr("Points"), p->points, 1, 200000, [](ChainNode& n, int v) { std::get<FlameParams>(n.params).points = v; });
        addDouble("scale", tr("Scale"), p->scale, 0.05, 4.0, 0.02, [](ChainNode& n, double v) { std::get<FlameParams>(n.params).scale = static_cast<float>(v); });
        addDouble("rotation", tr("Rotation"), p->rotation, -6.2832, 6.2832, 0.05, [](ChainNode& n, double v) { std::get<FlameParams>(n.params).rotation = static_cast<float>(v); });
        addDouble("rotationSpeed", tr("Rotation speed"), p->rotationSpeed, -2.0, 2.0, 0.02, [](ChainNode& n, double v) { std::get<FlameParams>(n.params).rotationSpeed = static_cast<float>(v); });
        addGradient("gradientPreset", tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<FlameParams>(n.params).gradientPreset = std::move(v); });
        addDouble("dotSize", tr("Dot size"), p->dotSize, 1.0, 16.0, 0.5, [](ChainNode& n, double v) { std::get<FlameParams>(n.params).dotSize = static_cast<float>(v); });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<FlameParams>(n.params).blend = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<FlameParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<FlameParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<FlameParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ReactionDiffusionParams>(&params))
    {
        addDouble("feed", tr("Feed"), p->feed, 0.0, 0.1, 0.001, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).feed = static_cast<float>(v); });
        addDouble("kill", tr("Kill"), p->kill, 0.0, 0.1, 0.001, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).kill = static_cast<float>(v); });
        addDouble("diffA", tr("Diffuse A"), p->diffA, 0.0, 2.0, 0.02, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).diffA = static_cast<float>(v); });
        addDouble("diffB", tr("Diffuse B"), p->diffB, 0.0, 2.0, 0.02, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).diffB = static_cast<float>(v); });
        addInt("stepsPerFrame", tr("Steps/frame"), p->stepsPerFrame, 1, 64, [](ChainNode& n, int v) { std::get<ReactionDiffusionParams>(n.params).stepsPerFrame = v; });
        addBool("seedOnBeat", tr("Seed on beat"), p->seedOnBeat, [](ChainNode& n, bool v) { std::get<ReactionDiffusionParams>(n.params).seedOnBeat = v; });
        addDouble("colorScale", tr("Colour scale"), p->colorScale, 0.01, 8.0, 0.05, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).colorScale = static_cast<float>(v); });
        addDouble("colorCycle", tr("Colour cycle"), p->colorCycle, -4.0, 4.0, 0.05, [](ChainNode& n, double v) { std::get<ReactionDiffusionParams>(n.params).colorCycle = static_cast<float>(v); });
        addGradient("gradientPreset", tr("Palette"), p->gradientPreset, [](ChainNode& n, std::string v) { std::get<ReactionDiffusionParams>(n.params).gradientPreset = std::move(v); });
        addEnum("blend", tr("Blend"), p->blend, {"Replace", "Additive", "50/50"}, [](ChainNode& n, int v) { std::get<ReactionDiffusionParams>(n.params).blend = v; });
        addScript("initCode", tr("Init code"), p->initCode, [](ChainNode& n, std::string v) { std::get<ReactionDiffusionParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame code"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ReactionDiffusionParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat code"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ReactionDiffusionParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<ShadertoyParams>(&params))
    {
        // Shadertoy-Node (Strang S, S65): Code in mainImage-Konvention, ein
        // Fragment-Pass. Herkunft (S3-Import) ist reine Anzeige.
        addHerkunft(p->herkunft);

        // Kompilierfehler des Render-Hosts: dank `#line 1` im Wrapper tragen
        // die Treiber-Logs die Zeilennummern des NUTZER-Codes. Aktualisiert
        // sich beim Neuaufbau des Editors (Knoten erneut anklicken).
        uint64_t stNodeId = 0;
        {
            QMutexLocker lock(m_mutex);
            if (ChainNode* n = nodeAtPath(path)) stNodeId = n->nodeId;
        }
        const std::string stErr =
            m_host != nullptr ? m_host->shadertoyError(stNodeId) : std::string();
        if (!stErr.empty())
        {
            auto* err = new QLabel(
                tr("⚠ Compile error:\n%1")
                    .arg(QString::fromStdString(stErr).left(600)),
                m_propContainer);
            err->setWordWrap(true);
            err->setStyleSheet(QStringLiteral("color:#d08080"));
            form->addRow(err);
        }

        // --- URL-/ID-Import (S3): offizielle API, GET NUR auf Knopfdruck -----
        {
            auto* row = new QWidget(m_propContainer);
            auto* hl = new QHBoxLayout(row);
            hl->setContentsMargins(0, 0, 0, 0);
            auto* urlEdit = new QLineEdit(row);
            urlEdit->setPlaceholderText(
                QStringLiteral("https://www.shadertoy.com/view/... or ID"));
            if (!p->herkunft.url.empty())
                urlEdit->setText(QString::fromStdString(p->herkunft.url));
            // Symbol statt „Importieren" (S78): die Zeile muss wie jedes Feld
            // auf kFeldMinBreite schrumpfen koennen; der Text steht im Tooltip.
            auto* importBtn = new QToolButton(row);
            importBtn->setText(QString::fromUtf8("⤓"));
            importBtn->setToolTip(
                tr("Import: loads the shader through the official API (visibility "
                   "'public+api' only). Needs an API key (field below)."));
            auto* openBtn = new QToolButton(row);
            openBtn->setText(QStringLiteral("🌐"));
            openBtn->setToolTip(
                tr("Open Shadertoy in the browser — this shader's page, "
                   "otherwise shadertoy.com for browsing."));
            hl->addWidget(urlEdit, 1);
            hl->addWidget(importBtn);
            hl->addWidget(openBtn);
            form->addRow(tr("Shadertoy URL / ID"), row);

            const std::string nodeUrl = p->herkunft.url;
            connect(openBtn, &QToolButton::clicked, this, [urlEdit, nodeUrl]() {
                // Feld-Inhalt gewinnt (frisch eingefügte URL), sonst die
                // Node-Metadaten, sonst die Startseite
                const QString typed = urlEdit->text().trimmed();
                const QString id = lumi::shadertoy::extractShaderId(typed);
                QUrl target(QStringLiteral("https://www.shadertoy.com/"));
                if (!id.isEmpty())
                    target = QUrl(QStringLiteral("https://www.shadertoy.com/view/") + id);
                else if (!nodeUrl.empty())
                    target = QUrl(QString::fromStdString(nodeUrl));
                QDesktopServices::openUrl(target);
            });

            auto* keyEdit = new QLineEdit(row->parentWidget());
            keyEdit->setEchoMode(QLineEdit::Password);
            keyEdit->setPlaceholderText(tr("Shadertoy API key (stored locally)"));
            {
                QSettings settings;
                keyEdit->setText(
                    settings.value(QStringLiteral("shadertoy/apiKey")).toString());
            }
            keyEdit->setToolTip(
                tr("Free at shadertoy.com → Profile → Apps. Stored ONLY "
                   "locally (QSettings) — never in the preset or repo."));
            connect(keyEdit, &QLineEdit::editingFinished, this, [keyEdit]() {
                QSettings settings;
                settings.setValue(QStringLiteral("shadertoy/apiKey"),
                                  keyEdit->text().trimmed());
            });
            form->addRow(tr("API key"), keyEdit);

            connect(importBtn, &QToolButton::clicked, this,
                    [this, path, urlEdit, keyEdit]() {
                        const QString id =
                            lumi::shadertoy::extractShaderId(urlEdit->text());
                        if (id.isEmpty())
                        {
                            QMessageBox::warning(
                                this, tr("Shadertoy import"),
                                tr("No shader ID recognised — enter a URL in the form "
                                   "shadertoy.com/view/<ID> or the ID itself."));
                            return;
                        }
                        const QString key = keyEdit->text().trimmed();
                        if (key.isEmpty())
                        {
                            QMessageBox::warning(
                                this, tr("Shadertoy import"),
                                tr("API key missing. Create one for free: "
                                   "shadertoy.com → Profile → Apps, then enter "
                                   "it here (stays local)."));
                            return;
                        }
                        if (m_netManager == nullptr)
                        {
                            m_netManager = new QNetworkAccessManager(this);
                            m_netManager->setTransferTimeout(10000);
                        }
                        QNetworkRequest req(lumi::shadertoy::apiRequestUrl(id, key));
                        QNetworkReply* reply = m_netManager->get(req);
                        connect(reply, &QNetworkReply::finished, this,
                                [this, path, id, reply]() {
                                    reply->deleteLater();
                                    if (reply->error() != QNetworkReply::NoError)
                                    {
                                        QMessageBox::warning(
                                            this, tr("Shadertoy import"),
                                            tr("Network error: %1")
                                                .arg(reply->errorString()));
                                        return;
                                    }
                                    const auto result = lumi::shadertoy::parseApiReply(
                                        reply->readAll(), id);
                                    if (!result.ok)
                                    {
                                        QMessageBox::warning(this,
                                                             tr("Shadertoy import"),
                                                             result.error);
                                        return;
                                    }
                                    mutate(path, [&](ChainNode& n) {
                                        auto* sp =
                                            std::get_if<ShadertoyParams>(&n.params);
                                        if (sp == nullptr) return;
                                        const int blend = sp->blend;  // Nutzerwahl behalten
                                        *sp = result.params;
                                        sp->blend = blend;
                                        if (n.displayName.empty() ||
                                            n.displayName == "Shadertoy")
                                            n.displayName = sp->herkunft.name;
                                    });
                                    QString note =
                                        tr("Shader '%1' imported.")
                                            .arg(QString::fromStdString(
                                                result.params.herkunft.name));
                                    if (!result.report.isEmpty())
                                        note += QStringLiteral("\n\n") +
                                                result.report.join(QStringLiteral("\n"));
                                    QMessageBox::information(
                                        this, tr("Shadertoy import"), note);
                                    // Editor neu aufbauen: Code/Metadaten anzeigen
                                    QMetaObject::invokeMethod(
                                        this, [this, path] { buildPropertyEditor(path); },
                                        Qt::QueuedConnection);
                                });
                    });
        }

        // GLSL-Code-Editor mit ⓘ-Referenz + ⤢-Groß-Editor (AVS-Komfort).
        // Export-Namensbasis (S69): Shader-Metadaten-Name vor Node-Name.
        const QString stExportBase = !p->herkunft.name.empty()
                                         ? QString::fromStdString(p->herkunft.name)
                                     : !nodeName.empty()
                                         ? QString::fromStdString(nodeName)
                                         : QStringLiteral("shadertoy");
        addCodeEditor(
            tr("Shader code (Image)"), p->code,
            tr("void mainImage(out vec4 fragColor, in vec2 fragCoord) { ... }"),
            shadertoyReferenceHtml(),
            tr("mainImage in Shadertoy convention; fragCoord = pixel from "
               "bottom left. ⓘ shows uniforms, audio layout and "
               "buffer semantics; ⤢ opens the large editor."),
            160,
            lumi::scriptedit::shaderExportName(stExportBase,
                                               QStringLiteral("shadertoy"),
                                               QStringLiteral("image")),
            QStringLiteral("shadertoy"),
            [](ChainNode& n, std::string v) {
                std::get<ShadertoyParams>(n.params).code = std::move(v);
            });
        // Kanal-Bindungen: je iChannel Nichts / Buffer A–D / Audio (S4) sowie
        // Ketten-Eingang und AVS-Global-Buffer 1–8 (S72). Kodierung s.
        // kShadertoyInput* (SSOT); Combo-Index = Wert + 1 — die Liste MUSS
        // darum lückenlos in der Reihenfolge der Werte stehen.
        QStringList kInputNames = {tr("None"),     tr("Buffer A"), tr("Buffer B"),
                                   tr("Buffer C"), tr("Buffer D"), tr("Audio"),
                                   tr("Chain input")};
        for (int n = 1; n <= lumi::multieffect::kShadertoyInputAvsCount; ++n)
            kInputNames << tr("AVS buffer %1").arg(n);
        const int kInputMax = lumi::multieffect::kShadertoyInputMax + 1;
        const auto inputToIndex = [kInputMax](int v) {
            return std::clamp(v + 1, 0, kInputMax);
        };
        const auto indexToInput = [kInputMax](int idx) {
            return std::clamp(idx, 0, kInputMax) - 1;
        };
        const auto addInputCombos =
            [&](const std::array<int, 4>& input,
                std::function<std::array<int, 4>&(ChainNode&)> access,
                const QString& prefix) {
                for (int c = 0; c < 4; ++c)
                {
                    auto* combo = new QComboBox(m_propContainer);
                    combo->addItems(kInputNames);
                    combo->setCurrentIndex(
                        inputToIndex(input[static_cast<std::size_t>(c)]));
                    combo->setToolTip(
                        tr("Audio = 512x2 texture (row 0 spectrum, row 1 "
                           "waveform). A buffer reference to itself or a "
                           "later buffer reads the previous frame (feedback).\n"
                           "Chain input = the image this node "
                           "receives — lets Shadertoy image filters run "
                           "directly.\nAVS buffer 1–8 = the slots of "
                           "'Buffer Save', as they are when this node runs "
                           "(never written = black)."));
                    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                            this, [this, path, access, c, indexToInput](int idx) {
                                mutate(path, [&](ChainNode& n) {
                                    access(n)[static_cast<std::size_t>(c)] =
                                        indexToInput(idx);
                                });
                            });
                    form->addRow(prefix.arg(c), combo);
                }
            };
        addInputCombos(p->imageInput,
                       [](ChainNode& n) -> std::array<int, 4>& {
                           return std::get<ShadertoyParams>(n.params).imageInput;
                       },
                       tr("Image iChannel%1"));
        addEnum("blend", tr("Blend"), std::clamp(p->blend, 0, 2),
                {tr("Replace"), tr("Additive"), tr("50/50")},
                [](ChainNode& n, int v) {
                    std::get<ShadertoyParams>(n.params).blend = v;
                });
        // Block E (S79, Entscheid 1a): `blend` + eigene Uniforms per Formel.
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<ShadertoyParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<ShadertoyParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<ShadertoyParams>(n.params).beatCode = std::move(v); });

        // --- Buffer A..D (S4): je ein Code-Editor + Bindungen + Entfernen ----
        for (int bi = 0; bi < static_cast<int>(p->buffers.size()) && bi < 4; ++bi)
        {
            const auto& buf = p->buffers[static_cast<std::size_t>(bi)];
            auto* head = new QWidget(m_propContainer);
            auto* hh = new QHBoxLayout(head);
            hh->setContentsMargins(0, 8, 0, 0);
            hh->addWidget(new QLabel(
                tr("— Buffer %1 —").arg(QChar(static_cast<char16_t>('A' + bi))), head));
            hh->addStretch(1);
            auto* removeBtn = new QToolButton(head);
            removeBtn->setText(QStringLiteral("✕"));
            removeBtn->setToolTip(tr("Remove buffer (the following ones move up — "
                                     "adjust references if needed)"));
            connect(removeBtn, &QToolButton::clicked, this, [this, path, bi]() {
                mutate(path, [&](ChainNode& n) {
                    auto& sp = std::get<ShadertoyParams>(n.params);
                    if (bi < static_cast<int>(sp.buffers.size()))
                        sp.buffers.erase(sp.buffers.begin() + bi);
                });
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); },
                    Qt::QueuedConnection);
            });
            hh->addWidget(removeBtn);
            form->addRow(head);

            addCodeEditor(
                tr("Code Buffer %1").arg(QChar(static_cast<char16_t>('A' + bi))),
                buf.code,
                tr("void mainImage(out vec4 fragColor, in vec2 fragCoord) { ... }"),
                shadertoyReferenceHtml(),
                tr("Buffer pass: writes raw into an RGBA32F target (no "
                   "blend/clamp, alpha is kept). A binding to itself "
                   "reads the previous frame (feedback)."),
                120,
                lumi::scriptedit::shaderExportName(
                    stExportBase, QStringLiteral("shadertoy"),
                    QStringLiteral("buffer%1").arg(
                        QChar(static_cast<char16_t>('A' + bi)))),
                QStringLiteral("shadertoy"),
                [bi](ChainNode& n, std::string v) {
                    auto& sp = std::get<ShadertoyParams>(n.params);
                    if (bi < static_cast<int>(sp.buffers.size()))
                        sp.buffers[static_cast<std::size_t>(bi)].code = std::move(v);
                });
            addInputCombos(buf.input,
                           [bi](ChainNode& n) -> std::array<int, 4>& {
                               return std::get<ShadertoyParams>(n.params)
                                   .buffers[static_cast<std::size_t>(bi)]
                                   .input;
                           },
                           tr("%1 iChannel%2").arg(QChar(static_cast<char16_t>('A' + bi))).arg("%1"));
        }
        if (p->buffers.size() < 4)
        {
            auto* addBtn = new QPushButton(tr("Add buffer"), m_propContainer);
            addBtn->setToolTip(
                tr("New buffer pass (A–D): renders BEFORE the image pass into its "
                   "own RGBA32F target; a self-reference reads the previous frame."));
            connect(addBtn, &QPushButton::clicked, this, [this, path]() {
                mutate(path, [&](ChainNode& n) {
                    auto& sp = std::get<ShadertoyParams>(n.params);
                    if (sp.buffers.size() >= 4) return;
                    ShadertoyPass pass;
                    pass.code = "void mainImage(out vec4 fragColor, in vec2 "
                                "fragCoord)\n{\n    fragColor = vec4(0.0);\n}\n";
                    sp.buffers.push_back(std::move(pass));
                });
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); },
                    Qt::QueuedConnection);
            });
            form->addRow(addBtn);
        }

        auto* hint = new QLabel(
            tr("Uniforms: full Shadertoy set (iTime, iResolution, iChannel0-3, "
               "...) + bass/mid/treb/vol/beat. Audio binding = 512x2 texture "
               "(row 0 spectrum, row 1 waveform); buffers render A→D before "
               "the image pass."),
            m_propContainer);
        hint->setWordWrap(true);
        form->addRow(hint);
    }
    else if (auto* p = std::get_if<MeshWarpParams>(&params))
    {
        // Mesh-Warp-Node (Strang G1, S69): GPU-Vertex-Warp des Chain-Bilds.
        // Kompilierfehler des Render-Hosts (geteiltes stError-Feld — s.
        // shadertoyError()): aktualisiert sich beim Neuaufbau des Editors.
        uint64_t mwNodeId = 0;
        {
            QMutexLocker lock(m_mutex);
            if (ChainNode* n = nodeAtPath(path)) mwNodeId = n->nodeId;
        }
        const std::string mwErr =
            m_host != nullptr ? m_host->shadertoyError(mwNodeId) : std::string();
        if (!mwErr.empty())
        {
            auto* err = new QLabel(
                tr("⚠ Compile error:\n%1")
                    .arg(QString::fromStdString(mwErr).left(600)),
                m_propContainer);
            err->setWordWrap(true);
            err->setStyleSheet(QStringLiteral("color:#d08080"));
            form->addRow(err);
        }

        const QString mwExportBase = !nodeName.empty()
                                         ? QString::fromStdString(nodeName)
                                         : QStringLiteral("meshwarp");
        addCodeEditor(
            tr("Warp function (GLSL)"), p->code,
            tr("vec2 warp(vec2 uv) { return uv; }"), meshWarpReferenceHtml(),
            tr("Runs per grid vertex in the vertex shader; return value = source UV "
               "to sample. ⓘ shows the contract and the uniforms; ⤢ opens "
               "the large editor."),
            160,
            lumi::scriptedit::shaderExportName(mwExportBase,
                                               QStringLiteral("meshwarp")),
            QStringLiteral("meshwarp"),
            [](ChainNode& n, std::string v) {
                std::get<MeshWarpParams>(n.params).code = std::move(v);
            });
        addInt("gridX", tr("Grid X"), p->gridX, lumi::meshwarp::kMinGrid,
               lumi::meshwarp::kMaxGridX, [](ChainNode& n, int v) {
                   std::get<MeshWarpParams>(n.params).gridX = v;
               });
        addInt("gridY", tr("Grid Y"), p->gridY, lumi::meshwarp::kMinGrid,
               lumi::meshwarp::kMaxGridY, [](ChainNode& n, int v) {
                   std::get<MeshWarpParams>(n.params).gridY = v;
               });
        addDouble("mixAmount", tr("Mix"), p->mixAmount, 0.0, 1.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<MeshWarpParams>(n.params).mixAmount = v;
                  });
        addBool("wrapUv", tr("Repeat UV (wrap)"), p->wrapUv,
                [](ChainNode& n, bool v) {
                    std::get<MeshWarpParams>(n.params).wrapUv = v;
                });
        addScript("initCode", tr("Init"), p->initCode,
                  [](ChainNode& n, std::string v) {
                      std::get<MeshWarpParams>(n.params).initCode = std::move(v);
                  });
        addScript("frameCode", tr("Frame"), p->frameCode,
                  [](ChainNode& n, std::string v) {
                      std::get<MeshWarpParams>(n.params).frameCode = std::move(v);
                  });
        addScript("beatCode", tr("Beat"), p->beatCode,
                  [](ChainNode& n, std::string v) {
                      std::get<MeshWarpParams>(n.params).beatCode = std::move(v);
                  });
    }
    else if (auto* p = std::get_if<PixelFilterParams>(&params))
    {
        // Pixel-Filter-Node (Stilfilter-Strang, S70): Nutzer-GLSL je Pixel.
        // Kompilierfehler über das geteilte stError-Feld (shadertoyError +
        // Apply-Poll — Muster Mesh-Warp).
        uint64_t pfNodeId = 0;
        {
            QMutexLocker lock(m_mutex);
            if (ChainNode* n = nodeAtPath(path)) pfNodeId = n->nodeId;
        }
        const std::string pfErr =
            m_host != nullptr ? m_host->shadertoyError(pfNodeId) : std::string();
        if (!pfErr.empty())
        {
            auto* err = new QLabel(
                tr("⚠ Compile error:\n%1")
                    .arg(QString::fromStdString(pfErr).left(600)),
                m_propContainer);
            err->setWordWrap(true);
            err->setStyleSheet(QStringLiteral("color:#d08080"));
            form->addRow(err);
        }

        const QString pfExportBase = !nodeName.empty()
                                         ? QString::fromStdString(nodeName)
                                         : QStringLiteral("pixelfilter");
        addCodeEditor(
            tr("Filter function (GLSL)"), p->code,
            tr("vec4 farbe(vec2 uv, vec4 src) { return src; }"),
            pixelFilterReferenceHtml(),
            tr("Runs per pixel in the fragment shader; src = source pixel, neighbours "
               "via texture(uTex, …). ⓘ shows the contract and the uniforms; "
               "⤢ opens the large editor."),
            160,
            lumi::scriptedit::shaderExportName(pfExportBase,
                                               QStringLiteral("pixelfilter")),
            QStringLiteral("pixelfilter"),
            [](ChainNode& n, std::string v) {
                std::get<PixelFilterParams>(n.params).code = std::move(v);
            });
        addDouble("mixAmount", tr("Mix"), p->mixAmount, 0.0, 1.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<PixelFilterParams>(n.params).mixAmount = v;
                  });
        addScript("initCode", tr("Init"), p->initCode,
                  [](ChainNode& n, std::string v) {
                      std::get<PixelFilterParams>(n.params).initCode =
                          std::move(v);
                  });
        addScript("frameCode", tr("Frame"), p->frameCode,
                  [](ChainNode& n, std::string v) {
                      std::get<PixelFilterParams>(n.params).frameCode =
                          std::move(v);
                  });
        addScript("beatCode", tr("Beat"), p->beatCode,
                  [](ChainNode& n, std::string v) {
                      std::get<PixelFilterParams>(n.params).beatCode =
                          std::move(v);
                  });
    }
    else if (auto* p = std::get_if<IsfFilterParams>(&params))
    {
        namespace ic = lumi::multieffect::isffilter;
        uint64_t isfNodeIdVorab = 0;
        {
            QMutexLocker lock(m_mutex);
            if (ChainNode* n = nodeAtPath(path)) isfNodeIdVorab = n->nodeId;
        }

        // --- Import: EIGENER Knopf, nicht im Groß-Editor (Entscheid Patrik
        //     S72). Er lädt die `.fs` samt gleichnamiger `.vs` und schreibt
        //     Code, Quellen, Regler und Herkunft in EINEM Zug in den Knoten.
        {
            auto* importBtn = new QPushButton(tr("Import ISF file…"),
                                              m_propContainer);
            importBtn->setToolTip(
                tr("Loads a shader in Interactive Shader Format (.fs). "
                   "A .vs file of the same name is picked up automatically."));
            connect(importBtn, &QPushButton::clicked, this, [this, path]() {
                QSettings s;
                const QString dirKey =
                    QStringLiteral("editor/shaderFileDir/isffilter");
                const QString pfad = QFileDialog::getOpenFileName(
                    this, tr("Import ISF filter"),
                    s.value(dirKey, QString()).toString(),
                    tr("ISF shaders (*.fs);;All files (*)"));
                if (pfad.isEmpty()) return;
                QFile f(pfad);
                if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    QMessageBox::warning(this, tr("ISF import"),
                                         tr("File not readable: %1").arg(pfad));
                    return;
                }
                const QString inhalt = QString::fromUtf8(f.readAll());
                const QFileInfo fi(pfad);
                // Der `.vs`-Begleiter liegt NEBEN der `.fs` — der Dialog
                // liefert nur eine Datei, die Schwester wird hier gesucht.
                QString vertex;
                QFile vf(fi.absolutePath() + QLatin1Char('/') +
                         fi.completeBaseName() + QStringLiteral(".vs"));
                if (vf.open(QIODevice::ReadOnly | QIODevice::Text))
                    vertex = QString::fromUtf8(vf.readAll());

                const auto r = lumi::isf::importiereIsf(
                    inhalt, fi.completeBaseName(), vertex);
                if (!r.ok)
                {
                    QMessageBox::warning(this, tr("ISF import"), r.error);
                    return;
                }
                s.setValue(dirKey, fi.absolutePath());
                mutate(path, [&r](ChainNode& n) {
                    auto& ip = std::get<IsfFilterParams>(n.params);
                    ip.fragCode = r.fragCode.toStdString();
                    ip.vertexCode = r.vertexCode.toStdString();
                    ip.quellen = lumi::isf::alsBildQuellen(
                        r.bildInputs, r.audioWaveInputs, r.audioFftInputs);
                    ip.parameter = lumi::isf::alsParamGruppe(r.parameter);
                    ip.passes = r.passes;
                    // Umbenennen, solange der Name NICHT von Hand gesetzt
                    // wurde. Alt hiess die Bedingung nur „leer oder ISF
                    // Filter" — nach dem ersten Import trug der Knoten dann
                    // dauerhaft den Namen der ERSTEN Datei, waehrend Herkunft
                    // und Inhalt laengst zur zweiten gehoerten (Sichttest
                    // Patrik S72: Knoten „3d Rotate" zeigte „Auto Levels").
                    const bool nameVomImport =
                        n.displayName.empty() || n.displayName == "ISF Filter" ||
                        n.displayName == ip.herkunft.name;
                    ip.herkunft = r.herkunft;
                    if (nameVomImport) n.displayName = r.herkunft.name;
                });
                // Hinweise stehen IM PANEL, nicht in einem Dialog (Entscheid
                // Patrik S72) — ein Dialog je Effekt waere eine Klickorgie.
                {
                    QMutexLocker lock(m_mutex);
                    if (ChainNode* n = nodeAtPath(path))
                    {
                        if (r.report.isEmpty())
                            m_isfHinweise.remove(n->nodeId);
                        else
                            m_isfHinweise[n->nodeId] =
                                r.report.join(QLatin1Char('\n'));
                    }
                }
                QMetaObject::invokeMethod(
                        this, [this, path]() { buildPropertyEditor(path); },
                        Qt::QueuedConnection);
            });
            form->addRow(importBtn);
        }

        // Herkunft/Lizenz als FELD, nicht als Dialog (Entscheid Patrik S72).
        addHerkunft(p->herkunft);

        // Import-Hinweise ebenso als Zeile statt als Dialog.
        {
            const QString hinweise = m_isfHinweise.value(isfNodeIdVorab);
            if (!hinweise.isEmpty())
            {
                auto* lbl = new QLabel(hinweise, m_propContainer);
                lbl->setWordWrap(true);
                lbl->setStyleSheet(QStringLiteral("color:#a0a0a0"));
                form->addRow(lbl);
            }
        }

        // Sorten-Info (Wunsch Patrik): sie folgt der ANZAHL der Bildquellen,
        // nicht ihrer Bindung — eine Quelle auf „schwarz" ist ein Filter, der
        // Schwarz filtert; KEINE Quelle ist ein Generator.
        {
            int bilder = 0;
            for (const auto& q : p->quellen)
            {
                if (q.bindung != ic::kQuelleAudioWave &&
                    q.bindung != ic::kQuelleAudioFft)
                    ++bilder;
            }
            auto* info = new QLabel(
                tr("Kind: %1").arg(lumi::isf::sortenName(bilder)),
                m_propContainer);
            info->setToolTip(tr("Follows from the number of image inputs — "
                                "adding or removing inputs changes it."));
            form->addRow(info);
        }

        // Kompilierfehler über das geteilte stError-Feld (Muster Mesh-Warp).
        uint64_t isfNodeId = 0;
        {
            QMutexLocker lock(m_mutex);
            if (ChainNode* n = nodeAtPath(path)) isfNodeId = n->nodeId;
        }
        const std::string isfErr =
            m_host != nullptr ? m_host->shadertoyError(isfNodeId) : std::string();
        if (!isfErr.empty())
        {
            auto* err = new QLabel(tr("⚠ Compile error:\n%1")
                                       .arg(QString::fromStdString(isfErr).left(600)),
                                   m_propContainer);
            err->setWordWrap(true);
            err->setStyleSheet(QStringLiteral("color:#d08080"));
            form->addRow(err);
        }

        // --- Quellen-Liste: NAME (der Sampler im Shader) + BINDUNG, mit
        //     "+" und "✕". Ein selbst geschriebener Knoten bekommt seine
        //     Eingänge so von Hand; beim Import füllt der Parser sie.
        {
            QStringList bindungen = {tr("Chain image"), tr("black"),
                                     tr("Audio (waveform)"), tr("Audio (spectrum)")};
            for (int n = 1; n <= ic::kQuelleBufferAnzahl; ++n)
                bindungen << tr("AVS buffer %1").arg(n);
            // Kodierung -> Combo-Index (die Werte sind nicht lückenlos).
            const auto zuIndex = [](int b) {
                if (b == ic::kQuelleKette) return 0;
                if (b == ic::kQuelleSchwarz) return 1;
                if (b == ic::kQuelleAudioWave) return 2;
                if (b == ic::kQuelleAudioFft) return 3;
                return 4 + std::clamp(b - ic::kQuelleBufferBasis, 0,
                                      ic::kQuelleBufferAnzahl - 1);
            };
            const auto zuBindung = [](int i) {
                if (i == 0) return ic::kQuelleKette;
                if (i == 1) return ic::kQuelleSchwarz;
                if (i == 2) return ic::kQuelleAudioWave;
                if (i == 3) return ic::kQuelleAudioFft;
                return ic::kQuelleBufferBasis + (i - 4);
            };
            for (std::size_t qi = 0; qi < p->quellen.size(); ++qi)
            {
                const auto& q = p->quellen[qi];
                auto* row = new QWidget(m_propContainer);
                auto* hl = new QHBoxLayout(row);
                hl->setContentsMargins(0, 0, 0, 0);
                auto* nameEdit = new QLineEdit(QString::fromStdString(q.name), row);
                nameEdit->setToolTip(
                    tr("Identifier under which the shader samples this input "
                       "(e.g. inputImage)."));
                auto* combo = new QComboBox(row);
                combo->addItems(bindungen);
                combo->setCurrentIndex(zuIndex(q.bindung));
                auto* del = new QToolButton(row);
                del->setText(QStringLiteral("✕"));
                del->setToolTip(tr("Remove this input"));
                hl->addWidget(nameEdit, 1);
                hl->addWidget(combo, 1);
                hl->addWidget(del);
                form->addRow(tr("Input %1").arg(qi + 1), row);

                connect(nameEdit, &QLineEdit::editingFinished, this,
                        [this, path, qi, nameEdit]() {
                            const std::string t =
                                nameEdit->text().trimmed().toStdString();
                            mutate(path, [qi, &t](ChainNode& n) {
                                auto& ip = std::get<IsfFilterParams>(n.params);
                                if (qi < ip.quellen.size()) ip.quellen[qi].name = t;
                            });
                        });
                connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        this, [this, path, qi, zuBindung](int idx) {
                            mutate(path, [qi, idx, zuBindung](ChainNode& n) {
                                auto& ip = std::get<IsfFilterParams>(n.params);
                                if (qi < ip.quellen.size())
                                    ip.quellen[qi].bindung = zuBindung(idx);
                            });
                        });
                connect(del, &QToolButton::clicked, this, [this, path, qi]() {
                    mutate(path, [qi](ChainNode& n) {
                        auto& ip = std::get<IsfFilterParams>(n.params);
                        if (qi < ip.quellen.size())
                            ip.quellen.erase(ip.quellen.begin() +
                                             static_cast<long>(qi));
                    });
                    QMetaObject::invokeMethod(
                        this, [this, path]() { buildPropertyEditor(path); },
                        Qt::QueuedConnection);
                });
            }
            auto* addBtn = new QPushButton(tr("+ Input"), m_propContainer);
            addBtn->setToolTip(tr("Add another image input. The name "
                                  "must match the sampler in the shader."));
            connect(addBtn, &QPushButton::clicked, this, [this, path]() {
                mutate(path, [](ChainNode& n) {
                    auto& ip = std::get<IsfFilterParams>(n.params);
                    ip.quellen.push_back(
                        {"inputImage" + std::to_string(ip.quellen.size() + 1),
                         ic::kQuelleKette});
                });
                QMetaObject::invokeMethod(
                        this, [this, path]() { buildPropertyEditor(path); },
                        Qt::QueuedConnection);
            });
            form->addRow(addBtn);
        }

        // --- Parameter-Baum: die INPUTS des Shaders. Sie sind echte
        //     Uniforms — eine Änderung kostet KEINE Neuübersetzung.
        if (!p->parameter.leer())
        {
            auto* baum = lumi::parambaum::baueParameterBaum(
                m_propContainer, p->parameter,
                [this, path](const lumi::multieffect::ParamGruppe& neu) {
                    mutate(path, [&neu](ChainNode& n) {
                        std::get<IsfFilterParams>(n.params).parameter = neu;
                    });
                });
            baum->setMinimumHeight(120);
            baum->setMaximumHeight(280);
            form->addRow(baum);
        }

        // --- Die beiden Shader-Stufen ---------------------------------------
        const QString isfBase = !nodeName.empty()
                                    ? QString::fromStdString(nodeName)
                                    : QStringLiteral("isf");
        addCodeEditor(
            tr("Fragment (.fs)"), p->fragCode,
            tr("void main() { gl_FragColor = IMG_THIS_PIXEL(inputImage); }"),
            isfReferenceHtml(),
            tr("ISF code, executed unchanged: IMG_* macros, RENDERSIZE, "
               "TIME, PASSINDEX and getosc()/getspec() are available."),
            160,
            lumi::scriptedit::shaderExportName(isfBase, QStringLiteral("isffilter"),
                                               QStringLiteral("fs")),
            QStringLiteral("isffilter"),
            [](ChainNode& n, std::string v) {
                std::get<IsfFilterParams>(n.params).fragCode = std::move(v);
            });
        addCodeEditor(
            tr("Vertex (.vs) — optional"), p->vertexCode,
            tr("void main() { isf_vertShaderInit(); }"), isfReferenceHtml(),
            tr("Empty = default. This is where ISF filters precompute their "
               "neighbour coordinates; isf_vertShaderInit() sets position and "
               "coordinate."),
            110,
            lumi::scriptedit::shaderExportName(isfBase, QStringLiteral("isffilter"),
                                               QStringLiteral("vs")),
            QStringLiteral("isffilter/vs"),
            [](ChainNode& n, std::string v) {
                std::get<IsfFilterParams>(n.params).vertexCode = std::move(v);
            });

        addEnum("geometrie", tr("Geometry"), std::clamp(p->geometrie, 0, 2),
                {tr("Quad (faithful to ISF)"), tr("Single triangle"), tr("Grid")},
                [](ChainNode& n, int v) {
                    std::get<IsfFilterParams>(n.params).geometrie = v;
                });
        addInt("gridX", tr("Grid X"), p->gridX, ic::kGridMin, ic::kGridMax,
               [](ChainNode& n, int v) {
                   std::get<IsfFilterParams>(n.params).gridX = v;
               });
        addInt("gridY", tr("Grid Y"), p->gridY, ic::kGridMin, ic::kGridMax,
               [](ChainNode& n, int v) {
                   std::get<IsfFilterParams>(n.params).gridY = v;
               });
        addDouble("mixAmount", tr("Mix"), p->mixAmount, 0.0, 1.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<IsfFilterParams>(n.params).mixAmount = v;
                  });
        addEnum("blend", tr("Blend"), std::clamp(p->blend, 0, 2),
                {tr("Replace"), tr("Additive"), tr("50/50")},
                [](ChainNode& n, int v) {
                    std::get<IsfFilterParams>(n.params).blend = v;
                });
        // Block E (S79): Regler per Formel — Namen = key der Regler oben.
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<IsfFilterParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<IsfFilterParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<IsfFilterParams>(n.params).beatCode = std::move(v); });
    }
    else if (auto* p = std::get_if<GpuParticlesParams>(&params))
    {
        // GPU-Partikel-Node (Strang G2, S69): Render-Modul, zeichnet aufs
        // Chain-Bild. Kompilierfehler wie Mesh-Warp über das geteilte
        // stError-Feld (shadertoyError + Apply-Poll).
        uint64_t gpNodeId = 0;
        {
            QMutexLocker lock(m_mutex);
            if (ChainNode* n = nodeAtPath(path)) gpNodeId = n->nodeId;
        }
        const std::string gpErr =
            m_host != nullptr ? m_host->shadertoyError(gpNodeId) : std::string();
        if (!gpErr.empty())
        {
            auto* err = new QLabel(
                tr("⚠ Compile error:\n%1")
                    .arg(QString::fromStdString(gpErr).left(600)),
                m_propContainer);
            err->setWordWrap(true);
            err->setStyleSheet(QStringLiteral("color:#d08080"));
            form->addRow(err);
        }

        addInt("count", tr("Count"), p->count, lumi::gpuparticles::kMinCount,
               lumi::gpuparticles::kMaxCount, [](ChainNode& n, int v) {
                   std::get<GpuParticlesParams>(n.params).count = v;
               });
        addDouble("spawnX", tr("Spawn X"), p->spawnX, 0.0, 1.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).spawnX = v;
                  });
        addDouble("spawnY", tr("Spawn Y"), p->spawnY, 0.0, 1.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).spawnY = v;
                  });
        addDouble("spawnSpread", tr("Spawn spread"), p->spawnSpread, 0.0, 1.0,
                  0.005, [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).spawnSpread = v;
                  });
        addDouble("speed", tr("Speed (UV/s)"), p->speed, 0.0, 5.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).speed = v;
                  });
        addDouble("direction", tr("Direction (°)"), p->direction, -360.0, 360.0,
                  1.0, [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).direction = v;
                  });
        addDouble("fan", tr("Fan (°)"), p->fan, 0.0, 360.0, 1.0,
                  [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).fan = v;
                  });
        addDouble("gravityX", tr("Gravity X"), p->gravityX, -5.0, 5.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).gravityX = v;
                  });
        addDouble("gravityY", tr("Gravity Y"), p->gravityY, -5.0, 5.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).gravityY = v;
                  });
        addDouble("drag", tr("Drag (1/s)"), p->drag, 0.0, 10.0, 0.05,
                  [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).drag = v;
                  });
        addDouble("lifeSeconds", tr("Lifetime (s)"), p->lifeSeconds, 0.1, 30.0,
                  0.1, [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).lifeSeconds = v;
                  });
        addDouble("lifeJitter", tr("Lifetime spread"), p->lifeJitter, 0.0,
                  1.0, 0.05, [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).lifeJitter = v;
                  });
        addDouble("sizePx", tr("Size (px)"), p->sizePx, 0.5, 128.0, 0.5,
                  [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).sizePx = v;
                  });
        addDouble("sizeEndFactor", tr("End size factor"), p->sizeEndFactor, 0.0,
                  4.0, 0.05, [](ChainNode& n, double v) {
                      std::get<GpuParticlesParams>(n.params).sizeEndFactor = v;
                  });
        addColor("colorStart", tr("Colour at spawn"), p->colorStart,
                 [](ChainNode& n, uint32_t v) {
                     std::get<GpuParticlesParams>(n.params).colorStart = v;
                 });
        addColor("colorEnd", tr("Colour at end"), p->colorEnd,
                 [](ChainNode& n, uint32_t v) {
                     std::get<GpuParticlesParams>(n.params).colorEnd = v;
                 });
        addBool("additive", tr("Additive (glow)"), p->additive,
                [](ChainNode& n, bool v) {
                    std::get<GpuParticlesParams>(n.params).additive = v;
                });
        const QString gpExportBase = !nodeName.empty()
                                         ? QString::fromStdString(nodeName)
                                         : QStringLiteral("particles");
        addCodeEditor(
            tr("Force field (GLSL, optional)"), p->forceCode,
            tr("vec2 kraft(vec2 pos, vec2 vel, float alter) { return vec2(0.0); }"),
            gpuParticlesReferenceHtml(),
            tr("Additional acceleration per particle in UV/s² (on top of gravity "
               "and drag). ⓘ shows the contract and the uniforms; ⤢ opens "
               "the large editor. Empty = no additional force."),
            120,
            lumi::scriptedit::shaderExportName(gpExportBase,
                                               QStringLiteral("gpuparticles")),
            QStringLiteral("gpuparticles"),
            [](ChainNode& n, std::string v) {
                std::get<GpuParticlesParams>(n.params).forceCode = std::move(v);
            });
        addScript("initCode", tr("Init"), p->initCode,
                  [](ChainNode& n, std::string v) {
                      std::get<GpuParticlesParams>(n.params).initCode = std::move(v);
                  });
        addScript("frameCode", tr("Frame"), p->frameCode,
                  [](ChainNode& n, std::string v) {
                      std::get<GpuParticlesParams>(n.params).frameCode = std::move(v);
                  });
        addScript("beatCode", tr("Beat"), p->beatCode,
                  [](ChainNode& n, std::string v) {
                      std::get<GpuParticlesParams>(n.params).beatCode = std::move(v);
                  });
    }
    else if (auto* p = std::get_if<DebugBarsParams>(&params))
    {
        addColor("color", tr("Color"), p->color, [](ChainNode& n, uint32_t v) { std::get<DebugBarsParams>(n.params).color = v; });
        addDouble("orbitSpeed", tr("Orbit speed"), p->orbitSpeed, -10.0, 10.0, 0.1, [](ChainNode& n, double v) { std::get<DebugBarsParams>(n.params).orbitSpeed = static_cast<float>(v); });
    }
    else if (auto* p = std::get_if<HostGroupParams>(&params))
    {
        // HG1 (HostGruppen_Crossfade_Entwurf.md): Level-1-Host — eigener
        // persistenter Buffer (Feedback), eigener Buffer-Pool + ScriptContext.
        auto* info = new QLabel(
            tr("Host group: encapsulates a complete visual (its own "
               "feedback buffer, its own Buffer Save slots and script globals). "
               "Several active groups stack via Blend Out; the "
               "crossfade (HG2) blends on switching."),
            m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);

        addEnum("blendOut", tr("Blend Out"), static_cast<int>(p->blendOut), kBlendNames,
                [](ChainNode& n, int v) {
                    std::get<HostGroupParams>(n.params).blendOut =
                        static_cast<BlendMode>(v);
                });
        addInt("outAdjustAlpha", tr("Out Alpha"), p->outAdjustAlpha, 0, 255,
               [](ChainNode& n, int v) {
                   std::get<HostGroupParams>(n.params).outAdjustAlpha = v;
               });
        // Block E (S79, Entscheid 2a): nur `outadjustalpha` per Formel.
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<HostGroupParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<HostGroupParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<HostGroupParams>(n.params).beatCode = std::move(v); });

        auto* syncNote = new QLabel(
            tr("Note: the crossfade duration applies to ALL host groups "
               "in sync — a change here is carried over to all of them "
               "(design rule). The curves are individual per group."),
            m_propContainer);
        syncNote->setWordWrap(true);
        form->addRow(syncNote);
        addDouble("crossfadeSeconds", tr("Crossfade duration (s)"), p->crossfadeSeconds, 0.0, 60.0, 0.1,
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
        const QStringList curveNames = {tr("Linear"), tr("S-curve (smooth)"),
                                        tr("Ease-In"), tr("Ease-Out"),
                                        tr("Exponential")};
        addEnum("curveIn", tr("Fade-in curve"), std::clamp(p->curveIn, 0, 4), curveNames,
                [](ChainNode& n, int v) {
                    std::get<HostGroupParams>(n.params).curveIn = v;
                });
        addEnum("curveOut", tr("Fade-out curve"), std::clamp(p->curveOut, 0, 4), curveNames,
                [](ChainNode& n, int v) {
                    std::get<HostGroupParams>(n.params).curveOut = v;
                });

        // Manueller Wechsel-Trigger (Vorstufe der Visual-Playlist E6): diese
        // Gruppe einblenden, alle anderen Host-Gruppen ausblenden lassen.
        auto* switchBtn = new QPushButton(
            tr("Switch to this group (crossfade)"), m_propContainer);
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

        const auto editorNeu = [this, path]() {
            QMetaObject::invokeMethod(
                this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
        };

        // --- Presetverweis (S82): der Inhalt kommt aus der Datei und wird
        // nicht gespeichert. Gelesen wird AUSSERHALB des Render-Locks, erst
        // das Einsetzen laeuft unter mutateStructure.
        const auto verweisLaden = [this, path, editorNeu](const QString& datei) {
            ChainNode probe;
            HostGroupParams hp;
            hp.verweis = true;
            hp.sourceFile = datei.toStdString();
            probe.params = hp;
            QStringList report;
            MultiEffectVisualizer::resolvePresetReferences(probe, QString(), &report);
            if (!report.isEmpty())
            {
                QMessageBox::information(this, tr("Preset reference"), report.join('\n'));
            }
            mutateStructure([&] {
                ChainNode* ziel = nodeAtPath(path);
                auto* hg = ziel != nullptr ? std::get_if<HostGroupParams>(&ziel->params)
                                           : nullptr;
                if (hg == nullptr) return;
                hg->sourceFile = datei.toStdString();
                hg->verweis = true;
                ziel->children = std::move(probe.children);
            });
            editorNeu();
        };

        auto* verweisBox = new QCheckBox(tr("Preset reference (save the path only)"),
                                         m_propContainer);
        verweisBox->setChecked(p->verweis);
        verweisBox->setToolTip(
            tr("On: the chain saves only the path; the content is read fresh from "
               "the file on every load and does not appear in the tree.\n"
               "Off: the content is a copy in the chain and freely editable."));
        const std::string quelleJetzt = p->sourceFile;
        connect(verweisBox, &QCheckBox::toggled, this,
                [this, path, quelleJetzt, verweisLaden, editorNeu](bool an) {
                    if (an)
                    {
                        if (quelleJetzt.empty())
                        {
                            QMessageBox::information(
                                this, tr("Preset reference"),
                                tr("The group has no source file yet — load a "
                                   "preset first, then turn it into a reference."));
                            editorNeu();
                            return;
                        }
                        if (QMessageBox::question(
                                this, tr("Preset reference"),
                                tr("Turn into a reference? The content is re-read from "
                                   "the file; your own changes to it will be lost.")) !=
                            QMessageBox::Yes)
                        {
                            editorNeu();
                            return;
                        }
                        verweisLaden(QString::fromStdString(quelleJetzt));
                        return;
                    }
                    // Aus: der geladene Inhalt bleibt — jetzt als Kopie in der Kette
                    mutateStructure([&] {
                        ChainNode* ziel = nodeAtPath(path);
                        if (auto* hg = ziel != nullptr
                                           ? std::get_if<HostGroupParams>(&ziel->params)
                                           : nullptr)
                            hg->verweis = false;
                    });
                    editorNeu();
                });
        form->addRow(verweisBox);

        if (p->verweis)
        {
            // Pfad eintippen oder waehlen; „Neu laden" liest die Datei erneut
            auto* zeile = new QWidget(m_propContainer);
            auto* reihe = new QHBoxLayout(zeile);
            reihe->setContentsMargins(0, 0, 0, 0);
            auto* pfadEdit = new QLineEdit(QString::fromStdString(p->sourceFile), zeile);
            pfadEdit->setObjectName(QStringLiteral("presetVerweisPfad"));
            auto* oeffnen = new QPushButton(tr("Open…"), zeile);
            auto* neu = new QPushButton(tr("Reload"), zeile);
            reihe->addWidget(pfadEdit, 1);
            reihe->addWidget(oeffnen);
            reihe->addWidget(neu);
            form->addRow(tr("Reference to"), zeile);
            if (!QFileInfo::exists(pfadEdit->text()))
                pfadEdit->setToolTip(tr("File not found — the group stays empty."));
            connect(pfadEdit, &QLineEdit::editingFinished, this, [pfadEdit, verweisLaden,
                                                                  alt = p->sourceFile]() {
                if (pfadEdit->text().toStdString() == alt) return;  // nur Fokuswechsel
                verweisLaden(pfadEdit->text());
            });
            connect(oeffnen, &QPushButton::clicked, this, [this, verweisLaden]() {
                QSettings settings;
                const QString schluessel = QStringLiteral("effectPalette/presetGroupDir");
                const QString datei = QFileDialog::getOpenFileName(
                    this, tr("Preset reference"), settings.value(schluessel).toString(),
                    tr("Presets (*.avs *.milk *.lvfx *.lvfx2)"));
                if (datei.isEmpty()) return;
                settings.setValue(schluessel, QFileInfo(datei).absolutePath());
                verweisLaden(datei);
            });
            connect(neu, &QPushButton::clicked, this,
                    [pfadEdit, verweisLaden]() { verweisLaden(pfadEdit->text()); });
        }
        else if (!p->sourceFile.empty())
        {
            auto* src = new QLabel(tr("Source: %1").arg(QString::fromStdString(
                                       p->sourceFile)),
                                   m_propContainer);
            src->setWordWrap(true);
            form->addRow(src);
        }
        // Preset laden / aus der Quelle neu laden (S82, vorher nur .lvfx): der
        // Inhalt ist eine KOPIE (Entscheid Patrik S82, 2a) — die Quelle wird
        // nur auf Knopfdruck neu gelesen, eigene Aenderungen gehen dabei verloren.
        const auto inhaltErsetzen = [this, path](QString datei) {
            bool hatInhalt = false;
            {
                QMutexLocker lock(m_mutex);
                const ChainNode* gruppe = nodeAtPath(path);
                hatInhalt = gruppe != nullptr && !gruppe->children.empty();
            }
            if (hatInhalt &&
                QMessageBox::question(
                    this, tr("Load preset into group"),
                    tr("Replace the group's current content? Your own "
                       "changes to it will be lost.")) != QMessageBox::Yes)
                return;
            std::vector<ChainNode> inhalt;
            if (!presetGruppeLaden(datei, inhalt)) return;
            mutateStructure([&] {
                ChainNode* ziel = nodeAtPath(path);
                auto* hg = ziel != nullptr
                               ? std::get_if<HostGroupParams>(&ziel->params)
                               : nullptr;
                if (hg == nullptr) return;
                ziel->children = std::move(inhalt);
                hg->sourceFile = datei.toStdString();
            });
        };
        if (!p->verweis && !p->sourceFile.empty())
        {
            const QString quelle = QString::fromStdString(p->sourceFile);
            auto* reloadBtn = new QPushButton(tr("Reload from source"), m_propContainer);
            reloadBtn->setEnabled(QFileInfo::exists(quelle));
            reloadBtn->setToolTip(QFileInfo::exists(quelle)
                                      ? tr("Reads %1 again and replaces the "
                                           "group's content.").arg(quelle)
                                      : tr("Source file not found: %1").arg(quelle));
            connect(reloadBtn, &QPushButton::clicked, this,
                    [inhaltErsetzen, quelle]() { inhaltErsetzen(quelle); });
            form->addRow(reloadBtn);
        }
        if (!p->verweis)
        {
            auto* importBtn = new QPushButton(
                tr("Load preset into this group (.avs, .milk, .lvfx)..."), m_propContainer);
            connect(importBtn, &QPushButton::clicked, this,
                    [inhaltErsetzen]() { inhaltErsetzen(QString()); });
            form->addRow(importBtn);
        }
    }
    else if (auto* p = std::get_if<HostSwitcherParams>(&params))
    {
        // Host Switcher (S82): Liste der geschalteten Gruppen (Index = Position),
        // Ausloeser Zeit / Songwechsel / Skript. Schaltet nur zur Laufzeit.
        auto* info = new QLabel(
            tr("Switches between the host groups in the list — by time, on a "
               "song change or by script (`switch = n`, `nextgroup = 1`). The "
               "transition is the groups' crossfade. The chain is saved as "
               "configured; switching only applies at runtime."),
            m_propContainer);
        info->setWordWrap(true);
        form->addRow(info);

        // Gruppen der aktiven Kette (oberste Ebene) — Quelle fuer „Hinzufuegen"
        // und fuer die Markierung fehlender Eintraege
        QStringList vorhanden;
        uint64_t switcherId = 0;
        {
            QMutexLocker lock(m_mutex);
            for (const ChainNode& c : m_host->chain().children)
                if (c.isHostGroup()) vorhanden << QString::fromStdString(c.displayName);
            if (const ChainNode* self = nodeAtPath(path)) switcherId = self->nodeId;
        }
        const auto neuAufbauen = [this, path]() {
            QMetaObject::invokeMethod(
                this, [this, path] { buildPropertyEditor(path); }, Qt::QueuedConnection);
        };

        auto* liste = new QListWidget(m_propContainer);
        liste->setObjectName(QStringLiteral("hostSwitcherGroups"));
        for (size_t i = 0; i < p->groups.size(); ++i)
        {
            const QString name = QString::fromStdString(p->groups[i]);
            auto* item = new QListWidgetItem(
                QStringLiteral("%1   %2").arg(i + 1).arg(name) +
                    (vorhanden.contains(name) ? QString() : tr("   (missing in the chain)")),
                liste);
            item->setData(Qt::UserRole, static_cast<int>(i));
        }
        liste->setMaximumHeight(140);
        form->addRow(tr("Groups"), liste);

        // Aendern der Liste: eine Operation auf `groups`, danach Editor neu
        const auto listeAendern = [this, path, neuAufbauen](
                                      std::function<void(std::vector<std::string>&)> fn) {
            mutate(path, [&](ChainNode& n) { fn(std::get<HostSwitcherParams>(n.params).groups); });
            neuAufbauen();
        };

        auto* knoepfe = new QWidget(m_propContainer);
        auto* reihe = new QHBoxLayout(knoepfe);
        reihe->setContentsMargins(0, 0, 0, 0);
        auto* auswahl = new QComboBox(knoepfe);
        for (const QString& name : vorhanden)
        {
            if (std::find(p->groups.begin(), p->groups.end(), name.toStdString()) ==
                p->groups.end())
                auswahl->addItem(name);
        }
        auto* dazu = new QPushButton(tr("Add"), knoepfe);
        dazu->setEnabled(auswahl->count() > 0);
        auto* hoch = new QPushButton(QStringLiteral("▲"), knoepfe);
        auto* runter = new QPushButton(QStringLiteral("▼"), knoepfe);
        auto* weg = new QPushButton(tr("Remove"), knoepfe);
        hoch->setToolTip(tr("Move the selected group up one position (index - 1)"));
        runter->setToolTip(tr("Move the selected group down one position (index + 1)"));
        reihe->addWidget(auswahl, 1);
        reihe->addWidget(dazu);
        reihe->addWidget(hoch);
        reihe->addWidget(runter);
        reihe->addWidget(weg);
        form->addRow(QString(), knoepfe);

        connect(dazu, &QPushButton::clicked, this, [auswahl, listeAendern]() {
            const std::string name = auswahl->currentText().toStdString();
            if (name.empty()) return;
            listeAendern([&](std::vector<std::string>& g) { g.push_back(name); });
        });
        const auto verschieben = [liste, listeAendern](int delta) {
            const int i = liste->currentRow();
            if (i < 0) return;
            listeAendern([&](std::vector<std::string>& g) {
                const int j = i + delta;
                if (j < 0 || j >= static_cast<int>(g.size())) return;
                std::swap(g[static_cast<size_t>(i)], g[static_cast<size_t>(j)]);
            });
        };
        connect(hoch, &QPushButton::clicked, this, [verschieben]() { verschieben(-1); });
        connect(runter, &QPushButton::clicked, this, [verschieben]() { verschieben(+1); });
        connect(weg, &QPushButton::clicked, this, [liste, listeAendern]() {
            const int i = liste->currentRow();
            if (i < 0) return;
            listeAendern([&](std::vector<std::string>& g) {
                if (i < static_cast<int>(g.size())) g.erase(g.begin() + i);
            });
        });

        // Laufende Gruppe (Laufzeitwert, halbsekuendlich nachgefuehrt)
        auto* laeuft = new QLabel(m_propContainer);
        form->addRow(tr("Running"), laeuft);
        // Kopie der Namen: `p` zeigt in die lokale Editor-Kopie der Params, die
        // mit diesem Aufruf endet — der Timer lebt laenger
        const std::vector<std::string> namen = p->groups;
        const auto laeuftAktualisieren = [this, laeuft, switcherId, namen]() {
            int idx = 0;
            {
                QMutexLocker lock(m_mutex);
                if (m_host != nullptr) idx = m_host->debugSwitcherIndex(switcherId);
            }
            if (idx < 1 || idx > static_cast<int>(namen.size()))
            {
                laeuft->setText(tr("—"));
                return;
            }
            laeuft->setText(QStringLiteral("%1 — %2").arg(idx).arg(
                QString::fromStdString(namen[static_cast<size_t>(idx - 1)])));
        };
        auto* takt = new QTimer(laeuft);
        connect(takt, &QTimer::timeout, laeuft, laeuftAktualisieren);
        takt->start(500);
        laeuftAktualisieren();

        addInt("startIndex", tr("Start group"), p->startIndex, 1,
               std::max(1, static_cast<int>(p->groups.size())),
               [](ChainNode& n, int v) { std::get<HostSwitcherParams>(n.params).startIndex = v; });
        addDouble("intervalSeconds", tr("Switch after (s)"), p->intervalSeconds, 0.0, 3600.0, 1.0,
                  [](ChainNode& n, double v) {
                      std::get<HostSwitcherParams>(n.params).intervalSeconds = v;
                  });
        addBool("nextOnSongChange", tr("Next on song change"), p->nextOnSongChange,
                [](ChainNode& n, bool v) {
                    std::get<HostSwitcherParams>(n.params).nextOnSongChange = v;
                });
        addEnum("order", tr("Order"), std::clamp(p->order, 0, 1),
                {tr("List order"), tr("Random")},
                [](ChainNode& n, int v) { std::get<HostSwitcherParams>(n.params).order = v; });
        addScript("initCode", tr("Init"), p->initCode, [](ChainNode& n, std::string v) { std::get<HostSwitcherParams>(n.params).initCode = std::move(v); });
        addScript("frameCode", tr("Frame"), p->frameCode, [](ChainNode& n, std::string v) { std::get<HostSwitcherParams>(n.params).frameCode = std::move(v); });
        addScript("beatCode", tr("Beat"), p->beatCode, [](ChainNode& n, std::string v) { std::get<HostSwitcherParams>(n.params).beatCode = std::move(v); });
        // Song-Slot: allgemeine Zeile am Ende des Editors (ChainNode::songCode)
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
                tr("MilkDrop preset '%1' — PS%2 · Warp: %3 · Comp: %4 · %5 sprite(s)\n"
                   "Sections as children in the tree, in render order: Code → "
                   "Warp → Shapes → Waves → Comp → Sprites (Parameters = frame)")
                    .arg(QString::fromStdString(p->preset.name))
                    .arg(p->preset.psVersion)
                    .arg(className(p->preset.warpInfo.shaderClass))
                    .arg(className(p->preset.compInfo.shaderClass))
                    .arg(p->preset.sprites.size()),
                m_propContainer);
            info->setWordWrap(true);
            form->addRow(info);
            addInt("meshX", tr("Mesh X"), p->meshX, 8, 96,
                   [milkOf](ChainNode& n, int v) { milkOf(n).meshX = v; });
            addInt("meshY", tr("Mesh Y"), p->meshY, 6, 72,
                   [milkOf](ChainNode& n, int v) { milkOf(n).meshY = v; });
            addBool("debugGrid", tr("Calibration grid"), p->debugGrid,
                    [milkOf](ChainNode& n, bool v) { milkOf(n).debugGrid = v; });
            addBool("debugMotionVectors", tr("Motion vector overlay"),
                    p->debugMotionVectors, [milkOf](ChainNode& n, bool v) {
                        milkOf(n).debugMotionVectors = v;
                    });
            addText("presetDir", tr("Texture base folder"), p->presetDir,
                    [milkOf](ChainNode& n, std::string v) {
                        milkOf(n).presetDir = std::move(v);
                    });

            // --- Regelwerk (Strang R, S65) ---------------------------------------
            // Combo + vier Einzelschalter (nur bei Benutzerdefiniert aktiv; sonst
            // zeigen sie die abgeleiteten Werte) + PS-Overrides je Stufe. Jede
            // Aenderung bumpt via milkOf die Revision → der Render-Host baut
            // Skripte/Shader neu (Transpiler-Emission haengt am Schalter).
            {
                using lumi::milkdrop::PsOverride;
                using lumi::milkdrop::Regelwerk;
                using PS = lumi::milkdrop::PresetState;

                auto* rwCombo = new QComboBox(m_propContainer);
                rwCombo->addItems({tr("Legacy (faithful to original)"), tr("Modern (IEEE)"),
                                   tr("Custom")});
                rwCombo->setCurrentIndex(static_cast<int>(p->preset.regelwerk));
                rwCombo->setToolTip(
                    tr("Legacy = all D3D9 emulations ON (import default, "
                       "faithful to the original). Modern = pure IEEE/GL, no legacy "
                       "crutches. Custom = the individual switches count."));
                form->addRow(tr("Rule set"), rwCombo);

                const auto effektiv = lumi::milkdrop::effektiveSchalter(p->preset);
                const bool custom = p->preset.regelwerk == Regelwerk::Benutzerdefiniert;
                const auto makeSwitch = [&](const QString& label, const QString& tip,
                                            bool PS::* member, bool shown) {
                    auto* cb = new QCheckBox(m_propContainer);
                    cb->setChecked(shown);
                    cb->setEnabled(custom);
                    cb->setToolTip(tip);
                    connect(cb, &QCheckBox::toggled, this,
                            [this, path, milkOf, member](bool v) {
                                mutate(path, [&](ChainNode& n) {
                                    milkOf(n).preset.*member = v;
                                });
                            });
                    form->addRow(label, cb);
                    return cb;
                };
                auto* cbDiv = makeSwitch(
                    tr("D3D9 division contract"),
                    tr("D3D9 computes legacy float: 0/0 = 0, x/0 stays finite — "
                       "IEEE NaN would blacken sums (S64, Rainbow Attack NEON)."),
                    &PS::divVertragD3d9, effektiv.divVertragD3d9);
                auto* cbUnorm = makeSwitch(
                    tr("UNORM8 truncation"),
                    tr("D3D9 truncates on 8-bit writes, GL rounds — without "
                       "truncation the decay stalls at v*(1-d) < 0.5/255 "
                       "(S64, R211 grey floor)."),
                    &PS::unormTrunkierung, effektiv.unormTrunkierung);
                auto* cbQ = makeSwitch(
                    tr("q garbage epsilon"),
                    tr("Without per_frame_init the original reads uninitialised "
                       "heap (q = 1e-6 instead of 0) — divisions stay finite "
                       "(S64, R series)."),
                    &PS::qGarbageEpsilon, effektiv.qGarbageEpsilon);
                auto* cbUv = makeSwitch(
                    tr("UV sanitize"),
                    tr("NaN UVs → 0 (D3D9 conversion rule) and huge UVs wrapped "
                       "into the tile in double precision (fixed point keeps bits — "
                       "S64, 2077 look)."),
                    &PS::uvSanitize, effektiv.uvSanitize);

                connect(rwCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        this,
                        [this, path, milkOf, cbDiv, cbUnorm, cbQ, cbUv](int idx) {
                            const auto rw =
                                static_cast<Regelwerk>(std::clamp(idx, 0, 2));
                            // WYSIWYG beim Wechsel auf Benutzerdefiniert: die
                            // ANGEZEIGTEN Werte werden die Rohfelder — der Start
                            // entspricht dem Verhalten der alten Betriebsart.
                            mutate(path, [&](ChainNode& n) {
                                auto& mp = milkOf(n);
                                mp.preset.regelwerk = rw;
                                if (rw == Regelwerk::Benutzerdefiniert)
                                {
                                    mp.preset.divVertragD3d9 = cbDiv->isChecked();
                                    mp.preset.unormTrunkierung = cbUnorm->isChecked();
                                    mp.preset.qGarbageEpsilon = cbQ->isChecked();
                                    mp.preset.uvSanitize = cbUv->isChecked();
                                }
                            });
                            const bool cust = rw == Regelwerk::Benutzerdefiniert;
                            for (QCheckBox* cb : {cbDiv, cbUnorm, cbQ, cbUv})
                                cb->setEnabled(cust);
                            if (!cust)
                            {
                                const bool on = rw == Regelwerk::Legacy;
                                for (QCheckBox* cb : {cbDiv, cbUnorm, cbQ, cbUv})
                                {
                                    cb->blockSignals(true);
                                    cb->setChecked(on);
                                    cb->blockSignals(false);
                                }
                            }
                        });

                const auto makePsCombo = [&](const QString& label, PsOverride value,
                                             PsOverride PS::* member) {
                    auto* combo = new QComboBox(m_propContainer);
                    combo->addItems({tr("Auto (.milk)"), tr("PS2"), tr("PS3"),
                                     tr("Force MD1")});
                    combo->setCurrentIndex(static_cast<int>(value));
                    combo->setToolTip(
                        tr("Auto follows the PSVERSION fields of the .milk. PS2/PS3 "
                           "only override the declaration (the GLSL emission is "
                           "identical for both levels). Force MD1 ignores "
                           "the custom shader text and renders the exact "
                           "MD1 path (S64 strip bisection as a feature)."));
                    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                            this, [this, path, milkOf, member](int idx) {
                                mutate(path, [&](ChainNode& n) {
                                    milkOf(n).preset.*member =
                                        static_cast<PsOverride>(std::clamp(idx, 0, 3));
                                });
                            });
                    form->addRow(label, combo);
                };
                makePsCombo(tr("PS version Warp"), p->preset.psWarp, &PS::psWarp);
                makePsCombo(tr("PS version Comp"), p->preset.psComp, &PS::psComp);

                // Skript-Zufall (S80): eigener Schalter, NICHT Teil der
                // Betriebsart. Standard Original (Entscheid Patrik a) — die
                // Gitter-Sonde belegt es; „bisher" holt den alten Look zurueck.
                auto* randCombo = new QComboBox(m_propContainer);
                randCombo->addItems({tr("LumiViz (previous)"), tr("Original (ns-eel2)")});
                randCombo->setCurrentIndex(p->preset.eelRandOriginal ? 1 : 0);
                randCombo->setToolTip(
                    tr("rand() in scripts. Original (default) = like MilkDrop 2: "
                       "decimal number in [0, x], fixed seed — presets then look "
                       "as in the original. Previous = whole numbers (rand(1) is "
                       "then always 0), the look up to session 80."));
                connect(randCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        this, [this, path, milkOf](int idx) {
                            mutate(path, [&](ChainNode& n) {
                                milkOf(n).preset.eelRandOriginal = idx == 1;
                            });
                        });
                form->addRow(tr("Script random"), randCombo);

                // Audio (S80, Entscheid Patrik a): EIN Schalter fuer alles, was aus
                // dem Audio kommt — Custom-Waves UND bass/mid/treb (MilkdropAudio.hpp)
                auto* audioCombo = new QComboBox(m_propContainer);
                audioCombo->addItems({tr("LumiViz (previous)"), tr("Original (MilkDrop)")});
                audioCombo->setCurrentIndex(p->preset.audioOriginal ? 1 : 0);
                audioCombo->setToolTip(
                    tr("Where bass/mid/treb (and _att) and the custom waves "
                       "(value1/value2) come from. Original = like MilkDrop 2: its own "
                       "processing of the 8-bit waveform — FFT with envelope and "
                       "equalisation, alignment of the wave to the previous one, band energies; "
                       "checked against the original core (default). Previous = the app's "
                       "spectrum with estimated factors (sessions 39/43), the sound up to "
                       "session 80."));
                connect(audioCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        this, [this, path, milkOf](int idx) {
                            mutate(path, [&](ChainNode& n) {
                                milkOf(n).preset.audioOriginal = idx == 1;
                            });
                        });
                form->addRow(tr("Audio"), audioCombo);

                // Seitenverhaeltnis der Custom-Waves (S81, Entscheid Patrik a)
                auto* aspektCombo = new QComboBox(m_propContainer);
                aspektCombo->addItems({tr("LumiViz (previous)"), tr("Original (MilkDrop)")});
                aspektCombo->setCurrentIndex(p->preset.aspektOriginal ? 1 : 0);
                aspektCombo->setToolTip(
                    tr("How custom waves map their x/y values onto the image. Original "
                       "(default) = like MilkDrop: on a widescreen image the curve is "
                       "stretched vertically by the aspect ratio (4:3 by 4/3, 16:9 "
                       "by 16/9), y = 0.1 then lies below the image edge — this is how "
                       "the authors saw their presets. Previous = x/y are fractions "
                       "of the image (0 = left/bottom, 1 = right/top edge), the "
                       "curve stays unstretched in the image; the look of sessions 39 "
                       "to 80."));
                connect(aspektCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        this, [this, path, milkOf](int idx) {
                            mutate(path, [&](ChainNode& n) {
                                milkOf(n).preset.aspektOriginal = idx == 1;
                            });
                        });
                form->addRow(tr("Aspect ratio (waves)"), aspektCombo);
            }

            // --- Puffer-Wechsel (S66) --------------------------------------------
            // Was mit dem GEERBTEN Feedback-Bild beim .milk→.milk-Wechsel
            // passiert. BEWUSST ohne milkOf: kein Revision-Bump — der Schalter
            // wirkt erst beim naechsten Preset-Tausch und darf das laufende
            // Preset nicht neu starten.
            {
                auto* pwCombo = new QComboBox(m_propContainer);
                pwCombo->addItems({tr("App setting"),
                                   tr("Keep (original)"), tr("Clear"),
                                   tr("Fading (one-off mix)"),
                                   tr("Fade out (over time)")});
                pwCombo->setCurrentIndex(
                    static_cast<int>(p->pufferWechsel));
                pwCombo->setToolTip(
                    tr("What happens to the inherited image when the NEXT preset "
                       "is loaded. Keep = original MilkDrop (every "
                       "preset inherits the previous preset's image — chaotic "
                       "feedback presets tip into different branches depending "
                       "on their predecessor). Clear = fresh start as at app start "
                       "(deterministic noise seed, NOT black — otherwise "
                       "amplifier presets would stay dead). Fading = ONE-OFF "
                       "mix of inheritance and seed at the moment of the switch (share "
                       "below). Fade out = the inherited image dies away over the "
                       "set time, whatever the new preset draws "
                       "stays. App setting follows the default from the "
                       "Settings panel."));
                form->addRow(tr("Buffer on preset change"), pwCombo);

                auto* fadeSlider = new QSlider(Qt::Horizontal, m_propContainer);
                fadeSlider->setRange(0, 100);
                fadeSlider->setValue(
                    static_cast<int>(p->pufferFading * 100.0 + 0.5));
                fadeSlider->setEnabled(p->pufferWechsel ==
                                       PufferWechsel::Fading);
                fadeSlider->setToolTip(
                    tr("Share of the inherited image when fading: 0 % = like Clear, "
                       "100 % = like Keep."));
                form->addRow(tr("Fading inheritance (%)"), fadeSlider);

                auto* ausblendSpin = new QDoubleSpinBox(m_propContainer);
                ausblendSpin->setRange(0.1, 60.0);
                ausblendSpin->setSingleStep(0.5);
                ausblendSpin->setSuffix(tr(" s"));
                ausblendSpin->setValue(p->pufferAusblendSek);
                ausblendSpin->setEnabled(p->pufferWechsel ==
                                         PufferWechsel::Ausblenden);
                ausblendSpin->setToolTip(
                    tr("Fade out: time until the inherited image has fully "
                       "disappeared (the echo is additionally damped every "
                       "frame; fresh drawings of the new preset "
                       "stay)."));
                form->addRow(tr("Fade-out duration"), ausblendSpin);

                connect(pwCombo,
                        QOverload<int>::of(&QComboBox::currentIndexChanged),
                        this,
                        [this, path, fadeSlider, ausblendSpin](int idx) {
                            const auto pw = static_cast<PufferWechsel>(
                                std::clamp(idx, 0, 4));
                            mutate(path, [&](ChainNode& n) {
                                std::get<MilkdropNodeParams>(n.params)
                                    .pufferWechsel = pw;
                            });
                            fadeSlider->setEnabled(pw == PufferWechsel::Fading);
                            ausblendSpin->setEnabled(pw ==
                                                     PufferWechsel::Ausblenden);
                        });
                connect(fadeSlider, &QSlider::valueChanged, this,
                        [this, path](int v) {
                            mutate(path, [&](ChainNode& n) {
                                std::get<MilkdropNodeParams>(n.params)
                                    .pufferFading = v / 100.0;
                            });
                        });
                connect(ausblendSpin,
                        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                        this, [this, path](double v) {
                            mutate(path, [&](ChainNode& n) {
                                std::get<MilkdropNodeParams>(n.params)
                                    .pufferAusblendSek = v;
                            });
                        });
            }
        }
        else if (milkSection == 0)  // Code-Slots (EEL, Dialekt Milkdrop)
        {
            addScript("perFrameInit", tr("Init (per_frame_init)"), p->preset.perFrameInit,
                      [milkOf](ChainNode& n, std::string v) {
                          milkOf(n).preset.perFrameInit = std::move(v);
                      });
            addScript("perFrame", tr("Frame (per_frame)"), p->preset.perFrame,
                      [milkOf](ChainNode& n, std::string v) {
                          milkOf(n).preset.perFrame = std::move(v);
                      });
            addScript("perPixel", tr("Point (per_pixel)"), p->preset.perPixel,
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
                tr("The preset's built-in standard wave (always on in "
                   "MilkDrop). Start values — per_frame code can override wave_* "
                   "every frame."),
                m_propContainer);
            hint->setWordWrap(true);
            form->addRow(hint);
            addBool("basicWaveEnabled", tr("Base waveform active"),
                    p->preset.basicWaveEnabled, setB(&PS::basicWaveEnabled),
                    tr("LumiViz switch: off = the wave is not rendered. "
                       "MilkDrop presets only know alpha 0."),
                    [this, path] { syncMilkEyes(path); });
            addInt("waveMode", tr("Wave mode (0-7)"), p->preset.waveMode, 0, 7, setI(&PS::waveMode));
            addBool("additiveWaves", tr("Additive"), p->preset.additiveWaves, setB(&PS::additiveWaves));
            addBool("waveDots", tr("Dots instead of lines"), p->preset.waveDots, setB(&PS::waveDots));
            addBool("waveThick", tr("Thick"), p->preset.waveThick, setB(&PS::waveThick));
            addBool("modWaveAlphaByVolume", tr("Alpha by volume"), p->preset.modWaveAlphaByVolume, setB(&PS::modWaveAlphaByVolume));
            addBool("maximizeWaveColor", tr("Maximise colour"), p->preset.maximizeWaveColor, setB(&PS::maximizeWaveColor));
            addDouble("waveAlpha", tr("Alpha"), p->preset.waveAlpha, 0.0, 10.0, 0.01, setD(&PS::waveAlpha));
            addDouble("waveScale", tr("Scale"), p->preset.waveScale, 0.0, 100.0, 0.01, setD(&PS::waveScale));
            addDouble("waveSmoothing", tr("Smoothing"), p->preset.waveSmoothing, 0.0, 0.95, 0.01, setD(&PS::waveSmoothing));
            addDouble("waveMystery", tr("Mystery (wave_mystery)"), p->preset.waveMystery, -2.0, 2.0, 0.01, setD(&PS::waveMystery));
            addDouble("modWaveAlphaStart", tr("Mod alpha start"), p->preset.modWaveAlphaStart, 0.0, 2.0, 0.01, setD(&PS::modWaveAlphaStart));
            addDouble("modWaveAlphaEnd", tr("Mod alpha end"), p->preset.modWaveAlphaEnd, 0.0, 2.0, 0.01, setD(&PS::modWaveAlphaEnd));
            addDouble("waveR", tr("Red"), p->preset.waveR, 0.0, 1.0, 0.01, setD(&PS::waveR));
            addDouble("waveG", tr("Green"), p->preset.waveG, 0.0, 1.0, 0.01, setD(&PS::waveG));
            addDouble("waveB", tr("Blue"), p->preset.waveB, 0.0, 1.0, 0.01, setD(&PS::waveB));
            addDouble("waveX", tr("Position X"), p->preset.waveX, 0.0, 1.0, 0.01, setD(&PS::waveX));
            addDouble("waveY", tr("Position Y"), p->preset.waveY, 0.0, 1.0, 0.01, setD(&PS::waveY));
        }
        else if (milkSection == 1)  // Custom Waves (Sektion oder Einzel-Element)
        {
            if (p->preset.waves.empty())
                form->addRow(new QLabel(tr("No custom waves in the preset"),
                                        m_propContainer));
            if (milkElem < 0)
                form->addRow(new QLabel(
                    tr("Create new: 'Custom Wave' in the dropdown + '+'. "
                       "Remove/clone: select the wave in the tree."),
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
                addBool("enabled", tr("Wave %1 active").arg(w.index), w.enabled,
                        [milkOf, i](ChainNode& n, bool v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.waves.size())
                                mp.preset.waves[i].enabled = v;
                        },
                        QString(), [this, path] { syncMilkEyes(path); });
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
                    addInt("samples", tr("Samples"), w.samples, 2, 512, wI(&WS::samples));
                    addInt("sep", tr("L/R offset (sep)"), w.sep, 0, 256, wI(&WS::sep));
                    addBool("spectrum", tr("Spectrum instead of waveform"), w.spectrum, wB(&WS::spectrum));
                    addBool("useDots", tr("Dots"), w.useDots, wB(&WS::useDots));
                    addBool("drawThick", tr("Thick"), w.drawThick, wB(&WS::drawThick));
                    addBool("additive", tr("Additive"), w.additive, wB(&WS::additive));
                    addDouble("scaling", tr("Scale"), w.scaling, 0.0, 100.0, 0.01, wD(&WS::scaling));
                    addDouble("smoothing", tr("Smoothing"), w.smoothing, 0.0, 1.0, 0.01, wD(&WS::smoothing));
                    addDouble("r", tr("Red"), w.r, 0.0, 1.0, 0.01, wD(&WS::r));
                    addDouble("g", tr("Green"), w.g, 0.0, 1.0, 0.01, wD(&WS::g));
                    addDouble("b", tr("Blue"), w.b, 0.0, 1.0, 0.01, wD(&WS::b));
                    addDouble("a", tr("Alpha"), w.a, 0.0, 1.0, 0.01, wD(&WS::a));
                }
                addScript("milkdrop", tr("Wave %1 · Init").arg(w.index), w.initCode,
                          setWave(&lumi::milkdrop::WaveState::initCode));
                addScript("milkdrop", tr("Wave %1 · Frame").arg(w.index), w.frameCode,
                          setWave(&lumi::milkdrop::WaveState::frameCode));
                addScript("milkdrop", tr("Wave %1 · Point").arg(w.index), w.pointCode,
                          setWave(&lumi::milkdrop::WaveState::pointCode));
            }
        }
        else if (milkSection == 2)  // Custom Shapes (Sektion oder Einzel-Element)
        {
            if (p->preset.shapes.empty())
                form->addRow(new QLabel(tr("No custom shapes in the preset"),
                                        m_propContainer));
            if (milkElem < 0)
                form->addRow(new QLabel(
                    tr("Create new: 'Custom Shape' in the dropdown + '+'. "
                       "Remove/clone: select the shape in the tree."),
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
                addBool("enabled", tr("Shape %1 active").arg(s.index), s.enabled,
                        [milkOf, i](ChainNode& n, bool v) {
                            auto& mp = milkOf(n);
                            if (i < mp.preset.shapes.size())
                                mp.preset.shapes[i].enabled = v;
                        },
                        QString(), [this, path] { syncMilkEyes(path); });
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
                    addInt("sides", tr("Sides"), s.sides, 3, 100, sI(&SS::sides));
                    addInt("instances", tr("Instances (num_inst)"), s.instances, 1, 1024, sI(&SS::instances));
                    addBool("additive", tr("Additive"), s.additive, sB(&SS::additive));
                    addBool("thickOutline", tr("Thick outline"), s.thickOutline, sB(&SS::thickOutline));
                    addBool("textured", tr("Textured (previous frame)"), s.textured, sB(&SS::textured));
                    addDouble("texZoom", tr("Texture zoom"), s.texZoom, 0.01, 10.0, 0.01, sD(&SS::texZoom));
                    addDouble("texAng", tr("Texture angle"), s.texAng, -6.3, 6.3, 0.01, sD(&SS::texAng));
                    addDouble("x", tr("Position X"), s.x, 0.0, 1.0, 0.001, sD(&SS::x));
                    addDouble("y", tr("Position Y"), s.y, 0.0, 1.0, 0.001, sD(&SS::y));
                    addDouble("rad", tr("Radius"), s.rad, 0.0, 2.0, 0.001, sD(&SS::rad));
                    addDouble("ang", tr("Angle"), s.ang, -6.3, 6.3, 0.01, sD(&SS::ang));
                    addDouble("r", tr("Centre R"), s.r, 0.0, 1.0, 0.01, sD(&SS::r));
                    addDouble("g", tr("Centre G"), s.g, 0.0, 1.0, 0.01, sD(&SS::g));
                    addDouble("b", tr("Centre B"), s.b, 0.0, 1.0, 0.01, sD(&SS::b));
                    addDouble("a", tr("Centre A"), s.a, 0.0, 1.0, 0.01, sD(&SS::a));
                    addDouble("r2", tr("Edge R (r2)"), s.r2, 0.0, 1.0, 0.01, sD(&SS::r2));
                    addDouble("g2", tr("Edge G (g2)"), s.g2, 0.0, 1.0, 0.01, sD(&SS::g2));
                    addDouble("b2", tr("Edge B (b2)"), s.b2, 0.0, 1.0, 0.01, sD(&SS::b2));
                    addDouble("a2", tr("Edge A (a2)"), s.a2, 0.0, 1.0, 0.01, sD(&SS::a2));
                    addDouble("borderR", tr("Border R"), s.borderR, 0.0, 1.0, 0.01, sD(&SS::borderR));
                    addDouble("borderG", tr("Border G"), s.borderG, 0.0, 1.0, 0.01, sD(&SS::borderG));
                    addDouble("borderB", tr("Border B"), s.borderB, 0.0, 1.0, 0.01, sD(&SS::borderB));
                    addDouble("borderA", tr("Border A"), s.borderA, 0.0, 1.0, 0.01, sD(&SS::borderA));
                }
                addScript("milkdrop", tr("Shape %1 · Init").arg(s.index), s.initCode,
                          setShape(&lumi::milkdrop::ShapeState::initCode));
                addScript("milkdrop", tr("Shape %1 · Frame").arg(s.index), s.frameCode,
                          setShape(&lumi::milkdrop::ShapeState::frameCode));
            }
        }
        // Warp- und Comp-Shader sind seit S66 GETRENNTE Sektionen (3 = Warp,
        // 6 = Comp) — im Baum stehen sie an ihrer Pipeline-Position (Warp
        // frueh, Comp beim Composite). Editor-Logik bleibt gemeinsam (HLSL,
        // SSOT — Klassifikation wird beim Aendern neu abgeleitet).
        else if (milkSection == 3 || milkSection == 6)
        {
            auto addHlsl = [&](const QString& label, const std::string& value,
                               bool isWarp) {
                auto* edit = new QPlainTextEdit(m_propContainer);
                edit->setPlainText(QString::fromStdString(value));
                edit->setPlaceholderText(tr("HLSL shader_body (empty = MD1 path)"));
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
                // Inputs/Konstanten/Sampler/Funktionen der Praeambel.
                // ⤢-Groß-Editor nachgeführt (S65, wie AVS-Skripte; ohne
                // EEL-Highlighter — HLSL-Bezeichner würden falsch eingefärbt).
                auto* header = new QWidget(m_propContainer);
                auto* hl = new QHBoxLayout(header);
                hl->setContentsMargins(0, 0, 0, 0);
                hl->addWidget(new QLabel(label, header));
                hl->addStretch(1);
                auto* refBtn = new QToolButton(header);
                refBtn->setText(QString::fromUtf8("ⓘ"));
                refBtn->setToolTip(
                    tr("Shader reference (inputs, constants, samplers, functions)"));
                auto* expandBtn = new QToolButton(header);
                expandBtn->setText(QString::fromUtf8("⤢"));
                expandBtn->setToolTip(
                    tr("Open in large, resizable editor"));
                hl->addWidget(refBtn);
                hl->addWidget(expandBtn);
                connect(refBtn, &QToolButton::clicked, this, [this]() {
                    showScriptReference(this, milkShaderReferenceHtml());
                });
                // Groß-Editor mit Apply/Beautify (S69): HLSL-Syntaxprobe
                // synchron über den HlslTranspiler (leer = MD1-Pfad, keine
                // Probe); Re-Indent wie GLSL (Brace-basiert). Export-Vorschlag
                // aus Node-Name + warp/comp. ENDUNG .hlsl (Entscheid Patrik
                // S71): der Inhalt IST HLSL — externe Editoren faerben dann
                // richtig ein, und die Vertrags-Trennung bleibt ehrlich.
                const QString hlslExportBase =
                    nodeName.empty() ? QStringLiteral("milkdrop")
                                     : QString::fromStdString(nodeName);
                connect(expandBtn, &QToolButton::clicked, this,
                        [this, edit, label, isWarp, hlslExportBase]() {
                            lumi::scriptedit::ScriptEditorHooks hooks;
                            hooks.exportFileName =
                                lumi::scriptedit::shaderExportName(
                                    hlslExportBase, QStringLiteral("milkdrop"),
                                    isWarp ? QStringLiteral("warp")
                                           : QStringLiteral("comp"));
                            hooks.vertragKey = QStringLiteral("milkdrop");
                            hooks.vorlagenKey = isWarp ? QStringLiteral("milkdrop/warp")
                                                       : QStringLiteral("milkdrop/comp");
                            hooks.apply = [edit, isWarp](const QString& t) -> QString {
                                edit->setPlainText(t);  // textChanged -> mutate
                                if (t.trimmed().isEmpty()) return {};
                                const auto probe = lumi::hlsl::transpile(
                                    t.toStdString(),
                                    isWarp ? lumi::hlsl::ShaderKind::Warp
                                           : lumi::hlsl::ShaderKind::Comp);
                                return probe.ok
                                           ? QString()
                                           : QString::fromStdString(probe.error);
                            };
                            hooks.beautify = [](const QString& t) {
                                return QString::fromStdString(
                                    lumi::scripting::beautifyGlsl(
                                        t.toStdString(),
                                        lumi::scriptedit::formatOptionsFromSettings()));
                            };
                            QString out;
                            if (openScriptEditor(this, label, edit->toPlainText(),
                                                 milkShaderReferenceHtml(), out, {},
                                                 /*eelHighlight=*/false, hooks))
                            {
                                edit->setPlainText(out);  // textChanged -> mutate
                            }
                        });
                form->addRow(header);
                form->addRow(edit);
            };
            if (milkSection == 3)
                addHlsl(tr("Warp shader (HLSL)"), p->preset.warpShaderText, true);
            else
                addHlsl(tr("Comp shader (HLSL)"), p->preset.compShaderText, false);
        }
        else if (milkSection == 4)  // Sprites (N3.3 — Sektion oder Einzel-Element)
        {
            if (p->preset.sprites.empty())
                form->addRow(new QLabel(tr("No sprites in the preset"),
                                        m_propContainer));
            if (milkElem < 0)
                form->addRow(new QLabel(
                    tr("Create new: 'Sprite' in the dropdown + '+'. "
                       "Remove/clone: select the sprite in the tree."),
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
                addBool("enabled", t + tr(" active"), sp.enabled,
                        spriteSet([](SpriteState& s, bool v) { s.enabled = v; }),
                        QString(), [this, path] { syncMilkEyes(path); });
                // N3.2 nachgezogen (S76): numerische Startwerte nur in der
                // Einzel-Ansicht — die Sektions-Liste bleibt kompakt
                // (aktiv + Code), wie bei Waves/Shapes. Labels kurz, Details
                // im Tooltip (Panel-Breite folgt der Label-Spalte).
                if (milkElem >= 0)
                {
                    addText("imageName", tr("Image"), sp.imageName,
                            spriteSet([](SpriteState& s, std::string v) {
                                s.imageName = std::move(v);
                            }),
                            tr("File name, relative to the preset's "
                               "texture folder"));
                    addColor("colorKey", tr("Colorkey"), sp.colorKey,
                             spriteSet([](SpriteState& s, uint32_t v) {
                                 s.colorKey = v;
                             }));
                    addInt("layer", tr("Layer"), sp.layer, -100, 100,
                           spriteSet([](SpriteState& s, int v) { s.layer = v; }),
                           tr("Drawing order — lowest first"));
                    addInt("blendMode", tr("Blend mode"), sp.blendMode, 0, 4,
                           spriteSet([](SpriteState& s, int v) {
                               s.blendMode = v;
                           }),
                           tr("0=Alpha · 1=Decal · 2=Add · 3=SrcColor · "
                              "4=Colorkey"));
                    addDouble("alpha", tr("Alpha"), sp.alpha, 0.0, 1.0, 0.01,
                              spriteSet([](SpriteState& s, double v) { s.alpha = v; }));
                    addDouble("burn", tr("Burn"), sp.burn, 0.0, 1.0, 0.01,
                              spriteSet([](SpriteState& s, double v) { s.burn = v; }));
                    addDouble("x", tr("Position X"), sp.x, -5.0, 5.0, 0.01,
                              spriteSet([](SpriteState& s, double v) { s.x = v; }));
                    addDouble("y", tr("Position Y"), sp.y, -5.0, 5.0, 0.01,
                              spriteSet([](SpriteState& s, double v) { s.y = v; }));
                    addDouble("sx", tr("Scale X"), sp.sx, -10.0, 10.0, 0.01,
                              spriteSet([](SpriteState& s, double v) { s.sx = v; }),
                              tr("Negative = mirrored"));
                    addDouble("sy", tr("Scale Y"), sp.sy, -10.0, 10.0, 0.01,
                              spriteSet([](SpriteState& s, double v) { s.sy = v; }),
                              tr("Negative = mirrored"));
                    addDouble("rot", tr("Rotation (rad)"), sp.rot, -10.0, 10.0, 0.01,
                              spriteSet([](SpriteState& s, double v) { s.rot = v; }));
                    addDouble("speed", tr("Speed"), sp.speed, 0.0, 10.0, 0.1,
                              spriteSet([](SpriteState& s, double v) {
                                  s.speed = v;
                              }),
                              tr("Scales the sprite-local `time`"));
                    addDouble("repeatX", tr("Repeat X"), sp.repeatX, 0.01, 100.0, 0.1,
                              spriteSet([](SpriteState& s, double v) {
                                  s.repeatX = v;
                              }));
                    addDouble("repeatY", tr("Repeat Y"), sp.repeatY, 0.01, 100.0, 0.1,
                              spriteSet([](SpriteState& s, double v) {
                                  s.repeatY = v;
                              }));
                }
                addScript("code", t + tr(" · Code (per Frame)"), sp.code,
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
                tr("Start values of the preset — per_frame code can override many "
                   "of them every frame (zoom/rot/wave_*/…)."),
                m_propContainer);
            hint->setWordWrap(true);
            form->addRow(hint);
            const bool bakedComp =
                p->preset.compInfo.shaderClass == lumi::milk::ShaderClass::Md1Default ||
                p->preset.compInfo.shaderClass == lumi::milk::ShaderClass::Md1Plus;
            if (bakedComp || p->preset.compInfo.shaderClass == lumi::milk::ShaderClass::Custom)
            {
                auto* baked = new QLabel(
                    tr("Note: this preset has a comp shader — gamma/"
                       "echo/filter/fShader are BAKED IN there and have no effect "
                       "here (baked contract; empty the shader to activate "
                       "them)."),
                    m_propContainer);
                baked->setWordWrap(true);
                form->addRow(baked);
            }

            group(tr("General / Composite"));
            addBool("kaltstartSaat", tr("Cold-start noise seed"),
                    p->preset.kaltstartSaat, setB(&PS::kaltstartSaat),
                    tr("On = the feedback buffer starts with deterministic "
                       "noise (MilkDrop behaviour; amplifier presets "
                       "need it). Off = defined black start."));
            addDouble("decay", tr("Decay"), p->preset.decay, 0.0, 1.0, 0.001, setD(&PS::decay));
            addDouble("gammaAdj", tr("Gamma"), p->preset.gammaAdj, 0.0, 8.0, 0.01, setD(&PS::gammaAdj));
            addDouble("videoEchoZoom", tr("Echo zoom"), p->preset.videoEchoZoom, 0.001, 1000.0, 0.01, setD(&PS::videoEchoZoom));
            addDouble("videoEchoAlpha", tr("Echo alpha"), p->preset.videoEchoAlpha, 0.0, 1.0, 0.01, setD(&PS::videoEchoAlpha));
            addEnum("videoEchoOrientation", tr("Echo orientation"), p->preset.videoEchoOrientation,
                    {tr("None"), tr("H mirror"), tr("V mirror"), tr("Both")},
                    setI(&PS::videoEchoOrientation));
            addDouble("shader", tr("fShader colour wash"), p->preset.shader, 0.0, 1.0, 0.01, setD(&PS::shader));
            addBool("texWrap", tr("Texture wrap"), p->preset.texWrap, setB(&PS::texWrap));
            addBool("darkenCenter", tr("Darken Center"), p->preset.darkenCenter, setB(&PS::darkenCenter));
            addBool("brighten", tr("Brighten"), p->preset.brighten, setB(&PS::brighten));
            addBool("darken", tr("Darken"), p->preset.darken, setB(&PS::darken));
            addBool("solarize", tr("Solarize"), p->preset.solarize, setB(&PS::solarize));
            addBool("invert", tr("Invert"), p->preset.invert, setB(&PS::invert));

            // --- Gestaltung (S80) — KEIN Original-Verhalten --------------------
            // Linear-Drift, Radial-Drift, Weichzeichnen (MilkdropGestaltung.hpp),
            // je Werkzeug ein eigener Schalter (Wunsch Patrik). Winkel im Panel in
            // GRAD; das per_frame-Skript sieht dieselben Werte in Bogenmass
            // (drift_lin/drift_dir, drift_rad/drift_ang/drift_cx/drift_cy, weich).
            // Zwei Knoepfe setzen bekannte Stellungen: „Original" = alles aus,
            // „Weicher Sog ↖" = der Look bis S80 (Halbtexel-Versatz unter GL).
            group(tr("Styling (not original behaviour)"));
            {
                const auto makeCheck = [&](const char* field, const QString& label,
                                           bool value, bool PS::* member,
                                           const QString& tip) {
                    auto* cb = new QCheckBox(m_propContainer);
                    cb->setChecked(value);
                    connect(cb, &QCheckBox::toggled, this, [this, path, milkOf, member](bool v) {
                        mutate(path, [&](ChainNode& n) { milkOf(n).preset.*member = v; });
                    });
                    addRow(field, label, cb, tip);
                    return cb;
                };
                const auto makeSpin = [&](const char* field, const QString& label,
                                          double value, double lo, double hi, double step,
                                          int decimals, double PS::* member,
                                          const QString& tip) {
                    auto* spin = new QDoubleSpinBox(m_propContainer);
                    spin->setRange(lo, hi);
                    spin->setSingleStep(step);
                    spin->setDecimals(decimals);
                    spin->setValue(value);
                    connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                            [this, path, milkOf, member](double v) {
                                mutate(path, [&](ChainNode& n) {
                                    milkOf(n).preset.*member = v;
                                });
                            });
                    addRow(field, label, spin, tip);
                    return spin;
                };
                const QString skriptHinweis =
                    tr(" Readable and writable in per_frame code as %1 (radians).");

                auto* cbLin = makeCheck(
                    "gestaltLinAn", tr("Linear drift"), p->preset.gestaltLinAn,
                    &PS::gestaltLinAn,
                    tr("The whole image moves SHARPLY in one direction every frame "
                       "(fractions are saved up: 0.5 px = one whole pixel every "
                       "second frame). The original does not have this."));
                auto* spLinS = makeSpin(
                    "gestaltLinStaerke", tr("  Strength (px/frame)"),
                    p->preset.gestaltLinStaerke, 0.0, 4.0, 0.05, 4, &PS::gestaltLinStaerke,
                    tr("1 px/frame = 60 px/s at 60 fps.") +
                        skriptHinweis.arg(QStringLiteral("drift_lin")));
                auto* spLinR = makeSpin(
                    "gestaltLinRichtung", tr("  Direction (°)"),
                    p->preset.gestaltLinRichtung, 0.0, 360.0, 5.0, 1,
                    &PS::gestaltLinRichtung,
                    tr("0 = right, 90 = up, 180 = left, 270 = down.") +
                        skriptHinweis.arg(QStringLiteral("drift_dir")));

                auto* cbRad = makeCheck(
                    "gestaltRadAn", tr("Radial drift"), p->preset.gestaltRadAn,
                    &PS::gestaltRadAn,
                    tr("Every point moves equally fast, the direction depends on "
                       "its position relative to the centre: outwards, swirl or pull. "
                       "Unlike zoom/rot, equally fast everywhere. Always draws "
                       "somewhat soft. The original does not have this."));
                makeSpin(
                    "gestaltRadStaerke", tr("  Strength (px/frame)"),
                    p->preset.gestaltRadStaerke, 0.0, 4.0, 0.05, 4, &PS::gestaltRadStaerke,
                    tr("1 px/frame = 60 px/s at 60 fps.") +
                        skriptHinweis.arg(QStringLiteral("drift_rad")));
                makeSpin(
                    "gestaltRadWinkel", tr("  Angle (°)"),
                    p->preset.gestaltRadWinkel, 0.0, 360.0, 5.0, 1, &PS::gestaltRadWinkel,
                    tr("0 = outwards, 90 = counter-clockwise swirl, "
                       "180 = pull inwards, 270 = clockwise swirl; "
                       "spirals in between.") +
                        skriptHinweis.arg(QStringLiteral("drift_ang")));
                makeSpin("gestaltRadCx", tr("  Centre X (0–1)"), p->preset.gestaltRadCx,
                         0.0, 1.0, 0.05, 3, &PS::gestaltRadCx,
                         tr("0 = left edge, 1 = right edge.") +
                             skriptHinweis.arg(QStringLiteral("drift_cx")));
                makeSpin("gestaltRadCy", tr("  Centre Y (0–1)"), p->preset.gestaltRadCy,
                         0.0, 1.0, 0.05, 3, &PS::gestaltRadCy,
                         tr("0 = bottom edge, 1 = top edge.") +
                             skriptHinweis.arg(QStringLiteral("drift_cy")));

                auto* cbWeich = makeCheck(
                    "gestaltWeichAn", tr("Soften"), p->preset.gestaltWeichAn,
                    &PS::gestaltWeichAn,
                    tr("Blends the image with the neighbouring pixel every frame (oscillates by "
                       "±½ pixel, so nothing is shifted). With decay=1 this acts "
                       "like a slow fade-out. The original does not have this."));
                auto* spW = makeSpin(
                    "gestaltWeich", tr("  Strength (0–1)"), p->preset.gestaltWeich, 0.0, 1.0,
                    0.05, 2, &PS::gestaltWeich,
                    tr("0 = sharp, 1 = full half-pixel oscillation.") +
                        skriptHinweis.arg(QStringLiteral("weich")));

                auto* knoepfe = new QWidget(m_propContainer);
                auto* kl = new QHBoxLayout(knoepfe);
                kl->setContentsMargins(0, 0, 0, 0);
                auto* bOriginal = new QPushButton(tr("Original"), knoepfe);
                bOriginal->setToolTip(tr("All three tools off — like the original: "
                                         "the image stands still and sharp."));
                auto* bSog = new QPushButton(QString::fromUtf8("Soft pull ↖"), knoepfe);
                bSog->setToolTip(tr("Linear drift 0.7071 px at 135° and Soften 1: "
                                    "the image drifts softly away to the upper left — the look "
                                    "LumiViz had up to session 80."));
                kl->addWidget(bOriginal);
                kl->addWidget(bSog);
                kl->addStretch();
                form->addRow(tr("Setting"), knoepfe);
                // Die Knoepfe setzen nur die Bedienelemente — deren Signale
                // schreiben wie bei Handbedienung in die Kette.
                connect(bOriginal, &QPushButton::clicked, this, [cbLin, cbRad, cbWeich] {
                    cbLin->setChecked(false);
                    cbRad->setChecked(false);
                    cbWeich->setChecked(false);
                });
                connect(bSog, &QPushButton::clicked, this,
                        [cbLin, cbRad, cbWeich, spLinS, spLinR, spW] {
                            spLinS->setValue(0.7071);
                            spLinR->setValue(135.0);
                            spW->setValue(1.0);
                            cbLin->setChecked(true);
                            cbRad->setChecked(false);
                            cbWeich->setChecked(true);
                        });
            }

            group(tr("Base waveform"));
            {
                // S43: eigene Ansicht im Baum (Waves -> Basis-Waveform) —
                // hier nur der Wegweiser, keine doppelte Regler-Flaeche.
                auto* basisRef = new QLabel(
                    tr("Own view: section 'Waves' → "
                       "'Base waveform' in the tree."),
                    m_propContainer);
                basisRef->setWordWrap(true);
                form->addRow(basisRef);
            }

            group(tr("Motion (warp mesh)"));
            addDouble("zoom", tr("Zoom"), p->preset.zoom, 0.01, 10.0, 0.001, setD(&PS::zoom));
            addDouble("zoomExponent", tr("Zoom exponent"), p->preset.zoomExponent, 0.01, 10.0, 0.01, setD(&PS::zoomExponent));
            addDouble("rot", tr("Rotation"), p->preset.rot, -2.0, 2.0, 0.001, setD(&PS::rot));
            addDouble("warp", tr("Warp"), p->preset.warp, 0.0, 10.0, 0.01, setD(&PS::warp));
            addDouble("warpAnimSpeed", tr("Warp anim speed"), p->preset.warpAnimSpeed, 0.01, 10.0, 0.01, setD(&PS::warpAnimSpeed));
            addDouble("warpScale", tr("Warp scale"), p->preset.warpScale, 0.01, 10.0, 0.01, setD(&PS::warpScale));
            addDouble("cx", tr("Centre X (cx)"), p->preset.cx, 0.0, 1.0, 0.001, setD(&PS::cx));
            addDouble("cy", tr("Centre Y (cy)"), p->preset.cy, 0.0, 1.0, 0.001, setD(&PS::cy));
            addDouble("dx", tr("Drift X (dx)"), p->preset.dx, -1.0, 1.0, 0.001, setD(&PS::dx));
            addDouble("dy", tr("Drift Y (dy)"), p->preset.dy, -1.0, 1.0, 0.001, setD(&PS::dy));
            addDouble("sx", tr("Stretch X (sx)"), p->preset.sx, 0.01, 10.0, 0.001, setD(&PS::sx));
            addDouble("sy", tr("Stretch Y (sy)"), p->preset.sy, 0.01, 10.0, 0.001, setD(&PS::sy));

            group(tr("Borders"));
            addDouble("obSize", tr("Outer size"), p->preset.obSize, 0.0, 0.5, 0.001, setD(&PS::obSize));
            addDouble("obR", tr("Outer R"), p->preset.obR, 0.0, 1.0, 0.01, setD(&PS::obR));
            addDouble("obG", tr("Outer G"), p->preset.obG, 0.0, 1.0, 0.01, setD(&PS::obG));
            addDouble("obB", tr("Outer B"), p->preset.obB, 0.0, 1.0, 0.01, setD(&PS::obB));
            addDouble("obA", tr("Outer A"), p->preset.obA, 0.0, 1.0, 0.01, setD(&PS::obA));
            addDouble("ibSize", tr("Inner size"), p->preset.ibSize, 0.0, 0.5, 0.001, setD(&PS::ibSize));
            addDouble("ibR", tr("Inner R"), p->preset.ibR, 0.0, 1.0, 0.01, setD(&PS::ibR));
            addDouble("ibG", tr("Inner G"), p->preset.ibG, 0.0, 1.0, 0.01, setD(&PS::ibG));
            addDouble("ibB", tr("Inner B"), p->preset.ibB, 0.0, 1.0, 0.01, setD(&PS::ibB));
            addDouble("ibA", tr("Inner A"), p->preset.ibA, 0.0, 1.0, 0.01, setD(&PS::ibA));

            group(tr("Motion Vectors"));
            addDouble("mvX", tr("Grid X (mv_x)"), p->preset.mvX, 0.0, 64.0, 0.1, setD(&PS::mvX));
            addDouble("mvY", tr("Grid Y (mv_y)"), p->preset.mvY, 0.0, 48.0, 0.1, setD(&PS::mvY));
            addDouble("mvDX", tr("Offset X (mv_dx)"), p->preset.mvDX, -1.0, 1.0, 0.001, setD(&PS::mvDX));
            addDouble("mvDY", tr("Offset Y (mv_dy)"), p->preset.mvDY, -1.0, 1.0, 0.001, setD(&PS::mvDY));
            addDouble("mvL", tr("Length (mv_l)"), p->preset.mvL, 0.0, 10.0, 0.01, setD(&PS::mvL));
            addDouble("mvR", tr("Red (mv_r)"), p->preset.mvR, 0.0, 1.0, 0.01, setD(&PS::mvR));
            addDouble("mvG", tr("Green (mv_g)"), p->preset.mvG, 0.0, 1.0, 0.01, setD(&PS::mvG));
            addDouble("mvB", tr("Blue (mv_b)"), p->preset.mvB, 0.0, 1.0, 0.01, setD(&PS::mvB));
            addDouble("mvA", tr("Alpha (mv_a)"), p->preset.mvA, 0.0, 1.0, 0.01, setD(&PS::mvA));

            group(tr("Blur Pyramid"));
            addDouble("blur1Min", tr("Blur1 Min"), p->preset.blur1Min, 0.0, 1.0, 0.01, setD(&PS::blur1Min));
            addDouble("blur1Max", tr("Blur1 Max"), p->preset.blur1Max, 0.0, 1.0, 0.01, setD(&PS::blur1Max));
            addDouble("blur2Min", tr("Blur2 Min"), p->preset.blur2Min, 0.0, 1.0, 0.01, setD(&PS::blur2Min));
            addDouble("blur2Max", tr("Blur2 Max"), p->preset.blur2Max, 0.0, 1.0, 0.01, setD(&PS::blur2Max));
            addDouble("blur3Min", tr("Blur3 Min"), p->preset.blur3Min, 0.0, 1.0, 0.01, setD(&PS::blur3Min));
            addDouble("blur3Max", tr("Blur3 Max"), p->preset.blur3Max, 0.0, 1.0, 0.01, setD(&PS::blur3Max));
            addDouble("blur1EdgeDarken", tr("Blur1 Edge-Darken"), p->preset.blur1EdgeDarken, 0.0, 1.0, 0.01, setD(&PS::blur1EdgeDarken));
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
        form->addRow(tr("Text (';' separates)"), edit);
        addText("fontFace", tr("Font"), p->fontFace,
                [](ChainNode& n, std::string v) { std::get<TextParams>(n.params).fontFace = std::move(v); });
        addInt("fontHeight", tr("Height (px)"), std::abs(p->fontHeight), 4, 400,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).fontHeight = -v; });
        addInt("fontWeight", tr("Weight"), p->fontWeight, 100, 900,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).fontWeight = v; });
        addBool("italic", tr("Italic"), p->italic,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).italic = v; });
        addBool("underline", tr("Underline"), p->underline,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).underline = v; });
        addColor("color", tr("Color"), p->color,
                 [](ChainNode& n, uint32_t v) { std::get<TextParams>(n.params).color = v; });
        addEnum("blend", tr("Blend"), std::clamp(p->blend, 0, 2),
                {tr("Replace"), tr("Additive"), tr("50/50")},
                [](ChainNode& n, int v) { std::get<TextParams>(n.params).blend = v; });
        addBool("onBeat", tr("On beat"), p->onBeat,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).onBeat = v; });
        addInt("onBeatSpeed", tr("Beat hold (frames)"), p->onBeatSpeed, 1, 600,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).onBeatSpeed = v; });
        addInt("normSpeed", tr("Word time (frames)"), p->normSpeed, 1, 600,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).normSpeed = v; });
        addBool("insertBlank", tr("Insert blank"), p->insertBlank,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).insertBlank = v; });
        addBool("randomPos", tr("Random position"), p->randomPos,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).randomPos = v; });
        addBool("randomWord", tr("Random word"), p->randomWord,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).randomWord = v; });
        addEnum("hAlign", tr("H-Align"), std::clamp(p->hAlign, 0, 2),
                {tr("Left"), tr("Center"), tr("Right")},
                [](ChainNode& n, int v) { std::get<TextParams>(n.params).hAlign = v; });
        addEnum("vAlign", tr("V-Align"), std::clamp(p->vAlign, 0, 2),
                {tr("Top"), tr("Center"), tr("Bottom")},
                [](ChainNode& n, int v) { std::get<TextParams>(n.params).vAlign = v; });
        addInt("xShift", tr("X-Shift (%)"), p->xShift, -100, 100,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).xShift = v; });
        addInt("yShift", tr("Y-Shift (%)"), p->yShift, -100, 100,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).yShift = v; });
        addBool("outline", tr("Outline"), p->outline,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).outline = v; });
        addColor("outlineColor", tr("Outline color"), p->outlineColor,
                 [](ChainNode& n, uint32_t v) { std::get<TextParams>(n.params).outlineColor = v; });
        addInt("outlineSize", tr("Outline size"), p->outlineSize, 1, 32,
               [](ChainNode& n, int v) { std::get<TextParams>(n.params).outlineSize = v; });
        addBool("shadow", tr("Shadow"), p->shadow,
                [](ChainNode& n, bool v) { std::get<TextParams>(n.params).shadow = v; });
    }
    else if (auto* p = std::get_if<AviParams>(&params))
    {
        addText("filename", tr("File"), p->filename,
                [](ChainNode& n, std::string v) { std::get<AviParams>(n.params).filename = std::move(v); });
        addText("resolvedPath", tr("Resolved path"), p->resolvedPath,
                [](ChainNode& n, std::string v) { std::get<AviParams>(n.params).resolvedPath = std::move(v); });
        addEnum("blend", tr("Blend"), std::clamp(p->blend, 0, 2),
                {tr("Replace"), tr("Additive"), tr("50/50")},
                [](ChainNode& n, int v) { std::get<AviParams>(n.params).blend = v; });
        addBool("adapt", tr("Beat adaptive"), p->adapt,
                [](ChainNode& n, bool v) { std::get<AviParams>(n.params).adapt = v; });
        addInt("persist", tr("Persist (frames)"), p->persist, 0, 32,
               [](ChainNode& n, int v) { std::get<AviParams>(n.params).persist = v; });
        addInt("speedMs", tr("Frame time (ms)"), p->speedMs, 0, 1000,
               [](ChainNode& n, int v) { std::get<AviParams>(n.params).speedMs = v; });
    }
    else if (auto* p = std::get_if<VideoSourceParams>(&params))
    {
        namespace vs = lumi::multieffect::videosource;
        // videoSource (S70): LumiViz-Quellknoten — Datei/Kamera/Testaufnahme.
        // Der AVS-`avi`-Knoten bleibt daneben unangetastet (Kalibrierung).
        addEnum("source", tr("Source"), std::clamp(p->source, 0, vs::kSourceMax),
                {tr("File"), tr("Camera (live)"), tr("Camera Clip")},
                [](ChainNode& n, int v) {
                    std::get<VideoSourceParams>(n.params).source = v;
                });
        addText("filePath", tr("Video file"), p->filePath,
                [](ChainNode& n, std::string v) {
                    std::get<VideoSourceParams>(n.params).filePath = std::move(v);
                });
        {
            auto* browseBtn =
                new QPushButton(tr("Choose video file…"), m_propContainer);
            browseBtn->setToolTip(
                tr("Sets the Video file field. The path stays in the preset "
                   "— videos are not embedded."));
            connect(browseBtn, &QPushButton::clicked, this, [this, path]() {
                const QString file = QFileDialog::getOpenFileName(
                    this, tr("Choose video file"), QString(),
                    tr("Videos (*.mp4 *.mkv *.webm *.mov *.wmv *.avi);;"
                       "All files (*)"));
                if (file.isEmpty()) return;
                mutate(path, [&](ChainNode& n) {
                    std::get<VideoSourceParams>(n.params).filePath =
                        file.toStdString();
                });
                QMetaObject::invokeMethod(
                    this, [this, path] { buildPropertyEditor(path); },
                    Qt::QueuedConnection);
            });
            form->addRow(browseBtn);
        }
        {
            // Kamera-Wahl: die Liste zu zeigen oeffnet KEINE Kamera; das
            // Geraet startet erst nach der Freigabe (Knopf darunter) und nur,
            // wenn die Quelle "Kamera" gewaehlt ist — Vertrag Offene_Punkte §7.
            auto* combo = new QComboBox(m_propContainer);
            combo->addItem(tr("— none —"), QString());
            int selected = 0;
            const QString aktuelleId = QString::fromStdString(p->cameraId);
            const auto geraete = QMediaDevices::videoInputs();
            for (const QCameraDevice& d : geraete)
            {
                combo->addItem(d.description(), QString::fromUtf8(d.id()));
                if (QString::fromUtf8(d.id()) == aktuelleId)
                    selected = combo->count() - 1;
            }
            if (selected == 0 && !aktuelleId.isEmpty())
            {
                // Gespeicherte, gerade abwesende Kamera sichtbar halten —
                // sonst wuerde ein Panel-Besuch die Wahl still loeschen.
                combo->addItem(tr("(not connected) %1").arg(aktuelleId),
                               aktuelleId);
                selected = combo->count() - 1;
            }
            combo->setCurrentIndex(selected);
            combo->setToolTip(
                tr("Camera device for the source 'Camera'. It only starts "
                   "after the permission below — never when a preset is loaded."));
            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, [this, path, combo](int idx) {
                        const std::string id =
                            combo->itemData(idx).toString().toStdString();
                        mutate(path, [&](ChainNode& n) {
                            std::get<VideoSourceParams>(n.params).cameraId = id;
                        });
                    });
            form->addRow(tr("Camera"), combo);

            auto* camBtn = new QPushButton(
                tr("Allow camera (this app run)"), m_propContainer);
            camBtn->setToolTip(
                tr("Explicit permission: only then does the selected "
                   "camera start (Windows may ask for permission). Valid until "
                   "the app is closed."));
            camBtn->setEnabled(
                !lumi::services::LiveVideoFeed::instance().kameraErlaubt());
            connect(camBtn, &QPushButton::clicked, this, [camBtn]() {
                lumi::services::LiveVideoFeed::instance().erlaubeKamera();
                camBtn->setEnabled(false);
            });
            form->addRow(camBtn);
        }
        {
            // Testaufnahmen: Clips aus dem benutzerlokalen Ordner
            // (Settings-Tab „Kamera") — deterministischer Kamera-Stellvertreter.
            auto* combo = new QComboBox(m_propContainer);
            combo->addItem(tr("— none —"), QString());
            int selected = 0;
            const QString aktuell = QString::fromStdString(p->recordingName);
            const QDir ordner(
                lumi::services::LiveVideoFeed::testaufnahmenOrdner());
            const auto eintraege = ordner.entryList(QDir::Files, QDir::Name);
            for (const QString& name : eintraege)
            {
                combo->addItem(name, name);
                if (name == aktuell) selected = combo->count() - 1;
            }
            if (selected == 0 && !aktuell.isEmpty())
            {
                combo->addItem(tr("(missing) %1").arg(aktuell), aktuell);
                selected = combo->count() - 1;
            }
            combo->setCurrentIndex(selected);
            combo->setToolTip(
                tr("Camera clip as a deterministic source — runs "
                   "like a file (frame stepping, usable for probes)."));
            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, [this, path, combo](int idx) {
                        const std::string name =
                            combo->itemData(idx).toString().toStdString();
                        mutate(path, [&](ChainNode& n) {
                            std::get<VideoSourceParams>(n.params).recordingName =
                                name;
                        });
                    });
            form->addRow(tr("Camera Clip"), combo);
        }
        addBool("streaming", tr("Real-time streaming"), p->streaming,
                [](ChainNode& n, bool v) {
                    std::get<VideoSourceParams>(n.params).streaming = v;
                });
        addDouble("speed", tr("Speed"), p->speed, vs::kMinSpeed, vs::kMaxSpeed,
                  0.05, [](ChainNode& n, double v) {
                      std::get<VideoSourceParams>(n.params).speed = v;
                  });
        addBool("loop", tr("Loop"), p->loop, [](ChainNode& n, bool v) {
            std::get<VideoSourceParams>(n.params).loop = v;
        });
        addEnum("fit", tr("Fit"), std::clamp(p->fit, 0, vs::kFitMax),
                {tr("Stretch"), tr("Fit (letterbox)"),
                 tr("Fill (crop)")},
                [](ChainNode& n, int v) {
                    std::get<VideoSourceParams>(n.params).fit = v;
                });
        addEnum("blend", tr("Blend"), std::clamp(p->blend, 0, vs::kBlendMax),
                {tr("Replace"), tr("Additive"), tr("50/50")},
                [](ChainNode& n, int v) {
                    std::get<VideoSourceParams>(n.params).blend = v;
                });
        addDouble("opacity", tr("Opacity"), p->opacity, 0.0, 1.0, 0.01,
                  [](ChainNode& n, double v) {
                      std::get<VideoSourceParams>(n.params).opacity = v;
                  });
        addScript("initCode", tr("Init"), p->initCode,
                  [](ChainNode& n, std::string v) {
                      std::get<VideoSourceParams>(n.params).initCode =
                          std::move(v);
                  });
        addScript("frameCode", tr("Frame"), p->frameCode,
                  [](ChainNode& n, std::string v) {
                      std::get<VideoSourceParams>(n.params).frameCode =
                          std::move(v);
                  });
        addScript("beatCode", tr("Beat"), p->beatCode,
                  [](ChainNode& n, std::string v) {
                      std::get<VideoSourceParams>(n.params).beatCode =
                          std::move(v);
                  });
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

    // Song-Slot (S82) — fuer JEDEN Knoten mit Frame-Slot, am Ende des Editors:
    // laeuft einmal je Songwechsel vor dem Frame-Slot, in derselben Engine
    // (sieht und setzt dieselben Variablen). Ein Feld am Knoten, nicht je Typ.
    if (milkSection < 0 && milkElem < 0 && hatSongSlot(params))
    {
        addScript("songCode", tr("Song"), nodeSong,
                  [](ChainNode& n, std::string v) { n.songCode = std::move(v); });
    }

    begrenzeFeldbreiten(form);
    m_propLayout->addWidget(m_propPage);
    aktualisiereMindestbreite();
}

void MultiEffectPanel::begrenzeFeldbreiten(QFormLayout* form)
{
    // Texte (Beschreibungen, Hinweise, Fehler): umbrechen und die Seite nie
    // verbreitern. Mindestbreite 1 statt der Qt-Vorgabe (= laengstes Wort):
    // eine URL, ein Pfad oder eine lange Log-Zeile hat kein Leerzeichen und
    // schob die Seite sonst beliebig auf — jetzt wird so ein Wort am Rand
    // abgeschnitten, der Rest bricht um. Die Beschriftungen der Label-Spalte
    // bleiben einzeilig: zu ihnen scrollt man, wenn das Dock schmal ist.
    QSet<QWidget*> beschriftungen;
    for (int r = 0; r < form->rowCount(); ++r)
        if (QLayoutItem* it = form->itemAt(r, QFormLayout::LabelRole))
            if (QWidget* w = it->widget()) beschriftungen.insert(w);
    for (QLabel* label : m_propPage->findChildren<QLabel*>())
    {
        if (beschriftungen.contains(label)) continue;
        label->setWordWrap(true);
        label->setMinimumWidth(1);
    }

    // Combos: nicht nach dem laengsten Eintrag bemessen.
    for (QComboBox* combo : m_propPage->findChildren<QComboBox*>()) begrenzeCombo(combo);

    // Code- und Textfelder: an die Panelbreite gebunden, scrollen in sich
    // (Vorgabe Patrik S78) — ihre Qt-Mindestbreite ist ohnehin klein, hier nur
    // der gemeinsame Boden. Dasselbe fuer eingebettete Listen/Baeume (ISF).
    for (QAbstractScrollArea* area : m_propPage->findChildren<QAbstractScrollArea*>())
        if (qobject_cast<QPlainTextEdit*>(area) != nullptr ||
            qobject_cast<QAbstractItemView*>(area) != nullptr)
            area->setMinimumWidth(kFeldMinBreite);

    // Felder: gemeinsame Mindestbreite. Einfache Bedienelemente bekommen sie
    // fest (das deckelt zugleich breite Qt-Vorgaben, etwa Spinboxen mit
    // grossem Wertebereich). Zusammengesetzte Zeilen (eigenes Layout) nur, wenn
    // ihr Inhalt schmaler waere — eine feste Breite UNTER dem Inhalt wuerde
    // ihre Knoepfe uebereinanderschieben.
    for (int r = 0; r < form->rowCount(); ++r)
    {
        for (const auto rolle : {QFormLayout::FieldRole, QFormLayout::SpanningRole})
        {
            QLayoutItem* it = form->itemAt(r, rolle);
            QWidget* w = it != nullptr ? it->widget() : nullptr;
            if (w == nullptr || qobject_cast<QLabel*>(w) != nullptr) continue;
            if (w->layout() == nullptr)
                w->setMinimumWidth(kFeldMinBreite);
            else if (w->minimumSizeHint().width() < kFeldMinBreite)
                w->setMinimumWidth(kFeldMinBreite);
        }
    }
}

int MultiEffectPanel::editorRahmen() const
{
    // Alles, was im Editor zwischen Feld und Scrollarea-Aussenkante liegt. Den
    // senkrechten Balken immer einrechnen: sonst wuerde das Panel beim
    // Auftauchen des Balkens um dessen Breite zu schmal.
    const QMargins innen = m_propLayout->contentsMargins();
    return innen.left() + innen.right() + 2 * m_propScroll->frameWidth() +
           m_propScroll->verticalScrollBar()->sizeHint().width();
}

void MultiEffectPanel::aktualisiereMindestbreite()
{
    // Queued: eben eingefuegte Kinder zaehlen im Layout erst, wenn Qt sie
    // gezeigt hat (das passiert selbst queued nach addWidget).
    QTimer::singleShot(0, this, [this]() {
        if (m_propScroll == nullptr) return;
        // Waechter: jede Zeile, die nicht auf kFeldMinBreite schrumpfen kann,
        // haelt die ganze Feldspalte offen (Sichttest S78: die Preset-Zeile mit
        // „Save as…"). Ins Log, damit ein neues Modul nicht unbemerkt ausschert.
        if (m_propPage != nullptr)
            if (auto* form = qobject_cast<QFormLayout*>(m_propPage->layout()))
                for (int r = 0; r < form->rowCount(); ++r)
                {
                    QLayoutItem* it = form->itemAt(r, QFormLayout::FieldRole);
                    QWidget* w = it != nullptr ? it->widget() : nullptr;
                    if (w == nullptr) continue;
                    const int breite = std::max(w->minimumWidth(),
                                                w->minimumSizeHint().width());
                    if (breite <= kFeldMinBreite) continue;
                    QString name;
                    if (QLayoutItem* l = form->itemAt(r, QFormLayout::LabelRole))
                        if (auto* lbl = qobject_cast<QLabel*>(l->widget()))
                            name = lbl->text();
                    BasicLogger::logInfo(
                        "Effect Chain: Feldzeile '" + name.toStdString() +
                        "' braucht " + std::to_string(breite) + " px (> " +
                        std::to_string(kFeldMinBreite) + ")");
                }
        const int seite = m_propPage != nullptr
                              ? std::max(kFeldMinBreite, m_propPage->minimumSizeHint().width())
                              : kFeldMinBreite;
        m_propScroll->setMinimumWidth(seite + editorRahmen());
    });
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
