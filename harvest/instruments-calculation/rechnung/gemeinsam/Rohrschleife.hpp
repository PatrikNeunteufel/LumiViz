/**
 * @file Rohrschleife.hpp
 * @brief Gemeinsam: die Schleife eines Blechblasinstruments mit Wanderwellen — Kessel des Mundstuecks, ein
 *        zylindrisches Stueck als Verzoegerung (veraenderlich: Posaunenzug; druckabhaengig: Schmettern), der Rest der
 *        Bohrung als Reflexion und Abstrahlung aus der Uebertragungsmatrix (FRAGEN_KONSOLIDIERT T5 a, Entscheid Patrik
 *        2026-10-29: „der zylindrische Zug der Posaune als veränderliche Verzögerung, das Schmettern als
 *        druckabhängige Verzögerung im Zylinder innerhalb der Schleife (Cooper & Abel 2010) — es gehört zum Kern“)
 *
 * **Aufbau** (vom Mund zum Schallstueck):
 * 1. *Kessel* mit Volumen V (Nachgiebigkeit C = V/(ρc²)): ṗ_k = (U_Lippe − U_aus)/C, U_aus = (p_k − 2·p⁻)/Z_c — die
 *    Welle in den Zylinder ist p⁺ = p_k − p⁻ (der Kessel als konzentriertes Glied am Eingang; Hals und Seele offen).
 * 2. *Zylinder* der Laenge L_z (Radius r_z): p⁺ laeuft hin mit der Geschwindigkeit c + β·p⁺/(ρc) (β = (γ + 1)/2 —
 *    die Ausbreitung endlicher Amplitude nach Burgers, wie bei Cooper & Abel 2010 als druckabhaengige Verzoegerung;
 *    Lehrbuch, offen); ueberholt ein spaeterer Wert einen frueheren, verschmelzen sie (Stoss). Die Laenge kann sich je
 *    Takt aendern (Zug), die Verzoegerung wird gebrochen interpoliert. Zurueck laeuft die Welle linear.
 * 3. *Rest* (Bohrung hinter dem Zylinder bis zur Abstrahlung): die Reflexion R(ω) = (Z − Z_c)/(Z + Z_c) und die
 *    Abstrahlung (Druck im Abstand 1 m, Monopol ρ/(4π)·dU_Ende/dt) je ankommender Welle aus der Uebertragungsmatrix
 *    (`gemeinsam/Rohr`), dazu die Wandverluste des Zylinders (hin und zurueck, e^(−2(Γ − ik)L_z)); beides als FIR durch
 *    die inverse FFT, gefenstert, bis 99,99 % der Energie.
 *
 * **Abweichung vom Entwurf (begruendet):** T5 a nennt „Moden der Eingangsimpedanz je Griff, gekoppelt ueber
 * Wanderwellen wie bei Maestre 2018“; Maestre stellt die Reflexion modal dar, hier ist sie die abgetastete
 * Impulsantwort derselben linearen Bohrung (FIR) — gleichwertig fuer den Klang, ohne Anpassung von Polen an eine
 * glatte Reflexion. Die Moden der Eingangsimpedanz bleiben fuer die Vorhersage der Register (`impedanzModen`).
 *
 * Reines C++, deterministisch.
 */
#pragma once

#include "gemeinsam/Rohr.hpp"

#include <deque>
#include <vector>

namespace ic
{

/// Die linearen Teile einer Schleife, vorab je Griff: Reflexion und Abstrahlung als FIR.
struct SchleifenFilter
{
    std::vector<double> reflexion;   ///< p⁻ am Zylinderende je p⁺ dort (inkl. Wandverluste des Zylinders hin und zurueck)
    std::vector<double> abstrahlung; ///< Druck in 1 m je p⁺ am Zylinderende
    double zylinderZc = 0.0;
};

/// Die Filter fuer einen Rest `rest` hinter einem Zylinder mit Radius `zylinderRadiusM` und Laenge `zylinderLaengeM`
/// (die Verluste des Zylinders gehen mit dieser Laenge ein). `laengeMax` Takte hoechstens.
[[nodiscard]] SchleifenFilter schleifenFilter(const Bohrung& rest, double zylinderRadiusM, double zylinderLaengeM, int rate,
                                              const Luft& luft = {}, std::size_t laengeMax = 8192);

/// Die Schleife im Zeitbereich.
class Rohrschleife
{
public:
    Rohrschleife(SchleifenFilter filter, double kesselVolumenM3, double zylinderLaengeM, int rate, const Luft& luft = {});

    /// Der Druck im Kessel (bekannt vor dem Fluss dieses Takts).
    [[nodiscard]] double kesselDruck() const { return m_pk; }
    /// Den Fluss der Lippe in diesem Takt eintragen und einen Takt weiter; liefert den abgestrahlten Druck.
    double schritt(double flussLippe);
    /// Die Laenge des Zylinders ab dem naechsten Takt (Zug); haelt sie in [kleinste, groesste] der Puffer.
    void setzeZylinderLaenge(double laengeM);
    /// Das Schmettern an/aus (β = 0: lineare Ausbreitung).
    void setzeNichtlinear(double beta) { m_beta = beta; }
    /// Ein zweiter Satz Filter (vor dem ersten Takt), zwischen denen `setzeMischung` linear mischt — die Reflexion und
    /// Abstrahlung einer Bohrung, deren Ende sich in der Note aendert (Wah-Wah: die Hand am Harmon, T3b a). Die Mischung
    /// der Impulsantworten ist die Impulsantwort der gemischten Uebertragung (linear), nicht die einer Zwischenstellung
    /// der Hand (Naeherung, offen).
    void setzeZweitFilter(SchleifenFilter zweit);
    /// 0 = der erste Satz, 1 = der zweite.
    void setzeMischung(double w) { m_mischung = w; }

private:
    SchleifenFilter m_f;
    SchleifenFilter m_zweit;
    bool m_mitZweit = false;
    double m_mischung = 0.0;
    Luft m_luft;
    double m_rate;
    double m_c;         // Nachgiebigkeit des Kessels
    double m_kesselPol = 0.0;
    double m_pk = 0.0;  // Druck im Kessel
    double m_laenge;
    double m_beta = 1.2;
    double m_zeit = 0.0;
    // die Welle hin: (Ankunftszeit am Ende in Takten, Wert); die Ankunftszeiten sind nicht fallend
    std::deque<std::pair<double, double>> m_hin;
    double m_letzteAnkunft = -1e300;
    std::vector<double> m_historie;  // p⁺ am Zylinderende, rueckwaerts, doppelt (zusammenhaengend lesbar)
    std::size_t m_kopf = 0;
    std::vector<double> m_zurueck;  // p⁻ auf dem Rueckweg (Ring, feste Verzoegerung L/c gerechnet beim Eintragen)
    std::size_t m_pos = 0;
};

}  // namespace ic
