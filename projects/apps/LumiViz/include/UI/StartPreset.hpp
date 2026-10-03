/**
 * @file StartPreset.hpp
 * @brief Das laufende Preset der Kette ueber den Neustart (S93)
 *
 * Gesichert wird dort, wo das laufende Preset gesetzt wird
 * (`MainWindow::setzeLaufendesPreset`), geladen beim Start vom Hauptfenster.
 * Bis S93 fuehrte der Effect Browser beides — und merkte nur, was ueber ihn
 * selbst geoeffnet wurde (Befund Patrik S93). Der Browser markiert jetzt nur
 * noch, was laeuft.
 *
 * Abgelegt in den QSettings unter `presets/chain/current` als absoluter Pfad.
 * Kommt spaeter eine Timeline, setzt sie das laufende Preset ueber denselben
 * Aufruf; an der Ablage aendert das nichts.
 */
#pragma once

#include "UI/PresetListen.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>
#include <QString>

namespace lumi::ui::startpreset
{

inline const QString& schluessel()
{
    static const QString k = QStringLiteral("presets/chain/current");
    return k;
}

/// Wo der Effect Browser es bis S93 ablegte — einmal zu uebernehmen.
inline const QString& alterSchluessel()
{
    static const QString k = QStringLiteral("panels/import_browser/lastPreset");
    return k;
}

/// Das Preset, mit dem eine frische Installation beginnt (S73) — eines der
/// mitgelieferten. Relativ zur Exe: der Ordner `presets/` wird bei jedem Build
/// neben die jeweilige Exe kopiert (Root-CMakeLists). Fehlt die Datei (eigener
/// Build ohne asset/presets), bleibt der Start leer — kein Fehlerfall.
[[nodiscard]] inline QString mitgeliefert()
{
    return QCoreApplication::applicationDirPath() +
           QStringLiteral("/presets/avs/EyeCandy2/02_flowers.avs");
}

[[nodiscard]] inline QString gesichert()
{
    return QSettings().value(schluessel()).toString();
}

/// Das laufende Preset festhalten. Leer (neue Kette ohne Namen) aendert
/// nichts: das zuletzt BENANNTE Preset bleibt fuer den naechsten Start.
inline void sichere(const QString& pfad)
{
    if (pfad.isEmpty()) return;
    QSettings().setValue(schluessel(), PresetListen::normal(pfad));
}

/// Womit der Start beginnt: das Gesicherte — sonst, was der Effect Browser
/// vor S93 gemerkt hatte — sonst `vorgabe`. Was es nicht mehr gibt, zaehlt
/// nicht. Leer = nichts zu laden.
[[nodiscard]] inline QString waehle(const QString& vorgabe)
{
    QSettings settings;
    for (const QString& pfad : {settings.value(schluessel()).toString(),
                                settings.value(alterSchluessel()).toString(), vorgabe})
        if (!pfad.isEmpty() && QFileInfo::exists(pfad)) return pfad;
    return {};
}

/// Preset ueber den Ladeweg seiner Endung anwerfen — ohne Rueckfrage: beim
/// Start gibt es noch nichts Ungespeichertes.
inline void lade(IEventBus& bus, const QString& pfad)
{
    switch (presetDateiArt(pfad))
    {
        case PresetDateiArt::Avs:
            bus.publish(ImportAvsPresetEvent{pfad.toStdString(), true});
            return;
        case PresetDateiArt::Milk:
            bus.publish(ImportMilkPresetEvent{pfad.toStdString(), true});
            return;
        case PresetDateiArt::Kette:
            bus.publish(LoadEffectChainEvent{pfad.toStdString()});
            return;
    }
}

}  // namespace lumi::ui::startpreset
