/**
 ****************************************************************************************
 * @file   test_LoopTonspeicher.cpp
 * @brief  Loop: der Speicher je Ton (docs/audio/Spielweisen_Konzept.md §4.5,
 *         Stufe 29b; Entscheide H1 a, H3 a, TV2 a, TV4 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Fenster; die ersten zwei ohne Qt-Anwendung. Der erste Fall prueft den Speicher
 * allein (Grenze, der am laengsten nicht gebrauchte Ton geht zuerst, Zaehler);
 * der zweite `ladeKlaenge` mit Instrumenten und einer Klangdatei in einem
 * Wegwerf-Ordner und einer Engine aus Pappe, die mitschreibt, was sie dekodiert:
 * ein zweites Laden rechnet nichts, ein geaendertes Instrument rechnet nur seine
 * Toene neu, eine geaenderte Datei wird neu dekodiert, und jeder Ton gleicht
 * Bit fuer Bit dem, den `rendereTon` direkt rechnet. Der dritte (Stufe 29f, TS1 a)
 * prueft die Zeile Loop Sound Memory im Settings-Panel, in eigenen Einstellungen
 * (`LumiVizTest` / `LoopTonspeicher`): Grenze, Stand, Clear.
 ****************************************************************************************
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonspeicher.hpp"
#include "services/ServiceContainer.hpp"
#include "UI/panels/SettingsPanel.hpp"

#include <QApplication>
#include <QDir>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>

#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Ein Klang aus `frames` gleichen Werten (Stereo): `frames * 8` Bytes.
std::shared_ptr<const Klang> klangAus(int frames, float wert)
{
    Klang k;
    k.daten.assign(static_cast<std::size_t>(frames) * 2, wert);
    return std::make_shared<const Klang>(std::move(k));
}

/// Die Engine aus Pappe dekodiert: je vorhandener Datei so viele Werte, wie die Datei Bytes hat, und sie merkt,
/// wonach gefragt wurde.
class DekodierEngine : public papp::PappMischEngine
{
public:
    QStringList gefragt;
    [[nodiscard]] std::vector<float> decodeStereo(const QString& filePath, int, double) override
    {
        gefragt << filePath;
        const QFileInfo info(filePath);
        return info.exists() ? std::vector<float>(static_cast<std::size_t>(2 * info.size()), 0.25f) : std::vector<float>{};
    }
};

/// Eigene Einstellungen (`LumiVizTest` / `LoopTonspeicher`), leer zu Beginn und am Ende.
struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopTonspeicher"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

bool schreibe(const QString& pfad, const QByteArray& inhalt)
{
    QFile datei(pfad);
    return datei.open(QIODevice::WriteOnly) && datei.write(inhalt) == inhalt.size();
}

Instrument einfaches(double laut)
{
    Instrument instrument;
    instrument.titel = QStringLiteral("Probe");
    instrument.laut = laut;
    return instrument;
}

}  // namespace

TEST_SUITE("Loop Tonspeicher")
{
    TEST_CASE("Loop Tonspeicher 29b: Grenze, der aelteste geht zuerst, Zaehler")
    {
        Tonspeicher speicher;
        CHECK(speicher.grenze() == kTonspeicherVorgabeBytes);
        speicher.setzeGrenze(3 * 1000 * 8);  // drei Klaenge zu 1000 Frames

        speicher.lege(QStringLiteral("a"), klangAus(1000, 0.1f), 1.5);
        speicher.lege(QStringLiteral("b"), klangAus(1000, 0.2f), 2.5);
        speicher.lege(QStringLiteral("c"), klangAus(1000, 0.3f), 3.5);
        CHECK(speicher.stand().toene == 3);
        CHECK(speicher.stand().bytes == 3 * 1000 * 8);
        CHECK(speicher.rechenzeitMs(QStringLiteral("b")) == doctest::Approx(2.5));
        CHECK_FALSE(speicher.rechenzeitMs(QStringLiteral("x")).has_value());

        // a wird gebraucht — damit ist b der am laengsten nicht gebrauchte
        REQUIRE(speicher.hole(QStringLiteral("a")) != nullptr);
        CHECK(speicher.hole(QStringLiteral("x")) == nullptr);
        const std::shared_ptr<const Klang> gehalten = speicher.hole(QStringLiteral("b"));
        REQUIRE(gehalten != nullptr);
        (void)speicher.hole(QStringLiteral("a"));
        (void)speicher.hole(QStringLiteral("c"));
        speicher.lege(QStringLiteral("d"), klangAus(1000, 0.4f), 0.0);
        CHECK(speicher.hole(QStringLiteral("b")) == nullptr);
        CHECK(speicher.hole(QStringLiteral("a")) != nullptr);
        CHECK(speicher.stand().verworfen == 1);
        // wer einen verworfenen Ton haelt, behaelt ihn
        CHECK(gehalten->daten.front() == 0.2f);

        // derselbe Schluessel ersetzt, zaehlt die Bytes nicht doppelt
        speicher.lege(QStringLiteral("d"), klangAus(500, 0.5f), 0.0);
        CHECK(speicher.stand().toene == 3);
        CHECK(speicher.stand().bytes == (2 * 1000 + 500) * 8);
        CHECK(speicher.hole(QStringLiteral("d"))->daten.front() == 0.5f);

        // ein Ton allein ueber der Grenze bleibt nicht — wer ihn gelegt hat, haelt ihn
        const std::shared_ptr<const Klang> gross = klangAus(4000, 0.6f);
        speicher.lege(QStringLiteral("gross"), gross, 0.0);
        CHECK(speicher.hole(QStringLiteral("gross")) == nullptr);
        CHECK(gross->frames() == 4000);

        // der grosse hat alle anderen mitgenommen, bevor er selbst ging
        const TonspeicherStand stand = speicher.stand();
        CHECK(stand.toene == 0);
        CHECK(stand.gelegt == 6);
        CHECK(stand.treffer == 6);
        CHECK(stand.verworfen == 5);

        // eine kleinere Grenze gilt sofort; leere vergisst alles
        speicher.setzeGrenze(0);
        CHECK(speicher.stand().toene == 0);
        CHECK(speicher.stand().bytes == 0);
        speicher.leere();
        CHECK(speicher.stand().gelegt == 0);
        CHECK(speicher.stand().verworfen == 0);
    }

    TEST_CASE("Loop Tonspeicher 29b: ladeKlaenge rechnet und dekodiert nur, was neu ist")
    {
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString pfadA = QDir(ordner.path()).filePath(QStringLiteral("a.lvinst"));
        const QString pfadB = QDir(ordner.path()).filePath(QStringLiteral("b.lvinst"));
        const QString wav = QDir(ordner.path()).filePath(QStringLiteral("klopf.wav"));
        REQUIRE(speichereInstrument(einfaches(0.8), pfadA));
        REQUIRE(speichereInstrument(einfaches(0.6), pfadB));
        REQUIRE(schreibe(wav, QByteArray(300, 'x')));

        tonspeicher().leere();
        DekodierEngine engine;
        const QStringList namen = {tonKlang(pfadA, 60, 250), tonKlang(pfadA, 62, 250), tonKlang(pfadB, 60, 250), wav};
        QStringList bericht;

        // --- zum ersten Mal: alles gerechnet und dekodiert, Bit fuer Bit wie direkt gerechnet
        const Klaenge erste = ladeKlaenge(namen, engine, kRate, &bericht);
        CHECK(bericht.isEmpty());
        REQUIRE(erste.size() == 4);
        for (const auto& klang : erste) REQUIRE(klang != nullptr);
        CHECK(tonspeicher().stand().gelegt == 4);
        CHECK(tonspeicher().stand().treffer == 0);
        CHECK(engine.gefragt.size() == 1);
        Instrument a;
        REQUIRE(holeInstrument(pfadA, a));
        CHECK(erste[1]->daten == rendereTon(a, 62, 250.0, kRate).daten);
        CHECK(erste[3]->frames() == 300);
        const QString schluessel = tonSchluessel(namen[0], kRate, instrumentAbdruck(a));
        CHECK(tonspeicher().rechenzeitMs(schluessel).has_value());

        // --- noch einmal: nichts gerechnet, nichts dekodiert — dieselben Toene, nicht Kopien
        const Klaenge zweite = ladeKlaenge(namen, engine, kRate, &bericht);
        CHECK(tonspeicher().stand().gelegt == 4);
        CHECK(tonspeicher().stand().treffer == 4);
        CHECK(engine.gefragt.size() == 1);
        for (std::size_t i = 0; i < namen.size(); ++i) CHECK(zweite[i].get() == erste[i].get());

        // --- ein Instrument geaendert: nur seine Toene neu
        REQUIRE(speichereInstrument(einfaches(0.4), pfadA));
        const Klaenge dritte = ladeKlaenge(namen, engine, kRate, &bericht);
        CHECK(tonspeicher().stand().gelegt == 6);
        CHECK(dritte[0].get() != erste[0].get());
        CHECK(dritte[0]->daten != erste[0]->daten);
        CHECK(dritte[2].get() == erste[2].get());
        CHECK(dritte[3].get() == erste[3].get());
        Instrument neuA;
        REQUIRE(holeInstrument(pfadA, neuA));
        CHECK(dritte[0]->daten == rendereTon(neuA, 60, 250.0, kRate).daten);

        // --- die Datei geaendert: neu dekodiert
        REQUIRE(schreibe(wav, QByteArray(500, 'y')));
        const Klaenge vierte = ladeKlaenge(namen, engine, kRate, &bericht);
        CHECK(engine.gefragt.size() == 2);
        CHECK(vierte[3]->frames() == 500);
        CHECK(vierte[2].get() == erste[2].get());

        // --- eine andere Rate ist ein anderer Ton
        (void)ladeKlaenge({namen[2]}, engine, 48000, &bericht);
        CHECK(tonspeicher().stand().gelegt == 8);
        tonspeicher().leere();
    }

    TEST_CASE("Loop Tonspeicher 29f: Loop Sound Memory in den Settings — Grenze, Stand, Clear")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        tonspeicher().leere();
        const std::size_t vorher = tonspeicher().grenze();

        // --- ohne Eintrag die Vorgabe; was nicht einstellbar ist, wird geklemmt
        CHECK(gemerkteTonspeicherGrenzeMb() == 1024);
        QSettings().setValue(QStringLiteral("loop/soundMemoryMB"), 10);
        CHECK(gemerkteTonspeicherGrenzeMb() == kTonspeicherMinMb);
        QSettings().setValue(QStringLiteral("loop/soundMemoryMB"), 1 << 20);
        CHECK(gemerkteTonspeicherGrenzeMb() == kTonspeicherMaxMb);
        merkeTonspeicherGrenzeMb(1024);  // die Vorgabe steht nicht in den Settings
        CHECK_FALSE(QSettings().contains(QStringLiteral("loop/soundMemoryMB")));

        ServiceContainer dienste;
        {
            SettingsPanel settings(dienste);
            auto* grenze = settings.findChild<QDoubleSpinBox*>(QStringLiteral("loopSoundMemoryLimit"));
            auto* belegt = settings.findChild<QLabel*>(QStringLiteral("loopSoundMemoryInUse"));
            auto* leeren = settings.findChild<QPushButton*>(QStringLiteral("loopSoundMemoryClear"));
            REQUIRE(grenze != nullptr);
            REQUIRE(belegt != nullptr);
            REQUIRE(leeren != nullptr);
            CHECK(grenze->value() == doctest::Approx(1.0));
            CHECK(grenze->minimum() == doctest::Approx(0.25));
            CHECK(grenze->maximum() == doctest::Approx(64.0));
            CHECK_FALSE(grenze->keyboardTracking());  // eine halb getippte Zahl verwirft nichts
            CHECK(belegt->text() == QStringLiteral("in use: 0 MB · 0 sounds"));

            // --- die Grenze gilt sofort und wird gemerkt
            grenze->setValue(2.5);
            CHECK(tonspeicher().grenze() == std::size_t{2560} << 20);
            CHECK(QSettings().value(QStringLiteral("loop/soundMemoryMB")).toInt() == 2560);

            // --- der Stand zeigt, was der Speicher haelt (die Uhr schreibt jede Sekunde nach)
            tonspeicher().lege(QStringLiteral("a"), klangAus(1 << 17, 0.1f), 0.0);  // 1 MB
            tonspeicher().lege(QStringLiteral("b"), klangAus(1 << 17, 0.2f), 0.0);
            QElapsedTimer seit;
            seit.start();
            while (belegt->text() != QStringLiteral("in use: 2 MB · 2 sounds") && seit.elapsed() < 3000)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
            CHECK(belegt->text() == QStringLiteral("in use: 2 MB · 2 sounds"));

            // --- Clear vergisst alles; wer einen Ton haelt, behaelt ihn
            const std::shared_ptr<const Klang> gehalten = tonspeicher().hole(QStringLiteral("a"));
            leeren->click();
            CHECK(tonspeicher().stand().toene == 0);
            CHECK(belegt->text() == QStringLiteral("in use: 0 MB · 0 sounds"));
            REQUIRE(gehalten != nullptr);
            CHECK(gehalten->daten.front() == 0.1f);

            // --- eine kleinere Grenze verwirft sofort die am laengsten nicht gebrauchten
            for (int i = 0; i < 3; ++i)
                tonspeicher().lege(QStringLiteral("t%1").arg(i), klangAus(100 << 17, 0.3f), 0.0);  // je 100 MB
            grenze->setValue(0.25);
            CHECK(tonspeicher().stand().toene == 2);
            CHECK(tonspeicher().hole(QStringLiteral("t0")) == nullptr);
        }
        {
            // ein neu geoeffnetes Settings-Panel zeigt die gemerkte Grenze
            SettingsPanel settings(dienste);
            auto* grenze = settings.findChild<QDoubleSpinBox*>(QStringLiteral("loopSoundMemoryLimit"));
            REQUIRE(grenze != nullptr);
            CHECK(grenze->value() == doctest::Approx(0.25));
        }
        tonspeicher().leere();
        tonspeicher().setzeGrenze(vorher);
    }
}
