/**
 ****************************************************************************************
 * @file   MilkdropTextureResolve.hpp
 * @brief  Gemeinsame Datei-Aufloesung fuer Milkdrop-Texturen und Sprite-Bilder
 *         (S43 — SSOT fuer Render-Lader UND .lvfx-Einbettung beim Speichern)
 *
 * @author LumiPulse Team
 * @date   July 2026
 * @version 1.0.0
 *
 * @details
 * Suchregel (S43): vom Preset-Ordner AUFWAERTS (bis 4 Ebenen) jeweils
 * `textures/`, `sprites/` und den Ordner selbst — das Asset-Pack haelt Bilder
 * in beiden Ordnern, Presets liegen auch in Unterordnern. Nutzer:
 * MilkdropVisualizer (Laufzeit-Lader) und ChainSerializer (Einbettung beim
 * Speichern, Entscheid Patrik S43).
 ****************************************************************************************
 */

#pragma once

#include "core/Suchordner.hpp"

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>

namespace lumi::milkdrop {

/// Suchordner: vom Preset-Ordner aufwaerts — und danach die Suchordner aus den
/// Einstellungen (S116, core/Suchordner.hpp), je mit `textures/`, `sprites/`
/// und dem Ordner selbst. Was beim Preset liegt, gilt zuerst. Ohne presetDir
/// bleiben die aus den Einstellungen.
[[nodiscard]] inline QStringList textureSearchDirs(const QString& presetDir)
{
    QStringList dirs;
    if (!presetDir.isEmpty())
    {
        QDir walk(presetDir);
        for (int up = 0; up <= 4; ++up)
        {
            const QString base = walk.absolutePath();
            dirs << base + QStringLiteral("/textures") << base + QStringLiteral("/sprites")
                 << base;
            if (!walk.cdUp()) break;
        }
    }
    for (const QString& base : lumi::core::suchordner::liste())
        dirs << base + QStringLiteral("/textures") << base + QStringLiteral("/sprites") << base;
    return dirs;
}

/// Bild-Endungen wie der Referenz-Lader
[[nodiscard]] inline const QStringList& textureExtensions()
{
    static const QStringList kExt = {QStringLiteral("jpg"), QStringLiteral("jpeg"),
                                     QStringLiteral("jfif"), QStringLiteral("png"),
                                     QStringLiteral("bmp")};
    return kExt;
}

/// Textur-Basisname (`lines2`) → Dateipfad (leer = nicht gefunden)
[[nodiscard]] inline QString resolveTextureFile(const QString& presetDir,
                                                const QString& baseName)
{
    for (const QString& dir : textureSearchDirs(presetDir))
    {
        for (const QString& ext : textureExtensions())
        {
            const QString candidate = dir + QLatin1Char('/') + baseName +
                                      QLatin1Char('.') + ext;
            if (QFileInfo::exists(candidate)) return candidate;
        }
    }
    return {};
}

/// Sprite-imageName (ggf. relativer Pfad) → Dateipfad (leer = nicht gefunden)
[[nodiscard]] inline QString resolveSpriteFile(const QString& presetDir,
                                               const QString& rawName)
{
    QString rel = rawName;
    rel.replace(QLatin1Char('\\'), QLatin1Char('/'));
    // S93: ein per Dateidialog gewaehltes Bild steht als absoluter Pfad da —
    // es gilt ohne Suche, auch an einem Knoten ohne Preset-Ordner.
    if (!rel.isEmpty() && QFileInfo(rel).isAbsolute())
        return QFileInfo::exists(rel) ? rel : QString();
    if (rel.isEmpty()) return {};
    const QString fileName = QFileInfo(rel).fileName();
    const auto in = [&rel, &fileName](const QString& base) -> QString {
        const QString candidates[3] = {
            base + QLatin1Char('/') + rel,
            base + QStringLiteral("/sprites/") + fileName,
            base + QStringLiteral("/textures/") + fileName};
        for (const QString& c : candidates)
        {
            if (QFileInfo::exists(c)) return c;
        }
        return {};
    };
    if (!presetDir.isEmpty())
    {
        QDir walk(presetDir);
        for (int up = 0; up <= 4; ++up)
        {
            if (const QString treffer = in(walk.absolutePath()); !treffer.isEmpty()) return treffer;
            if (!walk.cdUp()) break;
        }
    }
    // danach die Suchordner aus den Einstellungen (S116)
    for (const QString& base : lumi::core::suchordner::liste())
        if (const QString treffer = in(base); !treffer.isEmpty()) return treffer;
    return {};
}

} // namespace lumi::milkdrop
