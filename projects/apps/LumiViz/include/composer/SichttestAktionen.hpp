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

/// Das Panel Composer zu diesem Fenster — im selben Fenster oder abgedockt.
/// nullptr = es gibt keines.
[[nodiscard]] ComposerPanel* panelFuer(VisualizerWidget& fenster);

}  // namespace sichttestaktion

/// Die Aktionen des Composers bei der Sichttest-Steuerung anmelden. Vor
/// `Anbindung::verbinde()` rufen.
void meldeSichttestAktionen(lumi::sichttest::Anbindung& anbindung, VisualizerWidget& fenster);

}  // namespace lumi::composer
