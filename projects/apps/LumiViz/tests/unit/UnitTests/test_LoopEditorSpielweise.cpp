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
#include "UI/widgets/Bedienhilfe.hpp"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QTemporaryDir>

#include <optional>

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
    i.saiten.leer = {40, 45, 50, 55, 59, 64};
    i.saiten.wahl = Saitenwahl::Hand;
    i.saiten.mensurMm = 645.0;
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

TEST_CASE("Loop Instrument Editor Spielweise: I und K — Versatz der Akkordnoten, Kuerzel der Artikulationen")
{
    Anwendung anwendung;
    // K: die Kuerzel, in fester Reihenfolge
    CHECK(NotenRaster::artikulationsKuerzel(0).isEmpty());
    CHECK(NotenRaster::artikulationsKuerzel(kPalmMute) == QStringLiteral("PM"));
    CHECK(NotenRaster::artikulationsKuerzel(kHarmonic | kPalmMute) == QString::fromUtf8("PM ◇"));
    CHECK(NotenRaster::artikulationsKuerzel(kArtikulationenAlle) == QString::fromUtf8("PM LR T ◇"));

    // I: 4/4, vier Schritte je Schlag; Strum der Gitarre ohne die Up-Stelle: ↓ ↑ ↓ ↑ in Achteln, 8 ms
    Saitensatz saiten = gitarre().saiten;
    saiten.strum = Strum{};
    Muster mel;
    mel.art = MusterArt::Instrument;
    uebernimmVorgaben(mel, saiten);
    // ein Akkord E2 A2 D3 auf Schritt 0 (Down), derselbe auf Schritt 2 (Up), ein E2 allein auf 4
    mel.noten = {Note{0, 2, 40, 1.0, -1}, Note{0, 2, 45, 1.0, -1}, Note{0, 2, 50, 1.0, -1},
                 Note{2, 2, 40, 1.0, -1}, Note{2, 2, 45, 1.0, -1}, Note{2, 2, 50, 1.0, -1},
                 Note{4, 2, 40, 1.0, -1, kPalmMute}};
    NotenRaster raster;
    raster.setSaiten(saiten);
    raster.setTempo(120.0);
    raster.setMuster(mel, 4, 4);
    const std::vector<Anschlagplan> plan = raster.anschlaege();
    REQUIRE(plan.size() == 7);
    // Down: die tiefe Saite zuerst, je Saite 8 ms
    CHECK(plan[0].richtung == Richtung::Ab);
    CHECK(plan[0].versatzMs == doctest::Approx(0.0));
    CHECK(plan[1].versatzMs == doctest::Approx(8.0));
    CHECK(plan[2].versatzMs == doctest::Approx(16.0));
    // Up: die hohe zuerst
    CHECK(plan[3].richtung == Richtung::Auf);
    CHECK(plan[5].versatzMs == doctest::Approx(0.0));
    CHECK(plan[3].versatzMs == doctest::Approx(16.0));
    // ein Schritt dauert bei 120 bpm 125 ms und ist kSchrittBreite breit: 8 ms sind einen Pixel
    CHECK(raster.versatzPixel(125.0) == NotenRaster::kSchrittBreite);
    CHECK(raster.versatzPixel(8.0) == 1);
    CHECK(raster.versatzPixel(16.0) == 2);
    raster.setTempo(60.0);
    CHECK(raster.versatzPixel(16.0) == 1);

    // ohne Saiten: nichts versetzt
    raster.setSaiten(Saitensatz{});
    for (const Anschlagplan& a : raster.anschlaege()) CHECK(a.versatzMs == 0.0);

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_EDITOR_STRUM_BILD"); !ziel.isEmpty())
    {
        // ein Bild des Rasters allein, gross genug fuer die paar Pixel des Versatzes: langsam, Spread 30 ms
        NotenRaster gross;
        Saitensatz breit = saiten;
        breit.strum.spreadMs = 30.0;
        Muster bild = mel;
        uebernimmVorgaben(bild, breit);
        bild.noten.push_back(Note{8, 4, 52, 1.0, -1, kLetRing | kHarmonic});
        bild.noten.push_back(Note{12, 4, 57, 1.0, -1, kTapping});
        gross.setSaiten(breit);
        gross.setTempo(80.0);
        gross.setMuster(bild, 4, 4);
        gross.resize(NotenRaster::kTastenBreite + 18 * NotenRaster::kSchrittBreite,
                     NotenRaster::kKopfHoehe + 24 * NotenRaster::kZeilenHoehe + NotenRaster::kLautHoehe);
        gross.zeigeHoehe(48);
        QString datei = ziel;
        datei.replace(QStringLiteral(".png"), QStringLiteral("_raster.png"));
        CHECK(gross.grab().save(datei));
    }
}

TEST_CASE("Loop Instrument Editor Spielweise: J — die Spur Strum, ein Klick schaltet den Akkord weiter")
{
    Anwendung anwendung;
    Saitensatz saiten = gitarre().saiten;
    saiten.strum = Strum{};
    Muster mel;
    mel.art = MusterArt::Instrument;
    uebernimmVorgaben(mel, saiten);
    // Akkorde auf Schritt 0 (Hand: Down) und 2 (Hand: Up), ein E2 allein auf 4
    mel.noten = {Note{0, 2, 40, 1.0, -1}, Note{0, 2, 45, 1.0, -1}, Note{0, 2, 50, 1.0, -1},
                 Note{2, 2, 40, 1.0, -1}, Note{2, 2, 45, 1.0, -1}, Note{2, 2, 50, 1.0, -1}, Note{4, 2, 40, 1.0, -1}};

    NotenRaster raster;
    raster.setMuster(mel, 4, 4);
    raster.resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite,
                  NotenRaster::kKopfHoehe + 20 * NotenRaster::kZeilenHoehe + NotenRaster::kLautHoehe +
                      NotenRaster::kStrumHoehe);
    // ohne Saiten gibt es die Spur nicht
    CHECK(raster.strumHoehe() == 0);
    CHECK(raster.lautOben() == raster.height() - NotenRaster::kLautHoehe);
    CHECK_FALSE(raster.schalteAnschlag(0));
    raster.setSaiten(saiten);
    CHECK(raster.strumHoehe() == NotenRaster::kStrumHoehe);
    CHECK(raster.lautOben() == raster.strumOben() - NotenRaster::kLautHoehe);
    CHECK(raster.hoeheBei(raster.strumOben() + 3) == -1);

    QStringList gemeldet;
    QObject::connect(&raster, &NotenRaster::geaendert, &raster,
                     [&](const Muster&, const QString& was) { gemeldet << was; });
    // eine einzelne Note und ein leerer Schritt: kein Akkord
    CHECK_FALSE(raster.schalteAnschlag(4));
    CHECK_FALSE(raster.schalteAnschlag(1));
    CHECK(gemeldet.isEmpty());

    // Follow hand → Down → Up → Together → Follow hand
    const auto art = [&](int schritt) -> std::optional<Anschlagart> {
        const Anschlag* a = anschlagAn(raster.muster(), schritt);
        return a != nullptr ? std::optional<Anschlagart>(a->art) : std::nullopt;
    };
    CHECK_FALSE(art(2).has_value());
    CHECK(raster.anschlaege()[3].richtung == Richtung::Auf);
    REQUIRE(raster.schalteAnschlag(2));
    CHECK(art(2) == Anschlagart::Ab);
    REQUIRE(gemeldet.size() == 1);
    CHECK(gemeldet.back() == QStringLiteral("Strum: Down"));
    CHECK(raster.anschlaege()[3].richtung == Richtung::Ab);  // die Planung folgt dem Eintrag
    REQUIRE(raster.schalteAnschlag(2));
    CHECK(art(2) == Anschlagart::Auf);
    REQUIRE(raster.schalteAnschlag(2));
    CHECK(art(2) == Anschlagart::Zugleich);
    for (const int platz : {3, 4, 5}) CHECK(raster.anschlaege()[static_cast<std::size_t>(platz)].versatzMs == 0.0);
    REQUIRE(raster.schalteAnschlag(2));
    CHECK_FALSE(art(2).has_value());
    CHECK(raster.muster().anschlaege.empty());
    CHECK(gemeldet.back() == QStringLiteral("Strum: Follow hand"));

    // ein eigener Spread bleibt beim Weiterschalten
    Muster mitSpread = raster.muster();
    setzeAnschlag(mitSpread, Anschlag{0, Anschlagart::Hand, 25.0});
    raster.setMuster(mitSpread, 4, 4);
    REQUIRE(raster.schalteAnschlag(0));
    REQUIRE(anschlagAn(raster.muster(), 0) != nullptr);
    CHECK(anschlagAn(raster.muster(), 0)->art == Anschlagart::Ab);
    CHECK(anschlagAn(raster.muster(), 0)->spreadMs == 25.0);

    // mit der Maus: ein Klick in die Spur am Schritt 2
    const int vorher = static_cast<int>(gemeldet.size());
    const QPoint inSpur(raster.xVonSchritt(2) + NotenRaster::kSchrittBreite / 2, raster.strumOben() + 8);
    QMouseEvent druck(QEvent::MouseButtonPress, inSpur, raster.mapToGlobal(inSpur), Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);
    QCoreApplication::sendEvent(&raster, &druck);
    QMouseEvent los(QEvent::MouseButtonRelease, inSpur, raster.mapToGlobal(inSpur), Qt::LeftButton, Qt::NoButton,
                    Qt::NoModifier);
    QCoreApplication::sendEvent(&raster, &los);
    CHECK(static_cast<int>(gemeldet.size()) == vorher + 1);
    CHECK(art(2) == Anschlagart::Ab);
    CHECK(raster.muster().noten.size() == 7);  // keine Note gesetzt

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_EDITOR_STRUM_BILD"); !ziel.isEmpty())
    {
        raster.zeigeHoehe(46);
        QString datei = ziel;
        datei.replace(QStringLiteral(".png"), QStringLiteral("_spur.png"));
        CHECK(raster.grab().save(datei));
    }
}

TEST_CASE("Loop Instrument Editor Spielweise: L und M — das Menue Strum, die Haken der Artikulationen, die Hinweiszeile")
{
    Anwendung anwendung;
    Saitensatz saiten = gitarre().saiten;
    saiten.strum = Strum{};
    Muster mel;
    mel.art = MusterArt::Instrument;
    uebernimmVorgaben(mel, saiten);
    mel.noten = {Note{0, 2, 40, 1.0, -1}, Note{0, 2, 45, 1.0, -1}, Note{0, 2, 50, 1.0, -1},
                 Note{2, 2, 40, 1.0, -1}, Note{2, 2, 45, 1.0, -1}, Note{2, 2, 50, 1.0, -1}, Note{4, 2, 40, 1.0, -1}};
    NotenRaster raster;
    raster.setSaiten(saiten);
    raster.setMuster(mel, 4, 4);
    raster.resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite,
                  NotenRaster::kKopfHoehe + 20 * NotenRaster::kZeilenHoehe + NotenRaster::kLautHoehe +
                      NotenRaster::kStrumHoehe);
    raster.zeigeHoehe(45);
    QStringList gemeldet;
    QObject::connect(&raster, &NotenRaster::geaendert, &raster,
                     [&](const Muster&, const QString& was) { gemeldet << was; });

    // nichts markiert: nichts zu tun
    CHECK_FALSE(raster.setzeAnschlagart(Anschlagart::Auf));
    CHECK_FALSE(raster.schalteArtikulation(kPalmMute));

    // markiert: eine Note des ersten Akkords und die einzelne E2 — zwei Schritte
    raster.markiere({NotenOrt{0, 45}, NotenOrt{4, 40}});
    REQUIRE(raster.setzeAnschlagart(Anschlagart::Auf));
    CHECK(gemeldet.back() == QStringLiteral("Strum: Up"));
    REQUIRE(anschlagAn(raster.muster(), 0) != nullptr);
    CHECK(anschlagAn(raster.muster(), 0)->art == Anschlagart::Auf);
    REQUIRE(anschlagAn(raster.muster(), 4) != nullptr);  // auch die einzelne Note: so bekommt eine Melodie ihre Richtung
    CHECK(anschlagAn(raster.muster(), 2) == nullptr);
    // Spread: die Art bleibt; −1 = wie das Muster
    REQUIRE(raster.setzeAnschlagSpread(30.0));
    CHECK(anschlagAn(raster.muster(), 0)->spreadMs == 30.0);
    CHECK(anschlagAn(raster.muster(), 0)->art == Anschlagart::Auf);
    CHECK(gemeldet.back() == QStringLiteral("Spread: 30 ms"));
    CHECK(raster.anschlaege()[2].versatzMs == doctest::Approx(0.0));  // Up: D3 zuerst
    CHECK(raster.anschlaege()[0].versatzMs == doctest::Approx(60.0));

    // die Haken: alle markierten bekommen Palm mute, beim zweiten Mal geht es weg
    REQUIRE(raster.schalteArtikulation(kPalmMute));
    CHECK(gemeldet.back() == QStringLiteral("Palm mute on"));
    CHECK(raster.muster().noten[1].artikulationen == kPalmMute);
    CHECK(raster.muster().noten[6].artikulationen == kPalmMute);
    CHECK(raster.muster().noten[0].artikulationen == 0);  // nicht markiert
    REQUIRE(raster.schalteArtikulation(kPalmMute));
    CHECK(gemeldet.back() == QStringLiteral("Palm mute off"));
    CHECK(raster.muster().noten[1].artikulationen == 0);

    // mit dem Menue: Together aus dem Untermenue Strum, dann der Haken Harmonic
    QStringList strumEintraege;
    QStringList haken;
    const auto rechtsklick = [&] {
        const QPoint aufNote(raster.xVonSchritt(4) + 6, raster.yVonHoehe(40) + NotenRaster::kZeilenHoehe / 2);
        QContextMenuEvent rechts(QContextMenuEvent::Mouse, aufNote, raster.mapToGlobal(aufNote));
        QCoreApplication::sendEvent(&raster, &rechts);
    };
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        auto* strum = menue.findChild<QMenu*>(QStringLiteral("instrumentStrumMenu"));
        REQUIRE(strum != nullptr);
        QAction* together = nullptr;
        for (QAction* a : strum->actions())
        {
            if (a->isSeparator()) continue;
            strumEintraege << a->text() + (a->isChecked() ? QStringLiteral(" [x]") : QString());
            if (a->text() == QStringLiteral("Together")) together = a;
        }
        for (QAction* a : menue.actions())
            if (a->isCheckable()) haken << a->text();
        return together;
    });
    rechtsklick();
    MESSAGE("Menue Strum: " << strumEintraege.join(QStringLiteral(" | ")).toStdString());
    CHECK(strumEintraege == QStringList{QStringLiteral("Follow hand"), QStringLiteral("Down"), QStringLiteral("Up [x]"),
                                        QStringLiteral("Together"), QStringLiteral("Spread... (30 ms)")});
    CHECK(haken == QStringList{QStringLiteral("Palm mute"), QStringLiteral("Let ring"), QStringLiteral("Tapping"),
                               QStringLiteral("Harmonic")});
    CHECK(anschlagAn(raster.muster(), 0)->art == Anschlagart::Zugleich);
    CHECK(anschlagAn(raster.muster(), 0)->spreadMs == 30.0);
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        for (QAction* a : menue.actions())
            if (a->text() == QStringLiteral("Harmonic")) return a;
        return nullptr;
    });
    rechtsklick();
    CHECK(gemeldet.back() == QStringLiteral("Harmonic on"));
    CHECK(raster.muster().noten[6].artikulationen == kHarmonic);

    // Spread... ueber den Dialog: zurueck auf „wie das Muster"
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        for (QAction* a : menue.findChild<QMenu*>(QStringLiteral("instrumentStrumMenu"))->actions())
            if (a->text().startsWith(QStringLiteral("Spread"))) return a;
        return nullptr;
    });
    lumi::ui::setzeDialogBeobachterFuerTests([&](QDialog& dialog) {
        CHECK(dialog.objectName() == QStringLiteral("strumSpreadQuestion"));
        auto* wert = dialog.findChild<QDoubleSpinBox*>(QStringLiteral("strumSpreadValue"));
        REQUIRE(wert != nullptr);
        CHECK(wert->value() == 30.0);
        wert->setValue(wert->minimum());
        CHECK(wert->text().startsWith(QStringLiteral("Like the pattern")));
        return static_cast<int>(QDialog::Accepted);
    });
    rechtsklick();
    lumi::ui::setzeDialogBeobachterFuerTests({});
    lumi::ui::setzeMenueBeobachterFuerTests({});
    CHECK(gemeldet.back() == QStringLiteral("Spread: like the pattern"));
    CHECK(anschlagAn(raster.muster(), 0)->spreadMs == -1.0);
    CHECK(anschlagAn(raster.muster(), 0)->art == Anschlagart::Zugleich);

    // ohne Saiten: kein Strum und keine Haken im Menue
    raster.setSaiten(Saitensatz{});
    bool hatStrum = true;
    int hakenOhne = 0;
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        hatStrum = menue.findChild<QMenu*>(QStringLiteral("instrumentStrumMenu")) != nullptr;
        for (QAction* a : menue.actions())
            if (a->isCheckable()) ++hakenOhne;
        return nullptr;
    });
    raster.zeigeHoehe(45);
    rechtsklick();
    lumi::ui::setzeMenueBeobachterFuerTests({});
    CHECK_FALSE(hatStrum);
    CHECK(hakenOhne == 0);
    CHECK_FALSE(raster.setzeAnschlagart(Anschlagart::Ab));

    // M: die Hinweiszeile nennt Pfeile und Kuerzel
    InstrumentEditor editor;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString datei = QDir(ordner.path()).filePath(QStringLiteral("steel.lvinst"));
    REQUIRE(speichereInstrument(gitarre(), datei));
    REQUIRE(editor.zeige(probe(datei), QStringLiteral("Mel")));
    const QString hinweis = editor.findChild<QLabel*>(QStringLiteral("instrumentStringHint"))->text();
    CHECK(hinweis.contains(QStringLiteral("Strum lane")));
    CHECK(hinweis.contains(QStringLiteral("PM palm mute")));
    CHECK(hinweis.contains(QString::fromUtf8("◇ harmonic")));
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
