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
#include "visualizers/multieffect/NodePresetStore.hpp"
#include "visualizers/multieffect/TrailPresetStore.hpp"

#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QTemporaryDir>

#include <cmath>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <map>
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
        // Effect List (S86): ihr Code laeuft nur mit „Use list code"
        if (auto* lp = std::get_if<ListParams>(&p)) lp->useCode = true;
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
    // alle Module mit eigenem Skript-Host (S86: + SuperScope, Effect List,
    // Dynamic Movement, Host Switcher)
    CHECK(typen == 23);
    CHECK(felder > 120);
}

namespace
{
/// F4: einen Knoten (mit Modulationen) zwei Frames laufen lassen; liefert die
/// letzte gesehene Frame-Kopie.
std::optional<EffectParams> laufeKnoten(ChainNode leaf)
{
    const std::size_t index = leaf.params.index();
    ChainNode root;
    root.params = ListParams{};
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

FeldModulation mod(const char* feld, ModQuelle q, double tiefe, double rate = 0.0)
{
    FeldModulation m;
    m.feld = feld;
    m.quelle = q;
    m.tiefe = tiefe;
    m.rateHz = rate;
    return m;
}
}  // namespace

TEST_CASE("Modulation F4: Tiefe, Spanne, Klemmen, Ganzzahl, nicht modulierbare Felder")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Rechteck-LFO mit Rate 0 steht fest auf +1: Wert = Regler + Tiefe * Spanne
    ChainNode leaf;
    leaf.params = BloomParams{};  // intensity 1 (0..8), radius 8 (1..32), threshold 0 (0..1)
    leaf.mods = {mod("intensity", ModQuelle::LfoRechteck, 0.1),
                 mod("radius", ModQuelle::LfoRechteck, -0.1),
                 mod("threshold", ModQuelle::LfoRechteck, 2.0),
                 mod("vignetteStrength", ModQuelle::LfoRechteck, 0.0),
                 mod("post", ModQuelle::LfoRechteck, 1.0)};
    const BloomParams vorgabe{};
    const auto k = laufeKnoten(leaf);
    REQUIRE(k.has_value());
    const auto& b = std::get<BloomParams>(*k);
    CHECK(b.intensity == doctest::Approx(1.0 + 0.1 * 8.0));  // Spanne = Bereich 0..8
    CHECK(b.radius == 4);                                     // 8 - 3.1 = 4.9 -> abgeschnitten (E6)
    CHECK(b.threshold == doctest::Approx(1.0));               // geklemmt auf hi
    CHECK(b.vignetteStrength == doctest::Approx(vorgabe.vignetteStrength));  // Tiefe 0
    CHECK(b.post == vorgabe.post);                            // Schalter: nicht modulierbar

    // Gegenprobe: Knotenschalter aus bzw. Haekchen aus -> Reglerwert. Ein
    // wirkungsloses Frame-Skript sorgt fuer die Frame-Kopie, die wir sehen.
    ChainNode aus = leaf;
    std::get<BloomParams>(aus.params).frameCode = "zz = 0;";
    aus.modsActive = false;
    const auto kAus = laufeKnoten(aus);
    REQUIRE(kAus.has_value());
    CHECK(std::get<BloomParams>(*kAus).intensity == doctest::Approx(vorgabe.intensity));
    ChainNode haken = leaf;
    std::get<BloomParams>(haken.params).frameCode = "zz = 0;";
    for (FeldModulation& m : haken.mods) m.aktiv = false;
    const auto kHaken = laufeKnoten(haken);
    REQUIRE(kHaken.has_value());
    CHECK(std::get<BloomParams>(*kHaken).radius == vorgabe.radius);
}

TEST_CASE("Modulation F4: Reihenfolge E5 — das Skript sieht den Wert und darf ueberschreiben")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    ChainNode leaf;
    BloomParams b;
    b.frameCode = "intensity = 0.5; vignettestrength = threshold;";
    leaf.params = b;
    leaf.mods = {mod("intensity", ModQuelle::LfoRechteck, 0.1),
                 mod("threshold", ModQuelle::LfoRechteck, 0.25)};
    const auto k = laufeKnoten(leaf);
    REQUIRE(k.has_value());
    const auto& e = std::get<BloomParams>(*k);
    CHECK(e.intensity == doctest::Approx(0.5));          // Skript ueberschreibt
    CHECK(e.threshold == doctest::Approx(0.25));         // Modulation ohne Skript-Eingriff
    CHECK(e.vignetteStrength == doctest::Approx(0.25));  // Skript sah den modulierten Wert
}

TEST_CASE("Modulation F4: LFOs laufen auf der Sim-Uhr, Zufall ist je Preset gleich")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Saege 15 Hz, zwei Frames zu 1/60 s: Phase 0.5 -> Wert 0 -> Reglerwert.
    // Sinus 15 Hz: Phase 0.5 -> sin(pi) = 0 ebenso; Dreieck: 3 - 4*0.5 = 1.
    ChainNode leaf;
    leaf.params = BloomParams{};
    leaf.mods = {mod("intensity", ModQuelle::LfoSaege, 0.1, 15.0),
                 mod("threshold", ModQuelle::LfoDreieck, 0.5, 15.0),
                 mod("vignetteStrength", ModQuelle::LfoZufall, 0.4, 1.0)};
    const auto a = laufeKnoten(leaf);
    const auto b = laufeKnoten(leaf);
    REQUIRE(a.has_value());
    REQUIRE(b.has_value());
    const auto& ea = std::get<BloomParams>(*a);
    CHECK(ea.intensity == doctest::Approx(1.0).epsilon(1e-5));
    CHECK(ea.threshold == doctest::Approx(0.5).epsilon(1e-5));
    // Zufall: deterministisch (gleiche Saat je Knoten und Feld) und bewegt
    CHECK(std::get<BloomParams>(*b).vignetteStrength == ea.vignetteStrength);
    CHECK(ea.vignetteStrength != doctest::Approx(BloomParams{}.vignetteStrength));
}

TEST_CASE("Modulation F4c: SuperScope und Effect List ohne Skripttraeger, uebrige Typen zu")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Rechteck-LFO mit Rate 0 steht fest auf +1: Wert = Regler + Tiefe * Spanne
    ChainNode scope;
    scope.params = SuperScopeParams{};  // pointCount 256 (1..4096), lineWidth 2 (1..20),
                                        // dotSize 4 (1..50), colorCycleFrames 60 (1..600)
    scope.mods = {mod("pointCount", ModQuelle::LfoRechteck, 0.1),
                  mod("lineWidth", ModQuelle::LfoRechteck, 0.5),
                  mod("dotSize", ModQuelle::LfoRechteck, -1.0),
                  mod("colorCycleFrames", ModQuelle::LfoRechteck, 0.0),
                  mod("renderMode", ModQuelle::LfoRechteck, 1.0)};
    const SuperScopeParams sv{};
    const auto ks = laufeKnoten(scope);
    REQUIRE(ks.has_value());
    const auto& s = std::get<SuperScopeParams>(*ks);
    CHECK(s.pointCount == 665);                           // 256 + 409.5 -> abgeschnitten (E6)
    CHECK(s.lineWidth == doctest::Approx(2.0 + 0.5 * 19.0));
    CHECK(s.dotSize == doctest::Approx(1.0));             // geklemmt auf lo
    CHECK(s.colorCycleFrames == sv.colorCycleFrames);     // Tiefe 0
    CHECK(s.renderMode == sv.renderMode);                 // Aufzaehlung: nicht modulierbar

    // Effect List als Kind der Wurzel: die Wurzel selbst traegt keine Modulation
    ChainNode liste;
    liste.params = ListParams{};  // in/outAdjustAlpha 128 (0..255), onBeatFrames 1 (1..200)
    liste.mods = {mod("inAdjustAlpha", ModQuelle::LfoRechteck, 0.25),
                  mod("outAdjustAlpha", ModQuelle::LfoRechteck, -1.0),
                  mod("bufferOut", ModQuelle::LfoRechteck, 0.5),
                  mod("onBeatFrames", ModQuelle::LfoRechteck, 0.0)};
    const auto kl = laufeKnoten(liste);
    REQUIRE(kl.has_value());
    const auto& l = std::get<ListParams>(*kl);
    CHECK(l.inAdjustAlpha == 191);   // 128 + 63.75 -> abgeschnitten
    CHECK(l.outAdjustAlpha == 0);    // geklemmt auf lo
    CHECK(l.bufferOut == 3);         // 0 + 3.5 -> abgeschnitten
    CHECK(l.onBeatFrames == 1);      // Tiefe 0

    // Gegenprobe: Knotenschalter aus -> keine Kopie, der Renderer laeuft auf
    // den Reglerwerten (ohne Traeger gibt es die Kopie nur bei Modulation)
    ChainNode aus = scope;
    aus.modsActive = false;
    CHECK(!laufeKnoten(aus).has_value());

    // Teil 2: Text, AVI, Dynamic Movement, Debug Bars
    ChainNode text;
    text.params = TextParams{};
    text.mods = {mod("normSpeed", ModQuelle::LfoRechteck, 0.1),      // 1..600
                 mod("fontHeight", ModQuelle::LfoRechteck, 0.5)};    // Struktur: zu
    const TextParams tv{};
    const auto kt = laufeKnoten(text);
    REQUIRE(kt.has_value());
    CHECK(std::get<TextParams>(*kt).normSpeed ==
          std::min(600, static_cast<int>(tv.normSpeed + 0.1 * 599.0)));
    CHECK(std::get<TextParams>(*kt).fontHeight == tv.fontHeight);

    ChainNode dm;
    dm.params = DynamicMovementParams{};
    dm.mods = {mod("buffern", ModQuelle::LfoRechteck, 0.5),          // 0..8
               mod("xres", ModQuelle::LfoRechteck, 0.5)};            // Struktur: zu
    const DynamicMovementParams dv{};
    const auto kd = laufeKnoten(dm);
    REQUIRE(kd.has_value());
    CHECK(std::get<DynamicMovementParams>(*kd).buffern ==
          std::min(8, static_cast<int>(dv.buffern + 4.0)));
    CHECK(std::get<DynamicMovementParams>(*kd).xres == dv.xres);

    ChainNode bars;
    bars.params = DebugBarsParams{};
    bars.mods = {mod("orbitSpeed", ModQuelle::LfoRechteck, 0.1)};    // -10..10
    const auto kb = laufeKnoten(bars);
    REQUIRE(kb.has_value());
    CHECK(std::get<DebugBarsParams>(*kb).orbitSpeed ==
          doctest::Approx(std::min(10.0, DebugBarsParams{}.orbitSpeed + 2.0)));

    ChainNode gruppe;
    gruppe.params = HostGroupParams{};  // outAdjustAlpha 128 (0..255)
    gruppe.mods = {mod("outAdjustAlpha", ModQuelle::LfoRechteck, 0.25),
                   mod("crossfadeSeconds", ModQuelle::LfoRechteck, 0.5)};  // 0..60
    const auto kg = laufeKnoten(gruppe);
    REQUIRE(kg.has_value());
    CHECK(std::get<HostGroupParams>(*kg).outAdjustAlpha == 191);
    CHECK(std::get<HostGroupParams>(*kg).crossfadeSeconds ==
          doctest::Approx(std::min(60.0, HostGroupParams{}.crossfadeSeconds + 30.0)));

    ChainNode avi;
    avi.params = AviParams{};
    avi.mods = {mod("persist", ModQuelle::LfoRechteck, 0.5)};  // 0..32
    const auto ka = laufeKnoten(avi);
    REQUIRE(ka.has_value());
    CHECK(std::get<AviParams>(*ka).persist ==
          std::min(32, static_cast<int>(AviParams{}.persist + 16.0)));

    // Offen sind genau diese Typen ohne Traeger; Host Switcher und Render
    // Scale laufen vor dem Frame und bleiben zu, Movement hat kein Zahlfeld
    std::set<std::string_view> offen;
    for (const ft::Typ* t : ft::alle())
    {
        if (t->traeger != ft::Traeger::Keiner) continue;
        for (const ft::Feld& f : t->felder)
            if (ft::modulierbar(*t, f)) offen.insert(t->typKey);
    }
    CHECK(offen == std::set<std::string_view>{"debugBars"});

    // Host Switcher: laeuft vorab in runHostSwitchers mit eigener Kopie
    ChainNode sw;
    sw.params = HostSwitcherParams{};  // intervalSeconds 0..3600
    sw.mods = {mod("intervalSeconds", ModQuelle::LfoRechteck, 0.01)};
    HostSwitcherParams hv;
    std::get<HostSwitcherParams>(sw.params).frameCode = "zz = 1;";  // Beobachter laeuft mit Skript
    const auto kw = laufeKnoten(sw);
    REQUIRE(kw.has_value());
    CHECK(std::get<HostSwitcherParams>(*kw).intervalSeconds ==
          doctest::Approx(hv.intervalSeconds + 36.0));
}

// S86 (Befund Patrik): beim Ziehen am Fensterrand brach der Blur-Nachzug ab und
// der Text flackerte — jeder Groessenwechsel verwarf den Bildspeicher der Liste.
// Jetzt wird das Bild in die neue Groesse uebernommen (wie AVS „reuse image on
// window resize").
TEST_CASE("Groessenwechsel S86: Liste und Wurzel behalten ihr Bild (Nachzug bleibt)")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    clear.params = ClearParams{};
    ChainNode liste;
    liste.params = ListParams{};
    ChainNode text;
    TextParams tp;
    tp.text = "LumiViz";
    tp.fontHeight = -40;
    text.params = tp;
    ChainNode blur;
    blur.params = BlurParams{};
    ChainNode stern;
    stern.params = OscStarParams{};
    liste.children = {text, blur, stern};
    root.children = {clear, liste};

    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    auto* f = QOpenGLContext::currentContext()->functions();
    const auto messe = [&](int w, int h, int frames) {
        vis.resize(QSize(w, h));
        f->glViewport(0, 0, w, h);
        for (int i = 0; i < frames; ++i) vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        int maxChroma = 0;
        double luma = 0.0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
            {
                const QRgb p = img.pixel(x, y);
                maxChroma = std::max({maxChroma, std::abs(qRed(p) - qGreen(p)),
                                      std::abs(qGreen(p) - qBlue(p))});
                luma += qGray(p);
            }
        luma /= std::max(1, img.width() * img.height());
        CHECK(maxChroma == 0);  // weisse Kette: nirgends Farbe
        return luma;
    };
    // Nachzug aufbauen, dann in kleinen Schritten die Groesse aendern: das
    // Bild bleibt hell. Vor S86 fiel es bei jedem Schritt auf rund ein Viertel.
    const double aufgebaut = messe(320, 240, 30);
    REQUIRE(aufgebaut > 5.0);
    for (const auto& [w, h] : {std::pair{321, 240}, {322, 241}, {310, 233}, {333, 251}})
    {
        CAPTURE(w);
        CHECK(messe(w, h, 1) > aufgebaut * 0.8);
    }
}

TEST_CASE("Text S86: Tags des Titels kommen vom Songwechsel bis ins Bild")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace tt = lumi::multieffect::texttags;
    const auto hell = [](const std::string& vorlage) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode text;
        TextParams tp;
        tp.text = vorlage;
        tp.fontHeight = -24;
        text.params = tp;
        root.children = {clear, text};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(160, 80));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 160, 80);
        vis.render(1.0f / 60.0f);
        vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        int n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 128) ++n;
        return n;
    };
    // kein Titel: die Vorlage ergibt ein leeres Wort — es wird nichts gezeichnet
    MultiEffectVisualizer::notifySongChanged(tt::SongTags{});
    CHECK(hell("[{artist} - ]{title}") == 0);
    // fester Text zeichnet unabhaengig davon
    const int fest = hell("MMMM");
    CHECK(fest > 20);

    tt::SongTags t;
    t.title = "MMMM";
    MultiEffectVisualizer::notifySongChanged(t);
    // dieselben Glyphen wie der feste Text
    CHECK(hell("[{artist} - ]{title}") == fest);
    // mit Interpret wird die Zeile laenger
    t.artist = "MM";
    MultiEffectVisualizer::notifySongChanged(t);
    CHECK(hell("[{artist} - ]{title}") > fest);

    MultiEffectVisualizer::notifySongChanged(tt::SongTags{});  // Zustand zuruecksetzen
}

TEST_CASE("Lyrics S86: laufende Zeile an der Abspielposition, Vorschau, unsynchroner Block")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    // Zeile 1 ist schmal („I"), Zeile 2 breit — die Zahl heller Pixel verraet,
    // welche Zeile in der Farbe der laufenden Zeile (weiss) gezeichnet ist.
    const auto hell = [](const LyricsParams& lp) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        root.children = {clear, lyr};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(320, 120));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 120);
        vis.render(1.0f / 60.0f);
        vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        int n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 200) ++n;
        return n;
    };
    LyricsParams lp;
    lp.fontHeight = -24;
    lp.outline = false;
    lp.previewLines = 0;
    lp.colorNext = 0x202020;  // Vorschau dunkel: zaehlt nicht als hell

    MultiEffectVisualizer::notifyPlayState(false);  // Position steht (keine Wanduhr im Test)
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    CHECK(hell(lp) == 0);  // kein Songtext: nichts

    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]I\n[00:20.00]MMMMMMMM\n"));
    MultiEffectVisualizer::notifyPlayPosition(5000);
    CHECK(hell(lp) == 0);  // vor der ersten Zeile
    MultiEffectVisualizer::notifyPlayPosition(12000);
    const int schmal = hell(lp);
    CHECK(schmal > 0);
    MultiEffectVisualizer::notifyPlayPosition(25000);
    const int breit = hell(lp);
    CHECK(breit > schmal * 3);  // die zweite Zeile laeuft

    // Versatz: +9000 ms laesst die zweite Zeile schon bei 12 s erscheinen
    MultiEffectVisualizer::notifyPlayPosition(12000);
    lp.offsetMs = 9000;
    CHECK(hell(lp) == breit);
    lp.offsetMs = 0;
    // S88: derselbe Versatz nur fuer den laufenden Titel — gleicher Sinn, und
    // er zaehlt zum Feld des Knotens dazu
    CHECK(hell(lp) == schmal);  // Gegenprobe: ohne Versatz die erste Zeile
    MultiEffectVisualizer::setSongtextTitelVersatz(9000);
    CHECK(hell(lp) == breit);
    lp.offsetMs = -9000;  // Knoten und Titel heben sich auf
    CHECK(hell(lp) == schmal);
    lp.offsetMs = 0;
    MultiEffectVisualizer::setSongtextTitelVersatz(0);

    // Vorschau: die naechste Zeile steht (dunkel) dabei — hell bleibt nur die laufende
    lp.previewLines = 1;
    CHECK(hell(lp) == doctest::Approx(schmal).epsilon(0.05));
    lp.colorNext = 0xFFFFFF;  // Gegenprobe: helle Vorschau zaehlt mit
    CHECK(hell(lp) > schmal * 3);
    lp.previewLines = 0;
    lp.colorNext = 0x202020;

    // Wort fuer Wort: nur die gesungenen Worte tragen die Farbe der Zeile
    MultiEffectVisualizer::setSongtext(
        ly::leseLrc("[00:10.00]MMMM <00:14.00>MMMM\n"));
    MultiEffectVisualizer::notifyPlayPosition(12000);
    const int halb = hell(lp);
    MultiEffectVisualizer::notifyPlayPosition(15000);
    const int ganz = hell(lp);
    CHECK(halb > 0);
    CHECK(ganz > halb * 1.6);

    // S87 (Befund Patrik): dieselbe Zeile zweimal hintereinander — gegen Ende
    // des ersten Durchgangs blendet sie zur Vorschau-Farbe, mit der zweiten
    // Zeitmarke ist sie wieder voll da. Eine ANDERE Folgezeile blendet nicht.
    MultiEffectVisualizer::setSongtext(
        ly::leseLrc("[00:10.00]MMMM\n[00:13.00]MMMM\n[00:16.00]IIII\n"));
    MultiEffectVisualizer::notifyPlayPosition(11000);
    const int voll = hell(lp);
    CHECK(voll > 0);
    MultiEffectVisualizer::notifyPlayPosition(12990);  // 10 ms vor der Wiederholung
    CHECK(hell(lp) == 0);                              // ausgeblendet (Vorschau-Farbe ist dunkel)
    MultiEffectVisualizer::notifyPlayPosition(12700);  // mitten in der Blende
    CHECK(hell(lp) < voll);
    MultiEffectVisualizer::notifyPlayPosition(13000);  // die Wiederholung setzt ein
    CHECK(hell(lp) == voll);
    MultiEffectVisualizer::notifyPlayPosition(15990);  // danach folgt eine andere Zeile
    CHECK(hell(lp) == voll);

    // Unsynchron: der Block steht — die Position spielt keine Rolle
    MultiEffectVisualizer::setSongtext(ly::leseLrc("I\nMMMMMMMM\n"));
    lp.showSyncMark = false;
    MultiEffectVisualizer::notifyPlayPosition(0);
    const int block = hell(lp);
    MultiEffectVisualizer::notifyPlayPosition(999000);
    CHECK(hell(lp) == block);
    CHECK(block == doctest::Approx(schmal).epsilon(0.05));  // erste Zeile

    MultiEffectVisualizer::setSongtext(ly::Songtext{});  // Zustand zuruecksetzen
    MultiEffectVisualizer::notifyPlayPosition(0);
}

TEST_CASE("Lyrics Trail S87: die eben beendete Zeile, fuer die Haltezeit, an der Stelle des Lyrics-Knotens")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);

    // Helle Pixel und ihr Schwerpunkt in y — der Lyrics-Knoten selbst ist
    // ausgeschaltet (das Auge), der Trail leiht sich nur Schrift und Lage.
    struct Bild
    {
        int hell = 0;
        double y = 0.0;
    };
    const auto zeichne = [](const LyricsTrailParams& tp, bool lyricsAn, bool trailAn,
                            const std::string& lyricsName = "Lyrics") {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        LyricsParams lp;
        lp.fontHeight = -24;
        lp.outline = false;
        lp.previewLines = 0;
        lp.colorNext = 0x202020;
        lyr.params = lp;
        lyr.displayName = lyricsName;
        lyr.enabled = lyricsAn;
        ChainNode trail;
        trail.params = tp;
        trail.enabled = trailAn;
        root.children = {clear, lyr, trail};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        vis.render(1.0f / 60.0f);
        vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        Bild b;
        double summe = 0.0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 200)
                {
                    ++b.hell;
                    summe += y;
                }
        if (b.hell > 0) b.y = summe / b.hell;
        return b;
    };

    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM\n[00:13.00]IIII\n"));
    LyricsTrailParams tp;  // Vorgaben: 300 ms, 8 % nach oben

    // die erste Zeile laeuft noch: nichts ist zu Ende
    MultiEffectVisualizer::notifyPlayPosition(12000);
    CHECK(zeichne(tp, false, true).hell == 0);
    // 100 ms nach dem Wechsel: die ALTE Zeile („MMMM", breit) steht da
    MultiEffectVisualizer::notifyPlayPosition(13100);
    const Bild alt = zeichne(tp, false, true);
    CHECK(alt.hell > 0);
    // … und zwar oberhalb der Stelle, an der der Lyrics-Knoten zeichnet
    const Bild neu = zeichne(tp, true, false);  // nur Lyrics: die neue Zeile „IIII"
    REQUIRE(neu.hell > 0);
    CHECK(alt.hell > neu.hell * 2);  // „MMMM" ist breiter als „IIII"
    CHECK(alt.y < neu.y - 10.0);     // 8 % von 240 px = 19 px hoeher
    // ohne Versatz steht sie genau dort, wo sie eben noch stand
    LyricsTrailParams ohne = tp;
    ohne.yShift = 0;
    CHECK(zeichne(ohne, false, true).y == doctest::Approx(neu.y).epsilon(0.02));
    // die Haltezeit ist um: nichts mehr (das Verschwinden machen die Effekte danach)
    MultiEffectVisualizer::notifyPlayPosition(13500);
    CHECK(zeichne(tp, false, true).hell == 0);
    LyricsTrailParams lang = tp;
    lang.holdMs = 1000;
    CHECK(zeichne(lang, false, true).hell == alt.hell);

    // Bezug ueber den Namen: ein falscher Name findet keinen Lyrics-Knoten
    MultiEffectVisualizer::notifyPlayPosition(13100);
    LyricsTrailParams benannt = tp;
    benannt.source = "Karaoke";
    CHECK(zeichne(benannt, false, true).hell == 0);
    CHECK(zeichne(benannt, false, true, "Karaoke").hell == alt.hell);

    // eigene Farbe statt der Zeilenfarbe
    LyricsTrailParams dunkel = tp;
    dunkel.useOwnColor = true;
    dunkel.color = 0x303030;
    CHECK(zeichne(dunkel, false, true).hell == 0);

    // unsynchroner Text hat keine Zeiten: der Trail zeichnet nichts
    MultiEffectVisualizer::setSongtext(ly::leseLrc("MMMM\nIIII\n"));
    CHECK(zeichne(tp, false, true).hell == 0);

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90, LB1 (docs/visuals/Lyrics_Block_Konzept.md): Befund per Test, bevor der
// Trail umgebaut wird. Eine Pause-Zeile (Zeit ohne Text) zwischen zwei
// Textzeilen — wie oft zeichnet der Trail die Zeile davor? Gezaehlt werden
// die Abschnitte zusammenhaengender Frames, in denen der Trail etwas zeichnet.
TEST_CASE("Lyrics Trail S90: Pause-Zeile — wie oft die alte Zeile gezeichnet wird")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);

    const auto abschnitte = [](const char* lrc) {
        MultiEffectVisualizer::setSongtext(ly::leseLrc(lrc));
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        LyricsParams lp;
        lp.fontHeight = -24;
        lp.outline = false;
        lp.previewLines = 0;
        lyr.params = lp;
        lyr.enabled = false;  // nur der Trail zeichnet
        ChainNode trail;
        trail.params = LyricsTrailParams{};  // Haltezeit 300 ms
        root.children = {clear, lyr, trail};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        int n = 0;
        bool war = false;
        // 12,5 s bis 17 s in Schritten von 1/60 s
        for (int frame = 0; frame <= 270; ++frame)
        {
            MultiEffectVisualizer::notifyPlayPosition(12500 + frame * 1000 / 60);
            vis.render(1.0f / 60.0f);
            const QImage img = vis.debugGrabRootSurface();
            bool hell = false;
            for (int y = 0; y < img.height() && !hell; ++y)
                for (int x = 0; x < img.width() && !hell; ++x)
                    hell = qGray(img.pixel(x, y)) > 200;
            if (hell && !war) ++n;
            war = hell;
        }
        return n;
    };

    // Gegenprobe: ohne Pause ein Wechsel bei 13 s und einer bei 16 s
    CHECK(abschnitte("[00:10.00]MMMM\n[00:13.00]MMMM IIII\n[00:16.00]IIII\n") == 2);
    // mit Pause: „MMMM" endet bei 13 s — einmal ist richtig
    const int mitPause = abschnitte("[00:10.00]MMMM\n[00:13.00]\n[00:16.00]IIII\n");
    MESSAGE("Trail-Abschnitte mit Pause-Zeile: " << mitPause);
    CHECK(mitPause == 1);

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S91 (Entscheid T2 a): das Feld Line am Lyrics Trail — einmal je gegangener
// Zeile; `line` ist die Zeile, die eben ging (die der Trail zeichnet).
TEST_CASE("Lyrics Trail S91: Feld Line laeuft einmal je gegangener Zeile, kennt line / linecount")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);

    LyricsTrailParams gesehen;
    MultiEffectVisualizer::setzeParamBeobachter([&](const ChainNode&, const EffectParams& p) {
        if (const auto* tp = std::get_if<LyricsTrailParams>(&p)) gesehen = *tp;
    });
    const auto kette = [](const LyricsParams& lp, const LyricsTrailParams& tp) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        ChainNode trail;
        trail.params = tp;
        root.children = {clear, lyr, trail};
        return root;
    };
    const auto starte = [](MultiEffectVisualizer& vis, ChainNode root) {
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(10);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
    };
    LyricsParams lp;
    lp.fontHeight = -24;
    lp.previewLines = 0;

    {
        // synchron: vier Zeilen, die dritte ist eine Pause (Zeit ohne Text)
        MultiEffectVisualizer::setSongtext(
            ly::leseLrc("[00:10.00]AAAA\n[00:13.00]BBBB\n[00:16.00]\n[00:19.00]CCCC\n"));
        LyricsTrailParams tp;
        tp.initCode = "n = 0;";
        tp.lineCode = "n = n + 1; xshift = line; yshift = -n;";
        tp.frameCode = "holdms = 100 + linecount;";  // linecount steht auch in Frame bereit
        MultiEffectVisualizer vis;
        starte(vis, kette(lp, tp));
        const auto bis = [&](int vonMs, int bisMs) {
            for (int ms = vonMs; ms <= bisMs; ms += 1000 / 60)
            {
                MultiEffectVisualizer::notifyPlayPosition(ms);
                vis.render(1.0f / 60.0f);
            }
        };
        bis(9500, 12000);  // die erste Zeile laeuft: noch ging keine — Line ruht
        CHECK(gesehen.xShift == 0);
        CHECK(gesehen.yShift == -8);
        CHECK(gesehen.holdMs == 104);
        bis(12000, 14000);  // Zeile 1 ging
        CHECK(gesehen.xShift == 1);
        CHECK(gesehen.yShift == -1);
        bis(14000, 17000);  // Zeile 2 ging (die Pause beginnt)
        CHECK(gesehen.xShift == 2);
        CHECK(gesehen.yShift == -2);
        bis(17000, 20000);  // die Pause endet: nichts ging — Line laeuft nicht noch einmal
        CHECK(gesehen.xShift == 2);
        CHECK(gesehen.yShift == -2);
    }
    {
        // die beiden Beispiele aus dem Benutzerhandbuch (S90: kein ungeprueftes
        // Beispiel mehr ins Handbuch)
        LyricsTrailParams tp;
        tp.lineCode = "xshift = rand(21) - 10; yshift = -6 - rand(8); "
                      "blendstrength = if(line % 2, 100, 50);";
        MultiEffectVisualizer vis;
        starte(vis, kette(lp, tp));
        const auto bis = [&](int vonMs, int bisMs) {
            for (int ms = vonMs; ms <= bisMs; ms += 1000 / 60)
            {
                MultiEffectVisualizer::notifyPlayPosition(ms);
                vis.render(1.0f / 60.0f);
            }
        };
        bis(12500, 14000);  // Zeile 1 ging: ungerade — volle Staerke
        CHECK(gesehen.blendStrength == 100);
        CHECK(gesehen.xShift >= -10);
        CHECK(gesehen.xShift <= 10);
        CHECK(gesehen.yShift <= -6);
        CHECK(gesehen.yShift >= -13);
        bis(14000, 17000);  // Zeile 2 ging: gerade — halbe Staerke
        CHECK(gesehen.blendStrength == 50);
    }
    {
        // Gegenprobe: ein Skript ohne Feld Line (der Beobachter sieht nur
        // Knoten mit Skript) — der Versatz bleibt, wie er am Knoten steht
        gesehen = LyricsTrailParams{};
        LyricsTrailParams tp;
        tp.xShift = 7;
        tp.frameCode = "holdms = 111;";
        MultiEffectVisualizer vis;
        starte(vis, kette(lp, tp));
        for (int ms = 12500; ms <= 14000; ms += 1000 / 60)
        {
            MultiEffectVisualizer::notifyPlayPosition(ms);
            vis.render(1.0f / 60.0f);
        }
        CHECK(gesehen.holdMs == 111);
        CHECK(gesehen.xShift == 7);
    }
    {
        // unsynchron: der Lyrics-Knoten rueckt per linetrigger; der Trail sieht,
        // welche Zeile er verlassen hat
        MultiEffectVisualizer::setSongtext(ly::leseLrc("AAAA\nBBBB\nCCCC\n"));
        LyricsParams rueckt = lp;
        rueckt.frameCode = "linetrigger = b;";  // Beats auf Frame 0, 10, 20, …
        LyricsTrailParams tp;
        tp.lineCode = "xshift = line;";
        MultiEffectVisualizer vis;
        starte(vis, kette(rueckt, tp));
        for (int i = 0; i < 5; ++i) vis.render(1.0f / 60.0f);
        CHECK(gesehen.xShift == 1);  // ein Beat: Zeile 1 ging
        for (int i = 0; i < 10; ++i) vis.render(1.0f / 60.0f);
        CHECK(gesehen.xShift == 2);  // zweiter Beat: Zeile 2 ging
    }

    MultiEffectVisualizer::setzeParamBeobachter({});
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90, LB1 (Zeilen-Puffer): der Trail legt das Bild auf, das STAND — nicht
// eine neu gezeichnete Fassung der alten Zeile. Sichtbar am Wort-Stand: das
// zweite Wort der ersten Zeile ist beim Zeilenwechsel noch nicht gesungen
// (dunkel); bis S89 zeichnete der Trail die ganze Zeile hell.
TEST_CASE("Lyrics Trail S90 LB1: der Trail uebernimmt das Zeilenbild mit Wort-Stand und Lage")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);

    struct Bild
    {
        int hell = 0;     ///< helle Pixel im Band
        double y = 0.0;   ///< ihr Schwerpunkt
    };
    const auto band = [](const QImage& img, int yVon, int yBis) {
        Bild b;
        double summe = 0.0;
        for (int y = yVon; y < yBis; ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 200)
                {
                    ++b.hell;
                    summe += y;
                }
        if (b.hell > 0) b.y = summe / b.hell;
        return b;
    };
    // Ein Visualizer laeuft von `vonMs` in Schritten von 1/60 s bis `bisMs` —
    // der Lyrics-Knoten zeichnet die erste Zeile also wirklich, bevor sie endet.
    const auto lauf = [](const char* lrc, const LyricsTrailParams& tp, bool ebene, int vonMs,
                         int bisMs, bool trailZuerst = false) {
        MultiEffectVisualizer::setSongtext(ly::leseLrc(lrc));
        MultiEffectVisualizer::setKaraokeEbene(ebene);
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        root.children = {clear};
        if (!ebene)
        {
            ChainNode lyr;
            LyricsParams lp;
            lp.fontHeight = -24;
            lp.outline = false;
            lp.previewLines = 0;
            lp.colorNext = 0x202020;  // nicht Gesungenes dunkel
            lyr.params = lp;
            ChainNode trail;
            trail.params = tp;
            if (trailZuerst) root.children.push_back(trail);
            root.children.push_back(lyr);
            if (!trailZuerst) root.children.push_back(trail);
        }
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        for (int ms = vonMs; ms <= bisMs; ms += 1000 / 60)
        {
            MultiEffectVisualizer::notifyPlayPosition(ms);
            vis.render(1.0f / 60.0f);
        }
        const QImage img = ebene ? vis.debugGrabMitKaraoke() : vis.debugGrabRootSurface();
        MultiEffectVisualizer::setKaraokeEbene(false);
        return img;
    };
    // das zweite Wort beginnt erst NACH dem Zeilenende: es bleibt ungesungen
    const char* halb = "[00:10.00]MMMM <00:14.00>MMMM\n[00:13.00]IIII\n";
    const char* voll = "[00:10.00]MMMM MMMM\n[00:13.00]IIII\n";
    LyricsTrailParams tp;
    tp.yShift = -30;  // 72 px ueber der Zeile: ein eigenes Band
    tp.holdMs = 1000;

    // vor dem Wechsel: die Zeile steht unten, oben ist nichts
    const QImage vorher = lauf(voll, tp, false, 12800, 12950);
    const Bild zeile = band(vorher, 120, 240);
    REQUIRE(zeile.hell > 0);
    CHECK(band(vorher, 0, 120).hell == 0);

    // nach dem Wechsel, ganze Zeile gesungen: der Trail zeigt sie ganz, 72 px hoeher
    const Bild ganz = band(lauf(voll, tp, false, 12800, 13100), 0, 150);
    REQUIRE(ganz.hell > 0);
    CHECK(ganz.hell == doctest::Approx(zeile.hell).epsilon(0.03));
    CHECK(ganz.y == doctest::Approx(zeile.y - 72.0).epsilon(0.02));

    // Wort-Stand: nur das erste Wort war gesungen — der Trail zeigt genau das
    const Bild halbB = band(lauf(halb, tp, false, 12800, 13100), 0, 150);
    CHECK(halbB.hell > ganz.hell * 0.3);
    CHECK(halbB.hell < ganz.hell * 0.7);
    // … auch wenn der Trail in der Kette VOR dem Lyrics-Knoten steht
    const Bild davor = band(lauf(halb, tp, false, 12800, 13100, true), 0, 150);
    CHECK(davor.hell == halbB.hell);
    // Gegenprobe: ein frischer Visualizer hat die Zeile nie gezeichnet — ohne
    // Zeilenbild zeichnet der Trail wie bis S89 neu, die ganze Zeile hell
    const Bild frisch = band(lauf(halb, tp, false, 13100, 13120), 0, 150);
    CHECK(frisch.hell == doctest::Approx(ganz.hell).epsilon(0.03));
    // eigene Farbe: immer neu gezeichnet, einfarbig (Entscheid L1 a)
    LyricsTrailParams eigen = tp;
    eigen.useOwnColor = true;
    eigen.color = 0xFFFFFF;
    const Bild eigenB = band(lauf(halb, eigen, false, 12800, 13100), 0, 150);
    CHECK(eigenB.hell == doctest::Approx(ganz.hell).epsilon(0.03));
    // die Haltezeit gilt weiter
    CHECK(band(lauf(halb, tp, false, 12800, 14200), 0, 150).hell == 0);

    // Karaoke-Ebene: auch dort traegt der Trail den Wort-Stand — mit halb
    // gesungener Zeile ist weniger hell im Bild als mit ganz gesungener
    const QImage ebeneHalb = lauf(halb, tp, true, 12800, 13060);
    const QImage ebeneVoll = lauf(voll, tp, true, 12800, 13060);
    const int hH = band(ebeneHalb, 0, 240).hell;
    const int hV = band(ebeneVoll, 0, 240).hell;
    MESSAGE("Ebene: hell mit halber Zeile " << hH << ", mit ganzer " << hV);
    if (const QString schnapp = qEnvironmentVariable("LUMIVIZ_LB1_SNAP"); !schnapp.isEmpty())
    {
        ebeneHalb.save(schnapp + "/trail_ebene_halb.png");
        ebeneVoll.save(schnapp + "/trail_ebene_voll.png");
        lauf(halb, tp, false, 12800, 13100).save(schnapp + "/trail_kette_halb.png");
        lauf(voll, tp, false, 12800, 13100).save(schnapp + "/trail_kette_voll.png");
    }
    CHECK(hH > 0);
    CHECK(hH < hV * 0.9);

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90 (Befund Patrik): mit Skript zeigt die Karaoke-Ebene beim Zeilenwechsel
// kurz eine andere Fassung als danach — in der Kette nicht. Der Test faehrt
// dasselbe Skript in beiden ueber einen Zeilenwechsel und vergleicht die
// Schrifthoehe Frame fuer Frame.
TEST_CASE("Lyrics S90: Skript am Lyrics-Knoten — Kette und Karaoke-Ebene laufen gleich")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeAblage(QString());
    MultiEffectVisualizer::setSongtext(
        ly::leseLrc("[00:10.00]MMMM\n[00:10.50]HHHH\n[00:11.00]MMMM HH\n[00:20.00]ende\n"));

    // Hoehe des hellen Bereichs je Frame (0 = nichts): die Schriftgroesse
    // Vorgabe: das Skript aus Patriks Karaoke2.lvkar
    const auto folge = [](bool ebene, const char* init = "reffont = fontheight;",
                          const char* frame = "fontheight= reffont;",
                          const char* beat = "fontheight= 2 * reffont;") {
        LyricsParams lp;
        lp.fontHeight = -20;
        lp.outline = false;
        lp.previewLines = 0;
        lp.vAlign = 1;
        lp.yShift = 0;
        lp.initCode = init;
        lp.frameCode = frame;
        lp.beatCode = beat;
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        root.children = {clear};
        if (!ebene)
        {
            ChainNode lyr;
            lyr.params = lp;
            root.children.push_back(lyr);
        }
        MultiEffectVisualizer::setKaraokeEbene(ebene);
        MultiEffectVisualizer vis;
        if (ebene)
        {
            vis.karaokeKette().children[0].params = lp;
            vis.karaokeKette().children[1].enabled = false;  // ohne Trail: nur die Zeile
            (void)vis.recompileKaraoke();
        }
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(12);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        std::vector<int> hoehen;
        for (int frame = 0; frame < 90; ++frame)
        {
            MultiEffectVisualizer::notifyPlayPosition(10000 + frame * 1000 / 60);
            vis.render(1.0f / 60.0f);
            const QImage img = ebene ? vis.debugGrabMitKaraoke() : vis.debugGrabRootSurface();
            int oben = -1, unten = -1;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qGray(img.pixel(x, y)) > 200)
                    {
                        if (oben < 0) oben = y;
                        unten = y;
                        break;
                    }
            hoehen.push_back(oben < 0 ? 0 : unten - oben + 1);
        }
        MultiEffectVisualizer::setKaraokeEbene(false);
        return hoehen;
    };
    const auto text = [](const std::vector<int>& v) {
        std::string s;
        for (const int h : v) s += std::to_string(h) + ' ';
        return s;
    };
    const std::vector<int> kette = folge(false);
    const std::vector<int> ebene = folge(true);
    MESSAGE("Kette: " << text(kette));
    MESSAGE("Ebene: " << text(ebene));
    CHECK(kette == ebene);
    // Entscheid B2 a / B1 b: Groesse, Lage und Farben folgen dem Skript aus
    // JEDEM Feld (die Grafikkarte setzt die Zeile aus Wort-Masken zusammen,
    // gerastert wird dafuer nichts). Das Skript tut damit, was es sagt: auf
    // jeden Beat (alle 12 Frames) ein Frame in doppelter Groesse, sonst die
    // Grundgroesse — und am Zeilenwechsel (Frame 30, 60) aendert sich nichts.
    REQUIRE(kette.size() == 90);
    const int klein = kette[5];
    const int gross = kette[12];
    CHECK(klein > 8);
    CHECK(gross > klein * 1.7);
    for (std::size_t i = 1; i < kette.size(); ++i)
    {
        CAPTURE(i);
        CHECK(kette[i] == (i % 12 == 0 ? gross : klein));
    }

    // Das Puls-Beispiel aus dem Benutzerhandbuch: auf den Beat gross, dann
    // klingt die Groesse von Frame zu Frame ab
    const std::vector<int> puls =
        folge(false, "ref = fontheight; p = 0;",
              "p = if(b, 1, p * 0.93); fontheight = ref * (1 + p);", "");
    MESSAGE("Puls: " << text(puls));
    // S92 (Sichttest E1): 0.85 klang zu schnell ab — mit 0.93 ist der Puls
    // kurz vor dem naechsten Beat noch zu sehen, mit 0.85 schon weg
    const std::vector<int> pulsAlt =
        folge(false, "ref = fontheight; p = 0;",
              "p = if(b, 1, p * 0.85); fontheight = ref * (1 + p);", "");
    CHECK(puls[18] > pulsAlt[18]);
    // Beat-Frame deutlich groesser als der Frame davor. Der Test schlaegt alle
    // 12 Frames (300 BPM) — mit 0.93 ist der Puls davor erst halb abgeklungen,
    // deshalb 1.2 statt der 1.5, die fuer 0.85 galten
    CHECK(puls[12] > puls[11] * 1.2);
    CHECK(puls[13] <= puls[12]);
    CHECK(puls[16] < puls[12]);
    CHECK(puls[23] < puls[16]);        // klingt weiter ab
    CHECK(puls[23] >= klein);          // nie unter die Grundgroesse

    // Gegenprobe: was das Feld LINE setzt, gilt beim Zeilenwechsel weiter —
    // auch wenn ein Frame-Feld danach dieselbe Variable ueberschreibt
    const auto mitLine = [](const char* frameCode) {
        LyricsParams lp;
        lp.fontHeight = -20;
        lp.outline = false;
        lp.previewLines = 0;
        lp.vAlign = 1;
        lp.yShift = 0;
        lp.lineCode = "fontheight = 20 + 20 * (line % 2);";  // Zeile 1: 40, Zeile 2: 20, Zeile 3: 40
        lp.frameCode = frameCode;
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        root.children = {clear, lyr};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        std::vector<int> hoehen;
        for (const int ms : {10100, 10200, 10600, 10700, 11100, 11200})
        {
            MultiEffectVisualizer::notifyPlayPosition(ms);
            vis.render(1.0f / 60.0f);
            const QImage img = vis.debugGrabRootSurface();
            int oben = -1, unten = -1;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (qGray(img.pixel(x, y)) > 200)
                    {
                        if (oben < 0) oben = y;
                        unten = y;
                        break;
                    }
            hoehen.push_back(oben < 0 ? 0 : unten - oben + 1);
        }
        return hoehen;
    };
    const std::vector<int> nurLine = mitLine("");
    MESSAGE("Line allein: " << text(nurLine));
    REQUIRE(nurLine.size() == 6);
    CHECK(nurLine[1] > nurLine[3] * 1.6);  // Zeile 1 gross, Zeile 2 klein
    CHECK(nurLine[5] == nurLine[1]);       // Zeile 3 wieder gross
    CHECK(nurLine[0] == nurLine[1]);       // innerhalb der Zeile ruhig
    // Ein Frame-Feld, das dieselbe Variable ueberschreibt, gewinnt jetzt in
    // jedem Frame (es laeuft nach Line) — die Schrift bleibt bei seinen 7 px.
    const std::vector<int> mitFrame = mitLine("fontheight = 7;");
    MESSAGE("Line + Frame: " << text(mitFrame));
    REQUIRE(mitFrame.size() == 6);
    for (std::size_t i = 0; i < mitFrame.size(); ++i)
    {
        CAPTURE(i);
        CHECK(mitFrame[i] > 0);
        CHECK(mitFrame[i] < nurLine[3]);
    }

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90, LB1b (Entscheid L4): die Vorschau-Zeilen eigens gestalten — Groesse,
// Gewicht, Lage relativ zur laufenden Zeile, dazu der Zeilenabstand. Die
// laufende Zeile bleibt, wo sie ist.
TEST_CASE("Lyrics S90 LB1b: Vorschau-Zeilen — Groesse, Gewicht, Lage, Zeilenabstand")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM\n[00:13.00]HHHH\n"));
    MultiEffectVisualizer::notifyPlayPosition(12000);

    struct Bild
    {
        int hell = 0;
        double x = 0.0;
        double y = 0.0;
    };
    // `vorschauHell`: die Vorschau-Zeile ist hell und die laufende dunkel —
    // oder umgekehrt. So laesst sich jede der beiden allein messen.
    const auto miss = [](LyricsParams lp, bool vorschauHell) {
        lp.fontHeight = -24;
        lp.outline = false;
        lp.previewLines = 1;
        lp.vAlign = 0;  // oben: die laufende Zeile ist der feste Punkt
        lp.yShift = 10;
        lp.color = vorschauHell ? 0x202020 : 0xFFFFFF;
        lp.colorNext = vorschauHell ? 0xFFFFFF : 0x202020;
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        root.children = {clear, lyr};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        vis.render(1.0f / 60.0f);
        vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        Bild b;
        double sx = 0.0, sy = 0.0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 200)
                {
                    ++b.hell;
                    sx += x;
                    sy += y;
                }
        if (b.hell > 0)
        {
            b.x = sx / b.hell;
            b.y = sy / b.hell;
        }
        return b;
    };

    const LyricsParams vorgabe;
    const Bild zeile = miss(vorgabe, false);
    const Bild vorschau = miss(vorgabe, true);
    REQUIRE(zeile.hell > 0);
    REQUIRE(vorschau.hell > 0);
    const double abstand = vorschau.y - zeile.y;
    CHECK(abstand > 20.0);  // die Vorschau steht eine Zeile tiefer

    // Groesse: halbe Schrift = etwa ein Viertel der Flaeche; die laufende bleibt
    LyricsParams klein;
    klein.previewScale = 50;
    CHECK(miss(klein, true).hell < vorschau.hell * 0.4);
    CHECK(miss(klein, true).hell > vorschau.hell * 0.1);
    CHECK(miss(klein, false).hell == zeile.hell);
    CHECK(miss(klein, false).y == doctest::Approx(zeile.y));
    LyricsParams gross;
    gross.previewScale = 150;
    CHECK(miss(gross, true).hell > vorschau.hell * 1.7);

    // Gewicht: normal statt fett — duenner, also weniger helle Pixel
    LyricsParams duenn;
    duenn.previewWeight = 400;
    CHECK(miss(duenn, true).hell < vorschau.hell * 0.95);
    CHECK(miss(duenn, false).hell == zeile.hell);  // die laufende bleibt fett
    LyricsParams fett;
    fett.previewWeight = 700;  // wie die Zeile selbst: nichts aendert sich
    CHECK(miss(fett, true).hell == vorschau.hell);

    // Lage: 10 % von 320 px nach rechts, 20 % von 240 px nach unten
    LyricsParams lage;
    lage.previewXShift = 10;
    lage.previewYShift = 20;
    const Bild verschoben = miss(lage, true);
    CHECK(verschoben.hell == vorschau.hell);
    CHECK(verschoben.x == doctest::Approx(vorschau.x + 32.0).epsilon(0.01));
    CHECK(verschoben.y == doctest::Approx(vorschau.y + 48.0).epsilon(0.01));
    CHECK(miss(lage, false).x == doctest::Approx(zeile.x));
    CHECK(miss(lage, false).y == doctest::Approx(zeile.y));

    // Zeilenabstand: doppelt so weit (230 statt 115 %)
    LyricsParams weit;
    weit.lineSpacing = 230;
    CHECK(miss(weit, true).y - miss(weit, false).y ==
          doctest::Approx(2.0 * abstand).epsilon(0.05));

    // per Skript: die Felder sind Skriptvariablen (Init, weil sie neu rastern)
    LyricsParams skript;
    skript.initCode = "previewscale = 50; previewyshift = 20;";
    const Bild s = miss(skript, true);
    LyricsParams beides = klein;
    beides.previewYShift = 20;
    const Bild f = miss(beides, true);
    // (mit Skript setzt die Grafikkarte die Zeile aus Masken zusammen, ohne
    // rastert die CPU — die Kanten weichen um Bruchteile eines Pixels ab)
    MESSAGE("Vorschau 50 %: helle Pixel per Skript (Masken) " << s.hell << ", per Feld (CPU) " << f.hell);
    CHECK(s.hell == doctest::Approx(f.hell).epsilon(0.3));
    CHECK(s.y == doctest::Approx(f.y).epsilon(0.01));

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90, LB2 (Entscheide L2 b mit Fliessen, L3 a): das laufende Wort — eigene
// Farbe, eigene Groesse, zwei Arten zu sitzen (Reflow / Overlay), die Groesse
// gleitet. Gemessen ueber drei reine Farben: gesungen rot, laufend gruen,
// noch nicht blau.
TEST_CASE("Lyrics S90 LB2: das laufende Wort — Farbe, Groesse, Reflow/Overlay, Fliessen")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    const char* mitWorten = "[00:10.00]MMMM <00:11.00>HHHH <00:12.00>MMMM\n[00:14.00]ende\n";

    struct Teil
    {
        int n = 0;
        int x0 = 1 << 20, x1 = -1, y0 = 1 << 20, y1 = -1;
        [[nodiscard]] int hoehe() const { return n > 0 ? y1 - y0 + 1 : 0; }
    };
    struct Bild
    {
        Teil rot, gruen, blau;
    };
    const auto werte = [](const QImage& img) {
        Bild b;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
            {
                const QRgb c = img.pixel(x, y);
                Teil* t = qRed(c) > 200 && qGreen(c) < 60 && qBlue(c) < 60     ? &b.rot
                          : qGreen(c) > 200 && qRed(c) < 60 && qBlue(c) < 60   ? &b.gruen
                          : qBlue(c) > 200 && qRed(c) < 60 && qGreen(c) < 60   ? &b.blau
                                                                               : nullptr;
                if (t == nullptr) continue;
                ++t->n;
                t->x0 = std::min(t->x0, x);
                t->x1 = std::max(t->x1, x);
                t->y0 = std::min(t->y0, y);
                t->y1 = std::max(t->y1, y);
            }
        return b;
    };
    const auto knoten = [](LyricsParams lp) {
        lp.fontHeight = -24;
        lp.outline = false;
        lp.previewLines = 0;
        lp.vAlign = 1;
        lp.yShift = 0;
        lp.color = 0xFF0000;
        lp.colorWord = 0x00FF00;
        lp.colorNext = 0x0000FF;
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        root.children = {clear, lyr};
        return root;
    };
    // ein Visualizer von `vonMs` bis `bisMs` in Schritten von 1/60 s
    const auto lauf = [&](const LyricsParams& lp, int vonMs, int bisMs, QSize groesse = QSize(480, 240)) {
        MultiEffectVisualizer vis;
        vis.setChain(knoten(lp));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(groesse);
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, groesse.width(),
                                                                 groesse.height());
        for (int ms = vonMs; ms <= bisMs; ms += 1000 / 60)
        {
            MultiEffectVisualizer::notifyPlayPosition(ms);
            vis.render(1.0f / 60.0f);
        }
        return vis.debugGrabRootSurface();
    };
    const QString schnapp = qEnvironmentVariable("LUMIVIZ_LB1_SNAP");
    const auto merke = [&](const QImage& img, const char* name) {
        if (!schnapp.isEmpty()) img.save(schnapp + '/' + QLatin1String(name) + ".png");
        return img;
    };

    MultiEffectVisualizer::setSongtext(ly::leseLrc(mitWorten));

    // --- Vorgaben: zwei Zustaende wie bisher, kein „gerade jetzt"
    const Bild aus = werte(lauf(LyricsParams{}, 11400, 11500));
    CHECK(aus.rot.n > 0);
    CHECK(aus.gruen.n == 0);
    CHECK(aus.blau.n > 0);

    // --- eigene Farbe: das zweite Wort laeuft (seit 11,0 s) — zwischen den beiden anderen
    LyricsParams farbig;
    farbig.useWordColor = true;
    farbig.wordFlowMs = 0;
    const Bild f = werte(merke(lauf(farbig, 11400, 11500), "lb2_farbe"));
    REQUIRE(f.rot.n > 0);
    REQUIRE(f.gruen.n > 0);
    REQUIRE(f.blau.n > 0);
    CHECK(f.rot.x1 < f.gruen.x0);
    CHECK(f.gruen.x1 < f.blau.x0);
    CHECK(f.gruen.hoehe() == f.rot.hoehe());  // gleiche Groesse
    // vor dem ersten Wort gibt es kein laufendes … (die Zeile beginnt mit dem ersten Wort)
    const Bild erstes = werte(lauf(farbig, 10400, 10500));
    CHECK(erstes.rot.n == 0);
    CHECK(erstes.gruen.n > 0);
    CHECK(erstes.gruen.x1 < erstes.blau.x0);

    // --- Groesse, Reflow: das Wort ist doppelt so hoch, die Nachbarn ruecken zur Seite
    LyricsParams gross = farbig;
    gross.wordScale = 200;
    const Bild r = werte(merke(lauf(gross, 11400, 11500), "lb2_reflow"));
    REQUIRE(r.gruen.n > 0);
    CHECK(r.gruen.hoehe() == doctest::Approx(2.0 * f.gruen.hoehe()).epsilon(0.12));
    CHECK(r.rot.x0 < f.rot.x0 - 10);    // nach links gerueckt
    CHECK(r.blau.x1 > f.blau.x1 + 10);  // nach rechts gerueckt
    CHECK(r.rot.hoehe() == f.rot.hoehe());  // die Nachbarn behalten ihre Groesse
    CHECK(r.rot.x1 < r.gruen.x0);       // nichts ueberdeckt sich
    CHECK(r.gruen.x1 < r.blau.x0);

    // --- Groesse, Overlay: die Zeile bleibt stehen, das Wort waechst um seine Mitte
    LyricsParams ueber = gross;
    ueber.wordLayout = 1;
    const Bild o = werte(merke(lauf(ueber, 11400, 11500), "lb2_overlay"));
    REQUIRE(o.gruen.n > 0);
    CHECK(o.gruen.hoehe() == r.gruen.hoehe());
    CHECK(o.rot.x0 == f.rot.x0);
    CHECK(o.blau.x1 == f.blau.x1);
    CHECK((o.gruen.x0 + o.gruen.x1) / 2 ==
          doctest::Approx((f.gruen.x0 + f.gruen.x1) / 2).epsilon(0.01));
    CHECK(o.gruen.x0 < f.gruen.x0);  // breiter als der Platz: liegt ueber den Nachbarn

    // --- Fliessen: gleich nach dem Wortwechsel noch klein, spaeter am Ziel, dazwischen wachsend
    LyricsParams fliesst = gross;
    fliesst.wordFlowMs = 300;
    const int kurz = werte(lauf(fliesst, 10900, 11040)).gruen.hoehe();
    const int mitte = werte(lauf(fliesst, 10900, 11250)).gruen.hoehe();
    const int ziel = werte(lauf(fliesst, 10900, 13900)).gruen.hoehe();  // drittes Wort seit 12,0 s
    MESSAGE("Fliessen: Hoehe nach 40 ms " << kurz << ", nach 250 ms " << mitte << ", am Ziel "
                                          << ziel << " (hart: " << r.gruen.hoehe() << ")");
    CHECK(kurz < mitte);
    CHECK(mitte < ziel);
    CHECK(kurz < r.gruen.hoehe() * 0.75);
    CHECK(ziel == r.gruen.hoehe());
    // das vorige Wort schrumpft waehrenddessen: 40 ms nach dem Wechsel ist Rot noch groesser als normal
    CHECK(werte(lauf(fliesst, 10900, 11040)).rot.hoehe() > f.rot.hoehe());

    // --- ohne Wort-Zeiten kein laufendes Wort (Entscheid L3 a)
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM HHHH MMMM\n[00:14.00]ende\n"));
    const Bild ohne = werte(lauf(gross, 11400, 11500));
    CHECK(ohne.gruen.n == 0);
    CHECK(ohne.rot.n > 0);

    // --- Rasterzeit (Messung, kein Urteil): 1920x1080, die Zeile rastert in
    // jedem Frame neu (Fliessen ueber 2 s) gegen eine ruhende Zeile
    MultiEffectVisualizer::setSongtext(ly::leseLrc(mitWorten));
    const auto zeit = [&](const LyricsParams& lp, bool kontur = true) {
        LyricsParams gr = lp;
        MultiEffectVisualizer vis;
        ChainNode kette = knoten(gr);
        std::get<LyricsParams>(kette.children[1].params).fontHeight = -96;
        std::get<LyricsParams>(kette.children[1].params).outline = kontur;
        vis.setChain(std::move(kette));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(1920, 1080));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 1920, 1080);
        MultiEffectVisualizer::notifyPlayPosition(10990);
        vis.render(1.0f / 60.0f);
        QElapsedTimer uhr;
        uhr.start();
        for (int i = 0; i < 60; ++i)
        {
            MultiEffectVisualizer::notifyPlayPosition(11000 + i * 1000 / 60);
            vis.render(1.0f / 60.0f);
        }
        QOpenGLContext::currentContext()->functions()->glFinish();
        return static_cast<double>(uhr.nsecsElapsed()) / 60.0e6;
    };
    LyricsParams ruht = gross;       // hart: nach dem ersten Frame steht die Zeile
    LyricsParams gleitet = gross;
    gleitet.wordFlowMs = 2000;       // gleitet die ganze Messung ueber
    const double msRuht = zeit(ruht);
    const double msGleitet = zeit(gleitet);
    MESSAGE("Rasterzeit 1920x1080, Schrift 96 px mit Kontur: ruhend " << msRuht
            << " ms/Frame, gleitend (Rastern je Frame) " << msGleitet << " ms/Frame; ohne Kontur gleitend "
            << zeit(gleitet, false) << " ms/Frame");
    CHECK(msGleitet > 0.0);

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90, LB3: das laufende Wort im Skript — `word`, `wordcount` und der Slot
// „Word", der einmal je Wortwechsel laeuft (nach Line, vor Frame). Ein
// laufendes Wort gibt es nur mit echten Wort-Zeiten (Entscheid L3 a).
TEST_CASE("Lyrics S90 LB3: Slot Word laeuft einmal je Wortwechsel, kennt word/wordcount")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    // Zeile 1: drei Worte mit Zeiten; Zeile 2: ohne Wort-Zeiten; Zeile 3: zwei Worte
    MultiEffectVisualizer::setSongtext(ly::leseLrc(
        "[00:10.00]MMMM <00:11.00>HHHH <00:12.00>MMMM\n[00:13.00]eins zwei drei vier\n"
        "[00:16.00]AAAA <00:17.00>BBBB\n"));

    std::map<std::string, LyricsParams> gesehen;
    MultiEffectVisualizer::setzeParamBeobachter([&](const ChainNode& n, const EffectParams& p) {
        if (const auto* lp = std::get_if<LyricsParams>(&p))
            gesehen[n.displayName.empty() ? std::string("Lyrics") : n.displayName] = *lp;
    });
    const auto kette = [](const LyricsParams& lp, bool skripte = true) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        lyr.displayName = "Zeile";
        lyr.scriptsActive = skripte;
        root.children = {clear, lyr};
        return root;
    };
    const auto frames = [](MultiEffectVisualizer& vis, int posMs, int n = 8) {
        MultiEffectVisualizer::notifyPlayPosition(posMs);
        for (int i = 0; i < n; ++i) vis.render(1.0f / 60.0f);
    };
    const auto starte = [](MultiEffectVisualizer& vis, ChainNode root) {
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);  // kein Beat: Strukturfelder kommen nur ueber Word
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
    };

    {
        // c zaehlt die Laeufe des Slots (xshift); yshift traegt das Wort, previewlines wordcount
        LyricsParams lp;
        lp.initCode = "c=0;";
        lp.wordCode = "c=c+1; xshift=c; yshift=-word; previewlines=wordcount;";
        MultiEffectVisualizer vis;
        starte(vis, kette(lp));
        frames(vis, 10500);  // Zeile 1, erstes Wort
        REQUIRE(gesehen.count("Zeile") == 1);
        CHECK(gesehen["Zeile"].xShift == 1);  // genau einmal, auch ueber 8 Frames
        CHECK(gesehen["Zeile"].yShift == -1);
        CHECK(gesehen["Zeile"].previewLines == 3);
        frames(vis, 10800);  // dasselbe Wort: kein Lauf
        CHECK(gesehen["Zeile"].xShift == 1);
        frames(vis, 11500);  // zweites Wort
        CHECK(gesehen["Zeile"].xShift == 2);
        CHECK(gesehen["Zeile"].yShift == -2);
        frames(vis, 12500);  // drittes Wort
        CHECK(gesehen["Zeile"].xShift == 3);
        CHECK(gesehen["Zeile"].yShift == -3);
        frames(vis, 14000);  // Zeile 2 hat keine Wort-Zeiten: der Slot ruht, die Werte bleiben
        CHECK(gesehen["Zeile"].xShift == 3);
        frames(vis, 16500);  // Zeile 3, erstes Wort — auch wenn die Wortnummer (1) schon einmal dran war
        CHECK(gesehen["Zeile"].xShift == 4);
        CHECK(gesehen["Zeile"].yShift == -1);
        CHECK(gesehen["Zeile"].previewLines == 2);
        frames(vis, 17500);  // zweites Wort der dritten Zeile
        CHECK(gesehen["Zeile"].xShift == 5);
        CHECK(gesehen["Zeile"].yShift == -2);
    }
    {
        // `word` / `wordcount` in den anderen Feldern; ohne Wort-Zeiten ist word 0
        // und wordcount zaehlt die Worte am Leerraum (hier vier)
        LyricsParams lp;
        lp.lineCode = "xshift = wordcount; yshift = -word;";
        MultiEffectVisualizer vis;
        starte(vis, kette(lp));
        frames(vis, 14000);  // Zeile 2
        CHECK(gesehen["Zeile"].xShift == 4);
        CHECK(gesehen["Zeile"].yShift == 0);
        frames(vis, 16500);  // Zeile 3: zwei Worte, das erste laeuft
        CHECK(gesehen["Zeile"].xShift == 2);
        CHECK(gesehen["Zeile"].yShift == -1);
    }
    {
        // Zeilenwechsel mit Wort-Zeiten: Line laeuft vor Word — Word sieht, was Line gesetzt hat
        LyricsParams lp;
        lp.lineCode = "xshift = 10;";
        lp.wordCode = "xshift = xshift + word;";
        MultiEffectVisualizer vis;
        starte(vis, kette(lp));
        frames(vis, 10500);
        CHECK(gesehen["Zeile"].xShift == 11);  // 10 von Line, +1 von Word
        frames(vis, 11500);
        CHECK(gesehen["Zeile"].xShift == 13);  // Word allein: +2
    }
    {
        // Felder, die die Masken neu rastern (Konturbreite, Gewicht, kursiv),
        // folgen dem Skript weiter nur aus Init, Song, Beat, Line und Word:
        // ein Frame-Feld schlaegt beim Wortwechsel nicht durch (B1 a). Lage,
        // Groesse und Farben dagegen folgen jedem Feld (B2 a).
        LyricsParams lp;
        lp.wordCode = "yshift = -word;";
        lp.frameCode = "outlinesize = 3 + word; xshift = 20 + word;";
        MultiEffectVisualizer vis;
        starte(vis, kette(lp));
        frames(vis, 10500);
        const int start = gesehen["Zeile"].outlineSize;  // der erste Frame ist frei: 4
        CHECK(start == 4);
        CHECK(gesehen["Zeile"].xShift == 21);
        frames(vis, 11500);
        CHECK(gesehen["Zeile"].outlineSize == start);  // nicht 5
        CHECK(gesehen["Zeile"].xShift == 22);          // Lage folgt dem Frame-Feld
        CHECK(gesehen["Zeile"].yShift == -2);          // Word selbst wirkt
    }
    {
        // „Script active" aus: auch Word ruht
        gesehen.clear();
        LyricsParams lp;
        lp.wordCode = "xshift=50;";
        MultiEffectVisualizer vis;
        starte(vis, kette(lp, false));
        frames(vis, 10500);
        CHECK(gesehen.count("Zeile") == 0);
    }
    {
        // das Feld ueberlebt Speichern und Laden
        LyricsParams lp;
        lp.wordCode = "xshift=word;";
        QStringList report;
        const ChainNode zurueck = nodeFromJson(nodeToJson(kette(lp)), &report);
        REQUIRE(zurueck.children.size() == 2);
        CHECK(std::get<LyricsParams>(zurueck.children[1].params).wordCode == "xshift=word;");
    }
    // Worte zaehlen (Modell)
    CHECK(ly::wortAnzahl(ly::Zeile{0, "eins  zwei\tdrei ", ly::Stimme::Keine, {}}) == 3);
    CHECK(ly::wortAnzahl(ly::Zeile{0, "", ly::Stimme::Keine, {}}) == 0);

    MultiEffectVisualizer::setzeParamBeobachter({});
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90 (Nebenbefund beim Umbau des Text-Shaders): der AVI-Knoten zeichnet
// ueber denselben Shader wie Text und Lyrics. Seit S89 traegt der Shader eine
// Staerke (`uStrength`) — der AVI-Pfad setzte sie nicht. Der Test zaehlt, was
// daraus folgt: ein AVI-Bild allein, und eines hinter einem Lyrics-Knoten mit
// Staerke 0, muessen so hell sein wie eines hinter einem mit Staerke 100.
TEST_CASE("AVI S90: das Bild haengt nicht an der Staerke eines Text- oder Lyrics-Knotens davor")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    // das Testvideo liegt im Repo: vom Quelltext aus nach oben suchen
    std::filesystem::path video;
    for (std::filesystem::path p = std::filesystem::path(__FILE__).parent_path(); !p.empty();
         p = p.parent_path())
    {
        const auto kandidat = p / "asset" / "calibration" / "fields" / "testvideo.avi";
        if (std::filesystem::exists(kandidat))
        {
            video = kandidat;
            break;
        }
        if (p == p.parent_path()) break;
    }
    REQUIRE_FALSE(video.empty());

    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM\n[00:20.00]ende\n"));
    MultiEffectVisualizer::notifyPlayPosition(11000);

    // mittlere Helligkeit des Bilds; `lyricsStaerke` < 0 = kein Lyrics-Knoten davor
    const auto hell = [&](int lyricsStaerke, bool mitAvi) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        root.children = {clear};
        if (lyricsStaerke >= 0)
        {
            ChainNode lyr;
            LyricsParams lp;
            lp.blendStrength = lyricsStaerke;
            lyr.params = lp;
            root.children.push_back(lyr);
        }
        if (mitAvi)
        {
            ChainNode avi;
            AviParams ap;
            ap.filename = video.filename().string();
            ap.resolvedPath = video.string();
            avi.params = ap;
            root.children.push_back(avi);
        }
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(160, 120));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 160, 120);
        for (int i = 0; i < 4; ++i) vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        double summe = 0.0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x) summe += qGray(img.pixel(x, y));
        return summe / (img.width() * img.height());
    };

    const double nachVoll = hell(100, true);  // Lyrics mit voller Staerke, dann das Video
    const double allein = hell(-1, true);
    const double nachNull = hell(0, true);    // Lyrics mit Staerke 0, dann das Video
    MESSAGE("AVI-Helligkeit: nach Lyrics 100 % " << nachVoll << ", allein " << allein
            << ", nach Lyrics 0 % " << nachNull);
    REQUIRE(nachVoll > 5.0);  // das Video ist da (ersetzt das ganze Bild)
    CHECK(allein == doctest::Approx(nachVoll).epsilon(0.001));
    CHECK(nachNull == doctest::Approx(nachVoll).epsilon(0.001));

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90, LB4 (Entscheid L5 a): unsynchronen Text per Skript weiterruecken —
// `linetrigger` eine Zeile, `wordtrigger` ein Wort; beide gelten nur fuer den
// Frame, in dem sie gesetzt werden. Am Zeilenende fuehrt `wordtrigger` in die
// naechste Zeile.
TEST_CASE("Lyrics S90 LB4: linetrigger / wordtrigger ruecken den unsynchronen Text")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    const char* block = "MMMM HHHH MMMM\nAAAA BBBB\nCCCC\n";

    std::map<std::string, LyricsParams> gesehen;
    MultiEffectVisualizer::setzeParamBeobachter([&](const ChainNode& n, const EffectParams& p) {
        if (const auto* lp = std::get_if<LyricsParams>(&p))
            gesehen[n.displayName.empty() ? std::string("Lyrics") : n.displayName] = *lp;
    });
    // Beats auf Frame 0, 10, 20, … — `frames` Frames laufen lassen
    const auto lauf = [&](LyricsParams lp, int frames) {
        lp.fontHeight = -24;
        lp.outline = false;
        lp.previewLines = 0;
        lp.showSyncMark = false;
        lp.vAlign = 1;
        lp.yShift = 0;
        lp.color = 0xFF0000;
        lp.colorWord = 0x00FF00;
        lp.colorNext = 0x0000FF;
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        lyr.displayName = "Zeile";
        root.children = {clear, lyr};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(10);
        vis.initialize();
        vis.resize(QSize(480, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 480, 240);
        for (int i = 0; i < frames; ++i) vis.render(1.0f / 60.0f);
        return vis.debugGrabRootSurface();
    };
    const auto zaehle = [](const QImage& img, int kanal) {  // 0 rot, 1 gruen, 2 blau
        int n = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
            {
                const QRgb c = img.pixel(x, y);
                const int w[3] = {qRed(c), qGreen(c), qBlue(c)};
                if (w[kanal] > 200 && w[(kanal + 1) % 3] < 60 && w[(kanal + 2) % 3] < 60) ++n;
            }
        return n;
    };

    MultiEffectVisualizer::setSongtext(ly::leseLrc(block));
    {
        // linetrigger auf jeden Beat: je Beat genau eine Zeile (Line traegt die Nummer nach xshift)
        LyricsParams lp;
        lp.lineCode = "xshift = line;";
        lp.frameCode = "linetrigger = b;";
        (void)lauf(lp, 5);   // ein Beat (Frame 0)
        CHECK(gesehen["Zeile"].xShift == 2);
        (void)lauf(lp, 15);  // zwei Beats
        CHECK(gesehen["Zeile"].xShift == 3);
        (void)lauf(lp, 45);  // fuenf Beats: am Textende bleibt der Block stehen
        CHECK(gesehen["Zeile"].xShift == 3);
        // Gegenprobe: ohne Ausloeser steht der Block
        LyricsParams steht;
        steht.lineCode = "xshift = line;";
        (void)lauf(steht, 45);
        CHECK(gesehen["Zeile"].xShift == 1);
    }
    {
        // einmal gesetzt rueckt nicht ewig: der Wert gilt nur fuer seinen Frame
        LyricsParams lp;
        lp.lineCode = "xshift = line;";
        lp.initCode = "linetrigger = 1;";
        (void)lauf(lp, 40);
        CHECK(gesehen["Zeile"].xShift == 2);
    }
    {
        // zurueck: zwei Beats vor, der dritte zurueck
        LyricsParams lp;
        lp.lineCode = "xshift = line;";
        lp.initCode = "k = 0;";
        lp.frameCode = "k = k + b; linetrigger = if(b, if(above(k, 2), -1, 1), 0);";
        (void)lauf(lp, 15);
        CHECK(gesehen["Zeile"].xShift == 3);
        (void)lauf(lp, 25);
        CHECK(gesehen["Zeile"].xShift == 2);
    }
    {
        // S92 (Sichttest G1): das Beispiel aus dem Benutzerhandbuch — eine
        // Zeile auf jeden VIERTEN Beat (je Beat ist fuer einen Text zu schnell)
        LyricsParams lp;
        lp.lineCode = "xshift = line;";
        lp.initCode = "n = 0;";
        lp.frameCode = "n = n + b; linetrigger = if(b, equal(n % 4, 0), 0);";
        (void)lauf(lp, 25);  // drei Beats: noch die erste Zeile
        CHECK(gesehen["Zeile"].xShift == 1);
        (void)lauf(lp, 35);  // vierter Beat (Frame 30): zweite Zeile
        CHECK(gesehen["Zeile"].xShift == 2);
        (void)lauf(lp, 65);  // sieben Beats: immer noch die zweite
        CHECK(gesehen["Zeile"].xShift == 2);
        (void)lauf(lp, 75);  // achter Beat (Frame 70): dritte Zeile
        CHECK(gesehen["Zeile"].xShift == 3);
    }
    {
        // S92 (Pruefer test_HandbuchBeispiele): zwei Beispiele aus dem
        // Benutzerhandbuch, die bisher in keinem Test liefen. Feld Line:
        // die Ausrichtung wechselt mit der Zeile (1 -> 1, 2 -> 2, 3 -> 0).
        LyricsParams lp;
        lp.lineCode = "halign = line % 3;";
        lp.frameCode = "linetrigger = b;";
        (void)lauf(lp, 5);   // ein Beat: zweite Zeile
        CHECK(gesehen["Zeile"].hAlign == 2);
        (void)lauf(lp, 15);  // zwei Beats: dritte Zeile
        CHECK(gesehen["Zeile"].hAlign == 0);
        LyricsParams steht;
        steht.lineCode = "halign = line % 3;";
        (void)lauf(steht, 5);  // ohne Ausloeser: erste Zeile
        CHECK(gesehen["Zeile"].hAlign == 1);
        // Feld Word: die Zeile steigt mit jedem Wort
        LyricsParams wp;
        wp.frameCode = "wordtrigger = b;";
        wp.wordCode = "yshift = -6 - word;";
        (void)lauf(wp, 5);   // erstes Wort
        CHECK(gesehen["Zeile"].yShift == -7);
        (void)lauf(wp, 25);  // drittes Wort
        CHECK(gesehen["Zeile"].yShift == -9);
    }
    {
        // wordtrigger auf jeden Beat: Wort fuer Wort, am Zeilenende in die naechste Zeile (L5 a)
        LyricsParams lp;
        lp.frameCode = "wordtrigger = b;";
        lp.wordCode = "yshift = -word; xshift = line; previewlines = wordcount;";
        (void)lauf(lp, 5);   // Beat 1: erstes Wort
        CHECK(gesehen["Zeile"].xShift == 1);
        CHECK(gesehen["Zeile"].yShift == -1);
        CHECK(gesehen["Zeile"].previewLines == 3);
        (void)lauf(lp, 25);  // Beat 3: drittes Wort
        CHECK(gesehen["Zeile"].yShift == -3);
        (void)lauf(lp, 35);  // Beat 4: naechste Zeile, ihr erstes Wort
        CHECK(gesehen["Zeile"].xShift == 2);
        CHECK(gesehen["Zeile"].yShift == -1);
        CHECK(gesehen["Zeile"].previewLines == 2);
        (void)lauf(lp, 55);  // Beat 6: dritte Zeile, erstes Wort
        CHECK(gesehen["Zeile"].xShift == 3);
        CHECK(gesehen["Zeile"].yShift == -1);
        (void)lauf(lp, 95);  // am Textende: das letzte Wort bleibt
        CHECK(gesehen["Zeile"].xShift == 3);
        CHECK(gesehen["Zeile"].yShift == -1);
    }
    {
        // S91: das Beispiel aus dem Benutzerhandbuch — am Zeilenende halten.
        // Gegenprobe ist der Block darueber: `wordtrigger = b;` laeuft in
        // derselben Zeit bis in die dritte Zeile.
        LyricsParams lp;
        lp.frameCode = "wordtrigger = if(below(word, wordcount), b, 0);";
        lp.wordCode = "yshift = -word; xshift = line;";
        (void)lauf(lp, 25);  // Beat 3: drittes (letztes) Wort der ersten Zeile
        CHECK(gesehen["Zeile"].xShift == 1);
        CHECK(gesehen["Zeile"].yShift == -3);
        (void)lauf(lp, 95);  // zehn Beats: bleibt auf dem letzten Wort der ersten Zeile
        CHECK(gesehen["Zeile"].xShift == 1);
        CHECK(gesehen["Zeile"].yShift == -3);
    }
    {
        // Wort-Stand im Bild: ohne wordtrigger die ganze Zeile in der Stimmfarbe (rot);
        // nach zwei wordtriggern: erstes Wort rot, zweites laeuft (gruen), drittes blau
        LyricsParams lp;
        lp.useWordColor = true;
        lp.wordFlowMs = 0;
        const QImage ohne = lauf(lp, 5);
        CHECK(zaehle(ohne, 0) > 0);
        CHECK(zaehle(ohne, 1) == 0);
        CHECK(zaehle(ohne, 2) == 0);
        lp.frameCode = "wordtrigger = b;";
        const QImage mit = lauf(lp, 15);
        CHECK(zaehle(mit, 0) > 0);
        CHECK(zaehle(mit, 1) > 0);
        CHECK(zaehle(mit, 2) > 0);
        // … und nach einem linetrigger steht die neue Zeile noch ungesungen (blau)
        lp.frameCode = "wordtrigger = if(equal(n, 0), b, 0); linetrigger = if(equal(n, 1), b, 0); n = n + b;";
        lp.initCode = "n = 0;";
        const QImage neu = lauf(lp, 15);
        CHECK(zaehle(neu, 0) == 0);
        CHECK(zaehle(neu, 1) == 0);
        CHECK(zaehle(neu, 2) > 0);
    }
    {
        // synchroner Text: die Abspielposition bestimmt — die Ausloeser wirken nicht
        MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM\n[00:13.00]HHHH\n"));
        MultiEffectVisualizer::notifyPlayPosition(11000);
        LyricsParams lp;
        lp.lineCode = "xshift = line;";
        lp.frameCode = "linetrigger = 1; wordtrigger = 1;";
        (void)lauf(lp, 20);
        CHECK(gesehen["Zeile"].xShift == 1);
    }
    // Worte in Stuecken (Modell): aneinandergereiht wieder der Text
    {
        const auto st = ly::wortStuecke(ly::Zeile{0, " eins  zwei drei ", ly::Stimme::Keine, {}});
        REQUIRE(st.size() == 3);
        CHECK(st[0] == " eins  ");
        CHECK(st[1] == "zwei ");
        CHECK(st[2] == "drei ");
        CHECK(ly::wortStuecke(ly::Zeile{0, "", ly::Stimme::Keine, {}}).empty());
    }

    MultiEffectVisualizer::setzeParamBeobachter({});
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90 (Frage Patrik): warum aendern sich Zeilenabstand und Lage, wenn ein
// Skript die Schrift vergroessert? Eine zu lange Zeile wurde verkleinert, bis
// sie in die Breite passte — der Zeilenabstand aber rechnete mit der
// unverkleinerten Schrift. Entscheid B3 a1: eine Schriftgroesse je Song.
TEST_CASE("Lyrics S90: eine Schriftgroesse je Song — lange Zeilen bestimmen Schrift und Abstand")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtext(
        ly::leseLrc("[00:10.00]MMMM MMMM MMMM\n[00:13.00]HH\n"));
    MultiEffectVisualizer::notifyPlayPosition(11000);

    struct Mass
    {
        int hoehe = 0;  ///< Hoehe der hellen Zeile
        int y = 0;      ///< ihre Oberkante
    };
    const auto miss = [](int schrift, bool vorschauHell) {
        LyricsParams lp;
        lp.fontHeight = -schrift;
        lp.outline = false;
        lp.previewLines = 1;
        lp.vAlign = 2;
        lp.yShift = -2;
        lp.color = vorschauHell ? 0x202020 : 0xFFFFFF;
        lp.colorNext = vorschauHell ? 0xFFFFFF : 0x202020;
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        root.children = {clear, lyr};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(480, 270));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 480, 270);
        vis.render(1.0f / 60.0f);
        vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        int oben = -1, unten = -1;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 200)
                {
                    if (oben < 0) oben = y;
                    unten = y;
                    break;
                }
        return Mass{oben < 0 ? 0 : unten - oben + 1, oben};
    };
    // Entscheid B3 a1: eine Schriftgroesse je Song. 40 px passt mit der langen
    // Zeile nicht in 480 px Breite, 60 px erst recht nicht — beide werden auf
    // dieselbe Groesse gesetzt, und der Zeilenabstand geht mit: gleiche
    // Zeilenhoehe, gleicher Abstand, gleiche Lage. (Bis S90 folgte der Abstand
    // der eingestellten Schrift: gleiche Buchstaben, aber 1,5-facher Abstand.)
    const Mass z40 = miss(40, false);
    const Mass z60 = miss(60, false);
    const Mass v40 = miss(40, true);
    const Mass v60 = miss(60, true);
    MESSAGE("Schrift 40: Zeile y=" << z40.y << " h=" << z40.hoehe << ", Vorschau y=" << v40.y
            << " h=" << v40.hoehe << " | Schrift 60: Zeile y=" << z60.y << " h=" << z60.hoehe
            << ", Vorschau y=" << v60.y);
    REQUIRE(z40.hoehe > 0);
    CHECK(std::abs(z60.hoehe - z40.hoehe) <= 1);
    CHECK(std::abs((v60.y - z60.y) - (v40.y - z40.y)) <= 2);
    CHECK(std::abs(z60.y - z40.y) <= 2);
    // die KURZE Vorschau-Zeile traegt dieselbe Schrift wie die lange laufende
    CHECK(std::abs(v40.hoehe - z40.hoehe) <= 1);
    // Untergrenze 50 %: bei 200 px passt die Zeile auch mit halber Schrift
    // nicht — sie wird dann einzeln weiter verkleinert und steht trotzdem ganz im Bild
    const Mass z200 = miss(200, false);
    CHECK(z200.hoehe > 0);
    CHECK(z200.hoehe <= z40.hoehe + 1);

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S92 (Sichttest D2, Entscheid 1a): Feld „Word spacing (%)" — der Abstand
// zwischen den Worten in Prozent des Leerzeichens der Schrift. Gemessen wird
// die Breite der laufenden Zeile, auf jedem der drei Wege: CPU (ohne Skript),
// Masken (mit Skript), Wort fuer Wort (mit Wort-Zeiten).
TEST_CASE("Lyrics S92: Wortabstand — breiter und enger auf allen Zeichenwegen, 100 = wie bisher")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::notifyPlayPosition(12500);

    const auto breite = [&](LyricsParams lp) {
        lp.fontHeight = -24;
        lp.outline = false;
        lp.previewLines = 0;
        lp.showSyncMark = false;
        lp.vAlign = 1;
        lp.yShift = 0;
        lp.color = 0xFFFFFF;
        lp.colorWord = 0xFFFFFF;
        lp.colorNext = 0xFFFFFF;
        lp.wordFlowMs = 0;
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = lp;
        root.children = {clear, lyr};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(480, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 480, 240);
        vis.render(1.0f / 60.0f);
        vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        int links = -1, rechts = -1;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 200)
                {
                    if (links < 0 || x < links) links = x;
                    rechts = std::max(rechts, x);
                }
        return links < 0 ? 0 : rechts - links + 1;
    };
    const auto mit = [](int abstand, const char* frameCode = "") {
        LyricsParams lp;
        lp.wordSpacing = abstand;
        lp.frameCode = frameCode;
        return lp;
    };

    // --- CPU-Weg: Zeile ohne Wort-Zeiten, Knoten ohne Skript ---
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MM MM MM\n[00:20.00]Ende\n"));
    const int cpu100 = breite(mit(100));
    const int cpu200 = breite(mit(200));
    const int cpu50 = breite(mit(50));
    const int cpu400 = breite(mit(400));
    MESSAGE("CPU: 50=" << cpu50 << " 100=" << cpu100 << " 200=" << cpu200 << " 400=" << cpu400);
    REQUIRE(cpu100 > 0);
    CHECK(cpu200 > cpu100 + 4);  // zwei Luecken, jede um ein Leerzeichen weiter
    CHECK(cpu50 < cpu100);
    CHECK(cpu400 > cpu200);
    // die Luecke waechst gleichmaessig: 100 -> 200 ist ein Leerzeichen je Luecke,
    // 200 -> 400 sind zwei
    CHECK(std::abs((cpu400 - cpu200) - 2 * (cpu200 - cpu100)) <= 3);
    // Werte ausserhalb 50..400 klemmen
    CHECK(breite(mit(9999)) == cpu400);
    CHECK(breite(mit(0)) == cpu50);
    // ein Wort allein hat keine Luecke: der Abstand aendert nichts
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMMMM\n[00:20.00]Ende\n"));
    CHECK(breite(mit(100)) == breite(mit(300)));

    // --- Masken-Weg: derselbe Text, der Knoten traegt ein Skript ---
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MM MM MM\n[00:20.00]Ende\n"));
    const char* neutral = "xshift = xshift;";
    const int gpu100 = breite(mit(100, neutral));
    const int gpu200 = breite(mit(200, neutral));
    const int gpu50 = breite(mit(50, neutral));
    MESSAGE("Masken: 50=" << gpu50 << " 100=" << gpu100 << " 200=" << gpu200);
    CHECK(std::abs(gpu100 - cpu100) <= 2);  // beide Wege setzen gleich
    CHECK(std::abs(gpu200 - cpu200) <= 2);
    CHECK(std::abs(gpu50 - cpu50) <= 2);
    // per Skript: das Feld folgt aus Init (es laesst die Masken neu rastern)
    {
        LyricsParams lp;
        lp.initCode = "wordspacing = 200;";
        CHECK(std::abs(breite(lp) - cpu200) <= 2);
    }

    // --- Wort fuer Wort: Text mit Wort-Zeiten, das laufende Wort eingefaerbt ---
    MultiEffectVisualizer::setSongtext(
        ly::leseLrc("[00:10.00]MM <00:11.00>MM <00:12.00>MM\n[00:20.00]Ende\n"));
    const auto worte = [&](int abstand, int layout) {
        LyricsParams lp;
        lp.wordSpacing = abstand;
        lp.useWordColor = true;
        lp.wordLayout = layout;
        return breite(lp);
    };
    const int wort100 = worte(100, 0);
    const int wort200 = worte(200, 0);
    MESSAGE("Worte: 100=" << wort100 << " 200=" << wort200);
    CHECK(std::abs(wort100 - cpu100) <= 2);
    CHECK(std::abs(wort200 - cpu200) <= 2);
    CHECK(std::abs(worte(200, 1) - wort200) <= 1);  // Overlay ohne Vergroesserung: gleich breit

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S90, LB1: Bilder des Lyrics-Knotens fuer den Vergleich vorher/nachher. Mit
// `LUMIVIZ_LB1_SNAP=<ordner>` schreibt der Test je Szene ein PNG — derselbe
// Lauf vor und nach einem Umbau am Rastern, die Ordner dann bitweise
// vergleichen. Ohne die Variable prueft er nur, dass jede Szene etwas zeichnet.
TEST_CASE("Lyrics S90 LB1: Szenen des Lyrics-Knotens (Schnappschuesse fuer den Vergleich)")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    const QString schnapp = qEnvironmentVariable("LUMIVIZ_LB1_SNAP");
    const char* synchron = "[00:10.00]MMMM <00:11.50>gggg\n[00:13.00]IIII jjjj\n"
                           "[00:16.00]Third line\n[00:19.00]Fourth\n";

    const auto bild = [&](const char* name, LyricsParams lp, bool mitTrail, bool ebene,
                          int posMs) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode balken;
        balken.params = DebugBarsParams{0x40C0FF, -2.5f};
        root.children = {balken};
        if (!ebene)
        {
            ChainNode lyr;
            lyr.params = lp;
            root.children.push_back(lyr);
        }
        if (mitTrail)
        {
            ChainNode trail;
            trail.params = LyricsTrailParams{};
            root.children.push_back(trail);
        }
        MultiEffectVisualizer::setKaraokeEbene(ebene);
        MultiEffectVisualizer::notifyPlayPosition(posMs);
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(480, 270));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 480, 270);
        vis.render(1.0f / 60.0f);
        vis.render(1.0f / 60.0f);
        const QImage img = ebene ? vis.debugGrabMitKaraoke() : vis.debugGrabRootSurface();
        MultiEffectVisualizer::setKaraokeEbene(false);
        if (!schnapp.isEmpty()) img.save(schnapp + '/' + QLatin1String(name) + ".png");
        return img;
    };
    // weicht das Bild vom blossen Hintergrund ab?
    const auto zeichnet = [&](const QImage& img) {
        static QImage leer;
        if (leer.isNull())
        {
            MultiEffectVisualizer::setSongtext(ly::Songtext{});
            leer = bild("leer", LyricsParams{}, false, false, 0);
        }
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (img.pixel(x, y) != leer.pixel(x, y)) return true;
        return false;
    };
    (void)zeichnet(QImage(1, 1, QImage::Format_RGB32));  // Hintergrund einmal holen

    MultiEffectVisualizer::setSongtext(ly::leseLrc(synchron));
    LyricsParams lp;
    lp.fontHeight = -28;
    lp.previewLines = 2;
    CHECK(zeichnet(bild("01_replace", lp, false, false, 12000)));
    LyricsParams a = lp;
    a.blend = 1;
    CHECK(zeichnet(bild("02_additive", a, false, false, 12000)));
    a.blend = 2;
    CHECK(zeichnet(bild("03_5050", a, false, false, 12000)));
    a.blend = 4;
    CHECK(zeichnet(bild("04_invert", a, false, false, 12000)));
    a = lp;
    a.blendStrength = 50;
    CHECK(zeichnet(bild("05_staerke50", a, false, false, 12000)));
    a = lp;
    a.outline = false;
    a.blend = 7;
    CHECK(zeichnet(bild("06_maximum_ohne_kontur", a, false, false, 12000)));
    a = lp;
    a.vAlign = 0;
    a.yShift = 4;
    a.previewLines = 0;
    CHECK(zeichnet(bild("07_oben_ohne_vorschau", a, false, false, 12000)));
    CHECK(zeichnet(bild("08_vor_der_ersten_zeile", lp, false, false, 5000)));
    CHECK(zeichnet(bild("09_trail_rueckfall", lp, true, false, 13100)));
    CHECK(zeichnet(bild("10_ebene", lp, false, true, 12000)));
    a = lp;
    a.outlineSize = 14;  // Reihen ueberlappen sich
    CHECK(zeichnet(bild("11_breite_kontur", a, false, false, 12000)));

    // LB1b: Vorschau kleiner, duenner, eingerueckt, enger gesetzt (nur zum Ansehen)
    a = lp;
    a.previewScale = 60;
    a.previewWeight = 400;
    a.previewXShift = 6;
    a.lineSpacing = 100;
    CHECK(zeichnet(bild("13_vorschau_gestaltet", a, false, false, 12000)));
    // B2 a: dieselben Szenen auf dem Masken-Weg (ein Skript, das nichts tut,
    // schaltet ihn ein) — zum Ansehen neben 01 und 04
    a = lp;
    a.initCode = "nichts = 0;";
    CHECK(zeichnet(bild("14_masken_replace", a, false, false, 12000)));
    a.blend = 4;
    CHECK(zeichnet(bild("15_masken_invert", a, false, false, 12000)));

    MultiEffectVisualizer::setSongtext(ly::leseLrc("MMMM gggg\nIIII jjjj\nThird line\n"));
    CHECK(zeichnet(bild("12_unsynchron_marke", lp, false, false, 0)));

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S88, KE1 (docs/visuals/Karaoke_Ebene_Konzept.md): Songtext ueber jedem Preset
// als eigene Ebene. Der Kern: die Ebene wird erst beim Present aufgelegt — die
// Root-Surface (das Feedback des naechsten Frames) bleibt BITGLEICH, ob die
// Ebene an oder aus ist. Und: ein Lyrics-Knoten in der Kette gewinnt.
TEST_CASE("Karaoke-Ebene S88: Text ueber dem Bild, Root-Surface unberuehrt, die Kette gewinnt")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));

    // Eine Kette MIT Rueckkopplung und ohne Clear: alles, was in die
    // Root-Surface geriete, bliebe dort und wuerde weitergetragen.
    const auto kette = [](bool mitLyrics, bool lyricsAn) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode balken;
        balken.params = DebugBarsParams{0x40C0FF, -2.5f};
        ChainNode drift;
        BlitterFeedbackParams bf;
        bf.scale = 34;
        bf.scale2 = 34;
        drift.params = bf;
        ChainNode fade;
        fade.params = FadeoutParams{2, 0x000000};
        root.children = {balken, drift, fade};
        if (mitLyrics)
        {
            ChainNode lyr;
            LyricsParams lp;
            lp.vAlign = 0;  // oben — die Ebene zeichnet unten
            lp.yShift = 6;
            lyr.params = lp;
            lyr.enabled = lyricsAn;
            root.children.push_back(lyr);
        }
        return root;
    };
    struct Lauf
    {
        QImage root;      ///< Root-Surface nach dem letzten Frame
        QImage fenster;   ///< wie am Fenster: Root + Ebene
        bool gezeichnet;  ///< die Ebene war im letzten Frame dran
    };
    const auto lauf = [&](ChainNode root, bool ebene, int posMs, int frames = 20) {
        MultiEffectVisualizer::setKaraokeEbene(ebene);
        MultiEffectVisualizer::notifyPlayPosition(posMs);
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        for (int i = 0; i < frames; ++i) vis.render(1.0f / 60.0f);
        Lauf l{vis.debugGrabRootSurface(), vis.debugGrabMitKaraoke(), vis.debugKaraokeGezeichnet()};
        MultiEffectVisualizer::setKaraokeEbene(false);
        return l;
    };
    const auto unterschied = [](const QImage& a, const QImage& b, int yVon, int yBis) {
        int n = 0;
        for (int y = yVon; y < yBis; ++y)
            for (int x = 0; x < a.width(); ++x)
                if (a.pixel(x, y) != b.pixel(x, y)) ++n;
        return n;
    };

    // --- Ebene aus: das Fenster zeigt genau die Root-Surface
    const Lauf aus = lauf(kette(false, false), false, 11000);
    REQUIRE_FALSE(aus.root.isNull());
    REQUIRE(aus.fenster.size() == aus.root.size());
    CHECK_FALSE(aus.gezeichnet);
    CHECK(unterschied(aus.fenster, aus.root, 0, aus.root.height()) == 0);

    // --- Ebene an, erste Zeile laeuft: Text im Fenster, unten
    const Lauf an = lauf(kette(false, false), true, 11000);
    CHECK(an.gezeichnet);
    CHECK(an.root == aus.root);  // der Kern: die Root-Surface ist BITGLEICH
    const int h = an.root.height();
    // toImage() liefert das Bild mit dem Ursprung oben: der Text steht in der unteren Haelfte
    CHECK(unterschied(an.fenster, an.root, h / 2, h) > 200);
    CHECK(unterschied(an.fenster, an.root, 0, h / 4) == 0);

    // --- kurz nach dem Zeilenwechsel: Trail und neue Zeile, Root weiter bitgleich
    const Lauf wechselAus = lauf(kette(false, false), false, 13060);
    const Lauf wechselAn = lauf(kette(false, false), true, 13060);
    CHECK(wechselAn.root == wechselAus.root);
    CHECK(unterschied(wechselAn.fenster, wechselAn.root, h / 2, h) > 100);
    // Bilder zum Ansehen (Merkregel S87: Bildwirkung an Bildern pruefen)
    if (const QString snap = qEnvironmentVariable("LUMIVIZ_KARAOKE_SNAP"); !snap.isEmpty())
    {
        an.fenster.save(snap + QStringLiteral("/karaoke_zeile.png"));
        wechselAn.fenster.save(snap + QStringLiteral("/karaoke_wechsel.png"));
        an.root.save(snap + QStringLiteral("/karaoke_root.png"));
    }

    // --- ohne Songtext zeichnet die Ebene nichts
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    const Lauf ohneText = lauf(kette(false, false), true, 11000);
    CHECK_FALSE(ohneText.gezeichnet);
    CHECK(unterschied(ohneText.fenster, ohneText.root, 0, h) == 0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));

    // --- die Kette gewinnt: ein eingeschalteter Lyrics-Knoten (oben), und die
    //     Ebene tritt zurueck — das Fenster ist die Root-Surface, der Text steht
    //     nur einmal da (oben, vom Knoten)
    const Lauf gewinnt = lauf(kette(true, true), true, 11000);
    CHECK_FALSE(gewinnt.gezeichnet);
    CHECK(unterschied(gewinnt.fenster, gewinnt.root, 0, h) == 0);
    CHECK(unterschied(gewinnt.root, aus.root, 0, h / 2) > 200);  // Gegenprobe: der Knoten zeichnet
    // ein AUSGESCHALTETER Lyrics-Knoten zaehlt nicht: die Ebene uebernimmt
    const Lauf knotenAus = lauf(kette(true, false), true, 11000);
    CHECK(knotenAus.gezeichnet);
    CHECK(unterschied(knotenAus.fenster, knotenAus.root, h / 2, h) > 200);

    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S91: der Trail der Ebene haelt ueber den Wechsel der Kette (neues Preset).
// Die Position laeuft ueber einen Zeilenwechsel, kurz danach wird die Kette
// getauscht; gezaehlt werden die Pixel, die die Ebene im Fenster ueber die
// Root-Surface legt — mit und ohne Tausch.
TEST_CASE("Karaoke-Ebene S91: der Trail haelt ueber den Wechsel der Kette")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
    MultiEffectVisualizer::setKaraokeEbene(true);

    const auto kette = [](uint32_t farbe) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode balken;
        balken.params = DebugBarsParams{farbe, -2.5f};
        root.children = {clear, balken};
        return root;
    };
    const auto ebenePixel = [](MultiEffectVisualizer& vis) {
        const QImage root = vis.debugGrabRootSurface();
        const QImage fenster = vis.debugGrabMitKaraoke();
        int n = 0;
        for (int y = 0; y < root.height(); ++y)
            for (int x = 0; x < root.width(); ++x)
                if (root.pixel(x, y) != fenster.pixel(x, y)) ++n;
        return n;
    };
    const QString snap = qEnvironmentVariable("LUMIVIZ_KARAOKE_SNAP");
    // Frame 30 ueberschreitet 13 s (Zeilenwechsel); bei Frame `tausch` wechselt die Kette
    const auto lauf = [&](int tausch, const char* name) {
        MultiEffectVisualizer vis;
        vis.setChain(kette(0x40C0FF));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        std::vector<int> pixel;
        for (int frame = 0; frame < 70; ++frame)
        {
            MultiEffectVisualizer::notifyPlayPosition(12500 + frame * 1000 / 60);
            if (frame == tausch) vis.setChain(kette(0xFF8040));
            vis.render(1.0f / 60.0f);
            pixel.push_back(ebenePixel(vis));
            if (!snap.isEmpty() && (frame == 36 || frame == 37 || frame == 45))
                vis.debugGrabMitKaraoke().save(
                    QStringLiteral("%1/wechsel_%2_f%3.png").arg(snap, QLatin1String(name)).arg(frame));
        }
        return pixel;
    };
    const std::vector<int> ohne = lauf(-1, "ohne");
    const std::vector<int> mit = lauf(37, "mit");
    // Der Trail ist da (Gegenprobe: nach dem Zeilenwechsel liegt mehr auf als davor) …
    CHECK(ohne[36] > ohne[29]);
    // … und der Tausch der Kette aendert an der Ebene nichts: in jedem Frame
    // dieselbe Zahl Pixel wie ohne Tausch. Vor S91 baute sich der Lyrics-Knoten
    // der Ebene beim Tausch neu auf, die gegangene Zeile wehte noch einmal von
    // ihrer Stelle los (gemessen in Frame 37: 5428 statt 6183 Pixel).
    for (std::size_t f = 0; f < ohne.size(); ++f)
    {
        CAPTURE(f);
        CHECK(mit[f] == ohne[f]);
    }

    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S92 (Wunsch Patrik): Trail-Presets — die Trail-Liste der Ebene als Datei.
// Jedes mitgelieferte Preset wird geladen, in die Ebene gesetzt und ueber einen
// Zeilenwechsel gerendert. Mit `LUMIVIZ_TRAIL_PRESET_SNAP=<ordner>` schreibt der
// Test je Preset fuenf Bilder (Kontaktbogen); mit
// `LUMIVIZ_TRAIL_PRESET_REWRITE=1` schreibt er jede Datei so zurueck, wie die
// App sie speichert (nach dem Anlegen von Hand).
TEST_CASE("Trail-Presets S92: jedes mitgelieferte laedt, zeichnet einen Trail, steht vollstaendig in der Datei")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    namespace tp = lumi::multieffect::trailpresets;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
    MultiEffectVisualizer::setKaraokeEbene(true);
    const QString snap = qEnvironmentVariable("LUMIVIZ_TRAIL_PRESET_SNAP");
    const bool rewrite = qEnvironmentVariableIsSet("LUMIVIZ_TRAIL_PRESET_REWRITE");

    std::vector<tp::Entry> mitgeliefert;
    for (const tp::Entry& e : tp::list())
        if (e.builtin) mitgeliefert.push_back(e);
    QStringList namen;
    for (const tp::Entry& e : mitgeliefert) namen << e.name;
    CAPTURE(namen.join(QStringLiteral(", ")).toStdString());
    REQUIRE(mitgeliefert.size() == 8);

    const auto ebenePixel = [](MultiEffectVisualizer& vis) {
        const QImage root = vis.debugGrabRootSurface();
        const QImage fenster = vis.debugGrabMitKaraoke();
        int n = 0;
        for (int y = 0; y < root.height(); ++y)
            for (int x = 0; x < root.width(); ++x)
                if (root.pixel(x, y) != fenster.pixel(x, y)) ++n;
        return n;
    };
    // Gegenprobe: mit ausgeschaltetem Trail liegt nach dem Zeilenwechsel nur die neue Zeile auf
    int ohneTrail = 0;

    for (int durchgang = -1; durchgang < static_cast<int>(mitgeliefert.size()); ++durchgang)
    {
        ChainNode liste;
        std::string name = "(trail off)";
        if (durchgang >= 0)
        {
            const tp::Entry& e = mitgeliefert[static_cast<std::size_t>(durchgang)];
            name = e.name.toStdString();
            QStringList report;
            REQUIRE_MESSAGE(tp::load(e.path, liste, &report),
                            name << ": " << report.join(QStringLiteral("; ")).toStdString());
            if (rewrite) REQUIRE(saveChainToFile(liste, e.path));
            // die Datei steht so da, wie die App sie schreibt (Tippfehler-Wache:
            // ein unbekanntes Feld ginge beim Lesen verloren und fehlte hier)
            QFile f(e.path);
            REQUIRE(f.open(QIODevice::ReadOnly));
            const QJsonObject datei = QJsonDocument::fromJson(f.readAll()).object();
            CHECK_MESSAGE(datei == chainToJson(liste), name << ": file differs from what the app writes");
            // ein Trail-Preset endet auf genau einen Lyrics Trail
            REQUIRE_FALSE(liste.children.empty());
            CHECK(std::holds_alternative<LyricsTrailParams>(liste.children.back().params));
        }
        CAPTURE(name);

        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode balken;
        balken.params = DebugBarsParams{0x203040, -2.5f};
        root.children = {clear, balken};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        if (durchgang >= 0)
        {
            REQUIRE(vis.setzeKaraokeTrailListe(liste));
            // die Liste der Ebene ist danach dieses Preset
            CHECK(tp::gleich(vis.karaokeKette().children[1], liste));
        }
        else
        {
            // das mitgelieferte Aussehen der Ebene IST das Preset „Drift Up"
            ChainNode drift;
            REQUIRE(tp::load(tp::finde(QStringLiteral("Drift Up")).path, drift, nullptr));
            CHECK(tp::gleich(vis.karaokeKette().children[1], drift));
            vis.karaokeKette().children[1].enabled = false;
            (void)vis.recompileKaraoke();
        }
        vis.initialize();
        vis.resize(QSize(480, 270));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 480, 270);
        std::vector<int> pixel;
        // Frame 30 ueberschreitet 13 s (Zeilenwechsel)
        for (int frame = 0; frame < 100; ++frame)
        {
            MultiEffectVisualizer::notifyPlayPosition(12500 + frame * 1000 / 60);
            vis.render(1.0f / 60.0f);
            pixel.push_back(ebenePixel(vis));
            if (!snap.isEmpty() && durchgang >= 0 &&
                (frame == 32 || frame == 38 || frame == 48 || frame == 65 || frame == 95))
                vis.debugGrabMitKaraoke().save(QStringLiteral("%1/%2_f%3.png")
                                                   .arg(snap, QString::fromStdString(name))
                                                   .arg(frame, 3, 10, QLatin1Char('0')));
        }
        if (durchgang < 0)
        {
            ohneTrail = pixel[36];
            REQUIRE(ohneTrail > 0);
            continue;
        }
        MESSAGE(name << ": f29=" << pixel[29] << " f36=" << pixel[36] << " f60=" << pixel[60]
                     << " f99=" << pixel[99] << " (ohne Trail f36=" << ohneTrail << ")");
        // der Trail ist da: kurz nach dem Wechsel liegt deutlich mehr auf als die neue Zeile allein
        CHECK(pixel[36] > ohneTrail * 1.3);
    }

    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S88, K6 (Entscheid Patrik a): Mischarten der Karaoke-Ebene — je Teil (Zeile,
// Trail) wie bei den Listen bekannt, dazu Invert und eine Staerke. Gerechnet
// wird per GL-Blending auf dem fertigen Bild; geprueft an den Pixeln im
// Inneren der Buchstaben (dort ist die Deckung 1) ueber einfarbigem Grund.
TEST_CASE("Karaoke-Ebene S88: Mischarten der Zeile rechnen wie angegeben, die Kontur bleibt normal")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    using Teil = MultiEffectVisualizer::KaraokeTeil;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
    const QRgb grund = qRgb(64, 128, 192);
    const QString snap = qEnvironmentVariable("LUMIVIZ_KARAOKE_SNAP");

    const auto fenster = [&](int zeile, int zeileStaerke, int trail, int posMs, const char* name) {
        MultiEffectVisualizer::setKaraokeEbene(true);
        MultiEffectVisualizer::setKaraokeMischung(Teil::Zeile, zeile, zeileStaerke);
        MultiEffectVisualizer::setKaraokeMischung(Teil::Trail, trail, 100);
        MultiEffectVisualizer::notifyPlayPosition(posMs);
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        ClearParams cp;
        cp.color = 0x4080C0;
        clear.params = cp;
        root.children = {clear};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        for (int i = 0; i < 12; ++i) vis.render(1.0f / 60.0f);
        const QImage root0 = vis.debugGrabRootSurface();
        REQUIRE(root0.pixel(5, 5) == grund);  // der Grund steht, die Ebene fasst ihn nicht an
        REQUIRE(root0.pixel(160, 150) == grund);
        const QImage bild = vis.debugGrabMitKaraoke();
        if (!snap.isEmpty()) bild.save(snap + QStringLiteral("/misch_%1.png").arg(QLatin1String(name)));
        MultiEffectVisualizer::setKaraokeEbene(false);
        MultiEffectVisualizer::setKaraokeMischung(Teil::Zeile, 0, 100);
        MultiEffectVisualizer::setKaraokeMischung(Teil::Trail, 1, 100);
        return bild;
    };

    // Normal: die Pixel im Inneren der laufenden Zeile sind reinweiss — sie
    // sind die Messpunkte fuer alle anderen Arten
    const QImage normal = fenster(0, 100, 1, 11000, "0_normal");
    std::vector<QPoint> innen;
    for (int y = 0; y < normal.height(); ++y)
        for (int x = 0; x < normal.width(); ++x)
            if (normal.pixel(x, y) == qRgb(255, 255, 255)) innen.emplace_back(x, y);
    REQUIRE(innen.size() > 300);

    // Erwartung je Art fuer Quelle weiss, Deckung 1, ueber (64,128,192)
    struct Fall
    {
        int art;
        const char* name;
        int r, g, b;
    };
    const Fall faelle[] = {
        {1, "1_additive", 255, 255, 255},
        {2, "2_5050", 160, 192, 224},
        {3, "3_subtract", 0, 0, 0},
        {4, "4_invert", 191, 127, 63},
        {5, "5_xor", 191, 127, 63},
        {6, "6_multiply", 64, 128, 192},
        {7, "7_maximum", 255, 255, 255},
        {8, "8_minimum", 64, 128, 192},
    };
    const auto falsch = [&](const QImage& bild, int r, int g, int b) {
        int n = 0;
        for (const QPoint& p : innen)
        {
            const QRgb c = bild.pixel(p);
            if (std::abs(qRed(c) - r) > 2 || std::abs(qGreen(c) - g) > 2 || std::abs(qBlue(c) - b) > 2)
                ++n;
        }
        return n;
    };
    for (const Fall& f : faelle)
    {
        CAPTURE(f.name);
        const QImage bild = fenster(f.art, 100, 1, 11000, f.name);
        CHECK(falsch(bild, f.r, f.g, f.b) == 0);
        // ausserhalb des Texts bleibt der Grund, wie er ist
        CHECK(bild.pixel(5, 5) == grund);
        // die Kontur bleibt normal: schwarze Pixel um die Buchstaben (ausser dort,
        // wo die Fuellung selbst schwarz wird — Subtract)
        int schwarz = 0;
        for (int y = 0; y < bild.height(); ++y)
            for (int x = 0; x < bild.width(); ++x)
                if (bild.pixel(x, y) == qRgb(0, 0, 0)) ++schwarz;
        CHECK(schwarz > 100);
    }

    // Staerke: Invert zu 50 % — weiss mit halber Deckung macht aus jedem Grund Mittelgrau
    const QImage halb = fenster(4, 50, 1, 11000, "4_invert_50");
    CHECK(falsch(halb, 128, 128, 128) == 0);
    // Staerke 0 = die Fuellung fehlt, die Kontur steht noch
    const QImage null = fenster(4, 0, 1, 11000, "4_invert_0");
    CHECK(falsch(null, 64, 128, 192) == 0);

    // Trail: kurz nach dem Zeilenwechsel; Additive hellt auf, Subtract dunkelt ab
    const QImage trailAdd = fenster(0, 100, 1, 13060, "trail_additive");
    const QImage trailSub = fenster(0, 100, 3, 13060, "trail_subtract");
    const QImage trailAus = fenster(0, 100, 1, 12900, "trail_keiner");  // vor dem Wechsel
    int heller = 0;
    int dunkler = 0;
    for (int y = 0; y < trailAdd.height(); ++y)
        for (int x = 0; x < trailAdd.width(); ++x)
        {
            if (trailSub.pixel(x, y) == trailAdd.pixel(x, y)) continue;
            if (qGray(trailAdd.pixel(x, y)) > qGray(grund)) ++heller;
            if (qGray(trailSub.pixel(x, y)) < qGray(grund)) ++dunkler;
        }
    CHECK(heller > 100);
    CHECK(dunkler > 100);
    CHECK(trailAus != trailAdd);  // Gegenprobe: ohne Wechsel kein Trail

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S89, K4 b (Entscheid Patrik): die Zeile der Karaoke-Ebene rastert in
// Fensteraufloesung. Mit Render Scale (Divisor 2) ist die Kette halb so gross
// wie das Fenster — der Text bleibt scharf, steht an derselben Stelle wie
// ohne Render Scale, und die Root-Surface bleibt bitgleich.
TEST_CASE("Karaoke-Ebene S89: Zeile in Fensteraufloesung — scharf mit Render Scale, Lage gleich")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
    MultiEffectVisualizer::notifyPlayPosition(11000);
    const QRgb grund = qRgb(64, 128, 192);

    struct Lauf
    {
        QImage root;
        QImage fenster;
    };
    const auto lauf = [&](bool ebene, int divisor, QSize groesse) {
        MultiEffectVisualizer::setKaraokeEbene(ebene);
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        ClearParams cp;
        cp.color = 0x4080C0;
        clear.params = cp;
        root.children = {clear};
        if (divisor > 1)
        {
            ChainNode scale;
            scale.params = RenderScaleParams{divisor, 0};
            root.children.push_back(scale);
        }
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(groesse);
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, groesse.width(),
                                                                  groesse.height());
        for (int i = 0; i < 12; ++i) vis.render(1.0f / 60.0f);
        Lauf l{vis.debugGrabRootSurface(), vis.debugGrabMitKaraoke()};
        MultiEffectVisualizer::setKaraokeEbene(false);
        return l;
    };
    const auto rahmen = [&](const QImage& bild) {
        QRect r;
        for (int y = 0; y < bild.height(); ++y)
            for (int x = 0; x < bild.width(); ++x)
                if (bild.pixel(x, y) != grund) r |= QRect(x, y, 1, 1);
        return r;
    };

    const Lauf klein = lauf(true, 1, QSize(320, 240));    // Vergleich: ohne Render Scale
    const Lauf gross = lauf(true, 2, QSize(640, 480));    // Kette 320x240, Fenster 640x480
    const Lauf grossAus = lauf(false, 2, QSize(640, 480));
    REQUIRE(gross.root.size() == QSize(320, 240));
    REQUIRE(gross.fenster.size() == QSize(640, 480));
    REQUIRE(klein.fenster.size() == QSize(320, 240));
    CHECK(gross.root == grossAus.root);  // Root-Surface bitgleich, Ebene an/aus

    // Lage: der Rahmen des Texts ist der des kleinen Laufs mal zwei
    const QRect rk = rahmen(klein.fenster);
    const QRect rg = rahmen(gross.fenster);
    REQUIRE(rk.width() > 50);
    CHECK(std::abs(rg.left() - 2 * rk.left()) <= 4);
    CHECK(std::abs(rg.top() - 2 * rk.top()) <= 4);
    CHECK(std::abs(rg.right() - (2 * rk.right() + 1)) <= 4);
    CHECK(std::abs(rg.bottom() - (2 * rk.bottom() + 1)) <= 4);

    // Scharf: ein aus der Kettenaufloesung (nearest) hochgezogenes Bild bestuende
    // aus einfarbigen 2x2-Bloecken. In Fensteraufloesung gerastert gibt es an den
    // Kanten viele Bloecke, die es nicht sind.
    int gemischt = 0;
    for (int y = 0; y + 1 < gross.fenster.height(); y += 2)
        for (int x = 0; x + 1 < gross.fenster.width(); x += 2)
        {
            const QRgb c = gross.fenster.pixel(x, y);
            if (gross.fenster.pixel(x + 1, y) != c || gross.fenster.pixel(x, y + 1) != c ||
                gross.fenster.pixel(x + 1, y + 1) != c)
                ++gemischt;
        }
    CHECK(gemischt > 200);

    if (const QString snap = qEnvironmentVariable("LUMIVIZ_KARAOKE_SNAP"); !snap.isEmpty())
    {
        klein.fenster.save(snap + QStringLiteral("/k4b_klein.png"));
        gross.fenster.save(snap + QStringLiteral("/k4b_gross.png"));
    }

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S89, KE2 Schritt 1: die Ebene ist ein eigener Baum (Lyrics + Trail-Liste),
// der sich im Kettenformat speichern und laden laesst (.lvkar). Geladen wird
// nur, was diese Form hat; eine gesetzte Ablage laedt jede neue Instanz selbst.
TEST_CASE("Karaoke-Ebene S89: Baum speichern, laden, Ablage beim Start — Fremdes wird abgewiesen")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    const QString datei = tmp.filePath(QStringLiteral("karaoke/current.lvkar"));
    constexpr std::uint64_t kBasis = 0xF000'0000'0000'0000ull;

    MultiEffectVisualizer::setKaraokeAblage(QString());
    MultiEffectVisualizer a;
    // Vorgabe: Lyrics + Liste mit drei Kindern, Ids im eigenen Bereich
    REQUIRE(a.karaokeKette().children.size() == 2);
    REQUIRE(std::holds_alternative<LyricsParams>(a.karaokeKette().children[0].params));
    REQUIRE(a.karaokeKette().children[1].children.size() == 3);
    CHECK(a.karaokeKette().children[0].nodeId > kBasis);
    CHECK(a.karaokeKette().children[1].children[2].nodeId > kBasis);
    CHECK(std::get<LyricsParams>(a.karaokeKette().children[0].params).vAlign == 2);

    // aendern (Zeile nach oben, Trail-Haltezeit) und speichern — der Ordner entsteht
    std::get<LyricsParams>(a.karaokeKette().children[0].params).vAlign = 0;
    std::get<LyricsTrailParams>(a.karaokeKette().children[1].children[2].params).holdMs = 250;
    (void)a.recompileKaraoke();
    REQUIRE(a.speichereKaraokeDatei(datei));
    REQUIRE(QFileInfo::exists(datei));

    // eine zweite Instanz laedt die Datei
    MultiEffectVisualizer b;
    const std::uint64_t idVorher = b.karaokeKette().children[0].nodeId;
    REQUIRE(b.ladeKaraokeDatei(datei));
    CHECK(std::get<LyricsParams>(b.karaokeKette().children[0].params).vAlign == 0);
    CHECK(std::get<LyricsTrailParams>(b.karaokeKette().children[1].children[2].params).holdMs == 250);
    CHECK(b.karaokeKette().children[0].nodeId > idVorher);  // frische Ids, nie wiederverwendet
    b.setzeKaraokeVorgabe();
    CHECK(std::get<LyricsParams>(b.karaokeKette().children[0].params).vAlign == 2);

    // eine gewoehnliche Kette ist kein Karaoke-Preset: abgewiesen, Ebene unveraendert
    const QString fremd = tmp.filePath(QStringLiteral("fremd.lvfx"));
    {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        root.children = {clear};
        REQUIRE(saveChainToFile(root, fremd));
    }
    QStringList meldung;
    CHECK_FALSE(b.ladeKaraokeDatei(fremd, &meldung));
    CHECK_FALSE(meldung.isEmpty());
    CHECK(b.karaokeKette().children.size() == 2);
    CHECK_FALSE(b.ladeKaraokeDatei(tmp.filePath(QStringLiteral("gibtsnicht.lvkar"))));

    // Ablage gesetzt: eine neue Instanz traegt die gespeicherte Ebene — und
    // zeichnet die Zeile oben statt unten
    MultiEffectVisualizer::setKaraokeAblage(datei);
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
    MultiEffectVisualizer::notifyPlayPosition(11000);
    MultiEffectVisualizer::setKaraokeEbene(true);
    {
        MultiEffectVisualizer c;
        CHECK(std::get<LyricsParams>(c.karaokeKette().children[0].params).vAlign == 0);
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        ClearParams cp;
        cp.color = 0x4080C0;
        clear.params = cp;
        root.children = {clear};
        c.setChain(std::move(root));
        c.setBeatPeriodOverride(100000);
        c.initialize();
        c.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        for (int i = 0; i < 12; ++i) c.render(1.0f / 60.0f);
        const QImage bild = c.debugGrabMitKaraoke();
        const QRgb grund = qRgb(64, 128, 192);
        int oben = 0;
        int unten = 0;
        for (int y = 0; y < bild.height(); ++y)
            for (int x = 0; x < bild.width(); ++x)
                if (bild.pixel(x, y) != grund) ++(y < bild.height() / 2 ? oben : unten);
        CHECK(oben > 200);
        CHECK(unten == 0);
    }
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setKaraokeAblage(QString());
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S89, KE2 Schritt 4: Mischart und Staerke stehen in den KNOTEN — die Ebene
// liest sie aus ihrem Lyrics-Knoten, und das neue Feld `blendStrength` wirkt
// auch am Lyrics-Knoten einer Kette (nimmt nur die Fuellung zurueck).
TEST_CASE("Karaoke-Ebene S89: Mischart und Staerke aus den Knoten, Staerke-Feld am Lyrics-Knoten")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
    MultiEffectVisualizer::notifyPlayPosition(11000);
    const QRgb grund = qRgb(64, 128, 192);

    // `ebene`: Mischart/Staerke in den Lyrics-Knoten der Ebene; sonst ein
    // Lyrics-Knoten in der Kette mit denselben Werten (Ebene aus)
    const auto bild = [&](bool ebene, int blend, int staerke) {
        MultiEffectVisualizer::setKaraokeEbene(ebene);
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        ClearParams cp;
        cp.color = 0x4080C0;
        clear.params = cp;
        root.children = {clear};
        if (!ebene)
        {
            ChainNode lyr;
            LyricsParams lp;
            lp.blend = blend;
            lp.blendStrength = staerke;
            lyr.params = lp;
            root.children.push_back(lyr);
        }
        MultiEffectVisualizer vis;
        if (ebene)
        {
            auto& lp = std::get<LyricsParams>(vis.karaokeKette().children[0].params);
            lp.blend = blend;
            lp.blendStrength = staerke;
            (void)vis.recompileKaraoke();
        }
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        for (int i = 0; i < 12; ++i) vis.render(1.0f / 60.0f);
        QImage b = ebene ? vis.debugGrabMitKaraoke() : vis.debugGrabRootSurface();
        MultiEffectVisualizer::setKaraokeEbene(false);
        return b;
    };
    const auto innenVon = [](const QImage& normal) {
        std::vector<QPoint> innen;
        for (int y = 0; y < normal.height(); ++y)
            for (int x = 0; x < normal.width(); ++x)
                if (normal.pixel(x, y) == qRgb(255, 255, 255)) innen.emplace_back(x, y);
        return innen;
    };
    const auto falsch = [](const QImage& b, const std::vector<QPoint>& innen, int r, int g, int bl) {
        int n = 0;
        for (const QPoint& p : innen)
        {
            const QRgb c = b.pixel(p);
            if (std::abs(qRed(c) - r) > 2 || std::abs(qGreen(c) - g) > 2 || std::abs(qBlue(c) - bl) > 2)
                ++n;
        }
        return n;
    };
    const auto schwarze = [](const QImage& b) {
        int n = 0;
        for (int y = 0; y < b.height(); ++y)
            for (int x = 0; x < b.width(); ++x)
                if (b.pixel(x, y) == qRgb(0, 0, 0)) ++n;
        return n;
    };

    // --- Ebene: die Werte des Knotens gelten (die prozessweite Vorgabe ist Normal/100)
    const std::vector<QPoint> innenE = innenVon(bild(true, 0, 100));
    REQUIRE(innenE.size() > 300);
    CHECK(falsch(bild(true, 4, 100), innenE, 191, 127, 63) == 0);   // Invert
    CHECK(falsch(bild(true, 4, 50), innenE, 128, 128, 128) == 0);   // Invert, halbe Staerke
    const QImage halbE = bild(true, 0, 50);                         // Normal, halbe Staerke
    CHECK(falsch(halbE, innenE, 160, 192, 224) == 0);
    CHECK(schwarze(halbE) > 100);                                   // die Kontur bleibt
    const QImage nullE = bild(true, 0, 0);
    CHECK(falsch(nullE, innenE, 64, 128, 192) == 0);                // Fuellung weg
    CHECK(schwarze(nullE) > 100);
    CHECK(nullE.pixel(5, 5) == grund);

    // --- Lyrics-Knoten in der Kette: das Staerke-Feld nimmt die Fuellung zurueck
    const std::vector<QPoint> innenK = innenVon(bild(false, 0, 100));
    REQUIRE(innenK.size() > 300);
    const QImage halbK = bild(false, 0, 50);
    CHECK(falsch(halbK, innenK, 160, 192, 224) == 0);
    CHECK(schwarze(halbK) > 100);
    CHECK(falsch(bild(false, 4, 50), innenK, 128, 128, 128) == 0);
    CHECK(falsch(bild(false, 1, 0), innenK, 64, 128, 192) == 0);

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S89 (Auftrag Patrik): der Lyrics-Knoten — und damit die Karaoke-Ebene — hat
// ein Skriptfeld „Line": es laeuft einmal je Zeilenwechsel, kennt `line` und
// `linecount`, und Felder, die neu rastern (Lage, Groesse, Farben), folgen ihm.
TEST_CASE("Lyrics S89: Slot Line laeuft einmal je Zeilenwechsel, kennt line/linecount, auch in der Karaoke-Ebene")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(
        ly::leseLrc("[00:10.00]MMMM\n[00:13.00]IIII\n[00:16.00]WWWW\n"));

    // was der Renderer nach den Skripten wirklich benutzt, je Knotenname
    std::map<std::string, LyricsParams> gesehen;
    MultiEffectVisualizer::setzeParamBeobachter([&](const ChainNode& n, const EffectParams& p) {
        if (const auto* lp = std::get_if<LyricsParams>(&p))
            gesehen[n.displayName.empty() ? std::string("Lyrics") : n.displayName] = *lp;
    });
    const auto kette = [](const std::string& lineCode) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        root.children = {clear};
        if (!lineCode.empty())
        {
            ChainNode lyr;
            LyricsParams lp;
            lp.initCode = "c=0;";
            lp.lineCode = lineCode;
            lyr.params = lp;
            lyr.displayName = "Zeile";
            root.children.push_back(lyr);
        }
        return root;
    };
    const auto frames = [](MultiEffectVisualizer& vis, int posMs, int n = 8) {
        MultiEffectVisualizer::notifyPlayPosition(posMs);
        for (int i = 0; i < n; ++i) vis.render(1.0f / 60.0f);
    };
    const auto starte = [](MultiEffectVisualizer& vis, ChainNode root) {
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);  // kein Beat: Strukturfelder duerfen nur ueber Line kommen
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
    };

    {
        // c zaehlt die Laeufe des Slots; xshift/yshift sind Felder, die neu rastern
        MultiEffectVisualizer vis;
        starte(vis, kette("c=c+1; xshift=c; yshift=-line*10;"));
        frames(vis, 11000);  // erste Zeile laeuft
        REQUIRE(gesehen.count("Zeile") == 1);
        CHECK(gesehen["Zeile"].xShift == 1);   // genau einmal gelaufen (auch ueber 8 Frames)
        CHECK(gesehen["Zeile"].yShift == -10); // line = 1
        frames(vis, 12500);  // dieselbe Zeile: kein Lauf
        CHECK(gesehen["Zeile"].xShift == 1);
        frames(vis, 13100);  // zweite Zeile
        CHECK(gesehen["Zeile"].xShift == 2);
        CHECK(gesehen["Zeile"].yShift == -20);
        frames(vis, 16500);  // dritte
        CHECK(gesehen["Zeile"].xShift == 3);
        CHECK(gesehen["Zeile"].yShift == -30);
        frames(vis, 5000);   // zurueckgespult vor die erste Zeile: line = 0
        CHECK(gesehen["Zeile"].xShift == 4);
        CHECK(gesehen["Zeile"].yShift == 0);
    }
    {
        // linecount
        MultiEffectVisualizer vis;
        starte(vis, kette("xshift=linecount;"));
        frames(vis, 11000);
        CHECK(gesehen["Zeile"].xShift == 3);
    }
    {
        // „Script active" aus: auch der Line-Slot ruht — der Renderer sieht die Felder des Knotens
        gesehen.clear();
        ChainNode root = kette("xshift=50;");
        root.children[1].scriptsActive = false;
        MultiEffectVisualizer vis;
        starte(vis, std::move(root));
        frames(vis, 11000);
        CHECK(gesehen.count("Zeile") == 0);  // keine Frame-Kopie, kein Skriptlauf
    }
    {
        // Karaoke-Ebene: ihr Lyrics-Knoten traegt dasselbe Feld
        gesehen.clear();
        MultiEffectVisualizer vis;
        std::get<LyricsParams>(vis.karaokeKette().children[0].params).lineCode = "yshift=-line*10;";
        (void)vis.recompileKaraoke();
        starte(vis, kette(""));
        MultiEffectVisualizer::setKaraokeEbene(true);
        frames(vis, 13100);
        MultiEffectVisualizer::setKaraokeEbene(false);
        REQUIRE(gesehen.count("Karaoke Lyrics") == 1);
        CHECK(gesehen["Karaoke Lyrics"].yShift == -20);
    }
    {
        // Lyrics Trail: stellt Line die Zeilen verschieden, weht die ALTE Zeile
        // von dort weg, wo sie stand — nicht von der Lage der neuen. Zeile 1
        // steht oben, Zeile 2 unten; kurz nach dem Wechsel zeichnet der Trail
        // (rot) die alte Zeile oben.
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        LyricsParams lp;
        lp.previewLines = 0;
        lp.lineCode = "valign = if(equal(line,1),0,2);";
        lyr.params = lp;
        ChainNode trail;
        LyricsTrailParams tp;
        tp.holdMs = 400;
        tp.xShift = 0;
        tp.yShift = 0;
        tp.useOwnColor = true;
        tp.color = 0xFF0000;
        trail.params = tp;
        root.children = {clear, lyr, trail};
        MultiEffectVisualizer vis;
        starte(vis, std::move(root));
        frames(vis, 11000);      // Zeile 1 laeuft (oben)
        frames(vis, 13060, 2);   // 60 ms in Zeile 2 (unten) — der Trail haelt Zeile 1
        const QImage bild = vis.debugGrabRootSurface();
        REQUIRE(!bild.isNull());
        int rotOben = 0;
        int rotUnten = 0;
        for (int y = 0; y < bild.height(); ++y)
            for (int x = 0; x < bild.width(); ++x)
                if (bild.pixel(x, y) == qRgb(255, 0, 0)) ++(y < bild.height() / 2 ? rotOben : rotUnten);
        CHECK(rotOben > 100);
        CHECK(rotUnten == 0);
    }
    {
        // das Feld ueberlebt Speichern und Laden
        ChainNode root = kette("xshift=line;");
        QStringList report;
        const ChainNode zurueck = nodeFromJson(nodeToJson(root), &report);
        REQUIRE(zurueck.children.size() == 2);
        CHECK(std::get<LyricsParams>(zurueck.children[1].params).lineCode == "xshift=line;");
    }

    MultiEffectVisualizer::setzeParamBeobachter({});
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S88 (Folgefrage Patrik zu K6): dieselben Mischarten am Lyrics-Knoten der
// Kette — „die Kette gewinnt" soll nicht aermer sein als die Ebene. Gerechnet
// im Text-Shader; ab Subtract mischt nur die Fuellung, die Kontur bleibt normal.
TEST_CASE("Lyrics S88: Mischarten am Knoten rechnen wie in der Karaoke-Ebene")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setKaraokeEbene(false);
    MultiEffectVisualizer::setSongtextTitelVersatz(0);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
    MultiEffectVisualizer::notifyPlayPosition(11000);
    const QRgb grund = qRgb(64, 128, 192);
    const QString snap = qEnvironmentVariable("LUMIVIZ_KARAOKE_SNAP");

    const auto bild = [&](int blend, const char* name) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        ClearParams cp;
        cp.color = 0x4080C0;
        clear.params = cp;
        ChainNode lyr;
        LyricsParams lp;
        lp.blend = blend;
        lyr.params = lp;
        root.children = {clear, lyr};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(100000);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        for (int i = 0; i < 4; ++i) vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        if (!snap.isEmpty()) img.save(snap + QStringLiteral("/knoten_%1.png").arg(QLatin1String(name)));
        return img;
    };

    const QImage normal = bild(0, "0_replace");
    std::vector<QPoint> innen;
    for (int y = 0; y < normal.height(); ++y)
        for (int x = 0; x < normal.width(); ++x)
            if (normal.pixel(x, y) == qRgb(255, 255, 255)) innen.emplace_back(x, y);
    REQUIRE(innen.size() > 300);
    int schwarzNormal = 0;
    for (int y = 0; y < normal.height(); ++y)
        for (int x = 0; x < normal.width(); ++x)
            if (normal.pixel(x, y) == qRgb(0, 0, 0)) ++schwarzNormal;
    REQUIRE(schwarzNormal > 100);  // die Kontur bei Replace

    struct Fall
    {
        int art;
        const char* name;
        int r, g, b;
    };
    // Quelle weiss, Deckung 1, ueber (64,128,192) — dieselbe Tabelle wie bei der Ebene
    const Fall faelle[] = {
        {1, "1_additive", 255, 255, 255}, {2, "2_5050", 160, 192, 224},
        {3, "3_subtract", 0, 0, 0},       {4, "4_invert", 191, 127, 63},
        {5, "5_xor", 191, 127, 63},       {6, "6_multiply", 64, 128, 192},
        {7, "7_maximum", 255, 255, 255},  {8, "8_minimum", 64, 128, 192},
    };
    for (const Fall& f : faelle)
    {
        CAPTURE(f.name);
        const QImage img = bild(f.art, f.name);
        int falsch = 0;
        for (const QPoint& p : innen)
        {
            const QRgb c = img.pixel(p);
            if (std::abs(qRed(c) - f.r) > 2 || std::abs(qGreen(c) - f.g) > 2 ||
                std::abs(qBlue(c) - f.b) > 2)
                ++falsch;
        }
        CHECK(falsch == 0);
        CHECK(img.pixel(5, 5) == grund);  // ausserhalb des Texts unveraendert
        if (f.art >= 3)  // ab Subtract bleibt die Kontur normal: schwarzer Ring
        {
            int schwarz = 0;
            for (int y = 0; y < img.height(); ++y)
                for (int x = 0; x < img.width(); ++x)
                    if (img.pixel(x, y) == qRgb(0, 0, 0)) ++schwarz;
            CHECK(schwarz > 100);
        }
    }
    // ein Wert jenseits der Liste wird auf die letzte Art geklemmt, nicht zu Unsinn
    CHECK(bild(99, "99_geklemmt") == bild(8, "8_minimum"));

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S88: seit Ids nie wiederverwendet werden (S87), blieb die Laufzeit eines im
// Panel entfernten Knotens bis zum naechsten Laden liegen (Texturen, Skript-
// Hosts, ein laufender Video-/Kamera-Feed). Der Render-Thread raeumt sie nach
// dem Edit ab — und nur sie.
TEST_CASE("Laufzeiten S88: ein entfernter Knoten nimmt seine Laufzeit mit, die uebrigen behalten ihre")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:01.00]MMMM\n"));
    MultiEffectVisualizer::notifyPlayPosition(2000);

    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    clear.params = ClearParams{};
    ChainNode a;
    a.params = LyricsParams{};
    ChainNode liste;  // der zweite Lyrics-Knoten steckt in einer Liste
    liste.params = ListParams{};
    ChainNode b;
    b.params = LyricsParams{};
    liste.children = {b};
    root.children = {clear, a, liste};
    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(160, 120));
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 160, 120);
    vis.render(1.0f / 60.0f);
    vis.render(1.0f / 60.0f);

    const uint64_t idA = vis.chain().children[1].nodeId;
    const uint64_t idB = vis.chain().children[2].children[0].nodeId;
    REQUIRE(idA != 0);
    REQUIRE(idB != 0);
    REQUIRE(idA != idB);
    REQUIRE(vis.debugHatLaufzeit(idA));
    REQUIRE(vis.debugHatLaufzeit(idB));
    const std::size_t vorher = vis.debugLaufzeitAnzahl();

    // Gegenprobe: ein Edit, der nichts entfernt, laesst alle Laufzeiten stehen
    vis.recompileChain();
    vis.render(1.0f / 60.0f);
    CHECK(vis.debugLaufzeitAnzahl() == vorher);
    CHECK(vis.debugHatLaufzeit(idA));

    // Knoten A entfernen (wie das Panel: Baum aendern, neu uebersetzen)
    vis.chain().children.erase(vis.chain().children.begin() + 1);
    vis.recompileChain();
    CHECK(vis.debugHatLaufzeit(idA));  // erst der Render-Thread raeumt ab
    vis.render(1.0f / 60.0f);
    CHECK_FALSE(vis.debugHatLaufzeit(idA));
    CHECK(vis.debugHatLaufzeit(idB));  // der in der Liste bleibt
    CHECK(vis.debugLaufzeitAnzahl() == vorher - 1);

    // ein neuer Knoten an derselben Stelle: neue Id, eigene, frische Laufzeit
    ChainNode neu;
    neu.params = LyricsParams{};
    vis.chain().children.insert(vis.chain().children.begin() + 1, neu);
    vis.recompileChain();
    vis.render(1.0f / 60.0f);
    const uint64_t idNeu = vis.chain().children[1].nodeId;
    CHECK(idNeu != idA);
    CHECK(vis.debugHatLaufzeit(idNeu));
    CHECK_FALSE(vis.debugHatLaufzeit(idA));

    // die ganze Liste entfernen: auch die Laufzeit des Knotens darin geht
    vis.chain().children.pop_back();
    vis.recompileChain();
    vis.render(1.0f / 60.0f);
    CHECK_FALSE(vis.debugHatLaufzeit(idB));
    CHECK(vis.debugHatLaufzeit(idNeu));

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// S88: zwei Reste des Trails. (1) Er las die GESPEICHERTEN Felder des
// Lyrics-Knotens — mit einem Skript („Karaoke Bounce") stand die alte Zeile an
// der Grundposition. (2) Unsynchronen Text kannte er nicht; dort rueckt der
// Lyrics-Knoten alle N Beats eine Zeile weiter.
TEST_CASE("Lyrics Trail S88: folgt den Skriptwerten des Lyrics-Knotens und kennt unsynchronen Text")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);

    struct Bild
    {
        int hell = 0;
        double y = 0.0;
    };
    // Die Zeilen des Lyrics-Knotens sind dunkel, der Trail zeichnet in eigener,
    // heller Farbe — helle Pixel gehoeren damit allein dem Trail.
    const auto zeichne = [](LyricsParams lp, bool trailAn, int frames, int beatAlle) {
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lp.fontHeight = -24;
        lp.outline = false;
        lp.previewLines = 0;
        lp.color = 0x202020;
        lp.colorNext = 0x202020;
        lyr.params = lp;
        ChainNode trail;
        LyricsTrailParams tp;
        tp.yShift = 0;
        tp.useOwnColor = true;
        tp.color = 0xFFFFFF;
        trail.params = tp;  // Haltezeit: Vorgabe 300 ms
        trail.enabled = trailAn;
        root.children = {clear, lyr, trail};
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(beatAlle);
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        for (int i = 0; i < frames; ++i) vis.render(1.0f / 60.0f);
        const QImage img = vis.debugGrabRootSurface();
        Bild b;
        double summe = 0.0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 200)
                {
                    ++b.hell;
                    summe += y;
                }
        if (b.hell > 0) b.y = summe / b.hell;
        return b;
    };

    // --- (1) Skriptwerte: 100 ms nach dem Zeilenwechsel
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM\n[00:13.00]IIII\n"));
    MultiEffectVisualizer::notifyPlayPosition(13100);
    LyricsParams schlicht;
    const Bild unten = zeichne(schlicht, true, 2, 100000);
    REQUIRE(unten.hell > 0);
    CHECK(zeichne(schlicht, false, 2, 100000).hell == 0);  // Gegenprobe: hell ist nur der Trail
    LyricsParams skript;
    skript.initCode = "yshift = -40;";  // das Skript hebt die Zeile um 40 % der Hoehe
    const Bild oben = zeichne(skript, true, 2, 100000);
    REQUIRE(oben.hell > 0);
    CHECK(oben.y < unten.y - 60.0);  // 40 % von 240 px = 96 px — der Trail geht mit
    CHECK(oben.hell == doctest::Approx(unten.hell).epsilon(0.05));  // dieselbe Zeile

    // --- (2) unsynchron: Beat auf Frame 0 rueckt von „MMMM" zu „IIII"
    MultiEffectVisualizer::setSongtext(ly::leseLrc("MMMM\nIIII\nIIII\n"));
    LyricsParams block;
    block.showSyncMark = false;
    block.beatsPerLine = 1;
    const Bild alt = zeichne(block, true, 2, 100000);
    CHECK(alt.hell > 0);  // die eben beendete Zeile steht da
    CHECK(alt.hell == doctest::Approx(unten.hell).epsilon(0.05));  // es ist „MMMM"
    CHECK(alt.y == doctest::Approx(unten.y).epsilon(0.02));        // an derselben Stelle
    CHECK(zeichne(block, false, 2, 100000).hell == 0);
    // die Haltezeit (300 ms = 18 Frames) ist um: nichts mehr
    CHECK(zeichne(block, true, 30, 100000).hell == 0);
    // Gegenprobe: ohne Weiterruecken (beats per line 0) gibt es keinen Wechsel
    LyricsParams steht = block;
    steht.beatsPerLine = 0;
    CHECK(zeichne(steht, true, 2, 100000).hell == 0);
    // die Marke „(not synced)" verschiebt die Zeile — der Trail rechnet sie ein
    LyricsParams marke = block;
    marke.showSyncMark = true;
    marke.vAlign = 0;  // oben: jede Reihe darueber schiebt nach unten
    LyricsParams ohneMarke = marke;
    ohneMarke.showSyncMark = false;
    CHECK(zeichne(marke, true, 2, 100000).y > zeichne(ohneMarke, true, 2, 100000).y + 5.0);

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

// Wunsch Patrik S87: der Trail in einer Effect List, die ihr Bild behaelt —
// Blitter Feedback zieht die alte Zeile nach hinten (zur Bildmitte, kleiner),
// Fadeout laesst sie verblassen. Genau diese Liste steht in
// asset/effectchain/20261001.mix.lvfx2.
//
// Abstimmung an gerenderten Bildern: `scale` 33 ist zu langsam — die Zeile
// wandert dann weniger als ein Pixel je Frame, das Fadeout frisst ihre
// Vorderkante schneller, als sie vorankommt, und sie schrumpft zu einem
// Strich. 35 traegt sie sichtbar weg. Ein Blur dazu hilft nicht: ohne
// `roundUp` loescht er duenne Striche, mit `roundUp` bleibt ein Grauschleier.
TEST_CASE("Lyrics Trail S87: in einer Liste mit Rueckkopplung verweht die Zeile nach hinten")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtext(
        ly::leseLrc("[00:10.00]MMMM MMMM MMMM\n[00:13.00]IIII\n[00:30.00]ende\n"));

    const QJsonObject liste = QJsonDocument::fromJson(R"({
        "type": "list", "name": "Lyrics Trail", "enabled": true,
        "clearEveryFrame": false, "blendIn": 0, "blendOut": 4,
        "inAdjustAlpha": 128, "outAdjustAlpha": 128,
        "bufferIn": 0, "bufferOut": 0, "bufferInInvert": false, "bufferOutInvert": false,
        "onBeatRender": false, "onBeatFrames": 1, "useCode": false,
        "initCode": "", "frameCode": "",
        "children": [
            {"type": "blitterFeedback", "name": "Drift back", "enabled": true,
             "scale": 35, "scale2": 35, "onBeat": false, "blend": false, "subpixel": true,
             "initCode": "", "frameCode": "", "beatCode": ""},
            {"type": "fadeout", "name": "Fadeout", "enabled": true, "fadeLen": 4, "color": 0,
             "initCode": "", "frameCode": "", "beatCode": ""},
            {"type": "lyricsTrail", "name": "Lyrics Trail", "enabled": true,
             "source": "", "holdMs": 120, "xShift": 0, "yShift": -7,
             "useOwnColor": false, "color": 16777215, "blend": 0,
             "initCode": "", "frameCode": "", "beatCode": ""}
        ]})").object();
    REQUIRE_FALSE(liste.isEmpty());

    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    clear.params = ClearParams{};
    ChainNode lyr;
    LyricsParams lp;
    lp.previewLines = 0;
    lp.fontHeight = -28;
    lyr.params = lp;
    lyr.enabled = false;  // nur die Spur zaehlen, nicht die laufende Zeile
    QStringList bericht;
    root.children = {clear, nodeFromJson(liste, &bericht), lyr};
    CHECK(bericht.isEmpty());
    REQUIRE(root.children[1].children.size() == 3);
    REQUIRE(std::holds_alternative<LyricsTrailParams>(root.children[1].children[2].params));

    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(480, 270));
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 480, 270);

    struct Bild
    {
        double summe = 0.0;  ///< Helligkeit ueber alles
        double y = 0.0;      ///< Schwerpunkt in y
    };
    const QString schnapp = qEnvironmentVariable("LUMIVIZ_TRAIL_SNAP");
    const auto bildBei = [&](int frame) {
        const QImage img = vis.debugGrabRootSurface();
        if (!schnapp.isEmpty()) img.save(schnapp + QStringLiteral("/trail_%1.png").arg(frame, 3, 10, QLatin1Char('0')));
        Bild b;
        double gewichtet = 0.0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
            {
                const int g = qGray(img.pixel(x, y));
                b.summe += g;
                gewichtet += static_cast<double>(g) * y;
            }
        if (b.summe > 0.0) b.y = gewichtet / b.summe;
        return b;
    };

    // ab 12,95 s in Schritten von 1/60 s; die Zeile wechselt bei 13,00 s (Frame 3)
    std::vector<Bild> bilder;
    for (int frame = 0; frame <= 120; ++frame)
    {
        MultiEffectVisualizer::notifyPlayPosition(12950 + frame * 1000 / 60);
        vis.render(1.0f / 60.0f);
        bilder.push_back(bildBei(frame));
    }
    CHECK(bilder[1].summe == 0.0);  // vor dem Wechsel: nichts
    const Bild& frisch = bilder[8];  // waehrend der Haltezeit: die Zeile steht da
    const Bild& spaeter = bilder[40];  // 0,5 s nach der Haltezeit: sie haengt noch nach …
    const Bild& ende = bilder[120];
    CHECK(frisch.summe > 0.0);
    CHECK(spaeter.summe > 0.0);
    CHECK(spaeter.summe < frisch.summe);       // … aber schwaecher
    CHECK(spaeter.y < frisch.y - 3.0);         // und zur Bildmitte hin gewandert (nach hinten)
    CHECK(ende.summe < frisch.summe * 0.05);   // nach 2 s ist sie weg

    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

TEST_CASE("Lyrics S87: jede mitgelieferte Voreinstellung zeichnet; die Skripte tun, was sie sagen")
{
    GlUmgebung gl;
    if (!gl.ok) return;
    namespace ly = lumi::multieffect::lyrics;
    namespace np = lumi::multieffect::nodepresets;

    MultiEffectVisualizer::notifyPlayState(false);
    MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:20.00]MMMM\n"));
    MultiEffectVisualizer::notifyPlayPosition(12000);

    const auto eintraege = np::list(QStringLiteral("lyrics"));
    REQUIRE(eintraege.size() >= 13);
    for (const np::Entry& e : eintraege)
    {
        CAPTURE(e.name.toStdString());
        EffectParams params = LyricsParams{};
        REQUIRE(np::load(e.path, QStringLiteral("lyrics"), params, nullptr));
        // der Song-Slot liegt am Knoten — aus der Datei dazulesen
        QFile f(e.path);
        REQUIRE(f.open(QIODevice::ReadOnly));
        const std::string song = QJsonDocument::fromJson(f.readAll())
                                     .object()
                                     .value(QStringLiteral("node"))
                                     .toObject()
                                     .value(QStringLiteral("songCode"))
                                     .toString()
                                     .toStdString();

        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        clear.params = ClearParams{};
        ChainNode lyr;
        lyr.params = params;
        lyr.songCode = song;
        root.children = {clear, lyr};

        std::set<std::uint32_t> farben;
        std::set<int> xs, ys, lagen;
        MultiEffectVisualizer::setzeParamBeobachter(
            [&](const ChainNode&, const EffectParams& kopie) {
                if (const auto* lp = std::get_if<LyricsParams>(&kopie))
                {
                    farben.insert(lp->color);
                    xs.insert(lp->xShift);
                    ys.insert(lp->yShift);
                    lagen.insert(lp->vAlign);
                }
            });
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(320, 240));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, 320, 240);
        vis.setBeatPeriodOverride(2);  // jeder zweite Frame ein Beat
        for (int i = 0; i < 80; ++i) vis.render(1.0f / 60.0f);
        MultiEffectVisualizer::setzeParamBeobachter({});

        // (1) es steht Text im Bild
        const QImage img = vis.debugGrabRootSurface();
        int hell = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                if (qGray(img.pixel(x, y)) > 90) ++hell;
        CHECK(hell > 50);

        // (2) die bewegten Varianten bewegen wirklich etwas
        if (e.name == QLatin1String("Karaoke Beat Colours"))
        {
            CHECK(farben.size() > 5);  // je Beat eine neue Farbe
        }
        else if (e.name == QLatin1String("Karaoke Song Colours"))
        {
            CHECK(farben.size() == 1);  // eine Farbe je Song, kein Wechsel im Lauf
            CHECK(*farben.begin() != 0xFFFFFFu);
        }
        else if (e.name == QLatin1String("Karaoke Bounce"))
        {
            CHECK(xs.size() > 1);
            CHECK(ys.size() > 1);
            CHECK(*xs.begin() >= -2);
            CHECK(*xs.rbegin() <= 2);
            CHECK(*ys.begin() >= -9);
            CHECK(*ys.rbegin() <= -6);
        }
        else if (e.name == QLatin1String("Karaoke Flip"))
        {
            CHECK(lagen == std::set<int>{0, 2});  // unten und oben, nie Mitte
        }
    }
    MultiEffectVisualizer::setSongtext(ly::Songtext{});
    MultiEffectVisualizer::notifyPlayPosition(0);
}

TEST_CASE("Modulation F4d: LFO-Rate in Beats — im Takt, phasengleich, folgt dem Tempo")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Bloom.intensity (Regler 1, Bereich 0..8), Tiefe 0.1 -> Ausschlag +-0.8.
    // Fester Beat alle `periode` Frames bei 60 fps; gelesen wird nach `frames`.
    const auto wert = [](ModQuelle q, double beats, int periode, int frames) {
        ChainNode leaf;
        leaf.params = BloomParams{};
        FeldModulation m = mod("intensity", q, 0.1, 7.0);  // rateHz ruht im Takt
        m.rateBeats = beats;
        leaf.mods = {m};
        ChainNode root;
        root.params = ListParams{};
        root.children.push_back(std::move(leaf));
        std::optional<EffectParams> gesehen;
        MultiEffectVisualizer::setzeParamBeobachter(
            [&](const ChainNode&, const EffectParams& kopie) {
                if (std::holds_alternative<BloomParams>(kopie)) gesehen = kopie;
            });
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.setBeatPeriodOverride(periode);
        vis.initialize();
        vis.resize(QSize(kSize, kSize));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
        for (int i = 0; i < frames; ++i) vis.render(1.0f / 60.0f);
        MultiEffectVisualizer::setzeParamBeobachter({});
        return gesehen ? static_cast<double>(std::get<BloomParams>(*gesehen).intensity)
                       : -1.0;
    };
    // Beat alle 30 Frames (120 BPM = der Startwert der Beat-Uhr). Nach 16
    // Frames steht die Uhr bei einem halben Beat.
    CHECK(wert(ModQuelle::LfoSaege, 1.0, 30, 16) == doctest::Approx(1.0).epsilon(1e-4));
    CHECK(wert(ModQuelle::LfoDreieck, 1.0, 30, 16) == doctest::Approx(1.8).epsilon(1e-4));
    CHECK(wert(ModQuelle::LfoSaege, 2.0, 30, 16) == doctest::Approx(0.6).epsilon(1e-4));
    // auf dem Beat beginnt die Schwingung: Saege ganz unten
    CHECK(wert(ModQuelle::LfoSaege, 1.0, 30, 31) == doctest::Approx(0.2).epsilon(1e-4));
    // Rechteck ueber zwei Beats: erster Beat oben, zweiter unten
    CHECK(wert(ModQuelle::LfoRechteck, 2.0, 30, 16) == doctest::Approx(1.8).epsilon(1e-4));
    CHECK(wert(ModQuelle::LfoRechteck, 2.0, 30, 46) == doctest::Approx(0.2).epsilon(1e-4));
    // anderes Tempo (Beat alle 20 Frames): nach einigen Beats hat die Uhr den
    // Abstand gelernt — ein halber Beat nach dem siebten Beat ist wieder Mitte
    CHECK(wert(ModQuelle::LfoSaege, 1.0, 20, 20 * 7 + 11) ==
          doctest::Approx(1.0).epsilon(0.03));
    // Gegenprobe: ohne Takt (rateBeats 0) laeuft derselbe LFO frei mit 7 Hz
    CHECK(wert(ModQuelle::LfoSaege, 0.0, 30, 16) != doctest::Approx(1.0).epsilon(1e-3));

    // Datei: rateBeats laeuft mit und fehlt, wenn der LFO frei laeuft
    ChainNode n;
    n.params = BloomParams{};
    FeldModulation m = mod("intensity", ModQuelle::LfoSinus, 0.1, 2.0);
    m.rateBeats = 4.0;
    n.mods = {m, mod("radius", ModQuelle::LfoSinus, 0.1, 2.0)};
    ChainNode root;
    root.params = ListParams{};
    root.children = {n};
    QStringList bericht;
    const ChainNode gelesen = chainFromJson(chainToJson(root), &bericht);
    REQUIRE(gelesen.children.size() == 1);
    REQUIRE(gelesen.children[0].mods.size() == 2);
    CHECK(gelesen.children[0].mods[0].rateBeats == doctest::Approx(4.0));
    CHECK(gelesen.children[0].mods[1].rateBeats == doctest::Approx(0.0));
}

TEST_CASE("Modulation F4b: Farbfelder — Helligkeit, Farbton, Kanal")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Rechteck-LFO mit Rate 0 steht fest auf +1: Aenderung = Tiefe
    const auto farbe = [](std::uint32_t start, ModZiel ziel, double tiefe) {
        ClearParams c;
        c.color = start;
        ChainNode n;
        n.params = c;
        FeldModulation m = mod("color", ModQuelle::LfoRechteck, tiefe);
        m.ziel = ziel;
        n.mods = {m};
        const auto k = laufeKnoten(n);
        REQUIRE(k.has_value());
        return static_cast<std::uint32_t>(std::get<ClearParams>(*k).color);
    };
    const auto kanal = [](std::uint32_t c, int k) {
        return static_cast<int>((c >> (16 - 8 * k)) & 0xFF);
    };
    // Kanal: nur dieser bewegt sich, geklemmt
    CHECK(farbe(0x804020, ModZiel::Rot, 0.25) == 0xC04020);
    CHECK(farbe(0x804020, ModZiel::Blau, 2.0) == 0x8040FF);
    CHECK(farbe(0x804020, ModZiel::Gruen, -1.0) == 0x800020);
    // Farbton: ein Drittel Umlauf macht aus Rot Gruen, ein ganzer nichts
    const std::uint32_t gruen = farbe(0xFF0000, ModZiel::Farbton, 1.0 / 3.0);
    CHECK(kanal(gruen, 1) == 255);
    CHECK(kanal(gruen, 0) <= 1);
    CHECK(kanal(gruen, 2) <= 1);
    // Helligkeit: bis zum Anschlag hell, Verhaeltnis der Kanaele bleibt
    const std::uint32_t hell = farbe(0x804020, ModZiel::Helligkeit, 1.0);
    CHECK(kanal(hell, 0) == 255);
    CHECK(kanal(hell, 1) == doctest::Approx(128).epsilon(0.02));
    CHECK(kanal(hell, 2) == doctest::Approx(64).epsilon(0.04));
    CHECK(farbe(0x804020, ModZiel::Helligkeit, -1.0) == 0x000000);
    // `Wert` an einem Farbfeld gilt als Helligkeit
    CHECK(farbe(0x804020, ModZiel::Wert, -1.0) == 0x000000);
    // Gegenprobe: Tiefe 0 laesst die Farbe bitgenau stehen
    CHECK(farbe(0x804021, ModZiel::Helligkeit, 0.0) == 0x804021);
    CHECK(farbe(0x804021, ModZiel::Farbton, 0.0) == 0x804021);

    // Datei: das Ziel laeuft mit, an Zahlfeldern steht es nicht
    ChainNode n;
    n.params = ClearParams{};
    FeldModulation m = mod("color", ModQuelle::Bass, 0.5);
    m.ziel = ModZiel::Farbton;
    n.mods = {m};
    ChainNode root;
    root.params = ListParams{};
    root.children = {n};
    const QJsonObject json = chainToJson(root);
    QStringList bericht;
    const ChainNode gelesen = chainFromJson(json, &bericht);
    REQUIRE(gelesen.children.size() == 1);
    REQUIRE(gelesen.children[0].mods.size() == 1);
    CHECK(gelesen.children[0].mods[0].ziel == ModZiel::Farbton);
}

TEST_CASE("Feldsteuerung S86: Text und AVI tragen ein Parameter-Skript")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Text: Live-Feld folgt dem Frame-Slot, die Lage (rastert neu) nur Init/Beat (E3)
    TextParams t;
    t.text = "EINS;ZWEI";
    t.xShift = 5;
    t.initCode = "yshift = 12;";
    t.frameCode = "normspeed = 33; xshift = xshift + 1;";
    EffectParams p = t;
    const auto k = laufe(p, p.index());
    REQUIRE(k.has_value());
    const auto& kt = std::get<TextParams>(*k);
    CHECK(kt.normSpeed == 33);
    CHECK(kt.yShift == 12);  // aus Init angenommen
    CHECK(kt.xShift == 6);   // Frame 1 (frisch) angenommen, Frame 2 verworfen

    // Datei: die Skripte laufen mit
    AviParams a;
    a.frameCode = "persist = 3;";
    ChainNode n;
    n.params = a;
    ChainNode n2;
    n2.params = t;
    ChainNode root;
    root.params = ListParams{};
    root.children = {n, n2};
    QStringList bericht;
    const ChainNode gelesen = chainFromJson(chainToJson(root), &bericht);
    REQUIRE(gelesen.children.size() == 2);
    CHECK(std::get<AviParams>(gelesen.children[0].params).frameCode == "persist = 3;");
    CHECK(std::get<TextParams>(gelesen.children[1].params).initCode == "yshift = 12;");
}

TEST_CASE("Feldsteuerung S86: Dynamic Movement — blend bleibt eine freie Skriptvariable")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // 18 Presets der Sammlung fuehren in Dynamic-Movement-Skripten eine eigene
    // Variable `blend`. Sie darf den Schalter „Blend" nicht umlegen; `wrap`
    // dagegen ist Feld und folgt dem Skript.
    DynamicMovementParams dm;
    dm.blend = false;
    dm.wrap = false;
    dm.frameCode = "blend = 1; wrap = 1;";
    EffectParams p = dm;
    const auto k = laufe(p, p.index());
    REQUIRE(k.has_value());
    CHECK(std::get<DynamicMovementParams>(*k).blend == false);
    CHECK(std::get<DynamicMovementParams>(*k).wrap == true);
}

TEST_CASE("Feldsteuerung S86: SuperScope — pointcount reicht an n weiter, n bleibt der Herr")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Das Punkt-Skript zaehlt seine Laeufe in reg20; ein Bloom-Skript dahinter
    // liest den Zaehler des Frames in ein Feld (radius 1..32).
    const auto punkte = [](const std::string& init, const std::string& frame) {
        SuperScopeParams sp;
        sp.pointCount = 8;
        sp.initCode = init;
        sp.frameCode = "reg20 = 0; " + frame;
        sp.pointCode = "reg20 = reg20 + 1; x = i; y = 0;";
        ChainNode scope;
        scope.params = sp;
        BloomParams b;
        b.frameCode = "radius = reg20;";
        ChainNode sonde;
        sonde.params = b;
        ChainNode root;
        root.params = ListParams{};
        root.children.push_back(scope);
        root.children.push_back(sonde);
        std::optional<EffectParams> gesehen;
        MultiEffectVisualizer::setzeParamBeobachter(
            [&](const ChainNode&, const EffectParams& kopie) {
                if (std::holds_alternative<BloomParams>(kopie)) gesehen = kopie;
            });
        MultiEffectVisualizer vis;
        vis.setChain(std::move(root));
        vis.initialize();
        vis.resize(QSize(kSize, kSize));
        QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
        for (int i = 0; i < 3; ++i) vis.render(1.0f / 60.0f);
        MultiEffectVisualizer::setzeParamBeobachter({});
        return gesehen ? std::get<BloomParams>(*gesehen).radius : -1;
    };
    CHECK(punkte("", "") == 8);                    // Regler
    CHECK(punkte("n = 5;", "") == 5);              // n aus Init (AVS)
    CHECK(punkte("", "pointcount = 12;") == 12);   // Feldname wirkt
    CHECK(punkte("pointcount = 12;", "n = 6;") == 6);  // n hat das letzte Wort
}

TEST_CASE("Feldsteuerung S86: Auswahlfeld ohne bekannten Bereich bleibt, wenn ein Skript laeuft")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Clear.blend steht mit 0..0 in der Tabelle (Liste entsteht im Panel). Bis
    // S86 klemmte das Zuruecklesen jedes solche Feld auf 0, sobald der Knoten
    // irgendein Skript trug (bei der Liste: blendOut = Ignore = kein Bild).
    ClearParams c;
    c.blend = 2;
    c.frameCode = "zz = 1;";
    EffectParams p = c;
    const auto k = laufe(p, p.index());
    REQUIRE(k.has_value());
    CHECK(std::get<ClearParams>(*k).blend == 2);
    // … und laesst sich weiter per Skript setzen
    c.frameCode = "blend = 3;";
    p = c;
    const auto k2 = laufe(p, p.index());
    REQUIRE(k2.has_value());
    CHECK(std::get<ClearParams>(*k2).blend == 3);
}

TEST_CASE("Feldsteuerung S86: Effect List — Felder per Listen-Code, Alphas nach r_list")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    const auto liste = [](const std::string& frame) {
        ListParams lp;
        lp.useCode = true;
        lp.frameCode = frame;
        EffectParams p = lp;
        // Wurzel und Blatt sind beide Listen — nur das Blatt traegt Code
        const auto k = laufe(p, p.index());
        REQUIRE(k.has_value());
        return std::get<ListParams>(*k);
    };
    // Feldname 0..255 wirkt, solange alphain unberuehrt bleibt
    CHECK(liste("inadjustalpha = 200; bufferout = 3; onbeatframes = 7;").inAdjustAlpha == 200);
    CHECK(liste("inadjustalpha = 200; bufferout = 3; onbeatframes = 7;").bufferOut == 3);
    CHECK(liste("inadjustalpha = 200; bufferout = 3; onbeatframes = 7;").onBeatFrames == 7);
    // alphain (0..1) gewinnt, abgeschnitten wie (int)(alphain*255)
    CHECK(liste("inadjustalpha = 200; alphain = 0.5;").inAdjustAlpha == 127);
    // unberuehrt: Reglerwert
    CHECK(liste("zz = 1;").outAdjustAlpha == 128);
}

TEST_CASE("Effect List S86: alphain/alphaout kommen je Frame aus dem Regler (r_list:404)")
{
    GlUmgebung gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — Test uebersprungen");
        return;
    }
    // Das Skript schreibt alphaout fort und spiegelt den gesehenen Wert nach
    // reg10. AVS belegt alphaout jeden Frame neu vor: nach beliebig vielen
    // Frames steht 128/255 * 0.5 — nicht (0.5)^n.
    ListParams lp;
    lp.useCode = true;
    lp.frameCode = "reg10 = alphaout; alphaout = alphaout * 0.5;";
    ChainNode liste;
    liste.params = lp;
    ChainNode sonde;  // liest reg10 ueber ein Parameter-Skript in ein Feld
    BloomParams b;
    b.frameCode = "threshold = reg10;";
    sonde.params = b;
    ChainNode root;
    root.params = ListParams{};
    root.children.push_back(liste);
    root.children.push_back(sonde);

    std::optional<EffectParams> gesehen;
    MultiEffectVisualizer::setzeParamBeobachter(
        [&](const ChainNode&, const EffectParams& kopie) {
            if (std::holds_alternative<BloomParams>(kopie)) gesehen = kopie;
        });
    MultiEffectVisualizer vis;
    vis.setChain(std::move(root));
    vis.initialize();
    vis.resize(QSize(kSize, kSize));
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, kSize, kSize);
    for (int i = 0; i < 4; ++i) vis.render(1.0f / 60.0f);
    REQUIRE(gesehen.has_value());
    CHECK(std::get<BloomParams>(*gesehen).threshold == doctest::Approx(128.0 / 255.0));

    // Mit Modulation sieht das Skript den modulierten Wert (E5)
    vis.chain().children[0].mods = {mod("outAdjustAlpha", ModQuelle::LfoRechteck, 0.25)};
    vis.recompileChain();
    for (int i = 0; i < 2; ++i) vis.render(1.0f / 60.0f);
    MultiEffectVisualizer::setzeParamBeobachter({});
    CHECK(std::get<BloomParams>(*gesehen).threshold == doctest::Approx(191.0 / 255.0));
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
