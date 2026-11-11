/**
 * @file ZungeStimmen.hpp
 * @brief Messen am Kern `freeReed` (Klasse 10): Tonhoehe, Ansprechen und Einschwingen einer Zunge
 *        (Instrumente_IC_Konzept.md 34j-2, IK2-2 a; der Stimmer folgt mit 34j-3)
 *
 * Uebernommen aus IC `rechnung/klassen/10_zunge/Zunge.cpp` (`4409088`), der Messteil von `rechneEineZunge` wortgleich:
 * IC mass im Kern, hier misst `InstrumentMeasure` am Ton (IK2-2 a) — die Tonhoehe mit YIN (B2, 30 … 3000 Hz) als Median der
 * stimmhaften Rahmen von der Haelfte des Haltens bis zu seinem Ende (mindestens 0,1 s, mehr als die Haelfte stimmhaft),
 * das Einschwingen (B5) ueber das Halten; die Spitze der Auslenkung gibt der Kern (`reedTip`). Gemessen wird die erste
 * Zunge allein, wie IC (ein Chor, vor Gehaeuse und Bezug).
 *
 * Ohne Qt.
 */
#pragma once

#include "InstrumentModels/zunge/Zunge.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace lumi::messung
{

/// Was IC `ZungenMessung` am Ton einer Zunge mass.
struct ZungenTon
{
    double spielHz = 0.0;
    bool schwingt = false;      ///< mehr als die Haelfte der Rahmen stimmhaft — auch an stummen Toenen moeglich (S188)
    double amplitudeRel = 0.0;  ///< aus dem Kern (`reedTip`)
    double einschwingS = 0.0;
};

/// Misst den Ton einer Zunge (`rechneEineZunge`, ohne Bezug) wie IC: `haltenS` und `rate` aus dem Anschlag.
[[nodiscard]] ZungenTon messeZungenTon(std::span<const double> ton, double haltenS, int rate, double amplitudeRel);

/// Eine Zunge mit f₀ und Zielfrequenz rechnen und messen; mit `ton` der Ton dazu.
[[nodiscard]] ZungenTon messeEineZunge(const kern::zunge::ZungenSpiel& spiel, double f0, double hz, const kern::zunge::ZungenAnschlag& a,
                                       std::int64_t frames, std::vector<double>* ton = nullptr);

/// Die erste Zunge des Tons wie IC `rechneZungenTon(…, &messung)`: der erste Chor der Seite (Bass unter `bassAb`).
[[nodiscard]] ZungenTon messeErsteZunge(const kern::zunge::ZungenSpiel& spiel, const kern::zunge::ZungenAnschlag& a, std::int64_t frames);

}  // namespace lumi::messung
