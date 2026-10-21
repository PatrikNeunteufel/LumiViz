/**
 ****************************************************************************************
 * @file   test_LoopKern.cpp
 * @brief  Loop: Modell, Datei `.lvloop` und Rechenkern
 *         (docs/audio/Loop_Konzept.md §4)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Fenster. Die Faelle zum Rechenkern zaehlen die gerechneten
 * Abtastwerte nach — bei 44100 Hz, 120 BPM und vier Schritten je Schlag ist
 * ein Schritt 5512,5 Abtastwerte lang.
 *
 * Die kleinen Loops hier haben einen Schlag je Takt: ein Muster von vier
 * Takten (die kuerzeste Laenge) hat dann 16 Schritte und dauert 88200
 * Abtastwerte.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
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
constexpr std::int64_t kKlein = 88200;  ///< ein Durchlauf des kleinen Loops

/// Ein Loop mit einem Kanal, einem Clip und einem Muster von vier Takten zu
/// je einem Schlag (16 Schritte), mit EINER Spur. `zeile` nennt die ersten
/// Schritte, der Rest ist aus.
Loop kleinLoop(std::vector<double> zeile, const QString& klang = QStringLiteral("a.wav"))
{
    Loop loop;
    loop.laut = 1.0;
    loop.schlaegeJeTakt = 1;
    Muster m;
    m.name = QStringLiteral("M");
    m.spuren = {{QStringLiteral("A"), klang, 1.0, 0.0, false}};
    m.zeilen = {std::move(zeile)};
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("M"), 0, 1}}}};
    loop.richte();
    return loop;
}

/// Ein Drumkit im Vier-Viertel-Takt: vier Spuren, zwei davon mit demselben Klang.
Loop drumLoop()
{
    Loop loop;
    loop.laut = 1.0;
    Muster m;
    m.name = QStringLiteral("Drums");
    m.spuren = {
        {QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
        {QStringLiteral("Snare"), QStringLiteral("kit:snare"), 0.9, 0.0, false},
        {QStringLiteral("Hi-Hat"), QStringLiteral("kit:hat_closed"), 0.6, 0.2, false},
        {QStringLiteral("Kick 2"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
    };
    m.zeilen = {
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {1, 0, 0.5, 0},
        std::vector<double>(64, 0.0),
    };
    m.zeilen[3][63] = 1.0;  // der letzte Schritt des Musters
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("Drums"), 0, 1}}}};
    loop.richte();
    return loop;
}

/// Der Loop aus der Datei im Fall „wird so gelesen, wie es dort steht".
Loop dateiLoop(const QString& clapDatei)
{
    Loop loop;
    loop.bpm = 97.5;
    loop.schlaegeJeTakt = 1;
    loop.swing = 0.25;
    loop.laut = 0.8;
    Muster m;
    m.name = QStringLiteral("A");
    m.spuren = {
        {QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
        {QStringLiteral("My Clap"), clapDatei, 0.8, -0.2, true},
    };
    m.zeilen = {
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.5, 0},
        {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0},
    };
    loop.muster = {m};
    loop.kanaele = {
        {QStringLiteral("Drums"), true, {{QStringLiteral("A"), 0, 2}, {QStringLiteral("A"), 12, 1}}},
        {QStringLiteral("Off"), false, {}},
    };
    loop.richte();
    return loop;
}

const char* const kDateiText = R"json({
  "format": "lumiviz-loop",
  "version": 2,
  "bpm": 97.5,
  "beatsPerBar": 1,
  "stepsPerBeat": 4,
  "swing": 0.25,
  "volume": 0.8,
  "patterns": [
    {
      "name": "A",
      "kind": "drumkit",
      "bars": 4,
      "tracks": [
        { "name": "Kick",    "sound": "kit:kick",         "volume": 1.0, "pan":  0.0, "mute": false },
        { "name": "My Clap", "sound": "samples/clap.wav", "volume": 0.8, "pan": -0.2, "mute": true }
      ],
      "rows": [
        [1, 0, 0, 0,   1, 0, 0, 0,   1, 0, 0, 0,   1, 0, 0.5, 0],
        [0, 0, 1, 0,   0, 0, 1, 0,   0, 0, 1, 0,   0, 0, 1, 0]
      ]
    }
  ],
  "channels": [
    { "name": "Drums", "enabled": true, "clips": [
        { "pattern": "A", "start": 0,  "repeat": 2 },
        { "pattern": "A", "start": 12, "repeat": 1 } ] },
    { "name": "Off", "enabled": false, "clips": [] }
  ]
})json";

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

/// Klaenge als Werte geteilt, wie der Rechenkern sie nimmt (Stufe 29b): ein leerer wird `nullptr`.
Klaenge geteilt(const std::vector<Klang>& werte)
{
    Klaenge klaenge;
    for (const Klang& klang : werte) klaenge.push_back(klang.daten.empty() ? nullptr : std::make_shared<const Klang>(klang));
    return klaenge;
}

std::vector<float> rechneBereich(const Ablauf& ablauf, const std::vector<Klang>& klaenge, std::int64_t von,
                                 int frames)
{
    // mit Unsinn vorbelegt: `rechne` muss den Block ganz ueberschreiben
    std::vector<float> aus(static_cast<std::size_t>(frames) * 2, 9.0f);
    rechne(ablauf, geteilt(klaenge), von, frames, aus.data());
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

TEST_CASE("Loop Modell: Kanaele, Clips und die Laenge eines Durchlaufs")
{
    Loop loop = dateiLoop(QStringLiteral("kit:clap"));
    CHECK(loop.schritteJeTakt() == 4);
    REQUIRE(loop.kanaele.size() == 2u);
    REQUIRE(loop.kanaele[0].clips.size() == 2u);
    // ein Clip ist immer ein Vielfaches seines Musters lang
    CHECK(loop.clipTakte(loop.kanaele[0].clips[0]) == 8);
    CHECK(loop.clipTakte(loop.kanaele[0].clips[1]) == 4);
    // der Durchlauf endet mit dem letzten Clip; die Luecke davor ist Stille
    CHECK(loop.durchlaufTakte() == 16);
    CHECK(loop.durchlaufSchritte() == 64);
    CHECK(loop.durchlaufMs() == doctest::Approx(64 * 60000.0 / 97.5 / 4));
    CHECK(loop.klangNamen() == QStringList{QStringLiteral("kit:kick"), QStringLiteral("kit:clap")});

    SUBCASE("ein Kanal, der aus ist, zaehlt fuer die Laenge mit")
    {
        loop.kanaele[1].clips = {{QStringLiteral("A"), 16, 1}};
        CHECK(loop.durchlaufTakte() == 20);
    }
    SUBCASE("ein leerer Loop ist ein Takt Stille")
    {
        Loop leer;
        leer.richte();
        CHECK(leer.durchlaufTakte() == 1);
        CHECK(leer.durchlaufSchritte() == 16);
        CHECK(leer.durchlaufMs() == doctest::Approx(2000.0));
        CHECK(leer.klangNamen().isEmpty());
        CHECK(leer.laut == 0.5);  // die Vorgabe laesst Luft nach oben
    }
    SUBCASE("jeder Klang einmal, Spuren ohne Klang zaehlen nicht")
    {
        Muster zwei = loop.muster[0];
        zwei.name = QStringLiteral("B");
        zwei.spuren[0].klang.clear();
        zwei.spuren.push_back({QStringLiteral("Tom"), QStringLiteral("kit:tom"), 1.0, 0.0, false});
        loop.muster.push_back(zwei);
        CHECK(loop.klangNamen() == QStringList{QStringLiteral("kit:kick"), QStringLiteral("kit:clap"),
                                               QStringLiteral("kit:tom")});
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
    Muster ohneName;
    ohneName.takte = 0;  // kuerzer als die kuerzeste Laenge (seit S124, B1 a: 1 Takt)
    ohneName.spuren = {{QStringLiteral("A"), QStringLiteral("kit:a"), 7.0, -3.0, false},
                       {QStringLiteral("B"), QStringLiteral("kit:b"), 1.0, 0.0, false}};
    ohneName.zeilen = {{3.0, -1.0, 0.5}};  // eine Zeile zu wenig, zu kurz, Werte ausserhalb
    Muster doppelt;
    doppelt.name = QStringLiteral("Pattern 1");
    doppelt.takte = 999;
    loop.muster = {ohneName, doppelt};
    loop.kanaele = {{QStringLiteral("K"),
                     true,
                     {{QStringLiteral("Pattern 1"), 6, 0},      // spaeter, ohne Wiederholung
                      {QStringLiteral("Gibtsnicht"), 2, 1},     // ohne Muster
                      {QStringLiteral("Pattern 1"), -3, 2}}}};  // vor dem Anfang, ueberlappt den ersten
    QStringList bericht;
    loop.richte(&bericht);

    CHECK(loop.bpm == kBpmMin);
    CHECK(loop.schlaegeJeTakt == 1);
    CHECK(loop.schritteJeSchlag == kRasterMax);
    CHECK(loop.swing == 1.0);
    CHECK(loop.laut == 0.0);
    CHECK(loop.muster[0].name == QStringLiteral("Pattern 1"));
    CHECK(loop.muster[0].takte == kTakteMin);
    CHECK(loop.muster[1].takte == kTakteMax);
    CHECK(loop.muster[0].spuren[0].laut == 1.0);
    CHECK(loop.muster[0].spuren[0].pan == -1.0);
    REQUIRE(loop.muster[0].zeilen.size() == 2u);
    REQUIRE(loop.muster[0].zeilen[0].size() == static_cast<std::size_t>(kTakteMin * kRasterMax));
    CHECK(loop.muster[0].zeilen[0][0] == 1.0);
    CHECK(loop.muster[0].zeilen[0][1] == 0.0);
    CHECK(loop.muster[0].zeilen[0][2] == 0.5);
    CHECK(loop.muster[0].zeilen[0][3] == 0.0);
    CHECK(loop.muster[0].zeilen[1].size() == loop.muster[0].zeilen[0].size());

    // der Clip ohne Muster ist weg; die beiden anderen: sortiert, ab Takt 0,
    // der fruehere endet nach 2 x 1 Takt, der spaetere bleibt bei Takt 6 (bis S124, mit 4 Takten, rueckte er
    // auf 8 — dass ein ueberlappender Clip nach hinten rueckt, prueft test_LoopDrumEditor.cpp, „setzeMusterTakte")
    REQUIRE(loop.kanaele[0].clips.size() == 2u);
    CHECK(loop.kanaele[0].clips[0] == LoopClip{QStringLiteral("Pattern 1"), 0, 2});
    CHECK(loop.kanaele[0].clips[1] == LoopClip{QStringLiteral("Pattern 1"), 6, 1});
    REQUIRE(bericht.size() == 2);
    CHECK(bericht[0] == QStringLiteral("Pattern name used twice: Pattern 1"));
    CHECK(bericht[1] == QStringLiteral("Pattern not found: Gibtsnicht"));

    loop.bpm = std::numeric_limits<double>::quiet_NaN();
    loop.richte();
    CHECK(loop.bpm == 120.0);
}

TEST_CASE("Loop Datei: eine Datei wird so gelesen, wie sie dasteht")
{
    QJsonParseError fehler;
    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray(kDateiText), &fehler);
    REQUIRE(fehler.error == QJsonParseError::NoError);

    Loop gelesen;
    REQUIRE(ausJson(doc.object(), gelesen));
    CHECK(gelesen == dateiLoop(QStringLiteral("samples/clap.wav")));

    SUBCASE("und kommt aus dem eigenen JSON gleich wieder zurueck")
    {
        Loop nochmal;
        REQUIRE(ausJson(alsJson(gelesen), nochmal));
        CHECK(nochmal == gelesen);
    }
    SUBCASE("ohne das Feld volume gilt die Vorgabe eines neuen Loops: Luft nach oben")
    {
        QJsonObject ohne = doc.object();
        ohne.remove(QStringLiteral("volume"));
        Loop leiser;
        REQUIRE(ausJson(ohne, leiser));
        CHECK(leiser.laut == 0.5);
    }
    SUBCASE("ein anderes Format oder die Fassung 1 ist kein Loop, der Loop bleibt, wie er war")
    {
        Loop bleibt = gelesen;
        bleibt.bpm = 99.0;
        QJsonObject fremd = doc.object();
        fremd[QStringLiteral("format")] = QStringLiteral("lumiviz-composition");
        CHECK(!ausJson(fremd, bleibt));
        QJsonObject alt = doc.object();
        alt[QStringLiteral("version")] = 1;
        CHECK(!ausJson(alt, bleibt));
        CHECK(bleibt.bpm == 99.0);
    }
    SUBCASE("ein Muster unbekannter Art bleibt weg, seine Clips auch — beides steht im Bericht")
    {
        QJsonObject neuer = doc.object();
        QJsonArray muster = neuer.value(QStringLiteral("patterns")).toArray();
        QJsonObject erstes = muster[0].toObject();
        // `instrument` kennt das Programm seit S110 (Stufe 9a) — eine Art, die es nicht gibt
        erstes[QStringLiteral("kind")] = QStringLiteral("vocals");
        muster[0] = erstes;
        neuer[QStringLiteral("patterns")] = muster;
        Loop ohne;
        QStringList bericht;
        REQUIRE(ausJson(neuer, ohne, {}, &bericht));
        CHECK(ohne.muster.empty());
        CHECK(ohne.kanaele[0].clips.empty());
        REQUIRE(bericht.size() >= 2);
        CHECK(bericht[0] == QStringLiteral("Unknown pattern kind 'vocals': A"));
        CHECK(bericht.contains(QStringLiteral("Pattern not found: A")));
    }
}

TEST_CASE("Loop Datei: die Effekte am Clip stehen in Fassung 3, Fassung 2 wird weiter gelesen")
{
    // Entscheide Patrik S113: E2 b (jeder Clip eigene Effekte), F2 a (Feld
    // `effects` am Clip, Fassung 3)
    Loop loop = dateiLoop(QStringLiteral("samples/clap.wav"));
    REQUIRE(loop.kanaele[0].clips.size() == 2u);
    CHECK(!loop.kanaele[0].clips[0].effekte.einerAn());
    // das Sample des Loops gibt es hier nicht — das steht im Bericht und
    // gehoert nicht zur Frage; alles andere darin waere ein Befund
    const auto nurFehlendeDateien = [](const QStringList& bericht) {
        return std::all_of(bericht.begin(), bericht.end(),
                           [](const QString& zeile) { return zeile.startsWith(QStringLiteral("File not found: ")); });
    };

    SUBCASE("ein Clip ohne Effekte traegt das Feld nicht; geschrieben wird Fassung 3")
    {
        const QJsonObject json = alsJson(loop);
        CHECK(json.value(QStringLiteral("version")).toInt() == 12);  // seit S140: die Spuren ueber der Zeit (12, test_LoopZeitspur.cpp); seit S139: der Fader je Kanal (11, test_LoopKern.cpp); seit S136: Legato, Slide, Bend (10, test_LoopSpielweise.cpp); S134: Strum, Anschlaege, Artikulationen (9, test_LoopSpielweise.cpp); S132: die Saite einer Note und String choice (8, test_LoopSaiten.cpp); S124: fxOn am Clip (E1 a, 7), Muster ab 1 Takt (B1 a, 6); S118: die Reihe `path` am Clip (test_LoopBausteine.cpp); S115: Drum-Spuren mit eigenem Sample oder Instrument (4)
        const QJsonObject clip =
            json.value(QStringLiteral("channels")).toArray()[0].toObject().value(QStringLiteral("clips")).toArray()[0].toObject();
        CHECK(!clip.contains(QStringLiteral("effects")));
        CHECK(!clip.contains(QStringLiteral("path")));
    }
    SUBCASE("die Effekte eines Clips kommen zurueck, der Nachbar derselben Basis bleibt ohne")
    {
        ClipEffekte& e = loop.kanaele[0].clips[1].effekte;
        e.hole<ClipVerzerrung>() = {true, 0.7, 0.2, 0.5};
        e.hole<ClipEcho>() = {true, EchoZeit::Viertel, 0.6, 0.1, 0.8};
        e.hole<ClipHall>().sekunden = 3.5;  // aus, aber eingestellt: der Wert bleibt
        CHECK(loop.kanaele[0].clips[1].effekte.einerAn());

        const QJsonObject json = alsJson(loop);
        const QJsonArray clips =
            json.value(QStringLiteral("channels")).toArray()[0].toObject().value(QStringLiteral("clips")).toArray();
        CHECK(!clips[0].toObject().contains(QStringLiteral("path")));
        // seit Fassung 5 die Reihe `path`: Distortion, Echo, Reverb — in der Folge, in der sie dazukamen
        const QJsonArray reihe = clips[1].toObject().value(QStringLiteral("path")).toArray();
        REQUIRE(reihe.size() == 3);
        CHECK(reihe[0].toObject().value(QStringLiteral("type")).toString() == QStringLiteral("distortion"));
        CHECK(reihe[1].toObject().value(QStringLiteral("type")).toString() == QStringLiteral("echo"));
        CHECK(reihe[1].toObject().value(QStringLiteral("time")).toString() == QStringLiteral("1/4"));
        CHECK(reihe[2].toObject().value(QStringLiteral("type")).toString() == QStringLiteral("reverb"));
        CHECK(reihe[2].toObject().value(QStringLiteral("on")).toBool(true) == false);

        Loop nochmal;
        QStringList bericht;
        REQUIRE(ausJson(json, nochmal, {}, &bericht));
        CHECK(nurFehlendeDateien(bericht));
        CHECK(nochmal == loop);
        CHECK(nochmal.kanaele[0].clips[0].effekte == ClipEffekte{});
        CHECK(nochmal.kanaele[0].clips[1].effekte.wert<ClipHall>().sekunden == 3.5);
    }
    SUBCASE("eine Datei der Fassung 2 wird gelesen, ohne Bericht und ohne Effekte")
    {
        QJsonObject alt = alsJson(loop);
        alt[QStringLiteral("version")] = 2;
        Loop gelesen;
        QStringList bericht;
        REQUIRE(ausJson(alt, gelesen, {}, &bericht));
        CHECK(nurFehlendeDateien(bericht));
        CHECK(gelesen == loop);
    }
    SUBCASE("eine neuere Fassung wird gelesen und gemeldet")
    {
        QJsonObject morgen = alsJson(loop);
        morgen[QStringLiteral("version")] = morgen.value(QStringLiteral("version")).toInt() + 1;  // eine Fassung von morgen
        Loop gelesen;
        QStringList bericht;
        REQUIRE(ausJson(morgen, gelesen, {}, &bericht));
        CHECK(bericht.contains(QStringLiteral("File was written by a newer version")));
    }
    SUBCASE("Werte ausserhalb kommen in ihre Grenzen, eine unbekannte Echo-Zeit steht im Bericht")
    {
        ClipEffekte& e = loop.kanaele[0].clips[0].effekte;
        e.hole<ClipEcho>().an = true;
        e.hole<ClipEcho>().feedback = 5.0;
        e.hole<ClipHall>().sekunden = 100.0;
        e.hole<ClipVerzerrung>().mix = -1.0;
        loop.richte();
        CHECK(loop.kanaele[0].clips[0].effekte.wert<ClipEcho>().feedback == kEchoFeedbackMax);
        CHECK(loop.kanaele[0].clips[0].effekte.wert<ClipHall>().sekunden == kHallSekMax);
        CHECK(loop.kanaele[0].clips[0].effekte.wert<ClipVerzerrung>().mix == 0.0);

        QJsonObject json = alsJson(loop);
        QJsonArray kanaele = json.value(QStringLiteral("channels")).toArray();
        QJsonObject kanal = kanaele[0].toObject();
        QJsonArray clips = kanal.value(QStringLiteral("clips")).toArray();
        QJsonObject clip = clips[0].toObject();
        QJsonArray reihe = clip.value(QStringLiteral("path")).toArray();
        REQUIRE(reihe.size() == 3);  // Distortion, Echo, Reverb
        QJsonObject echo = reihe[1].toObject();
        REQUIRE(echo.value(QStringLiteral("type")).toString() == QStringLiteral("echo"));
        echo[QStringLiteral("time")] = QStringLiteral("1/3");
        reihe[1] = echo;
        clip[QStringLiteral("path")] = reihe;
        clips[0] = clip;
        kanal[QStringLiteral("clips")] = clips;
        kanaele[0] = kanal;
        json[QStringLiteral("channels")] = kanaele;
        Loop gelesen;
        QStringList bericht;
        REQUIRE(ausJson(json, gelesen, {}, &bericht));
        CHECK(bericht.contains(QStringLiteral("Unknown echo time '1/3'")));
        CHECK(gelesen.kanaele[0].clips[0].effekte.wert<ClipEcho>().zeit == EchoZeit::AchtelPunktiert);
    }
    SUBCASE("die Echo-Zeit in Schlaegen")
    {
        CHECK(echoSchlaege(EchoZeit::Sechzehntel) == 0.25);
        CHECK(echoSchlaege(EchoZeit::AchtelPunktiert) == 0.75);
        CHECK(echoSchlaege(EchoZeit::Halbe) == 2.0);
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
    const Loop loop = dateiLoop(clap);
    const QString pfad = ordner.filePath(QStringLiteral("drums.lvloop"));
    REQUIRE(speichereLoop(loop, pfad));

    QFile datei(pfad);
    REQUIRE(datei.open(QIODevice::ReadOnly));
    const QByteArray inhalt = datei.readAll();
    datei.close();
    CHECK(inhalt.contains("\"samples/clap.wav\""));
    CHECK(inhalt.contains("\"kit:kick\""));
    CHECK(!inhalt.contains(ordner.path().toUtf8()));
    // die Schritte einer Spur stehen auf EINER Zeile
    CHECK(inhalt.contains("[1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0.5, 0]"));

    Loop gelesen;
    QStringList bericht;
    REQUIRE(ladeLoop(pfad, gelesen, &bericht));
    CHECK(bericht.isEmpty());
    CHECK(gelesen == loop);
    CHECK(gelesen.muster[0].spuren[1].klang == clap);

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
    CHECK(loop.muster.size() == 4u);
    CHECK(loop.kanaele.size() == 2u);
    CHECK(loop.durchlaufTakte() == 16);
    const QStringList namen = loop.klangNamen();
    CHECK(namen.size() == 6);
    for (const QString& klang : namen)
    {
        REQUIRE_MESSAGE(istKitKlang(klang), klang.toStdString());
        const QString datei = QStringLiteral("kit/%1.wav").arg(klang.mid(4));
        CHECK_MESSAGE(ordner.exists(datei), datei.toStdString());
    }
    // jeder Klang des Kits kommt im Beispiel vor
    for (const QString& datei : QDir(ordner.filePath(QStringLiteral("kit"))).entryList({QStringLiteral("*.wav")}))
    {
        const QString klang = QStringLiteral("kit:") + QFileInfo(datei).completeBaseName();
        CHECK_MESSAGE(namen.contains(klang), klang.toStdString());
    }
}

// =============================================================================
// Rechenkern: der Ablauf
// =============================================================================

TEST_CASE("Loop Ablauf: Stimmen, Stellen und Lautstaerken der Schlaege")
{
    Loop loop = drumLoop();
    const Ablauf ablauf = plane(loop, kKernRate);

    CHECK(ablauf.rate == kKernRate);
    CHECK(ablauf.schrittFrames == doctest::Approx(5512.5));
    CHECK(ablauf.durchlaufFrames == 352800);  // vier Takte zu 16 Schritten
    CHECK(ablauf.klangNamen == QStringList{QStringLiteral("kit:kick"), QStringLiteral("kit:snare"),
                                           QStringLiteral("kit:hat_closed")});
    // je Spur eine Stimme; zwei Spuren teilen sich den Klang, nicht die Stimme
    REQUIRE(ablauf.stimmen.size() == 4u);
    CHECK(ablauf.stimmen[0].klang == 0);
    CHECK(ablauf.stimmen[1].klang == 1);
    CHECK(ablauf.stimmen[2].klang == 2);
    CHECK(ablauf.stimmen[3].klang == 0);
    // Kick auf 1, 5, 9, 13 des ersten Takts
    const std::vector<Schlag>& kick = ablauf.stimmen[0].schlaege;
    REQUIRE(kick.size() == 4u);
    CHECK(kick[0].frame == 0);
    CHECK(kick[1].frame == 22050);
    CHECK(kick[2].frame == 44100);
    CHECK(kick[3].frame == 66150);
    CHECK(kick[0].links == 1.0f);
    CHECK(kick[0].rechts == 1.0f);
    // Snare: Spur 0.9
    REQUIRE(ablauf.stimmen[1].schlaege.size() == 2u);
    CHECK(ablauf.stimmen[1].schlaege[0].links == doctest::Approx(0.9f));
    // Hi-Hat: Spur 0.6, Panorama 0.2 nach rechts — links faellt ab, rechts bleibt
    const std::vector<Schlag>& hat = ablauf.stimmen[2].schlaege;
    REQUIRE(hat.size() == 2u);
    CHECK(hat[0].links == doctest::Approx(0.48f));
    CHECK(hat[0].rechts == doctest::Approx(0.6f));
    CHECK(hat[1].frame == 11025);  // Schritt 3
    CHECK(hat[1].links == doctest::Approx(0.24f));
    CHECK(hat[1].rechts == doctest::Approx(0.3f));
    // der letzte Schritt des Musters: 63 x 5512,5 = 347287,5
    REQUIRE(ablauf.stimmen[3].schlaege.size() == 1u);
    CHECK(ablauf.stimmen[3].schlaege[0].frame == 347288);

    SUBCASE("die Lautstaerke des Loops gilt fuer alle Spuren")
    {
        loop.laut = 0.5;
        const Ablauf leiser = plane(loop, kKernRate);
        CHECK(leiser.stimmen[0].schlaege[0].links == 0.5f);
        CHECK(leiser.stimmen[2].schlaege[0].rechts == doctest::Approx(0.3f));
    }
    SUBCASE("eine stumme Spur hat keine Stimme")
    {
        loop.muster[0].spuren[0].stumm = true;
        const Ablauf ohneKick = plane(loop, kKernRate);
        REQUIRE(ohneKick.stimmen.size() == 3u);
        CHECK(ohneKick.stimmen[0].klang == 1);
        CHECK(ohneKick.durchlaufFrames == 352800);
    }
    SUBCASE("ein Kanal, der aus ist, hat keine Stimmen — der Durchlauf bleibt gleich lang")
    {
        loop.kanaele[0].an = false;
        const Ablauf still = plane(loop, kKernRate);
        CHECK(still.stimmen.empty());
        CHECK(still.durchlaufFrames == 352800);
    }
    SUBCASE("ein Clip wiederholt sein Muster")
    {
        loop.kanaele[0].clips[0].wiederholungen = 2;
        const Ablauf doppelt = plane(loop, kKernRate);
        CHECK(doppelt.durchlaufFrames == 2 * 352800);
        REQUIRE(doppelt.stimmen.size() == 4u);  // dieselben Stimmen, mehr Schlaege
        REQUIRE(doppelt.stimmen[0].schlaege.size() == 8u);
        CHECK(doppelt.stimmen[0].schlaege[4].frame == 352800);
        CHECK(doppelt.stimmen[0].schlaege[7].frame == 352800 + 66150);
    }
    SUBCASE("ein Clip beginnt an seinem Takt, davor ist Stille")
    {
        loop.kanaele[0].clips[0].startTakt = 4;
        const Ablauf spaeter = plane(loop, kKernRate);
        CHECK(spaeter.durchlaufFrames == 2 * 352800);
        CHECK(spaeter.stimmen[0].schlaege[0].frame == 352800);
    }
    SUBCASE("zwei Clips desselben Musters im Kanal sind dieselben Stimmen")
    {
        loop.kanaele[0].clips.push_back({QStringLiteral("Drums"), 8, 1});
        const Ablauf zwei = plane(loop, kKernRate);
        CHECK(zwei.durchlaufFrames == 3 * 352800);
        REQUIRE(zwei.stimmen.size() == 4u);
        REQUIRE(zwei.stimmen[0].schlaege.size() == 8u);
        CHECK(zwei.stimmen[0].schlaege[4].frame == 2 * 352800);
    }
    SUBCASE("dasselbe Muster in einem zweiten Kanal sind eigene Stimmen")
    {
        loop.kanaele.push_back({QStringLiteral("K2"), true, {{QStringLiteral("Drums"), 0, 1}}});
        const Ablauf zwei = plane(loop, kKernRate);
        CHECK(zwei.stimmen.size() == 8u);
        CHECK(zwei.durchlaufFrames == 352800);
    }
    SUBCASE("das Tempo und die Abtastrate bestimmen die Stellen")
    {
        loop.bpm = 60.0;
        CHECK(plane(loop, kKernRate).stimmen[0].schlaege[1].frame == 44100);
        CHECK(plane(loop, 48000).stimmen[0].schlaege[1].frame == 48000);
        CHECK(plane(loop, 48000).durchlaufFrames == 16 * 48000);
        CHECK(plane(loop, 0).durchlaufFrames == 0);
        CHECK(plane(loop, 0).stimmen.empty());
    }
    SUBCASE("eine Spur ohne Klang hat eine Stimme, die schweigt")
    {
        loop.muster[0].spuren[1].klang.clear();
        const Ablauf ohne = plane(loop, kKernRate);
        CHECK(ohne.klangNamen.size() == 2);
        CHECK(ohne.stimmen[1].klang == -1);
    }
}

TEST_CASE("Loop Ablauf: Swing verzoegert jeden zweiten Schritt")
{
    Loop loop = kleinLoop({1, 1, 1, 1});
    {
        const Ablauf gerade = plane(loop, kKernRate);
        REQUIRE(gerade.stimmen.size() == 1u);
        const std::vector<Schlag>& s = gerade.stimmen[0].schlaege;
        REQUIRE(s.size() == 4u);
        CHECK(gerade.durchlaufFrames == kKlein);
        CHECK(s[1].frame == 5513);   // 5512,5 gerundet
        CHECK(s[2].frame == 11025);
        CHECK(s[3].frame == 16538);  // 16537,5 gerundet
    }
    loop.swing = 1.0;  // ein halber Schritt
    {
        const Ablauf voll = plane(loop, kKernRate);
        const std::vector<Schlag>& s = voll.stimmen[0].schlaege;
        CHECK(voll.durchlaufFrames == kKlein);
        CHECK(s[0].frame == 0);
        CHECK(s[1].frame == 8269);   // 1,5 Schritte = 8268,75
        CHECK(s[2].frame == 11025);  // der dritte bleibt
        CHECK(s[3].frame == 19294);  // 3,5 Schritte = 19293,75
    }
    loop.swing = 0.5;
    CHECK(plane(loop, kKernRate).stimmen[0].schlaege[1].frame == 6891);  // 1,25 Schritte = 6890,625
}

// =============================================================================
// Rechenkern: die Abtastwerte
// =============================================================================

TEST_CASE("Loop Rechenkern: jeder Schlag liegt auf dem Abtastwert, mit seiner Lautstaerke")
{
    const Ablauf ablauf = plane(kleinLoop({1, 0, 0.5, 0}), kKernRate);
    Klang klang;
    klang.daten = {0.1f, -0.1f, 0.2f, -0.2f, 0.3f, -0.3f};  // drei Abtastwerte, rechts gespiegelt
    const std::vector<float> block = rechneBereich(ablauf, {klang}, 0, static_cast<int>(2 * kKlein));

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
    CHECK(linksBei(block, kKlein - 1) == 0.0f);
    CHECK(linksBei(block, kKlein) == 0.1f);
    CHECK(linksBei(block, kKlein + 11026) == 0.2f * 0.5f);
}

TEST_CASE("Loop Rechenkern: dieselben Werte, gleich in welchen Bloecken und von wo gerechnet wird")
{
    // zwei Kanaele, zwei Muster, ein wiederholter Clip, Swing; drei Klaenge
    Loop loop;
    loop.laut = 1.0;
    loop.schlaegeJeTakt = 1;
    loop.swing = 0.3;
    Muster eins;
    eins.name = QStringLiteral("P1");
    eins.spuren = {{QStringLiteral("A"), QStringLiteral("a.wav"), 1.0, 0.0, false},
                   {QStringLiteral("B"), QStringLiteral("b.wav"), 0.8, -0.4, false},
                   {QStringLiteral("C"), QStringLiteral("a.wav"), 0.5, 0.5, false}};
    eins.zeilen = {{1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0},
                   {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0.5, 0, 0, 0, 1, 1},
                   {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
    Muster zwei;
    zwei.name = QStringLiteral("P2");
    zwei.spuren = {{QStringLiteral("D"), QStringLiteral("c.wav"), 1.0, 0.0, false}};
    zwei.zeilen = {{0.7, 0, 0, 0, 0, 0, 0.3, 0, 0, 0, 0, 0, 0, 0, 0, 1}};
    loop.muster = {eins, zwei};
    loop.kanaele = {{QStringLiteral("K1"), true, {{QStringLiteral("P1"), 0, 2}}},
                    {QStringLiteral("K2"), true, {{QStringLiteral("P2"), 4, 1}}}};
    loop.richte();
    const Ablauf ablauf = plane(loop, kKernRate);
    REQUIRE(ablauf.durchlaufFrames == 2 * kKlein);
    REQUIRE(ablauf.klangNamen.size() == 3);
    REQUIRE(ablauf.stimmen.size() == 4u);
    // Klaenge verschieden lang: kuerzer und laenger als der Abstand der Schlaege
    const std::vector<Klang> klaenge = {bunterKlang(3000, 1), bunterKlang(9000, 2), bunterKlang(20000, 4)};
    const int gesamt = static_cast<int>(3 * 2 * kKlein);  // drei Durchlaeufe
    const std::vector<float> amStueck = rechneBereich(ablauf, klaenge, 0, gesamt);

    SUBCASE("in ungleichen Bloecken")
    {
        std::vector<float> gestueckelt(amStueck.size(), 9.0f);
        const int groessen[] = {1, 7, 441, 220, 8167, 3, 1024};
        std::int64_t stelle = 0;
        for (int i = 0; stelle < gesamt; ++i)
        {
            const int frames = static_cast<int>(std::min<std::int64_t>(groessen[i % 7], gesamt - stelle));
            rechne(ablauf, geteilt(klaenge), stelle, frames, gestueckelt.data() + 2 * stelle);
            stelle += frames;
        }
        CHECK(std::memcmp(gestueckelt.data(), amStueck.data(), amStueck.size() * sizeof(float)) == 0);
    }
    SUBCASE("mitten hinein, auch ueber die Grenze eines Durchlaufs")
    {
        for (const std::int64_t von : {std::int64_t{200000}, 2 * kKlein - 2500, kKlein + 5513})
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
    SUBCASE("es klingt ueberhaupt etwas, in beiden Haelften des Durchlaufs")
    {
        const auto klingt = [&](std::int64_t von, std::int64_t bis) {
            return std::any_of(amStueck.begin() + 2 * von, amStueck.begin() + 2 * bis,
                               [](float w) { return w != 0.0f; });
        };
        CHECK(klingt(0, kKlein));
        CHECK(klingt(kKlein, 2 * kKlein));
    }
}

TEST_CASE("Loop Rechenkern: ein neuer Schlag schneidet den klingenden derselben Stimme ab")
{
    // zwei Schlaege im Abstand von 5513 Abtastwerten, der Klang ist 10000 lang
    const Ablauf ablauf = plane(kleinLoop({1, 1, 0, 0}), kKernRate);
    const std::vector<float> block = rechneBereich(ablauf, {gleichKlang(10000, 1.0f)}, 0, 22050);

    CHECK(linksBei(block, 5512) == 1.0f);                   // der erste allein
    CHECK(linksBei(block, 5513) == 2.0f);                   // der zweite setzt ein, der erste noch voll
    CHECK(linksBei(block, 5513 + 32) == 1.5f);              // der erste auf halbem Weg
    CHECK(linksBei(block, 5513 + 63) == 1.0f + 1.0f / 64.0f);
    CHECK(linksBei(block, 5513 + kSchnittFrames) == 1.0f);  // der erste ist weg
    CHECK(linksBei(block, 9999) == 1.0f);
    CHECK(linksBei(block, 10000) == 1.0f);                  // ohne Schnitt endete der erste hier
    CHECK(linksBei(block, 5513 + 9999) == 1.0f);            // der zweite klingt aus
    CHECK(linksBei(block, 5513 + 10000) == 0.0f);

    SUBCASE("verschiedene Stimmen schneiden einander nicht — sie addieren sich")
    {
        Loop loop = kleinLoop({1, 0, 0, 0});
        loop.muster[0].spuren.push_back({QStringLiteral("B"), QStringLiteral("a.wav"), 1.0, 0.0, false});
        loop.muster[0].zeilen.push_back({0, 1, 0, 0});
        loop.richte();
        const std::vector<float> beide =
            rechneBereich(plane(loop, kKernRate), {gleichKlang(10000, 1.0f)}, 0, 22050);
        CHECK(linksBei(beide, 5513 + 200) == 2.0f);  // weit hinter jeder Blende: beide voll
        CHECK(linksBei(beide, 9999) == 2.0f);
        CHECK(linksBei(beide, 10000) == 1.0f);       // der erste ist ausgeklungen
    }
}

TEST_CASE("Loop Rechenkern: der Ausklang reicht in den naechsten Durchlauf, vor dem ersten gibt es keinen")
{
    // ein Schlag auf dem letzten Schritt (15 x 5512,5 = 82687,5), der Klang ist 10000 lang
    std::vector<double> zeile(16, 0.0);
    zeile[15] = 1.0;
    const Ablauf ablauf = plane(kleinLoop(zeile), kKernRate);
    const std::vector<float> block =
        rechneBereich(ablauf, {gleichKlang(10000, 1.0f)}, 0, static_cast<int>(2 * kKlein));

    CHECK(linksBei(block, 0) == 0.0f);  // kein Ausklang eines Durchlaufs davor
    CHECK(linksBei(block, 82687) == 0.0f);
    CHECK(linksBei(block, 82688) == 1.0f);
    CHECK(linksBei(block, kKlein) == 1.0f);  // ueber die Grenze hinweg
    CHECK(linksBei(block, 92687) == 1.0f);
    CHECK(linksBei(block, 92688) == 0.0f);
    CHECK(linksBei(block, kKlein + 82688) == 1.0f);
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
        CHECK(ablauf.durchlaufFrames == kKlein);  // ein Takt zu 16 Schritten
        CHECK(still(rechneBereich(ablauf, {}, 0, 1000)));
    }
    SUBCASE("eine Stimme ohne Klang")
    {
        const Ablauf ablauf = plane(kleinLoop({1, 1, 1, 1}), kKernRate);
        CHECK(still(rechneBereich(ablauf, {}, 0, 1000)));
        CHECK(still(rechneBereich(ablauf, {Klang{}}, 0, 1000)));
        const Ablauf ohneKlang = plane(kleinLoop({1, 1, 1, 1}, QString()), kKernRate);
        CHECK(still(rechneBereich(ohneKlang, {gleichKlang(100, 1.0f)}, 0, 1000)));
    }
    SUBCASE("ein Ablauf ohne Rate")
    {
        const Ablauf ablauf = plane(kleinLoop({1, 1, 1, 1}), 0);
        CHECK(still(rechneBereich(ablauf, {gleichKlang(100, 1.0f)}, 0, 1000)));
    }
}
