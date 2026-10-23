/**
 * @file Umsetzungen.hpp
 * @brief Jede Umsetzung der austauschbaren Teile — Netzgeber und Eigenloeser — an einer
 *        Stelle (BM4 a, BM5-1 a: Tests gegen die Schnittstelle laufen fuer jede, das
 *        Werkzeug waehlt per Schalter). Eine neue Umsetzung wird nur hier eingetragen.
 *
 * Bindet die fremden Bibliotheken der Umsetzungen ein (CDT, Eigen, Spectra) — nur dort
 * einbinden, wo sie gebunden sind (Tests, InstrumentBodyTool).
 */
#pragma once

#include "InstrumentBody/EigenloeserSpectra.hpp"
#include "InstrumentBody/NetzCdt.hpp"

#include <memory>
#include <string>
#include <vector>

namespace lumi::koerper
{

/// Jeder Netzgeber; der erste ist die Vorgabe.
[[nodiscard]] inline std::vector<std::unique_ptr<Netzgeber>> netzgeber()
{
    std::vector<std::unique_ptr<Netzgeber>> alle;
    alle.push_back(std::make_unique<NetzgeberCdt>());
    return alle;
}

/// Jeder Eigenloeser; der erste ist die Vorgabe.
[[nodiscard]] inline std::vector<std::unique_ptr<Eigenloeser>> eigenloeser()
{
    std::vector<std::unique_ptr<Eigenloeser>> alle;
    alle.push_back(std::make_unique<EigenloeserSpectra>());
    return alle;
}

}  // namespace lumi::koerper
