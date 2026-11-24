/**
 * @file UmlaufEntwurf.hpp
 * @brief Der Entwurf des Umlauf-Filters der gezupften Saite: die Tafel je Taste (Harfe, Cembalo) und je Saite gebunden
 *        (Gitarre, Laute, Oud) — Instrumente_IC_Konzept.md UL1 a, UL2 a, UL3 a, UL3-1 a (mit der Nachschaerfung S205),
 *        ZK1 a; Herleitung `docs/modellierung/gezupfte-saite/2026-11-18_S205_herleitung.md` §9; Teilplan 34k-7c
 *
 * Uebertragen aus den Proben S204 / S205 (`proben/2026-11-17_S204_umlauf_umfang.py`, `…_dispersion_biquads.py`,
 * `2026-11-18_S205_gebunden_stuetzstellen.py`, `…_tafel_je_taste.py`) und den Rechnungen der Research-Werkbank, auf
 * denen sie aufbauen (`recherche/2026-11-16_S202_umlauf_filter_grundlagen_werkbank/umlauf_entwurf.py`,
 * `recherche/2026-11-17_S203_umlauf_filter_nacharbeit_werkbank/bank_voll.py`, `weg2.py`):
 *
 * - **Ziel** auf dem Raster 1 … 20 000 Hz in 0,5 Hz bei 48 kHz: Betrag und Verzug von exp(−2 i k L)
 *   (`kern::umlauf::umlaufDerSaite`); Teiltoene, wo der Verzug 2πn ist, Abklingrate σ = −ln|E| / τ_g.
 * - **Zaehlend** (UL1 a): σ ≤ 100 1/s; angepasst wird bis zur 1,5-fachen Grenze (die Zugabe nur bis 19,5 kHz).
 * - **Verlust** (UL3 a): Vorgabe nach Bank & Vaelimaeki 2003 ohne Glaettung und ohne Hochfrequenz-Vorgabe (unter f₁ der
 *   Wert bei f₁, zwischen den Sollpunkten pchip, darueber der letzte Wert), Mindestphase ueber das reelle Cepstrum
 *   (65 536 Werte); Gewicht 1 / (g ln g)²; Nenner aus dem Gleichungsfehler (instabile Pole gespiegelt), Zaehler ueber
 *   |B|² mit der Nebenbedingung |H| ≤ 1 auf 4097 Stellen (LSI → LDP → NNLS nach Lawson & Hanson 1974), dazu im
 *   Austausch jede Stelle, an der die Loesung sie auf 65 537 Stellen verletzt; dann der Spektralfaktor (eine doppelte
 *   Nullstelle am Einheitskreis wird zu einer zusammengelegt); Ordnung 8, sonst 4 oder 2.
 * - **Kette** (UL3 a): Start nach Abel & Smith an der Restphase (β 0,85), danach kleinste Quadrate wie scipy
 *   `least_squares(method="trf", x_scale="jac")` ueber logit(r / 0,99) und logit(θ / π), Fehler je Teilton in Cent, der
 *   Grundton zehnfach.
 * - **Glied** (UL3-1 a): Thiran 5 (je Taste der Reihe nach 5, 3, 1 — das erste, das die Schwelle haelt), in der
 *   zweiten Stufe der Anpassung mit seiner genauen Phase; ausgewertet am Grundton nachgestimmt.
 * - **Schwelle** (UL2 a): Lage 3 Cent je zaehlendem Teilton, Abklingrate −30,1 … +26,2 %.
 *
 * Abweichungen von der Vorlage: die SVD ist eine eigene (einseitiges Jacobi-Verfahren), die zufaelligen Starts kommen
 * aus einem eigenen Generator, und die Nebenbedingung |H| ≤ 1 gilt auch zwischen den 4097 Stellen (Austausch; in der
 * Vorlage haengt der Spektralfaktor dort an Rundungen, S220 Harfe T104). Laeuft ein Logit-Parameter der Kette an seine
 * Grenze (θ = π, eine Spalte von J exakt null), nimmt der Schritt im Vertrauensbereich bei Rangverlust α ≥ 0 (den Schritt
 * kleinster Norm), und die Logit-Parameter werden auf ±40 gehalten, wo sigmoid ohnehin klemmt (TK2 a; scipy rechnet dort
 * mit α < 0, und erst Rundungen von LAPACK halten den Abbruch nach xtol offen — S220 Cembalo T73). Die Zahlen sind daher
 * nicht Bit fuer Bit die der Proben, sie werden an denselben Schwellen gemessen.
 *
 * Ohne Qt.
 */
#pragma once

#include "InstrumentModels/gemeinsam/Umlauf.hpp"

#include <limits>
#include <optional>
#include <vector>

namespace lumi::messung::umlauf
{

using kern::umlauf::Biquad;
using kern::umlauf::Saitenwerte;
using kern::umlauf::Umlauffilter;

inline constexpr double kRate = 48000.0;
/// UL1 a: zaehlend ist ein Teilton bis zu dieser Abklingrate am starren Steg, 1/s.
inline constexpr double kGrenzeAbklingen = 100.0;
/// UL3-1 a nachgeschaerft (a): angepasst wird bis zu diesem Vielfachen der Grenze.
inline constexpr double kZugabe = 1.5;
inline constexpr double kRadiusMax = 0.99;
/// UL2 a.
inline constexpr double kCentSchwelle = 3.0;
inline constexpr double kAbklingenUnten = 100.0 * (1.0 / 1.4309 - 1.0);  ///< −30,1 %
inline constexpr double kAbklingenOben = 100.0 * (1.0 / 0.7923 - 1.0);   ///< +26,2 %

/// Wie gut ein Filter den Umlauf trifft, ueber die zaehlenden Teiltoene.
struct Bewertung
{
    bool auswertbar = false;
    double centGroesster = std::numeric_limits<double>::quiet_NaN();
    double abklingenGroesster = std::numeric_limits<double>::quiet_NaN();  ///< |%|
    double grundtonCent = std::numeric_limits<double>::quiet_NaN();
    int zaehlend = 0;
    int inSchwelle = 0;  ///< zaehlende Teiltoene mit dem Abklingen in der Schwelle

    [[nodiscard]] bool abklingenHaelt() const { return auswertbar && inSchwelle == zaehlend; }
    [[nodiscard]] bool haelt() const { return abklingenHaelt() && centGroesster <= kCentSchwelle; }
};

/// Misst ein Filter am Umlauf der Saite der Laenge `laenge` (wie die Vorlage `werte`: das Glied am Grundton nachgestimmt
/// — `filter` wird dafuer gestimmt).
[[nodiscard]] Bewertung bewerte(const Saitenwerte& saite, double laenge, Umlauffilter& filter);

/// Die Tafel einer Taste (Harfe, Cembalo).
struct TafelTaste
{
    Umlauffilter filter;
    Bewertung bewertung;
    double grundHz = 0.0;
    int zaehlendMitZugabe = 0;
    int abelSmith = 0;  ///< so viele Biquads platziert Abel & Smith
};

/// Entwirft das Filter einer Taste (`tafel_je_taste.py`): Glieder Thiran 5, 3, 1, Biquads der Reihe nach, bis UL2 a
/// haelt; sonst das beste. `std::nullopt`, wenn die Saite zu wenige Teiltoene traegt oder kein Glied in den Umlauf passt.
[[nodiscard]] std::optional<TafelTaste> entwirfJeTaste(const Saitenwerte& saite, double laenge);

/// Die Tafel einer Saite, gebunden ueber die Lage (Gitarre, Laute, Oud; UL3-1 a).
struct TafelSaite
{
    int buende = 0;
    double mensur = 0.0;
    int grad = 3;
    /// je Biquad die Beiwerte von logit(r / kRadiusMax) und logit(θ / π) als Legendre-Reihe in 2 u / buende − 1
    std::vector<std::vector<double>> radiusReihe, winkelReihe;
    struct Verluststelle
    {
        double lage = 0.0;  ///< in Buenden
        std::vector<double> b, a;
    };
    std::vector<Verluststelle> verlust;  ///< an den Buenden und auf halbem Weg, aufsteigend
    double startFaktor = 0.0;  ///< der gewaehlte Start (Abel & Smith bis zu diesem Vielfachen der Grenze)
    double daempfung = 0.0;    ///< die gewaehlte Daempfung der Beiwerte ab Grad 1
    /// groesster Fehler je Gruppe von Stellen: an den Buenden, auf halbem Weg, an Achteln, an ungeraden Sechzehnteln
    /// (die weder Anpassung noch Auswahl kennt)
    Bewertung buendeBewertung, halbBewertung, achtelBewertung, fremdBewertung;

    /// Die Kette an der Lage `lage` (Buende, gebrochen).
    [[nodiscard]] std::vector<Biquad> ketteAn(double lage) const;
    /// Der Verlust an der Lage, zwischen den Nachbarstellen gemittelt.
    void verlustAn(double lage, std::vector<double>& b, std::vector<double>& a) const;
};

/// Entwirft die Tafel einer Saite (`gebunden_stuetzstellen.py` mit `glied=T5 grad=3 rand=1.5 verlust=halb starts=4`).
[[nodiscard]] std::optional<TafelSaite> entwirfJeSaite(const Saitenwerte& saite, double mensur, int buende);

/// Das Filter einer Saite an der Lage `lage`: Kette und Verlust aus der Tafel, das Glied am Grundton gestimmt; misst es
/// zugleich (`bewertung`, wenn nicht `nullptr`).
[[nodiscard]] std::optional<Umlauffilter> filterAn(const TafelSaite& tafel, const Saitenwerte& saite, double lage,
                                                   Bewertung* bewertung = nullptr);

}  // namespace lumi::messung::umlauf
