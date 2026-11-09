/**
 * @file Kerne.cpp
 * @brief InstrumentModels: die Fabrik der eingebauten Kerne (BK1 a)
 */
#include "InstrumentModels/Kerne.hpp"

#include "InstrumentModels/hammer/HammerKern.hpp"
#include "InstrumentModels/saite/SaiteKern.hpp"
#include "InstrumentModels/stab/StabKern.hpp"
#include "InstrumentModels/trommel/TrommelKern.hpp"

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
        [[maybe_unused]] const std::vector<std::string> fehlerStab = f.registriere(stab::kernTyp());  // 34g: Klasse 4
        assert(fehlerStab.empty());
        [[maybe_unused]] const std::vector<std::string> fehlerTrommel = f.registriere(trommel::kernTyp());  // 34h: Klasse 5
        assert(fehlerTrommel.empty());
        [[maybe_unused]] const std::vector<std::string> fehlerSaite = f.registriere(saite::kernTyp());  // 34i: Klasse 1 Rest
        assert(fehlerSaite.empty());
        return f;
    }();
    return eingebaut;
}

}  // namespace lumi::kern
