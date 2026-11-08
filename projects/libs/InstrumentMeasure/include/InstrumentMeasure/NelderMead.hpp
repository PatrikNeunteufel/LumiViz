/**
 ****************************************************************************************
 * @file   NelderMead.hpp
 * @brief  Suche nach Nelder–Mead fuer das Setzen der Hammer-Saite (31d-7 B; SM2 a, SM4 a) —
 *         aus `test_LoopSalamanderSetzen.cpp` herausgeloest (S155), gemeinsam mit der Probe 7.5.4
 *
 * @details
 * Reines C++ ohne Qt (IK1). Spiegeln 1, Strecken 2, Stauchen ½, Schrumpfen ½; Ende nach
 * `maxAuswertungen` oder wenn die Reste des Simplex um weniger als 10⁻⁴ auseinanderliegen.
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <vector>

namespace lumi::messung
{

/// Das Ergebnis einer Suche nach Nelder–Mead.
struct NelderMead
{
    std::vector<double> x;
    double rest = 0.0;
    int auswertungen = 0;
};

/// Nelder–Mead ueber `f`, vom Start `x0` mit dem Anfangsschritt je Achse.
NelderMead nelderMead(const std::function<double(const std::vector<double>&)>& f, const std::vector<double>& x0,
                             const std::vector<double>& schritte, int maxAuswertungen);

}  // namespace lumi::messung
