/**
 ****************************************************************************************
 * @file   test_LoopHammerTon.cpp
 * @brief  Loop, Stufe 31b: ein Ton der Hammer-Saite — gemessen am gerechneten Ton
 *         (docs/audio/Spielweisen_Konzept.md §4.1, Entscheide Patrik S145/S146: HS1 a,
 *         HT1 a bis HT3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Dateien aus dem Repo; gemessen mit `SaitenMessung.hpp`.
 *
 * - Der Chor: Σ g = 1; ohne Verstimmung nur die Gleichlage; die Stufen liegen bei σ_s + Nσ_b
 *   und σ_s.
 * - Die Teiltoene liegen nach B gestreckt, der erste auf der Tonhoehe.
 * - Laut gespielt heller (der nichtlineare Hammer), nicht nur lauter.
 * - Eine Saite klingt mit *Decay* ab; drei verstimmte Saiten zweistufig.
 * - Der Daempfer beim Loslassen; ueber `undampedFrom` klingt der Ton weiter.
 * - Derselbe Aufruf gibt dieselben Werte; der Klang endet bei null, beide Seiten gleich.
 *
 * `LUMIVIZ_31B_WAV=<Ordner>` legt Hoerproben ab (Patrik hoert sie gesammelt spaeter).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/HammerTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include "SaitenMessung.hpp"  // betrag, spitze, cent, db, teilton

#include <QDir>

#include <algorithm>
#include <cmath>

using namespace lumi::loop;
using namespace saitentest;

namespace
{

Instrument fluegel()
{
    Instrument instrument;
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::Modal;
    s.anregung = Anregung::Hammer;
    instrument.tastatur = Tastatur{};
    return instrument;
}

Klang ton(const Instrument& instrument, int hoehe, double dauerMs, int stufe = 0)
{
    TonAngabe angabe;
    angabe.hoehe = hoehe;
    angabe.dauerMs = static_cast<int>(dauerMs);
    angabe.anschlagStufe = stufe;
    return rendereTon(instrument, angabe, kSaitenRate);
}

/// Der Pegel des Grundtons im Fenster ab `vonS` (0,2 s lang), in dB.
double pegel(const Klang& klang, double hz, double vonS)
{
    return db(betrag(klang, hz, vonS, 0.2));
}

}  // namespace

TEST_CASE("Loop Hammer-Ton 31b Chor: gekoppelte Moden")
{
    const double omega = 2.0 * kPi * 261.6;
    const double sigmaSaite = 0.25;
    const double sigmaSteg = 0.17;
    SUBCASE("ohne Verstimmung nur die Gleichlage, mit der ersten Stufe")
    {
        const auto moden = chorModen({omega, omega, omega}, sigmaSaite, sigmaSteg);
        REQUIRE(moden.size() == 1);
        CHECK(moden[0].lambda.real() == doctest::Approx(-(sigmaSaite + 3 * sigmaSteg)));
        CHECK(moden[0].lambda.imag() == doctest::Approx(omega));
        CHECK(std::abs(moden[0].anteil - 1.0) < 1e-12);
    }
    SUBCASE("verstimmt: Σ g = 1, die Abklingraten zwischen den beiden Stufen")
    {
        for (const int n : {2, 3})
        {
            CAPTURE(n);
            std::vector<double> omegas;
            for (int j = 0; j < n; ++j) omegas.push_back(omega * std::exp2((j - (n - 1) / 2.0) * 1.0 / 1200.0));
            const auto moden = chorModen(omegas, sigmaSaite, sigmaSteg);
            REQUIRE(moden.size() == static_cast<std::size_t>(n));
            std::complex<double> summe(0.0, 0.0);
            double spur = 0.0;
            for (const ChorMode& m : moden)
            {
                summe += m.anteil;
                spur += m.lambda.real();
                CHECK(-m.lambda.real() >= sigmaSaite - 1e-9);
                CHECK(-m.lambda.real() <= sigmaSaite + n * sigmaSteg + 1e-9);
            }
            CHECK(std::abs(summe - 1.0) < 1e-9);
            // die Spur von A bleibt: Σ Re λ = −N·σ_s − N·σ_b
            CHECK(spur == doctest::Approx(-n * sigmaSaite - n * sigmaSteg));
        }
    }
    SUBCASE("eine Saite: ein Mode mit dem ganzen Verlust")
    {
        const auto moden = chorModen({omega}, sigmaSaite, sigmaSteg);
        REQUIRE(moden.size() == 1);
        CHECK(moden[0].lambda.real() == doctest::Approx(-(sigmaSaite + sigmaSteg)));
    }
}

TEST_CASE("Loop Hammer-Ton 31b Teiltoene nach B")
{
    // E1 hat eine Saite (Vorgabe Chor) — keine Schwebung im Weg
    const Instrument instrument = fluegel();
    const int taste = 28;
    const double hz = tonHz(taste);
    const Klang klang = ton(instrument, taste, 3000.0);
    REQUIRE(klang.frames() > kSaitenRate);
    const double b = wertAn(instrument.schichten[0].hammerSaite.inharmonizitaet, taste, KurvenSkala::Logarithmisch);
    const double f0 = hz / std::sqrt(1.0 + b);
    // der erste Teilton auf der Tonhoehe
    CHECK(std::abs(cent(spitze(klang, hz, 20.0, 0.2, 1.5), hz)) <= 0.75);
    for (const int k : {5, 10, 15})
    {
        CAPTURE(k);
        const double soll = k * f0 * std::sqrt(1.0 + b * k * k);
        const double gemessen = spitze(klang, k * hz, 60.0, 0.2, 1.5);
        CAPTURE(cent(soll, k * hz));
        CHECK(std::abs(cent(gemessen, soll)) <= 1.0);
    }
}

TEST_CASE("Loop Hammer-Ton 31b laut heller, nicht nur lauter")
{
    const Instrument instrument = fluegel();
    const int taste = 48;  // C3
    const double hz = tonHz(taste);
    const Klang laut = ton(instrument, taste, 1000.0, 0);
    const Klang leise = ton(instrument, taste, 1000.0, -20);
    const double grundLaut = teilton(laut, hz, 1);
    const double grundLeise = teilton(leise, hz, 1);
    CHECK(grundLaut > grundLeise);
    // der achte Teilton waechst schneller als der Grundton
    const double hellLaut = db(teilton(laut, hz, 8) / grundLaut);
    const double hellLeise = db(teilton(leise, hz, 8) / grundLeise);
    MESSAGE("Teilton 8 zum Grundton: ff " << hellLaut << " dB, pp " << hellLeise << " dB; Grundton ff/pp "
                                          << db(grundLaut / grundLeise) << " dB");
    CHECK(hellLaut > hellLeise + 3.0);
}

TEST_CASE("Loop Hammer-Ton 31b Abklingen: eine Saite mit Decay, drei zweistufig")
{
    Instrument instrument = fluegel();
    SUBCASE("eine Saite: der Grundton faellt mit 60 dB je Decay")
    {
        const int taste = 28;
        const double hz = tonHz(taste);
        const double t60 = wertAn(instrument.schichten[0].hammerSaite.abklingS, taste, KurvenSkala::Logarithmisch);
        const Klang klang = ton(instrument, taste, 4000.0);
        const double fall = pegel(klang, hz, 0.5) - pegel(klang, hz, 3.5);
        MESSAGE("E1: " << fall << " dB in 3 s, soll " << 60.0 * 3.0 / t60);
        CHECK(fall == doctest::Approx(60.0 * 3.0 / t60).epsilon(0.15));
    }
    SUBCASE("drei verstimmte Saiten: spaeter langsamer als frueh")
    {
        const int taste = 60;
        const double hz = tonHz(taste);
        HammerSaite& h = instrument.schichten[0].hammerSaite;
        h.verstimmungCent = {{{21, 2.0}, {108, 2.0}}, {}};
        const Klang klang = ton(instrument, taste, 8000.0);
        const double frueh = (pegel(klang, hz, 0.2) - pegel(klang, hz, 1.2)) / 1.0;
        const double spaet = (pegel(klang, hz, 5.0) - pegel(klang, hz, 7.0)) / 2.0;
        MESSAGE("C4: frueh " << frueh << " dB/s, spaet " << spaet << " dB/s");
        CHECK(spaet < 0.7 * frueh);
        // ohne Verstimmung bleibt es eine Stufe
        h.verstimmungCent = {{{21, 0.0}, {108, 0.0}}, {}};
        const Klang rein = ton(instrument, taste, 8000.0);
        const double reinFrueh = pegel(rein, hz, 0.2) - pegel(rein, hz, 1.2);
        const double reinSpaet = (pegel(rein, hz, 5.0) - pegel(rein, hz, 7.0)) / 2.0;
        CHECK(reinSpaet == doctest::Approx(reinFrueh).epsilon(0.1));
    }
}

TEST_CASE("Loop Hammer-Ton 31b Daempfer und undampedFrom")
{
    const Instrument instrument = fluegel();
    SUBCASE("mit Daempfer: nach dem Loslassen schnell still")
    {
        const int taste = 60;
        const Klang klang = ton(instrument, taste, 300.0);
        const double vorher = pegel(klang, tonHz(taste), 0.1);
        const double nachher = pegel(klang, tonHz(taste), 0.7);
        MESSAGE("C4 mit Daempfer: " << vorher - nachher << " dB");
        CHECK(vorher - nachher > 30.0);
    }
    SUBCASE("ohne Daempfer (ab G6): klingt weiter, wie die Saite es will")
    {
        const int taste = 96;
        REQUIRE(taste >= kUngedaempftAbVorgabe);
        const Klang klang = ton(instrument, taste, 300.0);
        const double frei = pegel(klang, tonHz(taste), 0.1) - pegel(klang, tonHz(taste), 0.7);
        // dieselbe Taste mit Daempfer
        Instrument alle = instrument;
        alle.tastatur->ungedaempftAb = kTasteMax + 1;
        const Klang gedaempft = ton(alle, taste, 300.0);
        const double mit = pegel(gedaempft, tonHz(taste), 0.1) - pegel(gedaempft, tonHz(taste), 0.7);
        MESSAGE("C7 ohne Daempfer: " << frei << " dB, mit: " << mit << " dB in 0,6 s");
        CHECK(frei < mit - 30.0);
        // ohne Daempfer faellt sie mit der ersten Stufe (Decay) — nicht schneller
        const double t60 = wertAn(instrument.schichten[0].hammerSaite.abklingS, taste, KurvenSkala::Logarithmisch);
        CHECK(frei < 60.0 * 0.6 / t60 * 1.15);
    }
}

TEST_CASE("Loop Hammer-Ton 31b derselbe Ton, endet bei null")
{
    const Instrument instrument = fluegel();
    for (const int taste : {21, 45, 60, 84, 108})
    {
        CAPTURE(taste);
        const Klang a = ton(instrument, taste, 500.0);
        const Klang b = ton(instrument, taste, 500.0);
        CHECK(a.daten == b.daten);
        REQUIRE(a.frames() > 0);
        float spitzenwert = 0.0f;
        bool gleich = true;
        bool endlich = true;
        for (std::int64_t i = 0; i < a.frames(); ++i)
        {
            const float l = a.daten[static_cast<std::size_t>(2 * i)];
            const float r = a.daten[static_cast<std::size_t>(2 * i + 1)];
            gleich = gleich && l == r;
            endlich = endlich && std::isfinite(l);
            spitzenwert = std::max(spitzenwert, std::abs(l));
        }
        CHECK(gleich);
        CHECK(endlich);
        CHECK(spitzenwert > 0.0f);
        CHECK(a.daten.back() == 0.0f);
        MESSAGE("Taste " << taste << ": Spitze " << spitzenwert << ", " << a.frames() / double(kSaitenRate) << " s");
    }
}

TEST_CASE("Loop Hammer-Ton 31b Hoerproben (LUMIVIZ_31B_WAV)")
{
    const QString ordner = qEnvironmentVariable("LUMIVIZ_31B_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_31B_WAV=<Ordner> legt Hoerproben ab");
        return;
    }
    REQUIRE(QDir().mkpath(ordner));
    const Instrument instrument = fluegel();
    const auto lege = [&](const QString& name, const std::vector<std::pair<int, int>>& toene, double dauerMs, double abstandS) {
        const auto abstand = static_cast<std::size_t>(abstandS * kSaitenRate);
        std::vector<float> folge;
        std::size_t nr = 0;
        for (const auto& [hoehe, stufe] : toene)
        {
            const Klang k = ton(instrument, hoehe, dauerMs, stufe);
            const std::size_t ab = 2 * nr * abstand;
            if (folge.size() < ab + k.daten.size()) folge.resize(ab + k.daten.size(), 0.0f);
            for (std::size_t i = 0; i < k.daten.size(); ++i) folge[ab + i] += k.daten[i] * 0.5f;
            ++nr;
        }
        REQUIRE(schreibeWav(QDir(ordner).filePath(name + QStringLiteral(".wav")), folge, kSaitenRate));
    };
    // die Tasten C1 … C8 je ff, mf, pp
    for (const auto& [name, stufe] : {std::pair{"01_c_ff", 0}, std::pair{"02_c_mf", -12}, std::pair{"03_c_pp", -26}})
    {
        std::vector<std::pair<int, int>> toene;
        for (int hoehe = 24; hoehe <= 108; hoehe += 12) toene.emplace_back(hoehe, stufe);
        lege(QString::fromLatin1(name), toene, 1200.0, 1.5);
    }
    // C4 lang: die zweite Stufe des Chors
    lege(QStringLiteral("04_c4_lang"), {{60, 0}}, 8000.0, 0.0);
    // A0, E1 (eine Saite), G6 (ohne Daempfer) kurz angeschlagen
    lege(QStringLiteral("05_kurz"), {{21, -6}, {28, -6}, {91, -6}, {96, -6}}, 250.0, 1.5);
    MESSAGE("Hoerproben in " << ordner.toStdString());
}
