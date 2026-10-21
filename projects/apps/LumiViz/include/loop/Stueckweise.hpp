/**
 * @file Stueckweise.hpp
 * @brief Loop: ein stueckweise linearer Verlauf, der vorwaerts gelesen wird
 *        (Stufe 28c-1a im Rechenkern, seit 28c-2b auch im Ton einer Saite)
 *
 * Ein Verlauf ist eine Liste von Stellen (Abtastwerte) mit Werten, nach der
 * Stelle sortiert; zwei Eintraege an derselben Stelle sind ein Sprung. Der Leser
 * merkt sich das Stueck, in dem er zuletzt war — wer Abtastwert fuer Abtastwert
 * vorwaerts liest, sucht nicht jedes Mal von vorn. Kein Qt, keine Audio-Engine.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace lumi::loop
{

using Stuecke = std::vector<std::pair<std::int64_t, double>>;

class StueckLeser
{
public:
    explicit StueckLeser(const Stuecke& stuecke) : m_stuecke(stuecke) {}
    /// Der Wert an `x`: vor dem ersten Eintrag der erste, nach dem letzten der letzte,
    /// dazwischen linear; an zwei Eintraegen derselben Stelle gilt der spaetere. Ohne
    /// Eintraege 1.
    double bei(std::int64_t x)
    {
        if (m_stuecke.empty()) return 1.0;
        if (x < m_stuecke[m_i].first) m_i = 0;  // zurueck (ein neuer Schlag)
        while (m_i + 1 < m_stuecke.size() && m_stuecke[m_i + 1].first <= x) ++m_i;
        const auto& [xa, wa] = m_stuecke[m_i];
        if (x <= xa || m_i + 1 >= m_stuecke.size()) return x < xa ? m_stuecke.front().second : wa;
        const auto& [xb, wb] = m_stuecke[m_i + 1];
        return wa + (wb - wa) * static_cast<double>(x - xa) / static_cast<double>(xb - xa);
    }

private:
    const Stuecke& m_stuecke;
    std::size_t m_i = 0;
};

}  // namespace lumi::loop
