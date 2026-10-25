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
 ****************************************************************************************
 */

#include <doctest.h>

#include "Fluegelmessung.hpp"

#include "audio/WavFormat.hpp"
#include "loop/Fft.hpp"

#include <QString>
#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
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
    std::filesystem::remove_all(ordner);
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
