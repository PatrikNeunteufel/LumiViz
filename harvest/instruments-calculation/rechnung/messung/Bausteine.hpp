/**
 * @file Bausteine.hpp
 * @brief Messkette: die neuen Bausteine fuer die Klassen ausser dem Fluegel (`gemeinsam/messkette.md` §3, B1–B8) —
 *        reines C++ gegen `gemeinsam/Fft`, geprueft an synthetischen Signalen mit bekannten Werten (AUFTRAG_BAU §5;
 *        Genauigkeit wie 31d-1: Zeiten 3 %, Pegel 0,5 dB, Schwebung 2 % — je Baustein im Test belegt)
 *
 * | Baustein | misst | Verfahren (hier gebaut) |
 * |---|---|---|
 * | B1 `verfolgeTeiltoene` | Frequenz und Amplitude je Teilton je Rahmen | Spitzen der Kurzzeit-FFT (Hann, parabolisch auf dB), Verfolgung nach McAulay & Quatieri (Fortsetzung zur naechsten Spitze im Fenster, sonst Tod) |
 * | B2 `tonhoehe` | f0(t), stimmhaft ja/nein | YIN (de Cheveigné & Kawahara 2002: kumulierte mittlere normierte Differenz, absolute Schwelle, parabolisch) |
 * | B3 `vibrato` | Rate (Hz), Tiefe (Cent), AM je Teilton und ihre Lage zur FM | Spektrum der Abweichung von f0(t) bzw. der Huelle im Suchbereich 3–10 Hz (Essentia: 4–8 Hz) |
 * | B4 `rest` | Pegel des Rests gegen das Periodische, in Baendern | Rest = Signal minus Resynthese aus B1 (SMS, Serra & Smith 1990) |
 * | B5 `einschwingen` | Dauer bis zum regelmaessigen Ton | Zeit vom Einsatz, bis der Effektivwert je Rahmen 3 dB unter dem gehaltenen Teil erreicht und f0 stimmhaft ist |
 * | B6 `modalfit` | Frequenz, Daempfung, Amplitude und Phase je Mode ohne Reihe | ESPRIT (reell, Kovarianz der Hankel-Matrix, Jacobi) nach Ege, Boutillon & David 2009 — ohne die Zerlegung in Baender und ohne ESTER (Ordnung fest; offen) |
 * | B7 `baenderUeberZeit` | Energie je Terzband je Rahmen, Schwerpunkt | Kurzzeit-FFT, Summe je Band |
 * | B8 `gleiten` | Frequenz der staerksten Mode ueber die ersten 100–300 ms | B1 auf der staerksten Spur |
 *
 * **Erkennung nur am Bezug** (L4, `gemeinsam/messkette.md` §3a): `amplitudenBei` misst ein Signal bei vorgegebenen
 * Frequenzen und Zeiten (der Struktur des Bezugs) ohne eigene Erkennung — so wird das Modell gemessen.
 *
 * Belegkraft der Verfahren: die Quellen stehen in `gemeinsam/messkette.md` als „nur Bericht“; gebaut wurde aus der
 * Beschreibung dort und aus dem Lehrbuch, geprueft wird an synthetischen Signalen.
 */
#pragma once

#include "gemeinsam/Fft.hpp"

#include <complex>
#include <span>
#include <vector>

namespace ic::messung
{

// --- B6 Modalfit ohne Reihe --------------------------------------------------------------------------------

struct ModalfitMode
{
    double hz = 0.0;
    double sigma = 0.0;      ///< 1/s (T60 = ln1000/σ)
    double amplitude = 0.0;  ///< zur Zeit `ab` (dem Anfang des Ausschnitts)
    double phase = 0.0;
};

/// Die Moden eines abklingenden Signals im Ausschnitt [ab, ab + laenge): `ordnung` reelle Moden (2·ordnung komplexe
/// Pole, hoechstens 16), Hankel-Zeilen `zeilen`. Nach Frequenz sortiert; Moden mit σ < 0 (wachsend) fallen weg.
[[nodiscard]] std::vector<ModalfitMode> modalfit(std::span<const double> signal, int rate, std::size_t ab, std::size_t laenge,
                                                 int ordnung, int zeilen = 96);

// --- B1 Teiltonverfolgung ----------------------------------------------------------------------------------

struct Spur
{
    std::size_t start = 0;            ///< erster Rahmen
    std::vector<double> hz;           ///< je Rahmen ab `start`
    std::vector<double> pegelDb;
};

struct Verfolgung
{
    double schrittS = 0.0;
    double fensterS = 0.0;
    std::vector<Spur> spuren;
};

/// Spitzen ueber `schwelleDb` (gegen die groesste des Rahmens), je Rahmen; eine Spur setzt sich fort, wenn eine Spitze
/// innerhalb `sprungCent` liegt, sonst endet sie; neue Spitzen beginnen eine Spur. Spuren unter `mindestRahmen` fallen
/// weg.
[[nodiscard]] Verfolgung verfolgeTeiltoene(std::span<const double> signal, int rate, const Fft& fft, double fensterS = 0.046,
                                           double schrittS = 0.01, double schwelleDb = -60.0, double sprungCent = 50.0,
                                           std::size_t mindestRahmen = 5);

// --- B2 Tonhoehe (YIN) -------------------------------------------------------------------------------------

struct Tonhoehe
{
    double schrittS = 0.0;
    std::vector<double> hz;       ///< 0 = nicht stimmhaft
    std::vector<double> guete;    ///< das Minimum der normierten Differenz (0 = periodisch)
};

[[nodiscard]] Tonhoehe tonhoehe(std::span<const double> signal, int rate, double minHz, double maxHz, double fensterS = 0.04,
                                double schrittS = 0.005, double schwelle = 0.15);

// --- B3 Vibrato --------------------------------------------------------------------------------------------

struct Vibrato
{
    bool erkannt = false;
    double rateHz = 0.0;
    double tiefeCent = 0.0;       ///< halbe Spitze-Spitze-Abweichung
    double amTiefeDb = 0.0;       ///< halbe Spitze-Spitze-Schwankung der Huelle
    double amPhaseGrad = 0.0;     ///< Lage der AM gegen die FM (0 = gleichphasig)
};

/// Aus f0(t) (B2) und der Huelle in dB je Rahmen (gleicher Schritt): die staerkste Periodizitaet in [vonHz, bisHz].
/// `fensterS`: das Fenster, ueber das f0 und Huelle gemittelt sind — die Tiefe wird um dessen Daempfung
/// sinc(π·Rate·Fenster) berichtigt (ein Fenster von 40 ms nimmt einem Vibrato von 5,5 Hz 8 % der Tiefe).
[[nodiscard]] Vibrato vibrato(const std::vector<double>& f0Hz, const std::vector<double>& huelleDb, double schrittS, double vonHz = 3.0,
                              double bisHz = 10.0, double fensterS = 0.0);

// --- B4 Rest -----------------------------------------------------------------------------------------------

struct Rest
{
    double periodischDb = 0.0;  ///< Effektivwert der Resynthese
    double restDb = 0.0;        ///< Effektivwert des Rests
    double abstandDb = 0.0;     ///< periodisch − Rest (wie HNR)
    std::vector<double> restSignal;
};

/// Der Rest nach Abzug der Resynthese aus der Verfolgung (Phasen durch Integration der Frequenz, Amplituden linear
/// zwischen den Rahmen; die Phase je Spur so gewaehlt, dass sie im ersten Rahmen zum Signal passt).
[[nodiscard]] Rest rest(std::span<const double> signal, int rate, const Verfolgung& v, const Fft& fft);

// --- B5 Einschwingen ---------------------------------------------------------------------------------------

/// Die Zeit vom Einsatz (1 % der Spitze) bis der Effektivwert je 10 ms auf 3 dB an den Median der zweiten Haelfte
/// heranreicht — fuer gehaltene Toene. In s.
[[nodiscard]] double einschwingen(std::span<const double> signal, int rate);

// --- B7 Baender ueber der Zeit -----------------------------------------------------------------------------

struct Baender
{
    double schrittS = 0.0;
    std::vector<double> mitten;               ///< Terzmitten (Hz)
    std::vector<std::vector<double>> pegelDb; ///< je Rahmen je Band
    std::vector<double> schwerpunktHz;        ///< je Rahmen
};

[[nodiscard]] Baender baenderUeberZeit(std::span<const double> signal, int rate, const Fft& fft, double fensterS = 0.02,
                                       double schrittS = 0.01);

// --- B8 Gleiten --------------------------------------------------------------------------------------------

/// Die Frequenz der staerksten Spur ueber die ersten `bisS` Sekunden (je Rahmen von B1).
[[nodiscard]] std::vector<double> gleiten(const Verfolgung& v, double bisS = 0.3);

// --- Messung an der Struktur des Bezugs (L4) ----------------------------------------------------------------

/// Die Amplitude (dB) des Signals bei `hz` in Fenstern der Laenge `fensterS` um die Zeiten `zeitenS` (Hann, DFT bei
/// genau dieser Frequenz) — ohne eigene Erkennung; stetig in den Parametern des Modells.
[[nodiscard]] std::vector<double> amplitudenBei(std::span<const double> signal, int rate, double hz, const std::vector<double>& zeitenS,
                                                double fensterS);

}  // namespace ic::messung
