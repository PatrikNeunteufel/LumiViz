/**
 * @file Pedale.hpp
 * @brief InstrumentModels, Kern `pluckedString`: die Pedale der Harfe (K2 F-6 a; aus IC, seit 34k-7d-4 ohne das Spiel der
 *        Rechenfassung 1 — der Umfang kommt als zwei Tasten)
 *
 * Sieben Pedale stimmen je eine Stammtonklasse (C D E F G A H) ueber alle Oktaven: −1 ♭, 0 ♮, +1 ♯ (Wikipedia, nur
 * Bericht). Das Diagramm ist eine Passage-Spur mit Umschaltstellen; das Programm schlaegt es aus den Noten vor, der Nutzer
 * kann es aendern (enharmonische Wahl); was mit dem Diagramm nicht spielbar ist, wird markiert. Glissando und Bisbigliando
 * lesen es. Die Wechselzeit eines Pedals ist offen (keine Quelle) — der Vorschlag setzt die Umschaltstelle auf den Anfang
 * der Note, die sie braucht. In die Tafel kommt die Gruppe *Pedals* mit 34k-7d-6 (PD1 a, TM2 a).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include <array>
#include <vector>

namespace lumi::kern::zupf
{

/// Ein Pedaldiagramm: Stellung je Stammton C, D, E, F, G, A, H (Index 0 … 6).
using Pedale = std::array<int, 7>;

/// Eine Umschaltstelle der Pedal-Spur.
struct PedalStelle
{
    double abS = 0.0;
    Pedale pedale{0, 0, 0, 0, 0, 0, 0};
};

/// Eine Note einer Passage: ab `abS` gezupft.
struct PedalNote
{
    double abS = 0.0;
    int note = 60;
};

/// Die Tonhoehen aller Saiten der Harfe (Stammtoene in `noteMin` … `noteMax`) unter dem Diagramm.
[[nodiscard]] std::vector<int> harfenSaiten(int noteMin, int noteMax, const Pedale& p);
/// Die Saiten (als Stammton-Index 0 … 6), die unter dem Diagramm die Tonhoehe `note` geben — zwei: Bisbigliando moeglich.
[[nodiscard]] std::vector<int> saitenFuer(int note, const Pedale& p);
/// Spielbar unter dem Diagramm?
[[nodiscard]] bool spielbar(int note, const Pedale& p);
/// Ein Glissando von `von` nach `bis`: die Tonhoehen aller Saiten dazwischen unter dem Diagramm (in Spielrichtung).
[[nodiscard]] std::vector<int> glissandoNoten(int noteMin, int noteMax, int von, int bis, const Pedale& p);
/// Ein Diagramm, unter dem alle `noten` spielbar sind, mit den wenigsten Pedalen ausser ♮ (bei Gleichstand die erste in der
/// Suche); `ok` falsch, wenn es keins gibt (dann das beste mit den meisten Noten).
[[nodiscard]] Pedale pedaleVorschlag(const std::vector<int>& noten, bool* ok = nullptr);
/// Die Pedal-Spur fuer eine Passage: der Vorschlag laeuft die Noten in der Zeit ab und setzt eine Umschaltstelle, wo eine
/// Note mit dem bisherigen Diagramm nicht spielbar ist (nur das noetige Pedal wechselt).
[[nodiscard]] std::vector<PedalStelle> pedalSpurVorschlag(const std::vector<PedalNote>& noten);

}  // namespace lumi::kern::zupf
