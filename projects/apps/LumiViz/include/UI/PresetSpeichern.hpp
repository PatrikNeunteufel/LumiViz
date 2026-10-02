/**
 * @file PresetSpeichern.hpp
 * @brief Regeln fuer „Save" gegen „Save As" einer Effektkette (S89)
 *
 * Entscheid Patrik S89 (3a): **Save** ueberschreibt ohne Dialog nur eine
 * EIGENE `.lvfx`/`.lvfx2`. In allen anderen Faellen verhaelt es sich wie
 * **Save As**: neue Kette, importiertes `.avs`/`.milk`, ein mitgeliefertes
 * Preset aus dem Programm- oder Asset-Ordner, und wenn sich die Endung
 * aendern muesste (die Kette hat eine Host-Gruppe bekommen oder verloren —
 * Ketten mit Host-Gruppe sind `.lvfx2`, flache `.lvfx`).
 *
 * Header-only und ohne Fenster, damit die Regel im Unit-Test laeuft.
 */

#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>

namespace lumi::ui
{

/// Endung, die eine Kette beim Speichern traegt (mit Punkt).
[[nodiscard]] inline QString kettenEndung(bool hatHostGruppe)
{
    return hatHostGruppe ? QStringLiteral(".lvfx2") : QStringLiteral(".lvfx");
}

/// Liegt `pfad` in (oder unter) einem der Ordner? Ohne Gross/Klein (Windows).
[[nodiscard]] inline bool liegtIn(const QString& pfad, const QStringList& ordner)
{
    const QString p = QDir::cleanPath(QFileInfo(pfad).absoluteFilePath());
    for (const QString& o : ordner)
    {
        if (o.isEmpty()) continue;
        const QString basis = QDir::cleanPath(QFileInfo(o).absoluteFilePath());
        if (p.startsWith(basis + QLatin1Char('/'), Qt::CaseInsensitive)) return true;
    }
    return false;
}

/**
 * Ordner mit mitgelieferten Presets: der Programmordner und — im Arbeitsbaum —
 * der Ordner `asset` (Aufwaertssuche wie bei den Format-Icons). Was dort
 * liegt, ueberschreibt „Save" nie still.
 */
[[nodiscard]] inline QStringList mitgelieferteOrdner()
{
    QStringList ordner{QCoreApplication::applicationDirPath()};
    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 12; ++i)
    {
        if (QFileInfo::exists(dir.filePath(QStringLiteral("asset/img/logo/icons/lumiviz.ico"))))
        {
            ordner.append(dir.filePath(QStringLiteral("asset")));
            break;
        }
        if (!dir.cdUp()) break;
    }
    return ordner;
}

/**
 * Darf „Save" die Kette ohne Dialog nach `laufendesPreset` schreiben?
 *
 * @param laufendesPreset zuletzt geladene/gespeicherte Datei (leer = neue Kette)
 * @param hatHostGruppe   traegt die Kette JETZT eine Host-Gruppe
 * @param geschuetzt      Ordner, in denen nie still ueberschrieben wird
 */
[[nodiscard]] inline bool stillSpeicherbar(const QString& laufendesPreset, bool hatHostGruppe,
                                           const QStringList& geschuetzt)
{
    if (laufendesPreset.isEmpty()) return false;
    const QFileInfo fi(laufendesPreset);
    // nur eigene Ketten — und nur, wenn die Endung zur Kette passt
    if (QLatin1Char('.') + fi.suffix().toLower() != kettenEndung(hatHostGruppe)) return false;
    if (!fi.exists()) return false;  // weg oder verschoben: lieber fragen
    return !liegtIn(laufendesPreset, geschuetzt);
}

}  // namespace lumi::ui
