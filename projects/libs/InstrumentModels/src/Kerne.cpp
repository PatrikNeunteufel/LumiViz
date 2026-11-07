/**
 * @file Kerne.cpp
 * @brief InstrumentModels: die Fabrik der eingebauten Kerne (BK1 a)
 */
#include "InstrumentModels/Kerne.hpp"

#include "InstrumentModels/hammer/HammerKern.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace lumi::kern
{

const Fabrik& kerne()
{
    static const Fabrik eingebaut = [] {
        Fabrik f;
        [[maybe_unused]] const std::vector<std::string> fehler = f.registriere(hammer::kernTyp());
        assert(fehler.empty());
        return f;
    }();
    return eingebaut;
}

}  // namespace lumi::kern
