/**
 ****************************************************************************************
 * @file   SongtextQuelle.hpp
 * @brief  Den Songtext zu einem Titel finden — lokale Quellen in fester Reihenfolge
 *
 * @details
 * Session 86, Entscheid Patrik 2a: **synchron vor unsynchron, nah am Song vor
 * fern.**
 *
 * | # | Quelle | gilt als |
 * |---|--------|----------|
 * | 1 | eingebettet synchron (ID3 SYLT; oder eingebetteter Text MIT Zeitmarken) | synchron |
 * | 2 | `<Song>.lrc` neben dem Song (mit Zeitmarken) | synchron |
 * | 3 | `<Interpret> - <Titel>.lrc` im Benutzerordner (mit Zeitmarken) | synchron |
 * | 4 | eingebettet unsynchron (ID3 USLT, Vorbis LYRICS …) | unsynchron |
 * | 5 | `<Song>.txt` neben dem Song — oder eine `.lrc` dort ohne Zeitmarken | unsynchron |
 * | 6 | `<Interpret> - <Titel>.txt` im Benutzerordner — oder `.lrc` ohne Zeitmarken | unsynchron |
 *
 * Synchron ist, was Zeitmarken TRAEGT — die Endung entscheidet nicht
 * (`LyricsModel.hpp`). Der Online-Abruf (Entscheid 3a, Opt-in) kommt als
 * siebte Stufe dahinter und legt seinen Fund ueber `speichereSongtext()` ab
 * (Entscheid 4a); dieselbe Funktion nimmt ein spaeteres Werkzeug zum
 * Nachsynchronisieren.
 ****************************************************************************************
 */

#pragma once

#include "audio/AudioEvents.hpp"
#include "visualizers/multieffect/LyricsModel.hpp"

#include <QString>

namespace lumi::audio
{

/// Benutzerordner fuer Songtexte: `<AppData>/LumiViz/lyrics` (wird nicht angelegt).
[[nodiscard]] QString songtextOrdner();

/// Dateiname OHNE Endung im Benutzerordner: `<Interpret> - <Titel>`, fehlt der
/// Interpret nur der Titel; fuer Dateinamen verbotene Zeichen werden zu `_`.
/// Leer, wenn der Titel fehlt.
[[nodiscard]] QString songtextName(const TrackInfo& track);

/// Songtext nach der Reihenfolge oben; leer (`leer()`), wenn nichts gefunden.
/// `Songtext::quelle` nennt die Herkunft (englisch, fuer die Anzeige).
[[nodiscard]] lumi::multieffect::lyrics::Songtext findeSongtext(
    const TrackInfo& track, const QString& benutzerOrdner = songtextOrdner());

/// Songtext als UTF-8-LRC nach `pfad` schreiben (Ordner wird angelegt).
bool speichereSongtext(const lumi::multieffect::lyrics::Songtext& text, const QString& pfad);

}  // namespace lumi::audio
