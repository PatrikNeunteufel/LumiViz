/**
 * @file Begrenzer.hpp
 * @brief Ein Limiter fuer Stereo-Fliesskomma: haelt die Spitzen unter einer
 *        Decke, statt sie hart abzuschneiden (docs/ui/Composer_Konzept.md §9g,
 *        Entscheid Patrik S126, L4 a; Vorschau seit S130, L2-1 b)
 *
 * Ohne Engine und ohne Qt. Die Engine haengt ihn als DSP an einen Mixer
 * (`IAudioEngine::setMixerLimiter`).
 *
 * Wie er rechnet: je Frame die groessere Spitze beider Kanaele (beide Kanaele
 * gleich, damit das Stereobild steht) und daraus die Verstaerkung, die diesen
 * Frame genau auf die Decke braechte. Er schaut **rund 5 ms voraus**: die
 * Verstaerkung sinkt ueber die Vorschau gleitend auf das, was die kommende
 * Spitze braucht, und steigt danach weich zurueck (Loslassen). Dafuer kommt
 * der Ton um `verzoegerungFrames()` spaeter. Ohne Vorschau sprang die
 * Verstaerkung an jeder Spitze in einem Frame — das knackte hoerbar (Sichttest
 * 11, L2). Was nie ueber die Decke kommt, bleibt **Bit fuer Bit** gleich, nur
 * verzoegert — die Verstaerkung ist dann genau 1.
 *
 * Warum er die Decke einhaelt: die Verstaerkung eines Frames ist das Mittel
 * der Ziel-Verstaerkungen der letzten n Frames (n = Vorschau); jede davon ist
 * das Minimum ueber die n Frames ab ihr, deckt also den Frame selbst ab und
 * liegt nicht ueber dem, was er braucht.
 */
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

class Begrenzer
{
public:
    /// Decke −0,1 dBFS (L4 a).
    static constexpr float kDecke = 0.98855309f;
    /// So lange braucht die Verstaerkung, um nach einer Spitze rund 63 % des
    /// Wegs zurueck nach 1 zu gehen.
    static constexpr double kLoslassenMs = 80.0;
    /// So weit schaut er voraus (L2-1 b, S130).
    static constexpr double kVorschauMs = 5.0;

    explicit Begrenzer(int sampleRate);

    /// `frames` Stereo-Frames (links, rechts) an Ort und Stelle begrenzen; was
    /// herauskommt, ist um `verzoegerungFrames()` verzoegert (am Anfang Stille).
    void verarbeite(float* stereo, std::size_t frames);
    /// Zurueck auf den Anfang: Verstaerkung 1, Vorschau leer (etwa nach einem Sprung).
    void zuruecksetzen();
    /// Die Verstaerkung des zuletzt ausgegebenen Frames, 0…1 (1 = greift nicht).
    [[nodiscard]] double verstaerkung() const { return m_verstaerkung; }
    /// Um so viele Frames kommt der Ton spaeter.
    [[nodiscard]] std::size_t verzoegerungFrames() const { return m_n - 1; }
    /// Wie viele Frames seit dem Anlegen begrenzt wurden (Verstaerkung unter 1).
    /// Darf aus einem anderen Thread gelesen werden als dem, der rechnet.
    [[nodiscard]] std::size_t begrenzteFrames() const { return m_begrenzt.load(std::memory_order_relaxed); }

private:
    std::size_t m_n;    ///< Laenge der Vorschau in Frames (mindestens 1)
    double m_rueckweg;  ///< Anteil des Rest-Wegs nach 1 je Frame

    std::uint64_t m_frame = 0;      ///< Zaehler der eingegangenen Frames
    std::vector<float> m_verzug;    ///< die letzten n Frames, Stereo (Ring)
    std::vector<double> m_ziele;    ///< die letzten n Ziel-Verstaerkungen (Ring)
    double m_summe = 0.0;           ///< Summe von m_ziele
    std::size_t m_unterEins = 0;    ///< wie viele von m_ziele unter 1 liegen
    double m_ziel = 1.0;            ///< laufende Ziel-Verstaerkung (mit Loslassen)
    double m_verstaerkung = 1.0;

    // gleitendes Minimum der noetigen Verstaerkung ueber die Vorschau
    // (monotone Schlange, Ring mit n Plaetzen)
    std::vector<std::uint64_t> m_minFrame;
    std::vector<double> m_minWert;
    std::size_t m_minKopf = 0;
    std::size_t m_minAnzahl = 0;

    std::atomic<std::size_t> m_begrenzt{0};
};
