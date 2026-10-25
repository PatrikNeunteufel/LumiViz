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
 * - `LUMIVIZ_SALAMANDER=<Ordner>` misst alle 480 Toene beider Kanaele und schreibt die Datei neu;
 *   der Ordner ist `…\Salamander\SalamanderGrandPianoV3_48khz24bit` (README dort). Ohne die
 *   Variable meldet der Fall das und prueft nichts.
 * - Teil 2: der Boden im Vergleich (UF6 a) an Zahlen von Hand; `LUMIVIZ_31D3_MODELL=<Ordner>` misst den
 *   gerechneten Steinway D wie im Programm (Stufen −30 … 0 dB an der Stelle der Ebenen, V1 a) und den
 *   Glasboden, vergleicht mit `werte/salamander_c5.txt` und legt `modell_steinway.txt` und
 *   `vergleich.txt` in den Ordner.
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"
#include "ModellMessung.hpp"

#include "audio/WavDatei.hpp"
#include "audio/WavFormat.hpp"
#include "loop/Boden.hpp"
#include "loop/Fft.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonspeicher.hpp"

#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>

using namespace lumi::messung;

namespace
{

const std::filesystem::path kWerteDatei = "projects/apps/LumiViz/tests/unit/UnitTests/werte/salamander_c5.txt";

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
    REQUIRE(zurueck[0].helligkeit.size() == 7);
    CHECK(zurueck[0].helligkeit[1].wert == doctest::Approx(0.6));
    CHECK(zurueck[0].helligkeit[2].anzahl == 0);
    CHECK(zurueck[0].terzR == std::vector<double>{-11.0, -21.5});
    CHECK(zurueck[0].phantome == std::vector<std::pair<int, double>>{{6, -32.5}, {8, -28.0}});
    std::filesystem::remove_all(ordner);
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
                    messungen[i].push_back(messeTon(wav->stereo, wav->rate, tastenFrequenz(taste.midi), fft, phantome));
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
