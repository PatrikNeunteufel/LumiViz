/**
 ****************************************************************************************
 * @file   test_LoopSalamander.cpp
 * @brief  Loop, Stufe 31d-3: Salamander messen und je Taste verdichten
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S151/S152: UF1 a,
 *         UF3 a, UF4 a, UF6 a, D1 a, V1 a, WL1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung; die Messung selbst ohne Qt (`Fluegelmessung.hpp`, IK1).
 *
 * - Die 30 Tasten von *Salamander* und ihre Dateinamen.
 * - Verdichten je Taste an Messungen von Hand: Mediane, Anteil zweier Stufen, die Steigung der
 *   Helligkeit (V1 a), Pegelumfang, Pegel gegen das Mittel der Tastatur.
 * - Die Datei hin und zurueck, in einem Wegwerf-Ordner.
 * - `werte/salamander_c5.txt` (committet): 30 Tasten, jede mit B und Abklingzeit — liest vom
 *   Arbeitsordner aus (Test-Exe aus dem Projekt-Root starten).
 * - 31d-7 B (7.5.2): `werte/salamander_kurven.txt` — je Taste, Ebene v6–v16, Teilton 1–3 die Kurve nach
 *   Schroeder und die Autokovarianz (K1 a, K2 a, K5 a); das Kanalmittel und die Datei an Zahlen von Hand.
 * - `LUMIVIZ_SALAMANDER=<Ordner>` misst alle 480 Toene beider Kanaele und schreibt die Datei neu (seit 7.5.2
 *   auch `salamander_kurven.txt`);
 *   der Ordner ist `…\Salamander\SalamanderGrandPianoV3_48khz24bit` (README dort). Ohne die
 *   Variable meldet der Fall das und prueft nichts.
 * - Teil 2: der Boden im Vergleich (UF6 a) an Zahlen von Hand; `LUMIVIZ_31D3_MODELL=<Ordner>` misst den
 *   gerechneten Steinway D wie im Programm (Stufen −30 … 0 dB an der Stelle der Ebenen, V1 a) und den
 *   Glasboden, vergleicht mit `werte/salamander_c5.txt` und legt `modell_steinway.txt` und
 *   `vergleich.txt` in den Ordner.
 * - 31d-7 (P8 a 7): `LUMIVIZ_31D7_BILD=<Ordner>` mit `LUMIVIZ_SALAMANDER` legt je Taste ein Bild der
 *   Huellkurven von Teilton 1–3 ab — *Salamander* v12 gegen das Modell an Stufe −8 mit γ und δ von Hand.
 * - 31d-7 (SZ1 a): `LUMIVIZ_31D7_TIEFE=<Ordner>` mit `LUMIVIZ_SALAMANDER` veraendert an MIDI 60, 66, 81 je eine
 *   Groesse (zweite Ebene, Verstimmung, Stoss des Hammers je Saite) und legt die Tiefe der Einbrueche
 *   (`tiefe.txt`) und Bilder ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "Kurvenabstand.hpp"
#include "ModellMessung.hpp"

#include "InstrumentBody/Mobilitaet.hpp"
#include "audio/WavDatei.hpp"
#include "audio/WavFormat.hpp"
#include "loop/Boden.hpp"
#include "loop/Fft.hpp"
#include "loop/HammerSaite.hpp"
#include "loop/HammerTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonspeicher.hpp"

#include <QColor>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <thread>

using namespace lumi::messung;

namespace
{

const std::filesystem::path kWerteDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_c5.txt";
const std::filesystem::path kKurvenDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_kurven.txt";

/// Eine Messung von Hand: Teilton 1 bei `pegel`, Teilton 2 bei `pegel − 6 + steigung·(pegel + 40)`.
KanalMessung kanal(double pegel, double steigung, double b, double t60, bool zwei)
{
    KanalMessung m;
    m.erkannt = true;
    m.f0 = 110.0;
    m.b = b;
    m.pegelDb = pegel;
    TeiltonKurz t1{1, 110.0, pegel, {}, {}};
    t1.abklingen.erkannt = true;
    t1.abklingen.zweiStufen = zwei;
    t1.abklingen.t60Frueh = t60;
    t1.abklingen.t60Spaet = zwei ? 3.0 * t60 : t60;
    t1.abklingen.knickS = zwei ? 2.0 : 0.0;
    m.teiltoene.push_back(t1);
    m.teiltoene.push_back({2, 220.0, pegel - 6.0 + steigung * (pegel + 40.0), {}, {}});
    for (int k = 3; k <= 5; ++k) m.teiltoene.push_back({k, 110.0 * k, pegel - 10.0 * k, {}, {}});
    return m;
}

}  // namespace

TEST_CASE("Loop Salamander 31d-3 Tasten und Dateien")
{
    const std::vector<SalamanderTaste> tasten = salamanderTasten();
    REQUIRE(tasten.size() == 30);
    CHECK(tasten.front().midi == 21);
    CHECK(tasten.front().name == "A0");
    CHECK(tasten[1].name == "C1");
    CHECK(tasten[2].name == "D#1");
    CHECK(tasten[3].name == "F#1");
    CHECK(tasten[13].name == "C4");
    CHECK(tasten[13].midi == 60);
    CHECK(tasten.back().midi == 108);
    CHECK(tasten.back().name == "C8");
    CHECK(salamanderDatei("gibt_es_nicht", tasten[2], 16).filename().string() == "D#1v16.wav");
    CHECK(tastenFrequenz(69) == doctest::Approx(440.0));
    CHECK(tastenFrequenz(21) == doctest::Approx(27.5));
}

TEST_CASE("Loop Salamander 31d-3 verdichten je Taste")
{
    // 16 Ebenen, Teilton 1 von −40 bis −10 dB; Helligkeit von Teilton 2 waechst links mit 0,5 dB je dB,
    // rechts mit 0,3; B links 2e-4, rechts 4e-4; zwei Stufen nur ab v11
    std::vector<std::array<KanalMessung, 2>> messungen;
    for (int e = 1; e <= kEbenen; ++e)
    {
        const double pegel = -40.0 + 2.0 * (e - 1);
        messungen.push_back({kanal(pegel, 0.5, 2e-4, 4.0 + 0.1 * e, e >= 11), kanal(pegel + 1.0, 0.3, 4e-4, 4.0 + 0.1 * e, e >= 11)});
    }
    messungen[0][1].erkannt = false;  // ein Kanal ohne Messung zaehlt nicht
    // Tiefe der Einbrueche (TB1 a): Teilton 1 mit Schwebung, 10 + Index dB; Teilton 2 nur in v3 (zaehlt nicht,
    // vor v6) und v16 (zaehlt); Teilton 3 ohne erkannte Schwebung
    for (std::size_t e = 0; e < messungen.size(); ++e)
        for (KanalMessung& m : messungen[e])
        {
            m.teiltoene[0].schwebung = {true, 0.5, 8.0, 10.0 + static_cast<double>(e)};
            if (e == 2) m.teiltoene[1].schwebung = {true, 1.0, 8.0, 200.0};
            if (e == 15) m.teiltoene[1].schwebung = {true, 1.0, 8.0, 100.0};
            m.teiltoene[2].schwebung = {false, 1.5, 0.5, 300.0};
            // Teilton 2 (SM3 a): 3 s, ab v11 zwei Stufen mit 9 s
            m.teiltoene[1].abklingen.erkannt = true;
            m.teiltoene[1].abklingen.zweiStufen = e >= 10;
            m.teiltoene[1].abklingen.t60Frueh = 3.0;
            m.teiltoene[1].abklingen.t60Spaet = e >= 10 ? 9.0 : 3.0;
        }
    const TastenWerte w = verdichte(45, messungen);
    CHECK(w.midi == 45);
    CHECK(w.b.anzahl == 31);
    CHECK(w.b.wert == doctest::Approx(2e-4).scale(0.0));  // 16 mal 2e-4 gegen 15 mal 4e-4 (eine fehlt)
    // t60 nur ab v6: 11 Ebenen, beide Kanaele
    CHECK(w.t60Frueh.anzahl == 22);
    CHECK(w.t60Frueh.wert == doctest::Approx(4.0 + 0.1 * 11));
    CHECK(w.zweiStufenAnteil.wert == doctest::Approx(12.0 / 22.0));
    CHECK(w.t60Spaet.anzahl == 12);
    CHECK(w.knickS.wert == doctest::Approx(2.0));
    CHECK(w.tiefeDb.anzahl == 24);  // Teilton 1 in v6–v16 je Kanal (22), Teilton 2 in v16 (2)
    CHECK(w.tiefeDb.wert == doctest::Approx(21.0));
    CHECK(w.tiefeLeise.anzahl == 10);  // v6–v10: Teilton 1, 15 … 19 dB je Kanal
    CHECK(w.tiefeLeise.wert == doctest::Approx(17.0));
    CHECK(w.tiefeLaut.anzahl == 12);   // v12–v16: Teilton 1, 21 … 25 dB, dazu Teilton 2 in v16
    CHECK(w.tiefeLaut.wert == doctest::Approx(24.0));
    CHECK(w.t60FruehK[0].anzahl == 22);  // Teilton 2, v6–v16 je Kanal
    CHECK(w.t60FruehK[0].wert == doctest::Approx(3.0));
    CHECK(w.t60SpaetK[0].anzahl == 12);  // zwei Stufen nur ab v11
    CHECK(w.t60SpaetK[0].wert == doctest::Approx(9.0));
    CHECK(w.zweiStufenAnteilK[0].wert == doctest::Approx(12.0 / 22.0));
    CHECK(w.t60FruehK[1].anzahl == 0);   // Teilton 3 ohne Abklingen
    REQUIRE(w.helligkeit.size() == 7);
    CHECK(w.helligkeit[0].anzahl == 2);
    CHECK(w.helligkeit[0].wert == doctest::Approx(0.5).epsilon(1e-9));  // Median aus zweien: der obere
    CHECK(w.pegelumfang.wert == doctest::Approx(30.0));
    CHECK(w.pegelumfang.anzahl == 1);  // rechts fehlt v1

    // Pegel gegen das Mittel der Tastatur: zwei Tasten, die zweite 6 dB lauter
    std::vector<std::vector<std::array<KanalMessung, 2>>> zwei(2);
    for (int e = 1; e <= kEbenen; ++e)
    {
        zwei[0].push_back({kanal(-30.0, 0.0, 1e-4, 4.0, false), kanal(-30.0, 0.0, 1e-4, 4.0, false)});
        zwei[1].push_back({kanal(-24.0, 0.0, 1e-4, 4.0, false), kanal(-24.0, 0.0, 1e-4, 4.0, false)});
    }
    std::vector<TastenWerte> werte{verdichte(21, zwei[0]), verdichte(24, zwei[1])};
    setzeRelativePegel(werte, zwei);
    CHECK(werte[0].pegelL.wert == doctest::Approx(-3.0));
    CHECK(werte[1].pegelR.wert == doctest::Approx(3.0));
    CHECK(werte[1].pegelR.anzahl == kEbenen);
}

TEST_CASE("Loop Salamander 31d-3 Datei hin und zurueck")
{
    const std::filesystem::path ordner =
        std::filesystem::temp_directory_path() / ("lumiviz_salamander_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    REQUIRE(std::filesystem::create_directories(ordner));
    TastenWerte w;
    w.midi = 60;
    w.b = {3.5e-4, 31};
    w.t60Frueh = {6.25, 22};
    w.tiefeDb = {18.5, 24};
    w.tiefeLeise = {16.0, 10};
    w.tiefeLaut = {21.5, 12};
    w.t60FruehK = {Wert{5.5, 22}, Wert{4.4, 20}};
    w.t60SpaetK = {Wert{12.0, 10}, Wert{}};
    w.helligkeit = {{0.4, 2}, {0.6, 2}};
    w.terzL = {-10.0, -20.5};
    w.terzR = {-11.0, -21.5};
    w.phantome = {{6, -32.5}, {8, -28.0}};
    const std::filesystem::path pfad = ordner / "werte.txt";
    REQUIRE(schreibeWerte(pfad, {w}));
    const std::optional<SalamanderWerte> gelesen = liesWerte(pfad);
    REQUIRE(gelesen.has_value());
    REQUIRE(gelesen->count(60) == 1);
    const auto& t = gelesen->at(60);
    CHECK(std::stod(t.at("B").at(0)) == doctest::Approx(3.5e-4).scale(0.0));
    CHECK(t.at("B").at(1) == "31");
    CHECK(std::stod(t.at("t60_frueh").at(0)) == doctest::Approx(6.25));
    CHECK(std::stod(t.at("helligkeit_3").at(0)) == doctest::Approx(0.6));
    CHECK(t.at("terz_rechts") == std::vector<std::string>{"-11", "-21.5"});
    CHECK(t.at("phantome") == std::vector<std::string>{"6:-32.5", "8:-28"});
    CHECK(t.at("t60_spaet").at(1) == "0");  // nicht gemessen: Anzahl 0

    const std::vector<TastenWerte> zurueck = tastenWerteAus(*gelesen);
    REQUIRE(zurueck.size() == 1);
    CHECK(zurueck[0].midi == 60);
    CHECK(zurueck[0].b.anzahl == 31);
    CHECK(zurueck[0].t60Frueh.wert == doctest::Approx(6.25));
    CHECK(zurueck[0].tiefeDb.wert == doctest::Approx(18.5));
    CHECK(zurueck[0].tiefeDb.anzahl == 24);
    CHECK(zurueck[0].tiefeLeise.wert == doctest::Approx(16.0));
    CHECK(zurueck[0].tiefeLaut.anzahl == 12);
    CHECK(zurueck[0].t60FruehK[1].wert == doctest::Approx(4.4));
    CHECK(zurueck[0].t60SpaetK[0].anzahl == 10);
    CHECK(zurueck[0].t60SpaetK[1].anzahl == 0);
    REQUIRE(zurueck[0].helligkeit.size() == 7);
    CHECK(zurueck[0].helligkeit[1].wert == doctest::Approx(0.6));
    CHECK(zurueck[0].helligkeit[2].anzahl == 0);
    CHECK(zurueck[0].terzR == std::vector<double>{-11.0, -21.5});
    CHECK(zurueck[0].phantome == std::vector<std::pair<int, double>>{{6, -32.5}, {8, -28.0}});
    std::filesystem::remove_all(ordner);
}

TEST_CASE("Loop Salamander 31d-7 B Kurven: Kanalmittel und Datei hin und zurueck (7.5.2)")
{
    // zwei Kanaele gleich laut: das Mittel ist derselbe Pegel; einer 10 dB leiser: 10·lg((1 + 0,1)/2) = −2,6 dB
    Teilton a;
    a.k = 1;
    a.pegelDb = {0.0, -10.0, -20.0, -30.0};
    a.bodenDb = {-80.0, -80.0, -80.0, -80.0};
    a.bereichVon = 1;
    a.bereichBis = 3;
    Teilton b = a;
    b.pegelDb = {-10.0, -20.0, -30.0};
    b.bereichVon = 0;
    b.bereichBis = 3;
    const Teilton m = kanalMittel(a, b);
    REQUIRE(m.pegelDb.size() == 3);
    CHECK(m.pegelDb[0] == doctest::Approx(10.0 * std::log10(0.55)));
    CHECK(m.bodenDb[0] == doctest::Approx(-80.0));
    CHECK(m.bereichVon == 1);
    CHECK(m.bereichBis == 3);
    CHECK(kanalMittel(a, Teilton{}).pegelDb == a.pegelDb);

    const std::filesystem::path ordner =
        std::filesystem::temp_directory_path() / ("lumiviz_kurven_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    REQUIRE(std::filesystem::create_directories(ordner));
    KurvenBezug bezug;
    TeiltonKurven z;
    z.erkannt = true;
    z.bereichVon = 12;
    z.bereichBis = 840;
    z.kurve.rasterS = 0.05;
    z.kurve.ersterPunkt = 3;
    z.kurve.db = {0.0, -1.25, -2.5, -3.75};
    z.kurve.gueltig = 3;
    z.kovarianz.rasterS = 0.02;
    z.kovarianz.db2 = {9.25, -7.5, 0.0125};
    bezug[{60, 12, 2}] = z;
    bezug[{60, 12, 3}] = TeiltonKurven{};  // nicht erkannt: nicht geschrieben
    const std::filesystem::path pfad = ordner / "kurven.txt";
    REQUIRE(schreibeKurven(pfad, bezug));
    const std::optional<KurvenBezug> gelesen = liesKurven(pfad);
    REQUIRE(gelesen.has_value());
    REQUIRE(gelesen->size() == 1);
    const TeiltonKurven& g = gelesen->at({60, 12, 2});
    CHECK(g.erkannt);
    CHECK(g.bereichVon == 12);
    CHECK(g.bereichBis == 840);
    CHECK(g.kurve.ersterPunkt == 3);
    CHECK(g.kurve.gueltig == 3);
    CHECK(g.kurve.db == std::vector<double>{0.0, -1.25, -2.5, -3.75});
    REQUIRE(g.kovarianz.db2.size() == 3);
    CHECK(g.kovarianz.db2[0] == doctest::Approx(9.25));
    CHECK(g.kovarianz.db2[2] == doctest::Approx(0.0125));
    std::filesystem::remove_all(ordner);
}

TEST_CASE("Loop Salamander 31d-7 B mitgelieferte Kurven (7.5.2)")
{
    // K1 a, K2 a, K5 a: je Taste die Ebenen v6–v16, Teilton 1–3; die Kurve faellt, die Autokovarianz reicht bis 2 s
    const std::optional<KurvenBezug> bezug = liesKurven(kKurvenDatei);
    REQUIRE_MESSAGE(bezug.has_value(), "werte/salamander_kurven.txt nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    std::map<int, int> jeTaste;
    for (const auto& [s, z] : *bezug)
    {
        CAPTURE(s.midi);
        CAPTURE(s.ebene);
        CAPTURE(s.k);
        CHECK(s.ebene >= kKurvenEbeneAb);
        CHECK(s.ebene <= kEbenen);
        CHECK(s.k >= 1);
        CHECK(s.k <= kKurvenTeiltoene);
        REQUIRE(z.erkannt);
        REQUIRE_FALSE(z.kurve.db.empty());
        CHECK(z.kurve.db.front() == 0.0);
        CHECK(z.kurve.gueltig <= z.kurve.db.size());
        CHECK(std::is_sorted(z.kurve.db.rbegin(), z.kurve.db.rend()));  // nicht steigend
        CHECK(z.bereichBis > z.bereichVon);
        CHECK(z.kovarianz.db2.size() <= 101);
        ++jeTaste[s.midi];
    }
    CHECK(jeTaste.size() == 30);
    for (const auto& [midi, zahl] : jeTaste)
    {
        CAPTURE(midi);
        CHECK(zahl >= 11);  // mindestens Teilton 1 (oder einer) je Ebene
    }
    MESSAGE(bezug->size() << " Kurven, " << std::filesystem::file_size(kKurvenDatei) / 1024 << " KiB");

    // 7.5.3: der Abstand benachbarter Ebenen derselben Taste — die Groessenordnung, in der *Salamander* sich selbst
    // streut (nur berichtet; die Schwellen der Probe stehen in SW1 a)
    std::vector<double> abklingen;
    std::vector<double> schwebung;
    for (const auto& [s, z] : *bezug)
        if (const auto nachbar = bezug->find({s.midi, s.ebene + 1, s.k}); nachbar != bezug->end())
            if (const KurvenAbstand a = kurvenAbstand(nachbar->second, z); a.gilt)
            {
                abklingen.push_back(a.abklingenDb);
                schwebung.push_back(a.schwebungDb);
            }
    REQUIRE_FALSE(abklingen.empty());
    const auto quantil = [](std::vector<double> v, double p) {
        std::sort(v.begin(), v.end());
        return v[static_cast<std::size_t>(p * static_cast<double>(v.size() - 1))];
    };
    MESSAGE("Benachbarte Ebenen (" << abklingen.size() << " Paare): Abklingen Median " << quantil(abklingen, 0.5) << " dB, 90 % "
                                    << quantil(abklingen, 0.9) << " dB; Schwebung Median " << quantil(schwebung, 0.5) << " dB, 90 % "
                                    << quantil(schwebung, 0.9) << " dB");
}

TEST_CASE("Loop Salamander 31d-3 Boden im Vergleich (UF6 a)")
{
    // drei Tasten; a = b + eine gemeinsame Kurve (Raum, Mikrofon) + bei Taste 2 ein Buckel von 3 dB in einem Band
    const std::vector<int> tasten{21, 60, 96};
    const std::size_t baender = terzMitten().size();
    std::vector<std::vector<double>> a(3, std::vector<double>(baender));
    std::vector<std::vector<double>> b(3, std::vector<double>(baender));
    for (std::size_t t = 0; t < 3; ++t)
        for (std::size_t band = 0; band < baender; ++band)
        {
            b[t][band] = -20.0 - static_cast<double>(band) - 5.0 * static_cast<double>(t);
            a[t][band] = b[t][band] + 0.5 * static_cast<double>(band);
        }
    a[1][20] += 3.0;
    const BodenVergleich v = vergleicheBoden(tasten, a, b);
    REQUIRE(v.abweichungDb.size() == 3);
    // Band 0 (31 Hz) zaehlt nur fuer A0 (27,5 Hz); Band 20 (3,2 kHz) fuer alle drei
    CHECK(v.mittelDb[0] == doctest::Approx(0.0));
    CHECK(v.mittelDb[20] == doctest::Approx(10.0 + 1.0));
    CHECK(v.mittelDb[5] == doctest::Approx(2.5));
    CHECK(v.abweichungDb[0] > 0.0);
    CHECK(v.abweichungDb[1] > v.abweichungDb[0]);  // der Buckel liegt bei Taste 2, das Mittel nimmt ein Drittel davon
}

TEST_CASE("Loop Salamander 31d-3 mitgelieferte Werte")
{
    const std::optional<SalamanderWerte> werte = liesWerte(kWerteDatei);
    REQUIRE_MESSAGE(werte.has_value(), "werte/salamander_c5.txt nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    REQUIRE(werte->size() == 30);
    for (const SalamanderTaste& taste : salamanderTasten())
    {
        CAPTURE(taste.midi);
        REQUIRE(werte->count(taste.midi) == 1);
        const auto& t = werte->at(taste.midi);
        REQUIRE(t.count("B") == 1);
        if (taste.midi <= 90) CHECK(std::stoi(t.at("B").at(1)) > 0);
        if (std::stoi(t.at("B").at(1)) > 0)
        {
            const double b = std::stod(t.at("B").at(0));
            CHECK(b > 1e-6);
            CHECK(b < 0.05);
        }
        REQUIRE(t.count("t60_ausgleich") == 1);
        REQUIRE(t.count("t60_frueh") == 1);
        CHECK((std::stoi(t.at("t60_frueh").at(1)) > 0 || std::stoi(t.at("t60_ausgleich").at(1)) > 0));
        REQUIRE(t.count("terz_links") == 1);
        CHECK(t.at("terz_links").size() == terzMitten().size());
    }
}

TEST_CASE("Loop Salamander 31d-3 Messung (LUMIVIZ_SALAMANDER)")
{
    const QString variable = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_SALAMANDER=<Ordner> misst Salamander und schreibt werte/salamander_c5.txt neu");
        return;
    }
    const std::filesystem::path ordner = variable.toStdU16String();
    const std::vector<SalamanderTaste> tasten = salamanderTasten();
    for (const SalamanderTaste& taste : tasten)
        for (int e = 1; e <= kEbenen; ++e)
            REQUIRE_MESSAGE(std::filesystem::exists(salamanderDatei(ordner, taste, e)), salamanderDatei(ordner, taste, e).string());

    const auto start = std::chrono::steady_clock::now();
    const lumi::loop::Fft& fft = lumi::loop::fft();
    std::vector<std::vector<std::array<KanalMessung, 2>>> messungen(tasten.size());
    std::vector<std::vector<TonKurven>> kurven(tasten.size());  // 7.5.2: je Taste die Ebenen ab `kKurvenEbeneAb`
    std::vector<std::string> fehler(tasten.size());
    std::atomic<std::size_t> naechste{0};
    const unsigned zahl = std::max(1u, std::min(8u, std::thread::hardware_concurrency()));
    std::vector<std::thread> arbeiter;
    for (unsigned a = 0; a < zahl; ++a)
        arbeiter.emplace_back([&] {
            for (std::size_t i = naechste++; i < tasten.size(); i = naechste++)
            {
                const SalamanderTaste& taste = tasten[i];
                for (int e = 1; e <= kEbenen; ++e)
                {
                    const std::optional<lumi::audio::WavInhalt> wav = lumi::audio::liesWavDatei(salamanderDatei(ordner, taste, e));
                    if (!wav || wav->rate != 48000)
                    {
                        fehler[i] = salamanderDatei(ordner, taste, e).string() + " nicht zu lesen";
                        break;
                    }
                    const bool phantome = taste.midi <= kPhantomBis && e == kEbenen;
                    TonKurven k;
                    messungen[i].push_back(
                        messeTon(wav->stereo, wav->rate, tastenFrequenz(taste.midi), fft, phantome, e >= kKurvenEbeneAb ? &k : nullptr));
                    if (e >= kKurvenEbeneAb) kurven[i].push_back(k);
                }
            }
        });
    for (std::thread& t : arbeiter) t.join();
    for (const std::string& f : fehler) REQUIRE_MESSAGE(f.empty(), f);

    std::vector<TastenWerte> werte;
    for (std::size_t i = 0; i < tasten.size(); ++i) werte.push_back(verdichte(tasten[i].midi, messungen[i]));
    setzeRelativePegel(werte, messungen);
    const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    REQUIRE(schreibeWerte(kWerteDatei, werte));
    KurvenBezug bezug;
    for (std::size_t i = 0; i < tasten.size(); ++i)
        for (std::size_t j = 0; j < kurven[i].size(); ++j)
            for (int k = 1; k <= kKurvenTeiltoene; ++k)
                bezug[{tasten[i].midi, kKurvenEbeneAb + static_cast<int>(j), k}] = kurven[i][j][static_cast<std::size_t>(k - 1)];
    REQUIRE(schreibeKurven(kKurvenDatei, bezug));
    MESSAGE("Kurven: " << bezug.size() << " geschrieben nach " << kKurvenDatei.string() << ", "
                       << std::filesystem::file_size(kKurvenDatei) / 1024 << " KiB");

    std::ostringstream bericht;
    bericht << std::fixed << std::setprecision(2) << "Salamander gemessen in " << dauer << " s, geschrieben nach "
            << kWerteDatei.string() << "\n"
            << "Taste | B (1e-4) | T60 frueh / spaet s (zwei Stufen) | T60 Ausgleich | highDecay | Schwebung Cent | "
               "Hell. 2 / 4 dB/dB | Umfang dB | Pegel L / R dB | Direkt dB | Phantome\n";
    for (const TastenWerte& w : werte)
    {
        bericht << w.midi << " | " << 1e4 * w.b.wert << " (" << w.b.anzahl << ") | " << w.t60Frueh.wert << " / " << w.t60Spaet.wert
                << " (" << std::setprecision(0) << 100.0 * w.zweiStufenAnteil.wert << " %)" << std::setprecision(2) << " | "
                << w.t60Ausgleich.wert << " | " << w.highDecay.wert << " | " << w.schwebungCent.wert << " ("
                << w.schwebungCent.anzahl << ") | " << w.helligkeit[0].wert << " / " << w.helligkeit[2].wert << " | "
                << w.pegelumfang.wert << " | " << w.pegelL.wert << " / " << w.pegelR.wert << " | " << w.direkt.wert << " |";
        for (const auto& [n, db] : w.phantome) bericht << ' ' << n << ':' << db;
        bericht << '\n';
    }
    // die Blende am Ende: wie viel jeder Datei gemessen wurde
    std::vector<double> anteile;
    for (const auto& taste : messungen)
        for (const auto& ton : taste) anteile.push_back(ton[0].endeS / ton[0].laengeS);
    std::sort(anteile.begin(), anteile.end());
    bericht << "Gemessen bis vor die Blende: Anteil der Datei kleinster " << anteile.front() << ", Median "
            << anteile[anteile.size() / 2] << ", groesster " << anteile.back() << '\n';

    // die Einbrueche (TB1 a, Befund S154): wie viele der gezaehlten Schwebungen haben eine schwache
    // Grundschwingung (unter 2 dB) — ohne zwei Stufen der Verdacht auf eine verfehlte Stufe (C4 v12 links);
    // ein Einbruch ueber dreimal der Grundschwingung ist kein Fehler (scharfe echte Einbrueche, MIDI 81)
    bericht << "Einbrueche, Teiltoene 1-3, v6-v16: Taste | gezaehlt | Grundschw. < 2 dB | davon mit zwei Stufen | "
               "Einbruch > 3 x Grundschw. | tiefe_db alle | ohne Grundschw. < 2 dB (Anzahl)\n";
    for (std::size_t i = 0; i < tasten.size(); ++i)
    {
        std::vector<double> alle, sauber;
        int schwach = 0, verdacht = 0, verdachtZwei = 0;
        for (std::size_t e = 5; e < messungen[i].size(); ++e)
            for (const KanalMessung& m : messungen[i][e])
                for (int k = 1; k <= 3; ++k)
                    if (const TeiltonKurz* t = m.teilton(k); t != nullptr && t->schwebung.erkannt)
                    {
                        alle.push_back(t->schwebung.einbruchDb);
                        const bool klein = t->schwebung.tiefeDb < 2.0;
                        const bool weit = t->schwebung.einbruchDb > 3.0 * t->schwebung.tiefeDb;
                        schwach += klein ? 1 : 0;
                        verdacht += weit ? 1 : 0;
                        verdachtZwei += klein && t->abklingen.zweiStufen ? 1 : 0;
                        if (!klein) sauber.push_back(t->schwebung.einbruchDb);
                    }
        bericht << tasten[i].midi << " | " << alle.size() << " | " << schwach << " | " << verdachtZwei << " | " << verdacht << " | "
                << detail::medianWert(alle).wert << " | " << detail::medianWert(sauber).wert << " (" << sauber.size() << ")\n";
    }

    // die Tiefe je Stufe (US2, Bank 2000 S. 29: das Verhaeltnis der Saiten aendert sich mit der Staerke) — je Taste
    // der Median ueber Teilton 1–3 und beide Kanaele, in Gruppen zu vier Ebenen
    bericht << "Tiefe je Stufe: Taste | v1-v4 | v5-v8 | v9-v12 | v13-v16 (dB, Anzahl)\n";
    for (std::size_t i = 0; i < tasten.size(); ++i)
    {
        bericht << tasten[i].midi;
        for (std::size_t gruppe = 0; gruppe < 4; ++gruppe)
        {
            std::vector<double> t;
            for (std::size_t e = 4 * gruppe; e < 4 * gruppe + 4 && e < messungen[i].size(); ++e)
                for (const KanalMessung& m : messungen[i][e])
                    for (int k = 1; k <= 3; ++k)
                        if (const TeiltonKurz* teil = m.teilton(k); teil != nullptr && teil->schwebung.erkannt) t.push_back(teil->schwebung.einbruchDb);
            const Wert w = detail::medianWert(t);
            bericht << " | " << w.wert << " (" << w.anzahl << ')';
        }
        bericht << '\n';
    }
    MESSAGE(bericht.str());
    for (const TastenWerte& w : werte)
    {
        CAPTURE(w.midi);
        // B aus mindestens fuenf Teiltoenen unter 0,45·fs: bis F#6 (darueber bleibt B bei Rigaud, UF5 a)
        if (w.midi <= 90) CHECK(w.b.anzahl > 0);
        // eine Abklingzeit je Taste: aus Teilton 1 oder aus dem Ausgleich (C8 hat zu wenige Teiltoene dafuer)
        CHECK((w.t60Frueh.anzahl > 0 || w.t60Ausgleich.anzahl > 0));
    }
}

TEST_CASE("Loop Salamander 31d-3 Modell (LUMIVIZ_31D3_MODELL)")
{
    // Teil 2: dieselbe Messung an den gerechneten Toenen (UF3 a) — der Steinway D wie im Programm
    // (`rendereTon`, Boden lib:steinway_d), die Stufen −30 … 0 dB an der Stelle der Ebenen (V1 a); dazu
    // der Glasboden gleicher Dicke an Stufe −8 fuer den Kipppunkt von UF4 a (unterscheiden sich Boeden
    // im Verhaeltnis zu Salamander?). Schreibt `modell_steinway.txt` und `vergleich.txt` in den Ordner.
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D3_MODELL");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D3_MODELL=<Ordner> misst das Modell und vergleicht es mit werte/salamander_c5.txt");
        return;
    }
    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE_MESSAGE(gelesen.has_value(), "werte/salamander_c5.txt nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    const std::vector<TastenWerte> salamander = tastenWerteAus(*gelesen);
    REQUIRE(salamander.size() == 30);

    lumi::loop::tonspeicher().leere();
    const lumi::loop::Instrument holz = modelltest::fluegel(QStringLiteral("lib:steinway_d"));
    const lumi::loop::Instrument glas = modelltest::fluegel(QStringLiteral("lib:steinway_d_glass"));
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    REQUIRE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d_glass")));

    const int rate = modelltest::kRate;
    const std::vector<SalamanderTaste> tasten = salamanderTasten();
    std::vector<int> midis;
    for (const SalamanderTaste& t : tasten) midis.push_back(t.midi);
    constexpr int kVergleichsEbene = 12;  // v12 bzw. Stufe −8: der Boden (UF6 a)
    const auto start = std::chrono::steady_clock::now();
    const std::vector<std::vector<std::array<KanalMessung, 2>>> messungen = modelltest::messeTasten(holz, midis);
    std::vector<std::array<KanalMessung, 2>> glasMessung;
    for (const int midi : midis) glasMessung.push_back(modelltest::messe(glas, midi, modelltest::stufeVonEbene(kVergleichsEbene)));
    const std::vector<TastenWerte> modell = modelltest::verdichteAlle(midis, messungen);
    const double dauer = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    REQUIRE(schreibeWerte(ziel / "modell_steinway.txt", modell, kModellKopf));

    // der Boden (UF6 a): je Kanal Salamander gegen Holz, Salamander gegen Glas, Holz gegen Glas
    std::array<BodenVergleich, 2> gegenHolz;
    std::array<BodenVergleich, 2> gegenGlas;
    std::array<BodenVergleich, 2> holzGlas;
    for (std::size_t kanal = 0; kanal < 2; ++kanal)
    {
        std::vector<std::vector<double>> sal, h, g;
        for (std::size_t i = 0; i < tasten.size(); ++i)
        {
            sal.push_back(kanal == 0 ? salamander[i].terzL : salamander[i].terzR);
            h.push_back(messungen[i][kVergleichsEbene - 1][kanal].terzDb);
            g.push_back(glasMessung[i][kanal].terzDb);
        }
        gegenHolz[kanal] = vergleicheBoden(midis, sal, h);
        gegenGlas[kanal] = vergleicheBoden(midis, sal, g);
        holzGlas[kanal] = vergleicheBoden(midis, h, g);
    }

    std::ostringstream b;
    b << std::fixed << std::setprecision(2) << "Modell gemessen in " << dauer << " s (30 Tasten, je 16 Stufen und ein Glaston)\n"
      << "Je Taste Salamander / Modell. T60 in s, B in 1e-4, Helligkeit in dB je dB, Pegel und Direktschall in dB.\n"
      << "Taste | B | T60 frueh | T60 spaet (Anteil zweistufig) | T60 Ausgleich | highDecay | Schwebung Cent | Hell. 2 | "
         "Hell. 4 | Umfang | Pegel L | Pegel R | Direkt | Boden L/R: Sal.-Holz, Sal.-Glas, Holz-Glas\n";
    const auto paar = [&b](const Wert& x, const Wert& y, double faktor = 1.0) {
        b << ' ' << (x.anzahl > 0 ? faktor * x.wert : 0.0) << " / " << (y.anzahl > 0 ? faktor * y.wert : 0.0) << " |";
    };
    for (std::size_t i = 0; i < tasten.size(); ++i)
    {
        const TastenWerte& sa = salamander[i];
        const TastenWerte& mo = modell[i];
        b << sa.midi << " |";
        paar(sa.b, mo.b, 1e4);
        paar(sa.t60Frueh, mo.t60Frueh);
        b << ' ' << sa.t60Spaet.wert << " (" << std::setprecision(0) << 100.0 * sa.zweiStufenAnteil.wert << " %) / " << std::setprecision(2)
          << mo.t60Spaet.wert << " (" << std::setprecision(0) << 100.0 * mo.zweiStufenAnteil.wert << " %) |" << std::setprecision(2);
        paar(sa.t60Ausgleich, mo.t60Ausgleich);
        paar(sa.highDecay, mo.highDecay);
        paar(sa.schwebungCent, mo.schwebungCent);
        paar(sa.helligkeit[0], mo.helligkeit[0]);
        paar(sa.helligkeit[2], mo.helligkeit[2]);
        paar(sa.pegelumfang, mo.pegelumfang);
        paar(sa.pegelL, mo.pegelL);
        paar(sa.pegelR, mo.pegelR);
        paar(sa.direkt, mo.direkt);
        b << ' ' << gegenHolz[0].abweichungDb[i] << '/' << gegenHolz[1].abweichungDb[i] << ", " << gegenGlas[0].abweichungDb[i] << '/'
          << gegenGlas[1].abweichungDb[i] << ", " << holzGlas[0].abweichungDb[i] << '/' << holzGlas[1].abweichungDb[i] << '\n';
    }
    const auto mittel = [](const std::vector<double>& v) {
        double s = 0.0;
        for (const double x : v) s += x;
        return v.empty() ? 0.0 : s / static_cast<double>(v.size());
    };
    b << "Boden, Abweichung vom Mittel der Tastatur, gemittelt ueber die Tasten (L/R): Salamander gegen Holz "
      << mittel(gegenHolz[0].abweichungDb) << '/' << mittel(gegenHolz[1].abweichungDb) << " dB, gegen Glas "
      << mittel(gegenGlas[0].abweichungDb) << '/' << mittel(gegenGlas[1].abweichungDb) << " dB, Holz gegen Glas "
      << mittel(holzGlas[0].abweichungDb) << '/' << mittel(holzGlas[1].abweichungDb) << " dB\n"
      << "Mittel der Tastatur Salamander gegen Holz je Terzband (L, Raum + Mikrofon + Fluegel):";
    const std::vector<double> mitten = terzMitten();
    for (std::size_t band = 0; band < mitten.size(); ++band)
        b << ' ' << std::setprecision(0) << mitten[band] << ':' << std::setprecision(1) << gegenHolz[0].mittelDb[band];
    b << std::setprecision(2) << "\nPhantome im Bass (v16 / Stufe 0), n: Salamander / Modell in dB gegen den Querteilton\n";
    for (std::size_t i = 0; i < tasten.size(); ++i)
    {
        if (tasten[i].midi > kPhantomBis) continue;
        b << tasten[i].midi << ':';
        for (const auto& [n, db] : salamander[i].phantome)
        {
            b << ' ' << n << ' ' << db << " / ";
            const auto it = std::find_if(modell[i].phantome.begin(), modell[i].phantome.end(), [n = n](const auto& p) { return p.first == n; });
            if (it != modell[i].phantome.end())
                b << it->second;
            else
                b << '-';
            b << ';';
        }
        b << '\n';
    }
    std::ofstream(ziel / "vergleich.txt", std::ios::binary) << b.str();
    MESSAGE(b.str());

    // einige Toene des Modells zum Ansehen und Hoeren (32-bit float, wie gerechnet)
    for (const int midi : {36, 39, 60, 84, 108})
        for (const int stufe : {-30, -8, 0})
        {
            const lumi::loop::Klang k = modelltest::ton(holz, midi, stufe);
            const std::filesystem::path wav = ziel / ("modell_" + std::to_string(midi) + "_" + std::to_string(stufe) + ".wav");
            CHECK(lumi::audio::schreibeWav(QString::fromStdU16String(wav.u16string()), k.daten, rate, lumi::audio::WavTiefe::Gleitkomma32)
                      .geschrieben);
        }
    for (const TastenWerte& w : modell)
    {
        CAPTURE(w.midi);
        CHECK((w.t60Frueh.anzahl > 0 || w.t60Ausgleich.anzahl > 0));
    }
}

namespace
{

/// Der linke Kanal eines Tons in Stereo, bis vor die Blende am Ende, gemessen mit den Huellkurven.
Ergebnis linksGemessen(std::span<const float> stereo, int rate, int midi)
{
    auto [links, rechts] = trenne(stereo);
    std::vector<double> summe(links.size());
    for (std::size_t i = 0; i < summe.size(); ++i) summe[i] = links[i] + rechts[i];
    links.resize(endeVorBlende(summe, rate));
    return miss(links, rate, tastenFrequenz(midi), lumi::loop::fft());
}

struct BildKurve
{
    QString name;
    QColor farbe;
    Ergebnis messung;
};

/// Ein Bild je Taste: drei Zeilen (Teilton 1–3), jede Kurve auf ihre Spitze zu Beginn bezogen.
QImage huellkurvenBild(int midi, const std::vector<BildKurve>& kurven, double bisS, double kUnten = -90.0)
{
    constexpr int kBreite = 1600;
    constexpr int kZeile = 330;
    constexpr int kRandL = 60;
    constexpr int kRandR = 560;
    constexpr int kRandO = 30;
    QImage bild(kBreite, 3 * kZeile + 10, QImage::Format_ARGB32);
    bild.fill(Qt::white);
    QPainter p(&bild);
    p.setRenderHint(QPainter::Antialiasing);
    const double breite = kBreite - kRandL - kRandR;
    const double hoehe = kZeile - kRandO - 30;
    for (int n = 1; n <= 3; ++n)
    {
        const double oben = (n - 1) * kZeile + kRandO;
        const auto x = [&](double s) { return kRandL + breite * s / bisS; };
        const auto y = [&](double db) { return oben + hoehe * std::clamp(db, kUnten, 0.0) / kUnten; };
        p.setPen(QColor(225, 225, 225));
        for (int s = 0; s <= static_cast<int>(bisS); ++s) p.drawLine(QPointF(x(s), y(0.0)), QPointF(x(s), y(kUnten)));
        for (int db = 0; db >= static_cast<int>(kUnten); db -= 10) p.drawLine(QPointF(x(0.0), y(db)), QPointF(x(bisS), y(db)));
        p.setPen(Qt::black);
        p.drawText(QPointF(kRandL, oben - 8),
                   QStringLiteral("MIDI %1, Teilton %2 — Pegel in dB gegen die Spitze zu Beginn, Raster 1 s / 10 dB").arg(midi).arg(n));
        for (int db = 0; db >= static_cast<int>(kUnten); db -= 20) p.drawText(QPointF(8, y(db) + 4), QString::number(db));
        double legende = oben + 16;
        for (const BildKurve& k : kurven)
        {
            const Teilton* t = k.messung.teilton(n);
            if (t == nullptr || t->pegelDb.empty()) continue;
            double spitze = -300.0;
            const auto anfang = static_cast<std::size_t>(0.1 / k.messung.schrittS);
            for (std::size_t i = 0; i < std::min(anfang + 1, t->pegelDb.size()); ++i) spitze = std::max(spitze, t->pegelDb[i]);
            QPainterPath pfad;
            for (std::size_t i = 0; i < t->pegelDb.size(); ++i)
            {
                const double s = static_cast<double>(i) * k.messung.schrittS;
                if (s > bisS) break;
                const QPointF punkt(x(s), y(t->pegelDb[i] - spitze));
                if (i == 0)
                    pfad.moveTo(punkt);
                else
                    pfad.lineTo(punkt);
            }
            p.setPen(QPen(k.farbe, 1.4));
            p.drawPath(pfad);
            QString text = k.name;
            if (t->abklingen.erkannt)
                text += QStringLiteral(": T60 %1").arg(t->abklingen.t60Frueh, 0, 'f', 1) +
                        (t->abklingen.zweiStufen ? QStringLiteral(" / %1 s, Knick %2 s").arg(t->abklingen.t60Spaet, 0, 'f', 1).arg(t->abklingen.knickS, 0, 'f', 1)
                                                 : QStringLiteral(" s, eine Stufe")) +
                        QStringLiteral(", gemessen %1–%2 s").arg(t->abklingen.vonS, 0, 'f', 1).arg(t->abklingen.bisS, 0, 'f', 1);
            else
                text += QStringLiteral(": kein Abklingen");
            QString zweite = t->schwebung.erkannt ? QStringLiteral("    Schwebung %1 Hz, Einbruch %2 dB (Grundschwingung %3 dB)")
                                                        .arg(t->schwebung.frequenz, 0, 'f', 2)
                                                        .arg(t->schwebung.einbruchDb, 0, 'f', 1)
                                                        .arg(t->schwebung.tiefeDb, 0, 'f', 1)
                                                  : QStringLiteral("    keine Schwebung erkannt");
            p.setPen(k.farbe);
            p.drawText(QPointF(kBreite - kRandR + 10, legende), text);
            p.drawText(QPointF(kBreite - kRandR + 10, legende + 16), zweite);
            legende += 40;
        }
    }
    return bild;
}

}  // namespace

TEST_CASE("Loop Salamander 31d-7 Bilder der Huellkurven (LUMIVIZ_31D7_BILD)")
{
    // P8 a 7: vor dem Setzen ansehen, ob die Messung die Einbrueche beider Seiten trifft und wie γ und δ sie
    // formen — Salamander v12 gegen das Modell an Stufe −8 (V1 a), linker Kanal; das Modell mit der
    // Vorgabe (γ = δ = 0) und zwei Einstellungen von Hand
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D7_BILD");
    const QString salamanderOrdner = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    if (variable.isEmpty() || salamanderOrdner.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D7_BILD=<Ordner> mit LUMIVIZ_SALAMANDER=<Ordner> legt je Taste ein Bild der Huellkurven ab");
        return;
    }
    std::unique_ptr<QGuiApplication> anwendung;
    int argc = 1;
    char arg0[] = "UnitTests";
    char* argv[] = {arg0, nullptr};
    if (QCoreApplication::instance() == nullptr) anwendung = std::make_unique<QGuiApplication>(argc, argv);

    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::filesystem::path ordner = salamanderOrdner.toStdU16String();
    lumi::loop::tonspeicher().leere();
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");

    struct Einstellung
    {
        QString name;
        QColor farbe;
        double gammaGrad;
        double deltaCent;
    };
    const std::vector<Einstellung> einstellungen{{QStringLiteral("Modell γ = 0"), QColor(150, 150, 150), 0.0, 0.0},
                                                 {QStringLiteral("Modell γ = 20°, δ = 2 Cent"), QColor(30, 90, 220), 20.0, 2.0},
                                                 {QStringLiteral("Modell γ = 45°, δ = 2 Cent"), QColor(220, 50, 40), 45.0, 2.0}};
    constexpr int kEbene = 12;
    const std::vector<int> tasten{24, 33, 42, 60, 81};
    for (const int midi : tasten)
    {
        CAPTURE(midi);
        const std::vector<SalamanderTaste> alle = salamanderTasten();
        const SalamanderTaste& st = *std::find_if(alle.begin(), alle.end(), [midi](const SalamanderTaste& t) { return t.midi == midi; });
        const std::optional<lumi::audio::WavInhalt> wav = lumi::audio::liesWavDatei(salamanderDatei(ordner, st, kEbene));
        REQUIRE(wav.has_value());
        std::vector<BildKurve> kurven{{QStringLiteral("Salamander v%1").arg(kEbene), Qt::black, linksGemessen(wav->stereo, wav->rate, midi)}};
        for (const Einstellung& e : einstellungen)
        {
            lumi::loop::HammerSaite saite = lumi::loop::hammerSaiteVorgabe();
            saite.polarisationWinkelGrad = {{{lumi::loop::kTasteMin, e.gammaGrad}, {lumi::loop::kTasteMax, e.gammaGrad}}, {}};
            saite.polarisationCent = {{{lumi::loop::kTasteMin, e.deltaCent}, {lumi::loop::kTasteMax, e.deltaCent}}, {}};
            const lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            const lumi::loop::Klang klang = modelltest::ton(instrument, midi, modelltest::stufeVonEbene(kEbene));
            kurven.push_back({e.name, e.farbe, linksGemessen(klang.daten, modelltest::kRate, midi)});
        }
        for (const BildKurve& k : kurven) CHECK(k.messung.erkannt);
        const QImage bild = huellkurvenBild(midi, kurven, midi <= 72 ? 11.0 : 7.0);
        const std::filesystem::path pfad = ziel / ("huellkurven_" + std::to_string(midi) + ".png");
        CHECK(bild.save(QString::fromStdU16String(pfad.u16string())));
    }
    MESSAGE("Bilder unter " << ziel.string());
}

TEST_CASE("Loop Salamander 31d-7 was die Tiefe im Chor bestimmt (LUMIVIZ_31D7_TIEFE)")
{
    // SZ1 a: an Tasten mit drei Saiten bei fester Vorgabe je eine Groesse veraendert — die zweite Ebene, die
    // Verstimmung der Saiten, ein ungleicher Stoss des Hammers (seit 7.2 ueber `stossLeise`/`stossLaut`);
    // je Fall die Tiefe der Einbrueche (Teilton 1–3, linker Kanal, Stufe −8) und Bilder gegen Salamander v12
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D7_TIEFE");
    const QString salamanderOrdner = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    if (variable.isEmpty() || salamanderOrdner.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D7_TIEFE=<Ordner> mit LUMIVIZ_SALAMANDER=<Ordner> untersucht die Tiefe der Einbrueche im Chor");
        return;
    }
    std::unique_ptr<QGuiApplication> anwendung;
    int argc = 1;
    char arg0[] = "UnitTests";
    char* argv[] = {arg0, nullptr};
    if (QCoreApplication::instance() == nullptr) anwendung = std::make_unique<QGuiApplication>(argc, argv);

    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::filesystem::path ordner = salamanderOrdner.toStdU16String();
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE(gelesen.has_value());
    const std::vector<TastenWerte> salamander = tastenWerteAus(*gelesen);

    struct Fall
    {
        QString name;
        QColor farbe;
        int bild;  ///< 0: zweite Ebene und Verstimmung, 1: Stoss des Hammers
        double gammaGrad = 0.0;
        double deltaCent = 0.0;
        double verstimmungFaktor = 1.0;
        double stoss = 0.0;  ///< u (US2 a): 1 + u : 1 : 1 − u
        double spaetFaktor = 1.0;  ///< `lateDecay` der Vorgabe mal so viel (US7: die zweite Stufe an 81)
    };
    const std::vector<Fall> faelle{
        {QStringLiteral("Vorgabe"), QColor(150, 150, 150), 2},
        {QStringLiteral("γ = 20°, δ = 2 Cent"), QColor(30, 90, 220), 0, 20.0, 2.0},
        {QStringLiteral("γ = 20°, δ = 0"), QColor(0, 160, 160), 0, 20.0, 0.0},
        {QStringLiteral("Verstimmung 0"), QColor(220, 50, 40), 0, 0.0, 0.0, 0.0},
        {QStringLiteral("Verstimmung × 0,5"), QColor(230, 140, 20), 0, 0.0, 0.0, 0.5},
        {QStringLiteral("Verstimmung × 2"), QColor(140, 60, 170), 0, 0.0, 0.0, 2.0},
        // u = 0,25 ist der Stoss 1 : 0,8 : 0,6 der ersten Untersuchung (auf das Mittel bezogen)
        {QStringLiteral("Stoss u = 0,25"), QColor(30, 90, 220), 1, 0.0, 0.0, 1.0, 0.25},
        {QStringLiteral("Stoss u = 0,6"), QColor(220, 50, 40), 1, 0.0, 0.0, 1.0, 0.6},
        {QStringLiteral("Stoss u = 0,25, γ = 20°, δ = 2 Cent"), QColor(0, 160, 160), 1, 20.0, 2.0, 1.0, 0.25},
        {QStringLiteral("Stoss u = 0,25, lateDecay × 3"), QColor(230, 140, 20), 1, 0.0, 0.0, 1.0, 0.25, 3.0},
    };
    const auto tiefen = [](const Ergebnis& e) {
        std::vector<double> t;
        for (int k = 1; k <= 3; ++k)
            if (const Teilton* teil = e.teilton(k); teil != nullptr && teil->schwebung.erkannt) t.push_back(teil->schwebung.einbruchDb);
        return t;
    };
    const auto text = [](const std::vector<double>& t) {
        std::ostringstream s;
        s << std::fixed << std::setprecision(1);
        for (const double x : t) s << x << ' ';
        if (t.empty()) s << "- ";
        return s.str();
    };

    constexpr int kEbene = 12;
    std::ostringstream bericht;
    bericht << "Tiefe der Einbrueche, Teilton 1-3 mit erkannter Schwebung, linker Kanal, v12 / Stufe -8 (dB)\n";
    for (const int midi : {60, 66, 81})
    {
        CAPTURE(midi);
        const std::vector<SalamanderTaste> alle = salamanderTasten();
        const SalamanderTaste& st = *std::find_if(alle.begin(), alle.end(), [midi](const SalamanderTaste& t) { return t.midi == midi; });
        const std::optional<lumi::audio::WavInhalt> wav = lumi::audio::liesWavDatei(salamanderDatei(ordner, st, kEbene));
        REQUIRE(wav.has_value());
        const BildKurve sal{QStringLiteral("Salamander v%1").arg(kEbene), Qt::black, linksGemessen(wav->stereo, wav->rate, midi)};
        const auto it = std::find_if(salamander.begin(), salamander.end(), [midi](const TastenWerte& w) { return w.midi == midi; });
        bericht << "MIDI " << midi << ": Salamander v12 " << text(tiefen(sal.messung)) << "| tiefe_db v6-v16 " << std::fixed
                << std::setprecision(1) << (it != salamander.end() ? it->tiefeDb.wert : 0.0) << '\n';
        std::array<std::vector<BildKurve>, 2> bilder{std::vector<BildKurve>{sal}, std::vector<BildKurve>{sal}};
        for (const Fall& f : faelle)
        {
            lumi::loop::HammerSaite saite = lumi::loop::hammerSaiteVorgabe();
            const auto ueberall = [](double wert) {
                return lumi::loop::Tastenkurve{{{lumi::loop::kTasteMin, wert}, {lumi::loop::kTasteMax, wert}}, {}};
            };
            saite.polarisationWinkelGrad = ueberall(f.gammaGrad);
            saite.polarisationCent = ueberall(f.deltaCent);
            for (lumi::loop::TastenWert& p : saite.verstimmungCent.punkte) p.wert *= f.verstimmungFaktor;
            for (lumi::loop::TastenWert& p : saite.abklingSpaetS.punkte) p.wert *= f.spaetFaktor;
            saite.stossLeise = ueberall(f.stoss);
            saite.stossLaut = ueberall(f.stoss);
            const lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            const lumi::loop::Klang klang = modelltest::ton(instrument, midi, modelltest::stufeVonEbene(kEbene));
            const BildKurve kurve{f.name, f.farbe, linksGemessen(klang.daten, modelltest::kRate, midi)};
            CHECK(kurve.messung.erkannt);
            bericht << "  " << f.name.toStdString() << ": " << text(tiefen(kurve.messung)) << '\n';
            if (f.bild == 2)
            {
                bilder[0].push_back(kurve);
                bilder[1].push_back(kurve);
            }
            else
                bilder[static_cast<std::size_t>(f.bild)].push_back(kurve);
        }
        for (std::size_t b = 0; b < 2; ++b)
        {
            // bis −160 dB: auch, was unter dem Rauschen von Salamander liegt (US6 a, der Scheinwert an 81)
            const QImage bild = huellkurvenBild(midi, bilder[b], midi <= 72 ? 11.0 : 7.0, -160.0);
            const std::filesystem::path pfad = ziel / ("tiefe_" + std::to_string(midi) + (b == 0 ? "_ebene_verstimmung" : "_stoss") + ".png");
            CHECK(bild.save(QString::fromStdU16String(pfad.u16string())));
        }
    }
    std::ofstream(ziel / "tiefe.txt", std::ios::binary) << bericht.str();
    MESSAGE(bericht.str());
}

TEST_CASE("Loop Salamander 31d-7 Raster aus Stoss und γ (LUMIVIZ_31D7_RASTER)")
{
    // SZ2 a, Kipppunkt: laesst sich der Stoss je Saite getrennt von γ setzen? Je Taste ein Raster, gemessen ueber
    // alle 16 Stufen wie beim Setzen; reagieren Knick und Tiefe verschieden auf beide, sind sie trennbar
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D7_RASTER");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D7_RASTER=<Ordner> rechnet ein Raster aus Stoss je Saite und γ an MIDI 60 und 66");
        return;
    }
    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE(gelesen.has_value());
    const std::vector<TastenWerte> salamander = tastenWerteAus(*gelesen);
    const std::vector<int> tasten{60, 66};

    std::ostringstream bericht;
    bericht << std::fixed << std::setprecision(2)
            << "Raster: Stoss 1+u : 1 : 1-u, γ mit δ = 2 Cent; je Taste verdichtet ueber 16 Stufen (wie beim Setzen)\n"
            << "Taste | u | γ | Knick s (Anteil zwei Stufen) | T60 frueh / spaet s | Tiefe dB (Anzahl) | Schwebung Cent\n";
    for (const int midi : tasten)
    {
        const auto it = std::find_if(salamander.begin(), salamander.end(), [midi](const TastenWerte& w) { return w.midi == midi; });
        REQUIRE(it != salamander.end());
        bericht << midi << " | Salamander | | " << it->knickS.wert << " (" << it->zweiStufenAnteil.wert << ") | " << it->t60Frueh.wert << " / "
                << it->t60Spaet.wert << " | " << it->tiefeDb.wert << " (" << it->tiefeDb.anzahl << ") | " << it->schwebungCent.wert << '\n';
    }
    for (const double u : {0.0, 0.2, 0.4, 0.6})
        for (const double gammaGrad : {0.0, 10.0, 20.0})
        {
            lumi::loop::HammerSaite saite = lumi::loop::hammerSaiteVorgabe();
            const auto ueberall = [](double wert) {
                return lumi::loop::Tastenkurve{{{lumi::loop::kTasteMin, wert}, {lumi::loop::kTasteMax, wert}}, {}};
            };
            saite.polarisationWinkelGrad = ueberall(gammaGrad);
            saite.polarisationCent = ueberall(gammaGrad > 0.0 ? 2.0 : 0.0);
            saite.stossLeise = ueberall(u);
            saite.stossLaut = ueberall(u);
            const std::vector<TastenWerte> m =
                modelltest::verdichteAlle(tasten, modelltest::messeTasten(modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite), tasten));
            lumi::loop::tonspeicher().leere();
            for (const TastenWerte& w : m)
                bericht << w.midi << " | " << u << " | " << gammaGrad << " | " << w.knickS.wert << " (" << w.zweiStufenAnteil.wert << ") | "
                        << w.t60Frueh.wert << " / " << w.t60Spaet.wert << " | " << w.tiefeDb.wert << " (" << w.tiefeDb.anzahl << ") | "
                        << w.schwebungCent.wert << '\n';
        }
    std::ofstream(ziel / "raster.txt", std::ios::binary) << bericht.str();
    MESSAGE(bericht.str());
}

TEST_CASE("Loop Salamander 31d-7 B die zweite Stufe im Diskant (LUMIVIZ_31D7_DISKANT)")
{
    // 7.4 (US7 a): an 75, 81, 84 fehlt dem Modell die zweite Stufe (Befund S154). Je Taste von der Vorgabe aus je ein
    // Schritt mehr hin zu den Werten von Salamander — Stoss, Late decay, Decay, staerkerer Stoss, mehr Verstimmung —
    // gegen Salamander v12 an Stufe −8; Bilder bis −120 dB und ein Bericht je Teilton
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D7_DISKANT");
    const QString salamanderOrdner = qEnvironmentVariable("LUMIVIZ_SALAMANDER");
    if (variable.isEmpty() || salamanderOrdner.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D7_DISKANT=<Ordner> mit LUMIVIZ_SALAMANDER=<Ordner> untersucht die zweite Stufe an 75, 81, 84");
        return;
    }
    std::unique_ptr<QGuiApplication> anwendung;
    int argc = 1;
    char arg0[] = "UnitTests";
    char* argv[] = {arg0, nullptr};
    if (QCoreApplication::instance() == nullptr) anwendung = std::make_unique<QGuiApplication>(argc, argv);

    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::filesystem::path ordner = salamanderOrdner.toStdU16String();
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE(gelesen.has_value());
    const std::vector<TastenWerte> salamander = tastenWerteAus(*gelesen);

    struct Fall
    {
        QString name;
        QColor farbe;
        double stoss = 0.0;
        bool spaetWieSalamander = false;
        bool fruehWieSalamander = false;
        double verstimmungFaktor = 1.0;
        bool verstimmungWieSchwebung = false;  ///< die Verstimmung = die Schwebung von Salamander in Cent (RB1-1, S154)
    };
    const std::vector<Fall> faelle{
        {QStringLiteral("Vorgabe"), QColor(150, 150, 150)},
        {QStringLiteral("u = 0,25, Decay, Late decay wie Salamander, Verstimmung = Schwebung Salamander"), QColor(0, 130, 0), 0.25,
         true, true, 1.0, true},
        {QStringLiteral("u = 0,25"), QColor(30, 90, 220), 0.25},
        {QStringLiteral("u = 0,25, Late decay wie Salamander"), QColor(0, 160, 160), 0.25, true},
        {QStringLiteral("u = 0,25, Decay und Late decay wie Salamander"), QColor(220, 50, 40), 0.25, true, true},
        {QStringLiteral("u = 0,6, Decay und Late decay wie Salamander"), QColor(230, 140, 20), 0.6, true, true},
        {QStringLiteral("u = 0,25, Decay, Late decay wie Salamander, Verstimmung × 3"), QColor(140, 60, 170), 0.25, true, true, 3.0},
    };
    constexpr int kEbene = 12;
    std::ostringstream bericht;
    bericht << std::fixed << std::setprecision(2)
            << "Zweite Stufe im Diskant, linker Kanal, v12 / Stufe -8: je Teilton T60 frueh / spaet (zwei Stufen?), Knick, Einbruch\n";
    const auto zeile = [&bericht](const QString& name, const Ergebnis& e) {
        bericht << "  " << name.toStdString() << ':';
        for (int n = 1; n <= 3; ++n)
        {
            const Teilton* t = e.teilton(n);
            bericht << " | T" << n << ' ';
            if (t == nullptr || !t->abklingen.erkannt)
            {
                bericht << '-';
                continue;
            }
            bericht << t->abklingen.t60Frueh << " / " << t->abklingen.t60Spaet << (t->abklingen.zweiStufen ? " (2)" : " (1)") << " Knick "
                    << t->abklingen.knickS << " Einbruch " << (t->schwebung.erkannt ? t->schwebung.einbruchDb : 0.0);
        }
        bericht << '\n';
    };
    for (const int midi : {75, 81, 84})
    {
        CAPTURE(midi);
        const std::vector<SalamanderTaste> alle = salamanderTasten();
        const SalamanderTaste& st = *std::find_if(alle.begin(), alle.end(), [midi](const SalamanderTaste& t) { return t.midi == midi; });
        const std::optional<lumi::audio::WavInhalt> wav = lumi::audio::liesWavDatei(salamanderDatei(ordner, st, kEbene));
        REQUIRE(wav.has_value());
        const auto sw = std::find_if(salamander.begin(), salamander.end(), [midi](const TastenWerte& w) { return w.midi == midi; });
        REQUIRE(sw != salamander.end());
        bericht << "MIDI " << midi << " (Salamander v6-v16: T60 frueh " << sw->t60Frueh.wert << ", spaet " << sw->t60Spaet.wert << ", Knick "
                << sw->knickS.wert << ")\n";
        std::vector<BildKurve> kurven{{QStringLiteral("Salamander v%1").arg(kEbene), Qt::black, linksGemessen(wav->stereo, wav->rate, midi)}};
        zeile(kurven.front().name, kurven.front().messung);
        for (const Fall& f : faelle)
        {
            lumi::loop::HammerSaite saite = lumi::loop::hammerSaiteVorgabe();
            const auto ueberall = [](double wert) {
                return lumi::loop::Tastenkurve{{{lumi::loop::kTasteMin, wert}, {lumi::loop::kTasteMax, wert}}, {}};
            };
            saite.stossLeise = ueberall(f.stoss);
            saite.stossLaut = ueberall(f.stoss);
            if (f.spaetWieSalamander) saite.abklingSpaetS.abweichungen = {{midi, sw->t60Spaet.wert}};
            if (f.fruehWieSalamander) saite.abklingS.abweichungen = {{midi, sw->t60Frueh.wert}};
            const double cent = lumi::loop::wertAn(saite.verstimmungCent, midi, lumi::loop::KurvenSkala::Linear);
            saite.verstimmungCent.abweichungen = {{midi, f.verstimmungWieSchwebung ? sw->schwebungCent.wert : cent * f.verstimmungFaktor}};
            bericht << "    (" << f.name.toStdString() << ": Verstimmung " << saite.verstimmungCent.abweichungen.front().wert << " Cent)\n";
            const lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            const lumi::loop::Klang klang = modelltest::ton(instrument, midi, modelltest::stufeVonEbene(kEbene));
            kurven.push_back({f.name, f.farbe, linksGemessen(klang.daten, modelltest::kRate, midi)});
            CHECK(kurven.back().messung.erkannt);
            zeile(f.name, kurven.back().messung);
        }
        const QImage bild = huellkurvenBild(midi, kurven, 7.0, -120.0);
        CHECK(bild.save(QString::fromStdU16String((ziel / ("diskant_" + std::to_string(midi) + ".png")).u16string())));
    }
    std::ofstream(ziel / "diskant.txt", std::ios::binary) << bericht.str();
    MESSAGE(bericht.str());
}

TEST_CASE("Loop Salamander 31d-7 B Blindanteil am Steg aus dem Koerper (LUMIVIZ_31D7_BLIND)")
{
    // RB1 a: der Befund am Koerper vor den Umfangsfragen — je Taste mit Stegpunkt und je Teilton 1–6 der Vorgabe die
    // Mobilitaet Y am Stegpunkt (MAESSTRO Gl. 10, alle Moden des mitgelieferten Steinway D) und das Verhaeltnis
    // Im Y / Re Y; dazu der Median ueber ±2 Tasten. Bild und Tabelle; neben den Tasten von Salamander dessen Anteil
    // zweistufiger Messungen
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D7_BLIND");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D7_BLIND=<Ordner> rechnet Im Y / Re Y am Stegpunkt je Taste und Teilton");
        return;
    }
    std::unique_ptr<QGuiApplication> anwendung;
    int argc = 1;
    char arg0[] = "UnitTests";
    char* argv[] = {arg0, nullptr};
    if (QCoreApplication::instance() == nullptr) anwendung = std::make_unique<QGuiApplication>(argc, argv);

    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    const std::shared_ptr<const lumi::loop::BodenImKlang> boden = lumi::loop::bodenVon(QStringLiteral("lib:steinway_d"));
    REQUIRE_MESSAGE(boden, "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE(gelesen.has_value());
    const std::vector<TastenWerte> salamander = tastenWerteAus(*gelesen);
    const lumi::loop::HammerSaite& vorgabe = lumi::loop::hammerSaiteVorgabe();

    constexpr int kTeiltoene = 6;
    std::map<int, std::array<std::complex<double>, kTeiltoene>> y;      // je Taste
    std::map<int, std::array<std::complex<double>, kTeiltoene>> terz;   // Mittel von Y ueber das Terzband um den Teilton
    double hoechsteMode = 0.0;
    for (const lumi::koerper::ModeImKoerper& m : boden->moden) hoechsteMode = std::max(hoechsteMode, m.frequenzHz);
    for (int midi = lumi::loop::kTasteMin; midi <= lumi::loop::kTasteMax; ++midi)
    {
        const int stegpunkt = lumi::loop::stegpunktVon(boden->koerper, midi);
        if (stegpunkt < 0) continue;
        const double b = lumi::loop::wertAn(vorgabe.inharmonizitaet, midi, lumi::loop::KurvenSkala::Logarithmisch);
        const double f0 = tastenFrequenz(midi) / std::sqrt(1.0 + b);
        std::array<std::complex<double>, kTeiltoene> werte{};
        std::array<std::complex<double>, kTeiltoene> band{};
        for (int k = 1; k <= kTeiltoene; ++k)
        {
            const double fk = k * f0 * std::sqrt(1.0 + b * k * k);
            werte[static_cast<std::size_t>(k - 1)] = lumi::koerper::mobilitaet(boden->moden, static_cast<std::size_t>(stegpunkt), fk);
            // komplexes Mittel ueber das Terzband (Grenzen fk · 10^(±1/20)), 200 Punkte gleich im Logarithmus
            std::complex<double> summe{0.0, 0.0};
            for (int i = 0; i < 200; ++i)
                summe += lumi::koerper::mobilitaet(boden->moden, static_cast<std::size_t>(stegpunkt),
                                                   fk * std::pow(10.0, (-0.05 + 0.1 * (i + 0.5) / 200.0)));
            band[static_cast<std::size_t>(k - 1)] = summe / 200.0;
        }
        y[midi] = werte;
        terz[midi] = band;
    }
    REQUIRE(!y.empty());
    const auto verhaeltnis = [](std::complex<double> v) { return v.imag() / v.real(); };
    const auto geglaettet = [&](int midi, std::size_t k) {
        std::vector<double> nah;
        for (int m = midi - 2; m <= midi + 2; ++m)
            if (const auto it = y.find(m); it != y.end()) nah.push_back(verhaeltnis(it->second[k]));
        std::nth_element(nah.begin(), nah.begin() + static_cast<std::ptrdiff_t>(nah.size() / 2), nah.end());
        return nah[nah.size() / 2];
    };

    std::ostringstream bericht;
    bericht << std::fixed << std::setprecision(2)
            << "Im Y / Re Y am Stegpunkt (steinway_d), Teilton 1-6 der Vorgabe; [Median ueber +-2 Tasten]; {aus dem Terzband};"
               " Re Y in 1e-4 s/kg; Salamander: Anteil zweistufig; hoechste Mode des Koerpers "
            << hoechsteMode << " Hz\n";
    int ueberEins = 0;
    int zahl = 0;
    for (const auto& [midi, werte] : y)
    {
        bericht << midi << " |";
        for (std::size_t k = 0; k < werte.size(); ++k)
        {
            const double v = verhaeltnis(werte[k]);
            bericht << ' ' << v << " [" << geglaettet(midi, k) << "] {" << verhaeltnis(terz.at(midi)[k]) << '}';
            ueberEins += std::abs(v) > 1.0 ? 1 : 0;
            ++zahl;
        }
        bericht << " | Re Y T1 " << 1e4 * werte[0].real();
        if (const auto s = std::find_if(salamander.begin(), salamander.end(), [m = midi](const TastenWerte& w) { return w.midi == m; });
            s != salamander.end())
            bericht << " | Salamander " << s->zweiStufenAnteil.wert;
        bericht << '\n';
    }
    bericht << "|Im/Re| > 1 bei " << ueberEins << " von " << zahl << " Teiltoenen\n";
    std::ofstream(ziel / "blindanteil.txt", std::ios::binary) << bericht.str();
    MESSAGE(bericht.str());

    // Bild: Im/Re (auf ±5 begrenzt) ueber der Taste, Teilton 1–3 als Punkte, der Median als Linie
    constexpr int kBreite = 1400;
    constexpr int kHoehe = 600;
    QImage bild(kBreite, kHoehe, QImage::Format_ARGB32);
    bild.fill(Qt::white);
    QPainter p(&bild);
    p.setRenderHint(QPainter::Antialiasing);
    const auto x = [](double midi) { return 60.0 + (kBreite - 280.0) * (midi - 21.0) / 87.0; };
    const auto yy = [](double v) { return 30.0 + (kHoehe - 70.0) * (5.0 - std::clamp(v, -5.0, 5.0)) / 10.0; };
    p.setPen(QColor(225, 225, 225));
    for (int v = -5; v <= 5; ++v) p.drawLine(QPointF(x(21), yy(v)), QPointF(x(108), yy(v)));
    for (int m = 24; m <= 108; m += 12) p.drawLine(QPointF(x(m), yy(5)), QPointF(x(m), yy(-5)));
    p.setPen(Qt::black);
    for (int v = -5; v <= 5; v += 1) p.drawText(QPointF(20, yy(v) + 4), QString::number(v));
    for (int m = 24; m <= 108; m += 12) p.drawText(QPointF(x(m) - 8, kHoehe - 20), QString::number(m));
    p.drawText(QPointF(60, 18), QStringLiteral("Im Y / Re Y am Stegpunkt (steinway_d) je Taste (MIDI), auf ±5 begrenzt — Punkte: am Teilton, Linie: aus dem komplexen Mittel ueber das Terzband"));
    const std::array<QColor, 3> farben{QColor(30, 90, 220), QColor(220, 50, 40), QColor(0, 150, 60)};
    for (std::size_t k = 0; k < 3; ++k)
    {
        QPainterPath linie;
        bool erster = true;
        for (const auto& [midi, werte] : y)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(farben[k]);
            p.drawEllipse(QPointF(x(midi), yy(verhaeltnis(werte[k]))), 3.0, 3.0);
            const QPointF punkt(x(midi), yy(verhaeltnis(terz.at(midi)[k])));
            if (erster)
                linie.moveTo(punkt);
            else
                linie.lineTo(punkt);
            erster = false;
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(farben[k], 1.5));
        p.drawPath(linie);
        p.drawText(QPointF(kBreite - 200, 60 + 20 * static_cast<double>(k)), QStringLiteral("Teilton %1").arg(k + 1));
    }
    CHECK(bild.save(QString::fromStdU16String((ziel / "blindanteil.png").u16string())));
}

TEST_CASE("Loop Salamander 31d-7 B Moden gegen Ton (LUMIVIZ_31D7_MODEN)")
{
    // SZ3 a, die Probe vor den Umfangsfragen: misst die Teiltonmessung an der Summe der Moden (`hammerTeiltonModen`, je
    // Teilton Σ Im(g·e^(λt)), auf 1/k gewichtet) dasselbe wie am gerechneten Ton (Hammerkontakt, Laengskraft, Boden,
    // linker Kanal, Stufe −8)? An sechs Tasten, mit der Vorgabe und mit Werten nahe Salamander (u = 0,25; Decay, Late
    // decay und Verstimmung wie Salamander)
    const QString variable = qEnvironmentVariable("LUMIVIZ_31D7_MODEN");
    if (variable.isEmpty())
    {
        MESSAGE("LUMIVIZ_31D7_MODEN=<Ordner> vergleicht die Messung an der Summe der Moden mit der am gerechneten Ton");
        return;
    }
    const std::filesystem::path ziel = variable.toStdU16String();
    std::filesystem::create_directories(ziel);
    REQUIRE_MESSAGE(lumi::loop::bodenVon(QStringLiteral("lib:steinway_d")), "steinway_d nicht gefunden — aus dem Projekt-Root starten");
    const std::optional<SalamanderWerte> gelesen = liesWerte(kWerteDatei);
    REQUIRE(gelesen.has_value());
    const std::vector<TastenWerte> salamander = tastenWerteAus(*gelesen);
    constexpr int kStufe = -8;

    const auto zeile = [](std::ostringstream& b, const char* name, const Ergebnis& e) {
        b << "  " << name << ':';
        for (int n = 1; n <= 3; ++n)
        {
            const Teilton* t = e.teilton(n);
            b << " | T" << n << ' ';
            if (t == nullptr || !t->abklingen.erkannt)
            {
                b << '-';
                continue;
            }
            b << t->abklingen.t60Frueh << " / " << t->abklingen.t60Spaet << (t->abklingen.zweiStufen ? " (2)" : " (1)") << " Knick "
              << t->abklingen.knickS << " Schwebung " << (t->schwebung.erkannt ? t->schwebung.frequenz : 0.0) << " Hz Einbruch "
              << (t->schwebung.erkannt ? t->schwebung.einbruchDb : 0.0);
        }
        b << '\n';
    };

    std::ostringstream bericht;
    bericht << std::fixed << std::setprecision(2)
            << "Moden gegen Ton, Stufe -8: je Teilton T60 frueh / spaet (Stufen), Knick s, Schwebung Hz, Einbruch dB\n";
    for (const int midi : {33, 48, 60, 66, 81, 84})
    {
        CAPTURE(midi);
        const auto sw = std::find_if(salamander.begin(), salamander.end(), [midi](const TastenWerte& w) { return w.midi == midi; });
        REQUIRE(sw != salamander.end());
        for (const bool nahe : {false, true})
        {
            lumi::loop::HammerSaite saite = lumi::loop::hammerSaiteVorgabe();
            if (nahe)
            {
                const auto ueberall = [](double wert) {
                    return lumi::loop::Tastenkurve{{{lumi::loop::kTasteMin, wert}, {lumi::loop::kTasteMax, wert}}, {}};
                };
                saite.stossLeise = ueberall(0.25);
                saite.stossLaut = ueberall(0.25);
                if (sw->t60Frueh.anzahl > 0) saite.abklingS.abweichungen = {{midi, sw->t60Frueh.wert}};
                if (sw->t60Spaet.anzahl > 0) saite.abklingSpaetS.abweichungen = {{midi, sw->t60Spaet.wert}};
                if (sw->schwebungCent.anzahl > 0) saite.verstimmungCent.abweichungen = {{midi, sw->schwebungCent.wert}};
            }
            const double dauerS = midi <= 72 ? 12.0 : 8.0;
            const std::vector<double> summe = modelltest::modenTon(saite, midi, kStufe, dauerS, 12);
            const Ergebnis ausModen = miss(summe, modelltest::kRate, tastenFrequenz(midi), lumi::loop::fft());
            const lumi::loop::Instrument instrument = modelltest::fluegel(QStringLiteral("lib:steinway_d"), &saite);
            const lumi::loop::Klang klang = modelltest::ton(instrument, midi, kStufe);
            const Ergebnis ausTon = linksGemessen(klang.daten, modelltest::kRate, midi);
            CHECK(ausModen.erkannt);
            CHECK(ausTon.erkannt);
            bericht << "MIDI " << midi << (nahe ? " nahe Salamander" : " Vorgabe") << '\n';
            zeile(bericht, "Moden", ausModen);
            zeile(bericht, "Ton  ", ausTon);
        }
    }
    std::ofstream(ziel / "moden_gegen_ton.txt", std::ios::binary) << bericht.str();
    MESSAGE(bericht.str());
}
