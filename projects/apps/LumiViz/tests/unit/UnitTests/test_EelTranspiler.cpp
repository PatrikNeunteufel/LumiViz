/**
 ****************************************************************************************
 * @file   test_EelTranspiler.cpp
 * @brief  Golden-Tests fuer den EEL->Lua-Transpiler (Import-Phase Roadmap 2):
 *         Verhalten wird END-TO-END geprueft — EEL-Quelle -> transpile() ->
 *         Ausfuehrung in der LuaScriptEngine-Sandbox -> Zahlenvergleich gegen
 *         die EEL-Semantik aus Import_Analyse_AVS_MilkDrop.md §7.2
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include <EelTranspiler.hpp>

#include "scripting/LuaScriptEngine.hpp"

#include <cmath>
#include <string>

using lumi::eel::Dialect;
using lumi::eel::TranspileResult;
using lumi::eel::transpile;
using lumi::scripting::LuaScriptEngine;
using Slot = LuaScriptEngine::Slot;

namespace
{

/// Transpiliert EEL, fuehrt das Ergebnis in der Sandbox aus, liefert die Engine
/// fuer Variablen-Checks. REQUIREs bei Transpile-/Laufzeitfehlern.
class EelRunner
{
public:
    explicit EelRunner(const std::string& eelSource, Dialect dialect = Dialect::Avs)
    {
        m_result = transpile(eelSource, dialect);
        CAPTURE(eelSource);
        CAPTURE(m_result.error);
        REQUIRE(m_result.ok);
        CAPTURE(m_result.lua);
        REQUIRE(m_engine.compile(Slot::Frame, m_result.lua, "golden"));
    }

    void run()
    {
        CAPTURE(m_result.lua);
        CAPTURE(m_engine.lastError());
        REQUIRE(m_engine.run(Slot::Frame));
    }

    [[nodiscard]] double var(const char* name) const { return m_engine.number(name); }
    void setVar(const char* name, double v) { m_engine.setNumber(name, v); }
    [[nodiscard]] LuaScriptEngine& engine() { return m_engine; }
    [[nodiscard]] const TranspileResult& result() const { return m_result; }

private:
    TranspileResult m_result;
    LuaScriptEngine m_engine;
};

/// Kurzform: einzelnes Skript ausfuehren und eine Variable lesen
double evalEel(const std::string& src, const char* resultVar = "r",
               Dialect dialect = Dialect::Avs)
{
    EelRunner runner(src, dialect);
    runner.run();
    return runner.var(resultVar);
}

} // namespace

// =============================================================================
// Grundlagen
// =============================================================================

TEST_CASE("EelTranspiler: Superscope-Grundform x=i*2-1")
{
    EelRunner runner("x=i*2-1; y=v*0.8");
    runner.setVar("i", 0.5);
    runner.setVar("v", 1.0);
    runner.run();
    CHECK(runner.var("x") == doctest::Approx(0.0));
    CHECK(runner.var("y") == doctest::Approx(0.8));
}

TEST_CASE("EelTranspiler: leere Quelle -> ok, leeres Lua")
{
    const auto result = transpile("   \n// nur Kommentar\n", Dialect::Avs);
    CHECK(result.ok);
    CHECK(result.lua.empty());
}

TEST_CASE("EelTranspiler: Nicht-ASCII-Bytes werden wie im Original ignoriert")
{
    // S46/Anemone: UnConeD signiert Skripte mit ';\xA9;' (Copyright-Zeichen) —
    // das AVS-EEL schluckt solche Bytes; der Lexer-Abbruch toetete vorher den
    // ganzen Slot (Preset renderte schwarz).
    CHECK(evalEel("x = 1;\xA9;r = x + 1") == doctest::Approx(2.0));
}

TEST_CASE("EelTranspiler: Syntaxfehler -> ok=false mit Positionsangabe")
{
    const auto result = transpile("x = = 2", Dialect::Avs);
    CHECK_FALSE(result.ok);
    CHECK(result.error.find("Zeile") != std::string::npos);
}

TEST_CASE("EelTranspiler: Praezedenz und Potenz")
{
    CHECK(evalEel("r = 2 + 3 * 4") == doctest::Approx(14.0));
    CHECK(evalEel("r = (2 + 3) * 4") == doctest::Approx(20.0));
    // ^ rechtsassoziativ: 2^(3^2) = 512 (Milkdrop-Dialekt-Operator)
    CHECK(evalEel("r = 2 ^ 3 ^ 2", "r", Dialect::Milkdrop) == doctest::Approx(512.0));
    // unaeres Minus bindet staerker als ^ (EEL2): -2^2 = (-2)^2 = 4
    CHECK(evalEel("r = -2 ^ 2", "r", Dialect::Milkdrop) == doctest::Approx(4.0));
}

TEST_CASE("EelTranspiler: Zahlformate — Hex (3Bh, $x, 0x), $PI, Kommentare")
{
    CHECK(evalEel("r = 3Bh") == doctest::Approx(59.0));
    CHECK(evalEel("r = $x1F", "r", Dialect::Milkdrop) == doctest::Approx(31.0));
    CHECK(evalEel("r = 0x10") == doctest::Approx(16.0));
    CHECK(evalEel("r = $PI") == doctest::Approx(3.14159265358979));
    CHECK(evalEel("// Kommentar\nr = 1 /* mitten */ + 2") == doctest::Approx(3.0));
}

TEST_CASE("EelTranspiler: Case-Insensitivitaet")
{
    EelRunner runner("R = SIN(0) + X");
    runner.setVar("x", 5.0);
    runner.run();
    CHECK(runner.var("r") == doctest::Approx(5.0));
}

TEST_CASE("EelTranspiler: Lua-Reserviertwort als EEL-Variable")
{
    // 'end' ist in Lua reserviert -> wird auf 'end_' gemappt, Semantik bleibt
    CHECK(evalEel("end = 5; r = end + 1") == doctest::Approx(6.0));
}

// =============================================================================
// EEL-Semantik-Goldens (§7.2)
// =============================================================================

TEST_CASE("EelTranspiler: %-Operator ist Integer-Modulo mit EEL-Regeln")
{
    CHECK(evalEel("r = 7 % 3") == doctest::Approx(1.0));
    CHECK(evalEel("r = 7.4 % 3.2") == doctest::Approx(1.0));  // round -> 7 % 3
    // nseel_asm_mod rechnet den Rest UNSIGNED: int32(-7) wickelt ueber 2^32,
    // 4294967289 % 3 = 0 — per AvsRef-Probe belegt (S48; die fruehere
    // |Rest|-Annahme aus S44 war falsch).
    CHECK(evalEel("r = -7 % 3") == doctest::Approx(0.0));
    CHECK(evalEel("r = 5 % 0") == doctest::Approx(0.0));      // Divisor -> max(,1)
}

TEST_CASE("EelTranspiler: & | sind Bit-Operationen auf gerundeten Ints")
{
    CHECK(evalEel("r = 6 & 3") == doctest::Approx(2.0));
    CHECK(evalEel("r = 5 | 2") == doctest::Approx(7.0));
    CHECK(evalEel("r = 6.4 & 3") == doctest::Approx(2.0));
}

TEST_CASE("EelTranspiler: sqrt(|x|), sign, sigmoid, pow")
{
    CHECK(evalEel("r = sqrt(-4)") == doctest::Approx(2.0));
    CHECK(evalEel("r = sign(-3.2)") == doctest::Approx(-1.0));
    CHECK(evalEel("r = sigmoid(0, 1)") == doctest::Approx(0.5));
    CHECK(evalEel("r = pow(2, 10)") == doctest::Approx(1024.0));
    CHECK(evalEel("r = sqr(3)") == doctest::Approx(9.0));
}

TEST_CASE("EelTranspiler: Epsilon-Vergleiche (equal/band/bnot, 1e-5)")
{
    CHECK(evalEel("r = equal(1, 1.000001)") == doctest::Approx(1.0));
    CHECK(evalEel("r = equal(1, 1.00002)") == doctest::Approx(0.0));
    CHECK(evalEel("r = band(0.000001, 5)") == doctest::Approx(0.0));
    CHECK(evalEel("r = bor(0, 0.5)") == doctest::Approx(1.0));
    CHECK(evalEel("r = bnot(0.000005)") == doctest::Approx(1.0));
    CHECK(evalEel("r = above(2, 1) + below(2, 1)") == doctest::Approx(1.0));
}

TEST_CASE("EelTranspiler: if() ist lazy — nur der gewaehlte Zweig laeuft")
{
    // a darf NICHT geschrieben werden (Bedingung falsch), b muss 7 werden
    EelRunner runner("if(0, assign(a, 5), assign(b, 7)); r = a*10 + b");
    runner.run();
    CHECK(runner.var("a") == doctest::Approx(0.0));
    CHECK(runner.var("b") == doctest::Approx(7.0));
    CHECK(runner.var("r") == doctest::Approx(7.0));
}

TEST_CASE("EelTranspiler: if() als Ausdruck (pure Zweige)")
{
    EelRunner runner("r = if(above(x, 0), 1, 2)");
    runner.setVar("x", 5.0);
    runner.run();
    CHECK(runner.var("r") == doctest::Approx(1.0));
}

TEST_CASE("EelTranspiler: exec2/exec3 und assign()")
{
    EelRunner runner("r = exec2(assign(a, 1), 5)");
    runner.run();
    CHECK(runner.var("a") == doctest::Approx(1.0));
    CHECK(runner.var("r") == doctest::Approx(5.0));
    CHECK(evalEel("r = exec3(assign(a,1), assign(b,2), 9)") == doctest::Approx(9.0));
}

TEST_CASE("EelTranspiler: loop() mit AVS-Cap 4096")
{
    CHECK(evalEel("n = 0; loop(5, n = n + 1); r = n") == doctest::Approx(5.0));
    CHECK(evalEel("n = 0; loop(100000, n = n + 1); r = n") == doctest::Approx(4096.0));
}

TEST_CASE("EelTranspiler: megabuf — Funktions- und Klammersyntax")
{
    CHECK(evalEel("assign(megabuf(5), 2.5); r = megabuf(5)") == doctest::Approx(2.5));
    CHECK(evalEel("megabuf(10) = 3; r = megabuf(10)") == doctest::Approx(3.0));
    // Milkdrop: buf[i] -> megabuf((buf)+(i)); gmem[i] -> gmegabuf(i)
    CHECK(evalEel("buf[3] = 7; r = buf[3]", "r", Dialect::Milkdrop) == doctest::Approx(7.0));
    CHECK(evalEel("gmem[2] = 9; r = gmem[2]", "r", Dialect::Milkdrop) == doctest::Approx(9.0));
}

// =============================================================================
// MilkDrop-Dialekt: Infix-Operatoren, Zuweisung als Expression
// =============================================================================

TEST_CASE("EelTranspiler: MilkDrop-Vergleiche/Logik/Ternary")
{
    CHECK(evalEel("r = 3 > 2 ? 10 : 20", "r", Dialect::Milkdrop) == doctest::Approx(10.0));
    CHECK(evalEel("r = (3 > 2) && (3 < 5)", "r", Dialect::Milkdrop) == doctest::Approx(1.0));
    CHECK(evalEel("r = (1 > 2) || (3 == 3)", "r", Dialect::Milkdrop) == doctest::Approx(1.0));
    CHECK(evalEel("r = 1 != 1.000001", "r", Dialect::Milkdrop) == doctest::Approx(0.0));
    CHECK(evalEel("r = !0", "r", Dialect::Milkdrop) == doctest::Approx(1.0));
}

TEST_CASE("EelTranspiler: && ist lazy (rechte Seite mit Nebenwirkung)")
{
    EelRunner runner("r = (0) && (a = 5); s = (1) && (b = 3)", Dialect::Milkdrop);
    runner.run();
    CHECK(runner.var("a") == doctest::Approx(0.0));  // nie ausgefuehrt
    CHECK(runner.var("b") == doctest::Approx(3.0));
    CHECK(runner.var("r") == doctest::Approx(0.0));
    CHECK(runner.var("s") == doctest::Approx(1.0));
}

TEST_CASE("EelTranspiler: verschachtelte Zuweisung wird gehoisted")
{
    EelRunner runner("x = (y = 5) * 2", Dialect::Milkdrop);
    runner.run();
    CHECK(runner.var("y") == doctest::Approx(5.0));
    CHECK(runner.var("x") == doctest::Approx(10.0));
}

TEST_CASE("EelTranspiler: Kompound-Zuweisungen")
{
    CHECK(evalEel("y = 1; y += 2; r = y", "r", Dialect::Milkdrop) == doctest::Approx(3.0));
    CHECK(evalEel("y = 8; y /= 2; r = y", "r", Dialect::Milkdrop) == doctest::Approx(4.0));
    CHECK(evalEel("y = 7; y %= 3; r = y", "r", Dialect::Milkdrop) == doctest::Approx(1.0));
}

TEST_CASE("EelTranspiler: Klammer-Statement-Liste hat den Wert des letzten Ausdrucks")
{
    CHECK(evalEel("r = (a = 1; b = 2; a + b)", "r", Dialect::Milkdrop) == doctest::Approx(3.0));
}

// =============================================================================
// Warnungen und Fehlerphilosophie
// =============================================================================

TEST_CASE("EelTranspiler: AVS-8-Zeichen-Aliasing erzeugt Warnung (§10.2)")
{
    // 'counterval1'/'counterval2' teilen die ersten 8 Zeichen ('counterv') —
    // die AVS-Engine haette sie als DIESELBE Variable behandelt
    const auto result = transpile("counterval1 = 1; counterval2 = 2", Dialect::Avs);
    REQUIRE(result.ok);
    bool found = false;
    for (const auto& w : result.warnings)
    {
        if (w.find("counterval1") != std::string::npos &&
            w.find("counterval2") != std::string::npos)
        {
            found = true;
        }
    }
    CHECK(found);

    // Kein Fehlalarm: Unterschied innerhalb der ersten 8 Zeichen
    const auto clean = transpile("counter1 = 1; counter2 = 2", Dialect::Avs);
    REQUIRE(clean.ok);
    CHECK(clean.warnings.empty());

    // Milkdrop (16 Zeichen signifikant): keine 8-Zeichen-Warnung
    const auto md = transpile("counterval1 = 1; counterval2 = 2", Dialect::Milkdrop);
    REQUIRE(md.ok);
    CHECK(md.warnings.empty());
}

TEST_CASE("EelTranspiler: unbekannte Funktion -> Warnung, kein Abbruch")
{
    const auto result = transpile("r = frobnicate(1)", Dialect::Avs);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.warnings.empty());
    CHECK(result.warnings.front().find("frobnicate") != std::string::npos);
}

TEST_CASE("EelTranspiler: getkbmouse wird zum 0.0-Stub mit Warnung")
{
    const auto result = transpile("r = getkbmouse(1)", Dialect::Avs);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.warnings.empty());
    // ausfuehrbar ohne Host-Funktion:
    LuaScriptEngine engine;
    REQUIRE(engine.compile(Slot::Frame, result.lua, "stub"));
    REQUIRE(engine.run(Slot::Frame));
    CHECK(engine.number("r") == doctest::Approx(0.0));
}

// =============================================================================
// Realwelt-Form: AVS-Superscope-Punkt-Skript End-to-End
// =============================================================================

TEST_CASE("EelTranspiler: Spiral-Punkt-Skript (AVS-Stil) rechnet korrekt")
{
    EelRunner runner("r=0.1+i*0.7+v*0.15; a=i*$PI*8+t; x=sin(a)*r; y=cos(a)*r");
    runner.setVar("i", 0.25);
    runner.setVar("v", 0.4);
    runner.setVar("t", 1.5);
    runner.run();

    const double expectedR = 0.1 + 0.25 * 0.7 + 0.4 * 0.15;
    const double expectedA = 0.25 * 3.14159265358979323846 * 8.0 + 1.5;
    CHECK(runner.var("x") == doctest::Approx(std::sin(expectedA) * expectedR));
    CHECK(runner.var("y") == doctest::Approx(std::cos(expectedA) * expectedR));
}

// =============================================================================
// AVS-Grammatik: Zuweisung als Teilausdruck (S74)
//
// In AVS-EEL ist eine Zuweisung nur als eigenstaendige Anweisung erlaubt. Am
// laufenden Paar gemessen: `AvsRef` uebersetzt ein Skript mit einer Zuweisung
// im Argument GAR NICHT, der Effekt bleibt dort unsichtbar. Wir uebersetzen es
// und zeichnen — das ist ein echter Verhaltensunterschied und muss im
// Import-Bericht stehen.
//
// Fuer MilkDrop gilt die Einschraenkung NICHT (ns-eel2 kennt `_set` als
// Operator) — ebenfalls gemessen. Die Warnung darf dort nicht erscheinen.
// =============================================================================

namespace {

/// Enthaelt eine der Warnungen den Hinweis auf die Zuweisungs-Regel?
bool hatZuweisungsWarnung(const std::vector<std::string>& warnungen)
{
    for (const std::string& w : warnungen)
    {
        if (w.find("Zuweisung innerhalb eines Ausdrucks") != std::string::npos) return true;
    }
    return false;
}

}  // namespace

TEST_CASE("EelTranspiler: Zuweisung im Funktionsargument wird gemeldet (AVS)")
{
    const auto result = transpile("q=0; r=exec2(q=3, q+1)", Dialect::Avs);
    REQUIRE(result.ok);   // wir uebersetzen weiter — nur melden, nicht abbrechen
    CHECK(hatZuweisungsWarnung(result.warnings));
}

TEST_CASE("EelTranspiler: Zuweisung im loop-Rumpf wird gemeldet (AVS)")
{
    const auto result = transpile("q=0; loop(8, q=q+2); r=q", Dialect::Avs);
    REQUIRE(result.ok);
    CHECK(hatZuweisungsWarnung(result.warnings));
}

TEST_CASE("EelTranspiler: Zuweisung in Klammern wird gemeldet (AVS)")
{
    const auto result = transpile("q=0; r=(q=3)+1", Dialect::Avs);
    REQUIRE(result.ok);
    CHECK(hatZuweisungsWarnung(result.warnings));
}

TEST_CASE("EelTranspiler: gewoehnliche Anweisungsfolge loest KEINE Warnung aus")
{
    const auto result = transpile("q=3; r=exec2(q, q+1); s=q*2", Dialect::Avs);
    REQUIRE(result.ok);
    CHECK_FALSE(hatZuweisungsWarnung(result.warnings));
}

TEST_CASE("EelTranspiler: MilkDrop erlaubt die Form — keine Warnung")
{
    // ns-eel2 kennt `_set` als Operator; gegen MilkdropRef gemessen liefern
    // beide Seiten dasselbe Ergebnis (S74).
    const auto result = transpile("q=0; while(exec2(q=q+1, below(q,5)))",
                                  Dialect::Milkdrop);
    REQUIRE(result.ok);
    CHECK_FALSE(hatZuweisungsWarnung(result.warnings));
}
