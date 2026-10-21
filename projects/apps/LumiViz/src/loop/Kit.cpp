/**
 * @file Kit.cpp
 * @brief Loop: die Klaenge einer Spur finden und laden (Stufe 5)
 */

#include "loop/Kit.hpp"

#include "core/Mitgeliefert.hpp"
#include "loop/Instrument.hpp"
#include "loop/Sample.hpp"
#include "loop/Tonauftrag.hpp"

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
    // seit Stufe 29c dieselbe Arbeit wie im Hintergrund, hier auf dem rufenden Thread
    Tonauftrag auftrag(namen, rate, [&engine](const QString& datei, int r, double maxSekunden) {
        return engine.decodeStereo(datei, r, maxSekunden);
    });
    auftrag.rechneHier();
    if (bericht != nullptr)
        for (const QString& zeile : auftrag.bericht())
            if (!bericht->contains(zeile)) bericht->append(zeile);
    return auftrag.klaenge();
}

}  // namespace lumi::loop
