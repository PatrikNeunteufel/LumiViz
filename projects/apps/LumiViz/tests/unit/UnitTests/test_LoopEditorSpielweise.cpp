/**
 ****************************************************************************************
 * @file   test_LoopEditorSpielweise.cpp
 * @brief  Spielweisen, Stufe 24e: der Instrument-Editor zeigt Strum und Artikulationen
 *         (docs/audio/Saiten_Konzept.md §3 S-W5 a, S-W13 a; Skizze gebilligt, Patrik S134)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster; die Instrumente liegen als Dateien in einem
 * Wegwerf-Ordner. `LUMIVIZ_EDITOR_STRUM_BILD=<datei.png>` legt ein Bild des Editors ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"  // musterIn
#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"
#include "loop/Spielweise.hpp"
#include "loop/StrumZeile.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
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

/// Eine Gitarre mit eigener Strum-Vorgabe (Spread 12 ms, die erste Stelle Up)
/// und *Follow hand* als String choice.
Instrument gitarre()
{
    Instrument i;
    i.titel = QStringLiteral("Steel");
    i.saiten = Saitensatz{{40, 45, 50, 55, 59, 64}, 0, Saitenwahl::Hand, 645.0};
    i.saiten.strum.spreadMs = 12.0;
    i.saiten.strum.hand = {Richtung::Auf};
    i.schichten[0].schwingung = Schwingung::Saite;
    return i;
}

/// Ein Loop in 3/4 mit einem Muster „Mel" fuer dieses Instrument: ein Akkord E2 A2 D3
/// am Anfang, ein E2 allein auf Schritt 4.
Loop probe(const QString& instrument)
{
    Loop loop;
    loop.schlaegeJeTakt = 3;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = instrument;
    mel.noten = {Note{0, 4, 40, 1.0, -1}, Note{0, 4, 45, 1.0, -1}, Note{0, 4, 50, 1.0, -1}, Note{4, 2, 40, 1.0, -1}};
    loop.muster = {mel};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
    loop.richte();
    return loop;
}

}  // namespace

TEST_CASE("Loop Instrument Editor Spielweise: S-W13 a — Vorgaben beim Wechsel des Instruments, je Feld")
{
    const Saitensatz ohne;
    const Saitensatz steel = gitarre().saiten;
    Saitensatz nylon = steel;
    nylon.wahl = Saitenwahl::JeKlang;
    nylon.strum.spreadMs = 6.0;
    nylon.strum.hand = {};

    // eine neue Basis (ohne Saiten, leere Vorgaben) wird zur Gitarre mit deren Vorgaben
    Muster m;
    wechsleVorgaben(m, ohne, steel);
    CHECK(m.saitenwahl == Saitenwahl::Hand);
    CHECK(m.strum == steel.strum);
    // auf ein anderes Saiteninstrument: das Muster traegt die Vorgaben von Steel — es folgt
    wechsleVorgaben(m, steel, nylon);
    CHECK(m.saitenwahl == Saitenwahl::JeKlang);
    CHECK(m.strum == nylon.strum);

    // was am Muster geaendert wurde, bleibt — je Feld fuer sich
    Muster eigen;
    uebernimmVorgaben(eigen, steel);
    eigen.strum.spreadMs = 30.0;  // nur Strum geaendert
    wechsleVorgaben(eigen, steel, nylon);
    CHECK(eigen.strum.spreadMs == 30.0);
    CHECK(eigen.strum.hand == steel.strum.hand);
    CHECK(eigen.saitenwahl == Saitenwahl::JeKlang);  // String choice trug die Vorgabe: folgt

    // ueber ein Instrument ohne Saiten hinweg: die eigene Einstellung bleibt,
    // die Vorgabe wird zur leeren und dann zur naechsten Gitarre
    wechsleVorgaben(eigen, nylon, ohne);
    CHECK(eigen.strum.spreadMs == 30.0);
    CHECK(eigen.saitenwahl == ohne.wahl);
    wechsleVorgaben(eigen, ohne, steel);
    CHECK(eigen.strum.spreadMs == 30.0);
    CHECK(eigen.saitenwahl == Saitenwahl::Hand);

    // Anschlaege bleiben stehen
    Muster mitEintrag;
    mitEintrag.noten = {Note{0, 4, 40, 1.0, -1}};
    mitEintrag.anschlaege = {Anschlag{0, Anschlagart::Auf, -1.0}};
    wechsleVorgaben(mitEintrag, ohne, steel);
    CHECK(mitEintrag.anschlaege.size() == 1);
}

TEST_CASE("Loop Instrument Editor Spielweise: H — die Zeile Strum des Musters, im Raster des Loops")
{
    Anwendung anwendung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString steelDatei = QDir(ordner.path()).filePath(QStringLiteral("steel.lvinst"));
    REQUIRE(speichereInstrument(gitarre(), steelDatei));
    Instrument klavier;
    klavier.titel = QStringLiteral("Keys");
    const QString keysDatei = QDir(ordner.path()).filePath(QStringLiteral("keys.lvinst"));
    REQUIRE(speichereInstrument(klavier, keysDatei));

    InstrumentEditor editor;  // nicht gezeigt: kein Fenster
    Loop loop = probe(keysDatei);
    QStringList gemeldet;
    QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
        gemeldet << was;
        loop = neu;
        editor.setLoop(loop);
    });
    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    StrumZeile* zeile = editor.strumZeile();
    REQUIRE(zeile != nullptr);
    CHECK(editor.findChild<QComboBox*>(QStringLiteral("editorStrumHand")) != nullptr);  // die Teile heissen editorStrum…

    // ohne Saiten: keine Zeile Strum
    CHECK(zeile->isHidden());

    // S-W13 a: der Wechsel auf die Gitarre bringt ihre Vorgaben — in einem Schritt fuer Undo
    editor.setzeInstrument(steelDatei);
    CHECK(gemeldet == QStringList{QStringLiteral("Change instrument")});
    const auto mel = [&]() -> const Muster& { return *loop.findeMuster(QStringLiteral("Mel")); };
    CHECK(mel().saitenwahl == Saitenwahl::Hand);
    CHECK(mel().strum.spreadMs == 12.0);
    CHECK(mel().strum.hand == std::vector<Richtung>{Richtung::Auf});
    CHECK_FALSE(zeile->isHidden());
    CHECK(editor.findChild<QComboBox*>(QStringLiteral("instrumentStringChoice"))->currentData().toInt() ==
          static_cast<int>(Saitenwahl::Hand));

    // H: die Zeile zeigt Strum des Musters, im Raster des Loops: 3/4, 1/2 beat = sechs Stellen
    CHECK(zeile->strum() == mel().strum);
    CHECK(zeile->stellen() == 6);
    CHECK(zeile->weichtAb(0));  // die erste Stelle Up: hinterlegt
    CHECK_FALSE(zeile->weichtAb(1));

    // ein Klick auf ein Feld dreht es — am Muster, mit Undo
    zeile->dreheStelle(1);
    CHECK(gemeldet.back() == QStringLiteral("Change strum"));
    CHECK(mel().strum.hand == std::vector<Richtung>{Richtung::Auf, Richtung::Ab});
    CHECK(zeile->weichtAb(1));
    // Alternate setzt alle zurueck
    zeile->abwechselnd();
    CHECK(mel().strum.hand.empty());
    // Spread und Hand
    zeile->setzeSpread(20.0);
    CHECK(mel().strum.spreadMs == 20.0);
    zeile->setzeHandRaster(HandRaster::Schlag);
    CHECK(mel().strum.raster == HandRaster::Schlag);
    CHECK(zeile->stellen() == 3);
    const int schritte = static_cast<int>(gemeldet.size());

    // ein zweiter Wechsel: Strum ist geaendert und bleibt, String choice traegt die Vorgabe und folgt
    Instrument nylon = gitarre();
    nylon.titel = QStringLiteral("Nylon");
    nylon.saiten.wahl = Saitenwahl::JeKlang;
    nylon.saiten.strum = Strum{};
    const QString nylonDatei = QDir(ordner.path()).filePath(QStringLiteral("nylon.lvinst"));
    REQUIRE(speichereInstrument(nylon, nylonDatei));
    editor.setzeInstrument(nylonDatei);
    CHECK(static_cast<int>(gemeldet.size()) == schritte + 1);
    CHECK(mel().strum.spreadMs == 20.0);
    CHECK(mel().strum.raster == HandRaster::Schlag);
    CHECK(mel().saitenwahl == Saitenwahl::JeKlang);
    // dasselbe Instrument noch einmal: nichts
    editor.setzeInstrument(nylonDatei);
    CHECK(static_cast<int>(gemeldet.size()) == schritte + 1);

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_EDITOR_STRUM_BILD"); !ziel.isEmpty())
    {
        Loop bild = probe(steelDatei);
        uebernimmVorgaben(*musterIn(bild, QStringLiteral("Mel")), gitarre().saiten);
        InstrumentEditor gezeigt;
        REQUIRE(gezeigt.zeige(bild, QStringLiteral("Mel")));
        gezeigt.setAttribute(Qt::WA_DontShowOnScreen);
        gezeigt.resize(1280, 560);
        gezeigt.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        QCoreApplication::processEvents();
        gezeigt.raster()->zeigeHoehe(46);
        QCoreApplication::processEvents();
        CHECK(gezeigt.grab().save(ziel));
    }
}
