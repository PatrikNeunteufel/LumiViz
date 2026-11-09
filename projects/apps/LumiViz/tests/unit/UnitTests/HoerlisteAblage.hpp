/**
 ****************************************************************************************
 * @file   HoerlisteAblage.hpp
 * @brief  Tests: wo die Hoerlisten liegen (S182, AU3, AU3-1 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Hoerproben (Liste, Abdruecke und Klaenge beisammen, Konzept HoerTest_Helper §11/§12) liegen am **Hoerort**
 * `…\Visuals_Project\cmake\samples\Hoerproben\<LumiViz|IC>\<liste>\`, ausserhalb des Repos. Die Liste selbst
 * (`liste.json`, `klaenge.sha256`) bleibt zusaetzlich versioniert unter `hoertest/listen/<liste>/` (IC:
 * `hoertest/listen/ic/<liste>/`) — sie bindet jede Antwort an einen Commit. Der Fall `*Hoerlisten AU3-1*` prueft,
 * dass beide gleich sind.
 *
 * Liest vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 ****************************************************************************************
 */
#pragma once

#include <doctest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>

namespace hoerliste
{

/// Der Root des Repos (der Arbeitsordner der Test-Exe).
inline QDir repo()
{
    return QDir(QDir::currentPath());
}

/// Der Hoerort einer Liste: `…\cmake\samples\Hoerproben\<gruppe>\<liste>`.
inline QString hoerort(const QString& gruppe, const QString& liste)
{
    return QDir::cleanPath(repo().absoluteFilePath(QStringLiteral("../samples/Hoerproben/") + gruppe + QLatin1Char('/') + liste));
}

/// Die versionierte Kopie einer Liste im Repo (`hoertest/listen/<liste>`, IC unter `hoertest/listen/ic/<liste>`).
inline QString imRepo(const QString& gruppe, const QString& liste)
{
    return repo().absoluteFilePath(QStringLiteral("hoertest/listen/") + (gruppe == QLatin1String("IC") ? QStringLiteral("ic/") : QString()) + liste);
}

/// Nach dem Schreiben einer Liste am Hoerort: `liste.json` und `klaenge.sha256` ins Repo legen (AU3-1 a). Ein Ordner
/// innerhalb des Repos ist kein Hoerort (AU3: die Klaenge gehoeren nach `cmake\samples\Hoerproben`) — dann rot.
inline void legeInsRepo(const QDir& ort, const QString& gruppe = QStringLiteral("LumiViz"))
{
    const QString pfad = QDir::cleanPath(ort.absolutePath());
    const QString root = QDir::cleanPath(repo().absolutePath());
    if (pfad.startsWith(root + QLatin1Char('/'), Qt::CaseInsensitive))
    {
        FAIL_CHECK("Hoerort im Repo: " << pfad.toStdString() << " — die Klaenge gehoeren nach …\\cmake\\samples\\Hoerproben\\"
                                         << gruppe.toStdString() << "\\<liste> (AU3, S182)");
        return;
    }
    const QString ziel = imRepo(gruppe, ort.dirName());
    REQUIRE(QDir().mkpath(ziel));
    for (const char* name : {"liste.json", "klaenge.sha256"})
    {
        const QString q = ort.filePath(QLatin1String(name));
        const QString z = QDir(ziel).filePath(QLatin1String(name));
        QFile::remove(z);
        REQUIRE_MESSAGE(QFile::copy(q, z), q.toStdString());
    }
    MESSAGE("Liste ins Repo gelegt: " << ziel.toStdString());
}

}  // namespace hoerliste
