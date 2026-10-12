/**
 * @file Sample.hpp
 * @brief Loop: eigene Samples — wo sie liegen, wie eine Schicht sie nennt, und
 *        ihre geladenen Daten (docs/audio/Loop_Konzept.md §4h, Stufe 11a;
 *        Entscheide S1 a und S2 a, Patrik S115)
 *
 * Ein eigenes Sample liegt als Tondatei im Benutzerordner der App unter
 * `samples`, und eine Schicht nennt es als `user:<name>` — dieselbe Regel wie
 * bei den eigenen Instrumenten (P2 a). Eine Datei an freiem Ort steht beim
 * Pfad. Aufnahmen sind `<name>.wav`; eine hineinkopierte Datei behaelt ihre
 * Endung, der Name ohne Endung ist im Ordner eindeutig.
 *
 * Die Daten eines Samples werden einmal dekodiert und gemerkt — in
 * `kSampleRate`, Stereo. Dekodieren kann nur, wer eine Audio-Engine hat
 * (`ladeKlaenge` in Kit.hpp); `rendereTon` nimmt, was gemerkt ist.
 *
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/Rechenkern.hpp"

#include <QString>
#include <QStringList>

#include <memory>

namespace lumi::loop
{

/// Der Vorsatz eines eigenen Samples: `user:table_knock`.
inline constexpr const char* kSampleVorsatz = "user:";

/// Nennt `nennung` ein eigenes Sample (`user:<name>`)?
[[nodiscard]] bool istEigenesSample(const QString& nennung);
/// Der Ordner der eigenen Samples (wird nicht angelegt); leer = Qt nennt
/// keinen Benutzerordner.
[[nodiscard]] QString eigenerSampleOrdner();
/// Nur fuer Tests: ein anderer Ordner; leer = wieder der echte.
void setzeEigenenSampleOrdnerFuerTests(const QString& ordner);
/// Die Endungen, die im Ordner als Sample gelten (klein, ohne Punkt).
[[nodiscard]] const QStringList& sampleEndungen();
/// Die Datei eines eigenen Samples — die im Ordner liegt, mit welcher Endung
/// auch immer. Leer = kein gueltiger Name (`istEigenerName`), kein Ordner oder
/// keine solche Datei.
[[nodiscard]] QString eigeneSampleDatei(const QString& name);
/// Die eigenen Samples als `user:<name>`, nach Namen sortiert. Jedes Mal neu
/// gelesen. Dateien mit einem Namen, den keine Schicht nennen kann, fehlen.
[[nodiscard]] QStringList eigeneSamples();
/// Die Datei zu dem, was eine Schicht nennt: ein eigenes beim Namen, ein
/// Klang des mitgelieferten Kits (`kit:kick`, seit Stufe 11c), sonst die
/// Nennung selbst als Pfad. Leer = nichts genannt oder nicht gefunden.
[[nodiscard]] QString sampleDatei(const QString& nennung);
/// Wie ein Sample im Programm heisst: „table_knock" bzw. der Dateiname.
[[nodiscard]] QString sampleTitel(const QString& nennung);

/// Eine Tondatei zu den eigenen Samples kopieren (S2 a: „Copy the file to My
/// samples"). Der Name entsteht aus dem Dateinamen (`eigenerName`); liegt
/// unter ihm schon dieselbe Datei (gleicher Inhalt), wird sie genommen, bei
/// anderem Inhalt bekommt die Kopie eine Zahl dahinter (`cello_2`).
/// Leer = die Quelle fehlt, ihr Name ergibt keinen, oder das Schreiben
/// gelang nicht; sonst die Nennung `user:<name>`.
[[nodiscard]] QString nimmZuEigenen(const QString& quelle);
/// Ein eigenes Sample loeschen. false = es gab keins.
bool loescheEigenesSample(const QString& name);

/// Was mit einer ueber „File..." gewaehlten Tondatei geschieht (Entscheid
/// S2 a): gefragt wird, bis „Don't ask again" die Wahl merkt
/// (`loop/sampleCopyChoice`); in den Settings laesst sie sich aendern.
enum class SampleKopie : int
{
    Fragen = 0,
    Kopieren,  ///< zu den eigenen Samples, `user:<name>`
    AmOrt,     ///< die Datei bleibt, wo sie ist, und steht beim Pfad
};
[[nodiscard]] SampleKopie sampleKopieWahl();
void setzeSampleKopieWahl(SampleKopie wahl);

// --- die geladenen Daten ------------------------------------------------------

/// In dieser Abtastrate werden Samples dekodiert und gemerkt.
inline constexpr int kSampleRate = 48000;

/// Die Daten eines Samples, wie sie dekodiert wurden.
struct SampleDaten
{
    int rate = 0;  ///< Abtastrate von `klang`
    Klang klang;   ///< Stereo
};

/// Die gemerkten Daten zu einer Nennung; nullptr = (noch) nicht geladen, oder
/// die Datei hat sich seither geaendert — dann sind sie vergessen.
[[nodiscard]] std::shared_ptr<const SampleDaten> sampleDaten(const QString& nennung);
/// Daten merken — wer dekodiert hat (`ladeKlaenge`), eine Aufnahme, ein Test.
/// Leere Daten werden nicht gemerkt.
void merkeSampleDaten(const QString& nennung, int rate, std::vector<float> stereo);
/// Vergessen: eines, oder mit leerer Nennung alle.
void vergissSampleDaten(const QString& nennung = {});

}  // namespace lumi::loop
