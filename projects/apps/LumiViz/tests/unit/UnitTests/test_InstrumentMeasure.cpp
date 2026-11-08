/**
 ****************************************************************************************
 * @file   test_InstrumentMeasure.cpp
 * @brief  InstrumentMeasure, Stufe 34e-3: die Bausteine B1–B8 aus IC, die Messung an der Struktur des Bezugs, das
 *         Zielformat und der Zugang zu lokalen Aufnahmen (docs/audio/Instrumente_IC_Konzept.md 34e, IK2-2 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine. Die Faelle sind aus IC `rechnung/messung/test_messung.cpp` (bb7e6d6, IC
 * Session 5) uebernommen, mit denselben Signalen und Grenzen (Journal J21, J22); geaendert sind nur die Namen der
 * Schnittstellen (`lumi::kern::fft()`, `lumi::kern::kLn1000`) und die Pfade. Nicht uebernommen: der Fall „B6 Modalfit
 * am Modell“ — er rechnet einen Xylophonton mit dem Kern der Klasse 4, der erst mit 34g kommt.
 * Das Zielformat liest die Dateien unter `werte/` vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentCore/Fft.hpp"
#include "InstrumentCore/Modal.hpp"
#include "InstrumentCore/Pruefsumme.hpp"
#include "InstrumentMeasure/Bausteine.hpp"
#include "InstrumentMeasure/Fluegelmessung.hpp"
#include "InstrumentMeasure/Samples.hpp"
#include "InstrumentMeasure/Strukturmessung.hpp"
#include "InstrumentMeasure/Teiltonmessung.hpp"
#include "InstrumentModels/stab/Stab.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <numbers>
#include <string>
#include <vector>

using namespace lumi::messung;
using lumi::kern::fft;
using lumi::kern::kLn1000;
using lumi::kern::sigmaAusT60;

namespace
{

constexpr double kPi = std::numbers::pi;

const std::filesystem::path kWerte = "projects/apps/LumiViz/tests/unit/UnitTests/werte";

double dB(double x)
{
    return 20.0 * std::log10(std::max(x, 1e-300));
}

/// Ein fester Zufall (xorshift64*, Startwert fest wie in IC) mit Gauss nach Box–Muller.
struct Zufall
{
    std::uint64_t s = 0x9E3779B97F4A7C15ull;
    double gleich()
    {
        s ^= s >> 12;
        s ^= s << 25;
        s ^= s >> 27;
        return static_cast<double>((s * 0x2545F4914F6CDD1Dull) >> 11) / 9007199254740992.0;
    }
    double gauss()
    {
        const double u = std::max(1e-300, gleich());
        return std::sqrt(-2.0 * std::log(u)) * std::cos(2.0 * kPi * gleich());
    }
};

}  // namespace

TEST_CASE("InstrumentMeasure 34e Teiltonmessung an einem synthetischen Ton (Fall aus IC)")
{
    // zehn Teiltoene f_k = k·f0·√(1 + B·k²), T60 je Teilton fallend, Teilton 2 mit Schwebung 1,5 Hz (zwei Saiten)
    const int rate = 48000;
    const double f0 = 110.0, b = 4e-4;
    std::vector<double> ton(static_cast<std::size_t>(8 * rate), 0.0);
    for (int k = 1; k <= 10; ++k)
    {
        const double fk = k * f0 * std::sqrt(1.0 + b * k * k);
        const double t60 = 6.0 / (1.0 + 0.3 * (k - 1));
        const double sigma = kLn1000 / t60;
        for (std::size_t i = 0; i < ton.size(); ++i)
        {
            const double t = static_cast<double>(i) / rate;
            double a = std::exp(-sigma * t) / k;
            if (k == 2) a *= std::abs(std::cos(kPi * 1.5 * t)) * 0.6 + 0.4;  // Schwebung 1,5 Hz
            ton[i] += a * std::sin(2.0 * kPi * fk * t);
        }
    }
    // nur zehn Teiltoene da: ohne Rauschen findet die Suche darueber Lecks des Fensters — die Grenze sagt es ihr
    Einstellungen einst;
    einst.hoechsterTeilton = 10;
    const Ergebnis e = miss(ton, rate, f0 * std::sqrt(1.0 + b), fft(), einst);
    REQUIRE(e.erkannt);
    CHECK(e.b == doctest::Approx(b).epsilon(0.03));
    CHECK(e.f0 == doctest::Approx(f0).epsilon(1e-5));
    const Teilton* t1 = e.teilton(1);
    REQUIRE(t1 != nullptr);
    REQUIRE(t1->abklingen.erkannt);
    CHECK(t1->abklingen.t60Spaet == doctest::Approx(6.0).epsilon(0.03));
    const Teilton* t4 = e.teilton(4);
    REQUIRE(t4 != nullptr);
    CHECK(t4->abklingen.t60Spaet == doctest::Approx(6.0 / 1.9).epsilon(0.03));
    const Teilton* t2 = e.teilton(2);
    REQUIRE(t2 != nullptr);
    REQUIRE(t2->schwebung.erkannt);
    // die Huelle |cos(π·1,5·t)| schwebt mit 1,5 Hz
    CHECK(t2->schwebung.frequenz == doctest::Approx(1.5).epsilon(0.02));
}

TEST_CASE("InstrumentMeasure 34e B6 Modalfit: drei Moden ohne Reihe mit Rauschen −60 dB")
{
    const int rate = 48000;
    const double f[3] = {220.0, 880.0, 2200.0};
    const double t60[3] = {1.2, 0.4, 0.15};
    const double a[3] = {1.0, 0.3, 0.1};
    std::vector<double> x(static_cast<std::size_t>(rate), 0.0);
    Zufall z;
    for (std::size_t i = 0; i < x.size(); ++i)
    {
        const double t = static_cast<double>(i) / rate;
        for (int k = 0; k < 3; ++k) x[i] += a[k] * std::exp(-sigmaAusT60(t60[k]) * t) * std::cos(2.0 * kPi * f[k] * t + 0.3 * k);
        x[i] += 1e-3 * z.gauss();
    }
    // das Signal auf 8 kHz unterabgetastet (die Moden liegen darunter) — ESPRIT sieht so mehr Perioden je Zeile
    std::vector<double> d;
    for (std::size_t i = 0; i < x.size(); i += 6) d.push_back(x[i]);
    const std::vector<ModalfitMode> m = modalfit(d, rate / 6, 0, 4000, 3, 120);
    REQUIRE(m.size() == 3);
    for (int k = 0; k < 3; ++k)
    {
        CAPTURE(k);
        CHECK(m[static_cast<std::size_t>(k)].hz == doctest::Approx(f[k]).epsilon(1e-4));
        CHECK(kLn1000 / m[static_cast<std::size_t>(k)].sigma == doctest::Approx(t60[k]).epsilon(0.03));
        CHECK(std::abs(dB(m[static_cast<std::size_t>(k)].amplitude / a[k])) < 0.5);
    }
}

TEST_CASE("InstrumentMeasure 34g B6 Modalfit am Modell: die Moden eines gerechneten Xylophontons wiederfinden (1 : 3 : 9)")
{
    // aus IC `rechnung/messung/test_messung.cpp:119-149` (`bb7e6d6`, J41) — in 34e zurueckgestellt (er braucht den Stab)
    namespace stab = lumi::kern::stab;
    const stab::StabSpiel s = stab::baueVorgabeXylophon();
    stab::StabAnschlag an;
    an.note = 72;
    an.hz = 523.25;
    an.stufe = -12;
    stab::StabSpiel ohneRohr = s;
    ohneRohr.partner = stab::PartnerArt::Keiner;
    std::vector<float> ton;
    stab::rechneStabTon(ohneRohr, an, ton, 48000);
    std::vector<double> x(ton.begin() + 480, ton.end());
    const stab::StabModen soll = stab::stabModen(ohneRohr, an);
    const std::vector<ModalfitMode> m = modalfit(x, 48000, 0, 12000, 8, 160);
    // die erste und dritte Mode des Modells mit ihren T60 (die zweite bekommt in der Mitte nichts)
    for (int k = 0; k < 3; k += 2)
    {
        double beste = 1e9;
        const ModalfitMode* treffer = nullptr;
        for (const ModalfitMode& mm : m)
            if (std::abs(mm.hz - soll.stab[static_cast<std::size_t>(k)].hz) < beste)
            {
                beste = std::abs(mm.hz - soll.stab[static_cast<std::size_t>(k)].hz);
                treffer = &mm;
            }
        REQUIRE(treffer != nullptr);
        CAPTURE(k);
        CHECK(treffer->hz == doctest::Approx(soll.stab[static_cast<std::size_t>(k)].hz).epsilon(1e-3));
        CHECK(kLn1000 / treffer->sigma == doctest::Approx(soll.stab[static_cast<std::size_t>(k)].t60S).epsilon(0.03));
    }
}

TEST_CASE("InstrumentMeasure 34e B2 Tonhoehe (YIN) und B3 Vibrato: 5,5 Hz, 30 Cent, AM 2 dB gleichphasig")
{
    const int rate = 48000;
    const double f0 = 330.0, rateV = 5.5, tiefe = 30.0, amDb = 2.0;
    std::vector<double> x(static_cast<std::size_t>(3 * rate));
    double phase = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i)
    {
        const double t = static_cast<double>(i) / rate;
        const double mod = std::sin(2.0 * kPi * rateV * t);
        const double f = f0 * std::exp2(tiefe * mod / 1200.0);
        phase += 2.0 * kPi * f / rate;
        const double a = std::pow(10.0, amDb * mod / 20.0);
        x[i] = a * (std::sin(phase) + 0.5 * std::sin(2.0 * phase) + 0.25 * std::sin(3.0 * phase));
    }
    const Tonhoehe t = tonhoehe(x, rate, 100.0, 1000.0);
    REQUIRE(t.hz.size() > 100);
    double mittel = 0.0;
    for (const double h : t.hz) mittel += std::log2(h);
    mittel /= static_cast<double>(t.hz.size());
    CHECK(std::exp2(mittel) == doctest::Approx(f0).epsilon(1e-3));
    // die Huelle je Rahmen der Tonhoehe
    std::vector<double> huelle;
    const auto n = static_cast<std::size_t>(0.04 * rate);
    for (std::size_t r = 0; r < t.hz.size(); ++r)
    {
        const std::size_t ab = r * static_cast<std::size_t>(0.005 * rate);
        double s = 0.0;
        for (std::size_t i = 0; i < n; ++i) s += x[ab + i] * x[ab + i];
        huelle.push_back(10.0 * std::log10(s / n));
    }
    const Vibrato v = vibrato(t.hz, huelle, t.schrittS, 3.0, 10.0, 0.04);
    REQUIRE(v.erkannt);
    CHECK(v.rateHz == doctest::Approx(rateV).epsilon(0.02));
    CHECK(v.tiefeCent == doctest::Approx(tiefe).epsilon(0.05));
    CHECK(v.amTiefeDb == doctest::Approx(amDb).epsilon(0.1));
    CHECK(std::abs(v.amPhaseGrad) < 20.0);
}

TEST_CASE("InstrumentMeasure 34e B1 Verfolgung, B4 Rest, B8 Gleiten")
{
    // drei Teiltoene + Rauschen −30 dB; eine Mode gleitet um 5 %
    const int rate = 48000;
    Zufall z;
    std::vector<double> x(static_cast<std::size_t>(rate));
    double rauschen = 0.0, periodisch = 0.0;
    for (std::size_t i = 0; i < x.size(); ++i)
    {
        const double t = static_cast<double>(i) / rate;
        const double p = std::sin(2.0 * kPi * 200.0 * t) + 0.5 * std::sin(2.0 * kPi * 400.0 * t + 1.0) + 0.25 * std::sin(2.0 * kPi * 600.0 * t + 2.0);
        const double r = 0.05 * z.gauss();
        x[i] = p + r;
        periodisch += p * p;
        rauschen += r * r;
    }
    const Verfolgung v = verfolgeTeiltoene(x, rate, fft(), 0.046, 0.01, -40.0);
    int gefunden = 0;
    for (const Spur& s : v.spuren)
    {
        double m = 0.0;
        for (const double h : s.hz) m += h;
        m /= static_cast<double>(s.hz.size());
        if (s.hz.size() > 80 && (std::abs(m - 200.0) < 0.5 || std::abs(m - 400.0) < 0.5 || std::abs(m - 600.0) < 0.5)) ++gefunden;
    }
    CHECK(gefunden == 3);
    const Rest r = rest(x, rate, v, fft());
    CHECK(r.abstandDb == doctest::Approx(10.0 * std::log10(periodisch / rauschen)).epsilon(0.03));
    // Gleiten: f(t) = 150·(1 + 0,05·e^(−t/0,1))
    std::vector<double> g(static_cast<std::size_t>(rate / 2));
    double ph = 0.0;
    for (std::size_t i = 0; i < g.size(); ++i)
    {
        const double t = static_cast<double>(i) / rate;
        ph += 2.0 * kPi * 150.0 * (1.0 + 0.05 * std::exp(-t / 0.1)) / rate;
        g[i] = std::exp(-t) * std::sin(ph);
    }
    const Verfolgung vg = verfolgeTeiltoene(g, rate, fft(), 0.03, 0.005, -40.0);
    const std::vector<double> f = gleiten(vg, 0.3);
    REQUIRE(f.size() > 40);
    // der Rahmen bei 0,2 s (Mitte des Fensters bei 0,2 + 0,015)
    const double t = 40 * 0.005 + 0.015;
    CHECK(f[40] == doctest::Approx(150.0 * (1.0 + 0.05 * std::exp(-t / 0.1))).epsilon(2e-3));
}

TEST_CASE("InstrumentMeasure 34e B5 Einschwingen und B7 Baender")
{
    // ein Ton, der in 80 ms einschwingt; Schwerpunkt eines Sinus
    const int rate = 48000;
    std::vector<double> x(static_cast<std::size_t>(rate));
    for (std::size_t i = 0; i < x.size(); ++i)
    {
        const double t = static_cast<double>(i) / rate;
        x[i] = (1.0 - std::exp(-t / 0.03)) * std::sin(2.0 * kPi * 440.0 * t);
    }
    // −3 dB in der Leistung: (1 − e^(−t/τ))² = 0,5 → t = −τ·ln(1 − 1/√2) = 36,8 ms
    CHECK(einschwingen(x, rate) == doctest::Approx(-0.03 * std::log(1.0 - 1.0 / std::sqrt(2.0))).epsilon(0.15));
    const Baender b = baenderUeberZeit(x, rate, fft());
    REQUIRE(!b.schwerpunktHz.empty());
    CHECK(b.schwerpunktHz.back() == doctest::Approx(440.0).epsilon(0.03));
}

TEST_CASE("InstrumentMeasure 34e Messung an der Struktur des Bezugs: stetig im Parameter des Modells (L4)")
{
    // ein Teilton mit wandernder Frequenz: gemessen bei der festen Frequenz des Bezugs aendert sich der Pegel glatt
    const int rate = 48000;
    double vorher = 0.0;
    int spruenge = 0;
    for (int i = 0; i <= 20; ++i)
    {
        const double f = 440.0 * (1.0 + 0.0005 * i);
        std::vector<double> x(static_cast<std::size_t>(rate / 2));
        for (std::size_t j = 0; j < x.size(); ++j) x[j] = std::sin(2.0 * kPi * f * static_cast<double>(j) / rate);
        const double db = amplitudenBei(x, rate, 440.0, {0.25}, 0.1).front();
        if (i > 0 && std::abs(db - vorher) > 3.0) ++spruenge;
        vorher = db;
    }
    CHECK(spruenge == 0);
}

TEST_CASE("InstrumentMeasure 34e Zielformat: salamander_ziel.txt und salamander_struktur.txt lesen und wieder schreiben")
{
    const std::optional<Ziele> z = liesZiele(kWerte / "salamander_ziel.txt");
    REQUIRE_MESSAGE(z.has_value(), "werte/salamander_ziel.txt nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    CHECK(!z->empty());
    const auto pfad = std::filesystem::temp_directory_path() / "lumiviz_34e_ziel.txt";
    REQUIRE(schreibeZiele(pfad, *z));
    const std::optional<Ziele> z2 = liesZiele(pfad);
    REQUIRE(z2.has_value());
    CHECK(z2->size() == z->size());
    const std::optional<Strukturen> s = liesStruktur(kWerte / "salamander_struktur.txt");
    REQUIRE(s.has_value());
    CHECK(!s->empty());
    const auto pfad2 = std::filesystem::temp_directory_path() / "lumiviz_34e_struktur.txt";
    REQUIRE(schreibeStruktur(pfad2, *s));
    const std::optional<Strukturen> s2 = liesStruktur(pfad2);
    REQUIRE(s2.has_value());
    CHECK(s2->size() == s->size());
    std::filesystem::remove(pfad);
    std::filesystem::remove(pfad2);
}

TEST_CASE("InstrumentMeasure 34e Samples: ohne Variable klare Meldung; mit Ordner Summen lesen und Dateien pruefen")
{
    std::string meldung;
    CHECK_FALSE(samplesWurzelAus("LUMIVIZ_PROBE_SAMPLES", nullptr, meldung).has_value());
    CHECK(meldung.find("LUMIVIZ_PROBE_SAMPLES") != std::string::npos);
    CHECK_FALSE(samplesWurzelAus("LUMIVIZ_PROBE_SAMPLES", "/gibt/es/nicht", meldung).has_value());
    CHECK(meldung.find("keinen Ordner") != std::string::npos);
    const auto ordner = std::filesystem::temp_directory_path() / "lumiviz_34e_samples";
    std::filesystem::remove_all(ordner);
    std::filesystem::create_directories(ordner / "Klasse 4 - Platte und Stab" / "Probe");
    const auto datei = ordner / "Klasse 4 - Platte und Stab" / "Probe" / "ton.wav";
    {
        // ein beliebiger Inhalt genuegt (in IC eine WAV-Datei aus `schreibeWav`)
        std::ofstream s(datei, std::ios::binary);
        for (int i = 0; i < 4000; ++i) s.put(static_cast<char>(i * 13 + 1));
    }
    const std::string sha = sha256Datei(datei);
    CHECK(sha.size() == 64);
    {
        std::ofstream s(ordner / "SHA256SUMS");
        s << "# Kommentar\n" << sha << "  Klasse 4 - Platte und Stab/Probe/ton.wav\n";
    }
    const auto wurzel = samplesWurzelAus("LUMIVIZ_PROBE_SAMPLES", ordner.string().c_str(), meldung);
    REQUIRE(wurzel.has_value());
    const std::vector<SummenEintrag> e = liesSummen(ordner / "SHA256SUMS");
    REQUIRE(e.size() == 1);
    CHECK(e[0].pfad == "Klasse 4 - Platte und Stab/Probe/ton.wav");
    CHECK(pruefeDatei(*wurzel / e[0].pfad, e[0].sha256));
    CHECK_FALSE(pruefeDatei(*wurzel / e[0].pfad, std::string(64, '0')));
    std::filesystem::remove_all(ordner);
}
