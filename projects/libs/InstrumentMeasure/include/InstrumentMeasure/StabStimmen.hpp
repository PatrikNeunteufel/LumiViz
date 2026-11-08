/**
 * @file StabStimmen.hpp
 * @brief Der Stimmer des Kerns `bar` (Klasse 4): der Pegel des Partners und die Fenster der Bogenkraft am Modell
 *        (Instrumente_IC_Konzept.md 34g-4, IK2-11 a, KS2 a, KS3 a)
 *
 * Uebernommen aus IC (`bb7e6d6`), die Rechnung wortgleich:
 * - **Pegel des Partners** aus `rechnung/klassen/04_platte_stab/Vorgaben.cpp` `setzePartnerAbstrahl`: an der Note des
 *   Ziels bei Stufe −6 der Effektivwert der ersten 0,5 s mit Partner gegen ohne, das Gewicht in 40 Halbierungen von
 *   10⁻⁶ bis 10⁶ (log), gerechnet mit dem Bezug 1 wie in IC (dort vor `normiere`). Ein Instrument ohne akustischen
 *   Partner bekommt 1 (ohne Wirkung).
 * - **Fenster des Bogens** aus `Stab.cpp` `stimmeBogen`: je dritte Note und je Stufe (v_B bei pp, mf, ff) die
 *   Kraefte im Raster 2^(1/8) von 10 mN bis 200 N, bei denen der Stab anschwingt (+20 dB), auf ±15 Cent an f_1 spielt
 *   (YIN, B2, im gehaltenen Teil — in IC im Kern gemessen, hier aus `StabMessung::gehalten`) und der Bogen in 5–98 %
 *   der Takte haftet; der laengste Lauf, innen um je einen Schritt gekuerzt. Gerechnet mit dem Bezug nach `normiere`
 *   wie in IC (dort nach `normiere`). Ohne Bogen 1 (ohne Wirkung).
 *
 * Ohne Qt.
 */
#pragma once

#include "InstrumentMeasure/Stimmen.hpp"
#include "InstrumentModels/stab/Stab.hpp"

#include <array>

namespace lumi::messung
{

/// Der Stimmer des Kerns `bar`.
[[nodiscard]] Stimmer stabStimmer();

/// Der Pegel des Partners eines Spiels nach seinem Ziel (siehe Kopf), mit Belegkraft *Rechnung*.
[[nodiscard]] kern::Kurve stabPartnerPegel(const kern::stab::StabSpiel& spiel);

/// Die Fenster der Bogenkraft (unten und oben je pp, mf, ff), mit Belegkraft *Rechnung*; das Spiel traegt seinen Bezug.
struct BogenFenster
{
    std::array<kern::Kurve, 3> unten, oben;
};
[[nodiscard]] BogenFenster stabBogenFenster(const kern::stab::StabSpiel& spiel);

/// Die Tonhoehe eines gestrichenen Tons im gehaltenen Teil (Median der stimmhaften Rahmen, YIN 30 … 4000 Hz; wie IC
/// `StabMessung::spielHz`); 0, wenn er nicht schwingt (weniger als die Haelfte stimmhaft, oder kein gehaltener Teil).
[[nodiscard]] double stabSpielHz(const kern::stab::StabMessung& messung, int rate);

}  // namespace lumi::messung
