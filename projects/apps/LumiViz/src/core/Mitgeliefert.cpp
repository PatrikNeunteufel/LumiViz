/**
 * @file Mitgeliefert.cpp
 * @brief Wo ein mitgelieferter Bereich liegt (Datenablage_Konzept.md, Stufe A1)
 */

#include "core/Mitgeliefert.hpp"

#include <BasicLogger.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include <mutex>
#include <set>

namespace lumi::core::mitgeliefert
{

namespace
{

/// Ein leerer Programmordner (kein `QCoreApplication`, etwa in einem Test
/// ohne App) heisst wie bis S114: der Arbeitsordner.
QString start(const QString& exeOrdner)
{
    return QDir(exeOrdner).absolutePath();
}

QString kopieNebenExe(const QString& exeOrdner, const QString& bereich)
{
    return QDir(start(exeOrdner)).filePath(QStringLiteral("asset/") + bereich);
}

/// Ab dem Programmordner selbst, wie bis S114. Die Kopie neben der Exe
/// (`ausser`) zaehlt hier nicht, auch wenn ihr Anker mitkopiert wurde (die
/// Icons).
QString aufwaerts(const QString& exeOrdner, const QString& bereich, const QString& anker, const QString& ausser)
{
    QDir dir(start(exeOrdner));
    for (int i = 0; i < kEbenenAufwaerts; ++i)
    {
        const QString kandidat = dir.filePath(QStringLiteral("asset/") + bereich);
        if (kandidat != ausser && QFileInfo::exists(kandidat + QLatin1Char('/') + anker)) return kandidat;
        if (!dir.cdUp()) break;
    }
    return {};
}

}  // namespace

Fund suche(const QString& exeOrdner, const QString& bereich, const QString& anker, bool projektZuerst)
{
    if (bereich.isEmpty()) return {};
    const QString neben = kopieNebenExe(exeOrdner, bereich);
    const QString kopie = QFileInfo(neben).isDir() ? neben : QString();
    const QString projekt = aufwaerts(exeOrdner, bereich, anker, neben);
    if (projektZuerst && !projekt.isEmpty()) return {projekt, Herkunft::Projekt};
    if (!kopie.isEmpty()) return {kopie, Herkunft::NebenExe};
    if (!projekt.isEmpty()) return {projekt, Herkunft::Projekt};
    return {};
}

QString ordner(const QString& bereich, const QString& anker)
{
    const QString exe = QCoreApplication::applicationDirPath();
    const Fund fund = suche(exe, bereich, anker, projektZuerst());

    // je Bereich einmal sagen, woher gelesen wird
    static std::mutex sperre;
    static std::set<QString> gemeldet;
    bool neu = false;
    {
        const std::lock_guard<std::mutex> halter(sperre);
        neu = gemeldet.insert(bereich).second;
    }
    if (neu)
    {
        // ohne App-Objekt begann die Suche im Arbeitsordner — das ist nicht „neben der Exe“ (ANFUEHRUNG_OK)
        const char* woher = fund.herkunft == Herkunft::NebenExe  ? (exe.isEmpty() ? "working folder" : "next to the executable")
                            : fund.herkunft == Herkunft::Projekt ? "project folder"
                                                                 : "not found";
        BasicLogger::logInfo("Bundled data '" + bereich.toStdString() + "': " + woher +
                             (fund.ordner.isEmpty() ? std::string()
                                                    : " (" + QDir::toNativeSeparators(fund.ordner).toStdString() + ")"));
    }
    return fund.ordner;
}

bool projektZuerst()
{
    return QSettings().value(QLatin1String(kSchluesselProjektZuerst), false).toBool();
}

void setzeProjektZuerst(bool an)
{
    QSettings().setValue(QLatin1String(kSchluesselProjektZuerst), an);
}

bool projektVorhanden()
{
    const QString exe = QCoreApplication::applicationDirPath();
    const QString bereich = QStringLiteral("img/logo/icons");
    return !aufwaerts(exe, bereich, QStringLiteral("lumiviz.ico"), kopieNebenExe(exe, bereich)).isEmpty();
}

}  // namespace lumi::core::mitgeliefert
