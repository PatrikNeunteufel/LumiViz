/**
 * @file test_ShapeKnoten.cpp
 * @brief Klang aus einer Form, Stufe 19b: der Knoten Shape der Effect Chain
 *        (docs/audio/Shape_Konzept.md §10; Entscheide Patrik S121: V1 a bis V4 a)
 *
 * Der erste Fall misst nur: was ein Feld Per point und was die Form selbst je
 * Bild kosten. Er meldet Zahlen und prueft nur, dass es traegt. Danach die
 * aufbereitete Zeichnung (`shape/Zeichenform.hpp`, ohne Qt-Anwendung), der
 * Knoten in der Datei, und zwei Faelle mit einem GL-Kontext ohne Fenster: der
 * Knoten im Bild — mit eigenen Zeichnungen in einem Wegwerf-Ordner — und die
 * mitgelieferten Presets aus `asset/nodepresets/shape`.
 * `LUMIVIZ_SHAPE_KNOTEN_BILD=<datei.png>` legt je Preset ein Bild auf ein Blatt.
 */

#include <doctest.h>

#include "scripting/ScriptSlotHost.hpp"
#include "shape/Kontur.hpp"
#include "shape/Zeichenform.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/FieldBindings.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QColor>
#include <QDir>
#include <QGuiApplication>
#include <QImage>
#include <QJsonArray>
#include <QJsonObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPainterPath>
#include <QStringList>
#include <QSurfaceFormat>
#include <QTemporaryDir>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

using lumi::scripting::ScriptSlotHost;
using Slot = ScriptSlotHost::Slot;
using namespace lumi::shape;
using namespace lumi::multieffect;

namespace
{

/// Eine feste Rechenarbeit, gegen die sich eine Messung vergleichen laesst:
/// drosselt der Rechner, dauert auch sie laenger (S106).
double taktprobeMs()
{
    const auto start = std::chrono::steady_clock::now();
    volatile double summe = 0.0;
    for (int i = 1; i <= 4'000'000; ++i) summe = summe + std::sqrt(static_cast<double>(i));
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

QPainterPath quadrat(double links = 0.0, double oben = 0.0, double kante = 100.0)
{
    QPainterPath pfad;
    pfad.moveTo(links, oben);
    pfad.lineTo(links + kante, oben);
    pfad.lineTo(links + kante, oben + kante);
    pfad.lineTo(links, oben + kante);
    pfad.closeSubpath();
    return pfad;
}

/// Zwei Quadrate nebeneinander: links Kennung 1 (blau), rechts Kennung 2 (rot).
/// Der Rahmen ist 300 breit und 100 hoch — eingepasst liegt das linke um
/// x = −2/3, das rechte um x = +2/3, beide mit der Kante 2/3.
Zeichnung zweiQuadrate()
{
    Zeichnung z;
    z.titel = QStringLiteral("Two squares");
    REQUIRE(z.neueKontur(quadrat(0.0), QStringLiteral("Left"), QStringLiteral("#0000ff")) == 1);
    REQUIRE(z.neueKontur(quadrat(200.0), QStringLiteral("Right"), QStringLiteral("#ff0000")) == 2);
    return z;
}

/// Der Ordner der eigenen Zeichnungen fuer einen Fall; danach wieder der echte.
struct EigenerOrdner
{
    QTemporaryDir tmp;
    EigenerOrdner()
    {
        REQUIRE(tmp.isValid());
        setzeEigenenKonturOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("contours")));
    }
    ~EigenerOrdner() { setzeEigenenKonturOrdnerFuerTests(QString()); }
};

/// Ein GL-Kontext ohne Fenster; `ok` = false, wo es keinen gibt (dann meldet der Fall es).
struct GlOhneFenster
{
    QOpenGLContext ctx;
    QOffscreenSurface surface;
    bool ok = false;
    GlOhneFenster()
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

constexpr int kBild = 128;

double hell(const QImage& bild, int x, int y)
{
    const QColor c = bild.pixelColor(x, y);
    return std::max({c.redF(), c.greenF(), c.blueF()});
}

/// Wie viele Pixel des Ausschnitts leuchten.
int leuchtend(const QImage& bild, int x0, int y0, int x1, int y1)
{
    int zahl = 0;
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x)
            if (hell(bild, x, y) > 0.2) ++zahl;
    return zahl;
}
int leuchtend(const QImage& bild) { return leuchtend(bild, 0, 0, bild.width(), bild.height()); }

/// Leuchtet es in der Naehe dieser Stelle (ein Pixel Spiel fuer die Linienbreite)?
bool leuchtetBei(const QImage& bild, int x, int y) { return leuchtend(bild, x - 2, y - 2, x + 3, y + 3) > 0; }

/// Die Kette Clear -> Shape.
ChainNode kette(const ShapeParams& shape)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    clear.params = ClearParams{};
    ChainNode knoten;
    knoten.params = shape;
    root.children.push_back(std::move(clear));
    root.children.push_back(std::move(knoten));
    return root;
}

/// Die kuerzeste von drei Messungen, mit der laengsten Taktprobe daneben.
template <typename Arbeit>
void messe(const std::string& name, Arbeit&& arbeit)
{
    double bester = 1.0e30;
    double takt = 0.0;
    for (int lauf = 0; lauf < 3; ++lauf)
    {
        takt = std::max(takt, taktprobeMs());
        const auto start = std::chrono::steady_clock::now();
        arbeit();
        bester = std::min(
            bester, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
    }
    MESSAGE(name << ": " << bester << " ms je Bild (Taktprobe " << takt << " ms)");
}

}  // namespace

TEST_CASE("Shape Knoten Messung: Per point und die Form je Bild")
{
    // --- Per point: das Skript von Wave Outline und Rainbow Outline zusammen, je Punkt
    //     vier Werte hinein und sechs heraus — wie es der Knoten taete
    ScriptSlotHost host("shape", {}, ScriptSlotHost::Dialect::Avs);
    host.setSource(Slot::Point, "r=1+v*0.3; x=x*r; y=y*r; red=0.5+0.5*sin(i*6.283); "
                                "green=0.5+0.5*sin(i*6.283+2.094); blue=0.5+0.5*sin(i*6.283+4.189);");
    REQUIRE_MESSAGE(host.compileAll(), host.lastError());
    auto& engine = host.engine();
    const auto perPoint = [&](int punkte) {
        double summe = 0.0;
        for (int p = 0; p < punkte; ++p)
        {
            const double i = static_cast<double>(p) / punkte;
            engine.setNumber("i", i);
            engine.setNumber("x", std::cos(i * 6.283));
            engine.setNumber("y", std::sin(i * 6.283));
            engine.setNumber("contour", 1.0);
            engine.setNumber("v", 0.25);
            engine.setNumber("red", 1.0);
            engine.setNumber("green", 1.0);
            engine.setNumber("blue", 1.0);
            engine.setNumber("skip", 0.0);
            host.run(Slot::Point);
            summe += engine.number("x") + engine.number("y") + engine.number("red") + engine.number("green") +
                     engine.number("blue") + engine.number("skip");
        }
        return summe;
    };
    double halt = 0.0;
    for (const int punkte : {512, 4096, 4 * 4096})
        messe("Per point, " + std::to_string(punkte) + " Punkte", [&] { halt += perPoint(punkte); });
    CHECK(std::isfinite(halt));

    // --- die Form: umriss rechnet je Punkt jede Harmonische
    const Reihe r = reihe(normiert(abtasten(quadrat())));
    REQUIRE(r.harmonische() == kHarmonischeMax);
    std::size_t punkteGesamt = 0;
    for (const int punkte : {512, 4096})
        for (const double harmonische : {16.0, 64.0, 512.0})
            messe("umriss, " + std::to_string(punkte) + " Punkte, " + std::to_string(static_cast<int>(harmonische)) +
                      " Harmonics",
                  [&] { punkteGesamt += umriss(r, harmonische, punkte).size(); });
    CHECK(punkteGesamt > 0);
}

TEST_CASE("Shape Zeichenform: gemeinsam eingepasst — welche Kontur gemeint ist — Umriss und Kreise")
{
    Zeichnung zeichnung = zweiQuadrate();
    Zeichenform form = zeichenform(zeichnung);
    REQUIRE(form.konturen.size() == 2);

    // --- alle Konturen liegen gemeinsam in −1 … 1, jede an ihrer Stelle
    const auto rahmen = [](const ZeichenKontur& k) {
        double l = 9.0, r = -9.0, u = 9.0, o = -9.0;
        for (const Punkt& p : k.punkte)
        {
            l = std::min(l, p.real());
            r = std::max(r, p.real());
            u = std::min(u, p.imag());
            o = std::max(o, p.imag());
        }
        return std::array<double, 4>{l, r, u, o};
    };
    const auto links = rahmen(form.konturen[0]);
    const auto rechts = rahmen(form.konturen[1]);
    CHECK(links[0] == doctest::Approx(-1.0));
    CHECK(links[1] == doctest::Approx(-1.0 / 3.0));
    CHECK(rechts[0] == doctest::Approx(1.0 / 3.0));
    CHECK(rechts[1] == doctest::Approx(1.0));
    CHECK(links[2] == doctest::Approx(-1.0 / 3.0));
    CHECK(links[3] == doctest::Approx(1.0 / 3.0));
    // die Farbe kommt aus der Datei
    CHECK(form.konturen[0].blau == doctest::Approx(1.0f));
    CHECK(form.konturen[0].rot == doctest::Approx(0.0f));
    CHECK(form.konturen[1].rot == doctest::Approx(1.0f));

    // --- welche Kontur gemeint ist: 0 = jede mit Haken, sonst die eine Kennung
    CHECK(gemeinteKonturen(form, 0) == std::vector<std::size_t>{0, 1});
    CHECK(gemeinteKonturen(form, 2) == std::vector<std::size_t>{1});
    CHECK(gemeinteKonturen(form, 7).empty());
    zeichnung.konturen[1].gewaehlt = false;
    Zeichenform ohneHaken = zeichenform(zeichnung);
    CHECK(gemeinteKonturen(ohneHaken, 0) == std::vector<std::size_t>{0});
    CHECK(gemeinteKonturen(ohneHaken, 2) == std::vector<std::size_t>{1});  // beim Namen genannt: auch ohne Haken
    // ohne Haken steht die andere trotzdem, wo sie stand
    CHECK(rahmen(ohneHaken.konturen[0])[0] == doctest::Approx(-1.0));
    // eine leere Zeichnung ergibt eine leere Form
    CHECK(zeichenform(Zeichnung{}).leer());

    // --- der Umriss: mit vielen Teiltoenen das Quadrat an seiner Stelle
    ZeichenKontur& k = form.konturen[1];
    const std::vector<Punkt>& fein = umrissVon(k, 512.0, 256);
    REQUIRE(fein.size() == 256);
    double l = 9.0, r = -9.0;
    for (const Punkt& p : fein)
    {
        l = std::min(l, p.real());
        r = std::max(r, p.real());
    }
    CHECK(l == doctest::Approx(1.0 / 3.0).epsilon(0.02));
    CHECK(r == doctest::Approx(1.0).epsilon(0.02));
    // gemerkt: dieselben Werte, dieselbe Liste; ein anderer Wert rechnet neu
    const Punkt* davor = fein.data();
    CHECK(umrissVon(k, 512.0, 256).data() == davor);
    const Punkt erster = fein.front();
    CHECK(umrissVon(k, 1.0, 256).front() != erster);
    CHECK(umrissVon(k, 1.0, 64).size() == 64);
    // mit einem Teilton ist das Quadrat ein Kreis um seine Mitte
    const std::vector<Punkt>& kreis = umrissVon(k, 1.0, 64);
    const Punkt mitte{2.0 / 3.0, 0.0};
    const double radius = std::abs(kreis.front() - mitte);
    CHECK(radius > 0.3);
    for (const Punkt& p : kreis) CHECK(std::abs(p - mitte) == doctest::Approx(radius).epsilon(0.001));

    // --- die Kreise: jeder sitzt auf dem vorigen, die Spitze ist der Punkt des Umrisses
    for (const double umlauf : {0.0, 0.3, 0.77})
    {
        const Zeigerbild bild = zeigerbild(k, umlauf, 8.0);
        REQUIRE(bild.kreise.size() == 16);  // +1, −1, … +8, −8
        REQUIRE(bild.arme.size() == 17);
        CHECK(std::abs(bild.arme.front() - mitte) < 1.0e-9);
        for (std::size_t n = 0; n < bild.kreise.size(); ++n)
        {
            CHECK(std::abs(bild.kreise[n].mitte - bild.arme[n]) < 1.0e-12);
            CHECK(bild.kreise[n].radius == doctest::Approx(std::abs(bild.arme[n + 1] - bild.arme[n])));
        }
        CHECK(std::abs(bild.arme.back() - wert(k.reihe, umlauf, 8.0, true)) < 1.0e-9);
    }
    // Harmonics laeuft weich: bei 2,5 spielt das dritte Paar halb mit
    const Zeigerbild halb = zeigerbild(k, 0.1, 2.5);
    const Zeigerbild ganz = zeigerbild(k, 0.1, 3.0);
    REQUIRE(halb.kreise.size() == 6);
    CHECK(halb.kreise[4].radius == doctest::Approx(ganz.kreise[4].radius * 0.5));
}

TEST_CASE("Shape Knoten: die Datei — Vorgaben, Hin und Zurueck, Grenzen, die Feldtabelle")
{
    CHECK(effectTypeKey(EffectParams{ShapeParams{}}) == "shape");
    CHECK(std::string(effectTypeName(EffectParams{ShapeParams{}})) == "Shape");

    ChainNode root;
    root.params = ListParams{};
    ChainNode blatt;
    ShapeParams sp;
    sp.shape = "user:mein_herz";
    sp.contour = 3;
    sp.harmonics = 12.5f;
    sp.points = 1024;
    sp.lineWidth = 3.5f;
    sp.xShift = -20.0f;
    sp.yShift = 15.0f;
    sp.size = 120.0f;
    sp.rotation = 90.0f;
    sp.useOwnColor = true;
    sp.color = 0x336699;
    sp.lineBlend = 3;
    sp.showCircles = true;
    sp.speed = 1.5f;
    sp.initCode = "t=0";
    sp.frameCode = "t=t+1";
    sp.beatCode = "size=100";
    sp.pointCode = "x=x*2";
    blatt.params = sp;
    root.children.push_back(blatt);
    const ChainNode zurueck = chainFromJson(chainToJson(root), nullptr);
    REQUIRE(zurueck.children.size() == 1);
    const auto& p = std::get<ShapeParams>(zurueck.children[0].params);
    CHECK(p.shape == "user:mein_herz");
    CHECK(p.contour == 3);
    CHECK(p.harmonics == doctest::Approx(12.5f));
    CHECK(p.points == 1024);
    CHECK(p.lineWidth == doctest::Approx(3.5f));
    CHECK(p.xShift == doctest::Approx(-20.0f));
    CHECK(p.yShift == doctest::Approx(15.0f));
    CHECK(p.size == doctest::Approx(120.0f));
    CHECK(p.rotation == doctest::Approx(90.0f));
    CHECK(p.useOwnColor);
    CHECK(p.color == 0x336699u);
    CHECK(p.lineBlend == 3);
    CHECK(p.showCircles);
    CHECK(p.speed == doctest::Approx(1.5f));
    CHECK(p.initCode == "t=0");
    CHECK(p.frameCode == "t=t+1");
    CHECK(p.beatCode == "size=100");
    CHECK(p.pointCode == "x=x*2");

    // --- was ausserhalb liegt, kommt in seine Grenzen; was fehlt, ist die Vorgabe
    {
        QJsonObject datei = chainToJson(root);
        QJsonObject wurzel = datei.value(QStringLiteral("root")).toObject();
        QJsonArray kinder = wurzel.value(QStringLiteral("children")).toArray();
        REQUIRE(kinder.size() == 1);
        QJsonObject knoten = kinder[0].toObject();
        knoten[QStringLiteral("harmonics")] = 9999;
        knoten[QStringLiteral("points")] = 3;
        knoten[QStringLiteral("size")] = -5;
        knoten[QStringLiteral("lineBlend")] = 42;
        knoten[QStringLiteral("contour")] = -4;
        knoten.remove(QStringLiteral("speed"));
        knoten.remove(QStringLiteral("shape"));
        kinder[0] = knoten;
        wurzel[QStringLiteral("children")] = kinder;
        datei[QStringLiteral("root")] = wurzel;
        const ChainNode gelesen = chainFromJson(datei, nullptr);
        REQUIRE(gelesen.children.size() == 1);
        const auto& q = std::get<ShapeParams>(gelesen.children[0].params);
        CHECK(q.harmonics == doctest::Approx(512.0f));
        CHECK(q.points == 16);
        CHECK(q.size == doctest::Approx(1.0f));
        CHECK(q.lineBlend == 9);
        CHECK(q.contour == 0);
        CHECK(q.speed == doctest::Approx(ShapeParams{}.speed));
        CHECK(q.shape == ShapeParams{}.shape);
    }

    // --- die Feldtabelle: eigener Skript-Host, jeder Regler ist Skriptvariable
    const feldtabelle::Typ* typ = feldtabelle::typ(EffectParams{ShapeParams{}});
    REQUIRE(typ != nullptr);
    CHECK(typ->traeger == feldtabelle::Traeger::EigenerHost);
    for (const char* name : {"contour", "harmonics", "points", "linewidth", "xshift", "yshift", "size", "rotation",
                             "useowncolor", "color", "lineblend", "showcircles", "speed"})
    {
        CAPTURE(name);
        const bool da = std::any_of(typ->felder.begin(), typ->felder.end(),
                                    [name](const feldtabelle::Feld& f) { return f.name == name; });
        CHECK(da);
    }
    // aus dem Feld Frame steuerbar: kein Feld baut etwas Teures neu
    for (const feldtabelle::Feld& f : typ->felder) CHECK(f.klasse == feldtabelle::Klasse::Live);
}

TEST_CASE("Shape Knoten im Bild: Lage, Groesse, Drehung, Farbe, Contour, die Skriptfelder, Show circles")
{
    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    QGuiApplication app(argc, argv);
    GlOhneFenster gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
        return;
    }

    // Zwei eigene Zeichnungen im Wegwerf-Ordner: ein rotes Quadrat, zwei Quadrate nebeneinander
    EigenerOrdner ordner;
    {
        Zeichnung eins;
        REQUIRE(eins.neueKontur(quadrat(), QStringLiteral("Square"), QStringLiteral("#ff0000")) == 1);
        REQUIRE(speichereEigeneZeichnung(eins, QStringLiteral("quadrat")));
        Zeichnung zwei = zweiQuadrate();
        REQUIRE(speichereEigeneZeichnung(zwei, QStringLiteral("zwei")));
    }

    MultiEffectVisualizer vis;
    vis.setChain(kette(ShapeParams{}));
    vis.initialize();
    vis.resize(QSize(kBild, kBild));
    gl.ctx.functions()->glViewport(0, 0, kBild, kBild);
    const float dt = 1.0f / 60.0f;
    const auto bildVon = [&](const ShapeParams& p, bool skripteAn = true, int bilder = 1) {
        ChainNode k = kette(p);
        k.children[1].scriptsActive = skripteAn;
        vis.setChain(std::move(k));
        for (int n = 0; n < bilder; ++n) vis.render(dt);
        return vis.debugGrabRootSurface();
    };

    // Das Quadrat fuellt eingepasst −1 … 1; mit Size 50 liegen seine Kanten bei 32 und 96.
    ShapeParams grund;
    grund.shape = "user:quadrat";
    grund.size = 50.0f;
    grund.harmonics = 512.0f;
    const QImage bild = bildVon(grund);
    REQUIRE(!bild.isNull());
    REQUIRE(bild.width() == kBild);
    CHECK(leuchtetBei(bild, 96, 64));
    CHECK(leuchtetBei(bild, 32, 64));
    CHECK(leuchtetBei(bild, 64, 32));
    CHECK(leuchtetBei(bild, 64, 96));
    CHECK(leuchtetBei(bild, 96, 96));  // die Ecke
    CHECK_FALSE(leuchtetBei(bild, 64, 64));  // ein Umriss, keine Flaeche
    CHECK_FALSE(leuchtetBei(bild, 10, 10));
    {
        // die Farbe der Kontur aus der Datei: rot
        double rot = 0.0, blau = 0.0;
        for (int x = 92; x <= 100; ++x)
        {
            rot = std::max(rot, static_cast<double>(bild.pixelColor(x, 64).redF()));
            blau = std::max(blau, static_cast<double>(bild.pixelColor(x, 64).blueF()));
        }
        CHECK(rot > 0.8);
        CHECK(blau < 0.1);
    }

    // --- Harmonics: mit einem Teilton ein Kreis — die Ecke bleibt dunkel
    {
        ShapeParams p = grund;
        p.harmonics = 1.0f;
        const QImage kreis = bildVon(p);
        CHECK(leuchtend(kreis) > 50);
        CHECK_FALSE(leuchtetBei(kreis, 96, 96));
    }
    // --- Rotation 45°: eine Spitze zeigt nach rechts, die Kantenmitte ist leer
    {
        ShapeParams p = grund;
        p.rotation = 45.0f;
        const QImage gedreht = bildVon(p);
        CHECK(leuchtetBei(gedreht, 109, 64));
        CHECK_FALSE(leuchtetBei(gedreht, 96, 64));
    }
    // --- X shift 10 %: ein Zehntel der Breite nach rechts
    {
        ShapeParams p = grund;
        p.xShift = 10.0f;
        const QImage verschoben = bildVon(p);
        CHECK(leuchtetBei(verschoben, 109, 64));
        CHECK_FALSE(leuchtetBei(verschoben, 96, 64));
    }
    // --- Y shift negativ: nach oben
    {
        ShapeParams p = grund;
        p.yShift = -20.0f;
        const QImage oben = bildVon(p);
        CHECK(leuchtend(oben, 0, 0, kBild, 60) > leuchtend(oben, 0, 68, kBild, kBild));
    }
    // --- Use own color
    {
        ShapeParams p = grund;
        p.useOwnColor = true;
        p.color = 0x00FF00;
        const QImage gruen = bildVon(p);
        double g = 0.0, r = 0.0;
        for (int x = 92; x <= 100; ++x)
        {
            g = std::max(g, static_cast<double>(gruen.pixelColor(x, 64).greenF()));
            r = std::max(r, static_cast<double>(gruen.pixelColor(x, 64).redF()));
        }
        CHECK(g > 0.8);
        CHECK(r < 0.1);
    }
    // --- eine Zeichnung, die es nicht gibt: nichts, und nichts stuerzt
    {
        ShapeParams p = grund;
        p.shape = "user:gibt_es_nicht";
        CHECK(leuchtend(bildVon(p)) == 0);
    }

    // --- Contour: 0 zeichnet beide, eine Kennung nur die eine, eine fremde nichts
    ShapeParams zwei = grund;
    zwei.shape = "user:zwei";
    {
        const QImage beide = bildVon(zwei);
        CHECK(leuchtend(beide, 0, 0, 60, kBild) > 20);
        CHECK(leuchtend(beide, 68, 0, kBild, kBild) > 20);
        ShapeParams p = zwei;
        p.contour = 2;
        const QImage rechts = bildVon(p);
        CHECK(leuchtend(rechts, 0, 0, 60, kBild) == 0);
        CHECK(leuchtend(rechts, 68, 0, kBild, kBild) > 20);
        p.contour = 7;
        CHECK(leuchtend(bildVon(p)) == 0);
    }

    // --- Frame: ein Skript setzt einen Regler
    {
        ShapeParams p = grund;
        p.frameCode = "size=25";
        const QImage klein = bildVon(p);
        CHECK(leuchtetBei(klein, 80, 64));
        CHECK_FALSE(leuchtetBei(klein, 96, 64));
        // Script active aus: der Regler gilt
        const QImage ohne = bildVon(p, /*skripteAn=*/false);
        CHECK(leuchtetBei(ohne, 96, 64));
    }
    // --- Init laeuft einmal, Frame schreibt fort: Harmonics waechst weich aus dem Kreis
    {
        ShapeParams p = grund;
        p.initCode = "harmonics=1";
        p.frameCode = "harmonics=harmonics+0.5";
        CHECK_FALSE(leuchtetBei(bildVon(p, true, 1), 96, 96));
        CHECK(leuchtetBei(bildVon(p, true, 400), 96, 96));
    }
    // --- Beat
    {
        ShapeParams p = grund;
        p.beatCode = "rotation=45";
        vis.setBeatPeriodOverride(1);
        const QImage gedreht = bildVon(p);
        vis.setBeatPeriodOverride(0);
        CHECK(leuchtetBei(gedreht, 109, 64));
    }
    // --- Point: x und y je Punkt, y nach unten wie bei SuperScope
    {
        ShapeParams p = grund;
        p.pointCode = "x=x*0.5";
        const QImage schmal = bildVon(p);
        CHECK(leuchtetBei(schmal, 80, 64));
        CHECK(leuchtetBei(schmal, 64, 32));
        CHECK_FALSE(leuchtetBei(schmal, 96, 64));
        p.pointCode = "y=y-0.4";
        const QImage oben = bildVon(p);
        CHECK(leuchtend(oben, 0, 0, kBild, 60) > leuchtend(oben, 0, 68, kBild, kBild));
        p.pointCode = "skip=1";
        CHECK(leuchtend(bildVon(p)) == 0);
        p.pointCode = "skip = above(i, 0.5);";  // Beispiel im Handbuch §11
        const int halb = leuchtend(bildVon(p));
        CHECK(halb > 50);
        CHECK(halb < leuchtend(bild) * 3 / 4);
    }
    // --- die Beispiele des Handbuchs (§11 Shape), woertlich
    {
        // Beat laesst die Form zusammenfallen, Frame laesst sie wieder scharf werden
        ShapeParams p = grund;
        p.beatCode = "harmonics = 3;";
        p.frameCode = "harmonics = harmonics + (64 - harmonics) * 0.06;";
        vis.setBeatPeriodOverride(1);
        CHECK_FALSE(leuchtetBei(bildVon(p, true, 5), 96, 96));  // jeder Beat: drei Teiltoene, die Ecke fehlt
        vis.setBeatPeriodOverride(0);
        CHECK(leuchtetBei(bildVon(p, true, 200), 96, 96));  // ohne Beat waechst es auf 64
        // die Wellenform schiebt jeden Punkt nach aussen: mit v = 1 um 35 %
        ShapeParams q = grund;
        q.pointCode = "r = 1 + v * 0.35; x = x * r; y = y * r;";
        const std::vector<float> spektrum(2 * 512, 0.0f);
        const std::vector<float> welle(2 * 512, 1.0f);
        vis.setChain(kette(q));
        vis.updateAudioStereo(spektrum.data(), 512, welle.data(), 512, 2);
        vis.render(dt);
        const QImage weit = vis.debugGrabRootSurface();
        CHECK(leuchtetBei(weit, 107, 64));  // 64 + 32 * 1,35
        CHECK_FALSE(leuchtetBei(weit, 96, 64));
        const std::vector<float> still(2 * 512, 0.0f);
        vis.updateAudioStereo(spektrum.data(), 512, still.data(), 512, 2);
    }
    // --- Point: `contour` nennt die Kontur, die gerade gezeichnet wird — und die Wahl bleibt
    {
        ShapeParams p = zwei;
        p.pointCode = "red=if(equal(contour,2),0,1); green=if(equal(contour,2),1,0); blue=0;";
        const QImage farbig = bildVon(p, true, 3);
        double linksRot = 0.0, linksGruen = 0.0, rechtsRot = 0.0, rechtsGruen = 0.0;
        for (int y = 0; y < kBild; ++y)
            for (int x = 0; x < kBild; ++x)
            {
                const QColor c = farbig.pixelColor(x, y);
                double& rot = x < 64 ? linksRot : rechtsRot;
                double& gruen = x < 64 ? linksGruen : rechtsGruen;
                rot = std::max(rot, static_cast<double>(c.redF()));
                gruen = std::max(gruen, static_cast<double>(c.greenF()));
            }
        CHECK(linksRot > 0.8);
        CHECK(linksGruen < 0.1);
        CHECK(rechtsGruen > 0.8);
        CHECK(rechtsRot < 0.1);
    }
    // --- Show circles: Kreise und Arme kommen dazu, der erste Arm beginnt in der Mitte
    {
        ShapeParams p = grund;
        p.harmonics = 8.0f;
        const int ohne = leuchtend(bildVon(p));
        p.showCircles = true;
        p.speed = 0.0f;
        const QImage mit = bildVon(p);
        CHECK(leuchtend(mit) > ohne + 50);
        CHECK(leuchtetBei(mit, 64, 64));
    }

    vis.cleanup();
    gl.ctx.doneCurrent();
}

TEST_CASE("Shape Knoten: die neun mitgelieferten Presets laden, uebersetzen und zeichnen")
{
    static int argc = 1;
    static char arg0[] = "UnitTests";
    static char* argv[] = {arg0, nullptr};
    QGuiApplication app(argc, argv);

    const auto eintraege = nodepresets::list(QStringLiteral("shape"));
    // Entscheid V3 a (Patrik S121): Heart, Epicycles, Harmonics Sweep, Beat Harmonics, Beat Pulse,
    // Bass Spin, Wave Outline, Rainbow Outline, Contour Switch — wer eines dazulegt, stellt die Zahl nach
    REQUIRE(eintraege.size() == 9);

    std::vector<std::pair<QString, ShapeParams>> presets;
    for (const nodepresets::Entry& e : eintraege)
    {
        CAPTURE(e.name.toStdString());
        EffectParams params{ShapeParams{}};
        QStringList bericht;
        REQUIRE(nodepresets::load(e.path, QStringLiteral("shape"), params, &bericht));
        const ShapeParams& p = std::get<ShapeParams>(params);
        // die Form gibt es, und die genannte Kontur auch
        Zeichnung zeichnung;
        REQUIRE(holeZeichnung(QString::fromStdString(p.shape), zeichnung));
        if (p.contour != 0) CHECK(zeichnung.finde(p.contour) != nullptr);
        // jedes Skriptfeld uebersetzt
        ScriptSlotHost host("shape", {}, ScriptSlotHost::Dialect::Avs);
        host.setSource(Slot::Init, p.initCode);
        host.setSource(Slot::Frame, p.frameCode);
        host.setSource(Slot::Beat, p.beatCode);
        host.setSource(Slot::Point, p.pointCode);
        CHECK_MESSAGE(host.compileAll(), host.lastError());
        presets.emplace_back(e.name, p);
    }

    GlOhneFenster gl;
    if (!gl.ok)
    {
        MESSAGE("Kein GL-Kontext moeglich (Headless?) — das Zeichnen uebersprungen");
        return;
    }
    constexpr int kKachel = 256;
    MultiEffectVisualizer vis;
    vis.setChain(kette(ShapeParams{}));
    vis.initialize();
    vis.resize(QSize(kKachel, kKachel));
    gl.ctx.functions()->glViewport(0, 0, kKachel, kKachel);
    vis.setBeatPeriodOverride(12);  // ein Beat alle zwoelf Bilder
    QImage blatt(3 * kKachel, 3 * kKachel, QImage::Format_RGB32);
    blatt.fill(Qt::black);
    QPainter maler(&blatt);
    maler.setPen(Qt::white);
    int platz = 0;
    for (const auto& [name, p] : presets)
    {
        CAPTURE(name.toStdString());
        vis.setChain(kette(p));
        for (int n = 0; n < 40; ++n) vis.render(1.0f / 60.0f);
        const QImage bild = vis.debugGrabRootSurface();
        REQUIRE(!bild.isNull());
        CHECK(leuchtend(bild) > 100);
        const QPoint ecke((platz % 3) * kKachel, (platz / 3) * kKachel);
        maler.drawImage(ecke, bild);
        maler.drawText(ecke + QPoint(6, 14), name);
        ++platz;
    }
    maler.end();
    const QString ziel = qEnvironmentVariable("LUMIVIZ_SHAPE_KNOTEN_BILD");
    if (!ziel.isEmpty())
    {
        CHECK(blatt.save(ziel));
        MESSAGE("Blatt der Presets: " << ziel.toStdString());
    }
    vis.cleanup();
    gl.ctx.doneCurrent();
}
