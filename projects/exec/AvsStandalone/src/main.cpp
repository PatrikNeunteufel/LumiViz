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
 * @date   Juli 2026
 * @version 1.0.0
 *
 * @details
 * Aufruf:
 *   AvsStandalone [presetDateiOderOrdner] [--auto] [--frames N]
 *                 [--out DIR] [--size WxH]
 *
 * - Ohne Argument wird der Referenz-Korpus gesucht
 *   (`../ref/vis_avs/avs/vis_avs/presets`, vom Exe-Pfad aufwaerts).
 * - Interaktiv: ←/→ Preset wechseln · R neu laden · S Screenshot · Esc beenden.
 * - `--auto`: jedes Preset N Frames rendern (Default 120), Screenshot +
 *   Pixel-Statistik, Import-Report auf Konsole (Zeilen OHNE ℹ-Praefix sind
 *   Warnungen) — Exit-Code 0 nur, wenn alle Presets geladen haben.
 *
 * Der Host laeuft hier im GUI-Thread (paintGL) — dieselben Methoden, die in
 * der App der Render-Thread aufruft. Audio kommt synthetisch (Sinus +
 * Beat-Puls), damit Scopes/Beat-Effekte leben.
 ****************************************************************************************
 */

#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "services/LiveVideoFeed.hpp"  // Kamera-Freigabe + Teardown-Messung (S71)

#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QKeyEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLWindow>
#include <QStringList>
#include <QSurfaceFormat>
#include <QTimer>

#include <algorithm>
#include <cmath>
#include <cstdio>
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

    /// --no-ape: aus APEs uebersetzte Knoten abschalten (AvsRef kennt sie nicht)
    void setNoApe(bool on) { m_noApe = on; }

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
        loadPreset(0);
    }

    void resizeGL(int w, int h) override
    {
        if (m_viz != nullptr) m_viz->resize(QSize(w, h));
    }

    void paintGL() override
    {
        if (m_viz == nullptr) return;
        feedSyntheticAudio();
        m_viz->render(1.0f / 60.0f);
        if (m_auto && m_saveEvery > 0 && m_frameInPreset % m_saveEvery == 0)
        {
            saveShot(QStringLiteral("f%1").arg(m_frameInPreset + 1, 4, 10,
                                               QLatin1Char('0')));
        }
        ++m_frameInPreset;
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

    void feedSyntheticAudio()
    {
        constexpr int kFrames = 576;
        constexpr int kBins = 512;
        // Beat-Puls ~120 BPM fuer Beat-Effekte + lebendige Scopes
        const double beat = 0.55 + 0.45 * std::max(0.0, std::sin(m_time * 2.0 * kPi * 2.0));
        static std::vector<float> wave;
        static std::vector<float> spec;
        wave.assign(kFrames * 2, 0.0f);
        spec.assign(kBins * 2, 0.0f);
        for (int i = 0; i < kFrames; ++i)
        {
            const double ph = m_time * 220.0 * 2.0 * kPi + i * (2.0 * kPi / 64.0);
            const float l = static_cast<float>(beat * 0.5 * std::sin(ph));
            const float r = static_cast<float>(beat * 0.5 * std::sin(ph + 0.7));
            wave[static_cast<std::size_t>(i) * 2 + 0] = l;
            wave[static_cast<std::size_t>(i) * 2 + 1] = r;
        }
        for (int b = 0; b < kBins; ++b)
        {
            const float v = static_cast<float>(beat * 0.8 / (1.0 + b * 0.03));
            spec[static_cast<std::size_t>(b) * 2 + 0] = v;
            // Vorgabe: BEIDE Kanaele gleich. Das ist Absicht — an diesem Signal
            // haengen Matrix, Modul-Sonden und alle Feld-Sonden; wer es aendert,
            // muss alles neu einmessen.
            //
            // `--stereo-spektrum` (S55) macht daraus ein Signal, das sich
            // links/rechts unterscheidet: rechts faellt steiler ab und wird zu
            // hohen Baendern hin leiser. Nur damit koennen Kanalfelder
            // (`timescope.channel`/`useChannel`) ueberhaupt etwas zeigen —
            // vorher waren links, rechts und Mitte zwangslaeufig identisch und
            // die Sonden mussten „nicht pruefbar" heissen. Weiterhin
            // DETERMINISTISCH: nur eine andere Formel, kein echtes Material.
            spec[static_cast<std::size_t>(b) * 2 + 1] =
                m_stereoSpektrum
                    ? static_cast<float>(beat * 0.8 / (1.0 + b * 0.12))
                    : v;
        }
        m_viz->updateAudioStereo(spec.data(), kBins, wave.data(), kFrames, 2);
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
    parser.process(app);

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
