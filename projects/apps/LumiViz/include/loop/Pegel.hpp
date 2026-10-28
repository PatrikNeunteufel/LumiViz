/**
 * @file Pegel.hpp
 * @brief Loop: wie laut ein Instrument oder ein Drum-Klang ist — die
 *        Standardprobe und ihre Messung nach EBU R128 (Loop_Konzept §4n,
 *        Entscheide Patrik S139, LL2 a und LL4 a; Stufe 27b)
 *
 * Ohne Engine und ohne Fenster. Ein Instrument wird mit der **Standardprobe**
 * gemessen: eine Tonleiter ueber zwei Oktaven und ein Akkord, gerechnet wie
 * das Programm einen Loop rechnet (`plane` mit den Saiten des Instruments,
 * Klaenge aus `rendereTon`, `rechne`); das Mass ist das **Short-term-
 * Maximum** (3 s). Ein Drum-Klang wird mit **einem Schlag** gemessen, das
 * Mass ist das **Momentary-Maximum** (0,4 s). Sample-Schichten spielen, was
 * zu ihrer Aufnahme gemerkt ist (`sampleDaten`, Sample.hpp) — wer misst,
 * sorgt dafuer, dass es gemerkt ist.
 */
#pragma once

#include "audio/Lautheit.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <optional>
#include <vector>

namespace lumi::loop
{

/// Der Bezug (Entscheide Patrik S139, LL2 a, LL7 a): so laut misst die
/// Standardprobe jedes mitgelieferten Instruments ausser den Drums — der Median
/// vor Stufe 27c, Short-term-Maximum bei Loop 1. `asset/loop/instrument_pegel.py`
/// traegt denselben Wert (`BEZUG_LUFS`; ein Test vergleicht beide).
inline constexpr double kInstrumentBezugLufs = -13.4;

/// So weit darf ein Instrument vom Bezug abweichen, ohne dass der Creator fragt
/// oder das Fenster *Instruments* es markiert (Stufe 27f, Entscheid Patrik LL14 a).
inline constexpr double kBezugToleranzDb = 1.0;

/// Die Rate, in der der Creator und das Fenster *Instruments* messen (27f) — eine,
/// damit beide denselben gemerkten Wert finden (IP1 a, CR1 a).
inline constexpr int kPegelRate = 44100;

/// Wie weit die Standardprobe von `instrument` vom Bezug abweicht, in dB
/// (positiv = lauter als die anderen). Leer = nicht zu messen: die Art `drums`
/// (sie folgen den Kits, LL10 a), eine Sample-Schicht, deren Aufnahme nicht
/// gemerkt ist (sie schwiege und verfaelschte die Messung), oder Stille.
[[nodiscard]] std::optional<double> abstandZumBezug(const Instrument& instrument, int rate);
/// Das Volume, mit dem `instrument` auf dem Bezug laege — begrenzt auf
/// 0 … `kInstrumentLautMax`. Leer wie oben.
[[nodiscard]] std::optional<double> lautAufBezug(const Instrument& instrument, int rate);

/// Ob `abstandZumBezug` dieses Instrument misst: nicht die Art `drums`, nicht mit
/// einer Sample-Schicht, deren Aufnahme nicht gemerkt ist.
[[nodiscard]] bool pegelMessbar(const Instrument& instrument);

/// Wie `abstandZumBezug`, aber gemerkt (Entscheid Patrik IP1 a, S149): unter dem
/// Abdruck des Instruments (`instrumentAbdruck`, wie im Tonspeicher), der
/// Klangfassung und der Rate. Ein unveraendertes Instrument wird nur einmal
/// gemessen. Nicht gemerkt wird, was `pegelMessbar` ablehnt — eine Aufnahme kann
/// spaeter gemerkt sein. Von jedem Thread aus.
[[nodiscard]] std::optional<double> abstandZumBezugGemerkt(const Instrument& instrument, int rate);
/// Was gemerkt ist, ohne zu messen: aussen leer = nichts gemerkt.
[[nodiscard]] std::optional<std::optional<double>> gemerkterAbstand(const Instrument& instrument, int rate);
/// So oft hat `abstandZumBezugGemerkt` wirklich gemessen (fuer Tests).
[[nodiscard]] int gemesseneAbstaende();
/// Alles Gemerkte vergessen (fuer Tests).
void vergissGemerkteAbstaende();

/// Die Standardprobe als Loop: 120 BPM, 4/4, Sechzehntel; eine Dur-Tonleiter
/// ueber zwei Oktaven (15 Toene, je zwei Schritte, Lautstaerke 0,9), zwei
/// Schritte Pause, dann der Akkord Grundton, Terz, Quinte, Oktave ueber einen
/// Takt (0,7); dahinter ein leerer Takt fuer den Ausklang. Grundton C4, fuer
/// die Art `bass` C2. Loop und Muster mit Lautstaerke 1, Muster in der Mitte.
[[nodiscard]] Loop standardProbe(const QString& instrument, const QString& art);

/// Die Standardprobe von `instrument` rechnen — ein Durchlauf, Stereo, in
/// `rate`. Die Planung bekommt die Saiten dieses Instruments (auch wenn es
/// noch keine Datei hat), jeder Ton wird mit ihm gerechnet.
[[nodiscard]] std::vector<float> rechneStandardprobe(const Instrument& instrument, int rate);

/// Die Standardprobe messen; das Mass fuer den Bezug ist `kurzzeitMaxLufs`.
[[nodiscard]] audio::Lautheit messeInstrument(const Instrument& instrument, int rate);

/// Einen Schlag messen: der Klang einmal, dahinter eine Sekunde Stille (damit
/// auch ein kurzer Klang das Fenster von 0,4 s fuellt); das Mass fuer den
/// Bezug ist `momentanMaxLufs`.
[[nodiscard]] audio::Lautheit messeSchlag(const Klang& klang, int rate);

/// Ein Instrument als Drum-Klang: einmal auf C2, `kSpurDauerMs` lang — wie
/// eine Drum-Spur es spielt — und als Schlag gemessen.
[[nodiscard]] audio::Lautheit messeDrumInstrument(const Instrument& instrument, int rate);

/// Die Standardprobe fuer Drums (Stufe 27d, Entscheid Patrik LL10 a): `schlag`
/// auf jedem Viertel, zwei Takte bei 120 BPM, dahinter ein leerer Takt —
/// gerechnet wie ein Loop (`plane`, `rechne`), Spur und Loop mit Volume 1.
/// Gemessen wie die Probe der Instrumente: das Mass ist `kurzzeitMaxLufs`.
/// So liegt die Kick eines Kits auf derselben Skala wie ein Instrument.
[[nodiscard]] std::vector<float> rechneDrumProbe(const Klang& schlag, int rate);
[[nodiscard]] audio::Lautheit messeDrumProbe(const Klang& schlag, int rate);

}  // namespace lumi::loop
