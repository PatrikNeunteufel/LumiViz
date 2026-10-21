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
 * Ohne Ton, ohne Fenster, ohne Qt-Anwendung. Der erste Fall prueft den Speicher
 * allein (Grenze, der am laengsten nicht gebrauchte Ton geht zuerst, Zaehler);
 * der zweite `ladeKlaenge` mit Instrumenten und einer Klangdatei in einem
 * Wegwerf-Ordner und einer Engine aus Pappe, die mitschreibt, was sie dekodiert:
 * ein zweites Laden rechnet nichts, ein geaendertes Instrument rechnet nur seine
 * Toene neu, eine geaenderte Datei wird neu dekodiert, und jeder Ton gleicht
 * Bit fuer Bit dem, den `rendereTon` direkt rechnet.
 ****************************************************************************************
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonspeicher.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
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
}
