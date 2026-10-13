/**
 * @file test_Datensicherung.cpp
 * @brief Die eigenen Daten exportieren und importieren — die Regeln, in zwei
 *        Wegwerf-Ordnern, ohne App und ohne Registry
 *        (docs/core-services/Datenablage_Konzept.md, Stufe A4; Entscheide
 *        D6 a, D7 a, E1 a)
 */

#include <doctest.h>

#include "core/Datensicherung.hpp"
#include "core/ZipArchiv.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSettings>
#include <QTemporaryDir>

namespace
{

namespace ds = lumi::core::datensicherung;

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

bool gibtEs(const QString& wurzel, const QString& rel)
{
    return QFileInfo::exists(QDir(wurzel).filePath(rel));
}

/// Ein Benutzerordner mit etwas in fuenf Bereichen — und mit Dateien, die
/// nie mitgehen duerfen.
void fuelle(const QString& wurzel)
{
    lege(wurzel, QStringLiteral("presets/visuals/equalizer/Neon bars.json"), "neon");
    lege(wurzel, QStringLiteral("presets/visuals/superscope/Spiral 3.json"), "spiral");
    lege(wurzel, QStringLiteral("presets/smoothing/Weich.json"), "weich");
    lege(wurzel, QStringLiteral("presets/gradients/Sunset.json"), "sunset");
    lege(wurzel, QStringLiteral("nodepresets/simpleScope/Ring.json"), "ring");
    lege(wurzel, QStringLiteral("karaoke/test.lvkar"), "kar");
    lege(wurzel, QStringLiteral("karaoke/trails/Gl\u00fchen.lvtrail"), "trail");
    lege(wurzel, QStringLiteral("karaoke/current.lvkar"), "laufender stand");
    lege(wurzel, QStringLiteral("lyrics/Lied.lrc"), "[00:01.00]la");
    lege(wurzel, QStringLiteral("lyrics/offsets.json"), "{}");
    lege(wurzel, QStringLiteral("lyrics/notfound.json"), "{}");
    // nichts davon gehoert in einen Export
    lege(wurzel, QStringLiteral("session.m3u8"), "playlist");
    lege(wurzel, QStringLiteral("testaufnahmen/clip.mp4"), "video");
}

QStringList alleBereiche()
{
    QStringList ids;
    for (const ds::Bereich& bereich : ds::bereiche()) ids << bereich.id;
    return ids;
}

struct Ordner
{
    QTemporaryDir tmp;
    QString quelle;  ///< der Benutzerordner, aus dem exportiert wird
    QString ziel;    ///< der Benutzerordner, in den importiert wird
    QString datei;

    Ordner()
    {
        REQUIRE(tmp.isValid());
        quelle = QDir(tmp.path()).filePath(QStringLiteral("quelle"));
        ziel = QDir(tmp.path()).filePath(QStringLiteral("ziel"));
        datei = QDir(tmp.path()).filePath(QStringLiteral("sicherung.lvdata"));
        REQUIRE(QDir().mkpath(quelle));
        REQUIRE(QDir().mkpath(ziel));
    }
};

const ds::ImportBereich* finde(const ds::Inhalt& inhalt, const QString& id)
{
    for (const ds::ImportBereich& bereich : inhalt.bereiche)
        if (bereich.id == id) return &bereich;
    return nullptr;
}

}  // namespace

TEST_CASE("Datensicherung: die Bereiche — feste Namen, jeder mit Ordner, nur die Songtexte anfangs aus")
{
    const auto& bereiche = ds::bereiche();
    REQUIRE(bereiche.size() == 11);  // seit S120 dazu „Shapes" (Klang aus einer Form, Stufe 19a); S115: „Samples" (Loop, Stufe 11a)
    QStringList ids;
    for (const ds::Bereich& bereich : bereiche)
    {
        CAPTURE(bereich.id.toStdString());
        CHECK_FALSE(bereich.titel.isEmpty());
        CHECK_FALSE(bereich.ordner.isEmpty());
        CHECK_FALSE(ids.contains(bereich.id));
        ids << bereich.id;
        CHECK(bereich.anfangsAn == (bereich.id != QLatin1String("lyrics")));
        CHECK(ds::findeBereich(bereich.id) == &bereich);
    }
    CHECK(ds::findeBereich(QStringLiteral("gibt_es_nicht")) == nullptr);
    // kein Bereich nimmt die Testaufnahmen
    for (const ds::Bereich& bereich : bereiche) CHECK_FALSE(bereich.ordner.contains(QStringLiteral("testaufnahmen")));
}

TEST_CASE("Datensicherung: sammle — mit Unterordnern, ohne laufenden Stand und Zwischenspeicher")
{
    const Ordner ordner;
    fuelle(ordner.quelle);

    const auto visuals = ds::sammle(ordner.quelle, *ds::findeBereich(QStringLiteral("visuals")));
    REQUIRE(visuals.size() == 3);  // zwei unter visuals, eines unter smoothing
    CHECK(visuals[0].rel == QStringLiteral("presets/smoothing/Weich.json"));
    CHECK(visuals[1].rel == QStringLiteral("presets/visuals/equalizer/Neon bars.json"));
    CHECK(visuals[1].groesse == 4);
    CHECK(visuals[1].geaendert.isValid());

    const auto karaoke = ds::sammle(ordner.quelle, *ds::findeBereich(QStringLiteral("karaoke")));
    REQUIRE(karaoke.size() == 2);  // ohne current.lvkar
    for (const ds::Datei& datei : karaoke) CHECK_FALSE(datei.rel.endsWith(QStringLiteral("current.lvkar")));

    const auto lyrics = ds::sammle(ordner.quelle, *ds::findeBereich(QStringLiteral("lyrics")));
    REQUIRE(lyrics.size() == 2);  // der Text und die Versaetze, ohne notfound.json

    CHECK(ds::sammle(ordner.quelle, *ds::findeBereich(QStringLiteral("instruments"))).empty());  // Ordner fehlt
    CHECK(ds::sammle(QString(), *ds::findeBereich(QStringLiteral("visuals"))).empty());
}

TEST_CASE("Datensicherung: Export und Import in einen leeren Ordner — alles kommt an, sonst nichts")
{
    const Ordner ordner;
    fuelle(ordner.quelle);

    const ds::ExportErgebnis exportiert =
        ds::exportiere(ordner.quelle, {alleBereiche(), false}, {}, ordner.datei, QStringLiteral("0.12.0"));
    REQUIRE_MESSAGE(exportiert.ok, exportiert.fehler.toStdString());
    CHECK(exportiert.dateien == 9);
    CHECK(exportiert.bereiche == 5);

    const ds::Inhalt inhalt = ds::oeffne(ordner.datei, ordner.ziel);
    REQUIRE_MESSAGE(inhalt.ok, inhalt.fehler.toStdString());
    CHECK(inhalt.fassung == ds::kFassung);
    CHECK(inhalt.appVersion == QStringLiteral("0.12.0"));
    CHECK(inhalt.exportiert.isValid());
    CHECK_FALSE(inhalt.hatEinstellungen);
    CHECK(inhalt.ausgelassen == 0);
    // nur die Bereiche, die in der Datei stecken, in der Reihenfolge des Dialogs
    REQUIRE(inhalt.bereiche.size() == 5);
    CHECK(inhalt.bereiche[0].id == QStringLiteral("visuals"));
    CHECK(inhalt.bereiche[1].id == QStringLiteral("gradients"));
    CHECK(inhalt.bereiche[2].id == QStringLiteral("nodepresets"));
    CHECK(inhalt.bereiche[3].id == QStringLiteral("karaoke"));
    CHECK(inhalt.bereiche[4].id == QStringLiteral("lyrics"));
    for (const ds::ImportBereich& bereich : inhalt.bereiche)
    {
        CHECK(bereich.vergeben() == 0);
        for (const ds::ImportDatei& datei : bereich.dateien) CHECK(datei.zustand == ds::Zustand::Neu);
    }

    const ds::ImportErgebnis importiert = ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), {}, {}});
    REQUIRE_MESSAGE(importiert.ok, importiert.fehler.toStdString());
    CHECK(importiert.eingelesen == 9);
    CHECK(importiert.ersetzt + importiert.umbenannt + importiert.uebersprungen + importiert.gleich == 0);

    CHECK(lies(ordner.ziel, QStringLiteral("presets/visuals/equalizer/Neon bars.json")) == "neon");
    CHECK(lies(ordner.ziel, QStringLiteral("karaoke/trails/Gl\u00fchen.lvtrail")) == "trail");
    CHECK(lies(ordner.ziel, QStringLiteral("lyrics/offsets.json")) == "{}");
    CHECK_FALSE(gibtEs(ordner.ziel, QStringLiteral("karaoke/current.lvkar")));
    CHECK_FALSE(gibtEs(ordner.ziel, QStringLiteral("lyrics/notfound.json")));
    CHECK_FALSE(gibtEs(ordner.ziel, QStringLiteral("session.m3u8")));
    CHECK_FALSE(gibtEs(ordner.ziel, QStringLiteral("testaufnahmen")));

    // die Zeit der Datei kommt mit (ein Zip haelt sie auf zwei Sekunden genau)
    const QString neon = QStringLiteral("presets/visuals/equalizer/Neon bars.json");
    CHECK(qAbs(QFileInfo(QDir(ordner.quelle).filePath(neon))
                   .lastModified()
                   .secsTo(QFileInfo(QDir(ordner.ziel).filePath(neon)).lastModified())) <= 2);
}

TEST_CASE("Datensicherung: nur gewaehlte Bereiche — im Export und im Import")
{
    const Ordner ordner;
    fuelle(ordner.quelle);

    // Export: nur zwei Bereiche
    const auto exportiert = ds::exportiere(ordner.quelle, {{QStringLiteral("gradients"), QStringLiteral("karaoke")}, false},
                                           {}, ordner.datei, QStringLiteral("0.12.0"));
    REQUIRE(exportiert.ok);
    CHECK(exportiert.dateien == 3);
    const ds::Inhalt inhalt = ds::oeffne(ordner.datei, ordner.ziel);
    REQUIRE(inhalt.ok);
    REQUIRE(inhalt.bereiche.size() == 2);
    CHECK(finde(inhalt, QStringLiteral("visuals")) == nullptr);

    // Import: davon nur einer
    const auto importiert = ds::importiere(ordner.datei, ordner.ziel, {{QStringLiteral("gradients")}, {}, {}});
    REQUIRE(importiert.ok);
    CHECK(importiert.eingelesen == 1);
    CHECK(gibtEs(ordner.ziel, QStringLiteral("presets/gradients/Sunset.json")));
    CHECK_FALSE(gibtEs(ordner.ziel, QStringLiteral("karaoke")));

    // ein gewaehlter Bereich ohne Dateien ist kein Fehler: das Archiv traegt ihn nur nicht
    const auto leer = ds::exportiere(ordner.quelle, {{QStringLiteral("instruments")}, false}, {}, ordner.datei,
                                     QStringLiteral("0.12.0"));
    REQUIRE(leer.ok);
    CHECK(leer.dateien == 0);
    CHECK(ds::oeffne(ordner.datei, ordner.ziel).bereiche.empty());
}

TEST_CASE("Datensicherung: gleiche Namen — erkannt, gleiche Bytes zaehlen nicht (D7 a)")
{
    const Ordner ordner;
    fuelle(ordner.quelle);
    REQUIRE(ds::exportiere(ordner.quelle, {alleBereiche(), false}, {}, ordner.datei, QStringLiteral("0.12.0")).ok);

    // am Ziel: eine Datei gleich, eine mit anderem Inhalt, eine eigene dazu
    lege(ordner.ziel, QStringLiteral("presets/gradients/Sunset.json"), "sunset");
    lege(ordner.ziel, QStringLiteral("presets/visuals/equalizer/Neon bars.json"), "meine fassung");
    lege(ordner.ziel, QStringLiteral("presets/visuals/equalizer/Nur hier.json"), "eigen");

    const ds::Inhalt inhalt = ds::oeffne(ordner.datei, ordner.ziel);
    REQUIRE(inhalt.ok);
    const ds::ImportBereich* gradients = finde(inhalt, QStringLiteral("gradients"));
    REQUIRE(gradients != nullptr);
    CHECK(gradients->vergeben() == 0);
    CHECK(gradients->dateien.front().zustand == ds::Zustand::Gleich);

    const ds::ImportBereich* visuals = finde(inhalt, QStringLiteral("visuals"));
    REQUIRE(visuals != nullptr);
    CHECK(visuals->vergeben() == 1);
    const auto neon = std::find_if(visuals->dateien.begin(), visuals->dateien.end(),
                                   [](const ds::ImportDatei& d) { return d.rel.endsWith(QStringLiteral("Neon bars.json")); });
    REQUIRE(neon != visuals->dateien.end());
    CHECK(neon->zustand == ds::Zustand::NameVergeben);
    CHECK(neon->groesse == 4);
    CHECK(neon->eigeneGroesse == 13);
    CHECK(neon->eigeneGeaendert.isValid());

    // das Ansehen schreibt nichts
    CHECK(lies(ordner.ziel, QStringLiteral("presets/visuals/equalizer/Neon bars.json")) == "meine fassung");
    CHECK_FALSE(gibtEs(ordner.ziel, QStringLiteral("nodepresets")));
}

TEST_CASE("Datensicherung: die drei Regeln fuer den ganzen Import")
{
    const QString neon = QStringLiteral("presets/visuals/equalizer/Neon bars.json");
    const auto bereite = [&](const Ordner& ordner) {
        fuelle(ordner.quelle);
        REQUIRE(ds::exportiere(ordner.quelle, {alleBereiche(), false}, {}, ordner.datei, QStringLiteral("0.12.0")).ok);
        lege(ordner.ziel, neon, "meine fassung");
        lege(ordner.ziel, QStringLiteral("presets/gradients/Sunset.json"), "sunset");  // gleich
    };

    SUBCASE("Keep mine: die eigene bleibt, der Rest kommt")
    {
        const Ordner ordner;
        bereite(ordner);
        const auto e = ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), ds::Regel::MeineBehalten, {}});
        REQUIRE(e.ok);
        CHECK(e.uebersprungen == 1);
        CHECK(e.gleich == 1);
        CHECK(e.eingelesen == 7);
        CHECK(e.ersetzt == 0);
        CHECK(e.umbenannt == 0);
        CHECK(lies(ordner.ziel, neon) == "meine fassung");
    }
    SUBCASE("Replace mine: die importierte ersetzt")
    {
        const Ordner ordner;
        bereite(ordner);
        const auto e = ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), ds::Regel::Ersetzen, {}});
        REQUIRE(e.ok);
        CHECK(e.ersetzt == 1);
        CHECK(e.gleich == 1);
        CHECK(e.eingelesen == 7);
        CHECK(lies(ordner.ziel, neon) == "neon");
    }
    SUBCASE("Keep both: die importierte bekommt eine Zahl, die eigene bleibt")
    {
        const Ordner ordner;
        bereite(ordner);
        const auto e = ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), ds::Regel::BeideBehalten, {}});
        REQUIRE(e.ok);
        CHECK(e.umbenannt == 1);
        CHECK(e.gleich == 1);
        CHECK(lies(ordner.ziel, neon) == "meine fassung");
        CHECK(lies(ordner.ziel, QStringLiteral("presets/visuals/equalizer/Neon bars (2).json")) == "neon");

        // ist die (2) schon da, wird es die (3)
        lege(ordner.ziel, neon, "wieder anders");
        const auto nochmal = ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), ds::Regel::BeideBehalten, {}});
        REQUIRE(nochmal.ok);
        CHECK(nochmal.umbenannt == 1);
        CHECK(lies(ordner.ziel, QStringLiteral("presets/visuals/equalizer/Neon bars (3).json")) == "neon");
    }
}

TEST_CASE("Datensicherung: je Datei entscheiden — was fehlt, behaelt die eigene")
{
    const Ordner ordner;
    fuelle(ordner.quelle);
    REQUIRE(ds::exportiere(ordner.quelle, {alleBereiche(), false}, {}, ordner.datei, QStringLiteral("0.12.0")).ok);
    const QString neon = QStringLiteral("presets/visuals/equalizer/Neon bars.json");
    const QString spiral = QStringLiteral("presets/visuals/superscope/Spiral 3.json");
    const QString ring = QStringLiteral("nodepresets/simpleScope/Ring.json");
    const QString sunset = QStringLiteral("presets/gradients/Sunset.json");
    for (const QString& rel : {neon, spiral, ring, sunset}) lege(ordner.ziel, rel, "meine");

    ds::ImportWahl wahl{alleBereiche(), ds::Regel::JeDatei, {}};
    wahl.jeDatei[neon] = ds::Wahl::Importierte;
    wahl.jeDatei[spiral] = ds::Wahl::Beide;
    wahl.jeDatei[ring] = ds::Wahl::Meine;
    // sunset steht nicht in der Liste

    const auto e = ds::importiere(ordner.datei, ordner.ziel, wahl);
    REQUIRE(e.ok);
    CHECK(e.ersetzt == 1);
    CHECK(e.umbenannt == 1);
    CHECK(e.uebersprungen == 2);
    CHECK(lies(ordner.ziel, neon) == "neon");
    CHECK(lies(ordner.ziel, spiral) == "meine");
    CHECK(lies(ordner.ziel, QStringLiteral("presets/visuals/superscope/Spiral 3 (2).json")) == "spiral");
    CHECK(lies(ordner.ziel, ring) == "meine");
    CHECK(lies(ordner.ziel, sunset) == "meine");
}

TEST_CASE("Datensicherung: nameMitZahl — vor der Endung, die erste freie Zahl")
{
    const auto nie = [](const QString&) { return false; };
    CHECK(ds::nameMitZahl(QStringLiteral("a/b/Neon.json"), nie) == QStringLiteral("a/b/Neon (2).json"));
    CHECK(ds::nameMitZahl(QStringLiteral("Neon.v2.json"), nie) == QStringLiteral("Neon.v2 (2).json"));
    CHECK(ds::nameMitZahl(QStringLiteral("a/ohne_endung"), nie) == QStringLiteral("a/ohne_endung (2)"));
    CHECK(ds::nameMitZahl(QStringLiteral("a/.versteckt"), nie) == QStringLiteral("a/.versteckt (2)"));
    const auto bisVier = [](const QString& name) {
        return name.contains(QStringLiteral("(2)")) || name.contains(QStringLiteral("(3)")) ||
               name.contains(QStringLiteral("(4)"));
    };
    CHECK(ds::nameMitZahl(QStringLiteral("a/Neon.json"), bisVier) == QStringLiteral("a/Neon (5).json"));
}

TEST_CASE("Datensicherung: ein Archiv schreibt nur in die Ordner der Bereiche")
{
    const Ordner ordner;
    using lumi::core::ZipEintrag;
    const QByteArray manifest =
        QJsonDocument(QJsonObject{{QStringLiteral("format"), QLatin1String(ds::kFormat)},
                                  {QStringLiteral("version"), ds::kFassung},
                                  {QStringLiteral("appVersion"), QStringLiteral("0.12.0")}})
            .toJson();
    const std::vector<ZipEintrag> boese = {
        {QStringLiteral("manifest.json"), manifest, {}},
        {QStringLiteral("data/presets/gradients/Gut.json"), "gut", {}},
        {QStringLiteral("data/presets/gradients/../../../raus.txt"), "x", {}},
        {QStringLiteral("data/../raus2.txt"), "x", {}},
        {QStringLiteral("data//presets/gradients/Leer.json"), "x", {}},
        // Namen mit Laufwerk oder `\` nimmt schon miniz beim Schreiben nicht an —
        // ein solches Archiv laesst sich hier nicht bauen; `pfadInOrdnung` lehnt sie trotzdem ab
        {QStringLiteral("data/session.m3u8"), "x", {}},              // kein Bereich
        {QStringLiteral("data/testaufnahmen/clip.mp4"), "x", {}},    // kein Bereich
        {QStringLiteral("data/karaoke/current.lvkar"), "x", {}},     // der laufende Stand
        {QStringLiteral("data/lyrics/notfound.json"), "x", {}},      // Zwischenspeicher
        {QStringLiteral("irgendwo/anders.txt"), "x", {}},            // nicht unter data/
    };
    REQUIRE(lumi::core::schreibeZip(ordner.datei, boese));

    const ds::Inhalt inhalt = ds::oeffne(ordner.datei, ordner.ziel);
    REQUIRE_MESSAGE(inhalt.ok, inhalt.fehler.toStdString());
    CHECK(inhalt.ausgelassen == 8);
    REQUIRE(inhalt.bereiche.size() == 1);
    CHECK(inhalt.bereiche.front().dateien.size() == 1);

    const auto e = ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), ds::Regel::Ersetzen, {}});
    REQUIRE(e.ok);
    CHECK(e.eingelesen == 1);
    CHECK(lies(ordner.ziel, QStringLiteral("presets/gradients/Gut.json")) == "gut");
    // sonst liegt im ganzen Wegwerf-Ordner nichts Neues: nur die Sicherung, `quelle` und `ziel`
    CHECK(QDir(ordner.tmp.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot).size() == 3);
    CHECK(QDir(ordner.ziel).entryList(QDir::AllEntries | QDir::NoDotAndDotDot) == QStringList{QStringLiteral("presets")});
    CHECK(QDir(QDir(ordner.ziel).filePath(QStringLiteral("presets/gradients")))
              .entryList(QDir::Files) == QStringList{QStringLiteral("Gut.json")});
}

TEST_CASE("Datensicherung: fremde, neuere und kaputte Dateien werden abgelehnt und gemeldet")
{
    const Ordner ordner;
    using lumi::core::ZipEintrag;

    SUBCASE("kein Zip")
    {
        lege(ordner.tmp.path(), QStringLiteral("sicherung.lvdata"), "kein archiv");
        const ds::Inhalt inhalt = ds::oeffne(ordner.datei, ordner.ziel);
        CHECK_FALSE(inhalt.ok);
        CHECK_FALSE(inhalt.fehler.isEmpty());
        CHECK_FALSE(ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), {}, {}}).ok);
    }
    SUBCASE("ein Zip ohne Manifest")
    {
        REQUIRE(lumi::core::schreibeZip(ordner.datei, {{QStringLiteral("data/presets/gradients/A.json"), "a", {}}}));
        CHECK_FALSE(ds::oeffne(ordner.datei, ordner.ziel).ok);
        CHECK_FALSE(ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), {}, {}}).ok);
        CHECK_FALSE(gibtEs(ordner.ziel, QStringLiteral("presets")));
    }
    SUBCASE("ein anderes Format")
    {
        REQUIRE(lumi::core::schreibeZip(
            ordner.datei, {{QStringLiteral("manifest.json"), R"({"format":"etwas-anderes","version":1})", {}}}));
        CHECK_FALSE(ds::oeffne(ordner.datei, ordner.ziel).ok);
    }
    SUBCASE("eine neuere Fassung: die Meldung nennt die LumiViz-Version")
    {
        const QByteArray manifest =
            QJsonDocument(QJsonObject{{QStringLiteral("format"), QLatin1String(ds::kFormat)},
                                      {QStringLiteral("version"), ds::kFassung + 1},
                                      {QStringLiteral("appVersion"), QStringLiteral("9.9.9")}})
                .toJson();
        REQUIRE(lumi::core::schreibeZip(ordner.datei,
                                        {{QStringLiteral("manifest.json"), manifest, {}},
                                         {QStringLiteral("data/presets/gradients/A.json"), "a", {}}}));
        const ds::Inhalt inhalt = ds::oeffne(ordner.datei, ordner.ziel);
        CHECK_FALSE(inhalt.ok);
        CHECK(inhalt.fehler.contains(QStringLiteral("9.9.9")));
        CHECK_FALSE(ds::importiere(ordner.datei, ordner.ziel, {alleBereiche(), {}, {}}).ok);
        CHECK_FALSE(gibtEs(ordner.ziel, QStringLiteral("presets")));
    }
    SUBCASE("ohne Benutzerordner wird nichts importiert")
    {
        fuelle(ordner.quelle);
        REQUIRE(ds::exportiere(ordner.quelle, {alleBereiche(), false}, {}, ordner.datei, QStringLiteral("0.12.0")).ok);
        CHECK_FALSE(ds::importiere(ordner.datei, QString(), {alleBereiche(), {}, {}}).ok);
    }
}

TEST_CASE("Datensicherung: die Einstellungen — nur was mitgehen darf, hin und zurueck")
{
    // die Liste (Datenablage_Konzept.md §4a)
    for (const char* mit : {"shortcuts/view.screenshot", "effectPalette/favorites", "effectPalette/recent",
                            "instrumentPalette/favorites", "instrumentPalette/recent",
                            "presets/chain/favorites", "presets/karaoke/favorites", "player/shuffle",
                            "composer/atEnd", "composer/muteProfile", "milkdrop/pufferWechsel", "editor/indentWidth",
                            "lyrics/onlineSearch", "karaoke/enabled", "loop/drumRightClickDeletes",
                            "configpanel/preview/equalizer/stage:1"})
    {
        CAPTURE(mit);
        CHECK(ds::einstellungGehtMit(QString::fromLatin1(mit)));
    }
    for (const char* ohne : {"shadertoy/apiKey", "DockManager/State", "panels/loop/geometry", "effectPalette/geometry",
                             "karaoke/trailSplitter", "loop/lastDir", "loop/lastFile", "composer/lastDir",
                             "effectPalette/presetGroupDir", "editor/shaderFileDir/isffilter", "import/imageSearchDir", "import/searchDirs",
                             "screenshot/baseDir", "player/resumeFile", "player/resumePositionMs",
                             "composer/session/file", "playlist/sessionIndex", "presets/chain/current",
                             "presets/chain/recent", "karaoke/currentPreset", "ui/lastVisualizer",
                             "data/bundledFromProjectFirst", "panels/import_browser/lastPreset",
                             "instrumentPalette/geometry", "etwas/neues"})
    {
        CAPTURE(ohne);
        CHECK_FALSE(ds::einstellungGehtMit(QString::fromLatin1(ohne)));
    }

    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    QSettings quelle(QDir(tmp.path()).filePath(QStringLiteral("quelle.ini")), QSettings::IniFormat);
    quelle.setValue(QStringLiteral("player/shuffle"), true);
    quelle.setValue(QStringLiteral("composer/newClipSeconds"), 12);
    quelle.setValue(QStringLiteral("composer/rowHeightPercent"), 87.5);
    quelle.setValue(QStringLiteral("composer/muteProfile"), QStringLiteral("Weich"));
    quelle.setValue(QStringLiteral("presets/chain/favorites"),
                    QStringList{QStringLiteral("D:/Presets/a.lvfx"), QStringLiteral("D:/Presets/b \u00fc.lvfx")});
    quelle.setValue(QStringLiteral("shortcuts/view.screenshot"), QStringLiteral("Ctrl+P"));
    quelle.setValue(QStringLiteral("shadertoy/apiKey"), QStringLiteral("GEHEIM"));
    quelle.setValue(QStringLiteral("loop/lastDir"), QStringLiteral("D:/Loops"));
    quelle.setValue(QStringLiteral("DockManager/State"), QByteArray("\x01\x02\x03", 3));

    const QJsonObject werte = ds::leseEinstellungen(quelle);
    CHECK(werte.size() == 6);
    CHECK_FALSE(werte.contains(QStringLiteral("shadertoy/apiKey")));
    CHECK_FALSE(werte.contains(QStringLiteral("loop/lastDir")));
    CHECK_FALSE(werte.contains(QStringLiteral("DockManager/State")));
    // der Schluessel steht auch nicht als Text im JSON
    CHECK_FALSE(QJsonDocument(werte).toJson().contains("GEHEIM"));

    // ein Import, in dem jemand mehr untergebracht hat, setzt trotzdem nur die Liste
    QJsonObject untergeschoben = werte;
    untergeschoben.insert(QStringLiteral("shadertoy/apiKey"), QStringLiteral("FREMD"));
    untergeschoben.insert(QStringLiteral("loop/lastDir"), QStringLiteral("X:/fremd"));
    QSettings ziel(QDir(tmp.path()).filePath(QStringLiteral("ziel.ini")), QSettings::IniFormat);
    ziel.setValue(QStringLiteral("shadertoy/apiKey"), QStringLiteral("MEINER"));
    ziel.setValue(QStringLiteral("player/shuffle"), false);
    ziel.setValue(QStringLiteral("player/repeatMode"), 2);  // steht nicht im Import: bleibt
    CHECK(ds::schreibeEinstellungen(ziel, untergeschoben) == 6);

    CHECK(ziel.value(QStringLiteral("player/shuffle")).toBool() == true);
    CHECK(ziel.value(QStringLiteral("composer/newClipSeconds")).toInt() == 12);
    CHECK(ziel.value(QStringLiteral("composer/rowHeightPercent")).toDouble() == doctest::Approx(87.5));
    CHECK(ziel.value(QStringLiteral("composer/muteProfile")).toString() == QStringLiteral("Weich"));
    CHECK(ziel.value(QStringLiteral("presets/chain/favorites")).toStringList() ==
          QStringList{QStringLiteral("D:/Presets/a.lvfx"), QStringLiteral("D:/Presets/b \u00fc.lvfx")});
    CHECK(ziel.value(QStringLiteral("shortcuts/view.screenshot")).toString() == QStringLiteral("Ctrl+P"));
    CHECK(ziel.value(QStringLiteral("shadertoy/apiKey")).toString() == QStringLiteral("MEINER"));
    CHECK_FALSE(ziel.contains(QStringLiteral("loop/lastDir")));
    CHECK(ziel.value(QStringLiteral("player/repeatMode")).toInt() == 2);
}

TEST_CASE("Datensicherung: Einstellungen im Archiv — nur wenn gewaehlt, und sie kommen wieder heraus")
{
    const Ordner ordner;
    fuelle(ordner.quelle);
    const QJsonObject werte{{QStringLiteral("player/shuffle"), true},
                            {QStringLiteral("composer/muteProfile"), QStringLiteral("Weich")}};

    REQUIRE(ds::exportiere(ordner.quelle, {{QStringLiteral("gradients")}, false}, werte, ordner.datei,
                           QStringLiteral("0.12.0"))
                .ok);
    CHECK_FALSE(ds::oeffne(ordner.datei, ordner.ziel).hatEinstellungen);

    REQUIRE(ds::exportiere(ordner.quelle, {{QStringLiteral("gradients")}, true}, werte, ordner.datei,
                           QStringLiteral("0.12.0"))
                .ok);
    const ds::Inhalt inhalt = ds::oeffne(ordner.datei, ordner.ziel);
    REQUIRE(inhalt.ok);
    CHECK(inhalt.hatEinstellungen);
    CHECK(inhalt.einstellungen == werte);
    CHECK(inhalt.ausgelassen == 0);

    // nur Einstellungen, kein Bereich: auch das ist eine Datei
    REQUIRE(ds::exportiere(ordner.quelle, {{}, true}, werte, ordner.datei, QStringLiteral("0.12.0")).ok);
    const ds::Inhalt nurEinstellungen = ds::oeffne(ordner.datei, ordner.ziel);
    REQUIRE(nurEinstellungen.ok);
    CHECK(nurEinstellungen.bereiche.empty());
    CHECK(nurEinstellungen.hatEinstellungen);
}
