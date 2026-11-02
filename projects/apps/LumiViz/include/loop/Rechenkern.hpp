/**
 * @file Rechenkern.hpp
 * @brief Loop: aus einem Loop und einer Stelle werden Abtastwerte
 *        (docs/audio/Loop_Konzept.md, Entscheid L1 a)
 *
 * Zwei Schritte:
 *
 * - `plane` uebersetzt einen Loop fuer eine Abtastrate in einen `Ablauf`: je
 *   **Stimme** die Schlaege eines Durchlaufs, jeder mit seiner Stelle in
 *   Abtastwerten und seiner Lautstaerke links und rechts. Eine Stimme ist
 *   eine Spur eines Musters in einem Kanal — oder, bei einem
 *   Instrument-Muster, ein Ton (Tonhoehe und Dauer): seine Noten sind die
 *   Schlaege, sein Klang ist vorab gerechnet (`loop/Instrument.hpp`).
 * - `rechne` fuellt einen Block: was der Ablauf, endlos wiederholt, an den
 *   Abtastwerten `[von, von + frames)` hoeren laesst.
 *
 * `rechne` hat **kein Gedaechtnis**: derselbe Bereich ergibt dieselben Werte,
 * gleich in welchen Bloecken und in welcher Reihenfolge gerechnet wird. So
 * laesst sich die Stelle frei setzen (die Quelle im Mixer fuehrt sie selbst,
 * Beleg §6), und ein Test zaehlt die Werte ohne Tonausgabe nach. Es holt
 * keinen Speicher und liest keine Datei — es darf im Ton-Thread laufen.
 *
 * Ein neuer Schlag einer Stimme schneidet ihren noch klingenden ab, mit einer
 * Blende ueber `kSchnittFrames` (wie eine Drum-Maschine) — auch ueber die
 * Wiederholungen eines Clips und ueber Clips desselben Musters im Kanal
 * hinweg. Verschiedene Stimmen schneiden einander nicht. Die Stimme eines
 * eingefrorenen Clips (Stufe 30f) schneidet sich auch selbst nicht (FV1 a). Vor dem Abtastwert 0
 * gibt es nichts: der erste Durchlauf beginnt ohne Ausklang eines frueheren.
 *
 * Keine Audio-Engine.
 */
#pragma once

#include "loop/Loop.hpp"

#include <QStringList>

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace lumi::loop
{

/// So lange klingt ein abgeschnittener Schlag noch aus, in Abtastwerten.
/// Zwei Schlaege einer Stimme liegen immer weiter auseinander (bei 400 BPM
/// und 16 Schritten je Schlag sind es an 44100 Hz noch 413).
inline constexpr int kSchnittFrames = 64;

/// Ein Klang in der Abtastrate des Ablaufs: Stereo, links und rechts im Wechsel.
struct Klang
{
    std::vector<float> daten;
    [[nodiscard]] std::int64_t frames() const { return static_cast<std::int64_t>(daten.size() / 2); }
};

/// Die Klaenge eines Ablaufs, je Name einer (Stufe 29b, Entscheid TV4 a): geteilt mit
/// dem Tonspeicher und mit jedem anderen Programm, das denselben Ton spielt.
/// `nullptr` = kein Klang, die Stimme schweigt (wie ein leerer).
using Klaenge = std::vector<std::shared_ptr<const Klang>>;

/// Ein Schlag im Durchlauf.
struct Schlag
{
    std::int64_t frame = 0;  ///< Stelle im Durchlauf
    /// Lautstaerke: Schritt mal Spur, mit Panorama; ohne Bus auch mal Fader und Loop
    /// (mit Bus traegt der Bus beide, nach den Effekten — WX5 a)
    float links = 0.0f;
    float rechts = 0.0f;
    /// Index in `Ablauf::busse`, wenn der Clip des Schlags Effekte hat; -1 =
    /// der Schlag geht unbearbeitet in die Summe.
    int bus = -1;
    /// Index in `Ablauf::ausdruecke`, wenn sein Clip einen Verlauf Expression
    /// traegt (Stufe 28c-1); -1 = keiner, der Ton klingt, wie er ist.
    int ausdruck = -1;
    /// Index in `Ablauf::abgriffe`, wenn der Schlag ohne Bus in einem Kanal liegt, den ein
    /// Baustein als Steuersignal nennt (Stufe 15l); seine Lautstaerke traegt dann weder Fader
    /// noch Loop — beides traegt der Abgriff. -1 = keiner.
    int abgriff = -1;
};

/// Der Verlauf Expression eines Clips (Stufe 28c-1; Entscheide Patrik SF5 d, SK4 a,
/// SK9 a): die Spur des Musters ueber alle Wiederholungen und die Kurve am Clip, beide
/// 0 … 1, in Abtastwerten des Durchlaufs. Ein Ton des Clips wird mit dem Pegel an
/// seiner Stelle malgenommen — auch sein Nachklang, mit dem Wert am Ende des Clips.
struct Ausdruck
{
    /// Die Spur des Musters: je Durchlauf ihre Punkte, davor und dahinter der erste
    /// und der letzte Wert; zwei Eintraege an derselben Stelle sind ein Sprung (an
    /// der Naht). Leer = 1.
    std::vector<std::pair<std::int64_t, double>> muster;
    /// Die Kurve am Clip (SK8 a); leer = 1.
    std::vector<std::pair<std::int64_t, double>> clip;
    std::int64_t ende = 0;  ///< das Ende des Clips — dahinter gilt der Wert dort

    /// e = Muster mal Clip an `frame`; der Pegel einer Synth-Schicht ist e² (SK9 a:
    /// 40 · lg e dB, wie die MIDI-Empfehlung).
    [[nodiscard]] double wert(std::int64_t frame) const;
    [[nodiscard]] double pegel(std::int64_t frame) const
    {
        const double e = wert(frame);
        return e * e;
    }
};

/// Ein Clip mit eingeschalteten Effekten (Stufe 12, G1 b): seine Schlaege
/// kommen getrennt von der Summe heraus, damit seine Effekte nur ihn treffen.
struct Bus
{
    int kanal = 0;      ///< Index des Kanals im Loop
    int startTakt = 0;  ///< zusammen mit dem Kanal: welcher Clip
    ClipEffekte effekte;
    /// Der Fader des Kanals (Stufe 27e-1) mal die Lautstaerke des Loops (Stufe 30,
    /// WX5 a): wer den Bus nach den Effekten dazumischt, nimmt ihn mal diesen
    /// Faktor. Die Schlaege im Bus tragen beides nicht — so verstellt die
    /// Lautstaerke des Loops nicht die Zerrung eines Clips. In einem ausgeschalteten
    /// Kanal, der nur als Steuersignal klingt (Stufe 15l), 0.
    float faktor = 1.0f;
    /// Index in `Ablauf::abgriffe`, wenn sein Kanal als Steuersignal genannt ist (Stufe 15l):
    /// der Bus geht nach den Effekten in den Abgriff, nicht mit `faktor` in die Summe. -1 = nicht.
    int abgriff = -1;
    /// Je Baustein der Reihe der Index des Abgriffs, den er als Steuersignal bekommt
    /// (`steuerKanal`); -1 = keins. Leer = keiner hat eins.
    std::vector<int> steuer;
};

/// Ein Kanal, den ein Baustein als Steuersignal nennt (Stufe 15l, Entscheid Patrik WV5 a):
/// sein Klang hinter den Effekten seiner Clips, vor Fader und Schalter. Wer rechnet,
/// sammelt ihn in einem eigenen Block, gibt ihn den Bausteinen, die ihn nennen, und mischt
/// ihn mal `faktor` in die Summe.
struct Abgriff
{
    int kanal = 0;                ///< Index des Kanals im Loop
    float faktor = 0.0f;          ///< Fader mal Lautstaerke des Loops; 0, wenn der Kanal aus ist
    std::vector<int> busse;       ///< Indizes in `Ablauf::busse`: die Clips des Kanals mit Effekten
};

/// Eine Spur eines Musters in einem Kanal, mit allen ihren Schlaegen.
struct Stimme
{
    int klang = -1;                 ///< Index in `Ablauf::klangNamen`; -1 = kein Klang
    std::vector<Schlag> schlaege;   ///< nach Stelle aufsteigend
    /// Schneidet ein neuer Schlag den klingenden ab? Nicht bei einer eingefrorenen
    /// Wellenform (Stufe 30f, Entscheid FV1 a): ihr Nachklang klingt unter den
    /// naechsten Durchlaeufen zu Ende, wie Hall und Echo live ueber die Naht klingen.
    bool schneidet = true;
};

/// Ein Loop, fuer eine Abtastrate uebersetzt. Aendert sich der Loop, entsteht
/// ein neuer Ablauf — dieser bleibt, wie er ist.
struct Ablauf
{
    int rate = 0;
    double schrittFrames = 0.0;         ///< Laenge eines Schritts in Abtastwerten (nicht ganzzahlig)
    std::int64_t durchlaufFrames = 0;   ///< Laenge eines Durchlaufs; 0 = nichts zu spielen
    /// Die Klaenge, die der Ablauf braucht (`Loop::klangNamen`): wer `rechne`
    /// ruft, reicht sie in dieser Reihenfolge herein.
    QStringList klangNamen;
    std::vector<Stimme> stimmen;        ///< nur Stimmen mit Schlaegen
    double bpm = 0.0;                   ///< das Tempo des Loops — die Echo-Zeit haengt daran
    /// Je Clip mit eingeschalteten Effekten in einem Kanal, der an ist, einer
    /// — auch wenn der Clip gerade keinen Schlag hat: sein Nachklang soll
    /// nicht abreissen, wenn jemand den letzten Schritt loescht.
    std::vector<Bus> busse;
    /// Je Clip mit einem Verlauf Expression einer (Stufe 28c-1): ein Drumkit mit der
    /// Kurve am Clip, ein Instrument ohne Saiten dazu mit der Spur des Musters. Ein
    /// Instrument mit Saiten traegt Expression im Ton (Stufe 28c-2b, SK12 a:
    /// `TonAngabe::ausdruckMuster`, `ausdruckClip`) und hat hier keinen Eintrag.
    std::vector<Ausdruck> ausdruecke;
    /// Je Kanal, den ein wirksamer Baustein als Steuersignal nennt (auch mittelbar, auch
    /// ausgeschaltet), einer (Stufe 15l, WV5 a) — **in der Folge, in der sie zu rechnen
    /// sind**: ein Abgriff steht hinter jedem, dessen Signal seine Busse brauchen. Leer =
    /// keiner, alles wie bis Stufe 15k.
    std::vector<Abgriff> abgriffe;
};

/// Den Loop uebersetzen. Er muss gerichtet sein (`Loop::richte`). Kanaele,
/// die aus sind, und stumme Spuren haben keine Stimme; die Laenge des
/// Durchlaufs aendern sie nicht.
/// Panorama: in der Mitte beide Seiten voll, zur Seite faellt die andere ab.
/// Mit `saiten` (Stufe 23b, Entscheid S-A1 a) spielt ein Muster, dessen
/// Instrument Saiten traegt, je Vorkommen den Ton aus `toeneImKanal`
/// (Saitenplan.hpp) — mit Saite und Ende; die Namen stimmen mit
/// `loop.klangNamen(saiten)` ueberein. Ohne `saiten` wie bis Stufe 22.
/// Mit `eingefroren` (Stufe 30f) spielt ein eingefrorener Clip eine Stimme mit seiner
/// Wellenform: je Durchlauf ein Schlag am Anfang des Clips, mit Fader und Lautstaerke
/// des Loops (beide live, F3 a), ohne Bus und ohne Expression (beides steckt in der
/// Wellenform), ohne sich selbst abzuschneiden (FV1 a). Die Namen stimmen dann mit
/// `loop.klangNamen(saiten, eingefroren)` ueberein.
/// Ein Wave-Muster (Stufe 15c, WV8 a) ist je Kanal eine Stimme: je Wiederholung ein Schlag
/// am Anfang des Musters, ohne Swing und Panorama, mit Expression des Musters und am Clip.
/// Ein Kanal, den ein wirksamer Baustein als Steuersignal nennt (Stufe 15l), hat einen
/// Abgriff — auch ausgeschaltet (dann klingt er nicht, steuert aber); ohne solchen Baustein
/// rechnet alles Bit fuer Bit wie bis Stufe 15k.
[[nodiscard]] Ablauf plane(const Loop& loop, int rate, const InstrumentVon& saiten = {}, const Eingefroren& eingefroren = {});

/// `frames` Abtastwerte ab `von` nach `aus` schreiben (Stereo, `2 * frames`
/// Werte; der Block wird ueberschrieben). `klaenge[i]` ist der Klang zu
/// `ablauf.klangNamen[i]`; fehlt er, ist er `nullptr` oder leer, schweigt die Stimme.
///
/// `busAus`: je Eintrag in `ablauf.busse` ein eigener Block derselben Groesse
/// (auch er wird ueberschrieben) — die Schlaege eines Clips mit Effekten
/// landen dann dort statt in `aus`, unbearbeitet; wer ruft, rechnet die
/// Effekte und mischt dazu. Ohne `busAus` geht alles unbearbeitet nach `aus`.
///
/// `abgriffAus` (Stufe 15l): je Eintrag in `ablauf.abgriffe` ein Block — die Schlaege ohne
/// Bus eines genannten Kanals landen dort, ohne Fader und Loop; wer ruft, gibt die Busse
/// des Kanals dazu und mischt den Block mal `Abgriff::faktor`. Ohne `abgriffAus` gehen sie
/// mit diesem Faktor nach `aus`.
void rechne(const Ablauf& ablauf, const Klaenge& klaenge, std::int64_t von, int frames, float* aus,
            float* const* busAus = nullptr, float* const* abgriffAus = nullptr);

}  // namespace lumi::loop
