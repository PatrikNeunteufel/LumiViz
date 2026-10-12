/**
 * @file SampleFrage.hpp
 * @brief Loop: die Rueckfrage zu einer gewaehlten Tondatei — kopieren oder am
 *        Ort lassen (docs/audio/Loop_Konzept.md §4h, Entscheid S2 a)
 *
 * Gestellt im Instrument-Creator („File...") und im Drum-Editor („Sample
 * file..."). Die gemerkte Wahl steht in `Sample.hpp` (`sampleKopieWahl`).
 */
#pragma once

#include "loop/Sample.hpp"

#include <QString>

class QWidget;

namespace lumi::loop
{

/// Was mit der Tondatei `pfad` geschieht: `Kopieren` oder `AmOrt` — oder
/// `Fragen`, wenn der Dialog abgebrochen wurde. Gefragt wird nur, solange
/// keine Wahl gemerkt ist; eine Datei, die schon bei den eigenen Samples
/// liegt, gilt ohne Frage als kopiert. `ohneKopie` ist der Satz vor dem Pfad
/// („Without a copy the instrument names the file at"), `objektName` der Name
/// des Dialogs.
[[nodiscard]] SampleKopie waehleSampleKopie(QWidget* eltern, const QString& pfad, const QString& ohneKopie,
                                            const QString& objektName);

}  // namespace lumi::loop
