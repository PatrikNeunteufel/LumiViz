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
    // 0 = nicht gesetzt (auch, was keine Zahl ist), sonst in den Grenzen
    saiten.mensurMm = saiten.mensurMm > 0.0 ? std::clamp(saiten.mensurMm, kMensurMinMm, kMensurMaxMm) : 0.0;
    if (saiten.wahl != Saitenwahl::Hand) saiten.wahl = Saitenwahl::JeKlang;
}

const std::vector<Stimmungsvorlage>& stimmungsvorlagen()
{
    // die Toene als MIDI-Nummern: E2 = 40, A2 = 45, D3 = 50, G3 = 55, B3 = 59, E4 = 64.
    // Die Mensuren (Saiten_Konzept §6b): Gitarre 25,5" = 648 mm (Errede, UIUC Physics 406,
    // „Fourier Analysis IV", S. 2); E-Bass 34" = 864 mm (Fender, Herstellerangabe ueber
    // Haendler); Ukulele Konzert 15" = 381 mm (Groessenklasse 15-16"); Violine 327 mm
    // (Jansson, KTH 2002, S. 7.3).
    static const std::vector<Stimmungsvorlage> vorlagen = {
        {"guitar", "Guitar standard", {40, 45, 50, 55, 59, 64}, 648.0},
        {"dropD", "Drop D", {38, 45, 50, 55, 59, 64}, 648.0},
        {"bass4", "Bass 4", {28, 33, 38, 43}, 864.0},
        {"bass5", "Bass 5", {23, 28, 33, 38, 43}, 864.0},
        {"ukulele", "Ukulele", {67, 60, 64, 69}, 381.0},
        {"violin", "Violin", {55, 62, 69, 76}, 327.0},
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
