/**
 * @file stimme_luftstrahl.cpp
 * @brief Werkzeug: die Stimmung der Floeten und Labialpfeifen (Klasse 9) durch das Modell hindurch als erzeugte Datei
 *        `klassen/09_luftstrahl_rohr/Stimmung.inc` (wirksame Laenge, Fenster des Drucks bzw. Aufschnitt, Register je
 *        Note)
 *
 *   ic_stimme_luftstrahl <Pfad zu Stimmung.inc>
 */
#include "klassen/09_luftstrahl_rohr/Luftstrahl.hpp"

#include <cstdio>
#include <fstream>

using namespace ic;
using namespace ic::luftstrahl;

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
        std::fprintf(stderr, "Aufruf: ic_stimme_luftstrahl <Pfad zu Stimmung.inc>\n");
        return 2;
    }
    std::ofstream aus(argv[1]);
    aus << "// Erzeugt von werkzeuge/stimme_luftstrahl.cpp (ic_stimme_luftstrahl) — nicht von Hand aendern.\n";
    for (const auto& [name, bau] : {std::pair{"Blockfloete", &baueVorgabeBlockfloete}, std::pair{"Labialpfeife", &baueVorgabeLabialpfeife},
                                    std::pair{"Shakuhachi", &baueVorgabeShakuhachi}})
    {
        StrahlSpiel s = bau(false);
        stimme(s);
        aus << "inline void stimmung" << name << "(StrahlSpiel& s)\n{\n";
        kurve(aus, "laengeJeNote", s.laengeJeNote);
        kurve(aus, "druckUntenJeNote", s.druckUntenJeNote);
        kurve(aus, "druckObenJeNote", s.druckObenJeNote);
        kurve(aus, "aufschnittJeNote", s.aufschnittJeNote);
        aus << "    s.registerJeNote = {";
        for (std::size_t i = 0; i < s.registerJeNote.size(); ++i) aus << (i ? ", " : "") << s.registerJeNote[i];
        aus << "};\n}\n";
        std::printf("%s gestimmt\n", name);
        std::fflush(stdout);
    }
    return 0;
}
