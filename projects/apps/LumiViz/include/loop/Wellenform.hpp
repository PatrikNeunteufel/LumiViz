/**
 * @file Wellenform.hpp
 * @brief Loop: ein Clip als Wellenform (docs/audio/Spielweisen_Konzept.md §4.4,
 *        Stufe 30a; Entscheide F3 a, WX1 a, WX3 a, WX4 a, WX5 a)
 *
 * Zwei Rechnungen, beide ohne Engine und ohne Oberflaeche, beide durch den
 * `Klangerzeuger` selbst — derselbe Weg wie live:
 *
 * - **Ein Clip im Loop-Panel** (F3 a): nur dieser Clip an seiner Stelle, ueber
 *   alle seine Wiederholungen, mit seinen Effekten und der Kurve am Clip — ohne
 *   Fader seines Kanals und ohne die Lautstaerke des Loops (beide wirken nach
 *   den Effekten, WX5 a, und bleiben live). Dahinter der **Nachklang**:
 *   ausklingende Toene, Hall, Echo. Damit dabei nichts zum Anfang umspringt
 *   (die Saitenplanung daempft sonst die letzten Toene mit den ersten), spielt
 *   die Rechnung einen Durchlauf, der laenger ist als der Nachklang.
 * - **Der ganze Loop im Loop-Panel** (WX7 a, WX9 a): wie das Panel ihn spielt, mit
 *   seinem Limiter — nahtlos oder mit Nachklang, ueber einen oder mehrere Durchlaeufe.
 * - **Ein Loop-Clip im Composer** (WX3 a): der Loop ab seinem Versatz ueber die
 *   Laenge des Clips, mit Fadern und Schaltern seiner Kanaele und seiner
 *   Lautstaerke — ohne Gain, Blenden und Pegelkurve des Composers, ohne Limiter
 *   und ohne Nachklang: die Datei endet, wo der Clip endet.
 *
 * Die Klaenge reicht herein, wer ruft (etwa ueber einen `Tonauftrag`), in der
 * Reihenfolge von `klangNamen` der Fassung, die gerechnet wird.
 */
#pragma once

#include "loop/Bearbeiten.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QStringList>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

namespace lumi::loop
{

/// Der Nachklang endet, wenn die Summe so lange unter dieser Grenze bleibt — hoechstens
/// so lange nach dem Ende des Clips (Stufe 30a, WX4 a; Werte von Claude gesetzt).
inline constexpr double kNachklangStilleDb = -96.0;
inline constexpr int kNachklangStilleMs = 500;
inline constexpr int kNachklangHoechstensMs = 30000;

/// Was `rechneClip` spielt: der Loop mit nur diesem Clip in seinem Kanal, Fader 0 dB,
/// Lautstaerke 1, nur sein Muster — und ein Durchlauf, der fuer den laengsten Nachklang
/// reicht (ein ausgeschalteter Kanal mit einem leeren Muster dahinter). Leer = den Clip
/// oder sein Muster gibt es nicht. Seine `klangNamen(saiten)` sind die Klaenge, die
/// `rechneClip` braucht.
[[nodiscard]] std::optional<Loop> clipFassung(const Loop& loop, ClipOrt ort);

/// Ein gerechneter Clip.
struct Wellenform
{
    Klang klang;                        ///< Stereo, links und rechts im Wechsel
    int rate = 0;
    std::int64_t clipFrames = 0;        ///< bis zum Ende des Clips; dahinter der Nachklang
    bool nachklangGekappt = false;      ///< nach `kNachklangHoechstensMs` noch nicht still
};

/// Fortschritt 0 … 1; false = abbrechen.
using Fortschritt = std::function<bool(double)>;

/// Den Clip an `ort` rechnen (F3 a, siehe oben). `klaenge` gehoeren zu
/// `clipFassung(loop, ort)->klangNamen(saiten)`. Leer = den Clip gibt es nicht, oder
/// `fortschritt` hat abgebrochen.
[[nodiscard]] std::optional<Wellenform> rechneClip(const Loop& loop, ClipOrt ort, int rate,
                                                   std::shared_ptr<const Klaenge> klaenge, const SaitenVon& saiten,
                                                   const Fortschritt& fortschritt = {});

/// Wie der ganze Loop endet (WX9 a).
enum class LoopEnde
{
    Nahtlos,    ///< Durchlaeufe, wie sie ab dem zweiten klingen — wiederholbar ohne Luecke
    Nachklang,  ///< aus der Stille, hinten klingt es aus
};

/// Hoechstens so viele Durchlaeufe am Stueck (*Passes*, WX9 a).
inline constexpr int kDurchlaeufeMax = 64;

/// Was `rechneLoop` mit Nachklang spielt: die Clips jedes Kanals `durchlaeufe`-mal
/// hintereinander (die Saitenplanung laeuft so ueber die Durchlaeufe wie im Kreis),
/// dahinter Platz fuer den laengsten Nachklang. Ohne Clips leer.
[[nodiscard]] std::optional<Loop> loopFassung(const Loop& loop, int durchlaeufe);

/// Die Klaenge, die `rechneLoop` braucht — je nach Ende andere (mit Nachklang die der
/// `loopFassung`).
[[nodiscard]] QStringList loopKlangNamen(const Loop& loop, LoopEnde ende, int durchlaeufe, const SaitenVon& saiten);

/// Den ganzen Loop rechnen, wie das Loop-Panel ihn spielt (WX7 a, WX9 a): mit Fadern,
/// Schaltern, Lautstaerke und — wenn er an ist — seinem Limiter. `Nahtlos`: ein Durchlauf
/// vorweg fuer Effekte und Limiter, verworfen, dann `durchlaeufe` Durchlaeufe. `Nachklang`:
/// ab der Stille, dahinter der Nachklang wie bei `rechneClip`; Effekte am Clip haben je
/// Durchlauf ihr eigenes Gedaechtnis (ein Drive ueber den Nachklang des vorigen hinweg
/// kann sich darum leicht von live unterscheiden). `clipFrames` = die Durchlaeufe.
/// `klaenge` gehoeren zu `loopKlangNamen`.
[[nodiscard]] std::optional<Wellenform> rechneLoop(const Loop& loop, LoopEnde ende, int durchlaeufe, int rate,
                                                   std::shared_ptr<const Klaenge> klaenge, const SaitenVon& saiten,
                                                   const Fortschritt& fortschritt = {});

/// Einen Loop-Clip des Composers rechnen (WX3 a): `loop` ab `versatzMs` ueber `laengeMs`,
/// wie die Tonfuehrung ihn spielt (der Durchlauf wiederholt sich). `klaenge` gehoeren zu
/// `loop.klangNamen(saiten)`. Leer = nichts zu rechnen, oder abgebrochen.
[[nodiscard]] std::optional<Wellenform> rechneAusschnitt(const Loop& loop, double versatzMs, double laengeMs, int rate,
                                                         std::shared_ptr<const Klaenge> klaenge,
                                                         const SaitenVon& saiten, const Fortschritt& fortschritt = {});

}  // namespace lumi::loop
