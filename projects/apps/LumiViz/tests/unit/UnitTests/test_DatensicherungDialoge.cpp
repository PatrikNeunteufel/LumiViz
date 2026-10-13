/**
 * @file test_DatensicherungDialoge.cpp
 * @brief Die Dialoge fuer Export und Import der eigenen Daten
 *        (docs/core-services/Datenablage_Konzept.md §4a, Stufe A4b) — ohne
 *        gezeigtes Fenster, in Wegwerf-Ordnern, die Einstellungen in einer
 *        Ini-Datei.
 *
 * Bilder zum Ansehen: `LUMIVIZ_EXPORT_DIALOG_BILD`, `LUMIVIZ_IMPORT_DIALOG_BILD`
 * und `LUMIVIZ_IMPORT_WAHL_BILD=<datei.png>`.
 */

#include <doctest.h>

#include "UI/dialogs/DatensicherungDialoge.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

namespace
{

namespace ds = lumi::core::datensicherung;
using lumi::ui::DatenExportDialog;
using lumi::ui::DatenImportDialog;
using lumi::ui::DatenJeDateiDialog;

void lege(const QString& wurzel, const QString& rel, const QByteArray& inhalt)
{
    const QString pfad = QDir(wurzel).filePath(rel);
    REQUIRE(QDir().mkpath(QFileInfo(pfad).absolutePath()));
    QFile datei(pfad);
    REQUIRE(datei.open(QIODevice::WriteOnly));
    datei.write(inhalt);
}

QByteArray lies(const QString& wurzel, const QString& rel)
{
    QFile datei(QDir(wurzel).filePath(rel));
    return datei.open(QIODevice::ReadOnly) ? datei.readAll() : QByteArray("<fehlt>");
}

void fuelle(const QString& wurzel)
{
    lege(wurzel, QStringLiteral("presets/visuals/equalizer/Neon bars.json"), "neon");
    lege(wurzel, QStringLiteral("presets/visuals/equalizer/Warm glow.json"), "warm");
    lege(wurzel, QStringLiteral("presets/visuals/superscope/Spiral 3.json"), "spiral");
    lege(wurzel, QStringLiteral("presets/gradients/Sunset.json"), "sunset");
    lege(wurzel, QStringLiteral("presets/gradients/Ice.json"), "ice");
    lege(wurzel, QStringLiteral("nodepresets/simpleScope/Ring.json"), "ring");
    lege(wurzel, QStringLiteral("karaoke/trails/Glow.lvtrail"), "trail");
    lege(wurzel, QStringLiteral("composer/fades/Soft.lvfade"), "fade");
    lege(wurzel, QStringLiteral("lyrics/Lied.lrc"), "[00:01.00]la");
    lege(wurzel, QStringLiteral("lyrics/offsets.json"), "{}");
}

struct Aufbau
{
    QTemporaryDir tmp;
    QString quelle;
    QString ziel;
    QString datei;
    QSettings einstellungen;

    Aufbau()
        : quelle(QDir(tmp.path()).filePath(QStringLiteral("quelle"))),
          ziel(QDir(tmp.path()).filePath(QStringLiteral("ziel"))),
          datei(QDir(tmp.path()).filePath(QStringLiteral("sicherung.lvdata"))),
          einstellungen(QDir(tmp.path()).filePath(QStringLiteral("einstellungen.ini")), QSettings::IniFormat)
    {
        REQUIRE(tmp.isValid());
        REQUIRE(QDir().mkpath(quelle));
        REQUIRE(QDir().mkpath(ziel));
        fuelle(quelle);
    }
};

void bild(QWidget& fenster, const char* variable)
{
    fenster.resize(fenster.sizeHint().expandedTo(fenster.size()));
    const QImage aufnahme = fenster.grab().toImage();
    CHECK_FALSE(aufnahme.isNull());
    if (const QString ziel = qEnvironmentVariable(variable); !ziel.isEmpty()) CHECK(aufnahme.save(ziel));
}

}  // namespace

TEST_CASE("Datensicherung Dialog Export: Bereiche mit Zahl, Songtexte und Settings anfangs aus, die Summe zaehlt mit")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Aufbau a;

    DatenExportDialog dialog(a.quelle, nullptr, &a.einstellungen);
    CHECK(dialog.windowTitle() == QStringLiteral("Export your data"));

    // je Bereich eine Zeile; anfangs an ist alles Eigene mit Namen
    for (const ds::Bereich& bereich : ds::bereiche())
    {
        CAPTURE(bereich.id.toStdString());
        auto* haken = dialog.findChild<QCheckBox*>(QStringLiteral("dataExport_") + bereich.id);
        REQUIRE(haken != nullptr);
        CHECK(haken->text() == bereich.titel);
        CHECK(haken->isChecked() == bereich.anfangsAn);
    }
    CHECK_FALSE(dialog.istAn(QStringLiteral("settings")));
    CHECK_FALSE(dialog.istAn(QStringLiteral("lyrics")));

    auto* summe = dialog.findChild<QLabel*>(QStringLiteral("dataExportSummary"));
    auto* knopf = dialog.findChild<QPushButton*>(QStringLiteral("dataExportRun"));
    REQUIRE(summe != nullptr);
    REQUIRE(knopf != nullptr);
    // 3 + 2 + 1 + 1 + 1 Dateien in fuenf Bereichen (ohne die Songtexte)
    CHECK(summe->text() == QStringLiteral("8 file(s) in 5 group(s)"));
    CHECK(knopf->isEnabled());
    // der vorgeschlagene Name traegt das Datum und die Endung
    CHECK(QFileInfo(dialog.ziel()).fileName().startsWith(QStringLiteral("LumiViz_20")));
    CHECK(dialog.ziel().endsWith(QStringLiteral(".lvdata")));

    dialog.setzeAn(QStringLiteral("lyrics"), true);
    dialog.setzeAn(QStringLiteral("settings"), true);
    CHECK(summe->text() == QStringLiteral("10 file(s) in 6 group(s), and settings"));
    bild(dialog, "LUMIVIZ_EXPORT_DIALOG_BILD");

    // Select none: nichts zu exportieren, der Knopf ist aus; Select all holt alles
    dialog.findChild<QPushButton*>(QStringLiteral("dataExportNone"))->click();
    CHECK(summe->text() == QStringLiteral("0 file(s) in 0 group(s)"));
    CHECK_FALSE(knopf->isEnabled());
    CHECK(dialog.wahl().bereiche.isEmpty());
    dialog.findChild<QPushButton*>(QStringLiteral("dataExportAll"))->click();
    CHECK(dialog.wahl().bereiche.size() == static_cast<int>(ds::bereiche().size()));
    CHECK(dialog.wahl().einstellungen);
    CHECK(knopf->isEnabled());

    // ohne Ziel kein Export
    dialog.setzeZiel(QString());
    CHECK_FALSE(knopf->isEnabled());
}

TEST_CASE("Datensicherung Dialog Export: schreibt die Datei, merkt den Ordner, haengt die Endung an")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Aufbau a;
    a.einstellungen.setValue(QStringLiteral("player/shuffle"), true);
    a.einstellungen.setValue(QStringLiteral("shadertoy/apiKey"), QStringLiteral("GEHEIM"));

    DatenExportDialog dialog(a.quelle, nullptr, &a.einstellungen);
    dialog.setzeAn(QStringLiteral("settings"), true);
    dialog.setzeAn(QStringLiteral("composer"), false);
    // ohne Endung getippt
    dialog.setzeZiel(QDir(a.tmp.path()).filePath(QStringLiteral("sicherung")));
    const ds::ExportErgebnis ergebnis = dialog.fuehreAus();
    REQUIRE_MESSAGE(ergebnis.ok, ergebnis.fehler.toStdString());
    CHECK(ergebnis.dateien == 7);
    CHECK(ergebnis.bereiche == 4);
    REQUIRE(QFileInfo::exists(a.datei));
    CHECK(QDir(a.einstellungen.value(QStringLiteral("data/lastExportDir")).toString()) == QDir(a.tmp.path()));

    const ds::Inhalt inhalt = ds::oeffne(a.datei, a.ziel);
    REQUIRE(inhalt.ok);
    CHECK(inhalt.bereiche.size() == 4);
    CHECK(inhalt.hatEinstellungen);
    CHECK(inhalt.einstellungen.value(QStringLiteral("player/shuffle")).toBool());
    CHECK_FALSE(inhalt.einstellungen.contains(QStringLiteral("shadertoy/apiKey")));
    CHECK_FALSE(inhalt.einstellungen.contains(QStringLiteral("data/lastExportDir")));
}

TEST_CASE("Datensicherung Dialog Import: nur was in der Datei steckt, Zahlen und gleiche Namen, die vier Regeln")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Aufbau a;
    a.einstellungen.setValue(QStringLiteral("player/shuffle"), true);
    {
        DatenExportDialog exportDialog(a.quelle, nullptr, &a.einstellungen);
        exportDialog.setzeAn(QStringLiteral("settings"), true);
        exportDialog.setzeAn(QStringLiteral("composer"), false);  // steckt dann nicht in der Datei
        exportDialog.setzeZiel(a.datei);
        REQUIRE(exportDialog.fuehreAus().ok);
    }
    // am Ziel: zwei Namen vergeben, eine Datei gleich
    lege(a.ziel, QStringLiteral("presets/visuals/equalizer/Neon bars.json"), "meine fassung");
    lege(a.ziel, QStringLiteral("presets/gradients/Sunset.json"), "mein abend");
    lege(a.ziel, QStringLiteral("presets/gradients/Ice.json"), "ice");

    QSettings zielEinstellungen(QDir(a.tmp.path()).filePath(QStringLiteral("ziel.ini")), QSettings::IniFormat);
    zielEinstellungen.setValue(QStringLiteral("player/shuffle"), false);

    DatenImportDialog dialog(a.datei, a.ziel, nullptr, &zielEinstellungen);
    REQUIRE(dialog.inhalt().ok);
    CHECK(dialog.windowTitle() == QStringLiteral("Import data"));
    auto* kopf = dialog.findChild<QLabel*>(QStringLiteral("dataImportHead"));
    REQUIRE(kopf != nullptr);
    CHECK(kopf->text().startsWith(QStringLiteral("sicherung.lvdata · exported with LumiViz ")));
    // davor das Datei-Icon des Exports (S120)
    auto* kopfIcon = dialog.findChild<QLabel*>(QStringLiteral("dataImportHeadIcon"));
    REQUIRE(kopfIcon != nullptr);
    CHECK_FALSE(kopfIcon->pixmap().isNull());

    // nur die Bereiche aus der Datei: kein Composer, keine Songtexte, keine leeren
    for (const char* da : {"visuals", "gradients", "nodepresets", "karaoke", "settings"})
    {
        CAPTURE(da);
        CHECK(dialog.findChild<QCheckBox*>(QStringLiteral("dataImport_") + QLatin1String(da)) != nullptr);
    }
    for (const char* fehlt : {"composer", "lyrics", "shadertemplates", "shadertoy", "instruments"})
    {
        CAPTURE(fehlt);
        CHECK(dialog.findChild<QCheckBox*>(QStringLiteral("dataImport_") + QLatin1String(fehlt)) == nullptr);
    }
    CHECK(dialog.istAn(QStringLiteral("visuals")));
    CHECK_FALSE(dialog.istAn(QStringLiteral("settings")));  // anfangs aus: sie ersetzen

    auto* hinweis = dialog.findChild<QLabel*>(QStringLiteral("dataImportHint"));
    REQUIRE(hinweis != nullptr);
    CHECK(dialog.vergeben() == 2);
    CHECK(hinweis->text() == QStringLiteral("2 name(s) already exist"));
    CHECK(dialog.regel() == ds::Regel::MeineBehalten);
    for (const char* name : {"dataImportKeepMine", "dataImportReplace", "dataImportKeepBoth", "dataImportDecide"})
    {
        auto* knopf = dialog.findChild<QRadioButton*>(QLatin1String(name));
        REQUIRE(knopf != nullptr);
        CHECK(knopf->isEnabled());
    }
    bild(dialog, "LUMIVIZ_IMPORT_DIALOG_BILD");

    // ein Bereich abgewaehlt: seine Namen zaehlen nicht mehr
    dialog.setzeAn(QStringLiteral("gradients"), false);
    CHECK(dialog.vergeben() == 1);
    CHECK(hinweis->text() == QStringLiteral("1 name(s) already exist"));
    dialog.setzeAn(QStringLiteral("visuals"), false);
    CHECK(hinweis->text() == QStringLiteral("No name exists yet"));
    CHECK_FALSE(dialog.findChild<QRadioButton*>(QStringLiteral("dataImportReplace"))->isEnabled());
    // gar nichts gewaehlt: Import ist aus
    dialog.setzeAn(QStringLiteral("nodepresets"), false);
    dialog.setzeAn(QStringLiteral("karaoke"), false);
    CHECK_FALSE(dialog.findChild<QPushButton*>(QStringLiteral("dataImportRun"))->isEnabled());

    // alles wieder an, „Replace mine", dazu die Einstellungen
    for (const char* id : {"visuals", "gradients", "nodepresets", "karaoke", "settings"})
        dialog.setzeAn(QLatin1String(id), true);
    dialog.setzeRegel(ds::Regel::Ersetzen);
    CHECK(dialog.regel() == ds::Regel::Ersetzen);
    const ds::ImportErgebnis ergebnis = dialog.fuehreAus();
    REQUIRE_MESSAGE(ergebnis.ok, ergebnis.fehler.toStdString());
    CHECK(ergebnis.ersetzt == 2);
    CHECK(ergebnis.gleich == 1);
    CHECK(ergebnis.eingelesen == 4);
    CHECK(lies(a.ziel, QStringLiteral("presets/visuals/equalizer/Neon bars.json")) == "neon");
    CHECK(lies(a.ziel, QStringLiteral("presets/gradients/Sunset.json")) == "sunset");
    CHECK_FALSE(QFileInfo::exists(QDir(a.ziel).filePath(QStringLiteral("composer"))));
    CHECK(dialog.einstellungenGesetzt() == 1);
    CHECK(zielEinstellungen.value(QStringLiteral("player/shuffle")).toBool());
}

TEST_CASE("Datensicherung Dialog Import: ohne Haken bei Settings bleiben die eigenen")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Aufbau a;
    a.einstellungen.setValue(QStringLiteral("player/shuffle"), true);
    {
        DatenExportDialog exportDialog(a.quelle, nullptr, &a.einstellungen);
        exportDialog.setzeAn(QStringLiteral("settings"), true);
        exportDialog.setzeZiel(a.datei);
        REQUIRE(exportDialog.fuehreAus().ok);
    }
    QSettings zielEinstellungen(QDir(a.tmp.path()).filePath(QStringLiteral("ziel.ini")), QSettings::IniFormat);
    zielEinstellungen.setValue(QStringLiteral("player/shuffle"), false);

    DatenImportDialog dialog(a.datei, a.ziel, nullptr, &zielEinstellungen);
    REQUIRE(dialog.inhalt().ok);
    REQUIRE(dialog.fuehreAus().ok);
    CHECK(dialog.einstellungenGesetzt() == 0);
    CHECK_FALSE(zielEinstellungen.value(QStringLiteral("player/shuffle")).toBool());
}

TEST_CASE("Datensicherung Dialog Import: eine Datei, die sich nicht lesen laesst — Meldung, Import aus")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Aufbau a;
    lege(a.tmp.path(), QStringLiteral("sicherung.lvdata"), "kein archiv");

    DatenImportDialog dialog(a.datei, a.ziel, nullptr, &a.einstellungen);
    CHECK_FALSE(dialog.inhalt().ok);
    auto* kopf = dialog.findChild<QLabel*>(QStringLiteral("dataImportHead"));
    REQUIRE(kopf != nullptr);
    CHECK(kopf->text().contains(dialog.inhalt().fehler));
    CHECK_FALSE(dialog.findChild<QPushButton*>(QStringLiteral("dataImportRun"))->isEnabled());
    CHECK(dialog.findChild<QRadioButton*>(QStringLiteral("dataImportReplace")) == nullptr);
}

TEST_CASE("Datensicherung Dialog Je Datei: eine Zeile je gleichem Namen, die Haken ergeben die Wahl")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    Aufbau a;
    REQUIRE(ds::exportiere(a.quelle, {{QStringLiteral("visuals"), QStringLiteral("gradients")}, false}, {}, a.datei,
                           QStringLiteral("0.12.0"))
                .ok);
    const QString neon = QStringLiteral("presets/visuals/equalizer/Neon bars.json");
    const QString warm = QStringLiteral("presets/visuals/equalizer/Warm glow.json");
    const QString spiral = QStringLiteral("presets/visuals/superscope/Spiral 3.json");
    const QString sunset = QStringLiteral("presets/gradients/Sunset.json");
    for (const QString& rel : {neon, warm, spiral, sunset}) lege(a.ziel, rel, "meine fassung");
    lege(a.ziel, QStringLiteral("presets/gradients/Ice.json"), "ice");  // gleich: keine Zeile

    const ds::Inhalt inhalt = ds::oeffne(a.datei, a.ziel);
    REQUIRE(inhalt.ok);
    DatenJeDateiDialog liste(inhalt.bereiche);
    CHECK(liste.zeilen() == 4);
    CHECK(liste.windowTitle() == QStringLiteral("4 name(s) already exist"));
    // anfangs bleibt ueberall die eigene
    CHECK(liste.summe() == QStringLiteral("Keeps 4 of yours, replaces 0, adds 0 under a new name"));
    for (const auto& [rel, was] : liste.wahl()) CHECK(was == ds::Wahl::Meine);

    liste.setze(neon, false, true);    // die importierte nehmen
    liste.setze(warm, true, true);     // beide
    liste.setze(spiral, false, false);  // beide aus: die eigene bleibt
    CHECK(liste.summe() == QStringLiteral("Keeps 2 of yours, replaces 1, adds 1 under a new name"));
    const auto wahl = liste.wahl();
    CHECK(wahl.at(neon) == ds::Wahl::Importierte);
    CHECK(wahl.at(warm) == ds::Wahl::Beide);
    CHECK(wahl.at(spiral) == ds::Wahl::Meine);
    CHECK(wahl.at(sunset) == ds::Wahl::Meine);
    // S120: eine Zeile traegt das Icon ihrer Dateiart, wo es eines gibt — der
    // Verlauf das seines Bereichs, ein Visualizer-Preset (`.json`) keins
    {
        auto* baum = liste.findChild<QTreeWidget*>(QStringLiteral("dataDecideList"));
        REQUIRE(baum != nullptr);
        int mitIcon = 0;
        int ohneIcon = 0;
        for (QTreeWidgetItemIterator it(baum); *it != nullptr; ++it)
        {
            if ((*it)->childCount() > 0) continue;  // eine Kopfzeile
            // ohne Icon steht ein leeres da, damit die Namen untereinander stehen
            REQUIRE_FALSE((*it)->icon(0).isNull());
            const QImage icon = (*it)->icon(0).pixmap(16, 16).toImage().convertToFormat(QImage::Format_ARGB32);
            bool hat = false;
            for (int y = 0; y < icon.height() && !hat; ++y)
                for (int x = 0; x < icon.width() && !hat; ++x) hat = qAlpha(icon.pixel(x, y)) != 0;
            CAPTURE((*it)->text(0).toStdString());
            CHECK(hat == ((*it)->text(0) == QStringLiteral("Sunset")));
            (hat ? mitIcon : ohneIcon) += 1;
        }
        CHECK(mitIcon == 1);
        CHECK(ohneIcon == 3);
    }
    bild(liste, "LUMIVIZ_IMPORT_WAHL_BILD");

    // die Wahl durch den Import-Dialog: genau das geschieht
    DatenImportDialog dialog(a.datei, a.ziel, nullptr, &a.einstellungen);
    dialog.setzeRegel(ds::Regel::JeDatei);
    dialog.setzeJeDatei(wahl);
    const ds::ImportErgebnis ergebnis = dialog.fuehreAus();
    REQUIRE(ergebnis.ok);
    CHECK(ergebnis.ersetzt == 1);
    CHECK(ergebnis.umbenannt == 1);
    CHECK(ergebnis.uebersprungen == 2);
    CHECK(ergebnis.gleich == 1);
    CHECK(lies(a.ziel, neon) == "neon");
    CHECK(lies(a.ziel, warm) == "meine fassung");
    CHECK(lies(a.ziel, QStringLiteral("presets/visuals/equalizer/Warm glow (2).json")) == "warm");
    CHECK(lies(a.ziel, spiral) == "meine fassung");

    // die drei Knoepfe ueber der Liste setzen ganze Spalten
    liste.findChild<QPushButton*>(QStringLiteral("dataDecideAllImported"))->click();
    CHECK(liste.summe() == QStringLiteral("Keeps 0 of yours, replaces 4, adds 0 under a new name"));
    liste.findChild<QPushButton*>(QStringLiteral("dataDecideAllBoth"))->click();
    CHECK(liste.summe() == QStringLiteral("Keeps 0 of yours, replaces 0, adds 4 under a new name"));
    liste.findChild<QPushButton*>(QStringLiteral("dataDecideAllMine"))->click();
    CHECK(liste.summe() == QStringLiteral("Keeps 4 of yours, replaces 0, adds 0 under a new name"));
}

TEST_CASE("Datensicherung Dialog: groesseAlsText")
{
    CHECK(lumi::ui::groesseAlsText(0) == QStringLiteral("0 B"));
    CHECK(lumi::ui::groesseAlsText(812) == QStringLiteral("812 B"));
    CHECK(lumi::ui::groesseAlsText(1024) == QStringLiteral("1 KB"));
    CHECK(lumi::ui::groesseAlsText(2600) == QStringLiteral("3 KB"));
    CHECK(lumi::ui::groesseAlsText(1500000) == QStringLiteral("1.4 MB"));
}
