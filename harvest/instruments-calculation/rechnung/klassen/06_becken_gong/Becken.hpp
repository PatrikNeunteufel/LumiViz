/**
 * @file Becken.hpp
 * @brief Klasse 6 Becken und Gong: ein Schlag auf eine duenne Platte mit geometrischer Nichtlinearitaet (von Karman),
 *        modal, energiestabil (SAV) — vorab je Staerke und Schlagstelle (`klassen/Klasse 6 - Becken und Gong/
 *        modell-entwurf.md`; Entscheide Patrik 2026-10-29: F-1 … F-6 a ueber FRAGEN_KONSOLIDIERT T6, T7, T8, T9, E1)
 *
 * **Platte** (eigene Herleitung nach Touzé, Thomas & Chaigne — nur Bericht): freie Kreisplatte, Moden
 * W = J_m(λr/a) + C·I_m(λr/a) (Randbedingungen M_r = V_r = 0), die cos-Familie (die angeschlagene); die Airy-
 * Spannungsfunktionen eingespannt Φ = J_m(ζr/a) − J_m(ζ)/I_m(ζ)·I_m(ζr/a). Mit ∫W² dA = ∫Φ² dA = 1:
 * q̈_k + 2σ_k q̇_k + ω_k² q_k = −E/(2ρ)·Σ_n ζ_n⁻⁴·η_n·(H^n q)_k + p_k/(ρh), η_n = qᵀH^n q, H^n_pq = ∫Φ_n·L(W_p, W_q) dA
 * (von-Karman-Klammer L); das Potential V = E/(8ρ)·Σ ζ⁻⁴η². Die Winkelauswahl (m_n = m_p ± m_q) macht H duenn.
 *
 * **Zeitschritt:** SAV (scalar auxiliary variable, Bilbao, Ducceschi & Zama — nur Bericht): ψ = √(2V + ε), die Kraft
 * ½(ψ^{n+½} + ψ^{n−½})·∇V/ψ; ein Rang-1-System (Sherman–Morrison), energiestabil ohne Iteration (explizites
 * Stoermer–Verlet ist „highly prone to explosive instability“, DAFx23 — nur Bericht). Lineare Teile mit exakter
 * Frequenz (2(1 − cos ωΔt)/Δt²). **Daempfung:** ξ = 8·10⁻³·ω^−0,4 (Humbert, nur Bericht).
 *
 * **Schlag:** Schlaegel/Stock mit Hertz-Kontakt explizit. **Ausgang:** die Beschleunigung an drei Punkten (offen: die
 * Abstrahlung). **Choke** (F-4 a): Daempfung der Hand ab dem Ende der Note. **Hi-Hat** (F-3 a): die Oeffnung als
 * Daempfung (der Kontakt der Schalen ist nicht gebaut — numerisch ungeloest, Sekiguchi & Samejima 2023).
 *
 * Deterministisch (E1 a: feste Reihenfolge, feste Rechenregeln — die Rechnung ist chaotisch, das Ergebnis je Compiler
 * als Abdruck).
 */
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ic::becken
{

/// Eine Mode der freien Kreisplatte (cos-Familie), normiert ∫W² dA = 1 (mit a = 1: in ρ = r/a).
struct PlattenMode
{
    int m = 0;
    double lambda = 0.0;  ///< λ = k·a
    double c = 0.0;       ///< W = J_m(λρ) + c·I_m(λρ), dann ×norm
    double norm = 1.0;
    double hz = 0.0;
};

/// Vorab: Moden, Spannungsmoden, Kopplung (duenn).
struct Kopplung
{
    std::vector<PlattenMode> moden;
    std::vector<double> zeta4;            ///< ζ_n⁴ (dimensionslos)
    /// H^n_pq duenn: je Eintrag n, p, q, Wert (p ≤ q; dimensionslos, ×a⁻⁵)
    struct Eintrag
    {
        int n, p, q;
        double h;
    };
    std::vector<Eintrag> h;
    int spannungsModen = 0;
};

enum class Stelle
{
    Flaeche,  ///< r = 0,6·a (Bow)
    Kuppe,    ///< r = 0,15·a (Bell)
    Rand,     ///< r = 0,95·a (Edge)
};

struct BeckenSpiel
{
    std::string name;
    int note = 49;                 ///< Rolle im Kit
    double radiusM = 0.229;
    double dickeM = 0.9e-3;
    double eModul = 110e9;         ///< B20 (nur Rechenwerte, Sekiguchi & Samejima — offen)
    double dichte = 8860.0;
    double nu = 0.3;               ///< offen
    double bisHz = 3000.0;         ///< so hoch die nichtlinearen Moden
    int spannungsModen = 60;
    double schlaegelMasseKg = 0.05;
    double schlaegelQ0 = 2e8;      ///< N/m^p (offen)
    double schlaegelP = 1.5;
    double tempoPp = 1.0, tempoFf = 10.0;  ///< m/s (offen)
    double chokeS = 0.05;          ///< so schnell daempft die Hand (offen)
    bool hihat = false;
    double ausgangBezug = 1.0;
    std::string kennzeichen = "ungemessen";
    std::shared_ptr<const Kopplung> kopplung;
};

struct BeckenAnschlag
{
    double staerke = 0.5;
    Stelle stelle = Stelle::Flaeche;
    double chokeAbS = -1.0;   ///< ≥ 0: Choke ab dieser Zeit (F-4 a)
    double oeffnung = 1.0;    ///< Hi-Hat: 0 = geschlossen … 1 = offen (F-3 a)
    int rate = 48000;
};

struct BeckenMessung
{
    double kontaktS = 0.0;
    double kraftSpitzeN = 0.0;
    double energieAnfang = 0.0;    ///< nach dem Schlag
    double energieMaxFehler = 0.0; ///< ohne Daempfung: groesste relative Abweichung der Energie (Test)
    std::vector<double> bandEnergie;  ///< je 50 ms: Anteil der Energie ueber 1 kHz (Aufbluehen)
};

/// Die Moden einer freien Kreisplatte bis λ_max (cos-Familie, ohne Starrkoerper), aufsteigend.
[[nodiscard]] std::vector<PlattenMode> freiePlatte(double nu, double lambdaMax);
/// Die Moden und die Kopplung vorab.
void bereite(BeckenSpiel& s);
void rechneBeckenTon(const BeckenSpiel& s, const BeckenAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                     BeckenMessung* messung = nullptr, bool ohneDaempfung = false);

[[nodiscard]] BeckenSpiel baueVorgabeTamtam();
[[nodiscard]] BeckenSpiel baueVorgabeGong();
[[nodiscard]] BeckenSpiel baueVorgabeCrash();
[[nodiscard]] BeckenSpiel baueVorgabeChina();
[[nodiscard]] BeckenSpiel baueVorgabeSplash();
[[nodiscard]] BeckenSpiel baueVorgabeRide();
[[nodiscard]] BeckenSpiel baueVorgabeHihat();

}  // namespace ic::becken
