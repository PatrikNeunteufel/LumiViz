/**
 * @file Fft.hpp
 * @brief InstrumentCore: die reelle FFT hinter einer eigenen Schnittstelle und die Faltung
 *        (Spielweisen_Konzept.md §4.1, FT1 a, FF1 a; Instrumente_IC_Konzept.md IK2-2 a, IK2-2-1 a, KN2 a)
 *
 * Kein Typ einer fremden Bibliothek geht ueber diese Schnittstelle; die Umsetzung mit pocketfft
 * (`externals/pocketfft`, BSD-3) steht in `Fft.cpp`. Eine andere Umsetzung kommt in `fftUmsetzungen()` dazu, die
 * Tests laufen gegen jede. Daneben die DFT nach Definition als **Pruefumsetzung** (`direkteDft()`, aus IC
 * `gemeinsam/Fft.cpp`): O(n²), nur fuer kleine n in Tests — sie steht nicht in `fftUmsetzungen()`, weil die Faelle dort
 * bis 65536 Werte rechnen (Uebernahme-Journal).
 *
 * Zustandslos, von mehreren Threads zugleich benutzbar. Reines C++ ohne Qt (seit S175 hier, vorher `loop/Fft.hpp` der
 * App).
 */
#pragma once

#include <complex>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace lumi::kern
{

/// Die reelle FFT. Eine Umsetzung ist zustandslos und darf von mehreren Threads zugleich benutzt werden.
class Fft
{
public:
    virtual ~Fft() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    /// n reelle Werte → n/2 + 1 komplexe, ohne Normierung: X_k = Σ_t x_t e^(−2πi k t / n).
    /// `spektrum` muss n/2 + 1 Werte fassen (sonst `std::invalid_argument`).
    virtual void vorwaerts(std::span<const double> zeit, std::span<std::complex<double>> spektrum) const = 0;
    /// n/2 + 1 komplexe → n reelle, mit 1/n: rueckwaerts(vorwaerts(x)) = x. n ist `zeit.size()`.
    virtual void rueckwaerts(std::span<const std::complex<double>> spektrum, std::span<double> zeit) const = 0;
};

/// Jede Umsetzung fuer den Gebrauch; die erste ist die Vorgabe.
[[nodiscard]] std::vector<std::unique_ptr<Fft>> fftUmsetzungen();

/// Die Vorgabe, einmal fuer das ganze Programm.
[[nodiscard]] const Fft& fft();

/// Die DFT nach Definition (O(n²)) — nur als Bezug in Tests, fuer kleine n.
[[nodiscard]] std::unique_ptr<Fft> direkteDft();

/// Die kleinste Zweierpotenz, die mindestens n ist (n = 0 → 1).
[[nodiscard]] std::size_t zweierpotenzAb(std::size_t n);

/// Die lineare Faltung a * b, Laenge a + b − 1 (leer, wenn eines leer ist), ueber die FFT auf
/// `zweierpotenzAb(a + b − 1)` Werten.
[[nodiscard]] std::vector<double> falte(const Fft& umsetzung, std::span<const double> a, std::span<const double> b);

}  // namespace lumi::kern
