/**
 ****************************************************************************************
 * @file   ScriptFormatter.hpp
 * @brief  Pure text beautifiers for the chain-script editors: EEL statement
 *         re-flow + brace-based GLSL/HLSL re-indent (Offene_Punkte §7, S69)
 *
 * @author LumiPulse Team
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Deliberately minimal, clang-format-like only in spirit (Entscheid Patrik,
 * 2026-08-05): the two entry points rewrite WHITESPACE ONLY — every token of
 * the input survives byte-identical (comments included), so a beautify can
 * never change what a script computes. No Qt, no allocation tricks: plain
 * std::string in/out, fully unit-testable (doctest).
 *
 * - beautifyEel(): one statement per line (break after `;`), indent by paren
 *   depth (covers loop(/if(/exec2/3( bodies and `(a;b)` blocks alike),
 *   optional single spaces around binary operators, blank-line runs clamped.
 * - beautifyGlsl(): line-based brace re-indent (also fits HLSL shader_body),
 *   preprocessor lines at column 0, block-comment interiors untouched,
 *   blank-line runs clamped. Token spacing within a line is preserved.
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

namespace lumi::scripting {

/// Shared knobs of both beautifiers (Settings-Dialog: gemeinsamer Block).
struct FormatOptions
{
    int indentWidth = 4;               ///< Leerzeichen je Einzugsstufe (>= 0)
    bool spaceAroundOperators = true;  ///< nur EEL: ` = ` statt `=` (aus: kompakt)
    int maxBlankLines = 2;             ///< max. aufeinanderfolgende Leerzeilen (>= 0)
};

namespace fmtdetail {

[[nodiscard]] inline bool isIdentChar(char c)
{
    return (std::isalnum(static_cast<unsigned char>(c)) != 0) || c == '_';
}

/// Strip comments/strings from one GLSL line for brace counting. `inBlock`
/// carries the /* ... */ state across lines.
[[nodiscard]] inline std::string glslCodeOnly(std::string_view line, bool& inBlock)
{
    std::string code;
    for (std::size_t i = 0; i < line.size(); ++i)
    {
        if (inBlock)
        {
            if (line[i] == '*' && i + 1 < line.size() && line[i + 1] == '/')
            {
                inBlock = false;
                ++i;
            }
            continue;
        }
        if (line[i] == '/' && i + 1 < line.size())
        {
            if (line[i + 1] == '/') break;  // Zeilenkommentar: Rest ignorieren
            if (line[i + 1] == '*')
            {
                inBlock = true;
                ++i;
                continue;
            }
        }
        code += line[i];
    }
    return code;
}

}  // namespace fmtdetail

/**
 * @brief Brace-based re-indent for GLSL/HLSL sources (whitespace-only rewrite).
 *
 * Each line keeps its own token spacing; only the leading indent is rebuilt
 * from the brace depth. Lines inside block comments are left untouched;
 * `#`-preprocessor lines go to column 0. Blank-line runs are clamped to
 * opt.maxBlankLines.
 */
[[nodiscard]] inline std::string beautifyGlsl(std::string_view src,
                                              const FormatOptions& opt = {})
{
    const int width = std::max(0, opt.indentWidth);
    const int maxBlank = std::max(0, opt.maxBlankLines);

    std::string out;
    out.reserve(src.size() + src.size() / 8);
    int depth = 0;
    bool inBlock = false;
    int blankRun = 0;
    bool any = false;  // schon eine Inhaltszeile ausgegeben?

    std::size_t pos = 0;
    while (pos <= src.size())
    {
        const std::size_t nl = src.find('\n', pos);
        std::string_view line = src.substr(
            pos, nl == std::string_view::npos ? std::string_view::npos : nl - pos);
        const bool last = (nl == std::string_view::npos);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);

        if (inBlock)
        {
            // Innere Block-Kommentar-Zeilen unangetastet lassen (nur rtrim).
            std::string_view kept = line;
            while (!kept.empty() &&
                   std::isspace(static_cast<unsigned char>(kept.back())) != 0)
                kept.remove_suffix(1);
            const std::string code = fmtdetail::glslCodeOnly(line, inBlock);
            for (const char c : code)
            {
                if (c == '{') ++depth;
                if (c == '}') depth = std::max(0, depth - 1);
            }
            out.append(kept);
            out += '\n';
            any = true;
        }
        else
        {
            std::string_view t = line;
            while (!t.empty() && std::isspace(static_cast<unsigned char>(t.front())) != 0)
                t.remove_prefix(1);
            while (!t.empty() && std::isspace(static_cast<unsigned char>(t.back())) != 0)
                t.remove_suffix(1);
            if (t.empty())
            {
                ++blankRun;
            }
            else
            {
                if (any)
                    for (int i = 0; i < std::min(blankRun, maxBlank); ++i) out += '\n';
                blankRun = 0;
                const std::string code = fmtdetail::glslCodeOnly(t, inBlock);
                // Beginnt die Zeile mit '}', bestimmt der INNERSTE Closer die
                // Spalte (eine Stufe zurueck) — weitere '}' auf derselben
                // Zeile ruecken sie nicht weiter nach links.
                const int leadingClose = (t.front() == '}') ? 1 : 0;
                const int indent =
                    (t.front() == '#') ? 0 : std::max(0, depth - leadingClose);
                out.append(static_cast<std::size_t>(indent) *
                               static_cast<std::size_t>(width),
                           ' ');
                out.append(t);
                out += '\n';
                any = true;
                for (const char c : code)
                {
                    if (c == '{') ++depth;
                    if (c == '}') depth = std::max(0, depth - 1);
                }
            }
        }
        if (last) break;
        pos = nl + 1;
    }
    // Genau ein '\n' je Zeile — das letzte entfernen, wenn Inhalt da ist.
    if (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

/**
 * @brief EEL statement re-flow (whitespace-only rewrite).
 *
 * One statement per line (break after each `;`), indent = paren depth at the
 * line start (a leading `)` uses the enclosing level). Trailing `// …`
 * comments stay glued to their statement; standalone comments keep their own
 * line. With opt.spaceAroundOperators, binary operators get single spaces
 * (unary +/-/! stay attached); off = compact (no spaces around operators).
 * Blank-line runs between statements are clamped to opt.maxBlankLines.
 */
[[nodiscard]] inline std::string beautifyEel(std::string_view src,
                                             const FormatOptions& opt = {})
{
    using fmtdetail::isIdentChar;
    const int width = std::max(0, opt.indentWidth);
    const int maxBlank = std::max(0, opt.maxBlankLines);

    std::string out;
    out.reserve(src.size() + src.size() / 4);
    std::string line;       // aktuelle Zeile OHNE Einzug
    int lineIndent = 0;     // Einzugsstufe der aktuellen Zeile
    int depth = 0;          // aktuelle Klammertiefe
    int nlPending = 0;      // Input-Zeilenumbrüche seit dem letzten Token
    int nlSinceSemi = 0;    // Umbrüche seit dem letzten ';' (Kommentar-Kleben)
    bool pendingBreak = false;  // ';' gesehen — Umbruch vor dem nächsten Token
    bool needSpace = false;     // erzwungener Abstand (nach Block-Kommentar)
    // true = das zuletzt ausgegebene Token kann links von einem BINÄREN
    // Operator stehen (Operand); false = Operator/Beginn → +/-/! sind unär.
    bool prevOperand = false;

    const auto flushLine = [&]() {
        if (!line.empty())
        {
            out.append(static_cast<std::size_t>(lineIndent) *
                           static_cast<std::size_t>(width),
                       ' ');
            out.append(line);
            out += '\n';
            line.clear();
        }
        needSpace = false;
    };
    const auto rtrimLine = [&]() {
        while (!line.empty() && line.back() == ' ') line.pop_back();
    };
    // Zeilenbeginn vorbereiten: aufgestauten Umbruch + geklemmte Leerzeilen
    // ausgeben, Einzug der neuen Zeile festhalten (closer eine Stufe zurück).
    // Umbruch-Zähler wird je Token verbraucht — Umbrüche MITTEN in einem
    // Statement sind bloßer Weißraum, nur an Statement-Grenzen Leerzeilen.
    const auto beginToken = [&](bool tokenIsCloser) {
        if (pendingBreak)
        {
            flushLine();
            pendingBreak = false;
        }
        if (line.empty())
        {
            if (!out.empty())
                for (int i = 0; i < std::min(std::max(nlPending - 1, 0), maxBlank);
                     ++i)
                    out += '\n';
            lineIndent = std::max(0, depth - (tokenIsCloser ? 1 : 0));
        }
        nlPending = 0;
    };
    const auto append = [&](std::string_view text, bool operand) {
        if (needSpace && !line.empty()) line += ' ';
        needSpace = false;
        if (!line.empty() && operand && isIdentChar(line.back())) line += ' ';
        line.append(text);
        prevOperand = operand;
    };

    std::size_t i = 0;
    const std::size_t n = src.size();
    while (i < n)
    {
        const char c = src[i];
        // --- Weißraum ---------------------------------------------------------
        if (c == '\n')
        {
            ++nlPending;
            ++nlSinceSemi;
            ++i;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(c)) != 0)
        {
            ++i;
            continue;
        }
        // --- Kommentare -------------------------------------------------------
        if (c == '/' && i + 1 < n && src[i + 1] == '/')
        {
            std::size_t end = src.find('\n', i);
            if (end == std::string_view::npos) end = n;
            std::string_view comment = src.substr(i, end - i);
            while (!comment.empty() &&
                   std::isspace(static_cast<unsigned char>(comment.back())) != 0)
                comment.remove_suffix(1);
            if (pendingBreak && nlSinceSemi == 0)
            {
                // Kommentar hinter dem ';' derselben Input-Zeile: ankleben.
                rtrimLine();
                line += "  ";
                line.append(comment);
                flushLine();
                pendingBreak = false;
            }
            else
            {
                beginToken(false);
                if (!line.empty())
                {
                    rtrimLine();
                    line += "  ";
                    line.append(comment);
                }
                else
                {
                    line.append(comment);
                }
                flushLine();
            }
            i = end;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*')
        {
            std::size_t end = src.find("*/", i + 2);
            end = (end == std::string_view::npos) ? n : end + 2;
            beginToken(false);
            append(src.substr(i, end - i), /*operand=*/false);
            needSpace = true;
            i = end;
            continue;
        }
        // --- Interpunktion ----------------------------------------------------
        if (c == ';')
        {
            beginToken(false);
            rtrimLine();
            line += ';';
            pendingBreak = true;
            nlSinceSemi = 0;
            needSpace = false;
            prevOperand = false;
            ++i;
            continue;
        }
        if (c == ',')
        {
            beginToken(false);
            rtrimLine();
            line += ", ";
            needSpace = false;
            prevOperand = false;
            ++i;
            continue;
        }
        if (c == '(')
        {
            beginToken(false);
            line += '(';
            ++depth;
            prevOperand = false;
            ++i;
            continue;
        }
        if (c == ')')
        {
            beginToken(/*tokenIsCloser=*/true);
            rtrimLine();
            line += ')';
            depth = std::max(0, depth - 1);
            needSpace = false;
            prevOperand = true;
            ++i;
            continue;
        }
        // --- Operanden --------------------------------------------------------
        if (isIdentChar(c) || c == '$' || c == '.')
        {
            std::size_t end = i;
            if (c == '$')
            {
                ++end;
                if (end < n && src[end] == '\'')  // $'c'
                {
                    end = std::min(n, end + 3);
                }
                else
                {
                    while (end < n && isIdentChar(src[end])) ++end;
                }
            }
            else
            {
                const bool number =
                    (std::isdigit(static_cast<unsigned char>(c)) != 0) || c == '.';
                while (end < n && (isIdentChar(src[end]) || src[end] == '.')) ++end;
                // Exponent 1e-5 / 2E+3 gehört zur Zahl.
                if (number && end < n && end > i &&
                    (src[end - 1] == 'e' || src[end - 1] == 'E') &&
                    (src[end] == '+' || src[end] == '-'))
                {
                    ++end;
                    while (end < n && isIdentChar(src[end])) ++end;
                }
            }
            beginToken(false);
            append(src.substr(i, end - i), /*operand=*/true);
            i = end;
            continue;
        }
        // --- Operatoren -------------------------------------------------------
        {
            static constexpr std::string_view kTwo[] = {
                "==", "!=", "<=", ">=", "&&", "||", "+=", "-=",
                "*=", "/=", "%=", "^=", "|=", "&="};
            std::string_view op;
            for (const std::string_view two : kTwo)
                if (src.compare(i, 2, two) == 0)
                {
                    op = src.substr(i, 2);
                    break;
                }
            if (op.empty()) op = src.substr(i, 1);
            const bool unary =
                !prevOperand && (op == "-" || op == "+" || op == "!");
            beginToken(false);
            if (unary || !opt.spaceAroundOperators)
            {
                append(op, /*operand=*/false);
            }
            else
            {
                rtrimLine();
                if (!line.empty()) line += ' ';
                line.append(op);
                line += ' ';
                prevOperand = false;
            }
            i += op.size();
            continue;
        }
    }
    pendingBreak = false;
    flushLine();
    if (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

}  // namespace lumi::scripting
