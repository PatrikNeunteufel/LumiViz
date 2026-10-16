/**
 * @file Begrenzer.cpp
 * @brief Limiter fuer den Mixer des Composers (Composer_Konzept §9g, L4 a, L2-1 b)
 */

#include "audio/Begrenzer.hpp"

#include <algorithm>
#include <cmath>

Begrenzer::Begrenzer(int sampleRate)
    : m_n(std::max<std::size_t>(1, static_cast<std::size_t>(std::lround(kVorschauMs * std::max(1, sampleRate) / 1000.0))))
    , m_rueckweg(1.0 - std::exp(-1000.0 / (kLoslassenMs * static_cast<double>(std::max(1, sampleRate)))))
    , m_verzug(2 * m_n)
    , m_ziele(m_n)
    , m_minFrame(m_n)
    , m_minWert(m_n)
{
    zuruecksetzen();
}

void Begrenzer::zuruecksetzen()
{
    m_frame = 0;
    std::fill(m_verzug.begin(), m_verzug.end(), 0.0f);
    std::fill(m_ziele.begin(), m_ziele.end(), 1.0);
    m_summe = static_cast<double>(m_n);
    m_unterEins = 0;
    m_ziel = 1.0;
    m_verstaerkung = 1.0;
    m_minKopf = 0;
    m_minAnzahl = 0;
}

void Begrenzer::verarbeite(float* stereo, std::size_t frames)
{
    std::size_t begrenzt = 0;
    for (std::size_t i = 0; i < frames; ++i)
    {
        float& links = stereo[2 * i];
        float& rechts = stereo[2 * i + 1];
        const std::uint64_t t = m_frame++;

        // --- was dieser Frame braucht, ins gleitende Minimum
        const double spitze = std::max(std::abs(links), std::abs(rechts));
        const double noetig = spitze > kDecke ? kDecke / spitze : 1.0;
        while (m_minAnzahl > 0 && m_minWert[(m_minKopf + m_minAnzahl - 1) % m_n] >= noetig) --m_minAnzahl;
        const std::size_t hinten = (m_minKopf + m_minAnzahl) % m_n;
        m_minFrame[hinten] = t;
        m_minWert[hinten] = noetig;
        ++m_minAnzahl;
        // das Fenster reicht vom Frame t − (n − 1) bis t
        while (m_minFrame[m_minKopf] + (m_n - 1) < t)
        {
            m_minKopf = (m_minKopf + 1) % m_n;
            --m_minAnzahl;
        }
        const double vorschau = m_minWert[m_minKopf];

        // --- Ziel-Verstaerkung: weich zurueck nach 1 (die letzten Reste auf genau
        //     1), nie ueber dem, was die Vorschau verlangt
        if (m_ziel < 1.0)
        {
            m_ziel += (1.0 - m_ziel) * m_rueckweg;
            if (1.0 - m_ziel < 1e-6) m_ziel = 1.0;
        }
        m_ziel = std::min(m_ziel, vorschau);

        // --- die Verstaerkung: Mittel der letzten n Ziele (gleitender Anlauf)
        const std::size_t platz = static_cast<std::size_t>(t % m_n);
        const double alt = m_ziele[platz];
        if (alt < 1.0) --m_unterEins;
        if (m_ziel < 1.0) ++m_unterEins;
        m_ziele[platz] = m_ziel;
        m_summe += m_ziel - alt;
        if (platz == m_n - 1)  // einmal je Umlauf genau nachrechnen, gegen Rundungsdrift
        {
            m_summe = 0.0;
            for (const double z : m_ziele) m_summe += z;
        }
        m_verstaerkung = m_unterEins == 0 ? 1.0 : std::min(1.0, m_summe / static_cast<double>(m_n));

        // --- verzoegern: den Frame ablegen, den von vor n − 1 Frames ausgeben
        m_verzug[2 * platz] = links;
        m_verzug[2 * platz + 1] = rechts;
        const std::size_t aus = static_cast<std::size_t>((t + 1) % m_n);
        links = m_verzug[2 * aus];
        rechts = m_verzug[2 * aus + 1];
        if (m_verstaerkung < 1.0)
        {
            links = static_cast<float>(links * m_verstaerkung);
            rechts = static_cast<float>(rechts * m_verstaerkung);
            ++begrenzt;
        }
    }
    if (begrenzt > 0) m_begrenzt.fetch_add(begrenzt, std::memory_order_relaxed);
}
