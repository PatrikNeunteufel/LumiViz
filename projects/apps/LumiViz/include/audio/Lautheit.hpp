/**
 * @file Lautheit.hpp
 * @brief Lautheit nach EBU R128 / ITU-R BS.1770 und Spitze eines
 *        Stereo-Signals (Composer_Konzept §9g, Entscheid Patrik S126, L2 a;
 *        seit S139 unter `audio/` fuer Composer und Loop, Loop_Konzept §4n,
 *        Stufe 27a)
 *
 * Ohne Engine und ohne Qt: der Baustein bekommt Samples (Stereo,
 * verschraenkt, Fliesskomma) und rechnet. Welcher Ausschnitt gemessen wird,
 * entscheidet der Aufrufer.
 *
 * Drei Masse nach EBU Tech 3341 (2023, S. 7): die integrierte Lautheit mit
 * beiden Gates, dazu das Maximum der **Momentary Loudness** (gleitendes
 * Rechteckfenster 0,4 s) und der **Short-term Loudness** (3 s), beide ohne
 * Gate. Das Fenster gleitet je Abtastwert, nicht in Stuecken — sonst traefe
 * es einen Ton von genau 0,4 s nicht ganz (Tech 3341 Fall 13).
 */
#pragma once

#include <cstddef>
#include <vector>

namespace lumi::audio
{

/// Wert fuer „nichts zu messen" (Stille, kein Ton), in LUFS bzw. dBFS.
inline constexpr double kLufsStille = -200.0;

/// Ergebnis einer Messung. `lufs` ist die integrierte Lautheit mit beiden
/// Gates; liegt alles unter dem absoluten Gate (oder ist der Ton kuerzer als
/// ein Block von 400 ms), ist `stille` wahr und `lufs` = `kLufsStille`.
/// `spitzeDb` ist die hoechste Sample-Spitze in dBFS (kein True Peak).
/// `momentanMaxLufs` und `kurzzeitMaxLufs` sind die Maxima der Momentary- und
/// der Short-term-Lautheit; `kLufsStille`, solange der Ton kuerzer als das
/// Fenster ist oder nur Stille traegt.
struct Lautheit
{
    double lufs = kLufsStille;
    double spitzeDb = kLufsStille;
    bool stille = true;
    double momentanMaxLufs = kLufsStille;
    double kurzzeitMaxLufs = kLufsStille;
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

    /// Ein gleitendes Rechteckfenster ueber die gefilterten Quadrate (beide
    /// Kanaele summiert): haelt das groesste Mittel, sobald es voll ist. Die
    /// Summe wird nach jedem Umlauf neu gezaehlt, damit sie nicht wegdriftet.
    struct Fenster
    {
        std::vector<double> werte;  ///< Ring, `werte.size()` Frames
        std::size_t stelle = 0;
        std::size_t gefuellt = 0;
        double summe = 0.0;
        double groesstesMittel = 0.0;
        void nimm(double quadrat);
    };

    Biquad m_shelf[2];     ///< K-Filter, Stufe 1 (Hochton-Anhebung), je Kanal
    Biquad m_hochpass[2];  ///< K-Filter, Stufe 2 (RLB-Hochpass), je Kanal
    std::size_t m_stueckFrames;      ///< 100 ms
    std::size_t m_imStueck = 0;      ///< Frames im laufenden Stueck
    double m_summe = 0.0;            ///< Summe der Quadrate beider Kanaele im laufenden Stueck
    std::vector<double> m_stuecke;   ///< mittleres Quadrat je fertigem Stueck (Kanaele summiert)
    float m_spitze = 0.0f;
    Fenster m_momentan;  ///< 0,4 s
    Fenster m_kurzzeit;  ///< 3 s
};

/// Bequem fuer einen ganzen Puffer.
[[nodiscard]] Lautheit messeLautheit(const std::vector<float>& stereo, int sampleRate);

}  // namespace lumi::audio
