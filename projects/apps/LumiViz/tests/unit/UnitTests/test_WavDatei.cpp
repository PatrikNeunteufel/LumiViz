/**
 ****************************************************************************************
 * @file   test_WavDatei.cpp
 * @brief  Eine WAV-Datei schreiben und lesen: 32-bit float, 24 und 16 Bit
 *         (docs/audio/Spielweisen_Konzept.md §4.4, Stufe 30b; Entscheid WX2 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * In Wegwerf-Ordnern. Geprueft: der Kopf jeder Art Byte fuer Byte, das
 * Zuruecklesen (32-bit float genau, 24 und 16 Bit auf ein halbes LSB), die
 * Kappung ueber ±1 mit ihrer Zahl, der Dither bei 16 Bit (fest: dieselben
 * Werte ergeben dieselbe Datei; ein Ton unter einem LSB bleibt hoerbar), die
 * Grenzen (nichts zu schreiben, falscher Ort — dann entsteht keine Datei), eine
 * Mono-Datei, und: die BassEngine liest alle drei Arten, wenn es ein
 * Ausgabegeraet gibt (sonst meldet der letzte Fall das und prueft nichts).
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/BassEngine.hpp"
#include "audio/WavDatei.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtEndian>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>
#include <string>
#include <vector>

using namespace lumi::audio;

namespace
{

constexpr int kRate = 48000;

/// Eine halbe Sekunde: links ein Sinus 440 Hz, rechts einer mit 660 Hz, beide 0,8; dazu
/// am Anfang Werte, die genau auf den Grenzen liegen.
std::vector<float> probe()
{
    std::vector<float> werte = {0.0f, 1.0f, -1.0f, 0.5f, 1e-6f, -1e-6f};
    for (int i = 0; i < kRate / 2; ++i)
    {
        const double t = static_cast<double>(i) / kRate;
        werte.push_back(static_cast<float>(0.8 * std::sin(2.0 * std::numbers::pi * 440.0 * t)));
        werte.push_back(static_cast<float>(0.8 * std::sin(2.0 * std::numbers::pi * 660.0 * t)));
    }
    return werte;
}

QByteArray inhalt(const QString& pfad)
{
    QFile datei(pfad);
    return datei.open(QIODevice::ReadOnly) ? datei.readAll() : QByteArray{};
}

template <typename T>
T zahl(const QByteArray& roh, qsizetype stelle)
{
    T wert;
    std::memcpy(&wert, roh.constData() + stelle, sizeof(T));
    return qFromLittleEndian(wert);
}

double groessteAbweichung(const std::vector<float>& a, const std::vector<float>& b)
{
    double abweichung = 0.0;
    for (std::size_t i = 0; i < std::min(a.size(), b.size()); ++i)
        abweichung = std::max(abweichung, std::abs(static_cast<double>(a[i]) - b[i]));
    return abweichung;
}

}  // namespace

TEST_SUITE("WavDatei")
{
    TEST_CASE("WavDatei 30b: drei Arten — der Kopf, und was zurueckkommt")
    {
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const std::vector<float> werte = probe();
        const auto frames = static_cast<qsizetype>(werte.size() / 2);

        SUBCASE("32-bit float: Format 3 mit fact, jeder Wert genau")
        {
            const QString pfad = QDir(tmp.path()).filePath(QStringLiteral("f32.wav"));
            const WavErgebnis e = schreibeWav(pfad, werte, kRate, WavTiefe::Gleitkomma32);
            REQUIRE(e.geschrieben);
            CHECK(e.gekappt == 0);
            CHECK(e.spitze == 1.0f);
            const QByteArray roh = inhalt(pfad);
            REQUIRE(roh.size() == 58 + frames * 8);
            CHECK(roh.left(4) == "RIFF");
            CHECK(zahl<quint32>(roh, 4) == roh.size() - 8);
            CHECK(roh.mid(8, 8) == "WAVEfmt ");
            CHECK(zahl<quint32>(roh, 16) == 18);
            CHECK(zahl<quint16>(roh, 20) == 3);
            CHECK(zahl<quint16>(roh, 22) == 2);
            CHECK(zahl<quint32>(roh, 24) == kRate);
            CHECK(zahl<quint32>(roh, 28) == kRate * 8);
            CHECK(zahl<quint16>(roh, 32) == 8);
            CHECK(zahl<quint16>(roh, 34) == 32);
            CHECK(roh.mid(38, 4) == "fact");
            CHECK(zahl<quint32>(roh, 46) == frames);
            CHECK(roh.mid(50, 4) == "data");
            CHECK(zahl<quint32>(roh, 54) == frames * 8);
            const std::optional<WavInhalt> gelesen = liesWav(pfad);
            REQUIRE(gelesen.has_value());
            CHECK(gelesen->rate == kRate);
            CHECK(gelesen->tiefe == WavTiefe::Gleitkomma32);
            CHECK(gelesen->stereo == werte);
        }
        SUBCASE("24 Bit: PCM, drei Bytes, auf ein halbes LSB")
        {
            const QString pfad = QDir(tmp.path()).filePath(QStringLiteral("p24.wav"));
            REQUIRE(schreibeWav(pfad, werte, kRate, WavTiefe::Pcm24).geschrieben);
            const QByteArray roh = inhalt(pfad);
            REQUIRE(roh.size() == 44 + frames * 6);
            CHECK(zahl<quint16>(roh, 20) == 1);
            CHECK(zahl<quint16>(roh, 32) == 6);
            CHECK(zahl<quint16>(roh, 34) == 24);
            // 1 und -1 als groesster und kleinster Wert, 0,5 gerundet
            CHECK((static_cast<qint32>(zahl<quint32>(roh, 44 + 3) << 8) >> 8) == 8388607);
            const std::optional<WavInhalt> gelesen = liesWav(pfad);
            REQUIRE(gelesen.has_value());
            CHECK(gelesen->tiefe == WavTiefe::Pcm24);
            REQUIRE(gelesen->stereo.size() == werte.size());
            CHECK(gelesen->stereo[1] == 1.0f);
            CHECK(gelesen->stereo[2] == -1.0f);
            CHECK(groessteAbweichung(gelesen->stereo, werte) <= 0.5 / 8388607.0 + 1e-9);
        }
        SUBCASE("16 Bit ohne Dither: PCM, gerundet, auf ein halbes LSB — wie der Schreiber der Aufnahme seit 11b")
        {
            const QString pfad = QDir(tmp.path()).filePath(QStringLiteral("p16.wav"));
            REQUIRE(schreibeWav(pfad, werte, kRate, WavTiefe::Pcm16).geschrieben);
            const QByteArray roh = inhalt(pfad);
            REQUIRE(roh.size() == 44 + frames * 4);
            CHECK(zahl<quint16>(roh, 34) == 16);
            for (qsizetype i = 0; i < 2 * frames; ++i)
            {
                const double wert = std::clamp(static_cast<double>(werte[static_cast<std::size_t>(i)]), -1.0, 1.0);
                if (zahl<qint16>(roh, 44 + 2 * i) != static_cast<qint16>(std::lround(wert * 32767.0)))
                {
                    FAIL("Wert " << i << " anders als lround(wert * 32767)");
                    break;
                }
            }
            const std::optional<WavInhalt> gelesen = liesWav(pfad);
            REQUIRE(gelesen.has_value());
            CHECK(gelesen->tiefe == WavTiefe::Pcm16);
            CHECK(groessteAbweichung(gelesen->stereo, werte) <= 0.5 / 32767.0 + 1e-9);
        }
    }

    TEST_CASE("WavDatei 30b: ueber 0 dB — 32-bit float behaelt, 24 und 16 Bit kappen und zaehlen")
    {
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const std::vector<float> werte = {0.25f, 1.5f, -2.0f, 0.9f, 1.0f, -1.0001f};
        const QString f32 = QDir(tmp.path()).filePath(QStringLiteral("f32.wav"));
        const WavErgebnis frei = schreibeWav(f32, werte, kRate, WavTiefe::Gleitkomma32);
        REQUIRE(frei.geschrieben);
        CHECK(frei.gekappt == 0);
        CHECK(frei.spitze == 2.0f);
        CHECK(liesWav(f32)->stereo == werte);

        for (const WavTiefe tiefe : {WavTiefe::Pcm24, WavTiefe::Pcm16})
        {
            const QString pfad = QDir(tmp.path()).filePath(QStringLiteral("pcm.wav"));
            const WavErgebnis e = schreibeWav(pfad, werte, kRate, tiefe);
            REQUIRE(e.geschrieben);
            CHECK(e.gekappt == 3);  // 1,5, -2 und -1,0001; genau 1 ist nicht gekappt
            CHECK(e.spitze == 2.0f);
            const std::vector<float> gelesen = liesWav(pfad)->stereo;
            CHECK(gelesen[1] == 1.0f);
            CHECK(gelesen[2] == -1.0f);
        }
    }

    TEST_CASE("WavDatei 30b: Dither bei 16 Bit — fest, hoechstens anderthalb LSB, ein leiser Ton bleibt")
    {
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        // ein Sinus mit 0,4 LSB: ohne Dither wird er ganz zu null
        std::vector<float> leise;
        for (int i = 0; i < kRate; ++i)
        {
            const auto w = static_cast<float>(0.4 / 32767.0 * std::sin(2.0 * std::numbers::pi * 100.0 * i / kRate));
            leise.push_back(w);
            leise.push_back(w);
        }
        const QString ohne = QDir(tmp.path()).filePath(QStringLiteral("ohne.wav"));
        const QString mit1 = QDir(tmp.path()).filePath(QStringLiteral("mit1.wav"));
        const QString mit2 = QDir(tmp.path()).filePath(QStringLiteral("mit2.wav"));
        REQUIRE(schreibeWav(ohne, leise, kRate, WavTiefe::Pcm16).geschrieben);
        REQUIRE(schreibeWav(mit1, leise, kRate, WavTiefe::Pcm16, true).geschrieben);
        REQUIRE(schreibeWav(mit2, leise, kRate, WavTiefe::Pcm16, true).geschrieben);
        CHECK(inhalt(mit1) == inhalt(mit2));  // derselbe Startwert: dieselbe Datei

        const std::vector<float> nichts = liesWav(ohne)->stereo;
        CHECK(std::all_of(nichts.begin(), nichts.end(), [](float w) { return w == 0.0f; }));
        const std::vector<float> gedithert = liesWav(mit1)->stereo;
        CHECK(groessteAbweichung(gedithert, leise) <= 1.5 / 32767.0 + 1e-9);
        // der Ton steckt im Rauschen: das Mittel aus Ton mal gelesen ist deutlich ueber null
        double korrelation = 0.0;
        double energie = 0.0;
        for (std::size_t i = 0; i < leise.size(); ++i)
        {
            korrelation += static_cast<double>(leise[i]) * gedithert[i];
            energie += static_cast<double>(leise[i]) * leise[i];
        }
        MESSAGE("Dither: Ton mal gelesen / Energie des Tons = " << korrelation / energie);
        CHECK(korrelation / energie == doctest::Approx(1.0).epsilon(0.15));

        // 24 Bit und 32-bit float kennen keinen Dither — der Schalter aendert dort nichts
        const QString a = QDir(tmp.path()).filePath(QStringLiteral("a.wav"));
        const QString b = QDir(tmp.path()).filePath(QStringLiteral("b.wav"));
        REQUIRE(schreibeWav(a, leise, kRate, WavTiefe::Pcm24).geschrieben);
        REQUIRE(schreibeWav(b, leise, kRate, WavTiefe::Pcm24, true).geschrieben);
        CHECK(inhalt(a) == inhalt(b));
    }

    TEST_CASE("WavDatei 30b: Grenzen — nichts zu schreiben, falscher Ort, Mono lesen")
    {
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const QString pfad = QDir(tmp.path()).filePath(QStringLiteral("x.wav"));
        CHECK_FALSE(schreibeWav(pfad, std::vector<float>{}, kRate, WavTiefe::Pcm16).geschrieben);
        CHECK_FALSE(schreibeWav(pfad, std::vector<float>{0.1f}, kRate, WavTiefe::Pcm16).geschrieben);  // kein ganzer Frame
        CHECK_FALSE(schreibeWav(pfad, std::vector<float>{0.1f, 0.1f}, 0, WavTiefe::Pcm16).geschrieben);
        CHECK_FALSE(QFileInfo::exists(pfad));
        CHECK_FALSE(schreibeWav(QDir(tmp.path()).filePath(QStringLiteral("gibt/es/nicht/x.wav")),
                                std::vector<float>{0.1f, 0.1f}, kRate, WavTiefe::Gleitkomma32)
                        .geschrieben);
        CHECK_FALSE(liesWav(QDir(tmp.path()).filePath(QStringLiteral("fehlt.wav"))).has_value());

        // eine Mono-Datei mit 16 Bit, von Hand: der Wert auf beide Seiten
        QByteArray mono;
        const auto haenge = [&mono](auto wert) {
            const auto klein = qToLittleEndian(wert);
            mono.append(reinterpret_cast<const char*>(&klein), sizeof(klein));
        };
        mono.append("RIFF", 4);
        haenge(quint32{36 + 4});
        mono.append("WAVEfmt ", 8);
        haenge(quint32{16});
        haenge(quint16{1});
        haenge(quint16{1});
        haenge(quint32{22050});
        haenge(quint32{44100});
        haenge(quint16{2});
        haenge(quint16{16});
        mono.append("data", 4);
        haenge(quint32{4});
        haenge(qint16{16384});
        haenge(qint16{-32767});
        QFile datei(pfad);
        REQUIRE(datei.open(QIODevice::WriteOnly));
        REQUIRE(datei.write(mono) == mono.size());
        datei.close();
        const std::optional<WavInhalt> gelesen = liesWav(pfad);
        REQUIRE(gelesen.has_value());
        CHECK(gelesen->rate == 22050);
        REQUIRE(gelesen->stereo.size() == 4);
        CHECK(gelesen->stereo[0] == gelesen->stereo[1]);
        CHECK(gelesen->stereo[0] == doctest::Approx(16384.0 / 32767.0));
        CHECK(gelesen->stereo[3] == -1.0f);
    }

    TEST_CASE("WavDatei 30b: die BassEngine liest alle drei Arten (nur mit Ausgabegeraet)")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QCoreApplication app{argc, argv};
        BassEngine engine;
        if (!engine.initialize())
        {
            MESSAGE("kein Ausgabegeraet: das Wiederlesen durch BASS ist nicht geprueft (" << engine.getLastError().toStdString() << ")");
            return;
        }
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const std::vector<float> werte = probe();
        const struct
        {
            WavTiefe tiefe;
            const char* name;
            double grenze;
        } arten[] = {// BASS liest PCM mit 1/32768 (1/8388608), geschrieben wird mit 32767 (8388607) wie seit
                     // 11b: dazu ein halbes LSB Rundung — zusammen hoechstens anderthalb LSB
                     {WavTiefe::Gleitkomma32, "f32.wav", 1e-7},
                     {WavTiefe::Pcm24, "p24.wav", 1.5 / 8388607.0},
                     {WavTiefe::Pcm16, "p16.wav", 1.5 / 32767.0}};
        for (const auto& art : arten)
        {
            const std::string name = art.name;
            CAPTURE(name);
            const QString pfad = QDir(tmp.path()).filePath(QString::fromLatin1(art.name));
            REQUIRE(schreibeWav(pfad, werte, kRate, art.tiefe).geschrieben);
            const std::vector<float> gelesen = engine.decodeStereo(pfad, kRate, 30.0);
            REQUIRE(gelesen.size() == werte.size());
            const double abweichung = groessteAbweichung(gelesen, werte);
            MESSAGE(name << " durch die BassEngine: groesste Abweichung " << abweichung);
            CHECK(abweichung <= art.grenze);
        }
    }
}
