/**
 * @file AppAktionen.hpp
 * @brief Sichttest-Steuerung: die Aktionen, die nicht dem Composer gehoeren —
 *        Panels, Karaoke-Ebene, Playlist und Player
 *        (docs/ui/Composer_Konzept.md §12.2 Naht N20)
 *
 * Jede Aktion ist eine Funktion, die sich auch ohne Tester rufen laesst.
 * Keine oeffnet einen Dialog.
 *
 * Threads: GUI-Thread.
 *
 * Rueckbau (Konzept §12): Teil von `include/sichttest` / `src/sichttest`.
 */
#pragma once

#include "sichttest/Anbindung.hpp"

#include <QJsonObject>
#include <QString>

class VisualizerWidget;

namespace lumi::sichttest
{

namespace appaktion
{

/// `panel_zeigen name=<Kennung oder Titel>` — ein Panel oeffnen und nach vorn
/// holen (`composer`, `karaoke`, `player`, `playlist` … oder der Titel, wie er
/// im Programm steht, etwa „Effect Chain"). Gross und klein spielt keine Rolle.
[[nodiscard]] Ergebnis panelZeigen(const QJsonObject& argumente);
/// `preset_laden datei=<pfad>` — ein Preset (`.avs`, `.milk`, `.lvfx`,
/// `.lvfx2`) in die Effektkette laden, ueber den Ladeweg seiner Endung und
/// ohne Rueckfrage zur ungespeicherten Kette (Schalter `FragtNach`; Naht N24).
/// Die Aktion gibt den Auftrag ab: scheitert das Laden selbst, meldet LumiViz
/// das wie immer in einem eigenen Dialog — die Antwort an den Tester sagt es nicht.
[[nodiscard]] Ergebnis presetLaden(VisualizerWidget& fenster, const QString& root,
                                   const QJsonObject& argumente);
/// `karaoke an|aus` — der Schalter Karaoke on (derselbe wie im Panel Karaoke,
/// in View → Karaoke und auf Ctrl+K).
[[nodiscard]] Ergebnis karaoke(VisualizerWidget& fenster, const QJsonObject& argumente);
/// `karaoke_laden datei=<pfad>` — ein Karaoke-Preset (`.lvkar`) ohne Dialog in
/// die Karaoke-Ebene laden und sie einschalten. Ungespeichertes der Ebene geht
/// ohne Rueckfrage verloren (Schalter `FragtNach`).
[[nodiscard]] Ergebnis karaokeLaden(VisualizerWidget& fenster, const QString& root,
                                    const QJsonObject& argumente);
/// `titel_laden datei=<pfad> [spielen=ja]` — einen Titel in die Playlist
/// legen (steht er schon dort, bleibt es bei dem einen Eintrag) und auf
/// Wunsch abspielen.
[[nodiscard]] Ergebnis titelLaden(VisualizerWidget& fenster, const QString& root,
                                  const QJsonObject& argumente);
/// `player start|pause|stopp` — der Transport des Players. Fuehrt der
/// Composer den Ton, schweigt der Player dabei wie immer.
[[nodiscard]] Ergebnis player(VisualizerWidget& fenster, const QJsonObject& argumente);

}  // namespace appaktion

/// Die Aktionen der App bei der Sichttest-Steuerung anmelden. Vor
/// `Anbindung::verbinde()` rufen.
void meldeAppAktionen(Anbindung& anbindung, VisualizerWidget& fenster);

}  // namespace lumi::sichttest
