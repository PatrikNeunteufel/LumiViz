/**
 * @file test_LoopEffekteSchalter.cpp
 * @brief Loop, S124: der Schalter der Effekte je Clip (Entscheid Patrik E1 a) —
 *        der Bezugsclip der Editoren, Play pattern mit Effekten, FX, die matte
 *        Marke, Fassung 7 der `.lvloop`
 *
 * Ohne Ton und ohne gezeigtes Fenster. `LUMIVIZ_FX_SCHALTER_BILD=<datei.png>`
 * legt ein Bild der Taktachse ab: ein Clip mit Effekten an, einer aus.
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"
#include "loop/ClipEffektFenster.hpp"
#include "loop/DrumEditor.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Taktachse.hpp"

#include <QApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QPushButton>

using namespace lumi::loop;

namespace
{

/// „Grund" (4 Takte) in Drums bei Takt 1 (mit Echo) und Takt 5 (ohne Effekte), in Perc bei Takt 9 (mit Hall).
Loop probe()
{
    Loop loop;
    Muster grund;
    grund.name = QStringLiteral("Grund");
    grund.takte = 4;
    grund.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    grund.zeilen = {std::vector<double>(64, 0.0)};
    grund.zeilen[0][0] = 1.0;
    loop.muster = {grund};
    const ClipEffekte echo{{ClipEcho{true, EchoZeit::Viertel, 0.5, 0.2, 0.4}}};
    const ClipEffekte hall{{ClipHall{true, 0.5, 2.0, 0.4, 0.3}}};
    loop.kanaele = {{QStringLiteral("Drums"), true, {LoopClip{QStringLiteral("Grund"), 0, 1, echo}, LoopClip{QStringLiteral("Grund"), 4, 1}}},
                    {QStringLiteral("Perc"), true, {LoopClip{QStringLiteral("Grund"), 8, 1, hall}}}};
    loop.richte();
    return loop;
}

const LoopClip& clipBei(const Loop& loop, ClipOrt ort)
{
    return loop.kanaele[static_cast<std::size_t>(ort.kanal)].clips[static_cast<std::size_t>(findeClip(loop, ort))];
}

}  // namespace

TEST_SUITE("LoopEffekteSchalter")
{
    TEST_CASE("Loop FX-Schalter: Bezugsclip, Schalter, Rechenkern, Datei, Solo-Fassung (E1 a)")
    {
        Loop loop = probe();
        // --- der Bezugsclip: der markierte, wenn er das Muster spielt — sonst der erste
        CHECK(bezugsclip(loop, QStringLiteral("Grund"), std::nullopt) == ClipOrt{0, 0});
        CHECK(bezugsclip(loop, QStringLiteral("Grund"), ClipOrt{1, 8}) == ClipOrt{1, 8});
        CHECK(bezugsclip(loop, QStringLiteral("Grund"), ClipOrt{0, 99}) == ClipOrt{0, 0});  // gibt es nicht
        CHECK_FALSE(bezugsclip(loop, QStringLiteral("Gibt es nicht"), std::nullopt).has_value());
        CHECK(clipText(loop, {1, 8}) == QStringLiteral("Perc, bar 9"));

        // --- der Schalter: aus = trocken, die Reihe bleibt
        CHECK(plane(loop, 48000).busse.size() == 2u);
        REQUIRE(setzeEffekteAn(loop, {0, 0}, false));
        CHECK_FALSE(setzeEffekteAn(loop, {0, 0}, false));  // stand schon so
        CHECK_FALSE(setzeEffekteAn(loop, {0, 99}, true));   // gibt es nicht
        CHECK_FALSE(clipBei(loop, {0, 0}).effekteAn);
        CHECK_FALSE(clipBei(loop, {0, 0}).effekteWirken());
        CHECK(clipBei(loop, {0, 0}).effekte.einerAn());     // die Reihe ist noch da
        CHECK(plane(loop, 48000).busse.size() == 1u);       // nur der Hall in Perc

        // --- die Datei: Fassung 7, `fxOn` nur am abgeschalteten Clip
        const QJsonObject json = alsJson(loop);
        CHECK(json.value(QStringLiteral("version")).toInt() == 9);  // fxOn seit 7; geschrieben wird die juengste
        const QJsonArray drums = json.value(QStringLiteral("channels")).toArray().at(0).toObject().value(QStringLiteral("clips")).toArray();
        CHECK(drums.at(0).toObject().value(QStringLiteral("fxOn")) == QJsonValue(false));
        CHECK_FALSE(drums.at(1).toObject().contains(QStringLiteral("fxOn")));
        Loop gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen == loop);

        // --- Play pattern: der Solo-Loop traegt die Effekte und den Schalter des Bezugsclips
        const Loop ohneMarke = soloFassung(loop, QStringLiteral("Grund"), std::nullopt);
        REQUIRE(ohneMarke.kanaele.size() == 1u);
        REQUIRE(ohneMarke.kanaele[0].clips.size() == 1u);
        CHECK(ohneMarke.kanaele[0].clips[0].effekte == clipBei(loop, {0, 0}).effekte);
        CHECK_FALSE(ohneMarke.kanaele[0].clips[0].effekteAn);
        const Loop mitPerc = soloFassung(loop, QStringLiteral("Grund"), ClipOrt{1, 8});
        CHECK(mitPerc.kanaele[0].clips[0].effekte == clipBei(loop, {1, 8}).effekte);
        CHECK(mitPerc.kanaele[0].clips[0].effekteAn);
        CHECK(plane(mitPerc, 48000).busse.size() == 1u);
        const Loop ohneEffekte = soloFassung(loop, QStringLiteral("Grund"), ClipOrt{0, 4});
        CHECK(ohneEffekte.kanaele[0].clips[0].effekte.reihe.empty());
    }

    TEST_CASE("Loop FX-Schalter: Knopf FX in den Editoren, Haken im Fenster Clip effects, matte Marke (E1 a)")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        // --- Drum-Editor: FX gilt dem Bezugsclip
        DrumEditor editor;
        QString zuletzt;
        QObject::connect(&editor, &DrumEditor::bearbeitet, &editor, [&](const Loop& neu, const QString& was) {
            zuletzt = was;
            editor.setLoop(neu);
        });
        REQUIRE(editor.zeige(probe(), QStringLiteral("Grund")));
        auto* fx = editor.findChild<QPushButton*>(QStringLiteral("drumFx"));
        REQUIRE(fx != nullptr);
        CHECK(editor.bezugsclip() == ClipOrt{0, 0});
        CHECK(fx->isEnabled());
        CHECK(fx->isChecked());
        CHECK(fx->toolTip().contains(QStringLiteral("Drums, bar 1")));
        REQUIRE(editor.setzeEffekteAn(false));
        CHECK(zuletzt == QStringLiteral("Effects off"));
        CHECK_FALSE(clipBei(editor.loop(), {0, 0}).effekteAn);
        CHECK_FALSE(fx->isChecked());
        CHECK(fx->toolTip().contains(QStringLiteral(": off")));
        // ein anderer Clip derselben Basis markiert: er ist der Bezugsclip
        editor.setMarkierterClip(ClipOrt{1, 8});
        CHECK(editor.bezugsclip() == ClipOrt{1, 8});
        CHECK(fx->isChecked());
        // einer ohne Effekte: FX ist grau
        editor.setMarkierterClip(ClipOrt{0, 4});
        CHECK_FALSE(fx->isEnabled());
        CHECK(fx->toolTip().contains(QStringLiteral("none")));
        CHECK_FALSE(editor.setzeEffekteAn(true));  // nichts zu schalten: der Schalter steht schon an

        // --- Instrument-Editor: derselbe Knopf
        InstrumentEditor instrument;
        CHECK(instrument.findChild<QPushButton*>(QStringLiteral("instrumentFx")) != nullptr);

        // --- Clip effects: der Haken Effects on fuer alle markierten Clips, halb bei verschiedenen
        Loop loop = editor.loop();  // Drums Takt 1 aus, Perc Takt 9 an
        ClipEffektFenster fenster;
        QObject::connect(&fenster, &ClipEffektFenster::bearbeitet, &fenster, [&](const Loop& neu, const QString& was) {
            zuletzt = was;
            loop = neu;
        });
        fenster.zeige(loop, {{0, 0}, {1, 8}});
        CHECK(fenster.effekteAnStand() == Qt::PartiallyChecked);
        REQUIRE(fenster.setzeEffekteAn(true));
        CHECK(zuletzt == QStringLiteral("Effects on"));
        CHECK(clipBei(loop, {0, 0}).effekteAn);
        CHECK(fenster.effekteAnStand() == Qt::Checked);

        // `LUMIVIZ_FX_SCHALTER_BILD=<datei.png>`: die Taktachse mit einer hellen und einer matten Marke fx
        if (const QString bild = qEnvironmentVariable("LUMIVIZ_FX_SCHALTER_BILD"); !bild.isEmpty())
        {
            Loop zwei = probe();
            (void)setzeEffekteAn(zwei, {1, 8}, false);
            Taktachse achse;
            achse.resize(900, 200);
            achse.setLoop(zwei, true);
            CHECK(achse.grab().save(bild));
        }
    }
}
