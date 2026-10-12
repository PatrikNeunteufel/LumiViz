/**
 * @file Suchordner.hpp
 * @brief Die Liste der Suchordner fuer Bilder und Texturen importierter Presets
 *        (Wunsch Patrik S116)
 *
 * AVS-Presets nennen ihre Bilder (Texer, Texer II, Picture) und Videos nur beim
 * Dateinamen, MilkDrop-Presets ihre Texturen und Sprites ebenso. Gesucht wird
 * zuerst beim Preset und in den Ordnern darueber — wie bisher. Was dort nicht
 * liegt, wird in den Ordnern dieser Liste gesucht, in ihrer Reihenfolge. So
 * findet ein Preset aus einer kleinen Auswahl die Bilder einer grossen Sammlung
 * an anderem Ort.
 *
 * Abgelegt in den Einstellungen unter `import/searchDirs` (eine Liste). Der
 * einzelne Ordner von frueher (`import/imageSearchDir`, S53) gilt als erster
 * Eintrag, solange die Liste noch nie geschrieben wurde.
 *
 * Header-only, nur QtCore — auch die Einzelprogramme (MilkdropStandalone)
 * binden es ueber `MilkdropTextureResolve.hpp` ein.
 */
#pragma once

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QStringList>

namespace lumi::core::suchordner
{

inline constexpr const char* kSchluessel = "import/searchDirs";
/// Der einzelne Ordner bis S116.
inline constexpr const char* kSchluesselAlt = "import/imageSearchDir";

/// Ohne Leeres und ohne Doppelte (ohne Gross/Klein, wie Windows), Trennzeichen einheitlich.
[[nodiscard]] inline QStringList bereinigt(const QStringList& ordner)
{
    QStringList sauber;
    for (const QString& roh : ordner)
    {
        const QString einer = QDir::cleanPath(QDir::fromNativeSeparators(roh.trimmed()));
        if (roh.trimmed().isEmpty() || sauber.contains(einer, Qt::CaseInsensitive)) continue;
        sauber << einer;
    }
    return sauber;
}

/// Die Suchordner in ihrer Reihenfolge. Ordner, die es gerade nicht gibt
/// (ein Laufwerk ist nicht angeschlossen), stehen mit darin.
[[nodiscard]] inline QStringList liste()
{
    const QSettings einstellungen;
    if (einstellungen.contains(QLatin1String(kSchluessel)))
        return bereinigt(einstellungen.value(QLatin1String(kSchluessel)).toStringList());
    // noch nie geschrieben: der einzelne Ordner von frueher
    return bereinigt({einstellungen.value(QLatin1String(kSchluesselAlt)).toString()});
}

/// Die Liste setzen (auch eine leere: dann gilt der alte Ordner nicht mehr).
inline void setzeListe(const QStringList& ordner)
{
    QSettings einstellungen;
    einstellungen.setValue(QLatin1String(kSchluessel), bereinigt(ordner));
    einstellungen.remove(QLatin1String(kSchluesselAlt));
}

/// Eine Datei in den Suchordnern finden: `name` wie genannt (auch mit
/// Unterordner) oder nur ihr Dateiname, je Ordner direkt darin. Leer = in
/// keinem.
[[nodiscard]] inline QString findeDatei(const QString& name, const QStringList& ordner)
{
    if (name.isEmpty()) return {};
    const QString nurName = QFileInfo(name).fileName();
    for (const QString& einer : ordner)
    {
        const QDir dir(einer);
        for (const QString& kandidat : {dir.filePath(name), dir.filePath(nurName)})
            if (QFileInfo(kandidat).isFile()) return kandidat;
    }
    return {};
}
[[nodiscard]] inline QString findeDatei(const QString& name)
{
    return findeDatei(name, liste());
}

}  // namespace lumi::core::suchordner
