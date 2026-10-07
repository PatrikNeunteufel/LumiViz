/**
 * @file StartDatei.hpp
 * @brief Eine Datei als Argument: `LumiViz.exe <datei>` (S101)
 *
 * Der Explorer startet ein Programm mit dem Pfad der angeklickten Datei als
 * Argument. Hier steht, welche Angabe als Datei gilt, von welcher Art sie ist
 * und ueber welchen Ladeweg sie geht — derselbe wie beim Doppelklick im
 * Effect Browser. Eine Datei als Argument geht dem zuletzt aktiven Preset
 * (`UI/StartPreset.hpp`) vor.
 *
 * Kompositionen (`.lvcomp`) erkennt diese Datei nur an der Endung; geoeffnet
 * werden sie vom Hauptfenster (Composer-Naht N8).
 */
#pragma once

#include "UI/StartPreset.hpp"
#include "services/IEventBus.hpp"
#include "services/events/UIEvents.hpp"

#include <QFileInfo>
#include <QString>
#include <QStringList>

namespace lumi::ui::startdatei
{

enum class Art
{
    Keine,       ///< keine Datei, oder eine, die LumiViz nicht oeffnet
    Preset,      ///< .avs, .milk, .lvfx, .lvfx2 — in die Effektkette
    Karaoke,     ///< .lvkar — in die Karaoke-Ebene, die Kette bleibt
    Komposition  ///< .lvcomp — in den Composer
};

[[nodiscard]] inline Art art(const QString& pfad)
{
    const QString endung = QFileInfo(pfad).suffix().toLower();
    if (endung == QLatin1String("avs") || endung == QLatin1String("milk") ||
        endung == QLatin1String("lvfx") || endung == QLatin1String("lvfx2"))
        return Art::Preset;
    if (endung == QLatin1String("lvkar")) return Art::Karaoke;
    if (endung == QLatin1String("lvcomp")) return Art::Komposition;
    return Art::Keine;
}

/// Die Datei unter den Argumenten des Programms: die erste Angabe hinter dem
/// Programmnamen, die kein Schalter ist (`-…`), eine vorhandene Datei benennt
/// und von einer Art ist, die LumiViz oeffnet. Als absoluter Pfad — eine
/// relative Angabe gilt ab dem Arbeitsverzeichnis. Leer = keine.
[[nodiscard]] inline QString ausArgumenten(const QStringList& argumente)
{
    for (qsizetype i = 1; i < argumente.size(); ++i)
    {
        const QString& angabe = argumente[i];
        if (angabe.startsWith(QLatin1Char('-'))) continue;
        const QFileInfo datei(angabe);
        if (datei.isFile() && art(angabe) != Art::Keine) return datei.absoluteFilePath();
    }
    return {};
}

/// Ein Preset oder Karaoke-Preset ueber den Ladeweg seiner Endung oeffnen.
/// `beimStart`: ohne Rueckfrage zur Kette — es gibt noch nichts
/// Ungespeichertes. Sonst fragt der Empfaenger wie beim Doppelklick im Effect
/// Browser. false = nichts gemeldet (keine Datei dieser Arten; eine
/// Komposition oeffnet der Aufrufer).
inline bool oeffne(IEventBus& bus, const QString& pfad, bool beimStart)
{
    switch (art(pfad))
    {
        case Art::Preset:
            if (beimStart)
            {
                startpreset::lade(bus, pfad);
                return true;
            }
            switch (presetDateiArt(pfad))
            {
                case PresetDateiArt::Avs:
                    bus.publish(ImportAvsPresetEvent{pfad.toStdString()});
                    break;
                case PresetDateiArt::Milk:
                    bus.publish(ImportMilkPresetEvent{pfad.toStdString()});
                    break;
                case PresetDateiArt::Kette:
                    bus.publish(LoadEffectChainEvent{pfad.toStdString()});
                    break;
            }
            return true;
        case Art::Karaoke:
            bus.publish(LoadKaraokePresetEvent{pfad.toStdString()});
            return true;
        case Art::Keine:
        case Art::Komposition:
            break;
    }
    return false;
}

}  // namespace lumi::ui::startdatei
