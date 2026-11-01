/**
 * @file PYin.hpp
 * @brief Loop: der Tonhoehenverlauf einer einstimmigen Aufnahme mit pYIN — eigener Nachbau nach
 *        Mauch & Dixon, „PYIN: A fundamental frequency estimator using probabilistic threshold
 *        distributions“, ICASSP 2014 (docs/audio/Loop_Konzept.md §4p, WV6 a, Stufe 15g)
 *
 * **Stufe 1 (§2.1 des Papers):** je Rahmen die Differenzfunktion d(τ) (Gl. 1, ueber die
 * Autokorrelation Gl. 2 und 3 mit `loop/Fft`), normiert wie bei YIN zu d′(τ). Statt einer
 * Schwelle s gelten 100 Schwellen 0,01 … 1 mit einer Beta-Verteilung als Gewicht; je Schwelle
 * waehlt YIN eine Periode (das erste Tal mit d′ < s im Bereich des HMM; sonst das tiefste Tal im
 * Bereich mit dem Gewicht p_a = 0,01). Die Summe der Gewichte je Periode ist ihre Wahrscheinlichkeit (Gl. 4, 5);
 * die Frequenz wird wie bei YIN parabolisch verfeinert.
 *
 * **Stufe 2 (§2.2):** ein HMM mit 480 Stufen zu 10 Cent von 55 Hz bis knapp 880 Hz, je Stufe ein
 * Zustand gesungen und einer nicht gesungen; Beobachtung nach Gl. 6, Wechsel gesungen / nicht
 * gesungen 0,99 / 0,01 (Gl. 7), die Tonhoehe springt hoechstens 25 Stufen je Rahmen,
 * dreieckig gewichtet (Gl. 8); Anfang gleich verteilt ueber die Zustaende nicht gesungen;
 * Viterbi.
 *
 * **Abweichungen vom Text des Papers:** (1) dort steht fuer die Beta-Verteilungen „α = 1“; Bild 2
 * und die genannten Mittelwerte 0,1 / 0,15 / 0,2 passen nur zu α = 2 (Herleitung in
 * docs/modellierung/wave-clips/recherche/2026-11-01_S169_pyin.md). (2) **Talregel** (Entscheid
 * Patrik PY2 a, S170; wie librosa `pyin`): das Paper sucht ab τ = 2 die erste Stelle unter s. Bei
 * hohen Stimmen liegt die halbe Periode (oder ein Tal bei einem Formanten um 1 kHz) dann ueber
 * der obersten Stufe des HMM, faellt weg, und der Rahmen gilt als stumm — ein Sopran bei 440 Hz
 * war zu 0 % gesungen, mit der Regel zu 99,6 %; an vocadito gleich (Bericht
 * docs/modellierung/wave-clips/proben/S170_pruefstein_15h.md).
 *
 * Rahmen 2048, Schritt 256 bei 44,1 kHz wie im Paper — bei anderer Rate dieselben Zeiten.
 * Rahmen k beginnt W / 2 vor dem Abtastwert k · Schritt (W = halbe Rahmenlaenge; vor dem
 * Anfang und hinter dem Ende Stille): d(τ) misst um Anfang + (W + τ) / 2, so liegt die gemessene
 * Stelle τ / 2 hinter k · Schritt (das Paper legt den Zeitbezug nicht fest; von Claude gesetzt,
 * gegengeprueft an vocadito). Reines C++ ohne Qt; derselbe Aufruf gibt dieselben Werte.
 */
#pragma once

#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// Wie im Paper (§2.2): 480 Stufen zu 10 Cent ab 55 Hz.
inline constexpr int kPYinStufen = 480;
inline constexpr double kPYinUntenHz = 55.0;
/// Rahmen und Schritt bei 44,1 kHz (§3).
inline constexpr int kPYinRahmen441 = 2048;
inline constexpr int kPYinSchritt441 = 256;

/// Die Einstellungen; die Vorgaben sind die des Papers.
struct PYinEinstellungen
{
    /// Der Mittelwert der Beta-Verteilung der Schwellen (0,10, 0,15 oder 0,20 im Paper; α = 2).
    /// 0,20 als Vorgabe (Entscheid Patrik PY1 a, S169): an vocadito das beste F-Mass (94,2 % gegen
    /// 91,4 % mit 0,10) und die wenigsten falschen Rahmen; das Paper bevorzugt an synthetischem
    /// Gesang knapp 0,10 (Bericht docs/modellierung/wave-clips/proben/S169_pyin_vocadito.md).
    double schwelleMittel = 0.20;
    double pa = 0.01;            ///< Gewicht des kleinsten d′, wenn keine Periode unter der Schwelle liegt (Gl. 5)
    double wechsel = 0.01;       ///< Wechsel gesungen / nicht gesungen je Rahmen (Gl. 7)
    int sprungStufen = 25;       ///< so weit springt die Tonhoehe hoechstens je Rahmen (Gl. 8)
};

/// Ein Kandidat aus Stufe 1.
struct PYinKandidat
{
    double hz = 0.0;
    double p = 0.0;  ///< seine Wahrscheinlichkeit (Gl. 4)
};

/// Der Verlauf: je Rahmen die Frequenz (0 = nicht gesungen) und wie sicher Stufe 1 einen Ton sah.
struct Tonhoehenverlauf
{
    int rate = 0;
    int schritt = 0;              ///< Abtastwerte je Rahmen
    std::vector<float> hz;        ///< je Rahmen; 0 = nicht gesungen
    std::vector<float> stimmhaft; ///< je Rahmen die Summe der Kandidaten (0 … 1)

    [[nodiscard]] double zeitS(std::size_t rahmen) const { return rate > 0 ? static_cast<double>(rahmen) * schritt / rate : 0.0; }
    [[nodiscard]] bool operator==(const Tonhoehenverlauf&) const = default;
};

/// Stufe 1 allein, fuer Tests: die Kandidaten des Rahmens um Abtastwert `mitte` (Mono).
[[nodiscard]] std::vector<PYinKandidat> pyinKandidaten(const std::vector<double>& mono, int rate, std::int64_t mitte,
                                                       const PYinEinstellungen& einstellungen = {});

/// Den Verlauf messen: `stereo` (links und rechts im Wechsel; gemessen an ihrer Mitte) in `rate`.
[[nodiscard]] Tonhoehenverlauf messeTonhoehenverlauf(const std::vector<float>& stereo, int rate,
                                                     const PYinEinstellungen& einstellungen = {});

/// Die Frequenz der Stufe `stufe` (0 … 479) des HMM.
[[nodiscard]] double pyinStufeHz(int stufe);

}  // namespace lumi::loop
