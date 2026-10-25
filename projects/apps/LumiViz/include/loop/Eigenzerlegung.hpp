/**
 * @file Eigenzerlegung.hpp
 * @brief Loop: Eigenwerte und Eigenvektoren kleiner komplexer Matrizen
 *        (docs/audio/Spielweisen_Konzept.md §4.1, Stufe 31d-7; Entscheid Patrik S153: P6 a)
 *
 * Fuer die Moden der Hammer-Saite je Teilton: hoechstens zwei Ebenen je Saite und drei
 * Saiten, also bis 6 × 6, allgemein komplex. Hessenberg-Form, QR mit Verschiebung nach
 * Wilkinson zur Schur-Form, die Eigenvektoren daraus, isolierte durch einen Schritt
 * inverser Iteration nachgeschaerft; die linken als Inverse der rechten. Eine schon
 * diagonale Matrix wird unveraendert uebernommen (Werte = Diagonale, Vektoren = Einheit).
 *
 * Gegen Eigen geprueft (`test_LoopKleinEigen`); Eigen selbst ist nur an die Tests gebunden.
 * Kein Qt, kein `windows.h` (IK1). Derselbe Aufruf ergibt dieselben Werte.
 */
#pragma once

#include <complex>
#include <cstddef>
#include <vector>

namespace lumi::loop
{

/// So gross darf eine Matrix hoechstens sein.
inline constexpr int kKleineMatrixMax = 16;

/// Eine kleine quadratische Matrix, zeilenweise.
struct KleineMatrix
{
    int n = 0;
    std::vector<std::complex<double>> werte;

    KleineMatrix() = default;
    explicit KleineMatrix(int groesse) : n(groesse), werte(static_cast<std::size_t>(groesse) * static_cast<std::size_t>(groesse)) {}

    [[nodiscard]] std::complex<double>& operator()(int zeile, int spalte)
    {
        return werte[static_cast<std::size_t>(zeile) * static_cast<std::size_t>(n) + static_cast<std::size_t>(spalte)];
    }
    [[nodiscard]] const std::complex<double>& operator()(int zeile, int spalte) const
    {
        return werte[static_cast<std::size_t>(zeile) * static_cast<std::size_t>(n) + static_cast<std::size_t>(spalte)];
    }
};

/// A = rechts · diag(werte) · links.
struct Eigenzerlegung
{
    std::vector<std::complex<double>> werte;
    /// Spalte m: der Eigenvektor zu `werte[m]`, euklidisch auf 1 gebracht.
    KleineMatrix rechts;
    /// Zeile m: der linke Eigenvektor, so dass `links · rechts` die Einheit ist (die Inverse von `rechts`).
    KleineMatrix links;
    /// Falsch, wenn die QR-Iteration nicht zusammenlief oder `rechts` nicht umkehrbar war (eine
    /// defekte Matrix) — die Werte sind dann nur genaehert.
    bool vollstaendig = true;
};

/// Die Eigenzerlegung von `a` (1 … `kKleineMatrixMax`). Eine leere Matrix gibt eine leere Zerlegung.
[[nodiscard]] Eigenzerlegung zerlegeKlein(const KleineMatrix& a);

}  // namespace lumi::loop
