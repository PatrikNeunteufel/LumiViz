/**
 ****************************************************************************************
 * @file   test_ComposerSitzung.cpp
 * @brief  Composer, Stufe C2b: die Sitzung — geladen ist nicht an, Transport,
 *         das Ende, der Stand ueber den Neustart
 *         (docs/ui/Composer_Konzept.md §7, §8)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Alles ueber die echte Fassade (`VisualizerWidget` mit Render-Thread und
 * Fenster): nur so zeigt sich, ob das Bild wirklich von der Komposition bzw.
 * wieder von der Kette kommt. Die Einstellungen liegen in einem eigenen
 * QSettings-Bereich und werden am Ende geloescht.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "TestFenster.hpp"
#include "UI/panels/KaraokePanel.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bildquelle.hpp"
#include "composer/Komposition.hpp"
#include "composer/Sitzung.hpp"
#include "services/EventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImage>
#include <QMutexLocker>
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>

#include <climits>
#include <functional>
#include <variant>

using namespace lumi::multieffect;
using namespace lumi::composer;

namespace
{

/// Eigener QSettings-Bereich fuer die Dauer des Tests, danach leer.
struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("ComposerSitzung"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

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

QString kompositionsDatei(const QTemporaryDir& dir, const QString& name, const QString& preset,
                          std::int64_t laengeMs)
{
    VisualClip c;
    c.datei = preset;
    c.startMs = 0;
    c.endeMs = laengeMs;
    Kanal k;
    k.art = KanalArt::Visual;
    k.name = QStringLiteral("CH1");
    k.clips = {c};
    Komposition komposition;
    komposition.kanaele.push_back(k);
    const QString pfad = dir.filePath(name);
    REQUIRE(speichereKomposition(komposition, pfad));
    return pfad;
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

}  // namespace

TEST_SUITE("ComposerSitzung")
{
    TEST_CASE("C2b Sitzung: geladen ist nicht an — Composer on, Transport, Ende und Neustart ueber die Fassade")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString rot = farbPreset(dir, QStringLiteral("Rot.lvfx"), 0xFF0000);
        const QString lang = kompositionsDatei(dir, QStringLiteral("lang.lvcomp"), rot, 60000);
        const QString kurz = kompositionsDatei(dir, QStringLiteral("kurz.lvcomp"), rot, 300);

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
        // sicher ein Bild NACH dem letzten Eingriff
        const auto bildJetzt = [&]() {
            REQUIRE(nimmAuf());
            REQUIRE(nimmAuf());
            return mitte();
        };
        if (!widget.hasVisualizer() || !nimmAuf())
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        const QColor kRot(255, 0, 0);

        CHECK(Sitzung::finde(widget) == nullptr);
        Sitzung& sitzung = Sitzung::fuer(widget);
        CHECK(Sitzung::finde(widget) == &sitzung);
        CHECK(&Sitzung::fuer(widget) == &sitzung);  // eine je Fenster
        int aenderungen = 0;
        QObject::connect(&sitzung, &Sitzung::geaendert, &app, [&] { ++aenderungen; });

        SUBCASE("geladen ist nicht an; Composer on zeigt die gemerkte Stelle, angehalten")
        {
            // ohne Komposition tut nichts etwas
            sitzung.setAn(true);
            sitzung.spiele();
            CHECK_FALSE(sitzung.an());
            CHECK_FALSE(sitzung.geladen());
            CHECK(sitzung.laengeMs() == 0);

            REQUIRE(sitzung.lade(lang));
            CHECK(aenderungen >= 1);
            CHECK(sitzung.geladen());
            CHECK_FALSE(sitzung.an());
            CHECK_FALSE(kompositionLaeuft(widget));
            CHECK(sitzung.laengeMs() == 60000);
            CHECK(sitzung.komposition().kanaele.size() == 1);
            CHECK(bildJetzt() != kRot);  // die Kette liefert das Bild

            // vorbereiten bei ausgeschaltetem Composer: die Stelle wird gemerkt
            sitzung.springe(20000);
            CHECK(sitzung.stelleMs() == 20000);
            sitzung.springe(999999);
            CHECK(sitzung.stelleMs() == 60000);  // geklemmt
            sitzung.springe(20000);

            sitzung.setAn(true);
            REQUIRE(sitzung.an());
            CHECK(kompositionLaeuft(widget));
            CHECK_FALSE(sitzung.laeuft());
            CHECK(sitzung.stelleMs() == 20000);
            CHECK(bildJetzt() == kRot);
            CHECK(sitzung.stelleMs() == 20000);  // steht

            // Play, Pause, Sprung im Lauf, Stopp
            sitzung.spiele();
            CHECK(sitzung.laeuft());  // sofort, nicht erst nach dem naechsten Bild
            REQUIRE(warte([&] { return sitzung.stelleMs() > 20050; }));
            sitzung.pausiere();
            CHECK_FALSE(sitzung.laeuft());
            const std::int64_t gehalten = sitzung.stelleMs();
            REQUIRE(nimmAuf());
            CHECK(sitzung.stelleMs() == gehalten);
            sitzung.spiele();
            sitzung.springe(40000);
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() >= 40000);
            sitzung.stoppe();
            CHECK_FALSE(sitzung.laeuft());
            CHECK(sitzung.stelleMs() == 0);
            CHECK(sitzung.an());  // Stopp schaltet nicht aus

            // Composer aus: die Kette liefert wieder das Bild, die Stelle bleibt
            sitzung.springe(31000);
            sitzung.setAn(false);
            CHECK_FALSE(sitzung.an());
            CHECK(sitzung.geladen());
            CHECK(sitzung.stelleMs() == 31000);
            CHECK(bildJetzt() != kRot);

            // Play bei ausgeschaltetem Composer schaltet ihn ein
            sitzung.spiele();
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() >= 31000);

            sitzung.entlade();
            CHECK_FALSE(sitzung.an());
            CHECK_FALSE(sitzung.geladen());
            CHECK(sitzung.datei().isEmpty());
            sitzung.entlade();  // zweimal schadet nicht
        }

        SUBCASE("C3a bearbeiten: die geaenderte Fassung laeuft an derselben Stelle weiter; gespeichert wird in die Datei")
        {
            // ohne Komposition gibt es nichts zu bearbeiten
            Komposition leer;
            sitzung.setzeKomposition(leer);
            CHECK_FALSE(sitzung.geladen());
            CHECK_FALSE(sitzung.ungespeichert());
            CHECK_FALSE(sitzung.speichere());

            REQUIRE(sitzung.oeffne(lang));
            const int ladung = sitzung.ladung();
            sitzung.springe(20000);
            REQUIRE(sitzung.laeuft());
            int bearbeitet = 0;
            QObject::connect(&sitzung, &Sitzung::kompositionBearbeitet, &app, [&] { ++bearbeitet; });

            // dieselbe Fassung noch einmal: nichts geschieht
            sitzung.setzeKomposition(sitzung.komposition());
            CHECK(bearbeitet == 0);
            CHECK_FALSE(sitzung.ungespeichert());

            // der Clip endet frueher: laeuft weiter, die Stelle bleibt, die Laenge folgt
            Komposition kuerzer = sitzung.komposition();
            kuerzer.kanaele[0].clips[0].endeMs = 40000;
            sitzung.setzeKomposition(kuerzer);
            CHECK(bearbeitet == 1);
            CHECK(sitzung.ungespeichert());
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());
            CHECK(sitzung.laengeMs() == 40000);
            CHECK(sitzung.stelleMs() >= 20000);
            CHECK(sitzung.stelleMs() < 30000);
            CHECK(sitzung.ladung() == ladung);  // bearbeitet ist nicht neu geladen
            CHECK(bildJetzt() == kRot);

            // kuerzer als die Stelle: die Uhr steht am neuen Ende
            kuerzer.kanaele[0].clips[0].endeMs = 5000;
            sitzung.setzeKomposition(kuerzer);
            CHECK(sitzung.stelleMs() == 5000);
            CHECK_FALSE(sitzung.laeuft());

            // zurueck auf die Fassung der Datei: nichts mehr offen
            Komposition wieLang;
            REQUIRE(ladeKomposition(lang, wieLang));
            sitzung.setzeKomposition(wieLang);
            CHECK_FALSE(sitzung.ungespeichert());

            // speichern unter anderem Namen: das ist danach die geladene Datei
            sitzung.setzeKomposition(kuerzer);
            const QString kopie = dir.filePath(QStringLiteral("kopie.lvcomp"));
            REQUIRE(sitzung.speichereNach(kopie));
            CHECK_FALSE(sitzung.ungespeichert());
            CHECK(sitzung.datei() == QFileInfo(kopie).absoluteFilePath());
            Komposition gelesen;
            REQUIRE(ladeKomposition(kopie, gelesen));
            CHECK(gelesen.kanaele[0].clips[0].endeMs == 5000);
            REQUIRE(ladeKomposition(lang, gelesen));
            CHECK(gelesen.kanaele[0].clips[0].endeMs == 60000);  // die alte Datei blieb

            // bei ausgeschaltetem Composer: die gemerkte Stelle bleibt im Clip
            sitzung.setAn(false);
            sitzung.springe(4000);
            kuerzer.kanaele[0].clips[0].endeMs = 2000;
            sitzung.setzeKomposition(kuerzer);
            CHECK(sitzung.stelleMs() == 2000);
            REQUIRE(sitzung.speichere());
            CHECK(sitzung.ladung() == ladung);
        }

        SUBCASE("C3c Sync on Effect Chain: am angeklickten Clip, sonst in Ruhe — und nie an etwas, das es nicht mehr gibt")
        {
            // Der Bus der App: Effect Chain und Visualizer Config folgen diesem Ereignis.
            struct BusFuerDieDauer
            {
                EventBus bus;
                BusFuerDieDauer() { Sitzung::setzeBus(&bus); }
                ~BusFuerDieDauer() { Sitzung::setzeBus(nullptr); }
            } dienst;
            std::vector<void*> gemeldet;
            dienst.bus.subscribe<VisualizerChangedEvent>(
                [&](const VisualizerChangedEvent& e) { gemeldet.push_back(e.visualizerPtr); });
            void* const kette = static_cast<void*>(widget.visualizer());
            const auto alsMeldung = [](MultiEffectVisualizer* r) {
                return static_cast<void*>(static_cast<IVisualizer*>(r));
            };

            // CH1: Rot 0–30 s, Blau 30–60 s
            const QString blau = farbPreset(dir, QStringLiteral("Blau.lvfx"), 0x0000FF);
            Komposition k;
            Kanal kanal;
            kanal.name = QStringLiteral("CH1");
            VisualClip a;
            a.datei = rot;
            a.startMs = 0;
            a.endeMs = 30000;
            VisualClip b = a;
            b.datei = blau;
            b.startMs = 30000;
            b.endeMs = 60000;
            kanal.clips = {a, b};
            k.kanaele = {kanal};
            const QString zwei = dir.filePath(QStringLiteral("zwei.lvcomp"));
            REQUIRE(speichereKomposition(k, zwei));

            // geladen, Composer aus: Effect Chain bleibt an der Kette — nichts wird gemeldet
            REQUIRE(sitzung.lade(zwei));
            CHECK(sitzung.bindung() == Sitzung::Bindung::Kette);
            CHECK(gemeldet.empty());

            // Composer an, Sync aus: Effect Chain ruht (zuletzt gemeldet: nichts)
            CHECK_FALSE(sitzung.sync());
            sitzung.setAn(true);
            CHECK(sitzung.bindung() == Sitzung::Bindung::Ruht);
            REQUIRE(gemeldet.size() == 2);
            CHECK(gemeldet[0] == kette);  // erst die Kette — Visualizer Config uebergeht „nichts"
            CHECK(gemeldet[1] == nullptr);

            // Sync an, aber kein Clip angeklickt: ruht weiter
            sitzung.setSync(true);
            CHECK(QSettings().value(QStringLiteral("composer/syncEffectChain")).toBool());
            CHECK(sitzung.bindung() == Sitzung::Bindung::Ruht);

            // ein Clip angeklickt: Effect Chain haengt an seinem Renderer
            sitzung.waehleClip(0, 0);
            REQUIRE(warte([&] { return sitzung.bindung() == Sitzung::Bindung::Clip; }));
            MultiEffectVisualizer* const erster = sitzung.gebundenerRenderer();
            REQUIRE(erster != nullptr);
            CHECK(gemeldet.back() == alsMeldung(erster));
            CHECK(gemeldet.back() != kette);
            CHECK(bildJetzt() == kRot);

            // seine Kette aendern, wie es Effect Chain tut: sofort im Bild — auch bei stehender Uhr
            CHECK_FALSE(sitzung.laeuft());
            const QColor kGruen(0, 255, 0);
            {
                ChainNode wurzel;
                wurzel.params = ListParams{};
                ChainNode fuellen;
                ClearParams cp;
                cp.color = 0x00FF00;
                fuellen.params = cp;
                wurzel.children.push_back(std::move(fuellen));
                QMutexLocker halt(&widget.renderMutex());
                (void)erster->setChain(std::move(wurzel));
            }
            CHECK(bildJetzt() == kGruen);

            // die Komposition bearbeiten (der Clip wird kuerzer, der andere rueckt vor):
            // derselbe Renderer, die geaenderte Kette bleibt, nichts wird neu gemeldet
            const auto meldungen = gemeldet.size();
            Komposition bearbeitet = sitzung.komposition();
            bearbeitet.kanaele[0].clips[0].endeMs = 20000;
            bearbeitet.kanaele[0].clips[1].startMs = 20000;
            sitzung.setzeKomposition(bearbeitet);
            REQUIRE(warte([&] { return sitzung.laengeMs() == 60000; }));
            for (int i = 0; i < 12; ++i) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            CHECK(bildJetzt() == kGruen);
            CHECK(sitzung.bindung() == Sitzung::Bindung::Clip);
            CHECK(sitzung.gebundenerRenderer() == erster);
            CHECK(gemeldet.size() == meldungen);

            // der Clip zieht an einen anderen Platz (hinter den zweiten): die Auswahl nennt ihn
            // unter seiner neuen Nummer — immer noch derselbe Renderer
            Komposition vertauscht = sitzung.komposition();
            vertauscht.kanaele[0].clips[0].startMs = 70000;
            vertauscht.kanaele[0].clips[0].endeMs = 90000;
            sitzung.waehleClip(0, 1);  // wie das Panel: erst die Auswahl,
            sitzung.setzeKomposition(vertauscht);  // dann die neue Fassung
            for (int i = 0; i < 12; ++i) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            REQUIRE(nimmAuf());
            CHECK(sitzung.komposition().kanaele[0].clips[1].datei == rot);
            CHECK(sitzung.bindung() == Sitzung::Bindung::Clip);
            CHECK(sitzung.gebundenerRenderer() == erster);
            CHECK(gemeldet.size() == meldungen);

            // ein anderer Clip angeklickt: erst zurueck an die Kette, dann an den neuen
            sitzung.waehleClip(0, 0);  // Blau
            REQUIRE(warte([&] {
                return sitzung.bindung() == Sitzung::Bindung::Clip && sitzung.gebundenerRenderer() != erster;
            }));
            MultiEffectVisualizer* const zweiter = sitzung.gebundenerRenderer();
            REQUIRE(zweiter != nullptr);
            REQUIRE(gemeldet.size() >= meldungen + 2);
            CHECK(gemeldet[gemeldet.size() - 2] == kette);
            CHECK(gemeldet.back() == alsMeldung(zweiter));

            // der angeklickte Clip wird geloescht: Effect Chain ruht — der Zeiger, den sie
            // bis dahin hielt, blieb gueltig (hier: der Lauf endet nicht in einem Absturz)
            Komposition ohne = sitzung.komposition();
            ohne.kanaele[0].clips.erase(ohne.kanaele[0].clips.begin());
            sitzung.waehleClip(-1, -1);
            sitzung.setzeKomposition(ohne);
            REQUIRE(warte([&] { return sitzung.bindung() == Sitzung::Bindung::Ruht; }));
            CHECK(gemeldet.back() == nullptr);
            CHECK(gemeldet[gemeldet.size() - 2] == kette);

            // wieder der verbliebene Clip (Rot mit der geaenderten Kette — sie ging mit dem
            // Abwaehlen verloren: er laedt neu aus seiner Datei)
            sitzung.waehleClip(0, 0);
            REQUIRE(warte([&] { return sitzung.bindung() == Sitzung::Bindung::Clip; }));
            CHECK(sitzung.gebundenerRenderer() != nullptr);

            // Sync aus: ruht; Composer aus: zurueck an die Kette der App
            sitzung.setSync(false);
            CHECK(sitzung.bindung() == Sitzung::Bindung::Ruht);
            CHECK(gemeldet.back() == nullptr);
            sitzung.setSync(true);
            REQUIRE(warte([&] { return sitzung.bindung() == Sitzung::Bindung::Clip; }));
            sitzung.setAn(false);
            CHECK(sitzung.bindung() == Sitzung::Bindung::Kette);
            CHECK(gemeldet.back() == kette);
            CHECK(bildJetzt() != kGruen);

            // ohne Bus (das Composer-Panel gab es nie): nichts wird gemeldet, nichts gebunden
            Sitzung::setzeBus(nullptr);
            const auto vorher = gemeldet.size();
            sitzung.setAn(true);
            for (int i = 0; i < 6; ++i) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            CHECK(sitzung.bindung() == Sitzung::Bindung::Kette);
            CHECK(gemeldet.size() == vorher);
            sitzung.entlade();
        }

        SUBCASE("S101 Karaoke-Panel bei laufendem Composer (Befund Patrik): es bleibt an der Ebene der App — und der Composer zieht nach")
        {
            using lumi::multieffect::LyricsParams;
            MultiEffectVisualizer::setKaraokeAblage(dir.filePath(QStringLiteral("karaoke/current.lvkar")));
            MultiEffectVisualizer::setKaraokeEbene(false);
            struct Aufraeumen
            {
                ~Aufraeumen()
                {
                    Sitzung::setzeBus(nullptr);
                    MultiEffectVisualizer::setKaraokeEbene(false);
                    MultiEffectVisualizer::setKaraokeAblage(QString());
                }
            } aufraeumen;

            // die Karaoke-Ebene gibt es nur am Multi Effect (das Fenster beginnt mit Pulsing)
            REQUIRE(widget.setVisualizer(QStringLiteral("multieffect")));
            REQUIRE(nimmAuf());
            auto* const kette = dynamic_cast<MultiEffectVisualizer*>(widget.visualizer());
            REQUIRE(kette != nullptr);
            const auto yShiftVon = [&](const MultiEffectVisualizer* r) {
                QMutexLocker halt(&widget.renderMutex());
                return std::get<LyricsParams>(r->karaokeKette().children[0].params).yShift;
            };
            const int vorgabe = yShiftVon(kette);
            // zwei Karaoke-Presets, am Feld „Y shift" zu unterscheiden
            const auto presetMit = [&](const QString& name, int yShift) {
                MultiEffectVisualizer v;
                std::get<LyricsParams>(v.karaokeKette().children[0].params).yShift = yShift;
                const QString pfad = dir.filePath(name);
                REQUIRE(v.speichereKaraokeDatei(pfad));
                return pfad;
            };
            const QString eins = presetMit(QStringLiteral("Eins.lvkar"), vorgabe - 20);
            const QString zwei = presetMit(QStringLiteral("Zwei.lvkar"), vorgabe - 40);

            // ein Bus fuer Panel und Composer, wie in der App
            ServiceContainer panelDienste;
            panelDienste.registerInstance<IEventBus>(std::make_unique<EventBus>());
            IEventBus* const bus = panelDienste.tryResolve<IEventBus>();
            Sitzung::setzeBus(bus);
            KaraokePanel panel(panelDienste);
            panel.resize(520, 700);
            testfenster::platziere(panel);
            panel.show();
            bus->publish(VisualizerChangedEvent("multieffect", "Multi Effect", widget.visualizer(),
                                                &widget.renderMutex()));
            QCoreApplication::processEvents();

            // Composer aus: laden geht, wie immer
            REQUIRE(panel.ladeDatei(eins));
            CHECK(yShiftVon(kette) == vorgabe - 20);

            // Composer an, Sync aus: Effect Chain ruht — das Karaoke-Panel nicht
            REQUIRE(sitzung.lade(lang));
            sitzung.setAn(true);
            CHECK(sitzung.bindung() == Sitzung::Bindung::Ruht);
            // die Ebene an: der Composer rechnet sie in einem eigenen Renderer
            // (dem Traeger) — er zeigt die Ebene der App
            MultiEffectVisualizer::setKaraokeEbene(true);
            const auto yShiftTraeger = [&]() {
                QMutexLocker halt(&widget.renderMutex());
                const auto* q = dynamic_cast<const Bildquelle*>(widget.bildquelle());
                const MultiEffectVisualizer* t = q != nullptr ? q->karaokeTraegerNurSehen() : nullptr;
                return t != nullptr ? std::get<LyricsParams>(t->karaokeKette().children[0].params).yShift
                                    : INT_MIN;
            };
            REQUIRE(warte([&] { return yShiftTraeger() == vorgabe - 20; }));

            CHECK(panel.ladeDatei(zwei));
            CHECK(yShiftVon(kette) == vorgabe - 40);
            CHECK(warte([&] { return yShiftTraeger() == vorgabe - 40; }));  // der Composer zieht nach

            // eine Einstellung, wie sie der Editor des Panels macht — ohne Datei
            {
                QMutexLocker halt(&widget.renderMutex());
                std::get<LyricsParams>(kette->karaokeKette().children[0].params).yShift = vorgabe - 60;
                (void)kette->recompileKaraoke();
                kette->merkeKaraokeStand();  // sonst fragt das naechste Laden nach dem Ungespeicherten
            }
            CHECK(warte([&] { return yShiftTraeger() == vorgabe - 60; }));

            // Sync an einem Clip: Effect Chain haengt an seinem Renderer — das
            // Karaoke-Panel weiter an der Ebene der App
            sitzung.setSync(true);
            sitzung.waehleClip(0, 0);
            REQUIRE(warte([&] { return sitzung.bindung() == Sitzung::Bindung::Clip; }));
            CHECK(panel.ladeDatei(eins));
            CHECK(yShiftVon(kette) == vorgabe - 20);
            CHECK(warte([&] { return yShiftTraeger() == vorgabe - 20; }));

            // Ebene aus und wieder an: der neue Traeger beginnt mit der Ebene
            // der App, nicht mit dem Stand der Ablage
            {
                QMutexLocker halt(&widget.renderMutex());
                std::get<LyricsParams>(kette->karaokeKette().children[0].params).yShift = vorgabe - 80;
                (void)kette->recompileKaraoke();
            }
            MultiEffectVisualizer::setKaraokeEbene(false);
            REQUIRE(warte([&] { return yShiftTraeger() == INT_MIN; }));
            MultiEffectVisualizer::setKaraokeEbene(true);
            CHECK(warte([&] { return yShiftTraeger() == vorgabe - 80; }));

            sitzung.setSync(false);
            sitzung.setAn(false);
            sitzung.entlade();
        }

        SUBCASE("C3c Speichern: die Kette eines Clips geht unter neuem Namen weg — das Original bleibt, gewaehlte Clips wechseln mit")
        {
            struct BusFuerDieDauer
            {
                EventBus bus;
                BusFuerDieDauer() { Sitzung::setzeBus(&bus); }
                ~BusFuerDieDauer() { Sitzung::setzeBus(nullptr); }
            } dienst;
            int gemeldet = 0;
            dienst.bus.subscribe<VisualizerChangedEvent>([&](const VisualizerChangedEvent&) { ++gemeldet; });

            // dreimal dasselbe Preset: CH1 0–10 s und 20–30 s, CH2 0–10 s
            Komposition k;
            Kanal eins;
            eins.name = QStringLiteral("CH1");
            VisualClip a;
            a.datei = rot;
            a.startMs = 0;
            a.endeMs = 10000;
            VisualClip b = a;
            b.startMs = 20000;
            b.endeMs = 30000;
            eins.clips = {a, b};
            Kanal zwei;
            zwei.name = QStringLiteral("CH2");
            zwei.clips = {a};
            k.kanaele = {eins, zwei};
            const QString datei = dir.filePath(QStringLiteral("drei.lvcomp"));
            REQUIRE(speichereKomposition(k, datei));
            REQUIRE(sitzung.lade(datei));

            // ohne gebundenen Clip gibt es nichts zu speichern
            const QString gruenDatei = dir.filePath(QStringLiteral("Gruen.lvfx"));
            CHECK(sitzung.gebundeneDatei().isEmpty());
            CHECK(sitzung.andereClipsMitDerDatei().empty());
            CHECK_FALSE(sitzung.speichereClipKette(gruenDatei));
            CHECK_FALSE(clipKetteGebunden(widget));

            sitzung.setSync(true);
            sitzung.setAn(true);
            sitzung.waehleClip(0, 0);
            REQUIRE(warte([&] { return sitzung.bindung() == Sitzung::Bindung::Clip; }));
            CHECK(clipKetteGebunden(widget));
            MultiEffectVisualizer* const renderer = sitzung.gebundenerRenderer();
            CHECK(QFileInfo(sitzung.gebundeneDatei()) == QFileInfo(rot));
            CHECK_FALSE(sitzung.gebundeneKetteHatHostGruppe());
            const auto andere = sitzung.andereClipsMitDerDatei();
            REQUIRE(andere.size() == 2);
            CHECK(andere[0] == std::pair<int, int>{0, 1});
            CHECK(andere[1] == std::pair<int, int>{1, 0});

            // die Kette gruen machen (wie in Effect Chain)
            {
                ChainNode wurzel;
                wurzel.params = ListParams{};
                ChainNode fuellen;
                ClearParams cp;
                cp.color = 0x00FF00;
                fuellen.params = cp;
                wurzel.children.push_back(std::move(fuellen));
                QMutexLocker halt(&widget.renderMutex());
                (void)renderer->setChain(std::move(wurzel));
                // (`setChain` merkt den Stand selbst als gespeichert — ob die
                // Kette als geaendert gilt, prueft dieser Test darum nicht)
            }

            // ueber das Original geht es nicht
            CHECK_FALSE(sitzung.speichereClipKette(rot));
            CHECK(QFileInfo(sitzung.gebundeneDatei()) == QFileInfo(rot));

            // unter neuem Namen: dieser Clip und der in CH2 wechseln, der zweite in CH1 nicht
            const int meldungen = gemeldet;
            REQUIRE(sitzung.speichereClipKette(gruenDatei, {{1, 0}}));
            const auto& kanaele = sitzung.komposition().kanaele;
            CHECK(QFileInfo(kanaele[0].clips[0].datei) == QFileInfo(gruenDatei));
            CHECK(QFileInfo(kanaele[1].clips[0].datei) == QFileInfo(gruenDatei));
            CHECK(QFileInfo(kanaele[0].clips[1].datei) == QFileInfo(rot));
            CHECK(sitzung.ungespeichert());  // die Komposition verweist jetzt anders
            {
                QMutexLocker halt(&widget.renderMutex());
                CHECK_FALSE(renderer->ketteGeaendert());  // die Kette gilt als gespeichert
            }
            // die neue Datei traegt die geaenderte Kette, das Original die alte
            ChainNode gelesen;
            QStringList bericht;
            REQUIRE(loadChainFromFile(gruenDatei, gelesen, &bericht));
            REQUIRE(gelesen.children.size() == 1);
            CHECK(std::get<ClearParams>(gelesen.children[0].params).color == 0x00FF00);
            REQUIRE(loadChainFromFile(rot, gelesen, &bericht));
            CHECK(std::get<ClearParams>(gelesen.children[0].params).color == 0xFF0000);

            // Effect Chain haengt weiter am selben Renderer — nichts wurde neu gemeldet
            for (int i = 0; i < 12; ++i) QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
            CHECK(sitzung.bindung() == Sitzung::Bindung::Clip);
            CHECK(sitzung.gebundenerRenderer() == renderer);
            CHECK(gemeldet == meldungen);
            CHECK(sitzung.andereClipsMitDerDatei().size() == 1);  // jetzt: der Clip in CH2
            // im Bild: CH1 gruen, darueber CH2 — auch gruen, neu geladen aus der neuen Datei
            CHECK(bildJetzt() == QColor(0, 255, 0));
            // an der Stelle des zweiten CH1-Clips laeuft weiter das Original
            sitzung.springe(25000);
            CHECK(bildJetzt() == kRot);
            sitzung.entlade();
        }

        SUBCASE("Oeffnen von Hand: spielt sofort — oder haelt am Anfang (Einstellung)")
        {
            CHECK_FALSE(Sitzung::oeffnenHaelt());
            REQUIRE(sitzung.oeffne(lang));
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());

            Sitzung::setOeffnenHaelt(true);
            REQUIRE(sitzung.oeffne(lang));
            CHECK(sitzung.an());
            CHECK_FALSE(sitzung.laeuft());
            CHECK(sitzung.stelleMs() == 0);
            CHECK(bildJetzt() == kRot);

            // keine Komposition: es bleibt alles, wie es war
            CHECK_FALSE(sitzung.oeffne(rot));
            CHECK(sitzung.an());
            CHECK(sitzung.datei() == QFileInfo(lang).absoluteFilePath());
        }

        SUBCASE("das Ende: halten, an den Anfang, Composer aus — und die Schleife")
        {
            CHECK(Sitzung::amEnde() == AmEnde::Halten);
            REQUIRE(sitzung.oeffne(kurz));
            REQUIRE(warte([&] { return !sitzung.laeuft(); }));
            CHECK(sitzung.an());
            CHECK(sitzung.stelleMs() == 300);
            REQUIRE(nimmAuf());
            CHECK(sitzung.an());  // bleibt an
            sitzung.spiele();     // am Ende: von vorn
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() < 300);

            Sitzung::setAmEnde(AmEnde::AnDenAnfang);
            CHECK(Sitzung::amEnde() == AmEnde::AnDenAnfang);
            REQUIRE(warte([&] { return !sitzung.laeuft() && sitzung.stelleMs() == 0; }));
            CHECK(sitzung.an());

            Sitzung::setAmEnde(AmEnde::ComposerAus);
            sitzung.spiele();
            REQUIRE(warte([&] { return !sitzung.an(); }));
            CHECK(sitzung.geladen());  // die Komposition bleibt im Panel
            CHECK(sitzung.stelleMs() == 0);
            CHECK(bildJetzt() != kRot);

            // Schleife: laeuft ueber das Ende hinweg weiter, auch bei „Composer aus"
            sitzung.setSchleife(true);
            CHECK(QSettings().value(QStringLiteral("composer/loop")).toBool());
            sitzung.spiele();
            QElapsedTimer uhr;
            uhr.start();
            REQUIRE(warte([&] { return uhr.elapsed() > 800; }));
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() < 300);
            // Schleife aus im Lauf: haelt wieder am Ende — hier: Composer aus
            sitzung.setSchleife(false);
            REQUIRE(warte([&] { return !sitzung.an(); }));
        }

        SUBCASE("der Neustart: ohne Resume ins Panel, Composer aus — mit Resume der Stand vom Beenden")
        {
            CHECK_FALSE(Sitzung::resumeAn());
            CHECK_FALSE(sitzung.stelleWiederHer());  // es gab nie eine

            REQUIRE(sitzung.oeffne(lang));
            sitzung.springe(25000);
            sitzung.sichereStand();  // wie bei aboutToQuit
            sitzung.entlade();       // „neu gestartet"

            // Resume aus: die Komposition liegt im Panel, die Kette laeuft
            REQUIRE(sitzung.stelleWiederHer());
            CHECK(sitzung.geladen());
            CHECK(sitzung.datei() == QFileInfo(lang).absoluteFilePath());
            CHECK_FALSE(sitzung.an());
            CHECK(sitzung.stelleMs() == 0);
            CHECK(bildJetzt() != kRot);

            // Resume an: lief sie, laeuft sie von der Stelle an weiter
            Sitzung::setResume(true);
            sitzung.entlade();
            REQUIRE(sitzung.stelleWiederHer());
            CHECK(sitzung.an());
            CHECK(sitzung.laeuft());
            CHECK(sitzung.stelleMs() >= 25000);
            CHECK(sitzung.stelleMs() < 40000);
            CHECK(bildJetzt() == kRot);

            // angehalten beendet: an der Stelle, angehalten
            sitzung.pausiere();
            sitzung.springe(12000);
            sitzung.sichereStand();
            sitzung.entlade();
            REQUIRE(sitzung.stelleWiederHer());
            CHECK(sitzung.an());
            CHECK_FALSE(sitzung.laeuft());
            CHECK(sitzung.stelleMs() == 12000);

            // Composer aus beendet: geladen, aus, die Stelle gemerkt
            sitzung.setAn(false);
            sitzung.sichereStand();
            sitzung.entlade();
            REQUIRE(sitzung.stelleWiederHer());
            CHECK_FALSE(sitzung.an());
            CHECK(sitzung.stelleMs() == 12000);

            // ohne Komposition beendet: der fruehere Stand bleibt nicht stehen
            sitzung.entlade();
            sitzung.sichereStand();
            CHECK_FALSE(sitzung.stelleWiederHer());

            // der Weg des Starts (Naht N4) ueber die Fassade
            REQUIRE(sitzung.oeffne(lang));
            sitzung.sichereStand();
            sitzung.entlade();
            CHECK(stelleKompositionWiederHer(widget));
            CHECK(kompositionLaeuft(widget));
        }
    }
}
