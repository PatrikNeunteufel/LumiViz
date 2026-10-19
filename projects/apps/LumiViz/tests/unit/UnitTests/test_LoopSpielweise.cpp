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
#include "loop/Rechenkern.hpp"
#include "loop/Saitenplan.hpp"
#include "loop/Spielweise.hpp"

#include "LoopOhneEngine.hpp"  // repoWurzel

#include <QApplication>
#include <QDir>
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

/// Eine Gitarre in Standardstimmung, wie die Planung sie vom Instrument erfaehrt.
Saitensatz gitarre()
{
    Saitensatz s;
    s.leer = {40, 45, 50, 55, 59, 64};
    return s;
}

SaitenVon nurGitarre()
{
    return [](const QString& instrument) { return instrument == QLatin1String("guitar") ? gitarre() : Saitensatz{}; };
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
        // --- ohne Spielweisen: Strum steht trotzdem da (S-W11 a), Abweichungen und sechste Werte nicht
        Loop schlicht = probe();
        const QJsonObject ohne = musterJson(alsJson(schlicht));
        CHECK(alsJson(schlicht).value(QStringLiteral("version")).toInt() == 10);
        CHECK(ohne.value(QStringLiteral("strum")).toObject() ==
              QJsonObject{{QStringLiteral("spread"), kSpreadVorgabeMs}, {QStringLiteral("hand"), QStringLiteral("halfBeat")}});
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
        wildNoten[1] = QJsonArray{0, 2, 47, 1.0, -1, 0x3C0 | kTapping};  // Bits von morgen (seit S136 sind 16 und 32 bekannt)
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

        // --- Strum steht auch mit der Vorgabe da (S-W11 a); Playing an jeder Saite Extended KS / Waveguide
        QJsonObject json = alsJson(gitarre);
        CHECK(json.value(QStringLiteral("version")).toInt() == 11);
        CHECK(json.value(QStringLiteral("strings")).toObject().value(QStringLiteral("strum")).toObject() == strumAlsJson(Strum{}));
        CHECK_FALSE(alsJson(Instrument{}).contains(QStringLiteral("strings")));  // ohne Saiten kein Block
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

    TEST_CASE("Loop Spielweise 24b: Richtung und Versatz je Akkord — aus der Hand, aus der Abweichung")
    {
        // probe(): Akkorde an 0 (Saiten 0, 1, 2), 4 (1, 2), 8 (0, 1, 3), eine Note an 12 (5).
        // Spread 8 ms je Saite (S-W12 a).
        Loop loop = probe();
        Muster& m = riff(loop);
        const std::vector<Griff> griffe = greifeNoten(m, gitarre());
        REQUIRE(griffe.size() == 9);
        CHECK(griffe[0].saite == 0);
        CHECK(griffe[1].saite == 1);
        CHECK(griffe[2].saite == 2);
        CHECK(griffe[7].saite == 3);
        const auto versatz = [&](const std::vector<Anschlagplan>& plan) {
            std::vector<double> ms;
            for (const Anschlagplan& a : plan) ms.push_back(a.versatzMs);
            return ms;
        };

        // --- die Vorgabe (S-W10 a): an — 0, 4, 8 sind Stellen der Hand nach unten (1/2 beat, gerade Stellen)
        std::vector<Anschlagplan> plan = schlageAn(m, gitarre(), griffe, 4, 4);
        CHECK(versatz(plan) == std::vector<double>{0, 8, 16, 0, 8, 0, 8, 24, 0});  // an 8 ueberspringt die Hand Saite 2
        for (const Anschlagplan& a : plan) CHECK(a.richtung == Richtung::Ab);

        // --- die Abweichungen: Up an 4, Together an 8, an 0 die Hand mit eigenem Spread 50 ms
        setzeAnschlag(m, Anschlag{4, Anschlagart::Auf});
        setzeAnschlag(m, Anschlag{8, Anschlagart::Zugleich});
        setzeAnschlag(m, Anschlag{0, Anschlagart::Hand, 50.0});
        plan = schlageAn(m, gitarre(), griffe, 4, 4);
        CHECK(versatz(plan) == std::vector<double>{0, 50, 100, 8, 0, 0, 0, 0, 0});
        CHECK(plan[3].richtung == Richtung::Auf);  // Up: die hohe Saite zuerst
        CHECK(plan[4].richtung == Richtung::Auf);
        CHECK(plan[5].richtung == Richtung::Ab);   // Together klingt wie Down
        CHECK(plan[0].richtung == Richtung::Ab);

        // --- das Muster der Hand: Stelle 6 (Schritt 12) nach oben; Spread 0 = zugleich
        Loop zwei = probe();
        Muster& n = riff(zwei);
        n.strum.hand = handAusText(QStringLiteral("DDDDDDUD"));
        plan = schlageAn(n, gitarre(), griffe, 4, 4);
        CHECK(plan[8].richtung == Richtung::Auf);
        CHECK(plan[0].richtung == Richtung::Ab);
        n.strum.spreadMs = 0.0;
        plan = schlageAn(n, gitarre(), griffe, 4, 4);
        for (const Anschlagplan& a : plan) CHECK(a.versatzMs == doctest::Approx(0.0));
        // im Raster Beat ist Schritt 12 Stelle 3 — abwechselnd Up
        n.strum = Strum{};
        n.strum.raster = HandRaster::Schlag;
        plan = schlageAn(n, gitarre(), griffe, 4, 4);
        CHECK(plan[3].richtung == Richtung::Auf);  // Schritt 4 = Schlag 2
        CHECK(plan[8].richtung == Richtung::Auf);

        // --- ohne Saiten: nichts
        plan = schlageAn(m, Saitensatz{}, std::vector<Griff>(m.noten.size()), 4, 4);
        for (const Anschlagplan& a : plan) CHECK(a == Anschlagplan{});
    }

    TEST_CASE("Loop Spielweise 24b: Harmonic — natuerliche Flageoletts vor kuenstlichen (S-W7 a)")
    {
        // --- die Teiltoene ueber dem Grundton
        CHECK(teiltonHalbtoene(1) == 0);
        CHECK(teiltonHalbtoene(2) == 12);
        CHECK(teiltonHalbtoene(3) == 19);
        CHECK(teiltonHalbtoene(4) == 24);
        CHECK(teiltonHalbtoene(5) == 28);
        CHECK(teiltonHalbtoene(6) == 31);
        CHECK(flageolettAuf(gitarre(), 0, 68) == Flageolett{5, 0, false});  // G#4 auf der tiefen E-Saite
        CHECK(flageolettAuf(gitarre(), 1, 61) == Flageolett{2, 4, true});   // C#4: gegriffen im 4. Bund der A-Saite
        CHECK(flageolettAuf(gitarre(), 4, 59) == Flageolett{});              // die leere Saite selbst ist kein Flageolett
        CHECK(flageolettAuf(gitarre(), 5, 100) == Flageolett{});             // der Knoten laege ueber dem 24. Bund

        // --- die Wahl der Saite: je Note fuer sich (verschiedene Schritte)
        Muster m;
        m.art = MusterArt::Instrument;
        m.instrument = QStringLiteral("guitar");
        m.noten = {Note{0, 1, 52, 1.0, -1, kHarmonic},   // E3: n = 2 auf der tiefen E-Saite
                   Note{2, 1, 59, 1.0, -1, kHarmonic},   // B3: n = 3 auf E — vor dem kuenstlichen auf A
                   Note{4, 1, 68, 1.0, -1, kHarmonic},   // G#4: n = 5 auf E
                   Note{6, 1, 62, 1.0, -1, kHarmonic},   // D4: n = 2 auf der D-Saite
                   Note{8, 1, 61, 1.0, -1, kHarmonic},   // C#4: kein natuerliches — kuenstlich, die tiefste Lage
                   Note{10, 1, 100, 1.0, -1, kHarmonic},  // E7: nirgends
                   Note{12, 1, 52},                        // E3 ohne Harmonic: gegriffen wie bisher
                   Note{14, 1, 52, 1.0, 1, kHarmonic}};    // gesetzt auf A, dort kein Flageolett: es gilt die Wahl
        const std::vector<Griff> griffe = greifeNoten(m, gitarre());
        REQUIRE(griffe.size() == 8);
        CHECK(griffe[0] == Griff{0, 0, true, 2});
        CHECK(griffe[1] == Griff{0, 0, true, 3});
        CHECK(griffe[2] == Griff{0, 0, true, 5});
        CHECK(griffe[3] == Griff{2, 0, true, 2});
        CHECK(griffe[4] == Griff{1, 4, true, 2});
        CHECK(griffe[5] == Griff{5, 36, false, 1});  // nicht greifbar: als gewoehnlicher Ton auf der naechstbesten Saite
        CHECK(griffe[6] == Griff{2, 2, true, 1});
        CHECK(griffe[7] == Griff{0, 0, true, 2});
        // gesetzt, wo es eines gibt: es gilt
        m.noten[7].saite = 0;
        CHECK(greifeNoten(m, gitarre())[7] == Griff{0, 0, true, 2});
    }

    TEST_CASE("Loop Spielweise 24b: die Toene je Vorkommen — Versatz, Richtung, Artikulationen, Let ring; plane setzt den Versatz")
    {
        Loop loop = probe();
        Muster& m = riff(loop);
        m.noten.push_back(Note{14, 2, 52, 1.0, -1, kHarmonic});  // Schritt 14: Stelle 7, Up
        m.noten[2].artikulationen = kLetRing;                     // E3 im ersten Akkord, Saite 2
        m.noten[8].artikulationen = kPalmMute | kTapping;         // E4 an 12
        loop.richte();
        REQUIRE(m.noten.size() == 10);
        REQUIRE(loop.durchlaufMs() == 2000.0);

        const std::vector<ClipToene> toene = toeneImKanal(loop, 0, nurGitarre());
        REQUIRE(toene.size() == 1);
        REQUIRE(toene[0].size() == 1);
        const std::vector<GeplanterTon>& t = toene[0][0];
        CHECK(t[1].versatzMs == doctest::Approx(8.0));
        CHECK(t[2].versatzMs == doctest::Approx(16.0));
        // Let ring: gehalten, bis die E3 des Akkords an 4 dieselbe Saite anschlaegt (508 ms) — 492 ms nach ihrem Anschlag
        CHECK(t[2].endeMs == 492);
        CHECK(t[2].dauerMs == 492);
        CHECK(t[1].dauerMs == 250);  // ohne Let ring: ihre Laenge
        // Palm mute und Tapping im Namen; Let ring nicht (es steckt in der Dauer)
        CHECK(t[8].artikulationen == (kPalmMute | kTapping));
        CHECK(t[2].artikulationen == 0);
        CHECK(tonKlang(t[8]).contains(QStringLiteral(":m:t")));
        // das Flageolett an 14: E3 als Teilton 2 der tiefen E-Saite, nach oben angeschlagen
        CHECK(t[9].teilton == 2);
        CHECK(t[9].saite == 0);
        CHECK(t[9].richtung == Richtung::Auf);
        CHECK(tonKlang(t[9]) == QStringLiteral("inst:guitar:52:250:s0:h2:u:e250"));

        // --- der Name zerlegt sich wieder in dieselbe Angabe
        TonAngabe zurueck;
        REQUIRE(zerlegeTon(tonKlang(t[9]), zurueck));
        CHECK(zurueck == static_cast<const TonAngabe&>(t[9]));
        REQUIRE(zerlegeTon(tonKlang(t[8]), zurueck));
        CHECK(zurueck == static_cast<const TonAngabe&>(t[8]));
        REQUIRE(zerlegeTon(QStringLiteral("inst:C:/ich/eigen.lvinst:52:250:s0:h3:u:m:t:e40"), zurueck));
        CHECK(zurueck == TonAngabe{QStringLiteral("C:/ich/eigen.lvinst"), 52, 250, 0, 40, Richtung::Auf, kPalmMute | kTapping, 3});

        // --- plane: jeder Schlag dort, wo sein Schritt ist, und um seinen Versatz spaeter (8000 Hz: 8 Abtastwerte je ms)
        const Ablauf ablauf = plane(loop, 8000, nurGitarre());
        for (std::size_t i = 0; i < t.size(); ++i)
        {
            const int klang = static_cast<int>(ablauf.klangNamen.indexOf(tonKlang(t[i])));
            REQUIRE(klang >= 0);
            const std::int64_t erwartet = std::llround((m.noten[i].schritt * 125.0 + t[i].versatzMs) * 8.0);
            bool gefunden = false;
            for (const Stimme& stimme : ablauf.stimmen)
                if (stimme.klang == klang)
                    for (const Schlag& schlag : stimme.schlaege) gefunden = gefunden || schlag.frame == erwartet;
            CAPTURE(i);
            CHECK(gefunden);
        }
        CHECK(ablauf.klangNamen == loop.klangNamen(nurGitarre()));
    }

    TEST_CASE("Loop Spielweise: die vier Beispiele der Spielweisen (S136, S-W22 a) spielen, was sie zeigen")
    {
        // aus der Datei, mit den Saiten der mitgelieferten Instrumente — wie das Programm sie plant
        const QDir ordner(looptest::repoWurzel().filePath(QStringLiteral("asset/loop")));
        const auto saitenVon = [&ordner](const QString& name) {
            Instrument instrument;
            REQUIRE(ladeInstrument(ordner.filePath(QStringLiteral("instruments/%1.lvinst").arg(name)), instrument));
            return instrument.saiten;
        };
        const auto lade = [&ordner](const char* datei) {
            Loop loop;
            REQUIRE(ladeLoop(ordner.filePath(QString::fromLatin1(datei)), loop));
            CHECK(loop.muster.size() >= 2);
            return loop;
        };
        const auto muster = [](const Loop& loop, const char* name) -> const Muster& {
            const auto it = std::find_if(loop.muster.begin(), loop.muster.end(),
                                         [name](const Muster& m) { return m.name == QString::fromLatin1(name); });
            REQUIRE(it != loop.muster.end());
            return *it;
        };
        const auto zaehle = [](const Muster& m, int bits) {
            return std::count_if(m.noten.begin(), m.noten.end(), [bits](const Note& n) { return (n.artikulationen & bits) == bits; });
        };

        // --- strum_demo: Spread der Guitar, Together auf der Eins des vierten Takts, Schluss mit 40 ms
        {
            const Loop loop = lade("strum_demo.lvloop");
            const Muster& gitarre = muster(loop, "Guitar");
            CHECK(gitarre.strum.spreadMs == doctest::Approx(7.5));
            CHECK(gitarre.strum.hand.empty());  // D D U U D U ist die abwechselnde Hand
            REQUIRE(gitarre.anschlaege.size() == 2);
            CHECK(gitarre.anschlaege[0] == Anschlag{48, Anschlagart::Zugleich, -1.0});
            CHECK(gitarre.anschlaege[1] == Anschlag{60, Anschlagart::Hand, 40.0});
            const Saitensatz saiten = saitenVon(gitarre.instrument);
            const auto griffe = greifeNoten(gitarre, saiten);
            CHECK(std::all_of(griffe.begin(), griffe.end(), [](const Griff& g) { return g.greifbar; }));
            const auto plan = schlageAn(gitarre, saiten, griffe, loop.schlaegeJeTakt, loop.schritteJeSchlag);
            int auf = 0;
            for (std::size_t i = 0; i < plan.size(); ++i)
                if (plan[i].richtung == Richtung::Auf) ++auf;
            CHECK(auf > 0);  // die Ups der Hand kommen an
        }
        // --- palm_mute_demo: gedaempfte Achtel, nur Downstrokes, der Schluss klingt aus
        {
            const Loop loop = lade("palm_mute_demo.lvloop");
            const Muster& riff = muster(loop, "Riff");
            CHECK(zaehle(riff, kPalmMute) == 3 * 7 * 2);
            CHECK(zaehle(riff, kLetRing) == 3);
            CHECK(handAlsText(riff.strum.hand) == QStringLiteral("DDDDDDDD"));
            CHECK(zaehle(muster(loop, "Bass"), kPalmMute) == 3 * 7);
            const auto plan = schlageAn(riff, saitenVon(riff.instrument), greifeNoten(riff, saitenVon(riff.instrument)),
                                        loop.schlaegeJeTakt, loop.schritteJeSchlag);
            CHECK(std::all_of(plan.begin(), plan.end(), [](const Anschlagplan& a) { return a.richtung == Richtung::Ab; }));
        }
        // --- harmonics_demo: jedes Flageolett ist ein natuerliches auf einer leeren Saite
        {
            const Loop loop = lade("harmonics_demo.lvloop");
            const Muster& glocke = muster(loop, "Harmonics");
            CHECK(zaehle(glocke, kHarmonic | kLetRing) == static_cast<long long>(glocke.noten.size()));
            const auto griffe = greifeNoten(glocke, saitenVon(glocke.instrument));
            REQUIRE(griffe.size() == glocke.noten.size());
            for (const Griff& g : griffe)
            {
                CHECK(g.greifbar);
                CHECK(g.teilton >= 2);
                CHECK(g.bund == 0);
            }
            CHECK(zaehle(muster(loop, "Arpeggio"), kLetRing) == 32);
        }
        // --- tapping_demo: der Lauf auf der hohen E-Saite, zwei Takte getippt, zwei gezupft
        {
            const Loop loop = lade("tapping_demo.lvloop");
            const Muster& lauf = muster(loop, "Tapping");
            CHECK(zaehle(lauf, kTapping) == 32);
            CHECK(lauf.noten.size() == 64);
            const auto griffe = greifeNoten(lauf, saitenVon(lauf.instrument));
            CHECK(std::all_of(griffe.begin(), griffe.end(), [](const Griff& g) { return g.greifbar && g.saite == 5; }));
        }
    }

    TEST_CASE("Loop Spielweise 25a: Legato, Slide und Bend — das Modell, die Grenzen, .lvloop Fassung 10")
    {
        // --- die Bits: zwei neue, ein unbekanntes faellt beim Richten weg
        CHECK(kArtikulationenAlle == 63);
        CHECK(kVerbunden == (kLegato | kSlide));

        // --- die Kurve: Grenzen, Raster in Vierteln, sortiert, dieselbe Zeit — der spaetere gilt
        std::vector<BendPunkt> bend{{0.5, 2.1}, {-0.2, 0.0}, {0.5, 1.9}, {2.0, 9.0}};
        richteBend(bend);
        REQUIRE(bend.size() == 3);
        CHECK(bend[0] == BendPunkt{0.0, 0.0});
        CHECK(bend[1] == BendPunkt{0.5, 2.0});  // 1,9 im Raster
        CHECK(bend[2] == BendPunkt{1.0, kBendHalbtoeneMax});
        // ohne Biegung: kein Bend
        std::vector<BendPunkt> flach{{0.0, 0.0}, {1.0, 0.1}};
        richteBend(flach);
        CHECK(flach.empty());
        // hoechstens kBendPunkteMax
        std::vector<BendPunkt> viele;
        for (int i = 0; i < 40; ++i) viele.push_back({i / 40.0, 1.0});
        richteBend(viele);
        CHECK(viele.size() == static_cast<std::size_t>(kBendPunkteMax));

        // --- die Biegung ueber die Zeit: linear zwischen den Punkten, davor und danach gehalten
        const std::vector<BendPunkt> hoch{{0.25, 0.0}, {0.5, 2.0}, {1.0, 2.0}};
        CHECK(bendBei(hoch, 0.0) == 0.0);
        CHECK(bendBei(hoch, 0.375) == doctest::Approx(1.0));
        CHECK(bendBei(hoch, 0.75) == doctest::Approx(2.0));
        CHECK(bendBei({}, 0.5) == 0.0);
        // ein Pre-bend: am Anfang schon gebogen, ein Sprung an derselben Zeit
        const std::vector<BendPunkt> vor{{0.0, 2.0}, {0.5, 2.0}, {1.0, 0.0}};
        CHECK(bendBei(vor, 0.0) == 2.0);
        CHECK(bendBei(vor, 0.75) == doctest::Approx(1.0));

        // --- die Datei: Legato und Slide als Bits, der Bend als siebter Wert, hin und zurueck
        Loop loop = probe();
        Muster& m = loop.muster[0];
        m.noten.push_back(Note{14, 2, 66, 0.8, -1, kLegato});
        Note gebogen{13, 3, 57, 0.9, 2, kSlide};
        gebogen.bend = {{0.0, 0.0}, {0.3, 2.0}, {1.0, 2.0}};
        m.noten.push_back(gebogen);
        loop.richte();
        const QJsonObject json = alsJson(loop);
        CHECK(json.value(QStringLiteral("version")).toInt() == 10);
        const QJsonArray noten = json.value(QStringLiteral("patterns")).toArray()[0].toObject().value(QStringLiteral("notes")).toArray();
        bool legatoGesehen = false, bendGesehen = false;
        for (const QJsonValue& v : noten)
        {
            const QJsonArray zeile = v.toArray();
            if (zeile[2].toInt() == 66)
            {
                legatoGesehen = true;
                CHECK(zeile.size() == 6);  // ohne Bend: Saite −1 und die Bits
                CHECK(zeile[5].toInt() == kLegato);
            }
            if (zeile[2].toInt() == 57 && zeile[0].toInt() == 13)
            {
                bendGesehen = true;
                REQUIRE(zeile.size() == 7);
                CHECK(zeile[6].toArray().size() == 3);
            }
        }
        CHECK(legatoGesehen);
        CHECK(bendGesehen);
        Loop zurueck;
        REQUIRE(ausJson(json, zurueck));
        CHECK(zurueck == loop);

        // --- eine Note nur mit Bend: Saite und Bits stehen trotzdem davor
        Loop nurBend = probe();
        nurBend.muster[0].noten[8].bend = {{0.0, 0.0}, {1.0, 1.0}};
        const QJsonArray zeilen =
            alsJson(nurBend).value(QStringLiteral("patterns")).toArray()[0].toObject().value(QStringLiteral("notes")).toArray();
        bool gesehen = false;
        for (const QJsonValue& v : zeilen)
            if (v.toArray().size() == 7)
            {
                gesehen = true;
                CHECK(v.toArray()[4].toInt() == -1);
                CHECK(v.toArray()[5].toInt() == 0);
            }
        CHECK(gesehen);

        // --- ein unbekanntes Bit (64) faellt weg, ein Bend ohne Biegung auch
        QJsonObject fremd = alsJson(probe());
        QJsonArray muster = fremd.value(QStringLiteral("patterns")).toArray();
        QJsonObject erstes = muster[0].toObject();
        QJsonArray mitFremdem = erstes.value(QStringLiteral("notes")).toArray();
        mitFremdem.append(QJsonArray{2, 1, 50, 1.0, -1, 64 | kSlide, QJsonArray{QJsonArray{0.0, 0.0}}});
        erstes[QStringLiteral("notes")] = mitFremdem;
        muster[0] = erstes;
        fremd[QStringLiteral("patterns")] = muster;
        Loop gelesen;
        REQUIRE(ausJson(fremd, gelesen));
        const auto it = std::find_if(gelesen.muster[0].noten.begin(), gelesen.muster[0].noten.end(),
                                     [](const Note& n) { return n.schritt == 2 && n.hoehe == 50; });
        REQUIRE(it != gelesen.muster[0].noten.end());
        CHECK(it->artikulationen == kSlide);
        CHECK(it->bend.empty());

        // --- der Bend wandert mit seiner Note: verschieben, kopieren und einfuegen, drehen
        Muster w = probe().muster[0];
        constexpr int kSchritte = 16;
        w.noten[8].bend = {{0.0, 0.0}, {0.5, 1.0}, {1.0, 1.0}};  // die Note an 12 (E4)
        w.noten[8].artikulationen = kLegato;
        std::vector<NotenOrt> orte{{12, 64}};
        REQUIRE(versetzeNoten(w, orte, 1, 2, kSchritte));
        const auto da = std::find_if(w.noten.begin(), w.noten.end(), [](const Note& n) { return n.schritt == 13; });
        REQUIRE(da != w.noten.end());
        CHECK(da->hoehe == 66);
        CHECK(da->bend.size() == 3);
        CHECK(da->artikulationen == kLegato);
        const std::vector<BendPunkt> erwartet = da->bend;  // `da` gilt nach dem Einfuegen nicht mehr
        const NotenKopie kopie = kopiereNoten(w, {{13, 66}});
        (void)fuegeNotenEin(w, kopie, 2, kSchritte);
        const auto kopiert = std::find_if(w.noten.begin(), w.noten.end(), [](const Note& n) { return n.schritt == 2 && n.hoehe == 66; });
        REQUIRE(kopiert != w.noten.end());
        CHECK(kopiert->bend == erwartet);
        CHECK(kopiert->artikulationen == kLegato);
        std::vector<NotenOrt> zuDrehen{{2, 66}};
        REQUIRE(dreheNoten(w, zuDrehen, 4, kSchritte));
        const auto gedreht = std::find_if(w.noten.begin(), w.noten.end(), [](const Note& n) { return n.schritt == 6 && n.hoehe == 66; });
        REQUIRE(gedreht != w.noten.end());
        CHECK(gedreht->bend.size() == 3);
    }
}
