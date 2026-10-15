/**
 * @file test_PublishSammler.cpp
 * @brief Publish, Stufe P-A: was ein Ding nennt — der Sammler
 *        (docs/core-services/Publish_Konzept.md §2a, §3; Entscheide P1 a, P3 a)
 *
 * Ohne Ton, ohne Fenster. Die eigenen Instrumente, Samples, Kits und Formen
 * liegen in Wegwerf-Ordnern; das Mitgelieferte liest der Test vom Arbeitsordner
 * aus — die Test-Exe aus dem Projekt-Root starten.
 */

#include <doctest.h>

#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Sample.hpp"
#include "paket/Sammler.hpp"
#include "shape/Kontur.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

namespace
{

namespace lp = lumi::loop;
namespace pb = lumi::paket;

struct PublishApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

/// Ein Benutzerordner in einem Wegwerf-Ordner, dazu `frei/` und `loops/`.
struct Ablage
{
    QTemporaryDir tmp;
    QString instrumente, samples, kits, konturen, frei, loops;

    Ablage()
    {
        REQUIRE(tmp.isValid());
        const QDir wurzel(tmp.path());
        instrumente = wurzel.filePath(QStringLiteral("user/instruments"));
        samples = wurzel.filePath(QStringLiteral("user/samples"));
        kits = wurzel.filePath(QStringLiteral("user/kits"));
        konturen = wurzel.filePath(QStringLiteral("user/contours"));
        frei = wurzel.filePath(QStringLiteral("frei"));
        loops = wurzel.filePath(QStringLiteral("loops"));
        for (const QString& o : {instrumente, samples, kits, konturen, frei, loops}) REQUIRE(QDir().mkpath(o));
        lp::setzeEigenenInstrumentOrdnerFuerTests(instrumente);
        lp::setzeEigenenSampleOrdnerFuerTests(samples);
        lp::setzeEigenenKitOrdnerFuerTests(kits);
        lumi::shape::setzeEigenenKonturOrdnerFuerTests(konturen);
    }
    ~Ablage()
    {
        lp::setzeEigenenInstrumentOrdnerFuerTests(QString());
        lp::setzeEigenenSampleOrdnerFuerTests(QString());
        lp::setzeEigenenKitOrdnerFuerTests(QString());
        lumi::shape::setzeEigenenKonturOrdnerFuerTests(QString());
    }
};

void lege(const QString& pfad, const QByteArray& inhalt)
{
    QFile datei(pfad);
    REQUIRE(datei.open(QIODevice::WriteOnly));
    datei.write(inhalt);
}

lp::Schicht sampleSchicht(const QString& sample)
{
    lp::Schicht s;
    s.schwingung = lp::Schwingung::Sample;
    s.sample = sample;
    return s;
}

lp::Schicht formSchicht(const QString& form)
{
    lp::Schicht s;
    s.schwingung = lp::Schwingung::Form;
    s.form = form;
    return s;
}

lp::Spur spur(const QString& name, const QString& klang, const QString& instrument = {})
{
    lp::Spur s(name, klang);
    s.instrument = instrument;
    return s;
}

/// Der Titanium-Fall und mehr: ein Loop, der eigene Dinge, Dateien neben sich
/// und auf der Platte, Mitgeliefertes und etwas Fehlendes nennt.
struct Bestand : Ablage
{
    QString loopDatei, pumpDatei, snareDatei, freiKick;

    Bestand()
    {
        // eigene: das Sample kick, die Form herz, das Instrument bass (nennt beide), das Kit mein
        lege(QDir(samples).filePath(QStringLiteral("kick.wav")), "eigener kick");
        lumi::shape::Zeichnung herz;
        herz.titel = QStringLiteral("Herz");
        REQUIRE(lumi::shape::speichereZeichnung(herz, QDir(konturen).filePath(QStringLiteral("herz.lvcontour"))));
        lp::Instrument bass;
        bass.titel = QStringLiteral("Mein Bass");
        bass.schichten = {sampleSchicht(QStringLiteral("user:kick")), formSchicht(QStringLiteral("user:herz"))};
        REQUIRE(lp::speichereInstrument(bass, QDir(instrumente).filePath(QStringLiteral("bass.lvinst"))));

        // an freiem Ort: zwei Samples auf der Platte, ein Instrument neben dem Loop
        snareDatei = QDir(frei).filePath(QStringLiteral("Snare Hit.wav"));
        lege(snareDatei, "snare");
        freiKick = QDir(frei).filePath(QStringLiteral("kick.wav"));
        lege(freiKick, "ein anderer kick");
        lp::Instrument pump;
        pump.titel = QStringLiteral("Pump Bass");
        pump.schichten = {sampleSchicht(snareDatei), formSchicht(QStringLiteral("heart"))};
        pumpDatei = QDir(loops).filePath(QStringLiteral("pump_bass.lvinst"));
        REQUIRE(lp::speichereInstrument(pump, pumpDatei));

        lp::DrumKit kit;
        kit.titel = QStringLiteral("Mein Kit");
        lp::KitKlang k1;
        k1.rolle = QStringLiteral("kick");
        k1.klang = QStringLiteral("user:kick");
        lp::KitKlang k2;
        k2.rolle = QStringLiteral("snare");
        k2.klang = QStringLiteral("kit:snare");  // steht neben dem Instrument, spielt nicht
        k2.instrument = QStringLiteral("user:bass");
        kit.klaenge = {k1, k2};
        REQUIRE(lp::speichereKit(QDir(kits).filePath(QStringLiteral("mein.lvkit")), kit));

        lp::Loop loop;
        lp::Muster drums;
        drums.name = QStringLiteral("D");
        drums.kit = QStringLiteral("user:mein");
        drums.spuren = {spur(QStringLiteral("Kick"), QStringLiteral("user:kick")),
                        spur(QStringLiteral("Snare"), QString(), pumpDatei),
                        spur(QStringLiteral("Clap"), QStringLiteral("lib:clap_1")),
                        spur(QStringLiteral("Hat"), QStringLiteral("kit:hat")),
                        spur(QStringLiteral("Tief"), freiKick),
                        spur(QStringLiteral("Weg"), QStringLiteral("user:gibt_es_nicht"))};
        lp::Muster klavier;
        klavier.name = QStringLiteral("P");
        klavier.art = lp::MusterArt::Instrument;
        klavier.instrument = QStringLiteral("piano");
        lp::Muster eigenes;
        eigenes.name = QStringLiteral("B");
        eigenes.art = lp::MusterArt::Instrument;
        eigenes.instrument = QStringLiteral("user:bass");
        loop.muster = {drums, klavier, eigenes};
        loop.richte();
        loopDatei = QDir(loops).filePath(QStringLiteral("titan.lvloop"));
        REQUIRE(lp::speichereLoop(loop, loopDatei));
    }
};

}  // namespace

TEST_SUITE("Publish")
{
    TEST_CASE("Publish Sammler: die Art einer Datei")
    {
        CHECK(pb::artVonDatei(QStringLiteral("a/b.lvinst")) == pb::Art::Instrument);
        CHECK(pb::artVonDatei(QStringLiteral("a/b.LVKIT")) == pb::Art::Kit);
        CHECK(pb::artVonDatei(QStringLiteral("b.lvloop")) == pb::Art::Loop);
        CHECK(pb::artVonDatei(QStringLiteral("b.lvcontour")) == pb::Art::Kontur);
        CHECK(pb::artVonDatei(QStringLiteral("b.flac")) == pb::Art::Sample);
        CHECK(pb::artVonDatei(QStringLiteral("b.wav")) == pb::Art::Sample);
        CHECK_FALSE(pb::artVonDatei(QStringLiteral("b.lvfx")).has_value());  // Stufe P-B
        CHECK_FALSE(pb::artVonDatei(QStringLiteral("b.txt")).has_value());
        CHECK(pb::artText(pb::Art::Instrument) == QStringLiteral("lvinst"));
        CHECK(pb::artText(pb::Art::Sample) == QStringLiteral("sample"));
    }

    TEST_CASE("Publish Sammler: ein Loop — Eigenes, freie Orte, Mitgeliefertes, Fehlendes, jede Datei einmal")
    {
        const PublishApp app;
        const Bestand b;
        const pb::Fund fund = pb::sammle(b.loopDatei);
        REQUIRE_MESSAGE(fund.ok, fund.fehler.toStdString());
        CHECK(fund.art == pb::Art::Loop);
        CHECK(fund.titel == QStringLiteral("titan"));
        CHECK(fund.hauptName.isEmpty());  // ein Loop kommt dorthin, wo der Empfaenger ihn ablegt

        QStringList gefunden;
        for (const pb::Zugehoerige& z : fund.zugehoerige) gefunden << pb::artText(z.art) + QLatin1Char(':') + z.name;
        // nach Art, darin nach Name; bass dreimal genannt, kick viermal — je einmal da
        CHECK(gefunden == QStringList{QStringLiteral("lvinst:bass"), QStringLiteral("lvinst:pump_bass"),
                                      QStringLiteral("lvkit:mein"), QStringLiteral("lvcontour:herz"),
                                      QStringLiteral("sample:kick"), QStringLiteral("sample:kick_2"),
                                      QStringLiteral("sample:snare_hit")});

        const pb::Zugehoerige* kick = fund.finde(pb::Art::Sample, QStringLiteral("kick"));
        REQUIRE(kick != nullptr);
        CHECK(kick->herkunft == pb::Herkunft::Eigen);  // die eigene behaelt ihren Namen
        CHECK(kick->nennungen == QStringList{QStringLiteral("user:kick")});
        CHECK(kick->genanntVon.contains(QStringLiteral("titan")));
        CHECK(kick->genanntVon.contains(QStringLiteral("Mein Bass")));
        CHECK(kick->genanntVon.contains(QStringLiteral("Mein Kit")));

        const pb::Zugehoerige* kick2 = fund.finde(pb::Art::Sample, QStringLiteral("kick_2"));
        REQUIRE(kick2 != nullptr);
        CHECK(kick2->herkunft == pb::Herkunft::FreierOrt);  // auf der Platte, gleicher Dateiname
        CHECK(QFileInfo(kick2->datei) == QFileInfo(b.freiKick));

        const pb::Zugehoerige* pump = fund.finde(pb::Art::Instrument, QStringLiteral("pump_bass"));
        REQUIRE(pump != nullptr);
        CHECK(pump->herkunft == pb::Herkunft::FreierOrt);
        CHECK(pump->genanntVon == QStringList{QStringLiteral("titan")});

        // was das Instrument neben dem Loop nennt, kommt mit: das Sample von der Platte
        const pb::Zugehoerige* snare = fund.finde(pb::Art::Sample, QStringLiteral("snare_hit"));
        REQUIRE(snare != nullptr);
        CHECK(snare->genanntVon == QStringList{QStringLiteral("Pump Bass")});
        CHECK(QFileInfo(snare->nennungen.value(0)) == QFileInfo(b.snareDatei));

        const pb::Zugehoerige* herz = fund.finde(pb::Art::Kontur, QStringLiteral("herz"));
        REQUIRE(herz != nullptr);
        CHECK(herz->genanntVon == QStringList{QStringLiteral("Mein Bass")});

        // Mitgeliefertes nur beim Namen; kit:snare spielt nicht (das Instrument spielt) und fehlt hier
        CHECK(fund.mitgeliefert == QStringList{QStringLiteral("heart"), QStringLiteral("kit:hat"),
                                               QStringLiteral("lib:clap_1"), QStringLiteral("piano")});

        REQUIRE(fund.fehlt.size() == 1);
        CHECK(fund.fehlt.front() == pb::Fehlend{pb::Art::Sample, QStringLiteral("user:gibt_es_nicht"),
                                                QStringLiteral("titan")});
    }

    TEST_CASE("Publish Sammler: ein Instrument — sein Name, was es nennt; als Pfad im eigenen Ordner zaehlt eigen")
    {
        const PublishApp app;
        const Bestand b;

        const pb::Fund pump = pb::sammle(b.pumpDatei);
        REQUIRE(pump.ok);
        CHECK(pump.art == pb::Art::Instrument);
        CHECK(pump.titel == QStringLiteral("Pump Bass"));
        CHECK(pump.hauptName == QStringLiteral("pump_bass"));
        REQUIRE(pump.zugehoerige.size() == 1);
        CHECK(pump.zugehoerige.front().name == QStringLiteral("snare_hit"));
        CHECK(pump.mitgeliefert == QStringList{QStringLiteral("heart")});

        // das eigene Instrument, als Datei gewaehlt: es heisst weiter bass
        const pb::Fund bass = pb::sammle(QDir(b.instrumente).filePath(QStringLiteral("bass.lvinst")));
        REQUIRE(bass.ok);
        CHECK(bass.hauptName == QStringLiteral("bass"));
        REQUIRE(bass.zugehoerige.size() == 2);
        CHECK(bass.zugehoerige[0].art == pb::Art::Kontur);
        CHECK(bass.zugehoerige[1].art == pb::Art::Sample);

        // ein eigenes Sample, als Pfad genannt, zaehlt als eigen und behaelt seinen Namen
        lp::Instrument perPfad;
        perPfad.titel = QStringLiteral("Per Pfad");
        perPfad.schichten = {sampleSchicht(QDir(b.samples).filePath(QStringLiteral("kick.wav")))};
        const QString datei = QDir(b.frei).filePath(QStringLiteral("per_pfad.lvinst"));
        REQUIRE(lp::speichereInstrument(perPfad, datei));
        const pb::Fund fund = pb::sammle(datei);
        REQUIRE(fund.zugehoerige.size() == 1);
        CHECK(fund.zugehoerige.front().herkunft == pb::Herkunft::Eigen);
        CHECK(fund.zugehoerige.front().name == QStringLiteral("kick"));
    }

    TEST_CASE("Publish Sammler: ein Kit, eine Form, ein Sample; was sich nicht veroeffentlichen laesst")
    {
        const PublishApp app;
        const Bestand b;

        const pb::Fund kit = pb::sammle(QDir(b.kits).filePath(QStringLiteral("mein.lvkit")));
        REQUIRE(kit.ok);
        CHECK(kit.titel == QStringLiteral("Mein Kit"));
        CHECK(kit.hauptName == QStringLiteral("mein"));
        QStringList namen;
        for (const pb::Zugehoerige& z : kit.zugehoerige) namen << z.name;
        CHECK(namen == QStringList{QStringLiteral("bass"), QStringLiteral("herz"), QStringLiteral("kick")});

        const pb::Fund herz = pb::sammle(QDir(b.konturen).filePath(QStringLiteral("herz.lvcontour")));
        REQUIRE(herz.ok);
        CHECK(herz.titel == QStringLiteral("Herz"));
        CHECK(herz.zugehoerige.empty());

        const pb::Fund snare = pb::sammle(b.snareDatei);
        REQUIRE(snare.ok);
        CHECK(snare.art == pb::Art::Sample);
        CHECK(snare.hauptName == QStringLiteral("snare_hit"));

        const pb::Fund fehlt = pb::sammle(QDir(b.frei).filePath(QStringLiteral("gibt_es_nicht.lvloop")));
        CHECK_FALSE(fehlt.ok);
        CHECK(fehlt.fehler.contains(QStringLiteral("not found")));

        const pb::Fund fremd = pb::sammle(QDir(b.frei).filePath(QStringLiteral("notiz.txt")));
        CHECK_FALSE(fremd.ok);
        CHECK_FALSE(fremd.fehler.isEmpty());
    }
}
