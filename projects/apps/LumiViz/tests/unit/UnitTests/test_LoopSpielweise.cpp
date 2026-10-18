/**
 ****************************************************************************************
 * @file   test_LoopSpielweise.cpp
 * @brief  Spielweisen, Stufe 24a: das Modell — Strum am Muster und am Instrument, die
 *         Abweichungen der Akkorde (`Muster::anschlaege`), die Artikulationen einer Note,
 *         Playing an der Saite; `.lvloop` Fassung 9, `.lvinst` Fassung 11
 *         (docs/audio/Saiten_Konzept.md; Entscheide Patrik S134: S-W2 a bis S-W6 a, S-W9 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Modell und Dateien ohne Ton. Der Beleg der Stufe: jede Stelle, die Noten bewegt, nimmt
 * die Abweichung ihres Akkords mit (S-W4 a) — je Stelle ein Teil unten; nur der letzte
 * Fall (das Ziehen einer einzelnen Note im Editor) braucht eine Qt-Anwendung, gezeigt wird
 * nichts. Wie es klingt und wann es anschlaegt, pruefen die Stufen 24b und 24c.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Auswahl.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentEditor.hpp"
#include "loop/Loop.hpp"
#include "loop/Spielweise.hpp"

#include <QApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QMouseEvent>

#include <vector>

using namespace lumi::loop;

namespace
{

/// Ein Loop mit einem Instrument-Muster aus drei Akkorden und einer Melodienote, ein Takt
/// mit 16 Schritten: Akkord an 0 (E-Dur-Griff), an 4 (zwei Noten), an 8 (drei Noten),
/// eine Note an 12.
Loop probe()
{
    Loop loop;
    Muster m;
    m.name = QStringLiteral("Strum");
    m.art = MusterArt::Instrument;
    m.instrument = QStringLiteral("guitar");
    m.takte = 1;
    m.noten = {Note{0, 2, 40}, Note{0, 2, 47}, Note{0, 2, 52}, Note{4, 2, 45}, Note{4, 2, 52},
               Note{8, 2, 43}, Note{8, 2, 47}, Note{8, 2, 55}, Note{12, 2, 64}};
    loop.muster.push_back(m);
    loop.kanaele = {{QStringLiteral("Guitar"), true, {{m.name, 0, 1}}}};
    loop.richte();
    return loop;
}

Muster& riff(Loop& loop)
{
    return loop.muster[0];
}

QJsonObject musterJson(const QJsonObject& wurzel)
{
    return wurzel.value(QStringLiteral("patterns")).toArray().at(0).toObject();
}

/// Die Schritte der Eintraege, in ihrer Reihenfolge.
std::vector<int> schritteVon(const Muster& m)
{
    std::vector<int> schritte;
    for (const Anschlag& a : m.anschlaege) schritte.push_back(a.schritt);
    return schritte;
}

void maus(QWidget& ziel, QEvent::Type art, QPoint ort, Qt::MouseButton taste, Qt::MouseButtons tasten)
{
    QMouseEvent ereignis(art, QPointF(ort), QPointF(ziel.mapToGlobal(ort)), taste, tasten, Qt::NoModifier);
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

}  // namespace

TEST_SUITE("LoopSpielweise")
{
    TEST_CASE("Loop Spielweise: die Hand — Stellen, Richtung, Grenzen, Namen")
    {
        // --- so viele Stellen hat ein Takt: 4/4 mit Sechzehnteln
        CHECK(handStellen(HandRaster::Schlag, 4, 4) == 4);
        CHECK(handStellen(HandRaster::HalberSchlag, 4, 4) == 8);
        CHECK(handStellen(HandRaster::Schritt, 4, 4) == 16);
        CHECK(handStellen(HandRaster::HalberSchlag, 3, 3) == 6);

        // --- die Stelle eines Schritts; dazwischen gilt die vorige (S-W3 a)
        CHECK(handStelleVon(HandRaster::HalberSchlag, 0, 4, 4) == 0);
        CHECK(handStelleVon(HandRaster::HalberSchlag, 1, 4, 4) == 0);  // „e" faellt auf den Abschlag
        CHECK(handStelleVon(HandRaster::HalberSchlag, 2, 4, 4) == 1);  // „und"
        CHECK(handStelleVon(HandRaster::HalberSchlag, 15, 4, 4) == 7);
        CHECK(handStelleVon(HandRaster::HalberSchlag, 16, 4, 4) == 0);  // der naechste Takt beginnt vorn
        CHECK(handStelleVon(HandRaster::Schlag, 7, 4, 4) == 1);
        CHECK(handStelleVon(HandRaster::Schritt, 21, 4, 4) == 5);
        // Triolen: der halbe Schlag liegt zwischen zwei Schritten — der zweite Schritt gehoert noch zur ersten Haelfte
        CHECK(handStelleVon(HandRaster::HalberSchlag, 1, 4, 3) == 0);
        CHECK(handStelleVon(HandRaster::HalberSchlag, 2, 4, 3) == 1);

        // --- die Richtung: gesetzt oder abwechselnd
        Strum strum;
        CHECK(strum.raster == HandRaster::HalberSchlag);  // Vorgabe: Achtel bei 4/4
        CHECK(strum.spreadMs == doctest::Approx(kSpreadVorgabeMs));
        CHECK(richtungAnStelle(strum, 0) == Richtung::Ab);
        CHECK(richtungAnStelle(strum, 1) == Richtung::Auf);
        CHECK(richtungAnStelle(strum, 6) == Richtung::Ab);
        strum.hand = handAusText(QStringLiteral("DDUU"));
        CHECK(richtungAnStelle(strum, 1) == Richtung::Ab);
        CHECK(richtungAnStelle(strum, 2) == Richtung::Auf);
        CHECK(richtungAnStelle(strum, 5) == Richtung::Auf);  // dahinter wieder abwechselnd
        CHECK(handAlsText(strum.hand) == QStringLiteral("DDUU"));
        CHECK(handAlsText(handAusText(QStringLiteral("dxu?"))) == QStringLiteral("DUUU"));  // unbekannt: abwechselnd

        // --- richten: eine Schreibweise — hinten weg, was ohnehin abwechselt
        Strum lang;
        lang.hand = handAusText(QStringLiteral("DDDUDUDU"));
        richteStrum(lang);
        CHECK(handAlsText(lang.hand) == QStringLiteral("DD"));  // ab der dritten Stelle wechselt es ohnehin ab
        Strum abwechselnd;
        abwechselnd.hand = handAusText(QStringLiteral("DUDUDU"));
        richteStrum(abwechselnd);
        CHECK(abwechselnd.hand.empty());
        CHECK(abwechselnd == Strum{});
        Strum zuViel;
        zuViel.hand = handAusText(QStringLiteral("UUUUUUUUUU"));
        richteStrum(zuViel, 4);  // ein Takt hat nur vier Stellen
        CHECK(handAlsText(zuViel.hand) == QStringLiteral("UUU"));  // U U U U — die vierte waere ohnehin Up
        Strum wild;
        wild.spreadMs = 9999.0;
        wild.raster = static_cast<HandRaster>(7);
        richteStrum(wild);
        CHECK(wild.spreadMs == doctest::Approx(kSpreadMaxMs));
        CHECK(wild.raster == HandRaster::HalberSchlag);
        wild.spreadMs = -3.0;
        richteStrum(wild);
        CHECK(wild.spreadMs == doctest::Approx(0.0));

        // --- die Namen in der Datei
        for (const HandRaster raster : {HandRaster::Schlag, HandRaster::HalberSchlag, HandRaster::Schritt})
            CHECK(handRasterAusName(handRasterName(raster)) == raster);
        CHECK(handRasterAusName(QStringLiteral("quarter")) == HandRaster::HalberSchlag);
        for (const Anschlagart art : {Anschlagart::Hand, Anschlagart::Ab, Anschlagart::Auf, Anschlagart::Zugleich})
            CHECK(anschlagartAusName(anschlagartName(art)) == art);
        CHECK(anschlagartAusName(QStringLiteral("sideways")) == Anschlagart::Hand);

        // --- ein Anschlag weicht ab, wenn Richtung oder Versatz es tun
        CHECK_FALSE(Anschlag{}.weichtAb());
        CHECK(Anschlag{0, Anschlagart::Auf}.weichtAb());
        CHECK(Anschlag{0, Anschlagart::Hand, 40.0}.weichtAb());
        CHECK(Anschlag{0, Anschlagart::Zugleich}.weichtAb());
    }

    TEST_CASE("Loop Spielweise: .lvloop Fassung 9 — Strum, Anschlaege, Artikulationen")
    {
        // --- ohne Spielweisen: kein Block, keine sechsten Werte — die Noten wie in Fassung 8
        Loop schlicht = probe();
        const QJsonObject ohne = musterJson(alsJson(schlicht));
        CHECK(alsJson(schlicht).value(QStringLiteral("version")).toInt() == 9);
        CHECK_FALSE(ohne.contains(QStringLiteral("strum")));
        CHECK_FALSE(ohne.contains(QStringLiteral("strokes")));
        for (const QJsonValue& zeile : ohne.value(QStringLiteral("notes")).toArray()) CHECK(zeile.toArray().size() == 4);

        // --- mit: Strum, zwei Abweichungen, Artikulationen an zwei Noten
        Loop loop = probe();
        Muster& m = riff(loop);
        m.strum.spreadMs = 35.0;
        m.strum.raster = HandRaster::Schritt;
        m.strum.hand = handAusText(QStringLiteral("DDU"));
        setzeAnschlag(m, Anschlag{4, Anschlagart::Auf});
        setzeAnschlag(m, Anschlag{8, Anschlagart::Hand, 80.0});
        m.noten[0].artikulationen = kPalmMute;
        m.noten[8].artikulationen = kHarmonic | kLetRing;
        m.noten[8].saite = 5;
        loop.richte();

        const QJsonObject json = alsJson(loop);
        const QJsonObject o = musterJson(json);
        const QJsonObject strum = o.value(QStringLiteral("strum")).toObject();
        CHECK(strum.value(QStringLiteral("spread")).toDouble() == doctest::Approx(35.0));
        CHECK(strum.value(QStringLiteral("hand")).toString() == QStringLiteral("step"));
        CHECK(strum.value(QStringLiteral("pattern")).toString() == QStringLiteral("DDU"));
        const QJsonArray anschlaege = o.value(QStringLiteral("strokes")).toArray();
        REQUIRE(anschlaege.size() == 2);
        CHECK(anschlaege[0].toObject() == QJsonObject{{QStringLiteral("step"), 4}, {QStringLiteral("stroke"), QStringLiteral("up")}});
        CHECK(anschlaege[1].toObject() == QJsonObject{{QStringLiteral("step"), 8},
                                                      {QStringLiteral("stroke"), QStringLiteral("hand")},
                                                      {QStringLiteral("spread"), 80.0}});
        const QJsonArray noten = o.value(QStringLiteral("notes")).toArray();
        CHECK(noten[0].toArray() == QJsonArray{0, 2, 40, 1.0, -1, kPalmMute});  // ohne Saite: −1 als Platzhalter
        CHECK(noten[1].toArray().size() == 4);
        CHECK(noten[8].toArray() == QJsonArray{12, 2, 64, 1.0, 5, kHarmonic | kLetRing});

        // --- und zurueck: dasselbe Modell
        Loop zurueck;
        REQUIRE(ausJson(json, zurueck));
        CHECK(zurueck == loop);

        // --- eine Datei der Fassung 8: kein Strum, keine Abweichungen, keine Artikulationen
        QJsonObject alt = json;
        alt[QStringLiteral("version")] = 8;
        QJsonObject altMuster = musterJson(alt);
        altMuster.remove(QStringLiteral("strum"));
        altMuster.remove(QStringLiteral("strokes"));
        QJsonArray altNoten;
        for (const QJsonValue& zeile : altMuster.value(QStringLiteral("notes")).toArray())
        {
            QJsonArray werte = zeile.toArray();
            while (werte.size() > 5) werte.removeLast();
            altNoten.append(werte);
        }
        altMuster[QStringLiteral("notes")] = altNoten;
        alt[QStringLiteral("patterns")] = QJsonArray{altMuster};
        Loop gelesen;
        REQUIRE(ausJson(alt, gelesen));
        CHECK(riff(gelesen).strum == Strum{});
        CHECK(riff(gelesen).anschlaege.empty());
        for (const Note& note : riff(gelesen).noten) CHECK(note.artikulationen == 0);

        // --- was dieses Programm nicht kennt oder was nicht passt, richtet `richte`
        QJsonObject wild = json;
        QJsonObject wildMuster = musterJson(wild);
        QJsonArray wildNoten = wildMuster.value(QStringLiteral("notes")).toArray();
        wildNoten[1] = QJsonArray{0, 2, 47, 1.0, -1, 0xF0 | kTapping};  // Bits von morgen
        wildMuster[QStringLiteral("notes")] = wildNoten;
        wildMuster[QStringLiteral("strokes")] = QJsonArray{
            QJsonObject{{QStringLiteral("step"), 8}, {QStringLiteral("stroke"), QStringLiteral("down")}},
            QJsonObject{{QStringLiteral("step"), 4}, {QStringLiteral("stroke"), QStringLiteral("together")}},
            QJsonObject{{QStringLiteral("step"), 8}, {QStringLiteral("stroke"), QStringLiteral("up")}},       // zweimal: der erste bleibt
            QJsonObject{{QStringLiteral("step"), 6}, {QStringLiteral("stroke"), QStringLiteral("up")}},       // dort schlaegt nichts an
            QJsonObject{{QStringLiteral("step"), 0}, {QStringLiteral("stroke"), QStringLiteral("hand")}},     // weicht nicht ab
            QJsonObject{{QStringLiteral("step"), 12}, {QStringLiteral("stroke"), QStringLiteral("sideways")}, {QStringLiteral("spread"), 9999}}};
        wildMuster[QStringLiteral("strum")] = QJsonObject{{QStringLiteral("spread"), -5},
                                                          {QStringLiteral("hand"), QStringLiteral("halfBeat")},
                                                          {QStringLiteral("pattern"), QStringLiteral("UUUUUUUUUUUU")}};
        wild[QStringLiteral("patterns")] = QJsonArray{wildMuster};
        Loop gerichtet;
        REQUIRE(ausJson(wild, gerichtet));
        const Muster& g = riff(gerichtet);
        CHECK(g.noten[1].artikulationen == kTapping);
        CHECK(schritteVon(g) == std::vector<int>{4, 8, 12});
        CHECK(g.anschlaege[0].art == Anschlagart::Zugleich);
        CHECK(g.anschlaege[1].art == Anschlagart::Ab);
        CHECK(g.anschlaege[2] == Anschlag{12, Anschlagart::Hand, kSpreadMaxMs});
        CHECK(g.strum.spreadMs == doctest::Approx(0.0));
        CHECK(handAlsText(g.strum.hand) == QStringLiteral("UUUUUUU"));  // acht Stellen bei 4/4; die achte wechselt ohnehin ab

        // --- ein Drumkit traegt nichts davon
        Loop drums;
        Muster d;
        d.name = QStringLiteral("D");
        d.strum.spreadMs = 50.0;
        d.anschlaege = {Anschlag{0, Anschlagart::Auf}};
        drums.muster = {d};
        drums.richte();
        CHECK(drums.muster[0].strum == Strum{});
        CHECK(drums.muster[0].anschlaege.empty());
    }

    TEST_CASE("Loop Spielweise: .lvinst Fassung 11 — Strum am Instrument, Playing an der Saite, Vorgaben fuer ein Muster")
    {
        Instrument gitarre;
        gitarre.titel = QStringLiteral("Strummer");
        gitarre.schichten[0].schwingung = Schwingung::Saite;
        gitarre.schichten[0].saitenModell = SaitenModell::Wellenleiter;
        gitarre.saiten.leer = {40, 45, 50, 55, 59, 64};

        // --- die Vorgabe des Programms steht nicht da; Playing an jeder Saite Extended KS / Waveguide
        QJsonObject json = alsJson(gitarre);
        CHECK(json.value(QStringLiteral("version")).toInt() == 11);
        CHECK_FALSE(json.value(QStringLiteral("strings")).toObject().contains(QStringLiteral("strum")));
        const QJsonObject spiel = json.value(QStringLiteral("layers")).toArray().at(0).toObject().value(QStringLiteral("playing")).toObject();
        CHECK(spiel.value(QStringLiteral("upLevel")).toDouble() == doctest::Approx(gitarre.schichten[0].aufschlagLaut));
        CHECK(spiel.value(QStringLiteral("mute")).toDouble() == doctest::Approx(gitarre.schichten[0].handDaempfung));
        CHECK(spiel.value(QStringLiteral("tapLevel")).toDouble() == doctest::Approx(gitarre.schichten[0].tappingLaut));
        CHECK(spiel.value(QStringLiteral("touch")).toDouble() == doctest::Approx(gitarre.schichten[0].knotenDruck));

        // --- eine Saite Classic traegt Playing nicht (S-W6 a: dort keine Gruppe)
        Instrument klassisch = gitarre;
        klassisch.schichten[0].saitenModell = SaitenModell::Klassisch;
        CHECK_FALSE(alsJson(klassisch).value(QStringLiteral("layers")).toArray().at(0).toObject().contains(QStringLiteral("playing")));

        // --- gesetzt: hin und zurueck
        gitarre.saiten.strum.spreadMs = 12.0;
        gitarre.saiten.strum.hand = handAusText(QStringLiteral("DDUUDU"));
        gitarre.schichten[0].aufschlagLaut = 0.5;
        gitarre.schichten[0].handDaempfung = 0.9;
        gitarre.schichten[0].tappingLaut = 0.4;
        gitarre.schichten[0].knotenDruck = 0.7;
        richteSaiten(gitarre.saiten);
        json = alsJson(gitarre);
        CHECK(json.value(QStringLiteral("strings")).toObject().value(QStringLiteral("strum")).toObject().value(QStringLiteral("pattern")).toString() ==
              QStringLiteral("DDU"));  // gerichtet: U D U am Ende wechseln ohnehin ab
        Instrument zurueck;
        REQUIRE(ausJson(json, zurueck));
        CHECK(zurueck.saiten == gitarre.saiten);
        CHECK(zurueck.schichten[0] == gitarre.schichten[0]);

        // --- Werte ausserhalb der Grenzen; eine Datei der Fassung 10 kennt Playing nicht
        QJsonObject wild = json;
        QJsonArray schichten = wild.value(QStringLiteral("layers")).toArray();
        QJsonObject s = schichten[0].toObject();
        s[QStringLiteral("playing")] = QJsonObject{{QStringLiteral("upLevel"), 7.0}, {QStringLiteral("mute"), -1.0}};
        schichten[0] = s;
        wild[QStringLiteral("layers")] = schichten;
        REQUIRE(ausJson(wild, zurueck));
        CHECK(zurueck.schichten[0].aufschlagLaut == doctest::Approx(1.0));
        CHECK(zurueck.schichten[0].handDaempfung == doctest::Approx(0.0));
        CHECK(zurueck.schichten[0].knotenDruck == doctest::Approx(Schicht{}.knotenDruck));  // fehlt: die Vorgabe
        QJsonObject zehn = alsJson(Instrument{});
        zehn[QStringLiteral("version")] = 10;
        CHECK(ausJson(zehn, zurueck));
        CHECK(zurueck.saiten.strum == Strum{});
        QJsonObject morgen = json;
        morgen[QStringLiteral("version")] = kInstrumentFassung + 1;
        bool zuNeu = false;
        CHECK_FALSE(ausJson(morgen, zurueck, &zuNeu));
        CHECK(zuNeu);

        // --- ein Muster bekommt die Vorgaben seines Instruments: String choice und Strum
        gitarre.saiten.wahl = Saitenwahl::Hand;
        Muster m;
        m.art = MusterArt::Instrument;
        uebernimmVorgaben(m, gitarre.saiten);
        CHECK(m.saitenwahl == Saitenwahl::Hand);
        CHECK(m.strum == gitarre.saiten.strum);
        uebernimmVorgaben(m, Saitensatz{});  // ein Instrument ohne Saiten: die Vorgaben des Programms
        CHECK(m.saitenwahl == Saitenwahl::JeKlang);
        CHECK(m.strum == Strum{});
    }

    TEST_CASE("Loop Spielweise: die Abweichung eines Akkords wandert mit seinen Noten")
    {
        const auto mitAbweichungen = [] {
            Loop loop = probe();
            setzeAnschlag(riff(loop), Anschlag{0, Anschlagart::Auf});
            setzeAnschlag(riff(loop), Anschlag{8, Anschlagart::Zugleich, 60.0});
            return loop;
        };
        const auto orteAn = [](const Muster& m, int schritt) {
            std::vector<NotenOrt> orte;
            for (const Note& note : m.noten)
                if (note.schritt == schritt) orte.push_back({note.schritt, note.hoehe});
            return orte;
        };
        constexpr int kSchritte = 16;

        SUBCASE("setzen, ersetzen, entfernen")
        {
            Loop loop = mitAbweichungen();
            Muster& m = riff(loop);
            CHECK(schritteVon(m) == std::vector<int>{0, 8});
            CHECK(anschlagAn(m, 0)->art == Anschlagart::Auf);
            CHECK(anschlagAn(m, 4) == nullptr);
            setzeAnschlag(m, Anschlag{0, Anschlagart::Ab});  // ersetzt
            CHECK(m.anschlaege.size() == 2);
            CHECK(anschlagAn(m, 0)->art == Anschlagart::Ab);
            setzeAnschlag(m, Anschlag{0});  // weicht nicht ab: entfernt
            CHECK(schritteVon(m) == std::vector<int>{8});
            setzeAnschlag(m, Anschlag{6, Anschlagart::Auf});  // dort schlaegt nichts an
            CHECK(schritteVon(m) == std::vector<int>{8});
        }

        SUBCASE("kopiereNoten und fuegeNotenEin: die Kopie traegt die Abweichung, einmal je Akkord")
        {
            Loop loop = mitAbweichungen();
            Muster& m = riff(loop);
            const NotenKopie kopie = kopiereNoten(m, orteAn(m, 8));
            REQUIRE(kopie.anschlaege.size() == 1);
            CHECK(kopie.anschlaege[0] == Anschlag{0, Anschlagart::Zugleich, 60.0});  // ab dem Anfang der Kopie
            // eingefuegt an Schritt 4: ersetzt dort die Vorgabe des Akkords
            (void)fuegeNotenEin(m, kopie, 4, kSchritte);
            CHECK(*anschlagAn(m, 4) == Anschlag{4, Anschlagart::Zugleich, 60.0});
            // eine Kopie ohne Abweichung laesst die am Ziel stehen
            const NotenKopie ohne = kopiereNoten(m, orteAn(m, 12));
            CHECK(ohne.anschlaege.empty());
            (void)fuegeNotenEin(m, ohne, 0, kSchritte);
            CHECK(anschlagAn(m, 0)->art == Anschlagart::Auf);
            // hinter dem Ende des Musters geht nichts an
            (void)fuegeNotenEin(m, kopie, 20, kSchritte);
            CHECK(schritteVon(m) == std::vector<int>{0, 4, 8});
        }

        SUBCASE("loescheNoten: ein Akkord ohne Noten hat keine Abweichung mehr — einer mit Resten behaelt sie")
        {
            Loop loop = mitAbweichungen();
            Muster& m = riff(loop);
            CHECK(loescheNoten(m, {{8, 43}, {8, 47}}));
            CHECK(anschlagAn(m, 8) != nullptr);  // die Note 55 schlaegt noch an
            CHECK(loescheNoten(m, orteAn(m, 8)));
            CHECK(schritteVon(m) == std::vector<int>{0});
        }

        SUBCASE("versetzeNoten: der ganze Akkord nimmt sie mit, ein Teil nimmt sie mit und laesst sie stehen")
        {
            Loop loop = mitAbweichungen();
            Muster& m = riff(loop);
            std::vector<NotenOrt> orte = orteAn(m, 0);
            REQUIRE(versetzeNoten(m, orte, 2, 0, kSchritte));
            CHECK(schritteVon(m) == std::vector<int>{2, 8});
            CHECK(anschlagAn(m, 2)->art == Anschlagart::Auf);
            // zwei der drei Noten an 8 nach 10 (eine Tonhoehe hoeher): beide Schritte tragen sie
            std::vector<NotenOrt> teil{{8, 43}, {8, 47}};
            REQUIRE(versetzeNoten(m, teil, 2, 1, kSchritte));
            CHECK(schritteVon(m) == std::vector<int>{2, 8, 10});
            CHECK(*anschlagAn(m, 10) == Anschlag{10, Anschlagart::Zugleich, 60.0});
            // ein Akkord ohne Abweichung, der auf einen mit Abweichung trifft: diese bleibt
            std::vector<NotenOrt> zwoelf = orteAn(m, 12);
            REQUIRE(versetzeNoten(m, zwoelf, -2, 0, kSchritte));
            CHECK(anschlagAn(m, 10)->art == Anschlagart::Zugleich);
            // ein Akkord mit Abweichung, der auf einen trifft: seine ersetzt die dort
            std::vector<NotenOrt> zwei = orteAn(m, 2);
            REQUIRE(versetzeNoten(m, zwei, 8, 0, kSchritte));
            CHECK(anschlagAn(m, 10)->art == Anschlagart::Auf);
            CHECK(anschlagAn(m, 2) == nullptr);
        }

        SUBCASE("dreheNoten: im Kreis — die Abweichung mit")
        {
            Loop loop = mitAbweichungen();
            Muster& m = riff(loop);
            std::vector<NotenOrt> alle;
            for (const Note& note : m.noten) alle.push_back({note.schritt, note.hoehe});
            REQUIRE(dreheNoten(m, alle, 6, kSchritte));
            CHECK(schritteVon(m) == std::vector<int>{6, 14});
            CHECK(anschlagAn(m, 6)->art == Anschlagart::Auf);
            CHECK(anschlagAn(m, 14)->art == Anschlagart::Zugleich);
        }

        SUBCASE("setzeMusterTakte mit Repeat when lengthening: die Abweichungen wiederholen sich")
        {
            Loop loop = mitAbweichungen();
            REQUIRE(setzeMusterTakte(loop, QStringLiteral("Strum"), 3, true));
            CHECK(schritteVon(riff(loop)) == std::vector<int>{0, 8, 16, 24, 32, 40});
            CHECK(anschlagAn(riff(loop), 40)->art == Anschlagart::Zugleich);
            // ohne Wiederholen: die neuen Takte sind leer, also auch ohne Abweichung
            Loop ohne = mitAbweichungen();
            REQUIRE(setzeMusterTakte(ohne, QStringLiteral("Strum"), 3, false));
            CHECK(schritteVon(riff(ohne)) == std::vector<int>{0, 8});
        }

        SUBCASE("setzeRaster: die Abweichung bleibt an ihrer Stelle im Takt — oder geht mit ihrer Note")
        {
            Loop loop = mitAbweichungen();
            REQUIRE(setzeRaster(loop, 4, 2));  // Achtel: Schritt 8 (Schlag 3) wird 4
            CHECK(schritteVon(riff(loop)) == std::vector<int>{0, 4});
            Loop drei = mitAbweichungen();
            REQUIRE(setzeRaster(drei, 2, 4));  // nur zwei Schlaege: Schlag 3 geht mit seinen Noten
            CHECK(schritteVon(riff(drei)) == std::vector<int>{0});
        }

        SUBCASE("das Ziehen einer einzelnen Note im Editor — auch ueber einen anderen Akkord hinweg")
        {
            int argc = 1;
            char arg0[10] = "UnitTests";
            char* argv[2] = {arg0, nullptr};
            QApplication app{argc, argv};

            Loop loop = probe();
            Muster& m = riff(loop);
            REQUIRE(loescheNoten(m, {{12, 64}}));
            REQUIRE(fuegeNotenEin(m, NotenKopie{{Note{0, 1, 60}}, 1, {}}, 12, kSchritte).weg == 0);
            setzeAnschlag(m, Anschlag{12, Anschlagart::Auf});
            setzeAnschlag(m, Anschlag{8, Anschlagart::Zugleich});
            loop.richte();

            NotenRaster raster;  // nicht gezeigt: kein Fenster
            raster.resize(NotenRaster::kTastenBreite + 16 * NotenRaster::kSchrittBreite,
                          NotenRaster::kKopfHoehe + 30 * NotenRaster::kZeilenHoehe + NotenRaster::kLautHoehe);
            Muster gemeldet = riff(loop);
            raster.setMuster(gemeldet, 4, 4);
            QObject::connect(&raster, &NotenRaster::geaendert, &raster, [&](const Muster& neu, const QString&) { gemeldet = neu; });
            raster.zeigeHoehe(60);
            const auto ort = [&](int schritt, int hoehe) {
                return QPoint(raster.xVonSchritt(schritt) + 4, raster.yVonHoehe(hoehe) + 5);
            };
            // die Note an 12 greifen, ueber den Akkord an 8 hinweg nach 2 ziehen
            maus(raster, QEvent::MouseButtonPress, ort(12, 60), Qt::LeftButton, Qt::LeftButton);
            maus(raster, QEvent::MouseMove, ort(8, 60), Qt::NoButton, Qt::LeftButton);
            CHECK(anschlagAn(raster.muster(), 8)->art == Anschlagart::Auf);  // unterwegs: sie bringt ihre mit
            maus(raster, QEvent::MouseMove, ort(2, 60), Qt::NoButton, Qt::LeftButton);
            maus(raster, QEvent::MouseButtonRelease, ort(2, 60), Qt::LeftButton, Qt::NoButton);
            // der Akkord an 8 hat wieder seine eigene; die Note an 2 traegt ihre, an 12 steht keine mehr
            CHECK(schritteVon(gemeldet) == std::vector<int>{2, 8});
            CHECK(anschlagAn(gemeldet, 2)->art == Anschlagart::Auf);
            CHECK(anschlagAn(gemeldet, 8)->art == Anschlagart::Zugleich);
        }
    }
}
