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

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>

namespace lumi::milkdrop {

/// Suchordner vom Preset-Ordner aufwaerts (leer, wenn presetDir leer ist)
[[nodiscard]] inline QStringList textureSearchDirs(const QString& presetDir)
{
    QStringList dirs;
    if (presetDir.isEmpty()) return dirs;
    QDir walk(presetDir);
    for (int up = 0; up <= 4; ++up)
    {
        const QString base = walk.absolutePath();
        dirs << base + QStringLiteral("/textures") << base + QStringLiteral("/sprites")
             << base;
        if (!walk.cdUp()) break;
    }
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
    if (presetDir.isEmpty()) return {};
    QString rel = rawName;
    rel.replace(QLatin1Char('\\'), QLatin1Char('/'));
    const QString fileName = QFileInfo(rel).fileName();
    QDir walk(presetDir);
    for (int up = 0; up <= 4; ++up)
    {
        const QString base = walk.absolutePath();
        const QString candidates[3] = {
            base + QLatin1Char('/') + rel,
            base + QStringLiteral("/sprites/") + fileName,
            base + QStringLiteral("/textures/") + fileName};
        for (const QString& c : candidates)
        {
            if (QFileInfo::exists(c)) return c;
        }
        if (!walk.cdUp()) break;
    }
    return {};
}

} // namespace lumi::milkdrop
