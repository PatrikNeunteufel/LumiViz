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

/// Die Datei zu einem Klang: fuer `kit:<Name>` die WAV-Datei im Kit, sonst der
/// Klang selbst als Pfad. Leer = kein Klang, ein Name mit unerlaubten Zeichen
/// (erlaubt: a–z, 0–9, `_`), oder das Kit ist nicht da.
[[nodiscard]] QString klangDatei(const QString& klang);

/// So lang darf ein Sample hoechstens sein; was laenger ist, wird abgeschnitten.
inline constexpr double kKlangMaxSekunden = 30.0;

/// Je Spur des Loops ihren Klang laden, in `rate`. Eine Spur, deren Klang
/// sich nicht laden laesst, bekommt einen leeren Klang (sie schweigt) und eine
/// Zeile im `bericht`.
[[nodiscard]] std::vector<Klang> ladeKlaenge(const Loop& loop, IAudioEngine& engine, int rate,
                                             QStringList* bericht = nullptr);

}  // namespace lumi::loop
