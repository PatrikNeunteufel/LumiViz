/**
 * @file Pedalplan.cpp
 * @brief Loop: wie lange Sustain und Sostenuto eine Note halten (Stufe 28b)
 */

#include "loop/Pedalplan.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace lumi::loop
{

namespace
{

/// Ein Wechsel eines Pedals im Clip: ab `stelle` (Schritte ab dem Anfang des Clips) der Wert.
struct Wechsel
{
    double stelle = 0.0;
    double wert = 0.0;
};

bool gedrueckt(double wert)
{
    return wert >= kPedalSchwelle;
}

/// Die Wechsel einer Pedal-Spur ueber den ganzen Clip: je Durchlauf am Anfang der
/// Wert dort (vor dem ersten Punkt oben, SK5 a), dann ihre Punkte; am Ende des
/// Clips losgelassen.
std::vector<Wechsel> wechselImClip(const std::vector<SpurPunkt>& spur, double schritte, int wiederholungen)
{
    std::vector<Wechsel> wechsel;
    if (spur.empty()) return wechsel;
    for (int r = 0; r < wiederholungen; ++r)
    {
        const double davor = r * schritte;
        wechsel.push_back({davor, stufeBei(spur, 0.0)});
        for (const SpurPunkt& p : spur)
            if (p.stelle > 0.0) wechsel.push_back({davor + p.stelle, p.wert});
    }
    wechsel.push_back({wiederholungen * schritte, 0.0});
    return wechsel;
}

/// Der Wert an `stelle`: der des letzten Wechsels davor (oder an ihr); vor dem ersten oben.
double wertBei(const std::vector<Wechsel>& wechsel, double stelle)
{
    double wert = 0.0;
    for (const Wechsel& w : wechsel)
    {
        if (w.stelle > stelle) break;
        wert = w.wert;
    }
    return wert;
}

/// Der erste Wechsel nach `stelle`, an dem das Pedal oben ist (spaetestens das Ende des Clips).
double losgelassenNach(const std::vector<Wechsel>& wechsel, double stelle)
{
    for (const Wechsel& w : wechsel)
        if (w.stelle > stelle && !gedrueckt(w.wert)) return w.stelle;
    return wechsel.empty() ? stelle : wechsel.back().stelle;
}

}  // namespace

bool hatPedal(const Muster& muster)
{
    return !muster.zeitspuren.von(Steuerspur::Sustain).empty() || !muster.zeitspuren.von(Steuerspur::Sostenuto).empty();
}

double gehaltenBis(const Loop& loop, const Muster& muster, int wiederholungen, int runde, std::size_t note)
{
    if (note >= muster.noten.size()) return 0.0;
    const Note& n = muster.noten[note];
    const double schritte = static_cast<double>(muster.takte) * loop.schritteJeTakt();
    const int mal = std::max(1, wiederholungen);
    const double anschlag = runde * schritte + n.schritt;
    const double ende = anschlag + n.laenge;
    if (!hatPedal(muster)) return ende;
    double bis = ende;

    // Sustain: liegt das Ende bei gedruecktem Pedal, haelt es bis zum Loslassen
    const std::vector<Wechsel> sustain = wechselImClip(muster.zeitspuren.von(Steuerspur::Sustain), schritte, mal);
    if (gedrueckt(wertBei(sustain, ende))) bis = std::max(bis, losgelassenNach(sustain, ende));

    // Sostenuto: getreten, waehrend die Note klingt (auch im Moment ihres Anschlags)
    const std::vector<Wechsel> sostenuto = wechselImClip(muster.zeitspuren.von(Steuerspur::Sostenuto), schritte, mal);
    double vorher = 0.0;
    for (const Wechsel& w : sostenuto)
    {
        const bool getreten = gedrueckt(w.wert) && !gedrueckt(vorher);
        vorher = w.wert;
        if (w.stelle >= ende) break;
        if (getreten && w.stelle >= anschlag) bis = std::max(bis, losgelassenNach(sostenuto, w.stelle));
    }
    // sass das Pedal schon vor dem Anschlag, faengt es die Note nicht (nur, was beim Treten klang)

    // derselbe Ton schlaegt im Clip neu an: dort endet der gehaltene
    for (int r = 0; r < mal; ++r)
        for (const Note& andere : muster.noten)
        {
            if (andere.hoehe != n.hoehe) continue;
            const double neu = r * schritte + andere.schritt;
            if (neu > anschlag && neu < bis) bis = std::max(ende, neu);
        }
    return std::min(bis, mal * schritte);
}

int gehaltenMs(const Loop& loop, const Muster& muster, int wiederholungen, int runde, std::size_t note)
{
    if (note >= muster.noten.size()) return 1;
    const Note& n = muster.noten[note];
    if (!hatPedal(muster) || loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return loop.notenMs(n);
    const double schritte = static_cast<double>(muster.takte) * loop.schritteJeTakt();
    const double anschlag = runde * schritte + n.schritt;
    const double bis = gehaltenBis(loop, muster, wiederholungen, runde, note);
    if (bis <= anschlag + n.laenge) return loop.notenMs(n);  // nicht gehalten: wie ohne Pedal
    const double schrittMs = 60000.0 / loop.bpm / loop.schritteJeSchlag;
    return std::max(loop.notenMs(n), static_cast<int>(std::lround((bis - anschlag) * schrittMs)));
}

}  // namespace lumi::loop
