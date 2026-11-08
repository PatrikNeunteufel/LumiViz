/**
 * @file Setzen.hpp
 * @note Uebernommen aus IC `rechnung/setzen/Setzen.hpp` (bb7e6d6, IC Session 5; Instrumente_IC_Konzept.md 34e-4,
 *       ME3 a, Journal J24). Geaendert: Namensraum `lumi::messung::setzen`; innen das LM je Block aus LumiViz
 *       (`levenbergMarquardtJeBlock`, 7.5.12 A2/A3) statt des eigenen LM je Gruppe; dazu Schritt und Ende je
 *       Parameter, ein Beobachter je Iteration und ein Startzustand fuer die Wiederaufnahme (ME3 a).
 * @brief Das Setz-Werkzeug (AUFTRAG_BAU §6, Schritt 5): Levenberg–Marquardt mit Grenzen nach dem Vorbild
 *        (`messung/LevenbergMarquardt.hpp`), aber **mit den Lehren L8–L15** (`VORBILD_FLUEGEL.md`)
 *
 * - **L8** — unabhaengige Gruppen (je Note): je Gruppe eigenes λ, eigene Annahme, eigenes Ende; die Auswertung aller
 *   Gruppen (Jacobi-Spalten, Versuchsschritte) je in **einem** Pool-Aufruf. In LumiViz rechnet das LM je Block
 *   (`InstrumentMeasure/LevenbergMarquardt.hpp`, eine Gruppe = ein Block) — mit **A3**: ein Parameter, dessen Wirkung
 *   √diag(JᵀJ) im normierten Raum unter `Einstellung::wirkungMindestens` Toleranzen liegt, bekommt in dieser Iteration
 *   keinen Schritt (die Entscheidung faellt je Iteration neu; L9 setzt die ganz wirkungslosen vorab fest).
 * - **L9** — die Wirkung je Parameter vor dem Lauf (Diagonale von JᵀJ): wirkungslose bleiben fest und stehen im Bericht.
 * - **L11** — verrauschte Spalten (die Jacobi-Spalte mit h und h/2 verschieden) als Warnung.
 * - **L12** — wellige Groessen zuerst eindimensional durch das Modell (Raster, dann goldener Schnitt), danach der Rest.
 * - **L16** (berichtigt L12) — tragen zwei freie Parameter dieselbe Wahrnehmung (`traegt`, z. B. die Schwebung aus
 *   Chor-Verstimmung und Polarisation), wird keiner von ihnen allein gestaffelt; sie stehen im Bericht als „nicht
 *   gestaffelt“. Die Glaette des **Ziels** je Parameter prueft `welligkeit` vor dem Plan.
 * - **L18** — das Ergebnis ist ein Vorschlag; gesetzt ist erst, was die Probe bestanden hat und gehoert wurde.
 * - **L7, L14** — Bericht mit Treffern **und** Rest, Diagonale, Grenzbefund; je Iteration eine Datei, die Wiederaufnahme
 *   liest die letzte; der Schnitt je Parameter und LM je Gruppe allein als Werkzeuge.
 * - **L13 mit HW1 a/HW2 a** (IC-A2) — die Hoerschwellen stehen je Groessenname in `werte/hoerschwellen.txt`
 *   (`messung/Hoerschwellen.hpp`, Skalen Faktor, dB, Ewert, unsymmetrisch); „hoerbar getroffen“ ist |d| ≤ max(τ, S).
 * - **HW2 a bis HW4 a** (IC-A3) — mehrere Startpunkte, der beste je Gruppe (A5), eindeutig ueber die Startpunkte, die
 *   Glaette je Parameter (ZF8 a (2)), die Probe (HW3 a) und systematische Groessen (`messung/Probe.hpp`).
 *
 * Gesucht wird im normierten Raum u ∈ [0, 1] je Parameter (linear oder logarithmisch zwischen den Grenzen); der Rest
 * ist r_i = (Messung_i − Ziel_i)/Toleranz_i, bei Skala Faktor ln(Messung_i/Ziel_i)/Toleranz_i (wie `Zielsumme`).
 * Deterministisch: die Reihenfolge der Gruppen und Spalten ist fest, der Pool aendert nur die Verteilung (`parallelJe`).
 */
#pragma once

#include "InstrumentMeasure/Hoerschwellen.hpp"
#include "InstrumentMeasure/Probe.hpp"

#include <cmath>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace lumi::messung::setzen
{

enum class Skala
{
    Linear,
    Log,
};

struct Parameter
{
    std::string name;
    double wert = 0.0;
    double unten = 0.0, oben = 1.0;
    Skala skala = Skala::Linear;
    bool wellig = false;  ///< L12: zuerst eindimensional
    bool fest = false;    ///< nicht gesucht (von Hand oder nach L9)
    std::string traegt{}; ///< L16: die Wahrnehmung, die er traegt (leer = keine geteilte)
    bool grenzeBelegt = false;  ///< HW3 a (3): die Grenzen haben eine Quelle; endet er dort, ist die Gruppe nicht erreichbar
    double schritt = 0.0;       ///< ME3 a: Differenz der Jacobi-Matrix im normierten Raum (0 = `Einstellung::schrittH`)
    double ende = 0.0;          ///< ME3 a: Schritt im normierten Raum, darunter klein (0 = `Einstellung::ende`)
};

struct Messgroesse
{
    std::string name;  ///< auch der Schluessel in `werte/hoerschwellen.txt` (laengstes passendes Praefix)
    double ziel = 0.0;
    double toleranz = 1.0;  ///< in der Skala: bei Faktor in ln, sonst in der Einheit der Groesse
    messung::Skala skala = messung::Skala::Dezibel;  ///< Faktor: r = ln(m/z)/τ; Dezibel und Cent: r = (m − z)/τ
};

/// Eine unabhaengige Gruppe (eine Note): ihre Parameter wirken nur auf ihre Messgroessen.
struct Gruppe
{
    std::string name;
    std::vector<Parameter> parameter;
    std::vector<Messgroesse> ziele;
};

/// Das Modell: die Messgroessen der Gruppe `gruppe` bei den Werten `werte` (in Parameter-Einheiten).
using Modell = std::function<std::vector<double>(std::size_t gruppe, const std::vector<double>& werte)>;

/// ME3 a: was eine Iteration je Gruppe getan hat (fuer Dateien des Aufrufers).
struct GruppenSchritt
{
    bool aktiv = false;  ///< in dieser Iteration gerechnet
    bool angenommen = false;
    double restVorher = 0.0, rest = 0.0;  ///< Σ r²
    double lambdaVorher = 0.0, lambda = 0.0;
    int versuche = 0, verworfen = 0;
    int iterationen = 0;                ///< so oft war die Gruppe aktiv
    std::vector<double> werte;          ///< nach der Iteration, in Parameter-Einheiten
    std::vector<double> diagonale;      ///< von JᵀJ im normierten Raum je Parameter (0: nicht frei), vor dem Schritt
    std::vector<std::size_t> ohneWirkung;  ///< A3: Parameter (Index in der Gruppe) ohne Schritt
    std::string ende;                   ///< leer, solange die Gruppe weiterlaeuft
};

struct Iteration
{
    int nummer = 0;
    std::vector<GruppenSchritt> gruppen;
    int auswertungen = 0;  ///< Auswertungen des Modells (Gruppe × Punkt) dieser Iteration
};

using Beobachter = std::function<void(const Iteration&)>;

/// ME3 a: wo die Suche wieder aufnimmt (die Werte stehen in den Gruppen); leere Listen heissen λ-Start, 0 Iterationen,
/// aktiv. Geht vor `Einstellung::protokoll`.
struct Start
{
    int nummer = 0;  ///< Iterationen schon gelaufen
    std::vector<double> lambda;
    std::vector<int> iteration;
    std::vector<std::string> ende;
};

struct Einstellung
{
    int iterationenHoechstens = 40;
    int versucheHoechstens = 8;
    double lambdaStart = 1e-2;
    double lambdaFaktor = 10.0;
    double ende = 0.001;          ///< Schritt im normierten Raum (je Parameter), darunter fertig
    double schrittH = 1e-3;       ///< Differenz fuer die Jacobi-Matrix (normiert)
    double wirkungMin = 1e-8;     ///< L9: Diagonale unter wirkungMin·max — wirkungslos
    double rauschGrenze = 0.3;    ///< L11: relative Abweichung der Spalte mit h und h/2
    double wirkungMindestens = 0.1;  ///< A3 (LumiViz 7.5.12, SP3 a): darunter kein Schritt in dieser Iteration (Toleranzen)
    int wellenRaster = 41;        ///< L12
    std::string protokoll;        ///< L7: Ordner fuer die Dateien je Iteration (leer = keine)
    bool wiederaufnahme = true;
    unsigned threads = 0;
    const messung::Hoerschwellen* hoerschwellen = nullptr;  ///< HW1 a; ohne: hoerbar getroffen = getroffen
    Beobachter beobachter{};      ///< ME3 a: nach jeder Iteration
    std::optional<Start> start{}; ///< ME3 a: Wiederaufnahme durch den Aufrufer
};

struct GruppenBericht
{
    std::string name;
    std::vector<double> werte;
    double restAnfang = 0.0, rest = 0.0;  ///< Σ r²
    int treffer = 0;                      ///< |r| ≤ 1 (ZF1 a)
    int hoerbarGetroffen = 0;             ///< HW2 a: |d| ≤ max(τ, S)
    std::vector<messung::Abweichung> groessenAnfang, groessen;  ///< je Messgroesse am Start und am Ende
    std::vector<double> diagonale;        ///< von JᵀJ am Ende
    std::vector<std::string> amRand, wirkungslos, verrauscht, nichtGestaffelt;  ///< nichtGestaffelt: L16
    std::vector<std::string> ohneWirkung;  ///< A3: in der letzten Iteration ohne Schritt
    double lambda = 0.0;                   ///< am Ende
    int iterationen = 0;
    std::string ende;
};

struct Bericht
{
    std::vector<GruppenBericht> gruppen;
    int ersteIteration = 0;  ///< > 0: aus der Wiederaufnahme
    [[nodiscard]] std::string text() const;
};

/// Setzt alle Gruppen (die Werte stehen danach in `gruppen`).
Bericht setze(std::vector<Gruppe>& gruppen, const Modell& modell, const Einstellung& e = {});

/// L14: der Schnitt je Parameter — `punkte` Werte ueber den Bereich, je Wert die Messgroessen.
[[nodiscard]] std::vector<std::pair<double, std::vector<double>>> schnitt(const Gruppe& g, std::size_t gruppe, std::size_t parameter,
                                                                          const Modell& modell, int punkte = 15);

/// L16: die Glaette des Ziels in einem Parameter — je Messgroesse die Zahl der inneren Minima von |r| auf dem Schnitt
/// (`punkte` Werte). Mehr als eins: das Ziel ist in diesem Parameter wellig; dann die Messgroesse aendern, nicht die Suche.
[[nodiscard]] std::vector<int> welligkeit(const Gruppe& g, std::size_t gruppe, std::size_t parameter, const Modell& modell, int punkte = 41);

/// Je Messgroesse der Gruppe Modell, Ziel und r bei den Werten `werte` (Parameter-Einheiten) — wie `Zielsumme`.
[[nodiscard]] std::vector<messung::Abweichung> abweichungen(const Gruppe& g, std::size_t gruppe, const Modell& modell,
                                                            const std::vector<double>& werte);

/// ZF8 a (2), HW3 a (2): die Glaette von Σ r² in einem Parameter — Schnitt mit `punkte` Werten ueber seinen Bereich, die
/// anderen auf ihren Werten in `g`. Glatt: hoechstens ein Minimum (die Raender zaehlen mit) und kein Schritt ueber das
/// Dreifache des mittleren (wie LumiViz `test_LoopSalamanderSuche.cpp`, Werkzeug `…_SCHNITT_X`).
struct Glaette
{
    int minima = 0;
    double groessterSchritt = 0.0, mittlererSchritt = 0.0;
    [[nodiscard]] bool glatt() const { return minima <= 1 && groessterSchritt <= 3.0 * mittlererSchritt; }
};
[[nodiscard]] Glaette glaette(const Gruppe& g, std::size_t gruppe, std::size_t parameter, const Modell& modell, int punkte = 15);

/// HW2 a, A5: dieselben Gruppen von mehreren Startpunkten gesetzt (`starts[s][g]`: die Werte der Gruppe g am Start s;
/// je Start ein eigener Unterordner `start_<s>` fuer das Protokoll), dazu je Gruppe der beste Lauf (kleinstes Σ r², bei
/// Gleichstand der fruehere).
struct Mehrfach
{
    std::vector<Bericht> laeufe;
    std::vector<std::vector<Gruppe>> gruppenJeLauf;
    std::vector<std::size_t> herkunft;  ///< je Gruppe der Index des besten Laufs
    std::vector<Gruppe> beste;
    Bericht bester;
};
[[nodiscard]] Mehrfach setzeMehrfach(const std::vector<Gruppe>& gruppen, const std::vector<std::vector<std::vector<double>>>& starts,
                                     const Modell& modell, const Einstellung& e = {});

/// HW2 a: je Messgroesse der Gruppe, ob die Startpunkte an ihr hoechstens max(τ, S) auseinander enden.
[[nodiscard]] std::vector<bool> eindeutig(const Mehrfach& m, std::size_t gruppe, const messung::Hoerschwellen& schwellen);

/// Die Gruppen eines Berichts als „Tasten“ der Messkette (midi = Index der Gruppe) — fuer `messung::systematik` (HW4 a)
/// und `messung::hoerZahlen`.
[[nodiscard]] std::vector<messung::TastenSumme> alsTasten(const Bericht& b, bool amAnfang = false);

/// HW3 a: die Probe. Besteht, wenn (2) die Glaette an mindestens 5/6 der Gruppen haelt (alle nicht festen Parameter am
/// besten Punkt glatt), (3) mindestens 5/6 der Messgroessen eindeutig sind (Gruppen, die nach ZF1 a nicht erreichbar
/// sind — ein Parameter mit belegter Grenze endet am Rand —, zaehlen nicht) und am besten Punkt mehr Groessen hoerbar
/// getroffen sind als an der Vorgabe (den Werten in `vorgabe`).
struct Probe
{
    int gruppen = 0, glatt = 0;
    int messbar = 0, eindeutig = 0;
    int hoerbarBest = 0, hoerbarVorgabe = 0;
    std::vector<std::string> nichtErreichbar, nichtGlatt;
    [[nodiscard]] bool glaetteHaelt() const { return gruppen > 0 && 6 * glatt >= 5 * gruppen; }
    [[nodiscard]] bool eindeutigGenug() const { return messbar > 0 && 6 * eindeutig >= 5 * messbar; }
    [[nodiscard]] bool besserAlsVorgabe() const { return hoerbarBest > hoerbarVorgabe; }
    [[nodiscard]] bool bestanden() const { return glaetteHaelt() && eindeutigGenug() && besserAlsVorgabe(); }
    [[nodiscard]] std::string text() const;
};
[[nodiscard]] Probe probe(const std::vector<Gruppe>& vorgabe, const Mehrfach& m, const Modell& modell, const messung::Hoerschwellen& schwellen,
                          int glattPunkte = 15);

}  // namespace lumi::messung::setzen
