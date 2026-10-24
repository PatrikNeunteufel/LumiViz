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
#include "loop/Rechenkern.hpp"

#include <QString>

#include <cstddef>
#include <memory>
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

// --- im Klang (Schritt 3: AW1 a, FT1 a, PG1 a, SB1 a) -----------------------------------

/// Ein Koerper, bereit fuer den Klang: einmal je Datei geladen und gemerkt.
struct BodenImKlang
{
    koerper::Koerper koerper;
    std::vector<koerper::ModeImKoerper> moden;  ///< gerechnet und gezogen
    double laengeS = 0.0;                       ///< `antwortLaengeS`
    QString abdruck;                            ///< SHA-256 der Datei (hex), Teil des Schluessels im Tonspeicher
    int links = 0;   ///< der Hoerort fuer links: `left`, sonst der erste (SB1 a)
    int rechts = 0;  ///< der Hoerort fuer rechts: `right`, sonst der zweite, bei einem der erste
};

/// Der Koerper zu einem Verweis `lib:<name>` (`asset/loop/instrument_bodies/<name>.lvinstbody`),
/// gemerkt fuer das ganze Programm; nullptr = nicht gefunden oder nicht lesbar (oder ohne Hoerort).
/// Von jedem Thread aus benutzbar.
[[nodiscard]] std::shared_ptr<const BodenImKlang> bodenVon(const QString& verweis);

/// Die Antwort einer Taste als Stereo-Klang (links und rechts je Abtastwert in Pa/N), aus dem
/// Tonspeicher unter Abdruck, Taste und Rate — oder neu gebildet und dort abgelegt (AW1 a).
/// nullptr = die Taste hat keinen Stegpunkt.
[[nodiscard]] std::shared_ptr<const Klang> bodenAntwortFuer(const BodenImKlang& boden, int taste, int rate);

/// Der Ton durch den Boden: die Kraft (Einheiten von `kHammerKraftBezug`) gefaltet mit der Antwort,
/// links und rechts, in Einheiten von `kSchalldruckBezug` (PG1 a), verschraenkt in `stereo`
/// (2 × (Ton + Antwort − 1) Werte).
void falteMitBoden(std::span<const float> kraft, const Klang& antwort, std::vector<float>& stereo,
                   const Fft& umsetzung = fft());

}  // namespace lumi::loop
