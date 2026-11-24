/**
 * @file Steg.hpp
 * @brief InstrumentCore: der Koerper am Steg, wie ihn ein Kern in seiner Saitenschleife rechnet (Instrumente_IC_Konzept.md
 *        GZ5 a, GZ5-1 a, GZ7 a; Teilplan 34k-7b)
 *
 * Die App laedt den Koerper eines Instruments einmal (`InstrumentBody`, `.lvinstbody`) und reicht ihn als `Steg` mit dem
 * Tonauftrag (`Tonauftrag::steg`); der Kern bindet `InstrumentBody` nicht ein. Ein `Steg` traegt:
 *
 * - die Moden der Koerperdatei (Frequenz, Verlustfaktor η, Richtung am Steg) — gemeinsam fuer alle Stegpunkte, damit der
 *   Kern die Punkte ueber den Koerper koppeln kann (Mitschwingen, GZ3 a: Y[(i,a),(j,b)] = Σ φ_ni φ_nj d_na d_nb c_n);
 * - je Stegpunkt den Formwert jeder Mode laengs ihrer Richtung und die Streuung φ_rms des statistischen Teils;
 * - die Dichte des statistischen Teils n(f) — der Kern rechnet daraus den Daempfer ueber den Moden,
 *   Re Y = n(f) · φ_rms² / 4 (GZ5-1 a).
 *
 * Er traegt **alle** Stegpunkte des Koerpers; welche ein Ton braucht (die gespielte Saite, die freien Saiten, ein Chor an
 * mehreren Punkten), weiss der Kern aus seiner Tafel und waehlt sie ueber `punktZu`. Die App gibt fuer jeden Stegpunkt,
 * an dem der Kern eine Kraft ausgibt (`Stegkraft`, `Tonkern::rechneAmSteg`), die Faltung mit der Antwort des ganzen
 * Koerpers dazu (GZ5 a).
 *
 * Die Richtung je Mode: Winkel gegen die Senkrechte der Decke in Grad, in der Ebene quer zu den Saiten; 0 = senkrecht
 * (jeder Koerper aus einem Bauplan), ±90 = nur parallel zur Decke (KQ35-2 a). Der Formwert steht laengs dieser Richtung:
 * senkrecht φ · cos, parallel φ · sin — wie `bridge` / `bridgeAngle` der Koerperdatei.
 *
 * Die Teile stehen in `Steg` (`Steg::Mode`, `Steg::Punkt`, `Steg::Dichte`): `zupf::StegMode` (die Stegmoden der Vorlage
 * aus IC) und `koerper::Stegpunkt` (der Ort im Bauplan) tragen dieselben Namen.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include <string>
#include <vector>

namespace lumi::kern
{

/// Der Koerper am Steg.
struct Steg
{
    /// Eine Mode des Koerpers.
    struct Mode
    {
        double hz = 0.0;
        double verlust = 0.0;     ///< Verlustfaktor η
        double winkelGrad = 0.0;  ///< die Richtung, in der sie den Steg bewegt, gegen die Senkrechte der Decke

        [[nodiscard]] bool operator==(const Mode&) const = default;
    };

    /// Ein Stegpunkt: wo eine Saite (oder ein Chor) auf dem Steg liegt.
    struct Punkt
    {
        int taste = -1;            ///< die Taste des Stegpunkts im Koerper (MIDI, `stegTasten` der Koerperdatei)
        std::vector<double> form;  ///< je Mode der Formwert laengs ihrer Richtung, 1/√kg
        double streuung = 0.0;     ///< φ_rms der Formwerte im statistischen Teil, 1/√kg; 0 = keiner

        [[nodiscard]] bool operator==(const Punkt&) const = default;
    };

    /// Die Dichte des statistischen Teils ueber den Moden der Datei: n(f) = dichte + anstieg · (f − vonHz) fuer
    /// vonHz ≤ f < bisHz. `dichte` 0 = keiner.
    struct Dichte
    {
        double vonHz = 0.0;
        double bisHz = 0.0;
        double dichte = 0.0;   ///< Moden je Hz bei `vonHz`
        double anstieg = 0.0;  ///< Moden je Hz², ≥ 0

        [[nodiscard]] double bei(double f) const
        {
            return f < vonHz || f >= bisHz ? 0.0 : dichte + anstieg * (f - vonHz);
        }

        [[nodiscard]] bool operator==(const Dichte&) const = default;
    };

    std::vector<Mode> moden;     ///< aufsteigend nach Frequenz
    std::vector<Punkt> punkte;   ///< je Taste hoechstens einer
    Dichte dichte;

    /// Der Stegpunkt dieser Taste, oder `nullptr` (genau gesucht wie `stegpunktVon` der App, KQ22 a).
    [[nodiscard]] const Punkt* punktZu(int taste) const;

    [[nodiscard]] bool operator==(const Steg&) const = default;
};

/// Was an einem Steg nicht stimmt (leer = gut): eine Mode mit Frequenz ≤ 0, Verlust ≤ 0 oder Winkel ausserhalb ±90, die
/// Moden nicht aufsteigend, ein Stegpunkt mit Taste ausserhalb 0 … 127 oder doppelt, mit einer anderen Zahl Formwerte als
/// Moden, mit einem Wert, der keine Zahl ist, oder mit negativer Streuung; eine Dichte < 0 oder mit `bisHz` ≤ `vonHz`.
[[nodiscard]] std::vector<std::string> pruefeSteg(const Steg& steg);

}  // namespace lumi::kern
