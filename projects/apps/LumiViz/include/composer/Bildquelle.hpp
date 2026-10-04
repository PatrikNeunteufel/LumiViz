/**
 * @file Bildquelle.hpp
 * @brief Composer: die Komposition als Bildquelle der App
 *        (docs/ui/Composer_Konzept.md §5, §12.2 Naht R3, Stufe C2a)
 *
 * Die Bildquelle setzt sich ueber die Composer-Naht R3 an die Stelle des einen
 * Renderers: der Render-Thread ruft sie je Bild, sie fragt ihre Uhr nach der
 * Stelle, laesst die Wiedergabe das Bild dieser Stelle rechnen und zeichnet
 * das Gemischte ins Fenster. Audiodaten reicht sie an die Renderer der Clips
 * weiter.
 *
 * **Steht die Uhr, steht das Bild:** gerechnet wird nur, wenn die Uhr laeuft
 * oder sich Stelle, Groesse oder Komposition geaendert haben. Sonst wird das
 * letzte Gemischte noch einmal gezeigt.
 *
 * Threads: `initialize` / `render` / `resize` / `cleanup` und die
 * Audio-Aufrufe kommen vom Render-Thread. Die Steuerung (`setKomposition`,
 * `uhr()`, Auskunft) ruft der GUI-Thread — **unter dem Render-Mutex** der
 * Fassade, wie jeder Zugriff auf einen laufenden Visualizer.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Komposition.hpp"
#include "composer/Uhr.hpp"
#include "visualizers/IVisualizer.hpp"

#include <QElapsedTimer>
#include <QSize>
#include <QStringList>

#include <cstdint>
#include <memory>
#include <vector>

class MultiEffectVisualizer;

namespace lumi::composer
{

class Wiedergabe;

/// Was die Bildquelle ueber ihre Laufzeit mitfuehrt (C2a, Messung). Die Zeiten
/// sind CPU-seitig — wie lange die GL-Aufrufe dauern, nicht wie lange die GPU
/// rechnet; die GPU-Last zeigt die Bildrate bei Frame Mode Unlimited.
struct Messung
{
    int bilder = 0;               ///< wirklich gerechnete Bilder
    double dauerSekunden = 0.0;   ///< vom ersten bis zum letzten gerechneten Bild
    double rechnenSummeMs = 0.0;  ///< Wiedergabe: Clips rechnen und mischen
    double rechnenMaxMs = 0.0;
    std::int64_t rechnenMaxStelleMs = 0;  ///< Stelle des laengsten Bilds
    double erstesBildMs = 0.0;            ///< enthaelt das Anlegen der ersten Renderer
    double spaeterMaxMs = 0.0;            ///< laengstes Bild NACH dem ersten (Ruck beim Vorwaermen)
    std::int64_t spaeterMaxStelleMs = 0;
    double zeichnenSummeMs = 0.0;         ///< das Gemischte ins Ziel
    double zeichnenMaxMs = 0.0;
    int gezeichnet = 0;
    int rendererMax = 0;  ///< hoechste Zahl gleichzeitiger Renderer (samt Karaoke-Traeger)
    /// Freier Grafikspeicher laut Treiber (GL_NVX_gpu_memory_info), in kB;
    /// negativ = der Treiber nennt ihn nicht. Der Wert gilt fuer das ganze
    /// System — andere Programme zaehlen mit.
    int speicherFreiVorherKb = -1;
    int speicherFreiTiefstKb = -1;
    int speicherGesamtKb = -1;
    /// Jedes Bild nach dem ersten, das laenger als `kLangMs` brauchte: Stelle
    /// und Dauer (hoechstens `kLangeMax` Eintraege, danach wird nur gezaehlt).
    static constexpr double kLangMs = 50.0;
    static constexpr int kLangeMax = 40;
    struct LangesBild
    {
        std::int64_t stelleMs = 0;
        double ms = 0.0;
        int renderer = 0;  ///< Renderer nach diesem Bild
    };
    std::vector<LangesBild> lange;
    int langeGesamt = 0;
    QString gpu;      ///< GL_RENDERER — auf welcher GPU gemessen wurde
    QSize groesse;    ///< Bildgroesse des letzten gerechneten Bilds

    [[nodiscard]] double rechnenMittelMs() const { return bilder > 0 ? rechnenSummeMs / bilder : 0.0; }
    [[nodiscard]] double zeichnenMittelMs() const
    {
        return gezeichnet > 0 ? zeichnenSummeMs / gezeichnet : 0.0;
    }
    /// Die Zusammenfassung als Zeilen fuer das Log.
    [[nodiscard]] QStringList alsText() const;
};

class Bildquelle : public IVisualizer
{
public:
    Bildquelle();
    /// Mit eigener Uhr (Test: austauschbare Zeitquelle).
    explicit Bildquelle(Uhr uhr);
    ~Bildquelle() override;
    Bildquelle(const Bildquelle&) = delete;
    Bildquelle& operator=(const Bildquelle&) = delete;

    // --- Steuerung (GUI-Thread, unter dem Render-Mutex) --------------------

    /// Neue Komposition: die Uhr haelt am Anfang, ihre Laenge ist die der
    /// Komposition. Die Renderer der alten werden mit dem naechsten Bild
    /// entladen (das braucht den GL-Kontext).
    void setKomposition(Komposition komposition);
    [[nodiscard]] Uhr& uhr() { return m_uhr; }
    [[nodiscard]] const Uhr& uhr() const { return m_uhr; }

    /// Wohin das Gemischte gezeichnet wird. Vorgabe (negativ): der
    /// Standard-Framebuffer des Kontexts, also das Fenster.
    void setZiel(int framebuffer) { m_ziel = framebuffer; }

    // --- Auskunft (Stand des letzten Bilds) --------------------------------
    [[nodiscard]] std::int64_t letzteStelleMs() const { return m_letzteStelle; }
    [[nodiscard]] int rendererAnzahl() const;
    [[nodiscard]] int gemischteClips() const;
    /// Was beim Richten und beim Anlegen der Renderer auffiel.
    [[nodiscard]] QStringList bericht() const;
    /// Wie viele Bilder wirklich gerechnet wurden (nicht nur noch einmal gezeigt).
    [[nodiscard]] int gerechneteBilder() const { return m_gerechnet; }
    /// Zeiten, Renderer und Grafikspeicher seit dem Anlegen dieser Bildquelle.
    [[nodiscard]] const Messung& messung() const { return m_messung; }
    /// Der Anlauf je Clip als Zeilen fuer das Log: Laden, Anlegen, erstes und
    /// zweites Bild — eine Zeile je angelegtem Renderer der laufenden Komposition.
    [[nodiscard]] QStringList anlaufText() const;
    /// Wie oft Audiodaten an einen Renderer gereicht wurden (Test).
    [[nodiscard]] int debugAudioGereicht() const { return m_audioGereicht; }

    // --- IVisualizer -------------------------------------------------------
    [[nodiscard]] QString visualizerId() const override { return QStringLiteral("composer"); }
    [[nodiscard]] QString visualizerName() const override { return QStringLiteral("Composer"); }
    [[nodiscard]] QString visualizerDescription() const override
    {
        return QStringLiteral("Plays a composition: visual clips on channels, mixed over time");
    }

    void initialize() override;
    void render(float deltaTime) override;
    void resize(const QSize& size) override;
    void cleanup() override;
    [[nodiscard]] bool isInitialized() const override { return m_angelegt; }

    void updateSpectrum(const float* spectrum, int count) override;
    void updateWaveform(const float* waveform, int count) override;
    void updateAudioStereo(const float* specInterleaved, int binsPerCh,
                           const float* waveInterleaved, int frames, int channels) override;

private:
    /// Renderer, die seit dem letzten Bild dazukamen, bekommen die zuletzt
    /// gereichten Audiodaten — sonst warten sie bis zum naechsten Snapshot.
    void versorgeNeue();
    void reicheAlles(MultiEffectVisualizer& renderer);

    Uhr m_uhr;
    std::unique_ptr<Wiedergabe> m_wiedergabe;  ///< lebt zwischen initialize und cleanup
    Komposition m_wartend;                     ///< gesetzt, aber noch nicht uebernommen
    bool m_neueKomposition = false;
    QStringList m_richtBericht;

    QSize m_groesse;
    QSize m_letzteGroesse;
    std::int64_t m_letzteStelle = -1;
    bool m_bildSteht = false;  ///< der Mischer haelt ein gueltiges Bild
    bool m_angelegt = false;
    int m_ziel = -1;
    int m_gerechnet = 0;
    int m_audioGereicht = 0;
    Messung m_messung;
    QElapsedTimer m_laufzeit;  ///< ab dem ersten gerechneten Bild

    // zuletzt gereichte Audiodaten
    std::vector<float> m_spektrum;
    std::vector<float> m_welle;
    std::vector<float> m_spektrumStereo;
    std::vector<float> m_welleStereo;
    int m_stereoBins = 0;
    int m_stereoFrames = 0;
    int m_stereoKanaele = 0;
    std::vector<MultiEffectVisualizer*> m_versorgt;  ///< Renderer des letzten Bilds
};

}  // namespace lumi::composer
