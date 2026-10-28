/**
 * @file Pedalplan.cpp
 * @brief Loop: wie lange Sustain und Sostenuto eine Note halten (Stufe 28b)
 */

#include "loop/Pedalplan.hpp"

#include "loop/HammerSaite.hpp"  // anschlagStufe (Stufe 31b)

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
    // 31d-9 (N1 a): Anfang und Dauer mit Versatz; ohne ihn wie bis Fassung 13
    const double anschlag = runde * schritte + notenAnfang(n);
    const double ende = anschlag + notenDauer(n);
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
            const double neu = r * schritte + notenAnfang(andere);
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
    const double anschlag = runde * schritte + notenAnfang(n);
    const double bis = gehaltenBis(loop, muster, wiederholungen, runde, note);
    if (bis <= anschlag + notenDauer(n)) return loop.notenMs(n);  // nicht gehalten: wie ohne Pedal
    const double schrittMs = 60000.0 / loop.bpm / loop.schritteJeSchlag;
    return std::max(loop.notenMs(n), static_cast<int>(std::lround((bis - anschlag) * schrittMs)));
}

bool spieltJeVorkommen(const Muster& muster)
{
    return hatPedal(muster) || !muster.zeitspuren.von(Steuerspur::Pitch).empty() ||
           !muster.zeitspuren.von(Steuerspur::Vibrato).empty();
}

namespace
{

/// Eine Kurve der Spur ueber den Ton: von `anschlag` bis `bis` (Schritte ab dem Anfang
/// des Clips), in Cent (`jeWert` Cent je Wert der Spur).
std::vector<TonPunkt> kurveImTon(const std::vector<SpurPunkt>& spur, double schritte, int mal, double anschlag,
                                 double bis, double schrittMs, double jeWert)
{
    std::vector<TonPunkt> punkte;
    if (spur.empty()) return punkte;
    const double clipEnde = mal * schritte;
    // der Wert an `x`: in seiner Wiederholung (`ab` = der Durchlauf, dessen Anfang gilt), hinter dem Clip der Wert dort
    const auto wert = [&](double x, int durchlauf) {
        const double imDurchlauf = std::min(x, clipEnde) - durchlauf * schritte;
        return kurveBei(spur, imDurchlauf, 0.0);
    };
    const auto setze = [&](double x, double w) {
        const TonPunkt p{static_cast<int>(std::lround((x - anschlag) * schrittMs)), static_cast<int>(std::lround(w * jeWert))};
        if (!punkte.empty() && punkte.back() == p) return;
        punkte.push_back(p);
    };
    const auto durchlaufVon = [&](double x) { return std::clamp(static_cast<int>(std::floor(x / schritte)), 0, mal - 1); };
    const int erster = durchlaufVon(anschlag);
    setze(anschlag, wert(anschlag, erster));
    for (int r = erster; r < mal; ++r)
    {
        const double davor = r * schritte;
        if (davor >= bis) break;
        if (r > erster)
        {
            // die Naht: der letzte Wert des vorigen Durchlaufs, dann der erste dieses
            setze(davor, wert(davor - 1e-9, r - 1));
            setze(davor, spur.front().wert);
        }
        for (const SpurPunkt& p : spur)
        {
            const double x = davor + p.stelle;
            if (x > anschlag && x < bis) setze(x, p.wert);
        }
    }
    setze(bis, wert(bis, durchlaufVon(std::min(bis, clipEnde) - 1e-9)));
    return punkte;
}

}  // namespace

int stufeDerNote(const Note& note, bool hammer)
{
    return hammer ? anschlagStufe(note.laut) : 0;
}

TonAngabe tonImClip(const Loop& loop, const Muster& muster, int wiederholungen, int runde, std::size_t note, bool hammer)
{
    TonAngabe ton;
    if (note >= muster.noten.size()) return ton;
    const Note& n = muster.noten[note];
    ton.instrument = muster.instrument;
    ton.hoehe = n.hoehe;
    ton.dauerMs = gehaltenMs(loop, muster, wiederholungen, runde, note);
    ton.anschlagStufe = stufeDerNote(n, hammer);  // Stufe 31b (HK2 a)
    setzeSpuren(ton, loop, muster, wiederholungen, runde, note);
    return ton;
}

void setzeSpuren(TonAngabe& ton, const Loop& loop, const Muster& muster, int wiederholungen, int runde, std::size_t note)
{
    ton.tonhoehe.clear();
    ton.vibrato.clear();
    if (note >= muster.noten.size() || loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return;
    const Note& n = muster.noten[note];
    const double schrittMs = 60000.0 / loop.bpm / loop.schritteJeSchlag;
    const double schritte = static_cast<double>(muster.takte) * loop.schritteJeTakt();
    const int mal = std::max(1, wiederholungen);
    const double anschlag = runde * schritte + notenAnfang(n);  // 31d-9 (N1 a): mit Versatz
    const double bis = anschlag + ton.dauerMs / schrittMs;
    ton.tonhoehe = kurveImTon(muster.zeitspuren.von(Steuerspur::Pitch), schritte, mal, anschlag, bis, schrittMs, 100.0);
    if (std::all_of(ton.tonhoehe.begin(), ton.tonhoehe.end(), [](const TonPunkt& p) { return p.cent == 0; }))
        ton.tonhoehe.clear();
    ton.vibrato = kurveImTon(muster.zeitspuren.von(Steuerspur::Vibrato), schritte, mal, anschlag, bis, schrittMs, 1.0);
}

void setzeAusdruck(TonAngabe& ton, const Loop& loop, const Muster& muster, const LoopClip& clip, int runde, std::size_t note,
                   double versatzMs)
{
    ton.ausdruckMuster.clear();
    ton.ausdruckClip.clear();
    if (note >= muster.noten.size() || loop.bpm <= 0.0 || loop.schritteJeSchlag <= 0) return;
    const double schrittMs = 60000.0 / loop.bpm / loop.schritteJeSchlag;
    const int jeTakt = loop.schritteJeTakt();
    const double schritte = static_cast<double>(muster.takte) * jeTakt;
    const int mal = std::max(1, clip.wiederholungen);
    const double clipEnde = mal * schritte;
    const double anschlag = runde * schritte + notenAnfang(muster.noten[note]) + versatzMs / schrittMs;  // 31d-9: mit Versatz
    // bis zum Ende des Clips, bei einem Ende des Tons bis dorthin (dahinter klingt er nicht)
    double bis = clipEnde;
    if (ton.endeMs > 0) bis = std::min(bis, anschlag + ton.endeMs / schrittMs);
    if (bis <= anschlag) bis = anschlag;
    const auto voll = [](const std::vector<PegelPunkt>& punkte) {
        return std::all_of(punkte.begin(), punkte.end(), [](const PegelPunkt& p) { return p.tausendstel == 1000; });
    };

    // die Spur des Musters, Durchlauf fuer Durchlauf wie Pitch und Vibrato
    for (const TonPunkt& p : kurveImTon(muster.zeitspuren.von(Steuerspur::Expression), schritte, mal, anschlag, bis,
                                        schrittMs, 1000.0))
        ton.ausdruckMuster.push_back({p.ms, std::clamp(p.cent, 0, 1000)});
    if (voll(ton.ausdruckMuster)) ton.ausdruckMuster.clear();

    // die Kurve am Clip (SK8 a, in Takten ab dem Anfang des Clips); hinter dem Ende des Clips der
    // Wert dort — auch wenn die Kurve dahinter Punkte traegt (ein gekuerzter Clip)
    if (!clip.expression.empty())
    {
        const auto setze = [&](double x) {
            const double w = kurveBei(clip.expression, std::min(x, clipEnde) / jeTakt, 1.0);
            const PegelPunkt p{static_cast<int>(std::lround((x - anschlag) * schrittMs)),
                               std::clamp(static_cast<int>(std::lround(w * 1000.0)), 0, 1000)};
            if (ton.ausdruckClip.empty() || !(ton.ausdruckClip.back() == p)) ton.ausdruckClip.push_back(p);
        };
        setze(anschlag);
        for (const SpurPunkt& p : clip.expression)
            if (const double x = p.stelle * jeTakt; x > anschlag && x < bis) setze(x);
        setze(bis);
        if (voll(ton.ausdruckClip)) ton.ausdruckClip.clear();
    }
}

}  // namespace lumi::loop
