/**
 * @file Ausgleich.hpp
 * @brief InstrumentCore: kleine dichte Ausgleichsrechnung — Matrix, Gauss, Householder-QR, kleinste Quadrate, NNLS
 *        (seit 0.10.0, 34k-7d-4; vorher in `InstrumentMeasure`: `Matrix` und `loese` in `LevenbergMarquardt`, `Qr`,
 *        `kleinsteQuadrate` und `nnls` anonym im Entwurf des Umlauf-Filters — wortgleich umgezogen, Bit fuer Bit gleich)
 *
 * Der Kern `pluckedString` passt zur Laufzeit die Gewichte seines Daempfers ueber 5 kHz mit `nnls` an (GZ5-1 a: sieben
 * breite Moden, φ² ≥ 0); der Entwurf des Umlauf-Filters (`InstrumentMeasure/UmlaufEntwurf`) rechnet mit denselben
 * Funktionen, `InstrumentMeasure` nimmt `Matrix` und `loese` per `using` in seinen Namensraum.
 *
 * Reines C++ ohne Qt, ohne Eigen — die Systeme sind klein (bis einige hundert Unbekannte).
 */
#pragma once

#include <cstddef>
#include <vector>

namespace lumi::kern
{

/// Eine dichte Matrix, zeilenweise.
struct Matrix
{
    std::size_t zeilen = 0;
    std::size_t spalten = 0;
    std::vector<double> werte;

    Matrix() = default;
    Matrix(std::size_t z, std::size_t s) : zeilen(z), spalten(s), werte(z * s, 0.0) {}
    double& operator()(std::size_t i, std::size_t j) { return werte[i * spalten + j]; }
    [[nodiscard]] double operator()(std::size_t i, std::size_t j) const { return werte[i * spalten + j]; }
    [[nodiscard]] bool operator==(const Matrix&) const = default;
};

/// Loest a·x = b (Gauss mit Spaltenpivot); eine Spalte ohne Pivot (singulaer) bekommt x = 0.
std::vector<double> loese(Matrix a, std::vector<double> b);

/// Householder-QR einer Matrix (zeilen ≥ spalten): R im oberen Dreieck, Qᵀ angewandt auf rechte Seiten.
struct Qr
{
    std::size_t m = 0, n = 0;
    std::vector<double> a;     ///< zeilenweise, nach der Zerlegung R im oberen Dreieck
    std::vector<double> beta;  ///< je Spiegelung
    std::vector<std::vector<double>> v;

    explicit Qr(const Matrix& A);
    /// b ← Qᵀ b
    void qtMal(std::vector<double>& b) const;
    [[nodiscard]] double r(std::size_t i, std::size_t j) const { return a[i * n + j]; }
    /// R x = c (oberes Dreieck, n × n); ein Pivot von null gibt 0
    [[nodiscard]] std::vector<double> rueck(const std::vector<double>& c) const;
};

/// min ||A x − b|| (zeilen ≥ spalten), ueber `Qr`.
std::vector<double> kleinsteQuadrate(const Matrix& A, std::vector<double> b);

/// NNLS nach Lawson & Hanson 1974 (Kap. 23): min ||A x − b||, x ≥ 0; Schranke wie scipy `nnls`
/// (10 · max(m, n) · ε). `ldpLaenge`: im LDP einer LSI die Laenge, auf die jede Spalte normiert wurde — dann gilt eine
/// Spalte als fertig, wenn die Verletzung ihrer Nebenbedingung (w_j · Laenge / |r_n|) unter 10⁻⁹ liegt (S220).
std::vector<double> nnls(const Matrix& A, const std::vector<double>& b, const std::vector<double>* ldpLaenge = nullptr);

}  // namespace lumi::kern
