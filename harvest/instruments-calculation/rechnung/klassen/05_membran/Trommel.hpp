/**
 * @file Trommel.hpp
 * @brief Klasse 5 Membran: ein Schlag auf eine Trommel — Felle modal mit Luftlast und Kessel (`gemeinsam/Membran`),
 *        Schlaegel bzw. Stock mit Hertz-Kontakt (`stosse`), Tonhoehengleiten aus der Energie, Teppich der Snare als
 *        Mischform, Spielweisen am Schritt, Wirbel ueber die Dauer (`klassen/Klasse 5 - Membran/modell-entwurf.md`;
 *        Entscheide Patrik 2026-10-29: F-1 … F-8 a ueber FRAGEN_KONSOLIDIERT T6, T7, T8)
 *
 * **Schlag:** der Kopf (Masse, Kontakt K·δ^α — Bilbao et al.: 0,028 kg, K = 1,6·10⁸, α = 2,54, nur Bericht) trifft
 * das Schlagfell an der Schlagstelle mit der Auftreffgeschwindigkeit der Staerke (F-2 a: gestuft im Namen, der Rest
 * Pegel); Kontakt am Modell (Dahl 2011: „5–8 ms for a mezzoforte stroke“, nachgelesen PDF-S. 2).
 *
 * **Tonhoehengleiten** (Marogna, Avanzini & Bank 2010 Gl. 20, nachgelesen): T_qs = E·h/(2πR²(1 − ν²)T₀)·E(t), die
 * Energie E(t) des Fells alle 32 Takte; jede Mode ×√(1 + T_qs/T₀).
 *
 * **Teppich** (F-6 a, Mischform — vereinfacht): ein Rauschen, getastet von der Bewegung des Resonanzfells in der Mitte
 * (die Draehte schlagen an, wenn das Fell sich entfernt), Band 2–8 kHz; **ohne** Drahtmoden (offen). **Hohe Frequenzen**
 * ueber `bisHz`: ein abklingendes Rauschen (statistischer Teil, KB1 a), Pegel mit der Kraft (offen).
 *
 * **Ausgang:** der Schall unter 45°: je Mode Beschleunigung × Fernfeld-Staerke × (−i)^m.
 */
#pragma once

#include "gemeinsam/Membran.hpp"
#include "gemeinsam/Modal.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ic::trommel
{

enum class Schlag
{
    Normal,     ///< an der Schlagstelle der Art
    Mitte,      ///< in der Mitte (nur m = 0; an der Pauke ein tonloser Schlag — Packer, nur Inhalt)
    Rand,       ///< nahe am Rand (r = 0,85·a)
    Randschlag, ///< Rimshot: Fell und Reifen zugleich (Klick des Reifens offen)
    Reifen,     ///< Cross-Stick: nur der Reifen (Klick), das Fell schwach
    Gedaempft,  ///< die Hand liegt auf: Daempfung ×6 (offen)
};

struct TrommelSpiel
{
    std::string name;
    int note = 36;                    ///< die Rolle im Kit (feste Note); Pauke: tiefste Note des Umfangs
    int noteMax = 36;                 ///< Pauke: hoechste Note (Pedal); sonst = note
    bool stimmbar = false;            ///< Pauke (F-5 a): die Note waehlt die Spannung, (1,1) auf der Note
    std::vector<Fell> felle;          ///< Schlagfell, (Resonanzfell)
    Kessel kessel;
    double schlagstelleRel = 0.4;     ///< r/a (Pauke ¼ Radius vom Rand: 0,75 — nur Bericht)
    Stossanreger schlaegel;
    double tempoPp = 1.0, tempoFf = 8.0;  ///< m/s (Dahl: Akzent ≈ doppelte Geschwindigkeit; Torin 5 m/s; offen)
    double bisHz = 4000.0;
    // --- Gleiten (Marogna et al. 2010): E·h, ν des Fells ---
    double youngH = 3.5e9 * 0.2e-3;  ///< E·h in N/m (Avanzini & Marogna S. 898: 3,5 GPa, 0,2 mm — nur Bericht)
    double nu = 0.2;
    // --- Teppich (Snare) ---
    bool teppich = false;
    // die Rauschanteile relativ zum normierten Ausgang (ein ff-Schlag hat die Spitze 0,5):
    double teppichPegel = 0.0;       ///< je Schnelle des Resonanzfells / Auftreffgeschwindigkeit (offen)
    double hochPegel = 0.02;         ///< statistischer Teil ueber `bisHz`, je 100 N Kraft (offen)
    double klickPegel = 0.3;         ///< Reifen (Rimshot, Cross-Stick) bei ff — halbphysikalisch (offen)
    // --- Wirbel ---
    double wirbelHz = 14.0;          ///< Schlaege je s (offen)
    int prellen = 1;                 ///< Aufschlaege je Schlag (Snare-Buzz 4–9, Miura — nur Bericht)
    double ausgangBezug = 1.0;
    std::string kennzeichen = "ungemessen";
    /// die gerechneten Moden (bei der Spannung der Vorgabe); geteilt, damit Kopien billig sind
    std::shared_ptr<const std::vector<FellMode>> moden;
    double referenzHz = 0.0;         ///< Pauke: (1,1) bei der Spannung der Vorgabe
};

struct TrommelAnschlag
{
    double staerke = 0.5;     ///< 0 … 1 (Geschwindigkeit geometrisch zwischen pp und ff)
    Schlag schlag = Schlag::Normal;
    int note = 0;             ///< Pauke: die Note; sonst egal
    double glissandoCent = 0.0;  ///< Pauke: Pedal ueber die Dauer (F-5 a, Pitch-Spur)
    double wirbelS = 0.0;     ///< > 0: Wirbel ueber diese Dauer (F-4 a)
    bool ohneTeppich = false; ///< Snare: Teppich ab
    int rate = 48000;
};

struct TrommelMessung
{
    double kontaktS = 0.0;
    double kraftSpitzeN = 0.0;
    double gleitenCent = 0.0;  ///< die Tonhoehe der tiefsten Mode 10 ms nach dem Schlag gegen 1 s danach
};

/// Die Moden einmal rechnen (an der Spannung der Vorgabe).
void bereite(TrommelSpiel& s);
void rechneTrommelTon(const TrommelSpiel& s, const TrommelAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                      TrommelMessung* messung = nullptr);

[[nodiscard]] TrommelSpiel baueVorgabeTom();
[[nodiscard]] TrommelSpiel baueVorgabePauke();
[[nodiscard]] TrommelSpiel baueVorgabeBassdrum();
[[nodiscard]] TrommelSpiel baueVorgabeSnare();
[[nodiscard]] TrommelSpiel baueVorgabeTabla();
[[nodiscard]] TrommelSpiel baueVorgabeConga();
[[nodiscard]] TrommelSpiel baueVorgabeBongo();

}  // namespace ic::trommel
