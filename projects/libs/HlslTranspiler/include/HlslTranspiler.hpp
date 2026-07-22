/**
 ****************************************************************************************
 * @file   HlslTranspiler.hpp
 * @brief  HLSL(ps_2/3-Teilmenge) → GLSL-330-Transpiler für MilkDrop-Preset-Shader
 *         (Import-Phase Stufe C1 — Entscheid E4, Session 40)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
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
 * BEWUSST NICHT in C1 (→ Stufe C3, sauberer Fehler statt Falschübersetzung):
 * for/while-Schleifen, tex3D/Volumen-Noise, Arrays, Funktionsdefinitionen,
 * Vektor-Vergleiche, Initialisierer-Listen, Präprozessor im Preset-Text.
 ****************************************************************************************
 */

#pragma once

#include <array>
#include <cstddef>
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
    Sampler2D,
    Sampler3D
};

struct Type
{
    Base base = Base::Unknown;

    [[nodiscard]] bool isVec() const
    {
        return base == Base::Vec2 || base == Base::Vec3 || base == Base::Vec4;
    }
    [[nodiscard]] bool isMat() const
    {
        return base == Base::Mat2 || base == Base::Mat3 || base == Base::Mat4;
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
    else if (s == "sampler" || s == "sampler2D") out = {Base::Sampler2D};
    else if (s == "sampler3D") out = {Base::Sampler3D};
    else if (s == "void") out = {Base::Void};
    else return false;
    return true;
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
            out += "(" + m.body + ")";
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
            if (trimmed.rfind("define", 0) != 0)
                fail(line, "Praeprozessor-Direktive wird nicht unterstuetzt");
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
            if (!m.body.empty() && m.body.back() == '\\')
                fail(line, "mehrzeilige #define werden nicht unterstuetzt");
            macros[name] = std::move(m);
            out += "\n";  // Zeilennummern stabil halten
        }
        else
        {
            out.append(raw);
            out += "\n";
        }
        if (nl == std::string_view::npos) break;
        pos = nl + 1;
        ++line;
    }
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
    [[nodiscard]] Token next()
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
        static constexpr std::array<std::string_view, 10> kMulti = {
            "+=", "-=", "*=", "/=", "==", "!=", "<=", ">=", "&&", "||"};
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
    enum class Kind { Decl, ExprStmt, If, Block, Return };
    Kind kind = Kind::ExprStmt;
    // Decl
    Type declType;
    std::vector<std::pair<std::string, Expr>> decls;  // name, init (init.kind==Number&&text empty → keiner)
    std::vector<bool> hasInit;
    // ExprStmt / If-Bedingung / Return-Wert
    Expr expr;
    bool hasExpr = false;  // Return: mit Wert?
    // If
    std::vector<Stmt> thenBody;  // auch Block-Inhalt
    std::vector<Stmt> elseBody;
    bool hasElse = false;
    int line = 0;
};

/// Hilfsfunktion vor shader_body (z. B. complex_mul) — wird GLSL-Funktion
struct FunctionDef
{
    Type returnType;
    std::string name;
    std::vector<std::pair<std::string, Type>> params;
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
            if (t.text == "for" || t.text == "while" || t.text == "do")
                fail(t.line, "Schleifen (" + t.text + ") sind Stufe C3");
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
                // optionale in/out-Qualifier (out waere Referenz-Semantik → C3)
                if (m_lex.peek().kind == Token::Kind::Ident && m_lex.peek().text == "in")
                    m_lex.next();
                if (m_lex.peek().kind == Token::Kind::Ident &&
                    (m_lex.peek().text == "out" || m_lex.peek().text == "inout"))
                {
                    fail(m_lex.peek().line, "out-Parameter sind Stufe C3");
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
            if (m_lex.peek().kind == Token::Kind::Punct && m_lex.peek().text == "[")
                fail(nameTok.line, "Arrays sind Stufe C3");
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
                fail(t.line, "Array-Indizes sind Stufe C3");
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
                    declare(name, s.declType);
                    if (s.declType.base == Base::Sampler3D)
                    {
                        m_out.usesTex3d = true;
                        fail(s.line, "sampler3D/Volumen-Noise ist Stufe C3");
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
                declare(name, s.declType);
                if (name.rfind("texsize_", 0) == 0)
                {
                    m_out.customTexsizes.push_back(name);
                    continue;  // Uniform kommt vom Host
                }
                m_out.glslGlobals += std::string(s.declType.glsl()) + " " + name + " = " +
                                     s.declType.glsl() + "(0.0);\n";
                if (s.hasInit[i])
                {
                    Expr initCopy = init;
                    EmitResult r = emitExpr(initCopy);
                    r = convertTo(r, s.declType, s.line);
                    m_globalInit += "    " + name + " = " + r.code + ";\n";
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
            m_functions[fn.name] = sig;

            std::string head = std::string(fn.returnType.glsl()) + " " + fn.name + "(";
            for (std::size_t i = 0; i < fn.params.size(); ++i)
            {
                if (i > 0) head += ", ";
                head += std::string(fn.params[i].second.glsl()) + " " + fn.params[i].first;
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

private:
    // --- Symboltabelle ------------------------------------------------------------------
    void pushScope() { m_scopes.emplace_back(); }
    void popScope() { m_scopes.pop_back(); }
    void declare(const std::string& name, Type t) { m_scopes.back()[name] = t; }
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
        smp3("sampler_noisevol_lq");
        smp3("sampler_noisevol_hq");
        v4("texsize_noisevol_lq");
        v4("texsize_noisevol_hq");
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
                declare(name, s.declType);
                std::string lineOut = pad + std::string(s.declType.glsl()) + " " + name;
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
            return {e.text, t};
        }

        case Expr::Kind::Member:
            return emitMember(e);

        case Expr::Kind::Ctor:
            return emitCtor(e);

        case Expr::Kind::Call:
            return emitCall(e);

        case Expr::Kind::Unary:
        {
            EmitResult a = emitExpr(e.args[0]);
            if (e.text == "!")
            {
                std::string cond = boolify(a, e.line);
                return {"(!" + cond + ")", {Base::Bool}};
            }
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
                                Base::Mat3, Base::Mat4})
        {
            if (e.text == Type{base}.glsl()) target = {base};
        }
        std::string code = e.text + "(";
        for (std::size_t i = 0; i < e.args.size(); ++i)
        {
            EmitResult a = emitExpr(e.args[i]);
            if (a.type.base == Base::Bool) a = convertTo(a, {Base::Float}, e.line);
            if (i > 0) code += ", ";
            code += a.code;
        }
        code += ")";
        return {code, target};
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
            if (a.type.size() != 1 || b.type.size() != 1)
                fail(e.line, "Vektor-Vergleiche sind Stufe C3");
            return {"(" + a.code + " " + op + " " + b.code + ")", {Base::Bool}};
        }
        if (op == "%")
        {
            // HLSL % auf floats → GLSL mod()
            Type common = commonType(a.type, b.type, e.line);
            return {"mod(" + a.code + ", " + b.code + ")", common};
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
            std::string code = name + "(";
            for (std::size_t i = 0; i < e.args.size(); ++i)
            {
                EmitResult a = emitExpr(e.args[i]);
                a = convertTo(a, fn->second.params[i], e.line);
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
        if (lower == "tex3d")
        {
            m_out.usesTex3d = true;
            fail(e.line, "tex3D/Volumen-Noise ist Stufe C3");
        }
        if (name == "mul")
        {
            requireArgs(e, 2);
            EmitResult a = emitExpr(e.args[0]);
            EmitResult b = emitExpr(e.args[1]);
            Type result;
            if (a.type.isMat() && b.type.isMat()) result = a.type;
            else if (a.type.isMat()) result = b.type;
            else if (b.type.isMat()) result = a.type;
            else result = commonType(a.type, b.type, e.line);
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
        static constexpr std::array<Intrinsic, 34> kIntrinsics = {{
            {"sin", "sin", 1, 0, false},       {"cos", "cos", 1, 0, false},
            {"tan", "tan", 1, 0, false},       {"asin", "asin", 1, 0, false},
            {"acos", "acos", 1, 0, false},     {"atan", "atan", 1, 0, false},
            {"sqrt", "sqrt", 1, 0, false},     {"abs", "abs", 1, 0, false},
            {"floor", "floor", 1, 0, false},   {"ceil", "ceil", 1, 0, false},
            {"exp", "exp", 1, 0, false},       {"exp2", "exp2", 1, 0, false},
            {"log", "log", 1, 0, false},       {"log2", "log2", 1, 0, false},
            {"sign", "sign", 1, 0, false},     {"frac", "fract", 1, 0, false},
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
        }};

        for (const Intrinsic& in : kIntrinsics)
        {
            if (lower != in.hlsl) continue;
            if (static_cast<int>(e.args.size()) != in.args)
                fail(e.line, name + ": " + std::to_string(in.args) + " Argument(e) erwartet");

            std::vector<EmitResult> args;
            args.reserve(e.args.size());
            int maxSize = 1;
            for (Expr& a : e.args)
            {
                EmitResult r = emitExpr(a);
                if (r.type.base == Base::Bool) r = convertTo(r, {Base::Float}, e.line);
                if (!r.type.isNumeric()) fail(e.line, name + ": numerisches Argument erwartet");
                maxSize = std::max(maxSize, r.type.size());
                args.push_back(std::move(r));
            }
            const Type common = Type::vec(maxSize);
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
    };

    HlslResult& m_out;
    std::vector<std::unordered_map<std::string, Type>> m_scopes;
    std::unordered_map<std::string, FunctionSig> m_functions;
    std::string m_globalInit;       ///< Initialisierer mutabler Globals (main-Anfang)
    Type m_returnType{Base::Float}; ///< aktuelle Funktions-Signatur
    bool m_inFunction = false;
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
