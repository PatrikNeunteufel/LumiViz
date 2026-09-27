/**
 ****************************************************************************************
 * @file   test_ScriptVariablesGl.cpp
 * @brief  Die Variablenreferenz verspricht nur, was die Laufzeit wirklich bindet
 *
 * @author Patrik Neunteufel
 * @date   September 2026
 *
 * @details
 * Bis S78 standen die Skriptvariablen eines Knotens an drei Stellen, und acht
 * Knoten versprachen Namen, die nie gebunden waren (Brightness `distance`,
 * Kleinian `p`/`q`, Custom BPM `skipfirst` …). Seit S79 ist
 * `ScriptVariables.hpp` die Quelle der Referenz im Panel; dieser Test haelt sie
 * an der Laufzeit fest.
 *
 * Verfahren: jede Alternative der `EffectParams`-Variante, die eine Tabelle
 * hat, bekommt in JEDEM Code-Slot ein Skript (sonst laeuft das opt-in-
 * Parameter-Skript gar nicht), laeuft zwei Frames, und die Sonde der
 * LuaScriptEngine sammelt jeden Namen, den die Laufzeit setzt oder liest.
 * Jeder pruefbare Tabellenname muss darunter sein.
 *
 * Grenze: die Sonde sieht, DASS ein Name gebunden ist, nicht, dass der
 * zurueckgelesene Wert im Renderpfad ankommt — das bleibt Sache der Sichttests.
 ****************************************************************************************
 */

#include <doctest.h>

#include "scripting/LuaScriptEngine.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ScriptVariables.hpp"

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QImage>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>

#include <cstring>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace lumi::multieffect;
using lumi::scripting::LuaScriptEngine;

namespace {

constexpr int kSize = 64;
constexpr const char* kProbeCode = "zz_probe=1;";
/// Init setzt zusaetzlich `n`: Texer II zeichnet nur so viele Sprites, wie das
/// Skript verlangt (Vorgabe 0, wie in AVS) — ohne `n` liefe der Punkt-Slot nie.
constexpr const char* kProbeInit = "zz_probe=1; n=4;";

template <std::size_t... I>
std::vector<EffectParams> allAlternatives(std::index_sequence<I...> /*unused*/)
{
    return {EffectParams{std::in_place_index<I>}...};
}

/// Jeden Code-Slot des Knotens belegen, damit jeder Skriptpfad laeuft.
void fillCodeSlots(EffectParams& params)
{
    std::visit(
        [](auto& p) {
            if constexpr (requires { p.initCode; }) p.initCode = kProbeInit;
            if constexpr (requires { p.frameCode; }) p.frameCode = kProbeCode;
            if constexpr (requires { p.beatCode; }) p.beatCode = kProbeCode;
            if constexpr (requires { p.pointCode; }) p.pointCode = kProbeCode;
            if constexpr (requires { p.pixelCode; }) p.pixelCode = kProbeCode;
            if constexpr (requires { p.levelCode; }) p.levelCode = kProbeCode;
            if constexpr (requires { p.useCode; }) p.useCode = true;
        },
        params);
}

std::set<std::string> touchedNames(const EffectParams& params)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode leaf;
    leaf.params = params;
    root.children.push_back(std::move(leaf));

    std::set<std::string> names;
    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(kSize, kSize));
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
    LuaScriptEngine::setNameProbe(&names);
    vis.render(1.0f / 60.0f);
    vis.render(1.0f / 60.0f);
    LuaScriptEngine::setNameProbe(nullptr);
    return names;
}

/// Tabellennamen, die die Laufzeit nie anfasst.
std::vector<std::string> missingNames(const ScriptVarTable& table,
                                      const std::set<std::string>& touched)
{
    std::vector<std::string> missing;
    auto check = [&](const std::vector<ScriptVar>& rows) {
        for (const ScriptVar& v : rows)
        {
            if (!v.probe || std::strcmp(v.dir, "own") == 0) continue;
            for (const std::string& n : splitScriptVarNames(v.names))
                if (touched.count(n) == 0) missing.push_back(n);
        }
    };
    check(table.vars);
    check(scriptVarBaseRows(table.base));
    return missing;
}

}  // namespace

TEST_CASE("ScriptVariables: splitScriptVarNames zerlegt an ' / '")
{
    const auto v = splitScriptVarNames("red / green / blue");
    REQUIRE(v.size() == 3);
    CHECK(v[0] == "red");
    CHECK(v[2] == "blue");
    CHECK(splitScriptVarNames("wave_a/r/g/b").size() == 1);  // kein Leerzeichen = ein Name
}

TEST_CASE("ScriptVariables: jede Tabellenvariable wird von der Laufzeit gebunden")
{
    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (!ctx.create())
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    QOffscreenSurface surface;
    surface.setFormat(ctx.format());
    surface.create();
    REQUIRE(surface.isValid());
    REQUIRE(ctx.makeCurrent(&surface));

    int checked = 0;
    for (EffectParams params :
         allAlternatives(std::make_index_sequence<std::variant_size_v<EffectParams>>{}))
    {
        const ScriptVarTable* table = scriptVariables(params);
        if (table == nullptr) continue;
        // Nur-Doku-Tabellen (Movement, MilkDrop) haben nichts Pruefbares.
        if (missingNames(*table, {}).empty()) continue;
        fillCodeSlots(params);
        const std::vector<std::string> missing = missingNames(*table, touchedNames(params));
        std::ostringstream list;
        for (const std::string& n : missing) list << ' ' << n;
        CHECK_MESSAGE(missing.empty(), std::string(effectTypeName(params))
                                           << ": in der Tabelle, aber nie gebunden:"
                                           << list.str());
        ++checked;
    }
    // Schutz gegen einen Test, der still nichts prueft.
    CHECK(checked > 60);
}

TEST_CASE("ScriptVariables: ISF-Regler lassen sich unter ihrem key per Skript setzen")
{
    // Block E (S79). Die Sonde oben kann die Regler nicht pruefen — ihre Namen
    // stehen erst im Preset. Deshalb hier die Wirkung: der Shader gibt seine
    // Regler als Farbe aus, das Skript setzt sie, das Bild muss folgen.
    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (!ctx.create())
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    QOffscreenSurface surface;
    surface.setFormat(ctx.format());
    surface.create();
    REQUIRE(surface.isValid());
    REQUIRE(ctx.makeCurrent(&surface));

    IsfFilterParams isf;
    isf.fragCode = "void main() { gl_FragColor = vec4(gain, pos.x, pos.y, 1.0); }";
    ParamWert gain;
    gain.key = "gain";
    gain.typ = ParamTyp::Zahl;
    gain.zahl = 0.0;
    ParamWert pos;
    pos.key = "pos";
    pos.typ = ParamTyp::Punkt2D;
    isf.parameter.werte = {gain, pos};

    auto renderCenter = [](IsfFilterParams p) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        leaf.params = std::move(p);
        root.children.push_back(std::move(leaf));
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(kSize, kSize));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
        vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        REQUIRE_FALSE(img.isNull());
        return img.pixelColor(kSize / 2, kSize / 2);
    };

    // Ohne Skript: alle Regler 0 -> schwarz.
    const QColor ohne = renderCenter(isf);
    CHECK(ohne.redF() < 0.05);
    CHECK(ohne.greenF() < 0.05);

    // Mit Skript: gain und die x-Komponente des Punkts auf 1.
    isf.frameCode = "gain = 1; pos_x = 1; pos_y = 0;";
    const QColor mit = renderCenter(isf);
    CHECK(mit.redF() > 0.95);
    CHECK(mit.greenF() > 0.95);
    CHECK(mit.blueF() < 0.05);
}
