/**
 ****************************************************************************************
 * @file   test_LoopEditorVerbunden.cpp
 * @brief  Tabs und Akkorde, Stufe 26d: Hammer-on, Pull-off, Slide und Bend im
 *         Instrument-Editor setzen und zeigen (docs/audio/Saiten_Konzept.md S-L3 a A–G,
 *         S-T5 a C, E; Skizzen gebilligt, Patrik S136/S137)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne gezeigtes Fenster; Menue und Dialog ueber die Bedienhilfe abgefangen.
 * Bedient wird ueber den Editor: die Aenderung kommt im Loop an. Die Vorlagen und die
 * Schrift der Hoehe ohne Qt-Anwendung. `LUMIVIZ_EDITOR_VERBUNDEN_BILD=<datei.png>` legt ein
 * Bild des Editors ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/BendKurve.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"
#include "loop/Spielweise.hpp"
#include "UI/widgets/Bedienhilfe.hpp"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QMenu>
#include <QMouseEvent>
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

Instrument gitarre()
{
    Instrument i;
    i.titel = QStringLiteral("Steel");
    i.saiten.leer = {40, 45, 50, 55, 59, 64};
    i.schichten[0].schwingung = Schwingung::Saite;
    return i;
}

/// „Mel": A3 auf 0, B3 auf 2, C4 auf 4, G3 auf 8 (alle zwei Schritte lang).
Loop probe(const QString& instrument)
{
    Loop loop;
    Muster mel;
    mel.name = QStringLiteral("Mel");
    mel.art = MusterArt::Instrument;
    mel.instrument = instrument;
    mel.noten = {Note{0, 2, 57, 1.0, -1}, Note{2, 2, 59, 1.0, -1}, Note{4, 2, 60, 1.0, -1}, Note{8, 4, 55, 1.0, -1}};
    loop.muster = {mel};
    loop.kanaele = {{QStringLiteral("Lead"), true, {{QStringLiteral("Mel"), 0, 1}}}};
    loop.richte();
    return loop;
}

int platzVon(const Muster& m, int schritt)
{
    for (std::size_t i = 0; i < m.noten.size(); ++i)
        if (m.noten[i].schritt == schritt) return static_cast<int>(i);
    return -1;
}

/// Im Menue (und seinen Untermenues) die Aktion mit diesem Text.
QAction* finde(QMenu& menue, const QString& text)
{
    for (QAction* a : menue.actions())
    {
        if (a->text() == text) return a;
        if (a->menu() != nullptr)
            if (QAction* tiefer = finde(*a->menu(), text)) return tiefer;
    }
    return nullptr;
}

}  // namespace

TEST_CASE("Loop Editor Verbunden 26d: Vorlagen des Bends und die Schrift seiner Hoehe")
{
    CHECK(bendText(1.0) == QStringLiteral("½"));
    CHECK(bendText(2.0) == QStringLiteral("full"));
    CHECK(bendText(3.0) == QStringLiteral("1½"));
    CHECK(bendText(0.5) == QStringLiteral("¼"));
    CHECK(bendText(2.5) == QStringLiteral("1¼"));
    CHECK(bendText(0.0) == QStringLiteral("0"));

    CHECK(bendVorlage(BendVorlage::Keine, 2.0).empty());
    CHECK(bendVorlage(BendVorlage::Bend, 2.0) == std::vector<BendPunkt>{{0.0, 0.0}, {0.3, 2.0}});
    CHECK(bendVorlage(BendVorlage::BendZurueck, 1.0) == std::vector<BendPunkt>{{0.0, 0.0}, {0.3, 1.0}, {0.6, 1.0}, {0.9, 0.0}});
    CHECK(bendVorlage(BendVorlage::VorBend, 3.0) == std::vector<BendPunkt>{{0.0, 3.0}});
    CHECK(bendVorlage(BendVorlage::VorBendZurueck, 2.0) == std::vector<BendPunkt>{{0.0, 2.0}, {0.5, 2.0}, {0.8, 0.0}});
    CHECK(bendVorlage(BendVorlage::Bend, 9.0) == std::vector<BendPunkt>{{0.0, 0.0}, {0.3, kBendHalbtoeneMax}});  // die Grenze
    for (const BendVorlage art : {BendVorlage::Keine, BendVorlage::Bend, BendVorlage::BendZurueck, BendVorlage::VorBend,
                                  BendVorlage::VorBendZurueck})
        for (const double h : {1.0, 2.0, 3.0}) CHECK(bendVorlageVon(bendVorlage(art, h)) == art);
    CHECK_FALSE(bendVorlageVon({{0.0, 0.0}, {0.5, 1.0}, {1.0, 0.5}}).has_value());  // eine eigene Form
    CHECK(bendHoehe({{0.0, 0.5}, {0.5, 2.5}}) == 2.5);
}

TEST_CASE("Loop Editor Verbunden 26d: ueber den Editor — Legato und Slide im Menue, Bend mit Vorlage, Hoehe, Edit bend...")
{
    Anwendung anwendung;
    QTemporaryDir ordner;
    REQUIRE(ordner.isValid());
    const QString datei = QDir(ordner.path()).filePath(QStringLiteral("steel.lvinst"));
    REQUIRE(speichereInstrument(gitarre(), datei));
    InstrumentEditor editor;
    Loop loop = probe(datei);
    QStringList gemeldet;
    QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
        gemeldet << was;
        loop = neu;
        editor.setLoop(loop);
    });
    REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
    NotenRaster* raster = editor.raster();
    raster->resize(NotenRaster::kTastenBreite + 32 * NotenRaster::kSchrittBreite, 600);
    raster->zeigeHoehe(58);
    const auto mel = [&]() -> const Muster& { return *loop.findeMuster(QStringLiteral("Mel")); };
    const auto rechtsklickAuf = [&](int schritt) {
        const Note& note = raster->muster().noten[static_cast<std::size_t>(platzVon(raster->muster(), schritt))];
        const QPoint ort(raster->xVonSchritt(schritt) + 6, raster->yVonHoehe(note.hoehe) + NotenRaster::kZeilenHoehe / 2);
        raster->markiere({NotenOrt{note.schritt, note.hoehe}});
        QContextMenuEvent rechts(QContextMenuEvent::Mouse, ort, raster->mapToGlobal(ort));
        QCoreApplication::sendEvent(raster, &rechts);
    };
    const auto waehle = [&](const QString& text) {
        lumi::ui::setzeMenueBeobachterFuerTests([text](QMenu& menue) -> QAction* { return finde(menue, text); });
    };

    // E: unter den vier Haken zwei neue — Legato und Slide
    QStringList haken;
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        for (QAction* a : menue.actions())
            if (a->isCheckable()) haken << a->text();
        return nullptr;
    });
    rechtsklickAuf(2);
    CHECK(haken == QStringList{QStringLiteral("Palm mute"), QStringLiteral("Let ring"), QStringLiteral("Tapping"),
                               QStringLiteral("Harmonic"), QStringLiteral("Legato (hammer-on / pull-off)"), QStringLiteral("Slide")});

    // B3 auf 2 per Legato von A3: ein Hammer-on — im Loop, und die Planung kennt die vorige Note
    waehle(QStringLiteral("Legato (hammer-on / pull-off)"));
    rechtsklickAuf(2);
    REQUIRE(!gemeldet.isEmpty());
    CHECK(gemeldet.back() == QStringLiteral("Legato (hammer-on / pull-off) on"));
    CHECK((mel().noten[static_cast<std::size_t>(platzVon(mel(), 2))].artikulationen & kLegato) != 0);
    CHECK(raster->griffe()[static_cast<std::size_t>(platzVon(raster->muster(), 2))].vorige == platzVon(raster->muster(), 0));
    // Slide schliesst Legato aus
    waehle(QStringLiteral("Slide"));
    rechtsklickAuf(2);
    const int b3 = platzVon(mel(), 2);
    CHECK((mel().noten[static_cast<std::size_t>(b3)].artikulationen & kSlide) != 0);
    CHECK((mel().noten[static_cast<std::size_t>(b3)].artikulationen & kLegato) == 0);
    // G3 auf 8 per Legato: ein Pull-off von der vorigen Note auf der G-Saite — das ist B3 (4. Bund);
    // C4 dazwischen liegt nach Per chord auf der H-Saite (1. Bund)
    waehle(QStringLiteral("Legato (hammer-on / pull-off)"));
    rechtsklickAuf(8);
    CHECK((mel().noten[static_cast<std::size_t>(platzVon(mel(), 8))].artikulationen & kLegato) != 0);
    CHECK(raster->griffe()[static_cast<std::size_t>(platzVon(raster->muster(), 8))].vorige == platzVon(raster->muster(), 2));
    CHECK_FALSE(raster->griffe()[static_cast<std::size_t>(platzVon(raster->muster(), 8))].verbindungFehlt);

    // F: Bend and release an A3, dann die Hoehe 1½, dann keiner
    waehle(QStringLiteral("Bend and release"));
    rechtsklickAuf(0);
    CHECK(gemeldet.back() == QStringLiteral("Bend: Bend and release"));
    CHECK(mel().noten[static_cast<std::size_t>(platzVon(mel(), 0))].bend == bendVorlage(BendVorlage::BendZurueck, 2.0));
    waehle(QStringLiteral("Height 1½"));
    rechtsklickAuf(0);
    CHECK(mel().noten[static_cast<std::size_t>(platzVon(mel(), 0))].bend == bendVorlage(BendVorlage::BendZurueck, 3.0));
    // das Menue zeigt, was gilt
    QStringList bendEintraege;
    lumi::ui::setzeMenueBeobachterFuerTests([&](QMenu& menue) -> QAction* {
        auto* bend = menue.findChild<QMenu*>(QStringLiteral("instrumentBendMenu"));
        REQUIRE(bend != nullptr);
        for (QAction* a : bend->actions())
            if (!a->isSeparator()) bendEintraege << a->text() + (a->isChecked() ? QStringLiteral(" [x]") : QString());
        return nullptr;
    });
    rechtsklickAuf(0);
    CHECK(bendEintraege == QStringList{QStringLiteral("None"), QStringLiteral("Bend"), QStringLiteral("Bend and release [x]"),
                                       QStringLiteral("Pre-bend"), QStringLiteral("Pre-bend and release"),
                                       QStringLiteral("Height ½"), QStringLiteral("Height full"),
                                       QStringLiteral("Height 1½ [x]"), QStringLiteral("Edit bend...")});
    waehle(QStringLiteral("None"));
    rechtsklickAuf(0);
    CHECK(mel().noten[static_cast<std::size_t>(platzVon(mel(), 0))].bend.empty());
    CHECK(gemeldet.back() == QStringLiteral("No bend"));

    // G: Edit bend... — Pre-bend aus der Vorlage, dazu per Doppelklick ein Punkt in der Mitte
    waehle(QStringLiteral("Edit bend..."));
    bool gesehen = false;
    lumi::ui::setzeDialogBeobachterFuerTests([&](QDialog& dialog) {
        gesehen = dialog.objectName() == QStringLiteral("bendDialog");
        auto* kurve = dialog.findChild<BendKurve*>(QStringLiteral("bendCurve"));
        auto* vorlage = dialog.findChild<QComboBox*>(QStringLiteral("bendTemplate"));
        auto* hoehe = dialog.findChild<QComboBox*>(QStringLiteral("bendHeight"));
        REQUIRE(kurve != nullptr);
        REQUIRE(vorlage != nullptr);
        REQUIRE(hoehe != nullptr);
        CHECK(vorlage->currentText() == QStringLiteral("None"));  // ohne Bend
        CHECK(hoehe->currentText() == QStringLiteral("full"));
        vorlage->setCurrentIndex(vorlage->findText(QStringLiteral("Pre-bend")));
        Q_EMIT vorlage->activated(vorlage->currentIndex());
        CHECK(kurve->punkte() == bendVorlage(BendVorlage::VorBend, 2.0));
        kurve->resize(400, 200);
        const QPointF mitte = kurve->ortVon({0.5, 1.0});
        QMouseEvent doppelt(QEvent::MouseButtonDblClick, mitte, kurve->mapToGlobal(mitte), Qt::LeftButton, Qt::LeftButton,
                            Qt::NoModifier);
        QCoreApplication::sendEvent(kurve, &doppelt);
        CHECK(kurve->punkte().size() == 2);
        CHECK(vorlage->currentIndex() == -1);  // eine eigene Form: keine Vorlage mehr
        return static_cast<int>(QDialog::Accepted);
    });
    rechtsklickAuf(0);
    lumi::ui::setzeDialogBeobachterFuerTests({});
    lumi::ui::setzeMenueBeobachterFuerTests({});
    CHECK(gesehen);
    const std::vector<BendPunkt> gesetzt = mel().noten[static_cast<std::size_t>(platzVon(mel(), 0))].bend;
    REQUIRE(gesetzt.size() == 2);
    CHECK(gesetzt[0] == BendPunkt{0.0, 2.0});
    CHECK(gesetzt[1].zeit == doctest::Approx(0.5).epsilon(0.02));
    CHECK(gesetzt[1].halbtoene == 1.0);
    CHECK(gemeldet.back() == QStringLiteral("Edit bend"));

    // in den Tab-Linien: ein Rechtsklick auf eine Zahl markiert ihre Note und oeffnet dasselbe Menue
    lumi::ui::setzeMenueBeobachterFuerTests([](QMenu& menue) -> QAction* {
        // die Vorlage „Bend" im Untermenue (das Untermenue selbst heisst auch so)
        auto* bend = menue.findChild<QMenu*>(QStringLiteral("instrumentBendMenu"));
        return bend != nullptr ? finde(*bend, QStringLiteral("Bend")) : nullptr;
    });
    raster->markiere({});
    const int g3 = platzVon(raster->muster(), 8);
    const QPoint zahl(raster->xVonSchritt(8) + 4, raster->tabLinieY(raster->griffe()[static_cast<std::size_t>(g3)].saite));
    REQUIRE(raster->tabNoteBei(zahl) == g3);
    QContextMenuEvent rechts(QContextMenuEvent::Mouse, zahl, raster->mapToGlobal(zahl));
    QCoreApplication::sendEvent(raster, &rechts);
    lumi::ui::setzeMenueBeobachterFuerTests({});
    CHECK(mel().noten[static_cast<std::size_t>(platzVon(mel(), 8))].bend == bendVorlage(BendVorlage::Bend, 2.0));

    if (const QString ziel = qEnvironmentVariable("LUMIVIZ_EDITOR_VERBUNDEN_BILD"); !ziel.isEmpty())
    {
        editor.setAttribute(Qt::WA_DontShowOnScreen);
        editor.resize(1280, 820);
        editor.show();  // das Layout steht erst nach dem Zeigen; auf den Bildschirm kommt nichts
        raster->zeigeHoehe(59);
        QCoreApplication::processEvents();
        CHECK(editor.grab().save(ziel));
    }
}
