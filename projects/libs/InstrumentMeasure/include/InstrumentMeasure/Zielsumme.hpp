/**
 ****************************************************************************************
 * @file   Zielsumme.hpp
 * @brief  7.5.9 (SM4 b): das Modell gegen Ziel und Toleranz von *Salamander* — je Taste und Groesse das Verhaeltnis
 *         der Abweichung zur Toleranz, die Summe der Quadrate als Ziel der Suche (docs/audio/Spielweisen_Konzept.md
 *         §4.1; Entscheide Patrik S156/S161: ZF1 a, ZF2 a, ZF4 a, ZF5 a, ZF7 a, SG1 a, UG1 a)
 *
 * @details
 * Reines C++ ohne Qt. Je Taste und Groesse aus `werte/salamander_ziel.txt` (`liesZiele`): Ziel z, Toleranz τ, Skala.
 * Der Modellwert m ist der Median ueber die Ebenen des Modells am Kanalmittel (`zielAus`, wie das Ziel selbst). Die
 * Abweichung d in der Skala der Groesse (`abstandIn`: Faktor |ln m/z|, dB und Cent |m − z|), geteilt durch die
 * Toleranz, nie kleiner als die Messgenauigkeit (UG1 a, `untergrenze`): r = d / max(τ, τ_min). Getroffen bei r ≤ 1
 * (ZF1 a); das Ziel der Suche ist Σ r² ueber alle Groessen aller Tasten (ZF5 a). Eine Groesse mit Ziel, die das
 * Modell nicht hergibt (etwa ein Teilton, der vor dem Bereich der Struktur verklingt), zaehlt mit r = 10
 * (`kFehltVerhaeltnis`, Lesart Claude S161 wie RV1 a); Groessen ohne Ziel zaehlen nicht (ZF7 a).
 ****************************************************************************************
 */

#pragma once

#include "InstrumentMeasure/Fluegelmessung.hpp"
#include "InstrumentMeasure/Strukturmessung.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace lumi::messung
{

/// Die Untergrenze der Toleranz (UG1 a): die Messgenauigkeit an synthetischen Signalen (Spielweisen_Konzept 31d-1:
/// Abklingzeiten auf 3 %, Pegel auf 0,5 dB, Schwebung auf 2 %).
inline const double kUntergrenzeFaktor = std::log(1.03);
inline constexpr double kUntergrenzeDb = 0.5;
inline constexpr double kUntergrenzeCentAnteil = 0.02;

/// Das Verhaeltnis einer Groesse mit Ziel, die das Modell nicht hergibt (Lesart Claude S161).
inline constexpr double kFehltVerhaeltnis = 10.0;

/// Die kleinste Toleranz in der Skala einer Groesse (UG1 a); bei Cent ein Anteil am Ziel.
double untergrenze(Skala skala, double ziel);

/// Eine Groesse an einer Taste: Modellwert, Ziel, wirksame Toleranz (mit Untergrenze) und ihr Verhaeltnis.
struct Abweichung
{
    std::string groesse;
    Skala skala = Skala::Dezibel;
    double modell = 0.0;
    double ziel = 0.0;
    double toleranz = 0.0;
    double verhaeltnis = 0.0;
    /// Das Verhaeltnis mit Vorzeichen (7.5.10): (m − z) / τ, bei Faktor ln(m/z) / τ — das Residuum der Suche; |residuum|
    /// = verhaeltnis. Fehlt der Modellwert, `kFehltVerhaeltnis`.
    double residuum = 0.0;
    bool getroffen = false;
    bool fehlt = false;
};

/// Eine Taste: ihre Groessen (in der Reihenfolge des Ziels), Σ r², wie viele getroffen und wie viele fehlen.
struct TastenSumme
{
    int midi = 0;
    std::vector<Abweichung> groessen;
    double summe = 0.0;
    int getroffen = 0;
    int fehlend = 0;
    bool fertig = false;
};

/// Vergleicht die Groessen des Modells an einer Taste (`modell`: je Groesse die Werte je Ebene) mit ihrem Ziel.
TastenSumme vergleiche(int midi, const TastenZiele& ziel, const std::vector<GroesseJeEbene>& modell);

/// Die Groessen des Modells an einer Taste (ZF2 a mit HU2 a): an der Struktur des Ziels je Teilton 1–3
/// (`strukturGroessen`) und der ganze Ton bis −20/−30/−40 dB. `ebenen[e]` und `messungen[e]` sind dieselbe Ebene
/// des Modells (aus `messeTon` mit `huellen`); der ganze Ton steht dort in beiden Kanaelen gleich (Kanalmittel).
/// Anders als `huellenGroessen` zaehlt jede uebergebene Ebene — das Modell rechnet nur die Ebenen der Suche (SG1 a).
std::vector<GroesseJeEbene> modellGroessen(const std::vector<TonHuellen>& ebenen,
                                                  const std::vector<std::array<KanalMessung, 2>>& messungen, const TonStruktur& struktur);

/// Die Residuen aller Tasten hintereinander, je Taste in der Reihenfolge ihrer Groessen (die des Ziels) — der Vektor,
/// dessen Quadratsumme Levenberg–Marquardt senkt (7.5.10). Die Zeilen einer Taste liegen beisammen.
std::vector<double> residuen(const std::vector<TastenSumme>& tasten);

/// Ueber alle Tasten: Σ r², getroffene und alle Groessen, fehlende, fertige Tasten und die Ausreisser (eine Taste ueber
/// dem Dreifachen des Mittels der Tastensummen, SP2 a).
struct Gesamt
{
    double summe = 0.0;
    int groessen = 0;
    int getroffen = 0;
    int fehlend = 0;
    int tasten = 0;
    int fertig = 0;
    std::vector<int> ausreisser;
};

Gesamt gesamt(const std::vector<TastenSumme>& tasten);

/// Der Bericht: je Taste eine Zeile (Σ r², getroffen / alle, fehlend, die drei groessten r mit Groesse), dann die
/// Gesamtzeile; mit `einzeln` dazu je Taste jede Groesse (Modell, Ziel, Toleranz, r).
std::string berichte(const std::vector<TastenSumme>& tasten, bool einzeln = false);

}  // namespace lumi::messung
