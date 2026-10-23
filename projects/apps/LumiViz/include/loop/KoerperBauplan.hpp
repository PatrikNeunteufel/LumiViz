/**
 * @file KoerperBauplan.hpp
 * @brief Die Datei `.lvinstbodyplan`: der Bauplan eines Instrumentkoerpers als JSON,
 *        und wo die mitgelieferten liegen (docs/audio/Spielweisen_Konzept.md §4.1;
 *        Entscheide Patrik S147: BM3 a, BM3-1 a, BM3-2 a, BM5 a; Stufe 31c-1)
 *
 * Was ein Bauplan ist, steht in der Bibliothek `InstrumentBody`
 * (`InstrumentBody/Bauplan.hpp`, reines C++). Hier nur die Datei: lesen und
 * schreiben mit Qt-JSON, einmal fuer die App und fuer das Werkzeug
 * `InstrumentBodyTool`, das diese Datei direkt mitkompiliert (BM5 a).
 *
 * Mitgelieferte Bauplaene liegen unter `asset/loop/instrument_bodies/` neben dem
 * gerechneten Koerper (`.lvinstbody`) und heissen beim Namen der Datei ohne
 * Endung (`steinway_d`); eine `.lvinst` nennt den Koerper `lib:steinway_d`.
 * Eigene Koerper gibt es erst mit einem Editor.
 *
 * Jedes Feld wird immer geschrieben, auch wenn es leer ist (Memory „Datei-Felder
 * immer schreiben"). Kein Widget, keine Qt-Anwendung noetig.
 */
#pragma once

#include "InstrumentBody/Bauplan.hpp"

#include <QJsonObject>
#include <QString>

namespace lumi::loop
{

/// Die Endung eines Bauplans, mit Punkt (BM3-2 a).
inline constexpr const char* kBauplanEndung = ".lvinstbodyplan";
/// Die Endung eines gerechneten Koerpers, mit Punkt (BM3-2 a).
inline constexpr const char* kKoerperEndung = ".lvinstbody";
/// `format` im Bauplan.
inline constexpr const char* kBauplanFormat = "lumiviz-instrument-body-plan";

/// Der Bauplan als JSON, alle Felder.
[[nodiscard]] QJsonObject alsJson(const koerper::Bauplan& plan);
/// Liest einen Bauplan. false = kein Bauplan (falsches `format`) oder eine
/// Fassung, die dieses Programm nicht kennt — dann ist `zuNeu` gesetzt; `plan`
/// bleibt, wie er war. Ob der Bauplan in sich stimmt, sagt `koerper::pruefe`.
bool ausJson(const QJsonObject& wurzel, koerper::Bauplan& plan, bool* zuNeu = nullptr);
bool speichereBauplan(const koerper::Bauplan& plan, const QString& pfad);
bool ladeBauplan(const QString& pfad, koerper::Bauplan& plan, bool* zuNeu = nullptr);

/// Der Ordner der mitgelieferten Koerper: `asset/loop/instrument_bodies`.
/// Leer = nicht gefunden.
[[nodiscard]] QString mitgelieferterKoerperOrdner();

}  // namespace lumi::loop
