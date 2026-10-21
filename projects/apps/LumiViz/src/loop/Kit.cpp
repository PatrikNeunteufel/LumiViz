/**
 * @file Kit.cpp
 * @brief Loop: die Klaenge einer Spur finden und laden (Stufe 5)
 */

#include "loop/Kit.hpp"

#include "core/Mitgeliefert.hpp"
#include "loop/Instrument.hpp"
#include "loop/Sample.hpp"
#include "loop/Tonspeicher.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <utility>

namespace lumi::loop
{

namespace
{

constexpr int kVorsatzLaenge = 4;  // "kit:" und "lib:"

/// `asset/loop/<unter>` — neben der Exe oder im Projektordner
/// (core/Mitgeliefert.hpp, S114).
QString sucheUnterLoop(const QString& unter)
{
    const QString loop = lumi::core::mitgeliefert::ordner(QStringLiteral("loop"));
    return loop.isEmpty() ? QString() : loop + QLatin1Char('/') + unter;
}

/// Die WAV-Dateien eines mitgelieferten Ordners als `<vorsatz><Name>`, nach Namen sortiert.
QStringList klaengeIn(const QString& ordner, const QString& vorsatz)
{
    if (ordner.isEmpty()) return {};
    QStringList klaenge;
    const QStringList dateien = QDir(ordner).entryList({QStringLiteral("*.wav")}, QDir::Files, QDir::Name);
    for (const QString& datei : dateien)
    {
        const QString klang = vorsatz + QFileInfo(datei).completeBaseName();
        if (!klangDatei(klang).isEmpty()) klaenge << klang;  // nur Namen, die `klangDatei` auch annimmt
    }
    return klaenge;
}

}  // namespace

QString kitOrdner()
{
    // einmal aufgeloest und gemerkt
    static const QString kOrdner = sucheUnterLoop(QStringLiteral("kit"));
    return kOrdner;
}

QString bibliothekOrdner()
{
    static const QString kOrdner = sucheUnterLoop(QStringLiteral("samples"));
    return kOrdner;
}

QStringList kitKlaenge()
{
    return klaengeIn(kitOrdner(), QStringLiteral("kit:"));
}

QStringList bibliotheksKlaenge()
{
    return klaengeIn(bibliothekOrdner(), QStringLiteral("lib:"));
}

QString klangDatei(const QString& klang)
{
    if (istTon(klang)) return {};  // ein gerechneter Ton hat keine Datei
    if (istEigenerKlang(klang)) return eigeneSampleDatei(klang.mid(static_cast<int>(qstrlen(kSampleVorsatz))));
    if (!istMitgelieferterKlang(klang)) return klang;
    const QString name = klang.mid(kVorsatzLaenge);
    const bool erlaubt = !name.isEmpty() && std::all_of(name.begin(), name.end(), [](QChar z) {
        return (z >= QLatin1Char('a') && z <= QLatin1Char('z')) || (z >= QLatin1Char('0') && z <= QLatin1Char('9')) ||
               z == QLatin1Char('_');
    });
    const QString ordner = istKitKlang(klang) ? kitOrdner() : bibliothekOrdner();
    if (!erlaubt || ordner.isEmpty()) return {};
    return QDir(ordner).filePath(name + QStringLiteral(".wav"));
}

Klaenge ladeKlaenge(const QStringList& namen, IAudioEngine& engine, int rate, QStringList* bericht)
{
    using Uhr = std::chrono::steady_clock;
    const auto msSeit = [](Uhr::time_point anfang) {
        return std::chrono::duration<double, std::milli>(Uhr::now() - anfang).count();
    };
    // ein Instrument, einmal geholt, mit seinem Fingerabdruck (TV2 a)
    struct Geholt
    {
        std::optional<Instrument> instrument;
        QByteArray abdruck;
    };
    Tonspeicher& speicher = tonspeicher();
    Klaenge klaenge(static_cast<std::size_t>(namen.size()));
    std::map<QString, Geholt> geholt;
    QStringList zuNeu;  // Dateien einer Fassung, die dieses Programm nicht kennt
    for (std::size_t i = 0; i < klaenge.size(); ++i)
    {
        const QString& klang = namen[static_cast<qsizetype>(i)];
        if (istTon(klang))
        {
            // der Ton eines gerechneten Instruments: aus dem Tonspeicher oder hier gerechnet
            TonAngabe ton;  // seit Stufe 23b mit Saite und Ende
            const QString& instrument = ton.instrument;
            const Geholt* gefunden = nullptr;
            if (zerlegeTon(klang, ton))
            {
                // je Instrument einmal holen — eine eigene Datei wird dabei gelesen
                auto ort = geholt.find(instrument);
                if (ort == geholt.end())
                {
                    Instrument neu;
                    bool neuer = false;
                    const bool da = holeInstrument(instrument, neu, &neuer);
                    if (neuer) zuNeu << instrument;
                    // die Aufnahmen seiner Sample-Schichten: einmal dekodiert und gemerkt (Stufe 11a)
                    if (da)
                        for (const Schicht& schicht : neu.schichten)
                        {
                            if (schicht.schwingung != Schwingung::Sample || !schicht.an) continue;
                            if (sampleDaten(schicht.sample) != nullptr) continue;
                            const QString datei = sampleDatei(schicht.sample);
                            if (!datei.isEmpty())
                                merkeSampleDaten(schicht.sample, kSampleRate,
                                                 engine.decodeStereo(datei, kSampleRate, kKlangMaxSekunden));
                            if (sampleDaten(schicht.sample) == nullptr && bericht != nullptr)
                            {
                                const QString zeile = QStringLiteral("Sample not loaded: %1")
                                                          .arg(schicht.sample.isEmpty() ? QStringLiteral("(none)")
                                                                                        : QDir::toNativeSeparators(schicht.sample));
                                if (!bericht->contains(zeile)) bericht->append(zeile);
                            }
                        }
                    Geholt eintrag;
                    if (da)
                    {
                        eintrag.abdruck = instrumentAbdruck(neu);
                        eintrag.instrument = std::move(neu);
                    }
                    ort = geholt.emplace(instrument, std::move(eintrag)).first;
                }
                if (ort->second.instrument.has_value()) gefunden = &ort->second;
            }
            if (gefunden != nullptr)
            {
                const QString schluessel = tonSchluessel(klang, rate, gefunden->abdruck);
                klaenge[i] = speicher.hole(schluessel);
                if (klaenge[i] == nullptr)
                {
                    const auto anfang = Uhr::now();
                    auto neu = std::make_shared<const Klang>(rendereTon(*gefunden->instrument, ton, rate));
                    speicher.lege(schluessel, neu, msSeit(anfang));
                    klaenge[i] = std::move(neu);
                }
            }
            else if (bericht != nullptr)
            {
                const QString zeile = (zuNeu.contains(instrument) ? QStringLiteral("Instrument needs a newer LumiViz: %1")
                                                                  : QStringLiteral("Instrument not found: %1"))
                                          .arg(instrument.isEmpty() ? klang : QDir::toNativeSeparators(instrument));
                if (!bericht->contains(zeile)) bericht->append(zeile);
            }
            continue;
        }
        // ein dekodierter Klang: ebenso aus dem Tonspeicher (TV3 a), sein Schluessel traegt die Datei —
        // nur eine Datei, die es gibt: an einer anderen liesse sich keine Aenderung erkennen
        const QString datei = klangDatei(klang);
        if (!datei.isEmpty())
        {
            const bool merkbar = QFileInfo(datei).isFile();
            const QString schluessel = merkbar ? klangSchluessel(klang, datei, rate) : QString();
            if (merkbar) klaenge[i] = speicher.hole(schluessel);
            if (klaenge[i] == nullptr)
            {
                const auto anfang = Uhr::now();
                Klang neu;
                neu.daten = engine.decodeStereo(datei, rate, kKlangMaxSekunden);
                if (!neu.daten.empty())
                {
                    auto geteilt = std::make_shared<const Klang>(std::move(neu));
                    if (merkbar) speicher.lege(schluessel, geteilt, msSeit(anfang));
                    klaenge[i] = std::move(geteilt);
                }
            }
        }
        if (klaenge[i] == nullptr && bericht != nullptr)
            bericht->append(QStringLiteral("Sound not loaded: %1").arg(QDir::toNativeSeparators(klang)));
    }
    return klaenge;
}

}  // namespace lumi::loop
