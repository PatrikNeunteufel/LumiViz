/**
 * @file stimme_blech.cpp
 * @brief Werkzeug: die Stimmung der Blechblaeser (Klasse 7) durch das Modell hindurch rechnen und als erzeugte Datei
 *        `klassen/07_lippe_rohr/Stimmung.inc` ablegen (Register, Zuglage, Anblasschwelle, Lippe je Note bei pp/mf/ff)
 *
 *   ic_stimme_blech <Pfad zu Stimmung.inc>
 *
 * Die Vorgaben (`Vorgaben.cpp`) lesen die Datei beim Bauen ein; ohne sie stimmen sie selbst (dauert ueber eine
 * Minute je Instrument). Nach jeder Aenderung am Modell neu erzeugen — der Test `Blech: gestimmte Vorgabe trifft`
 * meldet eine veraltete Datei.
 */
#include "klassen/07_lippe_rohr/Blech.hpp"

#include <cstdio>
#include <fstream>

using namespace ic;
using namespace ic::blech;

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
        std::fprintf(stderr, "Aufruf: ic_stimme_blech <Pfad zu Stimmung.inc>\n");
        return 2;
    }
    std::ofstream aus(argv[1]);
    aus << "// Erzeugt von werkzeuge/stimme_blech.cpp (ic_stimme_blech) — nicht von Hand aendern.\n"
           "// Die Stimmung der Blechblaeser durch das Modell hindurch (stimme), je Note ein Stuetzpunkt.\n";
    for (const auto& [name, bau] : {std::pair{"Posaune", &baueVorgabePosaune}, std::pair{"Alphorn", &baueVorgabeAlphorn},
                                     std::pair{"Trompete", &baueVorgabeTrompete}})
    {
        BlechSpiel s = bau(false);
        bereite(s);
        stimme(s, 1);
        aus << "inline void stimmung" << name << "(BlechSpiel& s)\n{\n";
        kurve(aus, "lippeJeNote", s.lippeJeNote);
        kurve(aus, "lippeJeNoteLeise", s.lippeJeNoteLeise);
        kurve(aus, "lippeJeNoteLaut", s.lippeJeNoteLaut);
        kurve(aus, "zugJeNote", s.zugJeNote);
        kurve(aus, "schwelleJeNote", s.schwelleJeNote);
        aus << "}\n";
        std::printf("%s gestimmt\n", name);
    }
    return 0;
}
