/**
 * @file AppAktionen.cpp
 * @brief Sichttest-Steuerung: Panels, Karaoke-Ebene, Playlist und Player
 *        (Konzept §12.2 Naht N20)
 */

#include "sichttest/AppAktionen.hpp"

#include "UI/panels/KaraokePanel.hpp"
#include "UI/panels/PanelBase.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "audio/IAudioPlayer.hpp"
#include "audio/IPlaylist.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "services/events/UIEvents.hpp"
#include "visualizers/MultiEffectVisualizer.hpp"

#include <DockWidget.h>

#include <QApplication>
#include <QDir>
#include <QFileInfo>

#include <string>
#include <QPointer>
#include <QStringList>

namespace lumi::sichttest
{

namespace appaktion
{

Ergebnis panelZeigen(const QJsonObject& argumente)
{
    const QString name = argument(argumente, QStringLiteral("name"));
    if (name.isEmpty()) return ungueltig(QStringLiteral("Expected: panel_zeigen name=<id or title>"));

    PanelBase* gefunden = nullptr;
    QStringList kennungen;
    const QWidgetList alle = QApplication::topLevelWidgets();
    for (QWidget* oben : alle)
    {
        QList<PanelBase*> panels = oben->findChildren<PanelBase*>();
        if (auto* selbst = qobject_cast<PanelBase*>(oben)) panels.prepend(selbst);
        for (PanelBase* panel : panels)
        {
            kennungen << panel->panelId();
            if (gefunden == nullptr && (panel->panelId().compare(name, Qt::CaseInsensitive) == 0 ||
                                        panel->panelTitle().compare(name, Qt::CaseInsensitive) == 0))
                gefunden = panel;
        }
    }
    if (gefunden == nullptr)
    {
        kennungen.removeDuplicates();
        kennungen.sort();
        return fehler(QStringLiteral("No panel named '%1'. Known: %2")
                          .arg(name, kennungen.join(QStringLiteral(", "))));
    }
    // das Dock, in dem das Panel sitzt (auch unter einer Rollflaeche)
    ads::CDockWidget* dock = nullptr;
    for (QWidget* w = gefunden; w != nullptr && dock == nullptr; w = w->parentWidget())
        dock = qobject_cast<ads::CDockWidget*>(w);
    if (dock == nullptr)
        return fehler(QStringLiteral("The panel '%1' is not docked.").arg(gefunden->panelId()));
    dock->toggleView(true);
    dock->raise();
    return ok();
}

Ergebnis presetLaden(VisualizerWidget& fenster, const QString& root, const QJsonObject& argumente)
{
    const QString angabe = argument(argumente, QStringLiteral("datei"));
    if (angabe.isEmpty()) return ungueltig(QStringLiteral("Argument datei=<path> is missing."));
    const QString datei = loeseAuf(angabe, root);
    if (datei.isEmpty())
        return ungueltig(QStringLiteral("File not found: %1 (relative paths start at %2)")
                             .arg(QDir::toNativeSeparators(angabe), QDir::toNativeSeparators(root)));
    auto* bus = fenster.dienste().tryResolve<IEventBus>();
    if (bus == nullptr) return fehler(QStringLiteral("There is no event bus."));

    // Jede Endung ueber ihren Ladeweg; die Frage nach der ungespeicherten
    // Kette hat der Tester schon gestellt (`confirmed`).
    const QString endung = QFileInfo(datei).suffix().toLower();
    const std::string pfad = datei.toStdString();
    if (endung == QLatin1String("avs"))
        bus->publish(ImportAvsPresetEvent{pfad, true});
    else if (endung == QLatin1String("milk"))
        bus->publish(ImportMilkPresetEvent{pfad, true});
    else if (endung == QLatin1String("lvfx") || endung == QLatin1String("lvfx2"))
        bus->publish(LoadEffectChainEvent{pfad, true});
    else
        return ungueltig(QStringLiteral("Not a preset (.avs, .milk, .lvfx, .lvfx2): %1")
                             .arg(QDir::toNativeSeparators(datei)));
    return ok();
}

Ergebnis karaoke(VisualizerWidget& fenster, const QJsonObject& argumente)
{
    const std::optional<bool> an = anOderAus(argumente);
    if (!an.has_value()) return ungueltig(QStringLiteral("Expected: karaoke an|aus"));
    KaraokePanel::setzeKaraokeAn(fenster.dienste().tryResolve<IEventBus>(), *an);
    return ok();
}

Ergebnis karaokeLaden(VisualizerWidget& fenster, const QString& root, const QJsonObject& argumente)
{
    const QString angabe = argument(argumente, QStringLiteral("datei"));
    if (angabe.isEmpty()) return ungueltig(QStringLiteral("Argument datei=<path> is missing."));
    const QString datei = loeseAuf(angabe, root);
    if (datei.isEmpty())
        return ungueltig(QStringLiteral("File not found: %1 (relative paths start at %2)")
                             .arg(QDir::toNativeSeparators(angabe), QDir::toNativeSeparators(root)));
    auto* host = dynamic_cast<MultiEffectVisualizer*>(fenster.visualizer());
    if (host == nullptr)
        return fehler(QStringLiteral("The karaoke layer works with the \"Multi Effect\" visualizer."));
    const KaraokePanel::Ziel ziel{host, &fenster.renderMutex(), fenster.dienste().tryResolve<IEventBus>(),
                                  fenster.window()};
    QStringList bericht;
    if (!KaraokePanel::ladeUndZeigeKaraokeDatei(ziel, datei, &bericht))
        return fehler(QStringLiteral("Could not load %1: %2")
                          .arg(QDir::toNativeSeparators(datei), bericht.join(QStringLiteral(" | "))));
    return ok(bericht.isEmpty()
                  ? QString()
                  : QStringLiteral("Loaded with remarks: %1").arg(bericht.join(QStringLiteral(" | "))));
}

Ergebnis titelLaden(VisualizerWidget& fenster, const QString& root, const QJsonObject& argumente)
{
    const QString angabe = argument(argumente, QStringLiteral("datei"));
    if (angabe.isEmpty()) return ungueltig(QStringLiteral("Argument datei=<path> is missing."));
    const QString datei = loeseAuf(angabe, root);
    if (datei.isEmpty())
        return ungueltig(QStringLiteral("File not found: %1 (relative paths start at %2)")
                             .arg(QDir::toNativeSeparators(angabe), QDir::toNativeSeparators(root)));
    auto* playlist = fenster.dienste().tryResolve<IPlaylist>();
    if (playlist == nullptr) return fehler(QStringLiteral("There is no playlist."));
    int platz = playlist->indexOf(datei);
    if (platz < 0) platz = playlist->addTrack(datei);
    if (platz < 0)
        return fehler(QStringLiteral("The playlist did not take %1.").arg(QDir::toNativeSeparators(datei)));

    const QString spielen = argumente.value(QStringLiteral("spielen")).toString().trimmed().toLower();
    if (spielen == QLatin1String("ja"))
    {
        auto* spieler = fenster.dienste().tryResolve<IAudioPlayer>();
        if (spieler == nullptr) return fehler(QStringLiteral("There is no player."));
        if (!spieler->playIndex(platz))
            return fehler(QStringLiteral("The player could not start %1.")
                              .arg(QDir::toNativeSeparators(datei)));
    }
    else if (!spielen.isEmpty() && spielen != QLatin1String("nein"))
    {
        return ungueltig(QStringLiteral("Expected: spielen=ja|nein"));
    }
    return ok();
}

Ergebnis player(VisualizerWidget& fenster, const QJsonObject& argumente)
{
    const QString was = argument(argumente, QStringLiteral("wert")).toLower();
    auto* spieler = fenster.dienste().tryResolve<IAudioPlayer>();
    if (spieler == nullptr) return fehler(QStringLiteral("There is no player."));
    if (was == QLatin1String("start"))
    {
        if (!spieler->hasTrack()) return fehler(QStringLiteral("The player has no track."));
        spieler->play();
        return ok();
    }
    if (was == QLatin1String("pause"))
    {
        spieler->pause();
        return ok();
    }
    if (was == QLatin1String("stopp") || was == QLatin1String("stop"))
    {
        spieler->stop();
        return ok();
    }
    return ungueltig(QStringLiteral("Expected: player start|pause|stopp"));
}

}  // namespace appaktion

void meldeAppAktionen(Anbindung& anbindung, VisualizerWidget& fenster)
{
    const QPointer<VisualizerWidget> ziel(&fenster);
    const QString root = anbindung.root();
    const auto ohneFenster = [] { return fehler(QStringLiteral("The visualizer window is gone.")); };

    anbindung.aktion(QStringLiteral("panel_zeigen"), QStringLiteral("Opens a panel and brings it to the front"),
                     QStringLiteral("name=<id or title>"),
                     [](const QJsonObject& argumente) { return appaktion::panelZeigen(argumente); });
    anbindung.aktion(
        QStringLiteral("preset_laden"),
        QStringLiteral("Loads a preset (.avs, .milk, .lvfx, .lvfx2) into the effect chain, without asking about unsaved changes"),
        QStringLiteral("datei=<path>"),
        [ziel, root, ohneFenster](const QJsonObject& argumente) -> Ergebnis {
            return ziel != nullptr ? appaktion::presetLaden(*ziel, root, argumente) : ohneFenster();
        },
        FragtNach);
    anbindung.aktion(QStringLiteral("karaoke"), QStringLiteral("Switch Karaoke on"), QStringLiteral("an|aus"),
                     [ziel, ohneFenster](const QJsonObject& argumente) -> Ergebnis {
                         return ziel != nullptr ? appaktion::karaoke(*ziel, argumente) : ohneFenster();
                     });
    anbindung.aktion(
        QStringLiteral("karaoke_laden"),
        QStringLiteral("Loads a karaoke preset (.lvkar) into the karaoke layer and switches it on, without any dialog"),
        QStringLiteral("datei=<path>"),
        [ziel, root, ohneFenster](const QJsonObject& argumente) -> Ergebnis {
            return ziel != nullptr ? appaktion::karaokeLaden(*ziel, root, argumente) : ohneFenster();
        },
        FragtNach);
    anbindung.aktion(QStringLiteral("titel_laden"), QStringLiteral("Puts a track into the playlist, optionally plays it"),
                     QStringLiteral("datei=<path> [spielen=ja]"),
                     [ziel, root, ohneFenster](const QJsonObject& argumente) -> Ergebnis {
                         return ziel != nullptr ? appaktion::titelLaden(*ziel, root, argumente)
                                                : ohneFenster();
                     });
    anbindung.aktion(QStringLiteral("player"), QStringLiteral("Transport of the player"),
                     QStringLiteral("start|pause|stopp"),
                     [ziel, ohneFenster](const QJsonObject& argumente) -> Ergebnis {
                         return ziel != nullptr ? appaktion::player(*ziel, argumente) : ohneFenster();
                     });
}

}  // namespace lumi::sichttest
