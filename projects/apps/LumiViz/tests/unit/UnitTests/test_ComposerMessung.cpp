/**
 ****************************************************************************************
 * @file   test_ComposerMessung.cpp
 * @brief  Composer, Stufe C2a: Messlauf einer Komposition im echten Render-Thread
 *         (docs/ui/Composer_Konzept.md §9c „geht nach C2": Zeiten im
 *         Release-Build, Bildspeicher gemessen)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Kein Test im engeren Sinn: der Fall laeuft nur, wenn
 * `LUMIVIZ_COMPOSER_MESSUNG=<datei.lvcomp>` gesetzt ist, sonst endet er sofort
 * (ohne Skip). Er faehrt die Komposition einmal von Anfang bis Ende — am
 * echten Fenster, im echten Render-Thread, ohne Bildbremse (Unlimited), mit
 * kuenstlichen Audiodaten — und gibt die Messung der Bildquelle samt der
 * Bildrate des Threads aus.
 *
 *  - `LUMIVIZ_COMPOSER_MESSUNG_GROESSE=1920x1080` (Vorgabe) — Bildgroesse in
 *    physischen Pixeln
 *  - `LUMIVIZ_COMPOSER_MESSUNG_AUS=<datei.txt>` — die Zeilen zusaetzlich in
 *    eine Datei
 *
 * Auf welcher GPU gemessen wird, bestimmt der Windows-Eintrag der Exe
 * (UserGpuPreferences); die erste Zeile der Messung nennt sie.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "TestFenster.hpp"
#include "UI/widgets/VisualizerRenderThread.hpp"
#include "composer/Bildquelle.hpp"
#include "composer/Komposition.hpp"
#include "visualizers/IVisualizer.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QMutex>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QThread>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>
#include <vector>

using namespace lumi::composer;

namespace
{

/// Der „eine Visualizer" neben der Bildquelle — der Thread braucht ihn.
struct LeererVisualizer : IVisualizer
{
    std::atomic<bool> angelegt{false};
    [[nodiscard]] QString visualizerId() const override { return QStringLiteral("leer"); }
    [[nodiscard]] QString visualizerName() const override { return QStringLiteral("Leer"); }
    [[nodiscard]] QString visualizerDescription() const override { return {}; }
    void initialize() override { angelegt = true; }
    void render(float) override
    {
        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
        f->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        f->glClear(GL_COLOR_BUFFER_BIT);
    }
    void resize(const QSize&) override {}
    void cleanup() override { angelegt = false; }
    [[nodiscard]] bool isInitialized() const override { return angelegt; }
};

}  // namespace

TEST_CASE("C2a Messlauf: eine Komposition im Render-Thread (nur mit LUMIVIZ_COMPOSER_MESSUNG)")
{
    const QString datei = qEnvironmentVariable("LUMIVIZ_COMPOSER_MESSUNG");
    if (datei.isEmpty()) return;

    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QGuiApplication app{argc, argv};

    // Movement-Tabelle: wie die App in Scheiben (LUMIVIZ_MOVEMENT_SCHEIBE_MS,
    // Vorgabe hier 0 = sofort wie in allen Tests); der Speicher beginnt leer
    struct Scheibe
    {
        ~Scheibe()
        {
            MultiEffectVisualizer::setMovementScheibeMs(0.0);
            MultiEffectVisualizer::leereMovementSpeicher();
        }
    } scheibeZurueck;
    const double scheibeMs = qEnvironmentVariable("LUMIVIZ_MOVEMENT_SCHEIBE_MS").toDouble();
    MultiEffectVisualizer::leereMovementSpeicher();
    MultiEffectVisualizer::setMovementScheibeMs(scheibeMs);

    Komposition komposition;
    QStringList bericht;
    REQUIRE(ladeKomposition(datei, komposition, &bericht));
    for (const QString& zeile : bericht) MESSAGE(zeile.toStdString());

    QSize groesse(1920, 1080);
    const QStringList teile =
        qEnvironmentVariable("LUMIVIZ_COMPOSER_MESSUNG_GROESSE").split(QLatin1Char('x'));
    if (teile.size() == 2 && teile[0].toInt() > 0 && teile[1].toInt() > 0)
        groesse = QSize(teile[0].toInt(), teile[1].toInt());

    VisualizerGLWindow fenster;
    QOpenGLContext kontext;
    kontext.setFormat(fenster.requestedFormat());
    REQUIRE(kontext.create());
    QMutex renderMutex;
    VisualizerRenderThread faden(fenster, kontext, renderMutex);
    fenster.attachThread(&faden);
    kontext.moveToThread(&faden);
    faden.setPacing(RenderPacing::Unlimited, 60);

    std::vector<double> bildraten;
    QObject::connect(&faden, &VisualizerRenderThread::fpsMeasured, &app,
                     [&](double fps) { bildraten.push_back(fps); });
    faden.start();

    // Fenstergroesse in logischen Pixeln, damit das Bild die gewuenschten
    // physischen hat
    fenster.setFlags(Qt::FramelessWindowHint);
    fenster.setPosition(0, 0);
    testfenster::platziere(fenster);  // erst der Bildschirm, dann dessen Skalierung
    const qreal dpr = fenster.devicePixelRatio();
    fenster.resize(static_cast<int>(std::lround(groesse.width() / dpr)),
                   static_cast<int>(std::lround(groesse.height() / dpr)));
    fenster.show();

    LeererVisualizer kette;
    faden.setVisualizer(&kette, nullptr);

    auto quelle = std::make_unique<Bildquelle>();
    Bildquelle* q = quelle.get();
    q->setKomposition(komposition);
    const std::int64_t laenge = q->uhr().laengeMs();
    REQUIRE(laenge > 0);
    q->uhr().starte();
    faden.setBildquelle(q, nullptr);

    // Kuenstliche Audiodaten, etwa 60-mal je Sekunde: ein wandernder Ton mit
    // einem Schlag alle halbe Sekunde
    std::vector<float> spektrum(256), welle(576);
    QElapsedTimer lauf;
    lauf.start();
    bool fertig = false;
    while (!fertig && lauf.elapsed() < laenge + 20000)
    {
        const double t = lauf.elapsed() / 1000.0;
        const double schlag = std::exp(-8.0 * std::fmod(t, 0.5));
        for (size_t i = 0; i < welle.size(); ++i)
            welle[i] = static_cast<float>(
                (0.3 + 0.6 * schlag) * std::sin(0.05 * static_cast<double>(i) * (1.0 + std::sin(t)) + 6.0 * t));
        for (size_t i = 0; i < spektrum.size(); ++i)
            spektrum[i] = static_cast<float>(
                std::clamp((0.2 + 0.8 * schlag) * std::exp(-static_cast<double>(i) / 40.0), 0.0, 1.0));
        faden.updateAudio(spektrum.data(), static_cast<int>(spektrum.size()), welle.data(),
                          static_cast<int>(welle.size()));
        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
        QThread::msleep(16);
        QMutexLocker halt(&renderMutex);
        fertig = q->messung().bilder > 0 && !q->uhr().laeuft();
    }

    QStringList zeilen;
    {
        QMutexLocker halt(&renderMutex);
        zeilen = q->messung().alsText();
        zeilen += q->anlaufText();
        for (const QString& zeile : q->bericht()) zeilen << QStringLiteral("  remark: %1").arg(zeile);
    }
    if (!bildraten.empty())
    {
        std::vector<double> sortiert = bildraten;
        std::sort(sortiert.begin(), sortiert.end());
        double summe = 0.0;
        for (double f : sortiert) summe += f;
        zeilen << QStringLiteral("  thread frame rate (unlimited, swap included): mean %1, median %2, "
                                 "lowest %3, highest %4 per second")
                      .arg(summe / static_cast<double>(sortiert.size()), 0, 'f', 0)
                      .arg(sortiert[sortiert.size() / 2], 0, 'f', 0)
                      .arg(sortiert.front(), 0, 'f', 0)
                      .arg(sortiert.back(), 0, 'f', 0);
    }
    zeilen << QStringLiteral("  movement table: %1, %2 table(s) shared")
                  .arg(scheibeMs > 0.0 ? QStringLiteral("slices of %1 ms").arg(scheibeMs)
                                       : QStringLiteral("at once"))
                  .arg(MultiEffectVisualizer::movementTabellenImSpeicher());
    zeilen.prepend(QStringLiteral("Composer measurement: %1").arg(datei));
    for (const QString& zeile : zeilen) MESSAGE(zeile.toStdString());

    const QString aus = qEnvironmentVariable("LUMIVIZ_COMPOSER_MESSUNG_AUS");
    if (!aus.isEmpty())
    {
        QFile f(aus);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text))
            f.write((zeilen.join(QLatin1Char('\n')) + QLatin1Char('\n')).toUtf8());
    }

    CHECK(fertig);
    faden.setBildquelle(nullptr, std::move(quelle));
    faden.stopAndWait();
    fenster.attachThread(nullptr);
}
