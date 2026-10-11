/**
 * @file Klangerzeuger.cpp
 * @brief Loop: die lebende Quelle im Mixer (Stufe 5)
 */

#include "loop/Klangerzeuger.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::loop
{

namespace
{

const std::vector<Klang> kKeineKlaenge;

}  // namespace

std::shared_ptr<const Programm> baueProgramm(const Loop& loop, int rate,
                                             std::shared_ptr<const std::vector<Klang>> klaenge)
{
    auto programm = std::make_shared<Programm>();
    programm->ablauf = plane(loop, rate);
    programm->klaenge = std::move(klaenge);
    return programm;
}

void Klangerzeuger::setzeProgramm(std::shared_ptr<const Programm> programm)
{
    const std::shared_ptr<const Programm> alt = m_programm.load();
    if (alt != nullptr && programm != nullptr && alt->ablauf.schrittFrames > 0.0 &&
        programm->ablauf.schrittFrames > 0.0 && alt->ablauf.schrittFrames != programm->ablauf.schrittFrames)
    {
        // dieselbe Stelle in Schritten, in der neuen Schrittlaenge. Rechnet
        // der Ton-Thread gerade einen Block, zaehlt er danach nicht weiter
        // (`render` nimmt seine Stelle nur, wenn sie noch gilt).
        const double schritte = static_cast<double>(m_stelle.load()) / alt->ablauf.schrittFrames;
        m_stelle.store(std::llround(schritte * programm->ablauf.schrittFrames));
    }
    m_programm.store(std::move(programm));
    m_davor = alt;
}

std::int64_t Klangerzeuger::stelleImDurchlauf() const
{
    const std::shared_ptr<const Programm> p = m_programm.load();
    if (p == nullptr || p->ablauf.durchlaufFrames <= 0) return 0;
    return m_stelle.load() % p->ablauf.durchlaufFrames;
}

void Klangerzeuger::render(float* out, int frames)
{
    if (frames <= 0) return;
    const std::shared_ptr<const Programm> p = m_programm.load();
    if (p == nullptr)
    {
        std::fill(out, out + 2 * static_cast<std::ptrdiff_t>(frames), 0.0f);
        return;
    }
    std::int64_t von = m_stelle.load();
    rechne(p->ablauf, p->klaenge != nullptr ? *p->klaenge : kKeineKlaenge, von, frames, out);
    // die Spitze des Blocks fuer die Anzeige im Editor (L6 a)
    float spitze = 0.0f;
    for (std::ptrdiff_t i = 0; i < 2 * static_cast<std::ptrdiff_t>(frames); ++i)
        spitze = std::max(spitze, std::abs(out[i]));
    float bisher = m_spitze.load();
    while (spitze > bisher && !m_spitze.compare_exchange_weak(bisher, spitze)) {}
    // weiterzaehlen — ausser jemand hat die Stelle inzwischen gesetzt
    m_stelle.compare_exchange_strong(von, von + frames);
}

}  // namespace lumi::loop
