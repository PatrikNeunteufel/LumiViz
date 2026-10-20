/**
 ****************************************************************************************
 * @file   test_LoopVibratoPitch.cpp
 * @brief  Loop, Stufe 28c-1b: die Spuren Vibrato und Pitch im Ton einer Synth-Schicht
 *         (docs/audio/Spielweisen_Konzept.md §3, Entscheide Patrik S139/S140: SF4 a,
 *         SK3 a, SK10 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Engine, ohne Dateien aus dem Repo: die Toene rechnet ein
 * Sinus-Instrument, das der Test baut; gemessen wird an den Nulldurchgaengen.
 *
 * - Der Name: `:q` und `:w` mit ms und Cent (auch negativ), hin und zurueck; ohne Spuren
 *   der Name wie bis Stufe 27.
 * - Die Planung: die Spuren ueber dem Ton in ms ab dem Anschlag, ueber die Naht, hinter
 *   dem Clip der Wert dort.
 * - Pitch: die Tonhoehe folgt der Spur bis *Pitch range* (SK3 a), gemessen.
 * - Vibrato: ohne eigene Rate 5,5 Hz (SK10 a) mit der Tiefe der Spur, gemessen; die Spur
 *   auf 0 schaltet das Vibrato der Schicht ab (SF4 a).
 * - Die Datei: `pitchRange` an jeder Schicht, immer, Fassung 14.
 * - Rechenzeit: Toene je Vorkommen gegen Toene je Tonhoehe (meldet nur Zahlen).
 ****************************************************************************************
 */

#include <doctest.h>

#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Pedalplan.hpp"
#include "loop/Rechenkern.hpp"

#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Ein Takt (16 Schritte, 120 BPM: ein Schritt 125 ms) des Instruments „probe" mit
/// diesen Noten, als Clip mit `mal` Wiederholungen.
Loop spurLoop(std::vector<Note> noten, int mal = 1, int takte = 1)
{
    Loop loop;
    loop.laut = 1.0;
    Muster m;
    m.name = QStringLiteral("P");
    m.art = MusterArt::Instrument;
    m.takte = takte;
    m.instrument = QStringLiteral("probe");
    m.noten = std::move(noten);
    loop.muster = {m};
    loop.kanaele = {Kanal{QStringLiteral("K"), true, {LoopClip(QStringLiteral("P"), 0, mal)}}};
    loop.richte();
    return loop;
}

/// Ein Sinus ohne Huellkurve, die stoert: sofort voll, gehalten.
Instrument sinus(double rangeHt = kPitchRangeVorgabe, double vibratoHz = 0.0, double vibratoCent = 0.0)
{
    Instrument instrument;
    instrument.name = QStringLiteral("probe");
    Schicht& s = instrument.schichten[0];
    s.anstiegMs = 1.0;
    s.abfallMs = 0.0;
    s.halten = 1.0;
    s.ausklangMs = 20.0;
    s.pitchRangeHt = rangeHt;
    s.vibratoHz = vibratoHz;
    s.vibratoCent = vibratoCent;
    return instrument;
}

/// Die Abstaende der steigenden Nulldurchgaenge links, als Frequenzen, zwischen zwei Stellen in ms.
std::vector<double> frequenzen(const Klang& klang, double vonMs, double bisMs)
{
    std::vector<double> hz;
    const auto von = static_cast<std::int64_t>(vonMs * kRate / 1000.0);
    const auto bis = std::min(klang.frames(), static_cast<std::int64_t>(bisMs * kRate / 1000.0));
    double vorige = -1.0;
    for (std::int64_t f = std::max<std::int64_t>(1, von); f < bis; ++f)
    {
        const float a = klang.daten[static_cast<std::size_t>(2 * (f - 1))];
        const float b = klang.daten[static_cast<std::size_t>(2 * f)];
        if (a < 0.0f && b >= 0.0f)
        {
            const double stelle = static_cast<double>(f - 1) + a / (a - b);  // linear zwischen den Werten
            if (vorige >= 0.0) hz.push_back(kRate / (stelle - vorige));
            vorige = stelle;
        }
    }
    return hz;
}

double mittel(const std::vector<double>& werte)
{
    double s = 0.0;
    for (const double w : werte) s += w;
    return werte.empty() ? 0.0 : s / static_cast<double>(werte.size());
}

}  // namespace

TEST_SUITE("LoopVibratoPitch")
{
    TEST_CASE("Loop Vibrato Pitch 28c-1b: der Name — :q und :w hin und zurueck, ohne Spuren wie bisher")
    {
        TonAngabe ton;
        ton.instrument = QStringLiteral("probe");
        ton.hoehe = 69;
        ton.dauerMs = 1000;
        CHECK(tonKlang(ton) == tonKlang(QStringLiteral("probe"), 69, 1000));
        ton.tonhoehe = {{0, 0}, {500, -150}, {500, 200}};
        ton.vibrato = {{0, 0}, {250, 40}};
        ton.teil = TonTeil::Eigen;
        ton.endeMs = 900;
        const QString name = tonKlang(ton);
        CHECK(name.contains(QStringLiteral(":q0=0,500=-150,500=200")));
        CHECK(name.contains(QStringLiteral(":w0=0,250=40")));
        TonAngabe zurueck;
        REQUIRE(zerlegeTon(name, zurueck));
        CHECK(zurueck == ton);
    }

    TEST_CASE("Loop Vibrato Pitch 28c-1b: die Planung — die Spuren ueber dem Ton, ueber die Naht, hinter dem Clip")
    {
        // ohne Spuren: derselbe Ton wie bis Stufe 27
        Loop ohne = spurLoop({Note{0, 16, 69}});
        CHECK_FALSE(spieltJeVorkommen(ohne.muster.front()));
        CHECK(tonKlang(tonImClip(ohne, ohne.muster.front(), 1, 0, 0)) == tonKlang(QStringLiteral("probe"), 69, 2000));

        // Pitch: 0 bei 0, −1 Halbton bei 8 — ueber eine Note von 16 Schritten (2000 ms)
        Loop loop = spurLoop({Note{0, 16, 69}});
        loop.muster.front().zeitspuren.von(Steuerspur::Pitch) = {{0.0, 0.0}, {8.0, -1.0}};
        loop.richte();
        CHECK(spieltJeVorkommen(loop.muster.front()));
        const TonAngabe ton = tonImClip(loop, loop.muster.front(), 1, 0, 0);
        CHECK(ton.dauerMs == 2000);
        CHECK(ton.tonhoehe == std::vector<TonPunkt>{{0, 0}, {1000, -100}, {2000, -100}});
        CHECK(ton.vibrato.empty());

        // eine Note ueber die Naht (Pedal haelt sie) — die Spur springt dort auf ihren ersten Wert
        Loop naht = spurLoop({Note{12, 4, 69}}, 2);
        naht.muster.front().zeitspuren.von(Steuerspur::Vibrato) = {{4.0, 20.0}, {12.0, 60.0}};
        naht.muster.front().zeitspuren.von(Steuerspur::Sustain) = {{0.0, 1.0}};
        naht.richte();
        const TonAngabe ueber = tonImClip(naht, naht.muster.front(), 2, 0, 0);
        CHECK(ueber.dauerMs == 2000);  // gehalten bis zum neuen Anschlag bei 28
        // ab 12: 60; an der Naht (16, 500 ms) 60, dann 20; bei 20 (1000 ms) 20, bei 28 (2000 ms) 60
        CHECK(ueber.vibrato == std::vector<TonPunkt>{{0, 60}, {500, 60}, {500, 20}, {1000, 20}, {2000, 60}});
        // die letzte Wiederholung: hinter dem Clip (32) der Wert dort
        const TonAngabe letzte = tonImClip(naht, naht.muster.front(), 2, 1, 0);
        CHECK(letzte.vibrato.back().cent == 60);

        // eine Spur Pitch, die ueberall 0 ist, faellt weg
        Loop null = spurLoop({Note{0, 4, 69}});
        null.muster.front().zeitspuren.von(Steuerspur::Pitch) = {{0.0, 0.0}};
        null.richte();
        CHECK(tonImClip(null, null.muster.front(), 1, 0, 0).tonhoehe.empty());

        // Namen und Planung passen zusammen
        const Ablauf ablauf = plane(loop, kRate);
        REQUIRE(ablauf.klangNamen.size() == 1);
        CHECK(ablauf.klangNamen.front() == tonKlang(ton));
        for (const Stimme& stimme : ablauf.stimmen) CHECK(stimme.klang == 0);
    }

    TEST_CASE("Loop Vibrato Pitch 28c-1b: Pitch folgt der Spur bis Pitch range — gemessen")
    {
        TonAngabe ton;
        ton.instrument = QStringLiteral("probe");
        ton.hoehe = 69;  // 440 Hz
        ton.dauerMs = 1000;
        const double a4 = 440.0;
        // ohne Spur
        CHECK(mittel(frequenzen(rendereTon(sinus(), ton, kRate), 100.0, 900.0)) == doctest::Approx(a4).epsilon(1e-3));
        // +1 Halbton, in der Spanne
        ton.tonhoehe = {{0, 100}};
        CHECK(mittel(frequenzen(rendereTon(sinus(), ton, kRate), 100.0, 900.0)) ==
              doctest::Approx(a4 * std::exp2(1.0 / 12.0)).epsilon(1e-3));
        // +7 Halbtoene: mit der Vorgabe ±2 gekappt, mit ±12 ganz
        ton.tonhoehe = {{0, 700}};
        CHECK(mittel(frequenzen(rendereTon(sinus(2.0), ton, kRate), 100.0, 900.0)) ==
              doctest::Approx(a4 * std::exp2(2.0 / 12.0)).epsilon(1e-3));
        CHECK(mittel(frequenzen(rendereTon(sinus(12.0), ton, kRate), 100.0, 900.0)) ==
              doctest::Approx(a4 * std::exp2(7.0 / 12.0)).epsilon(1e-3));
        // ein Fall: von 0 auf −2 Halbtoene zwischen 500 und 900 ms
        ton.tonhoehe = {{0, 0}, {500, 0}, {900, -200}};
        const Klang fall = rendereTon(sinus(), ton, kRate);
        CHECK(mittel(frequenzen(fall, 100.0, 450.0)) == doctest::Approx(a4).epsilon(1e-3));
        CHECK(mittel(frequenzen(fall, 900.0, 990.0)) == doctest::Approx(a4 * std::exp2(-2.0 / 12.0)).epsilon(2e-3));
    }

    TEST_CASE("Loop Vibrato Pitch 28c-1b: Vibrato — ohne eigene Rate 5,5 Hz mit der Tiefe der Spur, 0 schaltet ab")
    {
        TonAngabe ton;
        ton.instrument = QStringLiteral("probe");
        ton.hoehe = 69;
        ton.dauerMs = 2000;
        ton.vibrato = {{0, 50}};  // 50 Cent
        const std::vector<double> hz = frequenzen(rendereTon(sinus(), ton, kRate), 200.0, 1900.0);
        REQUIRE(hz.size() > 100);
        const double hoch = *std::max_element(hz.begin(), hz.end());
        const double tief = *std::min_element(hz.begin(), hz.end());
        MESSAGE("Vibrato 50 ct: " << tief << " bis " << hoch << " Hz");
        // die Messung je Periode mittelt ueber eine Schwingung: etwas weniger als die volle Tiefe
        CHECK(1200.0 * std::log2(hoch / 440.0) == doctest::Approx(50.0).epsilon(0.05));
        CHECK(1200.0 * std::log2(440.0 / tief) == doctest::Approx(50.0).epsilon(0.05));
        // die Rate: so oft geht die Frequenz in 1,7 s durch 440 Hz nach oben
        int durchgaenge = 0;
        for (std::size_t i = 1; i < hz.size(); ++i)
            if (hz[i - 1] < 440.0 && hz[i] >= 440.0) ++durchgaenge;
        CHECK(durchgaenge / 1.7 == doctest::Approx(kVibratoHzVorgabe).epsilon(0.1));

        // eine Schicht mit eigenem Vibrato (30 ct, 6 Hz): die Spur auf 0 schaltet es ab (SF4 a)
        TonAngabe still = ton;
        still.vibrato = {{0, 0}};
        const std::vector<double> ruhig = frequenzen(rendereTon(sinus(2.0, 6.0, 30.0), still, kRate), 200.0, 1900.0);
        CHECK(*std::max_element(ruhig.begin(), ruhig.end()) == doctest::Approx(440.0).epsilon(1e-3));
        // ohne Spur klingt ihr eigenes
        TonAngabe eigen = ton;
        eigen.vibrato.clear();
        const std::vector<double> schwingt = frequenzen(rendereTon(sinus(2.0, 6.0, 30.0), eigen, kRate), 200.0, 1900.0);
        CHECK(1200.0 * std::log2(*std::max_element(schwingt.begin(), schwingt.end()) / 440.0) ==
              doctest::Approx(30.0).epsilon(0.1));
    }

    TEST_CASE("Loop Vibrato Pitch 28c-1b: die Datei — pitchRange an jeder Schicht, immer, Fassung 14")
    {
        Instrument instrument = sinus(7.0);
        const QJsonObject json = alsJson(instrument);
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);  // pitchRange seit 14; geschrieben wird die juengste
        const QJsonObject schicht = json.value(QStringLiteral("layers")).toArray().at(0).toObject();
        REQUIRE(schicht.contains(QStringLiteral("pitchRange")));
        CHECK(schicht.value(QStringLiteral("pitchRange")).toDouble() == 7.0);
        Instrument gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen.schichten.front().pitchRangeHt == 7.0);
        // auch die Vorgabe steht da (Memory „Datei-Felder immer schreiben"); fehlt es, gilt ±2
        CHECK(alsJson(Instrument{}).value(QStringLiteral("layers")).toArray().at(0).toObject().contains(QStringLiteral("pitchRange")));
        QJsonObject ohne = json;
        QJsonArray schichten = ohne.value(QStringLiteral("layers")).toArray();
        QJsonObject s0 = schichten.at(0).toObject();
        s0.remove(QStringLiteral("pitchRange"));
        schichten[0] = s0;
        ohne[QStringLiteral("layers")] = schichten;
        Instrument alt;
        REQUIRE(ausJson(ohne, alt));
        CHECK(alt.schichten.front().pitchRangeHt == kPitchRangeVorgabe);
    }

    TEST_CASE("Loop Vibrato Pitch 28c-1b: Rechenzeit — Toene je Vorkommen gegen je Tonhoehe (meldet nur Zahlen)")
    {
        // 4 Takte, 16 Achtelnoten auf vier Tonhoehen, 4 Wiederholungen
        std::vector<Note> noten;
        for (int i = 0; i < 16; ++i) noten.push_back(Note{i * 4, 4, 60 + 2 * (i % 4)});
        Loop flach = spurLoop(noten, 4, 4);
        REQUIRE(flach.muster.front().noten.size() == 16);
        Loop kurvig = flach;
        kurvig.muster.front().zeitspuren.von(Steuerspur::Pitch) = {{0.0, 0.0}, {32.0, 1.0}, {63.0, -1.0}};
        kurvig.muster.front().zeitspuren.von(Steuerspur::Vibrato) = {{0.0, 0.0}, {48.0, 40.0}};
        kurvig.richte();
        const auto messe = [](const Loop& loop) {
            const Ablauf ablauf = plane(loop, kRate);
            const Instrument instrument = sinus();
            const auto beginn = std::chrono::steady_clock::now();
            for (const QString& name : ablauf.klangNamen)
            {
                TonAngabe ton;
                REQUIRE(zerlegeTon(name, ton));
                (void)rendereTon(instrument, ton, kRate);
            }
            const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - beginn).count();
            return std::pair<qsizetype, double>{ablauf.klangNamen.size(), ms};
        };
        const auto [flachToene, flachMs] = messe(flach);
        const auto [kurvigToene, kurvigMs] = messe(kurvig);
        MESSAGE("ohne Spuren: " << flachToene << " Toene in " << flachMs << " ms; mit Pitch und Vibrato: " << kurvigToene
                                << " Toene in " << kurvigMs << " ms");
        CHECK(flachToene == 4);    // je Tonhoehe einer
        CHECK(kurvigToene == 16);  // je Note einer — die Wiederholungen spielen dieselben
    }
}
