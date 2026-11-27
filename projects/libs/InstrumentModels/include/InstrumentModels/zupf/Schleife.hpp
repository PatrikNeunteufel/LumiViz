/**
 * @file Schleife.hpp
 * @brief InstrumentModels, Kern `pluckedString` Rechenfassung 2: die Saitenschleife einer Note an einem Stegpunkt
 *        (Instrumente_IC_Konzept.md GZ5 a, GZ5-1 a, ZU1 a, UL3 a; Schnittfolge 34k-7d-4 (a); Herleitung
 *        `docs/modellierung/gezupfte-saite/2026-11-18_S205_herleitung.md`)
 *
 * Was der Bezug des Pruefstands (`proben/2026-11-26_S222_bezug_zupf.py`, `werte/zupf_pruefstand.txt`) geschlossen je
 * Frequenz rechnet, hier im Takt (48 kHz) wie der Kandidat S200 (`proben/2026-11-16_S200_zeitbereich_kandidat.py`):
 *
 * - **Quelle** (ZU1 a): die Welle, die der Zupf am Steg einspeist, je Saite genau aus der Frequenzantwort und mit einer FFT
 *   in den Takt gebracht (`quelleImTakt`, wie `quelle` des Bezugs): Kraftstufe F0 = T d L / (x_p (L − x_p)), Fingerbreite
 *   als Gauss-Tiefpass, Neigung je Stufe (GZ6 a), bei der Harfe die Geschwindigkeit beim Abloesen als verteilter
 *   Kraftstoss ueber das Dreieck; der Winkel teilt sie auf die zwei Polarisationen (senkrecht cos, parallel sin). Gerechnet
 *   auf der Geraden ω − i s₀ (s₀ = 2 1/s) wie der Pruefstand, der Ton beginnt nach `vorlauf` (was vor dem Loslassen
 *   liegt — Vorschwinger der Bandgrenze und des Tiefpasses — faellt dorthin).
 * - **Umlauf** Steg → Sattel → Steg: je Saite und Polarisation das Umlauf-Filter (`umlauf::Umlauffilter`, UL3 a; aus der
 *   Tafel des Kerns, das Glied am Grundton gestimmt), mit dem ganzzahligen Verzug N ≥ 1 als Ringpuffer:
 *   v⁺[n] = g[n] − (H ∗ (u − v⁺))[n].
 * - **Steg** (GZ5 a): die Moden der Koerperdatei (`Steg::moden`, der Formwert des Stegpunkts senkrecht φ cos α, parallel
 *   φ sin α), „genau fuer stueckweise lineare Kraft“ diskretisiert (`moden_diskret` „foh“ des Kandidaten:
 *   z[n] = pol z[n−1] + B1 (e F)[n−1] + B0 (e F)[n], u = Re Σ e z), dazu der **Daempfer** ueber 5 kHz (GZ5-1 a) als sieben
 *   breite Moden (Q 1,5; Mitten 6 … 23 kHz) nur senkrecht, die Gewichte φ² ≥ 0 per NNLS an Re Y = n(f) φ_rms² / 4
 *   angepasst (`daempferAn`, wie `daempfer` des Bezugs). Der **Durchgriff** B0 macht den Steg in einem Schritt loesbar:
 *   (I + M Z) u = u_frei + 2 M Z v⁺ mit M = Σ Re B0 e eᵀ und Z = diag(√(T m)) je Saite und Polarisation, dann
 *   F = Z (2 v⁺ − u).
 * - **Ausgang**: die Stegkraft je Saite und Polarisation; die Summe senkrecht zur Decke gibt der Kern aus (GZ5 a).
 *
 * Mehrere Saiten eines Chors haengen am selben Stegpunkt und koppeln ueber ihn. Mitschwingen ueber mehrere Stegpunkte
 * kommt mit 7d-5.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Steg.hpp"
#include "InstrumentModels/gemeinsam/Umlauf.hpp"

#include <array>
#include <complex>
#include <cstddef>
#include <vector>

namespace lumi::kern::zupf
{

/// Der Takt der Schleife.
inline constexpr double kSchleifenRate = 48000.0;
/// Die Gerade ω − i s₀ der Quelle (wie der Pruefstand), 1/s.
inline constexpr double kQuelleS0 = 2.0;
/// Die Mitten der breiten Moden des Daempfers (GZ5-1 a), Hz, und ihr Verlustfaktor (Q 1,5).
inline constexpr std::array<double, 7> kDaempferMitten = {6000.0, 7500.0, 9500.0, 12000.0, 15000.0, 19000.0, 23000.0};
inline constexpr double kDaempferVerlust = 1.0 / 1.5;

/// Der Zupf einer Note (an allen Saiten eines Chors derselbe).
struct Zupfangabe
{
    double ort = 0.16;          ///< x_p, m vom Steg
    double auslenkung = 0.004;  ///< d, m
    double winkelGrad = 30.0;   ///< gegen die Senkrechte der Decke
    double breite = 0.0075;     ///< Fingerbreite, m (Gauss-Tiefpass)
    double tempo = 0.0;         ///< Geschwindigkeit beim Abloesen, m/s (Harfe; 0 = keine)
    double neigung = 0.0;       ///< Stufe in dB (≤ 0) fuer die Neigung je Oktave (GZ6 a); 0 = keine
};

/// Eine Saite in der Schleife.
struct SchleifenSaite
{
    umlauf::Saitenwerte werte;
    double laenge = 0.0;            ///< schwingende Laenge, m
    umlauf::Umlauffilter filter;    ///< mit gestimmtem Glied (N ≥ 1)
};

/// Der Daempfer an einem Stegpunkt: φ_rms senkrecht und die Gewichte φ² je Mitte.
struct Daempfer
{
    double streuung = 0.0;
    std::array<double, kDaempferMitten.size()> gewichte{};
};

/// GZ5-1 a: die Gewichte des Daempfers aus der Dichte des statistischen Teils und φ_rms senkrecht (wie `daempfer` des
/// Bezugs: Raster 0,5–4,5 kHz und 5,25–20 kHz in 250 Hz, unter `vonHz` Ziel 0 mit Gewicht 3). Ohne Dichte alle 0.
[[nodiscard]] Daempfer daempferAn(const Steg::Dichte& dichte, double streuung);

/// Die Wellenzahl k und die Impedanz z₀ = T k / ω der Saite bei komplexer Kreisfrequenz (wie `saite_wellen` des
/// Pruefstands; `umlauf::wellenzahl` ist der Fall reeller ω).
struct Welle
{
    std::complex<double> k, z0;
};
[[nodiscard]] Welle welleBei(const umlauf::Saitenwerte& saite, std::complex<double> omega);

/// Die Quelle g einer Saite am Steg im Takt (ohne Aufteilung auf die Polarisationen), `frames` Werte, der Ton ab
/// `vorlauf` Sekunden (siehe Kopf).
[[nodiscard]] std::vector<double> quelleImTakt(const umlauf::Saitenwerte& saite, double laenge, const Zupfangabe& zupf,
                                               std::size_t frames, double vorlauf);

/// Was die Schleife rechnet.
struct Schleifenauftrag
{
    std::vector<SchleifenSaite> saiten;  ///< am selben Stegpunkt (ein Chor: zusammen gezupft)
    Zupfangabe zupf;
    const Steg* steg = nullptr;          ///< die Moden und die Dichte
    const Steg::Punkt* punkt = nullptr;  ///< der Stegpunkt der Saiten
    std::size_t frames = 0;
    double vorlauf = 0.0;                ///< s
};

/// Die Stegkraft, je Frame.
struct Schleifenkraft
{
    std::vector<double> senkrecht;  ///< Summe ueber die Saiten, senkrecht zur Decke (der Ausgang des Kerns)
    std::vector<double> parallel;   ///< Summe ueber die Saiten, parallel zur Decke
    Daempfer daempfer;
};

/// Rechnet die Schleife (siehe Kopf). Wirft `std::invalid_argument`, wenn Saiten, Steg oder Punkt fehlen, der Punkt
/// nicht so viele Formwerte wie der Steg Moden traegt oder ein Filter N < 1 hat.
[[nodiscard]] Schleifenkraft rechneSchleife(const Schleifenauftrag& auftrag);

}  // namespace lumi::kern::zupf
