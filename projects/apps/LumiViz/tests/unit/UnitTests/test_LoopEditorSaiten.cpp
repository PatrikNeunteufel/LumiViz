/**
 ****************************************************************************************
 * @file   test_LoopEditorSaiten.cpp
 * @brief  Saiteninstrumente, Stufe 23f: der Instrument-Editor zeigt die Saiten
 *         (docs/audio/Saiten_Konzept.md §3; Skizze gebilligt, Patrik S133: S-U2 a, S-U2b a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster; das Instrument liegt als Datei in einem
 * Wegwerf-Ordner. Das Menue der Noten faengt `setzeMenueBeobachterFuerTests`.
 * `LUMIVIZ_EDITOR_SAITEN_BILD=<datei.png>` legt ein Bild des Editors ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"
#include "loop/SaitenZeile.hpp"
#include "UI/widgets/Bedienhilfe.hpp"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDir>
#include <QLabel>
#include <QMenu>
#include <QTemporaryDir>

using namespace lumi::loop;

namespace
{

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

/// Ein Loop mit einem Muster „Mel" fuer dieses Instrument: ein Akkord E2 A2 D3 am Anfang,
/// ein C2 (auf keiner Saite greifbar), ein E3 auf der D-Saite (selbst gesetzt), ein E2 allein.
Loop probe(const QString& instrument)
{
    Loop loop;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = instrument;
    mel.noten = {Note{0, 4, 40, 1.0, -1}, Note{0, 4, 45, 1.0, -1}, Note{0, 4, 50, 1.0, -1}, Note{4, 2, 36, 1.0, -1},
                 Note{8, 4, 52, 1.0, 2},  Note{12, 2, 40, 1.0, -1}};
    loop.muster = {mel};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
    loop.richte();
    return loop;
}

int platzVon(const Muster& muster, int schritt, int hoehe)
{
    for (std::size_t i = 0; i < muster.noten.size(); ++i)
        if (muster.noten[i].schritt == schritt && muster.noten[i].hoehe == hoehe) return static_cast<int>(i);
    return -1;
}

}  // namespace

TEST_CASE("Loop Instrument Editor Saiten: String choice, die Nummern, Nadel, nicht greifbar, das Menue String")
{
    Anwendung anwendung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    Instrument gitarre;
    gitarre.titel = QStringLiteral("Nylon");
    gitarre.saiten = Saitensatz{{40, 45, 50, 55, 59, 64}, 0, Saitenwahl::JeKlang, 650.0};
    gitarre.schichten[0].schwingung = Schwingung::Saite;
    const QString datei = QDir(ordner.path()).filePath(QStringLiteral("nylon.lvinst"));
    REQUIRE(speichereInstrument(gitarre, datei));

    InstrumentEditor editor;  // nicht gezeigt: kein Fenster
    Loop loop = probe(datei);
    QStringList gemeldet;
    QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
        gemeldet << was;
        loop = neu;
        editor.setLoop(loop);
    });
    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    NotenRaster* raster = editor.raster();
    auto* wahl = editor.findChild<QComboBox*>(QStringLiteral("instrumentStringChoice"));
    auto* hinweis = editor.findChild<QLabel*>(QStringLiteral("instrumentStringHint"));
    REQUIRE(wahl != nullptr);
    REQUIRE(hinweis != nullptr);

    // A und G: mit Saiten stehen String choice und die Hinweiszeile da
    CHECK_FALSE(wahl->isHidden());
    CHECK_FALSE(hinweis->isHidden());
    CHECK(wahl->currentText() == QStringLiteral("Per chord"));
    CHECK(raster->saiten().anzahl() == 6);

    // C: die Nummern — gezaehlt wie am Instrument, die hohe E-Saite ist 1 (S-U2b a)
    CHECK(NotenRaster::saitenNummer(raster->saiten(), 5) == 1);
    CHECK(NotenRaster::saitenNummer(raster->saiten(), 0) == 6);
    const Muster& mel = *loop.findeMuster(QStringLiteral("Mel"));
    std::vector<Griff> griffe = raster->griffe();
    REQUIRE(griffe.size() == mel.noten.size());
    // der Akkord E2 A2 D3: drei leere Saiten 6, 5, 4
    CHECK(NotenRaster::saitenNummer(raster->saiten(), griffe[static_cast<std::size_t>(platzVon(mel, 0, 40))].saite) == 6);
    CHECK(NotenRaster::saitenNummer(raster->saiten(), griffe[static_cast<std::size_t>(platzVon(mel, 0, 45))].saite) == 5);
    // E: C2 liegt unter der tiefsten Saite — nicht greifbar
    CHECK_FALSE(griffe[static_cast<std::size_t>(platzVon(mel, 4, 36))].greifbar);
    // D: E3 auf der D-Saite (Stelle 2), selbst gesetzt: Saite 4, Bund 2
    const Griff gesetzt = griffe[static_cast<std::size_t>(platzVon(mel, 8, 52))];
    CHECK(gesetzt.greifbar);
    CHECK(gesetzt.saite == 2);
    CHECK(gesetzt.bund == 2);

    // A: String choice stellt die Regel dieses Musters (mit Undo)
    wahl->setCurrentIndex(wahl->findData(static_cast<int>(Saitenwahl::Hand)));
    Q_EMIT wahl->activated(wahl->currentIndex());
    CHECK(loop.findeMuster(QStringLiteral("Mel"))->saitenwahl == Saitenwahl::Hand);
    CHECK(gemeldet.back() == QStringLiteral("Change string choice"));

    // F: das Menue String an der einzelnen E2 (Schritt 12)
    raster->resize(NotenRaster::kTastenBreite + 64 * NotenRaster::kSchrittBreite,
                   NotenRaster::kKopfHoehe + 20 * NotenRaster::kZeilenHoehe + NotenRaster::kLautHoehe);
    raster->zeigeHoehe(45);
    raster->markiere({NotenOrt{12, 40}});
    QStringList eintraege;
    bool autoGehakt = false;
    int gesperrt = 0;
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        auto* saite = menue.findChild<QMenu*>(QStringLiteral("instrumentStringMenu"));
        if (saite == nullptr) return nullptr;
        QAction* wahlE2 = nullptr;
        for (QAction* a : saite->actions())
        {
            eintraege << a->text();
            if (a->text().startsWith(QStringLiteral("Auto"))) autoGehakt = a->isChecked();
            if (!a->isEnabled()) ++gesperrt;
            if (a->text() == QString::fromUtf8("6 · E2")) wahlE2 = a;
        }
        return wahlE2;
    });
    const QPoint aufNote(raster->xVonSchritt(12) + 4, raster->yVonHoehe(40) + NotenRaster::kZeilenHoehe / 2);
    QContextMenuEvent rechts(QContextMenuEvent::Mouse, aufNote, raster->mapToGlobal(aufNote));
    QCoreApplication::sendEvent(raster, &rechts);
    lumi::ui::setzeMenueBeobachterFuerTests({});
    MESSAGE("Menue String: " << eintraege.join(QStringLiteral(" | ")).toStdString());
    REQUIRE(eintraege.size() == 7);
    CHECK(eintraege.front() == QStringLiteral("Auto (now 6)"));
    CHECK(autoGehakt);
    CHECK(eintraege[1] == QString::fromUtf8("1 · E4 (not playable)"));
    CHECK(gesperrt == 5);  // E2 geht nur auf der tiefen E-Saite
    // gewaehlt: Saite 6 — die Note traegt sie jetzt selbst
    const auto saiteDerE2 = [&] {
        const Muster& jetzt = *loop.findeMuster(QStringLiteral("Mel"));
        return jetzt.noten[static_cast<std::size_t>(platzVon(jetzt, 12, 40))].saite;
    };
    CHECK(saiteDerE2() == 0);
    CHECK(gemeldet.back() == QStringLiteral("String 6"));
    // und zurueck auf Auto
    CHECK(raster->setzeSaite(-1));
    CHECK(saiteDerE2() == -1);

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_EDITOR_SAITEN_BILD"); !ziel.isEmpty())
    {
        InstrumentEditor gezeigt;
        REQUIRE(gezeigt.zeige(probe(datei), QStringLiteral("Mel")));
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.resize(1280, 520);
        gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        QCoreApplication::processEvents();
        gezeigt.raster()->zeigeHoehe(46);
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
    }
}

TEST_CASE("Loop Instrument Editor Saiten: ohne Saiten bleibt alles wie bisher — und die Zeile Tuning nennt die Saiten")
{
    Anwendung anwendung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    Instrument klavier;
    klavier.titel = QStringLiteral("Keys");
    const QString datei = QDir(ordner.path()).filePath(QStringLiteral("keys.lvinst"));
    REQUIRE(speichereInstrument(klavier, datei));

    InstrumentEditor editor;
    REQUIRE(editor.zeige(probe(datei), QStringLiteral("Mel")));
    CHECK(editor.findChild<QComboBox*>(QStringLiteral("instrumentStringChoice"))->isHidden());
    CHECK(editor.findChild<QLabel*>(QStringLiteral("instrumentStringHint"))->isHidden());
    CHECK_FALSE(editor.raster()->saiten().istSaiteninstrument());
    // im Menue kein String
    NotenRaster* raster = editor.raster();
    raster->resize(NotenRaster::kTastenBreite + 64 * NotenRaster::kSchrittBreite,
                   NotenRaster::kKopfHoehe + 20 * NotenRaster::kZeilenHoehe + NotenRaster::kLautHoehe);
    raster->zeigeHoehe(45);
    raster->markiere({NotenOrt{12, 40}});
    bool hatString = true;
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        hatString = menue.findChild<QMenu*>(QStringLiteral("instrumentStringMenu")) != nullptr;
        return nullptr;
    });
    const QPoint aufNote(raster->xVonSchritt(12) + 4, raster->yVonHoehe(40) + NotenRaster::kZeilenHoehe / 2);
    QContextMenuEvent rechts(QContextMenuEvent::Mouse, aufNote, raster->mapToGlobal(aufNote));
    QCoreApplication::sendEvent(raster, &rechts);
    lumi::ui::setzeMenueBeobachterFuerTests({});
    CHECK_FALSE(hatString);
    CHECK_FALSE(raster->setzeSaite(0));

    // die Zeile Tuning: jedes Feld nennt seine Nummer, die erste Saite der Liste ist die hoechste Nummer
    SaitenZeile zeile;
    zeile.zeige(Saitensatz{{40, 45, 50, 55, 59, 64}, 0, Saitenwahl::JeKlang, 648.0});
    CHECK(zeile.findChild<TonFeld*>(QStringLiteral("creatorString1"))->toolTip().startsWith(QStringLiteral("String 6")));
    CHECK(zeile.findChild<TonFeld*>(QStringLiteral("creatorString6"))->toolTip().startsWith(QStringLiteral("String 1")));
}
