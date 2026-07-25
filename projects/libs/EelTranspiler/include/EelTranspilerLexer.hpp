/**
 ****************************************************************************************
 * @file   EelTranspilerLexer.hpp
 * @brief  Lexer for EEL sources (AVS ns-eel v1 + MilkDrop ns-eel2 dialects)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Tokenizes the EEL superset accepted by the transpiler (Import-Analyse §7):
 * - Numbers: decimal/scientific, hex as `3Bh` (AVS), `$x3B` (MilkDrop), `0x3B`
 *   (EEL2), char codes `$'c'`, constants $PI/$E/$PHI (textual value)
 * - Identifiers: case-insensitive (stored lowercased), [A-Za-z_][A-Za-z0-9_]*
 * - Comments: `//` and `/ * ... * /`
 * - Operators incl. the MilkDrop infix set (== != <= >= && || ?: ^ compound-assign)
 ****************************************************************************************
 */

#pragma once

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::eel::detail {

enum class Tok
{
    Number, Ident,
    LParen, RParen, LBracket, RBracket, Comma, Semi,
    Question, Colon,
    Assign, PlusAssign, MinusAssign, MulAssign, DivAssign, ModAssign,
    OrAssign, AndAssign, PowAssign,
    Plus, Minus, Star, Slash, Percent, Caret, Amp, Pipe, Bang,
    Lt, Gt, Le, Ge, EqEq, NotEq, AndAnd, OrOr,
    End
};

struct Token
{
    Tok kind = Tok::End;
    double number = 0.0;
    std::string text;   ///< identifier (lowercased)
    int line = 1;
    int col = 1;
};

class Lexer
{
public:
    explicit Lexer(std::string_view src) : m_src(src) {}

    /// @brief Tokenize the whole source. Returns false on lex error (see error()).
    bool tokenize(std::vector<Token>& out)
    {
        while (true)
        {
            skipWhitespaceAndComments();
            if (atEnd())
            {
                out.push_back(make(Tok::End));
                return true;
            }

            const char c = peek();
            if (std::isdigit(static_cast<unsigned char>(c)) || (c == '.' && isDigitAt(1)))
            {
                if (!lexNumber(out)) return false;
                continue;
            }
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
            {
                lexIdent(out);
                continue;
            }
            if (c == '$')
            {
                if (!lexDollar(out)) return false;
                continue;
            }
            // Nicht-ASCII-Bytes still ueberspringen wie das Original-EEL:
            // AVS-Autoren signieren Skripte mit Sonderzeichen (UnConeD: ';\xA9;'
            // = ';(c);' — Befund S46/Anemone: Lexer-Abbruch toetete den Slot
            // und liess alle Farben auf 0 -> Preset schwarz).
            if (static_cast<unsigned char>(c) >= 0x80)
            {
                advance();
                continue;
            }
            if (!lexOperator(out)) return false;
        }
    }

    [[nodiscard]] const std::string& error() const { return m_error; }

private:
    [[nodiscard]] bool atEnd() const { return m_pos >= m_src.size(); }
    [[nodiscard]] char peek(std::size_t ahead = 0) const
    {
        return (m_pos + ahead < m_src.size()) ? m_src[m_pos + ahead] : '\0';
    }
    [[nodiscard]] bool isDigitAt(std::size_t ahead) const
    {
        return std::isdigit(static_cast<unsigned char>(peek(ahead))) != 0;
    }
    char advance()
    {
        const char c = m_src[m_pos++];
        if (c == '\n') { ++m_line; m_col = 1; } else { ++m_col; }
        return c;
    }

    [[nodiscard]] Token make(Tok kind) const
    {
        Token t;
        t.kind = kind;
        t.line = m_line;
        t.col = m_col;
        return t;
    }

    bool fail(const std::string& msg)
    {
        m_error = "Zeile " + std::to_string(m_line) + ":" + std::to_string(m_col) + ": " + msg;
        return false;
    }

    void skipWhitespaceAndComments()
    {
        while (!atEnd())
        {
            const char c = peek();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                advance();
            }
            else if (c == '/' && peek(1) == '/')
            {
                while (!atEnd() && peek() != '\n') advance();
            }
            else if (c == '/' && peek(1) == '*')
            {
                advance(); advance();
                while (!atEnd() && !(peek() == '*' && peek(1) == '/')) advance();
                if (!atEnd()) { advance(); advance(); }
            }
            else
            {
                break;
            }
        }
    }

    static bool isHexDigit(char c)
    {
        return std::isxdigit(static_cast<unsigned char>(c)) != 0;
    }

    bool lexNumber(std::vector<Token>& out)
    {
        Token t = make(Tok::Number);

        // 0x... (EEL2 form)
        if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X') && isHexDigit(peek(2)))
        {
            advance(); advance();
            double value = 0.0;
            while (isHexDigit(peek()))
            {
                value = value * 16.0 + hexValue(advance());
            }
            t.number = value;
            out.push_back(t);
            return true;
        }

        // Tentative AVS suffix-hex: hexdigits followed by 'h'
        {
            std::size_t p = m_pos;
            while (p < m_src.size() && isHexDigit(m_src[p])) ++p;
            if (p > m_pos && p < m_src.size() && (m_src[p] == 'h' || m_src[p] == 'H'))
            {
                const char after = (p + 1 < m_src.size()) ? m_src[p + 1] : '\0';
                if (!std::isalnum(static_cast<unsigned char>(after)) && after != '_')
                {
                    double value = 0.0;
                    while (m_pos < p) value = value * 16.0 + hexValue(advance());
                    advance();  // 'h'
                    t.number = value;
                    out.push_back(t);
                    return true;
                }
            }
        }

        // Decimal / scientific
        std::string digits;
        while (std::isdigit(static_cast<unsigned char>(peek()))) digits += advance();
        if (peek() == '.')
        {
            digits += advance();
            while (std::isdigit(static_cast<unsigned char>(peek()))) digits += advance();
        }
        if ((peek() == 'e' || peek() == 'E') &&
            (isDigitAt(1) || ((peek(1) == '+' || peek(1) == '-') && isDigitAt(2))))
        {
            digits += advance();
            if (peek() == '+' || peek() == '-') digits += advance();
            while (std::isdigit(static_cast<unsigned char>(peek()))) digits += advance();
        }
        try
        {
            t.number = std::stod(digits);
        }
        catch (...)
        {
            return fail("ungueltige Zahl '" + digits + "'");
        }
        out.push_back(t);
        return true;
    }

    static double hexValue(char c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        return 10 + (c - 'A');
    }

    void lexIdent(std::vector<Token>& out)
    {
        Token t = make(Tok::Ident);
        while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
        {
            t.text += static_cast<char>(std::tolower(static_cast<unsigned char>(advance())));
        }
        out.push_back(t);
    }

    bool lexDollar(std::vector<Token>& out)
    {
        Token t = make(Tok::Number);
        advance();  // '$'

        const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(peek())));
        if (c == 'x')  // $x1F (MilkDrop hex)
        {
            advance();
            if (!isHexDigit(peek())) return fail("Hex-Ziffern nach $x erwartet");
            double value = 0.0;
            while (isHexDigit(peek())) value = value * 16.0 + hexValue(advance());
            t.number = value;
        }
        else if (peek() == '\'')  // $'c' (char code)
        {
            advance();
            if (atEnd()) return fail("Zeichen nach $' erwartet");
            t.number = static_cast<double>(static_cast<unsigned char>(advance()));
            if (peek() != '\'') return fail("schliessendes ' nach $'c erwartet");
            advance();
        }
        else  // $PI / $E / $PHI
        {
            std::string name;
            while (std::isalpha(static_cast<unsigned char>(peek())))
            {
                name += static_cast<char>(std::tolower(static_cast<unsigned char>(advance())));
            }
            if (name == "pi")       t.number = 3.14159265358979323846;
            else if (name == "e")   t.number = 2.71828183;
            else if (name == "phi") t.number = 1.61803399;
            else return fail("unbekannte $-Konstante '$" + name + "'");
        }
        out.push_back(t);
        return true;
    }

    bool lexOperator(std::vector<Token>& out)
    {
        const char c = peek();
        const char n = peek(1);
        Token t = make(Tok::End);

        auto push2 = [&](Tok kind) { advance(); advance(); t.kind = kind; out.push_back(t); return true; };
        auto push1 = [&](Tok kind) { advance(); t.kind = kind; out.push_back(t); return true; };

        switch (c)
        {
            case '+': return (n == '=') ? push2(Tok::PlusAssign) : push1(Tok::Plus);
            case '-': return (n == '=') ? push2(Tok::MinusAssign) : push1(Tok::Minus);
            case '*': return (n == '=') ? push2(Tok::MulAssign) : push1(Tok::Star);
            case '/': return (n == '=') ? push2(Tok::DivAssign) : push1(Tok::Slash);
            case '%': return (n == '=') ? push2(Tok::ModAssign) : push1(Tok::Percent);
            case '^': return (n == '=') ? push2(Tok::PowAssign) : push1(Tok::Caret);
            case '&': return (n == '&') ? push2(Tok::AndAnd)
                         : (n == '=') ? push2(Tok::AndAssign) : push1(Tok::Amp);
            case '|': return (n == '|') ? push2(Tok::OrOr)
                         : (n == '=') ? push2(Tok::OrAssign) : push1(Tok::Pipe);
            case '=': return (n == '=') ? push2(Tok::EqEq) : push1(Tok::Assign);
            case '!': return (n == '=') ? push2(Tok::NotEq) : push1(Tok::Bang);
            case '<': return (n == '=') ? push2(Tok::Le) : push1(Tok::Lt);
            case '>': return (n == '=') ? push2(Tok::Ge) : push1(Tok::Gt);
            case '(': return push1(Tok::LParen);
            case ')': return push1(Tok::RParen);
            case '[': return push1(Tok::LBracket);
            case ']': return push1(Tok::RBracket);
            case ',': return push1(Tok::Comma);
            case ';': return push1(Tok::Semi);
            case '?': return push1(Tok::Question);
            case ':': return push1(Tok::Colon);
            default:
                return fail(std::string("unerwartetes Zeichen '") + c + "'");
        }
    }

    std::string_view m_src;
    std::size_t m_pos = 0;
    int m_line = 1;
    int m_col = 1;
    std::string m_error;
};

} // namespace lumi::eel::detail
