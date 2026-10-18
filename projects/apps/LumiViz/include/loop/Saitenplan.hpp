/**
 * @file Saitenplan.hpp
 * @brief Loop: welche Saite eine Note spielt und wann die naechste auf
 *        derselben Saite sie abdaempft (docs/audio/Saiten_Konzept.md, Stufe
 *        23b; Entscheide Patrik S132: S-M2 a, S-M2b, S-M3 a, S-A1 a)
 *
 * Zwei Schritte:
 *
 * - `greifeNoten` waehlt je Note eines Musters die Saite — nach der Regel des
 *   Musters (*Per chord* oder *Follow hand*), es sei denn, die Note traegt eine
 *   greifbare. Das haengt nur am Muster und an den Saiten.
 * - `toeneImKanal` macht daraus je **Vorkommen** einer Note in einem Kanal
 *   einen Ton (`TonAngabe`): mit Saite und dem Ende, an dem die naechste Note
 *   auf derselben Saite beginnt — ueber die Wiederholungen eines Clips, ueber
 *   Clips desselben Musters im Kanal und ueber das Ende des Durchlaufs hinweg
 *   (der Loop wiederholt sich).
 *
 * `Loop::klangNamen` und `plane` rechnen beide hiermit; so passen die Namen
 * der Klaenge und die Schlaege zusammen. Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/Loop.hpp"
#include "loop/Saiten.hpp"

#include <vector>

namespace lumi::loop
{

/// So lange blendet ein Ton aus, den die naechste Note auf derselben Saite
/// abdaempft — er ist still, wenn sie anschlaegt (S-M3 a; die Zahl von Claude
/// gesetzt).
inline constexpr int kAbdaempfenMs = 5;

/// Wie eine Note gegriffen wird.
struct Griff
{
    int saite = -1;         ///< die Stelle in `Saitensatz::leer`
    int bund = 0;           ///< ab dem Capo; nur bei `greifbar` in 0 … `kBuendeMax`
    /// false: auf keiner freien Saite greifbar (zu tief, zu hoch, mehr Noten
    /// zugleich als Saiten). Die Note klingt trotzdem, auf der naechstbesten
    /// Saite — sie daempft keine andere und wird von keiner gedaempft.
    bool greifbar = false;

    [[nodiscard]] bool operator==(const Griff&) const = default;
};

/// Je Note des Musters (in seiner Reihenfolge) der Griff. Ohne Saiten: lauter
/// leere Griffe. Das Muster muss gerichtet sein (`Loop::richte`).
///
/// Die Noten werden nach ihrem Anschlag gruppiert. Wer an einem Schritt
/// anschlaegt, bekommt verschiedene Saiten; Saiten von Noten, die noch
/// gehalten werden, sind belegt. Unter den Moeglichkeiten zaehlt zuerst der
/// kleinste Abstand der gegriffenen Buende (leere Saiten zaehlen nicht mit);
/// bei *Per chord* danach die tiefste Lage, bei *Follow hand* danach die Naehe
/// zur Lage des vorigen Anschlags, dann die tiefste.
[[nodiscard]] std::vector<Griff> greifeNoten(const Muster& muster, const Saitensatz& saiten);

/// Die Toene eines Clips: `[Wiederholung][Note]`.
using ClipToene = std::vector<std::vector<TonAngabe>>;

/// Je Clip des Kanals (in seiner Reihenfolge) die Toene — leer fuer einen Clip,
/// dessen Muster kein Instrument mit Saiten spielt oder fehlt. Ohne `saiten`
/// (leere Funktion) lauter leere.
[[nodiscard]] std::vector<ClipToene> toeneImKanal(const Loop& loop, std::size_t kanal, const SaitenVon& saiten);

}  // namespace lumi::loop
