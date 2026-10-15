/**
 * @file PaketBestand.hpp
 * @brief Publish: der Bestand der Tests zu Sammler, Packen und Import — eigene
 *        Instrumente, Samples, Kits und Formen in Wegwerf-Ordnern, dazu ein
 *        Loop wie Titanium (Instrument neben sich, Samples auf der Platte,
 *        Mitgeliefertes, eine fehlende Datei)
 */
#pragma once

#include <doctest.h>

#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Sample.hpp"
#include "paket/Packen.hpp"
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
                        spur(QStringLiteral("Clap"), QStringLiteral("lib:clap_707")),
                        spur(QStringLiteral("Hat"), QStringLiteral("kit:hat_closed")),
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

/// Die eigenen Ordner des Empfaengers; solange er lebt, gelten sie.
struct Empfaenger
{
    QString instrumente, samples, kits, konturen, loops;

    explicit Empfaenger(const QString& wurzel)
    {
        const QDir w(wurzel);
        instrumente = w.filePath(QStringLiteral("empfaenger/instruments"));
        samples = w.filePath(QStringLiteral("empfaenger/samples"));
        kits = w.filePath(QStringLiteral("empfaenger/kits"));
        konturen = w.filePath(QStringLiteral("empfaenger/contours"));
        loops = w.filePath(QStringLiteral("empfaenger/loops"));
        for (const QString& o : {instrumente, samples, kits, konturen, loops}) REQUIRE(QDir().mkpath(o));
        lp::setzeEigenenInstrumentOrdnerFuerTests(instrumente);
        lp::setzeEigenenSampleOrdnerFuerTests(samples);
        lp::setzeEigenenKitOrdnerFuerTests(kits);
        lumi::shape::setzeEigenenKonturOrdnerFuerTests(konturen);
    }
    ~Empfaenger()
    {
        lp::setzeEigenenInstrumentOrdnerFuerTests(QString());
        lp::setzeEigenenSampleOrdnerFuerTests(QString());
        lp::setzeEigenenKitOrdnerFuerTests(QString());
        lumi::shape::setzeEigenenKonturOrdnerFuerTests(QString());
    }
};

/// Packt den Loop des Bestands; zurueck: der Pfad des Pakets.
[[maybe_unused]] QString packeTitan(const Bestand& b)
{
    const pb::Fund fund = pb::sammle(b.loopDatei);
    REQUIRE(fund.ok);
    const QString ziel = QDir(b.tmp.path()).filePath(pb::paketName(fund));
    REQUIRE(pb::packe(fund, ziel, QStringLiteral("1.2.3")).ok);
    return ziel;
}

}  // namespace
