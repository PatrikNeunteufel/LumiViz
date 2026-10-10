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

std::vector<Klang> ladeKlaenge(const Loop& loop, IAudioEngine& engine, int rate, QStringList* bericht)
{
    std::vector<Klang> klaenge(loop.spuren.size());
    for (std::size_t i = 0; i < loop.spuren.size(); ++i)
    {
        const QString& klang = loop.spuren[i].klang;
        if (klang.isEmpty()) continue;  // eine Spur ohne Klang ist kein Fehler
        const QString datei = klangDatei(klang);
        if (!datei.isEmpty()) klaenge[i].daten = engine.decodeStereo(datei, rate, kKlangMaxSekunden);
        if (klaenge[i].daten.empty() && bericht != nullptr)
            bericht->append(QStringLiteral("Sound not loaded: %1").arg(QDir::toNativeSeparators(klang)));
    }
    return klaenge;
}

}  // namespace lumi::loop
