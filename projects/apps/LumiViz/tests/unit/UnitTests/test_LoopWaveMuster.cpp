/**
 * @file test_LoopWaveMuster.cpp
 * @brief Loop, Stufe 15b: Modell und Datei des Wave-Musters (docs/audio/Loop_Konzept.md §4p,
 *        „Was festliegt“; Entscheide Patrik WV1 a bis WV9 a, S168)
 *
 * - Die Datei (Fassung 16): `kind: "wave"` mit allen Feldern, immer geschrieben, ohne
 *   `swingOffset`; wieder gelesen Bit fuer Bit gleich; die Datei genannt wie eine
 *   Sample-Schicht (eigenes beim Namen, sonst Pfad relativ zur Datei), was fehlt, im Bericht.
 * - `richte`: die Grenzen aus „Was festliegt“, 512 Takte nur fuer Wave-Muster, von den Spuren
 *   nur Expression und Pitch, die Abschnitte von Autotune sortiert und ohne Ueberlappung.
 * - Die Herkunft nach *Bounce* (WV7 a) in der Huelle einer `.lvclip`.
 * - Die Huellen aus §4o tragen Wave-Muster ohne eigene Fassung; `rechneUm` rechnet Lage,
 *   Takte und Spuren nach Schlaegen um, Ausschnitt und Autotune bleiben.
 * - Ein Wave-Muster ohne Datei klingt nicht: keine Klaenge, keine Stimmen (den Klang mit
 *   Datei rechnet Stufe 15c, test_LoopWaveKlang.cpp).
 *
 * Ohne Ton, ohne Fenster, ohne Qt-Anwendung.
 */

#include <doctest.h>

#include "loop/Austausch.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Sample.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>

#include <cmath>
#include <limits>

using namespace lumi::loop;

namespace
{

/// Ein Wave-Muster mit jedem Feld abseits der Vorgabe.
Muster vollesWave(const QString& datei)
{
    Muster m;
    m.name = QStringLiteral("Voc");
    m.art = MusterArt::Wave;
    m.takte = 8;
    WaveAngaben& w = m.wave;
    w.datei = datei;
    w.start = 1.25;
    w.ende = 9.5;
    w.versatz = -2.5;  // ein Auftakt von zweieinhalb Schritten
    w.gainDb = -3.0;
    w.fadeInMs = 15.0;
    w.fadeOutMs = 120.0;
    w.bezugBpm = 96.0;
    w.folgtTempo = true;
    w.tonhoehenArt = TonhoehenArt::Stimme;
    w.autotune.grundton = 9;
    w.autotune.skala = Skala::Moll;
    w.autotune.staerke = 0.8;
    w.autotune.speedMs = 35.0;
    w.autotune.abschnitte = {{1.5, 2.0, 69, -12, 1.0, 0.0}, {2.5, 3.25, 72, 0, 0.6, 50.0}};
    m.zeitspuren.von(Steuerspur::Expression) = {{0.0, 1.0}, {16.0, 0.5}};
    m.zeitspuren.von(Steuerspur::Pitch) = {{4.0, 0.0}, {8.0, 2.0}};
    return m;
}

/// Ein Loop mit dem Wave-Muster in einem Kanal, zwei Wiederholungen ab Takt 1.
Loop loopMit(const Muster& wave)
{
    Loop loop;
    loop.bpm = 100.0;
    loop.muster = {wave};
    loop.kanaele = {{QStringLiteral("Vocals"), true, {{wave.name, 1, 2}}}};
    loop.richte();
    return loop;
}

/// Ein Drum-Clip als Herkunft (wie *Bounce* sie ablegt): ein Kanal, ein Clip ab Takt 0.
Loop drumClip()
{
    Loop quelle;
    quelle.bpm = 100.0;
    Muster d;
    d.name = QStringLiteral("Beat");
    d.takte = 1;
    d.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    d.zeilen = {std::vector<double>(16, 0.0)};
    d.zeilen[0][0] = 1.0;
    d.zeilen[0][8] = 0.7;
    quelle.muster = {d};
    quelle.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("Beat"), 3, 2}}}};
    quelle.richte();
    const std::optional<Loop> clip = clipDatei(quelle, 0, 0);
    REQUIRE(clip.has_value());
    return *clip;
}

}  // namespace

TEST_SUITE("LoopWaveMuster")
{
    TEST_CASE("Loop Wave 15b: Datei — Fassung 16, alle Felder, wieder gelesen")
    {
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString datei = QDir(ordner.path()).filePath(QStringLiteral("takes/voc.wav"));
        REQUIRE(QDir(ordner.path()).mkpath(QStringLiteral("takes")));
        {
            QFile f(datei);
            REQUIRE(f.open(QIODevice::WriteOnly));
        }
        const Loop loop = loopMit(vollesWave(datei));

        const QJsonObject json = alsJson(loop, ordner.path());
        CHECK(json.value(QStringLiteral("version")).toInt() == 16);
        const QJsonArray muster = json.value(QStringLiteral("patterns")).toArray();
        REQUIRE(muster.size() == 1);
        const QJsonObject o = muster[0].toObject();
        CHECK(o.value(QStringLiteral("kind")).toString() == QStringLiteral("wave"));
        CHECK(o.value(QStringLiteral("file")).toString() == QStringLiteral("takes/voc.wav"));  // relativ zur Datei
        CHECK_FALSE(o.contains(QStringLiteral("swingOffset")));  // RS1-1 a: Wave-Muster haben keinen
        CHECK(o.value(QStringLiteral("pitchMethod")).toString() == QStringLiteral("voice"));
        const QJsonObject autotune = o.value(QStringLiteral("autotune")).toObject();
        CHECK(autotune.value(QStringLiteral("scale")).toString() == QStringLiteral("minor"));
        CHECK(autotune.value(QStringLiteral("sections")).toArray().size() == 2);

        Loop zurueck;
        QStringList bericht;
        REQUIRE(ausJson(json, zurueck, ordner.path(), &bericht));
        CHECK(bericht.isEmpty());
        CHECK(zurueck == loop);
        CHECK(zurueck.muster[0].art == MusterArt::Wave);
        CHECK(zurueck.muster[0].wave.datei == QDir::cleanPath(datei));

        SUBCASE("alle Felder immer geschrieben — auch mit den Vorgaben")
        {
            Muster leer;
            leer.name = QStringLiteral("Leer");
            leer.art = MusterArt::Wave;
            const QJsonObject l = alsJson(loopMit(leer)).value(QStringLiteral("patterns")).toArray()[0].toObject();
            for (const char* feld : {"name", "bars", "kind", "file", "start", "end", "offset", "gain", "fadeIn",
                                     "fadeOut", "referenceBpm", "followTempo", "pitchMethod", "lanes", "autotune",
                                     "origin"})
            {
                CAPTURE(feld);
                CHECK(l.contains(QLatin1String(feld)));
            }
            CHECK(l.value(QStringLiteral("origin")).toObject().isEmpty());
            const QJsonObject a = l.value(QStringLiteral("autotune")).toObject();
            for (const char* feld : {"key", "scale", "strength", "speed", "sections"})
            {
                CAPTURE(feld);
                CHECK(a.contains(QLatin1String(feld)));
            }
        }
        SUBCASE("ein eigenes Sample beim Namen; was fehlt, steht im Bericht")
        {
            QTemporaryDir samples;
            REQUIRE(samples.isValid());
            setzeEigenenSampleOrdnerFuerTests(samples.path());
            {
                QFile f(QDir(samples.path()).filePath(QStringLiteral("take_1.wav")));
                REQUIRE(f.open(QIODevice::WriteOnly));
            }
            Muster eigen = vollesWave(QStringLiteral("user:take_1"));
            Muster fehlt = vollesWave(QStringLiteral("user:take_2"));
            fehlt.name = QStringLiteral("Voc 2");
            Muster pfad = vollesWave(QDir(ordner.path()).filePath(QStringLiteral("weg.wav")));
            pfad.name = QStringLiteral("Voc 3");
            Loop drei;
            drei.muster = {eigen, fehlt, pfad};
            drei.richte();
            const QJsonObject j = alsJson(drei, ordner.path());
            CHECK(j.value(QStringLiteral("patterns")).toArray()[0].toObject().value(QStringLiteral("file")).toString() ==
                  QStringLiteral("user:take_1"));
            Loop gelesen;
            QStringList zeilen;
            REQUIRE(ausJson(j, gelesen, ordner.path(), &zeilen));
            setzeEigenenSampleOrdnerFuerTests(QString());
            CHECK(gelesen == drei);
            REQUIRE(zeilen.size() == 2);
            CHECK(zeilen[0] == QStringLiteral("Sample not found: user:take_2"));
            CHECK(zeilen[1].startsWith(QStringLiteral("File not found: ")));
        }
        SUBCASE("Fassung 15 liest weiter; ein Wave-Muster kennt sie nicht")
        {
            // ein Loop ohne Wave-Muster: unveraendert gelesen, wie bisher
            Loop alt;
            Muster d;
            d.name = QStringLiteral("D");
            d.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
            alt.muster = {d};
            alt.richte();
            QJsonObject j = alsJson(alt);
            j[QStringLiteral("version")] = 15;
            Loop gelesen;
            QStringList zeilen;
            REQUIRE(ausJson(j, gelesen, {}, &zeilen));
            CHECK(zeilen.isEmpty());
            CHECK(gelesen == alt);
        }
    }

    TEST_CASE("Loop Wave 15b: richte — die Grenzen")
    {
        Muster m = vollesWave(QStringLiteral("/irgendwo/voc.wav"));
        m.takte = 600;
        m.swingOffset = 0.4;
        m.instrument = QStringLiteral("piano");
        m.noten = {Note{0, 1, 60}};
        m.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
        m.zeitspuren.von(Steuerspur::Sustain) = {{0.0, 1.0}};
        m.zeitspuren.von(Steuerspur::Vibrato) = {{0.0, 20.0}};
        m.zeitspuren.schalter = {{QStringLiteral("unaCorda"), {{0.0, 1.0}}}};
        WaveAngaben& w = m.wave;
        w.gainDb = 40.0;
        w.fadeInMs = -5.0;
        w.fadeOutMs = 1.0e6;
        w.bezugBpm = std::numeric_limits<double>::quiet_NaN();
        w.tonhoehenArt = static_cast<TonhoehenArt>(7);
        w.start = -1.0;
        w.versatz = -1.0e9;
        w.autotune.grundton = 14;
        w.autotune.staerke = 2.0;
        w.autotune.speedMs = 900.0;
        w.autotune.abschnitte = {
            {3.0, 4.0, 200, 80, 1.5, -10.0},  // Zielton, Cent, Strength, Speed in ihre Grenzen
            {1.0, 3.5, 60, 0, 1.0, 0.0},      // ueberlappt den naechsten: der beginnt, wo dieser endet
            {5.0, 5.0, 62, 0, 1.0, 0.0},      // ohne Laenge: geht
            {-2.0, 0.5, 64, -80, 1.0, 0.0},   // Anfang ab 0
        };
        Loop loop = loopMit(m);
        const Muster& r = loop.muster[0];

        CHECK(r.takte == kWaveTakteMax);
        CHECK(r.swingOffset == 0.0);
        CHECK(r.instrument.isEmpty());
        CHECK(r.noten.empty());
        CHECK(r.spuren.empty());
        CHECK(r.zeilen.empty());
        CHECK(r.zeitspuren.von(Steuerspur::Sustain).empty());
        CHECK(r.zeitspuren.von(Steuerspur::Vibrato).empty());
        CHECK(r.zeitspuren.schalter.empty());
        CHECK(r.zeitspuren.von(Steuerspur::Expression).size() == 2);  // bleiben
        CHECK(r.zeitspuren.von(Steuerspur::Pitch).size() == 2);
        CHECK(r.wave.gainDb == kWaveGainMaxDb);
        CHECK(r.wave.fadeInMs == 0.0);
        CHECK(r.wave.fadeOutMs == kWaveBlendeMaxMs);
        CHECK(r.wave.bezugBpm == 120.0);
        CHECK(r.wave.tonhoehenArt == TonhoehenArt::Allgemein);
        CHECK(r.wave.start == 0.0);
        CHECK(r.wave.versatz == -static_cast<double>(kWaveTakteMax * loop.schritteJeTakt()));
        const Autotune& a = r.wave.autotune;
        CHECK(a.grundton == 11);
        CHECK(a.staerke == 1.0);
        CHECK(a.speedMs == kAutotuneSpeedMaxMs);
        REQUIRE(a.abschnitte.size() == 3);
        CHECK(a.abschnitte[0] == AutotuneAbschnitt{0.0, 0.5, 64, -kAutotuneCentMax, 1.0, 0.0});
        CHECK(a.abschnitte[1] == AutotuneAbschnitt{1.0, 3.5, 60, 0, 1.0, 0.0});
        CHECK(a.abschnitte[2] == AutotuneAbschnitt{3.5, 4.0, 127, kAutotuneCentMax, 1.0, 0.0});

        SUBCASE("ein leerer Ausschnitt reicht bis zum Ende")
        {
            Muster k = vollesWave(QStringLiteral("/x.wav"));
            k.wave.start = 4.0;
            k.wave.ende = 3.0;
            CHECK(loopMit(k).muster[0].wave.ende == 0.0);
        }
        SUBCASE("512 Takte nur fuer Wave-Muster; die anderen tragen keine Wave-Angaben")
        {
            Muster d;
            d.name = QStringLiteral("D");
            d.takte = 600;
            d.wave = vollesWave(QStringLiteral("/x.wav")).wave;
            Muster i = d;
            i.name = QStringLiteral("I");
            i.art = MusterArt::Instrument;
            Loop zwei;
            zwei.muster = {d, i};
            zwei.richte();
            CHECK(zwei.muster[0].takte == kTakteMax);
            CHECK(zwei.muster[1].takte == kTakteMax);
            CHECK(zwei.muster[0].wave == WaveAngaben{});
            CHECK(zwei.muster[1].wave == WaveAngaben{});
            CHECK(taktHoechstens(MusterArt::Wave) == 512);
        }
    }

    TEST_CASE("Loop Wave 15b: die Herkunft nach Bounce in der Huelle einer .lvclip")
    {
        Muster m = vollesWave(QStringLiteral("/x.wav"));
        m.wave.herkunft = Herkunft(drumClip());
        const Loop loop = loopMit(m);

        const QJsonObject o = alsJson(loop).value(QStringLiteral("patterns")).toArray()[0].toObject();
        const QJsonObject huelle = o.value(QStringLiteral("origin")).toObject();
        CHECK(huelle.value(QStringLiteral("format")).toString() == QStringLiteral("lumiviz-clip"));
        CHECK(huelle.value(QStringLiteral("version")).toInt() == kAustauschFassung);
        CHECK(huelle.value(QStringLiteral("loop")).toObject().value(QStringLiteral("version")).toInt() == 16);

        // die Datei gibt es nicht — das meldet der Bericht; was in der Herkunft fehlt, nicht
        const QString fehlt = QStringLiteral("File not found: %1").arg(QDir::toNativeSeparators(QStringLiteral("/x.wav")));
        Loop zurueck;
        QStringList bericht;
        REQUIRE(ausJson(alsJson(loop), zurueck, {}, &bericht));
        CHECK(bericht == QStringList{fehlt});
        CHECK(zurueck == loop);
        const Herkunft& h = zurueck.muster[0].wave.herkunft;
        REQUIRE_FALSE(h.leer());
        CHECK(h.clip().kanaele.size() == 1);
        CHECK(h.clip().kanaele[0].clips[0].startTakt == 0);
        CHECK(h.clip().kanaele[0].clips[0].wiederholungen == 2);

        SUBCASE("gleich heisst: der gleiche kleine Loop, nicht derselbe Zeiger")
        {
            CHECK(Herkunft(drumClip()) == Herkunft(drumClip()));
            Loop anders = drumClip();
            anders.muster[0].zeilen[0][4] = 0.5;
            CHECK_FALSE(Herkunft(anders) == Herkunft(drumClip()));
            CHECK_FALSE(Herkunft() == Herkunft(drumClip()));
            CHECK(Herkunft() == Herkunft());
        }
        SUBCASE("eine Herkunft, die keine .lvclip ist: leer, mit Zeile im Bericht")
        {
            QJsonObject j = alsJson(loop);
            QJsonArray muster = j.value(QStringLiteral("patterns")).toArray();
            QJsonObject w = muster[0].toObject();
            QJsonObject kaputt = w.value(QStringLiteral("origin")).toObject();
            kaputt[QStringLiteral("format")] = QStringLiteral("lumiviz-track");
            w[QStringLiteral("origin")] = kaputt;
            muster[0] = w;
            j[QStringLiteral("patterns")] = muster;
            Loop gelesen;
            QStringList zeilen;
            REQUIRE(ausJson(j, gelesen, {}, &zeilen));
            CHECK(gelesen.muster[0].wave.herkunft.leer());
            CHECK(zeilen == QStringList{fehlt, QStringLiteral("Bounce origin not readable: Voc")});
        }
    }

    TEST_CASE("Loop Wave 15b: Huellen aus §4o und rechneUm")
    {
        const Loop loop = loopMit(vollesWave(QStringLiteral("/x.wav")));

        SUBCASE("in .lvpattern, .lvclip und .lvtrack ohne eigene Fassung")
        {
            const std::optional<Loop> muster = musterDatei(loop, QStringLiteral("Voc"));
            const std::optional<Loop> clip = clipDatei(loop, 0, 0);
            const std::optional<Loop> spur = spurDatei(loop, 0);
            REQUIRE(muster.has_value());
            REQUIRE(clip.has_value());
            REQUIRE(spur.has_value());
            for (const auto& [art, teil] : {std::pair{AustauschArt::Muster, *muster}, std::pair{AustauschArt::Clip, *clip},
                                            std::pair{AustauschArt::Spur, *spur}})
            {
                CAPTURE(static_cast<int>(art));
                const QByteArray text = austauschText(art, teil);
                Loop gelesen;
                REQUIRE(austauschAusText(text, art, gelesen));
                CHECK(gelesen == teil);
                CHECK(gelesen.muster[0].art == MusterArt::Wave);
            }
        }
        SUBCASE("gleiches Raster: Bit fuer Bit; das Muster wird mitbenutzt")
        {
            Loop teil = *musterDatei(loop, QStringLiteral("Voc"));
            const Loop vorher = teil;
            (void)rechneUm(teil, teil.schlaegeJeTakt, teil.schritteJeSchlag);
            CHECK(teil == vorher);
            Loop ziel = loop;
            const Einsetzung e = setzeEin(ziel, teil, AustauschArt::Muster, 0, 5);
            CHECK(e.umbenannt.isEmpty());
            CHECK(ziel.muster.size() == 1);
        }
        SUBCASE("anderes Raster: Lage, Takte, Spuren nach Schlaegen; Ausschnitt und Autotune bleiben")
        {
            Loop teil = *musterDatei(loop, QStringLiteral("Voc"));  // 4/4 mit 4 Schritten je Schlag
            (void)rechneUm(teil, 3, 2);                             // 3/4 mit 2 Schritten je Schlag
            const Muster& r = teil.muster[0];
            const WaveAngaben& vorher = loop.muster[0].wave;
            CHECK(r.takte == 11);  // 32 Schlaege in Takten zu 3, aufgerundet
            CHECK(r.wave.versatz == doctest::Approx(-1.25));
            CHECK(r.zeitspuren.von(Steuerspur::Pitch)[1].stelle == doctest::Approx(4.0));
            CHECK(r.wave.start == vorher.start);
            CHECK(r.wave.ende == vorher.ende);
            CHECK(r.wave.autotune == vorher.autotune);
        }
        SUBCASE("ein Wave-Muster mit mehr als 64 Takten bleibt beim Umrechnen lang")
        {
            Muster lang = vollesWave(QStringLiteral("/x.wav"));
            lang.takte = 200;
            Loop teil = *musterDatei(loopMit(lang), QStringLiteral("Voc"));
            (void)rechneUm(teil, 2, 4);
            CHECK(teil.muster[0].takte == 400);
        }
    }

    TEST_CASE("Loop Wave 15b: ohne Datei klingt es nicht — keine Klaenge, keine Stimmen")
    {
        // mit Datei klingt es seit Stufe 15c (test_LoopWaveKlang.cpp)
        const Loop loop = loopMit(vollesWave(QString()));
        CHECK(loop.klangNamen().isEmpty());
        const Ablauf ablauf = plane(loop, 44100);
        CHECK(ablauf.stimmen.empty());
        CHECK(loop.durchlaufTakte() == 1 + 2 * 8);  // der Clip zaehlt mit seiner Laenge
    }
}
