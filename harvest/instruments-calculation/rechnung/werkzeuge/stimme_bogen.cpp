/**
 * @file stimme_bogen.cpp
 * @brief Werkzeug: die Stimmung der Streichinstrumente (Klasse 3) durch das Modell hindurch als erzeugte Datei
 *        `klassen/03_bogen_saite/Stimmung.inc` (Feinstimmung der Laenge, Fenster der Bogenkraft je Note)
 *
 *   ic_stimme_bogen <Pfad zu Stimmung.inc>
 */
#include "klassen/03_bogen_saite/Bogen.hpp"

#include <cstdio>
#include <fstream>

using namespace ic;
using namespace ic::bogen;

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
        std::fprintf(stderr, "Aufruf: ic_stimme_bogen <Pfad zu Stimmung.inc>\n");
        return 2;
    }
    std::ofstream aus(argv[1]);
    aus << "// Erzeugt von werkzeuge/stimme_bogen.cpp (ic_stimme_bogen) — nicht von Hand aendern.\n";
    for (const auto& [name, bau] : {std::pair{"Cello", &baueVorgabeCello}, std::pair{"Violine", &baueVorgabeVioline},
                              std::pair{"Viola", &baueVorgabeViola}, std::pair{"Kontrabass", &baueVorgabeKontrabass}})
    {
        StreichSpiel s = bau(false);
        stimme(s);
        aus << "inline void stimmung" << name << "(StreichSpiel& s)\n{\n";
        kurve(aus, "laengeJeNote", s.laengeJeNote);
        kurve(aus, "kraftUntenJeNote", s.kraftUntenJeNote);
        kurve(aus, "kraftObenJeNote", s.kraftObenJeNote);
        kurve(aus, "kraftUntenFfJeNote", s.kraftUntenFfJeNote);
        kurve(aus, "kraftObenFfJeNote", s.kraftObenFfJeNote);
        aus << "}\n";
        std::printf("%s gestimmt\n", name);
        std::fflush(stdout);
    }
    return 0;
}
