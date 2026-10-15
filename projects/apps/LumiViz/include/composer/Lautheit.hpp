/**
 * @file Lautheit.hpp
 * @brief Composer: Lautheit nach EBU R128 / ITU-R BS.1770 und Spitze eines
 *        Stereo-Signals (Composer_Konzept §9g, Entscheid Patrik S126, L2 a)
 *
 * Ohne Engine und ohne Qt: der Baustein bekommt Samples (Stereo,
 * verschraenkt, Fliesskomma) und rechnet. Welcher Ausschnitt eines Clips
 * gemessen wird (L3b a), entscheidet der Aufrufer.
 */
#pragma once

#include <cstddef>
#include <vector>

namespace lumi::composer
{

/// Wert fuer „nichts zu messen" (Stille, kein Ton), in LUFS bzw. dBFS.
inline constexpr double kLufsStille = -200.0;

/// Ergebnis einer Messung. `lufs` ist die integrierte Lautheit mit beiden
/// Gates; liegt alles unter dem absoluten Gate (oder ist der Ton kuerzer als
/// ein Block von 400 ms), ist `stille` wahr und `lufs` = `kLufsStille`.
/// `spitzeDb` ist die hoechste Sample-Spitze in dBFS (kein True Peak).
struct Lautheit
{
    double lufs = kLufsStille;
    double spitzeDb = kLufsStille;
    bool stille = true;
};

/// Misst Stereo-Samples laufend: `fuettere` beliebig oft, dann `ergebnis`.
/// So laesst sich ein langer Song stueckweise dekodieren und messen.
class LautheitsMessung
{
public:
    explicit LautheitsMessung(int sampleRate);

    /// `frames` Stereo-Frames (2 Werte je Frame, links zuerst).
    void fuettere(const float* stereo, std::size_t frames);
    [[nodiscard]] Lautheit ergebnis() const;

private:
    /// Ein Filter zweiter Ordnung (Direktform II transponiert).
    struct Biquad
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
        double z1 = 0.0, z2 = 0.0;
        double schritt(double x)
        {
            const double y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };

    Biquad m_shelf[2];     ///< K-Filter, Stufe 1 (Hochton-Anhebung), je Kanal
    Biquad m_hochpass[2];  ///< K-Filter, Stufe 2 (RLB-Hochpass), je Kanal
    std::size_t m_stueckFrames;      ///< 100 ms
    std::size_t m_imStueck = 0;      ///< Frames im laufenden Stueck
    double m_summe = 0.0;            ///< Summe der Quadrate beider Kanaele im laufenden Stueck
    std::vector<double> m_stuecke;   ///< mittleres Quadrat je fertigem Stueck (Kanaele summiert)
    float m_spitze = 0.0f;
};

/// Bequem fuer einen ganzen Puffer.
[[nodiscard]] Lautheit messeLautheit(const std::vector<float>& stereo, int sampleRate);

}  // namespace lumi::composer
