/**
 * @file Fft.hpp
 * @brief Gemeinsam: die reelle FFT hinter einer eigenen Schnittstelle und die Faltung (nach dem Vorbild
 *        gemeinsam/vorbild-lumiviz/code/loop/Fft.hpp; LumiViz FT1 a, FF1 a)
 *
 * Kein Typ einer fremden Bibliothek geht ueber diese Schnittstelle; die Umsetzung mit pocketfft (BSD-3,
 * `externals/pocketfft`, wie in LumiViz) steht in `Fft.cpp`. Daneben eine direkte DFT als zweite Umsetzung fuer
 * die Tests (langsam, nur kleine n). Zustandslos, von mehreren Threads zugleich benutzbar. Reines C++.
 */
#pragma once

#include <complex>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace ic
{

/// Die reelle FFT.
class Fft
{
public:
    virtual ~Fft() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    /// n reelle Werte → n/2 + 1 komplexe, ohne Normierung: X_k = Σ_t x_t e^(−2πi k t / n).
    virtual void vorwaerts(std::span<const double> zeit, std::span<std::complex<double>> spektrum) const = 0;
    /// n/2 + 1 komplexe → n reelle, mit 1/n: rueckwaerts(vorwaerts(x)) = x. n ist `zeit.size()`.
    virtual void rueckwaerts(std::span<const std::complex<double>> spektrum, std::span<double> zeit) const = 0;
};

/// Jede Umsetzung; die erste ist die Vorgabe (pocketfft), die zweite die direkte DFT.
[[nodiscard]] std::vector<std::unique_ptr<Fft>> fftUmsetzungen();

/// Die Vorgabe, einmal fuer das ganze Programm.
[[nodiscard]] const Fft& fft();

/// Die kleinste Zweierpotenz, die mindestens n ist (n = 0 → 1).
[[nodiscard]] std::size_t zweierpotenzAb(std::size_t n);

/// Die lineare Faltung a * b, Laenge a + b − 1 (leer, wenn eines leer ist), ueber die FFT.
[[nodiscard]] std::vector<double> falte(const Fft& umsetzung, std::span<const double> a, std::span<const double> b);

}  // namespace ic
