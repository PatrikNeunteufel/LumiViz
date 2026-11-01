/**
 * @file Dehner.hpp
 * @brief Loop: Dauer und Tonhoehe eines Klangs aendern (docs/audio/Loop_Konzept.md §4p;
 *        Entscheid Patrik WV6 a, Stufe 15f)
 *
 * Ein Wave-Muster wird vorab gerechnet (WV8 a). Seine Spur *Pitch* (Halbtoene ueber der Zeit)
 * und spaeter *Follow tempo* gehen durch einen `Dehner`: Stereo hinein, Stereo in einer neuen
 * Laenge heraus, die Tonhoehe je Block nach einer Kurve. Das Verhaeltnis der Laengen ist fest
 * ueber den ganzen Klang.
 *
 * Kein Typ einer fremden Bibliothek geht ueber diese Schnittstelle; die Umsetzung mit
 * Signalsmith Stretch steht in `Dehner.cpp` (`externals/signalsmith-stretch`), das eigene
 * TD-PSOLA fuer einstimmigen Gesang (WV6 a, Stufe 15h) in `Psola.cpp`. Die Tests laufen gegen
 * jede aus `dehnerUmsetzungen()`. Reines C++ ohne Qt.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace lumi::loop
{

/// So viele Abtastwerte rechnet ein Dehner mit derselben Tonhoehe (von Claude gesetzt): bei
/// 48 kHz rund 5 ms — feiner, als eine Kurve der Spur Pitch sich hoerbar aendert.
inline constexpr int kDehnerBlock = 256;

/// Dauer und Tonhoehe. Eine Umsetzung ist zustandslos (jeder Aufruf rechnet fuer sich) und darf
/// von mehreren Threads zugleich benutzt werden; zwei Aufrufe mit denselben Werten geben
/// dasselbe Ergebnis, Bit fuer Bit.
class Dehner
{
public:
    virtual ~Dehner() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    /// Bleiben die Formanten, wenn die Tonhoehe sich aendert? Dann folgt der Pegel der Huelle des
    /// Spektrums — ein Sinus wird nicht einfach ein hoeherer Sinus.
    [[nodiscard]] virtual bool behaeltFormanten() const { return false; }
    /// `stereo` (links und rechts im Wechsel, `rate` Abtastwerte je Sekunde) auf `ausFrames`
    /// Abtastwerte je Seite bringen; ist es so lang wie vorher, bleibt die Dauer. Die Tonhoehe in
    /// Halbtoenen gibt `halbtoene(k)` fuer Abtastwert k des Ergebnisses, gelesen je
    /// `kDehnerBlock` in der Mitte des Blocks; leer = 0. Ein leerer Klang gibt Stille der
    /// verlangten Laenge.
    [[nodiscard]] virtual std::vector<float> rechne(const std::vector<float>& stereo, int rate, std::int64_t ausFrames,
                                                    const std::function<double(std::int64_t)>& halbtoene) const = 0;
};

/// Jede Umsetzung; die erste ist die Vorgabe (*General*, WV6 a).
[[nodiscard]] std::vector<std::unique_ptr<Dehner>> dehnerUmsetzungen();

/// Die Vorgabe, einmal fuer das ganze Programm.
[[nodiscard]] const Dehner& dehner();

/// *Voice* (WV6 a): TD-PSOLA fuer einstimmigen Gesang, einmal fuer das ganze Programm.
[[nodiscard]] const Dehner& dehnerStimme();

/// Nur fuer den Pruefstein 15h (Bericht `docs/modellierung/wave-clips/proben/S170_pruefstein_15h.md`):
/// Signalsmith Stretch mit Formanterhalt (`setFormantFactor(1, true)`, Grundton je Block aus pYIN)
/// — der Gegenkandidat zu PSOLA, im Programm nicht gewaehlt (Urteil nach WV6 a).
[[nodiscard]] std::unique_ptr<Dehner> dehnerSignalsmithFormanten();

}  // namespace lumi::loop
