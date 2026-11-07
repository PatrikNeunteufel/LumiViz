/**
 * @file KernDatei.hpp
 * @brief Loop: die Werte eines Kerns in der Datei — `coreParams` einer Schicht, allgemein nach der Tafel
 *        (Instrumente_IC_Konzept.md IK2-4 a, IK2-11 a; Stufe 34d)
 *
 * Kein Code je Kern: was eine Tafel nennt, wird so geschrieben und gelesen —
 * - Zahl → Zahl, Ganzzahl → ganze Zahl, Schalter → `true`/`false`, Wahl → Text;
 * - Kurve → `{"points": [[note, wert], …], "overrides": [[note, wert], …], "evidence": "measured" | "source" |
 *   "computed" | "open", "source": "<Herkunft>"}` (bis Fassung 20 ohne `evidence` und `source` im Block
 *   `hammerString`);
 * - Liste → `[{…}, …]` je Eintrag mit den Feldern seiner Angabe.
 * Alle Werte werden immer geschrieben, in der Reihenfolge der Tafel. Die Abdruecke gestimmter Werte stehen daneben
 * unter `tuned` (`{"<name>": "<sha256>"}`), nur wenn es welche gibt.
 *
 * Gelesen wird ungerichtet: was die Datei nicht als die Art der Tafel traegt, fehlt danach — `richteWerte` meldet es.
 * Ein Name, den die Tafel nicht kennt, wird mitgelesen und von `richteWerte` gemeldet.
 *
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "InstrumentCore/Parametertafel.hpp"

#include <QJsonObject>
#include <QJsonValue>

namespace lumi::loop
{

/// Eine Kurve als JSON (mit Belegkraft und Herkunft) und zurueck (ungerichtet; fehlt `evidence`, gilt *Open*).
[[nodiscard]] QJsonObject kurveAlsJson(const kern::Kurve& kurve);
[[nodiscard]] kern::Kurve kurveAusJson(const QJsonValue& wert);

/// Die Werte nach der Tafel als `coreParams`; `tuned` dazu, wenn ein gestimmter Wert einen Abdruck hat.
[[nodiscard]] QJsonObject werteAlsJson(const kern::Parametertafel& tafel, const kern::Werte& werte);
/// `coreParams` nach der Tafel lesen (ungerichtet, danach `kern::richteWerte`).
[[nodiscard]] kern::Werte werteAusJson(const kern::Parametertafel& tafel, const QJsonObject& o);

}  // namespace lumi::loop
