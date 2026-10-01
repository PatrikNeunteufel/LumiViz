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

#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
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
