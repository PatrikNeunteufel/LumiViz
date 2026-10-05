/**
 * @file Bearbeiten.hpp
 * @brief Composer: Clips einer Komposition verschieben, an den Kanten ziehen,
 *        loeschen (docs/ui/Composer_Konzept.md §6, Stufe C3a)
 *
 * Reine Operationen auf dem Modell — kein Widget, kein GL. Jede haelt die
 * Regel „Clips eines Kanals ueberlappen nicht" ein; was geschieht, wenn der
 * gezogene Clip auf einen Nachbarn trifft, bestimmt `Stoss` (Entscheid Patrik
 * S95): anhalten, den Nachbarn kuerzen oder die Nachbarn schieben.
 *
 * Rueckbau (Konzept §12): Teil von `include/composer` / `src/composer`.
 */
#pragma once

#include "composer/Komposition.hpp"

#include <cstdint>

namespace lumi::composer
{

/// Was geschieht, wenn der gezogene Clip auf einen Nachbarn im selben Kanal trifft.
enum class Stoss : int
{
    Halten = 0,    ///< der gezogene Clip haelt am Nachbarn an (ohne Taste)
    Kuerzen = 1,   ///< der gezogene Clip gewinnt, der Nachbar wird gekuerzt (Ctrl)
    Schieben = 2,  ///< die Nachbarn ruecken mit, ihre Laenge bleibt (Ctrl+Shift)
};

/// Kuerzer wird kein Clip gezogen; ein Nachbar, der beim Kuerzen darunter
/// fiele, verschwindet.
inline constexpr std::int64_t kMinClipMs = 100;

/// Den Clip auf `neuerStartMs` verschieben; seine Laenge und Kurve bleiben.
/// Liefert den Platz des Clips im Kanal danach (er kann sich aendern), -1 =
/// es gibt den Clip nicht.
int verschiebeClip(Komposition& komposition, int kanal, int clip, std::int64_t neuerStartMs,
                   Stoss stoss = Stoss::Halten);

/// Wie `verschiebeClip`, und der Clip wechselt dabei in den Kanal `nachKanal`
/// (nur Visual-Kanaele). Liefert seinen Platz im Zielkanal, -1 = Clip oder
/// Zielkanal gibt es nicht — dann bleibt alles, wie es war.
int versetzeClip(Komposition& komposition, int vonKanal, int clip, int nachKanal,
                 std::int64_t neuerStartMs, Stoss stoss = Stoss::Halten);

/// Die linke (`links` = true) oder rechte Kante des Clips auf `zeitMs` ziehen.
/// Die Punkte der Pegelkurve behalten ihren Abstand zu der Kante, der sie
/// naeher liegen. Liefert den Platz des Clips danach, -1 = gibt es nicht.
int zieheKante(Komposition& komposition, int kanal, int clip, bool links, std::int64_t zeitMs,
               Stoss stoss = Stoss::Halten);

/// Den Clip entfernen. false = es gibt ihn nicht.
bool loescheClip(Komposition& komposition, int kanal, int clip);

// --- Neues hinein (C3b) -------------------------------------------------------

/// Laenge eines hineingezogenen Clips, wenn nichts im Weg ist.
inline constexpr std::int64_t kNeuerClipMs = 20000;

/// Einen neuen Clip mit vollem Pegel in den Kanal legen. Mit `Stoss::Halten`
/// passt er sich der Luecke an, in die er faellt: er reicht hoechstens bis zum
/// naechsten Clip; faellt er auf einen Clip, landet er in der naechsten Luecke.
/// Kuerzen und Schieben wie beim Verschieben. Liefert seinen Platz, -1 = den
/// Kanal gibt es nicht (oder er ist kein Visual-Kanal).
int fuegeClipEin(Komposition& komposition, int kanal, const QString& datei, std::int64_t startMs,
                 std::int64_t dauerMs = kNeuerClipMs, Stoss stoss = Stoss::Halten);

/// Einen leeren Visual-Kanal anhaengen; ohne Namen heisst er „CH<n>" mit der
/// kleinsten freien Nummer. Liefert seinen Platz.
int fuegeKanalEin(Komposition& komposition, const QString& name = {});

/// Den Kanal samt seinen Clips entfernen. false = es gibt ihn nicht.
bool loescheKanal(Komposition& komposition, int kanal);

/// Eine neue, leere Komposition: ein Kanal „CH1", das Ende folgt den Clips.
[[nodiscard]] Komposition neueKomposition();

/// Start und Ende eines Clips aendern und seine Pegelkurve mitnehmen: jeder
/// Punkt behaelt den Abstand zu der Kante, der er naeher liegt.
void setzeGrenzen(VisualClip& clip, std::int64_t startMs, std::int64_t endeMs);

/// Sind zwei Kompositionen gleich (Kanaele, Clips, Kurven, Laenge, Raster)?
[[nodiscard]] bool gleich(const Komposition& a, const Komposition& b);

}  // namespace lumi::composer
