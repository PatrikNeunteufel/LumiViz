/**
 * @file TestHilfe.hpp
 * @brief Hilfen fuer die Tests: der Abdruck je Compiler als Bitgleich-Waechter (AUFTRAG_BAU §4 Punkt 3f; wie
 *        `klangfassung.txt` im Vorbild) und die Rechenzeit
 *
 * Abdruecke stehen je Compiler in `werte/abdruck_<compiler>.txt` (Zeilen „Name<TAB>SHA-256“). Mit
 * `IC_ABDRUCK_SCHREIBEN=1` wird die Datei neu geschrieben (nach einer gewollten Aenderung am Klang); sonst wird
 * verglichen — ein fehlender Eintrag ist ein Fehler. Nach E1 a gilt die Bitgleichheit je Build; ein anderer Compiler
 * hat seine eigene Datei.
 */
#pragma once

#include "gemeinsam/Werkzeug.hpp"

#include <cstdlib>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

namespace ic::test
{

inline std::string compilerName()
{
#if defined(__clang__)
    return "clang";
#elif defined(_MSC_VER)
    return "msvc";
#elif defined(__GNUC__)
    return "gcc";
#else
    return "anderer";
#endif
}

inline std::string abdruckDatei()
{
    return std::string(IC_WERTE) + "/abdruck_" + compilerName() + ".txt";
}

/// Die Abdruecke der Datei (einmal gelesen); beim Schreiben die neuen.
struct Abdruecke
{
    std::map<std::string, std::string> alt;
    std::map<std::string, std::string> neu;
    bool schreiben = false;
    std::mutex sperre;

    Abdruecke()
    {
        schreiben = std::getenv("IC_ABDRUCK_SCHREIBEN") != nullptr;
        std::ifstream ein(abdruckDatei());
        std::string zeile;
        while (std::getline(ein, zeile))
        {
            if (zeile.empty() || zeile[0] == '#') continue;
            const auto tab = zeile.find('\t');
            if (tab != std::string::npos) alt[zeile.substr(0, tab)] = zeile.substr(tab + 1);
        }
    }

    static Abdruecke& alle()
    {
        static Abdruecke a;
        return a;
    }

    ~Abdruecke()
    {
        if (!schreiben) return;
        std::map<std::string, std::string> alles = alt;
        for (const auto& [k, v] : neu) alles[k] = v;
        std::ofstream aus(abdruckDatei());
        aus << "# Abdruecke (SHA-256 der float-Werte) je Ton unter " << compilerName()
            << " — geschrieben mit IC_ABDRUCK_SCHREIBEN=1 (rechnung/werkzeuge/TestHilfe.hpp)\n";
        for (const auto& [k, v] : alles) aus << k << '\t' << v << '\n';
    }
};

/// Gibt den erwarteten Abdruck zurueck (leer, wenn keiner da ist) und merkt den neuen.
inline std::string erwarteterAbdruck(const std::string& name, const std::string& abdruck)
{
    Abdruecke& a = Abdruecke::alle();
    const std::lock_guard<std::mutex> halt(a.sperre);
    a.neu[name] = abdruck;
    if (a.schreiben) return abdruck;
    const auto it = a.alt.find(name);
    return it == a.alt.end() ? std::string() : it->second;
}

/// Die Rechenzeit des Fluegel-Kerns im tiefsten Ton (A0, Stufe 0, 2 s gehalten, 48 kHz) auf dieser Maschine, als
/// Anteil der Echtzeit — einmal gemessen. Die Grenze der Klassen ist relativ dazu: der Fluegel liegt in LumiViz bei
/// 4,6 % (SCHNITTSTELLE.md), die Grenze bei rund 5 % (AUFTRAG_BAU §4 Punkt 3e); auf einer anderen Maschine gilt
/// dasselbe Verhaeltnis (Grenze = Fluegel hier · 5/4,6).
double fluegelAnteil();

inline double echtzeitGrenzeHier()
{
    return fluegelAnteil() * kEchtzeitGrenze / 0.046;
}

}  // namespace ic::test
