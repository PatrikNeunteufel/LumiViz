/**
 ****************************************************************************************
 * @file   EelTranspilerParser.hpp
 * @brief  AST and Pratt parser for the EEL superset
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Parses the EEL superset (AVS v1 + MilkDrop infix operators) into a small AST.
 * Precedence (low -> high), conventional C-like layering:
 *   = (right) < ?: < || < && < == != < relational < | < & < + - < * / % < ^ (right)
 *   < unary (- ! +)
 * Deviations from the historic engines (documented leniency, Import-Analyse §7.3):
 * - Assignments parse as expressions in BOTH dialects (AVS originals never
 *   contain nested assignments, so this is a superset).
 * - `;` inside parentheses is allowed everywhere (MilkDrop preprocessor behavior);
 *   the value of a parenthesized statement list is its last expression.
 ****************************************************************************************
 */

#pragma once

#include "EelTranspilerLexer.hpp"

#include <memory>
#include <string>
#include <vector>

namespace lumi::eel::detail {

enum class NodeKind
{
    Number,     ///< number
    Var,        ///< name
    Call,       ///< name(kids...)
    Index,      ///< kids[0] [ kids[1] ]  (megabuf addressing)
    Unary,      ///< op kids[0]
    Binary,     ///< kids[0] op kids[1]
    Ternary,    ///< kids[0] ? kids[1] : kids[2]
    Assign,     ///< target=kids[0], value=kids[1], op = Assign/PlusAssign/...
    StmtList    ///< kids... ; value = last kid
};

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Node
{
    NodeKind kind = NodeKind::Number;
    double number = 0.0;
    std::string name;           ///< Var/Call name (lowercased)
    Tok op = Tok::End;          ///< Unary/Binary/Assign operator
    std::vector<NodePtr> kids;
    int line = 1;

    static NodePtr make(NodeKind k)
    {
        auto n = std::make_unique<Node>();
        n->kind = k;
        return n;
    }
};

class Parser
{
public:
    explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

    /// @brief Parse a whole program (statement list). nullptr on error (see error()).
    NodePtr parseProgram()
    {
        auto list = Node::make(NodeKind::StmtList);
        list->line = peek().line;

        while (peek().kind != Tok::End)
        {
            if (peek().kind == Tok::Semi)  // empty statement
            {
                advance();
                continue;
            }
            auto expr = parseExpr(0);
            if (expr == nullptr) return nullptr;
            list->kids.push_back(std::move(expr));

            if (peek().kind == Tok::Semi)
            {
                advance();
            }
            else if (peek().kind != Tok::End)
            {
                fail("';' oder Ende erwartet");
                return nullptr;
            }
        }
        return list;
    }

    [[nodiscard]] const std::string& error() const { return m_error; }

private:
    // Binding powers (higher = tighter). 0 is the entry level.
    static int leftBindingPower(Tok t)
    {
        switch (t)
        {
            case Tok::Assign: case Tok::PlusAssign: case Tok::MinusAssign:
            case Tok::MulAssign: case Tok::DivAssign: case Tok::ModAssign:
            case Tok::OrAssign: case Tok::AndAssign: case Tok::PowAssign:
                return 10;
            case Tok::Question: return 20;
            case Tok::OrOr:     return 30;
            case Tok::AndAnd:   return 40;
            case Tok::EqEq: case Tok::NotEq: return 50;
            case Tok::Lt: case Tok::Gt: case Tok::Le: case Tok::Ge: return 60;
            case Tok::Pipe:     return 70;
            case Tok::Amp:      return 80;
            case Tok::Plus: case Tok::Minus: return 90;
            case Tok::Star: case Tok::Slash: case Tok::Percent: return 100;
            case Tok::Caret:    return 110;
            default:            return -1;  // not an infix operator
        }
    }

    NodePtr parseExpr(int minBp)
    {
        auto left = parseUnary();
        if (left == nullptr) return nullptr;

        while (true)
        {
            const Tok t = peek().kind;
            const int lbp = leftBindingPower(t);
            if (lbp < 0 || lbp < minBp) break;

            if (t == Tok::Question)  // ternary c ? a : b (right-assoc)
            {
                advance();
                auto a = parseExpr(0);
                if (a == nullptr) return nullptr;
                if (peek().kind != Tok::Colon) { fail("':' im ?:-Ausdruck erwartet"); return nullptr; }
                advance();
                auto b = parseExpr(lbp);  // right-assoc: same bp
                if (b == nullptr) return nullptr;
                auto node = Node::make(NodeKind::Ternary);
                node->line = left->line;
                node->kids.push_back(std::move(left));
                node->kids.push_back(std::move(a));
                node->kids.push_back(std::move(b));
                left = std::move(node);
                continue;
            }

            if (isAssignOp(t))  // right-assoc, target must be lvalue
            {
                if (!isLvalue(*left)) { fail("Zuweisungsziel ist kein L-Wert"); return nullptr; }
                advance();
                auto value = parseExpr(lbp);  // right-assoc
                if (value == nullptr) return nullptr;
                auto node = Node::make(NodeKind::Assign);
                node->op = t;
                node->line = left->line;
                node->kids.push_back(std::move(left));   // [0] target
                node->kids.push_back(std::move(value));  // [1] value
                left = std::move(node);
                continue;
            }

            // Ordinary binary operator. ^ is right-assoc; the rest left-assoc.
            advance();
            const int nextBp = (t == Tok::Caret) ? lbp : lbp + 1;
            auto right = parseExpr(nextBp);
            if (right == nullptr) return nullptr;
            auto node = Node::make(NodeKind::Binary);
            node->op = t;
            node->line = left->line;
            node->kids.push_back(std::move(left));
            node->kids.push_back(std::move(right));
            left = std::move(node);
        }
        return left;
    }

    static bool isAssignOp(Tok t)
    {
        switch (t)
        {
            case Tok::Assign: case Tok::PlusAssign: case Tok::MinusAssign:
            case Tok::MulAssign: case Tok::DivAssign: case Tok::ModAssign:
            case Tok::OrAssign: case Tok::AndAssign: case Tok::PowAssign:
                return true;
            default:
                return false;
        }
    }

    static bool isLvalue(const Node& n)
    {
        if (n.kind == NodeKind::Var) return true;
        if (n.kind == NodeKind::Index) return true;
        if (n.kind == NodeKind::Call && (n.name == "megabuf" || n.name == "gmegabuf")) return true;
        return false;
    }

    NodePtr parseUnary()
    {
        const Tok t = peek().kind;
        if (t == Tok::Minus || t == Tok::Bang || t == Tok::Plus)
        {
            const int line = peek().line;
            advance();
            auto operand = parseUnary();
            if (operand == nullptr) return nullptr;
            if (t == Tok::Plus) return operand;  // unary + is a no-op
            auto node = Node::make(NodeKind::Unary);
            node->op = t;
            node->line = line;
            node->kids.push_back(std::move(operand));
            return node;
        }
        return parsePostfix();
    }

    NodePtr parsePostfix()
    {
        auto base = parsePrimary();
        if (base == nullptr) return nullptr;

        while (peek().kind == Tok::LBracket)  // x[i] / gmem[i]
        {
            advance();
            auto idx = parseExpr(0);
            if (idx == nullptr) return nullptr;
            if (peek().kind != Tok::RBracket) { fail("']' erwartet"); return nullptr; }
            advance();
            auto node = Node::make(NodeKind::Index);
            node->line = base->line;
            node->kids.push_back(std::move(base));
            node->kids.push_back(std::move(idx));
            base = std::move(node);
        }
        return base;
    }

    NodePtr parsePrimary()
    {
        const Token& t = peek();

        if (t.kind == Tok::Number)
        {
            auto node = Node::make(NodeKind::Number);
            node->number = t.number;
            node->line = t.line;
            advance();
            return node;
        }

        if (t.kind == Tok::Ident)
        {
            std::string name = t.text;
            const int line = t.line;
            advance();
            if (peek().kind == Tok::LParen)  // function call
            {
                advance();
                auto node = Node::make(NodeKind::Call);
                node->name = std::move(name);
                node->line = line;
                if (peek().kind != Tok::RParen)
                {
                    while (true)
                    {
                        auto arg = parseExpr(0);
                        if (arg == nullptr) return nullptr;
                        node->kids.push_back(std::move(arg));
                        if (peek().kind == Tok::Comma) { advance(); continue; }
                        break;
                    }
                }
                if (peek().kind != Tok::RParen) { fail("')' im Funktionsaufruf erwartet"); return nullptr; }
                advance();
                return node;
            }
            auto node = Node::make(NodeKind::Var);
            node->name = std::move(name);
            node->line = line;
            return node;
        }

        if (t.kind == Tok::LParen)  // parenthesized expr or statement list
        {
            const int line = t.line;
            advance();
            auto list = Node::make(NodeKind::StmtList);
            list->line = line;
            while (true)
            {
                if (peek().kind == Tok::Semi) { advance(); continue; }
                if (peek().kind == Tok::RParen) break;
                auto expr = parseExpr(0);
                if (expr == nullptr) return nullptr;
                list->kids.push_back(std::move(expr));
                if (peek().kind == Tok::Semi) { advance(); continue; }
                break;
            }
            if (peek().kind != Tok::RParen) { fail("')' erwartet"); return nullptr; }
            advance();
            if (list->kids.empty()) { fail("leerer Klammerausdruck"); return nullptr; }
            if (list->kids.size() == 1)  // plain parenthesized expression
            {
                return std::move(list->kids.front());
            }
            return list;
        }

        fail("Ausdruck erwartet");
        return nullptr;
    }

    const Token& peek() const { return m_tokens[m_index]; }
    void advance() { if (m_index + 1 < m_tokens.size()) ++m_index; }

    void fail(const std::string& msg)
    {
        if (m_error.empty())
        {
            m_error = "Zeile " + std::to_string(peek().line) + ": " + msg;
        }
    }

    std::vector<Token> m_tokens;
    std::size_t m_index = 0;
    std::string m_error;
};

} // namespace lumi::eel::detail
