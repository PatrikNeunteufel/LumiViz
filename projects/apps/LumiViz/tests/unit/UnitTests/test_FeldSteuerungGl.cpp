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
