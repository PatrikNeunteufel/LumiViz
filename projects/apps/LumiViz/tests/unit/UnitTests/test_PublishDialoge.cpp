/**
 * @file test_PublishDialoge.cpp
 * @brief Publish, Stufe P-A: die Dialoge Publish und Import package
 *        (docs/core-services/Publish_Konzept.md §4b–§4d; Entscheide P6 a, P7 a, P8 a)
 *        — ohne gezeigtes Fenster, Bestand aus `PaketBestand.hpp`, die
 *        Einstellungen in einer Ini-Datei.
 *
 * Bilder zum Ansehen: `LUMIVIZ_PUBLISH_DIALOG_BILD` und
 * `LUMIVIZ_IMPORT_PAKET_BILD=<datei.png>`; seit Stufe 30i
 * `LUMIVIZ_PUBLISH_FROZEN_BILD=<datei.png>` (Publish mit eingefrorenen Clips, waehrend
 * Publish wartet; dazu `_import` fuer den Import-Dialog).
 */

#include "PaketBestand.hpp"

#include "UI/dialogs/PaketDialoge.hpp"
#include "UI/widgets/Bedienhilfe.hpp"
#include "loop/AufnahmeFenster.hpp"
#include "loop/FormFenster.hpp"
#include "loop/InstrumentCreator.hpp"
#include "loop/InstrumentPalette.hpp"  // KitFenster haelt sie in einem QPointer
#include "loop/KitFenster.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QToolButton>
#include <QTreeWidget>

namespace
{

namespace ds = lumi::core::datensicherung;
using lumi::ui::ImportPaketDialog;
using lumi::ui::PublishDialog;

struct DialogApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

void bild(QWidget& fenster, const char* variable, const QString& zusatz = {})
{
    fenster.resize(fenster.sizeHint().expandedTo(fenster.size()));
    const QImage aufnahme = fenster.grab().toImage();
    CHECK_FALSE(aufnahme.isNull());
    QString ziel = qEnvironmentVariable(variable);
    if (ziel.isEmpty()) return;
    if (!zusatz.isEmpty())
    {
        const QFileInfo info(ziel);
        ziel = info.dir().filePath(info.completeBaseName() + zusatz + QLatin1Char('.') + info.suffix());
    }
    CHECK(aufnahme.save(ziel));
}

/// Ein Ton fuer die Dialoge ohne Engine: die Rate, dekodiert wird nichts (Samples bleiben still).
lumi::ui::FreezeTon stillerTon()
{
    return {kFreezeRate, [](const QString&, int, double) { return std::vector<float>{}; }};
}

/// Solange er lebt, beantwortet er die Meldungen ueber `fuehreDialogAus` mit Ja und merkt die Namen.
struct Meldungen
{
    QStringList namen;
    Meldungen()
    {
        lumi::ui::setzeDialogBeobachterFuerTests([this](QDialog& d) {
            namen << d.objectName();
            return static_cast<int>(QMessageBox::Yes);
        });
    }
    ~Meldungen() { lumi::ui::setzeDialogBeobachterFuerTests({}); }
};

QStringList kinder(const QTreeWidgetItem* item, int spalte = 0)
{
    QStringList texte;
    for (int i = 0; i < item->childCount(); ++i) texte << item->child(i)->text(spalte);
    return texte;
}

}  // namespace

TEST_SUITE("Publish")
{
    TEST_CASE("Publish Dialog: der Baum des Inhalts mit Herkunft, Bundled grau und eingeklappt, Not found; Publish schreibt (P7 a)")
    {
        const DialogApp app;
        const Bestand b;
        QSettings einstellungen(QDir(b.tmp.path()).filePath(QStringLiteral("einstellungen.ini")), QSettings::IniFormat);
        pb::Fund fund = pb::sammle(b.loopDatei);
        REQUIRE(fund.ok);
        PublishDialog dialog(fund, nullptr, &einstellungen);
        CHECK(dialog.windowTitle() == QStringLiteral("Publish loop"));

        QTreeWidget* baum = dialog.inhaltsBaum();
        REQUIRE(baum->topLevelItemCount() == 3);  // das Hauptding, Bundled, Not found
        const QTreeWidgetItem* haupt = baum->topLevelItem(0);
        CHECK(haupt->text(0) == QStringLiteral("titan"));
        CHECK(haupt->text(1) == QStringLiteral("Loop · main file"));
        CHECK(kinder(haupt) == QStringList{QStringLiteral("Instruments (2)"), QStringLiteral("Kits (1)"),
                                           QStringLiteral("Shapes (1)"), QStringLiteral("Samples (3)")});
        const QTreeWidgetItem* instrumente = haupt->child(0);
        CHECK(kinder(instrumente) == QStringList{QStringLiteral("bass"), QStringLiteral("pump_bass")});
        CHECK(instrumente->child(0)->text(1) == QStringLiteral("yours · user:bass"));
        CHECK(instrumente->child(1)->text(1) == QStringLiteral("next to the loop → user:pump_bass"));
        const QTreeWidgetItem* samples = haupt->child(3);
        CHECK(samples->child(2)->text(1).startsWith(QDir::toNativeSeparators(b.snareDatei)));
        CHECK(samples->child(2)->text(1).endsWith(QStringLiteral("→ user:snare_hit")));

        const QTreeWidgetItem* mit = baum->topLevelItem(1);
        CHECK(mit->text(0) == QStringLiteral("Bundled, not included (4)"));
        CHECK_FALSE(mit->isExpanded());
        const QTreeWidgetItem* fehlt = baum->topLevelItem(2);
        CHECK(fehlt->text(0) == QStringLiteral("Not found (1)"));
        CHECK(kinder(fehlt) == QStringList{QStringLiteral("user:gibt_es_nicht")});
        CHECK(fehlt->isExpanded());

        CHECK(dialog.summe().startsWith(QStringLiteral("8 files, ")));
        QCheckBox* medien = dialog.findChild<QCheckBox*>(QStringLiteral("publishMedia"));
        REQUIRE(medien != nullptr);
        CHECK_FALSE(medien->isEnabled());  // Stufe P-B

        const QString ordner = QDir(b.tmp.path()).filePath(QStringLiteral("ausgabe"));
        REQUIRE(QDir().mkpath(ordner));
        dialog.setzeZielOrdner(ordner);
        dialog.setzeName(QStringLiteral("mein_titan"));  // ohne Endung: sie kommt dazu
        CHECK(QFileInfo(dialog.paketDatei()).fileName() == QStringLiteral("mein_titan.lvdata"));
        bild(dialog, "LUMIVIZ_PUBLISH_DIALOG_BILD");
        const pb::PackErgebnis e = dialog.fuehreAus();
        REQUIRE_MESSAGE(e.ok, e.fehler.toStdString());
        CHECK(pb::istPaket(dialog.paketDatei()));
        CHECK(QDir::fromNativeSeparators(einstellungen.value(QStringLiteral("data/lastPublishDir")).toString()) == ordner);
    }

    TEST_CASE("Import package Dialog: Kopf, Contents, Same names mit Vergleich und Wahl, Notes, Save to (P8 a, P6 a)")
    {
        const DialogApp app;
        const Bestand b;
        const QString paket = packeTitan(b);
        const Empfaenger e(b.tmp.path());
        lp::Instrument seiner;
        seiner.titel = QStringLiteral("Empfaenger Bass");
        REQUIRE(lp::speichereInstrument(seiner, QDir(e.instrumente).filePath(QStringLiteral("bass.lvinst"))));
        lege(QDir(e.samples).filePath(QStringLiteral("kick.wav")), "eigener kick");
        lege(QDir(e.samples).filePath(QStringLiteral("snare_hit.flac")), "andere snare");
        QSettings einstellungen(QDir(b.tmp.path()).filePath(QStringLiteral("einstellungen.ini")), QSettings::IniFormat);

        ImportPaketDialog dialog(paket, nullptr, &einstellungen);
        REQUIRE(dialog.inhalt().ok);
        CHECK(dialog.windowTitle() == QStringLiteral("Import package"));
        const QLabel* titel = dialog.findChild<QLabel*>(QStringLiteral("importPackageTitle"));
        REQUIRE(titel != nullptr);
        CHECK(titel->text() == QStringLiteral("Import package — titan"));
        const QLabel* info = dialog.findChild<QLabel*>(QStringLiteral("importPackageInfo"));
        CHECK(info->text().startsWith(QStringLiteral("Loop · published ")));
        CHECK(info->text().contains(QStringLiteral("with LumiViz 1.2.3 · 8 files")));

        // Contents: der Loop, dann je Datei, wohin sie kommt
        QTreeWidget* inhalt = dialog.inhaltsListe();
        REQUIRE(inhalt->topLevelItemCount() == 8);
        CHECK(inhalt->topLevelItem(0)->text(1) == QStringLiteral("Loop → Save to (below)"));
        QStringList wohin;
        for (int i = 1; i < inhalt->topLevelItemCount(); ++i)
            wohin << inhalt->topLevelItem(i)->text(0) + QLatin1Char(' ') + inhalt->topLevelItem(i)->text(1);
        CHECK(wohin.contains(QStringLiteral("bass → Instruments, same name (see below)")));
        CHECK(wohin.contains(QStringLiteral("kick → Samples, already here")));
        CHECK(wohin.contains(QStringLiteral("pump_bass → Instruments, new")));

        // Same names: zwei Zeilen, Vorgabe Keep both mit dem neuen Namen, ▶ je Seite
        QTreeWidget* gleiche = dialog.gleicheNamen();
        REQUIRE(gleiche != nullptr);
        CHECK(gleiche->topLevelItemCount() == 2);
        const QString bassPfad = QStringLiteral("data/instruments/bass.lvinst");
        const QString snarePfad = QStringLiteral("data/samples/snare_hit.wav");
        auto* bassWahl = dialog.findChild<QComboBox*>(QStringLiteral("use_") + bassPfad);
        REQUIRE(bassWahl != nullptr);
        CHECK(bassWahl->currentText() == QStringLiteral("Keep both → bass_2"));
        CHECK(dialog.wahl(snarePfad) == ds::Wahl::Beide);
        CHECK(dialog.summe() == QStringLiteral("Keep both renames 2 files; the package's loop will name them."));

        QStringList gespielt;
        QObject::connect(&dialog, &ImportPaketDialog::anspielenVerlangt,
                         [&gespielt](int, const QString& datei) { gespielt << QFileInfo(datei).fileName(); });
        auto* mein = dialog.findChild<QToolButton*>(QStringLiteral("playMine_snare_hit"));
        auto* ihr = dialog.findChild<QToolButton*>(QStringLiteral("playPackage_snare_hit"));
        REQUIRE(mein != nullptr);
        REQUIRE(ihr != nullptr);
        mein->click();
        ihr->click();
        CHECK(gespielt == QStringList{QStringLiteral("snare_hit.flac"), QStringLiteral("snare_hit.wav")});

        // Notes: die fehlende Datei des Absenders, das Mitgelieferte ist da
        const QStringList hinweise = dialog.hinweise();
        REQUIRE(hinweise.size() == 2);
        CHECK(hinweise[0].contains(QStringLiteral("user:gibt_es_nicht")));
        CHECK(hinweise[1].startsWith(QStringLiteral("Bundled with LumiViz, all present: ")));

        // Save to und Open after import
        CHECK(QFileInfo(dialog.ziel()).fileName() == QStringLiteral("titan.lvloop"));
        CHECK(dialog.oeffnen());
        bild(dialog, "LUMIVIZ_IMPORT_PAKET_BILD");

        dialog.setzeWahl(bassPfad, ds::Wahl::Meine);
        CHECK(dialog.summe() == QStringLiteral("Keep both renames 1 file; the package's loop will name them. "
                                               "Use mine keeps 1 file of yours."));
        dialog.setzeAlle(ds::Wahl::Importierte);
        CHECK(dialog.wahl(bassPfad) == ds::Wahl::Importierte);
        dialog.setzeAlle(ds::Wahl::Beide);
        dialog.setzeWahl(bassPfad, ds::Wahl::Meine);

        const QString ziel = QDir(e.loops).filePath(QStringLiteral("von_paket.lvloop"));
        dialog.setzeZiel(ziel);
        const pb::PaketErgebnis ergebnis = dialog.fuehreAus();
        REQUIRE_MESSAGE(ergebnis.ok, ergebnis.fehler.toStdString());
        CHECK(ergebnis.eigeneGenommen == 1);
        CHECK(ergebnis.umbenannt == 1);
        lp::Loop loop;
        REQUIRE(lp::ladeLoop(ziel, loop));
        CHECK(loop.muster.at(2).instrument == QStringLiteral("user:bass"));
        CHECK(QDir::fromNativeSeparators(einstellungen.value(QStringLiteral("data/lastPackageDir")).toString()) == e.loops);
    }

    TEST_CASE("Publish Knoepfe: Publish... im Instrument-Creator, in Kits, Edit shape und Record sample (P2 a)")
    {
        const DialogApp app;
        lp::InstrumentCreator creator;
        lp::KitFenster kits;
        lp::FormFenster form;
        lp::AufnahmeFenster aufnahme;
        const std::vector<std::pair<QWidget*, const char*>> orte = {
            {&creator, "creatorPublish"}, {&kits, "kitsPublish"}, {&form, "shapePublish"}, {&aufnahme, "recordPublish"}};
        for (const auto& [fenster, name] : orte)
        {
            CAPTURE(name);
            auto* knopf = fenster->findChild<QPushButton*>(QString::fromLatin1(name));
            REQUIRE(knopf != nullptr);
            CHECK(knopf->text() == QStringLiteral("Publish..."));
            CHECK_FALSE(knopf->toolTip().isEmpty());
        }
    }

    TEST_CASE("Import package Dialog: ohne gleiche Namen keine Liste; eine Datensicherung laesst sich nicht importieren")
    {
        const DialogApp app;
        const Bestand b;
        const QString paket = packeTitan(b);
        const Empfaenger e(b.tmp.path());
        QSettings einstellungen(QDir(b.tmp.path()).filePath(QStringLiteral("einstellungen.ini")), QSettings::IniFormat);

        ImportPaketDialog leer(paket, nullptr, &einstellungen);
        REQUIRE(leer.inhalt().ok);
        CHECK(leer.gleicheNamen() == nullptr);
        CHECK(leer.summe().isEmpty());

        const QString sicherung = QDir(b.tmp.path()).filePath(QStringLiteral("sicherung.lvdata"));
        REQUIRE(ds::exportiere(QDir(b.tmp.path()).filePath(QStringLiteral("user")), {{QStringLiteral("samples")}, false},
                               {}, sicherung, QStringLiteral("1.2.3"))
                    .ok);
        ImportPaketDialog keins(sicherung, nullptr, &einstellungen);
        CHECK_FALSE(keins.inhalt().ok);
        auto* knopf = keins.findChild<QPushButton*>(QStringLiteral("importPackageGo"));
        REQUIRE(knopf != nullptr);
        CHECK_FALSE(knopf->isEnabled());
    }

    TEST_CASE("Publish Dialog 30i: Frozen clips im Baum, Haken, Summe; fehlende rechnen ab dem Oeffnen, Publish wartet (FP2 a)")
    {
        const DialogApp app;
        const FreezeEinstellungen freezeEinstellungen;
        const FrozenBestand b;
        const FrozenOrdner ordner(b.absender);
        REQUIRE(lp::legeFrozen(b.bass, welle(0.25f), kFreezeRate));  // Drums fehlt noch
        QSettings einstellungen(QDir(b.tmp.path()).filePath(QStringLiteral("einstellungen.ini")), QSettings::IniFormat);
        const pb::Fund fund = pb::sammle(b.loopDatei);
        REQUIRE(fund.ok);

        // ohne Ton (wie bisher): keine Gruppe, kein Haken
        {
            PublishDialog ohne(fund, nullptr, &einstellungen);
            CHECK(ohne.frozenGruppe() == nullptr);
            CHECK(ohne.frozenHaken() == nullptr);
            CHECK_FALSE(ohne.rechnet());
            CHECK(ohne.summe().startsWith(QStringLiteral("8 files, ")));
        }

        PublishDialog dialog(fund, nullptr, &einstellungen, stillerTon());
        const QTreeWidgetItem* gruppe = dialog.frozenGruppe();
        REQUIRE(gruppe != nullptr);
        CHECK(gruppe->text(0) == QStringLiteral("Frozen clips (2)"));
        CHECK(gruppe->parent() == dialog.inhaltsBaum()->topLevelItem(0));
        CHECK(kinder(gruppe) == QStringList{QStringLiteral("Bass · bar 5"), QStringLiteral("Drums · bar 1")});
        CHECK(gruppe->child(0)->text(1).startsWith(QStringLiteral("ready · ")));
        CHECK(gruppe->child(1)->text(1).startsWith(QStringLiteral("computing")));  // ab dem Oeffnen
        CHECK(dialog.rechnet());

        QCheckBox* haken = dialog.frozenHaken();
        REQUIRE(haken != nullptr);
        CHECK(haken->text() == QStringLiteral("Include frozen clips"));
        CHECK(haken->isChecked());  // Vorgabe an
        CHECK(dialog.summe().startsWith(QStringLiteral("10 files, ")));
        haken->setChecked(false);
        CHECK(dialog.summe().startsWith(QStringLiteral("8 files, ")));
        CHECK_FALSE(einstellungen.value(QStringLiteral("data/publishFrozen")).toBool());
        haken->setChecked(true);
        CHECK(einstellungen.value(QStringLiteral("data/publishFrozen")).toBool());

        // Publish, solange gerechnet wird: es wartet, der Fuss zeigt den Stand
        const QString ausgabe = QDir(b.tmp.path()).filePath(QStringLiteral("ausgabe"));
        REQUIRE(QDir().mkpath(ausgabe));
        dialog.setzeZielOrdner(ausgabe);
        Meldungen meldungen;
        auto* publish = dialog.findChild<QPushButton*>(QStringLiteral("publishGo"));
        REQUIRE(publish != nullptr);
        publish->click();
        CHECK(dialog.wartet());
        CHECK_FALSE(publish->isEnabled());
        CHECK(dialog.summe().startsWith(QStringLiteral("Freezing clip 1 of 1")));
        CHECK(meldungen.namen.isEmpty());
        bild(dialog, "LUMIVIZ_PUBLISH_FROZEN_BILD");

        // fertig gerechnet: Publish laeuft von selbst weiter
        dialog.warteAufFreeze();
        CHECK_FALSE(dialog.rechnet());
        CHECK_FALSE(dialog.wartet());
        CHECK(gruppe->child(1)->text(1).startsWith(QStringLiteral("ready · ")));
        CHECK(meldungen.namen == QStringList{QStringLiteral("publishDone")});
        CHECK(dialog.result() == QDialog::Accepted);
        const pb::PaketInhalt inhalt = pb::oeffnePaket(dialog.paketDatei());
        REQUIRE_MESSAGE(inhalt.ok, inhalt.fehler.toStdString());
        CHECK(inhalt.frozen == 2);
        CHECK(QFileInfo::exists(lp::frozenDatei(b.drums)));  // Gerechnetes bleibt im Ordner frozen

        // ein zweites Oeffnen: alles liegt vor, nichts rechnet; ohne Haken geht keine Wellenform mit
        PublishDialog zweiter(fund, nullptr, &einstellungen, stillerTon());
        CHECK_FALSE(zweiter.rechnet());
        REQUIRE(zweiter.frozenHaken() != nullptr);
        zweiter.frozenHaken()->setChecked(false);
        zweiter.setzeZielOrdner(ausgabe);
        zweiter.setzeName(QStringLiteral("ohne_frozen"));
        const pb::PackErgebnis ohneFrozen = zweiter.fuehreAus();
        REQUIRE(ohneFrozen.ok);
        CHECK(ohneFrozen.frozen == 0);
    }

    TEST_CASE("Import package Dialog 30i: Frozen clips in Contents, Hinweis, Meldung nach dem Import (FP3 a)")
    {
        const DialogApp app;
        const FreezeEinstellungen freezeEinstellungen;
        const FrozenBestand b;
        QString paket;
        {
            const FrozenOrdner ordner(b.absender);
            REQUIRE(lp::legeFrozen(b.bass, welle(0.25f), kFreezeRate));
            REQUIRE(lp::legeFrozen(b.drums, welle(-0.5f), kFreezeRate));
            paket = packeMit(b, {b.bass, b.drums}, QStringLiteral("titan.lvloop.lvdata"));
        }
        const Empfaenger e(b.tmp.path());
        const FrozenOrdner ordner(b.empfaenger);
        QSettings einstellungen(QDir(b.tmp.path()).filePath(QStringLiteral("einstellungen.ini")), QSettings::IniFormat);

        ImportPaketDialog dialog(paket, nullptr, &einstellungen, kFreezeRate);
        REQUIRE(dialog.inhalt().ok);
        const QLabel* info = dialog.findChild<QLabel*>(QStringLiteral("importPackageInfo"));
        REQUIRE(info != nullptr);
        CHECK(info->text().contains(QStringLiteral("· 10 files")));
        QTreeWidget* inhalt = dialog.inhaltsListe();
        const QTreeWidgetItem* letzte = inhalt->topLevelItem(inhalt->topLevelItemCount() - 1);
        CHECK(letzte->text(0) == QStringLiteral("Frozen clips (2)"));
        CHECK(letzte->text(1).startsWith(QStringLiteral("→ frozen clips, ")));
        CHECK(dialog.hinweise().contains(QStringLiteral("Frozen clips: the receiver checks them on import")));
        bild(dialog, "LUMIVIZ_PUBLISH_FROZEN_BILD", QStringLiteral("_import"));

        dialog.setzeZiel(QDir(e.loops).filePath(QStringLiteral("titan.lvloop")));
        const pb::PaketErgebnis ergebnis = dialog.fuehreAus();
        REQUIRE_MESSAGE(ergebnis.ok, ergebnis.fehler.toStdString());
        CHECK(ergebnis.frozenGelegt == 2);
        CHECK(ImportPaketDialog::frozenMeldung(ergebnis) == QStringLiteral("Frozen clips: 2 of 2 ready to play"));

        pb::PaketErgebnis teils;
        teils.frozenClips = 3;
        teils.frozenBereit = 2;
        CHECK(ImportPaketDialog::frozenMeldung(teils) ==
              QStringLiteral("Frozen clips: 2 of 3 ready to play — 1 will be computed here"));
        CHECK(ImportPaketDialog::frozenMeldung(pb::PaketErgebnis{}).isEmpty());

        // ohne Rate (wie bisher): nichts wird abgelegt
        ImportPaketDialog ohne(paket, nullptr, &einstellungen);
        ohne.setzeZiel(QDir(e.loops).filePath(QStringLiteral("titan_ohne.lvloop")));
        CHECK(ohne.fuehreAus().frozenClips == 0);
    }
}
