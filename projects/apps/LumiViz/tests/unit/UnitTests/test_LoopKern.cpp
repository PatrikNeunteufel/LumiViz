/**
 ****************************************************************************************
 * @file   test_LoopKern.cpp
 * @brief  Loop, Stufe 3: Modell, Datei `.lvloop` und Rechenkern
 *         (docs/audio/Loop_Konzept.md §4 und §5)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Fenster. Die Faelle zum Rechenkern zaehlen die gerechneten
 * Abtastwerte nach — bei 44100 Hz, 120 BPM und vier Schritten je Schlag ist
 * ein Schritt 5512,5 Abtastwerte lang.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kKernRate = 44100;

/// Das Beispiel aus dem Konzept: drei Muster, Abfolge ueber acht Takte.
Loop beispielLoop(const QString& clapDatei)
{
    Loop loop;
    loop.spuren = {
        {QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
        {QStringLiteral("Snare"), QStringLiteral("kit:snare"), 0.9, 0.0, false},
        {QStringLiteral("Hi-Hat"), QStringLiteral("kit:hat_closed"), 0.6, 0.2, false},
        {QStringLiteral("My Clap"), clapDatei, 0.8, -0.2, false},
    };
    Muster grund;
    grund.name = QStringLiteral("Grund");
    grund.zeilen = {
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {1, 0, 0.5, 0, 1, 0, 0.5, 0, 1, 0, 0.5, 0, 1, 0, 0.5, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    Muster fill;
    fill.name = QStringLiteral("Fill");
    fill.zeilen = {
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0.6, 0.7, 0.8, 0.9, 1, 1},
        {1, 0, 0.5, 0, 1, 0, 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    };
    Muster pause;
    pause.name = QStringLiteral("Break");
    pause.zeilen = {
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };
    loop.muster = {grund, fill, pause};
    loop.abfolge = {QStringLiteral("Grund"), QStringLiteral("Grund"), QStringLiteral("Grund"),
                    QStringLiteral("Fill"),  QStringLiteral("Grund"), QStringLiteral("Grund"),
                    QStringLiteral("Grund"), QStringLiteral("Break")};
    return loop;
}

/// Ein Loop mit EINER Spur und einem Muster von einem Schlag (vier Schritte):
/// ein Durchlauf ist 22050 Abtastwerte lang.
Loop einSchlagLoop(std::vector<double> zeile)
{
    Loop loop;
    loop.schlaegeJeTakt = 1;
    loop.spuren = {{QStringLiteral("A"), QStringLiteral("kit:a"), 1.0, 0.0, false}};
    Muster m;
    m.name = QStringLiteral("M");
    m.zeilen = {std::move(zeile)};
    loop.muster = {m};
    loop.richte();
    return loop;
}

Klang gleichKlang(int frames, float wert)
{
    Klang k;
    k.daten.assign(static_cast<std::size_t>(frames) * 2, wert);
    return k;
}

/// Ein Klang aus festen, unregelmaessigen Werten (links und rechts verschieden).
Klang bunterKlang(int frames, std::uint32_t saat)
{
    Klang k;
    k.daten.resize(static_cast<std::size_t>(frames) * 2);
    for (float& wert : k.daten)
    {
        saat = saat * 1664525u + 1013904223u;
        wert = static_cast<float>(saat >> 8) / 8388608.0f - 1.0f;
    }
    return k;
}

std::vector<float> rechneBereich(const Ablauf& ablauf, const std::vector<Klang>& klaenge, std::int64_t von,
                                 int frames)
{
    // mit Unsinn vorbelegt: `rechne` muss den Block ganz ueberschreiben
    std::vector<float> aus(static_cast<std::size_t>(frames) * 2, 9.0f);
    rechne(ablauf, klaenge, von, frames, aus.data());
    return aus;
}

float linksBei(const std::vector<float>& block, std::int64_t frame)
{
    return block[static_cast<std::size_t>(frame) * 2];
}

}  // namespace

// =============================================================================
// Modell und Datei
// =============================================================================

TEST_CASE("Loop Modell: Raster, Durchlauf und Abfolge")
{
    Loop loop = beispielLoop(QStringLiteral("kit:clap"));
    QStringList bericht;
    loop.richte(&bericht);
    CHECK(bericht.isEmpty());
    CHECK(loop.schritteJeTakt() == 16);
    CHECK(loop.durchlaufSchritte() == 128);
    CHECK(loop.durchlaufMs() == doctest::Approx(16000.0));
    CHECK(loop.ablaufMuster().size() == 8u);
    CHECK(loop.ablaufMuster()[3] == loop.findeMuster(QStringLiteral("Fill")));

    SUBCASE("ohne Abfolge spielt das erste Muster")
    {
        loop.abfolge.clear();
        REQUIRE(loop.ablaufMuster().size() == 1u);
        CHECK(loop.ablaufMuster()[0] == &loop.muster[0]);
        CHECK(loop.durchlaufSchritte() == 16);
    }
    SUBCASE("ein Name, den es nicht gibt, ist ein Takt Stille und steht im Bericht")
    {
        loop.abfolge = {QStringLiteral("Grund"), QStringLiteral("Gibtsnicht")};
        loop.richte(&bericht);
        REQUIRE(bericht.size() == 1);
        CHECK(bericht[0] == QStringLiteral("Pattern not found: Gibtsnicht"));
        CHECK(loop.ablaufMuster()[1] == nullptr);
        CHECK(loop.durchlaufSchritte() == 32);
    }
    SUBCASE("ein leerer Loop ist ein Takt Stille")
    {
        Loop leer;
        leer.richte();
        REQUIRE(leer.ablaufMuster().size() == 1u);
        CHECK(leer.ablaufMuster()[0] == nullptr);
        CHECK(leer.durchlaufSchritte() == 16);
        CHECK(leer.durchlaufMs() == doctest::Approx(2000.0));
    }
    SUBCASE("ein Muster ueber zwei Takte im Dreivierteltakt mit Triolen")
    {
        loop.schlaegeJeTakt = 3;
        loop.schritteJeSchlag = 3;
        loop.muster[0].takte = 2;
        loop.abfolge = {QStringLiteral("Grund")};
        loop.richte();
        CHECK(loop.schritteJeTakt() == 9);
        CHECK(loop.durchlaufSchritte() == 18);
        CHECK(loop.muster[0].zeilen[0].size() == 18u);
    }
}

TEST_CASE("Loop Modell: richte bringt Unsinn in Ordnung")
{
    Loop loop;
    loop.bpm = 5.0;
    loop.schlaegeJeTakt = 0;
    loop.schritteJeSchlag = 99;
    loop.swing = 2.0;
    loop.laut = -1.0;
    loop.spuren = {{QStringLiteral("A"), QStringLiteral("kit:a"), 7.0, -3.0, false},
                   {QStringLiteral("B"), QStringLiteral("kit:b"), 1.0, 0.0, false}};
    Muster ohneName;
    ohneName.takte = 0;
    ohneName.zeilen = {{3.0, -1.0, 0.5}};  // eine Zeile zu wenig, zu kurz, Werte ausserhalb
    Muster doppelt;
    doppelt.name = QStringLiteral("Pattern 1");
    loop.muster = {ohneName, doppelt};
    QStringList bericht;
    loop.richte(&bericht);

    CHECK(loop.bpm == kBpmMin);
    CHECK(loop.schlaegeJeTakt == 1);
    CHECK(loop.schritteJeSchlag == kRasterMax);
    CHECK(loop.swing == 1.0);
    CHECK(loop.laut == 0.0);
    CHECK(loop.spuren[0].laut == 1.0);
    CHECK(loop.spuren[0].pan == -1.0);
    CHECK(loop.muster[0].name == QStringLiteral("Pattern 1"));
    CHECK(loop.muster[0].takte == 1);
    REQUIRE(loop.muster[0].zeilen.size() == 2u);
    REQUIRE(loop.muster[0].zeilen[0].size() == 16u);
    CHECK(loop.muster[0].zeilen[0][0] == 1.0);
    CHECK(loop.muster[0].zeilen[0][1] == 0.0);
    CHECK(loop.muster[0].zeilen[0][2] == 0.5);
    CHECK(loop.muster[0].zeilen[0][3] == 0.0);
    CHECK(loop.muster[0].zeilen[1].size() == 16u);
    REQUIRE(bericht.size() == 1);
    CHECK(bericht[0] == QStringLiteral("Pattern name used twice: Pattern 1"));

    loop.bpm = std::numeric_limits<double>::quiet_NaN();
    loop.richte();
    CHECK(loop.bpm == 120.0);
}

TEST_CASE("Loop Datei: das Beispiel aus dem Konzept wird so gelesen, wie es dort steht")
{
    const char* text = R"json({
      "format": "lumiviz-loop",
      "version": 1,
      "bpm": 120.0,
      "beatsPerBar": 4,
      "stepsPerBeat": 4,
      "swing": 0.0,
      "volume": 1.0,
      "tracks": [
        { "name": "Kick",    "sound": "kit:kick",         "volume": 1.0, "pan":  0.0, "mute": false },
        { "name": "Snare",   "sound": "kit:snare",        "volume": 0.9, "pan":  0.0, "mute": false },
        { "name": "Hi-Hat",  "sound": "kit:hat_closed",   "volume": 0.6, "pan":  0.2, "mute": false },
        { "name": "My Clap", "sound": "samples/clap.wav", "volume": 0.8, "pan": -0.2, "mute": false }
      ],
      "patterns": [
        { "name": "Grund", "bars": 1, "rows": [
            [1,0,0,0,  1,0,0,0,  1,0,0,0,  1,0,0,0],
            [0,0,0,0,  1,0,0,0,  0,0,0,0,  1,0,0,0],
            [1,0,0.5,0,  1,0,0.5,0,  1,0,0.5,0,  1,0,0.5,0],
            [0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0] ] },
        { "name": "Fill", "bars": 1, "rows": [
            [1,0,0,0,  1,0,0,0,  1,0,0,0,  0,0,0,0],
            [0,0,0,0,  1,0,0,0,  0,0,0.6,0.7,  0.8,0.9,1,1],
            [1,0,0.5,0,  1,0,0.5,0,  0,0,0,0,  0,0,0,0],
            [0,0,0,0,  0,0,0,0,  0,0,0,0,  1,0,0,0] ] },
        { "name": "Break", "bars": 1, "rows": [
            [1,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0],
            [0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0],
            [1,0,1,0,  1,0,1,0,  1,0,1,0,  1,0,1,0],
            [0,0,0,0,  0,0,0,0,  0,0,0,0,  0,0,0,0] ] }
      ],
      "sequence": ["Grund", "Grund", "Grund", "Fill", "Grund", "Grund", "Grund", "Break"]
    })json";
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray(text), &fehler);
    REQUIRE(fehler.error == QJsonParseError::NoError);

    Loop gelesen;
    REQUIRE(ausJson(doc.object(), gelesen));
    Loop soll = beispielLoop(QStringLiteral("samples/clap.wav"));
    soll.richte();
    CHECK(gelesen == soll);

    SUBCASE("und kommt aus dem eigenen JSON gleich wieder zurueck")
    {
        Loop nochmal;
        REQUIRE(ausJson(alsJson(gelesen), nochmal));
        CHECK(nochmal == gelesen);
    }
    SUBCASE("ein anderes Format ist kein Loop, der Loop bleibt, wie er war")
    {
        QJsonObject fremd = doc.object();
        fremd[QStringLiteral("format")] = QStringLiteral("lumiviz-composition");
        Loop bleibt = gelesen;
        bleibt.bpm = 99.0;
        CHECK(!ausJson(fremd, bleibt));
        CHECK(bleibt.bpm == 99.0);
    }
}

TEST_CASE("Loop Datei: speichern und laden, Sample-Pfade relativ zur Datei")
{
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    REQUIRE(QDir(ordner.path()).mkpath(QStringLiteral("samples")));
    const QString clap = ordner.filePath(QStringLiteral("samples/clap.wav"));
    {
        QFile f(clap);
        REQUIRE(f.open(QIODevice::WriteOnly));
    }
    Loop loop = beispielLoop(clap);
    loop.bpm = 97.5;
    loop.swing = 0.25;
    loop.spuren[1].stumm = true;
    loop.richte();
    const QString pfad = ordner.filePath(QStringLiteral("drums.lvloop"));
    REQUIRE(speichereLoop(loop, pfad));

    QFile datei(pfad);
    REQUIRE(datei.open(QIODevice::ReadOnly));
    const QByteArray inhalt = datei.readAll();
    datei.close();
    CHECK(inhalt.contains("\"samples/clap.wav\""));
    CHECK(inhalt.contains("\"kit:kick\""));
    CHECK(!inhalt.contains(ordner.path().toUtf8()));

    Loop gelesen;
    QStringList bericht;
    REQUIRE(ladeLoop(pfad, gelesen, &bericht));
    CHECK(bericht.isEmpty());
    CHECK(gelesen == loop);
    CHECK(gelesen.spuren[3].klang == clap);

    SUBCASE("fehlt das Sample, steht es im Bericht — geladen wird trotzdem")
    {
        REQUIRE(QFile::remove(clap));
        REQUIRE(ladeLoop(pfad, gelesen, &bericht));
        REQUIRE(bericht.size() == 1);
        CHECK(bericht[0].startsWith(QStringLiteral("File not found: ")));
        CHECK(gelesen == loop);
    }
    SUBCASE("eine Datei, die kein JSON ist, laedt nicht")
    {
        const QString kaputt = ordner.filePath(QStringLiteral("kaputt.lvloop"));
        QFile f(kaputt);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write("kein json");
        f.close();
        CHECK(!ladeLoop(kaputt, gelesen));
        CHECK(!ladeLoop(ordner.filePath(QStringLiteral("gibtsnicht.lvloop")), gelesen));
    }
}

TEST_CASE("Loop Datei: das Beispiel in asset/loop laedt ohne Bericht, seine Klaenge liegen im Kit")
{
    // .../projects/apps/LumiViz/tests/unit/UnitTests -> Wurzel des Repos
    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QDir ordner(wurzel.filePath(QStringLiteral("asset/loop")));
    REQUIRE_MESSAGE(ordner.exists(), ordner.path().toStdString());

    Loop loop;
    QStringList bericht;
    REQUIRE(ladeLoop(ordner.filePath(QStringLiteral("drums_demo.lvloop")), loop, &bericht));
    CHECK(bericht.isEmpty());
    CHECK(loop.spuren.size() == 6u);
    CHECK(loop.muster.size() == 3u);
    CHECK(loop.durchlaufSchritte() == 128);
    for (const Spur& spur : loop.spuren)
    {
        REQUIRE_MESSAGE(istKitKlang(spur.klang), spur.klang.toStdString());
        const QString datei = QStringLiteral("kit/%1.wav").arg(spur.klang.mid(4));
        CHECK_MESSAGE(ordner.exists(datei), datei.toStdString());
    }
    // jeder Klang des Kits kommt im Beispiel vor
    for (const QString& datei : QDir(ordner.filePath(QStringLiteral("kit"))).entryList({QStringLiteral("*.wav")}))
    {
        const QString klang = QStringLiteral("kit:") + QFileInfo(datei).completeBaseName();
        const bool benutzt = std::any_of(loop.spuren.begin(), loop.spuren.end(),
                                         [&klang](const Spur& spur) { return spur.klang == klang; });
        CHECK_MESSAGE(benutzt, klang.toStdString());
    }
}

// =============================================================================
// Rechenkern: der Ablauf
// =============================================================================

TEST_CASE("Loop Ablauf: Stellen und Lautstaerken der Schlaege")
{
    Loop loop = beispielLoop(QStringLiteral("kit:clap"));
    loop.abfolge = {QStringLiteral("Grund")};
    loop.richte();
    const Ablauf ablauf = plane(loop, kKernRate);

    CHECK(ablauf.rate == kKernRate);
    CHECK(ablauf.durchlaufFrames == 88200);
    REQUIRE(ablauf.spuren.size() == 4u);
    // Kick auf 1, 5, 9, 13
    REQUIRE(ablauf.spuren[0].size() == 4u);
    CHECK(ablauf.spuren[0][0].frame == 0);
    CHECK(ablauf.spuren[0][1].frame == 22050);
    CHECK(ablauf.spuren[0][2].frame == 44100);
    CHECK(ablauf.spuren[0][3].frame == 66150);
    CHECK(ablauf.spuren[0][0].links == 1.0f);
    CHECK(ablauf.spuren[0][0].rechts == 1.0f);
    // Snare: Spur 0.9
    REQUIRE(ablauf.spuren[1].size() == 2u);
    CHECK(ablauf.spuren[1][0].links == doctest::Approx(0.9f));
    // Hi-Hat: Spur 0.6, Panorama 0.2 nach rechts — links faellt ab, rechts bleibt
    REQUIRE(ablauf.spuren[2].size() == 8u);
    CHECK(ablauf.spuren[2][0].links == doctest::Approx(0.48f));
    CHECK(ablauf.spuren[2][0].rechts == doctest::Approx(0.6f));
    CHECK(ablauf.spuren[2][1].frame == 11025);  // Schritt 3
    CHECK(ablauf.spuren[2][1].links == doctest::Approx(0.24f));
    CHECK(ablauf.spuren[2][1].rechts == doctest::Approx(0.3f));
    // die Spur ohne Schritte
    CHECK(ablauf.spuren[3].empty());

    SUBCASE("die Lautstaerke des Loops gilt fuer alle Spuren")
    {
        loop.laut = 0.5;
        const Ablauf leiser = plane(loop, kKernRate);
        CHECK(leiser.spuren[0][0].links == 0.5f);
        CHECK(leiser.spuren[2][0].rechts == doctest::Approx(0.3f));
    }
    SUBCASE("eine stumme Spur hat keine Schlaege")
    {
        loop.spuren[0].stumm = true;
        const Ablauf ohneKick = plane(loop, kKernRate);
        CHECK(ohneKick.spuren[0].empty());
        CHECK(ohneKick.spuren[1].size() == 2u);
    }
    SUBCASE("die Abfolge reiht die Muster hintereinander")
    {
        loop.abfolge = {QStringLiteral("Break"), QStringLiteral("Gibtsnicht"), QStringLiteral("Grund")};
        const Ablauf folge = plane(loop, kKernRate);
        CHECK(folge.durchlaufFrames == 3 * 88200);
        // Kick: einmal im Break, nichts im Takt Stille, viermal im Grund
        REQUIRE(folge.spuren[0].size() == 5u);
        CHECK(folge.spuren[0][0].frame == 0);
        CHECK(folge.spuren[0][1].frame == 2 * 88200);
        CHECK(folge.spuren[0][4].frame == 2 * 88200 + 66150);
    }
    SUBCASE("das Tempo und die Abtastrate bestimmen die Stellen")
    {
        loop.bpm = 60.0;
        CHECK(plane(loop, kKernRate).spuren[0][1].frame == 44100);
        CHECK(plane(loop, 48000).spuren[0][1].frame == 48000);
        CHECK(plane(loop, 48000).durchlaufFrames == 4 * 48000);
        CHECK(plane(loop, 0).durchlaufFrames == 0);
    }
}

TEST_CASE("Loop Ablauf: Swing verzoegert jeden zweiten Schritt")
{
    Loop loop = einSchlagLoop({1, 1, 1, 1});
    {
        const Ablauf gerade = plane(loop, kKernRate);
        REQUIRE(gerade.spuren[0].size() == 4u);
        CHECK(gerade.durchlaufFrames == 22050);
        CHECK(gerade.spuren[0][1].frame == 5513);   // 5512,5 gerundet
        CHECK(gerade.spuren[0][2].frame == 11025);
        CHECK(gerade.spuren[0][3].frame == 16538);  // 16537,5 gerundet
    }
    loop.swing = 1.0;  // ein halber Schritt
    {
        const Ablauf voll = plane(loop, kKernRate);
        CHECK(voll.durchlaufFrames == 22050);
        CHECK(voll.spuren[0][0].frame == 0);
        CHECK(voll.spuren[0][1].frame == 8269);   // 1,5 Schritte = 8268,75
        CHECK(voll.spuren[0][2].frame == 11025);  // der dritte bleibt
        CHECK(voll.spuren[0][3].frame == 19294);  // 3,5 Schritte = 19293,75
    }
    loop.swing = 0.5;
    CHECK(plane(loop, kKernRate).spuren[0][1].frame == 6891);  // 1,25 Schritte = 6890,625
}

// =============================================================================
// Rechenkern: die Abtastwerte
// =============================================================================

TEST_CASE("Loop Rechenkern: jeder Schlag liegt auf dem Abtastwert, mit seiner Lautstaerke")
{
    const Loop loop = einSchlagLoop({1, 0, 0.5, 0});
    const Ablauf ablauf = plane(loop, kKernRate);
    Klang klang;
    klang.daten = {0.1f, -0.1f, 0.2f, -0.2f, 0.3f, -0.3f};  // drei Abtastwerte, rechts gespiegelt
    const std::vector<float> block = rechneBereich(ablauf, {klang}, 0, 2 * 22050);

    int klingt = 0;
    for (std::size_t i = 0; i < block.size(); i += 2)
        if (block[i] != 0.0f) ++klingt;
    CHECK(klingt == 12);  // zwei Schlaege, zwei Durchlaeufe, je drei Abtastwerte
    CHECK(linksBei(block, 0) == 0.1f);
    CHECK(block[1] == -0.1f);
    CHECK(linksBei(block, 1) == 0.2f);
    CHECK(linksBei(block, 2) == 0.3f);
    CHECK(linksBei(block, 3) == 0.0f);
    CHECK(linksBei(block, 11024) == 0.0f);
    CHECK(linksBei(block, 11025) == 0.1f * 0.5f);
    CHECK(linksBei(block, 11027) == 0.3f * 0.5f);
    // der zweite Durchlauf
    CHECK(linksBei(block, 22049) == 0.0f);
    CHECK(linksBei(block, 22050) == 0.1f);
    CHECK(linksBei(block, 22050 + 11026) == 0.2f * 0.5f);
}

TEST_CASE("Loop Rechenkern: dieselben Werte, gleich in welchen Bloecken und von wo gerechnet wird")
{
    Loop loop = beispielLoop(QStringLiteral("kit:clap"));
    loop.abfolge = {QStringLiteral("Grund"), QStringLiteral("Fill")};
    loop.swing = 0.3;
    loop.richte();
    const Ablauf ablauf = plane(loop, kKernRate);
    REQUIRE(ablauf.durchlaufFrames == 2 * 88200);
    // Klaenge verschieden lang: kuerzer und laenger als der Abstand der Schlaege
    const std::vector<Klang> klaenge = {bunterKlang(3000, 1), bunterKlang(9000, 2), bunterKlang(700, 3),
                                        bunterKlang(20000, 4)};
    const int gesamt = 3 * 2 * 88200;  // drei Durchlaeufe
    const std::vector<float> amStueck = rechneBereich(ablauf, klaenge, 0, gesamt);

    SUBCASE("in ungleichen Bloecken")
    {
        std::vector<float> gestueckelt(amStueck.size(), 9.0f);
        const int groessen[] = {1, 7, 441, 220, 8167, 3, 1024};
        std::int64_t stelle = 0;
        for (int i = 0; stelle < gesamt; ++i)
        {
            const int frames = static_cast<int>(std::min<std::int64_t>(groessen[i % 7], gesamt - stelle));
            rechne(ablauf, klaenge, stelle, frames, gestueckelt.data() + 2 * stelle);
            stelle += frames;
        }
        CHECK(std::memcmp(gestueckelt.data(), amStueck.data(), amStueck.size() * sizeof(float)) == 0);
    }
    SUBCASE("mitten hinein, auch ueber die Grenze eines Durchlaufs")
    {
        for (const std::int64_t von : {std::int64_t{200000}, std::int64_t{2 * 88200 - 2500}, std::int64_t{88200 + 5513}})
        {
            const std::vector<float> teil = rechneBereich(ablauf, klaenge, von, 5000);
            CHECK(std::memcmp(teil.data(), amStueck.data() + 2 * von, teil.size() * sizeof(float)) == 0);
        }
    }
    SUBCASE("vor dem Anfang ist Stille")
    {
        const std::vector<float> teil = rechneBereich(ablauf, klaenge, -100, 300);
        for (std::size_t i = 0; i < 200; ++i) CHECK(teil[i] == 0.0f);
        CHECK(std::memcmp(teil.data() + 200, amStueck.data(), 400 * sizeof(float)) == 0);
        const std::vector<float> ganzDavor = rechneBereich(ablauf, klaenge, -500, 100);
        CHECK(std::all_of(ganzDavor.begin(), ganzDavor.end(), [](float w) { return w == 0.0f; }));
    }
    SUBCASE("es klingt ueberhaupt etwas")
    {
        CHECK(std::any_of(amStueck.begin(), amStueck.end(), [](float w) { return w != 0.0f; }));
    }
}

TEST_CASE("Loop Rechenkern: ein neuer Schlag schneidet den klingenden derselben Spur ab")
{
    // zwei Schlaege im Abstand von 5513 Abtastwerten, der Klang ist 10000 lang
    const Ablauf ablauf = plane(einSchlagLoop({1, 1, 0, 0}), kKernRate);
    const std::vector<float> block = rechneBereich(ablauf, {gleichKlang(10000, 1.0f)}, 0, 22050);

    CHECK(linksBei(block, 5512) == 1.0f);                       // der erste allein
    CHECK(linksBei(block, 5513) == 2.0f);                       // der zweite setzt ein, der erste noch voll
    CHECK(linksBei(block, 5513 + 32) == 1.5f);                  // der erste auf halbem Weg
    CHECK(linksBei(block, 5513 + 63) == 1.0f + 1.0f / 64.0f);
    CHECK(linksBei(block, 5513 + kSchnittFrames) == 1.0f);      // der erste ist weg
    CHECK(linksBei(block, 9999) == 1.0f);
    CHECK(linksBei(block, 10000) == 1.0f);                      // ohne Schnitt endete der erste hier
    CHECK(linksBei(block, 5513 + 9999) == 1.0f);                // der zweite klingt aus
    CHECK(linksBei(block, 5513 + 10000) == 0.0f);
}

TEST_CASE("Loop Rechenkern: der Ausklang reicht in den naechsten Durchlauf, vor dem ersten gibt es keinen")
{
    // ein Schlag auf dem letzten Schritt (16538), der Klang ist 10000 lang
    const Ablauf ablauf = plane(einSchlagLoop({0, 0, 0, 1}), kKernRate);
    const std::vector<float> block = rechneBereich(ablauf, {gleichKlang(10000, 1.0f)}, 0, 2 * 22050);

    CHECK(linksBei(block, 0) == 0.0f);  // kein Ausklang eines Durchlaufs davor
    CHECK(linksBei(block, 16537) == 0.0f);
    CHECK(linksBei(block, 16538) == 1.0f);
    CHECK(linksBei(block, 22050) == 1.0f);  // ueber die Grenze hinweg
    CHECK(linksBei(block, 26537) == 1.0f);
    CHECK(linksBei(block, 26538) == 0.0f);
    CHECK(linksBei(block, 22050 + 16538) == 1.0f);
}

TEST_CASE("Loop Rechenkern: wo nichts zu spielen ist, ist Stille")
{
    const auto still = [](const std::vector<float>& block) {
        return std::all_of(block.begin(), block.end(), [](float w) { return w == 0.0f; });
    };
    SUBCASE("ein leerer Loop")
    {
        Loop leer;
        leer.richte();
        const Ablauf ablauf = plane(leer, kKernRate);
        CHECK(ablauf.durchlaufFrames == 88200);
        CHECK(still(rechneBereich(ablauf, {}, 0, 1000)));
    }
    SUBCASE("eine Spur ohne Klang")
    {
        const Ablauf ablauf = plane(einSchlagLoop({1, 1, 1, 1}), kKernRate);
        CHECK(still(rechneBereich(ablauf, {}, 0, 1000)));
        CHECK(still(rechneBereich(ablauf, {Klang{}}, 0, 1000)));
    }
    SUBCASE("ein Ablauf ohne Rate")
    {
        const Ablauf ablauf = plane(einSchlagLoop({1, 1, 1, 1}), 0);
        CHECK(still(rechneBereich(ablauf, {gleichKlang(100, 1.0f)}, 0, 1000)));
    }
}
