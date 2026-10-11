/**
 * @file Kit.cpp
 * @brief Loop: die Klaenge einer Spur finden und laden (Stufe 5)
 */

#include "loop/Kit.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <algorithm>

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
    for (std::size_t i = 0; i < klaenge.size(); ++i)
    {
        const QString& klang = namen[static_cast<qsizetype>(i)];
        const QString datei = klangDatei(klang);
        if (!datei.isEmpty()) klaenge[i].daten = engine.decodeStereo(datei, rate, kKlangMaxSekunden);
        if (klaenge[i].daten.empty() && bericht != nullptr)
            bericht->append(QStringLiteral("Sound not loaded: %1").arg(QDir::toNativeSeparators(klang)));
    }
    return klaenge;
}

}  // namespace lumi::loop
