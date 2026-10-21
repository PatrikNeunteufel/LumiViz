/**
 ****************************************************************************************
 * @file   test_LoopSpurenBeispiel.cpp
 * @brief  Loop, Stufe 28f: das Beispiel `lanes_demo` (asset/loop/make_spuren_loops.py)
 *         — jede Spur und die Kurve am Clip tun darin etwas
 *         (docs/audio/Spielweisen_Konzept.md §3)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Liest das Beispiel aus dem Repo (vom Arbeitsordner aus, wie test_LoopSaiten); ohne Ton,
 * ohne Fenster, ohne Engine.
 *
 * - Die Datei: Fassung 12 mit allen Feldern, die das Programm schreibt; das Programm liest
 *   und schreibt sie ohne Verlust.
 * - Die Spuren: Sustain an den Keys, Expression am Pad mit Kurve am Clip, Vibrato und Pitch
 *   am Lead, Expression an der Gitarre, Pitch am Bass.
 * - Die Planung wie im Programm (`saitenAusInstrumenten`): das Pedal haelt die Keys, das Pad
 *   bekommt seinen Verlauf im Rechenkern, die Toene der Gitarre tragen Expression (28c-2b).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Pedalplan.hpp"
#include "loop/Rechenkern.hpp"

#include "LoopOhneEngine.hpp"  // mitgelieferteLoops

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

using namespace lumi::loop;

TEST_SUITE("LoopSpurenBeispiel")
{
    TEST_CASE("Loop Spuren 28f: das Beispiel lanes_demo — Datei, Spuren, Planung")
    {
        QString pfad;
        for (const QString& p : looptest::mitgelieferteLoops())
            if (QFileInfo(p).fileName() == QStringLiteral("lanes_demo.lvloop")) pfad = p;
        REQUIRE_FALSE(pfad.isEmpty());  // mitgeliefert: es steht in asset/Asset.cmake (*_demo)

        // --- die Datei: Fassung 12 mit allen Feldern, die das Programm immer schreibt
        QFile datei(pfad);
        REQUIRE(datei.open(QIODevice::ReadOnly));
        const QJsonObject wurzel = QJsonDocument::fromJson(datei.readAll()).object();
        CHECK(wurzel.value(QStringLiteral("version")).toInt() == 12);
        CHECK(wurzel.contains(QStringLiteral("limiter")));
        for (const QJsonValue m : wurzel.value(QStringLiteral("patterns")).toArray())
            if (m.toObject().value(QStringLiteral("kind")).toString() == QStringLiteral("instrument"))
                CHECK(m.toObject().contains(QStringLiteral("lanes")));
        for (const QJsonValue k : wurzel.value(QStringLiteral("channels")).toArray())
        {
            CHECK(k.toObject().contains(QStringLiteral("gain")));
            for (const QJsonValue c : k.toObject().value(QStringLiteral("clips")).toArray())
                CHECK(c.toObject().contains(QStringLiteral("expression")));
        }
        Loop loop;
        QStringList bericht;
        REQUIRE(ladeLoop(pfad, loop, &bericht));
        CHECK(bericht.isEmpty());
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString kopie = ordner.filePath(QStringLiteral("lanes_demo.lvloop"));
        REQUIRE(speichereLoop(loop, kopie));
        Loop wieder;
        REQUIRE(ladeLoop(kopie, wieder));
        CHECK(wieder == loop);

        // --- jede Spur tut etwas
        const auto spur = [&loop](const char* muster, Steuerspur s) -> const std::vector<SpurPunkt>& {
            const Muster* m = loop.findeMuster(QString::fromLatin1(muster));
            REQUIRE(m != nullptr);
            return m->zeitspuren.von(s);
        };
        CHECK(spur("Keys", Steuerspur::Sustain).size() == 8);
        CHECK_FALSE(spur("Pad", Steuerspur::Expression).empty());
        CHECK_FALSE(spur("Lead", Steuerspur::Vibrato).empty());
        CHECK_FALSE(spur("Lead", Steuerspur::Pitch).empty());
        CHECK_FALSE(spur("Guitar", Steuerspur::Expression).empty());
        CHECK_FALSE(spur("Bass", Steuerspur::Pitch).empty());
        const Kanal* pad = nullptr;
        for (const Kanal& k : loop.kanaele)
            if (k.name == QStringLiteral("Pad")) pad = &k;
        REQUIRE(pad != nullptr);
        REQUIRE(pad->clips.size() == 1);
        CHECK(pad->clips.front().expression == std::vector<SpurPunkt>{{0.0, 0.2}, {2.0, 1.0}});

        // --- jedes Instrument traegt die Spuren, die es bekommt (SF6 a, SF7 a)
        const auto traegt = [](const char* name, Steuerspur s) {
            Instrument instrument;
            REQUIRE(holeInstrument(QString::fromLatin1(name), instrument));
            return traegtSpur(instrument, s);
        };
        CHECK(traegt("piano", Steuerspur::Sustain));
        CHECK(traegt("warm_pad", Steuerspur::Expression));
        CHECK(traegt("lead", Steuerspur::Vibrato));
        CHECK(traegt("clean_guitar", Steuerspur::Expression));  // ueber den Tonabnehmer
        CHECK(traegt("round_bass", Steuerspur::Pitch));

        // --- die Planung wie im Programm
        const SaitenVon saiten = saitenAusInstrumenten();
        const Muster* keys = loop.findeMuster(QStringLiteral("Keys"));
        REQUIRE(keys != nullptr);
        CHECK(gehaltenMs(loop, *keys, 2, 0, 0) > loop.notenMs(keys->noten.front()));  // das Pedal haelt
        const QStringList namen = loop.klangNamen(saiten);
        bool gitarreMitExpression = false;
        for (const QString& name : namen)
            if (name.contains(QStringLiteral("clean_guitar")) && name.contains(QStringLiteral(":x"))) gitarreMitExpression = true;
        CHECK(gitarreMitExpression);
        const Ablauf ablauf = plane(loop, 44100, saiten);
        CHECK_FALSE(ablauf.ausdruecke.empty());  // das Pad (und die Keys, der Lead) im Rechenkern
        for (const Stimme& stimme : ablauf.stimmen) CHECK(stimme.klang >= 0);
    }
}
