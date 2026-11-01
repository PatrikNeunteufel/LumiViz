/**
 * @file WaveMuster.hpp
 * @brief Loop: die Angaben eines Wave-Musters — eine Tondatei als dritte Art des
 *        Musters (docs/audio/Loop_Konzept.md §4p, Stufe 15b; Entscheide Patrik
 *        S168: WV1 a bis WV9 a, SK15-1 a)
 *
 * Ein Wave-Muster spielt einen **Ausschnitt** seiner Datei, ohne in sie einzugreifen
 * (WV3 a): `start`/`ende` in Sekunden der Datei, die **Lage** `versatz` in Schritten ab
 * dem Anfang des Musters (auch negativ — ein Auftakt). Dazu Gain und Blenden, ein
 * **Bezugstempo** mit *Follow tempo* (WV1-1 a), das Verfahren fuer Dauer und Tonhoehe
 * (WV6 a), die Spur **Autotune** aus Zielnoten (WV4 a, WV9 a) und nach *Bounce* die
 * **Herkunft** (WV7 a).
 *
 * Laenge (`Muster::takte`) und die Spuren Expression und Pitch (`Muster::zeitspuren`)
 * stehen am Muster wie bei einem Instrument. Hier steht nur das Modell — wie es
 * klingt, rechnet Stufe 15c. Kein Qt-Widget, keine Audio-Engine.
 *
 * `Wellenform` heisst im Loop schon die gerechnete Welle eines Clips (Freeze); daher
 * `WaveMuster`.
 */
#pragma once

#include <QString>

#include <memory>
#include <vector>

namespace lumi::loop
{

struct Loop;

/// So viele Takte traegt ein Wave-Muster hoechstens (WV3 a, von Claude gesetzt); die
/// uebrigen Muster behalten `kTakteMax`.
inline constexpr int kWaveTakteMax = 512;
/// Gain und Blenden (von Claude gesetzt, S168): Gain wie der Fader eines Kanals.
inline constexpr double kWaveGainMinDb = -60.0;
inline constexpr double kWaveGainMaxDb = 24.0;
inline constexpr double kWaveBlendeMaxMs = 10000.0;
/// Autotune: Strength 0 … 1, Speed (die Zeit bis zum Zielton) 0 … 500 ms, hoechstens so
/// viele Abschnitte (von Claude gesetzt, S168).
inline constexpr double kAutotuneSpeedMaxMs = 500.0;
inline constexpr int kAutotuneAbschnitteMax = 4096;
/// Der Zielton eines Abschnitts: eine MIDI-Nummer und Cent daneben, bis zum halben
/// Halbton (von Claude gesetzt, S169 — weiter waere der Nachbarton).
inline constexpr int kAutotuneCentMax = 50;

/// Das Verfahren fuer Dauer und Tonhoehe (WV6 a, *Pitch method*): *General*
/// (Signalsmith Stretch, jedes Material) oder *Voice* (TD-PSOLA, einstimmiger Gesang).
enum class TonhoehenArt : int
{
    Allgemein = 0,  ///< *General* — die Vorgabe
    Stimme = 1,     ///< *Voice*
};

/// Die Tonart, nach der *Detect notes* Zielnoten vorschlaegt (WV4 a): ein Grundton und
/// eine Skala — oder chromatisch (jeder Halbton ist ein Ziel).
enum class Skala : int
{
    Chromatisch = 0,
    Dur = 1,
    Moll = 2,
};

/// Ein Abschnitt der Spur Autotune (WV4 a): von `anfang` bis `ende` (Sekunden der Datei)
/// zieht er die gesungene Tonhoehe zum Zielton.
struct AutotuneAbschnitt
{
    double anfang = 0.0;
    double ende = 0.0;
    int zielHoehe = 60;  ///< MIDI-Nummer, 0 … 127
    int zielCent = 0;    ///< −`kAutotuneCentMax` … `kAutotuneCentMax`
    double staerke = 1.0;  ///< Strength 0 … 1
    double speedMs = 0.0;  ///< Speed 0 … `kAutotuneSpeedMaxMs`

    [[nodiscard]] bool operator==(const AutotuneAbschnitt&) const = default;
};

/// Die Spur Autotune eines Wave-Musters: die Tonart, die Vorgaben fuer neue Abschnitte
/// und die Abschnitte. Ohne Abschnitt klingt der Ton wie gesungen.
struct Autotune
{
    int grundton = 0;  ///< 0 = C … 11 = H
    Skala skala = Skala::Chromatisch;
    /// Die Vorgabe am Muster fuer neue Abschnitte (von Claude gesetzt: 100 % und 20 ms —
    /// gezogen, aber ohne Sprung).
    double staerke = 1.0;
    double speedMs = 20.0;
    /// Nach `richteAutotune` nach Anfang sortiert, ohne Ueberlappung, hoechstens
    /// `kAutotuneAbschnitteMax`.
    std::vector<AutotuneAbschnitt> abschnitte;

    [[nodiscard]] bool operator==(const Autotune&) const = default;
};

/// Die Herkunft nach *Bounce* (WV7 a): der Ursprungsclip als kleiner Loop in der Gestalt
/// einer `.lvclip` (Austausch.hpp) — ein Kanal mit einem Clip ab Takt 0 und sein Muster.
/// Leer = keine Herkunft. Zwei Herkuenfte sind gleich, wenn ihre Loops gleich sind.
class Herkunft
{
public:
    Herkunft() = default;
    explicit Herkunft(Loop clip);

    [[nodiscard]] bool leer() const { return !m_clip; }
    /// Der kleine Loop; nur gueltig, wenn nicht `leer`.
    [[nodiscard]] const Loop& clip() const { return *m_clip; }

    [[nodiscard]] bool operator==(const Herkunft& andere) const;

private:
    /// Geteilt und unveraenderlich: ein Muster wird oft kopiert (Undo, Austausch), die
    /// Herkunft nie geaendert, nur ersetzt.
    std::shared_ptr<const Loop> m_clip;
};

/// Was ein Wave-Muster neben Laenge und Spuren traegt („Was festliegt“, §4p).
struct WaveAngaben
{
    /// Die Tondatei, genannt wie eine Sample-Schicht (WV2 a): `user:<name>`, `lib:<name>`,
    /// sonst ein Pfad (absolut). Leer = keine Datei, das Muster schweigt.
    QString datei;
    double start = 0.0;  ///< Sekunden der Datei, ab 0
    double ende = 0.0;   ///< Sekunden der Datei; 0 = bis zum Ende
    /// Die Lage in Schritten ab dem Anfang des Musters, mit Bruchteil, auch negativ (ein
    /// Auftakt, hoerbar ab dem Musteranfang).
    double versatz = 0.0;
    double gainDb = 0.0;     ///< `kWaveGainMinDb` … `kWaveGainMaxDb`
    double fadeInMs = 0.0;   ///< 0 … `kWaveBlendeMaxMs`
    double fadeOutMs = 0.0;  ///< 0 … `kWaveBlendeMaxMs`; schneidet auch am Musterende
    /// Das Tempo, in dem die Datei gespielt ist (WV1-1 a): beim Einfuegen das des Loops.
    double bezugBpm = 120.0;
    bool folgtTempo = false;  ///< *Follow tempo*: auf das Tempo des Loops gedehnt
    TonhoehenArt tonhoehenArt = TonhoehenArt::Allgemein;
    Autotune autotune;
    Herkunft herkunft;

    [[nodiscard]] bool operator==(const WaveAngaben&) const = default;
};

/// Die Abschnitte in ihre Grenzen: Anfang ab 0, ein Abschnitt ohne Laenge geht; nach
/// Anfang sortiert, der spaetere beginnt, wo der fruehere endet (wie Clips); Zielton,
/// Strength, Speed in ihre Grenzen; hoechstens `kAutotuneAbschnitteMax`.
void richteAutotune(Autotune& autotune);

/// Die Angaben in ihre Grenzen. `schritte` = die Laenge des Musters in Schritten — so weit
/// reicht die Lage in beide Richtungen (weiter waere nichts zu hoeren).
void richteWave(WaveAngaben& wave, double schritte);

/// Wie die Werte in einer Datei heissen: `general`/`voice`, `chromatic`/`major`/`minor`.
[[nodiscard]] QString tonhoehenArtName(TonhoehenArt art);
[[nodiscard]] TonhoehenArt tonhoehenArtAusName(const QString& name);
[[nodiscard]] QString skalaName(Skala skala);
/// Ein unbekannter Name: chromatisch.
[[nodiscard]] Skala skalaAusName(const QString& name);

}  // namespace lumi::loop
