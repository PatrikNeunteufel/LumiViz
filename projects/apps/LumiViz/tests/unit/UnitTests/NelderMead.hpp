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
inline NelderMead nelderMead(const std::function<double(const std::vector<double>&)>& f, const std::vector<double>& x0,
                             const std::vector<double>& schritte, int maxAuswertungen)
{
    const std::size_t d = x0.size();
    std::vector<std::vector<double>> punkte{x0};
    for (std::size_t j = 0; j < d; ++j)
    {
        std::vector<double> p = x0;
        p[j] += schritte[j];
        punkte.push_back(p);
    }
    int auswertungen = 0;
    const auto werte = [&](const std::vector<double>& p) {
        ++auswertungen;
        return f(p);
    };
    std::vector<double> reste;
    for (const auto& p : punkte) reste.push_back(werte(p));
    while (auswertungen < maxAuswertungen)
    {
        std::vector<std::size_t> ordnung(d + 1);
        for (std::size_t i = 0; i <= d; ++i) ordnung[i] = i;
        std::sort(ordnung.begin(), ordnung.end(), [&](std::size_t a, std::size_t b) { return reste[a] < reste[b]; });
        const std::size_t best = ordnung.front();
        const std::size_t schlecht = ordnung.back();
        const std::size_t zweit = ordnung[d - 1];  // der zweitschlechteste
        if (reste[schlecht] - reste[best] < 1e-4) break;
        std::vector<double> mitte(d, 0.0);
        for (std::size_t i = 0; i <= d; ++i)
            if (i != schlecht)
                for (std::size_t j = 0; j < d; ++j) mitte[j] += punkte[i][j] / static_cast<double>(d);
        const auto entlang = [&](double t) {
            std::vector<double> p(d);
            for (std::size_t j = 0; j < d; ++j) p[j] = mitte[j] + t * (punkte[schlecht][j] - mitte[j]);
            return p;
        };
        const std::vector<double> gespiegelt = entlang(-1.0);
        const double rSpiegel = werte(gespiegelt);
        if (rSpiegel < reste[best])
        {
            const std::vector<double> gestreckt = entlang(-2.0);
            const double rStreck = werte(gestreckt);
            punkte[schlecht] = rStreck < rSpiegel ? gestreckt : gespiegelt;
            reste[schlecht] = std::min(rStreck, rSpiegel);
        }
        else if (rSpiegel < reste[zweit])
        {
            punkte[schlecht] = gespiegelt;
            reste[schlecht] = rSpiegel;
        }
        else
        {
            const std::vector<double> gestaucht = entlang(0.5);
            const double rStauch = werte(gestaucht);
            if (rStauch < reste[schlecht])
            {
                punkte[schlecht] = gestaucht;
                reste[schlecht] = rStauch;
            }
            else
                for (std::size_t i = 0; i <= d; ++i)
                {
                    if (i == best) continue;
                    for (std::size_t j = 0; j < d; ++j) punkte[i][j] = punkte[best][j] + 0.5 * (punkte[i][j] - punkte[best][j]);
                    reste[i] = werte(punkte[i]);
                }
        }
    }
    const auto best = static_cast<std::size_t>(std::min_element(reste.begin(), reste.end()) - reste.begin());
    return {punkte[best], reste[best], auswertungen};
}

}  // namespace lumi::messung
