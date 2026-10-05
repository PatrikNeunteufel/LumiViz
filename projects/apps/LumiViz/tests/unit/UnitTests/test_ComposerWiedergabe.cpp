/**
 ****************************************************************************************
 * @file   test_ComposerWiedergabe.cpp
 * @brief  Composer, Stufe C1b: Datenmodell, Datei `.lvcomp`, Uhr und Wiedergabe
 *         ohne Panel (docs/ui/Composer_Konzept.md §4–§6)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Der Kern der Stufe: an einer Stelle der Zeitachse zeigt das Bild die
 * erwarteten Clips mit den erwarteten Pegeln — beim Durchlauf wie nach einem
 * Sprung. Dazu Vorwaermen, Entladen und die Karaoke-Ebene einmal darueber.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "composer/Komposition.hpp"
#include "composer/Uhr.hpp"
#include "composer/Wiedergabe.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QTemporaryDir>

#include <cmath>

using namespace lumi::multieffect;
using namespace lumi::composer;

namespace
{

struct Gl
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QGuiApplication app{argc, argv};
    QOpenGLContext ctx;
    QOffscreenSurface surface;
    bool gut = false;

    Gl()
    {
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        if (!ctx.create()) return;
        surface.setFormat(ctx.format());
        surface.create();
        gut = surface.isValid() && ctx.makeCurrent(&surface);
    }
};

/// Preset-Datei, die das Bild einfarbig fuellt.
QString farbPreset(const QTemporaryDir& dir, const QString& name, uint32_t farbe)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    ClearParams cp;
    cp.color = farbe;
    clear.params = cp;
    root.children.push_back(std::move(clear));
    const QString pfad = dir.filePath(name);
    REQUIRE(saveChainToFile(root, pfad));
    return pfad;
}

/// Preset-Datei mit eigenem Zustand: wandernde Balken und Feedback.
QString balkenPreset(const QTemporaryDir& dir, const QString& name, uint32_t farbe)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    ClearParams cp;
    cp.onlyFirst = true;
    clear.params = cp;
    ChainNode balken;
    balken.params = DebugBarsParams{farbe, -2.5f};
    ChainNode blit;
    blit.params = BlitterFeedbackParams{};
    root.children = {clear, balken, blit};
    const QString pfad = dir.filePath(name);
    REQUIRE(saveChainToFile(root, pfad));
    return pfad;
}

VisualClip clip(const QString& datei, std::int64_t start, std::int64_t ende,
                Mischart art = Mischart::Normal)
{
    VisualClip c;
    c.datei = datei;
    c.startMs = start;
    c.endeMs = ende;
    c.mischart = art;
    return c;
}

Kanal visualKanal(const QString& name, std::vector<VisualClip> clips)
{
    Kanal k;
    k.art = KanalArt::Visual;
    k.name = name;
    k.clips = std::move(clips);
    return k;
}

int verschieden(const QImage& a, const QImage& b)
{
    if (a.size() != b.size()) return -1;
    int n = 0;
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x)
            if (a.pixel(x, y) != b.pixel(x, y)) ++n;
    return n;
}

}  // namespace

TEST_SUITE("ComposerWiedergabe")
{
    // -------------------------------------------------------------------------
    // Modell (ohne GL)
    // -------------------------------------------------------------------------
    TEST_CASE("C1b Modell: Pegelkurve — ohne Punkte voll, zwischen Punkten nach der Kurvenform, Blenden")
    {
        VisualClip c = clip(QStringLiteral("x.lvfx"), 1000, 5000);
        CHECK(c.dauerMs() == 4000);
        CHECK(c.pegelBei(0) == doctest::Approx(1.0));     // keine Kurve = voller Pegel
        CHECK(c.pegelBei(2500) == doctest::Approx(1.0));

        // eine Punktliste, wie alte Dateien sie tragen
        ausAltenPunkten(c, {{1000, 0.2, Kurvenform::Linear}, {3000, 0.8, Kurvenform::Linear}});
        CHECK(c.pegelBei(0) == doctest::Approx(0.2));      // vor dem ersten Punkt: dessen Pegel
        CHECK(c.pegelBei(1000) == doctest::Approx(0.2));
        CHECK(c.pegelBei(2000) == doctest::Approx(0.5));   // linear in der Mitte
        CHECK(c.pegelBei(3000) == doctest::Approx(0.8));
        CHECK(c.pegelBei(4000) == doctest::Approx(0.8));   // nach dem letzten: der letzte

        // die Formen: gleiche Endpunkte, anderer Weg
        CHECK(kurvenWert(Kurvenform::Linear, 0.5) == doctest::Approx(0.5));
        CHECK(kurvenWert(Kurvenform::SKurve, 0.5) == doctest::Approx(0.5));
        CHECK(kurvenWert(Kurvenform::SKurve, 0.25) < 0.25);
        CHECK(kurvenWert(Kurvenform::EaseIn, 0.5) == doctest::Approx(0.25));
        CHECK(kurvenWert(Kurvenform::EaseOut, 0.5) == doctest::Approx(0.75));
        CHECK(kurvenWert(Kurvenform::Exponentiell, 0.5) == doctest::Approx(0.125));
        CHECK(kurvenWert(Kurvenform::Halten, 0.99) == doctest::Approx(0.0));
        CHECK(kurvenWert(Kurvenform::Halten, 1.0) == doctest::Approx(1.0));
        for (int f = 0; f < kKurvenformen; ++f)
        {
            CHECK(kurvenWert(static_cast<Kurvenform>(f), 0.0) == doctest::Approx(0.0));
            CHECK(kurvenWert(static_cast<Kurvenform>(f), 1.0) == doctest::Approx(1.0));
        }
        ausAltenPunkten(c, {{0, 0.0, Kurvenform::EaseIn}, {2000, 1.0, Kurvenform::Linear}});
        CHECK(c.ein.dauerMs == 2000);  // das Muster einer Einblendung
        CHECK(c.pegelBei(1000) == doctest::Approx(0.25));

        // Ein- und Ausblenden
        c.setzeBlenden(1000, 2000);
        CHECK(c.pegelBei(0) == doctest::Approx(0.0));
        CHECK(c.pegelBei(500) == doctest::Approx(0.5));
        CHECK(c.pegelBei(1000) == doctest::Approx(1.0));
        CHECK(c.pegelBei(1500) == doctest::Approx(1.0));
        CHECK(c.pegelBei(3000) == doctest::Approx(0.5));
        CHECK(c.pegelBei(4000) == doctest::Approx(0.0));
        c.setzeBlenden(0, 1000);  // nur ausblenden
        CHECK(c.pegelBei(0) == doctest::Approx(1.0));
        CHECK(c.pegelBei(3500) == doctest::Approx(0.5));
        c.setzeBlenden(3000, 3000);  // laenger als der Clip: treffen sich in der Mitte
        CHECK(c.pegelBei(2000) == doctest::Approx(1.0));
        CHECK(c.pegelBei(1000) == doctest::Approx(0.5));
        CHECK(c.pegelBei(3000) == doctest::Approx(0.5));
        c.setzeBlenden(0, 0);
        CHECK(c.vollerPegel());
    }

    TEST_CASE("C1b Modell: richte — sortiert, loest Ueberlappung im Kanal, meldet es; Laenge")
    {
        Komposition k;
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"),
                                        {clip(QStringLiteral("b.lvfx"), 3000, 6000),
                                         clip(QStringLiteral("a.lvfx"), 0, 4000),
                                         clip(QStringLiteral("c.lvfx"), 9000, 9000)}));
        k.kanaele[0].clips[0].grundpegel = 2.0;
        k.kanaele[0].clips[0].ein.dauerMs = -5;
        k.kanaele[0].clips[0].aus.dauerMs = 9999;  // laenger als der Clip: bleibt, wie eingestellt
        k.kanaele[0].clips[0].stuecke = {ClipStueck{3.0, -100, false, Pegelform{}}};
        QStringList bericht;
        k.richte(&bericht);
        const auto& v = k.kanaele[0].clips;
        REQUIRE(v.size() == 3);
        CHECK(v[0].datei == QStringLiteral("a.lvfx"));  // nach Start sortiert
        CHECK(v[1].datei == QStringLiteral("b.lvfx"));
        CHECK(v[1].startMs == 4000);                    // beginnt, wo a endet
        CHECK(v[1].endeMs == 6000);
        CHECK(v[2].endeMs == 10000);                    // ohne Laenge: eine Sekunde
        CHECK(v[1].grundpegel == doctest::Approx(1.0));  // geklemmt
        CHECK(v[1].ein.dauerMs == 0);
        CHECK(v[1].aus.dauerMs == 9999);
        REQUIRE(v[1].stuecke.size() == 1);
        CHECK(v[1].stuecke[0].lage == doctest::Approx(1.0));
        CHECK(v[1].stuecke[0].weiteMs == 0);
        CHECK(v[1].stuecke[0].form.art == FormArt::Stueck);
        CHECK(v[1].stuecke[0].form.punkte.size() == 2);  // eine leere Form bekommt ihre Enden
        CHECK(bericht.size() == 2);
        CHECK(k.laenge() == 10000);                     // Ende des letzten Clips
        k.laengeMs = 60000;
        CHECK(k.laenge() == 60000);
    }

    TEST_CASE("C1b Datei: .lvcomp — Roundtrip aller Felder, relative Pfade, fehlende Datei gemeldet")
    {
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        REQUIRE(QDir(dir.path()).mkdir(QStringLiteral("presets")));
        const QString innen = dir.filePath(QStringLiteral("presets/Innen.lvfx"));
        {
            QFile f(innen);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write("{}");
        }
        const QString aussen = QDir::cleanPath(QDir::tempPath() + QStringLiteral("/gibt-es-nicht/Aussen.avs"));

        Komposition k;
        k.laengeMs = 120000;
        k.rasterSekunden = 0.5;
        VisualClip a = clip(innen, 1000, 9000, Mischart::Additive);
        a.grundpegel = 0.75;
        a.ein = {2000, einfacheBlende(Kurvenform::SKurve)};
        a.aus = {12000, einfacheBlende(Kurvenform::Halten)};  // laenger als der Clip: bleibt so in der Datei
        a.stuecke = {ClipStueck{0.25, 1500, true, delle()}};
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"), {a}));
        Kanal zwei = visualKanal(QStringLiteral("CH2"), {clip(aussen, 5000, 7000, Mischart::Xor)});
        zwei.an = false;
        k.kanaele.push_back(zwei);
        Kanal audio;
        audio.art = KanalArt::Audio;
        audio.name = QStringLiteral("Audio");
        // ein Ton-Clip (C5): Start und Ende wie ein Bild-Clip, dazu Einsatz, Song-Laenge und Pegelkurve
        Clip song = clip(innen, 2000, 62000);
        song.einsatzMs = 15000;
        song.quellMs = 200000;
        song.aus = {4000, einfacheBlende(Kurvenform::EaseOut)};
        audio.clips.push_back(song);
        k.kanaele.push_back(audio);

        const QString pfad = dir.filePath(QStringLiteral("Programm.lvcomp"));
        REQUIRE(speichereKomposition(k, pfad));

        // in der Datei: darunter relativ, ausserhalb absolut
        QFile f(pfad);
        REQUIRE(f.open(QIODevice::ReadOnly));
        const QJsonObject wurzel = QJsonDocument::fromJson(f.readAll()).object();
        CHECK(wurzel.value(QStringLiteral("format")).toString() == QStringLiteral("lumiviz-composition"));
        const QJsonArray kanaele = wurzel.value(QStringLiteral("channels")).toArray();
        REQUIRE(kanaele.size() == 3);
        CHECK(kanaele[0].toObject().value(QStringLiteral("clips")).toArray()[0].toObject()
                  .value(QStringLiteral("file")).toString() == QStringLiteral("presets/Innen.lvfx"));
        CHECK(QFileInfo(kanaele[1].toObject().value(QStringLiteral("clips")).toArray()[0].toObject()
                            .value(QStringLiteral("file")).toString()).isAbsolute());

        Komposition g;
        QStringList bericht;
        REQUIRE(ladeKomposition(pfad, g, &bericht));
        CHECK(g.laengeMs == 120000);
        CHECK(g.rasterSekunden == doctest::Approx(0.5));
        REQUIRE(g.kanaele.size() == 3);
        REQUIRE(g.kanaele[0].clips.size() == 1);
        const VisualClip& ga = g.kanaele[0].clips[0];
        CHECK(QFileInfo(ga.datei) == QFileInfo(innen));
        CHECK(ga.startMs == 1000);
        CHECK(ga.endeMs == 9000);
        CHECK(ga.mischart == Mischart::Additive);
        CHECK(ga.grundpegel == doctest::Approx(0.75));
        CHECK(ga.ein.dauerMs == 2000);
        CHECK(ga.ein.form.punkte.front().form == Kurvenform::SKurve);
        CHECK(ga.aus.dauerMs == 12000);  // die eingestellte Dauer, nicht die gestauchte
        CHECK(ga.aus.form.punkte.front().form == Kurvenform::Halten);
        REQUIRE(ga.stuecke.size() == 1);
        CHECK(ga.stuecke[0].lage == doctest::Approx(0.25));
        CHECK(ga.stuecke[0].weiteMs == 1500);
        CHECK(ga.stuecke[0].gekippt);
        CHECK(gleich(ga.stuecke[0].form, delle()));
        CHECK_FALSE(g.kanaele[1].an);
        CHECK(g.kanaele[1].clips[0].mischart == Mischart::Xor);
        CHECK(QFileInfo(g.kanaele[1].clips[0].datei) == QFileInfo(aussen));
        CHECK(g.kanaele[2].art == KanalArt::Audio);
        REQUIRE(g.kanaele[2].clips.size() == 1);
        CHECK(g.kanaele[2].clips[0].startMs == 2000);
        CHECK(g.kanaele[2].clips[0].endeMs == 62000);
        CHECK(g.kanaele[2].clips[0].einsatzMs == 15000);
        CHECK(g.kanaele[2].clips[0].quellMs == 200000);
        CHECK(g.kanaele[2].clips[0].aus.dauerMs == 4000);
        CHECK(g.kanaele[2].clips[0].aus.form.punkte.front().form == Kurvenform::EaseOut);
        CHECK(g.kanaele[2].clips[0].songStelle(1000) == 16000);
        // Einsatz und Song-Laenge gibt es nur am Ton — am Bild stehen sie nicht in der Datei
        CHECK(kanaele[2].toObject().value(QStringLiteral("clips")).toArray()[0].toObject()
                  .contains(QStringLiteral("offset")));
        CHECK_FALSE(kanaele[0].toObject().value(QStringLiteral("clips")).toArray()[0].toObject()
                        .contains(QStringLiteral("offset")));
        CHECK_FALSE(kanaele[2].toObject().value(QStringLiteral("clips")).toArray()[0].toObject()
                        .contains(QStringLiteral("blend")));
        // die fehlende Datei ist gemeldet, der Clip bleibt
        REQUIRE(bericht.size() == 1);
        CHECK(bericht[0].contains(QStringLiteral("Aussen.avs")));

        // etwas anderes ist keine Komposition
        Komposition h;
        CHECK_FALSE(ausJson(QJsonObject{{QStringLiteral("format"), QStringLiteral("anders")}}, h));
        CHECK_FALSE(ladeKomposition(dir.filePath(QStringLiteral("fehlt.lvcomp")), h));
    }

    TEST_CASE("C1b Uhr: Start, Stopp, Sprung, Ende — die Zeit laeuft nur, wenn sie laeuft")
    {
        std::int64_t jetzt = 500000;  // die Zeitquelle des Tests
        Uhr uhr([&jetzt] { return jetzt; });
        CHECK_FALSE(uhr.laeuft());
        CHECK(uhr.stelleMs() == 0);
        jetzt += 3000;
        CHECK(uhr.stelleMs() == 0);  // steht

        uhr.starte();
        CHECK(uhr.laeuft());
        jetzt += 1500;
        CHECK(uhr.stelleMs() == 1500);
        uhr.stoppe();
        jetzt += 9000;
        CHECK(uhr.stelleMs() == 1500);  // angehalten bleibt die Stelle

        uhr.springe(10000);  // Sprung im Stand
        CHECK(uhr.stelleMs() == 10000);
        CHECK_FALSE(uhr.laeuft());
        uhr.starte();
        jetzt += 250;
        CHECK(uhr.stelleMs() == 10250);
        uhr.springe(2000);  // Sprung im Lauf: laeuft von dort weiter
        CHECK(uhr.laeuft());
        jetzt += 100;
        CHECK(uhr.stelleMs() == 2100);
        uhr.springe(-50);
        CHECK(uhr.stelleMs() == 0);

        // Ende: die Uhr haelt dort; ein neuer Start beginnt von vorn
        uhr.setLaengeMs(5000);
        jetzt += 4000;
        CHECK(uhr.stelleMs() == 4000);
        CHECK(uhr.laeuft());
        jetzt += 4000;
        CHECK(uhr.stelleMs() == 5000);
        CHECK_FALSE(uhr.laeuft());
        uhr.starte();
        CHECK(uhr.laeuft());
        jetzt += 300;
        CHECK(uhr.stelleMs() == 300);
        uhr.springe(99999);
        CHECK(uhr.stelleMs() == 5000);
        uhr.anDenAnfang();
        CHECK(uhr.stelleMs() == 0);

        Uhr wand;  // die Vorgabe-Zeitquelle: steht, bis man sie startet
        CHECK(wand.stelleMs() == 0);
    }

    TEST_CASE("C2b Uhr: in der Schleife beginnt sie am Ende von vorn — und haelt wieder, wenn die Schleife aus ist")
    {
        std::int64_t jetzt = 70000;
        Uhr uhr([&jetzt] { return jetzt; });
        uhr.setLaengeMs(5000);
        uhr.setSchleife(true);
        CHECK(uhr.schleife());
        CHECK_FALSE(uhr.laeuft());  // die Schleife startet nichts

        uhr.starte();
        jetzt += 4000;
        CHECK(uhr.stelleMs() == 4000);
        jetzt += 2500;  // ueber das Ende: von vorn
        CHECK(uhr.laeuft());
        CHECK(uhr.stelleMs() == 1500);
        jetzt += 12000;  // mehr als zwei Runden weiter
        CHECK(uhr.laeuft());
        CHECK(uhr.stelleMs() == 3500);

        // angehalten bleibt die Stelle der Runde, nicht die rohe Zeit
        uhr.stoppe();
        jetzt += 9000;
        CHECK(uhr.stelleMs() == 3500);
        uhr.starte();
        jetzt += 500;
        CHECK(uhr.stelleMs() == 4000);

        // Sprung im Lauf, dann ueber das Ende
        uhr.springe(4800);
        jetzt += 300;
        CHECK(uhr.stelleMs() == 100);

        // Schleife aus, mitten in einer spaeteren Runde: die Stelle bleibt,
        // und am Ende haelt die Uhr wieder
        jetzt += 11000;  // 100 + 11000 = zwei Runden und 1100
        CHECK(uhr.stelleMs() == 1100);
        uhr.setSchleife(false);
        CHECK(uhr.laeuft());
        CHECK(uhr.stelleMs() == 1100);
        jetzt += 2000;
        CHECK(uhr.stelleMs() == 3100);
        jetzt += 5000;
        CHECK(uhr.stelleMs() == 5000);
        CHECK_FALSE(uhr.laeuft());

        // am Ende stehend die Schleife einschalten: steht weiter; Start von vorn
        uhr.setSchleife(true);
        CHECK_FALSE(uhr.laeuft());
        CHECK(uhr.stelleMs() == 5000);
        uhr.starte();
        jetzt += 200;
        CHECK(uhr.stelleMs() == 200);

        // ohne Laenge gibt es kein Ende und keine Schleife
        Uhr endlos([&jetzt] { return jetzt; });
        endlos.setSchleife(true);
        endlos.starte();
        jetzt += 123456;
        CHECK(endlos.stelleMs() == 123456);
        CHECK(endlos.laeuft());
    }

    // -------------------------------------------------------------------------
    // Wiedergabe (GL)
    // -------------------------------------------------------------------------
    TEST_CASE("C1b GL: an jeder Stelle zeigt das Bild die erwarteten Clips mit den erwarteten Pegeln — auch nach einem Sprung")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xC00000);    // 192
        const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x000080);  // 128
        const QString gruen = farbPreset(dir, QStringLiteral("Gruen.lvfx"), 0x00FF00);
        const QSize groesse(64, 64);

        // CH1: Rot 0–4 s, 1 s ein, 1 s aus · Luecke · Gruen 6–8 s
        // CH2: Blau 2–5 s additiv, haelt halben Pegel, steigt in der letzten Sekunde auf voll
        Komposition k;
        VisualClip cRot = clip(rot, 0, 4000);
        cRot.setzeBlenden(1000, 1000);
        VisualClip cBlau = clip(blau, 2000, 5000, Mischart::Additive);
        ausAltenPunkten(cBlau, {{0, 0.5, Kurvenform::Linear}, {2000, 0.5, Kurvenform::Linear},
                                {3000, 1.0, Kurvenform::Linear}});
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"), {cRot, clip(gruen, 6000, 8000)}));
        k.kanaele.push_back(visualKanal(QStringLiteral("CH2"), {cBlau}));

        Wiedergabe w;
        w.setKomposition(k);
        w.setVorlaufMs(0);  // hier zaehlt nur das Bild
        const auto bei = [&](std::int64_t stelle) {
            REQUIRE(w.rendere(stelle, 1.0f / 60.0f, groesse));
            return w.mischer().bild().pixelColor(32, 32);
        };
        const auto nah = [](int ist, double soll) { return std::abs(ist - soll) <= 2.0; };

        struct Erwartung
        {
            std::int64_t stelle;
            double r, g, b;
            int clips;
        };
        const std::vector<Erwartung> soll = {
            {0, 0, 0, 0, 1},             // Rot beginnt bei Pegel 0
            {500, 96, 0, 0, 1},          // halb eingeblendet
            {1500, 192, 0, 0, 1},        // voll
            {2500, 192, 0, 64, 2},       // + Blau additiv auf halbem Pegel
            {3500, 96, 0, 64, 2},        // Rot blendet aus, Blau haelt
            {4500, 0, 0, 96, 1},         // nur Blau, steigt (0,75 von 128)
            {5500, 0, 0, 0, 0},          // Luecke: schwarz
            {7000, 0, 255, 0, 1},        // Gruen
            {8000, 0, 0, 0, 0},          // Ende ist ausschliesslich
        };
        for (const Erwartung& e : soll)
        {
            const QColor c = bei(e.stelle);
            CAPTURE(e.stelle);
            CHECK(nah(c.red(), e.r));
            CHECK(nah(c.green(), e.g));
            CHECK(nah(c.blue(), e.b));
            CHECK(w.gemischteClips() == e.clips);
        }
        // --- dieselben Stellen in anderer Reihenfolge (Spruenge): dasselbe Bild
        for (const std::int64_t stelle : {7000, 500, 4500, 2500, 5500, 1500})
        {
            const auto it = std::find_if(soll.begin(), soll.end(),
                                         [&](const Erwartung& e) { return e.stelle == stelle; });
            const QColor c = bei(stelle);
            CAPTURE(stelle);
            CHECK(nah(c.red(), it->r));
            CHECK(nah(c.green(), it->g));
            CHECK(nah(c.blue(), it->b));
        }

        // --- ein ausgeschalteter Kanal faellt weg
        Komposition ohne = k;
        ohne.kanaele[1].an = false;
        w.setKomposition(ohne);
        const QColor c = bei(2500);
        CHECK(nah(c.red(), 192));
        CHECK(nah(c.blue(), 0));
        CHECK(w.gemischteClips() == 1);
    }

    TEST_CASE("C1b GL: Vorwaermen und Entladen — Renderer gibt es nur fuer Clips, die laufen oder gleich dran sind")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString a = farbPreset(dir, QStringLiteral("A.lvfx"), 0xFF0000);
        const QString b = farbPreset(dir, QStringLiteral("B.lvfx"), 0x0000FF);
        const QSize groesse(64, 64);
        Komposition k;
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"), {clip(a, 5000, 8000), clip(b, 20000, 22000)}));
        k.kanaele.push_back(visualKanal(QStringLiteral("CH2"),
                                        {clip(dir.filePath(QStringLiteral("fehlt.lvfx")), 30000, 31000)}));
        Wiedergabe w;
        w.setKomposition(k);
        CHECK(w.vorlaufMs() == 2000);
        const auto bei = [&](std::int64_t stelle) { REQUIRE(w.rendere(stelle, 1.0f / 60.0f, groesse)); };

        bei(0);  // weit vor allem
        CHECK(w.rendererAnzahl() == 0);
        CHECK(w.gemischteClips() == 0);
        bei(3500);  // im Vorlauf von A: angelegt, aber nicht gemischt
        CHECK(w.hatRenderer(0, 0));
        CHECK(w.gemischteClips() == 0);
        CHECK(w.mischer().bild().pixelColor(32, 32) == QColor(0, 0, 0));
        bei(6000);  // A laeuft
        CHECK(w.hatRenderer(0, 0));
        CHECK(w.gemischteClips() == 1);
        CHECK(w.mischer().bild().pixelColor(32, 32).red() > 250);
        bei(9000);  // A vorbei, B noch fern: entladen
        CHECK(w.rendererAnzahl() == 0);
        bei(19000);  // im Vorlauf von B
        CHECK(w.hatRenderer(0, 1));
        CHECK_FALSE(w.hatRenderer(0, 0));
        bei(6000);  // Sprung zurueck: B geht, A kommt frisch
        CHECK(w.hatRenderer(0, 0));
        CHECK_FALSE(w.hatRenderer(0, 1));
        CHECK(w.rendererAnzahl() == 1);

        // eine Datei, die es nicht gibt: gemeldet, schwarz, kein Absturz —
        // und nicht in jedem Bild neu versucht
        CHECK(w.bericht().isEmpty());
        bei(30500);
        bei(30600);
        CHECK(w.gemischteClips() == 0);
        CHECK(w.bericht().size() == 1);
        CHECK(w.bericht()[0].contains(QStringLiteral("fehlt.lvfx")));

        w.entladeAlles();
        CHECK(w.rendererAnzahl() == 0);
    }

    TEST_CASE("C1b GL: ein Clip auf vollem Pegel ist bitgleich mit dem Renderer allein — mit und ohne Vorwaermen")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString preset = balkenPreset(dir, QStringLiteral("Balken.lvfx"), 0x40C0FF);
        const QSize groesse(160, 120);
        constexpr int kFrames = 30;

        MultiEffectVisualizer allein;
        REQUIRE(allein.loadChainFile(preset));
        allein.initialize();
        allein.resize(groesse);
        Leinwand lAllein;
        for (int i = 0; i < kFrames; ++i) lAllein.rendere(allein, 1.0f / 60.0f, groesse);
        const QImage soll = lAllein.bild();

        // --- Clip ab Stelle 0: kein Vorwaermen moeglich, dieselben 30 Bilder
        Komposition k;
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"), {clip(preset, 0, 60000)}));
        Wiedergabe w;
        w.setKomposition(k);
        for (int i = 0; i < kFrames; ++i) REQUIRE(w.rendere(i * 1000 / 60, 1.0f / 60.0f, groesse));
        CHECK(verschieden(w.mischer().bild(), soll) == 0);

        // --- Clip ab 1 s mit Vorwaermen: hat beim Start ein Bild hinter sich
        Komposition spaet;
        spaet.kanaele.push_back(visualKanal(QStringLiteral("CH1"), {clip(preset, 1000, 60000)}));
        Wiedergabe v;
        v.setKomposition(spaet);
        REQUIRE(v.rendere(500, 1.0f / 60.0f, groesse));  // vorwaermen
        CHECK(v.hatRenderer(0, 0));
        for (int i = 0; i < kFrames; ++i) REQUIRE(v.rendere(1000 + i * 1000 / 60, 1.0f / 60.0f, groesse));
        const int abweichung = verschieden(v.mischer().bild(), soll);
        MESSAGE("vorgewaermter Clip nach " << kFrames << " Bildern gegen den Lauf allein: "
                << abweichung << " von " << groesse.width() * groesse.height() << " Pixeln anders");
        // BELEG: das Vorwaerm-Bild (Delta 0) verstellt den Zustand dieses
        // Presets nicht — der Clip beginnt wie ohne Vorwaermen.
        CHECK(abweichung == 0);

        // --- ohne Vorwaermen (Vorlauf 0): wieder bitgleich
        Wiedergabe o;
        o.setKomposition(spaet);
        o.setVorlaufMs(0);
        REQUIRE(o.rendere(500, 1.0f / 60.0f, groesse));
        CHECK_FALSE(o.hatRenderer(0, 0));
        for (int i = 0; i < kFrames; ++i) REQUIRE(o.rendere(1000 + i * 1000 / 60, 1.0f / 60.0f, groesse));
        CHECK(verschieden(o.mischer().bild(), soll) == 0);
    }

    TEST_CASE("C1b GL: die Karaoke-Ebene liegt einmal ueber dem Gemischten, auch in der Luecke")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        namespace ly = lumi::multieffect::lyrics;
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString a = farbPreset(dir, QStringLiteral("A.lvfx"), 0x203040);
        const QString b = farbPreset(dir, QStringLiteral("B.lvfx"), 0x102010);
        const QSize groesse(320, 240);
        Komposition k;
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"), {clip(a, 0, 4000)}));
        k.kanaele.push_back(visualKanal(QStringLiteral("CH2"), {clip(b, 0, 4000, Mischart::Additive)}));

        // ohne Ebene: das Bild der beiden Clips
        MultiEffectVisualizer::setKaraokeEbene(false);
        Wiedergabe ohne;
        ohne.setKomposition(k);
        for (int i = 0; i < 5; ++i) REQUIRE(ohne.rendere(1000, 1.0f / 60.0f, groesse));
        CHECK_FALSE(ohne.karaokeAufgelegt());
        const QImage bildOhne = ohne.mischer().bild();

        MultiEffectVisualizer::notifyPlayState(false);
        MultiEffectVisualizer::setSongtextTitelVersatz(0);
        MultiEffectVisualizer::setSongtext(ly::leseLrc("[00:10.00]MMMM MMMM\n[00:13.00]IIII\n"));
        MultiEffectVisualizer::setKaraokeEbene(true);
        MultiEffectVisualizer::notifyPlayPosition(11000);

        // ein Renderer allein mit demselben Songtext ueber einem der beiden
        // Presets: so viele Pixel Text (ueber der Vorgabe-Kette eines leeren
        // Renderers waeren es andere — der Grund dort ist nicht einfarbig)
        MultiEffectVisualizer allein;
        REQUIRE(allein.loadChainFile(a));
        allein.initialize();
        allein.resize(groesse);
        Leinwand lAllein;
        for (int i = 0; i < 5; ++i) lAllein.rendere(allein, 1.0f / 60.0f, groesse);
        REQUIRE(allein.debugKaraokeGezeichnet());
        const int textAllein = verschieden(lAllein.bild().convertToFormat(QImage::Format_ARGB32),
                                           allein.debugGrabRootSurface().convertToFormat(QImage::Format_ARGB32));
        REQUIRE(textAllein > 0);

        Wiedergabe mit;
        mit.setKomposition(k);
        for (int i = 0; i < 5; ++i) REQUIRE(mit.rendere(1000, 1.0f / 60.0f, groesse));
        CHECK(mit.karaokeAufgelegt());
        const int text = verschieden(mit.mischer().bild(), bildOhne);
        MESSAGE("Karaoke ueber zwei Clips: " << text << " Pixel, ein Renderer allein: " << textAllein);
        // einmal, nicht je Clip: genau so viele Pixel wie bei einem Renderer allein
        CHECK(text == textAllein);

        // in der Luecke: schwarz mit Songtext
        for (int i = 0; i < 3; ++i) REQUIRE(mit.rendere(9000, 1.0f / 60.0f, groesse));
        CHECK(mit.gemischteClips() == 0);
        CHECK(mit.karaokeAufgelegt());

        // Ebene aus: sofort weg
        MultiEffectVisualizer::setKaraokeEbene(false);
        REQUIRE(mit.rendere(1000, 1.0f / 60.0f, groesse));
        CHECK_FALSE(mit.karaokeAufgelegt());

        MultiEffectVisualizer::setSongtext(ly::Songtext{});
        MultiEffectVisualizer::notifyPlayPosition(0);
    }

    TEST_CASE("C3c GL: beim Bearbeiten behaelt ein Clip seinen Renderer — auch an anderem Platz")
    {
        Gl gl;
        if (!gl.gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString balken = balkenPreset(dir, QStringLiteral("Balken.lvfx"), 0xFF0000);
        const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x0000FF);
        const QSize groesse(64, 64);

        // CH1: Balken 0–10 s, Blau 20–30 s · CH2: noch einmal Balken 0–10 s
        Komposition k;
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"),
                                        {clip(balken, 0, 10000), clip(blau, 20000, 30000)}));
        k.kanaele.push_back(visualKanal(QStringLiteral("CH2"), {clip(balken, 0, 10000)}));
        Wiedergabe w;
        w.setKomposition(k);
        for (int i = 0; i < 3; ++i) REQUIRE(w.rendere(1000, 1.0f / 60.0f, groesse));
        MultiEffectVisualizer* const unten = w.rendererVon(0, 0);
        MultiEffectVisualizer* const oben = w.rendererVon(1, 0);
        REQUIRE(unten != nullptr);
        REQUIRE(oben != nullptr);
        CHECK(unten != oben);
        CHECK(w.rendererVon(0, 1) == nullptr);  // Blau laeuft noch nicht
        CHECK(w.rendererVon(5, 0) == nullptr);
        CHECK(w.rendererAnzahl() == 2);
        const auto anlaeufe = w.anlaeufe().size();

        // --- ein anderer Clip wird verschoben: beide Renderer bleiben, nichts legt sich neu an
        k.kanaele[0].clips[1].startMs = 40000;
        k.kanaele[0].clips[1].endeMs = 50000;
        w.ersetzeKomposition(k);
        CHECK(w.rendererVon(0, 0) == unten);
        CHECK(w.rendererVon(1, 0) == oben);
        REQUIRE(w.rendere(1016, 1.0f / 60.0f, groesse));
        CHECK(w.rendererVon(0, 0) == unten);
        CHECK(w.anlaeufe().size() == anlaeufe);
        CHECK(w.komposition().kanaele[0].clips[1].startMs == 40000);

        // --- der laufende Clip selbst wird laenger: er behaelt seinen Renderer
        k.kanaele[0].clips[0].endeMs = 15000;
        w.ersetzeKomposition(k);
        CHECK(w.rendererVon(0, 0) == unten);

        // --- der Clip aus CH1 zieht nach CH2 um: sein Renderer folgt ihm,
        //     der Clip, der dort schon lag, behaelt den seinen
        Komposition umgezogen = k;
        umgezogen.kanaele[1].clips.push_back(clip(balken, 12000, 27000));
        umgezogen.kanaele[0].clips.erase(umgezogen.kanaele[0].clips.begin());
        w.ersetzeKomposition(umgezogen);
        CHECK(w.rendererVon(1, 0) == oben);
        CHECK(w.rendererVon(1, 1) == unten);
        CHECK(w.rendererVon(0, 0) == nullptr);  // dort liegt jetzt Blau, das nicht laeuft
        CHECK(w.rendererAnzahl() == 2);
        CHECK(w.anlaeufe().size() == anlaeufe);

        // --- ein Clip verschwindet: sein Renderer geht
        Komposition ohne = umgezogen;
        ohne.kanaele[1].clips.erase(ohne.kanaele[1].clips.begin());  // der mit `oben`
        w.ersetzeKomposition(ohne);
        CHECK(w.rendererAnzahl() == 1);
        CHECK(w.rendererVon(1, 0) == unten);  // der verbliebene ist nachgerueckt

        // --- ein Clip bekommt eine andere Datei: der alte Renderer geht, ein neuer legt sich an
        Komposition andere = ohne;
        andere.kanaele[1].clips[0] = clip(blau, 0, 27000);
        w.ersetzeKomposition(andere);
        CHECK(w.rendererAnzahl() == 0);
        REQUIRE(w.rendere(1050, 1.0f / 60.0f, groesse));
        CHECK(w.rendererVon(1, 0) != nullptr);
        CHECK(w.anlaeufe().size() == anlaeufe + 1);

        // --- eine NEUE Komposition (setKomposition) entlaedt dagegen alles
        w.setKomposition(k);
        CHECK(w.rendererAnzahl() == 0);
    }
}
