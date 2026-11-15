/**
 * @file Koerper.hpp
 * @brief Die Datei `.lvinstbody`: ein gerechneter Instrumentkoerper als JSON
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S147/S148: BM3 a, BM5 a,
 *        PL31c-5 a; Stufe 31c-5d)
 *
 * Was ein Koerper ist, steht in der Bibliothek `InstrumentBody` (`InstrumentBody/Koerper.hpp`).
 * Hier nur die Datei: lesen und schreiben mit Qt-JSON, einmal fuer die App und fuer das Werkzeug
 * `InstrumentBodyTool`, das diese Datei direkt mitkompiliert (BM5 a), dazu der Abdruck eines
 * Bauplans — der Waechter vergleicht ihn mit dem, den der Koerper traegt.
 *
 * Jedes Feld wird immer geschrieben. Zahlen schreibt Qt so kurz, dass sie genau
 * zurueckkommen: Hin- und Rueckweg geben denselben Koerper. Der Samen des statistischen Teils
 * steht als Text (64 Bit passen nicht in eine JSON-Zahl). Kein Widget, keine Qt-Anwendung noetig.
 */
#pragma once

#include "InstrumentBody/Bauplan.hpp"
#include "InstrumentBody/Koerper.hpp"

#include <QByteArray>
#include <QJsonObject>
#include <QString>

namespace lumi::loop
{

/// `format` im Koerper.
inline constexpr const char* kKoerperFormat = "lumiviz-instrument-body";

/// Der Koerper als JSON, alle Felder.
[[nodiscard]] QJsonObject alsJson(const koerper::Koerper& koerper);
/// Liest einen Koerper. false = kein Koerper (falsches `format`, kaputte Felder) oder eine Fassung,
/// die dieses Programm nicht kennt — dann ist `zuNeu` gesetzt; `koerper` bleibt, wie er war.
bool ausJson(const QJsonObject& wurzel, koerper::Koerper& koerper, bool* zuNeu = nullptr);
bool speichereKoerper(const koerper::Koerper& koerper, const QString& pfad);
bool ladeKoerper(const QString& pfad, koerper::Koerper& koerper, bool* zuNeu = nullptr);

/// Der Abdruck eines Bauplans: SHA-256 (hex) ueber sein JSON in kompakter Form — gleich, solange
/// sich am Bauplan nichts aendert, was in der Datei steht.
[[nodiscard]] QString bauplanAbdruck(const koerper::Bauplan& plan);

/// `format` in der Beschreibung eines Modalfits `.lvinstbodyfit` (KQ25 a): woraus ein gemessener Koerper stammt
/// (Datensatz, Kanaele, Lage von Schlag und Aufnehmern, Einstellungen des Fits, Stegpunkte, Hoerorte, Annahmen).
/// Geschrieben von `asset/loop/instrument_bodies/make_gitarre.py`; das Programm liest sie nicht, nur der Waechter.
inline constexpr const char* kFitFormat = "lumiviz-instrument-body-fit";
inline constexpr const char* kFitEndung = ".lvinstbodyfit";

/// Der Abdruck einer Beschreibung `.lvinstbodyfit`: SHA-256 (hex) ueber die Bytes der Datei ohne Wagenruecklauf —
/// so haengt er nicht an den Zeilenenden, mit denen Git die Datei auscheckt.
[[nodiscard]] QString messungAbdruck(const QByteArray& inhalt);

}  // namespace lumi::loop
