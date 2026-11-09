/**
 * @file Rohr.hpp
 * @brief Gemeinsam: der Rohr-Kern fuer die Klassen 7, 8, 9 und den Resonator der Klasse 4 — Eingangsimpedanz aus
 *        einer Bohrung (Uebertragungsmatrizen mit Wandverlusten, Abstrahlung am offenen Ende), Moden der
 *        Eingangsimpedanz je Griff, Kopplung an einen Anreger im Zeitbereich (FRAGEN_KONSOLIDIERT T5 a, Entscheid
 *        Patrik 2026-10-29; `gemeinsam/rohr.md` §3)
 *
 * **Bohrung → Impedanz** (`gemeinsam/rohr.md` §3 Punkt 1): je Abschnitt ein Kegelstumpf (Zylinder als Sonderfall);
 * die Matrix eines Kegels in Kugelwellen nach Benade/Fletcher & Rossing (Lehrbuch, Gl. aus dem Gedaechtnis —
 * **offen**: nicht an einer Quelle im Repo nachgelesen; geprueft an den geschlossenen Faellen Zylinder offen/geschlossen
 * und Kegel), die Wandverluste als komplexe Wellenzahl
 * Γ = (iω/c)·[1 + (1 − i)/(√2·r_v)·(1 + (γ − 1)/√Pr)], r_v = a·√(ωρ/η) (Naeherung erster Ordnung fuer grosse r_v,
 * Keefe 1984 / Chaigne & Kergomard 2016 — **nur aus dem Gedaechtnis, offen**), beim Kegel mit dem mittleren Radius;
 * am offenen Ende die Abstrahlung ohne Flansch nach Levine–Schwinger in der Naeherung kleiner ka:
 * Z_r = Z_c·[(ka)²/4 + i·0,6133·ka] (offen wie oben; gueltig ka < 1 — darueber steht der Wert in `gueltigBisHz`).
 *
 * **Impedanz → Moden** (F-R1 a: „Moden der Eingangsimpedanz je Griff“): je Spitze von |Z| (Rohrblatt, Lippe:
 * geschlossener Eingang) bzw. von |Y| (Strahl: offener Eingang) eine Resonanz zweiter Ordnung
 * Z_n(s) = C_n·s / (s² + (ω_n/Q_n)·s + ω_n²), ω_n und Q_n aus der Spitze und ihrer Halbwertsbreite, C_n = Z_n,max·ω_n/Q_n.
 *
 * **Kopplung im Zeitbereich** (F-R6 a: „Kopplung über Wanderwellen an den Anreger … wie bei Maestre 2018“):
 * je Mode ein komplexer Zustand mit Impulsinvarianz; der Druck am Eingang ist p[n] = p_vorher[n] + D·u[n] — der Teil
 * aus der Vergangenheit und der Durchgriff D = ½·Σ C_n·Δt des laufenden Flusses (die Impulsantwort springt bei t = 0
 * auf Σ C_n; der Sprung zaehlt abgetastet zur Haelfte, sonst liegt der Kern zwischen den Spitzen rund 10 % zu hoch). Der Anreger loest damit seine
 * Gleichung u = F(p) im selben Takt (ohne Verzoegerung um einen Takt), danach traegt `nachher(u)` den Fluss ein.
 * **Abweichung vom Entwurf (begruendet):** Maestre 2018 koppelt ueber die Reflexion (Wanderwellen) statt ueber die
 * Impedanz; beides beschreibt dasselbe lineare Rohr, und die Form mit Durchgriff loest die verzoegerungsfreie
 * Schleife genauso (der Anreger sieht p⁻ bzw. p_vorher als bekannt). Die Waveguide-Stuecke (Zug, Schmettern,
 * Vokaltrakt, Strahl) haengen in den Klassen 7–9 an diesem Kern (`schritt` mit veraenderlicher Verzoegerung).
 *
 * Reines C++, deterministisch.
 */
#pragma once

#include "gemeinsam/Modal.hpp"

#include <functional>
#include <string>
#include <vector>

namespace ic
{

/// Die Luft (20 °C, Meereshoehe) — Lehrbuchwerte, Belegkraft „Rechnung“: ρ = 1,2041 kg/m³, c = 343,2 m/s,
/// η = 1,8205·10⁻⁵ Pa·s, γ = 1,4, Pr = 0,71 (offen: aus dem Gedaechtnis).
struct Luft
{
    double dichte = 1.2041;
    double schall = 343.2;
    double zaehigkeit = 1.8205e-5;
    double gamma = 1.4;
    double prandtl = 0.71;
};

/// Ein Abschnitt der Bohrung: Laenge und Radius am Anfang und am Ende (Kegelstumpf; gleich = Zylinder).
struct Abschnitt
{
    double laengeM = 0.1;
    double radiusEinM = 0.01;
    double radiusAusM = 0.01;
};

enum class Abschluss
{
    Offen,        ///< Abstrahlung ohne Flansch
    Geschlossen,  ///< starr (Resonator der Stabspiele am Boden)
    Ideal,        ///< Druck 0 (ohne Abstrahlung, fuer analytische Tests)
};

/// Eine Bohrung vom Eingang (Mundstueck) zum Ende.
struct Bohrung
{
    std::vector<Abschnitt> abschnitte;
    Abschluss ende = Abschluss::Offen;
    bool verluste = true;
    /// Ein Daempfer am offenen Ende (Klasse 7, T3b a): ein Helmholtz-Resonator parallel zur Abstrahlung der
    /// Restflaeche — Z_D = Z_c·(1/Q + i(ω/ω_D − ω_D/ω)); 0 Hz = ohne.
    double daempferHz = 0.0;
    double daempferQ = 5.0;
    double daempferRest = 1.0;
    /// Die Grenzfrequenz eines Gitters offener Tonloecher hinter dem Ende (Benade; Klasse 8 mit der wirksamen Laenge
    /// je Griff): darunter reflektiert das Ende wie offen, darueber laeuft die Welle weiter — Z_L → Z_c mit dem
    /// Gewicht x⁸/(1 + x⁸), x = f/f_c (die Form des Uebergangs ist offen; mit x⁴ daempfte der Widerstand schon die tiefen Moden); 0 Hz = ohne.
    double gitterGrenzeHz = 0.0;
    std::string herkunft;
};

/// Die Gesamtlaenge.
[[nodiscard]] double laenge(const Bohrung& b);

/// Die Kettenmatrix [p_ein; U_ein] = [[a, b], [c, d]]·[p_aus; U_aus] der Abschnitte (ohne Abschluss) bei f.
struct Kettenmatrix
{
    Komplex a{1.0, 0.0}, b{0.0, 0.0}, c{0.0, 0.0}, d{1.0, 0.0};
};
[[nodiscard]] Kettenmatrix kettenmatrix(const Bohrung& b, double hz, const Luft& luft = {});

/// Die Abschlussimpedanz am Ende (offen: Levine–Schwinger genaehert; geschlossen: unendlich → 1e300; ideal: 0).
[[nodiscard]] Komplex abschlussImpedanz(const Bohrung& b, double hz, const Luft& luft = {});

/// Die Wellenzahl Γ eines Zylinders mit Radius r bei f (mit Wandverlusten, wenn `verluste`).
[[nodiscard]] Komplex wellenzahl(double radiusM, double hz, const Luft& luft = {}, bool verluste = true);

/// Die Eingangsimpedanz Z = p/U [Pa·s/m³] bei f.
[[nodiscard]] Komplex eingangsImpedanz(const Bohrung& b, double hz, const Luft& luft = {});

/// Die Wellenimpedanz ρc/S am Eingang.
[[nodiscard]] double wellenImpedanz(const Bohrung& b, const Luft& luft = {});

/// Eine Mode der Impedanz.
struct ImpedanzMode
{
    double hz = 0.0;
    double q = 10.0;
    double spitze = 0.0;  ///< |Z| bzw. |Y| an der Spitze
};

/// Die Spitzen der Eingangsimpedanz (`admittanz` = falsch) bzw. -admittanz (wahr) bis `bisHz`, aus einem Raster
/// mit `schrittHz` und Nachschaerfen (Goldener Schnitt auf 10⁻⁶ Hz), Q aus der Halbwertsbreite (−3 dB).
[[nodiscard]] std::vector<ImpedanzMode> impedanzModen(const Bohrung& b, double bisHz, bool admittanz = false,
                                                      double schrittHz = 0.5, const Luft& luft = {});

/// Der Durchgriff D = ½·Σ C_n / rate.
[[nodiscard]] double durchgriffAus(const std::vector<ImpedanzMode>& moden, double rate);

/// Dasselbe fuer eine beliebige Betragskurve (etwa die Impedanz mit einem Mundstueck parallel) von `vonHz` bis `bisHz`.
/// `hoechstens` > 0: die Suche endet nach so vielen Moden.
[[nodiscard]] std::vector<ImpedanzMode> modenAusBetrag(const std::function<double(double)>& betrag, double vonHz, double bisHz,
                                                       double schrittHz = 0.5, std::size_t hoechstens = 0);

/// Der Rohr-Kern im Zeitbereich: Moden der Impedanz, p = p_vorher + D·u.
class RohrKern
{
public:
    RohrKern() = default;
    RohrKern(const std::vector<ImpedanzMode>& moden, double rate);

    /// Der Druck aus der Vergangenheit (ohne den Fluss dieses Takts).
    [[nodiscard]] double vorher() const;
    /// Der Durchgriff: so viel Druck je m³/s Fluss in diesem Takt.
    [[nodiscard]] double durchgriff() const { return m_durchgriff; }
    /// Den Fluss dieses Takts eintragen und einen Takt weiter.
    void nachher(double fluss);
    /// Die Moden.
    [[nodiscard]] std::size_t anzahl() const { return m_pol.size(); }
    /// Ein Ausgang aus den Moden mit komplexen Gewichten je Mode: Σ 2·Re(g_n·z_n) (etwa der abgestrahlte Druck, wenn
    /// g_n das Verhaeltnis p_Abstrahlung/p_Eingang an der Mode ist).
    [[nodiscard]] double ausgang(const std::vector<Komplex>& gewichte) const;
    /// Die Moden ueberblenden (Griffwechsel, Maestre 2018): die Zustaende bleiben, Pol und Gewicht gehen in
    /// `schritte` Takten linear auf die neuen ueber. Gleiche Zahl der Moden noetig, sonst sofort.
    void blendeAuf(const std::vector<ImpedanzMode>& moden, int schritte);

private:
    void setze(const std::vector<ImpedanzMode>& moden, std::vector<Komplex>& pol, std::vector<Komplex>& gewicht) const;
    double m_rate = 48000.0;
    std::vector<Komplex> m_zustand;
    std::vector<Komplex> m_pol;
    std::vector<Komplex> m_gewicht;  // C·r: so viel bringt u·Δt
    std::vector<Komplex> m_polZiel;
    std::vector<Komplex> m_gewichtZiel;
    std::vector<Komplex> m_polStart;
    std::vector<Komplex> m_gewichtStart;
    int m_blende = 0;
    int m_blendeSchritte = 0;
    double m_durchgriff = 0.0;
};

/// Die geschlossenen Faelle (fuer Tests): ein verlustfreier Zylinder der Laenge L, am Ende ideal offen (p = 0),
/// hat bei geschlossenem Eingang Spitzen von Z bei f_n = (2n − 1)·c/(4L); am Ende geschlossen bei f_n = n·c/(2L).
[[nodiscard]] double zylinderSpitzeHz(double laengeM, int n, bool endeOffen, const Luft& luft = {});

}  // namespace ic
