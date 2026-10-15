/**
 * @file test_LoopMenues.cpp
 * @brief Die Menues des Rechtsklicks, die kleinen Dialoge (Rename, Tags) und die
 *        Schieber im Loop (S125, Offene_Punkte §6 Loop: „ohne Test") — ueber
 *        `lumi::ui::zeigeMenue` und `fuehreDialogAus`: der Test bekommt Menue und
 *        Dialog statt der Anzeige, es erscheint kein Fenster.
 */

#include <doctest.h>

#include "UI/widgets/Bedienhilfe.hpp"
#include "loop/DrumEditor.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/InstrumentPalette.hpp"  // der Editor haelt sie in einem QPointer
#include "loop/KitFenster.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Taktachse.hpp"
#include "services/EventBus.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDialog>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QTemporaryDir>
#include <QToolButton>

using namespace lumi::loop;

namespace
{

/// Faengt Menues und Dialoge, solange er lebt.
struct Fang
{
    QStringList texte;     ///< die Eintraege des letzten Menues (ohne Trenner)
    QStringList grau;      ///< die grauen darunter
    QString waehle;        ///< diesen Eintrag waehlen; leer = keinen
    int menues = 0;
    QString dialogTitel;
    QString dialogText;    ///< bei einer Textfrage: was vorbelegt war
    std::function<int(QDialog&)> dialog = [](QDialog&) { return static_cast<int>(QDialog::Rejected); };

    Fang()
    {
        lumi::ui::setzeMenueBeobachterFuerTests([this](QMenu& menue) -> QAction* {
            ++menues;
            texte.clear();
            grau.clear();
            QAction* gewaehlt = nullptr;
            for (QAction* a : menue.actions())
            {
                if (a->isSeparator()) continue;
                texte << a->text();
                if (!a->isEnabled()) grau << a->text();
                if (!waehle.isEmpty() && a->text() == waehle) gewaehlt = a;
            }
            return gewaehlt;
        });
        lumi::ui::setzeDialogBeobachterFuerTests([this](QDialog& d) {
            dialogTitel = d.windowTitle();
            if (auto* frage = qobject_cast<QInputDialog*>(&d)) dialogText = frage->textValue();
            return dialog(d);
        });
    }
    ~Fang()
    {
        lumi::ui::setzeMenueBeobachterFuerTests({});
        lumi::ui::setzeDialogBeobachterFuerTests({});
    }
};

/// Eine Textfrage beantworten.
std::function<int(QDialog&)> antworte(const QString& text)
{
    return [text](QDialog& d) {
        if (auto* frage = qobject_cast<QInputDialog*>(&d)) frage->setTextValue(text);
        return static_cast<int>(QDialog::Accepted);
    };
}

void rechts(QWidget& ziel, QPoint ort)
{
    QContextMenuEvent ereignis(QContextMenuEvent::Mouse, ort, ziel.mapToGlobal(ort));
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

/// Wie in test_LoopArrangement: A (4 Takte) und B (8), Drums: A 0–8, B 8–16; Perc: A 4–8.
Loop arrangement()
{
    Loop loop;
    Muster a;
    a.name = QStringLiteral("A");
    a.takte = 4;
    Muster b;
    b.name = QStringLiteral("B");
    b.takte = 8;
    loop.muster = {a, b};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("A"), 0, 2}, {QStringLiteral("B"), 8, 1}}},
                    {QStringLiteral("Perc"), true, {{QStringLiteral("A"), 4, 1}}}};
    return loop;
}

/// Wie in test_LoopDrumEditor: die Basis Grund mit Kick und Snare, zuletzt mit dem Kit 808
/// (dann bietet Add track die Rollen an, die fehlen).
Loop schlagzeug()
{
    Loop loop;
    Muster grund;
    grund.name = QStringLiteral("Grund");
    grund.takte = 4;
    grund.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
                    {QStringLiteral("Snare"), QStringLiteral("kit:snare"), 0.8, 0.0, false}};
    grund.zeilen = {std::vector<double>(64, 0.0), std::vector<double>(64, 0.0)};
    grund.zeilen[0][0] = 1.0;
    grund.kit = QStringLiteral("808");
    loop.muster = {grund};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("Grund"), 0, 1}}}};
    loop.richte();
    return loop;
}

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopMenues"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

}  // namespace

TEST_SUITE("Loop Menues")
{
    TEST_CASE("Loop Menues: die Taktachse — Clip, Kanal, freie Stelle, mehrere Clips, Lineal")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        Fang fang;

        Taktachse achse;  // nicht gezeigt
        achse.resize(900, 260);
        achse.setLoop(arrangement(), true);
        int aenderungen = 0;
        Loop letzter = arrangement();
        QObject::connect(&achse, &Taktachse::bearbeitet, &achse, [&](const Loop& neu, const QString&) {
            ++aenderungen;
            letzter = neu;
            achse.zeigeBearbeitet(neu);
        });
        const int drumsY = achse.clipRechteck(0, 0).center().y();
        const int percY = achse.clipRechteck(1, 0).center().y();

        // --- auf einem Clip: die Eintraege, Rename base... fragt mit dem Namen der Basis
        rechts(achse, achse.clipRechteck(0, 0).center());
        for (const char* text : {"Edit base", "Make new base and edit", "Effects...", "Duplicate", "Rename base...", "Delete"})
        {
            CAPTURE(text);
            CHECK(fang.texte.contains(QString::fromLatin1(text)));
        }
        CHECK(fang.grau.isEmpty());
        fang.waehle = QStringLiteral("Rename base...");
        fang.dialog = antworte(QStringLiteral("Neu"));
        rechts(achse, achse.clipRechteck(0, 0).center());
        CHECK(fang.dialogTitel == QStringLiteral("Rename base"));
        CHECK(fang.dialogText == QStringLiteral("A"));
        CHECK(aenderungen == 1);
        CHECK(letzter.kanaele[0].clips[0].muster == QStringLiteral("Neu"));

        // --- am Kanalnamen: abgebrochen geschieht nichts
        fang.waehle = QStringLiteral("Rename channel...");
        fang.dialog = [](QDialog&) { return static_cast<int>(QDialog::Rejected); };
        rechts(achse, QPoint(10, percY));
        CHECK(fang.texte == QStringList{QStringLiteral("Rename channel..."), QStringLiteral("Move channel up"),
                                        QStringLiteral("Move channel down"), QStringLiteral("Turn channel off"),
                                        QStringLiteral("Delete channel")});
        CHECK(fang.grau == QStringList{QStringLiteral("Move channel down")});  // Perc ist der letzte
        CHECK(fang.dialogTitel == QStringLiteral("Rename channel"));
        CHECK(fang.dialogText == QStringLiteral("Perc"));
        CHECK(aenderungen == 1);

        // --- an einer freien Stelle: ein neuer Clip
        fang.waehle = QStringLiteral("New drumkit clip");
        const QPoint frei(achse.xVonTakt(17) + 4, drumsY);
        rechts(achse, frei);
        for (const char* text : {"New drumkit clip", "New instrument clip", "Add channel", "Select all clips"})
        {
            CAPTURE(text);
            CHECK(fang.texte.contains(QString::fromLatin1(text)));
        }
        CHECK(aenderungen == 2);
        CHECK(letzter.kanaele[0].clips.size() == 3);

        // --- alle markiert: Duplicate und Delete fuer alle, die Basis grau
        fang.waehle = QStringLiteral("Select all clips");
        rechts(achse, QPoint(achse.xVonTakt(19) + 4, percY));
        fang.waehle.clear();
        rechts(achse, achse.clipRechteck(0, 0).center());
        CHECK(fang.texte.contains(QStringLiteral("Duplicate 4 clips")));
        CHECK(fang.texte.contains(QStringLiteral("Delete 4 clips")));
        CHECK(fang.grau.contains(QStringLiteral("Edit base")));
        CHECK(fang.grau.contains(QStringLiteral("Rename base...")));

        // --- im Lineal: nur der Zoom
        rechts(achse, QPoint(achse.xVonTakt(2), 5));
        CHECK_FALSE(fang.texte.isEmpty());
        CHECK_FALSE(fang.texte.contains(QStringLiteral("Delete")));
        CHECK(aenderungen == 2);
    }

    TEST_CASE("Loop Menues: der Drum-Editor — Schritt, Spur, Rolle, Add track, die Schieber")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        Fang fang;

        DrumEditor editor;  // nicht gezeigt
        int aenderungen = 0;
        QString zuletzt;
        Loop letzter = schlagzeug();
        QObject::connect(&editor, &DrumEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            ++aenderungen;
            zuletzt = was;
            letzter = neu;
            editor.setLoop(neu);
        });
        REQUIRE(editor.zeige(schlagzeug(), QStringLiteral("Grund")));
        const auto grund = [&letzter]() -> const Muster& { return letzter.muster[0]; };

        // --- ein gesetzter Schritt: Clear this step
        SchrittRaster& raster = *editor.raster();
        fang.waehle = QStringLiteral("Clear this step");
        rechts(raster, QPoint(raster.xVonSchritt(0) + 3, raster.yVonSpur(0) + 5));
        CHECK(fang.texte.contains(QStringLiteral("Clear this step")));
        CHECK(fang.texte.contains(QStringLiteral("Select all")));
        CHECK(aenderungen == 1);
        CHECK(grund().zeilen[0][0] == 0.0);

        // --- die Spur: Rename track...
        fang.waehle = QStringLiteral("Rename track...");
        fang.dialog = antworte(QStringLiteral("Snare 2"));
        editor.spurMenue(1, QPoint());
        CHECK(fang.texte.contains(QStringLiteral("Remove track")));
        CHECK(fang.dialogText == QStringLiteral("Snare"));
        CHECK(grund().spuren[1].name == QStringLiteral("Snare 2"));

        // --- das Etikett Role: ein Menue mit den Rollen und No role
        fang.waehle = QStringLiteral("snare");
        auto* rolle = editor.findChild<QToolButton*>(QStringLiteral("drumTrackRole1"));
        REQUIRE(rolle != nullptr);
        rolle->click();
        CHECK(fang.texte.contains(QStringLiteral("kick")));
        CHECK(fang.texte.contains(QStringLiteral("No role")));
        CHECK(grund().spuren[1].rolle == QStringLiteral("snare"));

        // --- Add track: die Rollen des Kits, die fehlen, darunter Empty track
        fang.waehle = QStringLiteral("Empty track");
        auto* neu = editor.findChild<QPushButton*>(QStringLiteral("drumAddTrack"));
        REQUIRE(neu != nullptr);
        neu->click();
        CHECK(fang.texte.contains(QStringLiteral("Empty track")));
        CHECK(fang.texte.contains(QStringLiteral("Clap \u2014 from 808")));
        CHECK(grund().spuren.size() == 3);

        // --- die Schieber: beim Ziehen nichts, beim Loslassen ein Schritt; ein Klick ohne Ziehen sofort
        const int vorher = aenderungen;
        auto* laut = editor.findChild<QSlider*>(QStringLiteral("drumTrackVolume0"));
        auto* pan = editor.findChild<QSlider*>(QStringLiteral("drumTrackPan0"));
        REQUIRE(laut != nullptr);
        REQUIRE(pan != nullptr);
        laut->setSliderDown(true);
        laut->setValue(55);
        laut->setValue(40);
        CHECK(aenderungen == vorher);
        laut->setSliderDown(false);
        CHECK(aenderungen == vorher + 1);
        CHECK(grund().spuren[0].laut == doctest::Approx(0.4));
        pan->setValue(-50);
        CHECK(aenderungen == vorher + 2);
        CHECK(grund().spuren[0].pan == doctest::Approx(-0.5));
    }

    TEST_CASE("Loop Menues: der Instrument-Editor — ohne Auswahl sind Copy, Cut, Duplicate, Delete grau")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        Fang fang;

        Loop loop;
        Muster mel;
        mel.name = QStringLiteral("Mel");
        mel.art = MusterArt::Instrument;
        mel.instrument = QStringLiteral("piano");
        loop.muster = {mel};
        loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
        loop.richte();
        InstrumentEditor editor;  // nicht gezeigt
        REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
        NotenRaster& raster = *editor.raster();
        raster.resize(900, 400);
        // gleich unter der Zeile der Takte: welche Tonhoehe dort liegt, haengt am Ausschnitt
        rechts(raster, QPoint(raster.xVonSchritt(2) + 3, NotenRaster::kKopfHoehe + 4));
        CHECK(fang.menues == 1);
        CHECK(fang.texte == QStringList{QStringLiteral("Copy"), QStringLiteral("Cut"), QStringLiteral("Paste here"),
                                        QStringLiteral("Duplicate"), QStringLiteral("Delete"), QStringLiteral("Select all")});
        CHECK(fang.grau == QStringList{QStringLiteral("Copy"), QStringLiteral("Cut"), QStringLiteral("Paste here"),
                                       QStringLiteral("Duplicate"), QStringLiteral("Delete")});
    }

    TEST_CASE("Loop Menues: der Dialog Tags... — vorbelegt, ausgefuellt, ein Schritt fuer Undo")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        Fang fang;

        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString pfad = ordner.filePath(QStringLiteral("beat.lvloop"));
        Loop mitTags = arrangement();
        mitTags.tags.kuenstler = QStringLiteral("Patrik");
        REQUIRE(speichereLoop(mitTags, pfad));

        ServiceContainer dienste;
        dienste.registerSingleton<IEventBus>([](ServiceContainer&) { return std::make_unique<EventBus>(); });
        LoopPanel panel(dienste);  // nicht gezeigt
        REQUIRE(panel.lade(pfad));

        QStringList gesehen;
        fang.dialog = [&gesehen](QDialog& d) {
            const QList<QLineEdit*> felder = d.findChildren<QLineEdit*>();
            for (QLineEdit* f : felder) gesehen << f->text();
            if (felder.size() >= 6) felder[0]->setText(QStringLiteral("  Mein Beat "));
            if (auto* text = d.findChild<QPlainTextEdit*>()) text->setPlainText(QStringLiteral("la la"));
            return static_cast<int>(QDialog::Accepted);
        };
        auto* knopf = panel.findChild<QPushButton*>(QStringLiteral("loopTags"));
        REQUIRE(knopf != nullptr);
        knopf->click();
        CHECK(fang.dialogTitel == QStringLiteral("Loop tags"));
        REQUIRE(gesehen.size() == 6);  // Title, Artist, Album, Year, Genre, Track
        CHECK(gesehen[1] == QStringLiteral("Patrik"));
        CHECK(panel.loop().tags.titel == QStringLiteral("Mein Beat"));  // getrimmt
        CHECK(panel.loop().tags.kuenstler == QStringLiteral("Patrik"));
        CHECK(panel.loop().tags.text == QStringLiteral("la la"));

        // abgebrochen: nichts
        fang.dialog = [](QDialog& d) {
            if (auto* f = d.findChild<QLineEdit*>()) f->setText(QStringLiteral("anders"));
            return static_cast<int>(QDialog::Rejected);
        };
        knopf->click();
        CHECK(panel.loop().tags.titel == QStringLiteral("Mein Beat"));
    }

    TEST_CASE("S127 7b Loop: verschieben im Modell — Kanal, Spur mit Zeile, der Platz danach")
    {
        Loop loop = arrangement();
        CHECK_FALSE(kanalVerschieben(loop, 0, 0));
        CHECK_FALSE(kanalVerschieben(loop, 0, 2));
        REQUIRE(kanalVerschieben(loop, 0, 1));
        CHECK(loop.kanaele[0].name == QStringLiteral("Perc"));
        CHECK(loop.kanaele[1].name == QStringLiteral("Drums"));
        CHECK(loop.kanaele[1].clips.size() == 2);  // die Clips gehen mit

        Loop schlag = schlagzeug();
        schlag.muster[0].spuren.push_back({QStringLiteral("Hat"), QStringLiteral("kit:hat"), 0.5, 0.0, false});
        schlag.richte();
        schlag.muster[0].zeilen[2][3] = 0.7;
        REQUIRE(spurVerschieben(schlag, QStringLiteral("Grund"), 2, 0));
        const Muster& m = schlag.muster[0];
        CHECK(m.spuren[0].name == QStringLiteral("Hat"));
        CHECK(m.spuren[1].name == QStringLiteral("Kick"));
        CHECK(m.zeilen[0][3] == doctest::Approx(0.7));  // die Schritte gehen mit
        CHECK(m.zeilen[1][0] == doctest::Approx(1.0));
        CHECK_FALSE(spurVerschieben(schlag, QStringLiteral("Grund"), 0, 3));
        CHECK_FALSE(spurVerschieben(schlag, QStringLiteral("Gibts nicht"), 0, 1));

        // der Platz anderer nach einem Verschieben von 1 nach 3 (und zurueck)
        CHECK(platzNachVerschieben(1, 1, 3) == 3);
        CHECK(platzNachVerschieben(2, 1, 3) == 1);
        CHECK(platzNachVerschieben(3, 1, 3) == 2);
        CHECK(platzNachVerschieben(0, 1, 3) == 0);
        CHECK(platzNachVerschieben(4, 1, 3) == 4);
        CHECK(platzNachVerschieben(1, 3, 1) == 2);
        CHECK(platzNachVerschieben(3, 3, 1) == 1);
    }

    TEST_CASE("S127 7b Taktachse: Kanaele am Namen ziehen, Esc, Rechtsklick Move channel up/down")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        Fang fang;

        Taktachse achse;  // nicht gezeigt
        achse.resize(900, 260);
        Loop drei = arrangement();
        drei.kanaele.push_back({QStringLiteral("Bass"), true, {}});
        achse.setLoop(drei, true);
        Loop letzter = drei;
        QString was;
        QObject::connect(&achse, &Taktachse::bearbeitet, &achse, [&](const Loop& neu, const QString& w) {
            letzter = neu;
            was = w;
            achse.zeigeBearbeitet(neu);
        });
        // die Mitte einer Zeile — einmal am Anfang gemessen (nach dem Umsortieren hat Kanal 0 keinen Clip mehr)
        const int y0 = achse.clipRechteck(0, 0).center().y();
        const int zeile = achse.clipRechteck(1, 0).center().y() - y0;
        const auto mitte = [&](int kanal) { return y0 + kanal * zeile; };
        const auto maus = [&](QEvent::Type art, QPoint ort, Qt::MouseButtons tasten) {
            QMouseEvent e(art, QPointF(ort), QPointF(achse.mapToGlobal(ort)), Qt::LeftButton, tasten, Qt::NoModifier);
            QCoreApplication::sendEvent(&achse, &e);
        };

        // ein Clip in Drums markiert: er folgt seinem Kanal
        achse.waehle(0, 1);
        // --- Drums am Namen ganz nach unten ziehen: ein Strich, beim Loslassen ein Schritt
        maus(QEvent::MouseButtonPress, QPoint(10, mitte(0)), Qt::LeftButton);
        CHECK(achse.kanalImZug() == 0);
        maus(QEvent::MouseMove, QPoint(10, mitte(2)), Qt::LeftButton);
        CHECK(achse.kanalZiel() == 2);
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_LOOP_UMSORTIEREN_BILD"); !bild.isEmpty())
            CHECK(achse.grab().save(bild + QStringLiteral(".kanal.png")));
        maus(QEvent::MouseButtonRelease, QPoint(10, mitte(2)), Qt::NoButton);
        CHECK(achse.kanalImZug() == -1);
        CHECK(was == QStringLiteral("Move channel"));
        CHECK(letzter.kanaele[0].name == QStringLiteral("Perc"));
        CHECK(letzter.kanaele[1].name == QStringLiteral("Bass"));
        CHECK(letzter.kanaele[2].name == QStringLiteral("Drums"));
        CHECK(achse.gewaehlterKanal() == 2);  // die Auswahl ist mitgegangen
        CHECK(achse.auswahl() != nullptr);
        CHECK(achse.auswahl()->muster == QStringLiteral("B"));

        // --- ein Klick ohne Bewegung verschiebt nichts; Esc bricht ab
        was.clear();
        maus(QEvent::MouseButtonPress, QPoint(10, mitte(0)), Qt::LeftButton);
        maus(QEvent::MouseButtonRelease, QPoint(10, mitte(0)), Qt::NoButton);
        CHECK(was.isEmpty());
        maus(QEvent::MouseButtonPress, QPoint(10, mitte(0)), Qt::LeftButton);
        maus(QEvent::MouseMove, QPoint(10, mitte(2)), Qt::LeftButton);
        QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QCoreApplication::sendEvent(&achse, &esc);
        CHECK(achse.kanalImZug() == -1);
        maus(QEvent::MouseButtonRelease, QPoint(10, mitte(2)), Qt::NoButton);
        CHECK(was.isEmpty());

        // --- Rechtsklick: oben ist Move channel up grau; Move channel down legt ihn eins tiefer
        rechts(achse, QPoint(10, mitte(0)));
        CHECK(fang.grau == QStringList{QStringLiteral("Move channel up")});
        fang.waehle = QStringLiteral("Move channel down");
        rechts(achse, QPoint(10, mitte(0)));
        CHECK(letzter.kanaele[0].name == QStringLiteral("Bass"));
        CHECK(letzter.kanaele[1].name == QStringLiteral("Perc"));
        fang.waehle = QStringLiteral("Move channel up");
        rechts(achse, QPoint(10, mitte(2)));
        CHECK(letzter.kanaele[1].name == QStringLiteral("Drums"));
        CHECK(letzter.kanaele[2].name == QStringLiteral("Perc"));
    }

    TEST_CASE("S127 7b Drum-Editor: Spuren am Namen ziehen, Esc, Rechtsklick Move track up/down")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        Fang fang;

        Loop schlag = schlagzeug();
        schlag.muster[0].spuren.push_back({QStringLiteral("Hat"), QStringLiteral("kit:hat"), 0.5, 0.0, false});
        schlag.richte();
        DrumEditor editor;  // gelegt, aber nicht auf dem Bildschirm: die Zeilen brauchen ihre Lage
        Loop letzter = schlag;
        QString was;
        QObject::connect(&editor, &DrumEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& w) {
            letzter = neu;
            was = w;
            editor.setLoop(neu);
        });
        REQUIRE(editor.zeige(schlag, QStringLiteral("Grund")));
        editor.resize(900, 500);
        editor.setAttribute(Qt::WA_DontShowOnScreen);
        editor.show();
        QCoreApplication::processEvents();
        const auto name = [&](int s) { return editor.spurZeile(s)->findChild<QLabel*>(); };
        const auto namen = [&] {
            QStringList n;
            for (const Spur& s : letzter.muster[0].spuren) n << s.name;
            return n;
        };
        const auto maus = [&](int spur, QEvent::Type art, int zielSpur, Qt::MouseButtons tasten) {
            QLabel* l = name(spur);
            QWidget* ziel = editor.spurZeile(zielSpur);
            const QPoint global = ziel->mapToGlobal(QPoint(5, ziel->height() / 2));
            QMouseEvent e(art, QPointF(l->mapFromGlobal(global)), QPointF(global), Qt::LeftButton, tasten, Qt::NoModifier);
            QCoreApplication::sendEvent(l, &e);
        };
        REQUIRE(name(0) != nullptr);
        CHECK(name(0)->cursor().shape() == Qt::OpenHandCursor);

        // --- Kick (0) ans Ende ziehen; die gewaehlte Spur geht mit
        maus(0, QEvent::MouseButtonPress, 0, Qt::LeftButton);
        CHECK(editor.spurImZug() == 0);
        maus(0, QEvent::MouseMove, 2, Qt::LeftButton);
        CHECK(editor.spurZiel() == 2);
        auto* strich = editor.findChild<QWidget*>(QStringLiteral("drumTrackDropLine"));
        REQUIRE(strich != nullptr);
        CHECK_FALSE(strich->isHidden());
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_LOOP_UMSORTIEREN_BILD"); !bild.isEmpty())
            CHECK(editor.grab().save(bild + QStringLiteral(".spur.png")));
        maus(0, QEvent::MouseButtonRelease, 2, Qt::NoButton);
        CHECK(was == QStringLiteral("Move track"));
        CHECK(namen() == QStringList{QStringLiteral("Snare"), QStringLiteral("Hat"), QStringLiteral("Kick")});
        CHECK(letzter.muster[0].zeilen[2][0] == doctest::Approx(1.0));  // die Schritte der Kick gingen mit
        CHECK(editor.raster()->spur() == 2);
        CHECK(strich->isHidden());

        // --- Esc bricht ab
        was.clear();
        maus(0, QEvent::MouseButtonPress, 0, Qt::LeftButton);
        maus(0, QEvent::MouseMove, 2, Qt::LeftButton);
        QKeyEvent esc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QCoreApplication::sendEvent(&editor, &esc);
        CHECK(editor.spurImZug() == -1);
        maus(0, QEvent::MouseButtonRelease, 2, Qt::NoButton);
        CHECK(was.isEmpty());

        // --- Rechtsklick: unten ist Move track down grau; Move track up
        editor.spurMenue(2, QPoint());
        CHECK(fang.grau == QStringList{QStringLiteral("Move track down")});
        CHECK(fang.texte.contains(QStringLiteral("Move track up")));
        fang.waehle = QStringLiteral("Move track up");
        editor.spurMenue(2, QPoint());
        CHECK(namen() == QStringList{QStringLiteral("Snare"), QStringLiteral("Kick"), QStringLiteral("Hat")});
        fang.waehle = QStringLiteral("Move track down");
        editor.spurMenue(0, QPoint());
        CHECK(namen() == QStringList{QStringLiteral("Kick"), QStringLiteral("Snare"), QStringLiteral("Hat")});
    }
}
