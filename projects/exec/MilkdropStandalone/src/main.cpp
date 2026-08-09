/**
 ****************************************************************************************
 * @file   main.cpp
 * @brief  MilkdropStandalone — isoliertes Testprogramm fuer den C1/C2-Renderpfad
 *         (Session 41): treibt den ECHTEN MilkdropVisualizer in einem eigenen
 *         GL-3.3-Core-Fenster, ohne App-Infrastruktur (kein Docking, kein
 *         Panel, kein Render-Thread) — mit vollem Diagnose-Trace auf Konsole
 *         und in `<TEMP>/lumiviz_milkdrop_trace.log`
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026 (1.1.0: August 2026 — --ab-Wechsellauf mit Frame-Hashes,
 *         --blende (Sicht-Blende des Kerns) + --audio-beat (korrelierte
 *         Band-Huellkurven fuer Beat-Detektor-Presets), S67;
 *         1.2.0: August 2026 — --audio-datei, echte Musik statt Sinus, S74)
 * @version 1.2.0
 *
 * @details
 * Aufruf:
 *   MilkdropStandalone [presetDateiOderOrdner] [--auto] [--frames N]
 *                      [--out DIR] [--size WxH]
 *   MilkdropStandalone A.milk B.milk --ab [--wechsel loeschen|behalten]
 *                      [--frames M] [--ab-frames N] [--audio-neustart]
 *   MilkdropStandalone preset.milk --audio-datei X.mp3 [--audio-start SEK]
 *                      [--audio-gain F] [--audio-stumm]
 *
 * - Ohne Argument wird der c1-Kalibrier-Satz gesucht
 *   (`asset/calibration/milkdrop/c1`, vom Exe-Pfad aufwaerts).
 * - Interaktiv: ←/→ Preset wechseln · R neu laden · S Screenshot ·
 *   G Kalibrier-Raster · Esc beenden.
 * - `--auto`: jedes Preset N Frames rendern (Default 120), Screenshot +
 *   Pixel-Statistik (belegt "schwarz oder nicht"), dann beenden —
 *   Exit-Code 0 nur, wenn alle Presets den Custom-Branch erreicht haben.
 * - `--ab` (S67, Preset-Wechsel-Wächter): erstes Preset --frames M Frames
 *   rendern, dann auf das LETZTE Preset wechseln (dieselbe Visualizer-Instanz
 *   wie in der App; `--wechsel loeschen` ruft vorher requestFeedbackErbe(0) —
 *   exakt der App-Lösch-Pfad) und --ab-frames N Frames je einen FNV-1a-Hash
 *   + Mittel-RGB des Readbacks loggen. Nur EIN Preset ⇒ Kaltstart-Referenz
 *   (sofort hashen). Deterministisch: Audio ist rein m_time-getrieben,
 *   dt fix 1/60 — zwei Läufe mit gleichem M sind bis zum Wechsel bitgleich.
 *
 * - `--audio-datei` (S74): echte Musik statt des synthetischen Signals —
 *   bild-indiziert abgetastet, also weiterhin deterministisch. Interaktiv
 *   laeuft die Datei zusaetzlich HOERBAR mit (im --auto-/--ab-Lauf nicht).
 *   **Nur fuer Schaufenster und Augenschein:** MilkdropRef erzeugt sein Audio
 *   selbst, gegen echte Musik vergleicht man zwei verschiedene Eingaben.
 *
 * Der Visualizer laeuft hier im GUI-Thread (paintGL) — dieselben Methoden,
 * die in der App der Render-Thread aufruft. Audio kommt synthetisch (Sinus +
 * Beat-Puls), damit Waves/Loudness leben — oder aus einer Datei
 * (`--audio-datei`).
 ****************************************************************************************
 */

#include "visualizers/MilkdropVisualizer.hpp"
#include "visualizers/milkdrop/MilkdropTrace.hpp"

#include "AudioDateiQuelle.hpp"
#include "SynthAudio.hpp"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMediaDevices>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLWindow>
#include <QStringList>
#include <QSurfaceFormat>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

namespace
{

/// c1-Kalibrier-Satz vom Exe-Verzeichnis aufwaerts suchen (Repo-Layout)
QString locateCalibrationDir()
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 10; ++i)
    {
        const QString candidate =
            dir.absoluteFilePath(QStringLiteral("asset/calibration/milkdrop/c1"));
        if (QFileInfo::exists(candidate)) return candidate;
        if (!dir.cdUp()) break;
    }
    return {};
}

/// Pixel-Statistik eines RGBA-Readbacks — belegt den Sichtbefund in Zahlen
struct FrameStats
{
    double meanR = 0.0, meanG = 0.0, meanB = 0.0;
    double minLuma = 1.0, maxLuma = 0.0;
    QString describe() const
    {
        return QStringLiteral("mean RGB=(%1, %2, %3), Luma min=%4 max=%5")
            .arg(meanR, 0, 'f', 3)
            .arg(meanG, 0, 'f', 3)
            .arg(meanB, 0, 'f', 3)
            .arg(minLuma, 0, 'f', 3)
            .arg(maxLuma, 0, 'f', 3);
    }
};

FrameStats computeStats(const std::vector<unsigned char>& rgba, int w, int h)
{
    FrameStats st;
    if (w <= 0 || h <= 0) return st;
    double sumR = 0.0, sumG = 0.0, sumB = 0.0;
    const std::size_t pixels = static_cast<std::size_t>(w) * h;
    for (std::size_t p = 0; p < pixels; ++p)
    {
        const double r = rgba[p * 4 + 0] / 255.0;
        const double g = rgba[p * 4 + 1] / 255.0;
        const double b = rgba[p * 4 + 2] / 255.0;
        sumR += r;
        sumG += g;
        sumB += b;
        const double luma = 0.2126 * r + 0.7152 * g + 0.0722 * b;
        st.minLuma = std::min(st.minLuma, luma);
        st.maxLuma = std::max(st.maxLuma, luma);
    }
    st.meanR = sumR / pixels;
    st.meanG = sumG / pixels;
    st.meanB = sumB / pixels;
    return st;
}

class StandaloneWindow : public QOpenGLWindow
{
public:
    StandaloneWindow(QStringList presets, bool autoMode, int autoFrames, QString shotDir,
                     bool silence, bool dumpShaders)
        : m_presets(std::move(presets))
        , m_auto(autoMode)
        , m_autoFrames(autoFrames)
        , m_shotDir(std::move(shotDir))
        , m_silence(silence)
        , m_dumpShaders(dumpShaders)
    {
        setTitle(QStringLiteral("LumiViz MilkdropStandalone"));
    }

    /// A/B-Wechsellauf (S67) aktivieren — Details im Datei-Kopf
    void configureAbRun(bool loeschen, int abFrames, bool audioNeustart)
    {
        m_ab = true;
        m_abLoeschen = loeschen;
        m_abFrames = abFrames;
        m_abAudioNeustart = audioNeustart;
    }

    /// Sicht-Blende des Kerns aktivieren (S67, App-Verhalten nachstellen)
    void setBlende(bool an) { m_blende = an; }

    /// Korrelierte Audio-Huellkurven (S67, Diagnose Beat-Detektor-Presets)
    void setAudioBeat(bool an) { m_audioBeat = an; }
    /// Schlagfolge in Hz (2,0 = 120 BPM). Werte <= 0 werden auf 2,0 geklemmt.
    void setBeatHz(double hz) { m_beatHz = hz > 0.0 ? hz : 2.0; }
    /// Wandernde Spektralbalance + Oberwellen in der Wave.
    void setKlangfarbe(bool an) { m_klangfarbe = an; }

    /// Echte Musik statt des synthetischen Signals (S74). Der Zeiger gehoert
    /// dem Aufrufer und muss das Fenster ueberleben. NICHT fuer
    /// Referenzvergleiche — MilkdropRef erzeugt sein Audio selbst.
    void setAudioQuelle(const lumi::werkzeug::AudioDateiQuelle* quelle)
    {
        m_audioQuelle = quelle;
    }
    /// Die Datei zusaetzlich hoerbar ausgeben (nur interaktiv sinnvoll)
    void setAudioHoerbar(bool an) { m_audioHoerbar = an; }

    /// Muster des synthetischen Signals (S74): klassisch = Sinus+Beat-Puls wie
    /// bisher, musik = aus einer Aufnahme abgeleitete Huellkurven. Beides ist
    /// deterministisch und von MilkdropRef identisch erzeugbar.
    void setMuster(lumi::synth::Muster m) { m_muster = m; }

    /// Exit-Code des --auto-Laufs: 0 nur wenn alle Presets Custom rendern
    [[nodiscard]] bool allCustom() const { return m_allCustom; }

protected:
    void initializeGL() override
    {
        std::printf("[Standalone] GL initialisiert: %s | %s\n",
                    reinterpret_cast<const char*>(
                        context()->functions()->glGetString(GL_VERSION)),
                    reinterpret_cast<const char*>(
                        context()->functions()->glGetString(GL_RENDERER)));
        m_viz = std::make_unique<MilkdropVisualizer>();
        m_viz->initialize();
        m_viz->setSichtBlende(m_blende);
        m_viz->resize(size());
        starteKlangausgabe();
        loadPreset(0);
    }

    void resizeGL(int w, int h) override
    {
        if (m_viz != nullptr) m_viz->resize(QSize(w, h));
    }

    void paintGL() override
    {
        if (m_viz == nullptr) return;
        feedAudio();
        m_viz->render(1.0f / 60.0f);
        ++m_frameInPreset;
        ++m_audioBild;
        m_time += 1.0 / 60.0;

        if (m_ab)
        {
            stepAbRun();
        }
        else if (m_auto && m_frameInPreset >= m_autoFrames)
        {
            finishAutoPreset();
        }
        if (!m_closing) update();  // Dauerschleife (vsync-getaktet)
    }

    bool event(QEvent* ev) override
    {
        // GL-Aufraeumen SOLANGE das Plattform-Fenster noch lebt — danach gibt
        // es keinen current-faehigen Kontext mehr und die GL-Wrapper-Dtoren
        // des Visualizers wuerden ins Leere greifen (Access Violation)
        if (ev->type() == QEvent::Close)
        {
            // Klangausgabe VOR dem GL-Abbau stillegen (Lebenszyklus-Vertrag:
            // erst die Zulieferer stoppen, dann die Verbraucher abbauen)
            beendeKlangausgabe();
        }
        if (ev->type() == QEvent::Close && m_viz != nullptr)
        {
            makeCurrent();
            m_viz->cleanup();
            m_viz.reset();
            doneCurrent();
        }
        return QOpenGLWindow::event(ev);
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        switch (event->key())
        {
        case Qt::Key_Escape: close(); break;
        case Qt::Key_Right: loadPreset(m_index + 1); break;
        case Qt::Key_Left: loadPreset(m_index - 1); break;
        case Qt::Key_R: loadPreset(m_index); break;
        case Qt::Key_S: m_shotRequested = true; update(); break;
        case Qt::Key_G:
            m_grid = !m_grid;
            if (m_viz != nullptr)
            {
                m_viz->setParam("render.debugGrid", lumi::modules::ParamValue{m_grid});
            }
            break;
        default: QOpenGLWindow::keyPressEvent(event); break;
        }
    }

    void paintOverGL() override
    {
        // Screenshot NACH dem Frame (paintGL ist durch, Default-FBO gefuellt)
        if (m_shotRequested)
        {
            m_shotRequested = false;
            saveShot(QStringLiteral("manuell"));
        }
    }

private:
    void loadPreset(int index)
    {
        if (m_presets.isEmpty() || m_viz == nullptr) return;
        m_index = ((index % m_presets.size()) + m_presets.size()) % m_presets.size();
        const QString& path = m_presets[m_index];
        std::printf("\n[Standalone] === Preset %d/%d: %s ===\n", m_index + 1,
                    static_cast<int>(m_presets.size()),
                    qPrintable(QFileInfo(path).fileName()));

        QStringList report;
        const bool ok = m_viz->loadMilkFile(path, &report);
        for (const QString& line : report)
        {
            std::printf("[Import] %s\n", qPrintable(line));
        }
        if (!ok) std::printf("[Standalone] LADEN FEHLGESCHLAGEN\n");
        std::printf("[Standalone] warpCustomSrc=%zu Zeichen, compCustomSrc=%zu Zeichen\n",
                    m_viz->warpCustomSource().size(), m_viz->compCustomSource().size());
        if (m_dumpShaders) dumpShaderSources(path);
        setTitle(QStringLiteral("MilkdropStandalone — %1")
                     .arg(QFileInfo(path).completeBaseName()));
        m_frameInPreset = 0;
        update();
    }

    /// Uebersetzte GLSL-Quellen neben die Screenshots legen (Kalibrier-Diagnose)
    void dumpShaderSources(const QString& presetPath)
    {
        QDir().mkpath(m_shotDir);
        const QString base = m_shotDir + QStringLiteral("/") +
                             QFileInfo(presetPath).completeBaseName();
        const auto write = [](const QString& file, const std::string& src) {
            if (src.empty()) return;
            QFile f(file);
            if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
                f.write(src.data(), static_cast<qint64>(src.size()));
        };
        write(base + QStringLiteral("_warp.glsl"), m_viz->warpCustomSource());
        write(base + QStringLiteral("_comp.glsl"), m_viz->compCustomSource());
        std::printf("[Standalone] Shader-Dump: %s_{warp,comp}.glsl\n", qPrintable(base));
    }

    /// Klangausgabe der Audiodatei starten (nur wenn ausdruecklich gewuenscht
    /// und ein Ausgabegeraet passt). Scheitert das, laeuft der Lauf stumm
    /// weiter — das Bild haengt nicht an der Tonausgabe.
    void starteKlangausgabe()
    {
        if (!m_audioHoerbar || m_audioQuelle == nullptr || !m_audioQuelle->bereit()) return;
        QAudioFormat fmt;
        fmt.setSampleRate(lumi::werkzeug::AudioDateiQuelle::kAbtastrate);
        fmt.setChannelCount(lumi::werkzeug::AudioDateiQuelle::kKanaele);
        fmt.setSampleFormat(QAudioFormat::Float);
        const QAudioDevice geraet = QMediaDevices::defaultAudioOutput();
        if (geraet.isNull() || !geraet.isFormatSupported(fmt))
        {
            std::printf("[Audio] kein passendes Ausgabegeraet — Lauf bleibt stumm\n");
            std::fflush(stdout);
            return;
        }
        m_geber = std::make_unique<lumi::werkzeug::PcmGeber>(*m_audioQuelle);
        m_geber->open(QIODevice::ReadOnly);
        m_senke = std::make_unique<QAudioSink>(geraet, fmt);
        m_senke->start(m_geber.get());
        // Ausspuelen: der interaktive Lauf endet oft per Abbruch, und dann
        // bliebe die Zeile im Puffer stehen — genau die will man aber sehen
        std::printf("[Audio] Klangausgabe laeuft (%s)\n", qPrintable(geraet.description()));
        std::fflush(stdout);
    }

    void beendeKlangausgabe()
    {
        if (m_senke != nullptr) m_senke->stop();
        m_senke.reset();
        if (m_geber != nullptr) m_geber->close();
        m_geber.reset();
    }

    /// Audio-Weiche: echte Datei, wenn eine geladen ist — sonst das
    /// synthetische Signal. `--silence` schlaegt beides (Hunger-Test).
    void feedAudio()
    {
        if (m_audioQuelle != nullptr && m_audioQuelle->bereit() && !m_silence)
        {
            static std::vector<float> wave(
                lumi::werkzeug::AudioDateiQuelle::kWaveFrames * 2);
            static std::vector<float> spec(lumi::werkzeug::AudioDateiQuelle::kBins * 2);
            m_audioQuelle->frameFuellen(m_audioBild, wave.data(), spec.data());
            m_viz->updateAudioStereo(spec.data(), lumi::werkzeug::AudioDateiQuelle::kBins,
                                     wave.data(),
                                     lumi::werkzeug::AudioDateiQuelle::kWaveFrames, 2);
            return;
        }
        feedSyntheticAudio();
    }

    /// Synthetisches Signal aus dem GEMEINSAMEN Erzeuger (S74). Die Formel
    /// stand bis dahin viermal im Baum — hier, im AvsStandalone und in beiden
    /// Referenz-Werkzeugen, jedes Mal als Kopie. Jetzt bindet jeder dieselbe
    /// `SynthAudio.hpp` ein; Auseinanderlaufen ist baulich ausgeschlossen.
    /// Vorgaben ergeben Bit fuer Bit das alte Signal — auch --klangfarbe,
    /// --audio-beat und --beat-hz sind unveraendert mit uebergesiedelt.
    void feedSyntheticAudio()
    {
        lumi::synth::Optionen opt;
        opt.muster = m_muster;
        // MilkDrop-Geschmack: Bass/Mitten/Hoehen bekommen EIGENE Huellkurven.
        // Ein gemeinsamer Faktor machte sie nach der Loudness-Normalisierung
        // identisch, und Presets, die durch Banddifferenzen teilen, liefen in
        // 0/0-NaN (S64).
        opt.geschmack = lumi::synth::Geschmack::Milkdrop;
        opt.beatHz = m_beatHz;
        opt.klangfarbe = m_klangfarbe;
        opt.audioBeat = m_audioBeat;
        opt.stille = m_silence;
        static lumi::synth::Frame klang;
        lumi::synth::erzeuge(m_audioBild, opt, klang);
        m_viz->updateAudioStereo(klang.spec, lumi::synth::kBins, klang.wave,
                                 lumi::synth::kWaveFrames, 2);
    }

    FrameStats saveShot(const QString& tag)
    {
        const int w = std::max(1, width());
        const int h = std::max(1, height());
        std::vector<unsigned char> rgba(static_cast<std::size_t>(w) * h * 4);
        QOpenGLFunctions* f = context()->functions();
        f->glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());
        f->glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

        const FrameStats stats = computeStats(rgba, w, h);
        QImage img(rgba.data(), w, h, w * 4, QImage::Format_RGBA8888);
        const QString base = m_presets.isEmpty()
                                 ? QStringLiteral("frame")
                                 : QFileInfo(m_presets[m_index]).completeBaseName();
        QDir().mkpath(m_shotDir);
        const QString file = QStringLiteral("%1/%2_%3.png").arg(m_shotDir, base, tag);
        // glReadPixels liefert Zeile 0 = unten → fuer die PNG-Ansicht spiegeln
        img.mirrored().save(file);
        std::printf("[Standalone] Screenshot: %s — %s\n", qPrintable(file),
                    qPrintable(stats.describe()));
        return stats;
    }

    /// A/B-Wächter (S67): Phase 0 = Vorlauf-Preset, Phase 1 = Ziel-Preset mit
    /// Hash je Frame. Bei nur EINEM Preset startet Phase 1 sofort (Kaltstart-
    /// Referenz desselben Messformats).
    void stepAbRun()
    {
        if (m_abPhase == 0)
        {
            if (m_presets.size() < 2)
            {
                m_abPhase = 1;  // Kaltstart-Lauf: sofort messen (dieser Frame zaehlt)
            }
            else if (m_frameInPreset >= m_autoFrames)
            {
                std::printf("[AB] Wechsel nach %d Frames: '%s' -> '%s' (Modus %s%s)\n",
                            m_frameInPreset,
                            qPrintable(QFileInfo(m_presets[m_index]).completeBaseName()),
                            qPrintable(QFileInfo(m_presets.last()).completeBaseName()),
                            m_abLoeschen ? "Loeschen" : "Behalten",
                            m_abAudioNeustart ? ", Audio-Uhr neu" : "");
                // App-Reihenfolge (runMilkdropNode): erst requestFeedbackErbe
                // (setzt bei keep=0 auch den rand_preset-Seed zurueck), DANN
                // applyPresetState via loadMilkFile; der Puffer-Wipe folgt im
                // naechsten render() — exakt der Loesch-Pfad der App
                if (m_abLoeschen) m_viz->requestFeedbackErbe(0.0);
                if (m_abAudioNeustart)
                {
                    m_time = 0.0;
                    m_audioBild = 0;  // gilt fuer beide Quellen (S74)
                }
                m_abPhase = 1;
                loadPreset(static_cast<int>(m_presets.size()) - 1);
                return;  // Frame 1 des Ziel-Presets hasht der naechste paintGL
            }
            else
            {
                return;
            }
        }

        FrameStats stats;
        const quint64 hash = frameHash(&stats);
        std::printf("[AB] f%04d hash=%016llx rgb=(%.4f, %.4f, %.4f)\n", m_frameInPreset,
                    static_cast<unsigned long long>(hash), stats.meanR, stats.meanG,
                    stats.meanB);
        // Bild-Kontrollpunkte fuer den Sichtvergleich alt/neu
        if (m_frameInPreset == 1 || m_frameInPreset == 30 || m_frameInPreset == 120 ||
            m_frameInPreset == m_abFrames)
        {
            saveShot(QStringLiteral("ab_f%1").arg(m_frameInPreset, 4, 10, QLatin1Char('0')));
        }
        if (m_frameInPreset >= m_abFrames)
        {
            std::printf("[AB] fertig (%d Mess-Frames).\n", m_abFrames);
            std::fflush(stdout);
            m_closing = true;
            QTimer::singleShot(0, this, &QWindow::close);
        }
    }

    /// FNV-1a-64 ueber den kompletten RGBA-Readback des aktuellen Frames
    quint64 frameHash(FrameStats* statsOut)
    {
        const int w = std::max(1, width());
        const int h = std::max(1, height());
        std::vector<unsigned char> rgba(static_cast<std::size_t>(w) * h * 4);
        QOpenGLFunctions* f = context()->functions();
        f->glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());
        f->glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
        if (statsOut != nullptr) *statsOut = computeStats(rgba, w, h);
        quint64 hv = 14695981039346656037ULL;
        for (const unsigned char b : rgba)
        {
            hv ^= b;
            hv *= 1099511628211ULL;
        }
        return hv;
    }

    void finishAutoPreset()
    {
        const FrameStats stats = saveShot(QStringLiteral("auto"));
        // Custom-Pfad nur erwarten, wenn das Preset auch Custom-Shader HAT
        // (MD1-/Sprite-Presets laufen zu Recht ohne — kein Gate-Fehler)
        const auto& ps = m_viz->presetState();
        const bool expectCustom =
            ps.warpInfo.shaderClass == lumi::milk::ShaderClass::Custom ||
            ps.compInfo.shaderClass == lumi::milk::ShaderClass::Custom;
        const bool customActive =
            !m_viz->warpCustomSource().empty() || !m_viz->compCustomSource().empty();
        const bool looksBlack = stats.maxLuma < 0.02;
        std::printf("[Standalone] Ergebnis %s: custom=%s%s, GL-Fehler=%s, schwarz=%s\n",
                    qPrintable(QFileInfo(m_presets[m_index]).fileName()),
                    customActive ? "ja" : (expectCustom ? "NEIN(!)" : "nein"),
                    expectCustom ? "" : " (nicht erwartet)",
                    m_viz->customGlError().empty() ? "keiner"
                                                   : qPrintable(QString::fromStdString(
                                                         m_viz->customGlError()).left(120)),
                    looksBlack ? "JA(!)" : "nein");
        if ((expectCustom && !customActive) || !m_viz->customGlError().empty())
        {
            m_allCustom = false;
        }

        if (m_index + 1 < m_presets.size())
        {
            loadPreset(m_index + 1);
        }
        else
        {
            std::printf("\n[Standalone] --auto abgeschlossen (%d Presets), Ende.\n",
                        static_cast<int>(m_presets.size()));
            std::fflush(stdout);
            // NICHT direkt aus paintGL schliessen — Fenster-Teardown mitten im
            // Paint-Event crasht in Qt6Gui; erst den Event ausrollen lassen
            m_closing = true;
            QTimer::singleShot(0, this, &QWindow::close);
        }
    }

    std::unique_ptr<MilkdropVisualizer> m_viz;
    QStringList m_presets;
    int m_index = 0;
    bool m_auto = false;
    int m_autoFrames = 120;
    QString m_shotDir;
    int m_frameInPreset = 0;
    double m_time = 0.0;
    bool m_grid = false;
    bool m_shotRequested = false;
    bool m_allCustom = true;
    bool m_closing = false;
    bool m_silence = false;
    bool m_dumpShaders = false;
    bool m_blende = false;          ///< Sicht-Blende (S67, App-Verhalten)
    bool m_audioBeat = false;       ///< korrelierte Huellkurven (S67, Diagnose)
    double m_beatHz = 2.0;          ///< Schlagfolge (S73), 2,0 = 120 BPM
    bool m_klangfarbe = false;      ///< wandernde Spektralbalance (S73)
    bool m_ab = false;              ///< A/B-Wechsellauf (S67)
    bool m_abLoeschen = true;       ///< Wechselmodus: Loeschen (sonst Behalten)
    bool m_abAudioNeustart = false; ///< Audio-Uhr beim Wechsel auf 0
    int m_abFrames = 300;           ///< Mess-Frames nach dem Wechsel
    int m_abPhase = 0;              ///< 0 = Vorlauf, 1 = Messphase
    /// Echte Musik als Audioquelle (S74) — Eigentum des Aufrufers
    const lumi::werkzeug::AudioDateiQuelle* m_audioQuelle = nullptr;
    /// Bild-Zaehler der Audio-Zufuhr; ueber Preset-Wechsel hinweg fortlaufend
    /// (m_frameInPreset startet je Preset neu und taugt dafuer nicht)
    std::int64_t m_audioBild = 0;
    bool m_audioHoerbar = false;
    std::unique_ptr<lumi::werkzeug::PcmGeber> m_geber;
    std::unique_ptr<QAudioSink> m_senke;
    /// Muster des synthetischen Signals (S74); Vorgabe = das alte Verhalten
    lumi::synth::Muster m_muster = lumi::synth::Muster::Klassisch;
};

} // namespace

int main(int argc, char* argv[])
{
    // Messwerkzeug: 1 logischer Pixel = 1 Framebuffer-Pixel — sonst rendert der
    // Visualizer bei Windows-Skalierung (z. B. 150 %) nur ins linke untere
    // Teilrechteck des groesseren Fensters und --size waere display-abhaengig
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("MilkdropStandalone"));
    lumi::milkdrop::trace::setEcho(true);  // Trace zusaetzlich auf die Konsole

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Isolierter C1/C2-Renderpfad-Test (Session 41)"));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("preset"),
                                 QStringLiteral(".milk-Datei oder Preset-Ordner "
                                                "(Default: c1-Kalibrier-Satz)"));
    const QCommandLineOption optAuto(QStringLiteral("auto"),
                                     QStringLiteral("Batch: alle Presets rendern, "
                                                    "Screenshots + Statistik, beenden"));
    const QCommandLineOption optFrames(QStringLiteral("frames"),
                                       QStringLiteral("Frames je Preset im --auto-Modus"),
                                       QStringLiteral("N"), QStringLiteral("120"));
    const QCommandLineOption optOut(QStringLiteral("out"),
                                    QStringLiteral("Screenshot-Verzeichnis"),
                                    QStringLiteral("DIR"),
                                    QDir::tempPath() + QStringLiteral("/lumiviz_standalone"));
    const QCommandLineOption optSize(QStringLiteral("size"),
                                     QStringLiteral("Fenstergroesse WxH"),
                                     QStringLiteral("WxH"), QStringLiteral("800x600"));
    const QCommandLineOption optSilence(
        QStringLiteral("silence"),
        QStringLiteral("Stille statt synthetischem Audio (Hunger-Test)"));
    const QCommandLineOption optDumpShaders(
        QStringLiteral("dump-shaders"),
        QStringLiteral("uebersetzte GLSL-Quellen nach --out schreiben"));
    const QCommandLineOption optSeed(
        QStringLiteral("seed"),
        QStringLiteral("Kaltstart-Saat aktivieren (App-Verhalten). Default ist "
                       "SAATLOS wie MilkdropRef mit genulltem WDDM-VRAM "
                       "(Entscheid Patrik S64)"));
    const QCommandLineOption optAb(
        QStringLiteral("ab"),
        QStringLiteral("A/B-Wechsellauf (S67): erstes Preset --frames Frames, dann "
                       "Wechsel aufs letzte Preset, --ab-frames Frames hashen"));
    const QCommandLineOption optAbFrames(QStringLiteral("ab-frames"),
                                         QStringLiteral("Mess-Frames nach dem Wechsel"),
                                         QStringLiteral("N"), QStringLiteral("300"));
    const QCommandLineOption optWechsel(
        QStringLiteral("wechsel"),
        QStringLiteral("Wechselmodus im --ab-Lauf: loeschen|behalten"),
        QStringLiteral("MODUS"), QStringLiteral("loeschen"));
    const QCommandLineOption optAudioNeustart(
        QStringLiteral("audio-neustart"),
        QStringLiteral("Audio-Uhr beim --ab-Wechsel auf 0 setzen (Kaltstart-Vergleich)"));
    const QCommandLineOption optBlende(
        QStringLiteral("blende"),
        QStringLiteral("Sicht-Blende aktivieren (~0,5 s Schwarz-Einblendung "
                       "nach frischer Saat — App-Verhalten)"));
    const QCommandLineOption optAudioBeat(
        QStringLiteral("audio-beat"),
        QStringLiteral("korrelierte Band-Huellkurven mit Jitter (Diagnose: "
                       "Beat-Detektor-Presets wie pixies feuern nur, wenn "
                       "alle Baender gemeinsam pulsieren)"));
    parser.addOption(optAuto);
    parser.addOption(optFrames);
    parser.addOption(optOut);
    parser.addOption(optSize);
    parser.addOption(optSilence);
    parser.addOption(optDumpShaders);
    parser.addOption(optSeed);
    parser.addOption(optAb);
    parser.addOption(optAbFrames);
    parser.addOption(optWechsel);
    parser.addOption(optAudioNeustart);
    parser.addOption(optBlende);
    const QCommandLineOption optBeatHz(
        QStringLiteral("beat-hz"),
        QStringLiteral("Schlagfolge des synthetischen Audios in Hz "
                       "(Vorgabe 2 = 120 BPM; 4 oder 6 fuer dichte Beats)"),
        QStringLiteral("HZ"), QStringLiteral("2"));
    const QCommandLineOption optKlangfarbe(
        QStringLiteral("klangfarbe"),
        QStringLiteral("wandernde Spektralbalance (Bass<->Hoehen) und "
                       "Oberwellen in der Wave — sonst klingt jeder Frame "
                       "gleich gefaerbt und das Bild bleibt eintoenig"));
    // --- Echte Musik als Audioquelle (S74, Aufgabe 6) -----------------------------------
    const QCommandLineOption optAudioDatei(
        QStringLiteral("audio-datei"),
        QStringLiteral("Audiodatei (MP3/WAV/FLAC/...) statt des synthetischen "
                       "Signals. NUR fuer Schaufenster und Augenschein — "
                       "MilkdropRef erzeugt sein Audio selbst, jeder "
                       "Referenzvergleich mit echter Musik ist wertlos"),
        QStringLiteral("PFAD"));
    const QCommandLineOption optAudioStart(
        QStringLiteral("audio-start"),
        QStringLiteral("Startversatz in der Audiodatei in Sekunden"),
        QStringLiteral("SEK"), QStringLiteral("0"));
    const QCommandLineOption optAudioGain(
        QStringLiteral("audio-gain"),
        QStringLiteral("Faktor auf Wellenform und Spektrum der Audiodatei "
                       "(Vorgabe 1). Echte Musik ist deutlich leiser als das "
                       "synthetische Signal"),
        QStringLiteral("F"), QStringLiteral("1"));
    const QCommandLineOption optAudioStumm(
        QStringLiteral("audio-stumm"),
        QStringLiteral("die Audiodatei NICHT hoerbar ausgeben (im --auto- und "
                       "--ab-Lauf ohnehin die Vorgabe)"));
    parser.addOption(optAudioBeat);
    parser.addOption(optBeatHz);
    parser.addOption(optKlangfarbe);
    const QCommandLineOption optMuster(
        QStringLiteral("audio-muster"),
        QStringLiteral("Muster des SYNTHETISCHEN Signals: klassisch|musik. "
                       "klassisch = 220-Hz-Sinus + Beat-Puls (Vorgabe, "
                       "bit-identisch seit S41) · musik = aus einer Aufnahme "
                       "abgeleitete Bandhuellkurven, neu synthetisiert. BEIDE "
                       "sind deterministisch und von MilkdropRef identisch "
                       "erzeugbar — anders als --audio-datei"),
        QStringLiteral("NAME"), QStringLiteral("klassisch"));
    parser.addOption(optAudioDatei);
    parser.addOption(optAudioStart);
    parser.addOption(optAudioGain);
    parser.addOption(optAudioStumm);
    parser.addOption(optMuster);
    parser.process(app);

    lumi::synth::Muster muster = lumi::synth::Muster::Klassisch;
    if (!lumi::synth::musterAusText(qPrintable(parser.value(optMuster)), muster))
    {
        std::fprintf(stderr, "FEHLER: --audio-muster erwartet klassisch|musik\n");
        return 2;
    }
    if (muster == lumi::synth::Muster::Musik &&
        lumi::synth::eingebautesProfil().bilder <= 0)
    {
        std::fprintf(stderr, "FEHLER: kein Musik-Profil eingebaut — erst mit "
                             "AvsStandalone --audio-profil-schreiben erzeugen\n");
        return 2;
    }
    // Saatlos = Prüfstand-Vertrag: derselbe Kaltstart wie der Referenz-Renderer.
    // Die App behält ihre Saat (Verstärker-Presets beim ERSTEN Preset der
    // Sitzung); hier zählt Vergleichbarkeit.
    if (!parser.isSet(optSeed)) qputenv("LUMIVIZ_MILKDROP_NOSEED", "1");

    // --- Preset-Liste aufbauen (mehrere Pfade erlaubt — der --ab-Lauf braucht A und B)
    QStringList targets = parser.positionalArguments();
    if (targets.isEmpty()) targets << locateCalibrationDir();
    if (targets.first().isEmpty())
    {
        std::fprintf(stderr,
                     "FEHLER: kein Preset angegeben und c1-Kalibrier-Satz nicht gefunden\n");
        return 2;
    }
    QStringList presets;
    for (const QString& target : targets)
    {
        const QFileInfo info(target);
        if (info.isDir())
        {
            const QDir dir(target);
            for (const QString& f :
                 dir.entryList({QStringLiteral("*.milk")}, QDir::Files, QDir::Name))
            {
                presets << dir.absoluteFilePath(f);
            }
        }
        else if (info.isFile())
        {
            presets << info.absoluteFilePath();
        }
    }
    if (presets.isEmpty())
    {
        std::fprintf(stderr, "FEHLER: keine .milk-Presets unter '%s'\n",
                     qPrintable(targets.join(QStringLiteral("' '"))));
        return 2;
    }
    std::printf("[Standalone] %d Preset(s) | Trace: %s\n",
                static_cast<int>(presets.size()),
                qPrintable(lumi::milkdrop::trace::filePath()));

    // --- Fenster: GL 3.3 Core wie die App --------------------------------------------------
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(fmt);

    const QStringList wh = parser.value(optSize).split(QLatin1Char('x'));
    const int w = (wh.size() == 2) ? std::max(64, wh[0].toInt()) : 800;
    const int h = (wh.size() == 2) ? std::max(64, wh[1].toInt()) : 600;

    StandaloneWindow window(presets, parser.isSet(optAuto), parser.value(optFrames).toInt(),
                            parser.value(optOut), parser.isSet(optSilence),
                            parser.isSet(optDumpShaders));
    if (parser.isSet(optAb))
    {
        const QString modus = parser.value(optWechsel).toLower();
        if (modus != QStringLiteral("loeschen") && modus != QStringLiteral("behalten"))
        {
            std::fprintf(stderr, "FEHLER: --wechsel erwartet loeschen|behalten\n");
            return 2;
        }
        window.configureAbRun(modus == QStringLiteral("loeschen"),
                              std::max(1, parser.value(optAbFrames).toInt()),
                              parser.isSet(optAudioNeustart));
    }
    window.setBlende(parser.isSet(optBlende));
    window.setAudioBeat(parser.isSet(optAudioBeat));
    window.setBeatHz(parser.value(optBeatHz).toDouble());
    window.setKlangfarbe(parser.isSet(optKlangfarbe));
    window.setMuster(muster);
    if (muster == lumi::synth::Muster::Musik)
    {
        const lumi::synth::MusikProfil& p = lumi::synth::eingebautesProfil();
        std::printf("[Audio] Muster musik: %s, %d Bilder Schleife (%.2f s)\n", p.herkunft,
                    p.bilder, p.bilder / lumi::synth::kBildrate);
    }

    // --- Audiodatei laden, BEVOR das GL-Fenster steht (Dekodieren dauert) ---------------
    lumi::werkzeug::AudioDateiQuelle audioQuelle;
    if (parser.isSet(optAudioDatei))
    {
        QString fehler;
        audioQuelle.setVerstaerkung(parser.value(optAudioGain).toDouble());
        if (!audioQuelle.laden(parser.value(optAudioDatei), &fehler))
        {
            std::fprintf(stderr, "FEHLER: Audiodatei nicht nutzbar — %s\n",
                         qPrintable(fehler));
            return 2;
        }
        audioQuelle.setStartSekunden(parser.value(optAudioStart).toDouble());
        std::printf("[Audio] '%s' geladen: %.1f s, Start bei %.1f s, Gain %.2f\n",
                    qPrintable(audioQuelle.quelle()), audioQuelle.dauerSekunden(),
                    parser.value(optAudioStart).toDouble(),
                    parser.value(optAudioGain).toDouble());
        // Unuebersehbar: mit echter Musik ist JEDER Referenzvergleich hinfaellig
        std::printf("[Audio] ACHTUNG: echte Musik — NICHT fuer Referenzvergleiche "
                    "gegen MilkdropRef (dessen Audio ist synthetisch)\n");
        window.setAudioQuelle(&audioQuelle);
        // Hoerbar nur im interaktiven Lauf: Stapel- und A/B-Laeufe sollen
        // deterministisch und still durchlaufen
        const bool stapel = parser.isSet(optAuto) || parser.isSet(optAb);
        const bool hoerbar = !stapel && !parser.isSet(optAudioStumm);
        window.setAudioHoerbar(hoerbar);
        std::printf("[Audio] hoerbare Ausgabe: %s\n",
                    hoerbar ? "ja" : (stapel ? "nein (Stapellauf)" : "nein (--audio-stumm)"));
        // Ausspuelen: der interaktive Lauf endet meist per Abbruch, und dann
        // bliebe der ganze Audio-Block im Puffer stehen
        std::fflush(stdout);
    }
    window.resize(w, h);
    window.show();

    const int rc = app.exec();
    if (parser.isSet(optAuto) && !window.allCustom())
    {
        std::fprintf(stderr, "[Standalone] MINDESTENS EIN PRESET OHNE CUSTOM-PFAD\n");
        return 1;
    }
    return rc;
}
