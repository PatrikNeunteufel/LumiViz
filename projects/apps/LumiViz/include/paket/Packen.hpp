/**
 * @file Packen.hpp
 * @brief Publish, Stufe P-A: ein Fund als Paket `<name>.<art>.lvdata`
 *        (docs/core-services/Publish_Konzept.md §3; Entscheide P3 a, P5 a,
 *        Patrik S125)
 *
 * Das Paket ist eine `.lvdata` in **Fassung 2** (ein Zip):
 *
 *     manifest.json                 wie Fassung 1, dazu der Block `package`
 *     data/<Bereich>/<name>.<endung> jede Zugehoerige — und das Hauptding,
 *                                    wenn seine Art einen eigenen Ordner hat
 *     package/<datei>               das Hauptding ohne eigenen Ordner (ein Loop)
 *     frozen/<abdruck>.wav          die Wellenform eines eingefrorenen Clips (Stufe 30i,
 *                                    FP1 a) — im Manifest unter `package.frozen`; ein
 *                                    Programm vor 30i uebergeht sie
 *
 * Die Dateien im Paket nennen einander schon so, wie sie beim Empfaenger
 * heissen: jede Nennung einer Zugehoerigen ist `user:<name>` (P5 a).
 * Mitgeliefertes und Fehlendes bleibt, wie es war. Ein Programm, das nur
 * Fassung 1 kennt, weist das Paket ab und meldet die neuere Fassung.
 */
#pragma once

#include "loop/Bearbeiten.hpp"
#include "loop/Loop.hpp"
#include "loop/Auskunft.hpp"  // InstrumentVon (HT5 a); bringt Saiten.hpp mit
#include "paket/Sammler.hpp"

#include <QByteArray>
#include <QString>

#include <vector>

namespace lumi::paket
{

/// Die Fassung der `.lvdata`, die ein Paket traegt; ein Export aller Daten
/// bleibt bei Fassung 1 (`core::datensicherung::kFassung`).
inline constexpr int kPaketFassung = 2;

/// Der Ordner einer Art im Benutzerordner (`instruments`, `samples`, `kits`,
/// `contours`); leer = die Art hat keinen (Loop).
[[nodiscard]] QString bereichOrdner(Art art);
/// Die Kennung des Bereichs der Datensicherung (`instruments`, `samples`,
/// `kits`, `shapes`); leer = keiner.
[[nodiscard]] QString bereichId(Art art);

/// Der vorgeschlagene Name des Pakets: `<name>.<art>.lvdata` — etwa
/// `titan.lvloop.lvdata`, `pump_bass.lvinst.lvdata`.
[[nodiscard]] QString paketName(const Fund& fund);
/// Wo das Hauptding im Paket liegt (`package/titan.lvloop`,
/// `data/instruments/pump_bass.lvinst`).
[[nodiscard]] QString hauptPfad(const Fund& fund);
/// Wo eine Zugehoerige im Paket liegt (`data/samples/snare_hit.wav`).
[[nodiscard]] QString paketPfad(const Zugehoerige& zugehoerige);

/// Ein eingefrorener Clip des Loops, wie Publish ihn sieht (Stufe 30i, FP1 a).
struct FrozenTeil
{
    lumi::loop::ClipOrt ort;
    QString etikett;      ///< „Bass · bar 5“ — der Kanal, sonst das Muster
    QByteArray abdruck;   ///< der jetzige, nicht der gespeicherte (FV2 a)
    QString datei;        ///< die Wellenform; leer = sie liegt nicht vor
    qint64 groesse = 0;
};

/// Je Clip mit Marke, nach Kanal und Takt: der jetzige Abdruck fuer `rate` und ob seine
/// Wellenform vorliegt. Schreibt nichts.
[[nodiscard]] std::vector<FrozenTeil> frozenTeile(const lumi::loop::Loop& loop, int rate,
                                                  const lumi::loop::InstrumentVon& saiten);
/// Dasselbe fuer die gespeicherte Datei (Publish nimmt sie). Leer = keine lesbare Loop-Datei.
[[nodiscard]] std::vector<FrozenTeil> frozenTeile(const QString& loopDatei, int rate,
                                                  const lumi::loop::InstrumentVon& saiten);

struct PackErgebnis
{
    bool ok = false;
    QString fehler;
    int dateien = 0;      ///< ohne Manifest, mit den Wellenformen
    qint64 groesse = 0;   ///< der Dateien vor dem Packen
    int frozen = 0;       ///< davon Wellenformen
};

/// Schreibt das Paket nach `ziel` (ganz oder gar nicht). `fund` muss `ok` sein.
/// `frozen`: die Abdruecke, deren Wellenform mitgeht (aus `frozenTeile`); jeder einmal,
/// eine, die nicht (mehr) vorliegt, bleibt draussen.
[[nodiscard]] PackErgebnis packe(const Fund& fund, const QString& ziel, const QString& appVersion,
                                 const std::vector<QByteArray>& frozen = {});
/// Wo die Wellenform zum Abdruck im Paket liegt (`frozen/<abdruck>.wav`).
[[nodiscard]] QString frozenPfad(const QByteArray& abdruck);

}  // namespace lumi::paket
