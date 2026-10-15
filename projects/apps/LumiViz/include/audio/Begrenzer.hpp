/**
 * @file Begrenzer.hpp
 * @brief Ein Limiter fuer Stereo-Fliesskomma: haelt die Spitzen unter einer
 *        Decke, statt sie hart abzuschneiden (docs/ui/Composer_Konzept.md §9g,
 *        Entscheid Patrik S126, L4 a)
 *
 * Ohne Engine und ohne Qt. Die Engine haengt ihn als DSP an einen Mixer
 * (`IAudioEngine::setMixerLimiter`).
 *
 * Wie er rechnet: je Frame die groessere Spitze beider Kanaele (beide Kanaele
 * gleich, damit das Stereobild steht). Liegt sie mal der laufenden Verstaerkung
 * ueber der Decke, sinkt die Verstaerkung sofort so weit, dass sie genau auf
 * der Decke liegt; danach steigt sie weich zurueck (Loslassen). Ohne Vorschau:
 * so verzoegert er nichts, und was nie ueber die Decke kommt, bleibt **Bit fuer
 * Bit** gleich — die Verstaerkung ist dann genau 1.
 */
#pragma once

#include <atomic>
#include <cstddef>

class Begrenzer
{
public:
    /// Decke −0,1 dBFS (L4 a).
    static constexpr float kDecke = 0.98855309f;
    /// So lange braucht die Verstaerkung, um nach einer Spitze rund 63 % des
    /// Wegs zurueck nach 1 zu gehen.
    static constexpr double kLoslassenMs = 80.0;

    explicit Begrenzer(int sampleRate);

    /// `frames` Stereo-Frames (links, rechts) an Ort und Stelle begrenzen.
    void verarbeite(float* stereo, std::size_t frames);
    /// Verstaerkung zurueck auf 1 (etwa nach einem Sprung).
    void zuruecksetzen() { m_verstaerkung = 1.0; }
    /// Die laufende Verstaerkung, 0…1 (1 = greift nicht).
    [[nodiscard]] double verstaerkung() const { return m_verstaerkung; }
    /// Wie viele Frames seit dem Anlegen begrenzt wurden (Verstaerkung unter 1).
    /// Darf aus einem anderen Thread gelesen werden als dem, der rechnet.
    [[nodiscard]] std::size_t begrenzteFrames() const { return m_begrenzt.load(std::memory_order_relaxed); }

private:
    double m_rueckweg;  ///< Anteil des Rest-Wegs nach 1 je Frame
    double m_verstaerkung = 1.0;
    std::atomic<std::size_t> m_begrenzt{0};
};
