/**
 ****************************************************************************************
 * @file   test_LoopAuswahl.cpp
 * @brief  Loop: mehrere Schritte oder Noten eines Musters zugleich — die Regeln
 *         (docs/audio/Loop_Konzept.md §4i, Stufe 13; Entscheide V1 a, V2 a, V3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Reine Funktionen auf dem Modell: ohne Ton, ohne Fenster, ohne Qt-Anwendung.
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Auswahl.hpp"
#include "loop/Loop.hpp"

#include <vector>

using namespace lumi::loop;

namespace
{

/// Ein Drumkit-Muster mit drei Spuren zu je acht Schritten:
/// Kick  1 . . . 1 . . .
/// Snare . . 0,5 . . . 1 .
/// Hat   . . . . . . . 1
Muster drumMuster()
{
    Muster m;
    m.name = QStringLiteral("M");
    m.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
                {QStringLiteral("Snare"), QStringLiteral("kit:snare"), 1.0, 0.0, false},
                {QStringLiteral("Hat"), QStringLiteral("kit:hat_closed"), 1.0, 0.0, false}};
    m.zeilen = {{1, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0.5, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0, 1}};
    return m;
}

/// Ein Instrument-Muster von 16 Schritten: C4 ab 0 (2 lang), E4 ab 2 (1), G4 ab 4 (4), C5 ab 12 (4).
Muster notenMuster()
{
    Muster m;
    m.name = QStringLiteral("N");
    m.art = MusterArt::Instrument;
    m.noten = {{0, 2, 60, 1.0}, {2, 1, 64, 0.5}, {4, 4, 67, 0.8}, {12, 4, 72, 1.0}};
    return m;
}

constexpr int kNotenSchritte = 16;

}  // namespace

TEST_CASE("Loop Auswahl Schritte: kopieren nimmt genau die markierten, mit ihrer Lage zueinander")
{
    const Muster m = drumMuster();
    // ein Bereich: Kick und Snare, Schritte 0 bis 3 — auch die leeren zaehlen
    std::vector<SchrittOrt> bereich;
    for (int spur = 0; spur < 2; ++spur)
        for (int schritt = 0; schritt < 4; ++schritt) bereich.push_back({spur, schritt});
    const SchrittKopie kopie = kopiereSchritte(m, bereich);
    CHECK(kopie.teile.size() == 8u);
    CHECK(kopie.spuren == 2);
    CHECK(kopie.schritte == 4);
    CHECK(kopie.teile[0] == SchrittKopie::Teil{0, 0, 1.0});
    CHECK(kopie.teile[6] == SchrittKopie::Teil{1, 2, 0.5});  // die Lautstaerke geht mit
    CHECK(anfangVon(bereich) == SchrittOrt{0, 0});

    // verstreut (V2 a): Snare Schritt 6 und Hat Schritt 7 — der Anfang ist die Ecke oben links davon
    const std::vector<SchrittOrt> verstreut = {{2, 7}, {1, 6}, {1, 6}, {9, 9}};  // doppelt und ungueltig zaehlen nicht
    const SchrittKopie zwei = kopiereSchritte(m, verstreut);
    CHECK(zwei.teile.size() == 2u);
    CHECK(zwei.spuren == 2);
    CHECK(zwei.schritte == 2);
    CHECK(zwei.teile[0] == SchrittKopie::Teil{1, 1, 1.0});
    CHECK(zwei.teile[1] == SchrittKopie::Teil{0, 0, 1.0});
    CHECK(kopiereSchritte(m, {}).leer());
    CHECK(kopiereSchritte(m, {{5, 0}}).leer());
}

TEST_CASE("Loop Auswahl Schritte: einfuegen ersetzt den kopierten Bereich — oder legt nur dazu")
{
    Muster m = drumMuster();
    std::vector<SchrittOrt> ersterHalbtakt;
    for (int spur = 0; spur < 2; ++spur)
        for (int schritt = 0; schritt < 4; ++schritt) ersterHalbtakt.push_back({spur, schritt});
    const SchrittKopie kopie = kopiereSchritte(m, ersterHalbtakt);

    SUBCASE("ersetzen (V1 a): wo die Kopie leer ist, ist es danach leer")
    {
        std::vector<SchrittOrt> eingefuegt;
        CHECK(fuegeSchritteEin(m, kopie, {0, 4}, true, &eingefuegt) == 0);
        CHECK(m.zeilen[0] == std::vector<double>{1, 0, 0, 0, 1, 0, 0, 0});
        // die Snare auf Schritt 6 ist weg, dafuer steht die leise von Schritt 2 auf 6
        CHECK(m.zeilen[1] == std::vector<double>{0, 0, 0.5, 0, 0, 0, 0.5, 0});
        CHECK(m.zeilen[2] == drumMuster().zeilen[2]);  // die Hat war nicht in der Kopie
        CHECK(eingefuegt.size() == 8u);
        CHECK(eingefuegt.front() == SchrittOrt{0, 4});
    }
    SUBCASE("dazulegen: was am Ziel gesetzt ist, bleibt")
    {
        CHECK(fuegeSchritteEin(m, kopie, {1, 4}, false) == 0);  // eine Spur tiefer: Kick auf Snare, Snare auf Hat
        CHECK(m.zeilen[1] == std::vector<double>{0, 0, 0.5, 0, 1, 0, 1, 0});  // Schritt 4 neu, Schritt 6 bleibt
        CHECK(m.zeilen[2] == std::vector<double>{0, 0, 0, 0, 0, 0, 0.5, 1});
    }
    SUBCASE("was nicht passt, wird abgeschnitten und gezaehlt — gezaehlt werden gesetzte Schritte")
    {
        // ab Schritt 6: nur zwei der vier Schritte passen; der Schritt 2 der Snare (gesetzt) faellt hinaus
        std::vector<SchrittOrt> eingefuegt;
        CHECK(fuegeSchritteEin(m, kopie, {0, 6}, true, &eingefuegt) == 1);
        CHECK(m.zeilen[0] == std::vector<double>{1, 0, 0, 0, 1, 0, 1, 0});
        CHECK(m.zeilen[1] == std::vector<double>{0, 0, 0.5, 0, 0, 0, 0, 0});
        CHECK(eingefuegt.size() == 4u);
        // unter die letzte Spur: die Snare-Zeile der Kopie hat keinen Platz
        Muster unten = drumMuster();
        CHECK(fuegeSchritteEin(unten, kopie, {2, 0}, true) == 1);
        CHECK(unten.zeilen[2] == std::vector<double>{1, 0, 0, 0, 0, 0, 0, 1});
    }
    SUBCASE("verstreute Schritte ersetzen nur ihre Stellen (V2 a)")
    {
        const SchrittKopie zwei = kopiereSchritte(m, {{0, 0}, {0, 4}});  // die zwei Kicks, vier Schritte auseinander
        Muster ziel = drumMuster();
        ziel.zeilen[2] = {0, 1, 1, 1, 0, 1, 1, 1};
        CHECK(fuegeSchritteEin(ziel, zwei, {2, 0}, true) == 0);
        // nur Schritt 0 und 4 der Hat sind geschrieben, dazwischen bleibt alles
        CHECK(ziel.zeilen[2] == std::vector<double>{1, 1, 1, 1, 1, 1, 1, 1});
    }
}

TEST_CASE("Loop Auswahl Schritte: leeren, versetzen bis an den Rand, im Kreis drehen")
{
    Muster m = drumMuster();
    SUBCASE("leeren")
    {
        CHECK(leereSchritte(m, {{0, 0}, {0, 1}, {1, 2}}));
        CHECK(m.zeilen[0] == std::vector<double>{0, 0, 0, 0, 1, 0, 0, 0});
        CHECK(m.zeilen[1] == std::vector<double>{0, 0, 0, 0, 0, 0, 1, 0});
        CHECK_FALSE(leereSchritte(m, {{0, 1}, {7, 7}}));  // nichts gesetzt, nichts zu tun
    }
    SUBCASE("versetzen: die Schritte gehen mit, die Marken auch")
    {
        std::vector<SchrittOrt> orte = {{0, 0}, {1, 2}};
        REQUIRE(versetzeSchritte(m, orte, 1, 1));
        CHECK(orte == std::vector<SchrittOrt>{{1, 1}, {2, 3}});
        CHECK(m.zeilen[0] == std::vector<double>{0, 0, 0, 0, 1, 0, 0, 0});
        CHECK(m.zeilen[1] == std::vector<double>{0, 1, 0, 0, 0, 0, 1, 0});
        CHECK(m.zeilen[2] == std::vector<double>{0, 0, 0, 0.5, 0, 0, 0, 1});
        // eine Gruppe darf ueber sich selbst rutschen
        std::vector<SchrittOrt> nebeneinander = {{0, 4}, {0, 5}};
        REQUIRE(versetzeSchritte(m, nebeneinander, 0, 1));
        CHECK(m.zeilen[0] == std::vector<double>{0, 0, 0, 0, 0, 1, 0, 0});
    }
    SUBCASE("am Rand haelt die ganze Auswahl an (V3 a)")
    {
        const Muster vorher = m;
        std::vector<SchrittOrt> orte = {{0, 0}, {1, 6}};
        CHECK_FALSE(versetzeSchritte(m, orte, 0, -1));  // Schritt 0 kann nicht weiter nach links
        CHECK_FALSE(versetzeSchritte(m, orte, -1, 0));  // die Kick ist die oberste Spur
        CHECK_FALSE(versetzeSchritte(m, orte, 0, 2));   // die Snare stiesse hinten an
        CHECK_FALSE(versetzeSchritte(m, orte, 2, 0));   // unter der Hat ist nichts
        CHECK_FALSE(versetzeSchritte(m, orte, 0, 0));
        CHECK(m == vorher);
        CHECK(orte == std::vector<SchrittOrt>{{0, 0}, {1, 6}});
        REQUIRE(versetzeSchritte(m, orte, 0, 1));  // ein Schritt geht noch
    }
    SUBCASE("drehen: was hinten hinausgeht, kommt vorn herein")
    {
        std::vector<SchrittOrt> orte = {{1, 6}, {2, 7}};
        REQUIRE(dreheSchritte(m, orte, 1, 8));
        CHECK(orte == std::vector<SchrittOrt>{{1, 7}, {2, 0}});
        CHECK(m.zeilen[1] == std::vector<double>{0, 0, 0.5, 0, 0, 0, 0, 1});
        CHECK(m.zeilen[2] == std::vector<double>{1, 0, 0, 0, 0, 0, 0, 0});
        REQUIRE(dreheSchritte(m, orte, -1, 8));
        CHECK(m == drumMuster());
        // eine ganze Spur um einen Schritt
        std::vector<SchrittOrt> kick;
        for (int schritt = 0; schritt < 8; ++schritt) kick.push_back({0, schritt});
        REQUIRE(dreheSchritte(m, kick, -1, 8));
        CHECK(m.zeilen[0] == std::vector<double>{0, 0, 0, 1, 0, 0, 0, 1});
        CHECK_FALSE(dreheSchritte(m, kick, 8, 8));  // einmal ganz herum ist nichts
        CHECK_FALSE(dreheSchritte(m, kick, 1, 0));
    }
}

TEST_CASE("Loop Auswahl Noten: kopieren und einfuegen — die Tonhoehe bleibt, der Rest wird gezaehlt")
{
    Muster m = notenMuster();
    CHECK(findeNote(m, {2, 64}) == 1);
    CHECK(findeNote(m, {2, 65}) == -1);

    const NotenKopie kopie = kopiereNoten(m, {{4, 67}, {2, 64}, {9, 9}});  // E4 und G4; eine gibt es nicht
    REQUIRE(kopie.noten.size() == 2u);
    CHECK(kopie.noten[0] == Note{0, 1, 64, 0.5});  // ab dem ersten Einsatz gezaehlt, geordnet
    CHECK(kopie.noten[1] == Note{2, 4, 67, 0.8});
    CHECK(kopie.schritte == 6);
    CHECK(kopiereNoten(m, {}).leer());

    SUBCASE("einfuegen: die Noten kommen dazu, die uebrigen bleiben")
    {
        std::vector<NotenOrt> eingefuegt;
        const NotenRest rest = fuegeNotenEin(m, kopie, 8, kNotenSchritte, &eingefuegt);
        CHECK(rest.weg == 0);
        CHECK(rest.gekuerzt == 0);
        CHECK(m.noten.size() == 6u);
        CHECK(eingefuegt == std::vector<NotenOrt>{{8, 64}, {10, 67}});
        CHECK(m.noten[findeNote(m, {10, 67})] == Note{10, 4, 67, 0.8});
        // geordnet nach Schritt, dann Tonhoehe
        for (std::size_t i = 1; i < m.noten.size(); ++i) CHECK(m.noten[i - 1].schritt <= m.noten[i].schritt);
    }
    SUBCASE("eine Note auf demselben Schritt und derselben Tonhoehe wird ersetzt")
    {
        const NotenKopie eine = kopiereNoten(m, {{2, 64}});  // E4, ein Schritt, halbe Lautstaerke
        (void)fuegeNotenEin(m, eine, 2, kNotenSchritte);
        CHECK(m.noten.size() == 4u);
        Muster lang = notenMuster();
        lang.noten[1].laenge = 2;
        const NotenKopie laengere = kopiereNoten(lang, {{2, 64}});
        (void)fuegeNotenEin(m, laengere, 2, kNotenSchritte);
        CHECK(m.noten.size() == 4u);
        CHECK(m.noten[findeNote(m, {2, 64})].laenge == 2);
    }
    SUBCASE("ueber das Ende: gekuerzt oder weg, beides gezaehlt")
    {
        std::vector<NotenOrt> eingefuegt;
        // ab Schritt 13: E4 auf 13 passt; G4 auf 15 hat nur noch einen Schritt
        NotenRest rest = fuegeNotenEin(m, kopie, 13, kNotenSchritte, &eingefuegt);
        CHECK(rest.weg == 0);
        CHECK(rest.gekuerzt == 1);
        CHECK(m.noten[findeNote(m, {15, 67})].laenge == 1);
        // ab Schritt 15: E4 passt, G4 begaenne hinter dem Ende
        Muster zweites = notenMuster();
        rest = fuegeNotenEin(zweites, kopie, 15, kNotenSchritte, &eingefuegt);
        CHECK(rest.weg == 1);
        CHECK(rest.gekuerzt == 0);
        CHECK(eingefuegt == std::vector<NotenOrt>{{15, 64}});
    }
}

TEST_CASE("Loop Auswahl Noten: loeschen, versetzen bis an den Rand, im Kreis drehen")
{
    Muster m = notenMuster();
    SUBCASE("loeschen")
    {
        CHECK(loescheNoten(m, {{0, 60}, {12, 72}}));
        CHECK(m.noten.size() == 2u);
        CHECK_FALSE(loescheNoten(m, {{0, 60}}));
    }
    SUBCASE("versetzen in Zeit und Tonhoehe")
    {
        std::vector<NotenOrt> orte = {{0, 60}, {2, 64}};
        REQUIRE(versetzeNoten(m, orte, 4, 12, kNotenSchritte));
        CHECK(orte == std::vector<NotenOrt>{{4, 72}, {6, 76}});
        CHECK(m.noten.size() == 4u);
        CHECK(m.noten[findeNote(m, {4, 72})] == Note{4, 2, 72, 1.0});
        CHECK(findeNote(m, {4, 67}) >= 0);  // die G4 auf demselben Schritt bleibt: andere Tonhoehe
        // auf eine nicht markierte Note derselben Stelle: sie wird ersetzt
        std::vector<NotenOrt> eine = {{4, 72}};
        REQUIRE(versetzeNoten(m, eine, 0, -5, kNotenSchritte));
        CHECK(m.noten.size() == 3u);
        CHECK(m.noten[findeNote(m, {4, 67})].laenge == 2);
    }
    SUBCASE("am Rand haelt die ganze Auswahl an (V3 a)")
    {
        const Muster vorher = m;
        std::vector<NotenOrt> orte = {{0, 60}, {12, 72}};
        CHECK_FALSE(versetzeNoten(m, orte, -1, 0, kNotenSchritte));  // vor dem Anfang
        CHECK_FALSE(versetzeNoten(m, orte, 1, 0, kNotenSchritte));   // C5 reicht schon bis ans Ende
        // die Raender: seit S132 (S-T1 a) A0 und C8, vorher C1 und C7
        CHECK_FALSE(versetzeNoten(m, orte, 0, kHoeheMax - 72 + 1, kNotenSchritte));  // C5 laege ueber dem obersten Ton
        CHECK_FALSE(versetzeNoten(m, orte, 0, kHoeheMin - 60 - 1, kNotenSchritte));  // C4 laege unter dem untersten
        CHECK(m == vorher);
        REQUIRE(versetzeNoten(m, orte, 0, kHoeheMax - 72, kNotenSchritte));          // genau bis zum obersten
        CHECK(orte.back() == NotenOrt{12, kHoeheMax});
    }
    SUBCASE("drehen: im Kreis, eine Note am Ende wird gekuerzt")
    {
        std::vector<NotenOrt> orte = {{12, 72}, {0, 60}};
        int gekuerzt = -1;
        REQUIRE(dreheNoten(m, orte, 2, kNotenSchritte, &gekuerzt));
        // C5 (vier lang) steht jetzt auf 14 und hat nur noch zwei Schritte; C4 auf 2
        CHECK(orte == std::vector<NotenOrt>{{14, 72}, {2, 60}});
        CHECK(gekuerzt == 1);
        CHECK(m.noten[findeNote(m, {14, 72})].laenge == 2);
        // noch zwei weiter: C5 kommt vorn herein — mit der gekuerzten Laenge
        REQUIRE(dreheNoten(m, orte, 2, kNotenSchritte, &gekuerzt));
        CHECK(orte.front() == NotenOrt{0, 72});
        CHECK(gekuerzt == 0);
        CHECK_FALSE(dreheNoten(m, orte, 16, kNotenSchritte));
    }
}
