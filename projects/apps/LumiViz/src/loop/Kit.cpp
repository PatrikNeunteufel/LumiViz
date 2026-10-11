/**
 * @file Kit.cpp
 * @brief Loop: die Klaenge einer Spur finden und laden (Stufe 5)
 */

#include "loop/Kit.hpp"

#include "loop/Instrument.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <algorithm>
#include <map>
#include <optional>
#include <utility>

namespace lumi::loop
{

namespace
{

constexpr int kKitVorsatzLaenge = 4;  // "kit:"

/// Aufwaertssuche ab dem Programmordner; Anker ist die README des Ordners.
QString sucheKit()
{
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 12; ++i)
    {
        const QString kandidat = dir.filePath(QStringLiteral("asset/loop"));
        if (QFileInfo::exists(kandidat + QStringLiteral("/README.md"))) return kandidat + QStringLiteral("/kit");
        if (!dir.cdUp()) break;
    }
    return {};
}

}  // namespace

QString kitOrdner()
{
    // einmal aufgeloest und gemerkt
    static const QString kOrdner = sucheKit();
    return kOrdner;
}

QStringList kitKlaenge()
{
    const QString ordner = kitOrdner();
    if (ordner.isEmpty()) return {};
    QStringList klaenge;
    const QStringList dateien = QDir(ordner).entryList({QStringLiteral("*.wav")}, QDir::Files, QDir::Name);
    for (const QString& datei : dateien)
    {
        const QString klang = QStringLiteral("kit:") + QFileInfo(datei).completeBaseName();
        if (!klangDatei(klang).isEmpty()) klaenge << klang;  // nur Namen, die `klangDatei` auch annimmt
    }
    return klaenge;
}

QString klangDatei(const QString& klang)
{
    if (istTon(klang)) return {};  // ein gerechneter Ton hat keine Datei
    if (!istKitKlang(klang)) return klang;
    const QString name = klang.mid(kKitVorsatzLaenge);
    const bool erlaubt = !name.isEmpty() && std::all_of(name.begin(), name.end(), [](QChar z) {
        return (z >= QLatin1Char('a') && z <= QLatin1Char('z')) || (z >= QLatin1Char('0') && z <= QLatin1Char('9')) ||
               z == QLatin1Char('_');
    });
    const QString ordner = kitOrdner();
    if (!erlaubt || ordner.isEmpty()) return {};
    return QDir(ordner).filePath(name + QStringLiteral(".wav"));
}

std::vector<Klang> ladeKlaenge(const QStringList& namen, IAudioEngine& engine, int rate, QStringList* bericht)
{
    std::vector<Klang> klaenge(static_cast<std::size_t>(namen.size()));
    std::map<QString, std::optional<Instrument>> geholt;
    QStringList zuNeu;  // Dateien einer Fassung, die dieses Programm nicht kennt
    for (std::size_t i = 0; i < klaenge.size(); ++i)
    {
        const QString& klang = namen[static_cast<qsizetype>(i)];
        if (istTon(klang))
        {
            // der Ton eines gerechneten Instruments: er entsteht hier, nicht aus einer Datei
            QString instrument;
            int hoehe = 0;
            int dauerMs = 0;
            const Instrument* gefunden = nullptr;
            if (zerlegeTon(klang, instrument, hoehe, dauerMs))
            {
                // je Instrument einmal holen — eine eigene Datei wird dabei gelesen
                auto ort = geholt.find(instrument);
                if (ort == geholt.end())
                {
                    Instrument neu;
                    bool neuer = false;
                    const bool da = holeInstrument(instrument, neu, &neuer);
                    if (neuer) zuNeu << instrument;
                    ort = geholt.emplace(instrument, da ? std::optional<Instrument>(std::move(neu)) : std::nullopt).first;
                }
                if (ort->second.has_value()) gefunden = &*ort->second;
            }
            if (gefunden != nullptr)
                klaenge[i] = rendereTon(*gefunden, hoehe, dauerMs, rate);
            else if (bericht != nullptr)
            {
                const QString zeile = (zuNeu.contains(instrument) ? QStringLiteral("Instrument needs a newer LumiViz: %1")
                                                                  : QStringLiteral("Instrument not found: %1"))
                                          .arg(instrument.isEmpty() ? klang : QDir::toNativeSeparators(instrument));
                if (!bericht->contains(zeile)) bericht->append(zeile);
            }
            continue;
        }
        const QString datei = klangDatei(klang);
        if (!datei.isEmpty()) klaenge[i].daten = engine.decodeStereo(datei, rate, kKlangMaxSekunden);
        if (klaenge[i].daten.empty() && bericht != nullptr)
            bericht->append(QStringLiteral("Sound not loaded: %1").arg(QDir::toNativeSeparators(klang)));
    }
    return klaenge;
}

}  // namespace lumi::loop
