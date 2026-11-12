/**
 ****************************************************************************************
 * @file   test_LoopZunge.cpp
 * @brief  Stufe 34j-7: der Kern `freeReed` im Ton der App — laufender Wind (docs/audio/Instrumente_IC_Konzept.md 34j-7,
 *         ZV3 a, SK4 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Ohne Qt-Anwendung, ohne Ton, ohne Engine. Die Instrumente entstehen aus den Vorlagen des Kerns, gestimmt und normiert
 * wie `InstrumentTuneTool` (die Dateien kommen mit 34j-8).
 *
 * - Ein Kern mit dauernder Anregung bekommt Expression, Pitch und Vibrato der Note als Spuren in den Auftrag; die
 *   Expression wirkt nicht als Pegel hinter dem Ton: eine Expression des Musters gleich einem festen Wert der Spur des
 *   Kerns, Bit fuer Bit.
 * - Ein Crescendo am gehaltenen Ton macht ihn lauter und heller; die Spur Pitch senkt die Tonhoehe ueber das Ventil.
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentMeasure/Bausteine.hpp"
#include "InstrumentMeasure/ZungeStimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "InstrumentModels/zunge/ZungeKern.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numbers>
#include <string>
#include <vector>

using namespace lumi::loop;
namespace kern = lumi::kern;
namespace zunge = lumi::kern::zunge;

namespace
{

constexpr int kRate = 48000;

const zunge::Vorlage& vorlage(std::string_view name)
{
    for (const zunge::Vorlage& v : zunge::vorlagen())
        if (v.name == name) return v;
    FAIL("keine Vorlage " << name);
    return zunge::vorlagen().front();
}

/// Das Instrument einer Vorlage, gestimmt und normiert wie `InstrumentTuneTool`.
Instrument instrumentVon(std::string_view name)
{
    const zunge::Vorlage& v = vorlage(name);
    kern::Werte w = zunge::werteVon(v);
    // ueber den Stimmer des Kerns (alle gestimmten Werte, dann der Bezug) — keine eigene Liste der Namen
    const lumi::messung::StimmBericht b = lumi::messung::stimmeUndNormiere(kern::kerne(), lumi::messung::stimmer(), zunge::kKernName, w, true, true);
    REQUIRE_MESSAGE(b.fehler.empty(), (b.fehler.empty() ? std::string() : b.fehler.front()));
    REQUIRE(b.gestimmt == zunge::gestimmteNamen());
    REQUIRE(kern::richteWerte(zunge::kernTyp().tafel, w).empty());
    std::vector<std::string> fehler;
    Instrument i;
    i.titel = QString::fromStdString(v.titel);
    Schicht s;
    s.name = i.titel;
    s.kernName = QString::fromUtf8(zunge::kKernName.data(), static_cast<qsizetype>(zunge::kKernName.size()));
    s.kernWerte = w;
    richteSchicht(s, &fehler);
    REQUIRE(fehler.empty());
    i.schichten[0] = s;
    return i;
}

const Instrument& akkordeon()
{
    static const Instrument i = instrumentVon("accordion");
    return i;
}

const Instrument& melodica()
{
    static const Instrument i = instrumentVon("melodica");
    return i;
}

/// Der linke Kanal eines Tons.
std::vector<double> links(const Klang& k)
{
    std::vector<double> m;
    for (std::size_t i = 0; i + 1 < k.daten.size(); i += 2) m.push_back(k.daten[i]);
    return m;
}

double rmsDb(const std::vector<double>& x, double vonS, double bisS)
{
    const auto a = static_cast<std::size_t>(vonS * kRate), b = std::min(x.size(), static_cast<std::size_t>(bisS * kRate));
    double s = 0.0;
    for (std::size_t i = a; i < b; ++i) s += x[i] * x[i];
    return 10.0 * std::log10(s / static_cast<double>(std::max<std::size_t>(1, b - a)) + 1e-30);
}

/// Der Schwerpunkt des Spektrums ueber die ersten zwanzig Teiltoene (Fenster nach Hann).
double schwerpunktHz(const std::vector<double>& x, double vonS, double f0)
{
    const auto a = static_cast<std::size_t>(vonS * kRate);
    const std::size_t n = 9600;
    double summe = 0.0, gewicht = 0.0;
    for (int k = 1; k <= 20; ++k)
    {
        std::complex<double> s(0.0, 0.0);
        for (std::size_t i = 0; i < n && a + i < x.size(); ++i)
            s += (0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * i / n)) * x[a + i] *
                 std::exp(std::complex<double>(0.0, -2.0 * std::numbers::pi * k * f0 * static_cast<double>(i) / kRate));
        summe += k * f0 * std::abs(s);
        gewicht += std::abs(s);
    }
    return summe / gewicht;
}

double tonhoeheHz(const std::vector<double>& x, double vonS, double bisS)
{
    const std::vector<double> stueck(x.begin() + static_cast<std::ptrdiff_t>(vonS * kRate), x.begin() + static_cast<std::ptrdiff_t>(bisS * kRate));
    const lumi::messung::Tonhoehe th = lumi::messung::tonhoehe(stueck, kRate, 30.0, 3000.0);
    std::vector<double> hz;
    for (const double f : th.hz)
        if (f > 0.0) hz.push_back(f);
    REQUIRE_FALSE(hz.empty());
    std::nth_element(hz.begin(), hz.begin() + static_cast<std::ptrdiff_t>(hz.size() / 2), hz.end());
    return hz[hz.size() / 2];
}

TonAngabe ton(int hoehe, int dauerMs)
{
    TonAngabe t;
    t.hoehe = hoehe;
    t.dauerMs = dauerMs;
    return t;
}

}  // namespace

TEST_CASE("Loop Zunge 34j-7: die Expression ist der Wind, kein Pegel (ZV3 a, SK4 a)")
{
    const Instrument& a = akkordeon();
    REQUIRE(anregungDauernd(a.schichten[0]));
    // die Expression des Musters, fest 0,5, gleich dem festen Wert der Spur des Kerns — beide gehen als Spur in den
    // Auftrag, keine wird als Pegel hinter den Ton gelegt
    TonAngabe muster = ton(69, 800);
    muster.ausdruckMuster = {{0, 500}};
    TonAngabe fest = ton(69, 800);
    fest.kernSpuren = {{QStringLiteral("expression"), 0.5}};
    const Klang km = rendereTon(a, muster, kRate), kf = rendereTon(a, fest, kRate);
    REQUIRE_FALSE(km.daten.empty());
    CHECK(km.daten == kf.daten);
    // Muster mal Clip: 0,5 mal 1 ist dasselbe; 0,5 mal 0,5 ist leiser, aber nicht um e² = −12 dB des Pegels, sondern wie der
    // Balg bei der Staerke 0,25 (S191: rund −10 LU gegen 1, −4 gegen 0,5)
    TonAngabe beide = muster;
    beide.ausdruckClip = {{0, 1000}};
    CHECK(rendereTon(a, beide, kRate).daten == km.daten);
    beide.ausdruckClip = {{0, 500}};
    const std::vector<double> l05 = links(km), l025 = links(rendereTon(a, beide, kRate));
    const double unterschied = rmsDb(l025, 0.4, 0.8) - rmsDb(l05, 0.4, 0.8);
    MESSAGE("Akkordeon A4: Expression 0,25 gegen 0,5 " << unterschied << " dB");
    CHECK(unterschied < -1.0);
    CHECK(unterschied > -10.0);
    // eine Schicht ohne dauernde Anregung behaelt den Pegel (SK12 a) — der Kern `bar` mit Bogen ist nicht betroffen
    CHECK_FALSE(kern::kerne().typ("bar")->auskunft.anregungDauernd);
}

TEST_CASE("Loop Zunge 34j-7: ein Crescendo am gehaltenen Ton wird lauter und heller")
{
    const Instrument& m = melodica();
    TonAngabe t = ton(69, 2000);
    t.ausdruckMuster = {{0, 0}, {1500, 1000}};
    const std::vector<double> x = links(rendereTon(m, t, kRate));
    const double leise = rmsDb(x, 0.25, 0.45), laut = rmsDb(x, 1.55, 1.75);
    const double hellLeise = schwerpunktHz(x, 0.25, 440.0), hellLaut = schwerpunktHz(x, 1.55, 440.0);
    MESSAGE("Melodica A4 Crescendo: " << laut - leise << " dB lauter, Schwerpunkt " << hellLeise << " → " << hellLaut << " Hz");
    CHECK(laut - leise > 8.0);
    // die Teiltoene aendern sich mit dem Wind (34j-7); Befund S191: im Modell wird der Ton dabei dunkler, nicht heller
    CHECK(std::abs(hellLaut / hellLeise - 1.0) > 0.05);
    // ohne Verlauf, fest bei 0 und bei 1: das Crescendo liegt zwischen beiden
    TonAngabe pp = ton(69, 2000), ff = ton(69, 2000);
    pp.ausdruckMuster = {{0, 0}};
    ff.ausdruckMuster = {{0, 1000}};
    CHECK(rmsDb(links(rendereTon(m, pp, kRate)), 1.55, 1.75) < laut);
    CHECK(rmsDb(links(rendereTon(m, ff, kRate)), 1.55, 1.75) > leise);
}

TEST_CASE("Loop Zunge 34j-7: die Spur Pitch senkt die Tonhoehe ueber das Ventil, das Vibrato laeuft mit")
{
    const Instrument& m = melodica();
    // am mittleren Druck (Expression 0,5), wo der Stimmer die Kurve misst (ZV8 a)
    TonAngabe t = ton(60, 2000);
    t.ausdruckMuster = {{0, 500}};
    t.tonhoehe = {{0, 0}, {600, 0}, {1200, -20}};
    const std::vector<double> x = links(rendereTon(m, t, kRate));
    const double vorher = tonhoeheHz(x, 0.3, 0.6), nachher = tonhoeheHz(x, 1.4, 1.9);
    const double cent = 1200.0 * std::log2(nachher / vorher);
    // mit vollem Wind (ohne Expression) wird derselbe Bend tiefer (ZV7 a)
    TonAngabe ff = t;
    ff.ausdruckMuster.clear();
    const std::vector<double> xf = links(rendereTon(m, ff, kRate));
    const double centFf = 1200.0 * std::log2(tonhoeheHz(xf, 1.4, 1.9) / tonhoeheHz(xf, 0.3, 0.6));
    MESSAGE("Melodica C4: Pitch −20 Cent ueber die Spur ergibt " << cent << " Cent in der Mitte, " << centFf << " Cent bei vollem Wind");
    CHECK(cent < -15.0);
    CHECK(cent > -25.0);
    CHECK(centFf < cent);
    // ohne Spur Pitch derselbe Ton bis 600 ms (die Spur steht dort bei 0)
    TonAngabe ohneSpur = ton(60, 2000);
    ohneSpur.ausdruckMuster = {{0, 500}};
    const std::vector<double> ohne = links(rendereTon(m, ohneSpur, kRate));
    CHECK(std::equal(x.begin(), x.begin() + 600 * kRate / 1000, ohne.begin()));
    // das Vibrato als Spur: aendert den Ton erst, wo es einsetzt
    TonAngabe v = ohneSpur;
    v.vibrato = {{0, 0}, {1000, 0}, {1001, 30}};
    const std::vector<double> xv = links(rendereTon(m, v, kRate));
    CHECK(std::equal(xv.begin(), xv.begin() + kRate, ohne.begin()));
    CHECK_FALSE(std::equal(xv.begin(), xv.end(), ohne.begin()));
}
