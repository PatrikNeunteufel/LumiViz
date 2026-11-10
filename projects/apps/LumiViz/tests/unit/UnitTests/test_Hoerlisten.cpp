/**
 ****************************************************************************************
 * @file   test_Hoerlisten.cpp
 * @brief  Hoerlisten (S182, AU3-1 a): die Liste im Repo und am Hoerort sind gleich
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Je Liste im Repo (`hoertest/listen/<liste>/liste.json`, IC unter `hoertest/listen/ic/<liste>/`) die Liste und die
 * Abdruecke am Hoerort (`…\cmake\samples\Hoerproben\<LumiViz|IC>\<liste>\`, HoerlisteAblage.hpp) Byte fuer Byte.
 * Fehlt der Hoerort (abgeschlossen und archiviert, oder nicht auf diesem Rechner), meldet der Fall das nur. Listen im
 * Format des Behelfs (ohne `liste.json`, etwa `pv1`) zaehlen nicht.
 *
 * Liest vom Arbeitsordner aus — die Test-Exe aus dem Projekt-Root starten.
 ****************************************************************************************
 */

#include <doctest.h>

#include "HoerlisteAblage.hpp"

#include <QDir>
#include <QFile>

#include <utility>

namespace
{

QByteArray inhalt(const QString& pfad)
{
    QFile f(pfad);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

}  // namespace

TEST_CASE("Hoerlisten AU3-1 a: die Liste im Repo und am Hoerort sind gleich")
{
    int listen = 0, verglichen = 0;
    for (const auto& [gruppe, wurzel] : {std::pair{QStringLiteral("LumiViz"), QStringLiteral("hoertest/listen")},
                                         std::pair{QStringLiteral("IC"), QStringLiteral("hoertest/listen/ic")}})
    {
        const QDir dir(hoerliste::repo().absoluteFilePath(wurzel));
        for (const QString& liste : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
        {
            if (!QFile::exists(dir.filePath(liste + QStringLiteral("/liste.json")))) continue;
            ++listen;
            const QString ort = hoerliste::hoerort(gruppe, liste);
            CAPTURE(ort.toStdString());
            if (!QDir(ort).exists())
            {
                MESSAGE("Hoerort fehlt (archiviert oder nicht hier): " << ort.toStdString());
                continue;
            }
            for (const char* name : {"liste.json", "klaenge.sha256"})
            {
                const QByteArray a = inhalt(dir.filePath(liste + QLatin1Char('/') + QLatin1String(name)));
                const QByteArray b = inhalt(QDir(ort).filePath(QLatin1String(name)));
                CHECK_MESSAGE(!a.isEmpty(), name);
                CHECK_MESSAGE(a == b, gruppe.toStdString() << "/" << liste.toStdString() << "/" << name);
            }
            ++verglichen;
        }
    }
    MESSAGE(listen << " Listen im Repo, " << verglichen << " am Hoerort verglichen");
    // S182: drei von LumiViz (stab_34g, membran_34h, klasse1_34i), elf aus IC; S186 (LG1 a): IC 01, 04, 05 durch die
    // drei von LumiViz abgeloest, aus dem Repo entfernt (am Hoerort mit `abgeloest.txt`) — bleiben acht aus IC
    CHECK(listen >= 11);
    for (const char* abgeloest : {"01_hammer_saite", "04_platte_stab", "05_membran"})
        CHECK_MESSAGE(!QDir(hoerliste::repo().absoluteFilePath(QStringLiteral("hoertest/listen/ic/") + QLatin1String(abgeloest))).exists(),
                      abgeloest << " ist abgeloest (LG1 a) — nicht wieder ins Repo legen");
}
