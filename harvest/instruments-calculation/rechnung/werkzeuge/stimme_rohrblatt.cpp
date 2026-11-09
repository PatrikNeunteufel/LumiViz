/**
 * @file stimme_rohrblatt.cpp
 * @brief Werkzeug: die Stimmung der Rohrblattinstrumente (Klasse 8) durch das Modell hindurch als erzeugte Datei
 *        `klassen/08_rohrblatt_rohr/Stimmung.inc` (wirksame Laenge und Schwelle je Note)
 *
 *   ic_stimme_rohrblatt <Pfad zu Stimmung.inc>
 */
#include "klassen/08_rohrblatt_rohr/Rohrblatt.hpp"

#include <cstdio>
#include <fstream>

using namespace ic;
using namespace ic::rohrblatt;

namespace
{

void kurve(std::ofstream& aus, const char* feld, const Kurve& k)
{
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
        std::fprintf(stderr, "Aufruf: ic_stimme_rohrblatt <Pfad zu Stimmung.inc>\n");
        return 2;
    }
    std::ofstream aus(argv[1]);
    aus << "// Erzeugt von werkzeuge/stimme_rohrblatt.cpp (ic_stimme_rohrblatt) — nicht von Hand aendern.\n";
    for (const auto& [name, bau] : {std::pair{"Klarinette", &baueVorgabeKlarinette}, std::pair{"Sopransaxophon", &baueVorgabeSopransaxophon},
                                    std::pair{"Tenorsaxophon", &baueVorgabeTenorsaxophon}, std::pair{"Altsaxophon", &baueVorgabeAltsaxophon}})
    {
        RohrblattSpiel s = bau(false);
        stimme(s);
        aus << "inline void stimmung" << name << "(RohrblattSpiel& s)\n{\n";
        kurve(aus, "laengeJeNote", s.laengeJeNote);
        kurve(aus, "schwelleJeNote", s.schwelleJeNote);
        kurve(aus, "obergrenzeJeNote", s.obergrenzeJeNote);
        aus << "}\n";
        std::printf("%s gestimmt\n", name);
        std::fflush(stdout);
    }
    return 0;
}
