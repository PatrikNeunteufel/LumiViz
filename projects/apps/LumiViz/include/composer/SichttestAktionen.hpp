/**
 * @file SichttestAktionen.hpp
 * @brief Composer: die Aktionen, die der Tester `Sichttest.exe` im Composer
 *        ausloest (docs/ui/Composer_Konzept.md §12.2 Naht N21)
 *
 * Die Sichttest-Listen beginnen jeden Abschnitt mit „Vorbereiten": Load...
 * einer Vorlage, Composer on, Edit, Fit. Diese Handgriffe gibt es hier als
 * Aktionen — jede eine Funktion, die sich auch ohne Tester rufen laesst (so
 * prueft sie `test_ComposerSichttestStart.cpp`).
 *
 * Threads: GUI-Thread. Keine Aktion oeffnet einen Dialog.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`; der
 * Aufruf im Hauptfenster ist Naht N21.
 */
#pragma once

#include "sichttest/Anbindung.hpp"

#include <QJsonObject>
#include <QString>

class VisualizerWidget;

namespace lumi::composer
{

class ComposerPanel;

namespace sichttestaktion
{

/// `komposition_laden datei=<pfad>` — eine `.lvcomp` ohne Dialog laden; eine
/// relative Datei gilt ab `root`. Danach ist der Composer aus, die Stelle
/// steht am Anfang, nichts laeuft (Entscheid Patrik L1: a) — ausser die
/// Einstellung `Sitzung::sichttestSpieltNachLaden` ist an: dann Composer on
/// und Abspielen. Ungespeichertes geht ohne Rueckfrage verloren (der Tester
/// fragt vorher, Schalter `FragtNach`). Bemerkungen des Ladens (ein fehlendes
/// Preset) kommen als Text der Antwort.
[[nodiscard]] lumi::sichttest::Ergebnis kompositionLaden(VisualizerWidget& fenster,
                                                         const QString& root,
                                                         const QJsonObject& argumente);
/// `composer an|aus` — der Schalter Composer on. Ohne geladene Komposition
/// laesst er sich nicht einschalten.
[[nodiscard]] lumi::sichttest::Ergebnis composer(VisualizerWidget& fenster,
                                                 const QJsonObject& argumente);
/// `edit an|aus` — der Schalter Edit des Panels Composer.
[[nodiscard]] lumi::sichttest::Ergebnis edit(VisualizerWidget& fenster, const QJsonObject& argumente);
/// `fit` — wie der Knopf Fit: die ganze Laenge auf der Breite, die Zeilen in Normalhoehe.
[[nodiscard]] lumi::sichttest::Ergebnis fit(VisualizerWidget& fenster);

/// `komposition_schliessen` — Composer aus und die Komposition aus dem Panel,
/// ohne Rueckfrage (Schalter `FragtNach` und `NurErstlauf`). Ist nichts
/// geladen, ist das erledigt.
[[nodiscard]] lumi::sichttest::Ergebnis kompositionSchliessen(VisualizerWidget& fenster);
/// `abspielmarke zeit=<m:ss>` — an eine Stelle springen (auf die Laenge
/// begrenzt); ob die Komposition laeuft, bleibt.
[[nodiscard]] lumi::sichttest::Ergebnis abspielmarke(VisualizerWidget& fenster,
                                                     const QJsonObject& argumente);
/// `wiedergabe start|pause|stopp` — der Transport des Composers. `start`
/// schaltet den Composer ein, wenn er aus ist; `stopp` haelt an und springt
/// an den Anfang.
[[nodiscard]] lumi::sichttest::Ergebnis wiedergabe(VisualizerWidget& fenster,
                                                   const QJsonObject& argumente);
/// `schleife an|aus` — Loop des Composers (bleibt ueber den Neustart).
[[nodiscard]] lumi::sichttest::Ergebnis schleife(VisualizerWidget& fenster,
                                                 const QJsonObject& argumente);
/// `sync_kette an|aus` — Sync on Effect Chain (bleibt ueber den Neustart).
[[nodiscard]] lumi::sichttest::Ergebnis syncKette(VisualizerWidget& fenster,
                                                  const QJsonObject& argumente);

/// Das Panel Composer zu diesem Fenster — im selben Fenster oder abgedockt.
/// nullptr = es gibt keines.
[[nodiscard]] ComposerPanel* panelFuer(VisualizerWidget& fenster);

}  // namespace sichttestaktion

/// Die Aktionen des Composers bei der Sichttest-Steuerung anmelden. Vor
/// `Anbindung::verbinde()` rufen.
void meldeSichttestAktionen(lumi::sichttest::Anbindung& anbindung, VisualizerWidget& fenster);

}  // namespace lumi::composer
