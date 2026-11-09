/**
 * @file test_sammlung.cpp
 * @brief Die Ablage der Hoerproben nach HoerTest_Helper Konzept 0.3.0 §11/§12 (IC-1, IC-2): Achsen aus dem Namen,
 *        Fassungen mit/ohne als Paar, Pegel nach BS.1770-4, `liste.json` und `klaenge.sha256`
 */
#include "doctest.h"

#include "gemeinsam/Json.hpp"
#include "gemeinsam/Werkzeug.hpp"
#include "werkzeuge/Hoerproben.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

using namespace ic;
using namespace ic::hoer;

namespace
{

std::string achse(const Klang& k, const std::string& name)
{
    for (const auto& [n, v] : k.achsen)
        if (n == name) return v;
    return "-";
}

std::vector<float> sinus(double amplitude, double sekunden)
{
    std::vector<float> s(static_cast<std::size_t>(sekunden * 48000));
    for (std::size_t i = 0; i < s.size(); ++i) s[i] = static_cast<float>(amplitude * std::sin(2.0 * std::numbers::pi * 440.0 * i / 48000.0));
    return s;
}

}  // namespace

TEST_CASE("Sammlung: Achsen und Fassung aus dem Namen")
{
    Klang k;
    achsenAus("Upright", "C4_a-12", k);
    CHECK(achse(k, "instrument") == "Upright");
    CHECK(achse(k, "ton") == "C4");
    CHECK(achse(k, "staerke") == "a-12");
    CHECK(achse(k, "spielweise") == "-");
    CHECK(k.fassung == "vorgabe");
    achsenAus("Violine", "Fis5_e60", k);
    CHECK(achse(k, "ton") == "Fis5");
    CHECK(achse(k, "staerke") == "e60");
    achsenAus("Vibraphon", "F4_gestrichen_e50", k);
    CHECK(achse(k, "staerke") == "e50");
    CHECK(achse(k, "spielweise") == "gestrichen");
    achsenAus("Hackbrett", "C4_folge_clip_mitschwingen", k);
    CHECK(k.fassung == "mit");
    CHECK(achse(k, "spielweise") == "folge_clip");
    Klang o;
    achsenAus("Hackbrett", "C4_folge_clip_ohne", o);
    CHECK(o.fassung == "ohne");
    CHECK(o.achsen == k.achsen);
    achsenAus("Blockfloete", "C5_ohne_schneidenton", k);  // „ohne“ mitten im Namen ist keine Fassung
    CHECK(k.fassung == "vorgabe");
    CHECK(achse(k, "spielweise") == "ohne_schneidenton");
    achsenAus("Tom", "e50", k);
    CHECK(achse(k, "ton") == "-");
    CHECK(achse(k, "staerke") == "e50");
    achsenAus("Orgel", "akkord_e20", k);
    CHECK(achse(k, "ton") == "-");
    CHECK(achse(k, "spielweise") == "akkord");
}

TEST_CASE("Sammlung: liste.json nach §12, Pegel nach BS.1770-4, Abdruck je Klang")
{
    const std::filesystem::path wurzel = std::filesystem::temp_directory_path() / "ic_sammlung_test";
    std::filesystem::remove_all(wurzel);
    Sammlung s;
    s.wurzel = wurzel.string();
    s.befehl = "ic_hoerproben test";
    s.lege("99_test", "Probe", "A4_e50", sinus(0.1, 2.0), 48000, "Klingt es?");
    s.lege("99_test", "Probe", "A4_folge_mitschwingen", sinus(0.5, 2.0), 48000, "Mitschwingen hoerbar?");
    s.lege("99_test", "Probe", "A4_folge_ohne", sinus(0.5, 2.0), 48000, "ohne");
    // leise mit einem Klick: auf −23 LUFS angehoben, uebersteuerte die Spitze — daher die ganze Liste tiefer
    std::vector<float> klick = sinus(0.01, 2.0);
    klick[1000] = 1.0f;
    s.lege("99_test", "Laut", "A4_e100", klick, 48000, "zu laut?");
    s.schreibeListen();
    const std::filesystem::path ordner = wurzel / "listen" / "99_test";
    REQUIRE(std::filesystem::exists(ordner / "liste.json"));
    JsonWert l;
    std::string fehler;
    REQUIRE_MESSAGE(leseJsonDatei((ordner / "liste.json").string(), l, &fehler), fehler);
    CHECK(l["schema"].zahl == 1.0);
    CHECK(l["gruppe_nach"].liste.front().text == "instrument");
    // der Klick erlaubt hoechstens lufs_vorher + 20 lg(0,99 / 1): der gemeinsame Pegel der Liste liegt dort (0,1 LU)
    const double klickVorher = l["messwerte"].liste[3]["werte"].liste[0]["wert"].zahl;
    const double ziel = l["pegel"]["lufs"].zahl;
    CHECK(ziel < -23.0);
    CHECK(ziel == doctest::Approx(std::floor((klickVorher + dB(0.99)) * 10.0) / 10.0).epsilon(0.011 / 30.0));
    REQUIRE(l["klaenge"].liste.size() == 4);
    CHECK(l["klaenge"].liste[0]["achsen"]["staerke"].text == "e50");
    CHECK(l["klaenge"].liste[1]["fassung"].text == "mit");
    // Punkte: ein Paar mit/ohne, zwei Einzelklaenge mit Frage
    REQUIRE(l["punkte"].liste.size() == 3);
    CHECK(l["punkte"].liste[0]["art"].text == "paar");
    CHECK(l["punkte"].liste[0]["frage"].text == "Mitschwingen hoerbar?");
    CHECK(l["punkte"].liste[1]["art"].text == "einzel");
    CHECK(l["punkte"].liste[1]["pole"].liste.size() == 2);
    // Messwerte je Klang: lufs_vorher
    REQUIRE(l["messwerte"].liste.size() == 4);
    const double vorher = l["messwerte"].liste[0]["werte"].liste[0]["wert"].zahl;
    CHECK(vorher == doctest::Approx(lufsNaeherung(sinus(0.1, 2.0), 48000)).epsilon(0.02 / 23.0));  // stetiger Ton: Gate ohne Wirkung
    // die Klaenge: alle auf dem Pegel der Liste (BS.1770-4), keiner ueber 0,99
    for (const char* id : {"Probe_A4_e50", "Laut_A4_e100"})
    {
        CAPTURE(id);
        std::vector<float> w;
        int rate = 0, kanaele = 0;
        REQUIRE(leseWav((ordner / "klaenge" / (std::string(id) + ".wav")).string(), w, rate, kanaele));
        CHECK(rate == 48000);
        CHECK(kanaele == 1);
        CHECK(lautheit(w, rate) == doctest::Approx(ziel).epsilon(0.01 / 30.0));
        CHECK(spitze(w) <= 0.99);
    }
    // klaenge.sha256: Kopf mit Befehl, je Klang eine Zeile mit dem Abdruck der Datei
    {
        std::ifstream sha(ordner / "klaenge.sha256");
        std::string kopf, zeile;
        std::getline(sha, kopf);
        CHECK(kopf.find("ic_hoerproben test") != std::string::npos);
        std::getline(sha, zeile);
        std::ifstream wav(ordner / "klaenge" / "Probe_A4_e50.wav", std::ios::binary);
        const std::string bytes((std::istreambuf_iterator<char>(wav)), std::istreambuf_iterator<char>());
        CHECK(zeile == sha256({reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size()}) + "  klaenge/Probe_A4_e50.wav");
    }  // unter Windows erst geschlossen loeschbar
    std::filesystem::remove_all(wurzel);
}
