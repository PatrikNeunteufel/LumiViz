/**
 * @file Modal.hpp
 * @brief InstrumentCore: der modale Kern — Moden als komplexe Pole mit Anteil vom Anreger zum Abnehmer,
 *        Impulsinvarianz, Summe der Moden als Ton, Nachklang aus der schwaechsten Daempfung; dazu die
 *        Anreger-Schnittstelle fuer einen Stoss (Hammer, Schlaegel, Stock) im Zeitbereich gegen die Auslenkung des
 *        Kerns (IC AUFTRAG_BAU §3)
 *
 * Uebernommen aus IC `rechnung/gemeinsam/Modal.hpp` (`bb7e6d6`); nur Namensraum und dieser Kopf geaendert
 * (Instrumente_IC_Konzept.md IK2-2 a, KN2 a; Uebernahme-Journal). In LumiViz bis 34d ohne Nutzer — der Hammer-Ton
 * rechnet seine Moden selbst (`src/loop/HammerTon.cpp`, bleibt wortgleich, IK2-2 a); Nutzer sind die Kerne der
 * Klassen 4 und 5 (34g, 34h).
 *
 * Nach dem Vorbild `HammerTon.cpp` der App (Bank, Zambon & Fontana 2010): je Mode ein Zustand z (komplex), die Auslenkung
 * ist Im(z), ein Schritt ist z ← p·(z + e·F·Δt) mit p = e^(λΔt) — die Mode sieht die Kraft erst im naechsten
 * Schritt (Impulsinvarianz, Bank 2010 Gl. 10), keine verzoegerungsfreie Schleife mit dem Anreger. Waehrend des
 * Kontakts rechnet der Stoss `ueberabtastung`-fach feiner (Vorbild: achtfach; Rucz & Ulveczki 2021 nennen fuer den
 * Schlaegel „an oversampling of 100–200 times“ — die Klasse waehlt).
 *
 * **Eine Mode** (Einheiten): eine Kraft F [N] am Anregungspunkt treibt die modale Koordinate q mit
 * q̈ + 2σq̇ + ω²q = φ_ein·F/m. Mit λ = −σ + iω_d ist die Impulsantwort der Auslenkung am Abnehmer
 * h(t) = (φ_ein·φ_aus / (m·ω_d))·e^(−σt)·sin(ω_d t) = Im(e·e^(λt))·φ_aus mit e = φ_ein/(m·ω_d). Der Ausgang einer
 * Mode ist `aus`·Im(z) — Auslenkung, Kraft am Steg oder Abstrahlgewicht, je nach Klasse.
 *
 * Der Kontakt (`Stossanreger`): eine Masse m_A mit Geschwindigkeit v0 gegen die Auslenkung y(x_s) = Σ φ_n q_n; die
 * Kraft aus der Eindrueckung δ = y_A − y(x_s) [mm] nach Stulov (Hysterese, wie der Hammer des Vorbilds, HU3-1 c):
 * F = F0·[φ − (ε/τ)·∫φ·e^(−(t−ξ)/τ)dξ], φ = δ₊^p, F0 = Q0/(1 − ε); mit ε = 0 die Hertz-Feder F = Q0·δ^p (Schlaegel nach
 * Rucz & Ulveczki 2021 Gl. 6: p = 3/2). Unter null zieht der Filz nicht. Eine zweite Form nach Hunt–Crossley
 * F = k·δ^α·(1 + λ_HC·δ̇) steht als Schalter bereit (Filz/Neopren beim Rhodes, Pfeifle 2017 — nur Bericht).
 *
 * Derselbe Aufruf ergibt dieselben Werte (feste Reihenfolge, E1 a). Reines C++.
 */
#pragma once

#include <array>
#include <complex>
#include <cstdint>
#include <vector>

namespace lumi::kern
{

using Komplex = std::complex<double>;

/// −60 dB: e^(−σ·T60) = 1/1000.
inline constexpr double kLn1000 = 6.907755278982137;

/// σ aus einer Abklingzeit T60 in s.
[[nodiscard]] inline double sigmaAusT60(double t60S) { return kLn1000 / t60S; }

/// Eine Mode beim Rechnen.
struct Mode
{
    Komplex lambda;            ///< −σ + iω_d in 1/s
    Komplex eingang;           ///< so viel bringt eine Kraft von 1 N·s am Anregungspunkt (φ_ein/(m·ω_d), oder komplex)
    double abnahme = 0.0;      ///< φ am Kontaktpunkt: die Auslenkung dort je Einheit von Im(z)
    /// Gewicht am Ausgang: der Ausgang ist Im(aus·z) — reell fuer die Auslenkung (Abnehmer, Steg), λ·g fuer die
    /// Geschwindigkeit, λ²·g fuer die Beschleunigung (Abstrahlung) einer frei schwingenden Mode
    Komplex aus{0.0, 0.0};
    Komplex zustand{0.0, 0.0};
};

/// Was der Ausgang einer Mode ist.
enum class Ausgang
{
    Auslenkung,
    Geschwindigkeit,
    Beschleunigung,
};

/// Eine Mode aus ihren reellen Groessen: Frequenz f [Hz] (gedaempft), Abklingzeit T60 [s], modale Masse m [kg],
/// Formwert am Anregungspunkt, am Kontaktpunkt (meist derselbe) und Gewicht am Ausgang (mal λ bzw. λ²).
[[nodiscard]] Mode modeAus(double hz, double t60S, double masseKg, double phiEin, double phiKontakt, double gewichtAus,
                           Ausgang ausgang = Ausgang::Auslenkung);

/// Eine Bank von Moden mit Impulsinvarianz.
class ModalBank
{
public:
    ModalBank() = default;
    ModalBank(std::vector<Mode> moden, double rate);

    [[nodiscard]] const std::vector<Mode>& moden() const { return m_moden; }
    [[nodiscard]] std::vector<Mode>& moden() { return m_moden; }
    [[nodiscard]] double rate() const { return m_rate; }

    /// Die Auslenkung am Kontaktpunkt: Σ abnahme·Im(z).
    [[nodiscard]] double auslenkung() const;
    /// Der Ausgang: Σ Im(aus·z).
    [[nodiscard]] double ausgang() const;
    /// Ein Schritt der Laenge Δt = 1/(rate·teilung) mit der Kraft F [N] am Anregungspunkt.
    void schritt(double kraft, int teilung = 1);
    /// Ein Schritt ohne Kraft.
    void frei();
    /// Die Daempfung jeder Mode auf mindestens `sigma` heben (Daempfer: er verkuerzt, nie verlaengert er; Bank 2010
    /// S. 12). Mit `sigma` ≤ 0 die urspruengliche.
    void daempfe(double sigma);

    /// Die Moden frei ausklingen lassen, je Mode ein Zeiger (wie im Vorbild): addiert in `summe` ab `ab`
    /// `frames − ab` Werte des Ausgangs; ein Daempfer greift ab `daempferAb` mit `sigmaDaempfer` (≤ 0: keiner).
    /// Eine Mode endet, wenn sie 140 dB unter dem Ganzen liegt.
    void klingeAus(std::vector<double>& summe, std::int64_t ab, std::int64_t frames, std::int64_t daempferAb = -1,
                   double sigmaDaempfer = 0.0) const;

private:
    void richtePole();
    std::vector<Mode> m_moden;
    std::vector<Komplex> m_pol;
    std::vector<Komplex> m_polFein;
    std::vector<Komplex> m_polDaempfer;
    int m_teilungFein = 0;
    double m_rate = 48000.0;
};

/// Die Impulsantwort des Ausgangs auf einen Kraftstoss von 1 N·s, `frames` Werte (ohne Kontakt).
[[nodiscard]] std::vector<double> impulsantwort(const std::vector<Mode>& moden, double rate, std::int64_t frames);

/// So viele Abtastwerte, bis die schwaechst gedaempfte Mode um `db` gefallen ist (hoechstens `maxFrames`).
[[nodiscard]] std::int64_t nachklangFrames(const std::vector<Mode>& moden, double rate, double db, std::int64_t maxFrames);

// --- ein gekoppeltes Paar (Naeherung rotierender Wellen) ------------------------------------------------------

/// Die exakte Exponentialmatrix eines Paars ż = M·z mit M = [[λ_a, −iκ], [−iκ, λ_b]] ueber Δt (zeilenweise
/// e00, e01, e10, e11). Zwei Moden fast gleicher Frequenz, ueber κ [1/s] gekoppelt (Resonator, Tonebar, Steg des
/// Hackbretts); die Eigenwerte (λ_a + λ_b)/2 ± √(((λ_a − λ_b)/2)² − κ²). Bei κ = |σ_a − σ_b|/2 fallen sie zusammen
/// (Entartung, L11) — die geschlossene Form bleibt dort stetig.
[[nodiscard]] std::array<Komplex, 4> paarSchritt(Komplex lambdaA, Komplex lambdaB, double kappa, double dt);
/// Dasselbe mit beliebiger (symmetrischer) Kopplung c auf der Nebendiagonale: M = [[λ_a, c], [c, λ_b]] — etwa der
/// Verlust am Steg c = −σ_V zwischen zwei Saiten wie im Chor des Vorbilds.
[[nodiscard]] std::array<Komplex, 4> paarSchrittMit(Komplex lambdaA, Komplex lambdaB, Komplex c, double dt);

// --- der Stoss ------------------------------------------------------------------------------------------------

/// Die Groessen eines stossenden Anregers (Hammer, Schlaegel, Stock).
struct Stossanreger
{
    double masseKg = 0.025;  ///< die Masse des Kopfes
    double q0 = 1.0;         ///< Steife in N/mm^p (bei langsamer Last)
    double p = 1.5;          ///< Exponent
    double eps = 0.0;        ///< Hysterese ε (Stulov), 0 … 0,999
    double tauS = 0.0;       ///< Relaxation τ in s
    bool huntCrossley = false;  ///< statt Stulov: F = q0·δ^p·(1 + lambdaHc·δ̇[mm/s])
    double lambdaHc = 0.0;   ///< s/mm
};

/// Was beim Stoss geschah.
struct StossMessung
{
    double kontaktS = 0.0;      ///< vom ersten bis zum letzten Druck
    double kraftSpitzeN = 0.0;
    std::int64_t frames = 0;    ///< so viele Abtastwerte hat der Stoss gerechnet (Ausgang in `summe`)
    int kontakte = 0;           ///< wie oft der Kopf neu anlag (Doppelschlag ≥ 2)
};

/// Den Stoss rechnen: der Kopf trifft mit `tempo` [m/s] die Bank am Kontaktpunkt; je grober Takt der Ausgang in
/// `summe` (ab 0), feiner gerechnet mit `ueberabtastung`; endet, wenn der Kopf `freiS` frei ist und sich entfernt,
/// spaetestens nach `maxS`. Die Bank behaelt ihren Zustand (danach `klingeAus`).
StossMessung stosse(ModalBank& bank, const Stossanreger& anreger, double tempo, std::vector<double>& summe, int ueberabtastung,
                    double maxS = 0.03, double freiS = 0.001);

}  // namespace lumi::kern
