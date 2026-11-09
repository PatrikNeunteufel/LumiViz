/**
 * @file stimme_stab.cpp
 * @brief Werkzeug: die Fenster der Bogenkraft am gestrichenen Stab (Klasse 4, Bogen × Stab nach T1 a) durch das Modell
 *        hindurch als erzeugte Datei `klassen/04_platte_stab/Stimmung.inc`
 *
 *   ic_stimme_stab <Pfad zu Stimmung.inc>
 */
#include "klassen/04_platte_stab/Stab.hpp"

#include <cstdio>
#include <fstream>
#include <string>

using namespace ic;
using namespace ic::stab;

namespace
{

void kurve(std::ofstream& aus, const char* feld, const Kurve& k)
{
    if (k.punkte.empty()) return;
    aus << "    s." << feld << " = kurveAusPunkten({";
    for (std::size_t i = 0; i < k.punkte.size(); ++i)
    {
        char z[64];
        std::snprintf(z, sizeof z, "%s{%d, %.17g}", i ? ", " : "", k.punkte[i].note, k.punkte[i].wert);
        aus << z;
    }
    aus << "}, Belegkraft::Rechnung, \"" << k.herkunft << "\");\n";
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "Aufruf: ic_stimme_stab <Pfad zu Stimmung.inc>\n");
        return 2;
    }
    std::ofstream aus(argv[1]);
    aus << "// Erzeugt von werkzeuge/stimme_stab.cpp (ic_stimme_stab) — nicht von Hand aendern.\n";
    StabSpiel s = baueVorgabeVibraphon(false);
    stimmeBogen(s);
    aus << "inline void stimmungVibraphonBogen(StabSpiel& s)\n{\n";
    for (int k = 0; k < 3; ++k)
    {
        const std::string i = "[" + std::to_string(k) + "]";
        kurve(aus, ("bogenKraftUnten" + i).c_str(), s.bogenKraftUnten[static_cast<std::size_t>(k)]);
        kurve(aus, ("bogenKraftOben" + i).c_str(), s.bogenKraftOben[static_cast<std::size_t>(k)]);
    }
    aus << "}\n";
    std::printf("Vibraphon (Bogen) gestimmt\n");
    return 0;
}
