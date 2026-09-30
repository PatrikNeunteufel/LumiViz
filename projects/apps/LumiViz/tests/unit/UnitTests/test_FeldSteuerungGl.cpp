/**
 ****************************************************************************************
 * @file   test_FeldSteuerungGl.cpp
 * @brief  Block F, Stufe F2: jedes Tabellenfeld wirkt per Parameter-Skript
 *
 * @details
 * `test_ScriptVariablesGl` weist nach, DASS ein Name gebunden ist. Dieser Test
 * weist die WIRKUNG nach: ein Frame-Skript setzt jedes Live-Feld eines Knotens
 * auf einen Gegenwert — und die Frame-Kopie, die der Renderer als `params`
 * bekommt, traegt genau diesen Wert (Typpflege inklusive). Dazu die
 * Strukturregel E3: ein Strukturfeld folgt dem Frame-Slot nicht (Gegenprobe:
 * ein Live-Feld mit demselben Skript tut es).
 *
 * Gesehen wird die Kopie ueber `MultiEffectVisualizer::setzeParamBeobachter`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/FieldBindings.hpp"

#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>

#include <cmath>
#include <functional>
#include <iomanip>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

using namespace lumi::multieffect;
namespace ft = lumi::multieffect::feldtabelle;

namespace
{

constexpr int kSize = 64;

EffectParams alternative(std::size_t i)
{
    EffectParams out;
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        ((I == i ? (out.emplace<I>(), true) : false) || ...);
    }(std::make_index_sequence<std::variant_size_v<EffectParams>>{});
    return out;
}

/// Zulaessiger Wert ungleich der Vorgabe (wie test_FieldBindings); NaN = keiner.
double gegenwert(const ft::Feld& f, double vorgabe)
{
    switch (f.art)
    {
    case ft::Art::Schalter:
        return vorgabe >= 0.5 ? 0.0 : 1.0;
    case ft::Art::Aufzaehlung:
        if (f.hi <= f.lo) return std::nan("");
        return vorgabe == f.lo ? f.hi : f.lo;
    case ft::Art::Ganzzahl:
    case ft::Art::Kommazahl:
    {
        double v = std::isnan(f.lo) ? vorgabe + 1.0 : f.lo + (f.hi - f.lo) * 0.37;
        if (f.art == ft::Art::Ganzzahl) v = std::round(v);
        if (v == vorgabe) v = std::isnan(f.hi) ? vorgabe + 2.0 : (v == f.hi ? f.lo : f.hi);
        return v;
    }
    case ft::Art::Farbe:
        return static_cast<double>(0x336699);  // r=0.2, g=0.4, b=0.6
    }
    return std::nan("");
}

/// Skript in den Frame-Slot (ersatzweise Init), je nachdem was der Typ hat.
bool setzeSkript(EffectParams& p, const std::string& code)
{
    return std::visit(
        [&](auto& x) {
            if constexpr (requires { x.frameCode; })
            {
                x.frameCode = code;
                return true;
            }
            else if constexpr (requires { x.initCode; })
            {
                x.initCode = code;
                return true;
            }
            return false;
        },
        p);
}

/// Zwei Frames rendern; liefert die letzte gesteuerte Frame-Kopie des Blatts.
std::optional<EffectParams> laufe(const EffectParams& blatt, std::size_t index)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode leaf;
    leaf.params = blatt;
    root.children.push_back(std::move(leaf));

    std::optional<EffectParams> gesehen;
    MultiEffectVisualizer::setzeParamBeobachter(
        [&](const ChainNode&, const EffectParams& kopie) {
            if (kopie.index() == index) gesehen = kopie;
        });
    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(kSize, kSize));
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
    vis.render(1.0f / 60.0f);
    vis.render(1.0f / 60.0f);
    MultiEffectVisualizer::setzeParamBeobachter({});
    return gesehen;
}

struct GlUmgebung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QGuiApplication app{argc, argv};
    QOpenGLContext ctx;
    QOffscreenSurface surface;
    bool ok = false;

    GlUmgebung()
    {
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        if (!ctx.create()) return;
        surface.setFormat(ctx.format());
        surface.create();
        ok = surface.isValid() && ctx.makeCurrent(&surface);
    }
};

std::string zahl(double v)
{
    std::ostringstream s;
    s << std::setprecision(10) << v;
    return s.str();
}

}  // namespace

namespace
{
/// Jedes Live-Feld der Typen, die `nimmt` annimmt, per Frame-Skript setzen und
/// in der Frame-Kopie wiederfinden. Liefert (Typen, Felder).
std::pair<int, int> pruefeLiveFelder(const std::function<bool(const ft::Typ&)>& nimmt)
{
    int typen = 0;
    int felder = 0;
    for (std::size_t i = 0; i < std::variant_size_v<EffectParams>; ++i)
    {
        const ft::Typ* typ = ft::typ(i);
        if (typ == nullptr || !nimmt(*typ)) continue;
        EffectParams p = alternative(i);
        std::string code;
        std::vector<std::pair<const ft::Feld*, double>> soll;
        for (const ft::Feld& f : typ->felder)
        {
            if (f.klasse != ft::Klasse::Live) continue;
            // F2b: Renderer-eigene Namen (Punkt-Ausgabe, feste Eingabe) sind
            // bewusst nicht per Frame-Skript steuerbar
            if (ft::rendererEigen(typ->typKey, f.name)) continue;
            const double v = gegenwert(f, f.lesen(p));
            if (std::isnan(v)) continue;
            const std::string n(f.name);
            if (f.art == ft::Art::Farbe)
                code += n + "_r=0.2; " + n + "_g=0.4; " + n + "_b=0.6; ";
            else
                code += n + "=" + zahl(v) + "; ";
            soll.emplace_back(&f, v);
        }
        if (soll.empty() || !setzeSkript(p, code)) continue;
        CAPTURE(std::string(typ->typKey));
        const auto kopie = laufe(p, i);
        REQUIRE_MESSAGE(kopie.has_value(), "keine gesteuerte Frame-Kopie gesehen");
        for (const auto& [f, v] : soll)
        {
            CHECK_MESSAGE(f->lesen(*kopie) == doctest::Approx(v),
                          std::string(typ->typKey) << "." << std::string(f->feld)
                                                   << " soll " << v << " ist "
                                                   << f->lesen(*kopie));
            ++felder;
        }
        ++typen;
    }
    return {typen, felder};
}
}  // namespace

TEST_CASE("Feldsteuerung F2: jedes Live-Feld wirkt per Frame-Skript in der Frame-Kopie")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    const auto [typen, felder] = pruefeLiveFelder(
        [](const ft::Typ& t) { return t.traeger == ft::Traeger::ParamSkript; });
    CHECK(typen > 45);   // Schutz gegen einen Test, der still nichts prueft
    CHECK(felder > 200);
}

TEST_CASE("Feldsteuerung F2b: auch Module mit eigenem Skript-Host setzen jedes Live-Feld")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    const auto [typen, felder] = pruefeLiveFelder(
        [](const ft::Typ& t) { return t.traeger == ft::Traeger::EigenerHost; });
    CHECK(typen == 19);  // alle Module mit eigenem Skript-Host (Stand S85)
    CHECK(felder > 120);
}

TEST_CASE("Feldsteuerung F2b: ein Regler kompiliert nicht neu — der Skriptzustand bleibt")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Fractal 2D zaehlt im Skript Frames mit und schreibt sie nach `power`.
    // Bis S85 stand jeder Reglerwert im Kompilier-Schnappschuss: ein Regler
    // baute den Host neu, Init lief wieder, der Zaehler begann bei 0.
    Fractal2DParams fr;
    fr.initCode = "k = 0;";
    fr.frameCode = "k = k + 1; power = 1 + k;";
    ChainNode root;
    root.params = ListParams{};
    ChainNode leaf;
    leaf.params = fr;
    root.children.push_back(std::move(leaf));

    std::optional<EffectParams> gesehen;
    MultiEffectVisualizer::setzeParamBeobachter(
        [&](const ChainNode&, const EffectParams& kopie) { gesehen = kopie; });
    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(kSize, kSize));
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
    vis.render(1.0f / 60.0f);
    vis.render(1.0f / 60.0f);
    REQUIRE(gesehen.has_value());
    CHECK(std::get<Fractal2DParams>(*gesehen).power == doctest::Approx(3.0));

    // Regler bewegen wie das Panel: Wert aendern + recompileChain
    std::get<Fractal2DParams>(vis.chain().children[0].params).colorScale = 7.5f;
    vis.recompileChain();
    vis.render(1.0f / 60.0f);
    MultiEffectVisualizer::setzeParamBeobachter({});
    const auto& k = std::get<Fractal2DParams>(*gesehen);
    CHECK(k.power == doctest::Approx(4.0));         // Zaehler lief weiter (kein Init)
    CHECK(k.colorScale == doctest::Approx(7.5));    // der bewegte Regler gilt
}

TEST_CASE("Feldsteuerung F2: Strukturfeld folgt dem Frame-Slot nicht (E3), Live-Feld schon")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Mesh Warp: gridx ist Struktur (Mesh neu bauen), mixamount ist live.
    MeshWarpParams mw;
    mw.gridX = 16;
    mw.mixAmount = 0.1;
    mw.frameCode = "gridx = gridx + 1; mixamount = mixamount + 0.1;";
    EffectParams p = mw;
    const auto kopie = laufe(p, p.index());
    REQUIRE(kopie.has_value());
    const auto& k = std::get<MeshWarpParams>(*kopie);
    // Frame 1 ist frisch (Init-Frame): die Aenderung wird angenommen. Frame 2
    // ohne Beat: verworfen — es bleibt bei 17, nicht 18.
    CHECK(k.gridX == 17);
    CHECK(k.mixAmount == doctest::Approx(0.3));  // Gegenprobe: live folgt jedem Frame
}
