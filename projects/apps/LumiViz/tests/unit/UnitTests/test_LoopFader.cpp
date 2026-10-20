/**
 ****************************************************************************************
 * @file   test_LoopFader.cpp
 * @brief  Loop, Stufe 27e-1: der Fader je Kanal in Modell, Datei und Klang
 *         (docs/audio/Loop_Konzept.md §4n, Entscheide Patrik S139, LL3 a, LL8 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Engine, ohne Dateien aus dem Repo: der Klang der
 * Drum-Spur ist ein gerechneter Sinus.
 *
 * - Die Datei: `gain` steht immer da (auch bei 0 dB), wird gelesen und auf
 *   `kKanalGainMinDb` … `kKanalGainMaxDb` begrenzt; fehlt es, gilt 0 dB; Fassung 11.
 * - Der Klang: +6,02 dB ist doppelt so laut, an der unteren Grenze schweigt der
 *   Kanal, 0 dB ist Bit fuer Bit wie ohne Fader; zwei Kanaele je mit eigenem Fader.
 * - Nach den Effekten: an einem Clip mit Distortion skaliert der Fader die
 *   verzerrte Summe — der Klang bleibt derselbe, nur lauter (ein Fader vor dem
 *   Verzerrer veraenderte ihn).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <cmath>
#include <memory>
#include <numbers>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Ein Takt Drums auf einem Kanal mit Fader `gainDb`; mit Distortion am Clip, wenn verlangt.
Loop drumLoop(double gainDb, bool mitVerzerrung = false)
{
    Loop loop;
    loop.laut = 1.0;
    Muster m;
    m.name = QStringLiteral("D");
    m.art = MusterArt::Drumkit;
    m.takte = 1;
    m.spuren = {Spur(QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0)};
    m.zeilen = {std::vector<double>(16, 0.0)};
    m.zeilen[0][0] = 1.0;
    m.zeilen[0][8] = 0.8;
    loop.muster = {m};
    ClipEffekte fx;
    if (mitVerzerrung) fx.reihe = {ClipVerzerrung{true, 0.8, 0.5, 1.0}};
    Kanal k{QStringLiteral("K"), true, {LoopClip(QStringLiteral("D"), 0, 1, fx)}, gainDb};
    loop.kanaele = {k};
    loop.richte();
    return loop;
}

/// Je Klangname des Ablaufs ein Sinus von 50 ms bei 220 Hz, Spitze 0,5.
std::vector<Klang> sinusKlaenge(const Ablauf& ablauf)
{
    Klang k;
    const int frames = kRate / 20;
    for (int i = 0; i < frames; ++i)
    {
        const auto x = static_cast<float>(0.5 * std::sin(2.0 * std::numbers::pi * 220.0 * i / kRate));
        k.daten.push_back(x);
        k.daten.push_back(x);
    }
    return std::vector<Klang>(static_cast<std::size_t>(ablauf.klangNamen.size()), k);
}

std::vector<float> rechneOhneEffekte(const Loop& loop)
{
    const Ablauf ablauf = plane(loop, kRate);
    std::vector<float> aus(static_cast<std::size_t>(ablauf.durchlaufFrames) * 2);
    rechne(ablauf, sinusKlaenge(ablauf), 0, static_cast<int>(ablauf.durchlaufFrames), aus.data());
    return aus;
}

/// Durch den Klangerzeuger, mit den Effekten der Clips.
std::vector<float> rechneMitEffekten(const Loop& loop)
{
    const Ablauf ablauf = plane(loop, kRate);
    const auto klaenge = std::make_shared<const std::vector<Klang>>(sinusKlaenge(ablauf));
    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(baueProgramm(loop, kRate, klaenge));
    const int frames = static_cast<int>(ablauf.durchlaufFrames);
    std::vector<float> aus(static_cast<std::size_t>(frames) * 2);
    for (int stelle = 0; stelle < frames; stelle += 441)
        erzeuger.render(aus.data() + 2 * static_cast<std::ptrdiff_t>(stelle), std::min(441, frames - stelle));
    return aus;
}

float spitze(const std::vector<float>& werte)
{
    float s = 0.0f;
    for (const float w : werte) s = std::max(s, std::abs(w));
    return s;
}

}  // namespace

TEST_SUITE("LoopFader")
{
    TEST_CASE("Loop Fader 27e-1: gain in der Datei — immer geschrieben, gelesen, begrenzt, fehlt = 0 dB, Fassung 11")
    {
        const Loop null = drumLoop(0.0);
        const QJsonObject json = alsJson(null);
        CHECK(json.value(QStringLiteral("version")).toInt() == 11);
        const QJsonObject kanal = json.value(QStringLiteral("channels")).toArray().at(0).toObject();
        REQUIRE(kanal.contains(QStringLiteral("gain")));  // auch bei 0 dB (Memory „Datei-Felder immer schreiben")
        CHECK(kanal.value(QStringLiteral("gain")).toDouble() == 0.0);

        Loop gelesen;
        REQUIRE(ausJson(alsJson(drumLoop(4.5)), gelesen));
        CHECK(gelesen.kanaele.front().gainDb == doctest::Approx(4.5));
        CHECK(gelesen == drumLoop(4.5));

        // ausserhalb der Grenzen wird begrenzt, ein fehlendes Feld ist 0 dB
        QJsonObject zuViel = alsJson(drumLoop(0.0));
        QJsonArray kanaele = zuViel.value(QStringLiteral("channels")).toArray();
        QJsonObject k = kanaele.at(0).toObject();
        k[QStringLiteral("gain")] = 99.0;
        kanaele[0] = k;
        zuViel[QStringLiteral("channels")] = kanaele;
        REQUIRE(ausJson(zuViel, gelesen));
        CHECK(gelesen.kanaele.front().gainDb == kKanalGainMaxDb);
        k[QStringLiteral("gain")] = -1000.0;
        kanaele[0] = k;
        zuViel[QStringLiteral("channels")] = kanaele;
        REQUIRE(ausJson(zuViel, gelesen));
        CHECK(gelesen.kanaele.front().gainDb == kKanalGainMinDb);
        k.remove(QStringLiteral("gain"));
        kanaele[0] = k;
        zuViel[QStringLiteral("channels")] = kanaele;
        zuViel[QStringLiteral("version")] = 10;
        REQUIRE(ausJson(zuViel, gelesen));
        CHECK(gelesen.kanaele.front().gainDb == 0.0);

        // der Faktor
        Kanal f;
        CHECK(f.gainFaktor() == 1.0);
        f.gainDb = 20.0 * std::log10(2.0);
        CHECK(f.gainFaktor() == doctest::Approx(2.0).epsilon(1e-12));
        f.gainDb = kKanalGainMinDb;
        CHECK(f.gainFaktor() == 0.0);
        f.gainDb = kKanalGainMaxDb;
        CHECK(f.gainFaktor() == doctest::Approx(std::pow(10.0, 24.0 / 20.0)));
    }

    TEST_CASE("Loop Limiter 27e-3: limiter in der Datei — immer geschrieben, fehlt = an, aus bleibt aus")
    {
        Loop an = drumLoop(0.0);
        CHECK(an.limiter);
        const QJsonObject json = alsJson(an);
        REQUIRE(json.contains(QStringLiteral("limiter")));
        CHECK(json.value(QStringLiteral("limiter")).toBool());
        Loop aus = an;
        aus.limiter = false;
        Loop gelesen;
        REQUIRE(ausJson(alsJson(aus), gelesen));
        CHECK_FALSE(gelesen.limiter);
        CHECK(gelesen == aus);
        QJsonObject ohne = alsJson(aus);
        ohne.remove(QStringLiteral("limiter"));
        REQUIRE(ausJson(ohne, gelesen));
        CHECK(gelesen.limiter);
    }

    TEST_CASE("Loop Fader 27e-1: der Fader am Klang — doppelt, still, bei 0 dB Bit fuer Bit")
    {
        const std::vector<float> null = rechneOhneEffekte(drumLoop(0.0));
        REQUIRE(spitze(null) > 0.1f);
        const std::vector<float> doppelt = rechneOhneEffekte(drumLoop(20.0 * std::log10(2.0)));
        REQUIRE(doppelt.size() == null.size());
        float abweichung = 0.0f;
        for (std::size_t i = 0; i < null.size(); ++i) abweichung = std::max(abweichung, std::abs(doppelt[i] - 2.0f * null[i]));
        CHECK(abweichung <= 1e-6f);
        CHECK(spitze(rechneOhneEffekte(drumLoop(kKanalGainMinDb))) == 0.0f);

        // 0 dB: dieselben Schlaege wie ein Kanal, der den Fader nie kannte
        const Ablauf a = plane(drumLoop(0.0), kRate);
        REQUIRE_FALSE(a.stimmen.empty());
        CHECK(a.stimmen.front().schlaege.front().links == 1.0f);

        // zwei Kanaele, je mit eigenem Fader: jeder Kanal fuer sich
        Loop zwei = drumLoop(0.0);
        Kanal zweiter = zwei.kanaele.front();
        zweiter.name = QStringLiteral("K2");
        zweiter.gainDb = 20.0 * std::log10(0.5);
        zwei.kanaele.push_back(zweiter);
        zwei.richte();
        const std::vector<float> summe = rechneOhneEffekte(zwei);
        abweichung = 0.0f;
        for (std::size_t i = 0; i < null.size(); ++i) abweichung = std::max(abweichung, std::abs(summe[i] - 1.5f * null[i]));
        CHECK(abweichung <= 1e-6f);
    }

    TEST_CASE("Loop Fader 27e-1: der Fader wirkt nach den Effekten des Clips")
    {
        const std::vector<float> null = rechneMitEffekten(drumLoop(0.0, true));
        REQUIRE(spitze(null) > 0.05f);
        const std::vector<float> doppelt = rechneMitEffekten(drumLoop(20.0 * std::log10(2.0), true));
        REQUIRE(doppelt.size() == null.size());
        float abweichung = 0.0f;
        for (std::size_t i = 0; i < null.size(); ++i) abweichung = std::max(abweichung, std::abs(doppelt[i] - 2.0f * null[i]));
        MESSAGE("groesste Abweichung vom Doppelten: " << abweichung << " bei Spitze " << spitze(null));
        CHECK(abweichung <= 1e-5f);

        // zum Vergleich: der Verzerrer veraendert den Klang — derselbe Clip doppelt so laut
        // in den Verzerrer gegeben waere nicht das Doppelte
        Loop lauterHinein = drumLoop(0.0, true);
        lauterHinein.muster.front().spuren.front().laut = 1.0;
        lauterHinein.muster.front().zeilen[0][0] = 1.0;
        Loop leiserHinein = lauterHinein;
        leiserHinein.muster.front().spuren.front().laut = 0.5;
        const std::vector<float> laut = rechneMitEffekten(lauterHinein);
        const std::vector<float> leise = rechneMitEffekten(leiserHinein);
        float unterschied = 0.0f;
        for (std::size_t i = 0; i < laut.size(); ++i) unterschied = std::max(unterschied, std::abs(laut[i] - 2.0f * leise[i]));
        CHECK(unterschied > 1e-3f);

        // ohne Busse (`rechne` ohne `busAus`) traegt ein Schlag mit Effekten den Fader selbst
        const std::vector<float> trocken = rechneOhneEffekte(drumLoop(0.0, true));
        const std::vector<float> trockenDoppelt = rechneOhneEffekte(drumLoop(20.0 * std::log10(2.0), true));
        abweichung = 0.0f;
        for (std::size_t i = 0; i < trocken.size(); ++i)
            abweichung = std::max(abweichung, std::abs(trockenDoppelt[i] - 2.0f * trocken[i]));
        CHECK(abweichung <= 1e-6f);
    }
}
