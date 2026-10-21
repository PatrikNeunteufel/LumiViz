/**
 ****************************************************************************************
 * @file   test_LoopTonauftrag.cpp
 * @brief  Loop: die Klaenge im Hintergrund laden (docs/audio/Spielweisen_Konzept.md
 *         §4.5, Stufe 29c; Entscheide H1 a, TV3 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Fenster. Der erste Fall laedt dieselben Namen einmal auf dem
 * rufenden Thread und einmal auf mehreren Threads — mit Instrumenten (eines mit
 * einer Sample-Schicht), doppelten Namen, einer Klangdatei, einem fehlenden
 * Instrument und einer fehlenden Datei, in einem Wegwerf-Ordner — und verlangt
 * dieselben Klaenge Bit fuer Bit und denselben Bericht. Der zweite haelt einen
 * Auftrag an, waehrend sein einziger Thread im Dekodierer wartet. Der dritte
 * dekodiert mit der BassEngine die mitgelieferten Samples auf mehreren Threads zugleich
 * und vergleicht mit dem Dekodieren nacheinander (TV3 a: die BASS-Hilfe sagt das
 * nicht ausdruecklich zu); ohne Ausgabegeraet meldet er das und prueft nichts.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "audio/BassEngine.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Sample.hpp"
#include "loop/Tonauftrag.hpp"
#include "loop/Tonspeicher.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <atomic>
#include <cmath>
#include <memory>
#include <numbers>
#include <thread>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct AuftragApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    std::unique_ptr<QCoreApplication> app;
    AuftragApp()
    {
        if (QCoreApplication::instance() == nullptr) app = std::make_unique<QCoreApplication>(argc, argv);
    }
};

/// Ein Sinus von 440 Hz, eine Sekunde, Stereo — was der Dekodierer aus Pappe aus jeder vorhandenen Datei macht.
std::vector<float> sinus(int rate)
{
    std::vector<float> werte;
    for (int i = 0; i < rate; ++i)
    {
        const auto x = static_cast<float>(0.5 * std::sin(2.0 * std::numbers::pi * 440.0 * i / rate));
        werte.push_back(x);
        werte.push_back(x);
    }
    return werte;
}

bool schreibe(const QString& pfad, const QByteArray& inhalt)
{
    QFile datei(pfad);
    return datei.open(QIODevice::WriteOnly) && datei.write(inhalt) == inhalt.size();
}

bool gleich(const Klaenge& a, const Klaenge& b)
{
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if ((a[i] == nullptr) != (b[i] == nullptr)) return false;
        if (a[i] != nullptr && a[i]->daten != b[i]->daten) return false;
    }
    return true;
}

}  // namespace

TEST_SUITE("Loop Tonauftrag")
{
    TEST_CASE("Loop Tonauftrag 29c: im Hintergrund wie auf einem Thread")
    {
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QDir d(ordner.path());
        const QString wav = d.filePath(QStringLiteral("klopf.wav"));
        REQUIRE(schreibe(wav, QByteArray(64, 'x')));

        // drei Instrumente: zwei gerechnet, eines aus der Aufnahme
        QStringList pfade;
        for (int i = 0; i < 2; ++i)
        {
            Instrument instrument;
            instrument.titel = QStringLiteral("Probe %1").arg(i);
            instrument.laut = 0.5 + 0.2 * i;
            pfade << d.filePath(QStringLiteral("p%1.lvinst").arg(i));
            REQUIRE(speichereInstrument(instrument, pfade.back()));
        }
        Instrument mitSample;
        mitSample.titel = QStringLiteral("Mit Sample");
        mitSample.schichten[0].schwingung = Schwingung::Sample;
        mitSample.schichten[0].sample = wav;
        mitSample.schichten[0].grundton = 69;
        pfade << d.filePath(QStringLiteral("s.lvinst"));
        REQUIRE(speichereInstrument(mitSample, pfade.back()));

        QStringList namen;
        for (const QString& pfad : pfade)
            for (const int hoehe : {48, 55, 60, 67, 72})
                namen << tonKlang(pfad, hoehe, 300);
        namen << tonKlang(pfade[0], 60, 300) << wav << tonKlang(pfade[1], 48, 300);  // der erste und der letzte doppelt
        namen << tonKlang(d.filePath(QStringLiteral("fehlt.lvinst")), 60, 300) << d.filePath(QStringLiteral("fehlt.wav"));

        std::atomic<int> dekodiert{0};
        const Dekodierer dekodierer = [&dekodiert](const QString& datei, int rate, double) {
            ++dekodiert;
            return QFileInfo::exists(datei) ? sinus(rate) : std::vector<float>{};
        };

        // --- auf dem rufenden Thread
        tonspeicher().leere();
        vergissSampleDaten();
        Tonauftrag hier(namen, kRate, dekodierer);
        CHECK(hier.gesamt() == namen.size() - 2);  // die zwei doppelten einmal
        hier.rechneHier();
        CHECK(hier.fertig());
        const Klaenge vorn = hier.klaenge();
        const QStringList berichtVorn = hier.bericht();
        CHECK(berichtVorn == QStringList{QStringLiteral("Instrument not found: ") + QDir::toNativeSeparators(d.filePath(QStringLiteral("fehlt.lvinst"))),
                                         QStringLiteral("Sound not loaded: ") + QDir::toNativeSeparators(d.filePath(QStringLiteral("fehlt.wav")))});
        CHECK(dekodiert == 3);  // das Sample einmal, die Klangdatei, die fehlende Datei
        REQUIRE(vorn.size() == static_cast<std::size_t>(namen.size()));
        CHECK(vorn[15].get() == vorn[2].get());  // ein doppelter Name: derselbe Klang

        // --- im Hintergrund, ohne Speicher: alles neu gerechnet, auf vier Threads
        tonspeicher().leere();
        vergissSampleDaten();
        dekodiert = 0;
        Tonauftrag hinten(namen, kRate, dekodierer);
        hinten.starte(4);
        hinten.warte();
        CHECK(hinten.fertig());
        CHECK_FALSE(hinten.abgebrochen());
        CHECK(hinten.erledigt() == hinten.gesamt());
        CHECK(gleich(hinten.klaenge(), vorn));
        CHECK(hinten.bericht() == berichtVorn);
        CHECK(dekodiert == 3);
        CHECK(tonspeicher().stand().gelegt == static_cast<std::uint64_t>(hinten.gesamt() - 2));  // ohne die zwei fehlenden
        tonspeicher().leere();
        vergissSampleDaten();
    }

    TEST_CASE("Loop Tonauftrag 29c: anhalten, waehrend der Thread dekodiert")
    {
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        QStringList namen;
        for (int i = 0; i < 5; ++i)
        {
            namen << QDir(ordner.path()).filePath(QStringLiteral("k%1.wav").arg(i));
            REQUIRE(schreibe(namen.back(), QByteArray(16 + i, 'x')));
        }
        std::atomic<bool> drin{false};
        std::atomic<bool> frei{false};
        const Dekodierer dekodierer = [&](const QString&, int rate, double) {
            drin = true;
            while (!frei) std::this_thread::yield();
            return sinus(rate);
        };
        tonspeicher().leere();
        Tonauftrag auftrag(namen, kRate, dekodierer);
        auftrag.starte(1);
        while (!drin) std::this_thread::yield();
        CHECK(auftrag.erledigt() == 0);
        auftrag.brichAb();
        frei = true;
        auftrag.warte();
        CHECK(auftrag.abgebrochen());
        CHECK(auftrag.erledigt() == 1);
        CHECK_FALSE(auftrag.fertig());
        const Klaenge klaenge = auftrag.klaenge();
        CHECK(klaenge[0] != nullptr);
        for (std::size_t i = 1; i < klaenge.size(); ++i) CHECK(klaenge[i] == nullptr);

        // der Destruktor haelt an und wartet — auch, wenn niemand `warte` ruft
        {
            frei = false;
            drin = false;
            tonspeicher().leere();
            Tonauftrag weg(namen, kRate, dekodierer);
            weg.starte(2);
            while (!drin) std::this_thread::yield();
            frei = true;
        }
        tonspeicher().leere();
    }

    TEST_CASE("Loop Tonauftrag 29c: BASS dekodiert auf mehreren Threads wie nacheinander")
    {
        AuftragApp app;
        BassEngine engine;
        if (!engine.initialize())
        {
            MESSAGE("kein Ausgabegeraet: nichts geprueft (" << engine.getLastError().toStdString() << ")");
            return;
        }
        // die mitgelieferten Samples: viele Dateien, viele Threads zugleich in BASS
        const QDir samples(looptest::repoWurzel().filePath(QStringLiteral("asset/loop/samples")));
        QStringList namen;
        for (const QString& datei : samples.entryList({QStringLiteral("*.wav")}, QDir::Files, QDir::Name))
            namen << samples.filePath(datei);
        REQUIRE(namen.size() >= 8);
        const Dekodierer dekodierer = [&engine](const QString& datei, int rate, double maxSekunden) {
            return engine.decodeStereo(datei, rate, maxSekunden);
        };
        Klaenge nacheinander;
        for (const QString& name : namen)
            nacheinander.push_back(std::make_shared<const Klang>(Klang{engine.decodeStereo(name, kRate, kKlangMaxSekunden)}));

        tonspeicher().leere();
        Tonauftrag auftrag(namen, kRate, dekodierer);
        auftrag.starte(8);
        auftrag.warte();
        CHECK(auftrag.fertig());
        CHECK(auftrag.bericht().isEmpty());
        CHECK(gleich(auftrag.klaenge(), nacheinander));
        MESSAGE(namen.size() << " Dateien auf 8 Threads dekodiert");
        tonspeicher().leere();
    }
}
