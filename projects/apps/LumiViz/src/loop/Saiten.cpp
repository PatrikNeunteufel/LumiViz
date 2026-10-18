/**
 * @file Saiten.cpp
 * @brief Loop: die Saiten eines Saiteninstruments (Stufe 23a)
 */

#include "loop/Saiten.hpp"

#include <algorithm>

namespace lumi::loop
{

void richteSaiten(Saitensatz& saiten)
{
    if (saiten.leer.size() > static_cast<std::size_t>(kSaitenMax)) saiten.leer.resize(kSaitenMax);
    for (int& ton : saiten.leer) ton = std::clamp(ton, 0, 127);
    saiten.capo = std::clamp(saiten.capo, 0, kCapoMax);
    if (saiten.wahl != Saitenwahl::Hand) saiten.wahl = Saitenwahl::JeKlang;
}

const std::vector<Stimmungsvorlage>& stimmungsvorlagen()
{
    // die Toene als MIDI-Nummern: E2 = 40, A2 = 45, D3 = 50, G3 = 55, B3 = 59, E4 = 64
    static const std::vector<Stimmungsvorlage> vorlagen = {
        {"guitar", "Guitar standard", {40, 45, 50, 55, 59, 64}},
        {"dropD", "Drop D", {38, 45, 50, 55, 59, 64}},
        {"bass4", "Bass 4", {28, 33, 38, 43}},
        {"bass5", "Bass 5", {23, 28, 33, 38, 43}},
        {"ukulele", "Ukulele", {67, 60, 64, 69}},
        {"violin", "Violin", {55, 62, 69, 76}},
    };
    return vorlagen;
}

const Stimmungsvorlage* vorlageVon(const Saitensatz& saiten)
{
    if (!saiten.istSaiteninstrument()) return nullptr;
    for (const Stimmungsvorlage& vorlage : stimmungsvorlagen())
        if (vorlage.leer == saiten.leer) return &vorlage;
    return nullptr;
}

}  // namespace lumi::loop
