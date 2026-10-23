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
#include "TestFenster.hpp"
#include "UI/panels/PlayerPanel.hpp"
#include "audio/AudioEvents.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/Komposition.hpp"
#include "composer/Tonfuehrung.hpp"
#include "loop/ComposerAbfragen.hpp"
#include "loop/DrumEditor.hpp"
#include "loop/Freeze.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Taktachse.hpp"
#include "loop/Offen.hpp"
#include "loop/SichttestAktionen.hpp"
#include "loop/Spieler.hpp"
#include "loop/Tonspeicher.hpp"
#include "services/EventBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <memory>
#include <mutex>
#include <thread>
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
    std::mutex sperre;
    std::atomic<bool> halten{false};  ///< das Dekodieren wartet, bis der Fall es freigibt (Stufe 29d)
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
        {
            const std::lock_guard<std::mutex> halt(sperre);  // die Threads eines Tonauftrags (Stufe 29d)
            dekodiert << filePath;
        }
        while (halten) std::this_thread::yield();
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
        panel.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29d)
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

TEST_CASE("Loop Sichttest-Aktionen: loop_laden haelt und laedt ohne Frage; loop_wiedergabe; loop_editor")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;
    using lumi::sichttest::Ergebnis;
    namespace aktion = lumi::loop::sichttestaktion;
    const auto mit = [](const char* schluessel, const QString& text) {
        return QJsonObject{{QString::fromLatin1(schluessel), text}};
    };

    // .../projects/apps/LumiViz/tests/unit/UnitTests -> Wurzel des Repos: dort beginnen relative Dateien
    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QString root = wurzel.absolutePath();
    const QString beispiel = QStringLiteral("asset\\loop\\drums_demo.lvloop");

    ServiceContainer dienste;
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });

    CHECK(aktion::panel() == nullptr);  // noch gibt es keines
    LoopPanel panel(dienste);           // nicht gezeigt: kein Fenster
    CHECK(aktion::panel() == &panel);

    // --- ohne Loop
    CHECK(aktion::wiedergabe(panel, mit("wert", QStringLiteral("start"))).status == Ergebnis::Status::Fehler);
    CHECK(aktion::editor(panel, mit("basis", QStringLiteral("Grund")), false).status == Ergebnis::Status::Fehler);

    // --- loop_laden: die Datei fehlt, gibt es nicht, ist kein Loop
    CHECK(aktion::loopLaden(panel, root, QJsonObject{}).status == Ergebnis::Status::Ungueltig);
    CHECK(aktion::loopLaden(panel, root, mit("datei", QStringLiteral("asset/nirgends.lvloop"))).status ==
          Ergebnis::Status::Ungueltig);
    CHECK(aktion::loopLaden(panel, root, mit("datei", QStringLiteral("Solution.json"))).status ==
          Ergebnis::Status::Fehler);
    CHECK_FALSE(panel.geladen());

    CHECK(aktion::loopLaden(panel, root, mit("datei", beispiel)).status == Ergebnis::Status::Ok);
    CHECK(QFileInfo(panel.datei()) == QFileInfo(wurzel.filePath(beispiel)));
    CHECK_FALSE(panel.ungespeichert());

    // --- loop_wiedergabe
    CHECK(aktion::wiedergabe(panel, QJsonObject{}).status == Ergebnis::Status::Ungueltig);
    CHECK(aktion::wiedergabe(panel, mit("wert", QStringLiteral("lauter"))).status == Ergebnis::Status::Ungueltig);
    CHECK(aktion::wiedergabe(panel, mit("wert", QStringLiteral("start"))).status == Ergebnis::Status::Ok);
    CHECK(panel.spieler().spielt());
    CHECK(aktion::wiedergabe(panel, mit("wert", QStringLiteral("stopp"))).status == Ergebnis::Status::Ok);
    CHECK_FALSE(panel.spieler().spielt());
    CHECK(aktion::wiedergabe(panel, mit("wert", QStringLiteral("stopp"))).status == Ergebnis::Status::Ok);  // schon aus

    // --- loop_laden haelt einen spielenden Loop an und verwirft Ungespeichertes ohne Frage
    Loop geaendert = panel.loop();
    geaendert.bpm = 99.0;
    panel.bearbeite(geaendert, QStringLiteral("Change tempo"));
    REQUIRE(panel.ungespeichert());
    REQUIRE(panel.spiele());
    CHECK(aktion::loopLaden(panel, root, mit("datei", beispiel)).status == Ergebnis::Status::Ok);
    CHECK_FALSE(panel.spieler().spielt());
    CHECK_FALSE(panel.ungespeichert());
    CHECK(panel.loop().bpm == doctest::Approx(120.0));

    // --- loop_editor: die Basis fehlt, gibt es nicht (die Antwort nennt, was es gibt), gibt es
    CHECK(aktion::editor(panel, QJsonObject{}, false).status == Ergebnis::Status::Ungueltig);
    const Ergebnis keine = aktion::editor(panel, mit("basis", QStringLiteral("gibt es nicht")), false);
    CHECK(keine.status == Ergebnis::Status::Fehler);
    CHECK(keine.text.contains(QStringLiteral("Grund")));
    CHECK(panel.editor() == nullptr);
    CHECK(aktion::editor(panel, mit("basis", QStringLiteral("Grund")), false).status == Ergebnis::Status::Ok);
    CHECK(panel.editor() != nullptr);
}

TEST_CASE("Loop Panel: New instrument clip und der Instrument-Editor — eine Taste spielt den Ton an")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;

    ServiceContainer dienste;
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });
    auto* engine = dynamic_cast<LoopPappEngine*>(dienste.tryResolve<IAudioEngine>());
    IAudioPlayer* player = dienste.tryResolve<IAudioPlayer>();
    REQUIRE(engine != nullptr);
    player->play();

    LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
    panel.neu();
    Taktachse* achse = panel.taktachse();
    auto* play = panel.findChild<QPushButton*>(QStringLiteral("loopPlay"));
    REQUIRE(play != nullptr);

    // --- ein Instrument-Clip und ein Drumkit-Clip: je Art ein Editor
    REQUIRE(achse->neuerClip(0, 0, MusterArt::Instrument));
    REQUIRE(achse->neuerClip(0, 4));
    REQUIRE(panel.loop().muster.size() == 2);
    const QString noten = panel.loop().muster[0].name;
    const QString drums = panel.loop().muster[1].name;
    CHECK(panel.loop().muster[0].art == MusterArt::Instrument);
    CHECK(panel.loop().muster[0].instrument == QStringLiteral("piano"));
    CHECK(panel.loop().muster[1].art == MusterArt::Drumkit);

    CHECK(panel.instrumentEditor() == nullptr);
    // auch die Aktion der Sichttest-Steuerung waehlt den Editor nach der Art der Basis
    CHECK(lumi::loop::sichttestaktion::editor(panel, QJsonObject{{QStringLiteral("basis"), noten}}, false).status ==
          lumi::sichttest::Ergebnis::Status::Ok);
    CHECK(panel.instrumentEditor() != nullptr);
    CHECK(panel.editor() == nullptr);
    REQUIRE(panel.oeffneEditor(noten, false));
    InstrumentEditor* editor = panel.instrumentEditor();
    REQUIRE(editor != nullptr);
    CHECK(panel.editor() == nullptr);  // der Drum-Editor ist ein anderes Fenster
    CHECK(editor->muster() == noten);
    REQUIRE(panel.oeffneEditor(drums, false));
    REQUIRE(panel.editor() != nullptr);
    CHECK(panel.editor()->muster() == drums);
    CHECK(editor->muster() == noten);  // jeder bleibt bei seiner Basis

    // --- eine Aenderung im Editor: das Panel fuehrt sie aus, alle zeigen sie
    editor->setzeInstrument(QStringLiteral("organ"));
    CHECK(panel.loop().findeMuster(noten)->instrument == QStringLiteral("organ"));
    CHECK(editor->loop() == panel.loop());
    CHECK(achse->loop() == panel.loop());
    CHECK(panel.ungespeichert());

    // --- eine Taste spielt den Ton an: einen Schlag lang, dann haelt der Spieler wieder
    CHECK_FALSE(panel.spieleTon(drums, 60));               // ein Drumkit hat keine Toene
    CHECK_FALSE(panel.spieleTon(noten, kHoeheMax + 1));
    CHECK_FALSE(panel.spieleTon(QStringLiteral("gibt es nicht"), 60));
    REQUIRE(panel.spieleTon(noten, 60));
    CHECK(panel.hoertVor());
    CHECK(panel.spieler().spielt());
    CHECK(engine->generator != nullptr);
    CHECK(!player->isPlaying());
    CHECK(play->text() == QStringLiteral("Play"));  // Anspielen zaehlt nicht als Spielen
    REQUIRE(panel.spieler().loop().muster.size() == 1);
    CHECK(panel.spieler().loop().muster[0].noten == std::vector<Note>{{0, 4, 60, 1.0}});
    CHECK(panel.spieler().loop().muster[0].instrument == QStringLiteral("organ"));
    // eine zweite Taste loest die erste ab
    REQUIRE(panel.spieleTon(noten, 64));
    CHECK(panel.spieler().loop().muster[0].noten == std::vector<Note>{{0, 4, 64, 1.0}});
    // nach dem Ton und seinem Ausklang (ein Schlag bei 120 BPM, dazu 600 ms) ist es still
    QElapsedTimer uhr;
    uhr.start();
    while (panel.hoertVor() && uhr.elapsed() < 4000) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    CHECK_FALSE(panel.hoertVor());
    CHECK(uhr.elapsed() >= 1000);
    CHECK_FALSE(panel.spieler().spielt());
    CHECK(player->isPlaying());                     // der Player spielt weiter
    CHECK(panel.spieler().loop() == panel.loop());  // und der Spieler hat den Loop wieder

    // --- eine Aenderung beendet das Anspielen sofort — sonst spielte ploetzlich der Loop
    REQUIRE(panel.spieleTon(noten, 60));
    editor->setzeInstrument(QStringLiteral("lead"));
    CHECK_FALSE(panel.hoertVor());
    CHECK_FALSE(panel.spieler().spielt());

    // --- Play waehrend eines angespielten Tons spielt den Loop; spielt er, wird nichts angespielt
    REQUIRE(panel.spieleTon(noten, 60));
    play->click();
    CHECK_FALSE(panel.hoertVor());
    CHECK(panel.spieler().spielt());
    CHECK(panel.spieler().loop() == panel.loop());
    CHECK(play->text() == QStringLiteral("Stop"));
    CHECK_FALSE(panel.spieleTon(noten, 60));
    // der Editor sagt, warum die Tasten jetzt stumm bleiben
    auto* hinweis = editor->findChild<QLabel*>(QStringLiteral("instrumentHint"));
    REQUIRE(hinweis != nullptr);
    CHECK(hinweis->text() == QStringLiteral("Keys are silent while the loop plays"));
    play->click();
    CHECK_FALSE(panel.spieler().spielt());
    CHECK(hinweis->text().isEmpty());

    // --- der Instrument-Creator spielt sein Instrument an: beim Namen oder beim Pfad einer Datei
    REQUIRE(panel.spieleInstrumentTon(QStringLiteral("strings"), 62));
    CHECK(panel.hoertVor());
    CHECK(panel.spieler().loop().muster[0].instrument == QStringLiteral("strings"));
    CHECK(panel.spieler().loop().muster[0].noten == std::vector<Note>{{0, 4, 62, 1.0}});
    CHECK_FALSE(panel.spieleInstrumentTon(QString(), 62));
    CHECK_FALSE(panel.spieleInstrumentTon(QStringLiteral("strings"), 5));
    // „Repeat note" im Creator (S116): der Ton darf neun Sekunden klingen — das Muster ist so lang,
    // dass es nicht vorher von vorn beginnt (bei 120 BPM dauert ein Takt zwei Sekunden)
    REQUIRE(panel.spieleInstrumentTon(QStringLiteral("strings"), 62, 9000));
    CHECK(panel.hoertVor());
    CHECK(panel.spieler().loop().muster[0].takte == 6);
    CHECK(panel.spieler().loop().muster[0].noten == std::vector<Note>{{0, 4, 62, 1.0}});  // die Note selbst bleibt ein Schlag
    panel.halte();
    CHECK_FALSE(panel.hoertVor());
    CHECK(panel.spieler().loop() == panel.loop());

    // --- Play as im Creator (Stufe 24d): Noten mit Saite, Artikulation und Anschlag — jede an
    //     Schritt 0, einen Schlag lang; ein Akkord bleibt einer
    {
        Muster vorlage;
        vorlage.noten = {Note{7, 1, 52, 1.0, 2, kPalmMute}, Note{3, 9, 47, 0.5, 1}};
        vorlage.anschlaege = {Anschlag{7, Anschlagart::Auf}};
        vorlage.strum.spreadMs = 30.0;
        REQUIRE(panel.spieleInstrumentNoten(QStringLiteral("guitar"), vorlage));
        CHECK(panel.hoertVor());
        const Muster& gespielt = panel.spieler().loop().muster[0];
        CHECK(gespielt.instrument == QStringLiteral("guitar"));
        CHECK(gespielt.art == MusterArt::Instrument);
        CHECK(gespielt.noten == std::vector<Note>{Note{0, 4, 47, 0.5, 1}, Note{0, 4, 52, 1.0, 2, kPalmMute}});
        CHECK(gespielt.anschlaege == std::vector<Anschlag>{Anschlag{0, Anschlagart::Auf}});
        CHECK(gespielt.strum.spreadMs == doctest::Approx(30.0));
        CHECK_FALSE(panel.spieleInstrumentNoten(QStringLiteral("guitar"), Muster{}));  // keine Noten
        panel.halte();
        // eine Taste des Editors nimmt die Abweichungen der Akkorde ihres Musters nicht mit
        CHECK(panel.spieleTon(noten, 60));
        CHECK(panel.spieler().loop().muster[0].anschlaege.empty());
        panel.halte();
        // das Fenster Chords (Stufe 26e-2): Down-up behaelt seine Schritte — zwei Anschlaege
        Muster griff;
        griff.noten = {Note{0, 2, 48, 1.0, 1}, Note{0, 2, 52, 1.0, 2}, Note{2, 8, 48, 1.0, 1}, Note{2, 8, 52, 1.0, 2}};
        griff.anschlaege = {Anschlag{0, Anschlagart::Ab}, Anschlag{2, Anschlagart::Auf}};
        REQUIRE(panel.spieleAkkordVor(QStringLiteral("guitar"), griff));
        CHECK(panel.hoertVor());
        CHECK(panel.spieler().loop().muster[0].noten == griff.noten);
        CHECK(panel.spieler().loop().muster[0].anschlaege == griff.anschlaege);
        CHECK(panel.spieler().loop().muster[0].instrument == QStringLiteral("guitar"));
        CHECK_FALSE(panel.spieleAkkordVor(QStringLiteral("guitar"), Muster{}));
        CHECK_FALSE(panel.spieleAkkordVor(QString(), griff));
        panel.halte();
        // der Creator (Stufe 26g): Hammer-on behaelt seine Schritte, ein einzelner Schritt wird ein Schlag
        Muster hammer;
        hammer.noten = {Note{0, 2, 55, 1.0}, Note{2, 4, 57, 1.0, -1, kLegato}};
        REQUIRE(panel.spieleCreatorNoten(QStringLiteral("guitar"), hammer, 0));
        CHECK(panel.spieler().loop().muster[0].noten == hammer.noten);
        panel.halte();
        Muster einer;
        einer.noten = {Note{0, 1, 57, 1.0}};
        REQUIRE(panel.spieleCreatorNoten(QStringLiteral("guitar"), einer, 0));
        CHECK(panel.spieler().loop().muster[0].noten == std::vector<Note>{Note{0, 4, 57, 1.0}});
        panel.halte();
    }

    // --- der Drum-Editor spielt einen Schlag einer Instrument-Spur an (Stufe 11c, Schritt 2):
    //     ein Muster aus dieser einen Spur mit einem Schlag am Anfang, mit ihrer Note und Length
    {
        DrumEditor* drumEditor = panel.editor();
        REQUIRE(drumEditor != nullptr);
        drumEditor->spurHinzu();
        const int spur = static_cast<int>(panel.loop().findeMuster(drums)->spuren.size()) - 1;
        REQUIRE(spur >= 0);
        CHECK_FALSE(drumEditor->spieleSpur(spur));  // noch ein Klang des Kits: Play gibt es nur am Instrument
        drumEditor->setzeSpurInstrument(spur, QStringLiteral("slap_bass"));
        drumEditor->setzeDauer(spur, 120);
        const Spur& gesetzt = panel.loop().findeMuster(drums)->spuren[static_cast<std::size_t>(spur)];
        CHECK(gesetzt.instrument == QStringLiteral("slap_bass"));
        CHECK(gesetzt.hoehe == 36);  // ein Bass spielt tief (K2 a)
        CHECK(gesetzt.dauerMs == 120);
        CHECK_FALSE(panel.spieleSpur(drums, spur + 1));
        CHECK_FALSE(panel.spieleSpur(noten, 0));  // ein Instrument-Muster hat keine Spuren
        CHECK_FALSE(panel.spieleSpur(QStringLiteral("gibt es nicht"), 0));
        REQUIRE(drumEditor->spieleSpur(spur));  // das Signal des Editors: das Panel spielt
        CHECK(panel.hoertVor());
        CHECK(panel.spieler().spielt());
        REQUIRE(panel.spieler().loop().muster.size() == 1);
        const Muster& schlag = panel.spieler().loop().muster[0];
        CHECK(schlag.art == MusterArt::Drumkit);
        REQUIRE(schlag.spuren.size() == 1);
        CHECK(schlag.spuren[0].spielt() == tonKlang(QStringLiteral("slap_bass"), 36, 120));
        REQUIRE(schlag.zeilen.size() == 1);
        CHECK(schlag.zeilen[0][0] == 1.0);
        CHECK(std::count_if(schlag.zeilen[0].begin(), schlag.zeilen[0].end(), [](double laut) { return laut > 0.0; }) == 1);
        panel.halte();
        CHECK_FALSE(panel.hoertVor());
        CHECK(panel.spieler().loop() == panel.loop());
    }

    // --- Play pattern im Instrument-Editor spielt nur das Muster
    auto* playMuster = editor->findChild<QPushButton*>(QStringLiteral("instrumentPlay"));
    REQUIRE(playMuster != nullptr);
    playMuster->click();
    CHECK(panel.soloMuster() == noten);
    CHECK(playMuster->text() == QStringLiteral("Stop"));
    playMuster->click();
    CHECK(panel.soloMuster().isEmpty());
    CHECK_FALSE(panel.spieler().spielt());
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
    spieler.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29d)
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

// Stufe 27e-3 (S139, Entscheide Patrik LL7 a, LL13 a): der Begrenzer am Mixer des Loops
TEST_CASE("Loop Limiter 27e-3: der Spieler haengt den Begrenzer an, wie der Loop es sagt")
{
    LoopPappEngine engine;
    SpielPlayer player;
    Spieler spieler;
    Loop loop = spielLoop({1, 0, 0, 0});
    CHECK(loop.limiter);  // Vorgabe an
    spieler.setzeLoop(loop);
    REQUIRE(spieler.starte(engine, player));
    REQUIRE_FALSE(engine.limiterAufrufe.empty());
    CHECK(engine.limiterAn);
    // aus: beim naechsten Stand des Loops, waehrend er spielt
    loop.limiter = false;
    spieler.setzeLoop(loop);
    CHECK_FALSE(engine.limiterAn);
    loop.limiter = true;
    spieler.setzeLoop(loop);
    CHECK(engine.limiterAn);
    spieler.halte(false);
    // ein Loop, der aus ist, startet ohne Begrenzer
    loop.limiter = false;
    spieler.setzeLoop(loop);
    REQUIRE(spieler.starte(engine, player));
    CHECK_FALSE(engine.limiterAn);
    spieler.halte(false);
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
    spieler.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29d)
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
    spieler.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29d)
    REQUIRE(engine.dekodiert.size() == 2);
    CHECK(engine.dekodiert[1] == QStringLiteral("C:/s/b.wav"));
    CHECK(spieler.bericht().isEmpty());

    // ein Klang, der sich nicht laden laesst: Bericht, die Spur schweigt
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}, QStringLiteral("C:/s/gibtsnicht.wav")));
    spieler.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29d)
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
    spieler.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29d)

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

TEST_CASE("Loop Spieler: die Spitze der Summe — der groesste Betrag seit der letzten Frage")
{
    LoopPappEngine engine;
    SpielPlayer player;
    Spieler spieler;
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));
    CHECK(spieler.nimmSpitze() == 0.0f);  // er spielt nicht
    REQUIRE(spieler.starte(engine, player));
    REQUIRE(engine.generator != nullptr);
    spieler.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29d)
    CHECK(spieler.nimmSpitze() == 0.0f);  // noch kein Block gerechnet
    CHECK(ersterWert(engine.generator) == 0.5f);
    CHECK(spieler.nimmSpitze() == 0.5f);
    CHECK(spieler.nimmSpitze() == 0.0f);  // die Frage setzt sie zurueck

    // zwei Spuren auf demselben Schlag addieren sich: die Summe geht ueber 1 (L6 a)
    Loop laut = spielLoop({1, 0, 0, 0});
    laut.muster[0].spuren.push_back({QStringLiteral("B"), QStringLiteral("C:/s/b.wav"), 1.0, 0.0, false});
    laut.muster[0].spuren.push_back({QStringLiteral("C"), QStringLiteral("C:/s/c.wav"), 1.0, 0.0, false});
    laut.muster[0].zeilen = {{1, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}};
    spieler.setzeLoop(laut);
    REQUIRE(spieler.starte(engine, player));  // von vorn
    spieler.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29d)
    (void)ersterWert(engine.generator);
    CHECK(spieler.nimmSpitze() == doctest::Approx(1.5f));
}

TEST_CASE("Loop Panel: Play pattern spielt ein Muster allein, Play wieder den ganzen Loop")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;

    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QString beispiel = wurzel.filePath(QStringLiteral("asset/loop/drums_demo.lvloop"));
    REQUIRE(QFileInfo::exists(beispiel));

    ServiceContainer dienste;
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });
    IAudioPlayer* player = dienste.tryResolve<IAudioPlayer>();
    REQUIRE(player != nullptr);
    player->play();

    LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
    REQUIRE(panel.lade(beispiel));
    auto* play = panel.findChild<QPushButton*>(QStringLiteral("loopPlay"));
    REQUIRE(play != nullptr);
    REQUIRE(panel.loop().muster.size() >= 2);
    const Muster basis = panel.loop().muster[0];

    CHECK_FALSE(panel.spieleMuster(QStringLiteral("gibt es nicht")));
    CHECK_FALSE(panel.spieler().spielt());

    REQUIRE(panel.spieleMuster(basis.name));
    CHECK(panel.spieler().spielt());
    CHECK(panel.soloMuster() == basis.name);
    CHECK(play->text() == QStringLiteral("Stop"));
    CHECK_FALSE(player->isPlaying());
    // gespielt wird ein Loop aus einem Kanal mit einem Clip dieses Musters, so lang wie das Muster
    const Loop& allein = panel.spieler().loop();
    REQUIRE(allein.muster.size() == 1);
    CHECK(allein.muster[0] == basis);
    REQUIRE(allein.kanaele.size() == 1);
    REQUIRE(allein.kanaele[0].clips.size() == 1);
    CHECK(allein.durchlaufTakte() == basis.takte);
    CHECK(allein.bpm == panel.loop().bpm);
    CHECK(panel.loop().kanaele.size() == 2);  // der Loop des Panels bleibt, wie er ist

    // Play im Panel haelt an; danach spielt Play den ganzen Loop
    play->click();
    CHECK_FALSE(panel.spieler().spielt());
    CHECK(panel.soloMuster().isEmpty());
    CHECK(panel.spieler().loop() == panel.loop());
    CHECK(player->isPlaying());
    REQUIRE(panel.spieleMuster(basis.name));
    REQUIRE(panel.spiele());
    CHECK(panel.soloMuster().isEmpty());
    CHECK(panel.spieler().loop() == panel.loop());
    panel.halte();
}

// =============================================================================
// Stufe 7: ein Loop als Clip im Audio-Kanal des Composers (Loop_Konzept §4e)
// =============================================================================

namespace
{

/// Eine Komposition mit einem Audio-Kanal und einem Clip dieser Datei.
lumi::composer::Komposition mitClip(const QString& datei, std::int64_t start, std::int64_t ende,
                                    std::int64_t einsatz = 0)
{
    lumi::composer::Clip clip;
    clip.datei = datei;
    clip.startMs = start;
    clip.endeMs = ende;
    clip.einsatzMs = einsatz;
    lumi::composer::Kanal kanal;
    kanal.art = lumi::composer::KanalArt::Audio;
    kanal.name = QStringLiteral("A1");
    kanal.clips = {clip};
    lumi::composer::Komposition k;
    k.kanaele = {kanal};
    return k;
}

/// Die Stelle des Klangerzeugers im Durchlauf, in Millisekunden.
double stelleMs(const LoopPappEngine& engine)
{
    const auto* erzeuger = dynamic_cast<const Klangerzeuger*>(engine.generator);
    REQUIRE(erzeuger != nullptr);
    return 1000.0 * static_cast<double>(erzeuger->stelleImDurchlauf()) / engine.getSampleRate();
}

}  // namespace

TEST_CASE("Loop im Composer: ein Loop-Clip ist ein Klangerzeuger im Mixer, wiederholt sich und zaehlt als Song")
{
    using lumi::composer::Tonfuehrung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    // 4 Takte zu einem Schlag, 4 Schritte je Schlag, 120 BPM: ein Durchlauf dauert 2000 ms
    Loop loop = spielLoop({1, 0, 0, 0});
    loop.tags.titel = QStringLiteral("Mein Beat");
    loop.tags.kuenstler = QStringLiteral("Ich");
    loop.tags.album = QStringLiteral("Erste Loops");
    loop.tags.jahr = QStringLiteral("2026");
    loop.tags.text = QStringLiteral("la la");
    const QString pfad = ordner.filePath(QStringLiteral("beat.lvloop"));
    REQUIRE(speichereLoop(loop, pfad));

    LoopPappEngine engine;
    SpielPlayer player;
    Tonfuehrung fuehrung;
    REQUIRE(fuehrung.uebernimm(engine, player));
    // der Clip laeuft von 1000 bis 6000: zweieinhalb Durchlaeufe
    const lumi::composer::Komposition k = mitClip(pfad, 1000, 6000);

    SUBCASE("Vorlauf, Einstieg, Sprung ueber den Durchlauf hinaus, Ende")
    {
        // im Vorlauf: die Quelle steht schon im Mixer, angehalten — einen Song meldet noch niemand
        Tonfuehrung::Ergebnis e = fuehrung.takt(k, 0, true);
        fuehrung.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29e)
        CHECK(fuehrung.quellen() == 1);
        CHECK(fuehrung.loops() == 1);
        REQUIRE(engine.generator != nullptr);
        CHECK(engine.stream(engine.generatorQuelle).haelt);
        CHECK_FALSE(e.hatSong);
        CHECK(engine.zahl(QStringLiteral("generator")) == 1);
        CHECK(engine.dekodiert == QStringList{QStringLiteral("C:/s/a.wav")});  // sein Klang ist geladen

        // der Einstieg: sie laeuft, von vorn — und ist der laufende Song, mit den Tags der Datei
        e = fuehrung.takt(k, 1000, true);
        CHECK_FALSE(engine.stream(engine.generatorQuelle).haelt);
        CHECK(stelleMs(engine) == doctest::Approx(0.0));
        REQUIRE(e.hatSong);
        REQUIRE(e.songGewechselt);
        CHECK(e.song.filePath == pfad);
        CHECK(e.song.title == QStringLiteral("Mein Beat"));
        CHECK(e.song.artist == QStringLiteral("Ich"));
        CHECK(e.song.album == QStringLiteral("Erste Loops"));
        CHECK(e.song.year == QStringLiteral("2026"));
        CHECK(e.song.lyricsPlain == QStringLiteral("la la"));
        CHECK(e.song.durationMs == 2000);
        CHECK(e.songDauerMs == 2000);
        CHECK(e.songStelleMs == 0);
        CHECK(e.songLaeuft);

        // derselbe Song im naechsten Takt: nicht noch einmal gemeldet
        e = fuehrung.takt(k, 1033, true);
        CHECK(e.hatSong);
        CHECK_FALSE(e.songGewechselt);

        // ein Sprung der Uhr in den zweiten Durchlauf: 3500 ms im Clip sind 1500 ms im Loop
        e = fuehrung.takt(k, 4500, true);
        CHECK(stelleMs(engine) == doctest::Approx(1500.0).epsilon(0.001));
        CHECK(e.songStelleMs == 1500);
        CHECK(e.songDauerMs == 2000);
        CHECK(fuehrung.nachgesetzt() == 0);  // ein Sprung ist kein Davonlaufen

        // hinter dem Clip: die Quelle geht
        e = fuehrung.takt(k, 6100, true);
        CHECK(fuehrung.quellen() == 0);
        CHECK(engine.generator == nullptr);
        CHECK_FALSE(e.hatSong);
    }

    SUBCASE("laeuft der Klangerzeuger der Uhr davon, wird er nachgesetzt wie ein Song (T1 a)")
    {
        (void)fuehrung.takt(k, 1000, true);
        // die Engine aus Pappe rechnet nichts: der Klangerzeuger bleibt stehen, die Uhr laeuft
        for (std::int64_t stelle = 1033; stelle <= 1490; stelle += 33) (void)fuehrung.takt(k, stelle, true);
        CHECK(fuehrung.nachgesetzt() == 0);  // in der Schonfrist wird nicht geprueft
        for (std::int64_t stelle = 1500; stelle <= 1800; stelle += 33) (void)fuehrung.takt(k, stelle, true);
        CHECK(fuehrung.nachgesetzt() >= 1);
        // gesetzt wurde auf die Stelle der Uhr im Loop
        CHECK(stelleMs(engine) > 500.0);
        CHECK(stelleMs(engine) <= 800.0);
    }

    SUBCASE("der Einsatz: der Clip beginnt mitten im Durchlauf")
    {
        const lumi::composer::Komposition mitEinsatz = mitClip(pfad, 1000, 6000, 500);
        Tonfuehrung::Ergebnis e = fuehrung.takt(mitEinsatz, 1000, true);
        CHECK(stelleMs(engine) == doctest::Approx(500.0).epsilon(0.001));
        CHECK(e.songStelleMs == 500);
        // nach 1700 ms im Clip: 2200 ms im Loop, also 200 ms im zweiten Durchlauf
        e = fuehrung.takt(mitEinsatz, 1000, false);
        e = fuehrung.takt(mitEinsatz, 2700, false);
        CHECK(stelleMs(engine) == doctest::Approx(200.0).epsilon(0.001));
        CHECK(e.songStelleMs == 200);
        CHECK_FALSE(e.songLaeuft);  // die Uhr steht
    }

    SUBCASE("hat das Loop-Panel die Datei offen, spielt der Clip dessen Fassung und folgt ihr (L7 a)")
    {
        Tonfuehrung::Ergebnis e = fuehrung.takt(k, 1000, true);
        fuehrung.warteAufToene();  // die Klaenge kommen aus dem Hintergrund (Stufe 29e)
        REQUIRE(e.songGewechselt);
        e = fuehrung.takt(k, 1400, true);
        CHECK_FALSE(e.songGewechselt);

        // das Panel meldet eine Fassung mit anderem Titel und doppeltem Tempo (Durchlauf 1000 ms)
        Loop offen = loop;
        offen.tags.titel = QStringLiteral("Neuer Titel");
        offen.bpm = 240.0;
        meldeOffen(pfad, offen);
        e = fuehrung.takt(k, 1433, true);
        REQUIRE(e.songGewechselt);  // er heisst jetzt anders: neu gemeldet
        CHECK(e.song.title == QStringLiteral("Neuer Titel"));
        CHECK(e.song.durationMs == 1000);
        CHECK(e.songDauerMs == 1000);
        // dieselbe Stelle der Uhr liegt woanders im Loop: 433 ms im Clip sind 433 ms im kuerzeren Durchlauf
        CHECK(stelleMs(engine) == doctest::Approx(433.0).epsilon(0.01));
        CHECK(engine.zahl(QStringLiteral("generator")) == 1);  // dieselbe Quelle, kein neuer Klangerzeuger
        CHECK(engine.dekodiert.size() == 1);                   // die Klaenge sind dieselben: nichts neu geladen

        // eine Meldung ohne Aenderung, und eine fuer eine andere Datei: nichts geschieht
        meldeOffen(pfad, offen);
        e = fuehrung.takt(k, 1466, true);
        CHECK_FALSE(e.songGewechselt);
        Loop fremd = loop;
        fremd.tags.titel = QStringLiteral("Fremd");
        meldeOffen(ordner.filePath(QStringLiteral("anderer.lvloop")), fremd);
        e = fuehrung.takt(k, 1500, true);
        CHECK_FALSE(e.songGewechselt);
        meldeOffen({}, {});  // aufraeumen: die Ablage gilt fuer das ganze Programm

        // eine neue Quelle (nach einem Ende) liest wieder die Datei
        (void)fuehrung.takt(k, 9000, true);
        CHECK(engine.generator == nullptr);
        e = fuehrung.takt(k, 2000, true);
        REQUIRE(e.songGewechselt);
        CHECK(e.song.title == QStringLiteral("Mein Beat"));
        CHECK(e.song.durationMs == 2000);
    }

    SUBCASE("eine Datei, die kein Loop ist oder fehlt: ein Versuch, dann Stille")
    {
        const lumi::composer::Komposition fehlt = mitClip(ordner.filePath(QStringLiteral("fehlt.lvloop")), 0, 4000);
        Tonfuehrung::Ergebnis e = fuehrung.takt(fehlt, 100, true);
        CHECK(fuehrung.quellen() == 0);
        CHECK(fuehrung.loops() == 0);
        CHECK(engine.generator == nullptr);
        e = fuehrung.takt(fehlt, 133, true);
        CHECK(engine.zahl(QStringLiteral("generator")) == 0);
    }

    SUBCASE("Stufe 29e: fehlen Toene, schweigen nur sie (D); nach einer Aenderung klingt das Bisherige (C)")
    {
        // D: der Clip beginnt, sein Klang wird noch gerechnet — der Klangerzeuger laeuft, still
        engine.halten = true;
        (void)fuehrung.takt(k, 1000, true);
        Tonfuehrung::Vorbereitung v = fuehrung.vorbereitung();
        CHECK(v.gesamt == 1);
        CHECK(v.erledigt == 0);
        CHECK(v.luecken);
        REQUIRE(engine.generator != nullptr);
        CHECK(ersterWert(engine.generator) == 0.0f);
        engine.halten = false;
        fuehrung.warteAufToene();
        CHECK(fuehrung.vorbereitung().gesamt == 0);

        // C: das Panel meldet eine Fassung mit einem anderen Klang — bis er da ist, klingt der bisherige
        Loop offen = loop;
        offen.muster[0].spuren[0].klang = QStringLiteral("C:/s/b.wav");
        engine.halten = true;
        meldeOffen(pfad, offen);
        (void)fuehrung.takt(k, 1033, true);
        v = fuehrung.vorbereitung();
        CHECK(v.gesamt == 1);
        CHECK_FALSE(v.luecken);
        engine.halten = false;
        fuehrung.warteAufToene();
        CHECK(fuehrung.vorbereitung().gesamt == 0);
        CHECK(engine.dekodiert.contains(QStringLiteral("C:/s/b.wav")));
        meldeOffen({}, {});
    }

    SUBCASE("ohne Titel in den Tags gilt der Dateiname")
    {
        Loop ohne = spielLoop({1, 0, 0, 0});
        const QString namenlos = ordner.filePath(QStringLiteral("Drums am Abend.lvloop"));
        REQUIRE(speichereLoop(ohne, namenlos));
        const Tonfuehrung::Ergebnis e = fuehrung.takt(mitClip(namenlos, 0, 4000), 0, true);
        REQUIRE(e.songGewechselt);
        CHECK(e.song.title == QStringLiteral("Drums am Abend"));
        CHECK(e.song.artist.isEmpty());
    }

    fuehrung.gibFrei();
    CHECK(engine.generator == nullptr);
}

TEST_CASE("S127 7a Loop Panel: Play des ganzen Loops meldet ihn als laufenden Song, Play pattern nicht, Stop gibt zurueck")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());

    // ein Loop mit Tags und zwei Mustern
    Loop loop = spielLoop({1, 0, 0, 0});
    loop.tags.titel = QStringLiteral("Mein Beat");
    loop.tags.kuenstler = QStringLiteral("Ich");
    loop.tags.text = QStringLiteral("la la");
    Muster zwei = loop.muster[0];
    zwei.name = QStringLiteral("N");
    loop.muster.push_back(zwei);
    const QString pfad = ordner.filePath(QStringLiteral("beat.lvloop"));
    REQUIRE(speichereLoop(loop, pfad));

    ServiceContainer dienste;
    dienste.registerSingleton<IEventBus>([](ServiceContainer&) { return std::make_unique<EventBus>(); });
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });
    IEventBus* bus = dienste.tryResolve<IEventBus>();
    REQUIRE(bus != nullptr);
    std::vector<TrackInfo> songs;
    std::vector<int> stellen;
    std::vector<PlaybackState> zustaende;
    const auto aboSong = bus->subscribeScoped<TrackChangedEvent>([&](const TrackChangedEvent& e) { songs.push_back(e.track); });
    const auto aboStelle =
        bus->subscribeScoped<PlaybackPositionEvent>([&](const PlaybackPositionEvent& e) { stellen.push_back(e.positionMs); });
    const auto aboZustand =
        bus->subscribeScoped<PlaybackStateEvent>([&](const PlaybackStateEvent& e) { zustaende.push_back(e.state); });
    const auto takte = [] {
        QElapsedTimer uhr;
        uhr.start();
        while (uhr.elapsed() < 150) QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    };

    LoopPanel panel(dienste);  // nicht gezeigt: der Takt laeuft trotzdem
    REQUIRE(panel.lade(pfad));
    takte();
    CHECK(songs.empty());  // nichts spielt: nichts gemeldet
    CHECK(laufenderLoop() == nullptr);

    // --- Play: der Loop ist der laufende Song, mit seinen Tags; „laeuft", die Stelle
    REQUIRE(panel.spiele());
    takte();
    REQUIRE(songs.size() == 1);
    CHECK(songs[0].filePath == pfad);
    CHECK(songs[0].title == QStringLiteral("Mein Beat"));
    CHECK(songs[0].artist == QStringLiteral("Ich"));
    CHECK(songs[0].lyricsPlain == QStringLiteral("la la"));
    CHECK(songs[0].durationMs == static_cast<int>(std::lround(panel.loop().durchlaufMs())));
    REQUIRE(laufenderLoop() != nullptr);
    CHECK(laufenderLoop()->title == QStringLiteral("Mein Beat"));
    // das Player-Panel, erst jetzt sichtbar, zeigt ihn als „Loop - <Titel>"
    PlayerPanel spielerPanel(dienste);
    testfenster::platziere(spielerPanel);
    spielerPanel.show();
    QCoreApplication::processEvents();
    const auto loopTitel = [&]() -> QString {
        for (const QLabel* l : spielerPanel.findChildren<QLabel*>())
            if (l->text().startsWith(QStringLiteral("Loop - "))) return l->text();
        return {};
    };
    CHECK(loopTitel() == QStringLiteral("Loop - Mein Beat"));
    REQUIRE_FALSE(zustaende.empty());
    CHECK(zustaende.back() == PlaybackState::Playing);
    CHECK_FALSE(stellen.empty());
    // ein Sprung: die Stelle kommt mit
    panel.setzeStartstelle(8);  // 8 von 16 Schritten = die Mitte des Durchlaufs
    takte();
    CHECK(stellen.back() == static_cast<int>(std::lround(panel.loop().durchlaufMs() / 2.0)));
    CHECK(songs.size() == 1);  // derselbe Song: nicht noch einmal

    // --- andere Tags: neu gemeldet
    LoopTags neueTags = panel.loop().tags;
    neueTags.titel = QStringLiteral("Anders");
    panel.setzeTags(neueTags);
    takte();
    REQUIRE(songs.size() == 2);
    CHECK(songs[1].title == QStringLiteral("Anders"));
    CHECK(loopTitel() == QStringLiteral("Loop - Anders"));  // das Panel hoert am Bus mit

    // --- Play pattern: Vorhoeren — der Song des Players gilt wieder
    REQUIRE(panel.spieleMuster(QStringLiteral("N")));
    takte();
    REQUIRE(songs.size() == 3);
    CHECK(songs[2].filePath == QStringLiteral("song"));  // der Titel des Players
    CHECK(laufenderLoop() == nullptr);
    CHECK(zustaende.back() == PlaybackState::Paused);

    // --- wieder Play, dann Stop: zurueck zum Player
    REQUIRE(panel.spiele());
    takte();
    REQUIRE(songs.size() == 4);
    CHECK(songs[3].title == QStringLiteral("Anders"));
    panel.halte();
    takte();
    REQUIRE(songs.size() == 5);
    CHECK(songs[4].filePath == QStringLiteral("song"));
    CHECK(laufenderLoop() == nullptr);
    CHECK(loopTitel().isEmpty());  // wieder der Titel des Players

    // --- ein Loop ohne Datei meldet sich nicht
    panel.neu();
    const std::size_t vorher = songs.size();
    if (panel.spiele()) takte();
    CHECK(songs.size() == vorher);
    CHECK(laufenderLoop() == nullptr);
    panel.halte();
}

TEST_CASE("S127 Loop 3b im Composer: ein Loop mit Effekten — der Klangerzeuger des Clips traegt sie")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};  // die mitgelieferten Instrumente
    using namespace lumi::composer;
    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QString pfad = wurzel.filePath(QStringLiteral("asset/loop/effects_demo.lvloop"));
    Loop mitFx;
    REQUIRE(ladeLoop(pfad, mitFx));
    // die Stelle, an der ein Clip mit Effekten spielt
    int startTakt = -1;
    for (const auto& kanal : mitFx.kanaele)
        for (const LoopClip& c : kanal.clips)
            if (c.effekteWirken() && startTakt < 0) startTakt = c.startTakt;
    REQUIRE(startTakt >= 0);
    const double msJeTakt = mitFx.durchlaufMs() / mitFx.durchlaufTakte();

    LoopPappEngine engine;
    SpielPlayer player;
    Tonfuehrung fuehrung;
    REQUIRE(fuehrung.uebernimm(engine, player));
    const auto bei = static_cast<std::int64_t>(std::llround(startTakt * msJeTakt));
    const Komposition k = mitClip(pfad, 0, 120000);
    (void)fuehrung.takt(k, bei, true);
    // die Klaenge kommen aus dem Hintergrund (Stufe 29e); erst der naechste Takt schaltet um —
    // ohne das spielte der Clip still, wenn kein Fall davor den Tonspeicher gefuellt hat
    fuehrung.warteAufToene();
    (void)fuehrung.takt(k, bei, true);
    REQUIRE(fuehrung.loops() == 1);
    const auto* erzeuger = dynamic_cast<const Klangerzeuger*>(engine.generator);
    REQUIRE(erzeuger != nullptr);
    const auto programm = erzeuger->programm();
    REQUIRE(programm != nullptr);
    CHECK(programm->effekte != nullptr);  // die Effekte der Clips sind im Programm des Composers
    CHECK_FALSE(programm->ablauf.busse.empty());
    // er rechnet: an der Stelle eines Clips mit Effekten kommt Ton heraus, und nichts ist kaputt
    std::vector<float> block(2 * 4096, 0.0f);
    float spitze = 0.0f;
    int kaputt = 0;
    for (int i = 0; i < 20; ++i)
    {
        engine.generator->render(block.data(), 4096);
        for (float x : block)
        {
            if (!std::isfinite(x)) ++kaputt;
            spitze = std::max(spitze, std::abs(x));
        }
    }
    CHECK(kaputt == 0);
    CHECK(spitze > 0.0f);
    fuehrung.gibFrei();
}

TEST_CASE("S127 W2 Loop im Composer: die Stelle je Loop fuer Panel und Editoren — der lauteste Clip, -1 ohne")
{
    using namespace lumi::composer;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString pfad = ordner.filePath(QStringLiteral("beat.lvloop"));
    REQUIRE(speichereLoop(spielLoop({1, 0, 0, 0}), pfad));  // ein Durchlauf 2000 ms

    // A1: der Loop 1000–6000 auf vollem Pegel; A2: derselbe Loop 3000–8000, ab 300 ms, halb so laut
    Komposition k = mitClip(pfad, 1000, 6000);
    lumi::composer::Kanal zweiter = k.kanaele[0];
    zweiter.name = QStringLiteral("A2");
    zweiter.clips[0].startMs = 3000;
    zweiter.clips[0].endeMs = 8000;
    zweiter.clips[0].einsatzMs = 300;
    zweiter.clips[0].grundpegel = 0.5;
    k.kanaele.push_back(zweiter);

    LoopPappEngine engine;
    SpielPlayer player;
    Tonfuehrung fuehrung;
    CHECK(fuehrung.loopStelleMs(pfad) == -1.0);  // fuehrt noch nicht
    REQUIRE(fuehrung.uebernimm(engine, player));
    (void)fuehrung.takt(k, 0, true);
    CHECK(fuehrung.loopStelleMs(pfad) == -1.0);  // im Vorlauf: noch kein Clip an der Stelle
    (void)fuehrung.takt(k, 2500, true);
    CHECK(fuehrung.loopStelleMs(pfad) == doctest::Approx(1500.0));
    // die Schreibweise des Pfads zaehlt nicht
    CHECK(fuehrung.loopStelleMs(QDir::toNativeSeparators(pfad)) == doctest::Approx(1500.0));
    CHECK(fuehrung.loopStelleMs(ordner.filePath(QStringLiteral("anderer.lvloop"))) == -1.0);
    CHECK(fuehrung.loopStelleMs({}) == -1.0);

    // beide spielen: A1 bei 2500 im Clip = 500 im Durchlauf, A2 bei 500 + 300 = 800 — A1 ist lauter
    (void)fuehrung.takt(k, 3500, true);
    CHECK(fuehrung.loopStelleMs(pfad) == doctest::Approx(500.0));
    // S130 W8 a: beide Stellen, der lauteste zuerst
    REQUIRE(fuehrung.loopStellenMs(pfad).size() == 2);
    CHECK(fuehrung.loopStellenMs(pfad)[0] == doctest::Approx(500.0));
    CHECK(fuehrung.loopStellenMs(pfad)[1] == doctest::Approx(800.0));
    // jetzt ist A2 lauter
    k.kanaele[0].clips[0].grundpegel = 0.4;
    k.kanaele[1].clips[0].grundpegel = 1.0;
    (void)fuehrung.takt(k, 3533, true);
    CHECK(fuehrung.loopStelleMs(pfad) == doctest::Approx(833.0));
    REQUIRE(fuehrung.loopStellenMs(pfad).size() == 2);
    CHECK(fuehrung.loopStellenMs(pfad)[1] == doctest::Approx(533.0));
    // das Gain zaehlt nicht (W2 a: die Pegelkurve)
    k.kanaele[0].clips[0].gainDb = 24.0;
    (void)fuehrung.takt(k, 3566, true);
    CHECK(fuehrung.loopStelleMs(pfad) == doctest::Approx(866.0));
    // nur noch A2: hinter dem Ende von A1
    (void)fuehrung.takt(k, 7000, true);
    CHECK(fuehrung.loopStelleMs(pfad) == doctest::Approx(300.0));  // 4000 + 300 = 4300 → 300
    CHECK(fuehrung.loopStellenMs(pfad).size() == 1);
    // dahinter keiner, und frei gegeben auch nicht
    (void)fuehrung.takt(k, 9000, true);
    CHECK(fuehrung.loopStelleMs(pfad) == -1.0);
    (void)fuehrung.takt(k, 2500, true);
    fuehrung.gibFrei();
    CHECK(fuehrung.loopStelleMs(pfad) == -1.0);
}

TEST_CASE("S127 W1 W3 Loop Panel: die Marke in den Editoren beim Play des Arrangements, Scroll with playhead")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;

    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QString beispiel = wurzel.filePath(QStringLiteral("asset/loop/drums_demo.lvloop"));
    REQUIRE(QFileInfo::exists(beispiel));

    ServiceContainer dienste;
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });

    LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
    REQUIRE(panel.lade(beispiel));
    const Loop& loop = panel.loop();
    const int spt = loop.schritteJeTakt();
    // drums_demo: in Drums Grund (Takt 1–4), Fill (5–8), Grund (9–12), Break (13–16)
    const LoopClip& zweiterGrund = loop.kanaele[0].clips[2];
    REQUIRE(zweiterGrund.muster == loop.kanaele[0].clips[0].muster);
    const QString grund = zweiterGrund.muster;
    const QString fill = loop.kanaele[0].clips[1].muster;
    REQUIRE(panel.oeffneEditor(grund, false));
    DrumEditor* editor = panel.editor();
    REQUIRE(editor != nullptr);
    auto* play = editor->findChild<QPushButton*>(QStringLiteral("drumPlay"));

    // der Haken: da, Vorgabe an, gemerkt
    auto* mitlaufen = panel.findChild<QCheckBox*>(QStringLiteral("loopScrollWithPlayhead"));
    REQUIRE(mitlaufen != nullptr);
    CHECK(mitlaufen->isChecked());
    mitlaufen->setChecked(false);
    CHECK_FALSE(QSettings().value(QStringLiteral("loop/scrollWithPlayhead")).toBool());
    mitlaufen->setChecked(true);

    // nichts spielt: keine Marke
    panel.zeigeStandJetzt();
    CHECK(editor->raster()->marke() < 0.0);

    // Play des ganzen Loops, die Stelle in Takt 10, Schritt 4: im zweiten Clip Grund
    REQUIRE(panel.spiele());
    const int stelle = (zweiterGrund.startTakt + 1) * spt + 3;
    panel.setzeStartstelle(stelle);
    panel.zeigeStandJetzt();
    CHECK(panel.spieler().stelleSchritte() == doctest::Approx(stelle));
    CHECK(panel.taktachse()->spielt());
    // die Kaskade (W1 a, Zusatz Patrik): der Editor zeigt die Stelle im Muster, ohne Play pattern
    CHECK(editor->raster()->marke() == doctest::Approx(spt + 3));
    if (play != nullptr) CHECK(play->text() == QStringLiteral("Play pattern"));
    CHECK(panel.taktachse()->mitlaufen() != lumi::ui::Achse::Mitlaufen::Aus);
    // ein anderes Muster im Editor: an dieser Stelle spielt es nicht
    REQUIRE(panel.oeffneEditor(fill, false));
    panel.zeigeStandJetzt();
    CHECK(editor->raster()->marke() < 0.0);
    // ohne Haken rollt die Achse nicht mit
    mitlaufen->setChecked(false);
    panel.zeigeStandJetzt();
    CHECK(panel.taktachse()->mitlaufen() == lumi::ui::Achse::Mitlaufen::Aus);
    panel.halte();
    panel.zeigeStandJetzt();
    CHECK(editor->raster()->marke() < 0.0);
}

TEST_CASE("S130 W8 a Loop Panel: spielt der Composer denselben Loop mehrfach, die leiseren Clips blasser — in Achse und Editor")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;

    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QString beispiel = wurzel.filePath(QStringLiteral("asset/loop/drums_demo.lvloop"));
    REQUIRE(QFileInfo::exists(beispiel));

    ServiceContainer dienste;
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });

    LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
    REQUIRE(panel.lade(beispiel));
    const Loop& loop = panel.loop();
    const int spt = loop.schritteJeTakt();
    // drums_demo: in Drums Grund (Takt 1–4), Fill (5–8), Grund (9–12), Break (13–16)
    const QString grund = loop.kanaele[0].clips[0].muster;
    REQUIRE(loop.kanaele[0].clips[2].muster == grund);
    REQUIRE(panel.oeffneEditor(grund, false));
    DrumEditor* editor = panel.editor();
    REQUIRE(editor != nullptr);

    // der Composer spielt diesen Loop dreimal zugleich: der lauteste in Takt 10, Schritt 4
    // (zweiter Grund), leiser in Takt 2, Schritt 2 (erster Grund) und in Takt 6 (Fill)
    const double msJeSchritt = 1000.0 / loop.schritteVonMs(1000.0);
    const double lautester = 9 * spt + 3;
    const double leiserGrund = 1 * spt + 1;
    const double leiserFill = 5 * spt;
    const QString datei = panel.datei();
    lumi::loop::setzeComposerAbfragen(
        {[] { return true; },
         [&](const QString& gefragt) {
             return gefragt == datei ? std::vector<double>{lautester * msJeSchritt, leiserGrund * msJeSchritt,
                                                           leiserFill * msJeSchritt}
                                     : std::vector<double>{};
         },
         [] { return false; }});
    panel.zeigeStandJetzt();

    // die Achse: die Marke beim lautesten, die beiden anderen blasser
    CHECK(panel.taktachse()->spielt());
    const std::vector<double>& neben = panel.taktachse()->nebenStellenSchritte();
    REQUIRE(neben.size() == 2);
    CHECK(neben[0] == doctest::Approx(leiserGrund));
    CHECK(neben[1] == doctest::Approx(leiserFill));
    // der Editor (Grund): die Marke im zweiten Clip Grund, blass die Stelle im ersten — Fill liegt nicht in Grund
    CHECK(editor->raster()->marke() == doctest::Approx(spt + 3));
    REQUIRE(editor->raster()->nebenMarken().size() == 1);
    CHECK(editor->raster()->nebenMarken()[0] == doctest::Approx(spt + 1));

    const QString bild = qEnvironmentVariable("LUMIVIZ_LOOP_W8_BILD");
    if (!bild.isEmpty())
    {
        panel.taktachse()->resize(900, 220);
        CHECK(panel.taktachse()->grab().save(bild));
        editor->resize(900, 360);
        CHECK(editor->grab().save(QFileInfo(bild).path() + QLatin1Char('/') + QFileInfo(bild).completeBaseName() +
                                  QStringLiteral("_editor.png")));
    }

    // spielt er ihn nur noch einmal: keine blassen Marken mehr
    lumi::loop::setzeComposerAbfragen(
        {[] { return true; },
         [&](const QString&) { return std::vector<double>{lautester * msJeSchritt}; },
         [] { return false; }});
    panel.zeigeStandJetzt();
    CHECK(panel.taktachse()->nebenStellenSchritte().empty());
    CHECK(editor->raster()->nebenMarken().empty());
    CHECK(editor->raster()->marke() == doctest::Approx(spt + 3));
    // und gar nicht: keine Marke
    lumi::loop::setzeComposerAbfragen({});
    panel.zeigeStandJetzt();
    CHECK_FALSE(panel.taktachse()->spielt());
    CHECK(editor->raster()->marke() < 0.0);
    CHECK(editor->raster()->nebenMarken().empty());
}

TEST_CASE("S130 W8-2 a Loop Panel: liegt ein Muster zugleich in zwei Kanaelen, zeigt der Editor beide Stellen — der zuletzt begonnene Clip voll")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;
    lumi::loop::setzeComposerAbfragen({});  // der Composer spielt nicht

    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QString beispiel = wurzel.filePath(QStringLiteral("asset/loop/drums_demo.lvloop"));
    REQUIRE(QFileInfo::exists(beispiel));

    ServiceContainer dienste;
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });

    // wie im Sichttest 11 (beat_A): Grund in Drums ab Takt 1, dazu in Percussion ab Takt 2
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    Loop vorlage;
    REQUIRE(lumi::loop::ladeLoop(beispiel, vorlage));
    const QString grund = vorlage.kanaele[0].clips[0].muster;
    REQUIRE(vorlage.kanaele.size() >= 2);
    vorlage.kanaele[1].clips.push_back({grund, 1, 1});
    vorlage.richte();
    const QString datei = ordner.filePath(QStringLiteral("doppelt.lvloop"));
    REQUIRE(lumi::loop::speichereLoop(vorlage, datei));

    LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
    REQUIRE(panel.lade(datei));
    const int spt = panel.loop().schritteJeTakt();
    REQUIRE(panel.oeffneEditor(grund, false));
    DrumEditor* editor = panel.editor();
    REQUIRE(editor != nullptr);

    // Play des ganzen Loops, Takt 3, Schritt 13: in Drums Takt 3 des Musters, in Percussion Takt 2
    REQUIRE(panel.spiele());
    panel.setzeStartstelle(2 * spt + 12);
    panel.zeigeStandJetzt();
    // (der Spieler laeuft zwischen Setzen und Fragen ein paar Mikrosekunden weiter)
    CHECK(editor->raster()->marke() == doctest::Approx(spt + 12).epsilon(0.01));  // Percussion, zuletzt begonnen
    REQUIRE(editor->raster()->nebenMarken().size() == 1);
    CHECK(editor->raster()->nebenMarken()[0] == doctest::Approx(2 * spt + 12).epsilon(0.01));  // Drums, blasser
    // in Takt 1 liegt nur der Clip in Drums: eine Marke
    panel.setzeStartstelle(5);
    panel.zeigeStandJetzt();
    CHECK(editor->raster()->marke() == doctest::Approx(5).epsilon(0.01));
    CHECK(editor->raster()->nebenMarken().empty());
    panel.halte();
    panel.zeigeStandJetzt();
    CHECK(editor->raster()->marke() < 0.0);
    CHECK(editor->raster()->nebenMarken().empty());
}

TEST_CASE("Loop im Composer: ein Loop-Clip hat kein Ende — die Komposition laesst ihm seine Laenge")
{
    using namespace lumi::composer;
    // ein Loop und ein Song, beide laenger als ihre Quelle (2000 ms)
    Komposition k = mitClip(QStringLiteral("C:/x/beat.lvloop"), 0, 9000);
    Clip song;
    song.datei = QStringLiteral("C:/x/lied.mp3");
    song.startMs = 10000;
    song.endeMs = 19000;
    k.kanaele[0].clips.push_back(song);
    CHECK(istLoop(QStringLiteral("C:/x/beat.lvloop")));
    CHECK(istLoop(QStringLiteral("C:/x/BEAT.LVLOOP")));
    CHECK_FALSE(istLoop(QStringLiteral("C:/x/lied.mp3")));

    k.ergaenzeTon([](const QString&) -> std::int64_t { return 2000; });
    // der Song wird auf seine Laenge gekuerzt, der Loop bleibt und traegt keine Song-Laenge
    CHECK(k.kanaele[0].clips[0].endeMs == 9000);
    CHECK(k.kanaele[0].clips[0].quellMs == 0);
    CHECK(k.kanaele[0].clips[1].endeMs == 12000);
    CHECK(k.kanaele[0].clips[1].quellMs == 2000);

    // ein neuer Loop-Clip ist einen Durchlauf lang und laesst sich beliebig laenger ziehen
    lumi::composer::Kanal leer;
    leer.art = KanalArt::Audio;
    k.kanaele.push_back(leer);
    REQUIRE(fuegeSongEin(k, 1, QStringLiteral("C:/x/beat.lvloop"), 3000, 2000) == 0);
    CHECK(k.kanaele[1].clips[0].startMs == 3000);
    CHECK(k.kanaele[1].clips[0].endeMs == 5000);
    CHECK(k.kanaele[1].clips[0].quellMs == 0);
    REQUIRE(zieheKante(k, 1, 0, false, 30000) == 0);
    CHECK(k.kanaele[1].clips[0].endeMs == 30000);
    // ein Song daneben haelt an seiner Laenge an
    REQUIRE(fuegeSongEin(k, 1, QStringLiteral("C:/x/lied.mp3"), 40000, 2000) == 1);
    REQUIRE(zieheKante(k, 1, 1, false, 60000) == 1);
    CHECK(k.kanaele[1].clips[1].endeMs == 42000);
    k.kanaele.pop_back();

    // ueber die Datei: dasselbe
    const QJsonObject json = alsJson(k);
    Komposition gelesen;
    REQUIRE(ausJson(json, gelesen, {}, nullptr, [](const QString&) -> std::int64_t { return 2000; }));
    REQUIRE(gelesen.kanaele.size() == 1);
    REQUIRE(gelesen.kanaele[0].clips.size() == 2);
    CHECK(gelesen.kanaele[0].clips[0].endeMs == 9000);
    CHECK(gelesen.kanaele[0].clips[0].quellMs == 0);
}

// =============================================================================
// Stufe 29d: Play wartet auf die Toene (H2 a, TV5 a, TV6 a)
// =============================================================================

TEST_CASE("Loop Spieler 29d: Play wartet still, bis die Toene da sind — Stop bricht ab")
{
    LoopPappEngine engine;
    SpielPlayer player;
    Spieler spieler;
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));
    engine.halten = true;
    REQUIRE(spieler.starte(engine, player));
    CHECK(spieler.spielt());
    CHECK(spieler.wartet());
    CHECK(spieler.bereitet());
    CHECK(spieler.vorzubereiten() == 1);
    CHECK(spieler.vorbereitet() == 0);
    REQUIRE(engine.generator != nullptr);
    CHECK(ersterWert(engine.generator) == 0.0f);  // still, solange gerechnet wird
    CHECK(spieler.springe(2.0));                  // gemerkt: dort beginnt er
    spieler.takt();
    CHECK(spieler.wartet());

    engine.halten = false;
    spieler.warteAufToene();
    CHECK_FALSE(spieler.wartet());
    CHECK_FALSE(spieler.bereitet());
    CHECK(spieler.stelleSchritte() == doctest::Approx(2.0));
    REQUIRE(spieler.starte(engine, player));  // von vorn
    CHECK(ersterWert(engine.generator) == 0.5f);
    spieler.halte();

    // Stop, waehrend gerechnet wird: nichts spielt, nichts wartet
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}, QStringLiteral("C:/s/c.wav")));
    engine.halten = true;
    REQUIRE(spieler.starte(engine, player));
    CHECK(spieler.wartet());
    spieler.halte();
    CHECK_FALSE(spieler.spielt());
    CHECK_FALSE(spieler.wartet());
    CHECK_FALSE(spieler.bereitet());
    engine.halten = false;  // der verworfene Auftrag laeuft aus — der Spieler wartet am Ende auf ihn
}

TEST_CASE("Loop Spieler 29d: eine Bearbeitung waehrend des Spielens — die alten Toene klingen, bis die neuen da sind")
{
    LoopPappEngine engine;
    SpielPlayer player;
    Spieler spieler;
    spieler.setzeLoop(spielLoop({1, 0, 0, 0}));
    REQUIRE(spieler.starte(engine, player));
    spieler.warteAufToene();
    CHECK(ersterWert(engine.generator) == 0.5f);

    engine.halten = true;
    spieler.setzeLoop(spielLoop({0.5, 0, 0, 0}, QStringLiteral("C:/s/b.wav")));
    CHECK(spieler.bereitet());
    CHECK_FALSE(spieler.wartet());
    REQUIRE(spieler.starte(engine, player));  // von vorn: noch das bisherige Programm
    CHECK(ersterWert(engine.generator) == 0.5f);

    engine.halten = false;
    spieler.warteAufToene();
    CHECK_FALSE(spieler.bereitet());
    REQUIRE(spieler.starte(engine, player));
    CHECK(ersterWert(engine.generator) == 0.25f);  // jetzt die Bearbeitung
}

TEST_CASE("Loop Spieler 29d: ladeVor — Play findet die Toene fertig, oder uebernimmt den Auftrag")
{
    LoopPappEngine engine;
    SpielPlayer player;
    {
        Spieler spieler;
        spieler.setzeLoop(spielLoop({1, 0, 0, 0}));
        spieler.ladeVor(engine);
        CHECK(spieler.laedtVor());
        for (int i = 0; i < 2000 && spieler.laedtVor(); ++i)
        {
            spieler.takt();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        REQUIRE_FALSE(spieler.laedtVor());
        CHECK(engine.dekodiert.size() == 1);
        REQUIRE(spieler.starte(engine, player));
        CHECK_FALSE(spieler.wartet());  // nichts zu rechnen: er spielt sofort
        CHECK(ersterWert(engine.generator) == 0.5f);
        CHECK(engine.dekodiert.size() == 1);
    }
    {
        // Play mitten im Vorladen: derselbe Auftrag rechnet weiter, nichts doppelt
        Spieler spieler;
        spieler.setzeLoop(spielLoop({1, 0, 0, 0}, QStringLiteral("C:/s/d.wav")));
        engine.halten = true;
        spieler.ladeVor(engine);
        REQUIRE(spieler.starte(engine, player));
        CHECK(spieler.wartet());
        CHECK_FALSE(spieler.laedtVor());
        engine.halten = false;
        spieler.warteAufToene();
        CHECK(engine.dekodiert.count(QStringLiteral("C:/s/d.wav")) == 1);
        CHECK(ersterWert(engine.generator) == 0.5f);
    }
}

TEST_CASE("Loop Panel 29d: an der Stelle von Bar · Step steht, was vorbereitet wird")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    LoopEinstellungen einstellungen;
    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    const QString beispiel = wurzel.filePath(QStringLiteral("asset/loop/drums_demo.lvloop"));

    ServiceContainer dienste;
    dienste.registerSingleton<IAudioEngine>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new LoopPappEngine); });
    dienste.registerSingleton<IAudioPlayer>(
        [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new SpielPlayer); });
    auto* engine = dynamic_cast<LoopPappEngine*>(dienste.tryResolve<IAudioEngine>());
    REQUIRE(engine != nullptr);
    tonspeicher().leere();  // die Kit-Klaenge sollen noch nicht im Speicher liegen
    {
        LoopPanel panel(dienste);
        auto* play = panel.findChild<QPushButton*>(QStringLiteral("loopPlay"));
        auto* stelle = panel.findChild<QLabel*>(QStringLiteral("loopPosition"));
        REQUIRE(play != nullptr);
        REQUIRE(stelle != nullptr);

        engine->halten = true;
        REQUIRE(panel.lade(beispiel));
        CHECK(panel.spieler().laedtVor());  // H1 a: beim Oeffnen vorab
        play->click();
        CHECK(play->text() == QStringLiteral("Stop"));
        CHECK(panel.spieler().wartet());
        CHECK_FALSE(panel.spieler().laedtVor());  // Play hat den Auftrag uebernommen
        CHECK(stelle->text() == QStringLiteral("Preparing sounds… 0 / 6"));

        engine->halten = false;
        panel.warteAufToene();
        CHECK_FALSE(panel.spieler().wartet());
        CHECK(stelle->text() == QStringLiteral("Bar 1 · Step 1"));
        play->click();
        CHECK(play->text() == QStringLiteral("Play"));

        // der Drum-Editor: Play pattern wartet, seine Hinweiszeile sagt es (Patrik S142)
        const QString basis = panel.loop().muster[0].name;
        REQUIRE(panel.oeffneEditor(basis, false));
        DrumEditor* editor = panel.editor();
        REQUIRE(editor != nullptr);
        auto* hinweis = editor->findChild<QLabel*>(QStringLiteral("drumHint"));
        REQUIRE(hinweis != nullptr);
        CHECK(hinweis->text().isEmpty());
        tonspeicher().leere();  // sonst laege alles schon im Speicher
        engine->halten = true;
        REQUIRE(panel.spieleMuster(basis));
        CHECK(panel.spieler().wartet());
        CHECK(hinweis->text().startsWith(QStringLiteral("Preparing sounds… 0 / ")));
        CHECK(stelle->text().isEmpty());  // ein Muster allein: im Arrangement keine Stelle
        engine->halten = false;
        panel.warteAufToene();
        CHECK(hinweis->text().isEmpty());
        panel.halte();
    }
    tonspeicher().leere();  // Klaenge der Engine aus Pappe zu echten Dateien: nicht liegen lassen
}

namespace
{

/// Ein Wegwerf-Ordner `frozen` und ein leerer Tonspeicher fuer einen Fall (Stufe 30f).
struct FrozenOrdner
{
    QTemporaryDir ordner;
    FrozenOrdner()
    {
        REQUIRE(ordner.isValid());
        setzeFrozenOrdnerFuerTests(ordner.path());
        tonspeicher().leere();
    }
    ~FrozenOrdner()
    {
        setzeFrozenOrdnerFuerTests({});
        tonspeicher().leere();
    }
};

/// Der Loop des Spielers mit eingefrorenem Clip; `abdruck` = sein jetziger Abdruck.
Loop eingefrorenerLoop(QByteArray& abdruck)
{
    Loop loop = spielLoop({1, 0, 0, 0});
    loop.richte();
    abdruck = clipAbdruck(loop, ClipOrt{0, 0}, 44100, instrumentAuskunft());
    REQUIRE(abdruck.size() == 40);
    loop.kanaele[0].clips[0].frozen = QString::fromLatin1(abdruck);
    return loop;
}

}  // namespace

TEST_CASE("Loop Spieler 30f: ein eingefrorener Clip spielt seine Wellenform — ohne Datei live, eine unlesbare geht")
{
    FrozenOrdner frozen;
    LoopPappEngine engine;
    SpielPlayer player;
    QByteArray abdruck;
    const Loop loop = eingefrorenerLoop(abdruck);

    SUBCASE("die Datei liegt vor: die Wellenform, nichts dekodiert, nichts gerechnet")
    {
        REQUIRE(legeFrozen(abdruck, std::vector<float>(200, 0.25f), 44100));
        CHECK(eingefrorenVon(loop, 44100, instrumentAuskunft()).size() == 1);
        Spieler spieler;
        spieler.setzeLoop(loop);
        REQUIRE(spieler.starte(engine, player));
        spieler.warteAufToene();
        CHECK(ersterWert(engine.generator) == 0.25f);
        CHECK(engine.dekodiert.isEmpty());  // das Muster liegt nur im eingefrorenen Clip
        CHECK(spieler.bericht().isEmpty());
        spieler.halte();
        // ein zweites Mal: aus dem Tonspeicher, sofort
        Spieler wieder;
        wieder.setzeLoop(loop);
        REQUIRE(wieder.starte(engine, player));
        CHECK_FALSE(wieder.wartet());
        CHECK(ersterWert(engine.generator) == 0.25f);
        wieder.halte();
    }
    SUBCASE("keine Datei zum jetzigen Abdruck: live, wie ohne Marke")
    {
        CHECK(eingefrorenVon(loop, 44100, instrumentAuskunft()).empty());
        Spieler spieler;
        spieler.setzeLoop(loop);
        REQUIRE(spieler.starte(engine, player));
        spieler.warteAufToene();
        CHECK(ersterWert(engine.generator) == 0.5f);
        CHECK(engine.dekodiert == QStringList{QStringLiteral("C:/s/a.wav")});
        spieler.halte();
    }
    SUBCASE("verändert seit dem Einfrieren: die Datei zum gespeicherten Abdruck spielt nicht (FV2 a)")
    {
        REQUIRE(legeFrozen(abdruck, std::vector<float>(200, 0.25f), 44100));
        Loop geaendert = loop;
        geaendert.muster[0].zeilen[0][0] = 0.8;  // ein Schlag leiser: ein anderer Abdruck
        CHECK(eingefrorenVon(geaendert, 44100, instrumentAuskunft()).empty());
        Spieler spieler;
        spieler.setzeLoop(geaendert);
        REQUIRE(spieler.starte(engine, player));
        spieler.warteAufToene();
        CHECK(ersterWert(engine.generator) == doctest::Approx(0.4f));  // live: 0.5 mal 0.8
        spieler.halte();
    }
    SUBCASE("eine Datei, die sich nicht lesen laesst: sie geht, der Clip spielt live")
    {
        {
            QFile kaputt(frozenDatei(abdruck));
            REQUIRE(kaputt.open(QIODevice::WriteOnly));
            kaputt.write("keine WAV");
        }
        Spieler spieler;
        spieler.setzeLoop(loop);
        REQUIRE(spieler.starte(engine, player));
        spieler.warteAufToene();  // die Wellenform fehlt: er plant neu
        CHECK_FALSE(QFileInfo::exists(frozenDatei(abdruck)));
        CHECK(eingefrorenVon(loop, 44100, instrumentAuskunft()).empty());
        spieler.warteAufToene();  // die Toene fuer live
        CHECK(ersterWert(engine.generator) == 0.5f);
        spieler.halte();
        // neu abgelegt, zaehlt sie wieder
        REQUIRE(legeFrozen(abdruck, std::vector<float>(200, 0.25f), 44100));
        CHECK(eingefrorenVon(loop, 44100, instrumentAuskunft()).size() == 1);
    }
}

namespace
{

/// Wie `LoopPappEngine`, aber jede Datei klingt anders und zwei Sekunden lang: `a` 0.5, `b` 0.25,
/// jede andere 0.125 — so hoert man, welcher Klang an welcher Stimme liegt.
class KlangPappEngine : public LoopPappEngine
{
public:
    [[nodiscard]] std::vector<float> decodeStereo(const QString& filePath, int rate, double maxSekunden) override
    {
        (void)LoopPappEngine::decodeStereo(filePath, rate, maxSekunden);  // mitschreiben, halten
        const float wert = filePath.endsWith(QStringLiteral("a.wav"))   ? 0.5f
                           : filePath.endsWith(QStringLiteral("b.wav")) ? 0.25f
                                                                        : 0.125f;
        return std::vector<float>(2 * 88200, wert);
    }
};

}  // namespace

TEST_CASE("Loop im Composer S144: nach einer Aenderung klingt bis zu den neuen Toenen das Bisherige (C) — auch wenn sich die Namen verschieben")
{
    using lumi::composer::Tonfuehrung;
    tonspeicher().leere();
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    // jeder Schritt schlaegt a an: es klingt die ganze Zeit 0.5
    const Loop loop = spielLoop({1, 1, 1, 1});
    const QString pfad = ordner.filePath(QStringLiteral("namen.lvloop"));
    REQUIRE(speichereLoop(loop, pfad));

    KlangPappEngine engine;
    SpielPlayer player;
    Tonfuehrung fuehrung;
    REQUIRE(fuehrung.uebernimm(engine, player));
    const lumi::composer::Komposition k = mitClip(pfad, 0, 60000);
    (void)fuehrung.takt(k, 0, true);
    fuehrung.warteAufToene();
    (void)fuehrung.takt(k, 33, true);
    REQUIRE(engine.generator != nullptr);
    CHECK(ersterWert(engine.generator) == 0.5f);

    // das Panel meldet eine Fassung mit einer neuen Spur vorn (b, ohne Schlag): die Namen sind jetzt
    // [b, a] statt [a] — b wird noch dekodiert
    Loop offen = loop;
    offen.muster[0].spuren.insert(offen.muster[0].spuren.begin(),
                                  Spur{QStringLiteral("B"), QStringLiteral("C:/s/b.wav"), 1.0, 0.0, false});
    offen.muster[0].zeilen.insert(offen.muster[0].zeilen.begin(), std::vector<double>(4, 0.0));
    engine.halten = true;
    meldeOffen(pfad, offen);
    (void)fuehrung.takt(k, 66, true);
    CHECK(fuehrung.vorbereitung().gesamt == 2);  // b und a — a liegt nicht im Speicher (die Datei gibt es nicht)
    // C: bis b da ist, klingt das bisherige Programm — a auf jedem Schritt
    CHECK(ersterWert(engine.generator) == 0.5f);

    engine.halten = false;
    fuehrung.warteAufToene();
    (void)fuehrung.takt(k, 99, true);
    CHECK(fuehrung.vorbereitung().gesamt == 0);
    CHECK(ersterWert(engine.generator) == 0.5f);  // die neue Fassung: a schlaegt weiter, b schweigt
    meldeOffen({}, {});
    fuehrung.gibFrei();
    tonspeicher().leere();
}

TEST_CASE("Loop im Composer 30f: ein eingefrorener Clip im Loop-Clip spielt seine Wellenform")
{
    FrozenOrdner frozen;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    QByteArray abdruck;
    const Loop loop = eingefrorenerLoop(abdruck);
    const QString pfad = ordner.filePath(QStringLiteral("frozen.lvloop"));
    REQUIRE(speichereLoop(loop, pfad));
    REQUIRE(legeFrozen(abdruck, std::vector<float>(200, 0.25f), 44100));

    LoopPappEngine engine;
    SpielPlayer player;
    lumi::composer::Tonfuehrung fuehrung;
    REQUIRE(fuehrung.uebernimm(engine, player));
    const lumi::composer::Komposition k = mitClip(pfad, 1000, 6000);
    (void)fuehrung.takt(k, 0, true);
    fuehrung.warteAufToene();
    REQUIRE(engine.generator != nullptr);
    CHECK(engine.dekodiert.isEmpty());  // nichts dekodiert: die Wellenform kommt aus `frozen`
    CHECK(ersterWert(engine.generator) == 0.25f);
    fuehrung.gibFrei();
}
