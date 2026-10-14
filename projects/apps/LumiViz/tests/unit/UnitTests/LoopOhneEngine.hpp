/**
 ****************************************************************************************
 * @file   LoopOhneEngine.hpp
 * @brief  Loop-Tests: die Klaenge eines Ablaufs ohne Audio-Engine — Toene
 *         gerechnet, das Kit aus seinen WAV-Dateien im Repo
 *
 * @details
 * Wer einen Loop ohne Ton und ohne BASS rechnen will, braucht seine Klaenge:
 * die Toene der Instrumente rechnet das Programm selbst (`rendereTon`), die
 * Schlaege des Kits liest `liesWav` aus `asset/loop/kit`. Die mitgelieferten
 * Instrumente findet das Programm nur mit einer `QCoreApplication`.
 ****************************************************************************************
 */

#pragma once

#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Sample.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

#include <cstdint>
#include <cstring>
#include <vector>

namespace looptest
{

/// .../projects/apps/LumiViz/tests/unit/UnitTests -> Wurzel des Repos
inline QDir repoWurzel()
{
    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    return wurzel;
}

/// Eine WAV-Datei mit 16 Bit lesen, Stereo (eine Mono-Datei auf beide Seiten).
/// Leer = nicht zu lesen.
inline lumi::loop::Klang liesWav(const QString& pfad)
{
    lumi::loop::Klang klang;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return klang;
    const QByteArray roh = datei.readAll();
    if (roh.size() < 44 || !roh.startsWith("RIFF")) return klang;
    int kanaele = 1;
    qsizetype stelle = 12;
    while (stelle + 8 <= roh.size())
    {
        const QByteArray kennung = roh.mid(stelle, 4);
        std::uint32_t groesse = 0;
        std::memcpy(&groesse, roh.constData() + stelle + 4, 4);
        const char* inhalt = roh.constData() + stelle + 8;
        if (kennung == "fmt ")
        {
            std::uint16_t k = 1;
            std::memcpy(&k, inhalt + 2, 2);
            kanaele = k;
        }
        else if (kennung == "data")
        {
            const auto werte = static_cast<qsizetype>(groesse) / 2;
            for (qsizetype i = 0; i + kanaele <= werte; i += kanaele)
            {
                std::int16_t links = 0;
                std::int16_t rechts = 0;
                std::memcpy(&links, inhalt + 2 * i, 2);
                std::memcpy(&rechts, inhalt + 2 * (i + kanaele - 1), 2);
                klang.daten.push_back(static_cast<float>(links) / 32768.0f);
                klang.daten.push_back(static_cast<float>(rechts) / 32768.0f);
            }
            break;
        }
        stelle += 8 + static_cast<qsizetype>(groesse) + (groesse % 2);
    }
    return klang;
}

/// Die WAV-Datei eines mitgelieferten Klangs im Repo: `kit:` unter
/// `asset/loop/kit`, `lib:` unter `asset/loop/samples` (S123). Leer = keiner.
inline QString mitgelieferteWav(const QString& klang)
{
    using namespace lumi::loop;
    if (istKitKlang(klang)) return repoWurzel().filePath(QStringLiteral("asset/loop/kit/") + klang.mid(4) + QStringLiteral(".wav"));
    if (istBibliotheksKlang(klang))
        return repoWurzel().filePath(QStringLiteral("asset/loop/samples/") + klang.mid(4) + QStringLiteral(".wav"));
    return {};
}

/// Die Aufnahmen der Sample-Schichten eines Instruments merken, aus den WAV-Dateien
/// im Repo — ohne Engine (S123: Instrumente aus mitgelieferten Samples). Die Dateien
/// haben 44100 Hz; so werden sie gemerkt, das Programm rechnet von dort um.
inline void merkeSamplesOhneEngine(const lumi::loop::Instrument& instrument)
{
    using namespace lumi::loop;
    for (const Schicht& schicht : instrument.schichten)
    {
        if (schicht.schwingung != Schwingung::Sample || schicht.sample.isEmpty()) continue;
        if (sampleDaten(schicht.sample) != nullptr) continue;
        const QString wav = mitgelieferteWav(schicht.sample);
        Klang klang = liesWav(wav.isEmpty() ? sampleDatei(schicht.sample) : wav);
        if (!klang.daten.empty()) merkeSampleDaten(schicht.sample, 44100, std::move(klang.daten));
    }
}

/// Die Klaenge eines Ablaufs ohne Audio-Engine: Toene gerechnet, das Kit und
/// die mitgelieferten Samples aus ihren WAV-Dateien. Was sich nicht findet,
/// bleibt leer.
inline std::vector<lumi::loop::Klang> klaengeOhneEngine(const lumi::loop::Ablauf& ablauf)
{
    using namespace lumi::loop;
    std::vector<Klang> klaenge;
    for (const QString& name : ablauf.klangNamen)
    {
        QString instrument;
        int hoehe = 0;
        int ms = 0;
        if (zerlegeTon(name, instrument, hoehe, ms))
        {
            // mitgeliefert, eigen (`user:`) oder eine Datei neben dem Loop
            Instrument gefunden;
            const bool da = holeInstrument(instrument, gefunden);
            if (da) merkeSamplesOhneEngine(gefunden);
            klaenge.push_back(da ? rendereTon(gefunden, hoehe, ms, ablauf.rate) : Klang{});
        }
        else if (istMitgelieferterKlang(name))
            klaenge.push_back(liesWav(mitgelieferteWav(name)));
        else
            klaenge.push_back({});
    }
    return klaenge;
}

}  // namespace looptest
