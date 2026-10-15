/**
 * @file test_LoopEditorLage.cpp
 * @brief Drum- und Instrument-Editor merken Groesse und Lage (S125, Offene_Punkte §6
 *        Loop) — in eigenen Einstellungen (`LumiVizTest` / `LoopEditorLage`), ohne
 *        gezeigtes Fenster.
 *
 * Fuer die uebrigen Tests ist das Merken abgeschaltet (die Zeile unten gilt fuer
 * die ganze Test-Exe): sonst hinge ihr Ausgang daran, wo die Editoren im Programm
 * zuletzt standen.
 */

#include <doctest.h>

#include "loop/DrumEditor.hpp"
#include "loop/InstrumentEditor.hpp"

#include <QApplication>
#include <QSettings>

using namespace lumi::loop;

namespace
{

const bool kLageNichtMerken = (DrumEditor::setzeLageMerken(false), InstrumentEditor::setzeLageMerken(false), true);

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopEditorLage"));
        QSettings().clear();
        DrumEditor::setzeLageMerken(true);
        InstrumentEditor::setzeLageMerken(true);
    }
    ~EigeneEinstellungen()
    {
        DrumEditor::setzeLageMerken(false);
        InstrumentEditor::setzeLageMerken(false);
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

}  // namespace

TEST_SUITE("Loop Editoren")
{
    TEST_CASE("Loop Editoren: Drum- und Instrument-Editor merken Groesse und Lage, jeder fuer sich")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        CHECK(kLageNichtMerken);
        EigeneEinstellungen einstellungen;

        {
            DrumEditor zuerst;  // nichts gemerkt: die Vorgabe
            CHECK(zuerst.size() == QSize(944, 330));
            InstrumentEditor auchZuerst;
            CHECK(auchZuerst.size() == QSize(1100, 560));
        }
        {
            DrumEditor drum;
            drum.resize(720, 410);
            drum.merkeLage();
            InstrumentEditor noten;
            noten.resize(980, 600);
            noten.merkeLage();
        }
        DrumEditor drum;
        CHECK(drum.size() == QSize(720, 410));
        InstrumentEditor noten;
        CHECK(noten.size() == QSize(980, 600));

        // abgeschaltet: weder gelesen noch gemerkt
        DrumEditor::setzeLageMerken(false);
        DrumEditor ohne;
        CHECK(ohne.size() == QSize(944, 330));
        ohne.resize(500, 300);
        ohne.merkeLage();
        DrumEditor::setzeLageMerken(true);
        DrumEditor wieder;
        CHECK(wieder.size() == QSize(720, 410));
    }
}
