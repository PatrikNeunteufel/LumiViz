/**
 * @file test_LoopSample.cpp
 * @brief Loop, Stufe 11a: Samples — der Ordner `samples` im Benutzerordner,
 *        die gemerkten Daten, das Sample als Schicht eines Instruments
 *        (docs/audio/Loop_Konzept.md §4h; Entscheide S1 a und S2 a, Patrik S115)
 *
 * Ohne Ton, ohne Fenster: die Aufnahme ist ein gerechneter Sinus, die Engine
 * aus Pappe gibt ihn beim Dekodieren heraus. Eigene Samples liegen in einem
 * Wegwerf-Ordner (`setzeEigenenSampleOrdnerFuerTests`).
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Sample.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;
constexpr double kPi = 3.14159265358979323846;

/// Eigene Samples in einem Wegwerf-Ordner; nach dem Fall ist nichts gemerkt.
struct SampleOrdner
{
    QTemporaryDir tmp;
    QString ordner;

    SampleOrdner()
    {
        REQUIRE(tmp.isValid());
        ordner = QDir(tmp.path()).filePath(QStringLiteral("samples"));
        setzeEigenenSampleOrdnerFuerTests(ordner);
        vergissSampleDaten();
    }
    ~SampleOrdner()
    {
        setzeEigenenSampleOrdnerFuerTests(QString());
        vergissSampleDaten();
    }
    /// Eine Datei mit diesem Inhalt; ihr Pfad.
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

/// Ein Sinus als Aufnahme: `hz`, `sekunden` lang, in `rate`; rechts halb so laut.
std::vector<float> sinus(double hz, double sekunden, int rate)
{
    const auto frames = static_cast<std::size_t>(sekunden * rate);
    std::vector<float> daten(2 * frames);
    for (std::size_t t = 0; t < frames; ++t)
    {
        const auto wert = static_cast<float>(std::sin(2.0 * kPi * hz * static_cast<double>(t) / rate));
        daten[2 * t] = wert;
        daten[2 * t + 1] = 0.5f * wert;
    }
    return daten;
}

/// Ein Instrument mit einer Sample-Schicht: voll gehalten, kurzer Ausklang.
Instrument sampleInstrument(const QString& sample, int grundton = 69)
{
    Instrument instrument;
    instrument.titel = QStringLiteral("Probe");
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Sample;
    s.sample = sample;
    s.grundton = grundton;
    s.anstiegMs = 0.0;
    s.halten = 1.0;
    s.ausklangMs = 20.0;
    return instrument;
}

float spitze(const Klang& klang, std::int64_t von, std::int64_t bis, std::size_t seite = 0)
{
    float groesster = 0.0f;
    for (std::int64_t t = von; t < bis && t < klang.frames(); ++t)
        groesster = std::max(groesster, std::abs(klang.daten[static_cast<std::size_t>(2 * t) + seite]));
    return groesster;
}

/// Die Periode in Abtastwerten: die Verschiebung mit der groessten Uebereinstimmung.
int periode(const Klang& klang, int ab, int laenge, int von, int bis)
{
    int beste = von;
    double bestWert = -1e30;
    for (int lag = von; lag <= bis; ++lag)
    {
        double summe = 0.0;
        for (int i = 0; i < laenge; ++i)
            summe += static_cast<double>(klang.daten[static_cast<std::size_t>(2 * (ab + i))]) *
                     klang.daten[static_cast<std::size_t>(2 * (ab + i + lag))];
        if (summe > bestWert)
        {
            bestWert = summe;
            beste = lag;
        }
    }
    return beste;
}

/// Die Engine aus Pappe dekodiert: sie gibt einen Sinus heraus und merkt, wonach gefragt wurde.
class DekodierEngine : public papp::PappMischEngine
{
public:
    QStringList gefragt;
    [[nodiscard]] std::vector<float> decodeStereo(const QString& filePath, int sampleRate, double) override
    {
        gefragt << filePath;
        return QFileInfo::exists(filePath) ? sinus(440.0, 1.0, sampleRate) : std::vector<float>{};
    }
};

}  // namespace

TEST_SUITE("Loop Sample")
{
    TEST_CASE("Loop Sample: der Ordner samples — nennen, auflisten, dazukopieren, loeschen")
    {
        const SampleOrdner u;

        // --- ohne Ordner: leer, kein Fehler
        CHECK(eigeneSamples().isEmpty());
        CHECK(sampleDatei(QStringLiteral("user:table_knock")).isEmpty());
        CHECK(sampleDatei(QString()).isEmpty());
        CHECK(istEigenesSample(QStringLiteral("user:table_knock")));
        CHECK_FALSE(istEigenesSample(QStringLiteral("C:/x/knock.wav")));

        // --- dazukopieren: der Name aus dem Dateinamen, die Endung bleibt
        const QString quelle = u.datei(QStringLiteral("quelle/Table Knock.wav"), "AAAA");
        const std::uint64_t vorher = instrumentFassung();
        CHECK(nimmZuEigenen(quelle) == QStringLiteral("user:table_knock"));
        CHECK(QFileInfo::exists(QDir(u.ordner).filePath(QStringLiteral("table_knock.wav"))));
        CHECK(instrumentFassung() > vorher);  // wer Toene gerechnet hat, rechnet neu
        CHECK(sampleDatei(QStringLiteral("user:table_knock")) == QDir(u.ordner).filePath(QStringLiteral("table_knock.wav")));
        CHECK(sampleTitel(QStringLiteral("user:table_knock")) == QStringLiteral("table_knock"));
        CHECK(sampleTitel(quelle) == QStringLiteral("Table Knock.wav"));
        CHECK(sampleDatei(quelle) == quelle);  // eine Datei an freiem Ort steht beim Pfad

        // dieselbe Datei noch einmal: dieselbe Nennung, keine zweite Kopie
        CHECK(nimmZuEigenen(quelle) == QStringLiteral("user:table_knock"));
        CHECK(QDir(u.ordner).entryList(QDir::Files).size() == 1);
        // aus dem Ordner selbst gewaehlt: sie ist es schon
        CHECK(nimmZuEigenen(QDir(u.ordner).filePath(QStringLiteral("table_knock.wav"))) == QStringLiteral("user:table_knock"));
        CHECK(QDir(u.ordner).entryList(QDir::Files).size() == 1);
        // derselbe Name, anderer Inhalt: eine Zahl dahinter
        const QString andere = u.datei(QStringLiteral("woanders/table knock.wav"), "BBBBBB");
        CHECK(nimmZuEigenen(andere) == QStringLiteral("user:table_knock_2"));
        CHECK(nimmZuEigenen(andere) == QStringLiteral("user:table_knock_2"));
        // eine andere Endung bleibt, der Name ist ohne sie eindeutig
        const QString mp3 = u.datei(QStringLiteral("quelle/Table Knock.MP3"), "CCC");
        CHECK(nimmZuEigenen(mp3) == QStringLiteral("user:table_knock_3"));
        CHECK(QFileInfo::exists(QDir(u.ordner).filePath(QStringLiteral("table_knock_3.mp3"))));
        // was kein Sample ist, was fehlt, woraus kein Name wird
        CHECK(nimmZuEigenen(u.datei(QStringLiteral("quelle/notiz.txt"), "x")).isEmpty());
        CHECK(nimmZuEigenen(QDir(u.tmp.path()).filePath(QStringLiteral("fehlt.wav"))).isEmpty());
        CHECK(nimmZuEigenen(u.datei(QStringLiteral("quelle/###.wav"), "x")).isEmpty());

        // --- die Liste: nach Namen; was keine Schicht nennen kann, fehlt
        (void)u.datei(QStringLiteral("samples/Von Hand.wav"), "x");
        (void)u.datei(QStringLiteral("samples/liesmich.txt"), "x");
        CHECK(eigeneSamples() == QStringList{QStringLiteral("user:table_knock"), QStringLiteral("user:table_knock_2"),
                                             QStringLiteral("user:table_knock_3")});
        // eine Schicht kann nichts ausserhalb des Ordners nennen
        CHECK(sampleDatei(QStringLiteral("user:../quelle/Table Knock")).isEmpty());
        CHECK(sampleDatei(QStringLiteral("user:")).isEmpty());

        // --- loeschen
        REQUIRE(loescheEigenesSample(QStringLiteral("table_knock_2")));
        CHECK_FALSE(loescheEigenesSample(QStringLiteral("table_knock_2")));
        CHECK(eigeneSamples().size() == 2);
    }

    TEST_CASE("Loop Sample: die gemerkten Daten — geteilt je Datei, vergessen, wenn sie eine andere wird")
    {
        const SampleOrdner u;
        const QString pfad = u.datei(QStringLiteral("samples/ton.wav"), "AAAA");

        CHECK(sampleDaten(QStringLiteral("user:ton")) == nullptr);
        merkeSampleDaten(QStringLiteral("user:ton"), 48000, sinus(440.0, 0.1, 48000));
        const auto daten = sampleDaten(QStringLiteral("user:ton"));
        REQUIRE(daten != nullptr);
        CHECK(daten->rate == 48000);
        CHECK(daten->klang.frames() == 4800);
        CHECK(sampleDaten(pfad) == daten);  // dieselbe Datei beim Pfad genannt: dieselben Daten

        // Leeres wird nicht gemerkt
        merkeSampleDaten(QStringLiteral("frei:leer"), 48000, {});
        CHECK(sampleDaten(QStringLiteral("frei:leer")) == nullptr);
        merkeSampleDaten(QStringLiteral("frei:ohne_rate"), 0, sinus(440.0, 0.1, 48000));
        CHECK(sampleDaten(QStringLiteral("frei:ohne_rate")) == nullptr);

        // die Datei wird eine andere: was gemerkt war, gilt nicht mehr
        (void)u.datei(QStringLiteral("samples/ton.wav"), "BBBBBBBB");
        CHECK(sampleDaten(QStringLiteral("user:ton")) == nullptr);
        CHECK(daten->klang.frames() == 4800);  // wer sie noch haelt, behaelt sie

        // ohne Datei gemerkt (ein Test, eine Aufnahme vor dem Speichern)
        merkeSampleDaten(QStringLiteral("frei:probe"), 44100, sinus(440.0, 0.1, 44100));
        REQUIRE(sampleDaten(QStringLiteral("frei:probe")) != nullptr);
        vergissSampleDaten(QStringLiteral("frei:probe"));
        CHECK(sampleDaten(QStringLiteral("frei:probe")) == nullptr);
    }

    TEST_CASE("Loop Sample als Schicht: die Tonhoehe aus dem Abstand zum Grundton, Sample start, Sustain loop")
    {
        const SampleOrdner u;
        // die Aufnahme: A4 (440 Hz), eine Sekunde, in 48000 — gerechnet wird in 44100
        const QString nennung = QStringLiteral("frei:a4");
        merkeSampleDaten(nennung, 48000, sinus(440.0, 1.0, 48000));
        const Instrument instrument = sampleInstrument(nennung);

        // --- am Grundton: 440 Hz, rund 100 Abtastwerte je Schwingung; die Seiten bleiben verschieden
        const Klang a4 = rendereTon(instrument, 69, 500.0, kRate);
        REQUIRE(a4.frames() == 22050 + 882);  // die Note und 20 ms Ausklang
        const int p = periode(a4, 4000, 4000, 80, 125);
        CHECK(p >= 99);
        CHECK(p <= 102);
        CHECK(spitze(a4, 2000, 20000) == doctest::Approx(1.0f).epsilon(0.02));         // so laut wie die Aufnahme (seit Stufe 11c; davor halb)
        CHECK(spitze(a4, 2000, 20000, 1) == doctest::Approx(0.5f).epsilon(0.02));      // rechts halb so laut
        CHECK(std::abs(a4.daten[a4.daten.size() - 2]) < 0.002f);                       // endet bei null

        // --- eine Oktave hoeher: halbe Periode, und die Aufnahme ist nach einer halben Sekunde zu Ende
        const Klang a5 = rendereTon(instrument, 81, 1000.0, kRate);
        const int pHoch = periode(a5, 4000, 4000, 40, 62);
        CHECK(pHoch >= 49);
        CHECK(pHoch <= 51);
        CHECK(spitze(a5, 2000, 20000) > 0.4f);
        CHECK(spitze(a5, 23000, 44100) == 0.0f);
        // eine Oktave tiefer: doppelte Periode
        const int pTief = periode(rendereTon(instrument, 57, 500.0, kRate), 4000, 6000, 160, 250);
        CHECK(pTief >= 198);
        CHECK(pTief <= 203);

        // --- Fine tune: die Aufnahme liegt ueber dem Grundton — G#4 und +100 ct ist wieder A4
        Instrument verstimmt = sampleInstrument(nennung, 68);
        verstimmt.schichten[0].feinCent = 100.0;
        const Klang gleich = rendereTon(verstimmt, 69, 500.0, kRate);
        REQUIRE(gleich.daten.size() == a4.daten.size());
        double abweichung = 0.0;
        for (std::size_t i = 0; i < a4.daten.size(); ++i)
            abweichung = std::max(abweichung, static_cast<double>(std::abs(gleich.daten[i] - a4.daten[i])));
        CHECK(abweichung < 1e-4);
        // Transpose der Schicht gilt wie bei jeder anderen
        Instrument versetzt = instrument;
        versetzt.schichten[0].versatzHalbtoene = 12;
        CHECK(rendereTon(versetzt, 69, 1000.0, kRate).daten == a5.daten);

        // --- laenger als die Aufnahme: danach ist es still, der Klang bleibt so lang wie die Note
        const Klang lang = rendereTon(instrument, 69, 2000.0, kRate);
        CHECK(lang.frames() == 88200 + 882);
        CHECK(spitze(lang, 40000, 44000) > 0.4f);
        CHECK(spitze(lang, 44200, lang.frames()) == 0.0f);

        // --- Sample start: ab der Mitte bleibt eine halbe Sekunde
        Instrument spaeter = instrument;
        spaeter.schichten[0].sampleStartMs = 500.0;
        const Klang halb = rendereTon(spaeter, 69, 2000.0, kRate);
        CHECK(spitze(halb, 2000, 21000) > 0.4f);
        CHECK(spitze(halb, 22200, halb.frames()) == 0.0f);

        // --- Sustain loop: solange die Note dauert, wiederholt sich das Stueck; danach laeuft es aus
        Instrument gehalten = instrument;
        gehalten.schichten[0].schleife = true;
        gehalten.schichten[0].schleifeVonMs = 200.0;
        gehalten.schichten[0].schleifeBisMs = 700.0;
        const Klang schleife = rendereTon(gehalten, 69, 2000.0, kRate);
        CHECK(spitze(schleife, 44200, 88000) > 0.4f);  // wo es ohne Schleife still war
        CHECK(spitze(schleife, 44200, 88000) < 1.02f);  // an der Naht wird nichts lauter
        const int pSchleife = periode(schleife, 60000, 4000, 80, 125);
        CHECK(pSchleife >= 99);
        CHECK(pSchleife <= 102);
        CHECK(std::abs(schleife.daten[schleife.daten.size() - 2]) < 0.002f);
        // bis nicht groesser als von: keine Schleife
        gehalten.schichten[0].schleifeBisMs = 100.0;
        CHECK(rendereTon(gehalten, 69, 2000.0, kRate).daten == lang.daten);
        // der Haken aus: die Zahlen zaehlen nicht
        gehalten.schichten[0].schleifeBisMs = 700.0;
        gehalten.schichten[0].schleife = false;
        CHECK(rendereTon(gehalten, 69, 2000.0, kRate).daten == lang.daten);

        // --- derselbe Aufruf ergibt dieselben Werte
        CHECK(rendereTon(instrument, 69, 500.0, kRate).daten == a4.daten);

        // --- Filter und Drive gelten: ein enger Filter macht leiser, Drive formt um
        Instrument dumpf = instrument;
        dumpf.schichten[0].holeFilter().teilton = 0.25;  // Grenze bei einem Viertel des Tons
        CHECK(spitze(rendereTon(dumpf, 69, 500.0, kRate), 4000, 20000) < 0.2f);
        Instrument hart = instrument;
        hart.schichten[0].holeDrive().wert = 1.0;
        const Klang verzerrt = rendereTon(hart, 69, 500.0, kRate);
        CHECK(verzerrt.daten != a4.daten);
        CHECK(spitze(verzerrt, 2000, 20000) <= 1.001f);

        // --- nichts gemerkt oder nichts genannt: die Schicht schweigt, der Ton ist so lang wie sonst
        const Klang stumm = rendereTon(sampleInstrument(QStringLiteral("frei:fehlt")), 69, 500.0, kRate);
        CHECK(stumm.frames() == a4.frames());
        CHECK(spitze(stumm, 0, stumm.frames()) == 0.0f);
        CHECK(spitze(rendereTon(sampleInstrument(QString()), 69, 500.0, kRate), 0, 30000) == 0.0f);

        // --- gemischt mit einer gerechneten Schicht: die Summe
        Instrument gemischt = instrument;
        Schicht sinusSchicht;
        sinusSchicht.halten = 1.0;
        sinusSchicht.anstiegMs = 0.0;
        sinusSchicht.ausklangMs = 20.0;
        gemischt.schichten.push_back(sinusSchicht);
        Instrument nurSinus = gemischt;
        nurSinus.schichten.erase(nurSinus.schichten.begin());
        const Klang summe = rendereTon(gemischt, 69, 500.0, kRate);
        const Klang zweite = rendereTon(nurSinus, 69, 500.0, kRate);
        REQUIRE(summe.daten.size() == a4.daten.size());
        REQUIRE(zweite.daten.size() == a4.daten.size());
        bool stimmt = true;
        for (std::size_t i = 0; i < summe.daten.size(); ++i)
            if (summe.daten[i] != a4.daten[i] + zweite.daten[i]) stimmt = false;
        CHECK(stimmt);
    }

    TEST_CASE("Loop Sample in der Datei: Fassung 4 — nur die Sample-Schicht traegt die Felder, Fassung 3 liest weiter")
    {
        Instrument instrument = sampleInstrument(QStringLiteral("user:cello_a3"), 57);
        Schicht& s = instrument.schichten[0];
        s.name = QStringLiteral("Cello");
        s.feinCent = 12.0;
        s.sampleStartMs = 20.0;
        s.schleife = true;
        s.schleifeVonMs = 1100.0;
        s.schleifeBisMs = 1850.0;
        instrument.schichten.push_back(Schicht{});  // dazu ein Sinus

        const QJsonObject json = alsJson(instrument);
        CHECK(kInstrumentFassung >= 4);  // die Felder gibt es seit Fassung 4; geschrieben wird die juengste
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);
        const QJsonArray schichten = json.value(QStringLiteral("layers")).toArray();
        REQUIRE(schichten.size() == 2);
        const QJsonObject erste = schichten.at(0).toObject();
        CHECK(erste.value(QStringLiteral("waveform")).toString() == QStringLiteral("sample"));
        CHECK(erste.value(QStringLiteral("sample")).toString() == QStringLiteral("user:cello_a3"));
        CHECK(erste.value(QStringLiteral("rootNote")).toInt() == 57);
        CHECK(erste.value(QStringLiteral("fineTune")).toDouble() == doctest::Approx(12.0));
        CHECK(erste.value(QStringLiteral("sampleStart")).toDouble() == doctest::Approx(20.0));
        CHECK(erste.value(QStringLiteral("sustainLoop")).toBool());
        CHECK(erste.value(QStringLiteral("loopFrom")).toDouble() == doctest::Approx(1100.0));
        CHECK(erste.value(QStringLiteral("loopTo")).toDouble() == doctest::Approx(1850.0));
        const QJsonObject zweite = schichten.at(1).toObject();
        CHECK_FALSE(zweite.contains(QStringLiteral("sample")));
        CHECK_FALSE(zweite.contains(QStringLiteral("rootNote")));

        Instrument gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen == instrument);

        // --- Werte in ihren Grenzen
        QJsonObject wild = json;
        QJsonObject schicht = erste;
        schicht[QStringLiteral("rootNote")] = 300;
        schicht[QStringLiteral("fineTune")] = -500.0;
        schicht[QStringLiteral("sampleStart")] = -3.0;
        wild[QStringLiteral("layers")] = QJsonArray{schicht};
        REQUIRE(ausJson(wild, gelesen));
        CHECK(gelesen.schichten[0].grundton == 127);
        CHECK(gelesen.schichten[0].feinCent == doctest::Approx(-100.0));
        CHECK(gelesen.schichten[0].sampleStartMs == 0.0);

        // --- eine Datei der Fassung 3 kennt die Felder nicht: Grundeinstellung
        QJsonObject alt = json;
        alt[QStringLiteral("version")] = 3;
        alt[QStringLiteral("layers")] = QJsonArray{zweite};
        REQUIRE(ausJson(alt, gelesen));
        CHECK(gelesen.schichten[0] == Schicht{});
        CHECK(gelesen.schichten[0].sample.isEmpty());
        CHECK(gelesen.schichten[0].grundton == 60);

        // --- eine Fassung von morgen bleibt ungelesen
        QJsonObject morgen = json;
        morgen[QStringLiteral("version")] = kInstrumentFassung + 1;
        bool zuNeu = false;
        CHECK_FALSE(ausJson(morgen, gelesen, &zuNeu));
        CHECK(zuNeu);
    }

    TEST_CASE("Loop Sample: ladeKlaenge dekodiert die Aufnahme einmal — fehlt sie, steht es im Bericht")
    {
        const SampleOrdner u;
        DekodierEngine engine;

        // ein eigenes Sample und ein Instrument an freiem Ort, das es nennt
        (void)u.datei(QStringLiteral("samples/ton.wav"), "AAAA");
        const QString pfad = QDir(u.tmp.path()).filePath(QStringLiteral("probe.lvinst"));
        REQUIRE(speichereInstrument(sampleInstrument(QStringLiteral("user:ton")), pfad));

        QStringList bericht;
        const std::vector<Klang> klaenge =
            ladeKlaenge({tonKlang(pfad, 69, 250), tonKlang(pfad, 71, 250)}, engine, kRate, &bericht);
        REQUIRE(klaenge.size() == 2);
        CHECK(bericht.isEmpty());
        CHECK(engine.gefragt == QStringList{QDir(u.ordner).filePath(QStringLiteral("ton.wav"))});  // einmal, nicht je Ton
        CHECK(spitze(klaenge[0], 1000, 10000) > 0.4f);
        CHECK(spitze(klaenge[1], 1000, 10000) > 0.4f);
        const int p = periode(klaenge[0], 2000, 4000, 80, 125);
        CHECK(p >= 99);
        CHECK(p <= 102);
        REQUIRE(sampleDaten(QStringLiteral("user:ton")) != nullptr);
        CHECK(sampleDaten(QStringLiteral("user:ton"))->rate == kSampleRate);

        // noch einmal laden: die Daten sind gemerkt
        (void)ladeKlaenge({tonKlang(pfad, 69, 250)}, engine, kRate, &bericht);
        CHECK(engine.gefragt.size() == 1);

        // --- die Aufnahme fehlt, oder die Schicht nennt keine: der Ton ist still, der Bericht sagt es einmal
        const QString fehlt = QDir(u.tmp.path()).filePath(QStringLiteral("fehlt.wav"));
        const QString pfadFehlt = QDir(u.tmp.path()).filePath(QStringLiteral("ohne.lvinst"));
        Instrument ohne = sampleInstrument(fehlt);
        ohne.schichten.push_back(sampleInstrument(QString()).schichten[0]);
        ohne.schichten.push_back(sampleInstrument(QStringLiteral("user:gibt_es_nicht")).schichten[0]);
        ohne.schichten.push_back(sampleInstrument(QStringLiteral("user:ton")).schichten[0]);
        ohne.schichten[3].an = false;  // eine stumme Schicht wird nicht geladen
        REQUIRE(speichereInstrument(ohne, pfadFehlt));
        const std::vector<Klang> stumm =
            ladeKlaenge({tonKlang(pfadFehlt, 69, 250), tonKlang(pfadFehlt, 71, 250)}, engine, kRate, &bericht);
        CHECK(stumm[0].frames() > 0);
        CHECK(spitze(stumm[0], 0, stumm[0].frames()) == 0.0f);
        CHECK(bericht == QStringList{QStringLiteral("Sample not loaded: ") + QDir::toNativeSeparators(fehlt),
                                     QStringLiteral("Sample not loaded: (none)"),
                                     QStringLiteral("Sample not loaded: user:gibt_es_nicht")});
    }

    // Stufe 11d (Entscheid N2 a, Patrik S116): Pitch sweep gilt auch fuer eine Sample-Schicht
    TEST_CASE("Loop Sample 11d: Pitch sweep — die Aufnahme laeuft am Anfang schneller und ist frueher zu Ende")
    {
        const SampleOrdner u;
        // die Aufnahme: A4 (440 Hz), eine Sekunde
        const QString nennung = QStringLiteral("frei:a4_sweep");
        merkeSampleDaten(nennung, 48000, sinus(440.0, 1.0, 48000));
        Instrument instrument = sampleInstrument(nennung);
        const Klang gerade = rendereTon(instrument, 69, 1500.0, kRate);
        CHECK(spitze(gerade, 42000, 44000) > 0.9f);  // bis kurz vor einer Sekunde klingt sie

        // eine Oktave darueber beginnen, nach 300 ms angekommen
        instrument.schichten[0].tonhoeheStartHt = 12.0;
        instrument.schichten[0].tonhoeheZeitMs = 300.0;
        const Klang faellt = rendereTon(instrument, 69, 1500.0, kRate);
        REQUIRE(faellt.frames() == gerade.frames());
        // am Anfang fast 880 Hz: rund 50 Abtastwerte je Schwingung statt 100
        const int pAnfang = periode(faellt, 0, 300, 40, 75);
        CHECK(pAnfang >= 49);
        CHECK(pAnfang <= 57);
        // nach 600 ms auf dem Ton
        const int pSpaeter = periode(faellt, 26460, 4000, 80, 125);
        CHECK(pSpaeter >= 99);
        CHECK(pSpaeter <= 102);
        // der schnellere Anfang hat rund 100 ms der Aufnahme mehr verbraucht: sie endet bei rund 900 ms
        CHECK(spitze(faellt, 38000, 39000) > 0.9f);
        CHECK(spitze(faellt, 40500, 44100) == 0.0f);

        // darunter beginnen: sie steigt, und laeuft am Anfang langsamer
        instrument.schichten[0].tonhoeheStartHt = -12.0;
        const Klang steigt = rendereTon(instrument, 69, 1500.0, kRate);
        const int pTief = periode(steigt, 0, 600, 150, 230);
        CHECK(pTief >= 170);  // fast 440 / 2 Hz: bis zu 200 Abtastwerte je Schwingung
        CHECK(spitze(steigt, 44100, 45500) > 0.9f);  // nach einer Sekunde klingt sie noch

        // 0 st: die Zeit zaehlt nicht — Abtastwert fuer Abtastwert wie ohne
        instrument.schichten[0].tonhoeheStartHt = 0.0;
        CHECK(rendereTon(instrument, 69, 1500.0, kRate).daten == gerade.daten);

        // --- Stufe 11e: auch die Aufnahme geht durch Hochpass und Bandpass, je Seite ein Filter
        instrument.schichten[0].holeFilter().teilton = 8.0;  // 3520 Hz, die Aufnahme liegt bei 440 Hz
        instrument.schichten[0].holeFilter().art = FilterArt::Hochpass;
        const Klang duenn = rendereTon(instrument, 69, 1500.0, kRate);
        CHECK(spitze(duenn, 8820, 30000) < 0.03f);
        CHECK(spitze(duenn, 8820, 30000, 1) < 0.015f);  // rechts halb so laut, wie die Aufnahme
        CHECK(spitze(duenn, 8820, 30000, 1) > 0.0f);
        instrument.schichten[0].holeFilter().art = FilterArt::Bandpass;
        instrument.schichten[0].holeFilter().teilton = 1.0;
        CHECK(spitze(rendereTon(instrument, 69, 1500.0, kRate), 8820, 30000) == doctest::Approx(1.0f).epsilon(0.03));
    }

    // S123, Entscheide Patrik 1 a und 2 a: mitgelieferte Samples unter asset/loop/samples, im Loop `lib:<name>`.
    // Liest das Repo vom Arbeitsordner aus (die Test-Exe aus dem Projekt-Root starten).
    TEST_CASE("Loop Sample S123: die mitgelieferten Samples — lib:, die Liste, die Nennung in CREDITS.md, im Loop beim Namen")
    {
        // --- die Nennung
        CHECK(istBibliotheksKlang(QStringLiteral("lib:harp")));
        CHECK_FALSE(istBibliotheksKlang(QStringLiteral("kit:kick")));
        CHECK(istMitgelieferterKlang(QStringLiteral("lib:harp")));
        CHECK(istMitgelieferterKlang(QStringLiteral("kit:kick")));
        CHECK_FALSE(istMitgelieferterKlang(QStringLiteral("user:harp")));
        CHECK(sampleTitel(QStringLiteral("lib:thumb_piano")) == QStringLiteral("thumb_piano"));
        CHECK(klangDatei(QStringLiteral("lib:Harp")).isEmpty());   // nur a–z, 0–9, _
        CHECK(klangDatei(QStringLiteral("lib:../kit/kick")).isEmpty());
        CHECK(klangDatei(QStringLiteral("lib:")).isEmpty());

        // --- die Liste: was im Ordner liegt, jede Datei da und in CREDITS.md genannt
        REQUIRE_FALSE(bibliothekOrdner().isEmpty());
        const QStringList alle = bibliotheksKlaenge();
        // wer ein Sample dazulegt, traegt es in asset/loop/samples/make_samples.py ein und stellt die Zahl nach
        // 38 aus OLPC (S123), 37 aus DrumGizmo (S124, make_drumgizmo.py), S128 (C1 a) zwei Cowbells aus OLPC
        // und die Clave aus Wikimedia Commons (CC0)
        CHECK(alle.size() == 78);
        CHECK(std::is_sorted(alle.begin(), alle.end()));
        QFile credits(QDir(bibliothekOrdner()).filePath(QStringLiteral("CREDITS.md")));
        REQUIRE(credits.open(QIODevice::ReadOnly));
        const QString nennungen = QString::fromUtf8(credits.readAll());
        CHECK(nennungen.contains(QStringLiteral("Creative Commons Attribution 3.0")));
        CHECK(nennungen.contains(QStringLiteral("Creative Commons Attribution 4.0 International")));  // DrumGizmo
        CHECK(nennungen.contains(QStringLiteral("CC0 1.0")));  // die Clave
        for (const QString& klang : alle)
        {
            CAPTURE(klang.toStdString());
            CHECK(klang.startsWith(QStringLiteral("lib:")));
            CHECK(QFileInfo::exists(klangDatei(klang)));
            CHECK(sampleDatei(klang) == klangDatei(klang));
            CHECK(nennungen.contains(QStringLiteral("`") + klang.mid(4) + QStringLiteral(".wav`")));
        }
        CHECK(alle.contains(QStringLiteral("lib:voice_female_aah")));
        CHECK(alle.contains(QStringLiteral("lib:clap_707")));
        CHECK(alle.contains(QStringLiteral("lib:cowbell")));
        CHECK(alle.contains(QStringLiteral("lib:cowbell_tip")));
        CHECK(alle.contains(QStringLiteral("lib:clave")));

        // --- im Loop: eine Drum-Spur nennt das Sample beim Namen, nie als Pfad
        QJsonArray zeile;
        for (int i = 0; i < 64; ++i) zeile.append(i % 4 == 0 ? 1.0 : 0.0);
        QJsonObject spur{{QStringLiteral("name"), QStringLiteral("Snap")}, {QStringLiteral("sound"), QStringLiteral("lib:snap")},
                         {QStringLiteral("volume"), 1.0}, {QStringLiteral("pan"), 0.0}, {QStringLiteral("mute"), false}};
        QJsonObject muster{{QStringLiteral("name"), QStringLiteral("Beat")}, {QStringLiteral("kind"), QStringLiteral("drumkit")},
                           {QStringLiteral("bars"), 4}, {QStringLiteral("tracks"), QJsonArray{spur}},
                           {QStringLiteral("rows"), QJsonArray{zeile}}};
        QJsonObject datei{{QStringLiteral("format"), QStringLiteral("lumiviz-loop")}, {QStringLiteral("version"), 5},
                          {QStringLiteral("bpm"), 120.0}, {QStringLiteral("beatsPerBar"), 4}, {QStringLiteral("stepsPerBeat"), 4},
                          {QStringLiteral("patterns"), QJsonArray{muster}}, {QStringLiteral("channels"), QJsonArray{}}};
        const QString basis = QDir(bibliothekOrdner()).filePath(QStringLiteral(".."));  // auch unter dem Ordner des Loops kein Pfad
        Loop loop;
        QStringList bericht;
        REQUIRE(ausJson(datei, loop, basis, &bericht));
        CHECK(bericht.isEmpty());
        REQUIRE(loop.muster.size() == 1);
        CHECK(loop.muster[0].spuren[0].klang == QStringLiteral("lib:snap"));
        CHECK(alsJson(loop, basis).value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("tracks"))
                  .toArray().at(0).toObject().value(QStringLiteral("sound")).toString() == QStringLiteral("lib:snap"));
        // ein mitgeliefertes Sample, das es nicht gibt, steht im Bericht
        spur[QStringLiteral("sound")] = QStringLiteral("lib:gibt_es_nicht");
        muster[QStringLiteral("tracks")] = QJsonArray{spur};
        datei[QStringLiteral("patterns")] = QJsonArray{muster};
        REQUIRE(ausJson(datei, loop, basis, &bericht));
        CHECK(bericht == QStringList{QStringLiteral("Sample not found: lib:gibt_es_nicht")});
    }
}
