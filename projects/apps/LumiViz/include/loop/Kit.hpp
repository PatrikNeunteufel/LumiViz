/**
 * @file Kit.hpp
 * @brief Loop: die Klaenge einer Spur finden und laden
 *        (docs/audio/Loop_Konzept.md, Entscheid L5 a, Stufe 5)
 *
 * Ein Klang ist ein Name aus dem mitgelieferten Kit (`kit:kick` — die Datei
 * `asset/loop/kit/kick.wav`) oder der Pfad einer Sample-Datei. Geladen wird
 * ueber die Audio-Engine: ganz dekodiert, Stereo, in der Abtastrate, in der
 * der Rechenkern rechnet.
 */
#pragma once

#include "audio/IAudioEngine.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QString>
#include <QStringList>

#include <vector>

namespace lumi::loop
{

/// Der Ordner des mitgelieferten Kits: `asset/loop/kit`, vom Programmordner
/// aufwaerts gesucht (Anker ist `asset/loop/README.md`, wie bei den anderen
/// Asset-Ordnern). Leer = nicht gefunden.
[[nodiscard]] QString kitOrdner();

/// Der Ordner der mitgelieferten Samples: `asset/loop/samples` (Entscheid 1 a,
/// Patrik S123), neben dem Kit. Leer = der Loop-Ordner ist nicht gefunden.
[[nodiscard]] QString bibliothekOrdner();

/// Die Datei zu einem Klang: fuer `kit:<Name>` die WAV-Datei im Kit, fuer
/// `lib:<Name>` die unter den mitgelieferten Samples, fuer `user:<Name>` das
/// eigene Sample, sonst der Klang selbst als Pfad. Leer = kein Klang, ein Name
/// mit unerlaubten Zeichen (erlaubt: a–z, 0–9, `_`), oder der Ordner ist nicht da.
[[nodiscard]] QString klangDatei(const QString& klang);

/// Die Klaenge des mitgelieferten Kits als `kit:<Name>`, nach Namen sortiert —
/// was im Ordner des Kits als WAV liegt. Leer = das Kit ist nicht da.
[[nodiscard]] QStringList kitKlaenge();

/// Die mitgelieferten Samples als `lib:<Name>`, nach Namen sortiert — was im
/// Ordner `samples` als WAV liegt. Leer = keine da.
[[nodiscard]] QStringList bibliotheksKlaenge();

/// So lang darf ein Sample hoechstens sein; was laenger ist, wird abgeschnitten.
inline constexpr double kKlangMaxSekunden = 30.0;

/// Die Klaenge zu den Namen laden (`Loop::klangNamen`, `Ablauf::klangNamen`),
/// in `rate` — je Name einer, in derselben Reihenfolge. Seit Stufe 29b aus dem
/// Tonspeicher (`loop/Tonspeicher.hpp`): gerechnet oder dekodiert wird nur, was
/// dort noch nicht liegt. Ein Klang, der sich nicht laden laesst, bleibt
/// `nullptr` (seine Spuren schweigen) und bekommt eine Zeile im `bericht`.
[[nodiscard]] Klaenge ladeKlaenge(const QStringList& namen, IAudioEngine& engine, int rate,
                                  QStringList* bericht = nullptr);

}  // namespace lumi::loop
