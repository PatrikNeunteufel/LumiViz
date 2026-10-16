/**
 * @file Farbreihen.cpp
 * @brief Die Farbreihen der Zeilen in den Achsen (S100, S128)
 */

#include "UI/widgets/Farbreihen.hpp"

#include <array>
#include <cstddef>

namespace lumi::ui
{

std::int32_t reihenFarbe(Farbreihe reihe, int nr)
{
    // kühl — Blau (die Farbe, die CH1 schon immer hatte), Violett, Grün,
    // Türkis, Magenta, Indigo; warm — Orange (der Ton aller Audio-Clips bis
    // S99), Rot, Gelb, Rosa, Rotorange, Ocker.
    static constexpr std::array<std::int32_t, kReihenFarben> kuehl = {0x5785A5, 0x8157A5, 0x57A57E,
                                                                     0x57A5A5, 0xA557A5, 0x5757A5};
    static constexpr std::array<std::int32_t, kReihenFarben> warm = {0x967855, 0x965555, 0x968D55,
                                                                    0x96556B, 0x966755, 0x968555};
    const auto platz = static_cast<std::size_t>(((nr % kReihenFarben) + kReihenFarben) % kReihenFarben);
    return reihe == Farbreihe::Warm ? warm[platz] : kuehl[platz];
}

}  // namespace lumi::ui
