/**
 * @file Instrument.hpp
 * @brief Loop: gerechnete Instrumente — aus einem Satz Einstellungen und einer
 *        Tonhoehe wird ein Ton (docs/audio/Loop_Konzept.md §4f, Stufe 9,
 *        Entscheid I1 c)
 *
 * Ein Instrument besteht aus einer bis vier Schichten (Stufe 10c); jede ist
 * ein Satz Einstellungen fuer einen Klangerzeuger:
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

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <cstdint>
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

/// Eine Schicht eines Instruments: ein voller Satz Einstellungen fuer den
/// Klangerzeuger, dazu was nur die Schicht betrifft (Stufe 10c; Skizze
/// gebilligt, Patrik S111). Die Schichten eines Instruments werden beim
/// Rechnen eines Tons addiert.
struct Schicht
{
    QString name;                ///< wie sie in der Liste steht; leer = „Layer n"
    bool an = true;              ///< aus = stumm, bleibt aber im Instrument
    int versatzHalbtoene = 0;    ///< -24…24: so weit liegt die Schicht neben der Note
    double einsatzMs = 0.0;      ///< 0…1000: so lange nach dem Anschlag setzt sie ein (kein Echo)
    double verzerrung = 0.0;     ///< 0…1: der Verzerrer (Drive), 0 = aus (Entscheid E1 a)

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
    double laut = 1.0;              ///< die Lautstaerke der Schicht

    [[nodiscard]] bool operator==(const Schicht&) const = default;
};

/// So viele Schichten traegt ein Instrument hoechstens: jede wird je Ton
/// einzeln gerechnet.
inline constexpr int kSchichtenMax = 4;

struct Instrument
{
    /// Wie ein Muster es nennt: der Name eines mitgelieferten Instruments
    /// (`piano` — die Datei `asset/loop/instruments/piano.lvinst`) oder der
    /// Pfad einer eigenen Datei `.lvinst`. Steht nicht in der Datei.
    QString name;
    QString titel;      ///< wie es im Programm steht
    double laut = 1.0;  ///< die Lautstaerke des ganzen Instruments
    /// Mindestens eine, hoechstens `kSchichtenMax`.
    std::vector<Schicht> schichten = {Schicht{}};

    [[nodiscard]] bool operator==(const Instrument&) const = default;
};

// --- die Datei `.lvinst` (Entscheide Patrik S110, I2 b und I3 a; S111, F1 a) --
// Ein Instrument ist eine Datei, abgelegt wie die Klaenge des Kits: die
// mitgelieferten unter `asset/loop/instruments`, eigene irgendwo. Im Programm
// steht kein Instrument mehr.
//
// Fassung 2 traegt die Schichten als Liste `layers`; Fassung 1 ist flach und
// wird als Instrument mit einer Schicht gelesen. Geschrieben wird Fassung 2.

/// Die Fassung, die dieses Programm schreibt; eine hoehere liest es nicht.
inline constexpr int kInstrumentFassung = 2;

/// Das Instrument als JSON (ohne `name` — der ist, wie das Muster es nennt).
[[nodiscard]] QJsonObject alsJson(const Instrument& instrument);
/// Liest ein Instrument, Werte in ihren Grenzen. false = kein Instrument
/// (falsches Format) oder eine Fassung, die dieses Programm nicht kennt — dann
/// ist `zuNeu` gesetzt; `instrument` bleibt, wie es war. `name` bleibt.
bool ausJson(const QJsonObject& wurzel, Instrument& instrument, bool* zuNeu = nullptr);
bool speichereInstrument(const Instrument& instrument, const QString& pfad);
/// Liest die Datei; `name` wird der Pfad. Ohne Titel in der Datei gilt ihr Name.
bool ladeInstrument(const QString& pfad, Instrument& instrument, bool* zuNeu = nullptr);

/// Der Ordner der mitgelieferten Instrumente: `asset/loop/instruments`, neben
/// dem Kit (`kitOrdner`). Leer = nicht gefunden.
[[nodiscard]] QString instrumentOrdner();
/// Die mitgelieferten Instrumente — was im Ordner als `.lvinst` liegt, nach
/// Titel sortiert; `name` ist der Dateiname ohne Endung. Einmal gelesen und
/// gemerkt. Leer = der Ordner ist nicht da.
[[nodiscard]] const std::vector<Instrument>& instrumente();
/// Das mitgelieferte Instrument mit diesem Namen; nullptr = keins.
[[nodiscard]] const Instrument* findeInstrument(const QString& name);
/// Das Instrument, das ein Muster nennt: ein mitgeliefertes beim Namen oder
/// eine eigene Datei beim Pfad (`istInstrumentDatei`). false = nicht gefunden
/// oder nicht lesbar; `zuNeu` = die Datei traegt eine Fassung, die dieses
/// Programm nicht kennt.
bool holeInstrument(const QString& nennung, Instrument& instrument, bool* zuNeu = nullptr);
/// Zaehlt jede gespeicherte Instrument-Datei (`speichereInstrument`): wer
/// Toene gerechnet hat und sich die Zahl merkt, sieht, ob er neu rechnen muss.
[[nodiscard]] std::uint64_t instrumentFassung();

/// So viele Obertoene traegt ein Instrument hoechstens (der 2. bis 17. Teilton).
inline constexpr int kObertoeneMax = 16;

/// Die Frequenz einer Tonhoehe (MIDI-Nummer): 69 = 440 Hz.
[[nodiscard]] double tonHz(int hoehe);
/// Der Name einer Tonhoehe: 60 = „C4", 61 = „C#4".
[[nodiscard]] QString tonName(int hoehe);

/// Ein Ton laenger als das wird abgeschnitten (wie ein Sample, `kKlangMaxSekunden`).
inline constexpr double kTonMaxSekunden = 30.0;

/// Einen Ton rechnen: `dauerMs` lang gehalten, danach der Ausklang des
/// Instruments; der Klang endet bei null. Stereo, beide Seiten gleich.
/// Die eingeschalteten Schichten werden addiert — jede an ihrer Tonhoehe und
/// ab ihrem Einsatz; der Klang ist so lang wie die laengste. Ist keine an,
/// ist er still. Leer = `rate` oder `dauerMs` nicht groesser als 0.
[[nodiscard]] Klang rendereTon(const Instrument& instrument, int hoehe, double dauerMs, int rate);

}  // namespace lumi::loop
