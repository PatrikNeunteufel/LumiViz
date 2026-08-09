/**
 ****************************************************************************************
 * @file   MilkdropTrace.hpp
 * @brief  Diagnose-Trace fuer den MilkDrop-Host (Session 41, C1/C2-Befund):
 *         schreibt Lade- und Renderpfad-Entscheide zeitgestempelt in eine
 *         Log-Datei, damit sich der komplette Weg eines Presets (Parse →
 *         Klassifikation → Transpile → GL-Programm → Render-Branch)
 *         nachvollziehen laesst
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 *
 * @details
 * - Datei: `<TEMP>/lumiviz_milkdrop_trace.log` — wird beim ERSTEN Log-Eintrag
 *   des Prozesses geleert (ein Lauf = ein Log) und beginnt mit einer
 *   Kopfzeile (Exe-Pfad + PID) zur Binary-Verifikation.
 * - Thread-sicher (Mutex) und bewusst OHNE BasicLogger: die Render-Pfad-
 *   Einträge kommen aus dem Render-Thread (BasicLogger ist dort tabu).
 * - Die Aufrufer loggen nur EREIGNISSE und ZUSTANDSWECHSEL (kein
 *   Per-Frame-Spam) — die Datei bleibt klein, das Log bleibt immer an.
 * - `LUMIVIZ_MILKDROP_TRACE=0` (Umgebungsvariable) schaltet das Trace ab;
 *   `setEcho(true)` spiegelt Eintraege zusaetzlich auf stderr (Standalone).
 ****************************************************************************************
 */

#pragma once

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QThread>

#include <atomic>
#include <cstdio>

namespace lumi::milkdrop::trace {

/// @brief Pfad der Trace-Datei (fix im Temp-Verzeichnis, ein Log je Lauf)
[[nodiscard]] inline QString filePath()
{
    return QDir::tempPath() + QStringLiteral("/lumiviz_milkdrop_trace.log");
}

namespace detail {
inline std::atomic<bool>& echoFlag()
{
    static std::atomic<bool> echo{false};
    return echo;
}
} // namespace detail

/// @brief Eintraege zusaetzlich auf stderr spiegeln (Standalone-Konsole)
inline void setEcho(bool on)
{
    detail::echoFlag().store(on, std::memory_order_relaxed);
}

/// @brief Trace aktiv? (Umgebungsvariable LUMIVIZ_MILKDROP_TRACE=0 → aus)
[[nodiscard]] inline bool enabled()
{
    static const bool on = qgetenv("LUMIVIZ_MILKDROP_TRACE") != QByteArrayLiteral("0");
    return on;
}

/**
 * @brief Eine Trace-Zeile schreiben (zeitgestempelt + Thread-Kennung)
 *
 * Oeffnet die Datei je Eintrag im Append-Modus (kein offener Handle, jeder
 * Eintrag ist sofort auf Platte — wichtig, wenn die App danach abstuerzt).
 */
inline void log(const QString& message)
{
    if (!enabled()) return;

    static QMutex mutex;
    QMutexLocker lock(&mutex);

    static bool firstEntry = true;
    QFile file(filePath());
    const QIODevice::OpenMode mode =
        firstEntry ? (QIODevice::WriteOnly | QIODevice::Truncate)
                   : (QIODevice::WriteOnly | QIODevice::Append);
    if (!file.open(mode)) return;
    if (firstEntry)
    {
        firstEntry = false;
        const QString exe = (QCoreApplication::instance() != nullptr)
                                ? QCoreApplication::applicationFilePath()
                                : QStringLiteral("<keine QCoreApplication>");
        const QString head = QStringLiteral("==== LumiViz Milkdrop-Trace ==== %1 | exe=%2 | pid=%3\n")
                                 .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                                 .arg(exe)
                                 .arg(QCoreApplication::applicationPid());
        file.write(head.toUtf8());
    }

    const QString line =
        QStringLiteral("[%1|T%2] %3\n")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")))
            .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 16)
            .arg(message);
    file.write(line.toUtf8());
    file.close();

    if (detail::echoFlag().load(std::memory_order_relaxed))
    {
        std::fputs(qPrintable(line), stderr);
        std::fflush(stderr);
    }
}

} // namespace lumi::milkdrop::trace
