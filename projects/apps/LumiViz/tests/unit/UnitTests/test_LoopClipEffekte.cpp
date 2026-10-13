/**
 ****************************************************************************************
 * @file   test_LoopClipEffekte.cpp
 * @brief  Loop: das Fenster „Clip effects" und sein Weg ins Loop-Panel
 *         (docs/audio/Loop_Konzept.md §4g, Stufe 12d)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster. `LUMIVIZ_CLIP_EFFEKTE_BILD=<datei.png>`
 * legt ein Bild des Fensters mit zwei markierten Clips ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"
#include "loop/ClipEffektFenster.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Taktachse.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSlider>

#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

/// Drums: A ab Takt 0 und noch einmal ab Takt 4; Lead: B ab Takt 0. 100 BPM.
Loop fensterLoop()
{
    Loop loop;
    loop.bpm = 100.0;
    Muster a;
    a.name = QStringLiteral("Beat");
    Muster b;
    b.name = QStringLiteral("Lead");
    loop.muster = {a, b};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("Beat"), 0, 1}, {QStringLiteral("Beat"), 4, 1}}},
                    {QStringLiteral("Lead"), true, {{QStringLiteral("Lead"), 0, 1}}}};
    loop.richte();
    return loop;
}

const ClipEffekte& effekteIn(const Loop& loop, int kanal, int clip)
{
    return loop.kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(clip)].effekte;
}

}  // namespace

TEST_CASE("Loop Clip effects: das Fenster zeigt die markierten Clips und setzt jeden Wert fuer alle")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};

    ClipEffektFenster fenster;  // nicht gezeigt
    Loop loop = fensterLoop();
    int aenderungen = 0;
    QString zuletzt;
    QObject::connect(&fenster, &ClipEffektFenster::bearbeitet, &fenster, [&](const Loop& neu, const QString& was) {
        ++aenderungen;
        zuletzt = was;
        loop = neu;
        fenster.zeige(loop, fenster.orte());  // wie das Panel: die Fassung kommt zurueck
    });

    auto* kopf = fenster.findChild<QLabel*>(QStringLiteral("fxHead"));
    auto* clips = fenster.findChild<QLabel*>(QStringLiteral("fxClips"));
    auto* echoAn = fenster.findChild<QCheckBox*>(QStringLiteral("fxEchoOn"));
    auto* hallAn = fenster.findChild<QCheckBox*>(QStringLiteral("fxReverbOn"));
    auto* zeit = fenster.findChild<QComboBox*>(QStringLiteral("fxEchoTime"));
    auto* zeitMs = fenster.findChild<QLabel*>(QStringLiteral("fxEchoTimeValue"));
    auto* feedback = fenster.findChild<QSlider*>(QStringLiteral("fxEchoFeedback"));
    auto* decay = fenster.findChild<QSlider*>(QStringLiteral("fxReverbDecay"));
    auto* decayWert = fenster.findChild<QLabel*>(QStringLiteral("fxReverbDecayValue"));
    auto* mix = fenster.findChild<QSlider*>(QStringLiteral("fxReverbMix"));
    auto* mixWert = fenster.findChild<QLabel*>(QStringLiteral("fxReverbMixValue"));
    auto* drive = fenster.findChild<QSlider*>(QStringLiteral("fxDistDrive"));
    auto* kopieren = fenster.findChild<QPushButton*>(QStringLiteral("fxCopy"));
    auto* einfuegen = fenster.findChild<QPushButton*>(QStringLiteral("fxPaste"));
    auto* zurueck = fenster.findChild<QPushButton*>(QStringLiteral("fxReset"));
    REQUIRE(kopf != nullptr);
    REQUIRE(clips != nullptr);
    REQUIRE(echoAn != nullptr);
    REQUIRE(hallAn != nullptr);
    REQUIRE(zeit != nullptr);
    REQUIRE(zeitMs != nullptr);
    REQUIRE(feedback != nullptr);
    REQUIRE(decay != nullptr);
    REQUIRE(decayWert != nullptr);
    REQUIRE(mix != nullptr);
    REQUIRE(mixWert != nullptr);
    REQUIRE(drive != nullptr);
    REQUIRE(kopieren != nullptr);
    REQUIRE(einfuegen != nullptr);
    REQUIRE(zurueck != nullptr);

    SUBCASE("ohne Clip ist es gesperrt und sagt, was zu tun ist")
    {
        CHECK(kopf->text() == QStringLiteral("No clip selected"));
        CHECK(clips->text() == QStringLiteral("Select one or more clips in the Loop panel"));
        CHECK_FALSE(echoAn->isEnabled());
        CHECK_FALSE(zurueck->isEnabled());
        CHECK_FALSE(fenster.setzeZurueck());
        fenster.zeige(loop, {{0, 9}});  // den Clip gibt es nicht
        CHECK(fenster.orte().empty());
        CHECK(aenderungen == 0);
    }
    SUBCASE("ein Clip: die Regler stehen auf seinen Werten, jede Aenderung meldet eine neue Fassung")
    {
        fenster.zeige(loop, {{0, 4}});
        CHECK(kopf->text() == QStringLiteral("1 clip"));
        CHECK(clips->text() == QStringLiteral("Beat (bar 5)"));
        CHECK(echoAn->isEnabled());
        CHECK(echoAn->checkState() == Qt::Unchecked);
        CHECK(decay->value() == 18);  // 1,8 s, die Vorgabe
        CHECK(decayWert->text() == QStringLiteral("1.8 s"));
        CHECK(zeit->currentText() == QStringLiteral("1/8 dotted"));
        CHECK(zeitMs->text() == QStringLiteral("450 ms"));  // 0,75 Schlaege bei 100 BPM

        echoAn->click();
        CHECK(aenderungen == 1);
        CHECK(zuletzt == QStringLiteral("Clip effects"));
        CHECK(effekteIn(loop, 0, 1).wert<ClipEcho>().an);
        CHECK_FALSE(effekteIn(loop, 0, 0).wert<ClipEcho>().an);  // der andere Clip derselben Basis bleibt
        CHECK(echoAn->checkState() == Qt::Checked);

        feedback->setValue(70);
        CHECK(effekteIn(loop, 0, 1).wert<ClipEcho>().feedback == doctest::Approx(0.7));
        CHECK(feedback->maximum() == 90);  // darueber klaenge ein Echo nie aus
        decay->setValue(35);
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().sekunden == doctest::Approx(3.5));
        CHECK(decayWert->text() == QStringLiteral("3.5 s"));
        zeit->setCurrentIndex(zeit->findText(QStringLiteral("1/4")));
        Q_EMIT zeit->activated(zeit->currentIndex());
        CHECK(effekteIn(loop, 0, 1).wert<ClipEcho>().zeit == EchoZeit::Viertel);
        CHECK(zeitMs->text() == QStringLiteral("600 ms"));
        CHECK(aenderungen == 4);

        // derselbe Wert noch einmal meldet nichts
        feedback->setValue(70);
        CHECK(aenderungen == 4);
        // im Zug laeuft nur die Anzeige mit; gemeldet wird beim Loslassen
        mix->setSliderDown(true);
        mix->setValue(60);
        CHECK(mixWert->text() == QStringLiteral("60 %"));
        CHECK(aenderungen == 4);
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().mix == doctest::Approx(0.25));
        mix->setSliderDown(false);  // loest sliderReleased aus
        CHECK(aenderungen == 5);
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().mix == doctest::Approx(0.6));
    }
    SUBCASE("mehrere Clips: verschiedene Werte stehen als mixed, ein Regler setzt alle")
    {
        loop.kanaele[0].clips[1].effekte.hole<ClipHall>() = {true, 0.5, 4.0, 0.4, 0.5};
        loop.kanaele[0].clips[1].effekte.hole<ClipEcho>().zeit = EchoZeit::Halbe;
        fenster.zeige(loop, {{0, 4}, {1, 0}});
        CHECK(kopf->text() == QStringLiteral("2 clips"));
        CHECK(clips->text() == QStringLiteral("Beat (bar 5) · Lead (bar 1)"));
        CHECK(hallAn->checkState() == Qt::PartiallyChecked);  // einer an, einer aus
        CHECK(decayWert->text() == QStringLiteral("mixed"));
        CHECK(mixWert->text() == QStringLiteral("mixed"));
        CHECK(zeit->currentIndex() == -1);
        CHECK(zeitMs->text().isEmpty());
        CHECK(echoAn->checkState() == Qt::Unchecked);

        // ein Klick auf „teils an" schaltet alle an
        hallAn->click();
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().an);
        CHECK(effekteIn(loop, 1, 0).wert<ClipHall>().an);
        CHECK(hallAn->checkState() == Qt::Checked);
        // der Regler setzt den Wert fuer beide; was er nicht beruehrt, bleibt verschieden
        decay->setValue(25);
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().sekunden == doctest::Approx(2.5));
        CHECK(effekteIn(loop, 1, 0).wert<ClipHall>().sekunden == doctest::Approx(2.5));
        CHECK(decayWert->text() == QStringLiteral("2.5 s"));
        CHECK(mixWert->text() == QStringLiteral("mixed"));
        CHECK(effekteIn(loop, 0, 1).wert<ClipHall>().mix == doctest::Approx(0.5));
        CHECK_FALSE(effekteIn(loop, 0, 0).wert<ClipHall>().an);  // nicht markiert: unberuehrt
    }
    SUBCASE("Copy, Paste und Reset")
    {
        loop.kanaele[0].clips[0].effekte.hole<ClipVerzerrung>() = {true, 0.8, 0.2, 0.6};
        fenster.zeige(loop, {{0, 0}});
        CHECK_FALSE(einfuegen->isEnabled());  // noch nichts gemerkt
        CHECK(drive->value() == 80);
        kopieren->click();
        CHECK(einfuegen->isEnabled());
        CHECK(aenderungen == 0);  // merken aendert nichts

        fenster.zeige(loop, {{0, 4}, {1, 0}});
        einfuegen->click();
        CHECK(aenderungen == 1);
        CHECK(effekteIn(loop, 0, 1) == effekteIn(loop, 0, 0));
        CHECK(effekteIn(loop, 1, 0) == effekteIn(loop, 0, 0));
        CHECK(drive->value() == 80);

        zurueck->click();
        CHECK(aenderungen == 2);
        CHECK(effekteIn(loop, 0, 1) == ClipEffekte{});
        CHECK(effekteIn(loop, 1, 0) == ClipEffekte{});
        CHECK(effekteIn(loop, 0, 0).wert<ClipVerzerrung>().an);  // nicht markiert
        zurueck->click();  // schon zurueckgesetzt: nichts zu melden
        CHECK(aenderungen == 2);
    }
    SUBCASE("das Bild des Fensters")
    {
        loop.kanaele[0].clips[1].effekte.hole<ClipEcho>() = {true, EchoZeit::AchtelPunktiert, 0.45, 0.3, 0.35};
        loop.kanaele[0].clips[1].effekte.hole<ClipHall>() = {true, 0.55, 1.8, 0.4, 0.25};
        loop.kanaele[1].clips[0].effekte.hole<ClipHall>() = {true, 0.55, 3.0, 0.4, 0.25};
        fenster.zeige(loop, {{0, 4}, {1, 0}});
        fenster.setSpitze(0.88);
        fenster.resize(fenster.sizeHint());
        const QImage bild = fenster.grab().toImage();
        CHECK_FALSE(bild.isNull());
        if (const QString datei = qEnvironmentVariable("LUMIVIZ_CLIP_EFFEKTE_BILD"); !datei.isEmpty())
            CHECK(bild.save(datei));
    }
}

TEST_CASE("Loop Clip effects im Panel: Effects oeffnet das Fenster, es folgt den Marken, Undo holt zurueck")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    // eigene Einstellungen: das Panel merkt sich die zuletzt geladene Datei
    const QString org = QCoreApplication::organizationName();
    const QString name = QCoreApplication::applicationName();
    QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
    QCoreApplication::setApplicationName(QStringLiteral("LoopClipEffekte"));
    QSettings().clear();
    {
        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);

        LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
        Taktachse* achse = panel.taktachse();
        REQUIRE(achse != nullptr);
        CHECK_FALSE(panel.oeffneEffekte(false));  // kein Loop geladen
        CHECK(panel.effektFenster() == nullptr);

        panel.neu();
        REQUIRE(achse->neuerClip(0, 0));
        REQUIRE(achse->dupliziereAuswahl());  // zwei Clips derselben Basis: Takt 0 und Takt 4
        CHECK_FALSE(achse->markierte().empty());

        // „Effects..." im Menue des Clips
        int verlangt = 0;
        QObject::connect(achse, &Taktachse::effekteVerlangt, achse, [&verlangt] { ++verlangt; });
        achse->markiere({});
        CHECK_FALSE(achse->verlangeEffekte());  // ohne markierten Clip
        achse->markiere({{0, 4}});
        // (das Signal oeffnete das Fenster sichtbar — hier ohne Fenster)
        REQUIRE(panel.oeffneEffekte(false));
        ClipEffektFenster* fenster = panel.effektFenster();
        REQUIRE(fenster != nullptr);
        CHECK(fenster->orte() == std::vector<ClipOrt>{{0, 4}});

        auto* hallAn = fenster->findChild<QCheckBox*>(QStringLiteral("fxReverbOn"));
        auto* kopf = fenster->findChild<QLabel*>(QStringLiteral("fxHead"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("loopClip"));
        REQUIRE(hallAn != nullptr);
        REQUIRE(kopf != nullptr);
        REQUIRE(clipZeile != nullptr);

        // eine Aenderung im Fenster laeuft ueber den CommandBus des Panels
        hallAn->click();
        CHECK(panel.loop().kanaele[0].clips[1].effekte.wert<ClipHall>().an);
        CHECK_FALSE(panel.loop().kanaele[0].clips[0].effekte.wert<ClipHall>().an);
        CHECK(bus->undoDescription() == "Clip effects");
        CHECK(panel.spieler().loop() == panel.loop());  // was gespielt wuerde, hat den Effekt
        CHECK(panel.ungespeichert());
        CHECK(hallAn->checkState() == Qt::Checked);

        // das Fenster folgt den Marken
        achse->markiere({{0, 0}, {0, 4}});
        CHECK(kopf->text() == QStringLiteral("2 clips"));
        CHECK(hallAn->checkState() == Qt::PartiallyChecked);
        CHECK(clipZeile->text().startsWith(QStringLiteral("2 clips selected · 1 with effects")));
        achse->markiere({});
        CHECK(kopf->text() == QStringLiteral("No clip selected"));
        CHECK(clipZeile->text().contains(QStringLiteral("Ctrl+click")));

        // Undo: der Effekt ist wieder aus, das Fenster zeigt es
        achse->markiere({{0, 4}});
        REQUIRE(bus->undo());
        CHECK_FALSE(panel.loop().kanaele[0].clips[1].effekte.wert<ClipHall>().an);
        CHECK(hallAn->checkState() == Qt::Unchecked);
        REQUIRE(bus->redo());
        CHECK(hallAn->checkState() == Qt::Checked);

        // die Effekte reisen mit dem Clip: verschieben und kopieren im Panel
        std::vector<ClipOrt> orte = {{0, 4}};
        Loop verschoben = panel.loop();
        REQUIRE(versetzeClips(verschoben, orte, 0, 8));
        panel.bearbeite(verschoben, QStringLiteral("Move clip"));
        CHECK(panel.loop().kanaele[0].clips[1].startTakt == 12);
        CHECK(panel.loop().kanaele[0].clips[1].effekte.wert<ClipHall>().an);

        // New: das Fenster hat keinen Clip mehr
        panel.neu();
        CHECK(kopf->text() == QStringLiteral("No clip selected"));
        (void)verlangt;
    }
    QSettings().clear();
    QCoreApplication::setOrganizationName(org);
    QCoreApplication::setApplicationName(name);
}
