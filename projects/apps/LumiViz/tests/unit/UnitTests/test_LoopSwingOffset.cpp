/**
 * @file test_LoopSwingOffset.cpp
 * @brief Loop, Stufe 15a: Raster und Swing im Kopf des Loop-Panels, der Swing offset je
 *        Muster (docs/audio/Loop_Konzept.md §4p; Entscheide Patrik RS1 a, RS1-1 a,
 *        Skizze SK15a-1 a, S168)
 *
 * - Die Datei (Fassung 15): `swingOffset` an jedem Muster, immer geschrieben; fehlt es,
 *   gilt 0. `richte` begrenzt auf −1 … 1.
 * - `swingVon`: Loop-Swing + Offset auf 0 … 1, ohne Offset genau der Swing des Loops.
 * - Der Rechenkern verzoegert jeden zweiten Schritt um den Swing **seines Musters** — an
 *   Drum-Schritten und Noten; ein Muster mit Offset 0 spielt Bit fuer Bit wie bisher.
 * - Das Panel: *Beats per bar*, *Steps per beat* und *Swing* neben BPM, mit Undo.
 * - Der Instrument-Editor: die Zeile mit Raster und Swing offset wie im Drum-Editor
 *   (dort in test_LoopDrumEditor.cpp).
 *
 * Ohne Ton und ohne Fenster. Bei 44100 Hz, 120 BPM und vier Schritten je Schlag ist ein
 * Schritt 5512,5 Abtastwerte lang.
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/RasterZeile.hpp"
#include "loop/Rechenkern.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>

#include <cstdint>
#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Zwei Drum-Muster in zwei Kanaelen, je eine Spur mit Schlaegen auf den ersten vier
/// Schritten; `offset` an Muster B.
Loop zweiDrums(double swing, double offset)
{
    Loop loop;
    loop.laut = 1.0;
    loop.schlaegeJeTakt = 1;
    loop.swing = swing;
    Muster a;
    a.name = QStringLiteral("A");
    a.spuren = {{QStringLiteral("X"), QStringLiteral("a.wav"), 1.0, 0.0, false}};
    a.zeilen = {{1, 1, 1, 1}};
    Muster b = a;
    b.name = QStringLiteral("B");
    b.spuren[0].klang = QStringLiteral("b.wav");
    b.swingOffset = offset;
    loop.muster = {a, b};
    loop.kanaele = {{QStringLiteral("K1"), true, {{QStringLiteral("A"), 0, 1}}},
                    {QStringLiteral("K2"), true, {{QStringLiteral("B"), 0, 1}}}};
    loop.richte();
    return loop;
}

/// Die Schlaege der Stimme, die `klang` spielt.
const std::vector<Schlag>& schlaegeVon(const Ablauf& ablauf, const QString& klang)
{
    const int nr = static_cast<int>(ablauf.klangNamen.indexOf(klang));
    REQUIRE(nr >= 0);
    for (const Stimme& s : ablauf.stimmen)
        if (s.klang == nr) return s.schlaege;
    FAIL("keine Stimme fuer " << klang.toStdString());
    static const std::vector<Schlag> kLeer;
    return kLeer;
}

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopSwingOffset"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

}  // namespace

TEST_SUITE("LoopSwingOffset")
{
    TEST_CASE("Loop Swing offset 15a: Datei, Grenzen und swingVon")
    {
        Loop loop = zweiDrums(0.3, -0.1);

        SUBCASE("immer geschrieben, seit Fassung 15, und wieder gelesen")
        {
            const QJsonObject json = alsJson(loop);
            CHECK(json.value(QStringLiteral("version")).toInt() == 17);
            const QJsonArray muster = json.value(QStringLiteral("patterns")).toArray();
            REQUIRE(muster.size() == 2);
            CHECK(muster[0].toObject().contains(QStringLiteral("swingOffset")));  // auch 0 steht da
            CHECK(muster[0].toObject().value(QStringLiteral("swingOffset")).toDouble() == 0.0);
            CHECK(muster[1].toObject().value(QStringLiteral("swingOffset")).toDouble() == doctest::Approx(-0.1));
            QStringList bericht;
            Loop zurueck;
            REQUIRE(ausJson(json, zurueck, {}, &bericht));
            CHECK(zurueck == loop);
        }
        SUBCASE("fehlt das Feld (bis Fassung 14): wie der Loop")
        {
            QJsonObject json = alsJson(loop);
            json[QStringLiteral("version")] = 14;
            QJsonArray muster = json.value(QStringLiteral("patterns")).toArray();
            for (qsizetype i = 0; i < muster.size(); ++i)
            {
                QJsonObject m = muster[i].toObject();
                m.remove(QStringLiteral("swingOffset"));
                muster[i] = m;
            }
            json[QStringLiteral("patterns")] = muster;
            Loop alt;
            REQUIRE(ausJson(json, alt));
            CHECK(alt.muster[1].swingOffset == 0.0);
        }
        SUBCASE("richte begrenzt auf −1 … 1")
        {
            loop.muster[0].swingOffset = 3.0;
            loop.muster[1].swingOffset = -7.0;
            loop.richte();
            CHECK(loop.muster[0].swingOffset == 1.0);
            CHECK(loop.muster[1].swingOffset == -1.0);
        }
        SUBCASE("swingVon: Loop + Offset auf 0 … 1, ohne Offset genau der Loop")
        {
            CHECK(swingVon(loop, loop.muster[0]) == loop.swing);
            CHECK(swingVon(loop, loop.muster[1]) == doctest::Approx(0.2));
            loop.muster[1].swingOffset = -0.5;
            CHECK(swingVon(loop, loop.muster[1]) == 0.0);
            loop.muster[1].swingOffset = 0.9;
            CHECK(swingVon(loop, loop.muster[1]) == 1.0);
            loop.swing = 0.0;  // ein gerader Loop, ein Muster swingt trotzdem
            loop.muster[1].swingOffset = 0.3;
            CHECK(swingVon(loop, loop.muster[1]) == doctest::Approx(0.3));
        }
        SUBCASE("setzeSwingOffset: begrenzt, und false ohne Aenderung oder Muster")
        {
            CHECK(setzeSwingOffset(loop, QStringLiteral("A"), 2.0));
            CHECK(loop.muster[0].swingOffset == 1.0);
            CHECK_FALSE(setzeSwingOffset(loop, QStringLiteral("A"), 1.0));
            CHECK_FALSE(setzeSwingOffset(loop, QStringLiteral("gibt es nicht"), 0.2));
        }
    }

    TEST_CASE("Loop Swing offset 15a: der Rechenkern swingt je Muster")
    {
        SUBCASE("Drum-Schritte: A mit dem Swing des Loops, B mit Loop + Offset")
        {
            const Ablauf ablauf = plane(zweiDrums(0.5, 0.5), kRate);
            const std::vector<Schlag>& a = schlaegeVon(ablauf, QStringLiteral("a.wav"));
            const std::vector<Schlag>& b = schlaegeVon(ablauf, QStringLiteral("b.wav"));
            REQUIRE(a.size() == 4u);
            REQUIRE(b.size() == 4u);
            CHECK(a[1].frame == 6891);   // Swing 0,5: 1,25 Schritte = 6890,625
            CHECK(b[1].frame == 8269);   // Swing 1,0: 1,5 Schritte = 8268,75
            CHECK(a[2].frame == 11025);  // der dritte bleibt — in beiden
            CHECK(b[2].frame == 11025);
        }
        SUBCASE("ein Offset, der unter 0 fuehrte, spielt gerade")
        {
            const Ablauf ablauf = plane(zweiDrums(0.2, -0.6), kRate);
            CHECK(schlaegeVon(ablauf, QStringLiteral("b.wav"))[1].frame == 5513);  // 5512,5 gerundet
        }
        SUBCASE("Offset 0: Bit fuer Bit wie ohne das Feld")
        {
            const Loop mit = zweiDrums(0.3, 0.0);
            Loop ohne = mit;
            for (Muster& m : ohne.muster) m.swingOffset = 0.0;
            const Ablauf x = plane(mit, kRate);
            const Ablauf y = plane(ohne, kRate);
            REQUIRE(x.stimmen.size() == y.stimmen.size());
            for (std::size_t i = 0; i < x.stimmen.size(); ++i)
            {
                REQUIRE(x.stimmen[i].schlaege.size() == y.stimmen[i].schlaege.size());
                for (std::size_t k = 0; k < x.stimmen[i].schlaege.size(); ++k)
                    CHECK(x.stimmen[i].schlaege[k].frame == y.stimmen[i].schlaege[k].frame);
            }
        }
        SUBCASE("Noten eines Instrument-Musters")
        {
            Loop loop;
            loop.laut = 1.0;
            loop.schlaegeJeTakt = 1;
            loop.swing = 0.0;
            Muster mel;
            mel.name = QStringLiteral("Mel");
            mel.art = MusterArt::Instrument;
            mel.instrument = QStringLiteral("piano");
            mel.noten = {Note{0, 1, 60, 1.0}, Note{1, 1, 60, 1.0}};
            mel.swingOffset = 1.0;  // ein gerader Loop, das Muster swingt voll
            loop.muster = {mel};
            loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("Mel"), 0, 1}}}};
            loop.richte();
            const Ablauf ablauf = plane(loop, kRate);
            REQUIRE(ablauf.stimmen.size() == 1u);  // derselbe Ton zweimal: eine Stimme
            const std::vector<Schlag>& s = ablauf.stimmen[0].schlaege;
            REQUIRE(s.size() == 2u);
            CHECK(s[0].frame == 0);
            CHECK(s[1].frame == 8269);  // 1,5 Schritte
        }
    }

    TEST_CASE("Loop Swing offset 15a: Raster und Swing im Kopf des Panels, die Zeile im Instrument-Editor")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        SUBCASE("Panel: die drei Felder neben BPM, mit Undo")
        {
            ServiceContainer dienste;
            dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
            ICommandBus* bus = dienste.tryResolve<ICommandBus>();
            REQUIRE(bus != nullptr);
            LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
            auto* schlaege = panel.findChild<QSpinBox*>(QStringLiteral("loopBeats"));
            auto* jeSchlag = panel.findChild<QSpinBox*>(QStringLiteral("loopStepsPerBeat"));
            auto* swing = panel.findChild<QDoubleSpinBox*>(QStringLiteral("loopSwing"));
            REQUIRE(schlaege != nullptr);
            REQUIRE(jeSchlag != nullptr);
            REQUIRE(swing != nullptr);
            CHECK_FALSE(schlaege->isEnabled());  // ohne Loop
            CHECK_FALSE(swing->isEnabled());

            panel.neu();
            CHECK(schlaege->isEnabled());
            CHECK(schlaege->value() == 4);
            CHECK(jeSchlag->value() == 4);
            CHECK(swing->value() == 0.0);

            schlaege->setValue(3);
            CHECK(panel.loop().schlaegeJeTakt == 3);
            CHECK(bus->undoDescription() == "Change grid");
            jeSchlag->setValue(3);
            CHECK(panel.loop().schritteJeSchlag == 3);
            swing->setValue(40.0);
            CHECK(panel.loop().swing == doctest::Approx(0.4));
            CHECK(bus->undoDescription() == "Change swing");
            panel.setzeSwing(0.4);  // derselbe Wert: kein Schritt fuer Undo
            CHECK(bus->undoDescription() == "Change swing");

            REQUIRE(bus->undo());
            CHECK(panel.loop().swing == 0.0);
            CHECK(swing->value() == 0.0);  // das Feld folgt dem Undo
            REQUIRE(bus->undo());
            CHECK(jeSchlag->value() == 4);
            REQUIRE(bus->undo());
            CHECK(schlaege->value() == 4);
        }

        SUBCASE("Instrument-Editor: Raster als Text, Swing offset mit „plays“")
        {
            Loop loop;
            loop.swing = 0.2;
            Muster mel;
            mel.name = QStringLiteral("Mel");
            mel.art = MusterArt::Instrument;
            mel.instrument = QStringLiteral("piano");
            loop.muster = {mel};
            loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("Mel"), 0, 1}}}};
            loop.richte();

            InstrumentEditor editor;  // nicht gezeigt: kein Fenster
            QString zuletzt;
            QObject::connect(&editor, &InstrumentEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
                zuletzt = was;
                editor.setLoop(neu);  // wie das Panel: die Fassung kommt zurueck
            });
            REQUIRE(editor.zeige(loop, QStringLiteral("Mel")));
            RasterZeile* zeile = editor.rasterZeile();
            REQUIRE(zeile != nullptr);
            auto* offset = editor.findChild<QDoubleSpinBox*>(QStringLiteral("instrumentSwingOffset"));
            REQUIRE(offset != nullptr);
            CHECK(zeile->rasterText() == QStringLiteral("Grid of the loop: 4 beats per bar · 4 steps per beat · swing 20 %"));
            CHECK(zeile->spieltText() == QStringLiteral("plays 20 %"));
            offset->setValue(-15.0);
            CHECK(zuletzt == QStringLiteral("Change swing offset"));
            CHECK(editor.loop().findeMuster(QStringLiteral("Mel"))->swingOffset == doctest::Approx(-0.15));
            CHECK(zeile->spieltText() == QStringLiteral("plays 5 %"));
        }
    }
}
