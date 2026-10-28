/**
 ****************************************************************************************
 * @file   test_LoopNotenStimme.cpp
 * @brief  Loop, Stufe 31d-9: Stimme und Versatz je Note
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S157: HD2 a,
 *         HD2-1 a, N1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung, ohne Dateien aus dem Repo.
 *
 * - Die Datei (Fassung 14): `voices` am Muster immer; Stimme und Versatz in der Zeile
 *   der Note nur, wenn einer davon gesetzt ist — eine Note ohne bleibt vier Werte lang.
 * - Richten: Stimme 0 … 16, Versatz des Anfangs 0 … < 1, des Endes > −1 … 0, kein Ende
 *   hinter dem Muster; zwei gleiche Toene im selben Schritt mit verschiedenem Versatz
 *   bleiben beide.
 * - Planung: der Versatz verschiebt den Anschlag um genau seinen Bruchteil eines
 *   Schritts und kuerzt oder verlaengert die Dauer; ohne Versatz wie bis Fassung 13.
 * - Pedalplan: ein neuer Anschlag desselben Tons beendet den gehaltenen an seiner
 *   Stelle mit Versatz.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Loop.hpp"
#include "loop/Pedalplan.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Zeitspur.hpp"

#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 48000;  // bei 120 BPM, 4 Schritten je Schlag: 6000 Abtastwerte, 125 ms je Schritt

/// Ein Instrument-Muster „P" von einem Takt (16 Schritte) mit `noten`, in einem Clip.
Loop instrumentLoop(std::vector<Note> noten, QStringList stimmen = {})
{
    Loop loop;
    Muster m;
    m.name = QStringLiteral("P");
    m.art = MusterArt::Instrument;
    m.takte = 1;
    m.instrument = QStringLiteral("piano");
    m.noten = std::move(noten);
    m.stimmen = std::move(stimmen);
    loop.muster = {m};
    loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("P"), 0, 1)}}};
    loop.richte();
    return loop;
}

Note mitVersatz(int schritt, int laenge, int hoehe, double anfang, double ende, int stimme = 0)
{
    Note note{schritt, laenge, hoehe};
    note.versatzAnfang = anfang;
    note.versatzEnde = ende;
    note.stimme = stimme;
    return note;
}

std::vector<std::int64_t> anschlaege(const Ablauf& ablauf)
{
    std::vector<std::int64_t> frames;
    for (const Stimme& stimme : ablauf.stimmen)
        for (const Schlag& schlag : stimme.schlaege) frames.push_back(schlag.frame);
    std::sort(frames.begin(), frames.end());
    return frames;
}

}  // namespace

TEST_CASE("Loop Noten 31d-9 Stimme und Versatz in der Datei (Fassung 14)")
{
    const Loop loop = instrumentLoop({Note{0, 4, 60}, mitVersatz(4, 2, 64, 0.25, -0.5, 2)},
                                     {QStringLiteral("Left hand"), QStringLiteral("Right hand")});
    const QJsonObject json = alsJson(loop);
    CHECK(json.value(QStringLiteral("version")).toInt() == 14);
    const QJsonObject muster = json.value(QStringLiteral("patterns")).toArray().first().toObject();
    CHECK(muster.value(QStringLiteral("voices")).toArray() ==
          QJsonArray{QStringLiteral("Left hand"), QStringLiteral("Right hand")});
    const QJsonArray noten = muster.value(QStringLiteral("notes")).toArray();
    REQUIRE(noten.size() == 2);
    CHECK(noten[0].toArray().size() == 4);  // ohne Stimme und Versatz wie bis Fassung 13
    const QJsonArray zweite = noten[1].toArray();
    REQUIRE(zweite.size() == 10);
    CHECK(zweite[4].toInt() == -1);  // Saite: keine
    CHECK(zweite[5].toInt() == 0);   // keine Artikulationen
    CHECK(zweite[6].toArray().isEmpty());  // kein Bend
    CHECK(zweite[7].toInt() == 2);
    CHECK(zweite[8].toDouble() == 0.25);
    CHECK(zweite[9].toDouble() == -0.5);

    Loop gelesen;
    REQUIRE(ausJson(json, gelesen));
    CHECK(gelesen == loop);

    // ohne Stimmen: `voices` steht trotzdem da (leer)
    const QJsonObject ohne = alsJson(instrumentLoop({Note{0, 4, 60}})).value(QStringLiteral("patterns")).toArray().first().toObject();
    CHECK(ohne.contains(QStringLiteral("voices")));
    CHECK(ohne.value(QStringLiteral("voices")).toArray().isEmpty());

    // eine Zeile aus Fassung 13 (vier Werte) liest sich ohne Stimme, auf dem Raster
    QJsonObject alt = json;
    QJsonArray muster13 = alt.value(QStringLiteral("patterns")).toArray();
    QJsonObject m13 = muster13.first().toObject();
    // ausdruecklich angehaengt: `QJsonArray{QJsonArray{…}}` liest MSVC als Kopie der inneren Liste
    QJsonArray zeilen13;
    zeilen13.append(QJsonArray{4, 2, 64, 1.0});
    m13[QStringLiteral("notes")] = zeilen13;
    m13.remove(QStringLiteral("voices"));
    muster13[0] = m13;
    alt[QStringLiteral("patterns")] = muster13;
    alt[QStringLiteral("version")] = 13;
    Loop dreizehn;
    REQUIRE(ausJson(alt, dreizehn));
    REQUIRE(dreizehn.muster.front().noten.size() == 1);
    const Note& n = dreizehn.muster.front().noten.front();
    CHECK(n.stimme == 0);
    CHECK(n.versatzAnfang == 0.0);
    CHECK(n.versatzEnde == 0.0);
    CHECK(dreizehn.muster.front().stimmen.isEmpty());
}

TEST_CASE("Loop Noten 31d-9 Stimme und Versatz gerichtet")
{
    QStringList viele;
    for (int i = 1; i <= 20; ++i) viele << QStringLiteral("V%1").arg(i);
    const Loop loop = instrumentLoop({mitVersatz(0, 1, 60, 1.5, 0.4, 20),     // zu weit: 0,999, 0, Stimme 16
                                      mitVersatz(1, 1, 60, -0.2, -3.0, -3),   // 0, −0,999, Stimme 0
                                      mitVersatz(15, 1, 62, 0.5, 0.0),        // reichte bis 16,5: Ende −0,5
                                      mitVersatz(8, 1, 67, 0.5, 0.0),         // zwei gleiche Toene im Schritt 8 …
                                      mitVersatz(8, 1, 67, 0.0, 0.0),         // … mit verschiedenem Versatz
                                      mitVersatz(8, 1, 67, 0.0, 0.0)},        // derselbe Anfang: faellt weg
                                     viele);
    const Muster& m = loop.muster.front();
    CHECK(m.stimmen.size() == kStimmenMax);
    REQUIRE(m.noten.size() == 5);
    CHECK(m.noten[0].stimme == kStimmenMax);
    CHECK(m.noten[0].versatzAnfang == kVersatzHoechstens);
    CHECK(m.noten[0].versatzEnde == 0.0);
    CHECK(m.noten[1].stimme == 0);
    CHECK(m.noten[1].versatzAnfang == 0.0);
    CHECK(m.noten[1].versatzEnde == -kVersatzHoechstens);
    CHECK(notenDauer(m.noten[1]) > 0.0);
    // Schritt 8: nach dem Versatz sortiert, beide da
    CHECK(m.noten[2].schritt == 8);
    CHECK(m.noten[2].versatzAnfang == 0.0);
    CHECK(m.noten[3].schritt == 8);
    CHECK(m.noten[3].versatzAnfang == 0.5);
    // die letzte endet am Ende des Musters, nicht dahinter
    CHECK(m.noten[4].schritt == 15);
    CHECK(notenAnfang(m.noten[4]) + notenDauer(m.noten[4]) == doctest::Approx(16.0));
}

TEST_CASE("Loop Noten 31d-9 Versatz in der Planung: Anschlag und Dauer")
{
    // ohne Versatz: auf dem Raster, Dauer in ganzen Schritten — wie bis Fassung 13
    const Loop ohne = instrumentLoop({Note{2, 2, 60}, Note{8, 4, 62}});
    const Ablauf a = plane(ohne, kRate);
    CHECK(a.schrittFrames == 6000.0);
    CHECK(anschlaege(a) == std::vector<std::int64_t>{12000, 48000});
    CHECK(ohne.notenMs(ohne.muster.front().noten[0]) == 250);
    CHECK(a.klangNamen.contains(tonKlang(QStringLiteral("piano"), 60, 250)));

    // mit Versatz: ein halber Schritt spaeter, ein halber Schritt kuerzer
    const Loop mit = instrumentLoop({mitVersatz(2, 2, 60, 0.5, -0.5), Note{8, 4, 62}});
    const Ablauf b = plane(mit, kRate);
    CHECK(anschlaege(b) == std::vector<std::int64_t>{15000, 48000});
    CHECK(mit.notenMs(mit.muster.front().noten[0]) == 188);  // 1,5 Schritte = 187,5 ms
    CHECK(b.klangNamen.contains(tonKlang(QStringLiteral("piano"), 60, 188)));

    // ein Vorschlag kuerzer als ein Schritt: 0,3 bis 0,8
    const Loop vorschlag = instrumentLoop({mitVersatz(4, 1, 65, 0.3, -0.5)});
    CHECK(anschlaege(plane(vorschlag, kRate)) == std::vector<std::int64_t>{25800});
    CHECK(vorschlag.notenMs(vorschlag.muster.front().noten[0]) == 63);  // 0,5 Schritte = 62,5 ms
}

TEST_CASE("Loop Noten 31d-9 Versatz im Pedalplan: derselbe Ton schlaegt mit Versatz neu an")
{
    Loop loop = instrumentLoop({Note{0, 1, 60}, mitVersatz(2, 1, 60, 0.5, 0.0)});
    loop.muster.front().zeitspuren.von(Steuerspur::Sustain) = {SpurPunkt{0.0, 1.0}};
    loop.richte();
    const Muster& m = loop.muster.front();
    REQUIRE(m.noten.size() == 2);
    // das Pedal haelt die erste bis zum neuen Anschlag desselben Tons bei 2,5
    CHECK(gehaltenBis(loop, m, 1, 0, 0) == doctest::Approx(2.5));
    CHECK(gehaltenMs(loop, m, 1, 0, 0) == 313);  // 2,5 Schritte = 312,5 ms
}
