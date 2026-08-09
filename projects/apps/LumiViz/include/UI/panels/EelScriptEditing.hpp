/**
 ****************************************************************************************
 * @file   EelScriptEditing.hpp
 * @brief  Shared EEL script-editing toolkit: highlighter, symbol categories,
 *         reference/editor dialogs (extracted from MultiEffectPanel, Session 40)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.1.0
 *
 * @details
 * The SSOT variable categories (Skript_Variablen_Konzept §3) with the MilkDrop
 * set from the M2 contract, the category-aware EEL syntax highlighter (§4,
 * error marking §6), the HTML reference-table helpers + built-ins page, and the
 * two dialogs (read-only reference popup, large expand editor). Used by the
 * MultiEffectPanel and, from M6 on, the MilkdropPanel — header-only so panels
 * just include it (EelHighlighter has no Q_OBJECT: no moc needed).
 *
 * 1.1.0 (S69, Offene_Punkte §7): der Groß-Editor bekommt optionale Hooks —
 * Apply (übernehmen + recompilen ohne Schließen, Fehlertext IM Dialog, mit
 * Nach-Polling für asynchrone GL-Compiles) und Beautify (ScriptFormatter,
 * Optionen aus den QSettings, Block "editor/...").
 ****************************************************************************************
 */

#pragma once

#include "scripting/ScriptFormatter.hpp"

#include <QColor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSet>
#include <QSettings>
#include <QSplitter>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTimer>
#include <QVBoxLayout>

#include <array>
#include <functional>
#include <utility>

namespace lumi::scriptedit {

/// SSOT variable categories (Skript_Variablen_Konzept §3). Drives highlighting
/// (§4) and error marking (§6). Only ReadOnly + Constant are "write = error".
enum class SymCat
{
    None,          ///< custom-local (unknown identifier) — default colour
    ReadOnly,      ///< host sets, script must not write (w, h, dt)
    Input,         ///< host sets per frame/point, script reads (audio, i, v, beat)
    Output,        ///< script sets, host reads (x, y, red/green/blue, skip)
    InOut,         ///< both (accumulators, driven params)
    Constant,      ///< fixed (pi, pi2)
    CustomGlobal   ///< preset-global user vars (reg00-99, q1-64, gmegabuf)
};

/// Global (module-independent) classification. The per-module reference gives
/// the precise contract; this is the broad map for colouring + error marking.
/// The ReadOnly/Constant sets are kept small on purpose (no false-positive errors).
[[nodiscard]] inline SymCat symbolCategory(const QString& n)
{
    static const QSet<QString> kReadOnly = {"w", "h", "dt"};
    // Nur was die Lua-Praeambel wirklich vorbelegt (LuaScriptEngine: pi, pi2).
    // `phi`/`e` standen hier zu Unrecht: es gibt sie ausschliesslich als
    // EEL-Konstanten `$PHI`/`$E`, die der Transpiler zur Uebersetzungszeit
    // durch ihre Zahl ersetzt — als blosser Bezeichner sind beide ganz
    // gewoehnliche Variablen. Im Korpus ist `e` in 128 Presets genau das (S54).
    static const QSet<QString> kConstant = {"pi", "pi2"};
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

/**
 * @class EelHighlighter
 * @brief EEL/expression highlighter (no Q_OBJECT: only overrides a virtual).
 *
 * Colours functions/numbers/comments + variables by category, and
 * red-underlines writes to read-only / constant identifiers and to conflicting
 * (double-declared) globals.
 */
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
            "getspec", "getspecdb", "getosc", "gettime"};
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
            // Eine Konstante zu ueberschreiben ist KEIN Fehler: `$PI` kam in
            // AVS erst spaeter, aeltere Presets setzen `pi` selbst — im Korpus
            // 629 Stueck, mehr als die 469 mit `$PI` (Befund Patrik, S54). Die
            // Sandbox laesst es zu (test_ParamScript), und sie muss es: die
            // Autoren meinten `3.14159`, nicht die volle Kreiszahl, und
            // zwanzig Presets meinen mit `pi` ueberhaupt keine (`pi=2`).
            if (cat == SymCat::ReadOnly || m_conflicts.contains(name))
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

// =============================================================================
// Reference HTML helpers
// =============================================================================

[[nodiscard]] inline QString refRow(const char* name, const char* type, const char* range,
                                    const char* desc)
{
    return QStringLiteral("<tr><td><b>%1</b></td><td><i>%2</i></td><td>%3</td><td>%4</td></tr>")
        .arg(QString::fromUtf8(name), QString::fromUtf8(type), QString::fromUtf8(range),
             QString::fromUtf8(desc));
}

/// One function-reference row: signature (with params) · return · description.
[[nodiscard]] inline QString fnRow(const char* sig, const char* ret, const char* desc)
{
    return QStringLiteral("<tr><td><code>%1</code></td><td><i>%2</i></td><td>%3</td></tr>")
        .arg(QString::fromUtf8(sig), QString::fromUtf8(ret), QString::fromUtf8(desc));
}

[[nodiscard]] inline QString fnTable(const QString& rows)
{
    return QStringLiteral(
               "<table cellspacing='0' cellpadding='4' style='border-collapse:collapse'>"
               "<tr><th align='left'>Function</th><th align='left'>Returns</th>"
               "<th align='left'>Description</th></tr>%1</table>")
        .arg(rows);
}

[[nodiscard]] inline QString varTable(const QString& rows)
{
    return QStringLiteral(
               "<table cellspacing='0' cellpadding='4' "
               "style='border-collapse:collapse'>"
               "<tr><th align='left'>Name</th><th align='left'>Type</th>"
               "<th align='left'>Range</th><th align='left'>Meaning</th></tr>%1</table>")
        .arg(rows);
}

/// The EEL built-ins shared by every scripted module (functions + constants).
[[nodiscard]] inline QString builtinsHtml()
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
        fnRow("getspecdb(band, width, ch)", "0..1",
              "getspec on the WebAudio dB scale (20·log10 mapped from -100..-30 dB "
              "— same scale as the Shadertoy audio texture). Quiet spectrum tails "
              "that read ≈0 linearly become usable (~0.5-0.8). LumiViz extra — "
              "not an AVS/MilkDrop builtin") +
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
               "<p>EEL form (replaced by its number at translation time, as in "
               "AVS): <code>$PI</code> 3.141592653589793 · <code>$E</code> "
               "2.71828183 · <code>$PHI</code> 1.61803399 (golden ratio)</p>"
               "<p style='color:#888'>Older presets predate <code>$PI</code> and "
               "define <code>pi</code> themselves — that is allowed and wins over "
               "the preset value above.</p>"
               "<p style='color:#888'>All values are floating-point. Assignment: "
               "<code>x = expr;</code> — statements separated by <code>;</code>. "
               "Comments: <code>// …</code></p>")
        .arg(math).arg(logic).arg(mem).arg(ctrl).arg(audio);
}

/// Colour legend matching the EelHighlighter category palette.
[[nodiscard]] inline QString highlightLegendHtml()
{
    return QStringLiteral(
        "<h3>Highlight colours</h3><p>"
        "<span style='color:#9CDCFE'>read-only</span> &nbsp; "
        "<span style='color:#4EC9B0'>input</span> &nbsp; "
        "<span style='color:#DCDCAA'>output</span> &nbsp; "
        "<span style='color:#C586C0'>in/out</span> &nbsp; "
        "<span style='color:#4FC1FF'><i>constant</i></span> &nbsp; "
        "<span style='color:#D7BA7D'>custom-global</span> &nbsp; "
        "custom-local &nbsp; "
        "<span style='color:#F44747'>error</span> (writing a read-only/constant)</p>");
}

// =============================================================================
// Dialogs
// =============================================================================

/// Read-only reference popup (resizable window).
inline void showScriptReference(QWidget* parent, const QString& html)
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

/// Format-Optionen der Beautify-Kerne aus den QSettings (gemeinsamer Block
/// "Editor" im Settings-Dialog, S69). Defaults = FormatOptions-Defaults.
[[nodiscard]] inline lumi::scripting::FormatOptions formatOptionsFromSettings()
{
    QSettings s;
    lumi::scripting::FormatOptions o;
    o.indentWidth =
        s.value(QStringLiteral("editor/indentWidth"), o.indentWidth).toInt();
    o.spaceAroundOperators =
        s.value(QStringLiteral("editor/spaceAroundOperators"), o.spaceAroundOperators)
            .toBool();
    o.maxBlankLines =
        s.value(QStringLiteral("editor/maxBlankLines"), o.maxBlankLines).toInt();
    return o;
}

/**
 * @brief Optionale Hooks des Groß-Editors (S69, Offene_Punkte §7).
 *
 * Ohne Hooks verhält sich der Dialog wie bisher (nur OK/Cancel). Alle
 * Callbacks werden aus dem GUI-Thread gerufen, solange der modale Dialog
 * offen ist — Aufrufer dürfen `this`/Widgets capturen.
 */
struct ScriptEditorHooks
{
    /// Apply-Knopf: Text übernehmen + recompilen, Dialog bleibt offen
    /// (Live-Tuning gegen den Viewport). Rückgabe = SOFORT bekannter
    /// Fehlertext ("" = fehlerfrei/unbekannt) — z. B. eine synchrone
    /// Transpiler-Probe. Kein Hook = kein Apply-Knopf.
    std::function<QString(const QString&)> apply;
    /// Verzögerte Fehlerabfrage für Compiles, die erst im Render-Thread
    /// laufen (Shadertoy-GL): wird nach einem fehlerfreien Apply mehrfach
    /// gepollt und aktualisiert die Fehlerzeile im Dialog.
    std::function<QString()> pollError;
    /// Beautify-Knopf: Text -> verschönerter Text (ScriptFormatter).
    /// Kein Hook = kein Beautify-Knopf.
    std::function<QString(const QString&)> beautify;
    /// Dateinamens-Vorschlag für den Shader-Export, Muster
    /// `<preset>[.<slot>].<vertrag>.<endung>` (S69, Schema S71). Nicht leer =
    /// der Dialog zeigt Import…/Export…-Knöpfe (Datei ↔ Editor-Text, UTF-8).
    QString exportFileName;
    /// Fertiger Herkunfts-Kopf (s. `herkunftKopf`), den der Export dem Code
    /// VORANSTELLT — sonst verliert eine exportierte Datei Autor und Lizenz
    /// ihres Originals (Befund S71, Lizenz-Pflicht S72). Leer = kein Kopf.
    QString herkunftKopf;
    /// Vertrag dieses Feldes (`ShaderVertrag::key`, z. B. "pixelfilter").
    /// Steuert den Import-Dateifilter, das Ordner-Gedächtnis
    /// (`editor/shaderFileDir/<vertrag>`) und die Vertragsprüfung beim
    /// Import. Leer = keine Prüfung, gemeinsamer Ordner.
    QString vertragKey;
};

/**
 * @brief Ein bekannter Shader-Vertrag: welcher Knoten erwartet welchen Einstieg
 *
 * SSOT fuer drei Dinge, die sonst auseinanderlaufen (S71): den Export-Namen,
 * den Import-Dateifilter und die Vertragspruefung beim Import. Die Vertraege
 * sind paarweise UNVEREINBAR — ein Shadertoy-Shader in einem pixelFilter
 * ergibt nur einen kryptischen Compilerfehler tief aus dem Wrapper.
 */
struct ShaderVertrag
{
    QString key;       ///< Namensbestandteil/Ordner, z. B. "shadertoy"
    QString anzeige;   ///< Klartext fuer Meldungen, z. B. "Shadertoy"
    QString funktion;  ///< Einstiegspunkt, z. B. "mainImage"
    QString signatur;  ///< vollstaendige Signatur fuer die Meldung
    QString endung;    ///< "glsl" oder "hlsl" — was die Datei wirklich ist
    /// Weitere Muster, die im Dateidialog ZUERST auftauchen sollen, weil der
    /// Knoten sie direkt annimmt (S72: der Stilfilter uebersetzt ISF-`.fs`
    /// automatisch). Ohne diesen Eintrag zeigte die Vorauswahl nur
    /// `*.pixelfilter.glsl` — eine frisch heruntergeladene ISF-Datei war im
    /// Dialog schlicht unsichtbar.
    QString importZusatz;
};

/// Alle bekannten Vertraege (SSOT). Reihenfolge egal; `key` ist eindeutig.
[[nodiscard]] inline const QList<ShaderVertrag>& shaderVertraege()
{
    static const QList<ShaderVertrag> kVertraege = {
        {QStringLiteral("pixelfilter"), QStringLiteral("Stilfilter"),
         QStringLiteral("farbe"), QStringLiteral("vec4 farbe(vec2 uv, vec4 src)"),
         QStringLiteral("glsl")},
        // ISF-Filter (S72, Entscheid Patrik): eigener Knoten mit BEIDEN
        // Shader-Stufen. `main()` ist hier der Einstieg — ISF-Dateien werden
        // NICHT auf einen eigenen Vertrag umgeschrieben, sie laufen wie sie
        // sind. `*.fs` gehoert darum in die Vorauswahl des Dateidialogs.
        {QStringLiteral("isffilter"), QStringLiteral("ISF-Filter"),
         QStringLiteral("main"), QStringLiteral("void main()"),
         QStringLiteral("fs"), QStringLiteral("*.fs")},
        {QStringLiteral("shadertoy"), QStringLiteral("Shadertoy"),
         QStringLiteral("mainImage"),
         QStringLiteral("void mainImage(out vec4 fragColor, in vec2 fragCoord)"),
         QStringLiteral("glsl")},
        {QStringLiteral("meshwarp"), QStringLiteral("Mesh-Warp"),
         QStringLiteral("warp"), QStringLiteral("vec2 warp(vec2 uv)"),
         QStringLiteral("glsl")},
        {QStringLiteral("gpuparticles"), QStringLiteral("GPU-Partikel"),
         QStringLiteral("kraft"),
         QStringLiteral("vec2 kraft(vec2 pos, vec2 vel, float alter)"),
         QStringLiteral("glsl")},
        // Milkdrop-Felder sind HLSL — die Endung sagt das jetzt auch, damit
        // externe Editoren richtig einfaerben (Entscheid Patrik S71).
        {QStringLiteral("milkdrop"), QStringLiteral("MilkDrop"),
         QStringLiteral("shader_body"), QStringLiteral("shader_body { … }"),
         QStringLiteral("hlsl")},
    };
    return kVertraege;
}

/// Vertrag zu einem Schluessel (nullptr = unbekannt).
[[nodiscard]] inline const ShaderVertrag* shaderVertrag(const QString& key)
{
    for (const auto& v : shaderVertraege())
        if (v.key == key) return &v;
    return nullptr;
}

/// Kommt `funktion` im Text als AUFRUF/Definition vor (Wortgrenze + Klammer)?
[[nodiscard]] inline bool nenntFunktion(const QString& text, const QString& funktion)
{
    const QRegularExpression re(QStringLiteral("\\b") +
                                QRegularExpression::escape(funktion) +
                                QStringLiteral("\\s*[({]"));
    return re.match(text).hasMatch();
}

/// Traegt der Text einen eingebetteten ISF-JSON-Kopf (`/*{ … }*/` am Anfang)?
/// Bei ISF steckt die Parameter-Deklaration IM Shader, nicht in einer
/// Nachbardatei — eine `.fs` ist also fuer sich vollstaendig.
[[nodiscard]] inline bool istIsfDatei(const QString& text)
{
    const QString kopf = text.left(400).trimmed();
    return kopf.startsWith(QStringLiteral("/*")) &&
           kopf.contains(QLatin1Char('{')) &&
           (kopf.contains(QStringLiteral("\"ISFVSN\"")) ||
            kopf.contains(QStringLiteral("\"INPUTS\"")) ||
            kopf.contains(QStringLiteral("\"CATEGORIES\"")));
}

/**
 * @brief Passt `text` zum erwarteten Vertrag? Sonst: was steckt drin?
 * @param text       eingelesener Dateiinhalt
 * @param eigenerKey Vertrag des Feldes, in das geladen werden soll
 * @return leerer String = passt (oder nichts Bekanntes erkannt, dann nicht
 *         meckern); sonst ein fertiger Warntext samt Handlungsempfehlung.
 */
[[nodiscard]] inline QString pruefeShaderVertrag(const QString& text,
                                                 const QString& eigenerKey)
{
    const ShaderVertrag* eigen = shaderVertrag(eigenerKey);
    if (eigen == nullptr) return {};
    // Erwarteter Einstieg vorhanden => passt, keine Warnung.
    if (nenntFunktion(text, eigen->funktion)) return {};
    if (istIsfDatei(text))
    {
        return QObject::tr(
            "Diese Datei ist ein <b>ISF-Shader</b> (Parameter-Deklaration als "
            "JSON im Kopf).<br>Der Vertrag hier lautet <code>%1</code>.<br><br>"
            "ISF-Dateien lassen sich noch nicht automatisch übersetzen — der "
            "Code kann aber als Vorlage dienen. Trotzdem laden?")
            .arg(eigen->signatur);
    }
    for (const auto& fremd : shaderVertraege())
    {
        if (fremd.key == eigenerKey) continue;
        // `main` identifiziert KEIN Format — es steht in fast jedem Shader.
        // Der ISF-Knoten hat es als Einstieg; wuerde er hier mitlaufen, waere
        // jede gewoehnliche `void main()`-Datei angeblich ein ISF-Filter.
        // ISF wird stattdessen an seinem JSON-Kopf erkannt (Zweig oben).
        if (fremd.funktion == QLatin1String("main")) continue;
        if (!nenntFunktion(text, fremd.funktion)) continue;
        return QObject::tr(
            "Diese Datei sieht nach einem <b>%1</b>-Shader aus "
            "(<code>%2</code>).<br>Hier wird aber <code>%3</code> erwartet.<br><br>"
            "Dafür ist der <b>%1</b>-Knoten zuständig. Trotzdem laden?")
            .arg(fremd.anzeige, fremd.funktion, eigen->signatur);
    }
    return {};  // nichts Bekanntes erkannt — Fragment/Hilfsfunktionen erlauben
}

/**
 * @brief Freien Dateinamen im Ordner finden: `name.ext` → `name(2).ext`, …
 *
 * Der Zaehler sitzt VOR den Endungen, damit `.vertrag.endung` intakt bleibt
 * (`preset(2).image.shadertoy.glsl`, nicht `preset.image.shadertoy(2).glsl`).
 * Qt fragt beim Speichern zwar ohnehin nach dem Ueberschreiben — ein freier
 * Vorschlag verhindert aber, dass man in die Lage ueberhaupt kommt.
 */
[[nodiscard]] inline QString freierDateiname(const QString& ordner,
                                             const QString& vorschlag)
{
    if (ordner.isEmpty() || !QFileInfo::exists(ordner + QLatin1Char('/') + vorschlag))
        return vorschlag;
    // Stamm = alles vor der ERSTEN Endung, Rest = alle Endungen zusammen.
    const int punkt = vorschlag.indexOf(QLatin1Char('.'));
    const QString stamm = punkt < 0 ? vorschlag : vorschlag.left(punkt);
    const QString endungen = punkt < 0 ? QString() : vorschlag.mid(punkt);
    for (int i = 2; i < 1000; ++i)
    {
        const QString kandidat =
            stamm + QLatin1Char('(') + QString::number(i) + QLatin1Char(')') + endungen;
        if (!QFileInfo::exists(ordner + QLatin1Char('/') + kandidat)) return kandidat;
    }
    return vorschlag;
}

/**
 * @brief Export-Namensvorschlag `<preset>[.<slot>].<vertrag>.<endung>`
 *
 * Klassifikation von RECHTS nach LINKS immer spezifischer (Entscheid Patrik
 * S71, Muster der Dateiendungen selbst): Dateityp · Vertrag · Slot · Name.
 * @param slot optionaler Feld-Anteil (z. B. "image", "bufferA", "warp")
 */
[[nodiscard]] inline QString shaderExportName(QString presetName,
                                              const QString& vertragKey,
                                              const QString& slot = {})
{
    static const QRegularExpression kBad(QStringLiteral("[\\\\/:*?\"<>|]"));
    static const QRegularExpression kWs(QStringLiteral("\\s+"));
    presetName.replace(kBad, QString());
    presetName = presetName.trimmed();
    presetName.replace(kWs, QStringLiteral("_"));
    if (presetName.isEmpty()) presetName = QStringLiteral("preset");
    const ShaderVertrag* v = shaderVertrag(vertragKey);
    const QString endung = v != nullptr ? v->endung : QStringLiteral("glsl");
    QString name = presetName;
    if (!slot.isEmpty()) name += QLatin1Char('.') + slot;
    return name + QLatin1Char('.') + vertragKey + QLatin1Char('.') + endung;
}

/**
 * @brief Herkunfts-Kopf für den Shader-Export (Lizenz-Pflicht, S72)
 *
 * BEFUND S71: der Export schrieb nur den Code — Autor und Lizenz eines
 * importierten Shaders gingen dabei verloren. Wer die Datei weitergab, gab sie
 * ohne jeden Hinweis auf ihre Herkunft weiter. Der Kopf schließt das.
 *
 * Der Block beginnt bewusst mit `/* LumiViz` und NICHT mit `/*{` — letzteres
 * ist der ISF-JSON-Kopf, und ein Export darf nie versehentlich wie eine
 * ISF-Datei aussehen (ein Wächter-Test hält das fest). Alle vier Felder leer
 * ⇒ leerer Rückgabewert, dann wird gar nichts vorangestellt.
 */
[[nodiscard]] inline QString herkunftKopf(const QString& name, const QString& autor,
                                          const QString& url, const QString& lizenz)
{
    if (name.isEmpty() && autor.isEmpty() && url.isEmpty() && lizenz.isEmpty())
        return {};
    QString k = QStringLiteral("/* LumiViz-Export — Herkunft des Inhalts\n");
    const auto zeile = [&k](const char* feld, const QString& wert) {
        if (!wert.isEmpty())
            k += QStringLiteral("   %1: %2\n")
                     .arg(QString::fromUtf8(feld), QString(wert).replace(
                                                       QStringLiteral("*/"),
                                                       QStringLiteral("*_/")));
    };
    zeile("Titel", name);
    zeile("Autor", autor);
    zeile("Quelle", url);
    zeile("Lizenz", lizenz);
    k += QStringLiteral("   Beim Weitergeben gilt diese Lizenz.\n*/\n");
    return k;
}

/// Full, resizable editor: big code pane + the module's reference side-by-side.
/// Returns true and fills `out` when accepted. `eelHighlight = false` lässt den
/// EEL-Highlighter weg — für Fremdsprachen-Felder (HLSL/GLSL, Strang S/S42),
/// deren Bezeichner die EEL-Kategorien nur falsch einfärben würden.
/// `hooks` (S69): Apply-/Beautify-Knöpfe + Fehlerzeile im Dialog.
[[nodiscard]] inline bool openScriptEditor(QWidget* parent, const QString& label,
                                           const QString& text, const QString& refHtml,
                                           QString& out,
                                           const QSet<QString>& conflicts = {},
                                           bool eelHighlight = true,
                                           const ScriptEditorHooks& hooks = {})
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
    if (eelHighlight) new EelHighlighter(editor->document(), conflicts);
    auto* ref = new QTextBrowser(split);
    ref->setHtml(refHtml);
    split->addWidget(editor);
    split->addWidget(ref);
    split->setStretchFactor(0, 3);
    split->setStretchFactor(1, 2);
    lay->addWidget(split, 1);

    // Status-/Fehlerzeile (S69): zeigt das Apply-Ergebnis IM Dialog — nicht
    // nur im Panel dahinter (Offene_Punkte §7 Punkt 1).
    auto* status = new QLabel(&dlg);
    status->setWordWrap(true);
    status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    status->setVisible(false);
    lay->addWidget(status);
    const auto setStatus = [status](const QString& err) {
        if (err.isEmpty())
        {
            status->setStyleSheet(QStringLiteral("color:#7fbf7f"));
            status->setText(QObject::tr("✓ Übernommen — keine Fehler."));
        }
        else
        {
            status->setStyleSheet(QStringLiteral("color:#d08080"));
            status->setText(QStringLiteral("⚠ ") + err);
        }
        status->setVisible(true);
    };

    QDialogButtonBox::StandardButtons buttons =
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel;
    if (hooks.apply) buttons |= QDialogButtonBox::Apply;
    auto* bb = new QDialogButtonBox(buttons, &dlg);
    if (!hooks.exportFileName.isEmpty())
    {
        // Shader-Datei ↔ Editor-Text (Wunsch Patrik, S69; Vertrags-Trennung
        // S71). Der Ordner wird JE VERTRAG gemerkt — ein gemeinsames
        // Gedaechtnis fuehrte sonst direkt in die Verwechslung, weil die
        // Vertraege unvereinbar sind.
        const ShaderVertrag* vertrag = shaderVertrag(hooks.vertragKey);
        const QString eigeneEndung =
            vertrag != nullptr ? vertrag->endung : QStringLiteral("glsl");
        // Der eigene Vertrag steht zuerst; ISF (.fs/.vs) ist mit aufgefuehrt,
        // weil das die Konvention der groessten Filter-Fundgrube ist.
        QString passend = QStringLiteral("*.%1.%2").arg(
            hooks.vertragKey.isEmpty() ? QStringLiteral("*") : hooks.vertragKey,
            eigeneEndung);
        if (vertrag != nullptr && !vertrag->importZusatz.isEmpty())
            passend += QLatin1Char(' ') + vertrag->importZusatz;
        const QString kRest =
            QObject::tr(";;Shader (*.glsl *.hlsl *.frag *.vert *.fs *.vs *.txt)"
                        ";;Alle Dateien (*)");
        const QString filter = QObject::tr("Passende Shader (%1)").arg(passend) + kRest;
        // Beim EXPORT ohne den Import-Zusatz: wir schreiben nie ISF, `*.fs`
        // waere dort ein falsches Versprechen.
        const QString exportFilter =
            QObject::tr("Passende Shader (*.%1.%2)")
                .arg(hooks.vertragKey.isEmpty() ? QStringLiteral("*")
                                                : hooks.vertragKey,
                     eigeneEndung) +
            kRest;
        const QString dirKey = QStringLiteral("editor/shaderFileDir/") +
                               (hooks.vertragKey.isEmpty()
                                    ? QStringLiteral("allgemein")
                                    : hooks.vertragKey);
        const auto rememberedDir = [dirKey]() {
            QSettings s;
            QString d = s.value(dirKey, QString()).toString();
            // Ausweich auf das alte gemeinsame Gedaechtnis (Bestand vor S71)
            if (d.isEmpty())
                d = s.value(QStringLiteral("editor/shaderFileDir"), QString())
                        .toString();
            return d;
        };
        const auto rememberDir = [dirKey](const QString& filePath) {
            QSettings().setValue(dirKey, QFileInfo(filePath).absolutePath());
        };
        const QString vertragKey = hooks.vertragKey;
        auto* importBtn =
            bb->addButton(QObject::tr("Import…"), QDialogButtonBox::ActionRole);
        importBtn->setToolTip(
            QObject::tr("Shader-Datei laden — ersetzt den Editor-Inhalt "
                        "(übernommen wird erst mit Apply/OK)."));
        QObject::connect(
            importBtn, &QPushButton::clicked, &dlg,
            [editor, filter, vertragKey, rememberedDir, rememberDir, &dlg]() {
                const QString path = QFileDialog::getOpenFileName(
                    &dlg, QObject::tr("Shader importieren"), rememberedDir(),
                    filter);
                if (path.isEmpty()) return;
                QFile f(path);
                if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    QMessageBox::warning(
                        &dlg, QObject::tr("Import"),
                        QObject::tr("Datei nicht lesbar: %1").arg(path));
                    return;
                }
                const QString inhalt = QString::fromUtf8(f.readAll());
                // VERTRAGSPRUEFUNG (S71): Ordner und Filter helfen nur beim
                // gewohnten Weg — aus dem Download-Ordner greift keiner von
                // beiden. Deshalb hier in den Inhalt schauen und WARNEN, statt
                // den Nutzer in einen kryptischen Shader-Compilerfehler
                // laufen zu lassen. Bewusst kein Verbot: Fragmente und
                // Hilfsfunktionen zu laden bleibt erlaubt.
                const QString warnung = pruefeShaderVertrag(inhalt, vertragKey);
                if (!warnung.isEmpty())
                {
                    QMessageBox box(&dlg);
                    box.setIcon(QMessageBox::Warning);
                    box.setWindowTitle(QObject::tr("Anderer Shader-Vertrag"));
                    box.setTextFormat(Qt::RichText);
                    box.setText(warnung);
                    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                    box.setDefaultButton(QMessageBox::No);
                    box.button(QMessageBox::Yes)
                        ->setText(QObject::tr("Trotzdem laden"));
                    box.button(QMessageBox::No)->setText(QObject::tr("Abbrechen"));
                    if (box.exec() != QMessageBox::Yes) return;
                }
                editor->setPlainText(inhalt);
                rememberDir(path);
            });
        const QString exportName = hooks.exportFileName;
        const QString kopf = hooks.herkunftKopf;
        auto* exportBtn =
            bb->addButton(QObject::tr("Export…"), QDialogButtonBox::ActionRole);
        exportBtn->setToolTip(
            QObject::tr("Editor-Inhalt als Datei speichern (Vorschlag: %1).")
                .arg(exportName));
        QObject::connect(
            exportBtn, &QPushButton::clicked, &dlg,
            [editor, exportFilter, exportName, kopf, rememberedDir, rememberDir,
             &dlg]() {
                const QString dir = rememberedDir();
                // Freien Namen vorschlagen, statt auf die Ueberschreib-Frage
                // zu warten (Wunsch Patrik S71).
                const QString name = freierDateiname(dir, exportName);
                QString start =
                    dir.isEmpty() ? name : dir + QLatin1Char('/') + name;
                const QString path = QFileDialog::getSaveFileName(
                    &dlg, QObject::tr("Shader exportieren"), start, exportFilter);
                if (path.isEmpty()) return;
                QFile f(path);
                if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate |
                            QIODevice::Text))
                {
                    QMessageBox::warning(
                        &dlg, QObject::tr("Export"),
                        QObject::tr("Datei nicht schreibbar: %1").arg(path));
                    return;
                }
                // Herkunft VORANSTELLEN (S72) — nur, wenn der Code sie nicht
                // ohnehin schon traegt (Export einer zuvor exportierten Datei).
                QString inhalt = editor->toPlainText();
                if (!kopf.isEmpty() &&
                    !inhalt.contains(QStringLiteral("LumiViz-Export")))
                {
                    inhalt = kopf + inhalt;
                }
                f.write(inhalt.toUtf8());
                rememberDir(path);
            });
    }
    if (hooks.beautify)
    {
        auto* beautifyBtn =
            bb->addButton(QObject::tr("Beautify"), QDialogButtonBox::ActionRole);
        beautifyBtn->setToolTip(
            QObject::tr("Code neu formatieren (nur Weißraum — Einstellungen "
                        "unter Settings → Editor). Übernimmt noch nichts."));
        const auto beautifyFn = hooks.beautify;
        QObject::connect(beautifyBtn, &QPushButton::clicked, &dlg,
                         [editor, beautifyFn]() {
                             // Cursor-Zeile grob erhalten (Text wird ersetzt).
                             const int scroll = editor->verticalScrollBar() != nullptr
                                                    ? editor->verticalScrollBar()->value()
                                                    : 0;
                             editor->setPlainText(beautifyFn(editor->toPlainText()));
                             if (editor->verticalScrollBar() != nullptr)
                                 editor->verticalScrollBar()->setValue(scroll);
                         });
    }
    if (hooks.apply)
    {
        auto* applyBtn = bb->button(QDialogButtonBox::Apply);
        applyBtn->setToolTip(
            QObject::tr("Übernehmen ohne Schließen — die Chain recompiliert, "
                        "der Viewport zeigt den Stand sofort."));
        const auto applyFn = hooks.apply;
        const auto pollFn = hooks.pollError;
        QObject::connect(
            applyBtn, &QPushButton::clicked, &dlg,
            [editor, applyFn, pollFn, setStatus, &dlg]() {
                const QString err = applyFn(editor->toPlainText());
                setStatus(err);
                // GL-Compiles laufen erst im Render-Thread: kurz nachpollen,
                // damit der Treiber-Fehler (oder das Verschwinden eines alten)
                // noch im offenen Dialog ankommt.
                if (err.isEmpty() && pollFn)
                    for (const int delayMs : {300, 900, 1800})
                        QTimer::singleShot(delayMs, &dlg, [pollFn, setStatus]() {
                            setStatus(pollFn());
                        });
            });
    }
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

} // namespace lumi::scriptedit
