/**
 ****************************************************************************************
 * @file   test_LoopSalamanderStruktur.cpp
 * @brief  Loop, 7.5.8 (SM4 b): die Messung an der Struktur des Ziels
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S156/S161: ZF2 a, ZF3 a, ZF4 a, ZF7 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung (`Strukturmessung.hpp`, reines C++).
 *
 * - Die Struktur aus einer Ebene an einem Teilton von Hand (zwei Stufen mit Knick, Schwebung) und die Verdichtung
 *   ueber Ebenen (Knick und Schwebung nur, wo mindestens die Haelfte sie erkennt).
 * - Die Messung an fremder Struktur ist stetig: ein Teilton mit Knick bei 2 s, gemessen an Strukturen mit Knick
 *   1,5 … 2,5 s, aendert die spaete Stufe und die Tiefe des Knicks glatt; eine Schwebung von 1 Hz, gemessen bei
 *   0,8 … 1,2 Hz, hat ihren groessten Anteil bei 1 Hz und faellt glatt ab; ohne Knick in der Struktur eine Gerade,
 *   ohne Schwebung kein Anteil.
 * - Die Dateien hin und zurueck (`salamander_struktur.txt`, `salamander_ziel.txt`), in einem Wegwerf-Ordner.
 * - Die mitgelieferten Dateien: 30 Tasten, je Groesse der Bericht (Ziel, Toleranz, wie oft die Kanaele die
 *   Toleranz bestimmen, wie oft die Toleranz den Wert erreicht) — liest vom Arbeitsordner aus.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Strukturmessung.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

using namespace lumi::messung;

namespace
{

const std::filesystem::path kStrukturDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_struktur.txt";
const std::filesystem::path kZielDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_ziel.txt";

/// Ein Teilton von Hand: zwei Stufen (T60 `t1` bis `knickS`, dann `t2`) in dB, darueber eine Schwebung `fHz` mit
/// `tiefeDb` Spitze zu Spitze, `dauerS` lang im Schritt 10 ms; der Boden tief unten, der Bereich der ganze Ton.
Teilton synthetisch(double t1, double t2, double knickS, double tiefeDb, double fHz, double dauerS, double schrittS = 0.01)
{
    Teilton t;
    t.k = 1;
    t.frequenz = 220.0;
    const auto n = static_cast<std::size_t>(std::lround(dauerS / schrittS));
    for (std::size_t i = 0; i < n; ++i)
    {
        const double zeit = static_cast<double>(i) * schrittS;
        double l = zeit < knickS ? -60.0 / t1 * zeit : -60.0 / t1 * knickS - 60.0 / t2 * (zeit - knickS);
        l += 0.5 * tiefeDb * std::sin(2.0 * std::numbers::pi * fHz * zeit);
        t.pegelDb.push_back(l);
        t.bodenDb.push_back(-150.0);
    }
    t.anfangDb = 0.0;
    t.bereichVon = 0;
    t.bereichBis = n;
    return t;
}

TeiltonStruktur struktur(std::size_t von, std::size_t bis, double knickS, double hz)
{
    TeiltonStruktur s;
    s.da = true;
    s.von = von;
    s.bis = bis;
    s.knick = knickS > 0.0;
    s.knickS = knickS;
    s.schwebt = hz > 0.0;
    s.schwebungHz = hz;
    s.ebenen = 1;
    return s;
}

}  // namespace

TEST_CASE("Loop Salamander 7.5.8 Struktur aus Ebenen (ZF3 a)")
{
    // zwei Stufen 2 s / 10 s mit Knick bei 1,5 s, Schwebung 1 Hz mit 6 dB, 8 s lang
    const Teilton t = synthetisch(2.0, 10.0, 1.5, 6.0, 1.0, 8.0);
    const TeiltonStruktur s = strukturAus(t, 220.0);
    REQUIRE(s.da);
    CHECK(s.von == 0);
    CHECK(s.bis == 800);
    CHECK(s.knick);
    CHECK(s.knickS == doctest::Approx(1.5).epsilon(0.25));
    CHECK(s.schwebt);
    CHECK(s.schwebungHz == doctest::Approx(1.0).epsilon(0.03));

    // eine Gerade ohne Schwebung: kein Knick, keine Schwebung
    const TeiltonStruktur g = strukturAus(synthetisch(3.0, 3.0, 0.0, 0.0, 1.0, 8.0), 220.0);
    REQUIRE(g.da);
    CHECK_FALSE(g.knick);
    CHECK_FALSE(g.schwebt);

    // zu kurz oder ohne Bereich: nicht da
    Teilton kurz = synthetisch(2.0, 10.0, 1.5, 6.0, 1.0, 0.2);
    CHECK_FALSE(strukturAus(kurz, 220.0).da);
    kurz.bereichVon = kurz.bereichBis = 0;
    CHECK_FALSE(strukturAus(kurz, 220.0).da);

    // die Verdichtung: Knick und Schwebung, wo mindestens die Haelfte der Ebenen sie erkennt
    std::vector<TeiltonStruktur> ebenen;
    for (int e = 0; e < 5; ++e) ebenen.push_back(struktur(10 + static_cast<std::size_t>(e), 700 + 20 * static_cast<std::size_t>(e), e < 3 ? 1.4 + 0.1 * e : 0.0, e < 2 ? 1.0 + 0.1 * e : 0.0));
    ebenen.push_back(TeiltonStruktur{});  // eine Ebene ohne Bereich zaehlt nicht
    const TeiltonStruktur v = verdichteStruktur(ebenen);
    REQUIRE(v.da);
    CHECK(v.ebenen == 5);
    CHECK(v.von == 12);
    CHECK(v.bis == 740);
    CHECK(v.knick);  // 3 von 5
    CHECK(v.knickS == doctest::Approx(1.5));
    CHECK(v.mitKnick == 3);
    CHECK_FALSE(v.schwebt);  // 2 von 5
    CHECK(v.mitSchwebung == 2);
    CHECK_FALSE(verdichteStruktur({TeiltonStruktur{}, TeiltonStruktur{}}).da);

    // die hoechste gesuchte Schwebung wie `miss`: 8 Hz, im Bass die halbe Fensterrate
    CHECK(schwebungBisHz(440.0) == doctest::Approx(8.0));
    CHECK(schwebungBisHz(27.5) == doctest::Approx(0.5 / std::clamp(8.0 / 27.5, 0.02, 0.5)));
}

TEST_CASE("Loop Salamander 7.5.8 Messung an fremder Struktur ist stetig (ZF3 a)")
{
    // das "Modell": Knick bei 2,0 s, Stufen 2 s / 10 s, Schwebung 1 Hz mit 6 dB, 10 s lang
    const Teilton modell = synthetisch(2.0, 10.0, 2.0, 6.0, 1.0, 10.0);

    // an der eigenen Struktur: die Groessen treffen die Vorgabe
    const StrukturWerte eigen = misstAnStruktur(modell, struktur(0, 1000, 2.0, 1.0));
    REQUIRE(eigen.gemessen);
    CHECK(eigen.vorwaerts.erkannt);
    CHECK(eigen.vorwaerts.knickErkannt);
    // die erste Stufe ueber 0,5 s ab der Spitze: die Vorgabe — die Schwebung von 6 dB bei 1 Hz (3 dB Amplitude gegen
    // 15 dB Abfall in der halben Periode) verschiebt die Spitze nicht und hebt sich ueber die halbe Periode auf
    CHECK(eigen.vorwaerts.t60 == doctest::Approx(2.0).epsilon(0.1));
    const StrukturWerte ohneSchwebung = misstAnStruktur(synthetisch(2.0, 10.0, 2.0, 0.0, 1.0, 10.0), struktur(0, 1000, 2.0, 0.0));
    CHECK(ohneSchwebung.vorwaerts.t60 == doctest::Approx(2.0).epsilon(0.05));
    CHECK(eigen.spaet);
    CHECK(eigen.t60Spaet == doctest::Approx(10.0).epsilon(0.15));
    CHECK(eigen.schwebung);
    CHECK(eigen.schwebungDb == doctest::Approx(6.0).epsilon(0.2));
    CHECK(eigen.einbruchDb == doctest::Approx(6.0).epsilon(0.2));

    // der Knick der Struktur wandert 1,5 … 2,5 s: spaete Stufe und Tiefe des Knicks aendern sich glatt
    std::vector<double> spaet, tiefe;
    for (double knick = 1.5; knick <= 2.51; knick += 0.1)
    {
        const StrukturWerte w = misstAnStruktur(modell, struktur(0, 1000, knick, 1.0));
        REQUIRE(w.spaet);
        REQUIRE(w.vorwaerts.knickErkannt);
        spaet.push_back(w.t60Spaet);
        tiefe.push_back(w.vorwaerts.knickTiefeDb);
    }
    std::ostringstream bericht;
    bericht << std::fixed << std::setprecision(2) << "Knick 1,5 … 2,5 s: spaete Stufe";
    for (const double s : spaet) bericht << ' ' << s;
    bericht << " s; Tiefe des Knicks";
    for (const double d : tiefe) bericht << ' ' << d;
    bericht << " dB";
    MESSAGE(bericht.str());
    for (std::size_t i = 0; i + 1 < spaet.size(); ++i)
    {
        CHECK(std::abs(spaet[i + 1] - spaet[i]) < 1.0);   // kein Sprung: je 0,1 s Knick hoechstens 1 s
        CHECK(std::abs(tiefe[i + 1] - tiefe[i]) < 3.5);   // und hoechstens 3,5 dB (die fruehe Stufe faellt 3 dB je 0,1 s)
        CHECK(tiefe[i + 1] < tiefe[i]);                   // ein spaeterer Knick laesst weniger Energie dahinter
    }
    for (const double s : spaet) CHECK(s == doctest::Approx(10.0).epsilon(0.35));

    // die Frequenz der Struktur wandert 0,8 … 1,2 Hz in Schritten von 0,02 Hz: der Anteil ist bei 1 Hz am groessten und
    // faellt glatt (die Fensterfunktion ueber 10 s hat Nullstellen bei 0,9 und 1,1 Hz — die Groesse bleibt stetig)
    std::vector<double> anteil;
    for (double f = 0.8; f <= 1.2001; f += 0.02)
    {
        const StrukturWerte w = misstAnStruktur(modell, struktur(0, 1000, 2.0, f));
        REQUIRE(w.schwebung);
        anteil.push_back(w.schwebungDb);
    }
    std::ostringstream b2;
    b2 << std::fixed << std::setprecision(2) << "Schwebung 0,8 … 1,2 Hz: Anteil";
    for (const double a : anteil) b2 << ' ' << a;
    b2 << " dB";
    MESSAGE(b2.str());
    REQUIRE(anteil.size() == 21);
    const std::size_t mitte = 10;  // 1,0 Hz
    CHECK(anteil[mitte] == *std::max_element(anteil.begin(), anteil.end()));
    // je 0,02 Hz hoechstens 2 dB (die Hauptkeule der Fensterfunktion steigt von 0,9 bis 1,0 Hz um rund 6 dB)
    for (std::size_t i = 0; i + 1 < anteil.size(); ++i) CHECK(std::abs(anteil[i + 1] - anteil[i]) < 2.0);
    CHECK(anteil.front() < 0.5 * anteil[mitte]);
    CHECK(anteil.back() < 0.5 * anteil[mitte]);

    // ohne Knick in der Struktur: eine Gerade ueber den Bereich, keine Tiefe des Knicks
    const StrukturWerte eine = misstAnStruktur(synthetisch(3.0, 3.0, 0.0, 0.0, 1.0, 8.0), struktur(0, 800, 0.0, 0.0));
    REQUIRE(eine.gemessen);
    CHECK(eine.spaet);
    CHECK(eine.t60Spaet == doctest::Approx(3.0).epsilon(0.1));
    CHECK_FALSE(eine.vorwaerts.knickErkannt);
    CHECK_FALSE(eine.schwebung);

    // ohne Schwebung im Ton, Struktur mit 1 Hz: ein kleiner Anteil
    const StrukturWerte still = misstAnStruktur(synthetisch(2.0, 10.0, 2.0, 0.0, 1.0, 10.0), struktur(0, 1000, 2.0, 1.0));
    REQUIRE(still.schwebung);
    CHECK(still.schwebungDb < 0.5);

    // die Struktur reicht weiter als der Ton: gekuerzt auf den Ton; ohne Struktur oder zu kurz: nicht gemessen
    const StrukturWerte kurz = misstAnStruktur(synthetisch(2.0, 10.0, 2.0, 6.0, 1.0, 5.0), struktur(0, 1000, 2.0, 1.0));
    CHECK(kurz.gemessen);
    CHECK(kurz.spaet);
    CHECK_FALSE(misstAnStruktur(modell, TeiltonStruktur{}).gemessen);
    CHECK_FALSE(misstAnStruktur(modell, struktur(995, 1000, 0.0, 0.0)).gemessen);
}

TEST_CASE("Loop Salamander 7.5.8 Groessen an der Struktur je Ebene und die Dateien")
{
    // drei Ebenen: das Kanalmittel wie der linke Kanal, der rechte mit laengerer spaeter Stufe und tieferer Schwebung
    std::vector<TonHuellen> huellen;
    for (int e = 0; e < 3; ++e)
    {
        TonHuellen h;
        const double t2 = 10.0 + e;
        h.mittel.push_back(synthetisch(2.0, t2, 2.0, 6.0, 1.0, 10.0));
        h.links.push_back(synthetisch(2.0, t2, 2.0, 6.0, 1.0, 10.0));
        h.rechts.push_back(synthetisch(2.0, 1.5 * t2, 2.0, 9.0, 1.0, 10.0));
        huellen.push_back(std::move(h));
    }
    TonStruktur str;
    str[0] = struktur(0, 1000, 2.0, 1.0);
    const std::vector<GroesseJeEbene> groessen = strukturGroessen(huellen, str);
    REQUIRE(groessen.size() == 15);
    const auto finde = [&groessen](const std::string& name) -> const GroesseJeEbene& {
        for (const GroesseJeEbene& g : groessen)
            if (g.name == name) return g;
        static const GroesseJeEbene leer;
        return leer;
    };
    const GroesseJeEbene& spaet = finde("t60_spaet");
    REQUIRE(spaet.ebenen.size() == 3);
    CHECK(spaet.skala == Skala::Faktor);
    REQUIRE(spaet.ebenen[0].mittel.has_value());
    REQUIRE(spaet.ebenen[0].kanaele[1].has_value());
    CHECK(*spaet.ebenen[0].mittel == doctest::Approx(10.0).epsilon(0.15));
    CHECK(*spaet.ebenen[0].kanaele[1] / *spaet.ebenen[0].kanaele[0] == doctest::Approx(1.5).epsilon(0.1));
    const GroesseJeEbene& schweb = finde("schwebung_db_1");
    CHECK(schweb.skala == Skala::Dezibel);
    REQUIRE(schweb.ebenen[1].kanaele[1].has_value());
    CHECK(*schweb.ebenen[1].kanaele[1] - *schweb.ebenen[1].kanaele[0] == doctest::Approx(3.0).epsilon(0.1));
    CHECK_FALSE(finde("vorwaerts_t60_2").ebenen[0].mittel.has_value());  // Teilton 2 ohne Struktur
    // Ziel und Toleranz daraus: die Kanaele bestimmen die Toleranz der spaeten Stufe (Faktor 1,5 gegen 1,1 je Ebene)
    const TastenZiele z = ziele(groessen);
    REQUIRE(z.count("t60_spaet") == 1);
    CHECK(z.at("t60_spaet").ziel.anzahl == 3);
    CHECK(z.at("t60_spaet").toleranz.kanaele == doctest::Approx(std::log(1.5)).epsilon(0.1));
    CHECK(z.at("t60_spaet").toleranz.kanaele > z.at("t60_spaet").toleranz.ebenen);
    CHECK(z.count("vorwaerts_t60_2") == 0);

    // die Dateien hin und zurueck
    const std::filesystem::path ordner =
        std::filesystem::temp_directory_path() / ("lumiviz_struktur_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    REQUIRE(std::filesystem::create_directories(ordner));
    Strukturen strukturen;
    strukturen[60] = str;
    strukturen[60][2] = struktur(12, 640, 0.0, 0.0);
    strukturen[60][2].ebenen = 11;
    strukturen[60][2].mitKnick = 4;
    strukturen[60][2].mitSchwebung = 2;
    strukturen[66][0] = struktur(5, 900, 1.25, 0.0);
    REQUIRE(schreibeStruktur(ordner / "struktur.txt", strukturen));
    const std::optional<Strukturen> gelesen = liesStruktur(ordner / "struktur.txt");
    REQUIRE(gelesen.has_value());
    REQUIRE(gelesen->size() == 2);
    CHECK(gelesen->at(60)[0].knick);
    CHECK(gelesen->at(60)[0].knickS == doctest::Approx(2.0));
    CHECK(gelesen->at(60)[0].schwebungHz == doctest::Approx(1.0));
    CHECK_FALSE(gelesen->at(60)[1].da);
    CHECK(gelesen->at(60)[2].da);
    CHECK_FALSE(gelesen->at(60)[2].knick);
    CHECK_FALSE(gelesen->at(60)[2].schwebt);
    CHECK(gelesen->at(60)[2].bis == 640);
    CHECK(gelesen->at(60)[2].ebenen == 11);
    CHECK(gelesen->at(60)[2].mitKnick == 4);
    CHECK(gelesen->at(66)[0].knickS == doctest::Approx(1.25));
    CHECK_FALSE(gelesen->at(66)[0].schwebt);

    Ziele alle;
    alle[60] = z;
    alle[66]["tiefe_db_2"] = ZielGroesse{Skala::Dezibel, {7.5, 11}, Toleranz{Skala::Dezibel, 1.25, 0.5, 10, 11}};
    alle[66]["leer"] = ZielGroesse{};  // ohne Ziel: nicht geschrieben
    REQUIRE(schreibeZiele(ordner / "ziel.txt", alle));
    const std::optional<Ziele> zg = liesZiele(ordner / "ziel.txt");
    REQUIRE(zg.has_value());
    REQUIRE(zg->size() == 2);
    CHECK(zg->at(60).size() == z.size());
    CHECK(zg->at(60).at("t60_spaet").skala == Skala::Faktor);
    CHECK(zg->at(60).at("t60_spaet").ziel.wert == doctest::Approx(z.at("t60_spaet").ziel.wert));
    CHECK(zg->at(60).at("t60_spaet").toleranz.kanaele == doctest::Approx(z.at("t60_spaet").toleranz.kanaele));
    REQUIRE(zg->at(66).size() == 1);
    CHECK(zg->at(66).at("tiefe_db_2").ziel.wert == doctest::Approx(7.5));
    CHECK(zg->at(66).at("tiefe_db_2").ziel.anzahl == 11);
    CHECK(zg->at(66).at("tiefe_db_2").toleranz.wert() == doctest::Approx(1.25));
    CHECK(zg->at(66).at("tiefe_db_2").toleranz.paareKanaele == 11);
    std::filesystem::remove_all(ordner);
}

TEST_CASE("Loop Salamander 7.5.8 mitgelieferte Struktur, Ziel und Toleranz (ZF1 a, ZF3 a, ZF4 a)")
{
    const std::optional<Strukturen> strukturen = liesStruktur(kStrukturDatei);
    REQUIRE_MESSAGE(strukturen.has_value(), "werte/salamander_struktur.txt nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    const std::optional<Ziele> ziele = liesZiele(kZielDatei);
    REQUIRE_MESSAGE(ziele.has_value(), "werte/salamander_ziel.txt nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    REQUIRE(strukturen->size() == salamanderTasten().size());
    REQUIRE(ziele->size() == salamanderTasten().size());

    // die Struktur: je Taste wenigstens ein Teilton mit Bereich; Knick und Schwebung sind von der Mehrzahl getragen
    int teiltoene = 0, mitKnick = 0, mitSchwebung = 0;
    for (const auto& [midi, ton] : *strukturen)
    {
        CAPTURE(midi);
        int da = 0;
        for (const TeiltonStruktur& s : ton)
        {
            if (!s.da) continue;
            ++da;
            CHECK(s.bis > s.von);
            CHECK(s.ebenen >= 1);
            CHECK(s.ebenen <= kEbenen - kKurvenEbeneAb + 1);
            if (s.knick)
            {
                ++mitKnick;
                CHECK(2 * s.mitKnick >= s.ebenen);
                CHECK(s.knickS > 0.01 * static_cast<double>(s.von));
                CHECK(s.knickS < 0.01 * static_cast<double>(s.bis));
            }
            if (s.schwebt)
            {
                ++mitSchwebung;
                CHECK(2 * s.mitSchwebung >= s.ebenen);
                CHECK(s.schwebungHz > 0.0);
                CHECK(s.schwebungHz <= 8.0);
            }
        }
        CHECK(da >= 1);
        teiltoene += da;
    }
    MESSAGE("Struktur: " << teiltoene << " Teiltoene mit Bereich, " << mitKnick << " mit Knick, " << mitSchwebung << " mit Schwebung");

    // Ziel und Toleranz je Groesse: Skala wie `strukturGroessen`, endlich; der Bericht
    std::vector<std::string> namen;
    for (const auto& [midi, je] : *ziele)
        for (const auto& [name, z] : je)
            if (std::find(namen.begin(), namen.end(), name) == namen.end()) namen.push_back(name);
    std::ostringstream bericht;
    bericht << std::fixed << std::setprecision(3)
            << "Ziel und Toleranz (ZF4 a): Groesse | Skala | Tasten | Median Ziel | Median Toleranz | kleinste | groesste | "
               "Kanaele groesser an | Toleranz >= Wert bzw. Faktor 2 an\n";
    for (const std::string& name : namen)
    {
        std::vector<double> ziel, tol;
        int kanaeleMehr = 0, weit = 0;
        const char* skala = "";
        for (const auto& [midi, je] : *ziele)
        {
            const auto it = je.find(name);
            if (it == je.end()) continue;
            const ZielGroesse& z = it->second;
            CAPTURE(midi);
            CAPTURE(name);
            skala = skalaName(z.skala);
            CHECK(z.skala == z.toleranz.skala);
            CHECK(z.ziel.anzahl >= 1);
            CHECK(z.ziel.anzahl <= kEbenen - kKurvenEbeneAb + 1);
            CHECK(std::isfinite(z.ziel.wert));
            CHECK(std::isfinite(z.toleranz.wert()));
            CHECK(z.toleranz.wert() >= 0.0);
            if (z.skala == Skala::Faktor) CHECK(z.ziel.wert > 0.0);
            ziel.push_back(z.ziel.wert);
            tol.push_back(z.toleranz.wert());
            kanaeleMehr += z.toleranz.kanaele > z.toleranz.ebenen ? 1 : 0;
            const double marke = z.skala == Skala::Faktor ? std::log(2.0) : std::abs(z.ziel.wert);
            weit += marke > 0.0 && z.toleranz.wert() >= marke ? 1 : 0;
        }
        std::sort(tol.begin(), tol.end());
        bericht << name << " | " << skala << " | " << tol.size() << " | " << detail::medianWert(ziel).wert << " | " << tol[tol.size() / 2] << " | "
                << tol.front() << " | " << tol.back() << " | " << kanaeleMehr << " | " << weit << '\n';
    }
    MESSAGE(bericht.str());
    for (const auto& [midi, je] : *ziele)
    {
        CAPTURE(midi);
        // der ganze Ton bis -20 dB und die erste Stufe von Teilton 1 oder 2 tragen an jeder Taste ein Ziel
        CHECK(je.count("huelle_20_s") > 0);
        CHECK((je.count("vorwaerts_t60") > 0 || je.count("vorwaerts_t60_2") > 0));
    }
}
