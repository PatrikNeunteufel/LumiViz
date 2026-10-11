/**
 * @file SichttestAktionen.hpp
 * @brief Loop: die Aktionen, die der Tester `Sichttest.exe` im Panel Loop
 *        ausloest (docs/Sichttest_Steuerung.md §3; docs/audio/Loop_Konzept.md §4c)
 *
 * Die Abschnitte der Sichttest-Liste zum Loop beginnen mit „Vorbereiten": einen
 * Loop laden, den Drum-Editor einer Basis oeffnen. Diese Handgriffe gibt es
 * hier als Aktionen — jede eine Funktion, die sich auch ohne Tester rufen
 * laesst (so prueft sie `test_LoopSpieler.cpp`).
 *
 * Threads: GUI-Thread. Keine Aktion oeffnet einen Dialog.
 */
#pragma once

#include "sichttest/Anbindung.hpp"

#include <QJsonObject>
#include <QString>

namespace lumi::loop
{

class LoopPanel;

namespace sichttestaktion
{

/// `loop_laden datei=<pfad>` — eine `.lvloop` ohne Dialog ins Panel Loop
/// laden; eine relative Datei gilt ab `root`. Danach **haelt** der Loop: ein
/// spielender wird angehalten, jeder Abschnitt beginnt im selben Zustand.
/// Ungespeichertes geht ohne Rueckfrage verloren (der Tester fragt vorher,
/// Schalter `FragtNach`). Bemerkungen des Ladens (ein fehlendes Sample) kommen
/// nicht in die Antwort — sie stehen im Bericht des Panels.
[[nodiscard]] lumi::sichttest::Ergebnis loopLaden(LoopPanel& panel, const QString& root,
                                                  const QJsonObject& argumente);
/// `loop_wiedergabe start|stopp` — Play und Stop des Panels Loop. `start`
/// spielt den ganzen Loop von vorn; hat der Composer den Ton, scheitert es
/// und sagt, warum (Entscheid L7 a).
[[nodiscard]] lumi::sichttest::Ergebnis wiedergabe(LoopPanel& panel, const QJsonObject& argumente);
/// `loop_editor basis=<Name>` — den Editor dieser Basis oeffnen, wie **Edit
/// base** am Clip: den Drum-Editor oder, bei einem Instrument-Muster, den
/// Instrument-Editor. Gibt es die Basis nicht, nennt die Antwort die, die
/// es gibt. `zeigen` false fuellt den Editor nur (fuer Tests ohne Fenster).
[[nodiscard]] lumi::sichttest::Ergebnis editor(LoopPanel& panel, const QJsonObject& argumente,
                                               bool zeigen = true);

/// Das Panel Loop des Programms — in einem Dock oder abgedockt. nullptr = es
/// gibt keines.
[[nodiscard]] LoopPanel* panel();

}  // namespace sichttestaktion

/// Die Aktionen des Loops bei der Sichttest-Steuerung anmelden. Vor
/// `Anbindung::verbinde()` rufen.
void meldeSichttestAktionen(lumi::sichttest::Anbindung& anbindung);

}  // namespace lumi::loop
