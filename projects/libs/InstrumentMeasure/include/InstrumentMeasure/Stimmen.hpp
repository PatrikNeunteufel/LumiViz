/**
 * @file Stimmen.hpp
 * @brief Gestimmte Werte und der Bezug des Ausgangs als Werkzeug (Instrumente_IC_Konzept.md IK2-10 a, IK2-11 a;
 *        Stufe 34f-3, S177)
 *
 * **Gestimmte Werte** sind Kurven, die ein Werkzeug aus anderen Werten rechnet — etwa f0 und die Fenster des Drucks je
 * Note einer Zunge (IC `ic_stimme_zunge`). Sie sind Daten in der `.lvinst` mit Belegkraft *Rechnung* und dem Abdruck der
 * Eingaben, aus denen sie gestimmt wurden (`kern::eingabenAbdruck`). Gestimmt wird hier, in der Messung, weil das
 * Stimmen am gerechneten Ton misst (YIN, B2); ein Kern liefert dazu seinen `Stimmer`. Die eingebauten (`stimmer()`)
 * kommen mit ihren Klassen (34g: Klasse 4, 34j: Klasse 10, 34k: Klasse 2).
 *
 * **Der Bezug** (`Parametertafel::bezugWert`) ist eine Zahl der Tafel, die `kern::normiereNachTafel` neu rechnet: an
 * der mittleren Note bei Stufe 0 hat der Ton die Spitze 0,5. Er wird nach dem Stimmen gesetzt (er haengt an den
 * gestimmten Werten) und zaehlt deshalb nicht zu ihrem Abdruck.
 *
 * Ohne Qt; die Datei liest und schreibt der Aufrufer (`InstrumentTuneTool`, spaeter der Knopf im Creator, IK2-13).
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"
#include "InstrumentCore/Parametertafel.hpp"
#include "InstrumentCore/Tonkern.hpp"

#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lumi::messung
{

/// Die gestimmten Werte, die ein Stimmer rechnet: je Name die neue Kurve.
using GestimmteWerte = std::vector<std::pair<std::string, kern::Kurve>>;

/// Wie ein Kern seine gestimmten Werte rechnet: aus den Werten (nach der Tafel gerichtet) die gestimmten Kurven mit den
/// Namen `namen`, in der Reihenfolge der Tafel (KS2 a) — ein gestimmter Wert davor, der nicht in `namen` steht, gilt, wie
/// er in `werte` steht; einer in `namen` wird zuerst neu gestimmt, und die spaeteren nehmen ihn.
struct Stimmer
{
    std::string kern;
    std::function<GestimmteWerte(const kern::Fabrik& fabrik, const kern::Werte& werte, const std::vector<std::string>& namen)> stimme;
};

/// Die eingebauten Stimmer (IK2-11 a: Klasse 4 `bar` seit 34g, `StabStimmen.hpp`; Klasse 10 in 34j, Klasse 2 in 34k).
[[nodiscard]] const std::vector<Stimmer>& stimmer();

/// Der Stimmer eines Kerns in `liste`, oder `nullptr`.
[[nodiscard]] const Stimmer* stimmerFuer(const std::vector<Stimmer>& liste, std::string_view kern);

/// Was `stimmeUndNormiere` getan hat.
struct StimmBericht
{
    std::vector<std::string> gestimmt;  ///< die neu gestimmten Werte
    bool mitBezug = false;              ///< der Kern traegt einen Bezug
    double bezugVorher = 0.0, bezug = 0.0;
    std::vector<std::string> fehler;
    [[nodiscard]] bool geaendert() const { return !gestimmt.empty() || (mitBezug && bezug != bezugVorher); }
};

/// Stimmt die gestimmten Werte von `werte` (die veralteten und die spaeteren, die von einem davon abhaengen — KS2 a;
/// mit `alle` jeden), merkt ihre Abdruecke und setzt
/// danach — mit `bezug` — den Bezug neu. Fehlt einem Kern mit gestimmten Werten sein Stimmer, steht das in `fehler`.
[[nodiscard]] StimmBericht stimmeUndNormiere(const kern::Fabrik& fabrik, const std::vector<Stimmer>& liste, std::string_view kern,
                                             kern::Werte& werte, bool alle, bool bezug);

/// Was an `werte` veraltet ist, ohne zu rechnen: die gestimmten Werte mit fremdem Abdruck (IK2-11 a). Den Bezug prueft
/// `bezugVeraltet` (er muss einen Ton rechnen).
[[nodiscard]] std::vector<std::string> veralteteWerte(const kern::Fabrik& fabrik, std::string_view kern, const kern::Werte& werte);

/// Der Bezug in `werte` gegen den neu gerechneten: wahr, wenn er abweicht (relativ mehr als `toleranz`). Ein Kern ohne
/// Bezug ist nie veraltet; `neu` (wenn gegeben) bekommt den gerechneten.
[[nodiscard]] bool bezugVeraltet(const kern::Fabrik& fabrik, std::string_view kern, const kern::Werte& werte, double toleranz = 1e-12,
                                 double* neu = nullptr);

}  // namespace lumi::messung
