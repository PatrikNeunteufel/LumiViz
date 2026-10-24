/**
 * @file Boden.hpp
 * @brief Der Resonanzboden im Klang: die Antwort je Taste und Hoerort als Filter und die Faltung
 *        eines Tons damit (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S149/S150:
 *        AW1 a, FT1 a, PG1 a, FF1 a; Stufe 31c-7)
 *
 * Die Antwort entsteht im Spektrum aus den gerechneten und den gezogenen Moden des Koerpers
 * (`InstrumentBody/Antwort.hpp`, ST1 a) und wird ueber `Fft` zur Zeitreihe: je Abtastwert der Druck
 * in Pa je Newton am Steg. Ein Ton der Hammer-Saite (Kraft in Einheiten von `kHammerKraftBezug`)
 * gefaltet damit gibt den Schalldruck am Hoerort in Pa. Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "InstrumentBody/Koerper.hpp"
#include "loop/Fft.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace lumi::loop
{

/// Alle Moden eines Koerpers: die gerechneten und die gezogenen des statistischen Teils (ST1 a).
[[nodiscard]] std::vector<koerper::ModeImKoerper> alleModen(const koerper::Koerper& koerper);

/// Der Platz des Stegpunkts der Taste im Koerper; −1 = die Taste hat keinen.
[[nodiscard]] int stegpunktVon(const koerper::Koerper& koerper, int taste);

/// Die Antwort am Hoerort `hoerort` auf eine Kraft am Stegpunkt `stegpunkt`, je Abtastwert in Pa/N,
/// `laengeS` lang (AW1 a: `antwortLaengeS`). Gebildet im Spektrum auf der naechsten Zweierpotenz.
[[nodiscard]] std::vector<double> bodenAntwort(const std::vector<koerper::ModeImKoerper>& moden, std::size_t stegpunkt,
                                               std::size_t hoerort, int rate, double laengeS, const Fft& umsetzung = fft());

/// Der Schalldruck in Pa: die Kraft eines Tons (in Einheiten von `kHammerKraftBezug` N) gefaltet mit
/// der Antwort (Pa/N). Laenge Ton plus Antwort weniger eins (FT1 a).
[[nodiscard]] std::vector<double> schalldruck(std::span<const float> kraft, std::span<const double> antwort,
                                              const Fft& umsetzung = fft());

}  // namespace lumi::loop
