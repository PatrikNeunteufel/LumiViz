/**
 * @file test_LoopAufnahmeUhr.cpp
 * @brief Loop, Stufe 15k-1: Aufnahme an der Wiedergabe — Uhr, Latenz, Take, Fenster
 *        (docs/audio/Loop_Konzept.md §4p; Entscheide Patrik WV10-1 a, WR1 a, Skizze SK15k-1 a)
 *
 * - `uhrVersatz`: der Median ueber die Bloecke, an denen die Uhr lief.
 * - `messeLatenz` an einer gerechneten Aufnahme mit bekannter Latenz; zu wenige Klicks: keine.
 * - `schneideTake`: der Take sitzt an seiner Stelle, was vor der Aufnahme laege, ist still.
 * - `aufnahmeFassung`: Einzaehlen vor Takt 0 rueckt die Clips, der Klick als eigener Kanal.
 * - Die Latenz je Paar aus Eingang und Ausgang wird gemerkt.
 * - Das Fenster *Measure latency* und der Modus fuer Wave-Clips im Fenster *Record sample*,
 *   gegen eine Engine aus Pappe, die ein ideales Geraet spielt: was gespielt wird, kommt nach
 *   genau L Abtastwerten zurueck — gemessen wird L, der Take sitzt genau im Takt.
 * - Das Loop-Panel: *Record wave clip...* und *Record...* oeffnen das Fenster, *Keep take*
 *   legt den Clip an oder ersetzt die Datei des Musters.
 *
 * Ohne Ton und ohne gezeigtes Fenster.
 */

#include <doctest.h>

#include "PappAufnahme.hpp"
#include "loop/AufnahmeFenster.hpp"
#include "loop/AufnahmeUhr.hpp"
#include "loop/LatenzFenster.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Sample.hpp"
#include "loop/Taktachse.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QTemporaryDir>

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

using namespace lumi::loop;
using Marke = IAudioEngine::RecordClockMark;

namespace
{

struct Anwendung
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QApplication app{argc, argv};
};

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopAufnahmeUhr"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

constexpr int kRate = 48000;
constexpr int kBlock = 480;

/// 120 BPM, 4/4, vier Schritte je Schlag: ein Takt = 2 s = 96 000 Abtastwerte bei 48 kHz.
Loop vierTakte()
{
    Loop loop;
    Muster d;
    d.name = QStringLiteral("D");
    d.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    loop.muster = {d};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("D"), 0, 1}}}};
    loop.richte();
    return loop;
}

}  // namespace

TEST_SUITE("LoopAufnahmeUhr")
{
    TEST_CASE("Loop Aufnahme 15k-1: Uhr, Latenz, Take, Fassung")
    {
        SUBCASE("uhrVersatz: der Median, nur waehrend die Uhr lief")
        {
            const std::vector<Marke> marken{{480, 0},     {960, 0},      // vor der Wiedergabe: steht still
                                            {1440, 1940}, {1920, 2420}, {2400, 2905}, {2880, 3380}, {3360, 3860}};
            CHECK(uhrVersatz(marken) == 500);
            CHECK_FALSE(uhrVersatz({}).has_value());
            CHECK_FALSE(uhrVersatz({{480, 7}, {960, 7}}).has_value());
        }
        SUBCASE("messeLatenz: die Klicks mit 37,5 ms Latenz wiedergefunden")
        {
            const std::int64_t versatz = 1000;
            const std::int64_t latenz = 1800;  // 37,5 ms
            const std::vector<std::int64_t> klicks = klickStellen(kRate);
            const auto n = static_cast<std::size_t>(messungFrames(kRate));
            std::vector<float> aufnahme(2 * n, 0.0f);
            for (std::size_t k = 0; k < n; ++k)
            {
                // ein wenig Rauschen und jeder Klick, wo er ankommt: k = Klick − versatz + latenz
                float wert = (k % 7 == 0) ? 0.002f : 0.0f;
                for (const std::int64_t c : klicks) wert += klickWert(static_cast<std::int64_t>(k) - (c - versatz + latenz), kRate);
                aufnahme[2 * k] = wert;
                aufnahme[2 * k + 1] = wert;
            }
            const Latenzmessung m = messeLatenz(aufnahme, versatz, klicks, kRate);
            REQUIRE(m.ms.has_value());
            CHECK(*m.ms == doctest::Approx(37.5).epsilon(0.005));
            CHECK(m.gefunden == kLatenzKlicks);
            CHECK(m.erwartet == kLatenzKlicks);
            CHECK(m.streuungMs < 0.1);
            // nur zwei Klicks kommen an (ein leises Mikrofon): keine Messung
            std::vector<float> leise = aufnahme;
            for (std::size_t k = static_cast<std::size_t>(klicks[2] - versatz); k < n; ++k)
                leise[2 * k] = leise[2 * k + 1] = 0.0f;
            const Latenzmessung zuWenig = messeLatenz(leise, versatz, klicks, kRate);
            CHECK_FALSE(zuWenig.ms.has_value());
            CHECK(zuWenig.gefunden == 2);
        }
        SUBCASE("schneideTake: an seiner Stelle, vorn still, was fehlt")
        {
            // Abtastwert k traegt den Wert k; die Aufnahme liegt mit versatz 100 an der Uhr, Latenz 30
            std::vector<float> stereo;
            for (int k = 0; k < 1000; ++k) stereo.insert(stereo.end(), {static_cast<float>(k), static_cast<float>(-k)});
            const std::vector<float> take = schneideTake(stereo, 100, 30, 200, 260);
            REQUIRE(take.size() == 2 * 60);
            CHECK(take[0] == 130.0f);  // k = 200 − 100 + 30
            CHECK(take[1] == -130.0f);
            CHECK(take[2 * 59] == 189.0f);
            // die Aufnahme begann zu spaet: vorn still
            const std::vector<float> spaet = schneideTake(stereo, 100, 30, 50, 80);
            CHECK(spaet[0] == 0.0f);
            CHECK(spaet[2 * 20] == 0.0f);   // k = −20 + 20 = 0 → Wert 0
            CHECK(spaet[2 * 21] == 1.0f);
            CHECK(schneideTake(stereo, 0, 0, 10, 10).empty());
        }
        SUBCASE("aufnahmeFassung: Einzaehlen, Klick, verschobene Clips")
        {
            const Loop loop = vierTakte();
            // ab Takt 0 mit einem Takt Einzaehlen: alles rueckt um einen Takt
            const Aufnahmefassung f = aufnahmeFassung(loop, {0, 1, 2, true});
            CHECK(f.abspielTakt == 0);
            CHECK(f.aufnahmeTakt == 1);
            CHECK(f.loop.kanaele[0].clips[0].startTakt == 1);
            REQUIRE(f.loop.kanaele.size() == 2);
            const LoopClip& klick = f.loop.kanaele[1].clips[0];
            CHECK(klick.startTakt == 0);
            CHECK(klick.wiederholungen == 3);  // Einzaehlen und zwei Takte Aufnahme
            const Muster* klickMuster = f.loop.findeMuster(klick.muster);
            REQUIRE(klickMuster != nullptr);
            CHECK(klickMuster->spuren[0].klang == QStringLiteral("kit:hat_closed"));
            CHECK(klickMuster->zeilen[0][0] == 1.0);  // die Eins lauter
            CHECK(klickMuster->zeilen[0][4] == 0.6);
            // ab Takt 3 mit zwei Takten Einzaehlen, ohne Klick waehrend der Aufnahme: nichts rueckt
            const Aufnahmefassung g = aufnahmeFassung(loop, {3, 2, 0, false});
            CHECK(g.abspielTakt == 1);
            CHECK(g.aufnahmeTakt == 3);
            CHECK(g.loop.kanaele[0].clips[0].startTakt == 0);
            CHECK(g.loop.kanaele[1].clips[0].wiederholungen == 2);  // nur das Einzaehlen
            // ohne Einzaehlen und ohne Klick: kein Kanal dazu
            CHECK(aufnahmeFassung(loop, {2, 0, 0, false}).loop.kanaele.size() == 1);
            CHECK(taktFrames(loop, kRate) == doctest::Approx(96000.0));
        }
        SUBCASE("die Latenz je Paar aus Eingang und Ausgang")
        {
            Anwendung anwendung;
            EigeneEinstellungen einstellungen;
            CHECK_FALSE(gemerkteLatenzMs(QStringLiteral("Mic"), QStringLiteral("Speakers")).has_value());
            merkeLatenzMs(QStringLiteral("Mic"), QStringLiteral("Speakers"), 38.0);
            merkeLatenzMs(QStringLiteral("USB/In"), QStringLiteral("Speakers"), 12.0);
            CHECK(gemerkteLatenzMs(QStringLiteral("Mic"), QStringLiteral("Speakers")) == 38.0);
            CHECK(gemerkteLatenzMs(QStringLiteral("USB/In"), QStringLiteral("Speakers")) == 12.0);
            CHECK_FALSE(gemerkteLatenzMs(QStringLiteral("Mic"), QStringLiteral("Headphones")).has_value());
        }
    }

    TEST_CASE("Loop Aufnahme 15k-1: Measure latency gegen ein ideales Geraet")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        papp::AufnahmeEngine engine;
        engine.eingaenge = {papp::eingang(3, QStringLiteral("Mic"), true)};
        LatenzFenster fenster;
        fenster.setze(&engine, 3, QStringLiteral("Mic"), QStringLiteral("Speakers"));
        REQUIRE(fenster.starteMessung());
        REQUIRE(fenster.misst());
        REQUIRE(engine.generator == fenster.geber());
        // das ideale Geraet: was der Klickgeber spielt, kommt nach 1800 Abtastwerten (37,5 ms) an
        const std::int64_t latenz = 1800;
        std::vector<float> gespielt;
        std::vector<float> block(2 * kBlock);
        // bis das Fenster selbst beendet: danach gibt es keinen Klickgeber mehr
        while (fenster.misst())
        {
            engine.generator->render(block.data(), kBlock);
            gespielt.insert(gespielt.end(), block.begin(), block.end());
            std::vector<float> an(2 * kBlock, 0.0f);
            const auto ab = static_cast<std::int64_t>(engine.aufnahme.size() / 2);
            for (int i = 0; i < kBlock; ++i)
            {
                const std::int64_t quelle = ab + i - latenz;
                if (quelle >= 0) an[2 * i] = an[2 * i + 1] = gespielt[static_cast<std::size_t>(2 * quelle)];
            }
            engine.block(an);
            fenster.takt();
        }
        CHECK_FALSE(fenster.misst());
        REQUIRE(fenster.ergebnis().ms.has_value());
        CHECK(*fenster.ergebnis().ms == doctest::Approx(37.5).epsilon(0.005));
        CHECK(fenster.ergebnisText().startsWith(QStringLiteral("Result: 38 ms (5 of 5 clicks found")));
        double gemeldet = -1.0;
        QObject::connect(&fenster, &LatenzFenster::latenzGewaehlt, [&gemeldet](double ms) { gemeldet = ms; });
        REQUIRE(fenster.nimm());
        CHECK(gemeldet == doctest::Approx(37.5).epsilon(0.005));
        CHECK(gemerkteLatenzMs(QStringLiteral("Mic"), QStringLiteral("Speakers")).value_or(0.0) == doctest::Approx(37.5).epsilon(0.005));
    }

    TEST_CASE("Loop Aufnahme 15k-1: Record sample fuer Wave-Clips — der Take sitzt im Takt")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        QTemporaryDir samples;
        REQUIRE(samples.isValid());
        setzeEigenenSampleOrdnerFuerTests(samples.path());
        papp::AufnahmeEngine engine;
        engine.eingaenge = {papp::eingang(3, QStringLiteral("Mic"), true)};
        merkeLatenzMs(QStringLiteral("Mic"), QStringLiteral("Speakers"), 25.0);  // 1200 Abtastwerte

        // die Wiedergabe aus Pappe: die Uhr laeuft, wenn der Test sie stellt
        std::int64_t uhr = 0;
        bool bereit = false;
        Loop gespielt;
        int abspielTakt = -1;
        int gehalten = 0;
        AufnahmeFenster fenster;
        fenster.setEngine(&engine);
        AufnahmeFenster::WaveAufnahme auftrag;
        auftrag.loop = vierTakte();
        auftrag.plan = {2, 1, 0, true};  // ab Takt 3, ein Takt Einzaehlen, bis Stop
        auftrag.ausgang = QStringLiteral("Speakers");
        auftrag.rate = kRate;
        auftrag.spiele = [&](const Loop& fassung, int takt) {
            gespielt = fassung;
            abspielTakt = takt;
            return true;
        };
        auftrag.bereit = [&] { return bereit; };
        auftrag.halte = [&] { ++gehalten; };
        auftrag.uhr = [&] { return uhr; };
        fenster.beginneWaveClip(auftrag);
        CHECK(fenster.waveModus());
        CHECK(fenster.windowTitle() == QStringLiteral("Record wave clip"));
        CHECK(fenster.latenzMs() == 25.0);  // gemerkt fuer Mic und Speakers
        CHECK(fenster.plan() == WaveAufnahmePlan{2, 1, 0, true});
        CHECK(fenster.streifenText() == QStringLiteral("bar 2 · count-in | from bar 3 · recording until Stop, to the end of that bar"));

        REQUIRE(fenster.starteAufnahme());
        CHECK(abspielTakt == 1);  // ab dem Einzaehltakt
        fenster.takt();           // der Loop laedt noch: keine Aufnahme
        CHECK(engine.rufe.isEmpty());
        uhr = 96000;  // dort beginnt die Wiedergabe (Takt 2)
        bereit = true;
        fenster.takt();
        REQUIRE(engine.rufe == QStringList{QStringLiteral("start 3 48000 600")});
        // das ideale Geraet: der Abtastwert, der zur gespielten Stelle s gehoert, kommt mit der
        // Latenz an — sein Wert ist s / 1 000 000, so ist zu sehen, ob der Take im Takt sitzt
        const std::int64_t latenz = 1200;
        const auto spiele = [&](int bloecke) {
            for (int b = 0; b < bloecke; ++b)
            {
                std::vector<float> an(2 * kBlock);
                for (int i = 0; i < kBlock; ++i)
                {
                    const auto wert = static_cast<float>(static_cast<double>(uhr + i - latenz) / 1.0e6);
                    an[2 * i] = an[2 * i + 1] = wert;
                }
                uhr += kBlock;
                engine.block(an);
                fenster.takt();
            }
        };
        spiele(250);  // 2,5 s: Takt 2 und die Haelfte von Takt 3 gehoert
        CHECK(fenster.streifenText().contains(QStringLiteral("recording bar 3")));
        REQUIRE(fenster.stoppeAufnahme());  // Stop in Takt 3: der Take endet am Ende von Takt 3
        CHECK(fenster.streifenText().contains(QStringLiteral("ends at the end of the bar")));
        spiele(300);
        REQUIRE(engine.rufe.contains(QStringLiteral("stop")));
        CHECK(gehalten == 1);
        CHECK(fenster.takeTakte() == 1);
        REQUIRE(fenster.frames() == 96000);
        // der Take beginnt genau an Takt 3 (Uhr 192 000) und endet an Takt 4
        const std::vector<float> take = fenster.ergebnis();
        CHECK(take[0] == doctest::Approx(0.192));
        CHECK(take[2 * 95999] == doctest::Approx(0.287999));
        // die Fassung: Einzaehlen ab Takt 2, der Klick dazu
        CHECK(gespielt.kanaele.size() == 2);

        QString nennung;
        int start = -1;
        int takte = -1;
        QObject::connect(&fenster, &AufnahmeFenster::waveTakeBehalten, [&](const QString& n, int s, int t) {
            nennung = n;
            start = s;
            takte = t;
        });
        const QString behalten = fenster.behalteTake();
        setzeEigenenSampleOrdnerFuerTests(QString());
        CHECK(behalten == QStringLiteral("user:take_1"));
        CHECK(nennung == behalten);
        CHECK(start == 2);
        CHECK(takte == 1);
    }

    TEST_CASE("Loop Aufnahme 15k-1: das Panel — Record wave clip... und Keep take")
    {
        Anwendung anwendung;
        EigeneEinstellungen einstellungen;
        ServiceContainer dienste;
        dienste.registerSingleton<IAudioEngine>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new papp::AufnahmeEngine); });
        LoopPanel panel(dienste);  // nicht gezeigt
        panel.neu();
        REQUIRE(panel.nimmWaveClipAuf(0, 2, {}, false));
        AufnahmeFenster* fenster = panel.waveAufnahme();
        REQUIRE(fenster != nullptr);
        CHECK(fenster->waveModus());
        CHECK(fenster->plan().startTakt == 2);
        CHECK(fenster->plan().takte == 0);
        // Keep take meldet: ein Wave-Clip ab Takt 3, zwei Takte lang
        Q_EMIT fenster->waveTakeBehalten(QStringLiteral("user:take_1"), 2, 2);
        const Muster* take = panel.loop().findeMuster(QStringLiteral("take_1"));
        REQUIRE(take != nullptr);
        CHECK(take->art == MusterArt::Wave);
        CHECK(take->takte == 2);
        CHECK(take->wave.datei == QStringLiteral("user:take_1"));
        CHECK(panel.loop().kanaele[0].clips.back().startTakt == 2);

        SUBCASE("Record... im Wave editor: der Take ersetzt die Datei des Musters")
        {
            REQUIRE(panel.nimmWaveClipAuf(-1, 0, QStringLiteral("take_1"), false));
            CHECK(fenster->plan().startTakt == 2);  // der erste Clip des Musters
            CHECK(fenster->plan().takte == 2);      // so lang wie das Muster
            Q_EMIT fenster->waveTakeBehalten(QStringLiteral("user:take_2"), 2, 2);
            const Muster* neu = panel.loop().findeMuster(QStringLiteral("take_1"));
            REQUIRE(neu != nullptr);
            CHECK(neu->wave.datei == QStringLiteral("user:take_2"));
            CHECK(neu->wave.start == 0.0);
            CHECK(neu->wave.versatz == 0.0);
        }
    }
}
