/**
 ****************************************************************************************
 * @file   HlslTranspiler.hpp
 * @brief  HLSL(ps_2/3-Teilmenge) → GLSL-330-Transpiler für MilkDrop-Preset-Shader
 *         (Import-Phase Stufe C1 — Entscheid E4, Session 40)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.3.0
 *
 * @details
 * Übersetzt NUR den Preset-Anteil eines warp_/comp_-Shaders (Deklarationen +
 * `shader_body { ... }`) — die MilkDrop-Umgebung (include.fx: sampler_*,
 * GetBlurN, q1–q32, bass/mid/treb, rand_*, …) stellt der Host als
 * handgeschriebene GLSL-Präambel mit #defines bereit. Vertragsgrenze wie bei
 * EelTranspiler: Text → Text, kein GL, keine Qt — voll unit-testbar.
 *
 * C1-Teilmenge: Ausdrücke (Arithmetik, Vergleiche, &&/||, ?:), Zuweisungen
 * (= += -= *= /=), lokale Deklarationen, if/else, Swizzles, Konstruktoren
 * (floatN → vecN), Intrinsics (lerp→mix, frac→fract, saturate→clamp, tex2D→
 * texture, mul→*, …) mit Typ-Inferenz und GLSL-Promotions (skalar→vecN bei
 * Zuweisung/pow/mix; float-Literale bekommen einen Dezimalpunkt; numerische
 * Bedingungen werden zu `!= 0.0`). Globale `sampler`-/`float4 texsize_*`-
 * Deklarationen werden zu Uniforms und als Feature gemeldet.
 *
 * C3 (Session 43, komplett): for/while/do-while + break/continue + ++/--,
 * Arrays (lokal/global, Index mit int()-Cast), tex3D auf die eingebauten
 * noisevol_lq/hq-Sampler (3D-Texturen + Uniforms liefert der Host),
 * include.fx-Konstanten (M_PI, M_PI_2 = 2π, M_INV_PI_2) + q-Bänke _qa–_qh,
 * #if/#ifdef/#else/#endif (konstante Bedingungen), Alias- und
 * Statement-#defines, Nicht-Quadrat-Matrizen (float2x3/float3x2 mit
 * transponierter Konstruktor-Emission), mul()-Formen inkl. Vektor·Vektor
 * (dot), out/inout-Parameter, Vektor-Vergleiche (lessThan-Familie),
 * HLSL-Implicit-Truncation in Zuweisungen UND Intrinsics, bool↔float,
 * q-Schattenkopien bei Schreibzugriff.
 *
 * Session 52: die 24 **Rotationsmatrizen** `rot_{s,d,f,vf,uf,rand}1..4` als
 * Builtins vom Typ `float4x3` (= GLSL `mat3x4`), Host liefert die Uniforms.
 * Dazu **Matrix-Indizierung** `M[i]`: das ist in HLSL die ZEILE, in GLSL die
 * SPALTE — emittiert wird `transpose(M)[i]`. Beides zusammen, weil alle neun
 * Presets des Packs, die `rot_*` benutzen, es über den Index tun und keines
 * über `mul()`.
 *
 * NICHT unterstützt (sauberer Fehler): #elif, Nicht-Literal-#if.
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <charconv>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace lumi::hlsl {

/// Which shader slot the text belongs to (drives the builtin symbol set)
enum class ShaderKind
{
    Warp,
    Comp
};

/**
 * @brief Transpile result: GLSL fragments for the host's program assembly
 */
struct HlslResult
{
    bool ok = false;
    std::string glslBody;     ///< translated shader_body statements (no braces)
    std::string glslGlobals;  ///< const-Deklarationen des Presets (vor main)
    std::vector<std::string> customSamplers;  ///< user sampler_XXX (Uniform macht der Host)
    std::vector<std::string> customTexsizes;  ///< user texsize_XXX (Uniform macht der Host)
    bool usesTex3d = false;   ///< referenced a 3D sampler / tex3D (C3)
    std::string error;        ///< first parse/typing error ("Zeile N: ...")
};

namespace detail {

// =============================================================================
// Types
// =============================================================================

enum class Base
{
    Unknown,
    Void,
    Bool,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat2,
    Mat3,
    Mat4,
    Mat2x3,  ///< HLSL float2x3 (2 Zeilen x 3 Spalten) = GLSL mat3x2
    Mat3x2,  ///< HLSL float3x2 (3 Zeilen x 2 Spalten) = GLSL mat2x3
    Mat4x3,  ///< HLSL float4x3 (4 Zeilen x 3 Spalten) = GLSL mat3x4 — die rot_*-Matrizen
    Sampler2D,
    Sampler3D
};

struct Type
{
    Base base = Base::Unknown;
    int arraySize = 0;  ///< >0 = Array dieses Elementtyps (C3-Rest)

    [[nodiscard]] bool isVec() const
    {
        return base == Base::Vec2 || base == Base::Vec3 || base == Base::Vec4;
    }
    [[nodiscard]] bool isMat() const
    {
        return base == Base::Mat2 || base == Base::Mat3 || base == Base::Mat4 ||
               base == Base::Mat2x3 || base == Base::Mat3x2 || base == Base::Mat4x3;
    }
    [[nodiscard]] int matRows() const
    {
        switch (base)
        {
        case Base::Mat2: return 2;
        case Base::Mat3: return 3;
        case Base::Mat4: return 4;
        case Base::Mat2x3: return 2;
        case Base::Mat3x2: return 3;
        case Base::Mat4x3: return 4;
        default: return 0;
        }
    }
    [[nodiscard]] int matCols() const
    {
        switch (base)
        {
        case Base::Mat2: return 2;
        case Base::Mat3: return 3;
        case Base::Mat4: return 4;
        case Base::Mat2x3: return 3;
        case Base::Mat3x2: return 2;
        case Base::Mat4x3: return 3;
        default: return 0;
        }
    }
    [[nodiscard]] bool isNumeric() const { return base == Base::Float || isVec() || isMat(); }
    [[nodiscard]] int size() const
    {
        switch (base)
        {
        case Base::Float: return 1;
        case Base::Vec2: return 2;
        case Base::Vec3: return 3;
        case Base::Vec4: return 4;
        default: return 0;
        }
    }
    [[nodiscard]] static Type vec(int n)
    {
        switch (n)
        {
        case 1: return {Base::Float};
        case 2: return {Base::Vec2};
        case 3: return {Base::Vec3};
        default: return {Base::Vec4};
        }
    }
    [[nodiscard]] const char* glsl() const
    {
        switch (base)
        {
        case Base::Void: return "void";
        case Base::Bool: return "bool";
        case Base::Float: return "float";
        case Base::Vec2: return "vec2";
        case Base::Vec3: return "vec3";
        case Base::Vec4: return "vec4";
        case Base::Mat2: return "mat2";
        case Base::Mat3: return "mat3";
        case Base::Mat4: return "mat4";
        // GLSL benennt matSPALTENxZEILEN — HLSL floatZEILENxSPALTEN
        case Base::Mat2x3: return "mat3x2";
        case Base::Mat3x2: return "mat2x3";
        case Base::Mat4x3: return "mat3x4";
        default: return "float";
        }
    }
};

/// HLSL type keyword → Type (int/half werden zu float — ps2-Semantik)
[[nodiscard]] inline bool typeFromKeyword(std::string_view s, Type& out)
{
    if (s == "float" || s == "int" || s == "half" || s == "bool" || s == "float1" ||
        s == "half1" || s == "int1")
        out = {s == "bool" ? Base::Bool : Base::Float};
    else if (s == "float2" || s == "half2" || s == "int2" || s == "bool2") out = {Base::Vec2};
    else if (s == "float3" || s == "half3" || s == "int3" || s == "bool3") out = {Base::Vec3};
    else if (s == "float4" || s == "half4" || s == "int4" || s == "bool4") out = {Base::Vec4};
    else if (s == "float2x2") out = {Base::Mat2};
    else if (s == "float3x3") out = {Base::Mat3};
    else if (s == "float4x4") out = {Base::Mat4};
    else if (s == "float2x3") out = {Base::Mat2x3};
    else if (s == "float3x2") out = {Base::Mat3x2};
    else if (s == "float4x3") out = {Base::Mat4x3};
    else if (s == "sampler" || s == "sampler2D") out = {Base::Sampler2D};
    else if (s == "sampler3D") out = {Base::Sampler3D};
    else if (s == "void") out = {Base::Void};
    else return false;
    return true;
}

/// Eingebauter Volumen-Noise-Sampler? (sampler_[fc_|pc_|fw_|pw_]noisevol_lq/hq)
[[nodiscard]] inline bool isNoiseVolSampler(std::string_view name)
{
    if (name.rfind("sampler_", 0) != 0) return false;
    std::string_view base = name.substr(8);
    for (std::string_view prefix : {"fc_", "pc_", "fw_", "pw_"})
    {
        if (base.rfind(prefix, 0) == 0)
        {
            base = base.substr(3);
            break;
        }
    }
    return base == "noisevol_lq" || base == "noisevol_hq";
}

[[nodiscard]] inline std::string toLower(std::string_view s)
{
    std::string out(s);
    for (char& c : out)
    {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return out;
}

// =============================================================================
// Fehler (intern per Exception, nach außen als HlslResult::error)
// =============================================================================

struct TranspileError
{
    std::string message;
};

[[noreturn]] inline void fail(int line, const std::string& what)
{
    throw TranspileError{"Zeile " + std::to_string(line) + ": " + what};
}

// =============================================================================
// Praeprozessor-Vorlauf: #define-Makros (Objekt- und Funktionsform)
// =============================================================================

struct Macro
{
    std::vector<std::string> params;
    std::string body;
    bool functionLike = false;
};

[[nodiscard]] inline bool isIdentChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
           c == '_';
}

/// Kommentare aus einem Makro-Body entfernen (sonst frisst // den Restausdruck)
[[nodiscard]] inline std::string stripLineComment(std::string_view s)
{
    const std::size_t pos = s.find("//");
    std::string out(pos == std::string_view::npos ? s : s.substr(0, pos));
    while (!out.empty() && (out.back() == ' ' || out.back() == '\t' || out.back() == '\r'))
        out.pop_back();
    return out;
}

/// Ein Expansions-Durchlauf; liefert true, wenn etwas ersetzt wurde
inline bool expandMacrosOnce(std::string& text,
                             const std::unordered_map<std::string, Macro>& macros)
{
    std::string out;
    out.reserve(text.size());
    bool changed = false;
    std::size_t i = 0;
    while (i < text.size())
    {
        const char c = text[i];
        if (!isIdentChar(c) || (c >= '0' && c <= '9'))
        {
            out += c;
            ++i;
            continue;
        }
        std::size_t start = i;
        while (i < text.size() && isIdentChar(text[i])) ++i;
        const std::string word = text.substr(start, i - start);
        const auto it = macros.find(word);
        if (it == macros.end())
        {
            out += word;
            continue;
        }
        const Macro& m = it->second;
        if (!m.functionLike)
        {
            // Klammern schuetzen nur AUSDRUCKS-Makros (Praezedenz). Nackte
            // Bezeichner (#define sat saturate → Aufruf folgt) und
            // STATEMENT-Makros (#define go ret1=...;) muessen unverpackt
            // bleiben, sonst entsteht unparsebarer Text (S43-Befunde)
            bool loneIdent = !m.body.empty();
            for (const char bc : m.body)
                if (!isIdentChar(bc)) { loneIdent = false; break; }
            const bool statementLike =
                m.body.find(';') != std::string::npos ||
                m.body.find('{') != std::string::npos ||
                m.body.find('}') != std::string::npos;
            if (loneIdent || statementLike) out += m.body;
            else out += "(" + m.body + ")";
            changed = true;
            continue;
        }
        // Funktionsform: Argumente bis zur passenden Klammer einsammeln
        std::size_t j = i;
        while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) ++j;
        if (j >= text.size() || text[j] != '(')
        {
            out += word;  // Name ohne Aufruf → unveraendert
            continue;
        }
        ++j;
        std::vector<std::string> args;
        std::string current;
        int depth = 1;
        for (; j < text.size() && depth > 0; ++j)
        {
            const char a = text[j];
            if (a == '(') ++depth;
            if (a == ')')
            {
                --depth;
                if (depth == 0) break;
            }
            if (a == ',' && depth == 1)
            {
                args.push_back(current);
                current.clear();
                continue;
            }
            current += a;
        }
        if (depth != 0) fail(1, "Makro '" + word + "': Klammern unbalanciert");
        args.push_back(current);
        if (args.size() != m.params.size())
            fail(1, "Makro '" + word + "': falsche Argumentzahl");
        // Parameter wortweise ersetzen
        std::string expanded;
        std::size_t k = 0;
        while (k < m.body.size())
        {
            if (!isIdentChar(m.body[k]) || (m.body[k] >= '0' && m.body[k] <= '9'))
            {
                expanded += m.body[k];
                ++k;
                continue;
            }
            std::size_t ws = k;
            while (k < m.body.size() && isIdentChar(m.body[k])) ++k;
            const std::string token = m.body.substr(ws, k - ws);
            bool replaced = false;
            for (std::size_t p = 0; p < m.params.size(); ++p)
            {
                if (token == m.params[p])
                {
                    expanded += "(" + args[p] + ")";
                    replaced = true;
                    break;
                }
            }
            if (!replaced) expanded += token;
        }
        out += "(" + expanded + ")";
        i = j + 1;
        changed = true;
    }
    text.swap(out);
    return changed;
}

/// #define-Zeilen extrahieren + Makros expandieren; andere Direktiven → Fehler
[[nodiscard]] inline std::string preprocess(std::string_view src)
{
    std::unordered_map<std::string, Macro> macros;
    std::string out;
    out.reserve(src.size());
    std::size_t pos = 0;
    int line = 1;
    // #if-Zustand (C3-Rest): nur KONSTANTE Bedingungen (#if 0/1, #ifdef) —
    // condStack = aktiver Zweig je Ebene, takenStack = Zweig schon genommen?
    std::vector<bool> condStack;
    std::vector<bool> takenStack;
    const auto active = [&] {
        for (const bool b : condStack)
            if (!b) return false;
        return true;
    };
    while (pos <= src.size())
    {
        const std::size_t nl = src.find('\n', pos);
        std::string_view raw =
            src.substr(pos, nl == std::string_view::npos ? src.size() - pos : nl - pos);
        std::string_view trimmed = raw;
        while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t'))
            trimmed.remove_prefix(1);
        if (!trimmed.empty() && trimmed.front() == '#')
        {
            trimmed.remove_prefix(1);
            while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t'))
                trimmed.remove_prefix(1);
            const auto directiveArg = [&](std::size_t prefixLen) {
                std::string a(stripLineComment(trimmed.substr(prefixLen)));
                while (!a.empty() && (a.front() == ' ' || a.front() == '\t'))
                    a.erase(a.begin());
                return a;
            };
            if (trimmed.rfind("ifdef", 0) == 0 || trimmed.rfind("ifndef", 0) == 0)
            {
                const bool neg = trimmed[2] == 'n';
                const std::string name = directiveArg(neg ? 6 : 5);
                const bool cond = (macros.count(name) != 0) != neg;
                condStack.push_back(cond);
                takenStack.push_back(cond);
                out += "\n";
            }
            else if (trimmed.rfind("if", 0) == 0)
            {
                std::string a = directiveArg(2);
                double v = 0.0;
                const char* b = a.c_str();
                char* endp = nullptr;
                v = std::strtod(b, &endp);
                if (endp == b) fail(line, "#if: nur konstante Bedingungen (0/1)");
                const bool cond = v != 0.0;
                condStack.push_back(cond);
                takenStack.push_back(cond);
                out += "\n";
            }
            else if (trimmed.rfind("else", 0) == 0)
            {
                if (condStack.empty()) fail(line, "#else ohne #if");
                condStack.back() = !takenStack.back();
                takenStack.back() = true;
                out += "\n";
            }
            else if (trimmed.rfind("endif", 0) == 0)
            {
                if (condStack.empty()) fail(line, "#endif ohne #if");
                condStack.pop_back();
                takenStack.pop_back();
                out += "\n";
            }
            else if (trimmed.rfind("define", 0) != 0)
            {
                fail(line, "Praeprozessor-Direktive wird nicht unterstuetzt");
            }
            else if (!active())
            {
                out += "\n";  // #define im inaktiven Zweig ignorieren
            }
            else
            {
            trimmed.remove_prefix(6);
            while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t'))
                trimmed.remove_prefix(1);
            std::size_t n = 0;
            while (n < trimmed.size() && isIdentChar(trimmed[n])) ++n;
            if (n == 0) fail(line, "#define ohne Namen");
            Macro m;
            const std::string name(trimmed.substr(0, n));
            trimmed.remove_prefix(n);
            if (!trimmed.empty() && trimmed.front() == '(')
            {
                m.functionLike = true;
                trimmed.remove_prefix(1);
                std::string param;
                while (!trimmed.empty() && trimmed.front() != ')')
                {
                    const char c = trimmed.front();
                    trimmed.remove_prefix(1);
                    if (c == ',')
                    {
                        m.params.push_back(param);
                        param.clear();
                    }
                    else if (c != ' ' && c != '\t')
                    {
                        param += c;
                    }
                }
                if (trimmed.empty()) fail(line, "#define: ')' fehlt");
                trimmed.remove_prefix(1);
                if (!param.empty()) m.params.push_back(param);
            }
            m.body = stripLineComment(trimmed);
            while (!m.body.empty() && (m.body.front() == ' ' || m.body.front() == '\t'))
                m.body.erase(m.body.begin());
            if (!m.body.empty() && m.body.back() == '\\')
                fail(line, "mehrzeilige #define werden nicht unterstuetzt");
            macros[name] = std::move(m);
            out += "\n";  // Zeilennummern stabil halten
            }
        }
        else
        {
            if (active()) out.append(raw);  // inaktive #if-Zweige fallen weg
            out += "\n";
        }
        if (nl == std::string_view::npos) break;
        pos = nl + 1;
        ++line;
    }
    if (!condStack.empty()) fail(line, "#endif fehlt");
    if (!macros.empty())
    {
        for (int pass = 0; pass < 8; ++pass)
        {
            if (!expandMacrosOnce(out, macros)) break;
        }
    }
    return out;
}

// =============================================================================
// Lexer
// =============================================================================

struct Token
{
    enum class Kind { Ident, Number, Punct, End };
    Kind kind = Kind::End;
    std::string text;
    int line = 0;
};

class Lexer
{
public:
    explicit Lexer(std::string_view src) : m_src(src)
    {
        m_current = lex();
        m_ahead = lex();
    }

    [[nodiscard]] const Token& peek() const { return m_current; }
    [[nodiscard]] const Token& peek2() const { return m_ahead; }
    // BEWUSST ohne [[nodiscard]]: next() dient auch als reines Weiterschalten
    // (viele Aufrufer verwerfen das Token — MSVC C4834-Rauschen sonst)
    Token next()
    {
        Token t = m_current;
        m_current = m_ahead;
        m_ahead = lex();
        return t;
    }

private:
    [[nodiscard]] Token lex()
    {
        skipWhitespaceAndComments();
        Token out;
        out.line = m_line;
        if (m_pos >= m_src.size()) return out;

        const char c = m_src[m_pos];
        if (isIdentStart(c))
        {
            std::size_t start = m_pos;
            while (m_pos < m_src.size() && isIdentTail(m_src[m_pos])) ++m_pos;
            out.kind = Token::Kind::Ident;
            out.text = std::string(m_src.substr(start, m_pos - start));
            return out;
        }
        if (isDigit(c) || (c == '.' && m_pos + 1 < m_src.size() && isDigit(m_src[m_pos + 1])))
        {
            std::size_t start = m_pos;
            while (m_pos < m_src.size() &&
                   (isDigit(m_src[m_pos]) || m_src[m_pos] == '.' || m_src[m_pos] == 'e' ||
                    m_src[m_pos] == 'E' ||
                    ((m_src[m_pos] == '+' || m_src[m_pos] == '-') && m_pos > start &&
                     (m_src[m_pos - 1] == 'e' || m_src[m_pos - 1] == 'E'))))
            {
                ++m_pos;
            }
            // f/h-Suffix schlucken (GLSL kennt ihn nicht)
            if (m_pos < m_src.size() &&
                (m_src[m_pos] == 'f' || m_src[m_pos] == 'F' || m_src[m_pos] == 'h' ||
                 m_src[m_pos] == 'H'))
            {
                ++m_pos;
                out.kind = Token::Kind::Number;
                out.text = std::string(m_src.substr(start, m_pos - start - 1));
                return out;
            }
            out.kind = Token::Kind::Number;
            out.text = std::string(m_src.substr(start, m_pos - start));
            return out;
        }
        // Mehrzeichen-Operatoren zuerst
        static constexpr std::array<std::string_view, 12> kMulti = {
            "+=", "-=", "*=", "/=", "==", "!=", "<=", ">=", "&&", "||", "++", "--"};
        for (std::string_view op : kMulti)
        {
            if (m_src.substr(m_pos, op.size()) == op)
            {
                out.kind = Token::Kind::Punct;
                out.text = std::string(op);
                m_pos += op.size();
                return out;
            }
        }
        out.kind = Token::Kind::Punct;
        out.text = std::string(1, c);
        ++m_pos;
        return out;
    }

    void skipWhitespaceAndComments()
    {
        while (m_pos < m_src.size())
        {
            const char c = m_src[m_pos];
            if (c == '\n')
            {
                ++m_line;
                ++m_pos;
            }
            else if (c == ' ' || c == '\t' || c == '\r')
            {
                ++m_pos;
            }
            else if (c == '/' && m_pos + 1 < m_src.size() && m_src[m_pos + 1] == '/')
            {
                while (m_pos < m_src.size() && m_src[m_pos] != '\n') ++m_pos;
            }
            else if (c == '/' && m_pos + 1 < m_src.size() && m_src[m_pos + 1] == '*')
            {
                m_pos += 2;
                while (m_pos + 1 < m_src.size() &&
                       !(m_src[m_pos] == '*' && m_src[m_pos + 1] == '/'))
                {
                    if (m_src[m_pos] == '\n') ++m_line;
                    ++m_pos;
                }
                m_pos = (m_pos + 2 <= m_src.size()) ? m_pos + 2 : m_src.size();
            }
            else
            {
                break;
            }
        }
    }

    [[nodiscard]] static bool isDigit(char c) { return c >= '0' && c <= '9'; }
    [[nodiscard]] static bool isIdentStart(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }
    [[nodiscard]] static bool isIdentTail(char c) { return isIdentStart(c) || isDigit(c); }

    std::string_view m_src;
    std::size_t m_pos = 0;
    int m_line = 1;
    Token m_current;
    Token m_ahead;
};

// =============================================================================
// AST
// =============================================================================

struct Expr
{
    enum class Kind
    {
        Number,   // text
        Ident,    // text
        Call,     // text = function name, args
        Ctor,     // text = glsl type name, args (floatN(...))
        Member,   // args[0].text  (swizzle/member)
        Index,    // args[0][args[1]]
        Unary,    // text = op, args[0]
        Binary,   // text = op, args[0] op args[1]
        Ternary,  // args[0] ? args[1] : args[2]
        Assign    // text = op (=, +=, ...), args[0] = args[1]
    };
    Kind kind = Kind::Number;
    std::string text;
    std::vector<Expr> args;
    int line = 0;
};

struct Stmt
{
    enum class Kind { Decl, ExprStmt, If, Block, Return, While, DoWhile, For, Break, Continue };
    Kind kind = Kind::ExprStmt;
    // Decl
    Type declType;
    std::vector<std::pair<std::string, Expr>> decls;  // name, init (init.kind==Number&&text empty → keiner)
    std::vector<bool> hasInit;
    std::vector<int> declArray;  ///< je Deklarator: Array-Groesse (0 = keins)
    // ExprStmt / If-/While-/For-Bedingung / Return-Wert
    Expr expr;
    bool hasExpr = false;  // Return: mit Wert? / For: Bedingung vorhanden?
    // If/While/For: thenBody = Rumpf; If: elseBody = else-Zweig,
    // For: elseBody = Init-Statement(s) (C3)
    std::vector<Stmt> thenBody;  // auch Block-Inhalt
    std::vector<Stmt> elseBody;
    bool hasElse = false;
    // For: Schritt-Ausdruck (i++, i+=2, …)
    Expr stepExpr;
    bool hasStep = false;
    int line = 0;
};

/// Hilfsfunktion vor shader_body (z. B. complex_mul) — wird GLSL-Funktion
struct FunctionDef
{
    Type returnType;
    std::string name;
    std::vector<std::pair<std::string, Type>> params;
    std::vector<std::uint8_t> paramQual;  ///< 0=in, 1=out, 2=inout (C3-Rest)
    std::vector<Stmt> body;
    int line = 0;
};

// =============================================================================
// Parser (Praezedenz-Kaskade)
// =============================================================================

class Parser
{
public:
    explicit Parser(std::string_view src) : m_lex(src) {}

    /// Preset-Text: globale Deklarationen + Funktionen + shader_body-Block
    void parseProgram(std::vector<Stmt>& globals, std::vector<FunctionDef>& functions,
                      std::vector<Stmt>& body)
    {
        while (m_lex.peek().kind != Token::Kind::End)
        {
            const Token& t = m_lex.peek();
            if (t.kind == Token::Kind::Punct && t.text == ";")
            {
                m_lex.next();
                continue;
            }
            if (t.kind != Token::Kind::Ident)
                fail(t.line, "Deklaration oder shader_body erwartet ('" + t.text + "')");
            if (t.text == "shader_body")
            {
                m_lex.next();
                expectPunct("{");
                parseStmtList(body, "}");
                return;  // Rest (Kommentare etc.) ist bereits vom Lexer geschluckt
            }
            // const/static ueberspringen
            while (m_lex.peek().kind == Token::Kind::Ident &&
                   (m_lex.peek().text == "const" || m_lex.peek().text == "static"))
            {
                m_lex.next();
            }
            const Token typeTok = m_lex.next();
            Type declType;
            if (typeTok.kind != Token::Kind::Ident ||
                !typeFromKeyword(typeTok.text, declType))
            {
                fail(typeTok.line, "unbekannter Typ '" + typeTok.text + "'");
            }
            const Token nameTok = m_lex.next();
            if (nameTok.kind != Token::Kind::Ident)
                fail(nameTok.line, "Bezeichner erwartet");
            if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "(")
            {
                functions.push_back(parseFunctionRest(declType, nameTok));
            }
            else
            {
                globals.push_back(parseDeclTail(declType, nameTok));
            }
        }
        fail(m_lex.peek().line, "shader_body nicht gefunden");
    }

private:
    void parseStmtList(std::vector<Stmt>& out, const char* endPunct)
    {
        while (!(m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == endPunct))
        {
            if (m_lex.peek().kind == Token::Kind::End)
                fail(m_lex.peek().line, "unerwartetes Ende (fehlt '}')");
            out.push_back(parseStmt());
        }
        m_lex.next();  // endPunct
    }

    Stmt parseStmt()
    {
        const Token& t = m_lex.peek();
        Stmt s;
        s.line = t.line;

        if (t.kind == Token::Kind::Punct && t.text == ";")
        {
            m_lex.next();
            s.kind = Stmt::Kind::Block;  // leer
            return s;
        }
        if (t.kind == Token::Kind::Punct && t.text == "{")
        {
            m_lex.next();
            s.kind = Stmt::Kind::Block;
            parseStmtList(s.thenBody, "}");
            return s;
        }
        if (t.kind == Token::Kind::Ident)
        {
            if (t.text == "if") return parseIf();
            if (t.text == "for" || t.text == "while")
                return parseLoop(t.text == "for");
            if (t.text == "do")
            {
                // C3-Rest: do { ... } while (cond);
                s.kind = Stmt::Kind::DoWhile;
                m_lex.next();
                parseBranch(s.thenBody);
                if (!(m_lex.peek().kind == Token::Kind::Ident &&
                      m_lex.peek().text == "while"))
                    fail(s.line, "do: 'while' erwartet");
                m_lex.next();
                expectPunct("(");
                s.expr = parseExpr();
                s.hasExpr = true;
                expectPunct(")");
                expectPunct(";");
                return s;
            }
            if (t.text == "break" || t.text == "continue")
            {
                s.kind = t.text == "break" ? Stmt::Kind::Break : Stmt::Kind::Continue;
                m_lex.next();
                expectPunct(";");
                return s;
            }
            if (t.text == "return")
            {
                m_lex.next();
                s.kind = Stmt::Kind::Return;
                if (!(m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ";"))
                {
                    s.expr = parseExpr();
                    s.hasExpr = true;
                }
                expectPunct(";");
                return s;
            }
            Type declType;
            if (t.text == "const" || t.text == "static" ||
                typeFromKeyword(t.text, declType))
            {
                return parseDecl();
            }
        }
        s.kind = Stmt::Kind::ExprStmt;
        s.expr = parseExpr();
        expectPunct(";");
        return s;
    }

    FunctionDef parseFunctionRest(Type returnType, const Token& nameTok)
    {
        FunctionDef fn;
        fn.returnType = returnType;
        fn.name = nameTok.text;
        fn.line = nameTok.line;
        expectPunct("(");
        if (!(m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ")"))
        {
            for (;;)
            {
                // optionale in/out/inout-Qualifier — GLSL kennt sie nativ
                std::uint8_t qual = 0;
                if (m_lex.peek().kind == Token::Kind::Ident && m_lex.peek().text == "in")
                    m_lex.next();
                if (m_lex.peek().kind == Token::Kind::Ident &&
                    (m_lex.peek().text == "out" || m_lex.peek().text == "inout"))
                {
                    qual = m_lex.peek().text == "out" ? 1 : 2;
                    m_lex.next();
                }
                const Token pType = m_lex.next();
                Type paramType;
                if (pType.kind != Token::Kind::Ident ||
                    !typeFromKeyword(pType.text, paramType))
                {
                    fail(pType.line, "Parametertyp erwartet");
                }
                const Token pName = m_lex.next();
                if (pName.kind != Token::Kind::Ident)
                    fail(pName.line, "Parametername erwartet");
                skipSemantic();  // ": TEXCOORD0" etc.
                fn.params.emplace_back(pName.text, paramType);
                fn.paramQual.push_back(qual);
                if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ",")
                {
                    m_lex.next();
                    continue;
                }
                break;
            }
        }
        expectPunct(")");
        skipSemantic();  // ": COLOR0" am Funktionskopf
        expectPunct("{");
        parseStmtList(fn.body, "}");
        return fn;
    }

    /// HLSL-Semantik-Anhaengsel (": IDENT" oder ": register(c0)") ueberspringen
    void skipSemantic()
    {
        if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ":")
        {
            m_lex.next();
            const Token sem = m_lex.next();
            if (sem.kind != Token::Kind::Ident)
                fail(sem.line, "Semantik-Bezeichner erwartet");
            if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "(")
            {
                int depth = 0;
                do {
                    const Token t = m_lex.next();
                    if (t.kind == Token::Kind::End)
                        fail(sem.line, "')' fehlt in Semantik");
                    if (t.kind == Token::Kind::Punct && t.text == "(") ++depth;
                    if (t.kind == Token::Kind::Punct && t.text == ")") --depth;
                } while (depth > 0);
            }
        }
    }

    Stmt parseDeclTail(Type declType, const Token& firstName)
    {
        Stmt s;
        s.line = firstName.line;
        s.kind = Stmt::Kind::Decl;
        s.declType = declType;
        Token nameTok = firstName;
        for (;;)
        {
            if (nameTok.kind != Token::Kind::Ident)
                fail(nameTok.line, "Bezeichner erwartet");
            int arraySize = 0;  // C3-Rest: `float w[4]`
            if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "[")
            {
                m_lex.next();
                const Token n = m_lex.next();
                if (n.kind != Token::Kind::Number)
                    fail(nameTok.line, "Array-Groesse: Zahl erwartet");
                arraySize = std::atoi(n.text.c_str());
                if (arraySize <= 0) fail(nameTok.line, "Array-Groesse > 0 erwartet");
                expectPunct("]");
            }
            skipSemantic();  // ": register(cN)" an globalen Deklarationen
            Expr init;
            bool has = false;
            if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "=")
            {
                m_lex.next();
                if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "{")
                {
                    // Initialisierer-Liste { a, b, ... } → Konstruktor des Zieltyps
                    m_lex.next();
                    init.kind = Expr::Kind::Ctor;
                    init.text = declType.glsl();
                    init.line = nameTok.line;
                    for (;;)
                    {
                        init.args.push_back(parseAssign());
                        if (m_lex.peek().kind == Token::Kind::Punct &&
                            m_lex.peek().text == ",")
                        {
                            m_lex.next();
                            continue;
                        }
                        break;
                    }
                    expectPunct("}");
                }
                else
                {
                    init = parseAssign();
                }
                has = true;
            }
            s.decls.emplace_back(nameTok.text, std::move(init));
            s.hasInit.push_back(has);
            s.declArray.push_back(arraySize);
            if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ",")
            {
                m_lex.next();
                nameTok = m_lex.next();
                continue;
            }
            break;
        }
        expectPunct(";");
        return s;
    }

    Stmt parseIf()
    {
        Stmt s;
        s.line = m_lex.peek().line;
        s.kind = Stmt::Kind::If;
        m_lex.next();  // if
        expectPunct("(");
        s.expr = parseExpr();
        expectPunct(")");
        parseBranch(s.thenBody);
        if (m_lex.peek().kind == Token::Kind::Ident && m_lex.peek().text == "else")
        {
            m_lex.next();
            s.hasElse = true;
            parseBranch(s.elseBody);
        }
        return s;
    }

    /// C3: for(init; cond; step) / while(cond) — Rumpf via parseBranch
    Stmt parseLoop(bool isFor)
    {
        Stmt s;
        s.line = m_lex.peek().line;
        m_lex.next();  // for / while
        expectPunct("(");
        if (isFor)
        {
            s.kind = Stmt::Kind::For;
            // Init (Decl/Ausdruck/leer) — parseStmt konsumiert das ';' selbst;
            // landet im Init-Slot (elseBody, s. Stmt-Doku)
            if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ";")
                m_lex.next();
            else
                s.elseBody.push_back(parseStmt());
            if (!(m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ";"))
            {
                s.expr = parseExpr();
                s.hasExpr = true;
            }
            expectPunct(";");
            if (!(m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ")"))
            {
                s.stepExpr = parseExpr();
                s.hasStep = true;
            }
        }
        else
        {
            s.kind = Stmt::Kind::While;
            s.expr = parseExpr();
            s.hasExpr = true;
        }
        expectPunct(")");
        parseBranch(s.thenBody);
        return s;
    }

    void parseBranch(std::vector<Stmt>& out)
    {
        if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "{")
        {
            m_lex.next();
            parseStmtList(out, "}");
        }
        else
        {
            out.push_back(parseStmt());
        }
    }

    Stmt parseDecl()
    {
        // const/static ueberspringen
        while (m_lex.peek().kind == Token::Kind::Ident &&
               (m_lex.peek().text == "const" || m_lex.peek().text == "static"))
        {
            m_lex.next();
        }
        const Token typeTok = m_lex.next();
        Type declType;
        if (!typeFromKeyword(typeTok.text, declType))
            fail(typeTok.line, "unbekannter Typ '" + typeTok.text + "'");
        return parseDeclTail(declType, m_lex.next());
    }

    Expr parseExpr()
    {
        Expr e = parseAssign();
        // Komma-Operator (Sequenz) — existiert in GLSL genauso
        while (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ",")
        {
            Expr seq;
            seq.kind = Expr::Kind::Binary;
            seq.text = ",";
            seq.line = m_lex.next().line;
            seq.args.push_back(std::move(e));
            seq.args.push_back(parseAssign());
            e = std::move(seq);
        }
        return e;
    }

    Expr parseAssign()
    {
        Expr lhs = parseTernary();
        const Token& t = m_lex.peek();
        if (t.kind == Token::Kind::Punct &&
            (t.text == "=" || t.text == "+=" || t.text == "-=" || t.text == "*=" ||
             t.text == "/="))
        {
            Expr e;
            e.kind = Expr::Kind::Assign;
            e.text = t.text;
            e.line = t.line;
            m_lex.next();
            e.args.push_back(std::move(lhs));
            e.args.push_back(parseAssign());
            return e;
        }
        return lhs;
    }

    Expr parseTernary()
    {
        Expr cond = parseBinary(0);
        if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "?")
        {
            Expr e;
            e.kind = Expr::Kind::Ternary;
            e.line = m_lex.next().line;
            e.args.push_back(std::move(cond));
            e.args.push_back(parseAssign());
            expectPunct(":");
            e.args.push_back(parseAssign());
            return e;
        }
        return cond;
    }

    [[nodiscard]] static int precedence(const std::string& op)
    {
        if (op == "||") return 1;
        if (op == "&&") return 2;
        if (op == "==" || op == "!=") return 3;
        if (op == "<" || op == ">" || op == "<=" || op == ">=") return 4;
        if (op == "+" || op == "-") return 5;
        if (op == "*" || op == "/" || op == "%") return 6;
        return -1;
    }

    Expr parseBinary(int minPrec)
    {
        Expr lhs = parseUnary();
        for (;;)
        {
            const Token& t = m_lex.peek();
            if (t.kind != Token::Kind::Punct) return lhs;
            const int prec = precedence(t.text);
            if (prec < 0 || prec < minPrec) return lhs;
            Expr e;
            e.kind = Expr::Kind::Binary;
            e.text = t.text;
            e.line = t.line;
            m_lex.next();
            e.args.push_back(std::move(lhs));
            e.args.push_back(parseBinary(prec + 1));
            lhs = std::move(e);
        }
    }

    Expr parseUnary()
    {
        const Token& t = m_lex.peek();
        // C3: Praefix-Inkrement/-Dekrement (GLSL kann beides auf float)
        if (t.kind == Token::Kind::Punct && (t.text == "++" || t.text == "--"))
        {
            Expr e;
            e.kind = Expr::Kind::Unary;
            e.text = t.text;
            e.line = m_lex.next().line;
            e.args.push_back(parseUnary());
            return e;
        }
        if (t.kind == Token::Kind::Punct && (t.text == "-" || t.text == "+" || t.text == "!"))
        {
            Expr e;
            e.kind = Expr::Kind::Unary;
            e.text = t.text;
            e.line = m_lex.next().line;
            e.args.push_back(parseUnary());
            return e;
        }
        return parsePostfix();
    }

    Expr parsePostfix()
    {
        Expr e = parsePrimary();
        for (;;)
        {
            const Token& t = m_lex.peek();
            if (t.kind == Token::Kind::Punct && t.text == ".")
            {
                m_lex.next();
                const Token member = m_lex.next();
                if (member.kind != Token::Kind::Ident)
                    fail(member.line, "Swizzle/Member erwartet");
                Expr m;
                m.kind = Expr::Kind::Member;
                m.text = member.text;
                m.line = member.line;
                m.args.push_back(std::move(e));
                e = std::move(m);
            }
            else if (t.kind == Token::Kind::Punct && t.text == "[")
            {
                // C3-Rest: Array-/Vektor-Index
                const int ln = t.line;
                m_lex.next();
                Expr idx = parseExpr();
                expectPunct("]");
                Expr ix;
                ix.kind = Expr::Kind::Index;
                ix.line = ln;
                ix.args.push_back(std::move(e));
                ix.args.push_back(std::move(idx));
                e = std::move(ix);
            }
            else if (t.kind == Token::Kind::Punct &&
                     (t.text == "++" || t.text == "--"))
            {
                // C3: Postfix-Inkrement/-Dekrement (n++ in Schleifen)
                Expr u;
                u.kind = Expr::Kind::Unary;
                u.text = "post" + t.text;
                u.line = m_lex.next().line;
                u.args.push_back(std::move(e));
                e = std::move(u);
            }
            else
            {
                return e;
            }
        }
    }

    Expr parsePrimary()
    {
        const Token t = m_lex.next();
        if (t.kind == Token::Kind::Number)
        {
            Expr e;
            e.kind = Expr::Kind::Number;
            e.text = t.text;
            e.line = t.line;
            return e;
        }
        if (t.kind == Token::Kind::Punct && t.text == "(")
        {
            // C-Style-Cast "(float3)expr" — nur wenn direkt ')' folgt (sonst ist
            // es ein Konstruktor-Ausdruck in Klammern, z. B. "(float3(...))")
            if (m_lex.peek().kind == Token::Kind::Ident &&
                m_lex.peek2().kind == Token::Kind::Punct && m_lex.peek2().text == ")")
            {
                Type castType;
                if (typeFromKeyword(m_lex.peek().text, castType) && castType.isNumeric())
                {
                    const Token typeTok = m_lex.next();
                    expectPunct(")");
                    Expr e;
                    e.kind = Expr::Kind::Ctor;
                    e.text = castType.glsl();
                    e.line = typeTok.line;
                    e.args.push_back(parseUnary());
                    return e;
                }
            }
            Expr e = parseExpr();
            expectPunct(")");
            return e;
        }
        if (t.kind == Token::Kind::Ident)
        {
            Type ctorType;
            const bool isCtor = typeFromKeyword(t.text, ctorType) && ctorType.isNumeric();
            if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "(")
            {
                m_lex.next();
                Expr e;
                e.kind = isCtor ? Expr::Kind::Ctor : Expr::Kind::Call;
                e.text = isCtor ? std::string(ctorType.glsl()) : t.text;
                e.line = t.line;
                if (!(m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == ")"))
                {
                    for (;;)
                    {
                        e.args.push_back(parseAssign());
                        if (m_lex.peek().kind == Token::Kind::Punct &&
                            m_lex.peek().text == ",")
                        {
                            m_lex.next();
                            continue;
                        }
                        break;
                    }
                }
                expectPunct(")");
                return e;
            }
            Expr e;
            e.kind = Expr::Kind::Ident;
            e.text = t.text;
            e.line = t.line;
            return e;
        }
        fail(t.line, "unerwartetes Token '" + t.text + "'");
    }

    void expectPunct(const char* p)
    {
        const Token t = m_lex.next();
        if (t.kind != Token::Kind::Punct || t.text != p)
            fail(t.line, std::string("'") + p + "' erwartet, gefunden '" + t.text + "'");
    }

    Lexer m_lex;
};

// =============================================================================
// CodeGen mit Typ-Inferenz
// =============================================================================

struct EmitResult
{
    std::string code;
    Type type;
};

class CodeGen
{
public:
    explicit CodeGen(ShaderKind kind, HlslResult& out) : m_out(out)
    {
        pushScope();
        seedBuiltins(kind);
    }

    void emitGlobals(const std::vector<Stmt>& globals)
    {
        for (const Stmt& s : globals)
        {
            if (s.kind != Stmt::Kind::Decl)
                fail(s.line, "nur Deklarationen vor shader_body erlaubt");
            if (s.declType.base == Base::Sampler2D || s.declType.base == Base::Sampler3D)
            {
                for (const auto& [name, init] : s.decls)
                {
                    // Redeklaration der eingebauten Volumen-Noise-Sampler
                    // (haeufig als `sampler sampler_noisevol_hq;`): Builtin-Typ
                    // sampler3D behalten, KEIN Custom-Sampler (C3, S43)
                    if (isNoiseVolSampler(name))
                    {
                        m_out.usesTex3d = true;
                        continue;
                    }
                    declare(name, s.declType);
                    if (s.declType.base == Base::Sampler3D)
                    {
                        m_out.usesTex3d = true;
                        fail(s.line,
                             "sampler3D: nur die eingebauten noisevol-Sampler");
                    }
                    m_out.customSamplers.push_back(name);
                }
                continue;
            }
            // float4 texsize_XXX → Host-Uniform. Numerische Globals werden GLSL-
            // Globals mit Null-Init; ihre Initialisierer laufen am main-Anfang
            // (GLSL-Global-Initialisierer muessen konstant sein — q1 & Co. nicht).
            for (std::size_t i = 0; i < s.decls.size(); ++i)
            {
                const auto& [name, init] = s.decls[i];
                registerRename(name);
                m_userDeclared[name] = true;
                const int arr = i < s.declArray.size() ? s.declArray[i] : 0;
                if (arr > 0)  // C3-Rest: globales Array
                {
                    Type at = s.declType;
                    at.arraySize = arr;
                    declare(name, at);
                    std::string g = std::string(s.declType.glsl()) + " " + outName(name) +
                                    "[" + std::to_string(arr) + "]";
                    if (s.hasInit[i])
                    {
                        // {…}-Liste → GLSL-Array-Konstruktor (Literal-Init)
                        if (init.kind != Expr::Kind::Ctor)
                            fail(s.line, "Array-Init: {…}-Liste erwartet");
                        g += " = " + std::string(s.declType.glsl()) + "[" +
                             std::to_string(arr) + "](";
                        Expr initCopy = init;
                        for (std::size_t k = 0; k < initCopy.args.size(); ++k)
                        {
                            EmitResult r = emitExpr(initCopy.args[k]);
                            r = convertTo(r, s.declType, s.line);
                            if (k > 0) g += ", ";
                            g += r.code;
                        }
                        g += ")";
                    }
                    m_out.glslGlobals += g + ";\n";
                    continue;
                }
                declare(name, s.declType);
                if (name.rfind("texsize_", 0) == 0)
                {
                    m_out.customTexsizes.push_back(name);
                    continue;  // Uniform kommt vom Host
                }
                m_out.glslGlobals += std::string(s.declType.glsl()) + " " + outName(name) +
                                     " = " + s.declType.glsl() + "(0.0);\n";
                if (s.hasInit[i])
                {
                    Expr initCopy = init;
                    EmitResult r = emitExpr(initCopy);
                    r = convertTo(r, s.declType, s.line);
                    m_globalInit += "    " + outName(name) + " = " + r.code + ";\n";
                }
            }
        }
    }

    void emitFunctions(const std::vector<FunctionDef>& functions)
    {
        for (const FunctionDef& fn : functions)
        {
            if (m_functions.count(fn.name) != 0)
                fail(fn.line, "Funktion '" + fn.name + "' doppelt definiert");
            FunctionSig sig;
            sig.returnType = fn.returnType;
            for (const auto& [pname, ptype] : fn.params) sig.params.push_back(ptype);
            sig.paramQual = fn.paramQual;
            m_functions[fn.name] = sig;
            // GLSL-Builtin-Kollisionen (float2 noise3(...), …) → Alias
            registerRename(fn.name);
            m_userDeclared[fn.name] = true;

            std::string head =
                std::string(fn.returnType.glsl()) + " " + outName(fn.name) + "(";
            for (std::size_t i = 0; i < fn.params.size(); ++i)
            {
                if (i > 0) head += ", ";
                if (fn.paramQual[i] == 1) head += "out ";
                else if (fn.paramQual[i] == 2) head += "inout ";
                registerRename(fn.params[i].first);
                head += std::string(fn.params[i].second.glsl()) + " " +
                        outName(fn.params[i].first);
            }
            head += ")";

            // Body in eigenem Scope mit Parametern; Ausgabe in glslGlobals
            pushScope();
            for (const auto& [pname, ptype] : fn.params) declare(pname, ptype);
            m_returnType = fn.returnType;
            m_inFunction = true;
            std::string saved;
            saved.swap(m_out.glslBody);
            for (const Stmt& s : fn.body) emitStmt(s, 1);
            std::string fnBody;
            fnBody.swap(m_out.glslBody);
            m_out.glslBody.swap(saved);
            m_inFunction = false;
            popScope();

            m_out.glslGlobals += head + "\n{\n" + fnBody + "}\n";
        }
    }

    void emitBody(const std::vector<Stmt>& body)
    {
        m_out.glslBody += m_globalInit;
        for (const Stmt& s : body) emitStmt(s, 1);
    }

    /// Schattenkopien fuer beschriebene Uniforms — VOR den Body stellen
    [[nodiscard]] std::string shadowProlog() const
    {
        std::vector<std::string> names;
        names.reserve(m_shadow.size());
        for (const auto& [name, used] : m_shadow)
            if (used) names.push_back(name);
        std::sort(names.begin(), names.end());  // deterministische Emission
        std::string s;
        for (const std::string& name : names)
        {
            Type t;
            if (!lookup(name, t)) continue;
            s += "    " + std::string(t.glsl()) + " " + name + " = " + name +
                 ";  // Preset schreibt Uniform\n";
        }
        return s;
    }

private:
    // --- Symboltabelle ------------------------------------------------------------------
    void pushScope() { m_scopes.emplace_back(); }
    void popScope() { m_scopes.pop_back(); }
    void declare(const std::string& name, Type t) { m_scopes.back()[name] = t; }

    /// GLSL-Builtin-/Schluesselwoerter, die HLSL-Presets als Bezeichner nutzen
    /// (float3 mod; float2 noise3(...); …) — werden bei Deklaration umbenannt
    [[nodiscard]] static bool isGlslReservedName(const std::string& n)
    {
        static const std::array<const char*, 12> kReserved = {
            "mod", "mix", "step", "texture", "noise1", "noise2", "noise3",
            "noise4", "input", "output", "sample", "main"};
        for (const char* r : kReserved)
            if (n == r) return true;
        return false;
    }

    /// Deklarations-Hook: reservierte Namen bekommen einen GLSL-Aliasnamen
    void registerRename(const std::string& name)
    {
        if (isGlslReservedName(name)) m_rename[name] = name + "_hl";
    }

    /// Emissions-Name eines Bezeichners (Alias, falls umbenannt)
    [[nodiscard]] std::string outName(const std::string& name) const
    {
        const auto it = m_rename.find(name);
        return it == m_rename.end() ? name : it->second;
    }
    [[nodiscard]] bool lookup(const std::string& name, Type& out) const
    {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it)
        {
            const auto found = it->find(name);
            if (found != it->end())
            {
                out = found->second;
                return true;
            }
        }
        return false;
    }

    void seedBuiltins(ShaderKind kind)
    {
        const auto f = [&](const char* n) { declare(n, {Base::Float}); };
        const auto v2 = [&](const char* n) { declare(n, {Base::Vec2}); };
        const auto v3 = [&](const char* n) { declare(n, {Base::Vec3}); };
        const auto v4 = [&](const char* n) { declare(n, {Base::Vec4}); };
        const auto smp = [&](const char* n) { declare(n, {Base::Sampler2D}); };
        const auto smp3 = [&](const char* n) { declare(n, {Base::Sampler3D}); };

        v3("ret");
        v2("uv");
        v2("uv_orig");
        f("rad");
        f("ang");
        f("time");
        f("fps");
        f("frame");
        f("progress");
        f("bass");
        f("mid");
        f("treb");
        f("vol");
        f("bass_att");
        f("mid_att");
        f("treb_att");
        f("vol_att");
        v4("aspect");
        v4("texsize");
        v4("rand_preset");
        v4("rand_frame");
        v4("roam_cos");
        v4("roam_sin");
        v4("slow_roam_cos");
        v4("slow_roam_sin");
        // 24 Rotationsmatrizen (plugin.cpp:3241-3264). HLSL float4x3, benutzt
        // als mul(float4, rot_xx) -> float3. Reihenfolge = Drehgeschwindigkeit:
        // s(tatic) < d(rift) < f(ast) < vf < uf, dazu rand (jeden Frame neu).
        for (const char* group : {"s", "d", "f", "vf", "uf", "rand"})
        {
            for (int i = 1; i <= 4; ++i)
            {
                declare((std::string("rot_") + group + std::to_string(i)).c_str(),
                        {Base::Mat4x3});
            }
        }
        for (int i = 1; i <= 32; ++i) f(("q" + std::to_string(i)).c_str());
        f("blur1_min");
        f("blur1_max");
        f("blur2_min");
        f("blur2_max");
        f("blur3_min");
        f("blur3_max");
        if (kind == ShaderKind::Comp) v3("hue_shader");

        smp("sampler_main");
        smp("sampler_fc_main");
        smp("sampler_pc_main");
        smp("sampler_fw_main");
        smp("sampler_pw_main");
        smp("sampler_blur1");
        smp("sampler_blur2");
        smp("sampler_blur3");
        for (const char* base : {"noise_lq", "noise_lq_lite", "noise_mq", "noise_hq"})
        {
            smp((std::string("sampler_") + base).c_str());
            for (const char* prefix : {"fc_", "pc_", "fw_", "pw_"})
                smp((std::string("sampler_") + prefix + base).c_str());
            v4((std::string("texsize_") + base).c_str());
        }
        for (const char* base : {"noisevol_lq", "noisevol_hq"})
        {
            smp3((std::string("sampler_") + base).c_str());
            for (const char* prefix : {"fc_", "pc_", "fw_", "pw_"})
                smp3((std::string("sampler_") + prefix + base).c_str());
        }
        v4("texsize_noisevol_lq");
        v4("texsize_noisevol_hq");
        // include.fx-Konstanten (Werte liefert die GLSL-Praeambel des Hosts;
        // Referenz: M_PI_2 ist dort 2*pi, NICHT pi/2!)
        f("M_PI");
        f("M_PI_2");
        f("M_INV_PI_2");
        // rohe q-Baenke (_qa.._qh = q1-4 .. q29-32)
        for (char bank = 'a'; bank <= 'h'; ++bank)
            v4((std::string("_q") + bank).c_str());

        // Schreibbare HLSL-Globals = GLSL-Uniforms → Schattenkopie-Kandidaten
        // (HLSL erlaubt Zuweisungen an globale Konstanten, GLSL nicht)
        for (const char* n :
             {"time", "fps", "frame", "progress", "bass", "mid", "treb", "vol",
              "bass_att", "mid_att", "treb_att", "vol_att", "aspect", "texsize",
              "rand_preset", "rand_frame", "roam_cos", "roam_sin",
              "slow_roam_cos", "slow_roam_sin", "blur1_min", "blur1_max",
              "blur2_min", "blur2_max", "blur3_min", "blur3_max"})
            m_uniformNames[n] = true;
        for (int i = 1; i <= 32; ++i) m_uniformNames["q" + std::to_string(i)] = true;
        for (char bank = 'a'; bank <= 'h'; ++bank)
            m_uniformNames[std::string("_q") + bank] = true;
        for (const char* base : {"noise_lq", "noise_lq_lite", "noise_mq",
                                 "noise_hq", "noisevol_lq", "noisevol_hq"})
            m_uniformNames[std::string("texsize_") + base] = true;
        for (const char* group : {"s", "d", "f", "vf", "uf", "rand"})
            for (int i = 1; i <= 4; ++i)
                m_uniformNames[std::string("rot_") + group + std::to_string(i)] = true;
    }

    // --- Statements ---------------------------------------------------------------------
    void emitStmt(const Stmt& s, int indent)
    {
        const std::string pad(static_cast<std::size_t>(indent) * 4, ' ');
        switch (s.kind)
        {
        case Stmt::Kind::Block:
            if (!s.thenBody.empty())
            {
                m_out.glslBody += pad + "{\n";
                pushScope();
                for (const Stmt& c : s.thenBody) emitStmt(c, indent + 1);
                popScope();
                m_out.glslBody += pad + "}\n";
            }
            break;
        case Stmt::Kind::Decl:
        {
            for (std::size_t i = 0; i < s.decls.size(); ++i)
            {
                const auto& [name, init] = s.decls[i];
                registerRename(name);
                m_userDeclared[name] = true;
                const int arr = i < s.declArray.size() ? s.declArray[i] : 0;
                if (arr > 0)  // C3-Rest: lokales Array
                {
                    Type at = s.declType;
                    at.arraySize = arr;
                    declare(name, at);
                    std::string lineOut = pad + std::string(s.declType.glsl()) + " " +
                                          outName(name) + "[" + std::to_string(arr) + "]";
                    if (s.hasInit[i])
                    {
                        if (init.kind != Expr::Kind::Ctor)
                            fail(s.line, "Array-Init: {…}-Liste erwartet");
                        std::string ctor = std::string(s.declType.glsl()) + "[" +
                                           std::to_string(arr) + "](";
                        Expr initCopy = init;
                        for (std::size_t k = 0; k < initCopy.args.size(); ++k)
                        {
                            EmitResult r = emitExpr(initCopy.args[k]);
                            r = convertTo(r, s.declType, s.line);
                            if (k > 0) ctor += ", ";
                            ctor += r.code;
                        }
                        lineOut += " = " + ctor + ")";
                    }
                    m_out.glslBody += lineOut + ";\n";
                    continue;
                }
                declare(name, s.declType);
                std::string lineOut =
                    pad + std::string(s.declType.glsl()) + " " + outName(name);
                if (s.hasInit[i])
                {
                    Expr initCopy = init;
                    EmitResult r = emitExpr(initCopy);
                    r = convertTo(r, s.declType, s.line);
                    lineOut += " = " + r.code;
                }
                else
                {
                    lineOut += std::string(" = ") + s.declType.glsl() + "(0.0)";
                }
                m_out.glslBody += lineOut + ";\n";
            }
            break;
        }
        case Stmt::Kind::ExprStmt:
        {
            Expr e = s.expr;
            const EmitResult r = emitExpr(e);
            m_out.glslBody += pad + r.code + ";\n";
            break;
        }
        case Stmt::Kind::If:
        {
            Expr cond = s.expr;
            EmitResult c = emitExpr(cond);
            m_out.glslBody += pad + "if (" + boolify(c, s.line) + ")\n" + pad + "{\n";
            pushScope();
            for (const Stmt& t : s.thenBody) emitStmt(t, indent + 1);
            popScope();
            m_out.glslBody += pad + "}\n";
            if (s.hasElse)
            {
                m_out.glslBody += pad + "else\n" + pad + "{\n";
                pushScope();
                for (const Stmt& t : s.elseBody) emitStmt(t, indent + 1);
                popScope();
                m_out.glslBody += pad + "}\n";
            }
            break;
        }
        case Stmt::Kind::While:
        {
            Expr cond = s.expr;
            EmitResult c = emitExpr(cond);
            m_out.glslBody += pad + "while (" + boolify(c, s.line) + ")\n" + pad + "{\n";
            pushScope();
            for (const Stmt& t : s.thenBody) emitStmt(t, indent + 1);
            popScope();
            m_out.glslBody += pad + "}\n";
            break;
        }
        case Stmt::Kind::DoWhile:
        {
            m_out.glslBody += pad + "do\n" + pad + "{\n";
            pushScope();
            for (const Stmt& t : s.thenBody) emitStmt(t, indent + 1);
            popScope();
            Expr cond = s.expr;
            EmitResult c = emitExpr(cond);
            m_out.glslBody += pad + "} while (" + boolify(c, s.line) + ");\n";
            break;
        }
        case Stmt::Kind::For:
        {
            // Init-Statement(s) in einen umschliessenden Block, dann
            // for(; cond; step) — so bleibt die Decl-Emission wiederverwendet
            m_out.glslBody += pad + "{\n";
            pushScope();
            for (const Stmt& t : s.elseBody) emitStmt(t, indent + 1);
            std::string head = "for (; ";
            if (s.hasExpr)
            {
                Expr cond = s.expr;
                EmitResult c = emitExpr(cond);
                head += boolify(c, s.line);
            }
            head += "; ";
            if (s.hasStep)
            {
                Expr st = s.stepExpr;
                head += emitExpr(st).code;
            }
            head += ")";
            m_out.glslBody += pad + "    " + head + "\n" + pad + "    {\n";
            pushScope();
            for (const Stmt& t : s.thenBody) emitStmt(t, indent + 2);
            popScope();
            m_out.glslBody += pad + "    }\n";
            popScope();
            m_out.glslBody += pad + "}\n";
            break;
        }
        case Stmt::Kind::Break:
            m_out.glslBody += pad + "break;\n";
            break;
        case Stmt::Kind::Continue:
            m_out.glslBody += pad + "continue;\n";
            break;
        case Stmt::Kind::Return:
        {
            if (!m_inFunction)
                fail(s.line, "return im shader_body wird nicht unterstuetzt");
            if (s.hasExpr)
            {
                Expr e = s.expr;
                EmitResult r = emitExpr(e);
                r = convertTo(r, m_returnType, s.line);
                m_out.glslBody += pad + "return " + r.code + ";\n";
            }
            else
            {
                m_out.glslBody += pad + "return;\n";
            }
            break;
        }
        }
    }

    // --- Konvertierungen ----------------------------------------------------------------
    [[nodiscard]] static std::string numberToFloat(const std::string& text)
    {
        if (text.find('.') != std::string::npos || text.find('e') != std::string::npos ||
            text.find('E') != std::string::npos)
        {
            // ".5" → "0.5", "5." → "5.0" (GLSL akzeptiert beide, aber sauber ist sauber)
            std::string t = text;
            if (t.front() == '.') t.insert(0, "0");
            if (t.back() == '.') t += "0";
            return t;
        }
        return text + ".0";
    }

    /// Ausdruck in Zieltyp bringen (Skalar→vecN splatten, vecM→vecN kuerzen)
    [[nodiscard]] EmitResult convertTo(EmitResult r, Type target, int line) const
    {
        if (r.type.base == target.base) return r;
        if (target.base == Base::Float && r.type.base == Base::Bool)
        {
            r.code = "((" + r.code + ") ? 1.0 : 0.0)";
            r.type = {Base::Float};
            return r;
        }
        if (target.base == Base::Bool && r.type.base == Base::Float)
        {
            // HLSL: numerisch -> bool implizit (`bool b = f;`)
            r.code = "((" + r.code + ") != 0.0)";
            r.type = {Base::Bool};
            return r;
        }
        if (target.isVec() && r.type.base == Base::Bool)
        {
            // bool -> vecN ueber den Float-Umweg (`ret = above(a,b);`-Muster)
            r = convertTo(std::move(r), {Base::Float}, line);
        }
        if (target.isVec() && r.type.base == Base::Float)
        {
            r.code = std::string(target.glsl()) + "(" + r.code + ")";
            r.type = target;
            return r;
        }
        if (target.isVec() && r.type.isVec() && r.type.size() > target.size())
        {
            static const char* kSwiz[5] = {"", ".x", ".xy", ".xyz", ""};
            r.code = "(" + r.code + ")" + kSwiz[target.size()];
            r.type = target;
            return r;
        }
        if (target.base == Base::Float && r.type.isVec())
        {
            r.code = "(" + r.code + ").x";  // HLSL-Implicit-Truncation
            r.type = target;
            return r;
        }
        if (target.isMat() && r.type.base == Base::Float)
        {
            // HLSL: Skalar→Matrix REPLIZIERT in alle Elemente — GLSL matN(s)
            // waere nur die Diagonale → Spaltenvektoren explizit splatten
            const Type colVec = Type::vec(target.matRows());
            std::string code = std::string(target.glsl()) + "(";
            for (int c = 0; c < target.matCols(); ++c)
            {
                if (c > 0) code += ", ";
                code += std::string(colVec.glsl()) + "(" + r.code + ")";
            }
            code += ")";
            r.code = code;
            r.type = target;
            return r;
        }
        fail(line, std::string("Typkonflikt: ") + r.type.glsl() + " -> " + target.glsl());
    }

    [[nodiscard]] std::string boolify(EmitResult& r, int line) const
    {
        if (r.type.base == Base::Bool) return r.code;
        if (r.type.base == Base::Float) return "((" + r.code + ") != 0.0)";
        fail(line, "Bedingung muss skalar sein (Vektor-Vergleiche sind Stufe C3)");
    }

    // --- Expressions --------------------------------------------------------------------
    EmitResult emitExpr(Expr& e)
    {
        switch (e.kind)
        {
        case Expr::Kind::Number:
            return {numberToFloat(e.text), {Base::Float}};

        case Expr::Kind::Ident:
        {
            Type t;
            if (!lookup(e.text, t)) fail(e.line, "unbekannter Bezeichner '" + e.text + "'");
            return {outName(e.text), t};
        }

        case Expr::Kind::Member:
            return emitMember(e);

        case Expr::Kind::Ctor:
            return emitCtor(e);

        case Expr::Kind::Call:
            return emitCall(e);

        case Expr::Kind::Index:
        {
            // C3-Rest: Array-Element (auch vec[i] — HLSL erlaubt beides);
            // GLSL-Index ist int, unsere Zahlen sind float → int()-Cast
            EmitResult base = emitExpr(e.args[0]);
            EmitResult idx = emitExpr(e.args[1]);
            idx = convertTo(idx, {Base::Float}, e.line);
            Type elem;
            if (base.type.arraySize > 0)
            {
                elem = base.type;
                elem.arraySize = 0;
            }
            else if (base.type.isVec())
            {
                elem = {Base::Float};
            }
            else if (base.type.isMat())
            {
                // HLSL `M[i]` ist die ZEILE i (Laenge = Spaltenzahl), GLSL `m[i]`
                // die SPALTE. Unsere GLSL-Matrix ist die transponierte HLSL-Matrix
                // (matSpaltenxZeilen), also liefert `transpose(m)[i]` genau die
                // HLSL-Zeile — und wertet den Ausdruck nur einmal aus.
                // Alle neun Presets des Packs, die rot_* benutzen, tun es so
                // (`rot_f3[0] * .1`), keines ueber mul() (Session 52).
                elem = Type::vec(base.type.matCols());
                return {"transpose(" + base.code + ")[int(" + idx.code + ")]", elem};
            }
            else
            {
                fail(e.line, "Index auf Nicht-Array");
            }
            return {base.code + "[int(" + idx.code + ")]", elem};
        }

        case Expr::Kind::Unary:
        {
            EmitResult a = emitExpr(e.args[0]);
            if (e.text == "!")
            {
                std::string cond = boolify(a, e.line);
                return {"(!" + cond + ")", {Base::Bool}};
            }
            // C3: Postfix ++/-- (Praefix laeuft ueber den generischen Zweig)
            if (e.text == "post++" || e.text == "post--")
                return {"(" + a.code + e.text.substr(4) + ")", a.type};
            // HLSL: -(a>b) numerisch — GLSL kennt kein Minus auf bool
            if (a.type.base == Base::Bool && (e.text == "-" || e.text == "+"))
                a = convertTo(a, {Base::Float}, e.line);
            return {"(" + e.text + a.code + ")", a.type};
        }

        case Expr::Kind::Binary:
            return emitBinary(e);

        case Expr::Kind::Ternary:
        {
            EmitResult c = emitExpr(e.args[0]);
            EmitResult a = emitExpr(e.args[1]);
            EmitResult b = emitExpr(e.args[2]);
            Type common = commonType(a.type, b.type, e.line);
            a = convertTo(a, common, e.line);
            b = convertTo(b, common, e.line);
            return {"(" + boolify(c, e.line) + " ? " + a.code + " : " + b.code + ")", common};
        }

        case Expr::Kind::Assign:
            return emitAssign(e);

        default:
            fail(e.line, "nicht unterstuetzter Ausdruck");
        }
    }

    EmitResult emitMember(Expr& e)
    {
        EmitResult base = emitExpr(e.args[0]);
        const std::string& sw = e.text;
        if (!base.type.isVec() && base.type.base != Base::Float)
            fail(e.line, "Swizzle auf Nicht-Vektor");
        for (char c : sw)
        {
            if (std::string("xyzwrgba").find(c) == std::string::npos)
                fail(e.line, "unbekanntes Swizzle '" + sw + "'");
        }
        if (sw.size() > 4) fail(e.line, "Swizzle zu lang");
        // float.xxx (HLSL erlaubt Splat-Swizzle auf Skalar) → Konstruktor
        if (base.type.base == Base::Float)
        {
            const Type target = Type::vec(static_cast<int>(sw.size()));
            if (sw.size() == 1) return base;
            return {std::string(target.glsl()) + "(" + base.code + ")", target};
        }
        return {base.code + "." + sw, Type::vec(static_cast<int>(sw.size()))};
    }

    EmitResult emitCtor(Expr& e)
    {
        Type target;
        (void)typeFromKeyword("float", target);  // placeholder init
        for (const auto base : {Base::Float, Base::Vec2, Base::Vec3, Base::Vec4, Base::Mat2,
                                Base::Mat3, Base::Mat4, Base::Mat2x3, Base::Mat3x2})
        {
            if (e.text == Type{base}.glsl()) target = {base};
        }
        std::vector<EmitResult> args;
        args.reserve(e.args.size());
        for (Expr& arg : e.args)
        {
            EmitResult a = emitExpr(arg);
            if (a.type.base == Base::Bool) a = convertTo(a, {Base::Float}, e.line);
            args.push_back(std::move(a));
        }
        // Nicht-Quadrat-Matrizen (C3-Rest): HLSL konstruiert ZEILENweise,
        // GLSL SPALTENweise — transponierte Emission (Zeilen-Vektoren oder
        // Skalare in Zeilen-Major-Reihenfolge)
        if (target.base == Base::Mat2x3 || target.base == Base::Mat3x2)
        {
            const int rows = target.matRows();
            const int cols = target.matCols();
            std::string code = std::string(target.glsl()) + "(";
            bool first = true;
            const auto emitPart = [&](const std::string& part) {
                if (!first) code += ", ";
                code += part;
                first = false;
            };
            static const char kComp[5] = "xyzw";
            if (static_cast<int>(args.size()) == rows)
            {
                for (EmitResult& r : args) r = convertTo(r, Type::vec(cols), e.line);
                for (int c = 0; c < cols; ++c)
                    for (int r = 0; r < rows; ++r)
                        emitPart("(" + args[static_cast<std::size_t>(r)].code + ")." +
                                 kComp[c]);
            }
            else if (static_cast<int>(args.size()) == rows * cols)
            {
                for (EmitResult& r : args) r = convertTo(r, {Base::Float}, e.line);
                for (int c = 0; c < cols; ++c)
                    for (int r = 0; r < rows; ++r)
                        emitPart(args[static_cast<std::size_t>(r * cols + c)].code);
            }
            else
            {
                fail(e.line, e.text + ": Zeilen-Vektoren oder Skalare erwartet");
            }
            code += ")";
            return {code, target};
        }
        std::string code = e.text + "(";
        for (std::size_t i = 0; i < args.size(); ++i)
        {
            if (i > 0) code += ", ";
            code += args[i].code;
        }
        code += ")";
        return {code, target};
    }

    /// Nenner-Check fuer die _div-Emission: nur ein nachweislich von Null
    /// verschiedenes Zahlen-Literal (ggf. mit Vorzeichen) ist gefahrlos.
    [[nodiscard]] static bool isNonZeroNumberLiteral(const Expr& e)
    {
        const Expr* n = &e;
        while (n->kind == Expr::Kind::Unary && (n->text == "-" || n->text == "+"))
            n = &n->args[0];
        if (n->kind != Expr::Kind::Number) return false;
        return std::stod(n->text) != 0.0;
    }

    EmitResult emitBinary(Expr& e)
    {
        EmitResult a = emitExpr(e.args[0]);
        EmitResult b = emitExpr(e.args[1]);
        const std::string& op = e.text;

        if (op == ",")  // Sequenz-Operator (GLSL-nativ)
            return {"(" + a.code + ", " + b.code + ")", b.type};
        if (op == "&&" || op == "||")
        {
            return {"(" + boolify(a, e.line) + " " + op + " " + boolify(b, e.line) + ")",
                    {Base::Bool}};
        }
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=")
        {
            // C3: Vektor-Vergleiche — HLSL liefert komponentenweise 0/1;
            // GLSL-Aequivalent: lessThan()-Familie + Vektor-Konstruktor
            if (a.type.isVec() || b.type.isVec())
            {
                const Type common = commonType(a.type, b.type, e.line);
                a = convertTo(a, common, e.line);
                b = convertTo(b, common, e.line);
                const char* fn = op == "<"    ? "lessThan"
                                 : op == ">"  ? "greaterThan"
                                 : op == "<=" ? "lessThanEqual"
                                 : op == ">=" ? "greaterThanEqual"
                                 : op == "==" ? "equal"
                                              : "notEqual";
                return {std::string(common.glsl()) + "(" + fn + "(" + a.code + ", " +
                            b.code + "))",
                        common};
            }
            return {"(" + a.code + " " + op + " " + b.code + ")", {Base::Bool}};
        }
        if (op == "%")
        {
            // HLSL % auf floats → GLSL mod(); Operanden harmonisieren
            if (a.type.base == Base::Bool) a = convertTo(a, {Base::Float}, e.line);
            if (b.type.base == Base::Bool) b = convertTo(b, {Base::Float}, e.line);
            const Type common = commonType(a.type, b.type, e.line);
            a = convertTo(a, common, e.line);
            b = convertTo(b, common, e.line);
            return {"mod(" + a.code + ", " + b.code + ")", common};
        }

        // HLSL: bool-Operanden in Arithmetik implizit numerisch (`x * (a>b)`,
        // beliebtes Gate-Muster) — GLSL lehnt bool-Arithmetik ab (C3-Rest)
        if (a.type.base == Base::Bool) a = convertTo(a, {Base::Float}, e.line);
        if (b.type.base == Base::Bool) b = convertTo(b, {Base::Float}, e.line);

        // Division ueber _div (Host-Geruest): D3D9 rechnet Legacy-Float
        // (0*INF=0, rcp(0) endlich) — IEEE-`0/0` ergibt NaN und vergiftet
        // jede Summe (S64, Rainbow Attack NEON: Vollschwarz). Beweisbar
        // ungefaehrliche Literal-Nenner bleiben rohe Division.
        if (op == "/" && !a.type.isMat() && !b.type.isMat() &&
            !isNonZeroNumberLiteral(e.args[1]))
        {
            const Type common = commonType(a.type, b.type, e.line);
            a = convertTo(a, common, e.line);
            b = convertTo(b, common, e.line);
            return {"_div(" + a.code + ", " + b.code + ")", common};
        }

        // Arithmetik: mat*vec / vec*mat / mat*mat gehen in GLSL direkt
        if (a.type.isMat() || b.type.isMat())
        {
            Type result = a.type.isMat() && b.type.isMat()
                              ? a.type
                              : (a.type.isMat() ? b.type : a.type);
            return {"(" + a.code + " " + op + " " + b.code + ")", result};
        }
        const Type common = commonType(a.type, b.type, e.line);
        // HLSL-Implicit-Truncation: beide Vektoren auf die gemeinsame Groesse
        if (a.type.isVec() && b.type.isVec() && a.type.base != b.type.base)
        {
            a = convertTo(a, common, e.line);
            b = convertTo(b, common, e.line);
        }
        return {"(" + a.code + " " + op + " " + b.code + ")", common};
    }

    [[nodiscard]] Type commonType(Type a, Type b, int line) const
    {
        if (a.base == Base::Bool) a = {Base::Float};
        if (b.base == Base::Bool) b = {Base::Float};
        if (!a.isNumeric() || !b.isNumeric()) fail(line, "numerischer Ausdruck erwartet");
        if (a.base == b.base) return a;
        if (a.base == Base::Float) return b;
        if (b.base == Base::Float) return a;
        // vecN op vecM: HLSL kuerzt implizit auf den kleineren Vektor
        return Type::vec(std::min(a.size(), b.size()));
    }

    EmitResult emitAssign(Expr& e)
    {
        // Zuweisung an eine Uniform (q25=…, texsize.xy=…): im main-Prolog eine
        // lokale Schattenkopie anlegen (GLSL-Scope-Regel: `vec4 texsize =
        // texsize;` liest im Initializer noch die Uniform) — fxc erlaubte
        // Schreibzugriffe auf globale Konstanten. Vom Preset selbst
        // deklarierte gleichnamige Locals brauchen keinen Schatten.
        const Expr* root = &e.args[0];
        while ((root->kind == Expr::Kind::Member || root->kind == Expr::Kind::Index) &&
               !root->args.empty())
            root = &root->args[0];
        if (root->kind == Expr::Kind::Ident &&
            m_uniformNames.count(root->text) != 0 &&
            m_userDeclared.count(root->text) == 0)
        {
            m_shadow[root->text] = true;
        }
        EmitResult lhs = emitExpr(e.args[0]);
        EmitResult rhs = emitExpr(e.args[1]);
        if (e.text == "=")
        {
            rhs = convertTo(rhs, lhs.type, e.line);
            return {lhs.code + " = " + rhs.code, lhs.type};
        }
        // += -= *= /= : vec op float ist in GLSL erlaubt; float op vec nicht sinnvoll
        if (lhs.type.base == Base::Float && rhs.type.isVec())
            rhs = convertTo(rhs, {Base::Float}, e.line);
        if (rhs.type.base == Base::Bool) rhs = convertTo(rhs, {Base::Float}, e.line);
        // HLSL-Implicit-Truncation auch bei Compound-Zuweisungen: breiterer
        // RHS-Vektor wird auf die LHS-Breite gekuerzt (`ret -= tex2D(...)`,
        // `ret.xy *= 1-GetBlur1(...)` — fxc warnte nur, GLSL lehnt ab)
        if (lhs.type.isVec() && rhs.type.isVec() &&
            rhs.type.size() > lhs.type.size())
        {
            rhs = convertTo(rhs, lhs.type, e.line);
        }
        // `/=` unter dem D3D9-Divisionsvertrag (s. emitBinary) — Literal-Nenner
        // bleibt rohe Division
        if (e.text == "/=" && !lhs.type.isMat() && !isNonZeroNumberLiteral(e.args[1]))
        {
            rhs = convertTo(rhs, lhs.type, e.line);
            return {lhs.code + " = _div(" + lhs.code + ", " + rhs.code + ")", lhs.type};
        }
        return {lhs.code + " " + e.text + " " + rhs.code, lhs.type};
    }

    // --- Intrinsics ---------------------------------------------------------------------
    EmitResult emitCall(Expr& e)
    {
        const std::string& name = e.text;
        const std::string lower = toLower(name);  // fxc war case-insensitiv (tex2d!)

        // Nutzerfunktionen (vor shader_body definiert) zuerst
        if (const auto fn = m_functions.find(name); fn != m_functions.end())
        {
            if (e.args.size() != fn->second.params.size())
                fail(e.line, name + ": " + std::to_string(fn->second.params.size()) +
                                 " Argument(e) erwartet");
            std::string code = outName(name) + "(";
            for (std::size_t i = 0; i < e.args.size(); ++i)
            {
                EmitResult a = emitExpr(e.args[i]);
                // out/inout-Argumente muessen L-Values bleiben — keine
                // Konvertierungs-Wrapper (C3-Rest)
                const bool isOut = i < fn->second.paramQual.size() &&
                                   fn->second.paramQual[i] != 0;
                if (!isOut) a = convertTo(a, fn->second.params[i], e.line);
                if (i > 0) code += ", ";
                code += a.code;
            }
            code += ")";
            return {code, fn->second.returnType};
        }

        // Samplerfunktionen
        if (lower == "tex2d" || lower == "tex2dbias")
        {
            requireArgs(e, 2);
            EmitResult s = emitExpr(e.args[0]);
            if (s.type.base != Base::Sampler2D) fail(e.line, "tex2D: Sampler erwartet");
            EmitResult uvArg = emitExpr(e.args[1]);
            uvArg = convertTo(uvArg, {Base::Vec2}, e.line);
            return {"texture(" + s.code + ", " + uvArg.code + ")", {Base::Vec4}};
        }
        if (lower == "tex2dlod")
        {
            requireArgs(e, 2);
            EmitResult s = emitExpr(e.args[0]);
            if (s.type.base != Base::Sampler2D) fail(e.line, "tex2Dlod: Sampler erwartet");
            EmitResult a = emitExpr(e.args[1]);
            a = convertTo(a, {Base::Vec4}, e.line);
            return {"textureLod(" + s.code + ", (" + a.code + ").xy, (" + a.code + ").w)",
                    {Base::Vec4}};
        }
        if (lower == "tex3d" || lower == "tex3dbias")
        {
            // C3: Volumen-Noise — Koordinate ist vec3; akzeptiert auch als
            // `sampler` (2D) redeklarierte noisevol-Namen (Uniform ist 3D)
            m_out.usesTex3d = true;
            requireArgs(e, 2);
            EmitResult s = emitExpr(e.args[0]);
            if (s.type.base != Base::Sampler3D && s.type.base != Base::Sampler2D)
                fail(e.line, "tex3D: Sampler erwartet");
            EmitResult c = emitExpr(e.args[1]);
            c = convertTo(c, {Base::Vec3}, e.line);
            return {"texture(" + s.code + ", " + c.code + ")", {Base::Vec4}};
        }
        if (name == "mul")
        {
            requireArgs(e, 2);
            EmitResult a = emitExpr(e.args[0]);
            EmitResult b = emitExpr(e.args[1]);
            Type result;
            if (a.type.isMat() && b.type.isMat())
            {
                result = a.type;
            }
            else if (a.type.isMat() && b.type.isVec())
            {
                // HLSL mul(M[RxC], vC) -> vR; GLSL matCxR * vecC -> vecR
                b = convertTo(b, Type::vec(a.type.matCols()), e.line);
                result = Type::vec(a.type.matRows());
            }
            else if (a.type.isVec() && b.type.isMat())
            {
                // HLSL mul(vR, M[RxC]) -> vC; GLSL vecR * matCxR -> vecC
                a = convertTo(a, Type::vec(b.type.matRows()), e.line);
                result = Type::vec(b.type.matCols());
            }
            else if (a.type.isVec() && b.type.isVec())
            {
                // HLSL mul(vektor, vektor) = Skalarprodukt (C3-Rest)
                const Type common = commonType(a.type, b.type, e.line);
                a = convertTo(a, common, e.line);
                b = convertTo(b, common, e.line);
                return {"dot(" + a.code + ", " + b.code + ")", {Base::Float}};
            }
            else
            {
                result = commonType(a.type, b.type, e.line);
            }
            return {"(" + a.code + " * " + b.code + ")", result};
        }

        // Praeambel-Makros (GetBlurN un-biased etc.)
        if (name == "GetMain" || name == "GetPixel" || name == "GetBlur1" ||
            name == "GetBlur2" || name == "GetBlur3")
        {
            requireArgs(e, 1);
            EmitResult uvArg = emitExpr(e.args[0]);
            uvArg = convertTo(uvArg, {Base::Vec2}, e.line);
            return {name + "(" + uvArg.code + ")", {Base::Vec3}};
        }
        if (name == "lum")
        {
            requireArgs(e, 1);
            EmitResult a = emitExpr(e.args[0]);
            a = convertTo(a, {Base::Vec3}, e.line);
            return {"lum(" + a.code + ")", {Base::Float}};
        }

        struct Intrinsic
        {
            const char* hlsl;
            const char* glsl;
            int args;        // -1 = 1..2 (min/max-artig fix 2; hier: exakt)
            int resultRule;  // 0=Arg0, 1=Float, 2=MaxArgs
            bool promoteAll; // Argumente auf gemeinsamen Vektortyp heben
        };
        static constexpr std::array<Intrinsic, 38> kIntrinsics = {{
            {"sin", "sin", 1, 0, false},       {"cos", "cos", 1, 0, false},
            {"tan", "tan", 1, 0, false},       {"asin", "asin", 1, 0, false},
            {"acos", "acos", 1, 0, false},     {"atan", "atan", 1, 0, false},
            {"sqrt", "sqrt", 1, 0, false},     {"abs", "abs", 1, 0, false},
            {"floor", "floor", 1, 0, false},   {"ceil", "ceil", 1, 0, false},
            {"exp", "exp", 1, 0, false},       {"exp2", "exp2", 1, 0, false},
            {"log", "log", 1, 0, false},       {"log2", "log2", 1, 0, false},
            {"log10", "", 1, 0, false},
            {"sign", "sign", 1, 0, false},     {"frac", "fract", 1, 0, false},
            {"round", "round", 1, 0, false},
            {"normalize", "normalize", 1, 0, false},
            {"rsqrt", "inversesqrt", 1, 0, false},
            {"ddx", "dFdx", 1, 0, false},      {"ddy", "dFdy", 1, 0, false},
            {"length", "length", 1, 1, false}, {"saturate", "", 1, 0, false},
            {"atan2", "atan", 2, 2, true},     {"pow", "pow", 2, 2, true},
            {"fmod", "mod", 2, 2, false},      {"min", "min", 2, 2, false},
            {"max", "max", 2, 2, false},       {"step", "step", 2, 2, false},
            {"dot", "dot", 2, 1, true},        {"distance", "distance", 2, 1, true},
            {"cross", "cross", 2, 0, true},    {"reflect", "reflect", 2, 0, true},
            {"lerp", "mix", 3, 2, false},      {"clamp", "clamp", 3, 0, false},
            {"smoothstep", "smoothstep", 3, 3, false},
            {"modf", "modf", 2, 0, false},  // out-Param 2 wird durchgereicht
        }};

        for (const Intrinsic& in : kIntrinsics)
        {
            if (lower != in.hlsl) continue;
            if (static_cast<int>(e.args.size()) != in.args)
                fail(e.line, name + ": " + std::to_string(in.args) + " Argument(e) erwartet");

            std::vector<EmitResult> args;
            args.reserve(e.args.size());
            int minVec = 0;  // kleinste Vektor-Breite unter den Argumenten
            for (Expr& a : e.args)
            {
                EmitResult r = emitExpr(a);
                if (r.type.base == Base::Bool) r = convertTo(r, {Base::Float}, e.line);
                if (!r.type.isNumeric()) fail(e.line, name + ": numerisches Argument erwartet");
                if (r.type.isVec())
                    minVec = (minVec == 0) ? r.type.size()
                                           : std::min(minVec, r.type.size());
                args.push_back(std::move(r));
            }
            // HLSL-Implicit-Truncation (S43): gemischte Vektor-Breiten werden
            // auf die KLEINSTE Breite gekuerzt (lerp(vec3, tex2D(...)), …) —
            // fxc warnte nur, GLSL braucht exakte Breiten
            if (minVec > 0)
            {
                for (EmitResult& r : args)
                    if (r.type.isVec() && r.type.size() > minVec)
                        r = convertTo(r, Type::vec(minVec), e.line);
            }
            const Type common = Type::vec(minVec > 0 ? minVec : 1);
            if (in.promoteAll)
            {
                for (EmitResult& r : args) r = convertTo(r, common, e.line);
            }
            // lerp/mix: a und b muessen gleich sein, t darf float bleiben
            if (lower == "lerp")
            {
                const int abSize = std::max(args[0].type.size(), args[1].type.size());
                const Type ab = Type::vec(abSize);
                args[0] = convertTo(args[0], ab, e.line);
                args[1] = convertTo(args[1], ab, e.line);
                // vektorielles t skaliert HLSL komponentenweise — GLSL auch (vec-mix)
                if (args[2].type.isVec()) args[2] = convertTo(args[2], ab, e.line);
            }
            // min/max/fmod/step: (vec, float) ist in GLSL erlaubt, (float, vec) NICHT
            if ((lower == "min" || lower == "max" || lower == "fmod" || lower == "step") &&
                args[0].type.base == Base::Float && args[1].type.isVec())
            {
                args[0] = convertTo(args[0], args[1].type, e.line);
            }

            // saturate → clamp(x, 0.0, 1.0)
            if (lower == "saturate")
                return {"clamp(" + args[0].code + ", 0.0, 1.0)", args[0].type};
            // log10 → log(x)/ln(10) — GLSL 330 hat kein log10 (komponentenweise)
            if (lower == "log10")
                return {"(log(" + args[0].code + ") * 0.43429448190325176)",
                        args[0].type};

            std::string code = std::string(in.glsl) + "(";
            for (std::size_t i = 0; i < args.size(); ++i)
            {
                if (i > 0) code += ", ";
                code += args[i].code;
            }
            code += ")";

            Type result;
            switch (in.resultRule)
            {
            case 0: result = args[0].type; break;
            case 1: result = {Base::Float}; break;
            case 3: result = args.back().type; break;  // smoothstep: Typ von x
            default:
                result = (lower == "lerp") ? Type::vec(std::max(args[0].type.size(),
                                                                args[1].type.size()))
                                           : common;
                break;
            }
            return {code, result};
        }

        fail(e.line, "unbekannte Funktion '" + name + "'");
    }

    static void requireArgs(const Expr& e, std::size_t n)
    {
        if (e.args.size() != n)
            fail(e.line, e.text + ": " + std::to_string(n) + " Argument(e) erwartet");
    }

    struct FunctionSig
    {
        Type returnType;
        std::vector<Type> params;
        std::vector<std::uint8_t> paramQual;  ///< 0=in, 1=out, 2=inout
    };

    HlslResult& m_out;
    std::vector<std::unordered_map<std::string, Type>> m_scopes;
    std::unordered_map<std::string, FunctionSig> m_functions;
    std::string m_globalInit;       ///< Initialisierer mutabler Globals (main-Anfang)
    Type m_returnType{Base::Float}; ///< aktuelle Funktions-Signatur
    bool m_inFunction = false;
    std::unordered_map<std::string, bool> m_shadow;        ///< beschriebene Uniforms
    std::unordered_map<std::string, bool> m_uniformNames;  ///< Uniform-Builtins
    std::unordered_map<std::string, bool> m_userDeclared;  ///< Preset-eigene Namen
    std::unordered_map<std::string, std::string> m_rename; ///< reservierte Namen
};

} // namespace detail

/**
 * @brief Transpile one preset shader text (warp_/comp_ block) to GLSL parts
 */
[[nodiscard]] inline HlslResult transpile(std::string_view presetShaderText, ShaderKind kind)
{
    HlslResult result;
    try
    {
        const std::string preprocessed = detail::preprocess(presetShaderText);
        std::vector<detail::Stmt> globals;
        std::vector<detail::FunctionDef> functions;
        std::vector<detail::Stmt> body;
        detail::Parser parser(preprocessed);
        parser.parseProgram(globals, functions, body);
        detail::CodeGen gen(kind, result);
        gen.emitGlobals(globals);
        gen.emitFunctions(functions);
        gen.emitBody(body);
        result.glslBody = gen.shadowProlog() + result.glslBody;
        result.ok = true;
    }
    catch (const detail::TranspileError& err)
    {
        result.ok = false;
        result.error = err.message;
    }
    return result;
}

} // namespace lumi::hlsl
