/**
 ****************************************************************************************
 * @file   test_TextTags.cpp
 * @brief  Vorlagen des Text-Knotens: Tags einsetzen, wahlfreie Abschnitte
 *
 * @details
 * Session 86. Reine Zeichenkettenarbeit (`TextTags.hpp`) — die Verdrahtung vom
 * Titelwechsel bis ins Bild prüft `test_FeldSteuerungGl` („Text S86: Tags").
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/multieffect/TextTags.hpp"

using namespace lumi::multieffect::texttags;

namespace
{
SongTags voll()
{
    SongTags t;
    t.title = "Blue Monday";
    t.artist = "New Order";
    t.album = "Substance";
    t.year = "1983";
    t.genre = "Synthpop";
    t.track = "7";
    t.file = "07 - Blue Monday";
    return t;
}
}  // namespace

TEST_CASE("TextTags: Platzhalter werden eingesetzt, Namen ohne Gross/Klein")
{
    const SongTags t = voll();
    CHECK(ersetze("{title}", t) == "Blue Monday");
    CHECK(ersetze("{Artist} - {TITLE}", t) == "New Order - Blue Monday");
    CHECK(ersetze("{track}. {title} ({album}, {year}) [{genre}]", t) ==
          "7. Blue Monday (Substance, 1983) Synthpop");
    CHECK(ersetze("{file}", t) == "07 - Blue Monday");
    // ohne Platzhalter bleibt der Text, wie er ist
    CHECK(ersetze("EINS;ZWEI", t) == "EINS;ZWEI");
    CHECK_FALSE(hatPlatzhalter("EINS;ZWEI"));
    CHECK(hatPlatzhalter("a {title}"));
}

TEST_CASE("TextTags: unbekannter Name und offene Klammer bleiben woertlich")
{
    const SongTags t = voll();
    CHECK(ersetze("{titel}", t) == "{titel}");        // Tippfehler faellt auf
    CHECK(ersetze("{title", t) == "{title");
    CHECK(ersetze("a } b", t) == "a } b");
    CHECK(ersetze("[ohne Platzhalter]", t) == "[ohne Platzhalter]");
    CHECK(ersetze("[offen {title}", t) == "[offen Blue Monday");
}

TEST_CASE("TextTags: wahlfreier Abschnitt entfaellt, wenn ein Tag darin leer ist")
{
    SongTags t = voll();
    const char* vorlage = "[{artist} - ]{title}[ ({year})]";
    CHECK(ersetze(vorlage, t) == "New Order - Blue Monday (1983)");
    t.year.clear();
    CHECK(ersetze(vorlage, t) == "New Order - Blue Monday");
    t.artist.clear();
    CHECK(ersetze(vorlage, t) == "Blue Monday");
    // mehrere Platzhalter in einem Abschnitt: einer leer genuegt
    t = voll();
    t.album.clear();
    CHECK(ersetze("{title}[ ({album}, {year})]", t) == "Blue Monday");
    // ausserhalb eines Abschnitts bleibt ein leerer Tag einfach leer
    CHECK(ersetze("{album}|{title}", t) == "|Blue Monday");
    // nichts geladen: alles leer
    CHECK(ersetze(vorlage, SongTags{}) == "");
}

TEST_CASE("TextTags: ein Semikolon im Tag trennt keine Woerter")
{
    SongTags t = voll();
    t.artist = "A; B";
    // das Semikolon der VORLAGE trennt weiter, das im Wert wird zum Komma
    CHECK(ersetze("{artist};{title}", t) == "A, B;Blue Monday");
}
