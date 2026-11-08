/**
 * @file Tonkern.hpp
 * @brief InstrumentCore: die eine Ton-Schnittstelle der Rechenkerne — `Tonkern`, die Auskunft je Kern und die Fabrik
 *        je Name (Instrumente_IC_Konzept.md IK2-0 a, IK2-3 a, IK2-4 a, IK2-10 a)
 *
 * Ein **Kern** ist ein Rechenverfahren (Flügel, Stab, Membran, Zunge …). Seine Art beschreibt ein `KernTyp`: die
 * Parametertafel, die Auskunft und `erzeuge`, das aus gerichteten Werten einen `Tonkern` baut — dabei rechnet der
 * Kern seine Vorbereitung einmal (`vorbereite(Spiel)` in IK2-3 a: `bereite`, und was beim Laden zu rechnen ist). Ein
 * `Tonkern` gehoert zu einem Instrument, ist danach unveraenderlich und darf von mehreren Threads zugleich rechnen; die
 * App haelt ihn als `std::shared_ptr<const Tonkern>` und merkt ihn unter dem Abdruck des Instruments.
 *
 * Die Kerne selbst stehen nicht hier, sondern in der Bibliothek `InstrumentModels` (BK1 a): je Klasse ein Adapter, der
 * ihr `Spiel` haelt und ihr `rechne<X>Ton` ruft (der Fluegel: `InstrumentModels/hammer/HammerKern.hpp`), dazu die Fabrik
 * der eingebauten Kerne (`InstrumentModels/Kerne.hpp`). Diese Bibliothek kennt keinen Kern; die Tests bauen auch eigene
 * Fabriken.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Parametertafel.hpp"
#include "InstrumentCore/Tonauftrag.hpp"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace lumi::kern
{

/// Ueber welche Spanne ein Kern rechnet (IC T2 a).
enum class Rechenweg : int
{
    Note = 0,   ///< ein Ton je Note
    Kette = 1,  ///< ein Ton ueber mehrere gebundene Noten
    Clip = 2,   ///< die Noten eines Clips in einem Stueck (Zustand ueber die Noten)
};

/// Was die Planung ueber einen Kern wissen muss, ohne ihn zu erzeugen (ersetzt in 34d `InstrumentAuskunft::hammer`).
struct KernAuskunft
{
    Rechenweg rechenweg = Rechenweg::Note;
    /// die Anschlagstufe gehoert zum Namen des Tons (`:a<stufe>`) — der Kern rechnet je Stufe anders
    bool stufeImNamen = true;
    /// die Spuren, die der Kern liest (`spur::kExpression` …); der Editor zeigt genau diese (§6a)
    std::vector<std::string> spuren;
    /// die inneren Groessen, die der Kern auf Wunsch abgibt (Kontaktkraft, Moden …)
    std::vector<std::string> innere;
    /// die Vorbereitung (`erzeuge`) dauert spuerbar (Moden rechnen, stimmen) — die App rechnet sie im Hintergrund
    bool vorbereitungTeuer = false;
    /// der Koerper liegt ausserhalb des Kerns, an der Schicht (FU1 a: die App faltet ihn — der Fluegel; Klasse 2 mit
    /// `korpusImpulsantwort`); sonst rechnet der Kern ihn selbst (Klassen 4 und 5, IK2-10 a) und der Creator zeigt
    /// keinen Kasten *Body* (CK1 a, §6a 1; 34g)
    bool koerperAnDerSchicht = false;

    [[nodiscard]] bool operator==(const KernAuskunft&) const = default;
};

/// Eine innere Groesse beim Rechnen (IK2-3 a: „auf Wunsch als Daten, alles Messen am Ausgang in Messung“).
struct InnereGroesse
{
    std::string name;
    std::string einheit;
    std::vector<double> werte;
};

/// Die inneren Groessen, die ein Aufrufer haben will, und was der Kern dazu liefert.
struct Innere
{
    std::vector<std::string> gewuenscht;
    std::vector<InnereGroesse> ergebnis;
};

/// Ein Kern, vorbereitet fuer ein Instrument.
class Tonkern
{
public:
    virtual ~Tonkern() = default;

    /// So lange klingt der Ton nach dem Ende der letzten Note, in Frames, hoechstens `hoechstens`.
    [[nodiscard]] virtual Frames nachklang(const Tonauftrag& auftrag, Frames hoechstens) const = 0;

    /// Den Ton rechnen: `ton` hat danach `auftrag.frames` Werte Mono (Spitze um 0,5 bei der Bezugsnote und Stufe 0,
    /// `normiere`). Mit `innere` die gewuenschten inneren Groessen dazu, soweit der Kern sie kennt.
    virtual void rechne(const Tonauftrag& auftrag, std::vector<float>& ton, Innere* innere = nullptr) const = 0;
};

/// Die Art eines Kerns.
struct KernTyp
{
    Parametertafel tafel;
    KernAuskunft auskunft;
    /// baut den Kern aus Werten, die schon nach der Tafel gerichtet sind (`richteWerte` ohne Fehler)
    std::function<std::shared_ptr<const Tonkern>(const Werte& werte)> erzeuge;
};

/// Die Kerne je Name.
class Fabrik
{
public:
    /// Nimmt einen Typ auf; zurueck kommt, was nicht stimmt (Name leer oder doppelt, Tafel fehlerhaft, kein
    /// `erzeuge`) — dann wird er nicht aufgenommen.
    std::vector<std::string> registriere(KernTyp typ);

    /// Der Typ mit diesem Namen, oder `nullptr`.
    [[nodiscard]] const KernTyp* typ(std::string_view name) const;
    /// Die Namen in der Reihenfolge der Aufnahme.
    [[nodiscard]] std::vector<std::string> namen() const;

    /// Richtet `werte` nach der Tafel und baut den Kern. `nullptr`, wenn der Name unbekannt ist oder die Werte
    /// Fehler haben; dann steht in `fehler`, was.
    [[nodiscard]] std::shared_ptr<const Tonkern> erzeuge(std::string_view name, Werte werte,
                                                         std::vector<std::string>* fehler = nullptr) const;

private:
    std::vector<KernTyp> m_typen;
};

/// Diese Spitze hat der Ton an der Bezugsnote bei Stufe 0 (IC `normiere`; der Pegel stimmt LumiViz nach LUFS).
inline constexpr double kNormSpitze = 0.5;

/// Der Bezug des Ausgangs (IK2-10 a, IC `normiere` je Klasse — hier einmal): setzt den Wert `bezugName` (Zahl) zuerst
/// auf 1, rechnet `probe` und setzt ihn auf rohe Spitze / `kNormSpitze`; die rohe Spitze ist die Spitze des Tons mal dem
/// Bezug, mit dem er gerechnet wurde. `runden` Mal (zwei, wenn Teile des Klangs am Bezug haengen, wie die Rauschanteile
/// der Membran). Ein stummer Ton gibt 1. Bei Fehlern (Kern unbekannt, Werte falsch) 0 und `fehler`.
[[nodiscard]] double normiere(const Fabrik& fabrik, std::string_view kern, Werte werte, std::string_view bezugName,
                              const Tonauftrag& probe, int runden = 1, std::vector<std::string>* fehler = nullptr);

/// Die Probe des Bezugs (IC `normiere`: „Ton an der Mitte bei Stufe 0“, J10): die mittlere Note des Tastenumfangs
/// (abgerundet), gleichstufig zu A4 = 440 Hz, Stufe 0, eine Sekunde gehalten und gerechnet bei 48 kHz.
[[nodiscard]] Tonauftrag bezugsProbe(const Parametertafel& tafel);
/// Dasselbe an der Mitte des Umfangs des Instruments (`tastenumfang`, KS1 a; IC normiert an der Mitte des Instruments).
[[nodiscard]] Tonauftrag bezugsProbe(const Parametertafel& tafel, const Werte& werte);

/// Der Bezug nach der Tafel des Kerns (`bezugWert`, `bezugRunden`, `bezugsProbe` mit den Werten). 0 und `fehler`, wenn der Kern
/// unbekannt ist oder keinen Bezug traegt.
[[nodiscard]] double normiereNachTafel(const Fabrik& fabrik, std::string_view kern, const Werte& werte, std::vector<std::string>* fehler = nullptr);

}  // namespace lumi::kern
