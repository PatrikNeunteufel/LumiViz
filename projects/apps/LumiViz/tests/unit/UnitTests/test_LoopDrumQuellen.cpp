/**
 * @file test_LoopDrumQuellen.cpp
 * @brief Loop, Stufe 11c, Schritt 1: eine Drum-Spur mit drei Arten von Klang —
 *        aus dem Kit, ein eigenes Sample, ein Instrument mit Note und Laenge;
 *        `.lvloop` in Fassung 4; die Art „Drums"
 *        (docs/audio/Loop_Konzept.md §4h; Entscheide K1 a, K2 a, K3 a, Patrik S115)
 *
 * Ohne Ton, ohne Fenster. Eigene Samples liegen in einem Wegwerf-Ordner, die
 * Engine aus Pappe gibt beim Dekodieren einen gerechneten Sinus heraus.
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/InstrumentPalette.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Sample.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>

#include <cmath>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr double kPi = 3.14159265358979323846;

/// Eigene Samples in einem Wegwerf-Ordner; nach dem Fall ist nichts gemerkt.
struct Ordner
{
    QTemporaryDir tmp;
    QString samples;
    Ordner()
    {
        REQUIRE(tmp.isValid());
        samples = QDir(tmp.path()).filePath(QStringLiteral("samples"));
        setzeEigenenSampleOrdnerFuerTests(samples);
        vergissSampleDaten();
    }
    ~Ordner()
    {
        setzeEigenenSampleOrdnerFuerTests(QString());
        vergissSampleDaten();
    }
    [[nodiscard]] QString datei(const QString& rel, const QByteArray& inhalt) const
    {
        const QString pfad = QDir(tmp.path()).filePath(rel);
        REQUIRE(QDir().mkpath(QFileInfo(pfad).absolutePath()));
        QFile f(pfad);
        REQUIRE(f.open(QIODevice::WriteOnly));
        f.write(inhalt);
        return pfad;
    }
};

/// Ein Sinus von einer Sekunde in `rate`, rechts halb so laut.
std::vector<float> sinus(int rate)
{
    std::vector<float> daten(2 * static_cast<std::size_t>(rate));
    for (std::size_t t = 0; t < static_cast<std::size_t>(rate); ++t)
    {
        const auto wert = static_cast<float>(0.8 * std::sin(2.0 * kPi * 440.0 * static_cast<double>(t) / rate));
        daten[2 * t] = wert;
        daten[2 * t + 1] = 0.5f * wert;
    }
    return daten;
}

/// Die Engine aus Pappe dekodiert: was es als Datei gibt, ist ein Sinus.
class DekodierEngine : public papp::PappMischEngine
{
public:
    QStringList gefragt;
    [[nodiscard]] std::vector<float> decodeStereo(const QString& filePath, int sampleRate, double) override
    {
        gefragt << QFileInfo(filePath).fileName();
        return QFileInfo::exists(filePath) ? sinus(sampleRate) : std::vector<float>{};
    }
};

/// Ein Drumkit „D" mit drei Spuren: aus dem Kit, ein eigenes Sample, ein Instrument.
Loop dreiQuellen(const QString& instrument)
{
    Loop loop;
    loop.laut = 1.0;
    loop.schlaegeJeTakt = 1;
    Muster m;
    m.name = QStringLiteral("D");
    Spur zap{QStringLiteral("Zap"), QStringLiteral("kit:snare"), 0.7, 0.0, false};
    zap.instrument = instrument;
    zap.hoehe = 36;
    zap.dauerMs = 400;
    m.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false},
                {QStringLiteral("Knock"), QStringLiteral("user:knock"), 0.8, -0.5, false},
                zap};
    m.zeilen = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0.5}};
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("D"), 0, 1}}}};
    loop.richte();
    return loop;
}

}  // namespace

TEST_SUITE("Loop Drum-Quellen")
{
    TEST_CASE("Loop Drum-Spur: drei Arten von Klang — was sie spielt, die Namen im Ablauf")
    {
        // --- eine Spur spielt ihren Klang, oder den Ton ihres Instruments
        Spur spur;
        CHECK(spur.instrument.isEmpty());
        CHECK(spur.hoehe == 60);
        CHECK(spur.dauerMs == 250);
        CHECK(spur.spielt().isEmpty());
        spur.klang = QStringLiteral("kit:kick");
        CHECK(spur.spielt() == QStringLiteral("kit:kick"));
        spur.instrument = QStringLiteral("user:kick_2");
        spur.hoehe = 36;
        spur.dauerMs = 500;
        CHECK(spur.spielt() == QStringLiteral("inst:user:kick_2:36:500"));  // der Klang zaehlt nicht mehr, bleibt aber stehen
        CHECK(spur.klang == QStringLiteral("kit:kick"));
        QString instrument;
        int hoehe = 0;
        int dauer = 0;
        REQUIRE(zerlegeTon(spur.spielt(), instrument, hoehe, dauer));
        CHECK(instrument == QStringLiteral("user:kick_2"));
        CHECK(hoehe == 36);
        CHECK(dauer == 500);

        CHECK(istEigenerKlang(QStringLiteral("user:knock")));
        CHECK_FALSE(istEigenerKlang(QStringLiteral("kit:kick")));
        CHECK_FALSE(istEigenerKlang(QStringLiteral("C:/x/knock.wav")));

        // --- der Loop nennt je Spur, was sie spielt; der Ablauf bindet die Schlaege daran
        const Loop loop = dreiQuellen(QStringLiteral("organ"));
        CHECK(loop.klangNamen() ==
              QStringList{QStringLiteral("kit:kick"), QStringLiteral("user:knock"), QStringLiteral("inst:organ:36:400")});
        const Ablauf ablauf = plane(loop, 44100);
        REQUIRE(ablauf.stimmen.size() == 3);
        CHECK(ablauf.stimmen[0].klang == 0);
        CHECK(ablauf.stimmen[1].klang == 1);
        CHECK(ablauf.stimmen[2].klang == 2);
        REQUIRE(ablauf.stimmen[2].schlaege.size() == 2);
        CHECK(ablauf.stimmen[2].schlaege[0].links == doctest::Approx(0.7f));  // die Lautstaerke der Spur gilt auch dem Ton
        CHECK(ablauf.stimmen[2].schlaege[1].links == doctest::Approx(0.35f));

        // eine andere Note oder Laenge ist ein anderer Ton; zwei Spuren mit demselben teilen ihn sich
        Loop zwei = loop;
        zwei.muster[0].spuren[0] = zwei.muster[0].spuren[2];
        CHECK(zwei.klangNamen() == QStringList{QStringLiteral("inst:organ:36:400"), QStringLiteral("user:knock")});
        zwei.muster[0].spuren[0].hoehe = 38;
        CHECK(zwei.klangNamen().size() == 3);
        CHECK(zwei.klangNamen().first() == QStringLiteral("inst:organ:38:400"));
    }

    TEST_CASE("Loop Drum-Spur in der Datei: Fassung 4 — ein eigenes Sample bleibt, wie es heisst; das Instrument mit Note und Laenge")
    {
        const Ordner u;
        (void)u.datei(QStringLiteral("samples/knock.wav"), "AAAA");
        const QString instDatei = u.datei(QStringLiteral("loops/zap.lvinst"), "{}");
        const QString basis = QDir(u.tmp.path()).filePath(QStringLiteral("loops"));
        const Loop loop = dreiQuellen(instDatei);

        const QJsonObject json = alsJson(loop, basis);
        CHECK(json.value(QStringLiteral("version")).toInt() == 9);  // die Felder gibt es seit Fassung 4; geschrieben wird die juengste
        const QJsonArray spuren = json.value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("tracks")).toArray();
        REQUIRE(spuren.size() == 3);
        CHECK(spuren.at(0).toObject().value(QStringLiteral("sound")).toString() == QStringLiteral("kit:kick"));
        CHECK_FALSE(spuren.at(0).toObject().contains(QStringLiteral("instrument")));
        CHECK_FALSE(spuren.at(0).toObject().contains(QStringLiteral("note")));
        // ein eigenes Sample ist kein Pfad: es steht da, wie es heisst
        CHECK(spuren.at(1).toObject().value(QStringLiteral("sound")).toString() == QStringLiteral("user:knock"));
        // das Instrument: eine Datei unter dem Ordner des Loops steht relativ; der Klang bleibt daneben stehen
        const QJsonObject zap = spuren.at(2).toObject();
        CHECK(zap.value(QStringLiteral("instrument")).toString() == QStringLiteral("zap.lvinst"));
        CHECK(zap.value(QStringLiteral("note")).toInt() == 36);
        CHECK(zap.value(QStringLiteral("length")).toInt() == 400);
        CHECK(zap.value(QStringLiteral("sound")).toString() == QStringLiteral("kit:snare"));

        Loop gelesen;
        QStringList bericht;
        REQUIRE(ausJson(json, gelesen, basis, &bericht));
        CHECK(bericht.isEmpty());
        CHECK(gelesen == loop);

        // --- ein mitgeliefertes oder eigenes Instrument steht beim Namen
        Loop benannt = dreiQuellen(QStringLiteral("user:kick_2"));
        const QJsonObject json2 = alsJson(benannt, basis);
        CHECK(json2.value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("tracks")).toArray().at(2).toObject()
                  .value(QStringLiteral("instrument")).toString() == QStringLiteral("user:kick_2"));
        REQUIRE(ausJson(json2, gelesen, basis, &bericht));
        CHECK(gelesen == benannt);
        CHECK(bericht.isEmpty());  // ob es das Instrument gibt, sagt erst das Laden der Klaenge

        // --- was fehlt, steht im Bericht: das Sample, die Datei des Instruments
        REQUIRE(loescheEigenesSample(QStringLiteral("knock")));
        QFile::remove(instDatei);
        REQUIRE(ausJson(json, gelesen, basis, &bericht));
        CHECK(bericht == QStringList{QStringLiteral("Sample not found: user:knock"),
                                     QStringLiteral("File not found: ") + QDir::toNativeSeparators(instDatei)});
        CHECK(gelesen == loop);  // der Loop steht trotzdem da, wie er war

        // --- eine Datei der Fassung 3 kennt das alles nicht: die Spur spielt ihren Klang
        QJsonObject alt = json;
        alt[QStringLiteral("version")] = 3;
        {
            QJsonArray muster = alt.value(QStringLiteral("patterns")).toArray();
            QJsonObject m = muster.at(0).toObject();
            QJsonArray tracks = m.value(QStringLiteral("tracks")).toArray();
            QJsonObject dritte = tracks.at(2).toObject();
            dritte.remove(QStringLiteral("instrument"));
            dritte.remove(QStringLiteral("note"));
            dritte.remove(QStringLiteral("length"));
            tracks[2] = dritte;
            m[QStringLiteral("tracks")] = tracks;
            muster[0] = m;
            alt[QStringLiteral("patterns")] = muster;
        }
        bericht.clear();
        REQUIRE(ausJson(alt, gelesen, basis, &bericht));
        const Spur& ohne = gelesen.muster[0].spuren[2];
        CHECK(ohne.instrument.isEmpty());
        CHECK(ohne.hoehe == 60);
        CHECK(ohne.dauerMs == 250);
        CHECK(ohne.spielt() == QStringLiteral("kit:snare"));

        // --- Werte in ihren Grenzen; eine Fassung von morgen sagt es
        QJsonObject wild = json;
        {
            QJsonArray muster = wild.value(QStringLiteral("patterns")).toArray();
            QJsonObject m = muster.at(0).toObject();
            QJsonArray tracks = m.value(QStringLiteral("tracks")).toArray();
            QJsonObject dritte = tracks.at(2).toObject();
            dritte[QStringLiteral("note")] = 999;
            dritte[QStringLiteral("length")] = 0;
            tracks[2] = dritte;
            QJsonObject erste = tracks.at(0).toObject();
            erste[QStringLiteral("instrument")] = QStringLiteral("organ");
            erste[QStringLiteral("length")] = 99999999;
            tracks[0] = erste;
            m[QStringLiteral("tracks")] = tracks;
            muster[0] = m;
            wild[QStringLiteral("patterns")] = muster;
            wild[QStringLiteral("version")] = alsJson(Loop{}).value(QStringLiteral("version")).toInt() + 1;  // von morgen
        }
        bericht.clear();
        REQUIRE(ausJson(wild, gelesen, basis, &bericht));
        CHECK(gelesen.muster[0].spuren[2].hoehe == 127);
        CHECK(gelesen.muster[0].spuren[2].dauerMs == 1);
        CHECK(gelesen.muster[0].spuren[0].dauerMs == 30000);
        CHECK(bericht.contains(QStringLiteral("File was written by a newer version")));
    }

    TEST_CASE("Loop Drum-Spur: ein eigenes Sample wird geladen — und als Instrument gespielt klingt es wie die Datei")
    {
        const Ordner u;
        DekodierEngine engine;
        (void)u.datei(QStringLiteral("samples/knock.wav"), "AAAA");

        // --- ein eigenes Sample als Klang einer Spur: die Engine dekodiert seine Datei
        CHECK(klangDatei(QStringLiteral("user:knock")) == QDir(u.samples).filePath(QStringLiteral("knock.wav")));
        CHECK(klangDatei(QStringLiteral("user:gibt_es_nicht")).isEmpty());
        CHECK(klangDatei(QStringLiteral("user:../samples/knock")).isEmpty());  // nichts ausserhalb des Ordners
        QStringList bericht;
        const std::vector<Klang> direkt =
            ladeKlaenge({QStringLiteral("user:knock"), QStringLiteral("user:gibt_es_nicht")}, engine, 48000, &bericht);
        REQUIRE(direkt.size() == 2);
        CHECK(direkt[0].daten == sinus(48000));
        CHECK(direkt[1].frames() == 0);
        CHECK(bericht == QStringList{QStringLiteral("Sound not loaded: user:gibt_es_nicht")});

        // --- dasselbe Sample in einer Sample-Schicht, gespielt bei seinem Grundton und so lang, wie es ist
        //     (das macht „Edit sound..." aus einer Spur): es klingt wie die Datei — nicht halb so laut
        Instrument instrument;
        instrument.titel = QStringLiteral("Knock 2");
        Schicht& s = instrument.schichten[0];
        s.schwingung = Schwingung::Sample;
        s.sample = QStringLiteral("user:knock");
        s.grundton = 60;
        s.anstiegMs = 0.0;
        s.halten = 1.0;
        const QString pfad = QDir(u.tmp.path()).filePath(QStringLiteral("knock_2.lvinst"));
        REQUIRE(speichereInstrument(instrument, pfad));
        Spur spur;
        spur.instrument = pfad;
        spur.hoehe = 60;
        spur.dauerMs = 1000;
        bericht.clear();
        const std::vector<Klang> alsTon = ladeKlaenge({spur.spielt()}, engine, 48000, &bericht);
        REQUIRE(alsTon.size() == 1);
        CHECK(bericht.isEmpty());
        REQUIRE(alsTon[0].frames() >= 48000);
        bool gleich = true;
        for (std::size_t i = 0; i < 2 * 47999; ++i)
            if (alsTon[0].daten[i] != direkt[0].daten[i]) gleich = false;
        CHECK(gleich);
        // dahinter ist nichts mehr: die Aufnahme ist zu Ende
        float rest = 0.0f;
        for (std::size_t i = 2 * 48000; i < alsTon[0].daten.size(); ++i) rest = std::max(rest, std::abs(alsTon[0].daten[i]));
        CHECK(rest == 0.0f);
    }

    TEST_CASE("Loop Drum-Spur: eine Sample-Schicht nennt einen Klang des Kits — und die Art Drums")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QCoreApplication app{argc, argv};

        // --- `kit:kick` in einer Sample-Schicht: die Datei des Kits, auf jedem Rechner
        const QString kick = klangDatei(QStringLiteral("kit:kick"));
        REQUIRE_FALSE(kick.isEmpty());
        CHECK(sampleDatei(QStringLiteral("kit:kick")) == kick);
        CHECK(QFileInfo::exists(sampleDatei(QStringLiteral("kit:kick"))));
        CHECK(sampleTitel(QStringLiteral("kit:kick")) == QStringLiteral("kick"));
        CHECK(sampleDatei(QStringLiteral("kit:../kit/kick")).isEmpty());  // nur Namen, die das Kit kennt
        CHECK_FALSE(istEigenesSample(QStringLiteral("kit:kick")));

        // --- die neunte Art: zwischen Percussion und Unusual; ein Schlag spielt tief
        const std::vector<InstrumentArt>& arten = instrumentArten();
        REQUIRE(arten.size() == 9);
        CHECK(QLatin1String(arten[6].schluessel) == QLatin1String("percussion"));
        CHECK(QLatin1String(arten[7].schluessel) == QLatin1String("drums"));
        CHECK(QLatin1String(arten[7].titel) == QLatin1String("Drums"));
        CHECK(QLatin1String(arten[8].schluessel) == QLatin1String("unusual"));
        CHECK(gueltigeArt(QStringLiteral("Drums")) == QStringLiteral("drums"));
        CHECK(artTitel(QStringLiteral("drums")) == QStringLiteral("Drums"));
        CHECK(InstrumentPalette::hoeheFuerArt(QStringLiteral("drums")) == 36);
        CHECK(InstrumentPalette::hoeheFuerArt(QStringLiteral("bass")) == 36);
        CHECK(InstrumentPalette::hoeheFuerArt(QStringLiteral("keys")) == 60);
        // ein Instrument mit dieser Art liest und schreibt sich
        Instrument schlag;
        schlag.titel = QStringLiteral("Kick 2");
        schlag.art = QStringLiteral("drums");
        Instrument gelesen;
        REQUIRE(ausJson(alsJson(schlag), gelesen));
        CHECK(gelesen.art == QStringLiteral("drums"));
    }
}
