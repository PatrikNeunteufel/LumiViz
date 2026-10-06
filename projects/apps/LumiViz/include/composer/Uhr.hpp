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
        const std::int64_t stelle = rueckkehrMs();
        m_laengeMs = std::max<std::int64_t>(0, laengeMs);
        setzeStelle(stelle);
    }
    [[nodiscard]] std::int64_t laengeMs() const { return m_laengeMs; }

    /// Ueberlappung in der Schleife (Entscheide Patrik S99, Konzept §4): der
    /// Anfang beginnt um diese Zeit vor dem Ende — ein Umlauf dauert die
    /// Laenge weniger die Ueberlappung, und im Fenster laufen zwei Stellen
    /// zugleich (`stelleMs` vorn, `zweiteStelleMs` hinten). Ohne Schleife
    /// wirkt sie nicht.
    void setUeberlappungMs(std::int64_t ueberlappungMs)
    {
        const std::int64_t stelle = rueckkehrMs();
        const bool lief = laeuft();
        m_ueberlappungMs = std::max<std::int64_t>(0, ueberlappungMs);
        m_laeuft = lief;
        setzeStelle(stelle);
    }
    /// Die Ueberlappung, die wirkt: hoechstens die halbe Laenge.
    [[nodiscard]] std::int64_t ueberlappungMs() const { return std::min(m_ueberlappungMs, m_laengeMs / 2); }
    /// Die Dauer eines Umlaufs in der Schleife.
    [[nodiscard]] std::int64_t umlaufMs() const { return m_laengeMs - ueberlappungMs(); }

    void starte()
    {
        if (laeuft()) return;
        std::int64_t stelle = rueckkehrMs();
        // am Ende angekommen: von vorn
        if (m_laengeMs > 0 && stelle >= m_laengeMs) stelle = 0;
        m_stelleMs = stelle;
        m_ankerMs = m_quelle();
        m_laeuft = true;
    }
    void stoppe()
    {
        // im Fenster die hintere Stelle merken: nach dem Halt laufen wieder beide
        m_stelleMs = rueckkehrMs();
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
        // — im Fenster die hintere: ohne Schleife spielt die Komposition ihr Ende aus
        const std::int64_t stelle = rueckkehrMs();
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
    /// Gestartet und am Ende von selbst zum Stehen gekommen (nicht angehalten).
    [[nodiscard]] bool amEnde() const { return m_laeuft && !laeuft(); }
    [[nodiscard]] std::int64_t stelleMs() const
    {
        const std::int64_t s = roh();
        if (m_laengeMs <= 0) return s;
        if (!dreht()) return std::min(s, m_laengeMs);
        const std::int64_t umlauf = umlaufMs();
        if (m_laeuft) return s % umlauf;
        // steht: die Stelle bleibt, wo sie ist — im hinteren Fenster zeigt sie nach vorn
        const std::int64_t steht = std::min(s, m_laengeMs);
        return ueberlappungMs() > 0 && steht >= umlauf ? steht - umlauf : steht;
    }
    /// Die Stelle im hinteren Fenster, die in der Schleife zugleich mit
    /// `stelleMs` laeuft; -1 = keine. Es gibt sie nur, wenn schon ein Ende
    /// da ist, das hineinspielt: nicht in der ersten Runde und nicht nach
    /// einem Sprung ins vordere Fenster. Ein Sprung ins hintere Fenster
    /// dagegen zeigt beide.
    [[nodiscard]] std::int64_t zweiteStelleMs() const
    {
        if (!dreht() || ueberlappungMs() <= 0) return -1;
        const std::int64_t s = roh();
        const std::int64_t umlauf = umlaufMs();
        if (s < umlauf) return -1;
        const std::int64_t vorn = m_laeuft ? s % umlauf : std::min(s, m_laengeMs) - umlauf;
        return vorn < ueberlappungMs() ? vorn + umlauf : -1;
    }
    /// Die Stelle, an die ein `springe` den jetzigen Stand wiederherstellt:
    /// im Fenster die hintere (dann laufen wieder beide), sonst `stelleMs`.
    [[nodiscard]] std::int64_t rueckkehrMs() const
    {
        const std::int64_t zweite = zweiteStelleMs();
        return zweite >= 0 ? zweite : stelleMs();
    }
    /// Hat die Schleife schon einmal umgeschlagen (oder steht die Uhr hinter
    /// dem Umlauf)? Dann spielt im vorderen Fenster das Ende hinein.
    [[nodiscard]] bool umgelaufen() const { return dreht() && roh() >= umlaufMs(); }

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
    std::int64_t m_ueberlappungMs = 0;  ///< eingestellt; was wirkt, sagt `ueberlappungMs()`
    bool m_laeuft = false;
    bool m_schleife = false;
};

}  // namespace lumi::composer
