/**
 * @file test_PublishDialoge.cpp
 * @brief Publish, Stufe P-A: die Dialoge Publish und Import package
 *        (docs/core-services/Publish_Konzept.md §4b–§4d; Entscheide P6 a, P7 a, P8 a)
 *        — ohne gezeigtes Fenster, Bestand aus `PaketBestand.hpp`, die
 *        Einstellungen in einer Ini-Datei.
 *
 * Bilder zum Ansehen: `LUMIVIZ_PUBLISH_DIALOG_BILD` und
 * `LUMIVIZ_IMPORT_PAKET_BILD=<datei.png>`.
 */

#include "PaketBestand.hpp"

#include "UI/dialogs/PaketDialoge.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QImage>
#include <QLabel>
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

void bild(QWidget& fenster, const char* variable)
{
    fenster.resize(fenster.sizeHint().expandedTo(fenster.size()));
    const QImage aufnahme = fenster.grab().toImage();
    CHECK_FALSE(aufnahme.isNull());
    if (const QString ziel = qEnvironmentVariable(variable); !ziel.isEmpty()) CHECK(aufnahme.save(ziel));
}

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
}
