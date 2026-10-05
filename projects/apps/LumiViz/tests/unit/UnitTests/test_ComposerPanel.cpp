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
}
