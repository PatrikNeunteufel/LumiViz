/**
 ****************************************************************************************
 * @file   EelTranspilerCodeGen.hpp
 * @brief  AST -> Lua 5.4 source generation (EEL semantics via eel-Prelude)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.3.0 (S67: Division emittiert eel.div — Nenner 0 => 0,
 *          Referenz-Vertrag per Sonde gegen MilkdropRef gemessen)
 *
 * @details
 * Emits Lua source for the LuaScriptEngine sandbox (Import-Analyse §7.4):
 * - Embedded assignments are HOISTED into preceding statements in evaluation
 *   order; inside lazy constructs (?:, if(), &&, ||, loop, while) hoisting
 *   stays local to the branch (statement form with an env temp __eel_tN).
 * - Numbers are emitted in float form (5 -> 5.0) — no int/float mixing.
 * - EEL semantics map to the eel-Prelude: % -> eel.mod, & | -> eel.bitand/bitor,
 *   sqrt -> eel.sqrt, comparisons -> 1.0/0.0, truthiness via eel.truthy.
 * - Temps are env globals (__eel_tN), not locals — Lua's 200-local limit
 *   cannot be hit by pathological presets.
 * - AVS dialect: warns when two identifiers share their first 8 characters
 *   (the original engine would have aliased them — decision §10.2).
 ****************************************************************************************
 */

#pragma once

#include "EelTranspilerParser.hpp"

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace lumi::eel {

enum class Dialect
{
    Avs,        ///< AVS ns-eel v1 presets (loop cap 4096)
    Milkdrop    ///< MilkDrop ns-eel2 presets (loop cap 1048576)
};

} // namespace lumi::eel

namespace lumi::eel::detail {

class CodeGen
{
public:
    CodeGen(Dialect dialect, std::vector<std::string>& warnings)
        : m_dialect(dialect), m_warnings(warnings)
    {
    }

    /// @brief Generate the Lua chunk for a parsed program
    std::string generate(const Node& program)
    {
        std::vector<std::string> stmts;
        for (const auto& kid : program.kids)
        {
            genStatement(*kid, stmts);
        }
        checkAvsIdentifierAliasing();

        std::string out;
        for (const auto& s : stmts)
        {
            out += s;
            out += '\n';
        }
        return out;
    }

private:
    // =========================================================================
    // Statements
    // =========================================================================

    void genStatement(const Node& node, std::vector<std::string>& out)
    {
        if (node.kind == NodeKind::Assign)
        {
            genAssign(node, out, /*needValue=*/false);
            return;
        }
        if (node.kind == NodeKind::StmtList)
        {
            for (const auto& kid : node.kids) genStatement(*kid, out);
            return;
        }
        const std::string expr = genExpr(node, out);
        if (node.kind == NodeKind::Call)
        {
            // Calls are valid Lua statements — emit directly when possible
            if (!expr.empty() && expr.back() == ')' && expr.rfind("eel.", 0) == 0)
            {
                out.push_back(expr);
                return;
            }
        }
        out.push_back("__eel_discard = " + expr);
    }

    // =========================================================================
    // Expressions (return Lua expr string, hoist side effects into `out`)
    // =========================================================================

    std::string genExpr(const Node& node, std::vector<std::string>& out)
    {
        switch (node.kind)
        {
            case NodeKind::Number:  return formatNumber(node.number);
            case NodeKind::Var:     return mapIdent(node.name);
            case NodeKind::Unary:   return genUnary(node, out);
            case NodeKind::Binary:  return genBinary(node, out);
            case NodeKind::Ternary: return genLazyChoice(*node.kids[0], node.kids[1].get(),
                                                        node.kids[2].get(), out);
            case NodeKind::Assign:  return genAssign(node, out, /*needValue=*/true);
            case NodeKind::Index:   return genIndexRead(node, out);
            case NodeKind::Call:    return genCall(node, out);
            case NodeKind::StmtList:
            {
                for (std::size_t k = 0; k + 1 < node.kids.size(); ++k)
                {
                    genStatement(*node.kids[k], out);
                }
                return genExpr(*node.kids.back(), out);
            }
        }
        return "0.0";
    }

    std::string genUnary(const Node& node, std::vector<std::string>& out)
    {
        const std::string a = genExpr(*node.kids[0], out);
        if (node.op == Tok::Minus) return "(-" + a + ")";
        return "eel.bnot(" + a + ")";  // '!'
    }

    std::string genBinary(const Node& node, std::vector<std::string>& out)
    {
        // Lazy operators first (right side must not be pre-evaluated)
        if (node.op == Tok::AndAnd || node.op == Tok::OrOr)
        {
            return genLazyLogic(node, out);
        }

        const std::string a = genExpr(*node.kids[0], out);
        const std::string b = genExpr(*node.kids[1], out);
        switch (node.op)
        {
            case Tok::Plus:    return "(" + a + " + " + b + ")";
            case Tok::Minus:   return "(" + a + " - " + b + ")";
            case Tok::Star:    return "(" + a + " * " + b + ")";
            // EEL-Division (S67): Nenner 0 => 0 (MD3-Referenz-Vertrag, per
            // Sonde gegen MilkdropRef gemessen: x/0 und 0/0 sind dort exakt 0,
            // kein IEEE-inf/NaN; eel.div lebt im Prelude der Engine).
            // NUR Milkdrop-Dialekt — die AVS-evallib-Division sieht roh aus
            // (kein Schutz-Trailer); deren Vertrag entscheidet erst eine
            // AvsRef-Sonde, bis dahin bleibt AVS bei roher Lua-Division.
            case Tok::Slash:
                return m_dialect == Dialect::Avs
                           ? "(" + a + " / " + b + ")"
                           : "eel.div(" + a + ", " + b + ")";
            case Tok::Caret:   return "(" + a + " ^ " + b + ")";
            case Tok::Percent: return "eel.mod(" + a + ", " + b + ")";
            case Tok::Amp:     return "eel.bitand(" + a + ", " + b + ")";
            case Tok::Pipe:    return "eel.bitor(" + a + ", " + b + ")";
            case Tok::EqEq:    return "eel.equal(" + a + ", " + b + ")";
            case Tok::NotEq:   return "eel.bnot(eel.equal(" + a + ", " + b + "))";
            case Tok::Lt:      return "((" + a + " < " + b + ") and 1.0 or 0.0)";
            case Tok::Gt:      return "((" + a + " > " + b + ") and 1.0 or 0.0)";
            case Tok::Le:      return "((" + a + " <= " + b + ") and 1.0 or 0.0)";
            case Tok::Ge:      return "((" + a + " >= " + b + ") and 1.0 or 0.0)";
            default:           return "0.0";
        }
    }

    /// a && b / a || b — short-circuit, result 1.0/0.0
    std::string genLazyLogic(const Node& node, std::vector<std::string>& out)
    {
        const std::string a = genExpr(*node.kids[0], out);
        if (isPure(*node.kids[1]))
        {
            std::vector<std::string> none;
            const std::string b = genExpr(*node.kids[1], none);
            const std::string bTruth = "(eel.truthy(" + b + ") and 1.0 or 0.0)";
            if (node.op == Tok::AndAnd)
            {
                return "(eel.truthy(" + a + ") and " + bTruth + " or 0.0)";
            }
            return "(eel.truthy(" + a + ") and 1.0 or " + bTruth + ")";
        }

        // Impure right side: statement form
        const std::string temp = newTemp();
        std::vector<std::string> branch;
        const std::string b = genExpr(*node.kids[1], branch);
        branch.push_back(temp + " = (eel.truthy(" + b + ") and 1.0 or 0.0)");

        if (node.op == Tok::AndAnd)
        {
            out.push_back("if eel.truthy(" + a + ") then");
            appendIndented(out, branch);
            out.push_back("else " + temp + " = 0.0 end");
        }
        else
        {
            out.push_back("if eel.truthy(" + a + ") then " + temp + " = 1.0 else");
            appendIndented(out, branch);
            out.push_back("end");
        }
        return temp;
    }

    /// if(c, a, b) und c ? a : b — lazy branch selection
    std::string genLazyChoice(const Node& cond, const Node* a, const Node* b,
                              std::vector<std::string>& out)
    {
        const std::string c = genExpr(cond, out);
        if (a != nullptr && b != nullptr && isPure(*a) && isPure(*b))
        {
            std::vector<std::string> none;
            const std::string ea = genExpr(*a, none);
            const std::string eb = genExpr(*b, none);
            // EEL values are numbers — every number (incl. 0) is truthy in Lua,
            // so the and/or chain cannot mis-select the branch.
            return "(eel.truthy(" + c + ") and (" + ea + ") or (" + eb + "))";
        }

        const std::string temp = newTemp();
        std::vector<std::string> branchA;
        std::vector<std::string> branchB;
        const std::string ea = (a != nullptr) ? genExpr(*a, branchA) : "0.0";
        const std::string eb = (b != nullptr) ? genExpr(*b, branchB) : "0.0";
        branchA.push_back(temp + " = " + ea);
        branchB.push_back(temp + " = " + eb);

        out.push_back("if eel.truthy(" + c + ") then");
        appendIndented(out, branchA);
        out.push_back("else");
        appendIndented(out, branchB);
        out.push_back("end");
        return temp;
    }

    // =========================================================================
    // Assignment (plain and compound; Var / megabuf / index targets)
    // =========================================================================

    std::string genAssign(const Node& node, std::vector<std::string>& out, bool needValue)
    {
        const Node& target = *node.kids[0];
        const Node& valueNode = *node.kids[1];

        if (target.kind == NodeKind::Var)
        {
            const std::string name = mapIdent(target.name);
            std::string value = genExpr(valueNode, out);
            if (node.op != Tok::Assign)
            {
                value = compoundValue(name, node.op, value);
            }
            out.push_back(name + " = " + value);
            return name;  // reading the target after the write == the value
        }

        // Buffer target: megabuf(i) / gmegabuf(i) / base[idx]
        std::string writeFn = "eel.mbwrite";
        std::string indexExpr;
        if (target.kind == NodeKind::Call)
        {
            if (target.name == "gmegabuf") writeFn = "eel.gmbwrite";
            indexExpr = target.kids.empty() ? "0.0" : genExpr(*target.kids[0], out);
        }
        else  // Index
        {
            if (target.kids[0]->kind == NodeKind::Var && target.kids[0]->name == "gmem")
            {
                writeFn = "eel.gmbwrite";
                indexExpr = genExpr(*target.kids[1], out);
            }
            else
            {
                const std::string base = genExpr(*target.kids[0], out);
                const std::string idx = genExpr(*target.kids[1], out);
                indexExpr = "((" + base + ") + (" + idx + "))";
            }
        }

        // Hoist the index into a temp when it is needed twice (compound) or
        // when it is not a trivial expression
        if (node.op != Tok::Assign)
        {
            const std::string idxTemp = newTemp();
            out.push_back(idxTemp + " = " + indexExpr);
            const std::string readFn = (writeFn == "eel.gmbwrite") ? "eel.gmbread" : "eel.mbread";
            std::string value = genExpr(valueNode, out);
            value = compoundValue(readFn + "(" + idxTemp + ")", node.op, value);
            const std::string call = writeFn + "(" + idxTemp + ", " + value + ")";
            if (needValue) return call;   // mbwrite returns the written value
            out.push_back(call);
            return call;
        }

        const std::string value = genExpr(valueNode, out);
        const std::string call = writeFn + "(" + indexExpr + ", " + value + ")";
        if (needValue) return call;
        out.push_back(call);
        return call;
    }

    // seit S67 nicht mehr static: DivAssign braucht den Dialekt (eel.div)
    std::string compoundValue(const std::string& current, Tok op,
                              const std::string& value) const
    {
        switch (op)
        {
            case Tok::PlusAssign:  return "(" + current + " + " + value + ")";
            case Tok::MinusAssign: return "(" + current + " - " + value + ")";
            case Tok::MulAssign:   return "(" + current + " * " + value + ")";
            case Tok::DivAssign:
                return m_dialect == Dialect::Avs
                           ? "(" + current + " / " + value + ")"
                           : "eel.div(" + current + ", " + value + ")";
            case Tok::ModAssign:   return "eel.mod(" + current + ", " + value + ")";
            case Tok::OrAssign:    return "eel.bitor(" + current + ", " + value + ")";
            case Tok::AndAssign:   return "eel.bitand(" + current + ", " + value + ")";
            case Tok::PowAssign:   return "(" + current + " ^ " + value + ")";
            default:               return value;
        }
    }

    // =========================================================================
    // Buffer reads
    // =========================================================================

    std::string genIndexRead(const Node& node, std::vector<std::string>& out)
    {
        if (node.kids[0]->kind == NodeKind::Var && node.kids[0]->name == "gmem")
        {
            return "eel.gmbread(" + genExpr(*node.kids[1], out) + ")";
        }
        const std::string base = genExpr(*node.kids[0], out);
        const std::string idx = genExpr(*node.kids[1], out);
        return "eel.mbread(((" + base + ") + (" + idx + ")))";
    }

    // =========================================================================
    // Function calls (catalog dispatch, Import-Analyse §7.4)
    // =========================================================================

    std::string genCall(const Node& node, std::vector<std::string>& out)
    {
        const std::string& fn = node.name;
        const auto arg = [&](std::size_t k) -> std::string {
            return (k < node.kids.size()) ? genExpr(*node.kids[k], out) : std::string("0.0");
        };

        // --- control flow ---
        if (fn == "if")
        {
            if (node.kids.empty()) return "0.0";
            const Node* a = node.kids.size() > 1 ? node.kids[1].get() : nullptr;
            const Node* b = node.kids.size() > 2 ? node.kids[2].get() : nullptr;
            return genLazyChoice(*node.kids[0], a, b, out);
        }
        if (fn == "exec2" || fn == "exec3")
        {
            for (std::size_t k = 0; k + 1 < node.kids.size(); ++k)
            {
                genStatement(*node.kids[k], out);
            }
            return node.kids.empty() ? "0.0" : genExpr(*node.kids.back(), out);
        }
        if (fn == "loop")
        {
            const std::string count = arg(0);
            const std::string temp = newTemp();
            out.push_back(temp + " = " + count);
            out.push_back("if " + temp + " > " + loopCap() + " then " + temp + " = " + loopCap() + " end");
            std::vector<std::string> body;
            if (node.kids.size() > 1) genStatement(*node.kids[1], body);
            out.push_back("for _ = 1, " + temp + " do");
            appendIndented(out, body);
            out.push_back("end");
            return "0.0";
        }
        if (fn == "while")
        {
            std::vector<std::string> body;
            const std::string cond = node.kids.empty() ? "0.0" : genExpr(*node.kids[0], body);
            out.push_back("for _ = 1, " + loopCap() + " do");
            appendIndented(out, body);
            out.push_back("  if not eel.truthy(" + cond + ") then break end");
            out.push_back("end");
            return "0.0";
        }
        if (fn == "assign")
        {
            if (node.kids.size() == 2 && isLvalueNode(*node.kids[0]))
            {
                return genAssignFromParts(*node.kids[0], *node.kids[1], out);
            }
            warn(node.line, "assign(): Ziel ist kein L-Wert — Wert wird nur ausgewertet");
            return arg(1);
        }

        // --- buffers ---
        if (fn == "megabuf")  return "eel.mbread(" + arg(0) + ")";
        if (fn == "gmegabuf") return "eel.gmbread(" + arg(0) + ")";

        // --- eel prelude semantics ---
        if (fn == "sqrt")    return "eel.sqrt(" + arg(0) + ")";
        if (fn == "invsqrt") return "eel.invsqrt(" + arg(0) + ")";
        if (fn == "sigmoid") return "eel.sigmoid(" + arg(0) + ", " + arg(1) + ")";
        if (fn == "sign")    return "eel.sign(" + arg(0) + ")";
        if (fn == "equal")   return "eel.equal(" + arg(0) + ", " + arg(1) + ")";
        if (fn == "above")   return "eel.above(" + arg(0) + ", " + arg(1) + ")";
        if (fn == "below")   return "eel.below(" + arg(0) + ", " + arg(1) + ")";
        if (fn == "band")    return "eel.band(" + arg(0) + ", " + arg(1) + ")";
        if (fn == "bor")     return "eel.bor(" + arg(0) + ", " + arg(1) + ")";
        if (fn == "bnot")    return "eel.bnot(" + arg(0) + ")";
        if (fn == "pow")     return "((" + arg(0) + ") ^ (" + arg(1) + "))";
        if (fn == "sqr")
        {
            if (isPure(*node.kids[0]))
            {
                const std::string a = arg(0);
                return "((" + a + ") * (" + a + "))";
            }
            const std::string temp = newTemp();
            out.push_back(temp + " = " + arg(0));
            return "(" + temp + " * " + temp + ")";
        }
        if (fn == "log10")   return "log(" + arg(0) + ", 10.0)";
        // MilkDrop ns-eel2: "int" ist ein reiner Alias fuer floor (nseel-eval.c:284) —
        // KEINE Rundung, KEIN Abschneiden Richtung 0
        if (fn == "int")     return "floor(" + arg(0) + ")";

        // --- direct pass-through (sandbox env: math subset + host functions) ---
        static const std::set<std::string> kDirect = {
            "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "floor", "ceil",
            "exp", "log", "abs", "min", "max", "rand", "mod",  // mod = env-fmod
            "getosc", "getspec", "gettime",
            // getspecdb (S67): getspec in WebAudio-dB-Skala (LumiViz-Extra,
            // kein AVS-/MilkDrop-Builtin — Skala wie die Shadertoy-Audio-Textur)
            "getspecdb"
        };
        if (kDirect.count(fn) > 0)
        {
            std::string call = fn + "(";
            for (std::size_t k = 0; k < node.kids.size(); ++k)
            {
                if (k > 0) call += ", ";
                call += genExpr(*node.kids[k], out);
            }
            call += ")";
            return call;
        }

        // --- known no-ops (decision: Maus-Funktionen als Stubs) ---
        if (fn == "getkbmouse" || fn == "setmousepos" ||
            fn == "freembuf" || fn == "memcpy" || fn == "memset")
        {
            warn(node.line, "'" + fn + "' wird nicht unterstuetzt — ersetzt durch 0.0");
            for (const auto& kid : node.kids) genStatement(*kid, out);  // keep side effects
            return "0.0";
        }

        // --- unknown: warn + pass through (host may provide it) ---
        warn(node.line, "unbekannte Funktion '" + fn + "' — wird durchgereicht");
        std::string call = mapIdent(fn) + "(";
        for (std::size_t k = 0; k < node.kids.size(); ++k)
        {
            if (k > 0) call += ", ";
            call += genExpr(*node.kids[k], out);
        }
        call += ")";
        return call;
    }

    static bool isLvalueNode(const Node& n)
    {
        if (n.kind == NodeKind::Var || n.kind == NodeKind::Index) return true;
        return n.kind == NodeKind::Call && (n.name == "megabuf" || n.name == "gmegabuf");
    }

    /// assign(dest, value) — direct emission (dest already validated as lvalue)
    std::string genAssignFromParts(const Node& target, const Node& valueNode,
                                   std::vector<std::string>& out)
    {
        if (target.kind == NodeKind::Var)
        {
            const std::string name = mapIdent(target.name);
            const std::string value = genExpr(valueNode, out);
            out.push_back(name + " = " + value);
            return name;
        }
        std::string writeFn = "eel.mbwrite";
        std::string indexExpr = "0.0";
        if (target.kind == NodeKind::Call)
        {
            if (target.name == "gmegabuf") writeFn = "eel.gmbwrite";
            if (!target.kids.empty()) indexExpr = genExpr(*target.kids[0], out);
        }
        else if (target.kind == NodeKind::Index)
        {
            if (target.kids[0]->kind == NodeKind::Var && target.kids[0]->name == "gmem")
            {
                writeFn = "eel.gmbwrite";
                indexExpr = genExpr(*target.kids[1], out);
            }
            else
            {
                const std::string base = genExpr(*target.kids[0], out);
                const std::string idx = genExpr(*target.kids[1], out);
                indexExpr = "((" + base + ") + (" + idx + "))";
            }
        }
        const std::string value = genExpr(valueNode, out);
        return writeFn + "(" + indexExpr + ", " + value + ")";
    }

    // =========================================================================
    // Purity analysis (may an expression be evaluated lazily in-place?)
    // =========================================================================

    static bool isPure(const Node& node)
    {
        switch (node.kind)
        {
            case NodeKind::Assign:
                return false;
            case NodeKind::StmtList:
                if (node.kids.size() > 1) return false;
                break;
            case NodeKind::Call:
            {
                static const std::set<std::string> kPureFns = {
                    "sin", "cos", "tan", "asin", "acos", "atan", "atan2", "floor",
                    "ceil", "exp", "log", "log10", "abs", "min", "max", "sqrt",
                    "invsqrt", "sigmoid", "sign", "sqr", "pow", "equal", "above",
                    "below", "band", "bor", "bnot", "megabuf", "gmegabuf",
                    "getosc", "getspec", "getspecdb", "gettime", "if", "exec2",
                    "exec3"
                };
                if (kPureFns.count(node.name) == 0) return false;  // rand, loop, unknown...
                break;
            }
            default:
                break;
        }
        for (const auto& kid : node.kids)
        {
            if (!isPure(*kid)) return false;
        }
        return true;
    }

    // =========================================================================
    // Helpers
    // =========================================================================

    std::string newTemp() { return "__eel_t" + std::to_string(++m_tempCounter); }

    [[nodiscard]] std::string loopCap() const
    {
        return (m_dialect == Dialect::Avs) ? "4096" : "1048576";
    }

    static void appendIndented(std::vector<std::string>& out, const std::vector<std::string>& lines)
    {
        for (const auto& line : lines) out.push_back("  " + line);
    }

    static std::string formatNumber(double value)
    {
        char buf[40];
        std::snprintf(buf, sizeof(buf), "%.17g", value);
        std::string s = buf;
        if (s.find_first_of(".eEnN") == std::string::npos) s += ".0";
        return s;
    }

    std::string mapIdent(const std::string& name)
    {
        m_identifiers.insert(name);
        static const std::set<std::string> kLuaReserved = {
            "and", "break", "do", "else", "elseif", "end", "false", "for",
            "function", "goto", "if", "in", "local", "nil", "not", "or",
            "repeat", "return", "then", "true", "until", "while"
        };
        if (kLuaReserved.count(name) > 0) return name + "_";
        return name;
    }

    void warn(int line, const std::string& msg)
    {
        m_warnings.push_back("Zeile " + std::to_string(line) + ": " + msg);
    }

    /// AVS: nur die ersten 8 Zeichen waren signifikant — Kollisionen melden (§10.2)
    void checkAvsIdentifierAliasing()
    {
        if (m_dialect != Dialect::Avs) return;
        std::map<std::string, std::set<std::string>> byPrefix;
        for (const auto& name : m_identifiers)
        {
            byPrefix[name.substr(0, 8)].insert(name);
        }
        for (const auto& [prefix, names] : byPrefix)
        {
            if (names.size() > 1)
            {
                std::string list;
                for (const auto& n : names)
                {
                    if (!list.empty()) list += ", ";
                    list += n;
                }
                m_warnings.push_back(
                    "AVS-8-Zeichen-Aliasing moeglich (Praefix '" + prefix + "'): " + list +
                    " — Original haette dieselbe Variable verwendet, Import trennt sie");
            }
        }
    }

    Dialect m_dialect;
    std::vector<std::string>& m_warnings;
    int m_tempCounter = 0;
    std::set<std::string> m_identifiers;
};

} // namespace lumi::eel::detail
