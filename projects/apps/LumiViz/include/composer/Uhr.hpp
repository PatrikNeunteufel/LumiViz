/**
 * @file Uhr.hpp
 * @brief Composer: die Uhr mit eigenem Transport (Konzept §4, Entscheid C2 a)
 *
 * Ohne Audiokanal laeuft die Zeit des Composers nach der Wanduhr: Start,
 * Stopp, an den Anfang, Sprung. Die Stelle auf der Zeitachse bestimmt das
 * Bild — nicht die verstrichene Zeit. Am Ende der Komposition haelt die Uhr;
 * in der Schleife (Loop, C2b) beginnt sie dort von vorn.
 *
 * Die Zeitquelle ist austauschbar (Test); Vorgabe ist eine stetige Uhr.
 * Kein Qt-Objekt, kein Thread: wer zeichnet, fragt `stelleMs()`.
 */
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>

namespace lumi::composer
{

class Uhr
{
public:
    using Zeitquelle = std::function<std::int64_t()>;  ///< Millisekunden, stetig steigend

    Uhr() : m_quelle(&Uhr::wanduhrMs) {}
    explicit Uhr(Zeitquelle quelle) : m_quelle(std::move(quelle)) {}

    /// Laenge der Komposition; 0 = kein Ende. Eine Stelle dahinter wird geklemmt.
    void setLaengeMs(std::int64_t laengeMs)
    {
        const std::int64_t stelle = stelleMs();
        m_laengeMs = std::max<std::int64_t>(0, laengeMs);
        setzeStelle(stelle);
    }
    [[nodiscard]] std::int64_t laengeMs() const { return m_laengeMs; }

    void starte()
    {
        if (laeuft()) return;
        std::int64_t stelle = stelleMs();
        // am Ende angekommen: von vorn
        if (m_laengeMs > 0 && stelle >= m_laengeMs) stelle = 0;
        m_stelleMs = stelle;
        m_ankerMs = m_quelle();
        m_laeuft = true;
    }
    void stoppe()
    {
        m_stelleMs = stelleMs();
        m_laeuft = false;
    }
    void anDenAnfang() { springe(0); }
    /// An eine Stelle springen; ob die Uhr laeuft, bleibt.
    void springe(std::int64_t stelle) { setzeStelle(stelle); }

    /// Schleife (C2b, Knopf Loop): am Ende beginnt die Uhr von vorn, statt zu
    /// halten. Ohne Laenge gibt es kein Ende, also auch keine Schleife.
    void setSchleife(bool an)
    {
        if (an == m_schleife) return;
        // die Stelle festhalten: ohne Schleife zaehlt die rohe Zeit wieder ganz
        // — und ob sie laeuft: eine am Ende stehende Uhr laeuft nicht los
        const std::int64_t stelle = stelleMs();
        const bool lief = laeuft();
        m_schleife = an;
        m_laeuft = lief;
        setzeStelle(stelle);
    }
    [[nodiscard]] bool schleife() const { return m_schleife; }

    /// Laeuft die Uhr? Am Ende der Komposition haelt sie von selbst — ausser
    /// in der Schleife.
    [[nodiscard]] bool laeuft() const
    {
        return m_laeuft && (dreht() || !(m_laengeMs > 0 && roh() >= m_laengeMs));
    }
    [[nodiscard]] std::int64_t stelleMs() const
    {
        const std::int64_t s = roh();
        if (m_laengeMs <= 0) return s;
        return m_laeuft && dreht() ? s % m_laengeMs : std::min(s, m_laengeMs);
    }

private:
    static std::int64_t wanduhrMs()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }
    [[nodiscard]] bool dreht() const { return m_schleife && m_laengeMs > 0; }
    [[nodiscard]] std::int64_t roh() const
    {
        return m_laeuft ? m_stelleMs + (m_quelle() - m_ankerMs) : m_stelleMs;
    }
    void setzeStelle(std::int64_t stelle)
    {
        stelle = std::max<std::int64_t>(0, stelle);
        if (m_laengeMs > 0) stelle = std::min(stelle, m_laengeMs);
        m_stelleMs = stelle;
        if (m_laeuft) m_ankerMs = m_quelle();
    }

    Zeitquelle m_quelle;
    std::int64_t m_stelleMs = 0;  ///< Stelle beim letzten Start / Stopp / Sprung
    std::int64_t m_ankerMs = 0;   ///< Zeit der Quelle bei diesem Ereignis
    std::int64_t m_laengeMs = 0;
    bool m_laeuft = false;
    bool m_schleife = false;
};

}  // namespace lumi::composer
