/**
 ****************************************************************************************
 * @file   test_LoopAkkordDialog.cpp
 * @brief  Tabs und Akkorde, Stufe 26e-3: eigene Griffbilder — die Regeln zum Bearbeiten,
 *         der Dialog *Edit chord* (S-T11 a I–P) und *New / Edit / Duplicate / Delete* im
 *         Fenster *Chords* (S-T8 a H) — docs/audio/Saiten_Konzept.md §1
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Eigene Akkorde in einem Wegwerf-Ordner (`setzeEigenenAkkordOrdnerFuerTests`), eigene
 * Einstellungen (`LumiVizTest` / `LoopAkkordDialog`); Menue und Dialoge ueber die
 * Bedienhilfe abgefangen, kein Fenster erscheint. Die Regeln ohne Qt-Anwendung.
 * `LUMIVIZ_CHORD_EDIT_BILD=<datei.png>` legt ein Bild des Dialogs ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/widgets/Bedienhilfe.hpp"
#include "loop/AkkordDialog.hpp"
#include "loop/AkkordFenster.hpp"
#include "loop/GriffbildBild.hpp"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <QTemporaryDir>
#include <QToolButton>

#include <vector>

using namespace lumi::loop;

namespace
{

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Anwendung()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopAkkordDialog"));
        QSettings().clear();
    }
    ~Anwendung() { QSettings().clear(); }
};

/// Eigene Akkorde in einem Wegwerf-Ordner, solange er lebt.
struct Wegwerf
{
    QTemporaryDir ordner;
    Wegwerf() { setzeEigenenAkkordOrdnerFuerTests(ordner.path()); }
    ~Wegwerf() { setzeEigenenAkkordOrdnerFuerTests(QString()); }
};

Saitensatz standard()
{
    Saitensatz s;
    s.leer = {40, 45, 50, 55, 59, 64};
    return s;
}

Akkord leeresGriffbild()
{
    Akkord a;
    a.art = QStringLiteral("major");
    a.leer = standard().leer;
    a.buende.assign(6, -1);
    a.finger.assign(6, 0);
    return a;
}

void klicke(QWidget* w, QPointF ort, Qt::MouseButton taste)
{
    QMouseEvent druck(QEvent::MouseButtonPress, ort, w->mapToGlobal(ort), taste, taste, Qt::NoModifier);
    QCoreApplication::sendEvent(w, &druck);
    QMouseEvent los(QEvent::MouseButtonRelease, ort, w->mapToGlobal(ort), taste, Qt::NoButton, Qt::NoModifier);
    QCoreApplication::sendEvent(w, &los);
}

}  // namespace

TEST_CASE("Loop Akkord Dialog 26e-3: die Regeln zum Bearbeiten eines Griffbilds, freie Namen")
{
    Akkord a = leeresGriffbild();
    // ein Punkt je Saite; derselbe Bund nimmt ihn weg (×), ein anderer versetzt ihn
    schaltePunkt(a, 1, 3);
    CHECK(a.buende == std::vector<int>{-1, 3, -1, -1, -1, -1});
    schaltePunkt(a, 1, 2);
    CHECK(a.buende[1] == 2);
    schaltePunkt(a, 1, 2);
    CHECK(a.buende[1] == -1);
    schaltePunkt(a, 9, 2);           // keine Saite
    schaltePunkt(a, 1, kBuendeMax + 1);  // kein Bund
    CHECK(a.buende == std::vector<int>(6, -1));
    // ueber dem Sattel: × ↔ ○, ein Punkt geht dabei weg
    schalteLeer(a, 3);
    CHECK(a.buende[3] == 0);
    schalteLeer(a, 3);
    CHECK(a.buende[3] == -1);
    schaltePunkt(a, 3, 4);
    schalteLeer(a, 3);  // ein gegriffener Punkt wird ○
    CHECK(a.buende[3] == 0);
    // der Finger nur an einem gegriffenen Bund; ein neuer Punkt verliert ihn
    schaltePunkt(a, 2, 2);
    setzeFinger(a, 2, 2);
    CHECK(a.finger[2] == 2);
    setzeFinger(a, 0, 1);  // die Saite spielt nicht
    CHECK(a.finger[0] == 0);
    setzeFinger(a, 2, 7);
    CHECK(a.finger[2] == 4);
    schaltePunkt(a, 2, 3);
    CHECK(a.finger[2] == 0);

    // ein Barre: F-Dur aus E-Dur — erst die Form, dann der Barre am 1. Bund ueber alle Saiten
    Akkord f = leeresGriffbild();
    schaltePunkt(f, 1, 3);
    schaltePunkt(f, 2, 3);
    schaltePunkt(f, 3, 2);
    setzeFinger(f, 1, 3);
    schaltePunkt(f, 0, 1);
    setzeBarre(f, 0, 5, 1);
    CHECK(f.buende == std::vector<int>{1, 3, 3, 2, 1, 1});  // hoeher gegriffene bleiben
    CHECK(f.barre == std::vector<int>{1});
    CHECK(f.finger[0] == 1);
    CHECK(f.finger[5] == 1);
    CHECK(f.finger[1] == 3);
    // nimmt man Punkte des Barres weg, bis nur einer bleibt, faellt er weg
    schaltePunkt(f, 4, 1);
    CHECK(f.barre == std::vector<int>{1});
    schaltePunkt(f, 5, 1);
    CHECK(f.barre.empty());

    // freie Namen und Loeschen, in einem Wegwerf-Ordner
    Wegwerf wegwerf;
    REQUIRE(wegwerf.ordner.isValid());
    CHECK(freierAkkordName(QStringLiteral("C/E")) == QStringLiteral("c_e"));
    CHECK(freierAkkordName(QString()) == QStringLiteral("chord"));
    Akkord c = leeresGriffbild();
    c.titel = QStringLiteral("C");
    c.buende = {-1, 3, 2, 0, 1, 0};
    REQUIRE(speichereAkkord(eigeneAkkordDatei(QStringLiteral("c")), c));
    CHECK(freierAkkordName(QStringLiteral("C")) == QStringLiteral("c_2"));
    REQUIRE(speichereAkkord(eigeneAkkordDatei(QStringLiteral("c_2")), c));
    CHECK(freierAkkordName(QStringLiteral("C")) == QStringLiteral("c_3"));
    CHECK(loescheEigenenAkkord(QStringLiteral("user:c_2")));
    CHECK_FALSE(QFileInfo::exists(eigeneAkkordDatei(QStringLiteral("c_2"))));
    CHECK_FALSE(loescheEigenenAkkord(QStringLiteral("guitar/c_major_1")));
    CHECK_FALSE(loescheEigenenAkkord(QStringLiteral("user:gibt_es_nicht")));
}

TEST_CASE("Loop Akkord Dialog 26e-3: der Dialog Edit chord — Griffbrett mit der Maus, Name, Undo, Save (LUMIVIZ_CHORD_EDIT_BILD)")
{
    Anwendung anwendung;
    Wegwerf wegwerf;
    REQUIRE(wegwerf.ordner.isValid());
    Strum strum;
    AkkordDialog dialog(leeresGriffbild(), standard(), strum, 4, AkkordSpielart::Ab);
    CHECK(dialog.windowTitle() == QStringLiteral("New chord — your chords"));
    std::vector<Muster> gehoert;
    QObject::connect(&dialog, &AkkordDialog::vorhoerenVerlangt, &dialog, [&](const Muster& m) { gehoert.push_back(m); });

    // P: ohne gespielte Saite wird nicht gespeichert, ein Hinweis sagt warum
    CHECK_FALSE(dialog.speichere());
    CHECK(dialog.hinweis() == QStringLiteral("Set at least one string to play"));
    CHECK(dialog.result() != QDialog::Accepted);

    // N: mit der Maus — x32010
    GriffbildEingabe* brett = dialog.eingabe();
    brett->resize(220, 260);
    const auto lage = [&] { return brett->lage(); };
    klicke(brett, lage().punkt(1, 3), Qt::LeftButton);
    klicke(brett, lage().punkt(2, 2), Qt::LeftButton);
    klicke(brett, lage().punkt(4, 1), Qt::LeftButton);
    klicke(brett, lage().oben(3), Qt::LeftButton);
    klicke(brett, lage().oben(5), Qt::LeftButton);
    CHECK(dialog.akkord().buende == std::vector<int>{-1, 3, 2, 0, 1, 0});
    CHECK(dialog.hinweis().isEmpty());  // die Aenderung nimmt den Hinweis weg
    // Rechtsklick auf einen Punkt: das Menue der Finger
    QStringList eintraege;
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        for (QAction* a : menue.actions()) eintraege << a->text();
        return menue.actions().at(3);  // Finger 3
    });
    klicke(brett, lage().punkt(1, 3), Qt::RightButton);
    lumi::ui::setzeMenueBeobachterFuerTests({});
    CHECK(eintraege == QStringList{QStringLiteral("No finger"), QStringLiteral("Finger 1"), QStringLiteral("Finger 2"),
                                   QStringLiteral("Finger 3"), QStringLiteral("Finger 4")});
    CHECK(dialog.akkord().finger[1] == 3);

    // L: der Vorschlag, Use name
    REQUIRE(dialog.vorschlag().has_value());
    CHECK(*dialog.vorschlag() == AkkordName{0, QStringLiteral("major"), -1});
    REQUIRE(dialog.uebernimmVorschlag());

    // M: anhoeren, die Spielart wechselt im Dialog und meldet sich
    std::vector<AkkordSpielart> gewaehlt;
    QObject::connect(&dialog, &AkkordDialog::spielartGewaehlt, &dialog, [&](AkkordSpielart a) { gewaehlt.push_back(a); });
    auto* zugleich = dialog.findChild<QToolButton*>(QStringLiteral("chordPlayTogether"));
    REQUIRE(zugleich != nullptr);
    zugleich->click();
    CHECK(gewaehlt == std::vector<AkkordSpielart>{AkkordSpielart::Zugleich});
    REQUIRE(gehoert.size() == 1);
    CHECK(gehoert.back().anschlaege == std::vector<Anschlag>{Anschlag{0, Anschlagart::Zugleich}});

    // O: Undo und Redo — der Finger, dann der letzte Punkt
    REQUIRE(dialog.nimmZurueck());
    CHECK(dialog.akkord().finger[1] == 0);
    REQUIRE(dialog.nimmZurueck());
    CHECK(dialog.akkord().buende[5] == -1);
    REQUIRE(dialog.stelleWiederHer());
    REQUIRE(dialog.stelleWiederHer());
    CHECK(dialog.akkord().finger[1] == 3);
    CHECK_FALSE(dialog.stelleWiederHer());

    // J: der erste Bund — ein Barre gezogen am 5. Bund (A-Form um zwei Saiten)
    dialog.setzeErstenBund(5);
    CHECK(dialog.ersterBund() == 5);
    CHECK(brett->fenster() == GriffbildFenster{5, AkkordDialog::kBuendeImDialog});
    Akkord jetzt = dialog.akkord();
    klicke(brett, lage().punkt(2, 7), Qt::LeftButton);  // die D-Saite auf 7
    CHECK(dialog.akkord().buende[2] == 7);
    {
        // vom Punkt am 7. Bund ueber die G- und die H-Saite gezogen: ein Barre am 7. Bund
        const QPointF von = lage().punkt(2, 7);
        const QPointF bis = lage().punkt(4, 7);
        QMouseEvent druck(QEvent::MouseButtonPress, von, brett->mapToGlobal(von), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QCoreApplication::sendEvent(brett, &druck);
        QMouseEvent los(QEvent::MouseButtonRelease, bis, brett->mapToGlobal(bis), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(brett, &los);
    }
    CHECK(dialog.akkord().buende == std::vector<int>{-1, 3, 7, 7, 7, 0});
    CHECK(dialog.akkord().barre == std::vector<int>{7});
    REQUIRE(dialog.nimmZurueck());
    REQUIRE(dialog.nimmZurueck());
    CHECK(dialog.akkord().buende == jetzt.buende);
    dialog.setzeErstenBund(1);

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_CHORD_EDIT_BILD"); !ziel.isEmpty())
    {
        dialog.setAttribute(Qt::WA_DontShowOnScreen);
        dialog.show();
        QCoreApplication::processEvents();
        CHECK(dialog.grab().save(ziel));
        MESSAGE("Dialog Edit chord: " << ziel.toStdString());
        dialog.hide();
    }

    // P: Save schreibt ein eigenes (Titel aus Use name), der Dialog ist angenommen
    dialog.setzeBeschreibung(QStringLiteral("My open C"));
    REQUIRE(dialog.speichere());
    CHECK(dialog.result() == QDialog::Accepted);
    CHECK(dialog.gespeichert() == QStringLiteral("user:c"));
    Akkord gelesen;
    REQUIRE(holeAkkord(QStringLiteral("user:c"), gelesen));
    CHECK(gelesen.titel == QStringLiteral("C"));
    CHECK(gelesen.art == QStringLiteral("major"));
    CHECK(gelesen.beschreibung == QStringLiteral("My open C"));
    CHECK(gelesen.buende == std::vector<int>{-1, 3, 2, 0, 1, 0});
    CHECK(gelesen.finger == std::vector<int>{0, 3, 0, 0, 0, 0});
    CHECK(gelesen.herkunft.isEmpty());
}

TEST_CASE("Loop Akkord Dialog 26e-3: im Fenster — New, Edit (Vorlage und eigenes), Duplicate, Delete")
{
    Anwendung anwendung;
    Wegwerf wegwerf;
    REQUIRE(wegwerf.ordner.isValid());
    AkkordFenster fenster;
    Strum strum;
    fenster.setzeZiel(QStringLiteral("Mel"), QStringLiteral("Steel"), standard(), strum, 4);
    fenster.liesNeu();

    // New: der Dialog kommt mit einem leeren Griffbild dieser Stimmung
    lumi::ui::setzeDialogBeobachterFuerTests([](QDialog& d) {
        auto& dialog = dynamic_cast<AkkordDialog&>(d);
        CHECK(dialog.akkord().leer == standard().leer);
        CHECK(dialog.akkord().buende == std::vector<int>(6, -1));
        for (const auto& [s, b] : std::vector<std::pair<int, int>>{{1, 3}, {2, 2}, {4, 1}}) dialog.klickeBund(s, b);
        dialog.klickeOben(3);
        dialog.klickeOben(5);
        REQUIRE(dialog.uebernimmVorschlag());
        REQUIRE(dialog.speichere());
        return dialog.result();
    });
    REQUIRE(fenster.neu());
    CHECK(fenster.gewaehlt() == QStringLiteral("user:c"));
    CHECK(fenster.ordner() == QStringLiteral("own"));
    CHECK(fenster.baumZeilen().back() == QStringLiteral("Your chords (1)"));
    CHECK(fenster.galerieZeilen() == QStringList{QStringLiteral("[C major · 1]"), QStringLiteral("user:c")});

    // Edit an einer Vorlage, zwei Buende verschoben: eine eigene Kopie (user:c_2) in der verschobenen Lage
    REQUIRE(fenster.waehleOrdner(QStringLiteral("root:0:major")));
    REQUIRE(fenster.waehle(QStringLiteral("guitar/c_major_1")));
    REQUIRE(fenster.verschiebe(2));
    QString titel;
    lumi::ui::setzeDialogBeobachterFuerTests([&](QDialog& d) {
        auto& dialog = dynamic_cast<AkkordDialog&>(d);
        titel = dialog.windowTitle();
        CHECK(dialog.akkord().buende == std::vector<int>{-1, 5, 4, 2, 3, 2});
        REQUIRE(dialog.speichere());
        return dialog.result();
    });
    REQUIRE(fenster.bearbeite());
    CHECK(titel == QStringLiteral("New chord — your chords"));
    CHECK(fenster.gewaehlt() == QStringLiteral("user:c_2"));
    Akkord vorlage;
    REQUIRE(holeAkkord(QStringLiteral("guitar/c_major_1"), vorlage));
    CHECK(vorlage.buende == std::vector<int>{-1, 3, 2, 0, 1, 0});  // die Vorlage bleibt

    // Edit an einem eigenen: es ersetzt sich, kein neues
    lumi::ui::setzeDialogBeobachterFuerTests([&](QDialog& d) {
        auto& dialog = dynamic_cast<AkkordDialog&>(d);
        titel = dialog.windowTitle();
        dialog.setzeTitel(QStringLiteral("D shape"));
        REQUIRE(dialog.speichere());
        return dialog.result();
    });
    REQUIRE(fenster.bearbeite());
    CHECK(titel == QStringLiteral("Edit chord — your chords"));
    CHECK(fenster.gewaehlt() == QStringLiteral("user:c_2"));
    Akkord ersetzt;
    REQUIRE(holeAkkord(QStringLiteral("user:c_2"), ersetzt));
    CHECK(ersetzt.titel == QStringLiteral("D shape"));
    CHECK(QDir(wegwerf.ordner.path()).entryList({QStringLiteral("*.lvchord")}).size() == 2);
    // Cancel speichert nichts
    lumi::ui::setzeDialogBeobachterFuerTests([](QDialog&) { return static_cast<int>(QDialog::Rejected); });
    CHECK_FALSE(fenster.bearbeite());
    CHECK_FALSE(fenster.neu());

    // Duplicate: ohne Dialog, Keep both
    REQUIRE(fenster.waehle(QStringLiteral("user:c")));
    REQUIRE(fenster.dupliziere());
    CHECK(fenster.gewaehlt() == QStringLiteral("user:c_3"));

    // Delete: nur ein eigenes, nach Rueckfrage; Favoriten und Zuletzt verlieren es
    fenster.setzeFavorit(true);
    REQUIRE(fenster.favoriten().contains(QStringLiteral("user:c_3")));
    QString frage;
    lumi::ui::setzeDialogBeobachterFuerTests([&](QDialog& d) {
        frage = d.objectName();
        return static_cast<int>(QMessageBox::No);
    });
    CHECK_FALSE(fenster.loescheGewaehlten());
    CHECK(frage == QStringLiteral("chordDeleteQuestion"));
    CHECK(QFileInfo::exists(eigeneAkkordDatei(QStringLiteral("c_3"))));
    lumi::ui::setzeDialogBeobachterFuerTests([](QDialog&) { return static_cast<int>(QMessageBox::Yes); });
    REQUIRE(fenster.loescheGewaehlten());
    lumi::ui::setzeDialogBeobachterFuerTests({});
    CHECK_FALSE(QFileInfo::exists(eigeneAkkordDatei(QStringLiteral("c_3"))));
    CHECK_FALSE(fenster.favoriten().contains(QStringLiteral("user:c_3")));
    CHECK(fenster.baumZeilen().back() == QStringLiteral("Your chords (2)"));
    REQUIRE(fenster.waehle(QStringLiteral("guitar/c_major_1")));
    CHECK_FALSE(fenster.loescheGewaehlten());  // eine Vorlage nicht
}
