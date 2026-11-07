/**
 * @file Eigenzerlegung.hpp
 * @brief InstrumentCore: Eigenwerte und Eigenvektoren kleiner komplexer Matrizen (Stufe 31d-7, P6 a), mit Meldung
 *        entarteter Eigenwerte (IC Lehre L11)
 *
 * Uebernommen aus IC `rechnung/gemeinsam/Eigenzerlegung.hpp` (`bb7e6d6`), die reichere Fassung der LumiViz-Datei
 * `include/loop/Eigenzerlegung.hpp` (bis S174); nur Namensraum und dieser Kopf geaendert (Instrumente_IC_Konzept.md
 * IK2-2 a, KN2 a; Uebernahme-Journal).
 *
 * Abweichung von der LumiViz-Fassung: `kleinsterAbstand` und `entartet` melden, wenn zwei Eigenwerte so nah liegen, dass das
 * Nachschaerfen ausfaellt (Abstand unter 10⁻⁸ der Groesse der Matrix) — dann sind die Eigenvektoren des Paars
 * beliebig und Ableitungen nach den Parametern rauschen (L11, Taste 84 in S163). Der Aufrufer vermeidet das durch
 * Grenzen (etwa δ ≥ 0,01 Cent).
 *
 * Fuer die Moden der Hammer-Saite je Teilton: hoechstens zwei Ebenen je Saite und drei
 * Saiten, also bis 6 × 6, allgemein komplex. Hessenberg-Form, QR mit Verschiebung nach
 * Wilkinson zur Schur-Form, die Eigenvektoren daraus, isolierte durch einen Schritt
 * inverser Iteration nachgeschaerft; die linken als Inverse der rechten. Eine schon
 * diagonale Matrix wird unveraendert uebernommen (Werte = Diagonale, Vektoren = Einheit).
 *
 * Gegen Eigen geprueft (`test_LoopKleinEigen`); in IC dazu gegen geschlossene 2 × 2-Faelle und A·v = λv.
 * Kein Qt, kein `windows.h` (IK1). Derselbe Aufruf ergibt dieselben Werte.
 */
#pragma once

#include <complex>
#include <cstddef>
#include <vector>

namespace lumi::kern
{

/// So gross darf eine Matrix hoechstens sein.
/// Bis S174 in LumiViz 16; 32, damit der Luftraum mit bis zu 15 Kolben (2·(15 + 1) Zustaende) hineinpasst.
inline constexpr int kKleineMatrixMax = 32;

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
    /// Der kleinste Abstand zweier Eigenwerte, bezogen auf die Groesse der Matrix (Frobenius-Norm); unendlich bei
    /// einer diagonalen oder 1 × 1-Matrix.
    double kleinsterAbstand = 0.0;
    /// Wahr, wenn ein Eigenwert so nah an einem anderen liegt, dass das Nachschaerfen ausfiel (L11).
    bool entartet = false;
};

/// Unter diesem Abstand (gegen die Groesse der Matrix) gelten zwei Eigenwerte als entartet (wie im Vorbild:
/// dort faellt das Nachschaerfen aus).
inline constexpr double kEntartetAbstand = 1e-8;

/// Die Eigenzerlegung von `a` (1 … `kKleineMatrixMax`). Eine leere Matrix gibt eine leere Zerlegung.
[[nodiscard]] Eigenzerlegung zerlegeKlein(const KleineMatrix& a);

}  // namespace lumi::kern
