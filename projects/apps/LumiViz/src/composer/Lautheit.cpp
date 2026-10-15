/**
 * @file Lautheit.cpp
 * @brief Composer: Lautheit nach EBU R128 / ITU-R BS.1770 (Composer_Konzept §9g)
 *
 * Der Weg nach BS.1770: K-Filter je Kanal (Hochton-Anhebung, dann Hochpass),
 * mittleres Quadrat in Bloecken von 400 ms mit 75 % Ueberlappung (gerechnet
 * aus Stuecken von 100 ms), Lautheit eines Blocks −0,691 + 10·log10(Summe der
 * Kanaele), absolutes Gate −70 LUFS, relatives Gate 10 LU unter dem Mittel der
 * Bloecke ueber dem absoluten. Die Filter werden fuer jede Abtastrate aus
 * ihren Kennwerten gerechnet (die Norm nennt Koeffizienten nur fuer 48 kHz);
 * Kennwerte und Formeln wie in libebur128 (MIT). Belegt an den Prueftoenen
 * aus EBU Tech 3341 (test_ComposerLautheit.cpp).
 */

#include "composer/Lautheit.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace lumi::composer
{

namespace
{

constexpr double kVersatz = -0.691;
constexpr double kAbsolutesGate = -70.0;
constexpr double kRelativesGate = -10.0;
constexpr std::size_t kStueckeJeBlock = 4;  // 400 ms aus vier Stuecken zu 100 ms

double lautheitAus(double mittleresQuadrat)
{
    return kVersatz + 10.0 * std::log10(mittleresQuadrat);
}

}  // namespace

LautheitsMessung::LautheitsMessung(int sampleRate)
    : m_stueckFrames(static_cast<std::size_t>(std::max(1, sampleRate / 10)))
{
    const double fs = static_cast<double>(std::max(1, sampleRate));
    {
        // Stufe 1: Hochton-Anhebung um rund 4 dB
        const double f0 = 1681.974450955533;
        const double g = 3.999843853973347;
        const double q = 0.7071752369554196;
        const double k = std::tan(std::numbers::pi * f0 / fs);
        const double vh = std::pow(10.0, g / 20.0);
        const double vb = std::pow(vh, 0.4996667741545416);
        const double a0 = 1.0 + k / q + k * k;
        for (Biquad& b : m_shelf)
        {
            b.b0 = (vh + vb * k / q + k * k) / a0;
            b.b1 = 2.0 * (k * k - vh) / a0;
            b.b2 = (vh - vb * k / q + k * k) / a0;
            b.a1 = 2.0 * (k * k - 1.0) / a0;
            b.a2 = (1.0 - k / q + k * k) / a0;
        }
    }
    {
        // Stufe 2: Hochpass um 38 Hz
        const double f0 = 38.13547087602444;
        const double q = 0.5003270373238773;
        const double k = std::tan(std::numbers::pi * f0 / fs);
        const double a0 = 1.0 + k / q + k * k;
        for (Biquad& b : m_hochpass)
        {
            b.b0 = 1.0;
            b.b1 = -2.0;
            b.b2 = 1.0;
            b.a1 = 2.0 * (k * k - 1.0) / a0;
            b.a2 = (1.0 - k / q + k * k) / a0;
        }
    }
}

void LautheitsMessung::fuettere(const float* stereo, std::size_t frames)
{
    for (std::size_t i = 0; i < frames; ++i)
    {
        for (int kanal = 0; kanal < 2; ++kanal)
        {
            const float x = stereo[2 * i + static_cast<std::size_t>(kanal)];
            m_spitze = std::max(m_spitze, std::abs(x));
            const double y = m_hochpass[kanal].schritt(m_shelf[kanal].schritt(x));
            m_summe += y * y;
        }
        if (++m_imStueck == m_stueckFrames)
        {
            m_stuecke.push_back(m_summe / static_cast<double>(m_stueckFrames));
            m_summe = 0.0;
            m_imStueck = 0;
        }
    }
}

Lautheit LautheitsMessung::ergebnis() const
{
    Lautheit l;
    if (m_spitze > 0.0f) l.spitzeDb = 20.0 * std::log10(static_cast<double>(m_spitze));
    if (m_stuecke.size() < kStueckeJeBlock) return l;

    // die Bloecke: je vier Stuecke, um eines versetzt (75 % Ueberlappung)
    std::vector<double> bloecke;
    bloecke.reserve(m_stuecke.size() - kStueckeJeBlock + 1);
    for (std::size_t j = 0; j + kStueckeJeBlock <= m_stuecke.size(); ++j)
    {
        double z = 0.0;
        for (std::size_t s = 0; s < kStueckeJeBlock; ++s) z += m_stuecke[j + s];
        bloecke.push_back(z / static_cast<double>(kStueckeJeBlock));
    }

    const auto mittelUeber = [&bloecke](double schwelle, std::size_t& zahl) {
        double summe = 0.0;
        zahl = 0;
        for (double z : bloecke)
            if (z > 0.0 && lautheitAus(z) > schwelle)
            {
                summe += z;
                ++zahl;
            }
        return zahl > 0 ? summe / static_cast<double>(zahl) : 0.0;
    };
    std::size_t zahl = 0;
    const double ueberAbsolut = mittelUeber(kAbsolutesGate, zahl);
    if (zahl == 0) return l;
    const double relativ = std::max(kAbsolutesGate, lautheitAus(ueberAbsolut) + kRelativesGate);
    const double integriert = mittelUeber(relativ, zahl);
    if (zahl == 0) return l;
    l.lufs = lautheitAus(integriert);
    l.stille = false;
    return l;
}

Lautheit messeLautheit(const std::vector<float>& stereo, int sampleRate)
{
    LautheitsMessung messung(sampleRate);
    messung.fuettere(stereo.data(), stereo.size() / 2);
    return messung.ergebnis();
}

}  // namespace lumi::composer
