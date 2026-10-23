/**
 * @file DuenneMatrix.hpp
 * @brief Eine symmetrische, duenn besetzte Matrix (nur das obere Dreieck mit der
 *        Diagonale, zeilenweise) — das Format, in dem die Platte K und M uebergibt
 *        (Entscheid Patrik S147: BM5-1 a — kein Typ einer fremden Bibliothek geht
 *        ueber die Schnittstelle des Eigenloesers)
 *
 * Gebaut in zwei Schritten: erst das Muster (welche Eintraege es gibt), dann die
 * Werte hineinaddieren. So braucht der Bau keinen Zwischenspeicher fuer jeden
 * Elementbeitrag.
 */
#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace lumi::koerper
{

struct DuenneMatrix
{
    int n = 0;                         ///< Zeilen und Spalten
    std::vector<std::int64_t> start;   ///< n + 1 Eintraege: wo jede Zeile beginnt
    std::vector<int> spalten;          ///< je Eintrag die Spalte (≥ Zeile, aufsteigend)
    std::vector<double> werte;

    [[nodiscard]] std::int64_t eintraege() const { return static_cast<std::int64_t>(werte.size()); }

    /// Das Muster aus je Zeile den Spalten (≥ Zeile, beliebig, mit Doppelten); Werte 0.
    void setzeMuster(int groesse, std::vector<std::vector<int>>& zeilen)
    {
        n = groesse;
        start.assign(static_cast<std::size_t>(n) + 1, 0);
        spalten.clear();
        for (int i = 0; i < n; ++i)
        {
            auto& z = zeilen[static_cast<std::size_t>(i)];
            std::sort(z.begin(), z.end());
            z.erase(std::unique(z.begin(), z.end()), z.end());
            spalten.insert(spalten.end(), z.begin(), z.end());
            start[static_cast<std::size_t>(i) + 1] = static_cast<std::int64_t>(spalten.size());
            z.clear();
            z.shrink_to_fit();
        }
        werte.assign(spalten.size(), 0.0);
    }

    /// Die Stelle von (i, j) im Muster, i ≤ j; −1 = gibt es nicht.
    [[nodiscard]] std::int64_t stelle(int i, int j) const
    {
        const auto a = spalten.begin() + start[static_cast<std::size_t>(i)];
        const auto b = spalten.begin() + start[static_cast<std::size_t>(i) + 1];
        const auto it = std::lower_bound(a, b, j);
        return (it != b && *it == j) ? static_cast<std::int64_t>(it - spalten.begin()) : -1;
    }

    /// Den Wert (i, j) dazuaddieren; die Seite unter der Diagonale wird auf die obere gelegt.
    void addiere(int i, int j, double v)
    {
        if (i > j) std::swap(i, j);
        const std::int64_t k = stelle(i, j);
        if (k >= 0) werte[static_cast<std::size_t>(k)] += v;
    }

    /// Der Wert (i, j), beide Seiten; 0 ausserhalb des Musters.
    [[nodiscard]] double wert(int i, int j) const
    {
        if (i > j) std::swap(i, j);
        const std::int64_t k = stelle(i, j);
        return k >= 0 ? werte[static_cast<std::size_t>(k)] : 0.0;
    }

    /// y = A x (die ganze symmetrische Matrix).
    [[nodiscard]] std::vector<double> mal(const std::vector<double>& x) const
    {
        std::vector<double> y(static_cast<std::size_t>(n), 0.0);
        for (int i = 0; i < n; ++i)
        {
            for (std::int64_t k = start[static_cast<std::size_t>(i)]; k < start[static_cast<std::size_t>(i) + 1]; ++k)
            {
                const int j = spalten[static_cast<std::size_t>(k)];
                const double a = werte[static_cast<std::size_t>(k)];
                y[static_cast<std::size_t>(i)] += a * x[static_cast<std::size_t>(j)];
                if (j != i) y[static_cast<std::size_t>(j)] += a * x[static_cast<std::size_t>(i)];
            }
        }
        return y;
    }
};

}  // namespace lumi::koerper
