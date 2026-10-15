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
 *
 * Die Dateien im Paket nennen einander schon so, wie sie beim Empfaenger
 * heissen: jede Nennung einer Zugehoerigen ist `user:<name>` (P5 a).
 * Mitgeliefertes und Fehlendes bleibt, wie es war. Ein Programm, das nur
 * Fassung 1 kennt, weist das Paket ab und meldet die neuere Fassung.
 */
#pragma once

#include "paket/Sammler.hpp"

#include <QString>

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

struct PackErgebnis
{
    bool ok = false;
    QString fehler;
    int dateien = 0;      ///< ohne Manifest
    qint64 groesse = 0;   ///< der Dateien vor dem Packen
};

/// Schreibt das Paket nach `ziel` (ganz oder gar nicht). `fund` muss `ok` sein.
[[nodiscard]] PackErgebnis packe(const Fund& fund, const QString& ziel, const QString& appVersion);

}  // namespace lumi::paket
