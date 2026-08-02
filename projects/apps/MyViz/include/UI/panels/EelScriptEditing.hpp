/**
 ****************************************************************************************
 * @file   EelScriptEditing.hpp
 * @brief  Shared EEL script-editing toolkit: highlighter, symbol categories,
 *         reference/editor dialogs (extracted from MultiEffectPanel, Session 40)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * The SSOT variable categories (Skript_Variablen_Konzept §3) with the MilkDrop
 * set from the M2 contract, the category-aware EEL syntax highlighter (§4,
 * error marking §6), the HTML reference-table helpers + built-ins page, and the
 * two dialogs (read-only reference popup, large expand editor). Used by the
 * MultiEffectPanel and, from M6 on, the MilkdropPanel — header-only so panels
 * just include it (EelHighlighter has no Q_OBJECT: no moc needed).
 ****************************************************************************************
 */

#pragma once

#include <QColor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QSet>
#include <QSplitter>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QVBoxLayout>

#include <array>
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

/// Full, resizable editor: big code pane + the module's reference side-by-side.
/// Returns true and fills `out` when accepted. `eelHighlight = false` lässt den
/// EEL-Highlighter weg — für Fremdsprachen-Felder (HLSL/GLSL, Strang S/S42),
/// deren Bezeichner die EEL-Kategorien nur falsch einfärben würden.
[[nodiscard]] inline bool openScriptEditor(QWidget* parent, const QString& label,
                                           const QString& text, const QString& refHtml,
                                           QString& out,
                                           const QSet<QString>& conflicts = {},
                                           bool eelHighlight = true)
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

} // namespace lumi::scriptedit
