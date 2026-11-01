/**
 * @file test_LoopWaveKlang.cpp
 * @brief Loop, Stufe 15c: ein Wave-Muster klingt — Ausschnitt, Lage, Gain und Blenden, ohne
 *        Dehnen und Tonhoehe (docs/audio/Loop_Konzept.md §4p; Entscheide Patrik WV3 a, WV8 a)
 *
 * - Der Name `wave:<werte>|<datei>` beschreibt den Klang genau und wird genau zurueckgelesen.
 * - `rechneWave` gegen von Hand gerechnete Abtastwerte: Lage vor und hinter dem Anfang,
 *   Ende des Ausschnitts, Ende des Musters, Gain, beide Blenden.
 * - Der Rechenkern spielt den Klang je Wiederholung ab dem Anfang des Musters, mit Fader,
 *   Lautstaerke und Expression; `rechneClip` (Freeze, Export) ebenso.
 * - Der Tonauftrag dekodiert so viel der Datei, wie der Klang braucht, und legt ihn in den
 *   Tonspeicher; fehlt die Datei, steht das im Bericht.
 * - Der Abdruck des Clips (Freeze) folgt dem Inhalt der Datei, nicht ihrem Ort; die
 *   Herkunft geht nicht ein.
 *
 * Bei 1000 Hz, damit die Zahlen von Hand nachzurechnen sind: bei 120 BPM und vier Schritten
 * je Schlag ist ein Schritt 125 Abtastwerte lang. Die „Datei“ ist eine Rampe — links i/1000,
 * rechts −i/1000 an Abtastwert i. Ohne Ton, ohne Engine, ohne Qt-Anwendung.
 */

#include <doctest.h>

#include "loop/Bearbeiten.hpp"
#include "loop/Freeze.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Tonauftrag.hpp"
#include "loop/Tonspeicher.hpp"
#include "loop/WaveKlang.hpp"
#include "loop/Wellenform.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 1000;

/// Die „Datei“: `frames` Abtastwerte einer Rampe.
std::vector<float> rampe(int frames)
{
    std::vector<float> d(static_cast<std::size_t>(2 * frames));
    for (int i = 0; i < frames; ++i)
    {
        d[static_cast<std::size_t>(2 * i)] = static_cast<float>(i / 1000.0);
        d[static_cast<std::size_t>(2 * i + 1)] = static_cast<float>(-i / 1000.0);
    }
    return d;
}

/// Links an Abtastwert `k` eines Klangs; 0 dahinter.
double links(const Klang& k, std::int64_t frame)
{
    return frame < k.frames() ? k.daten[static_cast<std::size_t>(2 * frame)] : 0.0;
}

/// Ein Loop mit 120 BPM, einem Schlag je Takt und vier Schritten je Schlag (ein Takt = 500
/// Abtastwerte bei 1000 Hz); ein Wave-Muster von einem Takt, zwei Wiederholungen ab Takt 1.
Loop waveLoop(const QString& datei)
{
    Loop loop;
    loop.bpm = 120.0;
    loop.schlaegeJeTakt = 1;
    loop.schritteJeSchlag = 4;
    loop.laut = 0.5;
    Muster m;
    m.name = QStringLiteral("Voc");
    m.art = MusterArt::Wave;
    m.takte = 1;
    m.wave.datei = datei;
    loop.muster = {m};
    Kanal k{QStringLiteral("Vocals"), true, {{QStringLiteral("Voc"), 1, 2}}};
    k.gainDb = -6.0;
    loop.kanaele = {k};
    loop.richte();
    return loop;
}

/// Ein Dekodierer, der die Rampe liefert und mitschreibt, wie oft und wie weit er gerufen wurde.
struct Zaehler
{
    int mal = 0;
    double maxSekunden = 0.0;
    Dekodierer dekodierer()
    {
        return [this](const QString&, int, double sekunden) {
            ++mal;
            maxSekunden = sekunden;
            return rampe(200);
        };
    }
};

}  // namespace

TEST_SUITE("LoopWaveKlang")
{
    TEST_CASE("Loop Wave 15c: der Name beschreibt den Klang genau")
    {
        WaveKlang k;
        k.datei = QStringLiteral("C:/Meine Aufnahmen/take 1.wav");  // Doppelpunkt und Leerzeichen
        k.startS = 0.1;
        k.endeS = 2.0 / 3.0;
        k.versatzMs = -312.5;
        k.laengeMs = 16000.0;
        k.gainDb = -3.0;
        k.fadeInMs = 15.0;
        k.fadeOutMs = 120.0;
        const QString name = waveKlangName(k);
        CHECK(istWaveKlang(name));
        CHECK_FALSE(istTon(name));
        WaveKlang zurueck;
        REQUIRE(zerlegeWaveKlang(name, zurueck));
        CHECK(zurueck == k);  // Bit fuer Bit, auch 2/3
        CHECK_FALSE(zerlegeWaveKlang(QStringLiteral("wave:1,2,3|x.wav"), zurueck));
        CHECK_FALSE(zerlegeWaveKlang(QStringLiteral("wave:1,2,3,4,5,6,7"), zurueck));  // ohne Datei
        CHECK_FALSE(zerlegeWaveKlang(QStringLiteral("inst:piano:60:500"), zurueck));

        SUBCASE("aus dem Muster: Lage und Laenge beim Tempo und Raster des Loops")
        {
            Loop loop;  // 120 BPM, 4/4, vier Schritte je Schlag: ein Schritt 125 ms
            Muster m;
            m.name = QStringLiteral("Voc");
            m.art = MusterArt::Wave;
            m.takte = 8;
            m.wave.datei = QStringLiteral("user:take_1");
            m.wave.versatz = -2.5;
            m.wave.start = 1.25;
            loop.muster = {m};
            loop.richte();
            const WaveKlang w = waveKlangVon(loop, loop.muster[0]);
            CHECK(w.versatzMs == doctest::Approx(-312.5));
            CHECK(w.laengeMs == doctest::Approx(16000.0));
            CHECK(w.datei == QStringLiteral("user:take_1"));
            CHECK(loop.klangNamen() == QStringList{waveKlangName(w)});
            // was nicht klingt, aendert den Namen nicht; ein anderes Tempo schon
            Loop andere = loop;
            andere.muster[0].wave.bezugBpm = 90.0;
            andere.muster[0].wave.tonhoehenArt = TonhoehenArt::Stimme;
            CHECK(andere.klangNamen() == loop.klangNamen());
            andere.bpm = 100.0;
            CHECK(andere.klangNamen() != loop.klangNamen());
        }
    }

    TEST_CASE("Loop Wave 15f: die Spur Pitch — im Namen nur, wenn sie wirkt, und der Ton steigt")
    {
        Loop loop;  // 120 BPM, 4/4, vier Schritte je Schlag: ein Schritt 125 ms
        Muster m;
        m.name = QStringLiteral("Voc");
        m.art = MusterArt::Wave;
        m.takte = 1;
        m.wave.datei = QStringLiteral("user:take_1");
        loop.muster = {m};
        loop.richte();
        const QString ohne = waveKlangName(waveKlangVon(loop, loop.muster[0]));
        CHECK_FALSE(ohne.contains(QLatin1Char(';')));
        // Punkte, die nichts aendern: derselbe Name wie ohne Spur (Freeze-Abdruck und Tonspeicher bleiben)
        loop.muster[0].zeitspuren.von(Steuerspur::Pitch) = {{0.0, 0.0}, {8.0, 0.0}};
        loop.richte();
        CHECK(waveKlangName(waveKlangVon(loop, loop.muster[0])) == ohne);
        // ein Punkt, der wirkt: alle Punkte in ms im Namen, genau zurueckzulesen
        loop.muster[0].zeitspuren.von(Steuerspur::Pitch) = {{0.0, 0.0}, {8.0, 2.5}};
        loop.richte();
        const WaveKlang mit = waveKlangVon(loop, loop.muster[0]);
        CHECK(mit.tonhoehe == std::vector<SpurPunkt>{{0.0, 0.0}, {1000.0, 2.5}});
        const QString name = waveKlangName(mit);
        CHECK(name.contains(QStringLiteral(";0:0;1000:2.5|user:take_1")));
        WaveKlang zurueck;
        REQUIRE(zerlegeWaveKlang(name, zurueck));
        CHECK(zurueck == mit);
        CHECK_FALSE(zerlegeWaveKlang(QStringLiteral("wave:1,2,3,4,5,6,7;1000|x.wav"), zurueck));

        SUBCASE("rechneWave: ohne Spur unveraendert, mit +12 eine Oktave hoeher, gleich lang")
        {
            constexpr int rate = 48000;
            std::vector<float> datei(static_cast<std::size_t>(2 * rate));
            for (int i = 0; i < rate; ++i)
                datei[static_cast<std::size_t>(2 * i)] = datei[static_cast<std::size_t>(2 * i + 1)] =
                    static_cast<float>(0.5 * std::sin(2.0 * 3.14159265358979323846 * 440.0 * i / rate));
            WaveKlang k;
            k.laengeMs = 1000.0;
            const Klang roh = rechneWave(k, datei, rate);
            CHECK(roh.daten == datei);  // Gain 0 dB, keine Blenden: die Datei selbst
            k.tonhoehe = {{0.0, 12.0}};
            const Klang hoeher = rechneWave(k, datei, rate);
            REQUIRE(hoeher.frames() == roh.frames());
            // aufsteigende Nulldurchgaenge in der Mitte: doppelt so viele
            const auto durchgaenge = [](const Klang& klang) {
                int z = 0;
                for (std::int64_t i = 12000; i < 36000; ++i)
                    if (klang.daten[static_cast<std::size_t>(2 * (i - 1))] < 0.0f && klang.daten[static_cast<std::size_t>(2 * i)] >= 0.0f) ++z;
                return z;
            };
            CHECK(durchgaenge(roh) == doctest::Approx(220).epsilon(0.01));
            CHECK(durchgaenge(hoeher) == doctest::Approx(440).epsilon(0.01));
        }
    }

    TEST_CASE("Loop Wave 15c: rechneWave gegen von Hand gerechnete Abtastwerte")
    {
        const std::vector<float> datei = rampe(100);
        WaveKlang k;
        k.startS = 0.010;   // ab Abtastwert 10 der Datei
        k.laengeMs = 50.0;  // das Muster: 50 Abtastwerte

        SUBCASE("Lage hinter dem Anfang: Stille davor, dann die Datei ab dem Ausschnitt")
        {
            k.versatzMs = 5.0;  // d = 10 − 5 = 5, hoerbar ab 5
            const Klang w = rechneWave(k, datei, kRate);
            CHECK(w.frames() == 50);  // bis zum Ende des Musters
            CHECK(links(w, 4) == 0.0);
            CHECK(links(w, 5) == doctest::Approx(0.010));
            CHECK(links(w, 49) == doctest::Approx(0.054));
            CHECK(w.daten[2 * 49 + 1] == doctest::Approx(-0.054));
        }
        SUBCASE("ein Auftakt: hoerbar ab dem Musteranfang, mitten im Ausschnitt")
        {
            k.versatzMs = -20.0;  // d = 10 + 20 = 30
            const Klang w = rechneWave(k, datei, kRate);
            CHECK(w.frames() == 50);
            CHECK(links(w, 0) == doctest::Approx(0.030));
            CHECK(links(w, 49) == doctest::Approx(0.079));
        }
        SUBCASE("das Ende des Ausschnitts kommt vor dem Ende des Musters")
        {
            k.versatzMs = 5.0;
            k.endeS = 0.040;  // E = 40, b = 40 − 5 = 35
            const Klang w = rechneWave(k, datei, kRate);
            CHECK(w.frames() == 35);
            CHECK(links(w, 34) == doctest::Approx(0.039));
        }
        SUBCASE("die Datei endet vor dem Muster")
        {
            k.versatzMs = 0.0;
            k.laengeMs = 500.0;
            const Klang w = rechneWave(k, datei, kRate);
            CHECK(w.frames() == 90);  // 100 − 10
        }
        SUBCASE("Gain und beide Blenden, linear in Abtastwerten")
        {
            k.versatzMs = 5.0;  // a = 5, b = 50
            k.gainDb = -6.0;
            k.fadeInMs = 10.0;  // zehn Abtastwerte
            k.fadeOutMs = 4.0;  // vier
            const double g = std::pow(10.0, -6.0 / 20.0);
            const Klang w = rechneWave(k, datei, kRate);
            CHECK(links(w, 5) == 0.0);                                         // (5 − 5) / 10
            CHECK(links(w, 8) == doctest::Approx(0.013 * g * 0.3));             // (8 − 5) / 10
            CHECK(links(w, 20) == doctest::Approx(0.025 * g));                  // voll
            CHECK(links(w, 47) == doctest::Approx(0.052 * g * 0.75));           // (50 − 47) / 4
            CHECK(links(w, 49) == doctest::Approx(0.054 * g * 0.25));           // (50 − 49) / 4
            CHECK(waveGainFaktor(0.0) == 1.0);
            CHECK(waveGainFaktor(kWaveGainMinDb) == 0.0);
        }
        SUBCASE("nichts zu hoeren: leer")
        {
            k.startS = 0.5;  // hinter dem Ende der Datei
            CHECK(rechneWave(k, datei, kRate).frames() == 0);
            k.startS = 0.0;
            k.versatzMs = 60.0;  // hinter dem Ende des Musters
            CHECK(rechneWave(k, datei, kRate).frames() == 0);
        }
        SUBCASE("so viel der Datei wird gebraucht")
        {
            k.versatzMs = -20.0;
            CHECK(waveBedarfSekunden(k) == doctest::Approx(0.010 + 0.070 + 0.01));
            k.endeS = 0.040;
            CHECK(waveBedarfSekunden(k) == doctest::Approx(0.040 + 0.01));
        }
    }

    TEST_CASE("Loop Wave 15c: der Rechenkern spielt je Wiederholung ab dem Anfang des Musters")
    {
        Loop loop = waveLoop(QStringLiteral("/irgendwo/voc.wav"));
        const Ablauf ablauf = plane(loop, kRate);
        REQUIRE(ablauf.klangNamen.size() == 1);
        REQUIRE(ablauf.stimmen.size() == 1);
        REQUIRE(ablauf.stimmen[0].schlaege.size() == 2);
        CHECK(ablauf.stimmen[0].schlaege[0].frame == 500);
        CHECK(ablauf.stimmen[0].schlaege[1].frame == 1000);
        CHECK(ablauf.durchlaufFrames == 1500);

        WaveKlang w;
        REQUIRE(zerlegeWaveKlang(ablauf.klangNamen[0], w));
        const auto klang = std::make_shared<const Klang>(rechneWave(w, rampe(200), kRate));
        REQUIRE(klang->frames() == 200);  // die Datei endet vor dem Muster (500)
        const double faktor = std::pow(10.0, -6.0 / 20.0) * 0.5;  // Fader und Lautstaerke des Loops

        std::vector<float> aus(2 * 1500);
        rechne(ablauf, {klang}, 0, 1500, aus.data());
        CHECK(aus[2 * 499] == 0.0f);
        CHECK(aus[2 * 500] == 0.0f);  // die Rampe beginnt bei 0
        CHECK(aus[2 * 600] == doctest::Approx(0.100 * faktor));
        CHECK(aus[2 * 699] == doctest::Approx(0.199 * faktor));
        CHECK(aus[2 * 700] == 0.0f);
        CHECK(aus[2 * 1100] == doctest::Approx(0.100 * faktor));  // die zweite Wiederholung beginnt neu
        CHECK(aus[2 * 1100 + 1] == doctest::Approx(-0.100 * faktor));

        SUBCASE("in Bloecken gerechnet: dieselben Werte")
        {
            std::vector<float> stueck(2 * 77);
            rechne(ablauf, {klang}, 1050, 77, stueck.data());
            for (int i = 0; i < 77; ++i)
            {
                CAPTURE(i);
                CHECK(stueck[static_cast<std::size_t>(2 * i)] == aus[static_cast<std::size_t>(2 * (1050 + i))]);
            }
        }
        SUBCASE("Expression des Musters und am Clip: der Pegel e²")
        {
            loop.muster[0].zeitspuren.von(Steuerspur::Expression) = {{0.0, 0.5}};
            loop.kanaele[0].clips[0].expression = {{0.0, 0.8}};
            loop.richte();
            const Ablauf mit = plane(loop, kRate);
            std::vector<float> leiser(2 * 1500);
            rechne(mit, {klang}, 0, 1500, leiser.data());
            CHECK(leiser[2 * 600] == doctest::Approx(0.100 * faktor * 0.4 * 0.4));
        }
        SUBCASE("ein Kanal aus, ein Muster ohne Datei: keine Stimme")
        {
            Loop aus1 = loop;
            aus1.kanaele[0].an = false;
            CHECK(plane(aus1, kRate).stimmen.empty());
            Loop ohne = loop;
            ohne.muster[0].wave.datei.clear();
            CHECK(plane(ohne, kRate).stimmen.empty());
        }
        SUBCASE("rechneClip (Freeze, Export): der Clip allein, Fader 0 dB, Lautstaerke 1")
        {
            const std::optional<Loop> fassung = clipFassung(loop, ClipOrt{0, 1});
            REQUIRE(fassung.has_value());
            REQUIRE(fassung->klangNamen() == ablauf.klangNamen);
            const std::optional<Wellenform> welle =
                rechneClip(loop, ClipOrt{0, 1}, kRate, std::make_shared<const Klaenge>(Klaenge{klang}), {});
            REQUIRE(welle.has_value());
            CHECK(welle->clipFrames == 1000);  // zwei Wiederholungen
            CHECK(links(welle->klang, 100) == doctest::Approx(0.100));
            CHECK(links(welle->klang, 600) == doctest::Approx(0.100));
            CHECK(links(welle->klang, 300) == 0.0);
        }
    }

    TEST_CASE("Loop Wave 15c: der Tonauftrag dekodiert, schneidet und merkt")
    {
        tonspeicher().leere();
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString datei = QDir(ordner.path()).filePath(QStringLiteral("voc.wav"));
        {
            QFile f(datei);
            REQUIRE(f.open(QIODevice::WriteOnly));  // gelesen wird ueber den Dekodierer
            f.write("x");
        }
        const Loop loop = waveLoop(datei);
        const QStringList namen = loop.klangNamen();
        REQUIRE(namen.size() == 1);
        WaveKlang w;
        REQUIRE(zerlegeWaveKlang(namen[0], w));

        Zaehler zaehler;
        {
            Tonauftrag auftrag(namen, kRate, zaehler.dekodierer());
            CHECK_FALSE(auftrag.ausSpeicher());
            auftrag.rechneHier();
            CHECK(auftrag.bericht().isEmpty());
            const Klaenge klaenge = auftrag.klaenge();
            REQUIRE(klaenge.size() == 1);
            REQUIRE(klaenge[0] != nullptr);
            CHECK(klaenge[0]->daten == rechneWave(w, rampe(200), kRate).daten);
        }
        CHECK(zaehler.mal == 1);
        CHECK(zaehler.maxSekunden == doctest::Approx(waveBedarfSekunden(w)));
        {
            Tonauftrag nochmal(namen, kRate, zaehler.dekodierer());
            CHECK(nochmal.ausSpeicher());  // aus dem Tonspeicher
        }
        CHECK(zaehler.mal == 1);

        SUBCASE("eine Datei, die es nicht gibt: still, mit Zeile im Bericht")
        {
            const QString fehlt = QDir(ordner.path()).filePath(QStringLiteral("weg.wav"));
            Tonauftrag auftrag(waveLoop(fehlt).klangNamen(), kRate, [](const QString&, int, double) {
                return std::vector<float>{};
            });
            auftrag.rechneHier();
            CHECK(auftrag.klaenge()[0] == nullptr);
            CHECK(auftrag.bericht() == QStringList{QStringLiteral("Sound not loaded: %1").arg(QDir::toNativeSeparators(fehlt))});
        }
        tonspeicher().leere();
    }

    TEST_CASE("Loop Wave 15c: der Abdruck des Clips folgt dem Inhalt der Datei")
    {
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const auto schreibe = [&](const QString& name, const QByteArray& inhalt) {
            const QString pfad = QDir(ordner.path()).filePath(name);
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write(inhalt);
            return pfad;
        };
        const QString a = schreibe(QStringLiteral("a.wav"), "Aufnahme A");
        const Loop loop = waveLoop(a);
        const ClipOrt ort{0, 1};
        const QByteArray vorher = clipAbdruck(loop, ort, kRate, {});
        REQUIRE_FALSE(vorher.isEmpty());

        // dieselbe Aufnahme an einem anderen Ort: derselbe Abdruck
        const QString kopie = schreibe(QStringLiteral("kopie.wav"), "Aufnahme A");
        CHECK(clipAbdruck(waveLoop(kopie), ort, kRate, {}) == vorher);
        // eine andere Aufnahme: ein anderer
        const QString b = schreibe(QStringLiteral("b.wav"), "Aufnahme B");
        CHECK(clipAbdruck(waveLoop(b), ort, kRate, {}) != vorher);
        // was klingt, aendert ihn; die Herkunft nicht
        Loop lauter = loop;
        lauter.muster[0].wave.gainDb = 3.0;
        CHECK(clipAbdruck(lauter, ort, kRate, {}) != vorher);
        Loop mitHerkunft = loop;
        mitHerkunft.muster[0].wave.herkunft = Herkunft(waveLoop(b));
        CHECK(clipAbdruck(mitHerkunft, ort, kRate, {}) == vorher);
    }
}
