/**
 * @file ZupfStimmen.hpp
 * @brief Stimmen am Kern `pluckedString` (Klasse 2) Rechenfassung 2: die Tafel des Umlauf-Filters, entworfen aus den
 *        Saiten (Instrumente_IC_Konzept.md ZK1 a, IK2-11 a; 34k-7c, 34k-7d-3, 34k-7d-4)
 *
 * **Stimmer** (34k-7d-4): stimmt den gestimmten Wert `loopFilterTable` — Harfe und Cembalo je Taste
 * (`entwirfUmlaufJeTaste`), Gitarre, Laute und Oud je Saite des Satzes (`entwirfUmlaufJeSaite`); `InstrumentTuneTool`
 * schreibt sie in die `.lvinst`. Der Teil nach IC (der Faktor auf die Laufzeiten `delayFactor`, IC `stimme`, und die
 * Messung der Summe) ging mit Rechenfassung 1 (ZB1 a, S224).
 *
 * Ohne Qt.
 */
#pragma once

#include "InstrumentMeasure/Stimmen.hpp"
#include "InstrumentMeasure/UmlaufEntwurf.hpp"
#include "InstrumentModels/zupf/Saiten.hpp"
#include "InstrumentModels/zupf/UmlaufTafel.hpp"
#include "InstrumentModels/zupf/Zupf.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace lumi::messung
{

/// Der Stimmer des Kerns `pluckedString`: die Tafel des Umlauf-Filters (`loopFilterTable`) aus den Saiten der Werte —
/// je Saite des Satzes oder je Taste (ZK1 a; alle fuenf Instrumente zusammen rund 2 h, mit `LUMIVIZ_POOL_THREADS`
/// zu begrenzen).
[[nodiscard]] Stimmer zupfStimmer();

// ---------------------------------------------------------------- Rechenfassung 2 (34k-7d-3, ZK1 a)

/// Die Tafel des Umlauf-Filters eines Instruments, entworfen (`umlauf::entwirfJeTaste` / `entwirfJeSaite`), mit der
/// Bewertung je Stelle nach UL2 a.
struct UmlaufTafelEntwurf
{
    std::vector<kern::zupf::UmlaufEintrag> eintraege;
    /// je Stelle ein Name (`T60`, `Chor 1.2 Buende` …) und ihre Bewertung; je Saite vier: an den Buenden, auf halbem Weg,
    /// an Achteln, an fremden Stellen
    std::vector<std::pair<std::string, umlauf::Bewertung>> bewertungen;
    std::vector<std::string> fehler;  ///< Stellen, fuer die der Entwurf nichts fand
    double sekunden = 0.0;
};

/// Je Taste `von` … `bis` (Harfe, Cembalo); Tasten ohne Saite (`std::nullopt`) fehlen in der Tafel.
[[nodiscard]] UmlaufTafelEntwurf entwirfUmlaufJeTaste(int von, int bis,
                                                     const std::function<std::optional<kern::zupf::Tastensaite>(int)>& saite);
/// Je Saite des Satzes (Gitarre, Laute, Oud); eine Saite im Einklang mit der ersten des Chors bekommt keine eigene Tafel
/// (ihre Verstimmung ist eine Lage auf derselben). Nur die Choere `chorVon` … `chorBis` (Index im Satz von der tiefsten
/// an; −1 = bis zum letzten) — ein langer Entwurf laesst sich so in Stuecke teilen.
[[nodiscard]] UmlaufTafelEntwurf entwirfUmlaufJeSaite(const kern::zupf::Saitensatz& satz, int chorVon = 0, int chorBis = -1);

}  // namespace lumi::messung
