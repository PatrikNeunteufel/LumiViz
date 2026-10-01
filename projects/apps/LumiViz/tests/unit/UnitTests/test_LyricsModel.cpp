/**
 ****************************************************************************************
 * @file   test_LyricsModel.cpp
 * @brief  Songtext-Modell: LRC lesen und schreiben, laufende Zeile, Worte, Stimmen
 *
 * @details
 * Session 86. Reine Zeichenkettenarbeit (`LyricsModel.hpp`).
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/LyricsModel.hpp"

using namespace lumi::multieffect::lyrics;

TEST_CASE("LyricsModel: Zeitmarken in allen Schreibweisen, Kopfzeilen, Sortierung")
{
    const Songtext s = leseLrc("[ar:New Order]\n"
                               "[ti:Blue Monday]\n"
                               "[00:12.50]zweite\n"
                               "[00:05]erste\n"
                               "[01:02.345]vierte\r\n"
                               "[00:30:07]dritte\n");
    REQUIRE(s.synchron);
    REQUIRE(s.zeilen.size() == 4);
    CHECK(s.zeilen[0].text == "erste");
    CHECK(s.zeilen[0].zeitMs == 5000);
    CHECK(s.zeilen[1].zeitMs == 12500);
    CHECK(s.zeilen[2].text == "dritte");
    CHECK(s.zeilen[2].zeitMs == 30070);
    CHECK(s.zeilen[3].zeitMs == 62345);
    CHECK(s.zeilen[3].text == "vierte");  // \r abgeschnitten
}

TEST_CASE("LyricsModel: mehrere Marken je Zeile wiederholen sie; leere Zeile bleibt")
{
    const Songtext s = leseLrc("[00:10.00][00:40.00]Refrain\n[00:20.00]\n");
    REQUIRE(s.zeilen.size() == 3);
    CHECK(s.zeilen[0].text == "Refrain");
    CHECK(s.zeilen[1].text.empty());  // Pause: die Anzeige wird leer
    CHECK(s.zeilen[1].zeitMs == 20000);
    CHECK(s.zeilen[2].text == "Refrain");
    CHECK(s.zeilen[2].zeitMs == 40000);
}

TEST_CASE("LyricsModel: ohne Zeitmarke unsynchron — auch in einer .lrc")
{
    const Songtext s = leseLrc("\n\nErste Zeile\nZweite Zeile\n\nNeue Strophe\n[Refrain]\n\n");
    CHECK_FALSE(s.synchron);
    REQUIRE(s.zeilen.size() == 5);
    CHECK(s.zeilen[0].text == "Erste Zeile");
    CHECK(s.zeilen[2].text.empty());           // Strophentrenner bleibt
    CHECK(s.zeilen[4].text == "[Refrain]");    // eckige Klammer ohne Zeit ist Text
    CHECK(s.zeilen[0].zeitMs == -1);
    CHECK(zeileBei(s, 99999) == -1);
}

TEST_CASE("LyricsModel: laufende Zeile und Versatz")
{
    Songtext s = leseLrc("[00:10.00]a\n[00:20.00]b\n[00:30.00]c\n");
    CHECK(zeileBei(s, 0) == -1);       // vor der ersten Zeile
    CHECK(zeileBei(s, 9999) == -1);
    CHECK(zeileBei(s, 10000) == 0);
    CHECK(zeileBei(s, 19999) == 0);
    CHECK(zeileBei(s, 20000) == 1);
    CHECK(zeileBei(s, 999999) == 2);

    // [offset:+500]: der Text erscheint 500 ms FRUEHER
    s = leseLrc("[offset:500]\n[00:10.00]a\n");
    CHECK(s.versatzMs == 500);
    CHECK(zeileBei(s, 9400) == -1);
    CHECK(zeileBei(s, 9500) == 0);
    s = leseLrc("[offset:-500]\n[00:10.00]a\n");
    CHECK(zeileBei(s, 10000) == -1);
    CHECK(zeileBei(s, 10500) == 0);
}

TEST_CASE("LyricsModel: Wort-Zeitmarken (erweitertes LRC)")
{
    const Songtext s = leseLrc("[00:10.00]How <00:10.50>does <00:11.20>it feel\n");
    REQUIRE(s.zeilen.size() == 1);
    const Zeile& z = s.zeilen[0];
    CHECK(z.text == "How does it feel");
    REQUIRE(z.worte.size() == 3);
    CHECK(z.worte[0].text == "How ");
    CHECK(z.worte[0].zeitMs == 10000);  // das erste Stueck traegt die Zeilen-Zeit
    CHECK(z.worte[1].zeitMs == 10500);
    CHECK(z.worte[2].text == "it feel");
    CHECK(worteBei(s, z, 9000) == 0);
    CHECK(worteBei(s, z, 10000) == 1);
    CHECK(worteBei(s, z, 10700) == 2);
    CHECK(worteBei(s, z, 60000) == 3);
    // ohne Wort-Marken bleibt die Liste leer
    CHECK(leseLrc("[00:10.00]a < b\n").zeilen[0].worte.empty());
    CHECK(leseLrc("[00:10.00]a < b\n").zeilen[0].text == "a < b");
}

TEST_CASE("LyricsModel: Stimm-Marken fuer Duette gelten bis zur naechsten")
{
    const Songtext s = leseLrc("[00:01.00]ohne\n"
                               "[00:02.00]v1: sie\n"
                               "[00:03.00]noch sie\n"
                               "[00:04.00]V2: er\n"
                               "[00:05.00]v1+v2: beide\n"
                               "[00:06.00]F: zweite\n"
                               "[00:07.00]D: wieder beide\n"
                               "[00:08.00]M: erste\n");
    REQUIRE(s.zeilen.size() == 8);
    CHECK(s.zeilen[0].stimme == Stimme::Keine);
    CHECK(s.zeilen[1].stimme == Stimme::Eins);
    CHECK(s.zeilen[1].text == "sie");
    CHECK(s.zeilen[2].stimme == Stimme::Eins);  // gilt weiter
    CHECK(s.zeilen[3].stimme == Stimme::Zwei);
    CHECK(s.zeilen[4].stimme == Stimme::Beide);
    CHECK(s.zeilen[4].text == "beide");
    CHECK(s.zeilen[5].stimme == Stimme::Zwei);
    CHECK(s.zeilen[6].stimme == Stimme::Beide);
    CHECK(s.zeilen[7].stimme == Stimme::Eins);
}

TEST_CASE("LyricsModel: schreiben und wieder lesen ergibt dasselbe")
{
    Songtext s = leseLrc("[offset:-250]\n"
                         "[00:10.00]v1: How <00:10.50>does <00:11.20>it feel\n"
                         "[00:20.00]v2: To treat me\n"
                         "[00:30.00]\n"
                         "[00:40.00]v1+v2: like you do\n");
    const std::string lrc = schreibeLrc(s);
    const Songtext wieder = leseLrc(lrc);
    CHECK(wieder == s);
    CHECK(lrc.find("[offset:-250]") == 0);

    // Nachsynchronisieren: ein unsynchroner Text bekommt Zeiten und ist danach synchron
    Songtext roh = leseLrc("eins\nzwei\n");
    REQUIRE_FALSE(roh.synchron);
    roh.zeilen[0].zeitMs = 1500;
    roh.zeilen[1].zeitMs = 4250;
    const Songtext fertig = leseLrc(schreibeLrc(roh));
    CHECK(fertig.synchron);
    CHECK(fertig.zeilen[1].zeitMs == 4250);
    CHECK(zeileBei(fertig, 2000) == 0);
}

TEST_CASE("LyricsModel: UTF-8-BOM und kaputte Marken")
{
    const Songtext s = leseLrc("\xEF\xBB\xBF[00:01.00]mit BOM\n[00:99.00]keine Zeit\n[xx]\n");
    REQUIRE(s.synchron);
    REQUIRE(s.zeilen.size() == 1);  // die Zeilen ohne gueltige Zeit fallen im synchronen Text weg
    CHECK(s.zeilen[0].text == "mit BOM");
    CHECK(leseLrc("").leer());
}
