/**
 * @file UmlaufTafel.hpp
 * @brief InstrumentModels, Kern `pluckedString` Rechenfassung 2: die Tafel des Umlauf-Filters als Liste der Parametertafel
 *        (Instrumente_IC_Konzept.md ZK1 a — ein gestimmter Wert; UL3 a, UL3-1 a; Teilplan 34k-7d-3)
 *
 * Der Stimmer (`InstrumentMeasure/ZupfStimmen`) entwirft die Tafel aus den Saiten, `InstrumentTuneTool` schreibt sie in
 * die `.lvinst`, der Kern liest sie. Eine Liste, ein Eintrag je Stelle des Entwurfs:
 *
 * - **je Taste** (Harfe, Cembalo; `taste` ≥ 0): die Kette als Biquads (r, θ), der Verlust (eine Stelle, b und a) und die
 *   Ordnung des Thiran-Glieds (5, 3 oder 1 — die erste, die UL2 a hielt).
 * - **je Saite** (Gitarre, Laute, Oud; `chor` ≥ 0, `saite` im Chor): je Biquad die Legendre-Reihen von logit(r / 0,99) und
 *   logit(θ / π), der Verlust an den Buenden und auf halbem Weg (Lage und b, a). Thiran 5.
 *
 * N und D des Glieds stehen nicht darin: der Kern stimmt sie zur Laufzeit am Grundton (`umlauf::stimmeAmGrundton`,
 * Bit fuer Bit wie der Entwurf). Buende und Mensur kommen aus dem Saitensatz, nicht aus der Tafel. Zahlenfolgen stehen als
 * Listen von Eintraegen mit dem Feld `value`.
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Parametertafel.hpp"
#include "InstrumentModels/gemeinsam/Umlauf.hpp"

#include <string_view>
#include <vector>

namespace lumi::kern::zupf
{

/// Der Name des gestimmten Werts in der Tafel des Kerns.
inline constexpr std::string_view kUmlaufTafel = "loopFilterTable";

/// Ein Eintrag der Tafel.
struct UmlaufEintrag
{
    int taste = -1;  ///< je Taste: die Taste; sonst −1
    int chor = -1;   ///< je Saite: der Chor (Saite des Satzes, wie die App sie zaehlt); sonst −1
    int saite = 0;   ///< je Saite: die Saite im Chor (Laute: 1 die Oktavsaite)
    /// je Taste: Kette, Verlust (b, a) und Ordnung des Glieds; N und D bleiben 0 / 4,5 (der Kern stimmt sie)
    umlauf::Umlauffilter filter;
    /// je Saite: Grad, Reihen und Verlust; Buende und Mensur bleiben 0 (der Kern setzt sie aus dem Saitensatz)
    umlauf::Saitentafel tafel;

    [[nodiscard]] bool operator==(const UmlaufEintrag&) const = default;
};

/// Die Angabe der Liste (ohne `gestimmt`, das setzt die Tafel des Kerns).
[[nodiscard]] ParameterAngabe umlaufTafelAngabe();

/// Die Eintraege als Liste der Parametertafel und zurueck (verlustfrei).
[[nodiscard]] Liste umlaufTafelListe(const std::vector<UmlaufEintrag>& eintraege);
[[nodiscard]] std::vector<UmlaufEintrag> umlaufTafelEintraege(const Liste& liste);

}  // namespace lumi::kern::zupf
