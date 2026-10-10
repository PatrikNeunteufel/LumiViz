/**
 ****************************************************************************************
 * @file   test_LoopSpieler.cpp
 * @brief  Loop, Stufe 5b: der Spieler — ein Loop spielt allein, mit der
 *         Rangfolge Composer, Loop-Panel, Player
 *         (docs/audio/Loop_Konzept.md, Entscheide L4 a und L7 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton: gegen einen Player und eine Audio-Engine aus Pappe (`PappTon.hpp`),
 * hier um die selbst rechnende Quelle und das Dekodieren erweitert.
 ****************************************************************************************
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Spieler.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>

#include <climits>
#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

/// Die Engine aus Pappe mit Mixer, dazu die Naht des Loops.
class LoopPappEngine : public papp::PappMischEngine
{
public:
    IAudioGenerator* generator = nullptr;  ///< der eingehaengte Generator; nullptr = keiner
    AudioStreamHandle generatorQuelle = INVALID_STREAM;
    QStringList dekodiert;                 ///< die Dateien, in der Reihenfolge von `decodeStereo`
    bool ohneMixer = false;

    [[nodiscard]] AudioStreamHandle createMixer() override
    {
        return ohneMixer ? INVALID_STREAM : PappMischEngine::createMixer();
    }
    [[nodiscard]] AudioStreamHandle mixerAddGenerator(AudioStreamHandle, IAudioGenerator* gen) override
    {
        protokoll << QStringLiteral("generator");
        Stream s;
        s.datei = QStringLiteral("<generator>");
        s.dauer = INT_MAX;
        s.haelt = true;  // kommt angehalten
        m_streams.push_back(s);
        generator = gen;
        generatorQuelle = letzter();
        return generatorQuelle;
    }
    void mixerRemoveSource(AudioStreamHandle source) override
    {
        if (source == generatorQuelle) generator = nullptr;
        PappMischEngine::mixerRemoveSource(source);
    }
    [[nodiscard]] std::vector<float> decodeStereo(const QString& filePath, int, double) override
    {
        dekodiert << filePath;
        if (filePath.contains(QStringLiteral("gibtsnicht"))) return {};
        return std::vector<float>(200, 0.5f);  // 100 Abtastwerte auf 0.5
    }
};

/// Ein Loop mit einem Kanal, einem Clip und einem Muster von vier Takten zu
/// je einem Schlag (16 Schritte je Durchlauf), mit einer Spur (Sample-Datei,
/// kein Kit).
Loop spielLoop(std::vector<double> zeile, const QString& klang = QStringLiteral("C:/s/a.wav"))
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
    return loop;
}

/// Der Player aus Pappe mit einem geladenen Song.
struct SpielPlayer : papp::PappPlayer
{
    SpielPlayer()
    {
        laengen[QStringLiteral("song")] = 60000;
        load(QStringLiteral("song"));
    }
};

float ersterWert(IAudioGenerator* generator)
{
    std::vector<float> block(200, 9.0f);
    generator->render(block.data(), 100);
    return block[0];
}

/// Eigene Einstellungen fuer einen Fall mit Panel: die des Benutzers bleiben unberuehrt.
struct LoopEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    LoopEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopPanel"));
        QSettings().clear();
    }
    ~LoopEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

}  // namespace

TEST_CASE("Loop Panel: laden, Play und Stop, und der zuletzt geladene Loop kommt wieder")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;

    // .../projects/apps/LumiViz/tests/unit/UnitTests -> Wurzel des Repos
    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QString beispiel = wurzel.filePath(QStringLiteral("asset/loop/drums_demo.lvloop"));
    REQUIRE(QFileInfo::exists(beispiel));

    ServiceContainer dienste;
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });
    auto* engine = dynamic_cast<LoopPappEngine*>(dienste.tryResolve<IAudioEngine>());
    IAudioPlayer* player = dienste.tryResolve<IAudioPlayer>();
    REQUIRE(engine != nullptr);
    REQUIRE(player != nullptr);
    player->play();

    {
        LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
        auto* play = panel.findChild<QPushButton*>(QStringLiteral("loopPlay"));
        auto* datei = panel.findChild<QLabel*>(QStringLiteral("loopFile"));
        auto* info = panel.findChild<QLabel*>(QStringLiteral("loopInfo"));
        auto* bericht = panel.findChild<QLabel*>(QStringLiteral("loopReport"));
        REQUIRE(play != nullptr);
        REQUIRE(datei != nullptr);
        REQUIRE(info != nullptr);
        REQUIRE(bericht != nullptr);

        // ohne Loop: nichts zu spielen
        CHECK(datei->text() == QStringLiteral("No loop loaded"));
        CHECK(!play->isEnabled());
        CHECK(!panel.spiele());

        // eine Datei, die kein Loop ist
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString fremd = ordner.filePath(QStringLiteral("fremd.lvloop"));
        {
            QFile f(fremd);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write("{ \"format\": \"etwas anderes\" }");
        }
        CHECK(!panel.lade(fremd));
        CHECK(panel.datei().isEmpty());

        REQUIRE(panel.lade(beispiel));
        CHECK(datei->text() == QStringLiteral("drums_demo.lvloop"));
        CHECK(info->text().startsWith(QStringLiteral("120 BPM")));
        CHECK(info->text().contains(QStringLiteral("channels: 2")));
        CHECK(info->text().contains(QStringLiteral("bars: 16")));
        CHECK(info->text().contains(QStringLiteral("32.0 s")));
        CHECK(play->isEnabled());
        CHECK(play->text() == QStringLiteral("Play"));
        CHECK(bericht->text().isEmpty());

        play->click();
        CHECK(panel.spieler().spielt());
        CHECK(play->text() == QStringLiteral("Stop"));
        CHECK(!player->isPlaying());
        CHECK(lumi::loop::tonMixer() == engine->mixer);
        CHECK(engine->dekodiert.size() == 6);  // die sechs Klaenge des Kits
        CHECK(bericht->text().isEmpty());

        play->click();
        CHECK(!panel.spieler().spielt());
        CHECK(play->text() == QStringLiteral("Play"));
        CHECK(player->isPlaying());

        play->click();  // spielt wieder: das Panel geht gleich, der Ton wird frei
        REQUIRE(panel.spieler().spielt());
    }
    CHECK(lumi::loop::tonMixer() == INVALID_STREAM);
    CHECK(engine->generator == nullptr);

    // ein neues Panel: der Loop steht wieder da, gespielt wird nicht von selbst
    LoopPanel wieder(dienste);
    CHECK(wieder.datei() == beispiel);
    CHECK(!wieder.spieler().spielt());
    CHECK(wieder.findChild<QPushButton*>(QStringLiteral("loopPlay"))->isEnabled());
}

TEST_CASE("Loop Spieler: uebernimmt den Ton, der Player schweigt und spielt danach weiter")
{
    LoopPappEngine engine;
    SpielPlayer player;
    player.play();
    Spieler spieler;
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));
    CHECK(!spieler.spielt());
    CHECK(tonMixer() == INVALID_STREAM);
    CHECK(Spieler::spielender() == nullptr);

    REQUIRE(spieler.starte(engine, player));
    CHECK(spieler.spielt());
    CHECK(Spieler::spielender() == &spieler);
    // ein Mixer, darin der Generator; er laeuft, der Mixer spielt
    REQUIRE(engine.mixer != INVALID_STREAM);
    CHECK(spieler.mixer() == engine.mixer);
    CHECK(tonMixer() == engine.mixer);
    REQUIRE(engine.generator != nullptr);
    CHECK(!engine.stream(engine.generatorQuelle).haelt);
    CHECK(engine.stream(engine.mixer).spielt);
    // der Player schweigt
    CHECK(!player.isPlaying());
    CHECK(player.isPaused());
    // und es klingt: der erste Schlag, Klang 0.5
    CHECK(ersterWert(engine.generator) == 0.5f);
    CHECK(spieler.bericht().isEmpty());

    const int plays = player.zahl(QStringLiteral("play"));
    spieler.halte();
    CHECK(!spieler.spielt());
    CHECK(tonMixer() == INVALID_STREAM);
    CHECK(Spieler::spielender() == nullptr);
    CHECK(engine.generator == nullptr);
    CHECK(engine.zahl(QStringLiteral("remove <generator>")) == 1);
    CHECK(engine.zahl(QStringLiteral("free mixer")) == 1);
    // der Player spielt weiter, weil er vorher spielte
    CHECK(player.zahl(QStringLiteral("play")) == plays + 1);
    CHECK(player.isPlaying());

    SUBCASE("spielte der Player vorher nicht, bleibt er still")
    {
        player.pause();
        REQUIRE(spieler.starte(engine, player));
        const int vorher = player.zahl(QStringLiteral("play"));
        spieler.halte();
        CHECK(player.zahl(QStringLiteral("play")) == vorher);
        CHECK(!player.isPlaying());
    }
    SUBCASE("halte ohne playerWeiter laesst den Player still")
    {
        REQUIRE(spieler.starte(engine, player));
        spieler.halte(false);
        CHECK(!player.isPlaying());
    }
}

TEST_CASE("Loop Spieler: Rangfolge — der Composer steht ueber dem Loop")
{
    LoopPappEngine engine;
    SpielPlayer player;
    player.play();
    bool composerFuehrt = false;
    Spieler spieler([&composerFuehrt] { return composerFuehrt; });
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));

    SUBCASE("fuehrt der Composer, spielt der Loop nicht — nichts wird angefasst")
    {
        composerFuehrt = true;
        CHECK(!spieler.darfSpielen());
        CHECK(!spieler.starte(engine, player));
        CHECK(!spieler.spielt());
        CHECK(engine.protokoll.isEmpty());
        CHECK(player.isPlaying());
    }
    SUBCASE("uebernimmt der Composer, haelt der Loop im naechsten Takt — der Player bleibt dem Composer")
    {
        REQUIRE(spieler.starte(engine, player));
        spieler.takt();
        CHECK(spieler.spielt());
        composerFuehrt = true;
        const int plays = player.zahl(QStringLiteral("play"));
        spieler.takt();
        CHECK(!spieler.spielt());
        CHECK(tonMixer() == INVALID_STREAM);
        CHECK(engine.zahl(QStringLiteral("free mixer")) == 1);
        CHECK(player.zahl(QStringLiteral("play")) == plays);  // nicht weitergespielt
        // und er startet nicht wieder, solange der Composer fuehrt
        CHECK(!spieler.starte(engine, player));
        composerFuehrt = false;
        CHECK(spieler.starte(engine, player));
    }
}

TEST_CASE("Loop Spieler: im Takt schweigt der Player, seine Lautstaerke gilt fuer den Loop")
{
    LoopPappEngine engine;
    SpielPlayer player;
    Spieler spieler;
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));
    player.laut = 0.8f;
    REQUIRE(spieler.starte(engine, player));
    // schon beim Start gesetzt, bevor etwas zu hoeren ist
    CHECK(engine.stream(engine.mixer).lautstaerke == 0.8f);

    player.laut = 0.4f;
    spieler.takt();
    CHECK(engine.stream(engine.mixer).lautstaerke == 0.4f);
    player.stumm = true;
    spieler.takt();
    CHECK(engine.stream(engine.mixer).lautstaerke == 0.0f);
    player.stumm = false;
    spieler.takt();
    CHECK(engine.stream(engine.mixer).lautstaerke == 0.4f);

    // jemand startet den Player: der Loop steht darueber
    player.play();
    REQUIRE(player.isPlaying());
    spieler.takt();
    CHECK(!player.isPlaying());
    CHECK(spieler.spielt());
}

TEST_CASE("Loop Spieler: ein geaenderter Loop gilt sofort, Klaenge werden nur bei Bedarf neu geladen")
{
    LoopPappEngine engine;
    SpielPlayer player;
    Spieler spieler;
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));
    CHECK(engine.dekodiert.isEmpty());  // geladen wird erst, wenn gespielt wird
    REQUIRE(spieler.starte(engine, player));
    REQUIRE(engine.dekodiert.size() == 1);
    CHECK(engine.dekodiert[0] == QStringLiteral("C:/s/a.wav"));
    CHECK(ersterWert(engine.generator) == 0.5f);

    // ein anderes Muster, halbe Lautstaerke auf dem ersten Schritt: kein neues Laden
    spieler.setzeLoop(spielLoop({0.5, 0, 0, 0}));
    CHECK(engine.dekodiert.size() == 1);
    spieler.starte(engine, player);  // von vorn
    CHECK(ersterWert(engine.generator) == 0.25f);

    // ein anderer Klang: neu geladen
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}, QStringLiteral("C:/s/b.wav")));
    REQUIRE(engine.dekodiert.size() == 2);
    CHECK(engine.dekodiert[1] == QStringLiteral("C:/s/b.wav"));
    CHECK(spieler.bericht().isEmpty());

    // ein Klang, der sich nicht laden laesst: Bericht, die Spur schweigt
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}, QStringLiteral("C:/s/gibtsnicht.wav")));
    REQUIRE(spieler.bericht().size() == 1);
    CHECK(spieler.bericht()[0].startsWith(QStringLiteral("Sound not loaded: ")));
    spieler.starte(engine, player);
    CHECK(ersterWert(engine.generator) == 0.0f);
}

TEST_CASE("Loop Spieler: die Stelle in Schritten, und Play beginnt von vorn")
{
    LoopPappEngine engine;
    SpielPlayer player;
    Spieler spieler;
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));
    CHECK(spieler.stelleSchritte() == 0.0);
    REQUIRE(spieler.starte(engine, player));
    CHECK(spieler.stelleSchritte() == 0.0);

    // der Mixer holt 11025 Abtastwerte: bei 120 BPM und 44100 Hz zwei Schritte
    std::vector<float> block(2 * 11025);
    engine.generator->render(block.data(), 11025);
    CHECK(spieler.stelleSchritte() == doctest::Approx(2.0));
    // ueber den Durchlauf (16 Schritte) hinaus zaehlt sie wieder von vorn
    for (int i = 0; i < 8; ++i) engine.generator->render(block.data(), 11025);
    CHECK(spieler.stelleSchritte() == doctest::Approx(2.0));

    REQUIRE(spieler.starte(engine, player));  // spielt schon: von vorn, kein zweiter Mixer
    CHECK(spieler.stelleSchritte() == 0.0);
    CHECK(engine.zahl(QStringLiteral("mixer")) == 1);
}

TEST_CASE("Loop Spieler: ohne Mixer oder ohne die Naht bleibt alles, wie es war")
{
    SpielPlayer player;
    player.play();
    Spieler spieler;
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));

    SUBCASE("die Engine hat keinen Mixer")
    {
        LoopPappEngine engine;
        engine.ohneMixer = true;
        CHECK(!spieler.starte(engine, player));
    }
    SUBCASE("die Engine kennt keine selbst rechnende Quelle: der Mixer geht wieder")
    {
        papp::PappMischEngine engine;
        CHECK(!spieler.starte(engine, player));
        CHECK(engine.zahl(QStringLiteral("free mixer")) == 1);
    }
    CHECK(!spieler.spielt());
    CHECK(player.isPlaying());
    CHECK(tonMixer() == INVALID_STREAM);
}

TEST_CASE("Loop Spieler: es gibt nur einen Ton — und wer geht, gibt ihn frei")
{
    LoopPappEngine engine;
    SpielPlayer player;
    Spieler erster;
    erster.setzeLoop(spielLoop({1, 0, 0, 0}));
    REQUIRE(erster.starte(engine, player));
    {
        Spieler zweiter;
        zweiter.setzeLoop(spielLoop({1, 1, 1, 1}));
        REQUIRE(zweiter.starte(engine, player));
        CHECK(!erster.spielt());
        CHECK(Spieler::spielender() == &zweiter);
        CHECK(engine.zahl(QStringLiteral("free mixer")) == 1);
    }
    // der zweite ist zerstoert: Quelle und Mixer sind frei
    CHECK(Spieler::spielender() == nullptr);
    CHECK(engine.generator == nullptr);
    CHECK(engine.zahl(QStringLiteral("free mixer")) == 2);
}
