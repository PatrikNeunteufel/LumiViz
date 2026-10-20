/**
 ****************************************************************************************
 * @file   test_LoopPegel.cpp
 * @brief  Loop, Stufe 27b: die Standardprobe und die Messung der Instrumente und
 *         Drum-Klaenge nach EBU R128 (docs/audio/Loop_Konzept.md §4n, LL2 a, LL4 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Audio-Engine; die Instrumente aus dem Repo (die
 * Test-Exe aus dem Projekt-Root starten).
 *
 * - Die Standardprobe: Aufbau, Grundton je Art, Laenge.
 * - Die Messung folgt der Lautstaerke des Instruments (halbe Lautstaerke = 6,02 dB
 *   leiser), auch an einem Saiteninstrument.
 * - Messung (meldet nur Zahlen, prueft nichts ausser, dass gemessen wurde): jedes
 *   mitgelieferte Instrument mit der Standardprobe (Short-term-Maximum), die Drums
 *   mit einem Schlag (Momentary-Maximum), dazu je mitgeliefertem Kit jede Rolle mit
 *   Volume und ihr Abstand zur Kick. `LUMIVIZ_PEGEL_DATEI=<datei.tsv>` legt die Werte
 *   fuer die Skripte von 27c und 27d ab.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "audio/Lautheit.hpp"
#include "loop/DrumKit.hpp"
#include "loop/Instrument.hpp"
#include "loop/Pegel.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct PegelApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

double median(std::vector<double> werte)
{
    if (werte.empty()) return 0.0;
    std::sort(werte.begin(), werte.end());
    const std::size_t mitte = werte.size() / 2;
    return werte.size() % 2 == 1 ? werte[mitte] : (werte[mitte - 1] + werte[mitte]) / 2.0;
}

/// Ein Klang des Kits als Schlag: das Instrument auf seiner Note und Laenge,
/// sonst die WAV-Datei; mit dem Volume der Rolle. `kLufsStille`, wenn er fehlt.
double schlagDesKits(const KitKlang& k)
{
    Klang klang;
    if (!k.instrument.isEmpty())
    {
        Instrument instrument;
        if (!holeInstrument(k.instrument, instrument)) return lumi::audio::kLufsStille;
        looptest::merkeSamplesOhneEngine(instrument);
        klang = rendereTon(instrument, k.hoehe, k.dauerMs, kRate);
    }
    else
    {
        const QString wav = looptest::mitgelieferteWav(k.klang);
        if (wav.isEmpty()) return lumi::audio::kLufsStille;
        klang = looptest::liesWav(wav);
    }
    const lumi::audio::Lautheit l = messeSchlag(klang, kRate);
    if (l.momentanMaxLufs <= lumi::audio::kLufsStille || k.laut <= 0.0) return lumi::audio::kLufsStille;
    return l.momentanMaxLufs + 20.0 * std::log10(k.laut);
}

}  // namespace

TEST_SUITE("LoopPegel")
{
    TEST_CASE("Loop Pegel: die Standardprobe — Tonleiter und Akkord, Grundton je Art, vier Takte")
    {
        const Loop probe = standardProbe(QStringLiteral("piano"), QStringLiteral("keys"));
        REQUIRE(probe.muster.size() == 1);
        const Muster& m = probe.muster.front();
        CHECK(m.instrument == QStringLiteral("piano"));
        CHECK(m.takte == 4);
        CHECK(m.noten.size() == 19);
        CHECK(std::min_element(m.noten.begin(), m.noten.end(), [](const Note& a, const Note& b) {
                  return a.hoehe < b.hoehe;
              })->hoehe == 60);
        CHECK(probe.laut == 1.0);
        CHECK(probe.durchlaufMs() == doctest::Approx(8000.0));
        // der Akkord: vier Noten am selben Schritt, einen Takt lang
        int imAkkord = 0;
        for (const Note& n : m.noten)
            if (n.laenge == 16) ++imAkkord;
        CHECK(imAkkord == 4);

        const Loop bass = standardProbe(QStringLiteral("bass"), QStringLiteral("bass"));
        CHECK(std::min_element(bass.muster.front().noten.begin(), bass.muster.front().noten.end(),
                               [](const Note& a, const Note& b) { return a.hoehe < b.hoehe; })
                  ->hoehe == 36);
    }

    TEST_CASE("Loop Pegel: die Messung folgt der Lautstaerke des Instruments, auch mit Saiten")
    {
        const PegelApp app;
        for (const char* name : {"organ", "clean_guitar"})
        {
            CAPTURE(name);
            Instrument instrument;
            REQUIRE(holeInstrument(QString::fromLatin1(name), instrument));
            instrument.laut = 1.0;
            const lumi::audio::Lautheit voll = messeInstrument(instrument, kRate);
            instrument.laut = 0.5;
            const lumi::audio::Lautheit halb = messeInstrument(instrument, kRate);
            REQUIRE(voll.kurzzeitMaxLufs > -70.0);
            MESSAGE(name << ": " << voll.kurzzeitMaxLufs << " LUFS bei Volume 1, " << halb.kurzzeitMaxLufs << " bei 0,5");
            CHECK(std::abs(voll.kurzzeitMaxLufs - halb.kurzzeitMaxLufs - 20.0 * std::log10(2.0)) <= 0.05);
        }
        // eine Probe ist acht Sekunden, Stereo
        Instrument organ;
        REQUIRE(holeInstrument(QStringLiteral("organ"), organ));
        CHECK(rechneStandardprobe(organ, kRate).size() == static_cast<std::size_t>(8 * kRate * 2));
    }

    // Stufe 27c (LL2 a): die mitgelieferten Instrumente stehen auf dem Bezug. Wer eines im Klang
    // aendert, misst neu und laesst asset/loop/instrument_pegel.py --messung laufen.
    TEST_CASE("Loop Pegel 27c: jedes mitgelieferte Instrument ausser den Drums liegt auf dem Bezug (±1 LU)")
    {
        const PegelApp app;
        // derselbe Bezug im Programm und im Skript
        QFile skript(looptest::repoWurzel().filePath(QStringLiteral("asset/loop/instrument_pegel.py")));
        REQUIRE(skript.open(QIODevice::ReadOnly));
        const QRegularExpressionMatch treffer =
            QRegularExpression(QStringLiteral("\\nBEZUG_LUFS = (-?[0-9.]+)\\n")).match(QString::fromUtf8(skript.readAll()));
        REQUIRE(treffer.hasMatch());
        CHECK(treffer.captured(1).toDouble() == kInstrumentBezugLufs);

        int geprueft = 0;
        int ueberEins = 0;
        for (const Instrument& instrument : instrumente())
        {
            if (instrument.art == QLatin1String("drums")) continue;
            CAPTURE(instrument.name.toStdString());
            looptest::merkeSamplesOhneEngine(instrument);
            const double wert = messeInstrument(instrument, kRate).kurzzeitMaxLufs;
            CHECK(std::abs(wert - kInstrumentBezugLufs) <= 1.0);
            ++geprueft;
            if (instrument.laut > 1.0) ++ueberEins;
        }
        MESSAGE(geprueft << " Instrumente auf dem Bezug, davon " << ueberEins << " mit Volume ueber 1");
        CHECK(geprueft > 80);
    }

    TEST_CASE("Loop Pegel: Messung aller mitgelieferten Instrumente und Kits (meldet nur Zahlen)")
    {
        const PegelApp app;
        struct Zeile
        {
            QString name;
            QString art;
            double wert;  // Short-term-Maximum, bei Drums Momentary-Maximum
            double laut;
        };
        std::vector<Zeile> zeilen;
        for (const Instrument& instrument : instrumente())
        {
            looptest::merkeSamplesOhneEngine(instrument);
            const bool drum = instrument.art == QLatin1String("drums");
            const lumi::audio::Lautheit l = drum ? messeDrumInstrument(instrument, kRate) : messeInstrument(instrument, kRate);
            zeilen.push_back({instrument.name, instrument.art, drum ? l.momentanMaxLufs : l.kurzzeitMaxLufs, instrument.laut});
        }
        std::sort(zeilen.begin(), zeilen.end(), [](const Zeile& a, const Zeile& b) { return a.wert < b.wert; });
        std::vector<double> ohneDrums;
        for (const Zeile& z : zeilen)
        {
            MESSAGE(z.name.toStdString() << " (" << z.art.toStdString() << ", Volume " << z.laut << "): " << z.wert
                                         << (z.art == QLatin1String("drums") ? " LUFS M max" : " LUFS S max"));
            if (z.art != QLatin1String("drums")) ohneDrums.push_back(z.wert);
        }
        REQUIRE_FALSE(ohneDrums.empty());
        std::vector<double> sortiert = ohneDrums;
        std::sort(sortiert.begin(), sortiert.end());
        MESSAGE("ohne Drums: " << ohneDrums.size() << " Instrumente, " << sortiert.front() << " bis " << sortiert.back()
                               << " LUFS, Median " << median(ohneDrums));
        CHECK(zeilen.size() > 100);

        // die Kits: je Rolle der Schlag mit Volume und der Abstand zur Kick
        struct KitZeile
        {
            QString kit;
            QString rolle;
            QString klang;
            double wert;
        };
        std::vector<KitZeile> kitZeilen;
        std::map<QString, std::vector<double>> abstandJeArt;
        for (const DrumKit& kit : mitgelieferteKits())
        {
            double kick = lumi::audio::kLufsStille;
            for (const KitKlang& k : kit.klaenge)
                if (k.rolle.startsWith(QLatin1String("kick")))
                {
                    kick = schlagDesKits(k);
                    break;
                }
            for (const KitKlang& k : kit.klaenge)
            {
                const double wert = schlagDesKits(k);
                kitZeilen.push_back({kit.name, k.rolle, k.instrument.isEmpty() ? k.klang : k.instrument, wert});
                const QString art = QString(k.rolle).remove(QRegularExpression(QStringLiteral(" \\d+$")));
                MESSAGE("Kit " << kit.name.toStdString() << ", " << k.rolle.toStdString() << " ("
                               << (k.instrument.isEmpty() ? k.klang : k.instrument).toStdString() << ", Volume " << k.laut
                               << "): " << wert << " LUFS M max, Abstand zur Kick " << (wert - kick));
                if (wert > lumi::audio::kLufsStille && kick > lumi::audio::kLufsStille) abstandJeArt[art].push_back(wert - kick);
            }
        }
        for (const auto& [art, abstaende] : abstandJeArt)
            MESSAGE("Abstand zur Kick, " << art.toStdString() << ": Median " << median(abstaende) << " LU ueber "
                                         << abstaende.size() << " Kits");
        CHECK_FALSE(kitZeilen.empty());

        if (const QString datei = qEnvironmentVariable("LUMIVIZ_PEGEL_DATEI"); !datei.isEmpty())
        {
            QFile f(datei);
            REQUIRE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            QTextStream aus(&f);
            aus << "# Loop Pegel (test_LoopPegel.cpp, Stufe 27b): Art <Tab> Name <Tab> Wert LUFS <Tab> Volume bzw. Rolle\n";
            for (const Zeile& z : zeilen)
                aus << "instrument\t" << z.name << '\t' << QString::number(z.wert, 'f', 3) << '\t' << z.laut << '\t' << z.art << '\n';
            for (const KitZeile& k : kitZeilen)
                aus << "kit\t" << k.kit << '\t' << QString::number(k.wert, 'f', 3) << '\t' << k.rolle << '\t' << k.klang << '\n';
            MESSAGE("abgelegt: " << datei.toStdString());
        }
    }
}
