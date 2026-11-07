/**
 * @file Tonauftrag.cpp
 * @brief InstrumentCore: der Tonauftrag (IK2-3 a)
 */
#include "InstrumentCore/Tonauftrag.hpp"

#include "InstrumentCore/Anschlag.hpp"

#include <algorithm>
#include <cmath>

namespace lumi::kern
{

const Spur* findeSpur(const Tonauftrag& auftrag, std::string_view name)
{
    for (const Spur& s : auftrag.spuren)
        if (s.name == name) return &s;
    return nullptr;
}

double spurWert(const Spur& spur, Frames frame, double ohne)
{
    const auto& p = spur.punkte;
    if (p.empty()) return ohne;
    if (frame <= p.front().frame) return p.front().wert;
    if (frame >= p.back().frame) return p.back().wert;
    const auto oben = std::upper_bound(p.begin(), p.end(), frame, [](Frames f, const SpurPunkt& q) { return f < q.frame; });
    const SpurPunkt& b = *oben;
    const SpurPunkt& a = *(oben - 1);
    if (b.frame == a.frame) return b.wert;
    const double t = static_cast<double>(frame - a.frame) / static_cast<double>(b.frame - a.frame);
    return a.wert + t * (b.wert - a.wert);
}

Tonauftrag einzelnote(int note, double hz, int stufe, Frames halten, Frames frames, double rate)
{
    Tonauftrag auftrag;
    auftrag.rate = rate;
    auftrag.stufe = stufe;
    auftrag.frames = frames;
    auftrag.noten.push_back(KettenNote{0, halten, note, hz, false});
    return auftrag;
}

std::vector<std::string> pruefeAuftrag(const Tonauftrag& auftrag)
{
    std::vector<std::string> fehler;
    if (!(auftrag.rate > 0.0) || !std::isfinite(auftrag.rate)) fehler.push_back("rate must be positive");
    if (auftrag.stufe < kAnschlagStufeMin || auftrag.stufe > 0) fehler.push_back("stufe outside -40 ... 0");
    if (auftrag.frames < 0) fehler.push_back("frames negative");
    for (std::size_t i = 0; i < auftrag.noten.size(); ++i)
    {
        const KettenNote& n = auftrag.noten[i];
        const std::string wo = "note " + std::to_string(i);
        if (n.ende <= n.beginn) fehler.push_back(wo + ": ende not after beginn");
        if (n.note < 0 || n.note > 127) fehler.push_back(wo + ": note outside 0 ... 127");
        if (!(n.hz > 0.0) || !std::isfinite(n.hz)) fehler.push_back(wo + ": hz must be positive");
        if (i > 0 && n.beginn < auftrag.noten[i - 1].beginn) fehler.push_back(wo + ": not ordered by beginn");
    }
    for (std::size_t i = 0; i < auftrag.spuren.size(); ++i)
    {
        const Spur& s = auftrag.spuren[i];
        for (std::size_t k = 1; k < s.punkte.size(); ++k)
            if (s.punkte[k].frame < s.punkte[k - 1].frame)
            {
                fehler.push_back("track " + s.name + ": points not ordered by frame");
                break;
            }
        for (std::size_t j = 0; j < i; ++j)
            if (auftrag.spuren[j].name == s.name)
            {
                fehler.push_back("track " + s.name + ": named twice");
                break;
            }
    }
    return fehler;
}

}  // namespace lumi::kern
