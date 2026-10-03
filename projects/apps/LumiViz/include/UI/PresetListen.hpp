/**
 * @file PresetListen.hpp
 * @brief Favoriten und „zuletzt benutzt" fuer Preset-Dateien (S89)
 *
 * Auftrag Patrik S89: die Preset-Zeile im Effect-Chain-Panel und im
 * Karaoke-Panel zeigt nicht alle Dateien eines Ordners, sondern **Favoriten**
 * und die **zehn zuletzt benutzten** — je eine Liste fuer die Effektkette
 * (`.avs`, `.milk`, `.lvfx`, `.lvfx2`) und fuer Karaoke-Presets (`.lvkar`).
 *
 * Abgelegt in den QSettings (`presets/<art>/favorites|recent`) als absolute
 * Pfade. Verglichen wird ohne Gross/Klein (Windows). Header-only, ohne
 * Oberflaeche — wer eine Liste aendert, meldet `PresetListsChangedEvent`.
 */

#pragma once

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QStringList>

namespace lumi::ui
{

enum class PresetArt
{
    Kette,   ///< alles, was als Visual geladen wird
    Karaoke  ///< Karaoke-Presets (.lvkar)
};

/// Welcher Ladeweg zu einer Preset-Datei gehoert (S91) — die Endung
/// entscheidet, Gross-/Kleinschreibung egal; alles Unbekannte ist eine Kette.
enum class PresetDateiArt
{
    Kette,  ///< .lvfx / .lvfx2 (und alles Unbekannte) -> LoadEffectChainEvent
    Avs,    ///< .avs  -> ImportAvsPresetEvent
    Milk    ///< .milk -> ImportMilkPresetEvent
};

[[nodiscard]] inline PresetDateiArt presetDateiArt(const QString& pfad)
{
    const QString endung = QFileInfo(pfad).suffix().toLower();
    if (endung == QLatin1String("avs")) return PresetDateiArt::Avs;
    if (endung == QLatin1String("milk")) return PresetDateiArt::Milk;
    return PresetDateiArt::Kette;
}

class PresetListen
{
public:
    static constexpr int kZuletztMax = 10;

    /// Favoriten in der Reihenfolge, in der sie gesetzt wurden.
    [[nodiscard]] static QStringList favoriten(PresetArt art)
    {
        return QSettings().value(schluessel(art, "favorites")).toStringList();
    }

    /// Zuletzt benutzt, juengstes zuerst (hoechstens kZuletztMax).
    [[nodiscard]] static QStringList zuletzt(PresetArt art)
    {
        return QSettings().value(schluessel(art, "recent")).toStringList();
    }

    [[nodiscard]] static bool istFavorit(PresetArt art, const QString& pfad)
    {
        return position(favoriten(art), pfad) >= 0;
    }

    /// Favorit setzen oder nehmen. Leerer Pfad = nichts.
    static void setzeFavorit(PresetArt art, const QString& pfad, bool an)
    {
        if (pfad.isEmpty()) return;
        QStringList liste = favoriten(art);
        const int i = position(liste, pfad);
        if (an && i < 0) liste.append(normal(pfad));
        if (!an && i >= 0) liste.removeAt(i);
        QSettings().setValue(schluessel(art, "favorites"), liste);
    }

    /// Geladen oder gespeichert: an die Spitze von „zuletzt".
    static void merkeBenutzt(PresetArt art, const QString& pfad)
    {
        if (pfad.isEmpty()) return;
        QStringList liste = zuletzt(art);
        if (const int i = position(liste, pfad); i >= 0) liste.removeAt(i);
        liste.prepend(normal(pfad));
        while (liste.size() > kZuletztMax) liste.removeLast();
        QSettings().setValue(schluessel(art, "recent"), liste);
    }

    /// Aus beiden Listen nehmen (die Datei gibt es nicht mehr).
    static void vergiss(PresetArt art, const QString& pfad)
    {
        setzeFavorit(art, pfad, false);
        QStringList liste = zuletzt(art);
        if (const int i = position(liste, pfad); i >= 0)
        {
            liste.removeAt(i);
            QSettings().setValue(schluessel(art, "recent"), liste);
        }
    }

    /// Das laufende Preset der Kette, wie das Fenster es zuletzt gemeldet hat
    /// (leer = neue Kette). Nur im Speicher und nur vom Oberflaechen-Thread:
    /// ein Panel, das spaeter geoeffnet wird, kennt so den Stand, ohne das
    /// CurrentPresetChangedEvent gesehen zu haben.
    [[nodiscard]] static QString laufendesKettenPreset() { return laufendSpeicher(); }
    static void setzeLaufendesKettenPreset(const QString& pfad) { laufendSpeicher() = pfad; }

    /// Das zuletzt unter einem Namen geladene/gespeicherte Karaoke-Preset
    /// (leer = keins, z. B. nach „Default"). Ueberlebt den Neustart — die
    /// Ebene selbst tut es auch (`current.lvkar`).
    [[nodiscard]] static QString laufendesKaraokePreset()
    {
        return QSettings().value(QStringLiteral("karaoke/currentPreset")).toString();
    }
    static void setzeLaufendesKaraokePreset(const QString& pfad)
    {
        QSettings().setValue(QStringLiteral("karaoke/currentPreset"),
                             pfad.isEmpty() ? QString() : normal(pfad));
    }

    /// Einheitliche Schreibweise eines Pfads in den Listen.
    [[nodiscard]] static QString normal(const QString& pfad)
    {
        return QDir::cleanPath(QFileInfo(pfad).absoluteFilePath());
    }

    /// Stelle von `pfad` in `liste` (ohne Gross/Klein), -1 = fehlt.
    [[nodiscard]] static int position(const QStringList& liste, const QString& pfad)
    {
        const QString p = normal(pfad);
        for (int i = 0; i < liste.size(); ++i)
            if (liste.at(i).compare(p, Qt::CaseInsensitive) == 0) return i;
        return -1;
    }

private:
    [[nodiscard]] static QString& laufendSpeicher()
    {
        static QString pfad;
        return pfad;
    }

    [[nodiscard]] static QString schluessel(PresetArt art, const char* liste)
    {
        return QStringLiteral("presets/") +
               (art == PresetArt::Kette ? QStringLiteral("chain/") : QStringLiteral("karaoke/")) +
               QLatin1String(liste);
    }
};

}  // namespace lumi::ui
