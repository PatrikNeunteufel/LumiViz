/**
 * @file test_LoopKitFenster.cpp
 * @brief Loop, Stufe 21c: das Fenster Kits (docs/audio/Loop_Konzept.md §4l,
 *        Entscheide K7 a, K8 a, Patrik S123)
 *
 * Ohne Ton und ohne gezeigtes Fenster; eigene Kits in einem Wegwerf-Ordner,
 * die mitgelieferten aus dem Repo, eigene Einstellungen (`LumiVizTest` /
 * `LoopKitFenster`). `LUMIVIZ_KIT_FENSTER_BILD=<datei.png>` legt zwei Bilder
 * ab: ein mitgeliefertes Kit und (mit `_mine`) ein eigenes.
 */

#include <doctest.h>

#include "loop/DrumEditor.hpp"
#include "loop/DrumKit.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/KitFenster.hpp"
#include "loop/KlangAuswahl.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QToolButton>
#include <QTreeWidget>

#include <vector>

using namespace lumi::loop;

namespace
{

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopKitFenster"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

/// Ein Loop mit einem Drum-Muster „Grund" (Kick, Snare), das `kit` nennt.
Loop loopMitKit(const QString& kit)
{
    Loop loop;
    Muster grund;
    grund.name = QStringLiteral("Grund");
    grund.takte = 4;
    grund.kit = kit;
    grund.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
                    {QStringLiteral("Snare"), QStringLiteral("kit:snare"), 0.8, 0.0, false}};
    grund.zeilen = {std::vector<double>(64, 0.0), std::vector<double>(64, 0.0)};
    grund.zeilen[0][0] = 1.0;
    grund.zeilen[1][4] = 1.0;
    loop.muster = {grund};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("Grund"), 0, 1}}}};
    return loop;
}

}  // namespace

TEST_SUITE("LoopKitFenster")
{
    TEST_CASE("Kits 21c: Liste, mitgeliefert schreibgeschuetzt, Duplicate, Rename, Delete")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const QString ordner = QDir(tmp.path()).filePath(QStringLiteral("kits"));
        setzeEigenenKitOrdnerFuerTests(ordner);

        KitFenster fenster;
        int geschrieben = 0;
        int geaendertListe = 0;
        QObject::connect(&fenster, &KitFenster::gespeichert, &fenster, [&] { ++geschrieben; });
        QObject::connect(&fenster, &KitFenster::kitsGeaendert, &fenster, [&] { ++geaendertListe; });

        // --- ein mitgeliefertes Kit: offen, mit Schloss, nichts zu aendern
        fenster.oeffne(QStringLiteral("808"));
        CHECK(fenster.nennung() == QStringLiteral("808"));
        CHECK(fenster.kit().titel == QStringLiteral("808"));
        CHECK(fenster.schreibgeschuetzt());
        CHECK(fenster.hinweisZeile() == QStringLiteral("Built-in, read-only — Duplicate makes your own copy"));
        const QStringList liste = fenster.listenZeilen();
        CHECK(liste.first() == QStringLiteral("[Built-in kits]"));
        CHECK(liste.contains(QStringLiteral("*808")));
        CHECK(liste.contains(QStringLiteral("classic")));
        CHECK_FALSE(liste.contains(QStringLiteral("[My kits]")));  // noch keine eigenen
        CHECK(liste.size() == 1 + static_cast<int>(mitgelieferteKits().size()));
        CHECK_FALSE(fenster.liste()->currentItem()->icon(0).isNull());  // das Schloss
        REQUIRE(fenster.tabelle()->rowCount() == 8);
        CHECK(fenster.tabelle()->item(0, 0)->text() == QStringLiteral("kick"));
        CHECK(fenster.tabelle()->item(7, 0)->text() == QStringLiteral("crash"));
        CHECK_FALSE(fenster.klangAuswahl(1)->isEnabled());
        CHECK_FALSE(fenster.setzeLaut(1, 0.3));
        CHECK_FALSE(fenster.setzeKlang(1, QStringLiteral("kit:snare")));
        CHECK(fenster.fuegeRolleHinzu(QStringLiteral("fx")) == -1);
        CHECK_FALSE(fenster.entferneRolle(0));
        CHECK_FALSE(fenster.oeffnePalette(0, false));
        CHECK(fenster.speichere().isEmpty());
        CHECK_FALSE(fenster.benenne(QStringLiteral("Mein 808")));
        CHECK_FALSE(fenster.loesche());
        CHECK_FALSE(fenster.geaendert());
        CHECK_FALSE(fenster.findChild<QPushButton*>(QStringLiteral("kitsSave"))->isEnabled());
        CHECK_FALSE(fenster.findChild<QPushButton*>(QStringLiteral("kitsRename"))->isEnabled());
        CHECK(fenster.findChild<QPushButton*>(QStringLiteral("kitsDuplicate"))->isEnabled());
        // die Auswahl Sound nennt, was die Rolle spielt: ein Instrument bzw. ein Sample; kein Record sample...
        CHECK(fenster.klangZeilen(0).first() == QStringLiteral("instrument:drum_kick_808"));
        CHECK(fenster.klangAuswahl(1)->currentData().toString() == QStringLiteral("lib:snare_808"));
        CHECK(fenster.klangZeilen(1).contains(QStringLiteral("[Samples]")));
        CHECK(fenster.klangZeilen(1).contains(QStringLiteral("Sample file...")));
        CHECK_FALSE(fenster.klangZeilen(1).contains(QStringLiteral("Record sample...")));

        // ein Kit, das es nicht gibt: das erste der Liste
        fenster.oeffne(QStringLiteral("gibt_es_nicht"));
        CHECK(fenster.nennung() == mitgelieferteKits().front().name);
        CHECK_FALSE(fenster.waehle(QStringLiteral("gibt_es_nicht")));
        REQUIRE(fenster.waehle(QStringLiteral("808")));

        // --- Duplicate: eine eigene Kopie, sofort geschrieben und offen
        CHECK(fenster.dupliziere() == QStringLiteral("user:808_copy"));
        CHECK(geaendertListe == 1);
        CHECK(QFileInfo::exists(QDir(ordner).filePath(QStringLiteral("808_copy.lvkit"))));
        CHECK(fenster.nennung() == QStringLiteral("user:808_copy"));
        CHECK(fenster.kit().titel == QStringLiteral("808 copy"));
        CHECK_FALSE(fenster.schreibgeschuetzt());
        CHECK(fenster.hinweisZeile().isEmpty());
        CHECK(fenster.kit().klaenge.size() == 8);
        CHECK(fenster.listenZeilen().contains(QStringLiteral("[My kits]")));
        CHECK(fenster.listenZeilen().last() == QStringLiteral("*user:808_copy"));
        CHECK(fenster.liste()->currentItem()->icon(0).isNull());  // ein eigenes: kein Schloss
        CHECK(fenster.klangAuswahl(1)->isEnabled());
        // noch einmal: die naechste Nummer
        REQUIRE(fenster.waehle(QStringLiteral("808")));
        CHECK(fenster.dupliziere() == QStringLiteral("user:808_copy_2"));
        CHECK(fenster.kit().titel == QStringLiteral("808 copy 2"));

        // --- Rename: der Titel aendert sich, die Datei behaelt ihren Namen
        REQUIRE(fenster.waehle(QStringLiteral("user:808_copy")));
        CHECK_FALSE(fenster.benenne(QStringLiteral("   ")));
        REQUIRE(fenster.benenne(QStringLiteral("Mein 808")));
        CHECK(geschrieben == 1);
        CHECK(fenster.nennung() == QStringLiteral("user:808_copy"));
        DrumKit gelesen;
        REQUIRE(holeKit(QStringLiteral("user:808_copy"), gelesen));
        CHECK(gelesen.titel == QStringLiteral("Mein 808"));
        CHECK(fenster.liste()->currentItem()->text(0) == QStringLiteral("Mein 808"));
        // eine Aenderung von Hand bleibt beim Rename offen, ungespeichert
        REQUIRE(fenster.setzeLaut(0, 0.5));
        REQUIRE(fenster.benenne(QStringLiteral("Mein 808 neu")));
        CHECK(fenster.geaendert());
        REQUIRE(holeKit(QStringLiteral("user:808_copy"), gelesen));
        CHECK(gelesen.titel == QStringLiteral("Mein 808 neu"));
        CHECK(gelesen.klaenge[0].laut == doctest::Approx(0.279));  // geschrieben ist, was gespeichert war
        REQUIRE(fenster.rueckgaengig());  // Undo nimmt die Aenderung, nicht den Titel
        CHECK(fenster.kit().titel == QStringLiteral("Mein 808 neu"));
        CHECK_FALSE(fenster.geaendert());

        // --- Delete: die Datei ist weg, das erste der Liste ist offen
        REQUIRE(fenster.waehle(QStringLiteral("user:808_copy_2")));
        REQUIRE(fenster.loesche());
        CHECK_FALSE(QFileInfo::exists(QDir(ordner).filePath(QStringLiteral("808_copy_2.lvkit"))));
        CHECK(fenster.nennung() == mitgelieferteKits().front().name);
        CHECK_FALSE(fenster.listenZeilen().contains(QStringLiteral("user:808_copy_2")));
        CHECK(fenster.listenZeilen().contains(QStringLiteral("user:808_copy")));
        setzeEigenenKitOrdnerFuerTests(QString());
    }

    TEST_CASE("Kits 21c: Rollen bearbeiten — Sound, Volume, Pan, Add role, Remove role, Undo, Save, anspielen")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        setzeEigenenKitOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("kits")));

        KitFenster fenster;
        fenster.setKitKlaenge({QStringLiteral("kit:kick"), QStringLiteral("kit:snare"), QStringLiteral("kit:clap")});
        std::vector<std::vector<Spur>> angespielt;
        QObject::connect(&fenster, &KitFenster::anspielenVerlangt, &fenster,
                         [&](const std::vector<Spur>& spuren) { angespielt.push_back(spuren); });
        int geschrieben = 0;
        QObject::connect(&fenster, &KitFenster::gespeichert, &fenster, [&] { ++geschrieben; });
        fenster.oeffne(QStringLiteral("classic"));
        REQUIRE(fenster.dupliziere() == QStringLiteral("user:classic_copy"));
        const DrumKit start = fenster.kit();
        REQUIRE(start.klaenge.size() == 6);  // kick, snare, clap, hat, open hat, tom 1
        CHECK_FALSE(fenster.kannRueckgaengig());

        // --- Sound, Volume, Pan: je ein Schritt fuer Undo
        REQUIRE(fenster.setzeKlang(1, QStringLiteral("kit:clap")));
        CHECK(fenster.kit().klaenge[1].klang == QStringLiteral("kit:clap"));
        CHECK(fenster.klangAuswahl(1)->currentData().toString() == QStringLiteral("kit:clap"));
        CHECK_FALSE(fenster.setzeKlang(1, QStringLiteral("kit:clap")));  // dasselbe noch einmal: kein Schritt
        REQUIRE(fenster.setzeLaut(1, 0.25));
        REQUIRE(fenster.setzePan(1, -0.5));
        CHECK(fenster.kit().klaenge[1].laut == doctest::Approx(0.25));
        CHECK(fenster.kit().klaenge[1].pan == doctest::Approx(-0.5));
        auto* tabelle = fenster.tabelle();
        CHECK(qobject_cast<QSlider*>(tabelle->cellWidget(1, 2))->value() == 25);
        CHECK(qobject_cast<QSlider*>(tabelle->cellWidget(1, 3))->value() == -50);
        REQUIRE(fenster.setzeLaut(1, 7.0));  // in den Grenzen
        CHECK(fenster.kit().klaenge[1].laut == doctest::Approx(1.0));
        CHECK(fenster.geaendert());
        CHECK(fenster.windowTitle().endsWith(QStringLiteral(" *")));
        CHECK(fenster.findChild<QPushButton*>(QStringLiteral("kitsSave"))->isEnabled());
        // ein Instrument: Note nach seiner Art, Length 250 ms; ein Klang danach nimmt es wieder weg
        REQUIRE(fenster.setzeInstrument(2, QStringLiteral("drum_clap")));
        CHECK(fenster.kit().klaenge[2].instrument == QStringLiteral("drum_clap"));
        CHECK(fenster.kit().klaenge[2].hoehe == 36);
        CHECK(fenster.kit().klaenge[2].dauerMs == kSpurDauerMs);
        CHECK(fenster.klangZeilen(2).first() == QStringLiteral("instrument:drum_clap"));
        // die Auswahl ist ein und dieselbe Zeile geblieben (an Ort und Stelle nachgestellt)
        KlangAuswahl* auswahl2 = fenster.klangAuswahl(2);
        REQUIRE(fenster.setzeKlang(2, QStringLiteral("kit:clap")));
        CHECK(fenster.klangAuswahl(2) == auswahl2);
        CHECK(fenster.kit().klaenge[2].instrument.isEmpty());

        // --- Instrument... oeffnet das Fenster Instruments fuer die Rolle; Use setzt es
        REQUIRE(fenster.oeffnePalette(0, false));
        InstrumentPalette* palette = fenster.instrumentFenster();
        REQUIRE(palette != nullptr);
        REQUIRE(palette->waehle(QStringLiteral("drum_kick_808")));
        REQUIRE(palette->verwende());
        CHECK(fenster.kit().klaenge[0].instrument == QStringLiteral("drum_kick_808"));
        // Ctrl+Z im Fenster Instruments: das Undo des Kits
        palette->nimmZurueck();
        CHECK(fenster.kit().klaenge[0].instrument == start.klaenge[0].instrument);
        palette->stelleWiederHer();
        CHECK(fenster.kit().klaenge[0].instrument == QStringLiteral("drum_kick_808"));

        // --- Add role: je Art die naechste Nummer, an ihren Platz
        const QStringList frei = fenster.freieRollen();
        CHECK(frei.contains(QStringLiteral("kick 2")));
        CHECK(frei.contains(QStringLiteral("tom 2")));
        CHECK(frei.contains(QStringLiteral("ride")));
        CHECK(frei.contains(QStringLiteral("perc 1")));
        CHECK_FALSE(frei.contains(QStringLiteral("tom 1")));
        // der Knopf Add role ▾: sein Menue nennt beim Aufklappen genau diese Rollen
        auto* neuKnopf = fenster.findChild<QPushButton*>(QStringLiteral("kitsAddRole"));
        REQUIRE(neuKnopf != nullptr);
        REQUIRE(neuKnopf->menu() != nullptr);
        Q_EMIT neuKnopf->menu()->aboutToShow();
        QStringList imMenue;
        for (const QAction* eintrag : neuKnopf->menu()->actions()) imMenue << eintrag->text();
        CHECK(imMenue == frei);
        CHECK(fenster.fuegeRolleHinzu(QStringLiteral("snare")) == -1);  // schon da
        CHECK(fenster.fuegeRolleHinzu(QStringLiteral("cowbell")) == -1);  // keine Rolle
        const int ride = fenster.fuegeRolleHinzu(QStringLiteral("ride"));
        CHECK(ride == 5);  // nach open hat, vor tom 1 (die Reihenfolge der Arten)
        CHECK(fenster.zeile() == ride);
        CHECK(tabelle->item(ride, 0)->text() == QStringLiteral("ride"));
        CHECK(fenster.kit().klaenge[static_cast<std::size_t>(ride)].klang.isEmpty());
        CHECK(fenster.klangZeilen(ride).first() == QStringLiteral("(none)"));
        CHECK_FALSE(fenster.spieleRolle(ride));  // spielt nichts
        CHECK_FALSE(qobject_cast<QToolButton*>(tabelle->cellWidget(ride, 4))->isEnabled());
        REQUIRE(fenster.setzeKlang(ride, QStringLiteral("kit:hat_open")));
        CHECK(fenster.fuegeRolleHinzu(QStringLiteral("perc 1")) == 7);  // ganz hinten nach tom 1
        CHECK(fenster.kit().klaenge.size() == 8);
        // --- Remove role
        REQUIRE(fenster.entferneRolle(7));
        CHECK(fenster.kit().klaenge.size() == 7);
        CHECK_FALSE(fenster.entferneRolle(9));

        // --- Undo bis zum Anfang, Redo bis zum Ende
        const DrumKit ende = fenster.kit();
        int schritte = 0;
        while (fenster.rueckgaengig()) ++schritte;
        CHECK(fenster.kit() == start);
        CHECK_FALSE(fenster.geaendert());
        CHECK(tabelle->rowCount() == 6);
        for (int n = 0; n < schritte; ++n) REQUIRE(fenster.wiederherstellen());
        CHECK(fenster.kit() == ende);
        CHECK_FALSE(fenster.wiederherstellen());
        // eine neue Aenderung nimmt Redo weg
        REQUIRE(fenster.rueckgaengig());
        REQUIRE(fenster.setzePan(0, 0.1));
        CHECK_FALSE(fenster.kannWiederherstellen());

        // --- Save: die Datei traegt den Stand, Undo bleibt danach moeglich
        const DrumKit gespeichert = fenster.kit();
        CHECK(fenster.speichere() == QStringLiteral("user:classic_copy"));
        CHECK(geschrieben == 1);
        CHECK_FALSE(fenster.geaendert());
        DrumKit gelesen;
        REQUIRE(holeKit(QStringLiteral("user:classic_copy"), gelesen));
        CHECK(gelesen == gespeichert);
        CHECK(fenster.kannRueckgaengig());

        // --- ▶ und Play all: Spuren mit Klang, Volume und Pan der Rolle
        REQUIRE(fenster.spieleRolle(1));
        REQUIRE(angespielt.size() == 1);
        REQUIRE(angespielt[0].size() == 1);
        CHECK(angespielt[0][0].klang == QStringLiteral("kit:clap"));
        CHECK(angespielt[0][0].rolle == QStringLiteral("snare"));
        CHECK(angespielt[0][0].laut == doctest::Approx(1.0));
        CHECK(angespielt[0][0].pan == doctest::Approx(-0.5));
        REQUIRE(fenster.spieleAlle());
        REQUIRE(angespielt.size() == 2);
        // perc 1 kam mit dem Undo vor setzePan zurueck, ohne Klang: sie spielt nicht mit
        REQUIRE(fenster.kit().klaenge.size() == 8);
        CHECK(fenster.kit().klaenge[7].rolle == QStringLiteral("perc 1"));
        CHECK(angespielt[1].size() == 7);
        CHECK(angespielt[1][0].instrument == QStringLiteral("drum_kick_808"));
        CHECK_FALSE(fenster.spieleRolle(-1));
        setzeEigenenKitOrdnerFuerTests(QString());
    }

    TEST_CASE("Kits 21c: Used by, Edit kits... im Drum-Editor, die Auswahl Sound kuerzt, Play all im Panel")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        setzeEigenenKitOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("kits")));

        // --- die zuletzt geoeffneten Loops: juengster zuerst, hoechstens zehn, nur vorhandene
        CHECK(zuletztGeoeffneteLoops().isEmpty());
        const QString mitKit = QDir(tmp.path()).filePath(QStringLiteral("mit_808.lvloop"));
        const QString ohneKit = QDir(tmp.path()).filePath(QStringLiteral("ohne_kit.lvloop"));
        REQUIRE(speichereLoop(loopMitKit(QStringLiteral("808")), mitKit));
        REQUIRE(speichereLoop(loopMitKit(QString()), ohneKit));
        merkeGeoeffnetenLoop(mitKit);
        merkeGeoeffnetenLoop(ohneKit);
        merkeGeoeffnetenLoop(QDir(tmp.path()).filePath(QStringLiteral("weg.lvloop")));  // gibt es nicht
        CHECK(zuletztGeoeffneteLoops() == QStringList{QFileInfo(ohneKit).absoluteFilePath(), QFileInfo(mitKit).absoluteFilePath()});
        merkeGeoeffnetenLoop(mitKit);  // noch einmal: nach vorn, nicht doppelt
        CHECK(zuletztGeoeffneteLoops().first() == QFileInfo(mitKit).absoluteFilePath());
        CHECK(zuletztGeoeffneteLoops().size() == 2);
        for (int n = 0; n < 15; ++n) merkeGeoeffnetenLoop(QDir(tmp.path()).filePath(QStringLiteral("x%1.lvloop").arg(n)));
        CHECK(QSettings().value(QStringLiteral("loop/recentFiles")).toStringList().size() == kZuletztLoopsMax);
        merkeGeoeffnetenLoop(mitKit);
        CHECK(nenntKit(loopMitKit(QStringLiteral("808")), QStringLiteral("808")));
        CHECK_FALSE(nenntKit(loopMitKit(QStringLiteral("808")), QStringLiteral("classic")));
        CHECK(loopsMitKit(QStringLiteral("808"), {mitKit, ohneKit, mitKit}) == QStringList{QStringLiteral("mit_808")});
        CHECK_FALSE(mitgelieferteLoopDateien().isEmpty());
        CHECK(mitgelieferteLoopDateien().filter(QStringLiteral("/genres/")).size() >= 9);

        // --- Edit kits... im Drum-Editor: das Fenster auf dem Kit des Musters
        DrumEditor editor;
        QObject::connect(&editor, &DrumEditor::bearbeitet, &editor, [&](const Loop& neu, const QString&) { editor.setLoop(neu); });
        REQUIRE(editor.zeige(loopMitKit(QString()), QStringLiteral("Grund")));
        REQUIRE(editor.wendeKitAn(QStringLiteral("808")));
        CHECK(editor.kitFenster() == nullptr);
        editor.oeffneKits(false);
        KitFenster* fenster = editor.kitFenster();
        REQUIRE(fenster != nullptr);
        CHECK(fenster->isHidden());
        CHECK(fenster->nennung() == QStringLiteral("808"));
        // Used by: der offene Loop und die Datei unter den zuletzt geoeffneten
        CHECK(fenster->nutzerZeile().startsWith(QStringLiteral("Used by: the open loop, mit_808 · counted: built-in loops")));
        CHECK(fenster->findChild<QLabel*>(QStringLiteral("kitsUsedBy"))->text() == fenster->nutzerZeile());
        REQUIRE(fenster->waehle(QStringLiteral("classic")));
        CHECK(fenster->nutzerZeile().startsWith(QStringLiteral("Not used by any loop LumiViz knows")));

        // ▶ im Fenster geht ueber den Editor hinaus
        int angespielt = 0;
        QObject::connect(&editor, &DrumEditor::kitAnspielenVerlangt, &editor, [&](const std::vector<Spur>&) { ++angespielt; });
        REQUIRE(fenster->spieleRolle(0));
        CHECK(angespielt == 1);

        // ein eigenes Kit, im Fenster gespeichert: der Knopf Kit sagt danach „(changed)"
        REQUIRE(fenster->waehle(QStringLiteral("808")));
        REQUIRE(fenster->dupliziere() == QStringLiteral("user:808_copy"));
        CHECK(editor.kitZeilen().contains(QStringLiteral("user:808_copy")));  // gleich im Menue
        REQUIRE(editor.wendeKitAn(QStringLiteral("user:808_copy")));
        CHECK(editor.kitText() == QStringLiteral("Kit: 808 copy"));
        REQUIRE(fenster->setzeLaut(1, 0.1));  // snare
        CHECK(editor.kitText() == QStringLiteral("Kit: 808 copy"));  // erst nach Save
        REQUIRE_FALSE(fenster->speichere().isEmpty());
        CHECK(editor.kitText() == QStringLiteral("Kit: 808 copy (changed)"));
        auto* knopf = editor.findChild<QPushButton*>(QStringLiteral("drumKit"));
        REQUIRE(knopf != nullptr);
        CHECK(knopf->text() == editor.kitText());
        // Edit kits... noch einmal, auf demselben Kit: nichts geht verloren
        REQUIRE(fenster->setzePan(1, 0.4));
        editor.oeffneKits(false);
        CHECK(fenster->geaendert());
        CHECK(fenster->kit().klaenge[1].pan == doctest::Approx(0.4));
        REQUIRE(fenster->rueckgaengig());
        CHECK_FALSE(fenster->geaendert());

        // --- die Auswahl Sound: ein langer Name endet im schmalen Feld mit „…" und steht ganz im Tooltip
        KlangAuswahl auswahl;
        Spur lang;
        lang.klang = QStringLiteral("C:/Proben/ein_sehr_langer_name_fuer_eine_aufnahme_mit_vielen_woertern.wav");
        auswahl.fuelle(lang, {}, {}, {}, true);
        auswahl.resize(110, auswahl.sizeHint().height());
        CHECK(auswahl.currentText() == QStringLiteral("ein_sehr_langer_name_fuer_eine_aufnahme_mit_vielen_woertern.wav"));
        CHECK(auswahl.sichtbarerText().endsWith(QString::fromUtf8("\xE2\x80\xA6")));
        CHECK(auswahl.sichtbarerText().size() < auswahl.currentText().size());
        CHECK(auswahl.toolTip().startsWith(auswahl.currentText()));
        CHECK(auswahl.zeilen().contains(QStringLiteral("[File]")));
        CHECK(auswahl.zeilen().last() == QStringLiteral("Record sample..."));
        auswahl.resize(900, auswahl.height());
        CHECK(auswahl.sichtbarerText() == auswahl.currentText());

        // --- Play all im Panel: ohne Audio-Engine wird nichts gespielt, nichts bricht
        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        LoopPanel panel(dienste);
        CHECK_FALSE(panel.spieleSchlaege({}));
        panel.neu();
        CHECK_FALSE(panel.spieleSchlaege({Spur{}}));  // spielt nichts
        CHECK_FALSE(panel.spieleSchlaege(std::vector<Spur>(3, loopMitKit(QString()).muster[0].spuren[0])));

        // `LUMIVIZ_KIT_FENSTER_BILD=<datei.png>` legt zwei Bilder ab: ein mitgeliefertes Kit, ein eigenes (`_mine`)
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_KIT_FENSTER_BILD"); !bild.isEmpty())
        {
            REQUIRE(fenster->waehle(QStringLiteral("808")));
            fenster->resize(880, 520);
            CHECK(fenster->grab().save(bild));
            REQUIRE(fenster->waehle(QStringLiteral("user:808_copy")));
            REQUIRE(fenster->fuegeRolleHinzu(QStringLiteral("perc 1")) >= 0);
            REQUIRE(fenster->setzeKlang(fenster->zeile(), QStringLiteral("lib:tambourine")));
            const QFileInfo ziel(bild);
            CHECK(fenster->grab().save(ziel.dir().filePath(ziel.completeBaseName() + QStringLiteral("_mine.") + ziel.suffix())));
        }
        setzeEigenenKitOrdnerFuerTests(QString());
    }
}
