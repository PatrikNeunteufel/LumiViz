/**
 ****************************************************************************************
 * @file   main.cpp
 * @brief  AvsStandalone — isoliertes Testprogramm fuer den AVS-Renderpfad
 *         (Session 43, Vorbild MilkdropStandalone): treibt den ECHTEN
 *         MultiEffectVisualizer (loadAvsFile → uebersetzte Chain) in einem
 *         eigenen GL-3.3-Core-Fenster, ohne App-Infrastruktur (kein Docking,
 *         kein Panel, kein Render-Thread)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026 (1.1.0: August 2026 — --audio-datei, echte Musik statt
 *         Sinus, S74)
 * @version 1.1.0
 *
 * @details
 * Aufruf:
 *   AvsStandalone [presetDateiOderOrdner] [--auto] [--frames N]
 *                 [--out DIR] [--size WxH] [--render-scale N]
 *   AvsStandalone preset.avs --audio-datei X.mp3 [--audio-start SEK]
 *                 [--audio-gain F] [--audio-stumm]
 *
 * - `--render-scale N` (S73): Divisor des Import-Render-Scale-Knotens, das
 *   Gegenstueck zur App-Einstellung `import/avsRenderScaleDivisor`. **Bis S73
 *   fehlte er hier ganz** — der Standalone rendert dann ungeskaliert, und
 *   klassische AVS-Presets (feste Pixelgroessen) zerfallen bei grossen
 *   Fenstern in Streifen. Bei den ueblichen kleinen Fenstern faellt das nicht
 *   auf, weshalb die Luecke lange unbemerkt blieb. Wer das Bild der App
 *   nachstellen will, MUSS denselben Divisor setzen wie dort.
 *
 * - Ohne Argument wird der Referenz-Korpus gesucht
 *   (`../ref/vis_avs/avs/vis_avs/presets`, vom Exe-Pfad aufwaerts).
 * - Interaktiv: ←/→ Preset wechseln · R neu laden · S Screenshot · Esc beenden.
 * - `--auto`: jedes Preset N Frames rendern (Default 120), Screenshot +
 *   Pixel-Statistik, Import-Report auf Konsole (Zeilen OHNE ℹ-Praefix sind
 *   Warnungen) — Exit-Code 0 nur, wenn alle Presets geladen haben.
 *
 * - `--audio-datei` (S74): echte Musik statt des synthetischen Signals —
 *   bild-indiziert abgetastet, also weiterhin deterministisch. Interaktiv
 *   laeuft die Datei zusaetzlich HOERBAR mit (im --auto-Lauf nicht).
 *   **Nur fuer Schaufenster und Augenschein:** AvsRef erzeugt sein Audio
 *   selbst, gegen echte Musik vergleicht man zwei verschiedene Eingaben.
 *
 * Der Host laeuft hier im GUI-Thread (paintGL) — dieselben Methoden, die in
 * der App der Render-Thread aufruft. Audio kommt synthetisch (Sinus +
 * Beat-Puls), damit Scopes/Beat-Effekte leben — oder aus einer Datei
 * (`--audio-datei`).
 ****************************************************************************************
 */

#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "services/LiveVideoFeed.hpp"  // Kamera-Freigabe + Teardown-Messung (S71)

#include "AudioDateiQuelle.hpp"
#include "SynthAudio.hpp"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QJsonDocument>
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
#include <cstdlib>
#include <memory>
#include <vector>

namespace
{

constexpr double kPi = 3.14159265358979323846;

/// Alle aus APEs uebersetzten Knoten abschalten; liefert deren Anzahl.
int disableApeNodes(lumi::multieffect::ChainNode& node)
{
    int count = 0;
    if (node.fromApe && node.enabled)
    {
        node.enabled = false;
        ++count;
    }
    for (lumi::multieffect::ChainNode& child : node.children)
        count += disableApeNodes(child);
    return count;
}

/// Referenz-Korpus vom Exe-Verzeichnis aufwaerts suchen (../ref neben dem Repo)
QString locateCorpusDir()
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 10; ++i)
    {
        const QString candidate = dir.absoluteFilePath(
            QStringLiteral("../ref/vis_avs/avs/vis_avs/presets"));
        if (QFileInfo::exists(candidate)) return QDir(candidate).absolutePath();
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
    StandaloneWindow(QStringList presets, bool autoMode, int autoFrames, QString shotDir)
        : m_presets(std::move(presets))
        , m_auto(autoMode)
        , m_autoFrames(autoFrames)
        , m_shotDir(std::move(shotDir))
    {
        setTitle(QStringLiteral("LumiViz AvsStandalone"));
    }

    /// Exit-Code des --auto-Laufs: 0 nur wenn alle Presets geladen haben
    [[nodiscard]] bool allLoaded() const { return m_allLoaded; }

    /// --dump: uebersetzte Chain nach dem Laden als JSON ausgeben
    void setDumpChain(bool on) { m_dumpChain = on; }
    /// Linken und rechten Spektrumkanal unterschiedlich fuellen (s.
    /// feedSyntheticAudio). Aus = Vorgabe, damit bestehende Messungen gelten.
    void setStereoSpektrum(bool on) { m_stereoSpektrum = on; }

    /// --edit-nach DATEI: nach der HAELFTE der Frames die Parameter aus DATEI
    /// uebernehmen — ohne Runtime-Reset, also genau so, wie das Panel ein Feld
    /// aendert (`recompileChain()` == `compileChain(m_root)`; nur `loadChainFile`
    /// setzt `m_pendingRuntimeReset`). Damit laesst sich messen, ob ein Feld
    /// beim EDITIEREN dasselbe tut wie nach Speichern+Laden (Verdacht Patrik,
    /// S55). Die Struktur beider Presets muss gleich sein — sonst waere es
    /// kein Feld-Edit, sondern ein Preset-Wechsel.
    void setEditNach(QString datei) { m_editNach = std::move(datei); }

    /// --save-every M: im --auto-Lauf jeden M-ten Frame speichern (Frame-
    /// Zaehlung wie AvsRef: 0-basiert, Dateiname f%04d 1-basiert) — fuer
    /// Sequenz-/GIF-Vergleiche gegen den Referenz-Renderer (S46)
    void setSaveEvery(int every) { m_saveEvery = every; }

    /// --beat-period N: deterministischer Beat alle N Frames statt des
    /// Detektors — Gegenstueck zu AvsRef --beat-period (frame-exakte Diffs)
    void setBeatPeriod(int frames) { m_beatPeriod = frames; }
    /// Divisor des Import-Render-Scale-Knotens (1 = neutral) — die App holt
    /// ihn aus `import/avsRenderScaleDivisor`, hier kommt er per Schalter.
    void setRenderScale(int divisor) { m_renderScale = divisor > 0 ? divisor : 1; }

    /// --no-ape: aus APEs uebersetzte Knoten abschalten (AvsRef kennt sie nicht)
    void setNoApe(bool on) { m_noApe = on; }

    /// Echte Musik statt des synthetischen Signals (S74). Der Zeiger gehoert
    /// dem Aufrufer und muss das Fenster ueberleben. NICHT fuer
    /// Referenzvergleiche — AvsRef erzeugt sein Audio selbst.
    void setAudioQuelle(const lumi::werkzeug::AudioDateiQuelle* quelle)
    {
        m_audioQuelle = quelle;
    }
    /// Die Datei zusaetzlich hoerbar ausgeben (nur interaktiv sinnvoll)
    void setAudioHoerbar(bool an) { m_audioHoerbar = an; }

    /// Muster des synthetischen Signals (S74): klassisch = Sinus+Beat-Puls wie
    /// bisher, musik = aus einer Aufnahme abgeleitete Huellkurven. Beides ist
    /// deterministisch und auf der Referenz-Seite identisch erzeugbar.
    void setMuster(lumi::synth::Muster m) { m_muster = m; }

protected:
    void initializeGL() override
    {
        std::printf("[Standalone] GL initialisiert: %s | %s\n",
                    reinterpret_cast<const char*>(
                        context()->functions()->glGetString(GL_VERSION)),
                    reinterpret_cast<const char*>(
                        context()->functions()->glGetString(GL_RENDERER)));
        m_viz = std::make_unique<MultiEffectVisualizer>();
        m_viz->initialize();
        m_viz->resize(size());
        m_viz->setBeatPeriodOverride(m_beatPeriod);
        // Im Musik-Muster liefert die Beat-SPUR der Vorlage den Beat — die
        // Referenz-Seite rechnet mit demselben Feld (S74). Ein ausdrueckliches
        // --beat-period schlaegt sie aber: nur mit konstantem Beat laesst sich
        // messen, ob ein Befund am Renderpfad haengt oder am Pruefsignal.
        if (m_muster == lumi::synth::Muster::Musik && m_beatPeriod <= 0)
        {
            const lumi::synth::MusikProfil& p = lumi::synth::eingebautesProfil();
            if (p.bilder > 0) m_viz->setBeatTrackOverride(p.beats, p.bilder);
        }
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
        if (m_auto && m_saveEvery > 0 && m_frameInPreset % m_saveEvery == 0)
        {
            saveShot(QStringLiteral("f%1").arg(m_frameInPreset + 1, 4, 10,
                                               QLatin1Char('0')));
        }
        ++m_frameInPreset;
        ++m_audioBild;
        m_time += 1.0 / 60.0;
        if (!m_editNach.isEmpty() && !m_editGetan
            && m_frameInPreset >= m_autoFrames / 2)
        {
            wendeEditAn();
        }

        if (m_auto && m_frameInPreset >= m_autoFrames)
        {
            finishAutoPreset();
        }
        if (!m_closing) update();  // Dauerschleife (vsync-getaktet)
    }

    bool event(QEvent* ev) override
    {
        // Klangausgabe VOR dem GL-Abbau stillegen (Lebenszyklus-Vertrag:
        // erst die Zulieferer stoppen, dann die Verbraucher abbauen)
        if (ev->type() == QEvent::Close) beendeKlangausgabe();
        // GL-Aufraeumen SOLANGE das Plattform-Fenster noch lebt (s. Milkdrop-
        // Standalone: danach greifen die GL-Wrapper-Dtoren ins Leere)
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
        default: QOpenGLWindow::keyPressEvent(event); break;
        }
    }

    void paintOverGL() override
    {
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

        // Kein renderMutex noetig: Standalone hat keinen Render-Thread —
        // Laden und render() laufen sequenziell im GUI-Thread.
        // Endungs-Dispatch: .lvfx/.lvfx2 (Ketten-Bisektion!), .milk
        // (Milkdrop-Node im Host — der App-Weg, Session 63) oder .avs
        QStringList report;
        const QString suffix = QFileInfo(path).suffix().toLower();
        const bool isChain = suffix == QStringLiteral("lvfx") ||
                             suffix == QStringLiteral("lvfx2");
        const bool isMilk = suffix == QStringLiteral("milk");
        // Import-Render-Scale wie in der App (S73). Bis hierher kannte der
        // Standalone den Divisor NICHT — er rendert dann ungeskaliert, und
        // klassische AVS-Presets (feste Pixelgroessen) zerfallen bei grossen
        // Fenstern in Balken. Bei kleinen Fenstern faellt es nicht auf, was
        // die Luecke lange verdeckt hat. Die App holt den Wert aus
        // `import/avsRenderScaleDivisor`; hier kommt er per --render-scale,
        // damit das Werkzeug keine App-Einstellungen mitlesen muss.
        m_viz->setImportRenderScaleDivisor(m_renderScale);

        const bool ok = isChain  ? m_viz->loadChainFile(path, &report)
                        : isMilk ? m_viz->loadMilkFile(path, &report)
                                 : m_viz->loadAvsFile(path, &report);
        int warnings = 0;
        for (const QString& line : report)
        {
            // Konvention Import-Report: nur Zeilen OHNE ℹ-Praefix sind Warnungen
            if (!line.startsWith(QStringLiteral("ℹ"))) ++warnings;
            std::printf("[Import] %s\n", qPrintable(line));
        }
        m_lastWarnings = warnings;
        if (!ok)
        {
            std::printf("[Standalone] LADEN FEHLGESCHLAGEN\n");
            m_allLoaded = false;
        }
        if (m_noApe && !isChain)
        {
            // AvsRef laedt bewusst keine APE-DLLs (avsref_main.cpp:349-357) —
            // fuer den Vergleich muessen wir sie ebenfalls stilllegen, sonst
            // misst man den APE statt der Treue des restlichen Presets (S49).
            const int off = disableApeNodes(m_viz->chain());
            if (off > 0)
            {
                m_viz->recompileChain();
                std::printf("[Standalone] --no-ape: %d APE-Knoten abgeschaltet\n", off);
            }
        }
        if (m_dumpChain)
        {
            const QJsonDocument doc(lumi::multieffect::chainToJson(m_viz->chain()));
            std::printf("[Chain]\n%s\n", doc.toJson(QJsonDocument::Indented).constData());
        }
        setTitle(QStringLiteral("AvsStandalone — %1")
                     .arg(QFileInfo(path).completeBaseName()));
        m_frameInPreset = 0;
        update();
    }

    /// Params rekursiv uebernehmen; false, wenn die Struktur abweicht.
    static bool uebernehmeParams(lumi::multieffect::ChainNode& ziel,
                                 const lumi::multieffect::ChainNode& quelle)
    {
        if (ziel.children.size() != quelle.children.size()) return false;
        if (ziel.params.index() != quelle.params.index()) return false;
        ziel.params = quelle.params;
        for (std::size_t i = 0; i < ziel.children.size(); ++i)
            if (!uebernehmeParams(ziel.children[i], quelle.children[i])) return false;
        return true;
    }

    /// Den Panel-Edit nachbilden: Params tauschen, neu uebersetzen, KEIN
    /// Runtime-Reset.
    void wendeEditAn()
    {
        m_editGetan = true;
        lumi::multieffect::ChainNode neu;
        QStringList report;
        if (!lumi::multieffect::loadChainFromFile(m_editNach, neu, &report))
        {
            std::fprintf(stderr, "FEHLER: --edit-nach nicht lesbar: %s\n",
                         qPrintable(m_editNach));
            m_allLoaded = false;
            return;
        }
        if (!uebernehmeParams(m_viz->chain(), neu))
        {
            std::fprintf(stderr, "FEHLER: --edit-nach hat eine andere Struktur "
                                 "(das waere ein Preset-Wechsel, kein Feld-Edit)\n");
            m_allLoaded = false;
            return;
        }
        m_viz->recompileChain();
        std::printf("[Standalone] Edit angewandt nach Frame %d\n", m_frameInPreset);
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
    /// synthetische Signal.
    void feedAudio()
    {
        if (m_audioQuelle != nullptr && m_audioQuelle->bereit())
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
    /// stand bis dahin viermal im Baum — hier, im MilkdropStandalone und in
    /// beiden Referenz-Werkzeugen, jedes Mal als Kopie. Jetzt bindet jeder
    /// dieselbe `SynthAudio.hpp` ein; Auseinanderlaufen ist baulich
    /// ausgeschlossen. Vorgaben ergeben Bit fuer Bit das alte Signal.
    void feedSyntheticAudio()
    {
        lumi::synth::Optionen opt;
        opt.muster = m_muster;
        opt.geschmack = lumi::synth::Geschmack::Avs;
        // Vorgabe: BEIDE Spektrumkanaele gleich. Das ist Absicht — an diesem
        // Signal haengen Matrix, Modul-Sonden und alle Feld-Sonden; wer es
        // aendert, muss alles neu einmessen. `--stereo-spektrum` (S55) macht
        // links/rechts unterscheidbar, damit Kanalfelder ueberhaupt etwas
        // zeigen koennen.
        opt.stereoSpektrum = m_stereoSpektrum;
        static lumi::synth::Frame klang;
        lumi::synth::erzeuge(m_audioBild, opt, klang);
        m_viz->updateAudioStereo(klang.spec, lumi::synth::kBins, klang.wave,
                                 lumi::synth::kWaveFrames, 2);
    }

    FrameStats saveShot(const QString& tag)
    {
        // glReadPixels braucht PHYSISCHE Pixel (Merkregel) — mit logischer
        // Groesse las der Shot bei DPI-Skalierung nur den linken unteren
        // Ausschnitt (Befund Session 44, Text-Position wirkte verschoben).
        const qreal dpr = devicePixelRatio();
        const int w = std::max(1, static_cast<int>(width() * dpr));
        const int h = std::max(1, static_cast<int>(height() * dpr));
        std::vector<unsigned char> rgba(static_cast<std::size_t>(w) * h * 4);
        QOpenGLFunctions* f = context()->functions();
        f->glBindFramebuffer(GL_FRAMEBUFFER, context()->defaultFramebufferObject());
        f->glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

        const FrameStats stats = computeStats(rgba, w, h);
        QImage img(rgba.data(), w, h, w * 4, QImage::Format_RGBA8888);
        // fileName inkl. Endung (Punkt -> Unterstrich): .avs und .lvfx-Zwilling
        // teilen den Basisnamen — completeBaseName liess den Zwilling den
        // Screenshot des .avs ueberschreiben (Befund Session 45).
        const QString base = m_presets.isEmpty()
                                 ? QStringLiteral("frame")
                                 : QFileInfo(m_presets[m_index])
                                       .fileName()
                                       .replace(QLatin1Char('.'), QLatin1Char('_'));
        QDir().mkpath(m_shotDir);
        const QString file = QStringLiteral("%1/%2_%3.png").arg(m_shotDir, base, tag);
        // glReadPixels liefert Zeile 0 = unten → fuer die PNG-Ansicht spiegeln.
        // RGB ohne Alpha speichern: FBO-Alpha ist kein Bildinhalt — Alpha-0-
        // Pixel wurden im Viewer als "weisse" Phantom-Linien angezeigt (S45).
        img.flipped(Qt::Vertical)
            .convertToFormat(QImage::Format_RGB888)
            .save(file);
        std::printf("[Standalone] Screenshot: %s — %s\n", qPrintable(file),
                    qPrintable(stats.describe()));
        return stats;
    }

    void finishAutoPreset()
    {
        const FrameStats stats = saveShot(QStringLiteral("auto"));
        const bool looksBlack = stats.maxLuma < 0.02;
        std::printf("[Standalone] Ergebnis %s: Warnungen=%d, schwarz=%s\n",
                    qPrintable(QFileInfo(m_presets[m_index]).fileName()),
                    m_lastWarnings, looksBlack ? "JA(!)" : "nein");

        if (m_index + 1 < m_presets.size())
        {
            loadPreset(m_index + 1);
        }
        else
        {
            std::printf("\n[Standalone] --auto abgeschlossen (%d Presets), Ende.\n",
                        static_cast<int>(m_presets.size()));
            std::fflush(stdout);
            // NICHT direkt aus paintGL schliessen (Qt6Gui-Teardown-Crash) —
            // erst den Event ausrollen lassen
            m_closing = true;
            QTimer::singleShot(0, this, &QWindow::close);
        }
    }

    std::unique_ptr<MultiEffectVisualizer> m_viz;
    QStringList m_presets;
    int m_index = 0;
    bool m_auto = false;
    int m_autoFrames = 120;
    QString m_shotDir;
    int m_frameInPreset = 0;
    double m_time = 0.0;
    bool m_shotRequested = false;
    bool m_allLoaded = true;
    bool m_closing = false;
    bool m_dumpChain = false;
    bool m_stereoSpektrum = false;
    QString m_editNach;   // --edit-nach: Params zur Laufzeit uebernehmen
    bool m_editGetan = false;
    int m_saveEvery = 0;
    int m_beatPeriod = 0;
    bool m_noApe = false;
    int m_lastWarnings = 0;
    /// --render-scale: Divisor des Import-Render-Scale-Knotens (1 = neutral).
    /// Gegenstueck zur App-Einstellung `import/avsRenderScaleDivisor`.
    int m_renderScale = 1;
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
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("AvsStandalone"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Isolierter AVS-Renderpfad-Test (Session 43)"));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("preset"),
                                 QStringLiteral(".avs-Datei oder Preset-Ordner "
                                                "(Default: ref/vis_avs-Korpus)"));
    const QCommandLineOption optAuto(QStringLiteral("auto"),
                                     QStringLiteral("Batch: alle Presets rendern, "
                                                    "Screenshots + Statistik, beenden"));
    const QCommandLineOption optFrames(QStringLiteral("frames"),
                                       QStringLiteral("Frames je Preset im --auto-Modus"),
                                       QStringLiteral("N"), QStringLiteral("120"));
    const QCommandLineOption optOut(QStringLiteral("out"),
                                    QStringLiteral("Screenshot-Verzeichnis"),
                                    QStringLiteral("DIR"),
                                    QDir::tempPath() + QStringLiteral("/lumiviz_avs_standalone"));
    const QCommandLineOption optSize(QStringLiteral("size"),
                                     QStringLiteral("Fenstergroesse WxH"),
                                     QStringLiteral("WxH"), QStringLiteral("800x600"));
    const QCommandLineOption optDump(QStringLiteral("dump"),
                                     QStringLiteral("uebersetzte Chain nach dem Laden "
                                                    "als JSON ausgeben"));
    const QCommandLineOption optSaveEvery(
        QStringLiteral("save-every"),
        QStringLiteral("im --auto-Lauf jeden M-ten Frame speichern"),
        QStringLiteral("M"), QStringLiteral("0"));
    const QCommandLineOption optBeatPeriod(
        QStringLiteral("beat-period"),
        QStringLiteral("deterministischer Beat alle N Frames (wie AvsRef)"),
        QStringLiteral("N"), QStringLiteral("0"));
    const QCommandLineOption optRenderScale(
        QStringLiteral("render-scale"),
        QStringLiteral("Divisor des Import-Render-Scale-Knotens (1 = neutral). "
                       "Gegenstueck zur App-Einstellung "
                       "import/avsRenderScaleDivisor — klassische AVS-Presets "
                       "brauchen 2 oder 4, sonst zerfaellt das Bild bei "
                       "grossen Fenstern"),
        QStringLiteral("N"), QStringLiteral("1"));
    parser.addOption(optRenderScale);
    parser.addOption(optSaveEvery);
    parser.addOption(optBeatPeriod);
    parser.addOption(optAuto);
    parser.addOption(optFrames);
    parser.addOption(optOut);
    parser.addOption(optSize);
    const QCommandLineOption optStereoSpektrum(
        QStringLiteral("stereo-spektrum"),
        QStringLiteral("linken/rechten Spektrumkanal unterschiedlich fuellen "
                       "(fuer Kanalfelder; Vorgabe: beide gleich)"));
    parser.addOption(optDump);
    parser.addOption(optStereoSpektrum);
    const QCommandLineOption optEditNach(
        QStringLiteral("edit-nach"),
        QStringLiteral("nach der halben Lauflaenge die Parameter aus DATEI "
                       "uebernehmen (bildet einen Panel-Edit nach)"),
        QStringLiteral("DATEI"));
    parser.addOption(optEditNach);
    // MESS-SCHALTER (S71, Teardown-Untersuchung): gibt die Kamera fuer diesen
    // Lauf frei. Der Kamera-Vertrag (Offene_Punkte §7) verlangt eine
    // ausdrueckliche Nutzeraktion — das explizite Setzen dieses Flags IST
    // sie. Ohne das Flag bleibt der Standalone wie bisher geraetefrei, damit
    // Sonden-/Korpuslaeufe nie eine Kamera oeffnen.
    const QCommandLineOption optKamera(
        QStringLiteral("kamera-freigeben"),
        QStringLiteral("Kamera fuer diesen Lauf freigeben (nur Messlaeufe!)"));
    parser.addOption(optKamera);
    // Feed-Abbau am Prozessende messen (Lebenszyklus-Vertrag 6.1)
    const QCommandLineOption optFeedZeit(
        QStringLiteral("feed-teardown-messen"),
        QStringLiteral("Dauer von LiveVideoFeed::herunterfahren() ausgeben"));
    parser.addOption(optFeedZeit);
    // --- Echte Musik als Audioquelle (S74, Aufgabe 6) -----------------------------------
    const QCommandLineOption optAudioDatei(
        QStringLiteral("audio-datei"),
        QStringLiteral("Audiodatei (MP3/WAV/FLAC/...) statt des synthetischen "
                       "Signals. NUR fuer Schaufenster und Augenschein — "
                       "AvsRef erzeugt sein Audio selbst, jeder "
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
        QStringLiteral("die Audiodatei NICHT hoerbar ausgeben (im --auto-Lauf "
                       "ohnehin die Vorgabe)"));
    // --- Muster des synthetischen Signals (S74, Auftrag Patrik) --------------------------
    const QCommandLineOption optMuster(
        QStringLiteral("audio-muster"),
        QStringLiteral("Muster des SYNTHETISCHEN Signals: klassisch|musik. "
                       "klassisch = 220-Hz-Sinus + Beat-Puls (Vorgabe, "
                       "bit-identisch seit S43) · musik = aus einer Aufnahme "
                       "abgeleitete Bandhuellkurven samt Beat-Spur, neu "
                       "synthetisiert. BEIDE sind deterministisch und von "
                       "AvsRef identisch erzeugbar — anders als --audio-datei"),
        QStringLiteral("NAME"), QStringLiteral("klassisch"));
    const QCommandLineOption optProfilSchreiben(
        QStringLiteral("audio-profil-schreiben"),
        QStringLiteral("aus --audio-datei ein Musik-Profil rechnen und als "
                       "C++-Kopf schreiben (Ziel: "
                       "projects/exec/common/MusikProfil.hpp), dann beenden"),
        QStringLiteral("DATEI"));
    const QCommandLineOption optProfilDauer(
        QStringLiteral("audio-profil-dauer"),
        QStringLiteral("Laenge des Ausschnitts fuer --audio-profil-schreiben "
                       "in Sekunden (wird auf ganze Schlaege gerundet)"),
        QStringLiteral("SEK"), QStringLiteral("20"));
    parser.addOption(optAudioDatei);
    parser.addOption(optAudioStart);
    parser.addOption(optAudioGain);
    parser.addOption(optAudioStumm);
    parser.addOption(optMuster);
    parser.addOption(optProfilSchreiben);
    parser.addOption(optProfilDauer);
    parser.process(app);

    // --- Profil schreiben: eigener Betrieb, kein Fenster ---------------------------------
    if (parser.isSet(optProfilSchreiben))
    {
        if (!parser.isSet(optAudioDatei))
        {
            std::fprintf(stderr,
                         "FEHLER: --audio-profil-schreiben braucht --audio-datei\n");
            return 2;
        }
        lumi::werkzeug::AudioDateiQuelle quelle;
        QString fehler;
        if (!quelle.laden(parser.value(optAudioDatei), &fehler))
        {
            std::fprintf(stderr, "FEHLER: Audiodatei nicht nutzbar — %s\n",
                         qPrintable(fehler));
            return 2;
        }
        quelle.setStartSekunden(parser.value(optAudioStart).toDouble());
        if (!quelle.profilSchreiben(parser.value(optProfilSchreiben),
                                    parser.value(optProfilDauer).toDouble(), &fehler))
        {
            std::fprintf(stderr, "FEHLER: Profil nicht geschrieben — %s\n",
                         qPrintable(fehler));
            return 2;
        }
        return 0;
    }

    lumi::synth::Muster muster = lumi::synth::Muster::Klassisch;
    if (!lumi::synth::musterAusText(qPrintable(parser.value(optMuster)), muster))
    {
        std::fprintf(stderr, "FEHLER: --audio-muster erwartet klassisch|musik\n");
        return 2;
    }

    if (parser.isSet(optKamera))
    {
        lumi::services::LiveVideoFeed::instance().erlaubeKamera();
        std::printf("[Standalone] Kamera FREIGEGEBEN (Messlauf)\n");
    }

    // --- Preset-Liste aufbauen -------------------------------------------------------------
    QString target = parser.positionalArguments().isEmpty()
                         ? locateCorpusDir()
                         : parser.positionalArguments().first();
    if (target.isEmpty())
    {
        std::fprintf(stderr,
                     "FEHLER: kein Preset angegeben und ref/vis_avs-Korpus nicht gefunden\n");
        return 2;
    }
    QStringList presets;
    const QFileInfo info(target);
    if (info.isDir())
    {
        const QDir dir(target);
        for (const QString& f :
             dir.entryList({QStringLiteral("*.avs"), QStringLiteral("*.lvfx"),
                            QStringLiteral("*.lvfx2"), QStringLiteral("*.milk")},
                           QDir::Files, QDir::Name))
        {
            presets << dir.absoluteFilePath(f);
        }
    }
    else if (info.isFile())
    {
        presets << info.absoluteFilePath();
    }
    if (presets.isEmpty())
    {
        std::fprintf(stderr, "FEHLER: keine .avs-Presets unter '%s'\n", qPrintable(target));
        return 2;
    }
    std::printf("[Standalone] %d Preset(s)\n", static_cast<int>(presets.size()));

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
                            parser.value(optOut));
    window.setDumpChain(parser.isSet(optDump));
    window.setStereoSpektrum(parser.isSet(optStereoSpektrum));
    window.setEditNach(parser.value(optEditNach));
    window.setSaveEvery(parser.value(optSaveEvery).toInt());
    window.setBeatPeriod(parser.value(optBeatPeriod).toInt());
    window.setRenderScale(parser.value(optRenderScale).toInt());
    window.setMuster(muster);
    if (muster == lumi::synth::Muster::Musik)
    {
        const lumi::synth::MusikProfil& p = lumi::synth::eingebautesProfil();
        std::printf("[Audio] Muster musik: %s, %d Bilder Schleife (%.2f s)\n", p.herkunft,
                    p.bilder, p.bilder / lumi::synth::kBildrate);
        if (p.bilder <= 0)
        {
            std::fprintf(stderr, "FEHLER: kein Musik-Profil eingebaut — erst mit "
                                 "--audio-profil-schreiben erzeugen\n");
            return 2;
        }
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
                    "gegen AvsRef (dessen Audio ist synthetisch)\n");
        window.setAudioQuelle(&audioQuelle);
        // Hoerbar nur im interaktiven Lauf: Stapellaeufe sollen deterministisch
        // und still durchlaufen
        const bool hoerbar = !parser.isSet(optAuto) && !parser.isSet(optAudioStumm);
        window.setAudioHoerbar(hoerbar);
        std::printf("[Audio] hoerbare Ausgabe: %s\n",
                    hoerbar ? "ja"
                            : (parser.isSet(optAuto) ? "nein (Stapellauf)"
                                                     : "nein (--audio-stumm)"));
        // Ausspuelen: der interaktive Lauf endet meist per Abbruch, und dann
        // bliebe der ganze Audio-Block im Puffer stehen
        std::fflush(stdout);
    }
    window.resize(w, h);
    window.show();

    const int rc = app.exec();

    // Feed-Abbau NACH der Event-Loop, wie Application::shutdown es tut
    // (Lebenszyklus-Vertrag 6.1) — und messbar: genau hier hing die App.
    {
        QElapsedTimer uhr;
        uhr.start();
        lumi::services::LiveVideoFeed::instance().herunterfahren();
        const qint64 ms = uhr.elapsed();
        if (parser.isSet(optFeedZeit) || ms > 100)
        {
            std::printf("[Standalone] Feed-Teardown: %lld ms%s\n",
                        static_cast<long long>(ms),
                        ms > 1000 ? "   << BLOCKIERT" : "");
            std::fflush(stdout);
        }
    }

    if (parser.isSet(optAuto) && !window.allLoaded())
    {
        std::fprintf(stderr, "[Standalone] MINDESTENS EIN PRESET NICHT GELADEN\n");
        return 1;
    }
    // Marker S71: kommt diese Zeile, liegt ein danach folgender Haenger im
    // ~QGuiApplication bzw. in fremden statischen Destruktoren — NICHT mehr
    // in unserem Code. Nur im Messlauf, damit Sondenlaeufe still bleiben.
    if (parser.isSet(optFeedZeit))
    {
        std::printf("[Standalone] main() fertig — ab hier ~QGuiApplication\n");
        std::fflush(stdout);
    }
    return rc;
}
