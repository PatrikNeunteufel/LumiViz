/**
 ****************************************************************************************
 * @file   test_ComposerPanel.cpp
 * @brief  Composer, Stufe C2b: das Panel zum Ansehen
 *         (docs/ui/Composer_Konzept.md §7)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Das Panel an der echten Fassade (`VisualizerWidget` mit Render-Thread): die
 * Knoepfe bedienen die Sitzung, ein Klick ins Lineal springt, ein Klick auf
 * einen Clip zeigt ihn in der Clip-Zeile.
 *
 * `LUMIVIZ_COMPOSER_PANEL_BILD=<datei.png>` legt ein Bild des Panels ab.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "TestFenster.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/FormPresets.hpp"
#include "composer/Komposition.hpp"
#include "composer/KurvenEditor.hpp"
#include "composer/Sitzung.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QUrl>
#include <QElapsedTimer>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>

#include <functional>

using namespace lumi::multieffect;
using namespace lumi::composer;

namespace
{

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("ComposerPanel"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

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

VisualClip clip(const QString& datei, std::int64_t start, std::int64_t ende)
{
    VisualClip c;
    c.datei = datei;
    c.startMs = start;
    c.endeMs = ende;
    return c;
}

bool warte(const std::function<bool()>& bedingung, int hoechstensMs = 8000)
{
    QElapsedTimer uhr;
    uhr.start();
    while (!bedingung())
    {
        if (uhr.elapsed() > hoechstensMs) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(2);
    }
    return true;
}

void klick(QWidget& ziel, QPoint ort)
{
    QMouseEvent druck(QEvent::MouseButtonPress, QPointF(ort), QPointF(ziel.mapToGlobal(ort)),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &druck);
    QMouseEvent los(QEvent::MouseButtonRelease, QPointF(ort), QPointF(ziel.mapToGlobal(ort)),
                    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &los);
}

}  // namespace

TEST_SUITE("ComposerPanel")
{
    TEST_CASE("C2b Panel: die Zeit als Text")
    {
        CHECK(ComposerPanel::zeitText(0) == QStringLiteral("0:00.0"));
        CHECK(ComposerPanel::zeitText(83400) == QStringLiteral("1:23.4"));
        CHECK(ComposerPanel::zeitText(59999) == QStringLiteral("0:59.9"));
        CHECK(ComposerPanel::zeitText(3600000) == QStringLiteral("60:00.0"));
        CHECK(ComposerPanel::zeitText(-5) == QStringLiteral("0:00.0"));
    }

    TEST_CASE("C2b Panel: Knoepfe, Lineal und Clips bedienen die Sitzung des Fensters")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x0000FF);
        Komposition k;
        Kanal unten;
        unten.name = QStringLiteral("CH1");
        unten.clips = {clip(rot, 0, 20000), clip(blau, 30000, 60000)};
        unten.clips[0].setzeBlenden(3000, 5000);
        Kanal oben;
        oben.name = QStringLiteral("CH2");
        oben.clips = {clip(blau, 15000, 35000)};
        oben.clips[0].mischart = Mischart::Additive;
        k.kanaele = {unten, oben};
        const QString datei = dir.filePath(QStringLiteral("probe.lvcomp"));
        REQUIRE(speichereKomposition(k, datei));

        ServiceContainer dienste;
        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }

        ComposerPanel panel(dienste);
        panel.setFenster(&widget);
        panel.resize(900, 260);
        testfenster::platziere(panel);
        panel.show();
        REQUIRE(warte([&] { return panel.zeitachse()->width() > 400; }));

        auto* an = panel.findChild<QCheckBox*>(QStringLiteral("composerAn"));
        auto* spielen = panel.findChild<QPushButton*>(QStringLiteral("composerPlay"));
        auto* stopp = panel.findChild<QPushButton*>(QStringLiteral("composerStop"));
        auto* schleife = panel.findChild<QPushButton*>(QStringLiteral("composerLoop"));
        auto* resume = panel.findChild<QPushButton*>(QStringLiteral("composerResume"));
        auto* zeit = panel.findChild<QLabel*>(QStringLiteral("composerZeit"));
        auto* name = panel.findChild<QLabel*>(QStringLiteral("composerDatei"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("composerClip"));
        REQUIRE(an != nullptr);
        REQUIRE(spielen != nullptr);
        REQUIRE(stopp != nullptr);
        REQUIRE(schleife != nullptr);
        REQUIRE(resume != nullptr);
        REQUIRE(zeit != nullptr);
        REQUIRE(name != nullptr);
        REQUIRE(clipZeile != nullptr);
        Sitzung* sitzung = panel.sitzung();
        REQUIRE(sitzung == Sitzung::finde(widget));
        Zeitachse* achse = panel.zeitachse();

        // --- nichts geladen: nichts zu bedienen
        CHECK_FALSE(an->isEnabled());
        CHECK_FALSE(spielen->isEnabled());
        CHECK_FALSE(stopp->isEnabled());
        CHECK(zeit->text().isEmpty());
        CHECK(clipZeile->text().isEmpty());

        // --- geladen (ueber die Sitzung, wie es das Menue tut): das Panel zieht nach
        REQUIRE(sitzung->lade(datei));
        CHECK(an->isEnabled());
        CHECK_FALSE(an->isChecked());  // geladen ist nicht an
        CHECK(spielen->isEnabled());
        CHECK(name->text() == QStringLiteral("probe.lvcomp"));
        CHECK(zeit->text() == QStringLiteral("0:00.0 / 1:00.0"));
        CHECK(clipZeile->text() == QStringLiteral("No clip selected"));
        // die Achse zeigt die ganze Laenge und etwas Luft dahinter (C3b: zum Hinausziehen)
        CHECK(achse->xVon(0) == Zeitachse::kKopfBreite);
        CHECK(Zeitachse::anzeigeLaenge(sitzung->komposition()) == 72000);
        CHECK(achse->xVon(72000) == achse->width() - Zeitachse::kRand);
        CHECK(achse->xVon(60000) < achse->width() - Zeitachse::kRand - 50);
        CHECK(achse->stelleVon(achse->xVon(30000)) == doctest::Approx(30000).epsilon(0.01));
        // CH1 steht in der obersten Zeile (Entscheid Patrik S95)
        CHECK(achse->kanalVonZeile(0) == 0);
        CHECK(achse->kanalVonZeile(1) == 1);

        // --- Clips anklicken
        klick(*achse, QPoint(achse->xVon(25000), achse->yVonZeile(1)));  // CH2, Blau
        CHECK(achse->gewaehlterKanal() == 1);
        CHECK(achse->gewaehlterClip() == 0);
        CHECK(clipZeile->text() ==
              QStringLiteral("CH2 · Blau.lvfx · 0:15.0 – 0:35.0 · Additive"));
        klick(*achse, QPoint(achse->xVon(45000), achse->yVonZeile(0)));  // CH1, zweiter Clip
        CHECK(achse->gewaehlterKanal() == 0);
        CHECK(achse->gewaehlterClip() == 1);
        CHECK(clipZeile->text().startsWith(QStringLiteral("CH1 · Blau.lvfx · 0:30.0 – 1:00.0")));
        klick(*achse, QPoint(achse->xVon(25000), achse->yVonZeile(0)));  // Luecke in CH1
        CHECK(achse->auswahl() == nullptr);
        CHECK(clipZeile->text() == QStringLiteral("No clip selected"));

        // --- Klick ins Lineal bei ausgeschaltetem Composer: merkt die Stelle
        klick(*achse, QPoint(achse->xVon(30000), 6));
        CHECK(sitzung->stelleMs() == achse->stelleVon(achse->xVon(30000)));
        CHECK_FALSE(sitzung->an());
        // die Zeit zeigt die Stelle des Klicks (auf den Pixel gerundet)
        CHECK(zeit->text().startsWith(ComposerPanel::zeitText(sitzung->stelleMs())));
        CHECK(sitzung->stelleMs() == doctest::Approx(30000).epsilon(0.01));

        // --- Composer on
        an->click();
        REQUIRE(sitzung->an());
        CHECK(an->isChecked());
        CHECK_FALSE(sitzung->laeuft());

        // --- Play / Pause mit einem Knopf, Stopp
        spielen->click();
        CHECK(sitzung->laeuft());
        CHECK(spielen->toolTip() == QStringLiteral("Pause"));
        REQUIRE(warte([&] { return sitzung->stelleMs() > 30100; }));
        CHECK(zeit->text().startsWith(QStringLiteral("0:30.")));
        spielen->click();
        CHECK_FALSE(sitzung->laeuft());
        CHECK(spielen->toolTip() == QStringLiteral("Play"));
        klick(*achse, QPoint(achse->xVon(10000), 6));  // Sprung bei laufendem Composer
        CHECK(sitzung->stelleMs() == achse->stelleVon(achse->xVon(10000)));
        stopp->click();
        CHECK(sitzung->stelleMs() == 0);
        CHECK(sitzung->an());

        // --- Loop und Resume
        CHECK_FALSE(sitzung->schleife());
        schleife->click();
        CHECK(sitzung->schleife());
        CHECK(schleife->isChecked());
        CHECK_FALSE(Sitzung::resumeAn());
        resume->click();
        CHECK(Sitzung::resumeAn());
        CHECK(resume->isChecked());
        resume->click();
        CHECK_FALSE(Sitzung::resumeAn());

        const QString bild = qEnvironmentVariable("LUMIVIZ_COMPOSER_PANEL_BILD");
        if (!bild.isEmpty())
        {
            klick(*achse, QPoint(achse->xVon(25000), achse->yVonZeile(1)));
            sitzung->springe(22000);
            QCoreApplication::processEvents();
            CHECK(panel.grab().save(bild));
        }

        // --- Composer aus ueber den Schalter; die Komposition bleibt
        an->click();
        CHECK_FALSE(sitzung->an());
        CHECK_FALSE(an->isChecked());
        CHECK(sitzung->geladen());

        // --- Close Composition: das Panel ist wieder leer
        sitzung->entlade();
        CHECK_FALSE(an->isEnabled());
        CHECK(name->text().isEmpty());
        CHECK(zeit->text().isEmpty());
        CHECK(clipZeile->text().isEmpty());
    }

    TEST_CASE("C3a Panel: bearbeitet wird nur mit dem Schalter Edit — ziehen, loeschen, Mischart, Undo, Save")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x0000FF);
        Komposition k;
        Kanal unten;
        unten.name = QStringLiteral("CH1");
        unten.clips = {clip(rot, 0, 20000), clip(blau, 30000, 60000)};
        Kanal oben;
        oben.name = QStringLiteral("CH2");
        oben.clips = {clip(blau, 42000, 52000)};
        k.kanaele = {unten, oben};
        const QString datei = dir.filePath(QStringLiteral("probe.lvcomp"));
        REQUIRE(speichereKomposition(k, datei));

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>(
            [](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);
        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }

        ComposerPanel panel(dienste);
        panel.setFenster(&widget);
        panel.resize(900, 260);
        testfenster::platziere(panel);
        panel.show();
        REQUIRE(warte([&] { return panel.zeitachse()->width() > 400; }));
        Sitzung* sitzung = panel.sitzung();
        Zeitachse* achse = panel.zeitachse();
        auto* edit = panel.findChild<QCheckBox*>(QStringLiteral("composerEdit"));
        auto* speichern = panel.findChild<QPushButton*>(QStringLiteral("composerSave"));
        auto* name = panel.findChild<QLabel*>(QStringLiteral("composerDatei"));
        auto* mischart = panel.findChild<QComboBox*>(QStringLiteral("composerMischart"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("composerClip"));
        REQUIRE(edit != nullptr);
        REQUIRE(speichern != nullptr);
        REQUIRE(name != nullptr);
        REQUIRE(mischart != nullptr);
        REQUIRE(clipZeile != nullptr);
        CHECK_FALSE(edit->isEnabled());  // nichts geladen

        REQUIRE(sitzung->lade(datei));
        const auto ch1 = [&]() -> const std::vector<VisualClip>& {
            return sitzung->komposition().kanaele[0].clips;
        };
        const auto zug = [&](std::int64_t vonMs, std::int64_t nachMs, int zeile,
                             Qt::KeyboardModifiers tasten = Qt::NoModifier, int versatzPx = 0,
                             int nachZeile = -1) {
            const QPoint a(achse->xVon(vonMs) + versatzPx, achse->yVonZeile(zeile));
            const QPoint b(achse->xVon(nachMs) + versatzPx,
                           achse->yVonZeile(nachZeile < 0 ? zeile : nachZeile));
            QMouseEvent druck(QEvent::MouseButtonPress, QPointF(a), QPointF(achse->mapToGlobal(a)),
                              Qt::LeftButton, Qt::LeftButton, tasten);
            QCoreApplication::sendEvent(achse, &druck);
            for (int i = 1; i <= 4; ++i)
            {
                const QPoint m = a + (b - a) * i / 4;
                QMouseEvent zieh(QEvent::MouseMove, QPointF(m), QPointF(achse->mapToGlobal(m)),
                                 Qt::NoButton, Qt::LeftButton, tasten);
                QCoreApplication::sendEvent(achse, &zieh);
            }
            QMouseEvent los(QEvent::MouseButtonRelease, QPointF(b), QPointF(achse->mapToGlobal(b)),
                            Qt::LeftButton, Qt::NoButton, tasten);
            QCoreApplication::sendEvent(achse, &los);
        };

        // --- Edit aus (Vorgabe): ein Zug aendert nichts, die Mischart ist gesperrt
        CHECK(edit->isEnabled());
        CHECK_FALSE(edit->isChecked());
        zug(10000, 15000, 0);
        CHECK(ch1()[0].startMs == 0);
        CHECK_FALSE(sitzung->ungespeichert());
        CHECK(achse->gewaehlterClip() == 0);  // angeklickt ist er trotzdem
        CHECK_FALSE(mischart->isEnabled());
        CHECK_FALSE(speichern->isEnabled());
        CHECK_FALSE(bus->canUndo());

        // --- Edit an: verschieben, auf das Raster von 1 s
        edit->click();
        REQUIRE(achse->bearbeiten());
        CHECK(mischart->isEnabled());
        zug(10000, 15000, 0);
        CHECK(ch1()[0].startMs == 5000);
        CHECK(ch1()[0].endeMs == 25000);
        CHECK(sitzung->ungespeichert());
        CHECK(name->text() == QStringLiteral("probe.lvcomp *"));
        CHECK(speichern->isEnabled());
        CHECK(clipZeile->text().startsWith(QStringLiteral("CH1 · Rot.lvfx · 0:05.0 – 0:25.0")));

        // --- Undo und Redo ueber den CommandBus der App
        REQUIRE(bus->canUndo());
        CHECK(bus->undoDescription() == "Move clip");
        REQUIRE(bus->undo());
        CHECK(ch1()[0].startMs == 0);
        CHECK_FALSE(sitzung->ungespeichert());
        CHECK(name->text() == QStringLiteral("probe.lvcomp"));
        REQUIRE(bus->redo());
        CHECK(ch1()[0].startMs == 5000);

        // --- ohne Taste haelt der Clip am Nachbarn: buendig vor Blau
        zug(15000, 24000, 0);
        CHECK(ch1()[0].startMs == 10000);
        CHECK(ch1()[0].endeMs == 30000);
        REQUIRE(bus->undo());
        CHECK(ch1()[0].startMs == 5000);

        // --- die rechte Kante: rastet an der Kante des Clips im anderen Kanal ein (42 s liegt nicht an)
        zug(25000, 27300, 0, Qt::NoModifier, -3);
        CHECK(ch1()[0].startMs == 5000);
        CHECK(ch1()[0].endeMs == 27000);
        CHECK(bus->undoDescription() == "Resize clip");
        // mit Alt frei: nicht auf dem Raster
        zug(27000, 28400, 0, Qt::AltModifier, -3);
        CHECK(ch1()[0].endeMs > 27000);
        CHECK(ch1()[0].endeMs % 1000 != 0);
        REQUIRE(bus->undo());
        CHECK(ch1()[0].endeMs == 27000);

        // --- Ctrl: der Nachbar wird gekuerzt; Ctrl+Shift: er rueckt mit
        zug(15000, 22000, 0, Qt::ControlModifier);
        CHECK(ch1()[0].startMs == 12000);
        CHECK(ch1()[0].endeMs == 34000);
        CHECK(ch1()[1].startMs == 34000);
        CHECK(ch1()[1].endeMs == 60000);
        REQUIRE(bus->undo());
        CHECK(ch1()[1].startMs == 30000);
        zug(15000, 22000, 0, Qt::ControlModifier | Qt::ShiftModifier);
        CHECK(ch1()[0].endeMs == 34000);
        CHECK(ch1()[1].startMs == 34000);
        CHECK(ch1()[1].endeMs == 64000);
        CHECK(sitzung->laengeMs() == 64000);
        REQUIRE(bus->undo());
        CHECK(sitzung->laengeMs() == 60000);

        // --- in die Zeile darunter gezogen: der Clip wechselt den Kanal
        zug(15000, 15000, 0, Qt::NoModifier, 0, 1);
        REQUIRE(ch1().size() == 1);
        CHECK(ch1()[0].startMs == 30000);
        REQUIRE(sitzung->komposition().kanaele[1].clips.size() == 2);
        CHECK(sitzung->komposition().kanaele[1].clips[0].startMs == 5000);
        CHECK(sitzung->komposition().kanaele[1].clips[0].endeMs == 27000);
        CHECK(achse->gewaehlterKanal() == 1);
        CHECK(achse->gewaehlterClip() == 0);
        CHECK(clipZeile->text().startsWith(QStringLiteral("CH2 · Rot.lvfx · 0:05.0 – 0:27.0")));
        // und weiter nach rechts auf den Clip dort: ohne Taste haelt er buendig davor
        zug(15000, 35000, 1);
        CHECK(sitzung->komposition().kanaele[1].clips[0].startMs == 20000);
        CHECK(sitzung->komposition().kanaele[1].clips[0].endeMs == 42000);
        REQUIRE(bus->undo());
        REQUIRE(bus->undo());
        REQUIRE(ch1().size() == 2);
        CHECK(ch1()[0].startMs == 5000);
        CHECK(sitzung->komposition().kanaele[1].clips.size() == 1);

        // --- Mischart des angeklickten Clips
        klick(*achse, QPoint(achse->xVon(47000), achse->yVonZeile(1)));  // CH2
        REQUIRE(achse->gewaehlterKanal() == 1);
        CHECK(mischart->currentIndex() == 0);
        Q_EMIT mischart->activated(static_cast<int>(Mischart::Maximum));
        CHECK(sitzung->komposition().kanaele[1].clips[0].mischart == Mischart::Maximum);
        CHECK(clipZeile->text().endsWith(QStringLiteral("Maximum")));
        CHECK(bus->undoDescription() == "Change blend mode");

        // --- Entf loescht den angeklickten Clip
        QKeyEvent entf(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QCoreApplication::sendEvent(achse, &entf);
        CHECK(sitzung->komposition().kanaele[1].clips.empty());
        CHECK(achse->auswahl() == nullptr);
        REQUIRE(bus->undo());
        CHECK(sitzung->komposition().kanaele[1].clips.size() == 1);

        // --- Save schreibt in die geladene Datei; danach ist nichts mehr offen
        REQUIRE(sitzung->ungespeichert());
        speichern->click();
        CHECK_FALSE(sitzung->ungespeichert());
        CHECK(name->text() == QStringLiteral("probe.lvcomp"));
        Komposition gelesen;
        REQUIRE(ladeKomposition(datei, gelesen));
        CHECK(gelesen.kanaele[0].clips[0].endeMs == 27000);
        CHECK(gelesen.kanaele[1].clips[0].mischart == Mischart::Maximum);

        // --- das Ende (Befund Patrik S95): es folgt dem letzten Clip, bis „Fixed end" gesetzt ist
        auto* endeFest = panel.findChild<QCheckBox*>(QStringLiteral("composerFixedEnd"));
        auto* endeZeit = panel.findChild<QDoubleSpinBox*>(QStringLiteral("composerEndTime"));
        REQUIRE(endeFest != nullptr);
        REQUIRE(endeZeit != nullptr);
        CHECK(endeFest->isEnabled());
        CHECK_FALSE(endeFest->isChecked());
        CHECK_FALSE(endeZeit->isEnabled());
        CHECK(endeZeit->value() == doctest::Approx(60.0));
        zug(60000, 50000, 0, Qt::NoModifier, -3);  // die rechte Kante des letzten Clips hinein
        CHECK(ch1()[1].endeMs == 50000);
        CHECK(sitzung->laengeMs() == 52000);  // jetzt endet der Clip in CH2 zuletzt
        CHECK(endeZeit->value() == doctest::Approx(52.0));
        endeFest->click();  // fest: an der heutigen Stelle
        CHECK(sitzung->komposition().laengeMs == 52000);
        CHECK(endeZeit->isEnabled());
        endeZeit->setValue(70.0);
        CHECK(sitzung->laengeMs() == 70000);
        zug(50000, 40000, 0, Qt::NoModifier, -3);  // der Clip wird kuerzer — das feste Ende bleibt
        CHECK(ch1()[1].endeMs == 40000);
        CHECK(sitzung->laengeMs() == 70000);
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_COMPOSER_PANEL_BILD"); !bild.isEmpty())
        {
            QCoreApplication::processEvents();
            CHECK(panel.grab().save(bild + QStringLiteral(".ende.png")));
        }
        endeFest->click();  // wieder folgen
        CHECK(sitzung->komposition().laengeMs == 0);
        CHECK(sitzung->laengeMs() == 52000);
        CHECK_FALSE(endeZeit->isEnabled());
        for (int i = 0; i < 5; ++i) REQUIRE(bus->undo());  // folgen, kuerzer, 70 s, fest, Kante
        CHECK(ch1()[1].endeMs == 60000);
        CHECK(sitzung->laengeMs() == 60000);
        CHECK_FALSE(endeFest->isChecked());
        CHECK_FALSE(sitzung->ungespeichert());

        // --- die Schleife der Komposition (S99): Loop overlap und Songs overlap themselves
        //     gehoeren zur Komposition, laufen ueber Undo, und die Zeitachse zeigt die Fenster
        {
            auto* ueberlappung = panel.findChild<QDoubleSpinBox*>(QStringLiteral("composerLoopOverlap"));
            auto* songsDoppelt = panel.findChild<QCheckBox*>(QStringLiteral("composerSongsTwice"));
            auto* achse = panel.findChild<Zeitachse*>();
            REQUIRE(ueberlappung != nullptr);
            REQUIRE(songsDoppelt != nullptr);
            REQUIRE(achse != nullptr);
            CHECK(ueberlappung->isEnabled());  // Edit ist an
            CHECK(ueberlappung->value() == doctest::Approx(0.0));
            CHECK(songsDoppelt->isChecked());
            ueberlappung->setValue(12.5);
            CHECK(sitzung->komposition().ueberlappungMs == 12500);
            CHECK(sitzung->komposition().umlaufMs() == 47500);
            CHECK(sitzung->ungespeichert());
            songsDoppelt->click();
            CHECK_FALSE(sitzung->komposition().songsDoppelt);
            auto* bilderDoppelt = panel.findChild<QCheckBox*>(QStringLiteral("composerVisualsTwice"));
            REQUIRE(bilderDoppelt != nullptr);
            CHECK(bilderDoppelt->isEnabled());
            CHECK_FALSE(bilderDoppelt->isChecked());  // Vorgabe aus
            bilderDoppelt->click();
            CHECK(sitzung->komposition().bilderDoppelt);
            REQUIRE(bus->undo());
            CHECK_FALSE(sitzung->komposition().bilderDoppelt);
            CHECK_FALSE(bilderDoppelt->isChecked());
            // die Zeitachse hat die geaenderte Komposition, und sie kennt den Schalter Loop
            const bool schleifeVorher = sitzung->schleife();
            sitzung->setSchleife(true);
            QCoreApplication::processEvents();
            QThread::msleep(80);  // ein Takt des Panels
            QCoreApplication::processEvents();
            CHECK(achse->schleife());
            CHECK(achse->zweiteStelleMs() == -1);  // die Uhr steht vorn: kein Ende, das hineinspielt
            // die Blende der ganzen Komposition: zwei Felder, ueber Undo, Keile im Lineal
            auto* einblende = panel.findChild<QDoubleSpinBox*>(QStringLiteral("composerFadeIn"));
            auto* ausblende = panel.findChild<QDoubleSpinBox*>(QStringLiteral("composerFadeOut"));
            REQUIRE(einblende != nullptr);
            REQUIRE(ausblende != nullptr);
            CHECK(einblende->isEnabled());
            CHECK(einblende->value() == doctest::Approx(0.0));
            einblende->setValue(6.0);
            ausblende->setValue(9.0);
            CHECK(sitzung->komposition().einblendeMs == 6000);
            CHECK(sitzung->komposition().ausblendeMs == 9000);
            CHECK(sitzung->komposition().blendeBei(3000) == doctest::Approx(0.5));
            if (const QString bild = qEnvironmentVariable("LUMIVIZ_COMPOSER_PANEL_BILD"); !bild.isEmpty())
            {
                // fuer das Bild die zweite Marke von Hand: so sieht das Fenster im Lauf aus
                achse->setStelle(4000);
                achse->setSchleife(true, 51500);
                CHECK(panel.grab().save(bild + QStringLiteral(".schleife.png")));
            }
            sitzung->setSchleife(schleifeVorher);
            // Undo nimmt alles zurueck, die Felder folgen
            REQUIRE(bus->undo());
            CHECK(sitzung->komposition().ausblendeMs == 0);
            CHECK(ausblende->value() == doctest::Approx(0.0));
            REQUIRE(bus->undo());
            CHECK(sitzung->komposition().einblendeMs == 0);
            CHECK(einblende->value() == doctest::Approx(0.0));
            REQUIRE(bus->undo());
            CHECK(sitzung->komposition().songsDoppelt);
            CHECK(songsDoppelt->isChecked());
            REQUIRE(bus->undo());
            CHECK(sitzung->komposition().ueberlappungMs == 0);
            CHECK(ueberlappung->value() == doctest::Approx(0.0));
            CHECK_FALSE(sitzung->ungespeichert());
            // ohne Edit sind die Felder gesperrt
            edit->click();
            CHECK_FALSE(ueberlappung->isEnabled());
            CHECK_FALSE(songsDoppelt->isEnabled());
            edit->click();
            CHECK(ueberlappung->isEnabled());
        }

        // --- die Pegelkurve der ganzen Komposition (S100, Entscheid Patrik F7 c): die Zeile
        //     MASTER unter dem Lineal — anklickbar wie ein Clip, mit Griffen, Stuecken an
        //     fester Zeit und dem Curve Editor; alles ueber Undo
        {
            const int master = Zeitachse::kMaster;
            const auto ziehPunkt = [&](QPoint a, QPoint b) {
                QMouseEvent druck(QEvent::MouseButtonPress, QPointF(a), QPointF(achse->mapToGlobal(a)),
                                  Qt::LeftButton, Qt::LeftButton, Qt::AltModifier);
                QCoreApplication::sendEvent(achse, &druck);
                for (int i = 1; i <= 4; ++i)
                {
                    const QPoint m = a + (b - a) * i / 4;
                    QMouseEvent bewegt(QEvent::MouseMove, QPointF(m), QPointF(achse->mapToGlobal(m)),
                                       Qt::NoButton, Qt::LeftButton, Qt::AltModifier);  // Alt: ohne Einrasten
                    QCoreApplication::sendEvent(achse, &bewegt);
                }
                QMouseEvent los(QEvent::MouseButtonRelease, QPointF(b), QPointF(achse->mapToGlobal(b)),
                                Qt::LeftButton, Qt::NoButton, Qt::AltModifier);
                QCoreApplication::sendEvent(achse, &los);
            };
            auto* einblende = panel.findChild<QDoubleSpinBox*>(QStringLiteral("composerFadeIn"));
            REQUIRE(einblende != nullptr);
            REQUIRE_FALSE(sitzung->komposition().hatKurve());
            const int kanalZeileVorher = achse->yVonZeile(0);
            CHECK(kanalZeileVorher > Zeitachse::kKanaeleOben);  // die Kanaele beginnen unter der Zeile MASTER

            // ein Klick in die Zeile waehlt die Kurve — keinen Clip eines Kanals
            const QPoint mitte(achse->xVon(30000), achse->yVonZeile(-1));
            ziehPunkt(mitte, mitte);
            CHECK(achse->gesamtGewaehlt());
            REQUIRE(achse->auswahl() != nullptr);
            CHECK(achse->auswahl()->dauerMs() == 60000);
            CHECK_FALSE(mischart->isEnabled());
            CHECK_FALSE(achse->kopiere());  // sie ist kein Clip
            QKeyEvent entf(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &entf);  // Entf loescht nichts
            CHECK(sitzung->laengeMs() == 60000);
            CHECK_FALSE(sitzung->ungespeichert());
            CHECK(achse->blendenGriff(master, 0, true).x() >= 0);  // der Griff sitzt am Anfang, auch ohne Blende

            // das Feld Fade in: der Griff der Blende steht in der Zeile bei 6 s
            einblende->setValue(6.0);
            const QPoint griff = achse->blendenGriff(master, 0, true);
            REQUIRE(griff.x() >= 0);
            CHECK(std::abs(griff.x() - achse->xVon(6000)) <= 1);
            CHECK(griff.y() < Zeitachse::kKanaeleOben);
            CHECK(griff.y() > Zeitachse::kLinealHoehe);
            // am Griff ziehen: die Dauer folgt, das Feld auch
            ziehPunkt(griff, QPoint(achse->xVon(12000), griff.y()));
            CHECK(std::llabs(sitzung->komposition().einblendeMs - 12000) <= 400);
            CHECK(einblende->value() == doctest::Approx(sitzung->komposition().einblendeMs / 1000.0).epsilon(0.02));
            CHECK(achse->gesamtGewaehlt());
            // die Form (der Weg des Rechtsklicks)
            achse->setzeBlendenForm(master, 0, true, Kurvenform::SKurve);
            CHECK(sitzung->komposition().einblendeForm.punkte.front().form == Kurvenform::SKurve);

            // ein Stueck bei 30 s: es liegt an fester Zeit, der Curve Editor geht auf
            achse->stueckHinzu(master, 0, 30000);
            REQUIRE(sitzung->komposition().gesamtStuecke.size() == 1);
            CHECK(sitzung->komposition().gesamtStuecke[0].mitteMs == 30000);
            CHECK(sitzung->komposition().blendeBei(30000) == doctest::Approx(0.0));
            KurvenFenster* kurven = panel.kurvenFenster();
            REQUIRE(kurven != nullptr);
            CHECK(kurven->hatKurve());
            CHECK(kurven->teil() == Teil{TeilArt::Stueck, 0});
            // an der Raute ziehen: es wandert
            const QPoint raute = achse->stueckGriff(master, 0, 0, 0);
            REQUIRE(raute.x() >= 0);
            CHECK(raute.y() < Zeitachse::kKanaeleOben);
            ziehPunkt(raute, QPoint(achse->xVon(40000), raute.y()));
            REQUIRE(sitzung->komposition().gesamtStuecke.size() == 1);
            CHECK(std::llabs(sitzung->komposition().gesamtStuecke[0].mitteMs - 40000) <= 400);
            // der Curve Editor aendert die Kurve der Komposition, nicht die eines Clips
            REQUIRE(achse->auswahl() != nullptr);
            Clip ausEditor = *achse->auswahl();
            ausEditor.grundpegel = 0.5;
            Q_EMIT kurven->clipGeaendert(ausEditor, QStringLiteral("Change base level"));
            CHECK(sitzung->komposition().grundpegel == doctest::Approx(0.5));
            CHECK(sitzung->komposition().gesamtStuecke.size() == 1);
            CHECK(std::llabs(sitzung->komposition().gesamtStuecke[0].mitteMs - 40000) <= 400);
            CHECK(ch1()[1].grundpegel == doctest::Approx(1.0));
            if (const QString bild = qEnvironmentVariable("LUMIVIZ_COMPOSER_PANEL_BILD"); !bild.isEmpty())
            {
                QCoreApplication::processEvents();
                CHECK(panel.grab().save(bild + QStringLiteral(".master.png")));
            }
            // das Stueck loeschen
            achse->stueckLoeschen(master, 0, 0);
            CHECK(sitzung->komposition().gesamtStuecke.empty());
            // Undo nimmt alles zurueck: sieben Schritte
            for (int i = 0; i < 7; ++i) REQUIRE(bus->undo());
            CHECK_FALSE(sitzung->komposition().hatKurve());
            CHECK(einblende->value() == doctest::Approx(0.0));
            CHECK_FALSE(sitzung->ungespeichert());
            kurven->close();

            // die rechte Kante der Zeile (Wunsch Patrik S100): bei Fixed end das Ende ziehen —
            // ohne Fixed end folgt das Ende dem letzten Clip, die Kante laesst sich nicht ziehen
            const auto kante = [&] {
                return QPoint(achse->xVon(sitzung->laengeMs()) - 2, achse->yVonZeile(-1) + 6);
            };
            ziehPunkt(kante(), QPoint(achse->xVon(70000), kante().y()));
            CHECK(sitzung->komposition().laengeMs == 0);
            CHECK(sitzung->laengeMs() == 60000);
            CHECK_FALSE(sitzung->ungespeichert());
            auto* fest = panel.findChild<QCheckBox*>(QStringLiteral("composerFixedEnd"));
            auto* zeit = panel.findChild<QDoubleSpinBox*>(QStringLiteral("composerEndTime"));
            REQUIRE(fest != nullptr);
            REQUIRE(zeit != nullptr);
            fest->click();
            REQUIRE(sitzung->komposition().laengeMs == 60000);
            ziehPunkt(kante(), QPoint(achse->xVon(70000), kante().y()));
            CHECK(std::llabs(sitzung->laengeMs() - 70000) <= 400);
            CHECK(zeit->value() == doctest::Approx(sitzung->laengeMs() / 1000.0).epsilon(0.02));
            CHECK(achse->gesamtGewaehlt());
            REQUIRE(bus->undo());  // das Ziehen
            CHECK(sitzung->laengeMs() == 60000);
            REQUIRE(bus->undo());  // Fixed end
            CHECK(sitzung->komposition().laengeMs == 0);
            CHECK_FALSE(sitzung->ungespeichert());
        }

        // --- Kanalfarben (S100, Entscheid Patrik K1 b): die geladene Datei hat ihre Farben
        //     bekommen, ohne als geaendert zu gelten; setzen und zuruecksetzen ueber Undo
        {
            REQUIRE_FALSE(sitzung->ungespeichert());
            const std::int32_t vorher = sitzung->komposition().kanaele[0].farbe;
            CHECK(vorher == reihenFarbe(sitzung->komposition().kanaele[0].art, 0));
            achse->kanalFaerben(0, 0x20C040);
            CHECK(sitzung->komposition().kanaele[0].farbe == 0x20C040);
            CHECK(sitzung->ungespeichert());
            achse->kanalFaerben(0, 0x20C040);  // dieselbe noch einmal: kein Schritt
            achse->kanalFaerben(0, -1);        // zurueck auf die Reihe
            CHECK(sitzung->komposition().kanaele[0].farbe == vorher);
            REQUIRE(bus->undo());
            CHECK(sitzung->komposition().kanaele[0].farbe == 0x20C040);
            REQUIRE(bus->undo());
            CHECK(sitzung->komposition().kanaele[0].farbe == vorher);
            CHECK_FALSE(sitzung->ungespeichert());
        }

        // --- eine andere Ladung: die alten Undo-Schritte tun nichts mehr
        REQUIRE(sitzung->lade(datei));
        REQUIRE(bus->canUndo());
        (void)bus->undo();
        CHECK(ch1()[0].endeMs == 27000);
        CHECK_FALSE(sitzung->ungespeichert());

        // --- Edit aus: wieder gesperrt
        edit->click();
        zug(10000, 15000, 0);
        CHECK(ch1()[0].startMs == 5000);
        CHECK_FALSE(sitzung->ungespeichert());
    }

    TEST_CASE("C3b Panel: eine neue Komposition, Presets hineinziehen, Kanaele anlegen und verwalten")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x0000FF);

        // eine Kettendatei mit Host Switcher wird erkannt, eine ohne nicht
        {
            ChainNode root;
            root.params = ListParams{};
            ChainNode schalter;
            schalter.params = HostSwitcherParams{};
            root.children.push_back(std::move(schalter));
            const QString mit = dir.filePath(QStringLiteral("Wechsler.lvfx"));
            REQUIRE(saveChainToFile(root, mit));
            CHECK(ComposerPanel::hatHostSwitcher(mit));
            CHECK_FALSE(ComposerPanel::hatHostSwitcher(rot));
            CHECK_FALSE(ComposerPanel::hatHostSwitcher(dir.filePath(QStringLiteral("fehlt.lvfx"))));
            CHECK_FALSE(ComposerPanel::hatHostSwitcher(dir.filePath(QStringLiteral("etwas.avs"))));
        }

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>(
            [](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);
        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }

        ComposerPanel panel(dienste);
        panel.setFenster(&widget);
        panel.resize(900, 300);
        testfenster::platziere(panel);
        panel.show();
        REQUIRE(warte([&] { return panel.zeitachse()->width() > 400; }));
        Sitzung* sitzung = panel.sitzung();
        Zeitachse* achse = panel.zeitachse();
        auto* edit = panel.findChild<QCheckBox*>(QStringLiteral("composerEdit"));
        auto* name = panel.findChild<QLabel*>(QStringLiteral("composerDatei"));
        auto* speichern = panel.findChild<QPushButton*>(QStringLiteral("composerSave"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("composerClip"));
        REQUIRE(panel.findChild<QPushButton*>(QStringLiteral("composerNew")) != nullptr);
        REQUIRE(panel.findChild<QPushButton*>(QStringLiteral("composerLoad")) != nullptr);
        REQUIRE(edit != nullptr);
        REQUIRE(name != nullptr);
        REQUIRE(speichern != nullptr);
        REQUIRE(clipZeile != nullptr);
        const auto kanaele = [&]() -> const std::vector<Kanal>& { return sitzung->komposition().kanaele; };

        // --- neu: ein leerer Kanal, noch ohne Datei; die Achse zeigt 30 s
        sitzung->neu();
        CHECK(sitzung->geladen());
        CHECK(sitzung->datei().isEmpty());
        CHECK_FALSE(sitzung->ungespeichert());
        CHECK_FALSE(sitzung->speichere());  // ohne Datei geht nur „Save As"
        REQUIRE(kanaele().size() == 1);
        CHECK(kanaele()[0].name == QStringLiteral("CH1"));
        CHECK(name->text() == QStringLiteral("untitled"));
        CHECK(speichern->isEnabled());
        CHECK(Zeitachse::anzeigeLaenge(sitzung->komposition()) == 30000);
        CHECK(achse->xVon(30000) == achse->width() - Zeitachse::kRand);

        // --- ohne Edit faellt nichts hinein
        CHECK_FALSE(achse->lasseFallen(QPoint(achse->xVon(5000), achse->yVonZeile(0)), {rot}));
        CHECK(kanaele()[0].clips.empty());

        // --- Edit an: ein Preset hineinziehen — 20 s lang, auf dem Raster
        edit->click();
        REQUIRE(achse->bearbeiten());
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(5200), achse->yVonZeile(0)), {rot}));
        REQUIRE(kanaele()[0].clips.size() == 1);
        CHECK(kanaele()[0].clips[0].datei == rot);
        CHECK(kanaele()[0].clips[0].startMs == 5000);
        CHECK(kanaele()[0].clips[0].endeMs == 25000);
        CHECK(sitzung->laengeMs() == 25000);
        CHECK(sitzung->ungespeichert());
        CHECK(name->text() == QStringLiteral("untitled *"));
        CHECK(clipZeile->text().startsWith(QStringLiteral("CH1 · Rot.lvfx · 0:05.0 – 0:25.0")));
        CHECK(bus->undoDescription() == "Add clip");
        // im Lineal faellt nichts
        CHECK_FALSE(achse->lasseFallen(QPoint(achse->xVon(1000), 5), {blau}));

        // --- C4: Ein- und Ausblenden an den Griffen oben im Clip, Form per Auswahl, Bézier-Anfasser
        {
            const auto zieh = [&](QPoint a, QPoint b) {
                QMouseEvent druck(QEvent::MouseButtonPress, QPointF(a), QPointF(achse->mapToGlobal(a)),
                                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QCoreApplication::sendEvent(achse, &druck);
                for (int i = 1; i <= 4; ++i)
                {
                    const QPoint m = a + (b - a) * i / 4;
                    QMouseEvent bewegt(QEvent::MouseMove, QPointF(m), QPointF(achse->mapToGlobal(m)),
                                       Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
                    QCoreApplication::sendEvent(achse, &bewegt);
                }
                QMouseEvent los(QEvent::MouseButtonRelease, QPointF(b), QPointF(achse->mapToGlobal(b)),
                                Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                QCoreApplication::sendEvent(achse, &los);
            };
            const auto derClip = [&]() -> const VisualClip& { return kanaele()[0].clips[0]; };
            Blenden b;
            REQUIRE(liesBlenden(derClip(), b));
            CHECK(b.einMs == 0);
            CHECK(b.ausMs == 0);
            CHECK(achse->bezierGriff(0, 0, true, 0) == QPoint(-1, -1));  // keine Bézier-Blende

            // der Griff der Einblendung sitzt oben links; nach rechts gezogen: 3 s Einblendung
            const QPoint ein = achse->blendenGriff(0, 0, true);
            REQUIRE(ein.x() >= 0);
            zieh(ein, QPoint(achse->xVon(8000), ein.y()));
            REQUIRE(liesBlenden(derClip(), b));
            CHECK(b.einMs == 3000);
            CHECK(b.ausMs == 0);
            CHECK(derClip().pegelBei(1500) == doctest::Approx(0.5));
            CHECK(derClip().startMs == 5000);  // der Clip selbst blieb, wo er war
            CHECK(derClip().endeMs == 25000);
            CHECK(bus->undoDescription() == "Change fade");
            // der Griff der Ausblendung oben rechts; nach links gezogen: 5 s Ausblendung
            const QPoint aus = achse->blendenGriff(0, 0, false);
            zieh(aus, QPoint(achse->xVon(20000), aus.y()));
            REQUIRE(liesBlenden(derClip(), b));
            CHECK(b.einMs == 3000);
            CHECK(b.ausMs == 5000);
            CHECK(derClip().pegelBei(17500) == doctest::Approx(0.5));

            // die Form: Bézier — die Vorgabe ist eine weiche S-Kurve
            achse->setzeBlendenForm(0, 0, true, Kurvenform::Bezier);
            REQUIRE(liesBlenden(derClip(), b));
            CHECK(b.einForm == Kurvenform::Bezier);
            CHECK(b.ausForm == Kurvenform::Linear);
            CHECK(derClip().pegelBei(1500) == doctest::Approx(0.5).epsilon(0.02));
            CHECK(derClip().pegelBei(600) < 0.15);
            CHECK(bus->undoDescription() == "Change fade shape");
            // der erste Anfasser nach oben links gezogen: die Kurve steigt sofort steil
            const QPoint a1 = achse->bezierGriff(0, 0, true, 0);
            REQUIRE(a1.x() >= 0);
            zieh(a1, QPoint(achse->xVon(5000) + 1, achse->yVonZeile(0) - Zeitachse::kZeilenHoehe / 2 + 5));
            REQUIRE(liesBlenden(derClip(), b));
            CHECK(b.einBezier[0] < 0.1);
            CHECK(b.einBezier[1] > 0.8);
            CHECK(derClip().pegelBei(600) > 0.3);
            // eine Blende der Laenge null bekommt mit der Form zwei Sekunden
            achse->setzeBlendenForm(0, 0, false, Kurvenform::SKurve);
            REQUIRE(liesBlenden(derClip(), b));
            CHECK(b.ausMs == 5000);  // sie hatte schon eine Laenge: bleibt
            CHECK(b.ausForm == Kurvenform::SKurve);

            if (const QString bild = qEnvironmentVariable("LUMIVIZ_COMPOSER_PANEL_BILD"); !bild.isEmpty())
            {
                QCoreApplication::processEvents();
                CHECK(panel.grab().save(bild + QStringLiteral(".blenden.png")));
            }
            // gespeichert und gelesen: Form und Anfasser bleiben
            const QString probe = dir.filePath(QStringLiteral("blenden.lvcomp"));
            REQUIRE(speichereKomposition(sitzung->komposition(), probe));
            Komposition gelesen;
            REQUIRE(ladeKomposition(probe, gelesen));
            Blenden g;
            REQUIRE(liesBlenden(gelesen.kanaele[0].clips[0], g));
            CHECK(g.einForm == Kurvenform::Bezier);
            CHECK(g.einBezier[1] == doctest::Approx(b.einBezier[1]));
            CHECK(g.ausForm == Kurvenform::SKurve);
        }

        // --- C4 Teil 2: der Curve Editor — Punkte, Presets, Dauer, Grundpegel, Stuecke
        {
            QTemporaryDir presetDir;
            REQUIRE(presetDir.isValid());
            formpresets::setzeWurzelFuerTests(presetDir.path());
            const struct Zurueck
            {
                ~Zurueck() { formpresets::setzeWurzelFuerTests({}); }
            } zurueck;
            const auto zieh = [](QWidget* w, QPoint a, QPoint b) {
                QMouseEvent druck(QEvent::MouseButtonPress, QPointF(a), QPointF(w->mapToGlobal(a)),
                                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QCoreApplication::sendEvent(w, &druck);
                for (int i = 1; i <= 4; ++i)
                {
                    const QPoint m = a + (b - a) * i / 4;
                    QMouseEvent bewegt(QEvent::MouseMove, QPointF(m), QPointF(w->mapToGlobal(m)),
                                       Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
                    QCoreApplication::sendEvent(w, &bewegt);
                }
                QMouseEvent los(QEvent::MouseButtonRelease, QPointF(b), QPointF(w->mapToGlobal(b)),
                                Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
                QCoreApplication::sendEvent(w, &los);
            };
            const auto doppelklick = [](QWidget* w, QPoint ort) {
                QMouseEvent doppel(QEvent::MouseButtonDblClick, QPointF(ort), QPointF(w->mapToGlobal(ort)),
                                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                QCoreApplication::sendEvent(w, &doppel);
            };
            const auto derClip = [&]() -> const VisualClip& { return kanaele()[0].clips[0]; };
            CHECK(panel.kurvenFenster() == nullptr);  // das Fenster entsteht erst mit der ersten Kurve

            // Doppelklick in die Einblendung (1 s nach Clip-Anfang) oeffnet ihre Kurve
            doppelklick(achse, QPoint(achse->xVon(6000), achse->yVonZeile(0) + 6));
            KurvenFenster* fenster = panel.kurvenFenster();
            REQUIRE(fenster != nullptr);
            testfenster::platziere(*fenster);
            CHECK(fenster->isVisible());
            REQUIRE(fenster->hatKurve());
            CHECK(fenster->teil() == Teil{TeilArt::Ein, -1});
            Formflaeche* flaeche = fenster->flaeche();
            REQUIRE(warte([&] { return flaeche->width() > 150 && flaeche->height() > 100; }));
            REQUIRE(flaeche->form().punkte.size() == 2);
            CHECK(flaeche->form().punkte[0].form == Kurvenform::Bezier);
            auto* titel = fenster->findChild<QLabel*>(QStringLiteral("curveTitle"));
            auto* presets = fenster->findChild<QComboBox*>(QStringLiteral("curvePreset"));
            auto* dauer = fenster->findChild<QDoubleSpinBox*>(QStringLiteral("curveDuration"));
            auto* lage = fenster->findChild<QDoubleSpinBox*>(QStringLiteral("curvePosition"));
            auto* gekippt = fenster->findChild<QCheckBox*>(QStringLiteral("curveFlipped"));
            auto* grund = fenster->findChild<QDoubleSpinBox*>(QStringLiteral("curveBase"));
            auto* jetzt = fenster->findChild<QLabel*>(QStringLiteral("curveNow"));
            auto* ansicht = fenster->findChild<QLabel*>(QStringLiteral("curveView"));
            REQUIRE(ansicht != nullptr);
            REQUIRE(titel != nullptr);
            REQUIRE(presets != nullptr);
            REQUIRE(dauer != nullptr);
            REQUIRE(lage != nullptr);
            REQUIRE(gekippt != nullptr);
            REQUIRE(grund != nullptr);
            REQUIRE(jetzt != nullptr);
            CHECK(titel->text() == QStringLiteral("Fade in — Rot"));
            CHECK(dauer->value() == doctest::Approx(3.0));
            CHECK_FALSE(lage->isVisible());  // Lage und Flipped gibt es nur am Stueck

            // ein Punkt dazu: die Blende hat zwei Abschnitte — ueber Undo, das Fenster zeigt den Stand
            flaeche->fuegePunktEin(0.5, 0.9);
            REQUIRE(derClip().ein.form.punkte.size() == 3);
            CHECK(derClip().ein.form.punkte[1].y == doctest::Approx(0.9));
            CHECK(derClip().pegelBei(1500) == doctest::Approx(0.9));
            CHECK(bus->undoDescription() == "Add curve point");
            CHECK(flaeche->form().punkte.size() == 3);
            CHECK(flaeche->gewaehlterPunkt() == 1);
            // die Laenge laesst sich auf der Zeitachse weiter ziehen, Anfasser gibt es dort keine mehr
            CHECK(achse->blendenGriff(0, 0, true).x() >= 0);
            CHECK(achse->bezierGriff(0, 0, true, 0) == QPoint(-1, -1));
            zieh(achse, achse->blendenGriff(0, 0, true),
                 QPoint(achse->xVon(9000), achse->blendenGriff(0, 0, true).y()));
            CHECK(derClip().ein.dauerMs == 4000);
            CHECK(derClip().ein.form.punkte.size() == 3);  // die Form blieb
            REQUIRE(bus->undo());
            CHECK(derClip().ein.dauerMs == 3000);

            // den Punkt mit der Maus ziehen: nach links unten
            zieh(flaeche, flaeche->punktOrt(1), flaeche->ortVon(0.25, 0.5));
            CHECK(derClip().ein.form.punkte[1].x == doctest::Approx(0.25).epsilon(0.03));
            CHECK(derClip().ein.form.punkte[1].y == doctest::Approx(0.5).epsilon(0.03));
            CHECK(bus->undoDescription() == "Move curve point");
            // die Enden einer Blende bleiben, wo sie sind
            zieh(flaeche, flaeche->punktOrt(0), flaeche->ortVon(0.2, 0.6));
            CHECK(derClip().ein.form.punkte[0].y == doctest::Approx(0.0));
            CHECK(bus->undoDescription() == "Move curve point");
            // Doppelklick ins Leere setzt einen Punkt, Doppelklick auf ihn nimmt ihn weg
            doppelklick(flaeche, flaeche->ortVon(0.8, 0.2));
            REQUIRE(derClip().ein.form.punkte.size() == 4);
            doppelklick(flaeche, flaeche->punktOrt(2));
            REQUIRE(derClip().ein.form.punkte.size() == 3);
            CHECK(bus->undoDescription() == "Delete curve point");
            doppelklick(flaeche, flaeche->punktOrt(0));  // ein Ende bleibt
            CHECK(derClip().ein.form.punkte.size() == 3);

            // Undo: Schritt fuer Schritt zurueck — das Fenster folgt
            REQUIRE(bus->undo());  // der Punkt bei 0.8 ist wieder da
            REQUIRE(bus->undo());  // ... und wieder weg
            REQUIRE(bus->undo());  // der gezogene Punkt liegt wieder bei 0.5 / 0.9
            CHECK(flaeche->form().punkte[1].x == doctest::Approx(0.5));
            REQUIRE(bus->undo());
            CHECK(derClip().ein.form.punkte.size() == 2);
            CHECK(flaeche->form().punkte.size() == 2);

            // die Form eines Abschnitts
            flaeche->setzeAbschnittForm(0, Kurvenform::EaseIn);
            CHECK(derClip().ein.form.punkte[0].form == Kurvenform::EaseIn);
            CHECK(bus->undoDescription() == "Change curve shape");

            // Presets: speichern, die Liste zeigt es als das geltende; aendern — „custom"; laden; loeschen
            CHECK(presets->count() == 1);
            REQUIRE(fenster->speicherePreset(QStringLiteral("Mein Anlauf")));
            REQUIRE(presets->count() == 2);
            CHECK(presets->currentText() == QStringLiteral("Mein Anlauf"));
            flaeche->setzeAbschnittForm(0, Kurvenform::Linear);
            CHECK(presets->currentIndex() == 0);
            REQUIRE(fenster->ladePreset(QStringLiteral("mein anlauf")));
            CHECK(derClip().ein.form.punkte[0].form == Kurvenform::EaseIn);
            CHECK(derClip().ein.dauerMs == 3000);  // die Dauer gehoert nicht zum Preset
            CHECK(bus->undoDescription() == "Load curve preset");
            CHECK(presets->currentText() == QStringLiteral("Mein Anlauf"));
            CHECK_FALSE(fenster->ladePreset(QStringLiteral("gibt es nicht")));
            // ein Stueck-Preset gilt nicht fuer eine Blende
            REQUIRE(formpresets::speichere(QStringLiteral("Tal"), delle()));
            CHECK_FALSE(fenster->ladePreset(QStringLiteral("Tal")));
            REQUIRE(fenster->loeschePreset(QStringLiteral("Mein Anlauf")));
            CHECK(presets->count() == 1);

            // dieselben Presets stehen im Rechtsklick-Menue der Zeitachse (hier: die Wahl daraus)
            REQUIRE(formpresets::speichere(QStringLiteral("Steil"), einfacheBlende(Kurvenform::Exponentiell)));
            achse->setzeBlendenPreset(0, 0, true, QStringLiteral("Steil"));
            CHECK(derClip().ein.form.punkte[0].form == Kurvenform::Exponentiell);
            CHECK(derClip().ein.dauerMs == 3000);                            // die Dauer bleibt
            CHECK(derClip().aus.form.punkte[0].form == Kurvenform::SKurve);  // die andere Seite auch
            CHECK(bus->undoDescription() == "Change fade shape");
            CHECK(presets->currentText() == QStringLiteral("Steil"));  // der Curve Editor zeigt es
            {
                const auto schritte = bus->undoCount();
                achse->setzeBlendenPreset(0, 0, true, QStringLiteral("gibt es nicht"));
                achse->setzeBlendenPreset(0, 0, true, QStringLiteral("Tal"));  // ein Stueck-Preset
                CHECK(bus->undoCount() == schritte);
            }
            REQUIRE(bus->undo());
            CHECK(derClip().ein.form.punkte[0].form == Kurvenform::EaseIn);

            // die Dauer; zusammen laenger als der Clip: beide Zahlen, die eingestellte bleibt
            dauer->setValue(4.0);
            CHECK(derClip().ein.dauerMs == 4000);
            CHECK(bus->undoDescription() == "Change fade duration");
            CHECK(jetzt->text().isEmpty());
            dauer->setValue(18.0);  // 18 s + 5 s in einem Clip von 20 s
            CHECK(derClip().ein.dauerMs == 18000);
            CHECK(derClip().aus.dauerMs == 5000);
            CHECK(dauer->value() == doctest::Approx(18.0));
            CHECK(jetzt->text() == QStringLiteral("(now 15.65 s)"));
            // von Hand aendern: erst gilt der sichtbare Stand — die Ausblendung behaelt, was sie zeigt
            dauer->setValue(2.0);
            CHECK(derClip().ein.dauerMs == 2000);
            CHECK(derClip().aus.dauerMs == 4348);
            CHECK(jetzt->text().isEmpty());

            // der Grundpegel des Clips
            grund->setValue(50.0);
            CHECK(derClip().grundpegel == doctest::Approx(0.5));
            CHECK(derClip().pegelBei(10000) == doctest::Approx(0.5));
            CHECK(derClip().pegelBei(1000) == doctest::Approx(0.5 * kurvenWert(Kurvenform::EaseIn, 0.5)));
            CHECK(bus->undoDescription() == "Change base level");

            // ein Stueck mitten im Clip: entsteht als Delle, das Fenster haengt daran
            achse->stueckHinzu(0, 0, 10000);
            REQUIRE(derClip().stuecke.size() == 1);
            CHECK(derClip().stuecke[0].lage == doctest::Approx(0.5));
            CHECK(derClip().stuecke[0].weiteMs == kNeuesStueckMs);
            CHECK(fenster->teil() == Teil{TeilArt::Stueck, 0});
            CHECK(titel->text() == QStringLiteral("Shape 1 — Rot"));
            CHECK(lage->isVisible());
            CHECK(lage->value() == doctest::Approx(10.0));
            CHECK(dauer->value() == doctest::Approx(2.0));
            CHECK(derClip().pegelBei(10000) == doctest::Approx(0.0));  // die Delle: vom Grundpegel auf null
            CHECK(presets->count() == 2);  // jetzt die Stueck-Presets: „Tal"
            CHECK(presets->currentText() == QStringLiteral("Tal"));
            // die Flaeche zeigt die Wirkung: die Delle haengt nach unten ...
            CHECK_FALSE(flaeche->gekippt());
            CHECK(flaeche->punktOrt(1).y() > flaeche->punktOrt(0).y());
            CHECK_FALSE(ansicht->isVisible());
            gekippt->click();  // der Buckel: vom Grundpegel auf voll
            // ... der Buckel steht nach oben — dieselbe Form, anders gezeigt
            CHECK(flaeche->gekippt());
            CHECK(flaeche->punktOrt(1).y() < flaeche->punktOrt(0).y());
            CHECK(ansicht->isVisible());
            CHECK(presets->currentText() == QStringLiteral("Tal"));
            CHECK(derClip().stuecke[0].gekippt);
            CHECK(derClip().pegelBei(10000) == doctest::Approx(1.0));
            CHECK(derClip().pegelBei(9000) == doctest::Approx(0.5));  // am Rand ohne Sprung
            CHECK(bus->undoDescription() == "Flip shape");
            lage->setValue(5.0);
            CHECK(derClip().stuecke[0].lage == doctest::Approx(0.25));
            CHECK(bus->undoDescription() == "Move shape");
            dauer->setValue(4.0);
            CHECK(derClip().stuecke[0].weiteMs == 4000);
            CHECK(bus->undoDescription() == "Change shape width");
            // die Enden eines Stuecks duerfen in der Hoehe wandern, nicht zur Seite
            zieh(flaeche, flaeche->punktOrt(0), flaeche->ortVon(0.3, 0.5));
            CHECK(derClip().stuecke[0].form.punkte[0].x == doctest::Approx(0.0));
            CHECK(derClip().stuecke[0].form.punkte[0].y == doctest::Approx(0.5).epsilon(0.03));
            REQUIRE(bus->undo());

            // das Fenster folgt dem Teil, das man oeffnet
            achse->oeffneKurve(0, 0, {TeilArt::Aus, -1});
            CHECK(fenster->teil() == Teil{TeilArt::Aus, -1});
            CHECK(titel->text() == QStringLiteral("Fade out — Rot"));
            CHECK(dauer->value() == doctest::Approx(4.35));  // 4348 ms, das Feld zeigt zwei Stellen
            // eine Ausblendung ist rueckwaerts gezeigt: sie faellt von links oben nach rechts unten
            CHECK(flaeche->rueckwaerts());
            CHECK(ansicht->isVisible());
            CHECK(flaeche->punktOrt(0).x() > flaeche->punktOrt(1).x());  // der Fuss der Blende liegt rechts
            CHECK(flaeche->punktOrt(0).y() > flaeche->punktOrt(1).y());
            // gezeichnet wird in dieser Ansicht: ein Punkt links unten — nach einem Viertel
            // der Ausblendung ist der Pegel schon tief
            const QRect ganz = flaeche->rect();
            doppelklick(flaeche, QPoint(ganz.left() + ganz.width() / 4, ganz.top() + ganz.height() * 3 / 4));
            REQUIRE(derClip().aus.form.punkte.size() == 3);
            CHECK(derClip().aus.form.punkte[1].x > 0.6);  // in der Form liegt er hinten
            CHECK(derClip().aus.form.punkte[1].y < 0.4);
            {
                const Bereich aus = derClip().verteilung().aus;
                const double pegel = derClip().pegelBei(aus.vonMs + aus.weiteMs() / 4);
                CHECK(pegel < 0.5 * 0.45);  // Grundpegel 0.5; linear waeren es 0.5 * 0.75
            }
            REQUIRE(bus->undo());
            CHECK(derClip().aus.form.punkte.size() == 2);
            doppelklick(achse, QPoint(achse->xVon(5000 + 5000), achse->yVonZeile(0) + 6));
            CHECK(fenster->teil() == Teil{TeilArt::Stueck, 0});

            // die Griffe des Stuecks oben im Clip: die Raute verschiebt, die Striche aendern die Weite
            const QPoint raute = achse->stueckGriff(0, 0, 0, 0);
            REQUIRE(raute.x() >= 0);
            CHECK(std::abs(raute.x() - achse->xVon(5000 + 5000)) <= 1);
            zieh(achse, raute, QPoint(achse->xVon(5000 + 10000), raute.y()));
            CHECK(derClip().stuecke[0].lage == doctest::Approx(0.5));
            CHECK(derClip().stuecke[0].weiteMs == 4000);
            CHECK(bus->undoDescription() == "Move shape");
            CHECK(lage->value() == doctest::Approx(10.0));  // der Curve Editor zeigt die neue Lage
            // gegen die Ausblendung gezogen: das Stueck haelt an ihr an, gestaucht wird nichts
            zieh(achse, achse->stueckGriff(0, 0, 0, 0), QPoint(achse->xVon(5000 + 19000), raute.y()));
            CHECK(bereichVon(derClip(), {TeilArt::Stueck, 0}).bisMs == derClip().verteilung().aus.vonMs);
            CHECK(derClip().stuecke[0].weiteMs == 4000);
            CHECK_FALSE(derClip().verteilung().aus.gestaucht);
            REQUIRE(bus->undo());
            // die rechte Kante nach innen: 2 s breit, die Mitte bleibt
            const QPoint rechts = achse->stueckGriff(0, 0, 0, 1);
            REQUIRE(rechts.x() >= 0);
            zieh(achse, rechts, QPoint(achse->xVon(5000 + 11000), rechts.y()));
            CHECK(derClip().stuecke[0].weiteMs == 2000);
            CHECK(derClip().stuecke[0].lage == doctest::Approx(0.5));
            CHECK(bus->undoDescription() == "Change shape width");
            CHECK(dauer->value() == doctest::Approx(2.0));
            // die linke Kante nach aussen: wieder 4 s
            zieh(achse, achse->stueckGriff(0, 0, 0, -1), QPoint(achse->xVon(5000 + 8000), rechts.y()));
            CHECK(derClip().stuecke[0].weiteMs == 4000);
            CHECK(achse->stueckGriff(0, 0, 7, 0) == QPoint(-1, -1));  // das Stueck gibt es nicht

            // ein Stueck-Preset aus dem Rechtsklick-Menue: nur die Form wechselt
            {
                Pegelform spitz = delle();
                spitz.punkte[0].form = Kurvenform::Linear;
                spitz.punkte[1].form = Kurvenform::Linear;
                REQUIRE(formpresets::speichere(QStringLiteral("Spitz"), spitz));
                achse->setzeStueckPreset(0, 0, 0, QStringLiteral("Spitz"));
                CHECK(gleich(derClip().stuecke[0].form, spitz));
                CHECK(derClip().stuecke[0].gekippt);  // Flipped, Lage und Weite bleiben
                CHECK(derClip().stuecke[0].weiteMs == 4000);
                CHECK(bus->undoDescription() == "Change shape curve");
                CHECK(presets->currentText() == QStringLiteral("Spitz"));
                const auto schritte = bus->undoCount();
                achse->setzeStueckPreset(0, 0, 0, QStringLiteral("Steil"));  // ein Blenden-Preset
                achse->setzeStueckPreset(0, 0, 4, QStringLiteral("Spitz"));  // kein solches Stueck
                CHECK(bus->undoCount() == schritte);
                REQUIRE(bus->undo());
                CHECK(gleich(derClip().stuecke[0].form, delle()));
            }

            if (const QString bild = qEnvironmentVariable("LUMIVIZ_COMPOSER_PANEL_BILD"); !bild.isEmpty())
            {
                QCoreApplication::processEvents();
                CHECK(fenster->grab().save(bild + QStringLiteral(".kurve.png")));
                CHECK(panel.grab().save(bild + QStringLiteral(".stueck.png")));
            }

            // ohne Edit: ansehen ja, aendern nein
            edit->click();
            REQUIRE_FALSE(achse->bearbeiten());
            CHECK(fenster->hatKurve());
            CHECK_FALSE(flaeche->bearbeitbar());
            const auto schritte = bus->undoCount();
            flaeche->fuegePunktEin(0.3, 0.3);
            CHECK(derClip().stuecke[0].form.punkte.size() == 3);
            CHECK(bus->undoCount() == schritte);
            edit->click();
            REQUIRE(achse->bearbeiten());
            CHECK(flaeche->bearbeitbar());

            // das Stueck weg: das Fenster hat nichts mehr zu zeigen
            achse->stueckLoeschen(0, 0, 0);
            CHECK(derClip().stuecke.empty());
            CHECK(bus->undoDescription() == "Delete shape");
            CHECK_FALSE(fenster->hatKurve());
            CHECK(titel->text() == QStringLiteral("No curve selected"));
            // eine Blende, die es nicht gibt, entsteht beim Oeffnen
            Komposition ohne = sitzung->komposition();
            ohne.kanaele[0].clips[0].aus = {};
            sitzung->setzeKomposition(ohne);
            achse->oeffneKurve(0, 0, {TeilArt::Aus, -1});
            CHECK(derClip().aus.dauerMs == 2000);
            CHECK(bus->undoDescription() == "Add fade");
            CHECK(fenster->hatKurve());
            // ebenso bei der Wahl eines Presets: zwei Sekunden, mit seiner Form
            ohne = sitzung->komposition();
            ohne.kanaele[0].clips[0].ein = {};
            sitzung->setzeKomposition(ohne);
            achse->setzeBlendenPreset(0, 0, true, QStringLiteral("Steil"));
            CHECK(derClip().ein.dauerMs == 2000);
            CHECK(derClip().ein.form.punkte[0].form == Kurvenform::Exponentiell);
        }

        // --- in eine Luecke: der neue Clip reicht nur bis zum naechsten
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(0), achse->yVonZeile(0)), {blau}));
        REQUIRE(kanaele()[0].clips.size() == 2);
        CHECK(kanaele()[0].clips[0].datei == blau);
        CHECK(kanaele()[0].clips[0].startMs == 0);
        CHECK(kanaele()[0].clips[0].endeMs == 5000);
        // auf einen Clip: er landet in der naechsten Luecke — hier dahinter
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(15000), achse->yVonZeile(0)), {blau}));
        REQUIRE(kanaele()[0].clips.size() == 3);
        CHECK(kanaele()[0].clips[2].startMs == 25000);
        CHECK(kanaele()[0].clips[2].endeMs == 45000);
        REQUIRE(bus->undo());
        REQUIRE(bus->undo());
        REQUIRE(kanaele()[0].clips.size() == 1);

        // --- unter der letzten Zeile: ein neuer Kanal entsteht; zwei Dateien liegen hintereinander
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(10000), achse->yVonZeile(1)), {blau, rot}));
        REQUIRE(kanaele().size() == 2);
        CHECK(kanaele()[1].name == QStringLiteral("CH2"));
        REQUIRE(kanaele()[1].clips.size() == 2);
        CHECK(kanaele()[1].clips[0].datei == blau);
        CHECK(kanaele()[1].clips[0].startMs == 10000);
        CHECK(kanaele()[1].clips[1].datei == rot);
        CHECK(kanaele()[1].clips[1].startMs == 30000);
        CHECK(kanaele()[1].clips[1].endeMs == 50000);
        CHECK(bus->undoDescription() == "Add clips");
        CHECK(achse->gewaehlterKanal() == 1);
        CHECK(achse->gewaehlterClip() == 1);

        // --- der echte Weg: ein Drop mit Datei-URLs, wie ihn der Effect Browser liefert
        {
            QMimeData daten;
            daten.setUrls({QUrl::fromLocalFile(blau)});
            const QPointF ort(achse->xVon(26000), achse->yVonZeile(0));
            QDragEnterEvent hinein(ort.toPoint(), Qt::CopyAction, &daten, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &hinein);
            CHECK(hinein.isAccepted());
            QDropEvent fall(ort, Qt::CopyAction, &daten, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &fall);
            CHECK(fall.isAccepted());
            REQUIRE(kanaele()[0].clips.size() == 2);
            CHECK(kanaele()[0].clips[1].datei == blau);
            CHECK(kanaele()[0].clips[1].startMs == 26000);
            // was kein Preset ist, wird nicht angenommen
            QMimeData fremd;
            fremd.setUrls({QUrl::fromLocalFile(dir.filePath(QStringLiteral("lied.mp3")))});
            QDragEnterEvent nein(ort.toPoint(), Qt::CopyAction, &fremd, Qt::LeftButton, Qt::NoModifier);
            nein.ignore();
            QCoreApplication::sendEvent(achse, &nein);
            CHECK_FALSE(nein.isAccepted());
        }

        // --- Kanaele: anlegen, umbenennen, ausschalten, loeschen — alles ueber Undo
        achse->kanalHinzu();
        REQUIRE(kanaele().size() == 3);
        CHECK(kanaele()[2].name == QStringLiteral("CH3"));
        achse->kanalUmbenennen(2, QStringLiteral("  Sterne "));
        CHECK(kanaele()[2].name == QStringLiteral("Sterne"));
        achse->kanalUmbenennen(2, QStringLiteral("   "));  // leer gilt nicht
        CHECK(kanaele()[2].name == QStringLiteral("Sterne"));
        achse->kanalSchalten(1, false);
        CHECK_FALSE(kanaele()[1].an);
        CHECK(bus->undoDescription() == "Disable channel");
        achse->kanalLoeschen(1);
        REQUIRE(kanaele().size() == 2);
        CHECK(kanaele()[1].name == QStringLiteral("Sterne"));
        CHECK(achse->kanalBei(achse->yVonZeile(1)) == 1);
        CHECK(achse->kanalBei(achse->yVonZeile(2)) == -1);
        CHECK(achse->kanalBei(5) == -1);
        REQUIRE(bus->undo());  // der Kanal samt seinen Clips ist wieder da
        REQUIRE(kanaele().size() == 3);
        CHECK(kanaele()[1].clips.size() == 2);
        // ein neuer Kanal bekommt die kleinste freie Nummer
        achse->kanalLoeschen(0);
        achse->kanalHinzu();
        CHECK(kanaele().back().name == QStringLiteral("CH1"));

        // --- gespeichert unter einem Namen: die Datei laesst sich wieder laden
        const QString ziel = dir.filePath(QStringLiteral("neu.lvcomp"));
        REQUIRE(sitzung->speichereNach(ziel));
        CHECK(name->text() == QStringLiteral("neu.lvcomp"));
        CHECK_FALSE(speichern->isEnabled());
        Komposition gelesen;
        REQUIRE(ladeKomposition(ziel, gelesen));
        CHECK(gelesen.kanaele.size() == 3);
        CHECK(gelesen.kanaele[0].clips.size() == 2);  // der fruehere CH2

        // --- schliessen: das Panel ist leer, Edit geht aus
        sitzung->entlade();
        CHECK_FALSE(edit->isChecked());
        CHECK_FALSE(edit->isEnabled());
        CHECK_FALSE(achse->bearbeiten());
    }

    TEST_CASE("C5 Panel: ein Audio-Kanal — anlegen, Songs hineinziehen, Wellenform, Kanten und Blenden")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        // zwei „Songs": leere Dateien genuegen, Laenge und Wellenform kommen aus der Engine aus Pappe
        const QString eins = dir.filePath(QStringLiteral("Eins.mp3"));
        const QString zwei = dir.filePath(QStringLiteral("Zwei.mp3"));
        for (const QString& pfad : {eins, zwei})
        {
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
        }

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>(
            [](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        dienste.registerSingleton<IAudioEngine>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new papp::PappEngine); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        auto* engine = dynamic_cast<papp::PappEngine*>(dienste.tryResolve<IAudioEngine>());
        REQUIRE(bus != nullptr);
        REQUIRE(engine != nullptr);
        engine->laengen = {{eins, 20000}, {zwei, 8000}};

        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        ComposerPanel panel(dienste);
        panel.setFenster(&widget);
        panel.resize(900, 300);
        testfenster::platziere(panel);
        panel.show();
        REQUIRE(warte([&] { return panel.zeitachse()->width() > 400; }));
        Sitzung* sitzung = panel.sitzung();
        Zeitachse* achse = panel.zeitachse();
        auto* edit = panel.findChild<QCheckBox*>(QStringLiteral("composerEdit"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("composerClip"));
        auto* mischart = panel.findChild<QComboBox*>(QStringLiteral("composerMischart"));
        REQUIRE(edit != nullptr);
        REQUIRE(clipZeile != nullptr);
        REQUIRE(mischart != nullptr);
        const auto kanaele = [&]() -> const std::vector<Kanal>& { return sitzung->komposition().kanaele; };
        const auto zieh = [&](QPoint a, QPoint b) {
            QMouseEvent druck(QEvent::MouseButtonPress, QPointF(a), QPointF(achse->mapToGlobal(a)),
                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &druck);
            for (int i = 1; i <= 4; ++i)
            {
                const QPoint m = a + (b - a) * i / 4;
                QMouseEvent bewegt(QEvent::MouseMove, QPointF(m), QPointF(achse->mapToGlobal(m)),
                                   Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
                QCoreApplication::sendEvent(achse, &bewegt);
            }
            QMouseEvent los(QEvent::MouseButtonRelease, QPointF(b), QPointF(achse->mapToGlobal(b)),
                            Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &los);
        };

        sitzung->neu();
        edit->click();
        REQUIRE(achse->bearbeiten());

        // --- ein Audio-Kanal: heisst A1, ueber Undo
        achse->audioKanalHinzu();
        REQUIRE(kanaele().size() == 2);
        CHECK(kanaele()[1].art == KanalArt::Audio);
        CHECK(kanaele()[1].name == QStringLiteral("A1"));
        CHECK(bus->undoDescription() == "Add audio channel");

        // --- ein Song in den Audio-Kanal: so lang, wie er ist
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(2000), achse->yVonZeile(1)), {eins}));
        REQUIRE(kanaele()[1].clips.size() == 1);
        CHECK(kanaele()[1].clips[0].datei == eins);
        CHECK(kanaele()[1].clips[0].startMs == 2000);
        CHECK(kanaele()[1].clips[0].endeMs == 22000);
        CHECK(kanaele()[1].clips[0].quellMs == 20000);
        CHECK(kanaele()[1].clips[0].einsatzMs == 0);
        CHECK(bus->undoDescription() == "Add song");
        CHECK(sitzung->laengeMs() == 22000);  // das Ende folgt auch dem Ton
        // die Clip-Zeile nennt am Ton den Einsatz, und Blend ist nicht zu waehlen
        CHECK(clipZeile->text().startsWith(QStringLiteral("A1 · Eins.mp3 · 0:02.0 – 0:22.0 · from 0:00.0")));
        CHECK_FALSE(mischart->isEnabled());

        // --- was nicht passt, faellt nicht: ein Preset in den Audio-Kanal, ein Song in den Visual-Kanal
        CHECK_FALSE(achse->lasseFallen(QPoint(achse->xVon(25000), achse->yVonZeile(1)), {rot}));
        CHECK_FALSE(achse->lasseFallen(QPoint(achse->xVon(1000), achse->yVonZeile(0)), {zwei}));
        CHECK(kanaele()[0].clips.empty());
        CHECK(kanaele()[1].clips.size() == 1);
        // aus einer gemischten Auswahl nimmt jede Zeile das Ihre
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(0), achse->yVonZeile(0)), {zwei, rot}));
        REQUIRE(kanaele()[0].clips.size() == 1);
        CHECK(kanaele()[0].clips[0].datei == rot);
        CHECK(mischart->isEnabled());  // der angeklickte Clip ist jetzt ein Bild-Clip

        // --- unter der letzten Zeile: ein neuer Kanal der Art der Datei
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(3000), achse->yVonZeile(2)), {zwei}));
        REQUIRE(kanaele().size() == 3);
        CHECK(kanaele()[2].art == KanalArt::Audio);
        CHECK(kanaele()[2].name == QStringLiteral("A2"));
        CHECK(kanaele()[2].clips[0].endeMs == 3000 + 8000);
        REQUIRE(bus->undo());
        REQUIRE(kanaele().size() == 2);

        // --- der echte Weg: ein Drop mit Datei-URLs, wie Audio Browser und Playlist ihn liefern
        {
            QMimeData daten;
            daten.setUrls({QUrl::fromLocalFile(zwei)});
            const QPointF ort(achse->xVon(24000), achse->yVonZeile(1));
            QDragEnterEvent hinein(ort.toPoint(), Qt::CopyAction, &daten, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &hinein);
            CHECK(hinein.isAccepted());
            QDropEvent fall(ort, Qt::CopyAction, &daten, Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &fall);
            CHECK(fall.isAccepted());
            REQUIRE(kanaele()[1].clips.size() == 2);
            CHECK(kanaele()[1].clips[1].datei == zwei);
            CHECK(kanaele()[1].clips[1].startMs == 24000);
            CHECK(kanaele()[1].clips[1].endeMs == 32000);
        }

        // --- die Wellenform: die Huellkurve jedes Songs kommt im Hintergrund
        REQUIRE(warte([&] { return achse->hatHuelle(eins) && achse->hatHuelle(zwei); }));

        // --- die Kanten des Ton-Clips: links aendert den Einsatz, rechts schneidet ab — nie ueber den Song hinaus
        const auto song = [&]() -> const Clip& { return kanaele()[1].clips[0]; };
        const int y = achse->yVonZeile(1) + 6;
        zieh(QPoint(achse->xVon(2000) + 2, y), QPoint(achse->xVon(5000), y));
        CHECK(song().startMs == 5000);
        CHECK(song().einsatzMs == 3000);
        CHECK(song().endeMs == 22000);
        CHECK(bus->undoDescription() == "Resize clip");
        CHECK(clipZeile->text().contains(QStringLiteral("from 0:03.0 in the song")));
        zieh(QPoint(achse->xVon(5000) + 2, y), QPoint(achse->xVon(0), y));  // weiter, als der Song reicht
        CHECK(song().startMs == 2000);
        CHECK(song().einsatzMs == 0);
        zieh(QPoint(achse->xVon(22000) - 2, y), QPoint(achse->xVon(15000), y));
        CHECK(song().endeMs == 15000);
        zieh(QPoint(achse->xVon(15000) - 2, y), QPoint(achse->xVon(23500), y));  // der zweite Song liegt erst bei 24 s
        CHECK(song().endeMs == 22000);  // laenger als der Song geht nicht

        // --- Ein- und Ausblenden wie am Bild — der Pegel ist die Lautstaerke
        const QPoint ein = achse->blendenGriff(1, 0, true);
        REQUIRE(ein.x() >= 0);
        zieh(ein, QPoint(achse->xVon(6000), ein.y()));
        CHECK(song().ein.dauerMs == 4000);
        CHECK(song().pegelBei(2000) == doctest::Approx(0.5));
        achse->oeffneKurve(1, 0, {TeilArt::Ein, -1});  // der Curve Editor haengt auch an einem Ton-Clip
        REQUIRE(panel.kurvenFenster() != nullptr);
        testfenster::platziere(*panel.kurvenFenster());
        CHECK(panel.kurvenFenster()->hatKurve());

        if (const QString bild = qEnvironmentVariable("LUMIVIZ_COMPOSER_PANEL_BILD"); !bild.isEmpty())
        {
            QCoreApplication::processEvents();
            CHECK(panel.grab().save(bild + QStringLiteral(".ton.png")));
        }

        // --- gespeichert und gelesen: der Ton-Clip kommt ganz zurueck
        const QString ziel = dir.filePath(QStringLiteral("mitTon.lvcomp"));
        REQUIRE(sitzung->speichereNach(ziel));
        Komposition gelesen;
        REQUIRE(ladeKomposition(ziel, gelesen));
        CHECK(gleich(gelesen, sitzung->komposition()));
        CHECK(gelesen.kanaele[1].clips[0].ein.dauerMs == 4000);
    }

    TEST_CASE("S98 Panel: Schatten beim Hineinziehen, Vorgabelaenge, Einfuegemarke, Kopieren / Ausschneiden / Einfuegen")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        Zeitachse::leereAblage();

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString eins = dir.filePath(QStringLiteral("Eins.mp3"));
        {
            QFile f(eins);
            REQUIRE(f.open(QIODevice::WriteOnly));
        }

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>(
            [](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        dienste.registerSingleton<IAudioEngine>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new papp::PappEngine); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        auto* engine = dynamic_cast<papp::PappEngine*>(dienste.tryResolve<IAudioEngine>());
        REQUIRE(bus != nullptr);
        REQUIRE(engine != nullptr);
        engine->laengen = {{eins, 20000}};

        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        ComposerPanel panel(dienste);
        panel.setFenster(&widget);
        panel.resize(900, 300);
        testfenster::platziere(panel);
        panel.show();
        REQUIRE(warte([&] { return panel.zeitachse()->width() > 400; }));
        Sitzung* sitzung = panel.sitzung();
        Zeitachse* achse = panel.zeitachse();
        auto* edit = panel.findChild<QCheckBox*>(QStringLiteral("composerEdit"));
        auto* clipZeile = panel.findChild<QLabel*>(QStringLiteral("composerClip"));
        REQUIRE(edit != nullptr);
        REQUIRE(clipZeile != nullptr);
        const auto kanaele = [&]() -> const std::vector<Kanal>& { return sitzung->komposition().kanaele; };
        const auto ort = [&](int kanal, std::int64_t ms) { return QPoint(achse->xVon(ms), achse->yVonZeile(kanal)); };
        const auto taste = [&](int key) {
            QKeyEvent druck(QEvent::KeyPress, key, Qt::ControlModifier);
            QCoreApplication::sendEvent(achse, &druck);
        };

        sitzung->neu();  // CH1
        // ohne Edit faellt nichts — also auch kein Schatten
        CHECK(achse->fallSchatten(ort(0, 5000), {rot}).empty());
        edit->click();
        REQUIRE(achse->bearbeiten());
        achse->audioKanalHinzu();  // Kanal 1: A1
        achse->kanalHinzu();       // Kanal 2: CH2
        REQUIRE(kanaele().size() == 3);

        // --- der Schatten: wo ein hineingezogenes Preset landen wuerde, mit seinem Anfang
        CHECK(Sitzung::neuerClipMs() == 20000);  // die Vorgabelaenge
        auto schatten = achse->fallSchatten(ort(0, 5000), {rot});
        REQUIRE(schatten.size() == 1);
        CHECK(schatten[0].kanal == 0);
        CHECK(schatten[0].startMs == 5000);
        CHECK(schatten[0].endeMs == 25000);
        // in einer Zeile der anderen Art faellt nichts: kein Schatten
        CHECK(achse->fallSchatten(ort(1, 5000), {rot}).empty());
        CHECK(achse->fallSchatten(ort(0, 5000), {eins}).empty());
        // ein Song in A1: so lang, wie er ist
        schatten = achse->fallSchatten(ort(1, 3000), {eins});
        REQUIRE(schatten.size() == 1);
        CHECK(schatten[0].kanal == 1);
        CHECK(schatten[0].startMs == 3000);
        CHECK(schatten[0].endeMs == 23000);
        // unter der letzten Zeile: ein neuer Kanal
        schatten = achse->fallSchatten(ort(3, 2000), {rot});
        REQUIRE(schatten.size() == 1);
        CHECK(schatten[0].kanal == 3);
        // der Schatten aendert nichts ...
        CHECK(kanaele().size() == 3);
        CHECK(kanaele()[0].clips.empty());
        // ... und das Fallenlassen landet genau dort
        REQUIRE(achse->lasseFallen(ort(0, 5000), {rot}));
        REQUIRE(kanaele()[0].clips.size() == 1);
        CHECK(kanaele()[0].clips[0].startMs == 5000);
        CHECK(kanaele()[0].clips[0].endeMs == 25000);
        // auf einem Clip: der Schatten zeigt die naechste Luecke
        schatten = achse->fallSchatten(ort(0, 10000), {rot});
        REQUIRE(schatten.size() == 1);
        CHECK(schatten[0].startMs == 25000);

        // --- die Vorgabelaenge neuer Bild-Clips ist einstellbar
        Sitzung::setNeuerClipSekunden(8);
        schatten = achse->fallSchatten(ort(0, 40000), {rot});
        REQUIRE(schatten.size() == 1);
        CHECK(schatten[0].endeMs - schatten[0].startMs == 8000);
        REQUIRE(achse->lasseFallen(ort(0, 40000), {rot}));
        CHECK(kanaele()[0].clips[1].dauerMs() == 8000);
        Sitzung::setNeuerClipSekunden(5000);  // Unsinn wird geklemmt
        CHECK(Sitzung::neuerClipMs() == 600000);
        Sitzung::setNeuerClipSekunden(20);

        // --- die Einfuegemarke: ein Klick auf eine freie Stelle einer Zeile
        CHECK(achse->markeKanal() == -1);
        klick(*achse, ort(2, 12000));
        CHECK(achse->markeKanal() == 2);
        CHECK(achse->markeMs() == 12000);
        CHECK(achse->auswahl() == nullptr);
        CHECK(clipZeile->text().contains(QStringLiteral("Insert mark")));
        CHECK(clipZeile->text().contains(QStringLiteral("CH2")));
        // die Abspielmarke bleibt dabei, wo sie ist
        CHECK(sitzung->stelleMs() == 0);
        // ein Klick auf einen Clip waehlt ihn — die Marke bleibt
        klick(*achse, ort(0, 10000));
        REQUIRE(achse->auswahl() != nullptr);
        CHECK(achse->markeKanal() == 2);

        // --- kopieren und einfuegen: an die Marke, mit dem Anfang — und mit allem, was der Clip traegt
        achse->setzeBlendenForm(0, 0, true, Kurvenform::Linear);  // eine Einblendung von zwei Sekunden
        klick(*achse, ort(0, 10000));
        CHECK_FALSE(Zeitachse::ablageVoll());
        REQUIRE(achse->kopiere());
        bool ton = true;
        CHECK(Zeitachse::ablageVoll(&ton));
        CHECK_FALSE(ton);
        REQUIRE(achse->fuegeEin());
        REQUIRE(kanaele()[2].clips.size() == 1);
        CHECK(kanaele()[2].clips[0].startMs == 12000);
        CHECK(kanaele()[2].clips[0].endeMs == 32000);
        CHECK(kanaele()[2].clips[0].datei == rot);
        CHECK(kanaele()[2].clips[0].ein.dauerMs == 2000);
        CHECK(kanaele()[0].clips.size() == 2);  // das Original bleibt
        // der eingefuegte ist angeklickt, die Marke steht hinter ihm: noch einmal Einfuegen haengt an
        CHECK(achse->gewaehlterKanal() == 2);
        CHECK(achse->markeMs() == 32000);
        REQUIRE(achse->fuegeEin());
        REQUIRE(kanaele()[2].clips.size() == 2);
        CHECK(kanaele()[2].clips[1].startMs == 32000);
        // ueber Undo
        REQUIRE(bus->undo());
        CHECK(kanaele()[2].clips.size() == 1);
        REQUIRE(bus->redo());
        CHECK(kanaele()[2].clips.size() == 2);

        // --- mit den Tasten: Ctrl+X schneidet aus, Ctrl+V fuegt an der Marke ein
        klick(*achse, ort(2, 15000));
        REQUIRE(achse->auswahl() != nullptr);
        taste(Qt::Key_X);
        CHECK(kanaele()[2].clips.size() == 1);
        klick(*achse, ort(2, 60000));  // die Marke an eine neue Stelle
        CHECK(achse->markeMs() == 60000);
        taste(Qt::Key_V);
        REQUIRE(kanaele()[2].clips.size() == 2);
        CHECK(kanaele()[2].clips[1].startMs == 60000);
        CHECK(kanaele()[2].clips[1].dauerMs() == 20000);
        // Ctrl+C kopiert den angeklickten (den eben eingefuegten)
        Zeitachse::leereAblage();
        taste(Qt::Key_C);
        CHECK(Zeitachse::ablageVoll());

        // --- ein Bild-Clip geht nicht in einen Audio-Kanal
        klick(*achse, ort(1, 1000));
        CHECK(achse->markeKanal() == 1);
        CHECK_FALSE(achse->fuegeEin());
        CHECK(kanaele()[1].clips.empty());

        // --- ein Song: ohne Marke landet die Kopie hinter dem angeklickten Clip, in dessen Kanal
        REQUIRE(achse->lasseFallen(ort(1, 0), {eins}));
        REQUIRE(kanaele()[1].clips.size() == 1);
        achse->setMarke(-1, 0);
        klick(*achse, ort(1, 5000));
        REQUIRE(achse->auswahl() != nullptr);
        REQUIRE(achse->kopiere());
        CHECK(Zeitachse::ablageVoll(&ton));
        CHECK(ton);
        CHECK(achse->markeKanal() == -1);
        REQUIRE(achse->fuegeEin());
        REQUIRE(kanaele()[1].clips.size() == 2);
        CHECK(kanaele()[1].clips[1].startMs == 20000);
        CHECK(kanaele()[1].clips[1].quellMs == 20000);
        // und ein Song geht nicht in einen Bild-Kanal
        klick(*achse, ort(0, 100000));
        CHECK(achse->markeKanal() == 0);
        CHECK_FALSE(achse->fuegeEin());

        // --- ohne Edit: keine Marke, kein Einfuegen, kein Ausschneiden — Kopieren schon
        edit->click();
        REQUIRE_FALSE(achse->bearbeiten());
        CHECK(achse->markeKanal() == -1);
        klick(*achse, ort(1, 5000));
        REQUIRE(achse->auswahl() != nullptr);
        CHECK(achse->markeKanal() == -1);
        CHECK(achse->kopiere());
        CHECK_FALSE(achse->schneideAus());
        CHECK_FALSE(achse->fuegeEin());
        CHECK(kanaele()[1].clips.size() == 2);
        Zeitachse::leereAblage();
    }
}
