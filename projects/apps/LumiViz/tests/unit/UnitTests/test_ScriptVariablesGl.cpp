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
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"
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

TEST_CASE("Song-Slot (S82): laeuft bei JEDEM Knoten mit Frame-Slot genau einmal je Songwechsel")
{
    // Der Song-Slot ist ein Feld am Knoten, nicht je Typ — und rund 25
    // Skriptstellen im Renderer muessen ihn einzeln setzen und ausfuehren.
    // Dieser Test haelt jede davon fest: fehlt eine, faellt ihr Typ hier auf.
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
        if (!hatSongSlot(params)) continue;
        fillCodeSlots(params);

        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        leaf.params = params;
        leaf.songCode = "zz_song=1;";
        root.children.push_back(std::move(leaf));

        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(kSize, kSize));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
        vis.render(1.0f / 60.0f);  // kein Songwechsel: Song-Slot ruht
        const std::uint64_t vorher = LuaScriptEngine::songRunCount();
        MultiEffectVisualizer::notifySongChanged();
        vis.render(1.0f / 60.0f);  // hier genau einmal
        vis.render(1.0f / 60.0f);  // und nicht wieder
        const std::uint64_t laeufe = LuaScriptEngine::songRunCount() - vorher;
        CHECK_MESSAGE(laeufe == 1, std::string(effectTypeName(params))
                                       << ": Song-Slot lief " << laeufe << "-mal statt einmal");
        ++checked;
    }
    CHECK(checked > 70);  // Schutz gegen einen Test, der still nichts prueft
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

    // S85: ein ISF-Regler, der wie ein Feld des Knotens heisst (`gridx`),
    // weicht auf `in_gridx` aus — sonst baende das Skript Feld und Regler
    // unter einem Namen (bis S85 zaehlten nur `blend`/`mixamount`).
    IsfFilterParams kollision;
    kollision.fragCode = "void main() { gl_FragColor = vec4(gridx, 0.0, 0.0, 1.0); }";
    ParamWert gridx;
    gridx.key = "gridx";
    gridx.typ = ParamTyp::Zahl;
    gridx.zahl = 0.0;
    kollision.parameter.werte = {gridx};
    kollision.frameCode = "in_gridx = 1;";
    CHECK(renderCenter(kollision).redF() > 0.95);

    // Block E, Entscheid 1a: eigene Uniforms in Shadertoy und Pixel Filter —
    // was der Code mit `uniform` deklariert, setzt das Skript unter dem Namen.
    const auto renderNode = [](EffectParams params) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        leaf.params = std::move(params);
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

    ShadertoyParams st;
    st.code =
        "uniform float wobble;\n"
        "uniform vec2 tint;\n"
        "void mainImage(out vec4 c, in vec2 p) { c = vec4(wobble, tint.x, tint.y, 1.0); }\n";
    CHECK(renderNode(st).redF() < 0.05);  // ohne Skript: 0
    st.frameCode = "wobble = 1; tint_x = 1;";
    const QColor stMit = renderNode(st);
    CHECK(stMit.redF() > 0.95);
    CHECK(stMit.greenF() > 0.95);
    CHECK(stMit.blueF() < 0.05);

    PixelFilterParams pf;
    pf.code =
        "uniform float hell;\n"
        "vec4 farbe(vec2 uv, vec4 src) { return vec4(hell, hell, hell, 1.0); }\n";
    CHECK(renderNode(pf).redF() < 0.05);
    pf.frameCode = "hell = 1;";
    CHECK(renderNode(pf).redF() > 0.95);

    // S85: eigenes Uniform mit Feldnamen (`blend`) -> `in_blend`; die Namen
    // kommen jetzt aus der Feldtabelle statt aus der alten Hand-Liste.
    ShadertoyParams stKoll;
    stKoll.code =
        "uniform float blend;\n"
        "void mainImage(out vec4 c, in vec2 p) { c = vec4(blend, 0.0, 0.0, 1.0); }\n";
    stKoll.frameCode = "in_blend = 1;";
    CHECK(renderNode(stKoll).redF() > 0.95);
}

TEST_CASE("Script active (S83): aus = kein Slot laeuft, auch der Song-Slot nicht")
{
    // Der Schalter greift zentral in renderNode (Kopie der params mit leeren
    // EEL-Slots). Dieser Test haelt fest, dass das fuer JEDEN Typ mit Schalter
    // wirkt — eine Skriptstelle, die am params-Argument vorbei liest, fiele hier auf.
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

    // Laeuft der Knoten zwei Frames samt Songwechsel: wie oft liefen Slots,
    // wie oft der Song-Slot? (Die Namens-Sonde taugt hier nicht — sie sieht nur
    // Namen, die die LAUFZEIT bindet, nie skriptinterne wie zz_probe.)
    const auto lauf = [](const EffectParams& params, bool aktiv) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode leaf;
        leaf.params = params;
        leaf.songCode = "zz_song=1;";
        leaf.scriptsActive = aktiv;
        root.children.push_back(std::move(leaf));

        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(kSize, kSize));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
        const std::uint64_t slotsVorher = LuaScriptEngine::slotRunCount();
        const std::uint64_t songsVorher = LuaScriptEngine::songRunCount();
        vis.render(1.0f / 60.0f);
        MultiEffectVisualizer::notifySongChanged();
        vis.render(1.0f / 60.0f);
        return std::pair{LuaScriptEngine::slotRunCount() - slotsVorher,
                         LuaScriptEngine::songRunCount() - songsVorher};
    };

    int checked = 0;
    int gegenprobe = 0;  // Schalter an: das Skript laeuft wirklich
    for (EffectParams params :
         allAlternatives(std::make_index_sequence<std::variant_size_v<EffectParams>>{}))
    {
        if (!hatSkriptSchalter(params)) continue;
        fillCodeSlots(params);
        if (auto* m = std::get_if<MovementParams>(&params)) m->code = kProbeCode;

        // Nicht `slots`: das ist in Qt ein Makro.
        const auto [slotLaeufe, songLaeufe] = lauf(params, false);
        CHECK_MESSAGE(slotLaeufe == 0,
                      std::string(effectTypeName(params)) << ": " << slotLaeufe
                                                          << " Slot-Laeufe trotz Schalter aus");
        CHECK_MESSAGE(songLaeufe == 0,
                      std::string(effectTypeName(params)) << ": Song-Slot lief trotz Schalter aus");
        const auto [slotsAn, songsAn] = lauf(params, true);
        if (slotsAn > 0 && songsAn == 1) ++gegenprobe;
        ++checked;
    }
    CHECK(checked > 70);  // Schutz gegen einen Test, der still nichts prueft
    MESSAGE("Script active: " << checked << " Typen, Gegenprobe (an) lief bei " << gegenprobe);
    CHECK(gegenprobe > 60);
}

TEST_CASE("Script active (S83): nur 'aus' steht in der Datei, nie in einer Voreinstellung")
{
    ChainNode n;
    n.params = SuperScopeParams{};
    CHECK_FALSE(nodeToJson(n).contains(QStringLiteral("scriptsActive")));
    n.scriptsActive = false;
    const QJsonObject o = nodeToJson(n);
    REQUIRE(o.contains(QStringLiteral("scriptsActive")));
    CHECK_FALSE(nodeFromJson(o, nullptr).scriptsActive);
    CHECK(nodeFromJson(nodeToJson(ChainNode{}), nullptr).scriptsActive);  // Vorgabe an
    CHECK(lumi::multieffect::nodepresets::knotenRahmen().contains(QStringLiteral("scriptsActive")));
    CHECK_FALSE(lumi::multieffect::nodepresets::fieldNames(n.params).contains(
        QStringLiteral("scriptsActive")));
    // Effect List und Milkdrop tragen den Schalter nicht (eigene Mechanik).
    CHECK_FALSE(hatSkriptSchalter(ListParams{}));
    CHECK_FALSE(hatSkriptSchalter(MilkdropNodeParams{}));
    CHECK(hatSkriptSchalter(MovementParams{}));
}
