/**
 ****************************************************************************************
 * @file   test_ComposerAbgleich.cpp
 * @brief  Composer, Lautstaerke-Abgleich G5: das Modell und der Dialog Match
 *         loudness (docs/ui/Composer_Konzept.md §9g, G5-1 a bis G5-4 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * - Das Modell ohne Fenster: Zeilen, Vorschlag zum Clip und zu Fixed level,
 *   Spitze mit Vorschlag, Use suggestions, Neu-Lesen nach einer Aenderung.
 * - Der Dialog im echten Panel, mit der Engine aus Pappe (sie „dekodiert"
 *   einen Sinus je Datei): Knopf neben Gain, Edit grau, Messen im Hintergrund,
 *   Vorhoeren ohne Undo, Listen A/B, ▶, Ctrl+Z bei offenem Dialog, OK als ein
 *   Schritt, Rechtsklick mit Ziel, gemerkte Songs, Cancel. Kein Ton; das
 *   Fenster des Composers oeffnet sich (Bildschirm 2 mit
 *   `LUMIVIZ_REF_BILDSCHIRM`). `LUMIVIZ_MATCH_LOUDNESS_BILD=<datei.png>` legt
 *   ein Bild des Dialogs ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "PappTon.hpp"
#include "TestFenster.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "composer/Abgleich.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/ClipLautheit.hpp"
#include "composer/ComposerPanel.hpp"
#include "composer/Komposition.hpp"
#include "composer/LautheitsDialog.hpp"
#include "composer/Sitzung.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QFile>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>

#include <cstdlib>
#include <functional>

using namespace lumi::composer;

namespace
{

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("ComposerAbgleich"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

Clip clipVon(const QString& datei, std::int64_t start, std::int64_t ende)
{
    Clip c;
    c.datei = datei;
    c.startMs = start;
    c.endeMs = ende;
    return c;
}

Lautheit lautheit(double lufs, double spitze)
{
    Lautheit l;
    l.lufs = lufs;
    l.spitzeDb = spitze;
    l.stille = false;
    return l;
}

bool warte(const std::function<bool()>& bedingung, int hoechstensMs = 15000)
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

/// Bild-Kanal, A1 mit zwei Songs, A2 mit einem dritten Song.
Komposition probe()
{
    Komposition k;
    Kanal bild;
    bild.name = QStringLiteral("CH1");
    bild.clips = {clipVon(QStringLiteral("C:/p/rot.lvfx"), 0, 60000)};
    Kanal a1;
    a1.name = QStringLiteral("A1");
    a1.art = KanalArt::Audio;
    a1.clips = {clipVon(QStringLiteral("C:/m/eins.mp3"), 0, 30000), clipVon(QStringLiteral("C:/m/zwei.mp3"), 32000, 60000)};
    a1.clips[1].einsatzMs = 5000;
    Kanal a2;
    a2.name = QStringLiteral("A2");
    a2.art = KanalArt::Audio;
    a2.an = false;
    a2.clips = {clipVon(QStringLiteral("C:/m/drei.mp3"), 10000, 20000)};
    k.kanaele = {bild, a1, a2};
    return k;
}

}  // namespace

TEST_SUITE("ComposerAbgleich")
{
    TEST_CASE("G5 Abgleich: Zeilen, Vorschlag zum Clip und zu Fixed level, Spitze, Use suggestions")
    {
        Abgleich a(probe());
        REQUIRE(a.zeilen().size() == 3);  // nur Ton-Clips
        CHECK(a.zeilen()[0].kanal == 1);
        CHECK(a.zeilen()[1].clip == 1);
        CHECK(a.zeilen()[1].einsatzMs == 5000);
        CHECK(a.zeilen()[1].dauerMs == 28000);
        CHECK(a.zeilen()[1].name() == QStringLiteral("zwei"));
        CHECK_FALSE(a.zeilen()[2].spielt);  // A2 ist aus: grau, gemessen wird trotzdem
        CHECK(a.zeileVon(1, 1) == 1);
        CHECK(a.zeileVon(0, 0) == -1);

        // ohne Messung: kein Vorschlag, keine lauteste
        CHECK(a.lauteste() == -1);
        CHECK_FALSE(a.vorschlag(0).has_value());

        a.zeilen()[0].messung = lautheit(-9.0, -0.5);
        a.zeilen()[1].messung = lautheit(-21.04, -8.0);
        a.zeilen()[2].messung = lautheit(-14.0, -1.0);
        CHECK(a.lauteste() == 0);

        // Ziel ein Clip: er 0 dB, die anderen der Abstand, auf ein Zehntel
        a.setzeZiel(0);
        CHECK(*a.vorschlag(0) == 0.0);
        CHECK(*a.vorschlag(1) == doctest::Approx(12.0));
        CHECK(*a.vorschlag(2) == doctest::Approx(5.0));
        // die Spitze mit Vorschlag — ueber 0 dB warnt der Dialog (L4 a)
        CHECK(*a.spitzeMitVorschlag(1) == doctest::Approx(4.0));
        CHECK(*a.spitzeMitVorschlag(2) == doctest::Approx(4.0));
        CHECK(*a.spitzeMitVorschlag(0) == doctest::Approx(-0.5));

        // Fixed level
        a.setzeZiel(-1);
        a.setzeFestLufs(-14.0);
        CHECK(*a.vorschlag(0) == doctest::Approx(-5.0));
        CHECK(*a.vorschlag(2) == doctest::Approx(0.0));
        // geklemmt auf ±24 dB
        a.zeilen()[1].messung = lautheit(-60.0, -50.0);
        CHECK(*a.vorschlag(1) == doctest::Approx(kGainMaxDb));
        // still oder nicht gemessen: kein Vorschlag; ein stilles Ziel: keiner fuer alle
        Lautheit still;
        a.zeilen()[1].messung = still;
        CHECK_FALSE(a.vorschlag(1).has_value());
        a.setzeZiel(1);
        CHECK_FALSE(a.vorschlag(0).has_value());
        a.zeilen()[1].messung = lautheit(-21.04, -8.0);

        // Use suggestions, mitGains, geaendert
        a.setzeZiel(0);
        CHECK_FALSE(a.geaendert());
        CHECK(a.uebernimmVorschlaege() == 2);
        CHECK(a.geaendert());
        CHECK(a.uebernimmVorschlaege() == 0);
        const Komposition mit = a.mitGains(probe(), true);
        CHECK(mit.kanaele[1].clips[1].gainDb == doctest::Approx(12.0));
        CHECK(mit.kanaele[2].clips[0].gainDb == doctest::Approx(5.0));
        CHECK(mit.kanaele[0].clips[0].gainDb == 0.0);  // ein Bild-Clip bleibt
        const Komposition ohne = a.mitGains(mit, false);
        CHECK(ohne.kanaele[1].clips[1].gainDb == 0.0);
        CHECK(gleich(ohne, probe()));
    }

    TEST_CASE("G5 Abgleich: neu lesen nach einer Aenderung — Messung, Gain im Dialog und Ziel folgen ihrem Clip")
    {
        Komposition k = probe();
        k.kanaele[1].clips[0].gainDb = 6.0;
        Abgleich a(k);
        a.zeilen()[0].messung = lautheit(-9.0, -0.5);
        a.zeilen()[1].messung = lautheit(-21.0, -8.0);
        a.zeilen()[2].messung = lautheit(-14.0, -1.0);
        a.setzeZiel(2);
        a.zeilen()[1].gainDb = 12.0;  // im Dialog geaendert

        // Ctrl+Z: eins wieder auf 0 dB, drei liegt jetzt vor eins in A1
        Komposition danach = k;
        danach.kanaele[1].clips[0].gainDb = 0.0;
        Clip drei = danach.kanaele[2].clips[0];
        danach.kanaele[2].clips.clear();
        Clip neu = clipVon(QStringLiteral("C:/m/vier.mp3"), 0, 5000);
        danach.kanaele[2].clips = {neu};
        danach.kanaele[1].clips.insert(danach.kanaele[1].clips.begin(), drei);
        danach.kanaele[1].clips[1].startMs = 22000;  // eins verschoben: derselbe Ausschnitt, gleicher Clip
        danach.kanaele[1].clips[1].endeMs = 52000;
        danach.kanaele[1].clips[2].startMs = 53000;
        danach.kanaele[1].clips[2].endeMs = 81000;

        const std::vector<int> messen = a.liesNeu(danach);
        REQUIRE(a.zeilen().size() == 4);
        // drei, eins, zwei in A1, vier in A2
        CHECK(a.zeilen()[0].name() == QStringLiteral("drei"));
        CHECK(a.zeilen()[1].name() == QStringLiteral("eins"));
        CHECK(a.zeilen()[3].name() == QStringLiteral("vier"));
        CHECK(a.zeilen()[0].messung.has_value());
        CHECK(a.zeilen()[1].messung.has_value());
        CHECK(messen == std::vector<int>{3});  // nur vier ist neu
        // eins: „vorher" ist jetzt 0 dB (das Undo), im Dialog nicht geaendert — folgt
        CHECK(a.zeilen()[1].gainVorherDb == 0.0);
        CHECK(a.zeilen()[1].gainDb == 0.0);
        // zwei: im Dialog geaendert — bleibt
        CHECK(a.zeilen()[2].gainDb == doctest::Approx(12.0));
        // das Ziel folgt drei
        CHECK(a.ziel() == 0);

        // das Ziel verschwindet: dann der lauteste
        Komposition ohneDrei = danach;
        ohneDrei.kanaele[1].clips.erase(ohneDrei.kanaele[1].clips.begin());
        (void)a.liesNeu(ohneDrei);
        CHECK(a.ziel() == a.lauteste());
        CHECK(a.zeilen()[a.ziel()].name() == QStringLiteral("eins"));
        // Fixed level bleibt Fixed level
        a.setzeZiel(-1);
        (void)a.liesNeu(danach);
        CHECK(a.ziel() == -1);
    }

    TEST_CASE("G5 Dialog Match loudness im Panel: messen, vorhoeren, A/B, Play, Ctrl+Z, OK, Ziel, Cancel")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        SongLautheiten::setzeDateiFuerTests(dir.filePath(QStringLiteral("cache/loudness.json")));
        struct Zurueck
        {
            ~Zurueck() { SongLautheiten::setzeDateiFuerTests({}); }
        } zurueck;

        // ein Preset fuer den Bild-Kanal
        lumi::multieffect::ChainNode wurzel;
        wurzel.params = lumi::multieffect::ListParams{};
        const QString rot = dir.filePath(QStringLiteral("Rot.lvfx"));
        REQUIRE(lumi::multieffect::saveChainToFile(wurzel, rot));
        const QString eins = dir.filePath(QStringLiteral("Titanium.mp3"));
        const QString zwei = dir.filePath(QStringLiteral("Leise.mp3"));
        for (const QString& pfad : {eins, zwei})
        {
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
        }
        const QString loop = looptest::repoWurzel().filePath(QStringLiteral("asset/loop/effects_demo.lvloop"));
        REQUIRE(QFile::exists(loop));

        Komposition k;
        Kanal bild;
        bild.name = QStringLiteral("CH1");
        bild.clips = {clipVon(rot, 0, 60000)};
        Kanal a1;
        a1.name = QStringLiteral("A1");
        a1.art = KanalArt::Audio;
        a1.clips = {clipVon(eins, 0, 30000), clipVon(zwei, 32000, 60000)};
        a1.clips[0].quellMs = 30000;
        a1.clips[1].quellMs = 28000;
        Kanal a2;
        a2.name = QStringLiteral("A2");
        a2.art = KanalArt::Audio;
        a2.clips = {clipVon(loop, 0, 20000)};
        k.kanaele = {bild, a1, a2};
        const QString datei = dir.filePath(QStringLiteral("abgleich.lvcomp"));
        REQUIRE(speichereKomposition(k, datei));

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        dienste.registerSingleton<IAudioEngine>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new papp::PappEngine); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        auto* engine = dynamic_cast<papp::PappEngine*>(dienste.tryResolve<IAudioEngine>());
        REQUIRE(bus != nullptr);
        REQUIRE(engine != nullptr);
        engine->laengen = {{eins, 30000}, {zwei, 28000}};
        engine->pegelDbfs = {{eins, -14.0}, {zwei, -23.0}};

        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        ComposerPanel panel(dienste);
        panel.setFenster(&widget);
        panel.resize(900, 420);
        testfenster::platziere(panel);
        panel.show();
        REQUIRE(warte([&] { return panel.zeitachse()->width() > 400; }));
        Sitzung* sitzung = panel.sitzung();
        Zeitachse* achse = panel.zeitachse();
        auto* edit = panel.findChild<QCheckBox*>(QStringLiteral("composerEdit"));
        auto* knopf = panel.findChild<QPushButton*>(QStringLiteral("composerMatchLoudness"));
        REQUIRE(edit != nullptr);
        REQUIRE(knopf != nullptr);
        REQUIRE(sitzung->lade(datei));
        const auto gainVon = [&](int kanal, int c) {
            return sitzung->komposition().kanaele[static_cast<std::size_t>(kanal)].clips[static_cast<std::size_t>(c)].gainDb;
        };
        const auto waehle = [&](std::int64_t ms, int zeile) {
            const QPoint a(achse->xVon(ms), achse->yVonZeile(zeile));
            QMouseEvent druck(QEvent::MouseButtonPress, QPointF(a), QPointF(achse->mapToGlobal(a)), Qt::LeftButton,
                              Qt::LeftButton, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &druck);
            QMouseEvent los(QEvent::MouseButtonRelease, QPointF(a), QPointF(achse->mapToGlobal(a)), Qt::LeftButton,
                            Qt::NoButton, Qt::NoModifier);
            QCoreApplication::sendEvent(achse, &los);
        };

        // --- der Knopf: immer da, sobald eine Komposition geladen ist (Wunsch Patrik
        //     nach G5-4 a), waehlbar nur mit Edit
        CHECK(knopf->isVisible());  // nichts angeklickt
        CHECK_FALSE(knopf->isEnabled());
        waehle(10000, 0);
        CHECK(knopf->isVisible());  // ein Bild-Clip
        waehle(40000, 1);
        REQUIRE(achse->gewaehlterClip() == 1);
        CHECK(knopf->isVisible());
        CHECK_FALSE(knopf->isEnabled());
        CHECK(panel.oeffneAbgleich(1, 1, false) == nullptr);  // ohne Edit nicht
        edit->click();
        REQUIRE(achse->bearbeiten());
        // mit einem Bild-Clip angeklickt: waehlbar, keine Zeile markiert
        waehle(10000, 0);
        CHECK(knopf->isEnabled());
        knopf->click();
        REQUIRE(panel.abgleichDialog() != nullptr);
        CHECK(panel.abgleichDialog()->markiert() == -1);
        REQUIRE(warte([&] { return !panel.abgleichDialog()->misst(); }));  // die Songs sind danach gemerkt
        panel.abgleichDialog()->reject();
        REQUIRE(warte([&] { return panel.abgleichDialog() == nullptr; }));
        // vorher ein eigener Schritt: eins auf +6 dB (fuer Ctrl+Z bei offenem Dialog)
        waehle(10000, 1);
        panel.gainFeld()->setValue(6.0);
        REQUIRE(gainVon(1, 0) == doctest::Approx(6.0));
        waehle(40000, 1);
        CHECK(knopf->isEnabled());
        const std::size_t schritteVorher = bus->undoCount();
        knopf->click();
        LautheitsDialog* dialog = panel.abgleichDialog();
        REQUIRE(dialog != nullptr);

        // --- offen: Edit grau, der Haken bleibt, die Zeitachse bearbeitet nicht
        CHECK_FALSE(edit->isEnabled());
        CHECK(edit->isChecked());
        CHECK_FALSE(achse->bearbeiten());
        CHECK_FALSE(panel.gainFeld()->isEnabled());

        // --- messen im Hintergrund
        REQUIRE(warte([&] { return !dialog->misst(); }));
        const Abgleich& a = dialog->abgleich();
        REQUIRE(a.zeilen().size() == 3);
        CHECK(dialog->markiert() == 1);  // zwei, der Clip neben dessen Gain geklickt wurde
        REQUIRE(a.zeilen()[0].messung.has_value());
        REQUIRE(a.zeilen()[1].messung.has_value());
        REQUIRE(a.zeilen()[2].messung.has_value());
        CHECK(a.zeilen()[0].messung->lufs == doctest::Approx(-14.0).epsilon(0.01));
        CHECK(a.zeilen()[1].messung->lufs == doctest::Approx(-23.0).epsilon(0.01));
        CHECK(a.zeilen()[2].loop);
        CHECK_FALSE(a.zeilen()[2].messung->stille);
        MESSAGE("effects_demo im Dialog: " << a.zeilen()[2].messung->lufs << " LUFS");
        CHECK(a.ziel() == a.lauteste());  // L3 a: zuerst der lauteste
        const int dekodiertBeimErstenMal = engine->dekodiert.load();
        CHECK(dekodiertBeimErstenMal == 2);  // zwei Songs, beim ersten Oeffnen; jetzt aus dem Speicher

        // --- Ziel eins (ueber die Liste): zwei bekommt +9 dB vorgeschlagen
        auto* ziel = dialog->findChild<QComboBox*>(QStringLiteral("matchTarget"));
        REQUIRE(ziel != nullptr);
        CHECK(ziel->count() == 4);  // drei Clips und Fixed level
        ziel->setCurrentIndex(0);
        Q_EMIT ziel->activated(0);
        CHECK(a.ziel() == 0);
        CHECK(*a.vorschlag(1) == doctest::Approx(9.0));
        // −23 + 9 = −14 dBFS Spitze: keine Warnung fuer Leise; der Loop aber kaeme
        // mit seinem Vorschlag ueber 0 dB (seine Spitze liegt weit ueber seiner Lautheit)
        CHECK_FALSE(dialog->warnText().contains(QStringLiteral("Leise")));
        CHECK(*a.spitzeMitVorschlag(2) > 0.0);
        CHECK(dialog->warnText().startsWith(QStringLiteral("effects_demo: peak would reach +")));
        CHECK(dialog->warnText().endsWith(QStringLiteral("the limiter keeps it below 0 dB.")));

        // --- vorhoeren: sofort in der Sitzung, kein Undo-Schritt
        dialog->setzeGain(1, 9.0);
        CHECK(gainVon(1, 1) == doctest::Approx(9.0));
        CHECK(bus->undoCount() == schritteVorher);
        CHECK(sitzung->ungespeichert());
        // Listen A/B
        auto* ab = dialog->findChild<QPushButton*>(QStringLiteral("matchListenAB"));
        REQUIRE(ab != nullptr);
        CHECK(ab->text() == QStringLiteral("Hearing: B (dialog)"));
        ab->click();
        CHECK(dialog->hoertVorher());
        CHECK(ab->text() == QStringLiteral("Hearing: A (before)"));
        CHECK(gainVon(1, 1) == 0.0);
        CHECK(gainVon(1, 0) == doctest::Approx(6.0));
        ab->click();
        CHECK(gainVon(1, 1) == doctest::Approx(9.0));

        // --- ▶: die Mitte von zwei, der Composer spielt
        dialog->spieleZeile(1);
        CHECK(sitzung->laeuft());
        CHECK(std::llabs(sitzung->stelleMs() - 46000) < 1000);
        sitzung->pausiere();

        // --- Use suggestions: der Loop und eins (das Ziel: 0 dB) kommen dazu
        dialog->nimmVorschlaege();
        CHECK(gainVon(1, 0) == 0.0);  // eins ist das Ziel
        CHECK(gainVon(2, 0) == doctest::Approx(*a.vorschlag(2)));
        CHECK(bus->undoCount() == schritteVorher);

        if (const QString bild = qEnvironmentVariable("LUMIVIZ_MATCH_LOUDNESS_BILD"); !bild.isEmpty())
        {
            QCoreApplication::processEvents();
            CHECK(dialog->grab().save(bild));
            CHECK(panel.grab().save(bild + QStringLiteral(".panel.png")));
        }

        // --- Ctrl+Z bei offenem Dialog: eins zurueck auf 0 dB — die Liste neu, nichts neu dekodiert
        REQUIRE(bus->undo());
        CHECK(a.zeilen().size() == 3);
        CHECK(a.zeilen()[0].gainVorherDb == 0.0);
        CHECK(a.zeilen()[1].gainDb == doctest::Approx(9.0));  // im Dialog bleibt, was dort steht
        CHECK(gainVon(1, 1) == doctest::Approx(9.0));         // und die Vorschau mit ihm
        CHECK(engine->dekodiert.load() == dekodiertBeimErstenMal);

        // --- OK: EIN Schritt „Match loudness"
        const std::size_t vorOk = bus->undoCount();
        auto* ok = dialog->findChild<QPushButton*>(QStringLiteral("matchOk"));
        REQUIRE(ok != nullptr);
        ok->click();
        REQUIRE(warte([&] { return panel.abgleichDialog() == nullptr; }));
        CHECK(bus->undoCount() == vorOk + 1);
        CHECK(bus->undoDescription() == "Match loudness");
        CHECK(gainVon(1, 1) == doctest::Approx(9.0));
        CHECK(edit->isEnabled());
        CHECK(achse->bearbeiten());
        REQUIRE(bus->undo());
        CHECK(gainVon(1, 1) == 0.0);
        CHECK(gainVon(2, 0) == 0.0);
        REQUIRE(bus->redo());
        CHECK(gainVon(1, 1) == doctest::Approx(9.0));

        // --- Rechtsklick „Match loudness to this clip...": zwei ist das Ziel; die Songs kommen aus dem Speicher
        dialog = panel.oeffneAbgleich(1, 1, true);
        REQUIRE(dialog != nullptr);
        CHECK(dialog->abgleich().ziel() == 1);
        REQUIRE(warte([&] { return !dialog->misst(); }));
        CHECK(engine->dekodiert.load() == dekodiertBeimErstenMal);
        CHECK(dialog->abgleich().ziel() == 1);  // von Hand gewaehlt: bleibt, auch wenn ein anderer lauter ist
        CHECK(*dialog->abgleich().vorschlag(0) == doctest::Approx(-9.0));
        // Measure again: misst die Songs neu
        dialog->missNeu();
        REQUIRE(warte([&] { return !dialog->misst(); }));
        CHECK(engine->dekodiert.load() == dekodiertBeimErstenMal + 2);

        // --- Cancel: die Gains von vorher, kein Schritt
        const std::size_t vorCancel = bus->undoCount();
        dialog->setzeGain(0, -3.0);
        CHECK(gainVon(1, 0) == doctest::Approx(-3.0));
        auto* abbrechen = dialog->findChild<QPushButton*>(QStringLiteral("matchCancel"));
        REQUIRE(abbrechen != nullptr);
        abbrechen->click();
        REQUIRE(warte([&] { return panel.abgleichDialog() == nullptr; }));
        CHECK(gainVon(1, 0) == 0.0);
        CHECK(bus->undoCount() == vorCancel);
        CHECK(edit->isEnabled());

        // --- die Wahl Fixed level merkt sich der Dialog (L3 a)
        dialog = panel.oeffneAbgleich(1, 0, false);
        REQUIRE(dialog != nullptr);
        auto* zielListe = dialog->findChild<QComboBox*>(QStringLiteral("matchTarget"));
        REQUIRE(warte([&] { return !dialog->misst(); }));
        zielListe->setCurrentIndex(zielListe->count() - 1);
        Q_EMIT zielListe->activated(zielListe->count() - 1);
        CHECK(dialog->abgleich().ziel() == -1);
        dialog->reject();
        REQUIRE(warte([&] { return panel.abgleichDialog() == nullptr; }));
        CHECK(QSettings().value(QLatin1String(LautheitsDialog::kEinstellungFest)).toBool());
        dialog = panel.oeffneAbgleich(1, 0, false);
        REQUIRE(dialog != nullptr);
        CHECK(dialog->abgleich().ziel() == -1);
        REQUIRE(warte([&] { return !dialog->misst(); }));
        REQUIRE(dialog->abgleich().vorschlag(1).has_value());
        CHECK(*dialog->abgleich().vorschlag(1) == doctest::Approx(kAbgleichFestLufs + 23.0));
        dialog->reject();
        REQUIRE(warte([&] { return panel.abgleichDialog() == nullptr; }));

        // --- eine andere Komposition: der Dialog geht zu, ohne etwas zu schreiben
        dialog = panel.oeffneAbgleich(1, 0, false);
        REQUIRE(dialog != nullptr);
        sitzung->neu();
        REQUIRE(warte([&] { return panel.abgleichDialog() == nullptr; }));
        // eine Komposition ohne Ton-Clip: der Knopf ist da, aber grau
        CHECK(knopf->isVisible());
        CHECK_FALSE(knopf->isEnabled());
    }
}
