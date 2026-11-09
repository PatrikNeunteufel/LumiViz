/**
 * @file stimme_zunge.cpp
 * @brief Werkzeug: die Stimmung der Zungeninstrumente (Klasse 10) durch das Modell hindurch als erzeugte Datei
 *        `klassen/10_zunge/Stimmung.inc` (f₀ und Fenster des Drucks je Note)
 *
 *   ic_stimme_zunge <Pfad zu Stimmung.inc>
 */
#include "klassen/10_zunge/Zunge.hpp"

#include <cstdio>
#include <fstream>

using namespace ic;
using namespace ic::zunge;

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
        std::fprintf(stderr, "Aufruf: ic_stimme_zunge <Pfad zu Stimmung.inc>\n");
        return 2;
    }
    std::ofstream aus(argv[1]);
    aus << "// Erzeugt von werkzeuge/stimme_zunge.cpp (ic_stimme_zunge) — nicht von Hand aendern.\n";
    for (const auto& [name, bau] : {std::pair{"Akkordeon", &baueVorgabeAkkordeon}, std::pair{"Harmonium", &baueVorgabeHarmonium},
                                    std::pair{"Melodica", &baueVorgabeMelodica}, std::pair{"Mundharmonika", &baueVorgabeMundharmonika}})
    {
        ZungenSpiel s = bau(false);
        stimme(s);
        aus << "inline void stimmung" << name << "(ZungenSpiel& s)\n{\n";
        kurve(aus, "eigenfrequenzJeNote", s.eigenfrequenzJeNote);
        kurve(aus, "druckUntenJeNote", s.druckUntenJeNote);
        kurve(aus, "druckObenJeNote", s.druckObenJeNote);
        aus << "}\n";
        std::printf("%s gestimmt\n", name);
        std::fflush(stdout);
    }
    return 0;
}
