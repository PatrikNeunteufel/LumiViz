/**
 ****************************************************************************************
 * @file   test_LoopWerte.cpp
 * @brief  Loop: die gerechneten Werte jedes mitgelieferten Instruments und
 *         jedes Beispiel-Loops, festgehalten (docs/audio/Loop_Konzept.md §4j,
 *         Stufe 16b — der Beleg vor einem Umbau am Klangerzeuger)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Audio-Engine. Je mitgeliefertem Instrument drei
 * Toene, je mitgeliefertem Loop (was `asset/Asset.cmake` nennt, nicht die
 * Arbeitsdateien im Ordner) ein Durchlauf durch den Klangerzeuger (mit den
 * Effekten der Clips). Festgehalten sind je Eintrag die Zahl der
 * Abtastwerte, der groesste Betrag und die Summe der Betraege — in
 * `werte/loop_werte.txt` neben dieser Datei.
 *
 * - Ohne Umgebungsvariable vergleicht der Fall mit der Datei: die Zahl der
 *   Abtastwerte genau, Spitze und Summe auf ein Tausendstel (zwei Compiler
 *   runden nicht gleich).
 * - `LUMIVIZ_LOOP_WERTE_SCHREIBEN=1` schreibt die Datei neu — nach einer
 *   gewollten Aenderung am Klang oder an den mitgelieferten Dateien. Wer ein
 *   Instrument oder einen Loop dazulegt, sieht ihn als „nicht festgehalten“
 *   gemeldet, bis die Datei neu geschrieben ist.
 * - `LUMIVIZ_LOOP_WERTE_GENAU=<datei>`: gibt es die Datei nicht, legt der Fall
 *   dort je Eintrag eine Pruefsumme ueber jeden Abtastwert ab; gibt es sie,
 *   vergleicht er Bit fuer Bit. So belegt man einen Umbau, der nichts aendern
 *   darf: vorher ablegen, nachher vergleichen — je Compiler eine Datei.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/Instrument.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct WerteApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

struct Wert
{
    std::int64_t frames = 0;
    double spitze = 0.0;
    double summe = 0.0;
    std::uint64_t pruefsumme = 0;  ///< FNV-1a ueber die Bytes aller Abtastwerte
};

Wert messe(const std::vector<float>& daten)
{
    Wert wert;
    wert.frames = static_cast<std::int64_t>(daten.size() / 2);
    std::uint64_t summe = 1469598103934665603ull;
    for (const float abtastwert : daten)
    {
        wert.spitze = std::max(wert.spitze, static_cast<double>(std::abs(abtastwert)));
        wert.summe += std::abs(abtastwert);
        unsigned char bytes[sizeof(float)];
        std::memcpy(bytes, &abtastwert, sizeof(float));
        for (const unsigned char byte : bytes)
        {
            summe ^= byte;
            summe *= 1099511628211ull;
        }
    }
    wert.pruefsumme = summe;
    return wert;
}

/// Je Instrument drei Toene: tief und kurz, in der Mitte, hoch und sehr kurz.
struct Probe
{
    int hoehe;
    int dauerMs;
};
constexpr Probe kProben[] = {{36, 250}, {60, 500}, {81, 120}};

/// Ein Durchlauf eines Loops durch den Klangerzeuger (mit den Effekten der Clips); leer = nicht zu laden.
std::vector<float> rechneLoop(const QString& pfad)
{
    Loop loop;
    if (!ladeLoop(pfad, loop)) return {};
    const Ablauf ablauf = plane(loop, kRate);
    const auto klaenge = std::make_shared<const std::vector<Klang>>(looptest::klaengeOhneEngine(ablauf));
    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(baueProgramm(loop, kRate, klaenge));
    const int frames = static_cast<int>(ablauf.durchlaufFrames);
    std::vector<float> durchlauf(static_cast<std::size_t>(frames) * 2);
    for (int stelle = 0; stelle < frames; stelle += 441)
        erzeuger.render(durchlauf.data() + 2 * static_cast<std::ptrdiff_t>(stelle), std::min(441, frames - stelle));
    return durchlauf;
}

std::map<QString, Wert> rechneAlles()
{
    std::map<QString, Wert> alles;
    for (const Instrument& instrument : instrumente())
    {
        looptest::merkeSamplesOhneEngine(instrument);  // Sample-Schichten (S123)
        for (const Probe& probe : kProben)
            alles[QStringLiteral("inst:%1:%2:%3").arg(instrument.name).arg(probe.hoehe).arg(probe.dauerMs)] =
                messe(rendereTon(instrument, probe.hoehe, probe.dauerMs, kRate).daten);
    }

    // nur die mitgelieferten (asset/Asset.cmake), keine Arbeitsdateien (S129, W1 a, W2 a)
    const QDir wurzel(looptest::repoWurzel().filePath(QStringLiteral("asset/loop")));
    const QStringList loops = looptest::mitgelieferteLoops();
    REQUIRE(loops.size() >= 20);
    for (const QString& pfad : loops)
    {
        Loop loop;
        if (!ladeLoop(pfad, loop)) continue;
        // durch den Klangerzeuger: dort sitzen die Effekte der Clips
        alles[QStringLiteral("loop:") + wurzel.relativeFilePath(pfad)] = messe(rechneLoop(pfad));
    }
    return alles;
}

QString werteDatei()
{
    return QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir().filePath(QStringLiteral("werte/loop_werte.txt"));
}

}  // namespace

// Hoerproben (S123): `LUMIVIZ_LOOP_WAV=<datei.lvloop oder Ordner>` rechnet den Loop — oder jeden `.lvloop`
// im Ordner — einmal durch und legt ihn als WAV nach `LUMIVIZ_LOOP_WAV_ZIEL=<Ordner>`. Instrumente als
// Datei neben dem Loop und eigene (`user:`) gehen mit; ohne die Variable prueft der Fall nichts.
TEST_CASE("Loop Werte: ein Loop als WAV (LUMIVIZ_LOOP_WAV)")
{
    const QString quelle = qEnvironmentVariable("LUMIVIZ_LOOP_WAV");
    if (quelle.isEmpty())
    {
        MESSAGE("LUMIVIZ_LOOP_WAV nicht gesetzt — keine Hoerproben");
        return;
    }
    const WerteApp app;
    const QString ziel = qEnvironmentVariable("LUMIVIZ_LOOP_WAV_ZIEL");
    REQUIRE_FALSE(ziel.isEmpty());
    REQUIRE(QDir().mkpath(ziel));
    QStringList dateien;
    if (QFileInfo(quelle).isDir())
        for (const QString& name : QDir(quelle).entryList({QStringLiteral("*.lvloop")}, QDir::Files, QDir::Name))
            dateien << QDir(quelle).filePath(name);
    else
        dateien << quelle;
    REQUIRE_FALSE(dateien.isEmpty());
    for (const QString& pfad : dateien)
    {
        CAPTURE(pfad.toStdString());
        const std::vector<float> durchlauf = rechneLoop(pfad);
        REQUIRE_FALSE(durchlauf.empty());
        const QString wav = QDir(ziel).filePath(QFileInfo(pfad).completeBaseName() + QStringLiteral(".wav"));
        CHECK(schreibeWav(wav, durchlauf, kRate));
        MESSAGE(QFileInfo(wav).fileName().toStdString() << ": " << durchlauf.size() / 2 / kRate << " s, groesster Betrag "
                                                       << messe(durchlauf).spitze);
    }
}

TEST_CASE("Loop Werte: jedes mitgelieferte Instrument und jedes Beispiel rechnet, was festgehalten ist")
{
    const WerteApp app;
    const std::map<QString, Wert> alles = rechneAlles();
    REQUIRE(alles.size() > 150);

    // --- neu festhalten: nur auf Wunsch
    if (qEnvironmentVariableIntValue("LUMIVIZ_LOOP_WERTE_SCHREIBEN") == 1)
    {
        REQUIRE(QDir().mkpath(QFileInfo(werteDatei()).absolutePath()));
        QFile datei(werteDatei());
        REQUIRE(datei.open(QIODevice::WriteOnly));
        QTextStream aus(&datei);
        aus << "# Loop: gerechnete Werte, festgehalten von test_LoopWerte.cpp (LUMIVIZ_LOOP_WERTE_SCHREIBEN=1)\n";
        aus << "# Eintrag <Tab> Abtastwerte <Tab> groesster Betrag <Tab> Summe der Betraege\n";
        for (const auto& [name, wert] : alles)
            aus << name << '\t' << wert.frames << '\t' << QString::number(wert.spitze, 'g', 9) << '\t'
                << QString::number(wert.summe, 'g', 9) << '\n';
        MESSAGE("festgehalten: " << alles.size() << " Eintraege in " << werteDatei().toStdString());
    }

    // --- der Vergleich mit dem Festgehaltenen
    std::map<QString, Wert> festgehalten;
    {
        QFile datei(werteDatei());
        REQUIRE(datei.open(QIODevice::ReadOnly));
        QTextStream ein(&datei);
        while (!ein.atEnd())
        {
            const QString zeile = ein.readLine();
            if (zeile.startsWith(QLatin1Char('#')) || zeile.trimmed().isEmpty()) continue;
            const QStringList teile = zeile.split(QLatin1Char('\t'));
            REQUIRE(teile.size() == 4);
            festgehalten[teile[0]] = {teile[1].toLongLong(), teile[2].toDouble(), teile[3].toDouble(), 0};
        }
    }
    int verglichen = 0;
    QStringList neu;
    for (const auto& [name, wert] : alles)
    {
        const auto alt = festgehalten.find(name);
        if (alt == festgehalten.end())
        {
            neu << name;
            continue;
        }
        CAPTURE(name.toStdString());
        CHECK(wert.frames == alt->second.frames);
        CHECK(wert.spitze == doctest::Approx(alt->second.spitze).epsilon(0.001));
        CHECK(wert.summe == doctest::Approx(alt->second.summe).epsilon(0.001));
        ++verglichen;
    }
    QStringList fehlt;
    for (const auto& [name, wert] : festgehalten)
        if (alles.find(name) == alles.end()) fehlt << name;
    MESSAGE(verglichen << " Eintraege verglichen; nicht festgehalten: " << neu.size() << "; festgehalten, aber nicht mehr da: "
                       << fehlt.size());
    if (!neu.isEmpty()) MESSAGE("nicht festgehalten: " << neu.join(QStringLiteral(", ")).toStdString());
    if (!fehlt.isEmpty()) MESSAGE("nicht mehr da: " << fehlt.join(QStringLiteral(", ")).toStdString());
    CHECK(verglichen > 150);

    // --- Bit fuer Bit gegen eine eigene Datei: der Beleg fuer einen Umbau, der nichts aendern darf
    if (const QString genau = qEnvironmentVariable("LUMIVIZ_LOOP_WERTE_GENAU"); !genau.isEmpty())
    {
        QFile datei(genau);
        if (!datei.exists())
        {
            REQUIRE(datei.open(QIODevice::WriteOnly));
            QTextStream aus(&datei);
            for (const auto& [name, wert] : alles) aus << name << '\t' << wert.frames << '\t' << wert.pruefsumme << '\n';
            MESSAGE("Pruefsummen abgelegt: " << alles.size() << " Eintraege in " << genau.toStdString());
        }
        else
        {
            REQUIRE(datei.open(QIODevice::ReadOnly));
            QTextStream ein(&datei);
            int gleich = 0;
            QStringList anders;
            while (!ein.atEnd())
            {
                const QStringList teile = ein.readLine().split(QLatin1Char('\t'));
                if (teile.size() != 3) continue;
                const auto wert = alles.find(teile[0]);
                if (wert != alles.end() && wert->second.frames == teile[1].toLongLong() &&
                    wert->second.pruefsumme == teile[2].toULongLong())
                    ++gleich;
                else
                    anders << teile[0];
            }
            MESSAGE("Bit fuer Bit gleich: " << gleich << " von " << alles.size() << "; anders: " << anders.size());
            if (!anders.isEmpty()) MESSAGE("anders: " << anders.join(QStringLiteral(", ")).toStdString());
            CHECK(anders.isEmpty());
            CHECK(gleich == static_cast<int>(alles.size()));
        }
    }
}
