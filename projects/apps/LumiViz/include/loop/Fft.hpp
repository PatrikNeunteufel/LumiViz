/**
 * @file Fft.hpp
 * @brief Die reelle FFT und die Faltung je Ton (docs/audio/Spielweisen_Konzept.md §4.1;
 *        Entscheide Patrik S149/S150: FT1 a, FF1 a; Stufe 31c-7)
 *
 * Der Ton entsteht am Stueck vorab; mit Resonanzboden wird er je Hoerort mit der Antwort des
 * Bodens gefaltet, ueber eine FFT ueber den ganzen Ton, aufgefuellt auf die naechste
 * Zweierpotenz ueber Ton plus Antwort (FT1 a).
 *
 * Kein Typ einer fremden Bibliothek geht ueber diese Schnittstelle; die Umsetzung mit pocketfft
 * steht in `Fft.cpp` (`externals/pocketfft`). Eine andere Umsetzung kommt in `fftUmsetzungen()`
 * dazu, die Tests laufen gegen jede. Reines C++ ohne Qt.
 */
#pragma once

#include <complex>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace lumi::loop
{

/// Die reelle FFT. Eine Umsetzung ist zustandslos und darf von mehreren Threads zugleich
/// benutzt werden.
class Fft
{
public:
    virtual ~Fft() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    /// n reelle Werte → n/2 + 1 komplexe, ohne Normierung: X_k = Σ_t x_t e^(−2πi k t / n).
    /// `spektrum` muss n/2 + 1 Werte fassen.
    virtual void vorwaerts(std::span<const double> zeit, std::span<std::complex<double>> spektrum) const = 0;
    /// n/2 + 1 komplexe → n reelle, mit 1/n: rueckwaerts(vorwaerts(x)) = x. n ist `zeit.size()`.
    virtual void rueckwaerts(std::span<const std::complex<double>> spektrum, std::span<double> zeit) const = 0;
};

/// Jede Umsetzung; die erste ist die Vorgabe.
[[nodiscard]] std::vector<std::unique_ptr<Fft>> fftUmsetzungen();

/// Die Vorgabe, einmal fuer das ganze Programm.
[[nodiscard]] const Fft& fft();

/// Die kleinste Zweierpotenz, die mindestens n ist (n = 0 → 1).
[[nodiscard]] std::size_t zweierpotenzAb(std::size_t n);

/// Die lineare Faltung a * b, Laenge a + b − 1 (leer, wenn eines leer ist), ueber die FFT auf
/// `zweierpotenzAb(a + b − 1)` Werten.
[[nodiscard]] std::vector<double> falte(const Fft& umsetzung, std::span<const double> a, std::span<const double> b);

}  // namespace lumi::loop
