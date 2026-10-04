/**
 ****************************************************************************************
 * @file   test_ComposerBildquelle.cpp
 * @brief  Composer, Stufe C2a: die Komposition als Bildquelle der App
 *         (docs/ui/Composer_Konzept.md §5, §12.2 Naht R3)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 *  - Die Bildquelle allein (Offscreen-Kontext, eigenes Zielbild): die Stelle
 *    der Uhr bestimmt das Bild; steht die Uhr, steht das Bild; Audiodaten
 *    gehen an die Renderer der Clips, auch an neu dazugekommene; Aufraeumen
 *    und erneutes Anlegen behalten die Komposition.
 *  - Die Bildquelle im echten Render-Thread am echten Fenster: das Bild der
 *    Komposition kommt im Fenster an (Aufnahme ueber `requestCapture`), und
 *    nach dem Wegnehmen rechnet wieder der eine Visualizer.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "TestFenster.hpp"
#include "UI/widgets/VisualizerRenderThread.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bildquelle.hpp"
#include "services/ServiceContainer.hpp"
#include "composer/Komposition.hpp"
#include "composer/Uhr.hpp"
#include "visualizers/IVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QFile>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QImage>
#include <QMutex>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QTemporaryDir>
#include <QThread>

#include <atomic>
#include <functional>
#include <memory>

using namespace lumi::multieffect;
using namespace lumi::composer;

namespace
{

/// Preset-Datei, die das Bild einfarbig fuellt.
QString farbPreset(const QTemporaryDir& dir, const QString& name, uint32_t farbe)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    ClearParams cp;
    cp.color = farbe;
    clear.params = cp;
    root.children.push_back(std::move(clear));
    const QString pfad = dir.filePath(name);
    REQUIRE(saveChainToFile(root, pfad));
    return pfad;
}

/// Preset-Datei mit eigenem Zustand: wandernde Balken und Feedback.
QString balkenPreset(const QTemporaryDir& dir, const QString& name, uint32_t farbe)
{
    ChainNode root;
    root.params = ListParams{};
    ChainNode clear;
    ClearParams cp;
    cp.onlyFirst = true;
    clear.params = cp;
    ChainNode balken;
    balken.params = DebugBarsParams{farbe, -2.5f};
    ChainNode blit;
    blit.params = BlitterFeedbackParams{};
    root.children = {clear, balken, blit};
    const QString pfad = dir.filePath(name);
    REQUIRE(saveChainToFile(root, pfad));
    return pfad;
}

Kanal visualKanal(const QString& name, const QString& datei, std::int64_t start, std::int64_t ende)
{
    VisualClip c;
    c.datei = datei;
    c.startMs = start;
    c.endeMs = ende;
    Kanal k;
    k.art = KanalArt::Visual;
    k.name = name;
    k.visual = {c};
    return k;
}

int verschieden(const QImage& a, const QImage& b)
{
    if (a.size() != b.size()) return -1;
    int n = 0;
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x)
            if (a.pixel(x, y) != b.pixel(x, y)) ++n;
    return n;
}

bool warte(const std::function<bool()>& bedingung, int hoechstensMs = 8000)
{
    QElapsedTimer uhr;
    uhr.start();
    while (!bedingung())
    {
        if (uhr.elapsed() > hoechstensMs) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(2);
    }
    return true;
}

/// Der „eine Visualizer" der App fuer den Thread-Test: fuellt das Fenster gruen.
struct GruenerVisualizer : IVisualizer
{
    std::atomic<bool> angelegt{false};
    std::atomic<int> bilder{0};
    QSize groesse;

    [[nodiscard]] QString visualizerId() const override { return QStringLiteral("gruen"); }
    [[nodiscard]] QString visualizerName() const override { return QStringLiteral("Gruen"); }
    [[nodiscard]] QString visualizerDescription() const override { return {}; }
    void initialize() override { angelegt = true; }
    void render(float) override
    {
        QOpenGLContext* k = QOpenGLContext::currentContext();
        QOpenGLFunctions* f = k->functions();
        f->glBindFramebuffer(GL_FRAMEBUFFER, k->defaultFramebufferObject());
        f->glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
        ++bilder;
    }
    void resize(const QSize& s) override { groesse = s; }
    void cleanup() override { angelegt = false; }
    [[nodiscard]] bool isInitialized() const override { return angelegt; }
};

}  // namespace

TEST_SUITE("ComposerBildquelle")
{
    TEST_CASE("C2a Bildquelle: die Stelle der Uhr bestimmt das Bild, steht die Uhr, steht das Bild")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QGuiApplication app{argc, argv};
        QOpenGLContext ctx;
        QSurfaceFormat fmt;
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
        ctx.setFormat(fmt);
        QOffscreenSurface surface;
        bool gut = ctx.create();
        if (gut)
        {
            surface.setFormat(ctx.format());
            surface.create();
            gut = surface.isValid() && ctx.makeCurrent(&surface);
        }
        if (!gut)
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x0000FF);
        const QString balken = balkenPreset(dir, QStringLiteral("Balken.lvfx"), 0xFFFFFF);

        // Rot 0–1000 unten, Blau 500–1500 darueber, Balken 2000–4000
        Komposition k;
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"), rot, 0, 1000));
        k.kanaele.push_back(visualKanal(QStringLiteral("CH2"), blau, 500, 1500));
        k.kanaele.push_back(visualKanal(QStringLiteral("CH3"), balken, 2000, 4000));

        std::int64_t zeit = 0;  // die Wanduhr des Tests
        Bildquelle quelle{Uhr([&zeit] { return zeit; })};
        const QSize groesse(64, 48);
        QOpenGLFramebufferObject ziel(groesse);
        quelle.setZiel(static_cast<int>(ziel.handle()));
        quelle.setKomposition(k);
        CHECK(quelle.uhr().laengeMs() == 4000);
        CHECK_FALSE(quelle.uhr().laeuft());

        const float spektrum[4] = {0.1f, 0.2f, 0.3f, 0.4f};
        quelle.updateSpectrum(spektrum, 4);  // vor dem Anlegen: nur gemerkt
        CHECK(quelle.debugAudioGereicht() == 0);

        quelle.initialize();
        quelle.resize(groesse);
        REQUIRE(quelle.isInitialized());
        const QColor kRot(255, 0, 0), kBlau(0, 0, 255), kSchwarz(0, 0, 0);

        // --- Stelle 0, Uhr steht: Rot; der neue Renderer bekommt das Gemerkte
        quelle.render(0.016f);
        CHECK(ziel.toImage().pixelColor(32, 24) == kRot);
        CHECK(quelle.gemischteClips() == 1);
        CHECK(quelle.gerechneteBilder() == 1);
        // Rot laeuft, Blau und Balken sind vorgewaermt (Vorlauf 2 s) — alle
        // drei bekommen das Gemerkte genau einmal
        CHECK(quelle.rendererAnzahl() == 3);
        CHECK(quelle.debugAudioGereicht() == 3);

        // steht die Uhr, wird nicht neu gerechnet — das Bild bleibt
        quelle.render(0.016f);
        quelle.render(0.016f);
        CHECK(quelle.gerechneteBilder() == 1);
        CHECK(ziel.toImage().pixelColor(32, 24) == kRot);

        // --- Sprung bei stehender Uhr: einmal rechnen, dann wieder stehen ----
        quelle.uhr().springe(700);
        quelle.render(0.016f);
        CHECK(ziel.toImage().pixelColor(32, 24) == kBlau);  // Blau liegt ueber Rot
        CHECK(quelle.gemischteClips() == 2);
        CHECK(quelle.letzteStelleMs() == 700);
        quelle.render(0.016f);
        CHECK(quelle.gerechneteBilder() == 2);

        // Audiodaten gehen an jeden vorhandenen Renderer
        const int vorher = quelle.debugAudioGereicht();
        quelle.updateSpectrum(spektrum, 4);
        CHECK(quelle.debugAudioGereicht() == vorher + quelle.rendererAnzahl());

        // --- Luecke: schwarz --------------------------------------------------
        quelle.uhr().springe(1700);
        quelle.render(0.016f);
        CHECK(ziel.toImage().pixelColor(32, 24) == kSchwarz);
        CHECK(quelle.gemischteClips() == 0);

        // --- laufende Uhr: jedes Bild wird gerechnet, das Visual bewegt sich --
        quelle.uhr().springe(2000);
        quelle.uhr().starte();
        CHECK(quelle.uhr().laeuft());
        zeit += 100;
        quelle.render(0.1f);
        const QImage a = ziel.toImage();
        zeit += 100;
        quelle.render(0.1f);
        const QImage b = ziel.toImage();
        CHECK(quelle.letzteStelleMs() == 2200);
        CHECK(verschieden(a, b) > 0);
        const int gerechnet = quelle.gerechneteBilder();

        // --- Stopp: das Bild steht, auch ueber mehrere Bilder ----------------
        quelle.uhr().stoppe();
        zeit += 500;
        quelle.render(0.1f);
        const QImage c = ziel.toImage();
        quelle.render(0.1f);
        CHECK(verschieden(c, ziel.toImage()) == 0);
        CHECK(quelle.gerechneteBilder() == gerechnet);  // Stelle unveraendert: nichts gerechnet
        CHECK(verschieden(b, c) == 0);

        // --- Ende der Komposition: die Uhr haelt, das Bild ist schwarz --------
        quelle.uhr().starte();
        zeit += 10000;
        quelle.render(0.1f);
        CHECK_FALSE(quelle.uhr().laeuft());
        CHECK(quelle.letzteStelleMs() == 4000);
        CHECK(ziel.toImage().pixelColor(32, 24) == kSchwarz);

        // --- Groesse aendert sich bei stehender Uhr: einmal neu rechnen ------
        quelle.uhr().springe(200);
        quelle.render(0.016f);
        const int vorGroesse = quelle.gerechneteBilder();
        const QSize gross(96, 72);
        QOpenGLFramebufferObject ziel2(gross);
        quelle.setZiel(static_cast<int>(ziel2.handle()));
        quelle.resize(gross);
        quelle.render(0.016f);
        CHECK(quelle.gerechneteBilder() == vorGroesse + 1);
        CHECK(ziel2.toImage().pixelColor(90, 66) == kRot);

        // --- Messung: zaehlt, was gerechnet wurde ------------------------------
        {
            const Messung& m = quelle.messung();
            CHECK(m.bilder == quelle.gerechneteBilder());
            CHECK(m.gezeichnet > m.bilder);  // stehende Bilder werden nur noch einmal gezeigt
            CHECK(m.rendererMax == 3);
            CHECK(m.rechnenSummeMs > 0.0);
            CHECK(m.rechnenMaxMs >= m.rechnenMittelMs());
            CHECK(m.erstesBildMs > 0.0);
            CHECK(m.rechnenMaxMs >= m.erstesBildMs);
            CHECK(m.zeichnenMaxMs >= m.zeichnenMittelMs());
            // Grafikspeicher: entweder beide Werte oder keiner
            CHECK((m.speicherFreiVorherKb >= 0) == (m.speicherFreiTiefstKb >= 0));
            if (m.speicherFreiVorherKb >= 0)
            {
                CHECK(m.speicherFreiTiefstKb <= m.speicherFreiVorherKb);
            }
            CHECK_FALSE(m.gpu.isEmpty());
            CHECK(m.groesse == gross);  // die Groesse des letzten gerechneten Bilds
            const QStringList text = m.alsText();
            REQUIRE(text.size() == 6);
            CHECK(text[0].contains(QStringLiteral("%1 frames computed").arg(m.bilder)));
            for (const QString& zeile : text) MESSAGE(zeile.toStdString());
        }
        CHECK(Messung{}.alsText().size() == 1);  // nichts gerechnet: eine Zeile

        // --- Anlauf je Clip: Rot beim Betreten, Blau und Balken vorgewaermt;
        //     nach den Spruengen kamen Renderer neu dazu — jeder steht einmal da
        {
            const QStringList anlauf = quelle.anlaufText();
            CHECK(anlauf.size() >= 3);
            // im ersten Bild: erst die vorgewaermten, dann der laufende
            CHECK(anlauf[0].contains(QStringLiteral("warmed up")));
            CHECK(anlauf[0].contains(QStringLiteral("Blau.lvfx")));
            CHECK(anlauf[1].contains(QStringLiteral("Balken.lvfx")));
            CHECK(anlauf[2].contains(QStringLiteral("on entry")));
            CHECK(anlauf[2].contains(QStringLiteral("Rot.lvfx")));
            for (const QString& zeile : anlauf) MESSAGE(zeile.toStdString());
        }

        // --- Aufraeumen und erneut anlegen: die Komposition ist noch da ------
        quelle.cleanup();
        CHECK_FALSE(quelle.isInitialized());
        CHECK(quelle.rendererAnzahl() == 0);
        quelle.initialize();
        quelle.resize(gross);
        quelle.uhr().springe(700);
        quelle.render(0.016f);
        CHECK(ziel2.toImage().pixelColor(48, 36) == kBlau);

        // --- fehlende Datei: gemeldet, das Bild bleibt schwarz ----------------
        Komposition fehlt;
        fehlt.kanaele.push_back(
            visualKanal(QStringLiteral("CH1"), dir.filePath(QStringLiteral("gibtsnicht.lvfx")), 0, 1000));
        quelle.setKomposition(fehlt);
        CHECK(quelle.uhr().stelleMs() == 0);  // neue Komposition: angehalten am Anfang
        quelle.render(0.016f);
        CHECK(ziel2.toImage().pixelColor(48, 36) == kSchwarz);
        CHECK_FALSE(quelle.bericht().isEmpty());

        quelle.cleanup();
    }

    TEST_CASE("C2a Bildquelle im Render-Thread: das Bild der Komposition kommt im Fenster an")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QGuiApplication app{argc, argv};

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x0000FF);

        VisualizerGLWindow fenster;
        QOpenGLContext kontext;
        kontext.setFormat(fenster.requestedFormat());
        if (!kontext.create())
        {
            MESSAGE("Kein GL-Kontext moeglich (Headless?) — uebersprungen");
            return;
        }
        QMutex renderMutex;
        VisualizerRenderThread faden(fenster, kontext, renderMutex);
        fenster.attachThread(&faden);
        kontext.moveToThread(&faden);
        faden.setPacing(RenderPacing::Unlimited, 60);
        faden.start();
        fenster.resize(160, 120);
        testfenster::platziere(fenster);
        fenster.show();

        QImage aufnahme;
        int aufnahmen = 0;
        QObject::connect(&faden, &VisualizerRenderThread::frameCaptured, &app,
                         [&](const QImage& bild) {
                             aufnahme = bild;
                             ++aufnahmen;
                         });
        const auto nimmAuf = [&]() {
            const int vorher = aufnahmen;
            faden.requestCapture();
            return warte([&] { return aufnahmen > vorher; });
        };
        const auto mitte = [&]() {
            return aufnahme.pixelColor(aufnahme.width() / 2, aufnahme.height() / 2);
        };

        GruenerVisualizer kette;
        faden.setVisualizer(&kette, nullptr);
        if (!warte([&] { return kette.bilder >= 3; }))
        {
            MESSAGE("Fenster wird nicht sichtbar (Headless?) — uebersprungen");
            faden.stopAndWait();
            fenster.attachThread(nullptr);
            return;
        }
        REQUIRE(nimmAuf());
        CHECK(mitte() == QColor(0, 255, 0));  // Platz leer: der eine Visualizer

        // --- Komposition: Rot 0–60 s, darueber Blau ab 30 s -------------------
        Komposition k;
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"), rot, 0, 60000));
        k.kanaele.push_back(visualKanal(QStringLiteral("CH2"), blau, 30000, 60000));
        auto quelle = std::make_unique<Bildquelle>();
        Bildquelle* q = quelle.get();
        q->setKomposition(k);
        q->uhr().starte();
        faden.setBildquelle(q, nullptr);
        REQUIRE(warte([&] {
            QMutexLocker halt(&renderMutex);
            return q->gerechneteBilder() >= 3;
        }));
        REQUIRE(nimmAuf());
        CHECK(mitte() == QColor(255, 0, 0));
        {
            QMutexLocker halt(&renderMutex);
            CHECK(q->gemischteClips() == 1);
            CHECK(q->bericht().isEmpty());
        }

        // Sprung von der GUI-Seite, unter dem Render-Mutex
        {
            QMutexLocker halt(&renderMutex);
            q->uhr().springe(40000);
        }
        REQUIRE(warte([&] {
            QMutexLocker halt(&renderMutex);
            return q->letzteStelleMs() >= 40000;
        }));
        REQUIRE(nimmAuf());
        CHECK(mitte() == QColor(0, 0, 255));

        // Fenstergroesse aendern: das Bild fuellt weiter das ganze Fenster
        fenster.resize(220, 140);
        REQUIRE(warte([&] { return kette.groesse.width() > 160 * fenster.devicePixelRatio(); }));
        REQUIRE(nimmAuf());
        REQUIRE(nimmAuf());  // sicher ein Bild NACH dem Resize
        CHECK(aufnahme.pixelColor(aufnahme.width() - 3, aufnahme.height() - 3) == QColor(0, 0, 255));
        CHECK(aufnahme.pixelColor(2, 2) == QColor(0, 0, 255));

        // --- Wegnehmen: der eine Visualizer rechnet wieder --------------------
        faden.setBildquelle(nullptr, std::move(quelle));
        const int weiter = kette.bilder;
        REQUIRE(warte([&] { return kette.bilder >= weiter + 3; }));
        REQUIRE(nimmAuf());
        CHECK(mitte() == QColor(0, 255, 0));

        faden.stopAndWait();
        fenster.attachThread(nullptr);
    }

    TEST_CASE("C2a AppAnbindung: eine .lvcomp ueber die Fassade ein- und ausschalten")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        Komposition k;
        k.kanaele.push_back(visualKanal(QStringLiteral("CH1"), rot, 0, 60000));
        const QString datei = dir.filePath(QStringLiteral("probe.lvcomp"));
        REQUIRE(speichereKomposition(k, datei));

        ServiceContainer dienste;
        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();

        QImage aufnahme;
        int aufnahmen = 0;
        QObject::connect(&widget, &VisualizerWidget::frameCaptured, &app,
                         [&](const QImage& bild) {
                             aufnahme = bild;
                             ++aufnahmen;
                         });
        const auto nimmAuf = [&]() {
            const int vorher = aufnahmen;
            widget.requestScreenshot();
            return warte([&] { return aufnahmen > vorher; });
        };
        const auto mitte = [&]() {
            return aufnahme.pixelColor(aufnahme.width() / 2, aufnahme.height() / 2);
        };
        if (!widget.hasVisualizer() || !nimmAuf())
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        const QColor kRot(255, 0, 0);
        CHECK_FALSE(lumi::composer::kompositionLaeuft(widget));
        IVisualizer* const kette = widget.visualizer();

        // keine Komposition: nichts aendert sich
        const QString unsinn = dir.filePath(QStringLiteral("unsinn.lvcomp"));
        {
            QFile f(unsinn);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write("{\"irgendwas\": 1}");
        }
        CHECK_FALSE(starteKomposition(widget, unsinn));
        CHECK_FALSE(kompositionLaeuft(widget));

        // einschalten: die Komposition liefert das Bild, die Kette bleibt der Host
        QStringList bericht;
        REQUIRE(starteKomposition(widget, datei, &bericht));
        CHECK(bericht.isEmpty());
        CHECK(kompositionLaeuft(widget));
        CHECK(widget.visualizer() == kette);
        REQUIRE(warte([&] {
            QMutexLocker halt(&widget.renderMutex());
            auto* q = dynamic_cast<Bildquelle*>(widget.bildquelle());
            return q != nullptr && q->gerechneteBilder() >= 3;
        }));
        REQUIRE(nimmAuf());
        CHECK(mitte() == kRot);
        {
            QMutexLocker halt(&widget.renderMutex());
            auto* q = dynamic_cast<Bildquelle*>(widget.bildquelle());
            REQUIRE(q != nullptr);
            CHECK(q->uhr().laeuft());  // C2a: die Uhr startet sofort
        }

        // die Messung der laufenden Komposition ist von aussen lesbar
        CHECK(messungDerKomposition(widget).size() == 6);

        // noch einmal oeffnen ersetzt die laufende
        REQUIRE(starteKomposition(widget, datei));
        CHECK(kompositionLaeuft(widget));

        // ausschalten: die Kette liefert wieder das Bild
        schliesseKomposition(widget);
        CHECK_FALSE(kompositionLaeuft(widget));
        CHECK(widget.visualizer() == kette);
        REQUIRE(nimmAuf());
        REQUIRE(nimmAuf());  // sicher ein Bild NACH dem Wegnehmen
        CHECK(mitte() != kRot);
        CHECK(messungDerKomposition(widget).isEmpty());
        schliesseKomposition(widget);  // zweimal schadet nicht
    }
}
