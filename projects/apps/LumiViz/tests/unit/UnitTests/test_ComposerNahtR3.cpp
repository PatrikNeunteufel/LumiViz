/**
 ****************************************************************************************
 * @file   test_ComposerNahtR3.cpp
 * @brief  Composer, Stufe C2a: die Composer-Naht R3 im Render-Thread — eine
 *         Bildquelle neben dem einen Visualizer (docs/ui/Composer_Konzept.md §12.2)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Der Vergleichslauf und die uebrigen Unit-Tests beruehren den Render-Thread
 * nicht — dieser Test ist der Beleg fuer R3. Er faehrt den echten Thread an
 * einem echten Fenster, mit zaehlenden Visualizern ohne eigenes GL:
 *  - Platz leer („Composer aus"): der eine Visualizer wird angelegt, bekommt
 *    Audiodaten und rechnet jedes Bild.
 *  - Platz besetzt: die Bildquelle rechnet, der Visualizer ruht, beide bekommen
 *    Audiodaten und Groesse.
 *  - Wegnehmen: die Bildquelle wird auf dem Render-Thread aufgeraeumt und
 *    geloescht, der Visualizer rechnet weiter.
 *  - Herunterfahren mit besetztem Platz: beide werden aufgeraeumt.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "TestFenster.hpp"
#include "UI/widgets/VisualizerRenderThread.hpp"
#include "visualizers/IVisualizer.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QMutex>
#include <QOpenGLContext>
#include <QThread>

#include <atomic>
#include <functional>
#include <memory>

namespace
{

std::atomic<int> g_geloescht{0};
std::atomic<int> g_aufgeraeumtVorLoeschen{0};
std::atomic<QThread*> g_loeschThread{nullptr};

/// Visualizer ohne eigenes GL: zaehlt, was der Render-Thread mit ihm tut.
struct Zaehler : IVisualizer
{
    std::atomic<bool> angelegt{false};
    std::atomic<int> anlegen{0};
    std::atomic<int> bilder{0};
    std::atomic<int> groessen{0};
    std::atomic<int> aufraeumen{0};
    std::atomic<int> spektren{0};
    std::atomic<QThread*> anlegeThread{nullptr};
    bool meldeLoeschen = false;

    ~Zaehler() override
    {
        if (!meldeLoeschen) return;
        g_aufgeraeumtVorLoeschen = aufraeumen.load();
        g_loeschThread = QThread::currentThread();
        ++g_geloescht;
    }

    [[nodiscard]] QString visualizerId() const override { return QStringLiteral("zaehler"); }
    [[nodiscard]] QString visualizerName() const override { return QStringLiteral("Zaehler"); }
    [[nodiscard]] QString visualizerDescription() const override { return {}; }

    void initialize() override
    {
        anlegeThread = QThread::currentThread();
        ++anlegen;
        angelegt = true;
    }
    void render(float) override { ++bilder; }
    void resize(const QSize&) override { ++groessen; }
    void cleanup() override
    {
        ++aufraeumen;
        angelegt = false;
    }
    [[nodiscard]] bool isInitialized() const override { return angelegt; }
    void updateSpectrum(const float*, int) override { ++spektren; }
};

/// Warten, bis die Bedingung gilt — die Ereignisschleife laeuft dabei weiter
/// (das Fenster braucht sie fuer Expose und Resize).
bool warte(const std::function<bool()>& bedingung, int hoechstensMs = 5000)
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

}  // namespace

TEST_CASE("Composer R3: Bildquelle im Render-Thread — leer wie immer, besetzt rechnet die Quelle")
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QGuiApplication app{argc, argv};

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

    Zaehler kette;
    faden.setVisualizer(&kette, nullptr);
    if (!warte([&] { return kette.bilder >= 3; }))
    {
        MESSAGE("Fenster wird nicht sichtbar (Headless?) — uebersprungen");
        faden.stopAndWait();
        fenster.attachThread(nullptr);
        return;
    }

    const float spektrum[4] = {0.1f, 0.2f, 0.3f, 0.4f};

    SUBCASE("Ablauf")
    {
        // --- Platz leer: der eine Visualizer wie immer -----------------------
        CHECK(kette.anlegen == 1);
        CHECK(kette.anlegeThread.load() == &faden);
        CHECK(kette.aufraeumen == 0);
        faden.updateAudio(spektrum, 4, nullptr, 0);
        CHECK(warte([&] { return kette.spektren == 1; }));
        // ein Snapshot wird genau einmal gereicht, nicht je Bild
        const int bilderVorher = kette.bilder;
        CHECK(warte([&] { return kette.bilder >= bilderVorher + 5; }));
        CHECK(kette.spektren == 1);

        // --- Platz besetzt: die Bildquelle rechnet, die Kette ruht -----------
        auto quelle = std::make_unique<Zaehler>();
        quelle->meldeLoeschen = true;
        Zaehler* q = quelle.get();
        faden.setBildquelle(q, nullptr);
        REQUIRE(warte([&] { return q->bilder >= 1; }));
        CHECK(q->anlegen == 1);
        CHECK(q->anlegeThread.load() == &faden);
        CHECK(q->groessen >= 1);  // bekommt beim Anlegen die Fenstergroesse

        const int ketteSteht = kette.bilder;
        const int quelleVorher = q->bilder;
        CHECK(warte([&] { return q->bilder >= quelleVorher + 5; }));
        CHECK(kette.bilder == ketteSteht);
        CHECK(kette.aufraeumen == 0);  // bleibt angelegt

        // Audiodaten an beide
        faden.updateAudio(spektrum, 4, nullptr, 0);
        CHECK(warte([&] { return q->spektren == 1 && kette.spektren == 2; }));

        // Groesse an beide
        const int ketteGroessen = kette.groessen;
        const int quelleGroessen = q->groessen;
        fenster.resize(200, 150);
        CHECK(warte([&] {
            return kette.groessen > ketteGroessen && q->groessen > quelleGroessen;
        }));

        // --- Wegnehmen: aufraeumen und loeschen auf dem Render-Thread --------
        g_geloescht = 0;
        faden.setBildquelle(nullptr, std::move(quelle));
        REQUIRE(warte([&] { return g_geloescht == 1; }));
        CHECK(g_aufgeraeumtVorLoeschen == 1);
        CHECK(g_loeschThread.load() == &faden);

        const int ketteWeiter = kette.bilder;
        CHECK(warte([&] { return kette.bilder >= ketteWeiter + 5; }));
        CHECK(kette.anlegen == 1);  // nie neu angelegt
        CHECK(kette.aufraeumen == 0);

        // --- Herunterfahren mit besetztem Platz ------------------------------
        Zaehler zweite;  // gehoert dem Test wie sonst der Fassade
        faden.setBildquelle(&zweite, nullptr);
        REQUIRE(warte([&] { return zweite.bilder >= 1; }));
        faden.stopAndWait();
        CHECK(zweite.aufraeumen == 1);
        CHECK(kette.aufraeumen == 1);
    }

    SUBCASE("Setzen und Wegnehmen in einem Zug: nie angelegt, trotzdem geloescht")
    {
        auto quelle = std::make_unique<Zaehler>();
        quelle->meldeLoeschen = true;
        Zaehler* q = quelle.get();
        g_geloescht = 0;
        {
            // Unter dem Render-Mutex steht der Thread vor dem naechsten Bild —
            // beide Kommandos liegen dann sicher im selben Durchlauf oder das
            // erste wurde noch nicht angefasst.
            QMutexLocker halt(&renderMutex);
            faden.setBildquelle(q, nullptr);
            faden.setBildquelle(nullptr, std::move(quelle));
        }
        CHECK(warte([&] { return g_geloescht == 1; }));
        const int ketteWeiter = kette.bilder;
        CHECK(warte([&] { return kette.bilder >= ketteWeiter + 5; }));
        faden.stopAndWait();
        CHECK(kette.aufraeumen == 1);
    }

    faden.stopAndWait();
    fenster.attachThread(nullptr);
}
