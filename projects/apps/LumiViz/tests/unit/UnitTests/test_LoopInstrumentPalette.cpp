/**
 * @file test_LoopInstrumentPalette.cpp
 * @brief Loop: das Fenster „Instruments" und sein Weg in den Instrument-Editor
 *        (docs/audio/Loop_Konzept.md §4h, Stufe 14c) — ohne Ton und ohne
 *        gezeigtes Fenster; eigene Einstellungen, eigene Instrumente in einem
 *        Wegwerf-Ordner.
 *
 * `LUMIVIZ_INSTRUMENT_PALETTE_BILD=<datei.png>` legt ein Bild des Fensters ab.
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QTemporaryDir>
#include <QToolButton>
#include <QTreeWidget>

#include <memory>

using namespace lumi::loop;

namespace
{

/// Eigene Einstellungen und ein eigener Ordner fuer eigene Instrumente —
/// nach dem Fall ist alles wieder, wie es war.
struct Umgebung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    QTemporaryDir tmp;

    Umgebung()
    {
        REQUIRE(tmp.isValid());
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopInstrumentPalette"));
        QSettings().clear();
        setzeEigenenInstrumentOrdnerFuerTests(QDir(tmp.path()).filePath(QStringLiteral("instruments")));
    }
    ~Umgebung()
    {
        QSettings().clear();
        setzeEigenenInstrumentOrdnerFuerTests(QString());
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

/// Ein Loop mit zwei Instrument-Mustern: „Mel" (piano) und „Bass" (bass).
Loop probe()
{
    Loop loop;
    loop.laut = 1.0;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = QStringLiteral("piano");
    mel.noten = {{0, 4, 60, 1.0}};
    Muster bass = mel;
    bass.name = QStringLiteral("Bass");
    bass.instrument = QStringLiteral("bass");
    loop.muster = {mel, bass};
    loop.kanaele = {{QStringLiteral("K1"), true, {{QStringLiteral("Mel"), 0, 1}}},
                    {QStringLiteral("K2"), true, {{QStringLiteral("Bass"), 0, 1}}}};
    loop.richte();
    return loop;
}

QString kopfText(const InstrumentPalette& fenster, const QString& beginnt)
{
    const auto* baum = fenster.findChild<QTreeWidget*>(QStringLiteral("instrumentPaletteTree"));
    for (int i = 0; i < baum->topLevelItemCount(); ++i)
        if (baum->topLevelItem(i)->text(0).startsWith(beginnt)) return baum->topLevelItem(i)->text(0);
    return {};
}

}  // namespace

TEST_CASE("Loop Instruments-Fenster: der Baum nach Art — waehlen, anspielen, verwenden, Favoriten")
{
    Umgebung u;
    InstrumentPalette fenster;  // nicht gezeigt: kein Fenster
    QStringList toene;
    QObject::connect(&fenster, &InstrumentPalette::tonVerlangt, &fenster,
                     [&](const QString& nennung, int hoehe) { toene << QStringLiteral("%1 %2").arg(nennung).arg(hoehe); });
    QStringList verwendet;
    QObject::connect(&fenster, &InstrumentPalette::verwenden, &fenster,
                     [&](const QString& nennung) { verwendet << nennung; });

    CHECK(fenster.windowTitle() == QStringLiteral("Instruments"));

    // --- die acht Arten mit ihrer Zahl; Favorites und Recently used gibt es noch nicht;
    //     „My files" steht da und sagt, wie man es fuellt
    const QStringList zeilen = fenster.zeilen();
    CHECK(zeilen.first() == QStringLiteral("[Keys]"));
    CHECK_FALSE(zeilen.contains(QStringLiteral("[Favorites]")));
    CHECK_FALSE(zeilen.contains(QStringLiteral("[Recently used]")));
    CHECK(zeilen.last() == QStringLiteral("[My files]"));
    // (S123: aus Formen, aus Samples und Drums dazu — wer Instrumente dazulegt, stellt die Zahlen nach,
    // wie in test_LoopInstrument)
    CHECK(fenster.unter(QStringLiteral("Keys")).size() == 9);  // S133 (Stufe 23g): die alten als <name>_old
    CHECK(fenster.unter(QStringLiteral("Bass")).size() == 14);
    CHECK(fenster.unter(QStringLiteral("Lead")).size() == 7);
    CHECK(fenster.unter(QStringLiteral("Pads and strings")).size() == 9);
    CHECK(fenster.unter(QStringLiteral("Plucked")).size() == 17);
    CHECK(fenster.unter(QStringLiteral("Wind")).size() == 7);
    CHECK(fenster.unter(QStringLiteral("Percussion")).size() == 10);
    CHECK(fenster.unter(QStringLiteral("Drums")).size() == 20);  // Stufe 11f, S123, drei Besen S124
    CHECK(fenster.unter(QStringLiteral("Unusual")).size() == 8);
    CHECK(fenster.unter(QStringLiteral("My files")).isEmpty());
    CHECK(kopfText(fenster, QStringLiteral("Bass")) == QStringLiteral("Bass  (14)"));
    CHECK(kopfText(fenster, QStringLiteral("My files")) == QStringLiteral("My files"));
    // unter einer Art stehen sie nach Titel
    CHECK(fenster.unter(QStringLiteral("Wind")) ==
          QStringList{QStringLiteral("alto_flute"), QStringLiteral("alto_sax"), QStringLiteral("brass"),
                      QStringLiteral("shape_flower_flute"), QStringLiteral("flute"), QStringLiteral("sax"),
                      QStringLiteral("shakuhachi")});

    auto* titel = fenster.findChild<QLabel*>(QStringLiteral("instrumentPaletteTitle"));
    auto* satz = fenster.findChild<QLabel*>(QStringLiteral("instrumentPaletteDescription"));
    auto* ziel = fenster.findChild<QLabel*>(QStringLiteral("instrumentPaletteTarget"));
    auto* use = fenster.findChild<QPushButton*>(QStringLiteral("instrumentPaletteUse"));
    auto* play = fenster.findChild<QPushButton*>(QStringLiteral("instrumentPalettePlay"));
    auto* favorit = fenster.findChild<QToolButton*>(QStringLiteral("instrumentPaletteFavorite"));
    REQUIRE(titel != nullptr);
    REQUIRE(satz != nullptr);
    REQUIRE(ziel != nullptr);
    REQUIRE(use != nullptr);
    REQUIRE(play != nullptr);
    REQUIRE(favorit != nullptr);

    // --- das Fenster auf eine Art stellen (K3 a: aus einer Drum-Spur steht es auf Drums): ihr Kopf
    //     klappt auf; eine Art, unter der nichts steht, hat keinen Kopf
    {
        auto* baum = fenster.findChild<QTreeWidget*>(QStringLiteral("instrumentPaletteTree"));
        REQUIRE(baum != nullptr);
        QTreeWidgetItem* bass = nullptr;
        for (int i = 0; i < baum->topLevelItemCount(); ++i)
            if (baum->topLevelItem(i)->text(0).startsWith(QStringLiteral("Bass"))) bass = baum->topLevelItem(i);
        REQUIRE(bass != nullptr);
        bass->setExpanded(false);
        CHECK(fenster.zeigeArt(QStringLiteral("bass")));
        CHECK(bass->isExpanded());
        fenster.setzeSuche(QStringLiteral("gibt es ganz sicher nicht"));
        CHECK_FALSE(fenster.zeigeArt(QStringLiteral("bass")));
        fenster.setzeSuche(QString());
        CHECK(fenster.gewaehlt().isEmpty());  // stellen waehlt nichts
    }

    // --- nichts gewaehlt, kein Muster offen
    CHECK(fenster.gewaehlt().isEmpty());
    CHECK_FALSE(play->isEnabled());
    CHECK_FALSE(use->isEnabled());
    CHECK(ziel->text() == QStringLiteral("Open an instrument pattern to use an instrument"));
    CHECK_FALSE(fenster.spieleAn());
    CHECK_FALSE(fenster.verwende());

    // --- waehlen: unten Titel, Art, Genres und der Satz; ein Bass spielt tief
    CHECK_FALSE(fenster.waehle(QStringLiteral("tuba")));
    REQUIRE(fenster.waehle(QStringLiteral("slap_bass")));
    CHECK(fenster.gewaehlt() == QStringLiteral("slap_bass"));
    CHECK(titel->text() == QStringLiteral("<b>Slap Bass</b> · Bass · Funk"));
    CHECK(satz->text() == findeInstrument(QStringLiteral("slap_bass"))->beschreibung + QStringLiteral(" 3 layers."));
    CHECK(InstrumentPalette::hoeheFuerArt(QStringLiteral("bass")) == 36);
    CHECK(InstrumentPalette::hoeheFuerArt(QStringLiteral("keys")) == 60);
    CHECK(InstrumentPalette::hoeheFuerArt(QString()) == 60);
    CHECK(fenster.vorhoerHoehe() == 36);
    CHECK(play->isEnabled());
    CHECK_FALSE(use->isEnabled());  // weiter ohne Muster
    CHECK(toene.isEmpty());         // waehlen allein spielt nicht — das tut der Klick

    // --- anspielen: Play und die gewaehlte Tonhoehe; ein anderes Instrument stellt sie zurueck
    play->click();
    CHECK(toene == QStringList{QStringLiteral("slap_bass 36")});
    fenster.setzeVorhoerHoehe(48);
    REQUIRE(fenster.spieleAn());
    CHECK(toene.last() == QStringLiteral("slap_bass 48"));
    REQUIRE(fenster.waehle(QStringLiteral("piano")));
    CHECK(fenster.vorhoerHoehe() == 60);
    // ein Instrument ohne Genre: nur Art
    REQUIRE(fenster.waehle(QStringLiteral("guitar")));
    CHECK(titel->text() == QStringLiteral("<b>Guitar</b> · Plucked"));
    CHECK(satz->text().endsWith(QStringLiteral(" 1 layer.")));

    // --- verwenden: nur mit einem Muster; es rueckt in Recently used, die Wahl bleibt
    REQUIRE(fenster.waehle(QStringLiteral("piano")));
    fenster.setzeZiel(QStringLiteral("Mel"));
    CHECK(ziel->text() == QStringLiteral("For pattern: Mel"));
    CHECK(use->isEnabled());
    use->click();
    CHECK(verwendet == QStringList{QStringLiteral("piano")});
    CHECK(fenster.zuletzt() == QStringList{QStringLiteral("piano")});
    CHECK(fenster.unter(QStringLiteral("Recently used")) == QStringList{QStringLiteral("piano")});
    CHECK(fenster.gewaehlt() == QStringLiteral("piano"));
    REQUIRE(fenster.waehle(QStringLiteral("strings")));
    REQUIRE(fenster.verwende());
    REQUIRE(fenster.waehle(QStringLiteral("piano")));
    REQUIRE(fenster.verwende());  // noch einmal: rueckt nach vorn, steht nicht doppelt
    CHECK(fenster.zuletzt() == QStringList{QStringLiteral("piano"), QStringLiteral("strings")});

    // --- Favoriten
    CHECK_FALSE(favorit->isChecked());
    favorit->click();
    CHECK(fenster.istFavorit(QStringLiteral("piano")));
    CHECK(fenster.unter(QStringLiteral("Favorites")) == QStringList{QStringLiteral("piano")});
    CHECK(fenster.zeilen().first() == QStringLiteral("[Favorites]"));
    CHECK(favorit->isChecked());
    // Knopf und Ueberschrift tragen den Stern, wie in der Effect Palette (S116)
    CHECK(favorit->text() == QString::fromUtf8("★"));
    {
        auto* baum = fenster.findChild<QTreeWidget*>(QStringLiteral("instrumentPaletteTree"));
        REQUIRE(baum != nullptr);
        CHECK(baum->topLevelItem(0)->text(0) == QString::fromUtf8("★ Favorites  (1)"));
        CHECK(baum->topLevelItem(1)->text(0) == QStringLiteral("Recently used  (2)"));
    }
    REQUIRE(fenster.waehle(QStringLiteral("strings")));
    CHECK_FALSE(favorit->isChecked());
    fenster.setzeFavorit(true);
    CHECK(fenster.favoriten() == QStringList{QStringLiteral("piano"), QStringLiteral("strings")});
    fenster.setzeFavorit(false);
    CHECK(fenster.favoriten() == QStringList{QStringLiteral("piano")});

    // --- die App merkt beides: ein neues Fenster zeigt es wieder
    {
        InstrumentPalette zweites;
        CHECK(zweites.favoriten() == QStringList{QStringLiteral("piano")});
        CHECK(zweites.zuletzt() == QStringList{QStringLiteral("piano"), QStringLiteral("strings")});
        CHECK(zweites.unter(QStringLiteral("Favorites")) == QStringList{QStringLiteral("piano")});
    }

    // --- Recently used traegt hoechstens acht
    for (const char* name : {"bass", "guitar", "lead", "organ", "violin", "bell", "flute", "sax", "brass"})
    {
        REQUIRE(fenster.waehle(QString::fromLatin1(name)));
        REQUIRE(fenster.verwende());
    }
    CHECK(fenster.zuletzt().size() == InstrumentPalette::kZuletztMax);
    CHECK(fenster.zuletzt().first() == QStringLiteral("brass"));
    CHECK_FALSE(fenster.zuletzt().contains(QStringLiteral("piano")));  // das aelteste fiel heraus

    if (const QString bild = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_PALETTE_BILD"); !bild.isEmpty())
    {
        REQUIRE(fenster.waehle(QStringLiteral("slap_bass")));
        fenster.resize(440, 760);
        CHECK(fenster.grab().toImage().save(bild));
    }
}

TEST_CASE("Loop Instruments-Fenster: Suche und Genre — was nicht passt, verschwindet samt Kopf")
{
    Umgebung u;
    InstrumentPalette fenster;
    auto* genre = fenster.findChild<QComboBox*>(QStringLiteral("instrumentPaletteGenre"));
    REQUIRE(genre != nullptr);
    // „All" und die Genres, die die Instrumente tragen
    CHECK(genre->itemText(0) == QStringLiteral("All"));
    CHECK(genre->count() == bekannteGenres().size() + 1);
    CHECK(bekannteGenres().size() == 28);  // S123: Chillout, Electronica, Jazz, World aus make_showcase.py
    CHECK(bekannteGenres().contains(QStringLiteral("Funk")));

    // --- Genre: die vier aus dem Funk-Loop, je unter ihrer Art
    fenster.setzeGenre(QStringLiteral("Funk"));
    CHECK(fenster.zeilen() ==
          QStringList{QStringLiteral("[Keys]"), QStringLiteral("clavinet"), QStringLiteral("[Bass]"),
                      QStringLiteral("slap_bass"), QStringLiteral("[Plucked]"), QStringLiteral("muted_guitar"),
                      QStringLiteral("[Wind]"), QStringLiteral("brass")});
    CHECK(kopfText(fenster, QStringLiteral("Bass")) == QStringLiteral("Bass  (1)"));
    // ein weggefiltertes laesst sich nicht waehlen
    CHECK_FALSE(fenster.waehle(QStringLiteral("piano")));
    REQUIRE(fenster.waehle(QStringLiteral("slap_bass")));

    // --- dazu die Suche: beides muss passen
    fenster.setzeSuche(QStringLiteral("string"));
    CHECK(fenster.zeilen().contains(QStringLiteral("slap_bass")));   // „A plucked string …"
    CHECK(fenster.zeilen().contains(QStringLiteral("clavinet")));
    CHECK_FALSE(fenster.zeilen().contains(QStringLiteral("brass")));
    fenster.setzeSuche(QStringLiteral("brass"));
    CHECK(fenster.zeilen() == QStringList{QStringLiteral("[Wind]"), QStringLiteral("brass")});
    // das gewaehlte ist jetzt weggefiltert: nichts ist gewaehlt
    CHECK(fenster.gewaehlt().isEmpty());

    // --- nur die Suche: Titel, Art, Genre, Beschreibung; mehrere Worte muessen alle vorkommen
    fenster.setzeGenre(QString());
    fenster.setzeSuche(QStringLiteral("PLUCK"));
    const QStringList gezupft = fenster.zeilen();
    for (const char* da : {"pluck", "trance_pluck", "guitar", "nylon_guitar", "slap_bass", "upright_bass"})
    {
        CAPTURE(da);
        CHECK(gezupft.contains(QString::fromLatin1(da)));
    }
    CHECK_FALSE(gezupft.contains(QStringLiteral("piano")));
    CHECK_FALSE(gezupft.contains(QStringLiteral("[My files]")));  // gefiltert und leer: der Kopf ist weg
    fenster.setzeSuche(QStringLiteral("bass sub"));
    CHECK(fenster.zeilen().contains(QStringLiteral("sub_bass")));
    CHECK(fenster.zeilen().contains(QStringLiteral("reese_bass")));
    CHECK_FALSE(fenster.zeilen().contains(QStringLiteral("slap_bass")));
    fenster.setzeSuche(QStringLiteral("trance"));  // ein Genre als Suchwort
    CHECK(fenster.zeilen().contains(QStringLiteral("super_saw")));
    fenster.setzeSuche(QStringLiteral("gibt es nicht"));
    CHECK(fenster.zeilen().isEmpty());
    fenster.setzeSuche(QString());
    CHECK(fenster.unter(QStringLiteral("Bass")).size() == 14);
    CHECK(fenster.zeilen().last() == QStringLiteral("[My files]"));
}

TEST_CASE("Loop Instruments-Fenster: My files — die eigenen aus dem Benutzerordner, loeschen")
{
    Umgebung u;
    Instrument bass = *findeInstrument(QStringLiteral("slap_bass"));
    bass.titel = QStringLiteral("Mein Bass");
    bass.genres = {QStringLiteral("Funk"), QStringLiteral("Garage")};
    REQUIRE(speichereEigenes(bass, QStringLiteral("mein_bass")));

    InstrumentPalette fenster;
    CHECK(fenster.unter(QStringLiteral("My files")) == QStringList{QStringLiteral("user:mein_bass")});
    CHECK(kopfText(fenster, QStringLiteral("My files")) == QStringLiteral("My files  (1)"));
    // ein eigenes steht nur dort, nicht unter seiner Art — auch wenn es Bass ist
    CHECK(fenster.unter(QStringLiteral("Bass")).size() == 14);
    // sein Genre steht im Filter, und der Filter gilt auch fuer es
    auto* genre = fenster.findChild<QComboBox*>(QStringLiteral("instrumentPaletteGenre"));
    CHECK(genre->findData(QStringLiteral("Garage")) > 0);
    fenster.setzeGenre(QStringLiteral("Garage"));
    CHECK(fenster.zeilen() == QStringList{QStringLiteral("[My files]"), QStringLiteral("user:mein_bass")});
    fenster.setzeGenre(QString());

    REQUIRE(fenster.waehle(QStringLiteral("user:mein_bass")));
    CHECK(fenster.findChild<QLabel*>(QStringLiteral("instrumentPaletteTitle"))->text() ==
          QStringLiteral("<b>Mein Bass</b> · Bass · Funk, Garage · My files (mein_bass.lvinst)"));
    CHECK(fenster.vorhoerHoehe() == 36);
    fenster.setzeFavorit(true);
    fenster.setzeZiel(QStringLiteral("Mel"));
    REQUIRE(fenster.verwende());
    CHECK(fenster.unter(QStringLiteral("Favorites")) == QStringList{QStringLiteral("user:mein_bass")});

    // ein zweites kommt dazu, waehrend das Fenster offen ist
    Instrument zwei = bass;
    zwei.titel = QStringLiteral("Anderer");
    REQUIRE(speichereEigenes(zwei, QStringLiteral("anderer")));
    fenster.liesNeu();
    CHECK(fenster.unter(QStringLiteral("My files")) ==
          QStringList{QStringLiteral("user:anderer"), QStringLiteral("user:mein_bass")});
    CHECK(fenster.gewaehlt() == QStringLiteral("user:mein_bass"));  // die Wahl bleibt

    // --- loeschen: nur ein eigenes; es verschwindet auch aus Favorites und Recently used
    REQUIRE(fenster.waehle(QStringLiteral("piano")));
    CHECK_FALSE(fenster.loescheGewaehltes());
    REQUIRE(fenster.waehle(QStringLiteral("user:mein_bass")));
    REQUIRE(fenster.loescheGewaehltes());
    CHECK(fenster.unter(QStringLiteral("My files")) == QStringList{QStringLiteral("user:anderer")});
    CHECK(fenster.favoriten().isEmpty());
    CHECK(fenster.zuletzt().isEmpty());
    CHECK_FALSE(QFileInfo::exists(eigeneDatei(QStringLiteral("mein_bass"))));
    Instrument geholt;
    CHECK_FALSE(holeInstrument(QStringLiteral("user:mein_bass"), geholt));
}

TEST_CASE("Loop Instruments-Fenster im Editor: Browse oeffnet es fuer die Basis — Use setzt das Instrument")
{
    Umgebung u;
    InstrumentEditor editor;  // nicht gezeigt: kein Fenster
    Loop loop = probe();
    QStringList gemeldet;
    QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
        gemeldet << was;
        loop = neu;
        editor.setLoop(loop);  // wie das Panel: die Fassung gilt
    });
    QStringList toene;
    QObject::connect(&editor, &InstrumentEditor::instrumentTonVerlangt, &editor,
                     [&](const QString& nennung, int hoehe) { toene << QStringLiteral("%1 %2").arg(nennung).arg(hoehe); });

    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    auto* zeile = editor.findChild<QLabel*>(QStringLiteral("instrumentSound"));
    REQUIRE(zeile != nullptr);
    CHECK(zeile->text() == QStringLiteral("Piano"));
    CHECK(editor.palette() == nullptr);

    // --- Browse: das Fenster waehlt fuer diese Basis, ihr Instrument ist gewaehlt — ohne Ton
    editor.oeffnePalette(false);
    InstrumentPalette* fenster = editor.palette();
    REQUIRE(fenster != nullptr);
    CHECK(fenster->ziel() == QStringLiteral("Mel"));
    CHECK(fenster->gewaehlt() == QStringLiteral("piano"));
    CHECK(toene.isEmpty());

    // --- anspielen geht ueber den Editor hinaus; Use setzt das Instrument, ein Schritt fuer Undo
    REQUIRE(fenster->waehle(QStringLiteral("strings")));
    REQUIRE(fenster->spieleAn());
    CHECK(toene == QStringList{QStringLiteral("strings 60")});
    REQUIRE(fenster->verwende());
    CHECK(gemeldet == QStringList{QStringLiteral("Change instrument")});
    CHECK(loop.muster[0].instrument == QStringLiteral("strings"));
    CHECK(zeile->text() == QStringLiteral("Strings"));
    REQUIRE(fenster->verwende());  // dasselbe noch einmal: am Loop aendert sich nichts
    CHECK(gemeldet.size() == 1);

    // --- ein eigenes: in der Zeile steht „(My files)"
    Instrument bass = *findeInstrument(QStringLiteral("slap_bass"));
    bass.titel = QStringLiteral("Mein Bass");
    REQUIRE(speichereEigenes(bass, QStringLiteral("mein_bass")));
    fenster->liesNeu();
    REQUIRE(fenster->waehle(QStringLiteral("user:mein_bass")));
    REQUIRE(fenster->verwende());
    CHECK(loop.muster[0].instrument == QStringLiteral("user:mein_bass"));
    CHECK(zeile->text() == QStringLiteral("Mein Bass (My files)"));

    // --- der Editor zeigt eine andere Basis: das Fenster folgt
    REQUIRE(editor.zeige(loop, QStringLiteral("Bass")));
    CHECK(fenster->ziel() == QStringLiteral("Bass"));
    editor.oeffnePalette(false);
    CHECK(fenster->gewaehlt() == QStringLiteral("bass"));
    CHECK(editor.palette() == fenster);  // dasselbe Fenster, kein zweites
}

TEST_CASE("Loop Instruments-Fenster im Panel: Ctrl+Z und Ctrl+Y gelten im Fenster — Undo und Redo des Programms")
{
    // Befund Patrik S115 (Liste 10, P5): nach Use ist das Fenster das aktive, das
    // Kuerzel von Edit → Undo im Hauptfenster kam dort nicht an. Entscheid Z1 a.
    // Ohne gezeigtes Fenster: geprueft ist der Weg vom Kuerzel bis zum Loop, nicht
    // die Taste selbst.
    Umgebung u;
    ServiceContainer dienste;
    dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
    ICommandBus* bus = dienste.tryResolve<ICommandBus>();
    REQUIRE(bus != nullptr);

    LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
    panel.neu();
    panel.setzeLoop(probe());
    REQUIRE(panel.oeffneEditor(QStringLiteral("Mel"), false));
    InstrumentEditor* editor = panel.instrumentEditor();
    REQUIRE(editor != nullptr);
    editor->oeffnePalette(false);
    InstrumentPalette* fenster = editor->palette();
    REQUIRE(fenster != nullptr);
    auto* zeile = editor->findChild<QLabel*>(QStringLiteral("instrumentSound"));
    REQUIRE(zeile != nullptr);
    const auto instrument = [&] { return panel.loop().findeMuster(QStringLiteral("Mel"))->instrument; };

    REQUIRE(fenster->waehle(QStringLiteral("strings")));
    REQUIRE(fenster->verwende());
    CHECK(instrument() == QStringLiteral("strings"));
    CHECK(bus->undoDescription() == "Change instrument");

    // --- zurueck und wieder vor, vom Fenster aus
    fenster->nimmZurueck();
    CHECK(instrument() == QStringLiteral("piano"));
    CHECK(zeile->text() == QStringLiteral("Piano"));
    fenster->stelleWiederHer();
    CHECK(instrument() == QStringLiteral("strings"));
    CHECK(zeile->text() == QStringLiteral("Strings"));

    // --- die beiden Kuerzel am Fenster: Ctrl+Z und Ctrl+Y, sie loesen dasselbe aus
    auto* zurueck = fenster->findChild<QShortcut*>(QStringLiteral("instrumentPaletteUndo"));
    auto* vor = fenster->findChild<QShortcut*>(QStringLiteral("instrumentPaletteRedo"));
    REQUIRE(zurueck != nullptr);
    REQUIRE(vor != nullptr);
    CHECK(zurueck->keys().contains(QKeySequence(Qt::CTRL | Qt::Key_Z)));
    CHECK(vor->keys().contains(QKeySequence(Qt::CTRL | Qt::Key_Y)));
    Q_EMIT zurueck->activated();
    CHECK(instrument() == QStringLiteral("piano"));
    Q_EMIT vor->activated();
    CHECK(instrument() == QStringLiteral("strings"));

    // --- nichts mehr zurueckzunehmen: es geschieht nichts
    fenster->nimmZurueck();
    fenster->nimmZurueck();
    fenster->nimmZurueck();
    CHECK(instrument() == QStringLiteral("piano"));
}
