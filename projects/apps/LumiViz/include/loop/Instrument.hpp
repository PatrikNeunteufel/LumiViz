/**
 * @file Instrument.hpp
 * @brief Loop: gerechnete Instrumente — aus einem Satz Einstellungen und einer
 *        Tonhoehe wird ein Ton (docs/audio/Loop_Konzept.md §4f, Stufe 9,
 *        Entscheid I1 c)
 *
 * Ein Instrument ist ein Satz Einstellungen fuer einen Klangerzeuger:
 * Schwingung (der Funktionsgenerator), Huellkurve, Filter, Vibrato,
 * Verstimmung — dazu die gezupfte Saite als eigene Art. `rendereTon` rechnet
 * daraus einen Ton von gegebener Dauer **vorab** in einen `Klang`; der
 * Rechenkern spielt ihn wie den Schlag eines Drumkits. So bleibt der Kern
 * ohne Gedaechtnis, obwohl Filter und Saite eines haben.
 *
 * Derselbe Aufruf ergibt dieselben Werte: kein Zufall von aussen (das
 * Rauschen der Saite kommt aus einer festen Folge je Tonhoehe).
 *
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/Rechenkern.hpp"

#include <QString>
#include <QStringList>

#include <vector>

namespace lumi::loop
{

/// Die Schwingung eines Instruments. Die vier ersten sind ein Wellenzug ohne
/// Teiltoene ueber der halben Abtastrate; die Saite ist eine gezupfte Saite
/// (Karplus-Strong), die von selbst ausklingt.
enum class Schwingung : int
{
    Sinus = 0,
    Dreieck,
    Saege,
    Rechteck,
    Saite,
};

struct Instrument
{
    QString name;   ///< Kennung in der Datei: a–z, 0–9, `_`
    QString titel;  ///< wie es im Programm steht

    Schwingung schwingung = Schwingung::Sinus;
    /// Nur Sinus: die Anteile des 2., 3., … Teiltons (der Grundton zaehlt 1).
    std::vector<double> obertoene;
    int stimmen = 1;               ///< 1…3 gleiche Schwingungen uebereinander
    double verstimmungCent = 0.0;  ///< so weit liegen die aeusseren Stimmen neben dem Ton

    // --- Huellkurve ---
    double anstiegMs = 5.0;    ///< bis zur vollen Lautstaerke
    double abfallMs = 200.0;   ///< danach bis auf `halten`
    double halten = 0.7;       ///< 0…1, solange die Note dauert
    double ausklangMs = 150.0; ///< nach dem Ende der Note

    // --- Filter (Tiefpass, folgt der Tonhoehe) ---
    double filterTeilton = 0.0; ///< Grenze als Vielfaches des Grundtons; 0 = offen
    double filterHuelle = 0.0;  ///< so viele Vielfache oeffnet die Huellkurve dazu

    // --- Vibrato ---
    double vibratoHz = 0.0;
    double vibratoCent = 0.0;
    double vibratoEinsatzMs = 0.0;  ///< so lange waechst es an

    double saiteDaempfung = 0.996;  ///< nur Saite: 0.9…0.9999, groesser klingt laenger
    double laut = 1.0;              ///< Ausgleich zwischen den Instrumenten
};

/// Die mitgelieferten Instrumente, in der Reihenfolge der Auswahl:
/// Piano, Strings, Violin, Guitar, Bass, Organ, Lead.
[[nodiscard]] const std::vector<Instrument>& instrumente();
/// Das mitgelieferte Instrument mit dieser Kennung; nullptr = keins.
[[nodiscard]] const Instrument* findeInstrument(const QString& name);

/// Die Frequenz einer Tonhoehe (MIDI-Nummer): 69 = 440 Hz.
[[nodiscard]] double tonHz(int hoehe);
/// Der Name einer Tonhoehe: 60 = „C4", 61 = „C#4".
[[nodiscard]] QString tonName(int hoehe);

/// Ein Ton laenger als das wird abgeschnitten (wie ein Sample, `kKlangMaxSekunden`).
inline constexpr double kTonMaxSekunden = 30.0;

/// Einen Ton rechnen: `dauerMs` lang gehalten, danach der Ausklang des
/// Instruments; der Klang endet bei null. Stereo, beide Seiten gleich.
/// Leer = `rate` oder `dauerMs` nicht groesser als 0.
[[nodiscard]] Klang rendereTon(const Instrument& instrument, int hoehe, double dauerMs, int rate);

}  // namespace lumi::loop
