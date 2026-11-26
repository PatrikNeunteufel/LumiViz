/**
 * @file Umlauf.hpp
 * @brief InstrumentModels, gemeinsam: der Umlauf der Saite im Takt — das Ziel (die Saite als Welle) und das Filter, das
 *        ihn nachbildet (Instrumente_IC_Konzept.md UL1 a–UL3-1 a, ZK1 a; Herleitung `docs/modellierung/gezupfte-saite/
 *        2026-11-18_S205_herleitung.md` §2, §9; Teilplan 34k-7c)
 *
 * **Das Ziel.** Eine Welle laeuft vom Steg zum Sattel und zurueck: E(ω) = exp(−2 i k L) mit der Wellenzahl der steifen,
 * gedaempften Saite (Pruefstand S199 `saite_wellen`, `saiten_eta`):
 *
 * - k² = (−T + √(T² + 4 EI m ω²)) / (2 EI), k ← k · (1 − i η / 2)
 * - η nach Valette (Woodhouse u. a. 2012 Gl. 4): T (η_F + η_A / ω) / (T + EI k²) + EI η_B k² / (T + EI k²);
 * - monofil (Durchmesser > 0): die Luft nach Stokes mit dem Zuschlag (d + 0,2 mm) und die Biegung (Woodhouse &
 *   Lynch-Aird 2019 Gl. 11, 12, 14, 15), die Frequenz der Luft mindestens 20 Hz.
 *
 * Betrag |E| = exp(2 L Im k) (Im k < 0), Phase (positiv, waechst) 2 L Re k.
 *
 * **Das Filter** (UL3 a, UL3-1 a): H(z) = z^(−N) · Glied(D) · Kette(z) · Verlust(z) — ganzzahliger Verzug N, ein
 * Thiran-Allpass fuer den gebrochenen Verzug D (Ordnung 5, D in [4,5 ; 5,5)), eine Kette von Biquad-Allpaessen
 *
 *   A(z) = (r² − 2 r cos θ z⁻¹ + z⁻²) / (1 − 2 r cos θ z⁻¹ + r² z⁻²)
 *
 * und ein Verlust b(z) / a(z) (a₀ = 1, |H| ≤ 1). Der Thiran-Allpass: a_k = (−1)^k C(N,k) Π_{n=0..N} (D − N + n) /
 * (D − N + k + n), der Zaehler ist der Nenner gespiegelt.
 *
 * Phase heisst hier ueberall der **Verzug** −arg H (positiv, laengs ω stetig); Frequenzen ω in rad je Takt.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include <complex>
#include <cstddef>
#include <numbers>
#include <optional>
#include <vector>

namespace lumi::kern::umlauf
{

/// Die Werte einer Saite (SI). Durchmesser > 0: monofil (Luft nach Stokes), sonst Valette mit η_F und η_A.
struct Saitenwerte
{
    double zug = 0.0;             ///< T, N
    double masse = 0.0;           ///< m, kg/m
    double biegesteife = 0.0;     ///< EI, N m²
    double verlustBiegung = 0.0;  ///< η_B
    double verlustInnen = 0.0;    ///< η_F (Valette)
    double verlustLuft = 0.0;     ///< η_A (Valette), 1/s
    double durchmesser = 0.0;     ///< m; > 0: monofil
    double dichte = 0.0;          ///< kg/m³ (monofil)

    [[nodiscard]] bool operator==(const Saitenwerte&) const = default;
};

/// Die Wellenzahl k(ω) der Saite bei der Kreisfrequenz ω (rad/s, > 0).
[[nodiscard]] std::complex<double> wellenzahl(const Saitenwerte& saite, double omega);

/// Betrag und Verzug des Umlaufs exp(−2 i k L) bei `hz`.
struct Umlaufwert
{
    double betrag = 1.0;
    double verzug = 0.0;  ///< rad, positiv
};
[[nodiscard]] Umlaufwert umlaufDerSaite(const Saitenwerte& saite, double laenge, double hz);

/// Ein Biquad-Allpass mit Polradius r und Polwinkel θ.
struct Biquad
{
    double radius = 0.0;
    double winkel = 0.0;

    [[nodiscard]] bool operator==(const Biquad&) const = default;
};

/// Der Verzug der Kette bei ω (stetig, positiv): Σ 2ω + 2 atan2(r sin(ω − θ), 1 − r cos(ω − θ)) + 2 atan2(r sin(ω + θ),
/// 1 − r cos(ω + θ)).
[[nodiscard]] double verzugKette(const std::vector<Biquad>& kette, double omega);

/// Die Antwort b(e^{iω}) / a(e^{iω}) = Σ b_k e^{−ikω} / Σ a_k e^{−ikω}.
[[nodiscard]] std::complex<double> antwort(const std::vector<double>& b, const std::vector<double>& a, double omega);

/// Der Nenner des Thiran-Allpasses der Ordnung `ordnung` fuer den Verzug `verzug` (a₀ = 1); der Zaehler ist er gespiegelt.
[[nodiscard]] std::vector<double> thiranNenner(double verzug, int ordnung);

/// Der Verzug des Thiran-Allpasses bei ω, stetig von ω → 0 an (wie die Vorlage: von 10⁻⁶ an abgewickelt).
[[nodiscard]] double verzugThiran(double verzug, int ordnung, double omega);

/// Die untere Grenze des gebrochenen Verzugs: Thiran der Ordnung N nimmt D in [N − ½ ; N + ½).
[[nodiscard]] inline double verzugUnten(int ordnung) { return ordnung - 0.5; }

/// Das Umlauffilter einer Stelle (einer Taste, oder einer Lage auf einer Saite).
struct Umlauffilter
{
    int verzugGanz = 0;         ///< N
    int gliedOrdnung = 5;       ///< Thiran
    double gliedVerzug = 4.5;   ///< D
    std::vector<Biquad> kette;
    std::vector<double> b{1.0}, a{1.0};  ///< Verlust

    /// Die Antwort ohne z^(−N).
    [[nodiscard]] std::complex<double> antwort(double omega) const;

    [[nodiscard]] bool operator==(const Umlauffilter&) const = default;
};

/// Stimmt das Glied: `sollVerzug` ist der Verzug, den Glied und ganzzahliger Verzug bei `omega1` (dem Grundton)
/// zusammen tragen sollen (Ziel minus Verlust minus Kette). Setzt N = ⌊soll/ω₁ − (Ordnung − ½)⌋ und D mit acht
/// Newton-Schritten an der genauen Phase des Glieds (wie die Vorlage). Falsch, wenn kein Platz bleibt
/// (soll/ω₁ < Ordnung + ½) — dann bleibt das Filter unveraendert.
bool stimmeGlied(Umlauffilter& filter, double omega1, double sollVerzug);

// ---------------------------------------------------------------- zur Laufzeit (seit 0.9.0, 34k-7d-3)
// Was der Kern braucht, um ein Filter aus der gestimmten Tafel zu bilden: das Glied am Grundton stimmen und die gebundene
// Tafel einer Saite an einer Lage lesen. Der Entwurf (`InstrumentMeasure/UmlaufEntwurf`) rechnet mit denselben Funktionen.

/// Der Takt des Entwurfs und des Kerns.
inline constexpr double kRate = 48000.0;
/// Das Raster des Entwurfs: 1 … 20 000 Hz in 0,5 Hz.
inline constexpr std::size_t kRaster = 39999;
/// Die Frequenz der Stelle `i` des Rasters.
[[nodiscard]] inline double rasterHz(std::size_t i) { return 1.0 + 0.5 * static_cast<double>(i); }
/// Hz in rad je Takt.
[[nodiscard]] inline double omegaVon(double hz) { return 2.0 * std::numbers::pi * hz / kRate; }

/// Der Verzug −unwrap(arg h) laengs einer Folge von Antworten (wie numpy; die Folge auf dem Raster).
[[nodiscard]] std::vector<double> verzugVon(const std::vector<std::complex<double>>& h);

/// Der Grundton der Saite aus ihrem Umlauf: die Stelle, an der der Verzug 2π erreicht, auf dem Raster linear (wie der
/// Entwurf, `np.interp`). 0, wenn der Verzug bis 20 kHz 2π nicht erreicht.
[[nodiscard]] double grundtonDerSaite(const Saitenwerte& saite, double laenge);

/// Stimmt das Glied eines Filters am Grundton der Saite, wie der Entwurf es nach jeder Anpassung tut: Soll ist der Verzug
/// des Ziels minus der des Verlusts (abgewickelt auf dem Raster) minus der der Kette, je auf dem Raster bei f₁ linear.
/// Falsch, wenn die Saite keinen Grundton bis 20 kHz hat oder das Glied nicht passt (`stimmeGlied`).
bool stimmeAmGrundton(Umlauffilter& filter, const Saitenwerte& saite, double laenge);

/// Die obere Grenze des Polradius der Kette.
inline constexpr double kRadiusMax = 0.99;
/// Jenseits dieser Grenze ist sigmoid konstant (die Logit-Parameter der Kette, TK2 a).
inline constexpr double kLogitGrenze = 40.0;
[[nodiscard]] double sigmoid(double x);
/// Die Legendre-Polynome P₀ … P_grad bei t.
[[nodiscard]] std::vector<double> legendre(double t, int grad);

/// Die Tafel einer Saite, gebunden ueber die Lage (Gitarre, Laute, Oud; UL3-1 a): je Biquad die Beiwerte von
/// logit(r / kRadiusMax) und logit(θ / π) als Legendre-Reihe in 2 u / buende − 1, der Verlust an Stellen (Buende und
/// halbe), dazwischen linear. Das Glied stimmt der Kern an der Lage selbst (`filterAn`).
struct Saitentafel
{
    int buende = 0;
    double mensur = 0.0;
    int grad = 3;
    std::vector<std::vector<double>> radiusReihe, winkelReihe;
    struct Verluststelle
    {
        double lage = 0.0;  ///< in Buenden
        std::vector<double> b, a;

        [[nodiscard]] bool operator==(const Verluststelle&) const = default;
    };
    std::vector<Verluststelle> verlust;  ///< aufsteigend

    /// Die Kette an der Lage `lage` (Buende, gebrochen).
    [[nodiscard]] std::vector<Biquad> ketteAn(double lage) const;
    /// Der Verlust an der Lage, zwischen den Nachbarstellen gemittelt.
    void verlustAn(double lage, std::vector<double>& b, std::vector<double>& a) const;

    [[nodiscard]] bool operator==(const Saitentafel&) const = default;
};

/// Das Filter einer Saite an der Lage `lage` (Buende, gebrochen; die Laenge ist mensur · 2^(−lage/12)): Kette und Verlust
/// aus der Tafel, das Glied (Thiran 5) am Grundton gestimmt. `std::nullopt`, wenn es nicht passt.
[[nodiscard]] std::optional<Umlauffilter> filterAn(const Saitentafel& tafel, const Saitenwerte& saite, double lage);

/// Ein IIR in transponierter Direktform II (a₀ = 1), doppelt genau.
class Iir
{
public:
    Iir() = default;
    Iir(std::vector<double> b, std::vector<double> a);
    double schritt(double x);
    void leere();

private:
    std::vector<double> m_b, m_a, m_z;
};

/// Das Umlauffilter im Takt, ohne den ganzzahligen Verzug: Verlust, Kette, Glied hintereinander.
class Umlauflauf
{
public:
    Umlauflauf() = default;
    explicit Umlauflauf(const Umlauffilter& filter);
    double schritt(double x);
    void leere();

private:
    Iir m_verlust;
    std::vector<Iir> m_kette;
    Iir m_glied;
};

}  // namespace lumi::kern::umlauf
