/**
 * @file Membran.hpp
 * @brief Gemeinsam: die Moden kreisrunder Felle mit Luftlast und Kessel — vorab gerechnet (Klasse 5,
 *        `klassen/Klasse 5 - Membran/modell-entwurf.md` §2 Punkte 1–2)
 *
 * **Verfahren** (Galerkin, eigene Herleitung): je Ordnung m (Knotendurchmesser) die idealen Moden J_m(j_mn·r/a) als
 * Basis; die Steife T·∫∇φ·∇φ, die Masse ∫σ(r)·φφ (auch ungleich, Tabla); dazu die **Luftlast** einer Seite als
 * Rayleigh-Integral im Wellenzahlraum (das Fell in einer Schallwand — Naeherung): M_luft[n,n′] = A_m·∫ ρ/√(κ² − k₀²)·
 * W_n(κ)·W_n′(κ)·κ dκ fuer κ > k₀ und der Widerstand der Abstrahlung R[n,n′] = A_m·∫ ρω/√(k₀² − κ²)·W_n·W_n′·κ dκ fuer
 * κ < k₀, mit W_n die Hankel-Transformierte der Basis, A_m = 2π (m = 0) bzw. π; frequenzabhaengig, je Mode bis zur
 * Ruhe iteriert (wie Christian et al. 1984 und Packer 1993 Gl. 4.36 — nur Bericht). Der **Kessel** als Luftfeder auf
 * den Moden m = 0: K = ρc²/V·g_i·g_j mit g = ∫φ dA; zwei Felle (Tom, Snare) ueber dasselbe Volumen gekoppelt. Die
 * innere Luftmasse ist nicht gebaut (offen).
 *
 * **Daempfung** je Mode: σ = (d₁ + d₃·k²)/(2σ_F) (Avanzini & Marogna 2010 — nur Bericht) plus die Abstrahlung R/(2M).
 */
#pragma once

#include <functional>
#include <vector>

namespace ic
{

/// Ein Fell.
struct Fell
{
    double radiusM = 0.32;
    double spannungNm = 5360.0;   ///< T in N/m
    double dichteKgM2 = 0.262;    ///< σ der Flaeche (ohne Belag)
    /// Belag (Tabla: syahi): Dichte relativ zum Fell ueber r/a; leer = gleichmaessig
    std::function<double(double)> belag;
    double d1 = 1.25;             ///< kg/(m²·s) (Avanzini & Marogna, nur Bericht)
    double d3 = 5e-4;             ///< kg/s (dto.)
    bool abstrahlung = true;      ///< eine Seite strahlt in den Raum (Luftlast); falsch: ohne Luft
    /// Anteil der Luftmasse auf der Innenseite (Kessel, Trommel) gegen die eines Halbraums: 0 = ohne, 1 = wie aussen
    /// (Naeherung — der Kessel ist kein Halbraum; offen)
    double innenLuft = 0.0;
};

/// Eine gerechnete Mode eines Fells (bzw. eines gekoppelten Paars, dann `fell` = 0/1 des groessten Anteils).
struct FellMode
{
    int m = 0;            ///< Knotendurchmesser
    int n = 1;            ///< Ordnung in r (1 = keine Knotenkreise)
    double hz = 0.0;
    double t60S = 1.0;
    double masseKg = 1.0; ///< modale Masse (Form an der Stelle des groessten Betrags auf 1 — siehe `form`)
    /// Die Form ueber r/a je Fell: Koeffizienten in der Basis J_m(j_mk·r/a) (Fell 0, dann Fell 1)
    std::vector<double> koeffizienten;
    std::vector<double> nullen;  ///< j_mk der Basis (fuer `formAn`)
    int felle = 1;
    /// Die Fernfeld-Staerke unter 45° (Hankel-Transformierte bei k₀·sin 45°, mal (−i)^m — als Betrag mit Vorzeichen)
    double abstrahlung = 0.0;
};

struct Kessel
{
    double volumenM3 = 0.0;  ///< 0 = ohne (offen oder ohne Kessel)
};

/// Die Moden bis `bisHz` (die Luftlast bis `luftBisHz`, darueber ohne Luftmasse); je m so viele Funktionen in r, wie
/// ideale Moden unter 1,3·`bisHz` liegen, + 2, hoechstens `basis`. Ein Fell oder zwei ueber den Kessel gekoppelte.
[[nodiscard]] std::vector<FellMode> fellModen(const std::vector<Fell>& felle, const Kessel& kessel, double bisHz, double luftBisHz = 1000.0,
                                              int basis = 30);

/// Die Auslenkung der Mode an r/a (Winkel 0) auf Fell `fell`.
[[nodiscard]] double formAn(const FellMode& mode, double rRel, int fell = 0);

/// Die k-te Nullstelle (k ≥ 1) von J_m.
[[nodiscard]] double besselNull(int m, int k);

}  // namespace ic
