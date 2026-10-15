/**
 * @file Begrenzer.cpp
 * @brief Limiter fuer den Mixer des Composers (Composer_Konzept §9g, L4 a)
 */

#include "audio/Begrenzer.hpp"

#include <algorithm>
#include <cmath>

Begrenzer::Begrenzer(int sampleRate)
    : m_rueckweg(1.0 - std::exp(-1000.0 / (kLoslassenMs * static_cast<double>(std::max(1, sampleRate)))))
{
}

void Begrenzer::verarbeite(float* stereo, std::size_t frames)
{
    std::size_t begrenzt = 0;
    for (std::size_t i = 0; i < frames; ++i)
    {
        float& links = stereo[2 * i];
        float& rechts = stereo[2 * i + 1];
        // zurueck nach 1 — die letzten Reste auf genau 1, damit ein Signal
        // unter der Decke wieder Bit fuer Bit durchgeht
        if (m_verstaerkung < 1.0)
        {
            m_verstaerkung += (1.0 - m_verstaerkung) * m_rueckweg;
            if (1.0 - m_verstaerkung < 1e-6) m_verstaerkung = 1.0;
        }
        const double spitze = std::max(std::abs(links), std::abs(rechts));
        if (spitze * m_verstaerkung > kDecke) m_verstaerkung = kDecke / spitze;
        if (m_verstaerkung < 1.0)
        {
            links = static_cast<float>(links * m_verstaerkung);
            rechts = static_cast<float>(rechts * m_verstaerkung);
            ++begrenzt;
        }
    }
    if (begrenzt > 0) m_begrenzt.fetch_add(begrenzt, std::memory_order_relaxed);
}
