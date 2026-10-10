/**
 ****************************************************************************************
 * @file   test_LoopWiedergabe.cpp
 * @brief  Loop, Stufe 5: Klangerzeuger, Kit und die Naht in der Audio-Engine
 *         (docs/audio/Loop_Konzept.md §5 und §6)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * - Der Klangerzeuger ohne Engine: `render` von Hand gerufen, die Werte gegen
 *   den Rechenkern gezaehlt.
 * - Das Kit: Ordner, Dateien, Dekodieren ueber die `BassEngine`.
 * - Der Klangerzeuger im spielenden Mixer der `BassEngine`.
 *
 * Die Faelle mit Engine brauchen ein Ausgabegeraet; ohne melden sie das und
 * pruefen nichts. Der letzte spielt etwa drei Sekunden sehr leisen Ton.
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/BassEngine.hpp"
#include "loop/Kit.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QThread>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kSpielRate = 44100;
constexpr float kSpielLeise = 0.03f;  ///< Lautstaerke des Falls mit Ton

/// Qt-Anwendung fuer einen Fall (Programmordner fuer die Suche nach dem Kit
/// und fuer die Engine).
struct SpielApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    std::unique_ptr<QCoreApplication> app;
    SpielApp()
    {
        if (QCoreApplication::instance() == nullptr) app = std::make_unique<QCoreApplication>(argc, argv);
    }
};

/// Ein Loop mit einem Kanal, einem Clip und einem Muster von vier Takten zu
/// je einem Schlag (16 Schritte, bei 120 BPM 88200 Abtastwerte), mit einer Spur.
Loop schlagLoop(std::vector<double> zeile, double bpm = 120.0)
{
    Loop loop;
    loop.laut = 1.0;
    loop.bpm = bpm;
    loop.schlaegeJeTakt = 1;
    Muster m;
    m.name = QStringLiteral("M");
    m.spuren = {{QStringLiteral("A"), QStringLiteral("a.wav"), 1.0, 0.0, false}};
    m.zeilen = {std::move(zeile)};
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("M"), 0, 1}}}};
    loop.richte();
    return loop;
}

std::shared_ptr<const std::vector<Klang>> einKlang(int frames, float wert)
{
    Klang k;
    k.daten.assign(static_cast<std::size_t>(frames) * 2, wert);
    return std::make_shared<const std::vector<Klang>>(std::vector<Klang>{k});
}

std::vector<float> rendere(Klangerzeuger& erzeuger, int frames)
{
    std::vector<float> aus(static_cast<std::size_t>(frames) * 2, 9.0f);
    erzeuger.render(aus.data(), frames);
    return aus;
}

std::vector<float> sollWerte(const Programm& p, std::int64_t von, int frames)
{
    std::vector<float> aus(static_cast<std::size_t>(frames) * 2, 9.0f);
    rechne(p.ablauf, *p.klaenge, von, frames, aus.data());
    return aus;
}

bool gleich(const std::vector<float>& a, const std::vector<float>& b)
{
    return a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size() * sizeof(float)) == 0;
}

/// Das Beispiel aus `asset/loop`.
bool ladeBeispiel(Loop& loop)
{
    const QString datei = QDir(kitOrdner()).filePath(QStringLiteral("../drums_demo.lvloop"));
    return ladeLoop(datei, loop);
}

}  // namespace

// =============================================================================
// Der Klangerzeuger, von Hand gerufen
// =============================================================================

TEST_CASE("Loop Klangerzeuger: rechnet sein Programm Block fuer Block und zaehlt die Stelle")
{
    const auto programm = baueProgramm(schlagLoop({1, 0, 0.5, 1}), kSpielRate, einKlang(9000, 0.25f));
    REQUIRE(programm->ablauf.durchlaufFrames == 88200);
    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(programm);
    CHECK(erzeuger.programm() == programm);
    CHECK(erzeuger.stelle() == 0);

    std::int64_t stelle = 0;
    for (const int frames : {441, 220, 8167, 3, 30000, 60000})
    {
        CHECK(gleich(rendere(erzeuger, frames), sollWerte(*programm, stelle, frames)));
        stelle += frames;
        CHECK(erzeuger.stelle() == stelle);
    }
    // ueber den ersten Durchlauf hinaus
    CHECK(erzeuger.stelle() == 98831);
    CHECK(erzeuger.stelleImDurchlauf() == 98831 - 88200);

    SUBCASE("die Stelle laesst sich setzen")
    {
        erzeuger.setzeStelle(5000);
        CHECK(gleich(rendere(erzeuger, 2000), sollWerte(*programm, 5000, 2000)));
        CHECK(erzeuger.stelle() == 7000);
        erzeuger.setzeStelle(-5);
        CHECK(erzeuger.stelle() == 0);
    }
    SUBCASE("ein neues Programm gilt ab dem naechsten Block, die Stelle laeuft weiter")
    {
        const auto anderes = baueProgramm(schlagLoop({1, 1, 1, 1}), kSpielRate, einKlang(9000, 0.5f));
        erzeuger.setzeProgramm(anderes);
        CHECK(erzeuger.stelle() == 98831);  // gleiches Tempo: nichts umgerechnet
        CHECK(gleich(rendere(erzeuger, 4000), sollWerte(*anderes, 98831, 4000)));
    }
}

TEST_CASE("Loop Klangerzeuger: ohne Programm Stille, die Stelle bleibt stehen")
{
    Klangerzeuger erzeuger;
    const std::vector<float> block = rendere(erzeuger, 500);
    CHECK(std::all_of(block.begin(), block.end(), [](float w) { return w == 0.0f; }));
    CHECK(erzeuger.stelle() == 0);
    CHECK(erzeuger.stelleImDurchlauf() == 0);

    const auto programm = baueProgramm(schlagLoop({1, 0, 0, 0}), kSpielRate, einKlang(100, 1.0f));
    erzeuger.setzeProgramm(programm);
    (void)rendere(erzeuger, 500);
    CHECK(erzeuger.stelle() == 500);
    erzeuger.setzeProgramm(nullptr);
    const std::vector<float> still = rendere(erzeuger, 500);
    CHECK(std::all_of(still.begin(), still.end(), [](float w) { return w == 0.0f; }));
    CHECK(erzeuger.stelle() == 500);
}

TEST_CASE("Loop Klangerzeuger: ein anderes Tempo laesst die Stelle im Takt, nicht in der Zeit")
{
    const auto klaenge = einKlang(100, 1.0f);
    const auto bei120 = baueProgramm(schlagLoop({1, 1, 1, 1}, 120.0), kSpielRate, klaenge);
    const auto bei60 = baueProgramm(schlagLoop({1, 1, 1, 1}, 60.0), kSpielRate, klaenge);
    REQUIRE(bei120->ablauf.schrittFrames == doctest::Approx(5512.5));
    REQUIRE(bei60->ablauf.schrittFrames == doctest::Approx(11025.0));

    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(bei120);
    erzeuger.setzeStelle(3 * 88200 + 11025);  // vierter Durchlauf, auf dem dritten Schritt
    erzeuger.setzeProgramm(bei60);
    CHECK(erzeuger.stelle() == 2 * (3 * 88200 + 11025));
    CHECK(erzeuger.stelleImDurchlauf() == 22050);  // wieder der dritte Schritt
    erzeuger.setzeProgramm(bei120);
    CHECK(erzeuger.stelle() == 3 * 88200 + 11025);
}

// =============================================================================
// Das Kit
// =============================================================================

TEST_CASE("Loop Kit: der Ordner wird gefunden, ein Klang wird zu seiner Datei")
{
    SpielApp app;
    const QString ordner = kitOrdner();
    REQUIRE_MESSAGE(!ordner.isEmpty(), "asset/loop/kit vom Programmordner aufwaerts nicht gefunden");
    CHECK(ordner.endsWith(QStringLiteral("asset/loop/kit")));

    const QString kick = klangDatei(QStringLiteral("kit:kick"));
    CHECK(kick == QDir(ordner).filePath(QStringLiteral("kick.wav")));
    CHECK(QFileInfo::exists(kick));
    // ein Kit-Name fuehrt nie aus dem Ordner hinaus
    CHECK(klangDatei(QStringLiteral("kit:../drums_demo")).isEmpty());
    CHECK(klangDatei(QStringLiteral("kit:Kick")).isEmpty());
    CHECK(klangDatei(QStringLiteral("kit:")).isEmpty());
    // alles andere ist ein Pfad
    CHECK(klangDatei(QStringLiteral("C:/x/clap.wav")) == QStringLiteral("C:/x/clap.wav"));
    CHECK(klangDatei(QString()).isEmpty());
}

TEST_CASE("Loop Kit: die Engine dekodiert die Klaenge — Stereo, in der verlangten Abtastrate")
{
    SpielApp app;
    BassEngine engine;
    if (!engine.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << engine.getLastError().toStdString() << ")");
        return;
    }
    const QString kick = klangDatei(QStringLiteral("kit:kick"));

    const std::vector<float> roh = engine.decodeStereo(kick, 44100, 30.0);
    float spitze = 0.0f;
    bool beideGleich = true;
    for (std::size_t i = 0; i + 1 < roh.size(); i += 2)
    {
        spitze = std::max(spitze, std::abs(roh[i]));
        if (roh[i] != roh[i + 1]) beideGleich = false;
    }
    MESSAGE("kick.wav bei 44100 Hz: " << roh.size() / 2 << " Abtastwerte, Spitze " << std::to_string(spitze));
    CHECK(roh.size() / 2 == 22050u);  // die Datei hat 22050
    CHECK(beideGleich);               // Mono auf beide Seiten
    CHECK(spitze == doctest::Approx(0.9f).epsilon(0.01));

    const std::vector<float> um = engine.decodeStereo(kick, 48000, 30.0);
    MESSAGE("kick.wav bei 48000 Hz: " << um.size() / 2 << " Abtastwerte");
    CHECK(um.size() / 2 > 23900u);
    CHECK(um.size() / 2 < 24100u);

    CHECK(engine.decodeStereo(kick, 44100, 0.1).size() / 2 == 4410u);
    CHECK(engine.decodeStereo(QStringLiteral("C:/gibts/nicht.wav"), 44100, 30.0).empty());
    CHECK(engine.decodeStereo(kick, 0, 30.0).empty());

    SUBCASE("ladeKlaenge: je Name ein Klang, was fehlt, steht im Bericht")
    {
        Loop loop;
        REQUIRE(ladeBeispiel(loop));
        QStringList namen = loop.klangNamen();
        REQUIRE(namen.size() == 6);
        REQUIRE(namen[0] == QStringLiteral("kit:kick"));
        QStringList bericht;
        std::vector<Klang> klaenge = ladeKlaenge(namen, engine, 44100, &bericht);
        CHECK(bericht.isEmpty());
        REQUIRE(klaenge.size() == 6u);
        for (const Klang& k : klaenge) CHECK(k.frames() > 3000);
        CHECK(klaenge[0].frames() == 22050);

        namen[1] = QStringLiteral("kit:gibtsnicht");
        klaenge = ladeKlaenge(namen, engine, 44100, &bericht);
        REQUIRE(bericht.size() == 1);
        CHECK(bericht[0] == QStringLiteral("Sound not loaded: kit:gibtsnicht"));
        CHECK(klaenge[1].frames() == 0);
        CHECK(klaenge[2].frames() > 3000);
    }
    SUBCASE("das Beispiel uebersteuert nicht: die Summe bleibt unter 1")
    {
        Loop loop;
        REQUIRE(ladeBeispiel(loop));
        const Ablauf ablauf = plane(loop, 44100);
        const std::vector<Klang> klaenge = ladeKlaenge(ablauf.klangNamen, engine, 44100);
        std::vector<float> alles(static_cast<std::size_t>(ablauf.durchlaufFrames) * 2);
        rechne(ablauf, klaenge, 0, static_cast<int>(ablauf.durchlaufFrames), alles.data());
        float groesster = 0.0f;
        for (const float wert : alles) groesster = std::max(groesster, std::abs(wert));
        MESSAGE("drums_demo.lvloop: ein Durchlauf " << ablauf.durchlaufFrames << " Abtastwerte, groesster Betrag "
                                                    << std::to_string(groesster));
        CHECK(groesster > 0.3f);
        CHECK(groesster < 1.0f);
    }
}

// =============================================================================
// Der Klangerzeuger im spielenden Mixer
// =============================================================================

TEST_CASE("Loop Wiedergabe: der Klangerzeuger spielt das Beispiel im Mixer der BassEngine")
{
    SpielApp app;
    BassEngine engine;
    if (!engine.initialize())
    {
        MESSAGE("kein Ausgabegeraet: nichts geprueft (" << engine.getLastError().toStdString() << ")");
        return;
    }
    const int rate = engine.getSampleRate();
    Loop loop;
    REQUIRE(ladeBeispiel(loop));
    QStringList bericht;
    const auto klaenge =
        std::make_shared<const std::vector<Klang>>(ladeKlaenge(loop.klangNamen(), engine, rate, &bericht));
    REQUIRE(bericht.isEmpty());

    Klangerzeuger erzeuger;  // lebt laenger als seine Quelle im Mixer
    erzeuger.setzeProgramm(baueProgramm(loop, rate, klaenge));
    const AudioStreamHandle mixer = engine.createMixer();
    REQUIRE(mixer != INVALID_STREAM);
    engine.setVolume(mixer, kSpielLeise);
    const AudioStreamHandle quelle = engine.mixerAddGenerator(mixer, &erzeuger);
    REQUIRE(quelle != INVALID_STREAM);
    REQUIRE(engine.play(mixer));

    // angehalten eingehaengt: der Mixer laeuft, holt bei ihr aber nichts
    QThread::msleep(200);
    CHECK(!engine.isSourcePlaying(quelle));
    CHECK(erzeuger.stelle() == 0);

    QElapsedTimer uhr;
    uhr.start();
    REQUIRE(engine.setSourcePaused(quelle, false));
    float groesster = 0.0f;
    while (uhr.elapsed() < 1200)
    {
        float links = 0.0f, rechts = 0.0f;
        engine.getChannelLevels(mixer, links, rechts);
        groesster = std::max(groesster, std::max(links, rechts));
        QThread::msleep(10);
    }
    const qint64 vergangenMs = uhr.elapsed();
    const double gespieltMs = 1000.0 * static_cast<double>(erzeuger.stelle()) / rate;
    MESSAGE("in " << vergangenMs << " ms steht der Klangerzeuger bei " << erzeuger.stelle() << " Abtastwerten = "
                  << std::to_string(gespieltMs) << " ms; groesster Pegel des Mixers " << std::to_string(groesster));
    CHECK(engine.isSourcePlaying(quelle));
    CHECK(gespieltMs > 0.6 * static_cast<double>(vergangenMs));
    CHECK(gespieltMs < 1.5 * static_cast<double>(vergangenMs));
    CHECK(groesster > 0.05f);

    // ein neues Programm im Lauf (doppeltes Tempo): es laeuft weiter
    Loop schneller = loop;
    schneller.bpm = 240.0;
    const std::int64_t vorWechsel = erzeuger.stelle();
    erzeuger.setzeProgramm(baueProgramm(schneller, rate, klaenge));
    QThread::msleep(400);
    CHECK(erzeuger.stelle() != vorWechsel);

    // anhalten: die Stelle bleibt stehen und laesst sich setzen
    REQUIRE(engine.setSourcePaused(quelle, true));
    QThread::msleep(150);
    const std::int64_t gehalten = erzeuger.stelle();
    QThread::msleep(200);
    CHECK(erzeuger.stelle() == gehalten);
    erzeuger.setzeStelle(0);
    CHECK(erzeuger.stelle() == 0);

    // weiter von vorn
    REQUIRE(engine.setSourcePaused(quelle, false));
    QThread::msleep(300);
    CHECK(erzeuger.stelle() > 0);
    CHECK(erzeuger.stelle() < rate);

    engine.mixerRemoveSource(quelle);
    const std::int64_t amEnde = erzeuger.stelle();
    QThread::msleep(100);
    CHECK(erzeuger.stelle() == amEnde);  // nach dem Entfernen ruft niemand mehr
    engine.freeStream(mixer);
}
