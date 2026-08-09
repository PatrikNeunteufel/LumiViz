/**
 ****************************************************************************************
 * @file   AudioDateiQuelle.hpp
 * @brief  Deterministische Audio-Zufuhr aus einer Audiodatei fuer AvsStandalone
 *         und MilkdropStandalone (Session 74, Aufgabe 6 des Kalibrier-Plans)
 *
 * @author Patrik Neunteufel
 * @date   August 2026
 * @version 1.0.0
 *
 * @details
 * Die Standalones fuettern ihre Visualizer sonst mit einem synthetischen Signal
 * (Sinus + Beat-Puls). Presets, die auf echte Musikdynamik ausgelegt sind —
 * Bass-Einsaetze, Refrain-Aufbau, Stille zwischen Schlaegen — zeigen daran nie,
 * was sie koennen. Diese Klasse dekodiert eine Audiodatei (MP3, WAV, FLAC, …
 * alles was Qt Multimedia kann) EINMAL komplett in den Speicher und liefert
 * daraus je Bild dieselben zwei Bloecke, die auch die App liefert:
 *
 *   - **Wellenform**: `kWaveFrames` Stereo-Rahmen roher PCM ab der Bildposition
 *   - **Spektrum**:   `kBins` Baender je Kanal aus einer Hann-gefensterten
 *                     `kFftGroesse`-Punkt-FFT, linear 0..22050 Hz
 *
 * Das Bandmodell passt damit zu `MilkLoudness` (Terzen 200 / 761,2 / 2897,1 /
 * 11025 Hz, `MilkdropVisualizer::updateAudio`), das genau diese lineare
 * Aufteilung ueber die Bin-Anzahl voraussetzt.
 *
 * **Deterministisch:** abgetastet wird nach BILD-INDEX (`t = bild / kBildrate`),
 * nicht nach Echtzeit. Zwei Laeufe mit derselben Datei, demselben Startversatz
 * und derselben Bildzahl liefern bitgleiche Puffer — sonst waere das Werkzeug
 * fuer Vergleichslaeufe unbrauchbar. Laeuft die Datei aus, wird umlaufend
 * weitergelesen (Modulo), damit lange Laeufe nicht in Stille kippen.
 *
 * @warning **Nicht fuer Referenzvergleiche.** `AvsRef` und `MilkdropRef`
 *          erzeugen ihr Audio selbst — formelgleich zum synthetischen Signal
 *          der Standalones. Genau darauf beruht die Vergleichbarkeit. Hoert
 *          unsere Seite echte Musik und die Referenz weiter ihren Sinus,
 *          vergleicht man zwei verschiedene Eingaben und jede Zahl daraus ist
 *          wertlos. Diese Quelle ist fuer Schaufenster und Augenschein.
 ****************************************************************************************
 */

#pragma once

#include <QIODevice>
#include <QString>

#include <cstdint>
#include <vector>

namespace lumi::werkzeug
{

/**
 * @brief Dekodierte Audiodatei als bild-indizierte Wellenform-/Spektrum-Quelle
 */
class AudioDateiQuelle
{
public:
    /// Arbeits-Abtastrate — die Bandgrenzen von MilkLoudness rechnen gegen
    /// 22050 Hz Nyquist, also bleibt es bei 44,1 kHz.
    static constexpr int kAbtastrate = 44100;
    static constexpr int kKanaele = 2;
    /// Rahmen je Bild in der Wellenform (Vertrag der Standalones)
    static constexpr int kWaveFrames = 576;
    /// FFT-Laenge; die halbe Laenge ist die Bandzahl (wie BASS_DATA_FFT1024)
    static constexpr int kFftGroesse = 1024;
    static constexpr int kBins = kFftGroesse / 2;
    /// Bildrate, gegen die der Bild-Index in Sekunden umgerechnet wird. Die
    /// Standalones rendern mit festem dt = 1/60 — hier steht dieselbe Zahl.
    static constexpr double kBildrate = 60.0;

    /// Datei dekodieren (blockierend, mit eigener Ereignisschleife).
    /// @return false samt Text in @p fehler, wenn nichts dekodiert werden konnte
    bool laden(const QString& pfad, QString* fehler = nullptr);

    /// Startversatz in Sekunden — damit man eine interessante Stelle trifft
    /// statt der Einleitung. Wirkt auf Bild-Zufuhr UND hoerbare Ausgabe.
    void setStartSekunden(double sekunden);

    /// Faktor auf Wellenform und Spektrum (Vorgabe 1,0). Echte Musik ist
    /// deutlich leiser als das synthetische Signal (dort ~0,8 Spitzenwert im
    /// Spektrum); wer Presets vergleichbar „heiss" fahren will, dreht hier auf.
    /// Die HOERBARE Ausgabe bleibt unveraendert — sonst uebersteuert sie.
    void setVerstaerkung(double faktor);

    [[nodiscard]] bool bereit() const { return m_frames > 0; }
    [[nodiscard]] double dauerSekunden() const;
    [[nodiscard]] QString quelle() const { return m_quelle; }

    /**
     * @brief Puffer fuer ein Bild fuellen
     * @param bildIndex 0-basiert; bestimmt die Leseposition allein
     * @param wave      Ziel, @c kWaveFrames*2 Werte (interleaved L,R)
     * @param spec      Ziel, @c kBins*2 Werte (interleaved L,R)
     */
    void frameFuellen(std::int64_t bildIndex, float* wave, float* spec) const;

    /// Rohes PCM (interleaved Stereo, @c kAbtastrate) — fuer die hoerbare Ausgabe
    [[nodiscard]] const std::vector<float>& pcm() const { return m_pcm; }
    /// Startversatz in Rahmen (nicht in Samples)
    [[nodiscard]] std::int64_t startRahmen() const { return m_startRahmen; }

    /**
     * @brief Musik-Profil rechnen und als C++-Kopf schreiben (S74)
     *
     * Zerlegt @p sekunden ab dem Startversatz in Bilder zu 1/60 s und haelt je
     * Bild acht log-verteilte Bandhuellkurven plus eine Beat-Spur fest. Daraus
     * synthetisiert `SynthAudio.hpp` spaeter ein Signal, das die Dynamik der
     * Aufnahme hat, ohne die Aufnahme zu sein — und das beide Seiten eines
     * Referenzvergleichs identisch erzeugen koennen.
     *
     * Die Schleifenlaenge wird auf ein Vielfaches des erkannten Schlagabstands
     * gerundet, damit der Uebergang am Ende auf einen Schlag faellt statt
     * mitten hinein.
     *
     * **Bandnormierung je Band, nicht global** (bewusst): absolut betrachtet
     * liegen die oberen Baender einer Aufnahme dutzendfach unter dem Bass —
     * global normiert waeren sie im uint8-Raster fast durchweg 0 und das
     * Signal waere oben tot. Das Profil traegt darum die RELATIVE Dynamik je
     * Band; die absolute Klangbalance geht dabei verloren. Fuer ein
     * Pruefsignal ist das der richtige Tausch.
     */
    bool profilSchreiben(const QString& zielDatei, double sekunden,
                         QString* fehler = nullptr) const;

private:
    std::vector<float> m_pcm;        ///< interleaved Stereo @ kAbtastrate
    std::int64_t m_frames = 0;       ///< Rahmen (nicht Samples)
    std::int64_t m_startRahmen = 0;
    double m_verstaerkung = 1.0;
    QString m_quelle;
};

/**
 * @brief Nur-Lese-Geraet ueber das dekodierte PCM — Futter fuer QAudioSink
 *
 * Pull-Betrieb: die Klangausgabe holt sich ihr Material selbst, wir schreiben
 * nichts. Bewusst OHNE @c Q_OBJECT — die Klasse deklariert keine eigenen
 * Signale/Slots, und QAudioSink kommt mit den geerbten aus QIODevice aus.
 * Laeuft die Datei aus, beginnt sie am Startversatz von vorn (wie die
 * Bild-Zufuhr).
 */
class PcmGeber : public QIODevice
{
public:
    explicit PcmGeber(const AudioDateiQuelle& quelle);

    [[nodiscard]] bool isSequential() const override { return true; }

protected:
    qint64 readData(char* daten, qint64 maxGroesse) override;
    qint64 writeData(const char*, qint64) override { return -1; }

private:
    const AudioDateiQuelle* m_quelle;
    std::int64_t m_pos = 0;  ///< Leseposition in Samples (interleaved)
};

}  // namespace lumi::werkzeug
