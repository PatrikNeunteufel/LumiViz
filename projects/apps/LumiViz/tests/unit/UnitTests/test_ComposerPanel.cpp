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

#include "TestFenster.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Komposition.hpp"
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
        unten.visual = {clip(rot, 0, 20000), clip(blau, 30000, 60000)};
        unten.visual[0].setzeBlenden(3000, 5000);
        Kanal oben;
        oben.name = QStringLiteral("CH2");
        oben.visual = {clip(blau, 15000, 35000)};
        oben.visual[0].mischart = Mischart::Additive;
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
        CHECK(zeit->text().startsWith(QStringLiteral("0:30.0")));

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
        unten.visual = {clip(rot, 0, 20000), clip(blau, 30000, 60000)};
        Kanal oben;
        oben.name = QStringLiteral("CH2");
        oben.visual = {clip(blau, 42000, 52000)};
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
            return sitzung->komposition().kanaele[0].visual;
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
        REQUIRE(sitzung->komposition().kanaele[1].visual.size() == 2);
        CHECK(sitzung->komposition().kanaele[1].visual[0].startMs == 5000);
        CHECK(sitzung->komposition().kanaele[1].visual[0].endeMs == 27000);
        CHECK(achse->gewaehlterKanal() == 1);
        CHECK(achse->gewaehlterClip() == 0);
        CHECK(clipZeile->text().startsWith(QStringLiteral("CH2 · Rot.lvfx · 0:05.0 – 0:27.0")));
        // und weiter nach rechts auf den Clip dort: ohne Taste haelt er buendig davor
        zug(15000, 35000, 1);
        CHECK(sitzung->komposition().kanaele[1].visual[0].startMs == 20000);
        CHECK(sitzung->komposition().kanaele[1].visual[0].endeMs == 42000);
        REQUIRE(bus->undo());
        REQUIRE(bus->undo());
        REQUIRE(ch1().size() == 2);
        CHECK(ch1()[0].startMs == 5000);
        CHECK(sitzung->komposition().kanaele[1].visual.size() == 1);

        // --- Mischart des angeklickten Clips
        klick(*achse, QPoint(achse->xVon(47000), achse->yVonZeile(1)));  // CH2
        REQUIRE(achse->gewaehlterKanal() == 1);
        CHECK(mischart->currentIndex() == 0);
        Q_EMIT mischart->activated(static_cast<int>(Mischart::Maximum));
        CHECK(sitzung->komposition().kanaele[1].visual[0].mischart == Mischart::Maximum);
        CHECK(clipZeile->text().endsWith(QStringLiteral("Maximum")));
        CHECK(bus->undoDescription() == "Change blend mode");

        // --- Entf loescht den angeklickten Clip
        QKeyEvent entf(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        QCoreApplication::sendEvent(achse, &entf);
        CHECK(sitzung->komposition().kanaele[1].visual.empty());
        CHECK(achse->auswahl() == nullptr);
        REQUIRE(bus->undo());
        CHECK(sitzung->komposition().kanaele[1].visual.size() == 1);

        // --- Save schreibt in die geladene Datei; danach ist nichts mehr offen
        REQUIRE(sitzung->ungespeichert());
        speichern->click();
        CHECK_FALSE(sitzung->ungespeichert());
        CHECK(name->text() == QStringLiteral("probe.lvcomp"));
        Komposition gelesen;
        REQUIRE(ladeKomposition(datei, gelesen));
        CHECK(gelesen.kanaele[0].visual[0].endeMs == 27000);
        CHECK(gelesen.kanaele[1].visual[0].mischart == Mischart::Maximum);

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
        CHECK(kanaele()[0].visual.empty());

        // --- Edit an: ein Preset hineinziehen — 20 s lang, auf dem Raster
        edit->click();
        REQUIRE(achse->bearbeiten());
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(5200), achse->yVonZeile(0)), {rot}));
        REQUIRE(kanaele()[0].visual.size() == 1);
        CHECK(kanaele()[0].visual[0].datei == rot);
        CHECK(kanaele()[0].visual[0].startMs == 5000);
        CHECK(kanaele()[0].visual[0].endeMs == 25000);
        CHECK(sitzung->laengeMs() == 25000);
        CHECK(sitzung->ungespeichert());
        CHECK(name->text() == QStringLiteral("untitled *"));
        CHECK(clipZeile->text().startsWith(QStringLiteral("CH1 · Rot.lvfx · 0:05.0 – 0:25.0")));
        CHECK(bus->undoDescription() == "Add clip");
        // im Lineal faellt nichts
        CHECK_FALSE(achse->lasseFallen(QPoint(achse->xVon(1000), 5), {blau}));

        // --- in eine Luecke: der neue Clip reicht nur bis zum naechsten
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(0), achse->yVonZeile(0)), {blau}));
        REQUIRE(kanaele()[0].visual.size() == 2);
        CHECK(kanaele()[0].visual[0].datei == blau);
        CHECK(kanaele()[0].visual[0].startMs == 0);
        CHECK(kanaele()[0].visual[0].endeMs == 5000);
        // auf einen Clip: er landet in der naechsten Luecke — hier dahinter
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(15000), achse->yVonZeile(0)), {blau}));
        REQUIRE(kanaele()[0].visual.size() == 3);
        CHECK(kanaele()[0].visual[2].startMs == 25000);
        CHECK(kanaele()[0].visual[2].endeMs == 45000);
        REQUIRE(bus->undo());
        REQUIRE(bus->undo());
        REQUIRE(kanaele()[0].visual.size() == 1);

        // --- unter der letzten Zeile: ein neuer Kanal entsteht; zwei Dateien liegen hintereinander
        REQUIRE(achse->lasseFallen(QPoint(achse->xVon(10000), achse->yVonZeile(1)), {blau, rot}));
        REQUIRE(kanaele().size() == 2);
        CHECK(kanaele()[1].name == QStringLiteral("CH2"));
        REQUIRE(kanaele()[1].visual.size() == 2);
        CHECK(kanaele()[1].visual[0].datei == blau);
        CHECK(kanaele()[1].visual[0].startMs == 10000);
        CHECK(kanaele()[1].visual[1].datei == rot);
        CHECK(kanaele()[1].visual[1].startMs == 30000);
        CHECK(kanaele()[1].visual[1].endeMs == 50000);
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
            REQUIRE(kanaele()[0].visual.size() == 2);
            CHECK(kanaele()[0].visual[1].datei == blau);
            CHECK(kanaele()[0].visual[1].startMs == 26000);
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
        CHECK(kanaele()[1].visual.size() == 2);
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
        CHECK(gelesen.kanaele[0].visual.size() == 2);  // der fruehere CH2

        // --- schliessen: das Panel ist leer, Edit geht aus
        sitzung->entlade();
        CHECK_FALSE(edit->isChecked());
        CHECK_FALSE(edit->isEnabled());
        CHECK_FALSE(achse->bearbeiten());
    }
}
