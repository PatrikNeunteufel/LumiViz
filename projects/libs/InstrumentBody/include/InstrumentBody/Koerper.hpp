/**
 * @file Koerper.hpp
 * @brief Ein gerechneter Instrumentkoerper: was die Datei `.lvinstbody` traegt
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S146–S148: KB1 a, BM3 a,
 *        BM5 a, PL31c-5 a, ST1 a; Stufe 31c-5d)
 *
 * Die gerechneten Moden (je Mode Frequenz, Verlustfaktor, Wert der Form je Stegpunkt,
 * Abstrahlgewicht je Hoerort) und darueber der statistische Teil (seine Groessen und der Samen;
 * gezogen wird beim Bilden der Antworten). Dazu, woraus und wie der Koerper entstand: der
 * Abdruck des Bauplans und die Fassung der Rechnung — ein Test meldet einen Koerper, der nicht
 * mehr zu seinem Bauplan oder zur Rechnung passt (BM3 a, Muster des Waechters der Klangfassung).
 *
 * Lesen und schreiben mit Qt-JSON in der App (`loop/Koerper`); hier nur die Daten.
 */
#pragma once

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/Statistik.hpp"

#include <string>
#include <vector>

namespace lumi::koerper
{

/// Die Fassung der Datei `.lvinstbody`, die dieses Programm schreibt; eine hoehere liest es nicht.
inline constexpr int kKoerperFassung = 1;

/// Die Fassung der Rechnung (Netz, Platte, Loeser, Auswertung, statistischer Teil). Hochzaehlen,
/// wenn sich bei gleichem Bauplan ein anderer Koerper ergaebe — dann meldet der Waechter jeden
/// mitgelieferten Koerper als veraltet, bis das Werkzeug ihn neu rechnet.
inline constexpr int kRechenFassung = 1;

/// Wie ein Koerper gerechnet wurde.
struct Rechnung
{
    int fassung = 0;               ///< kRechenFassung beim Rechnen
    std::string netzgeber;         ///< Name der Umsetzung
    std::string loeser;            ///< Name der Umsetzung
    int ordnung = 0;               ///< k der Elemente
    double netzFuerHz = 0.0;       ///< die Elementgroesse nach MAESSTRO Gl. 1 fuer diese Frequenz
    double vonHz = 0.0;            ///< das Fenster der gerechneten Moden
    double bisHz = 0.0;
    int freieGroessen = 0;         ///< zur Auskunft
    double sekunden = 0.0;         ///< zur Auskunft

    [[nodiscard]] bool operator==(const Rechnung&) const = default;
};

struct Koerper
{
    std::string titel;
    std::string beschreibung;
    std::string bauplan;           ///< der Name der Bauplan-Datei, ohne Ordner
    std::string bauplanAbdruck;    ///< SHA-256 des Bauplans (wie `loop/Koerper` ihn bildet)
    Rechnung rechnung;
    std::vector<int> stegTasten;   ///< die Stegpunkte in der Reihenfolge der Werte je Mode (MIDI)
    std::vector<std::string> hoerorte;  ///< die Hoerorte in der Reihenfolge der Gewichte je Mode
    Luft luft;
    Daempfung daempfung;
    std::vector<ModeImKoerper> moden;   ///< aufsteigend
    StatistischerTeil statistik;        ///< dichte = 0: keiner

    [[nodiscard]] bool operator==(const Koerper&) const = default;
};

}  // namespace lumi::koerper
