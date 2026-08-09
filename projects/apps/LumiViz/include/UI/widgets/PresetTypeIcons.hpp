/**
 ****************************************************************************************
 * @file   PresetTypeIcons.hpp
 * @brief  Gemeinsame Format-Icons fuer Presets (AVS / MilkDrop / LumiViz)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Loest das Icon-Verzeichnis `asset/img/logo/icons` zur Laufzeit auf
 * (Aufwaertssuche vom Anwendungsverzeichnis — die Exe laeuft aus out/build/…)
 * und liefert die Format-Icons fuer Effect-Chain-Panel UND Import-Browser aus
 * einer Quelle (SSOT). Leeres Verzeichnis => Null-Icons; Aufrufer pruefen
 * `presetIconDir().isEmpty()` und fallen auf Text bzw. Standard-Icons zurueck.
 ****************************************************************************************
 */

#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QString>

namespace lumi::ui
{

/// Preset-Format fuer die Icon-Wahl (bewusst UI-lokal, kein Chain-Typ).
enum class PresetIconKind
{
    Avs,
    MilkDrop,
    Native
};

/// Icon-Verzeichnis (asset/img/logo/icons) — leer, wenn nicht gefunden.
inline QString presetIconDir()
{
    static const QString kDir = [] {
        QDir dir(QCoreApplication::applicationDirPath());
        for (int i = 0; i < 12; ++i)
        {
            const QString candidate =
                dir.filePath(QStringLiteral("asset/img/logo/icons"));
            if (QFileInfo::exists(candidate + QStringLiteral("/lumiviz.ico")))
                return candidate;
            if (!dir.cdUp()) break;
        }
        return QString();
    }();
    return kDir;
}

/// Format-Icon (Null-Icon, wenn kein Icon-Verzeichnis gefunden wurde).
inline const QIcon& presetTypeIcon(PresetIconKind kind)
{
    static const QIcon kAvs(presetIconDir() + QStringLiteral("/avs.ico"));
    static const QIcon kMilkDrop(presetIconDir() + QStringLiteral("/milkdrop.ico"));
    static const QIcon kNative(presetIconDir() + QStringLiteral("/lumiviz.ico"));
    switch (kind)
    {
        case PresetIconKind::Avs: return kAvs;
        case PresetIconKind::MilkDrop: return kMilkDrop;
        default: return kNative;
    }
}

/// Dateiendung -> Format-Icon (.milk / .lvfx / .lvfx2 / sonst = AVS —
/// gleiche Zuordnung wie der Import-Dispatch des Import-Browsers).
inline const QIcon& presetTypeIconForSuffix(const QString& suffix)
{
    if (suffix.compare(QStringLiteral("milk"), Qt::CaseInsensitive) == 0)
        return presetTypeIcon(PresetIconKind::MilkDrop);
    if (suffix.compare(QStringLiteral("lvfx"), Qt::CaseInsensitive) == 0 ||
        suffix.compare(QStringLiteral("lvfx2"), Qt::CaseInsensitive) == 0)
        return presetTypeIcon(PresetIconKind::Native);
    return presetTypeIcon(PresetIconKind::Avs);
}

}  // namespace lumi::ui
