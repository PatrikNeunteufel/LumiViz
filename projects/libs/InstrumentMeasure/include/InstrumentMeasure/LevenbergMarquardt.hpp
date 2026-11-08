/**
 ****************************************************************************************
 * @file   LevenbergMarquardt.hpp
 * @brief  7.5.10 (SM4 b): Levenberg–Marquardt mit Grenzen fuer das Setzen der Hammer-Saite an den Stuetzpunkten
 *         (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S156/S157: ZF6 a, SP2 a)
 *
 * @details
 * Reines C++ ohne Qt und ohne Eigen — die Normalgleichungen sind klein (rund 200 Parameter). Je Iteration wird die
 * Jacobi-Matrix J am Punkt x einmal gebildet; der Schritt loest (JᵀJ + λ·diag(JᵀJ))·δ = −Jᵀr (Marquardt-Skalierung,
 * Gauss mit Spaltenpivot). Ein Schritt mit kleinerem Rest Σr² wird genommen und λ durch `lambdaFaktor` geteilt, ein
 * schlechterer verworfen und λ mit ihm malgenommen (bis `versucheHoechstens` Versuche je Iteration). Die Grenzen
 * wirken durch Abschneiden nach dem Schritt. Ende, wenn jeder Schritt unter `LmRaum::ende` liegt (der Auftrag 7.5.10:
 * 1 % der Groesse — log 0,01, linear 1 % des Bereichs), wenn kein Versuch den Rest senkt, oder nach
 * `iterationenHoechstens`. Die Jacobi-Matrix je Gruppe (`jacobiJeGruppe`) lenkt alle Parameter einer Gruppe zugleich
 * aus — gueltig, wenn jeder Parameter der Gruppe nur auf seine eigenen Zeilen wirkt (Lesart Claude S161: ein
 * Stuetzpunkt wirkt nur auf den Ton seiner Taste).
 *
 * 7.5.12 (V2 a, A2 und A3): `levenbergMarquardtJeBlock` fuehrt dieselbe Suche je Block (je Taste) mit eigenem λ,
 * eigener Annahme und eigenem Ende; die Bloecke sind disjunkt und haben eigene Residuen. Alle aktiven Bloecke werden
 * zusammen ausgewertet: der Rueckruf `LmResiduenBloecke(punkte, bloecke)` gibt eine flache Liste zurueck, Eintrag
 * `p · bloecke.size() + k` sind die Residuen des Blocks `bloecke[k]` am Punkt `punkte[p]`. Die Jacobi-Matrix entsteht
 * aus einer Auswertung je Gruppe (alle Parameter der Gruppe in allen aktiven Bloecken ausgelenkt), die Versuchsschritte
 * aller noch nicht entschiedenen Bloecke liegen in einem gemeinsamen Punkt. Ein beendeter Block wird nicht mehr
 * ausgewertet. A3: ein Parameter, dessen Wirkung √diag(JᵀJ) · Bereich am aktuellen Punkt unter `wirkungMindestens`
 * Toleranzen liegt, bekommt keinen Schritt (`lmSchritt` mit Maske) — die Entscheidung faellt je Iteration neu.
 ****************************************************************************************
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace lumi::messung
{

/// Eine dichte Matrix, zeilenweise.
struct Matrix
{
    std::size_t zeilen = 0;
    std::size_t spalten = 0;
    std::vector<double> werte;

    Matrix() = default;
    Matrix(std::size_t z, std::size_t s) : zeilen(z), spalten(s), werte(z * s, 0.0) {}
    double& operator()(std::size_t i, std::size_t j) { return werte[i * spalten + j]; }
    [[nodiscard]] double operator()(std::size_t i, std::size_t j) const { return werte[i * spalten + j]; }
    [[nodiscard]] bool operator==(const Matrix&) const = default;
};

double quadratsumme(const std::vector<double>& r);

/// Loest a·x = b (Gauss mit Spaltenpivot); eine Spalte ohne Pivot (singulaer) bekommt x = 0.
std::vector<double> loese(Matrix a, std::vector<double> b);

/// Die Diagonale von JᵀJ — je Parameter, wie stark die Residuen auf ihn antworten.
std::vector<double> diagonaleJtJ(const Matrix& j);

/// Der Schritt δ aus (JᵀJ + λ·diag(JᵀJ))·δ = −Jᵀr; eine Diagonale von null wird fuer die Daempfung auf ein Billionstel
/// der groessten gehoben (sonst bliebe die Matrix bei einem Parameter ohne Wirkung singulaer).
std::vector<double> lmSchritt(const Matrix& j, const std::vector<double>& r, double lambda);

/// Der Schritt nur fuer die Parameter mit `frei[p]` (A3): die uebrigen fallen aus dem Gleichungssystem (Spalte
/// gestrichen) und bekommen 0. Sind alle frei, bitgleich `lmSchritt(j, r, lambda)`.
std::vector<double> lmSchritt(const Matrix& j, const std::vector<double>& r, double lambda, const std::vector<bool>& frei);

/// Grenzen und Ende je Parameter (in der Koordinate der Suche).
struct LmRaum
{
    std::vector<double> unten;
    std::vector<double> oben;
    /// Ein Schritt darunter gilt als klein; sind alle klein, endet die Suche.
    std::vector<double> ende;
};

struct LmEinstellungen
{
    double lambdaStart = 1e-3;
    double lambdaFaktor = 10.0;
    int iterationenHoechstens = 20;
    int versucheHoechstens = 10;
    /// A3 (nur `levenbergMarquardtJeBlock`): ein Parameter mit weniger Wirkung √diag(JᵀJ) · Bereich (in Toleranzen)
    /// bekommt keinen Schritt (SP3 a, Lesart Claude S162; Wert aus 7.5.10 `kNichtAufgeloest`).
    double wirkungMindestens = 0.1;
};

/// Was eine Iteration getan hat (fuer Bericht und Dateien).
struct LmIteration
{
    int nummer = 0;
    std::vector<double> x;    ///< nach der Iteration
    std::vector<double> r;
    double rest = 0.0;
    double restVorher = 0.0;
    double lambdaVorher = 0.0;
    double lambda = 0.0;      ///< nach der Iteration
    int versuche = 0;         ///< ausgewertete Schritte
    int verworfen = 0;
    bool angenommen = false;
    double schrittAnteil = 0.0;  ///< groesster |Schritt| / ende ueber die Parameter (< 1: klein)
    std::vector<double> diagonale;  ///< von JᵀJ am Punkt vor dem Schritt
    std::vector<std::size_t> anGrenze;
    int auswertungenResiduen = 0;
    std::string ende;  ///< leer, solange die Suche weiterlaeuft
};

struct LmErgebnis
{
    std::vector<double> x;
    std::vector<double> r;
    double rest = 0.0;
    double lambda = 0.0;
    int iterationen = 0;
    int auswertungen = 0;  ///< Auswertungen der Residuen (ohne die der Jacobi-Matrix)
    std::vector<std::size_t> anGrenze;
    std::string ende;
};

/// Wo die Suche beginnt: am Punkt `x` mit `lambda`, nach `iteration` schon gelaufenen Iterationen; `r` (wenn nicht leer)
/// sind die Residuen dort — dann wird x nicht neu ausgewertet (Wiederaufnahme).
struct LmStart
{
    LmStart() = default;
    LmStart(std::vector<double> x_, double lambda_ = 1e-3, int iteration_ = 0, std::vector<double> r_ = {})
        : x(std::move(x_)), lambda(lambda_), iteration(iteration_), r(std::move(r_))
    {
    }

    std::vector<double> x;
    double lambda = 1e-3;
    int iteration = 0;
    std::vector<double> r;
};

using LmResiduen = std::function<std::vector<double>(const std::vector<double>&)>;
using LmJacobi = std::function<Matrix(const std::vector<double>& x, const std::vector<double>& r)>;
using LmBeobachter = std::function<void(const LmIteration&)>;

std::vector<double> klemmeIn(const LmRaum& raum, std::vector<double> x);

std::vector<std::size_t> anGrenzeIn(const LmRaum& raum, const std::vector<double>& x);

/// Levenberg–Marquardt mit Grenzen (s. Dateikopf).
LmErgebnis levenbergMarquardt(const LmResiduen& residuen, const LmJacobi& jacobi, const LmStart& start, const LmRaum& raum,
                                     const LmEinstellungen& einst = {}, const LmBeobachter& beobachter = {});

/// Viele Punkte auf einmal ausgewertet (die Toene eines Aufrufs teilen sich einen Pool).
using LmResiduenViele = std::function<std::vector<std::vector<double>>(const std::vector<std::vector<double>>&)>;

/// Der Schritt der finiten Differenz je Parameter: `mass[j]`, nach unten gerichtet, wo der Schritt nach oben die Grenze
/// ueberschritte.
std::vector<double> differenzSchritte(const LmRaum& raum, const std::vector<double>& x, const std::vector<double>& mass);

/// Die Jacobi-Matrix je Gruppe: je Gruppe eine Auswertung mit allen ihren Parametern um h ausgelenkt; die Spalte eines
/// Parameters traegt nur seine Zeilen (`zeilen[j]`), sonst null. Alle Gruppen in einem Aufruf von `viele`.
Matrix jacobiJeGruppe(const LmResiduenViele& viele, const std::vector<double>& x, const std::vector<double>& r,
                             const std::vector<std::vector<std::size_t>>& gruppen, const std::vector<double>& h,
                             const std::vector<std::vector<std::size_t>>& zeilen);

/// Die Jacobi-Matrix einzeln: je Parameter eine Auswertung, alle Zeilen (zum Vergleich mit `jacobiJeGruppe`).
Matrix jacobiEinzeln(const LmResiduenViele& viele, const std::vector<double>& x, const std::vector<double>& r, const std::vector<double>& h);

// ---- 7.5.12 (A2, A3): je Block ----------------------------------------------------------------------------------

/// Ein Block: die Parameter einer Taste (Indizes in x, in der Reihenfolge der Spalten seiner Jacobi-Matrix).
struct LmBlock
{
    std::vector<std::size_t> parameter;
};

/// Die Residuen der Bloecke `bloecke` an jedem der `punkte`, flach: Eintrag p · bloecke.size() + k gehoert zu
/// `punkte[p]` und `bloecke[k]`.
using LmResiduenBloecke =
    std::function<std::vector<std::vector<double>>(const std::vector<std::vector<double>>& punkte, const std::vector<std::size_t>& bloecke)>;

/// Was ein Block in einer Iteration getan hat.
struct LmBlockSchritt
{
    bool aktiv = false;  ///< in dieser Iteration gerechnet
    double restVorher = 0.0;
    double rest = 0.0;
    double lambdaVorher = 0.0;
    double lambda = 0.0;
    int versuche = 0;
    int verworfen = 0;
    bool angenommen = false;
    double schrittAnteil = 0.0;
    std::vector<double> diagonale;         ///< von JᵀJ des Blocks, je Parameter des Blocks
    std::vector<std::size_t> ohneWirkung;  ///< Parameter (Indizes in x) ohne Schritt nach A3
    std::string ende;                      ///< leer, solange der Block weiterlaeuft
};

/// Was eine Iteration ueber alle Bloecke getan hat.
struct LmIterationBloecke
{
    int nummer = 0;
    std::vector<double> x;  ///< nach der Iteration
    std::vector<LmBlockSchritt> bloecke;
    int auswertungen = 0;  ///< Punkte × Bloecke dieser Iteration, mit denen der Jacobi-Matrix
};

/// Der Stand eines Blocks.
struct LmBlockStand
{
    std::vector<double> r;
    double rest = 0.0;
    double lambda = 0.0;
    int iterationen = 0;
    int auswertungen = 0;  ///< wie `LmErgebnis::auswertungen`: Residuen dieses Blocks ohne die der Jacobi-Matrix
    std::string ende;
    std::vector<double> diagonale;         ///< aus der letzten Iteration
    std::vector<std::size_t> ohneWirkung;  ///< aus der letzten Iteration
};

struct LmErgebnisBloecke
{
    std::vector<double> x;
    std::vector<LmBlockStand> bloecke;
    int iterationen = 0;   ///< gemeinsame Iterationen (die Nummer der Dateien)
    int auswertungen = 0;  ///< Punkte × Bloecke, mit denen der Jacobi-Matrix
    std::vector<std::size_t> anGrenze;
};

/// Wo die Suche je Block beginnt (Wiederaufnahme): leere Listen heissen `lambdaStart`, 0 Iterationen, aktiv.
struct LmStartBloecke
{
    LmStartBloecke() = default;
    LmStartBloecke(std::vector<double> x_) : x(std::move(x_)) {}

    std::vector<double> x;
    int nummer = 0;  ///< gemeinsame Iterationen schon gelaufen
    std::vector<double> lambda;
    std::vector<int> iteration;
    std::vector<std::string> ende;
};

using LmBeobachterBloecke = std::function<void(const LmIterationBloecke&)>;

/// Levenberg–Marquardt je Block (s. Dateikopf): `gruppen` sind die Gruppen der finiten Differenz (in jedem Block
/// hoechstens ein Parameter je Gruppe), `differenz` der Schritt je Parameter (`differenzSchritte`).
LmErgebnisBloecke levenbergMarquardtJeBlock(const LmResiduenBloecke& residuen, const std::vector<LmBlock>& bloecke,
                                                   const std::vector<std::vector<std::size_t>>& gruppen, const std::vector<double>& differenz,
                                                   const LmStartBloecke& start, const LmRaum& raum, const LmEinstellungen& einst = {},
                                                   const LmBeobachterBloecke& beobachter = {});

}  // namespace lumi::messung
