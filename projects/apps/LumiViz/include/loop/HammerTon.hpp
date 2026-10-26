/**
 * @file HammerTon.hpp
 * @brief Loop: ein Ton der Hammer-Saite — modale Saite, Filzhammer, Chor mit
 *        gekoppelten Moden, Daempfer (docs/audio/Spielweisen_Konzept.md §4.1,
 *        Stufe 31b; Entscheide Patrik S145/S146: HS1 a, HT1 a bis HT3 a)
 *
 * Nach Bank, Zambon & Fontana 2010: je Teilton k der Saite ein gedaempfter
 * Resonator, angestossen mit b0 = 2/(Lµ) an der Anschlagstelle, abgenommen als
 * Kraft am Steg (w_b,k = kπT0/L); der Hammer eine Masse mit der Feder
 * F = Q0·Δy^p, ohne Hysterese. Die Moden sind komplexe Pole nach der
 * Impulsinvarianz — eine Verzoegerung um einen Takt, keine verzoegerungsfreie
 * Schleife mit dem Hammer. Waehrend des Kontakts laeuft die Rechnung achtfach
 * feiner (`kHammerUeberabtastung`).
 *
 * Der Chor (HT3 a): je Teilton N ≤ 3 Saiten, jede um ihre Verstimmung
 * versetzt, ueber einen gemeinsamen Steg gekoppelt — die Eigenwerte von
 * A = diag(iω_j − σ_s) − σ_b·J geben Schwebung und zweistufigen Ausklang.
 * Seit 31d-7 (P2 a) schwingt jede Saite in zwei Eigenrichtungen, um γ gegen die
 * Senkrechte gedreht und um δ verstimmt; der Steg nimmt senkrecht σ_V, waagrecht
 * σ_H (`ebenenModen`, zerlegt mit `loop/Eigenzerlegung`).
 *
 * Derselbe Aufruf ergibt dieselben Werte. Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/HammerSaite.hpp"

#include <complex>
#include <cstdint>
#include <vector>

namespace lumi::loop
{

/// So viel feiner rechnet der Kontakt mit dem Hammer.
inline constexpr int kHammerUeberabtastung = 8;
/// Teiltoene nur bis zu diesem Anteil der Abtastrate.
inline constexpr double kHammerTeiltonGrenze = 0.45;
/// Hoechstens so viele Teiltoene je Saite.
inline constexpr int kHammerTeiltoeneMax = 400;

/// Was ein Ton der Hammer-Saite braucht.
struct HammerAnschlag
{
    int taste = 60;          ///< MIDI-Nummer — sie waehlt die Werte der Kurven
    double hz = 261.63;      ///< die Tonhoehe des ersten Teiltons
    int stufe = 0;           ///< Anschlagstufe (HK2 a), ≤ 0
    std::int64_t halten = 1; ///< so viele Abtastwerte bis zum Loslassen
    bool gedaempft = true;   ///< hat die Taste einen Daempfer (`undampedFrom`)?
    int rate = 48000;
};

/// Eine Mode des Chors an einem Teilton (fuer Tests und Messungen): der Eigenwert
/// λ = −α + iΩ in 1/s und ihr Anteil g an der Antwort am Steg (Σ g = 1).
struct ChorMode
{
    std::complex<double> lambda;
    std::complex<double> anteil;
};

/// Die Moden des Chors an einem Teilton (HT3 a): N Saiten mit den Kreisfrequenzen
/// `omega`, dem Verlust jeder Saite `sigmaSaite` und dem Verlust am Steg
/// `sigmaSteg`. Ohne Verstimmung bleibt nur die Gleichlage.
[[nodiscard]] std::vector<ChorMode> chorModen(const std::vector<double>& omega, double sigmaSaite, double sigmaSteg);

/// Eine Mode des Chors in zwei Ebenen (31d-7, P2 a): λ, ihr Anteil g = (c·v)(w·b)/N an der Antwort
/// vom Hammer zum Steg (c = b: der senkrechte Anteil jeder Stelle) und `form` — je Stelle 2j + a
/// (Saite j, Eigenrichtung a) die Auslenkung je Einheit des Zustands, der mit g angestossen wird:
/// v_i / (c·v).
struct EbenenMode
{
    std::complex<double> lambda;
    std::complex<double> anteil;
    std::vector<std::complex<double>> form;
};

/// Unter diesem Anteil (gegen den groessten des Teiltons) faellt eine Mode weg (P6 a) — mit γ = 0
/// die waagrechten.
inline constexpr double kEbenenAnteilMin = 1e-6;

/// Die Moden eines Teiltons mit N Saiten in je zwei Eigenrichtungen (31d-7, P2 a): `omega` die
/// Kreisfrequenzen der ersten, die zweite um `deltaCent` hoeher; die erste um `gamma` (rad) gegen
/// die Senkrechte des Stegs gedreht; `sigmaSaite` der Verlust jeder Saite, `sigmaV` und `sigmaH`
/// die Verluste senkrecht und waagrecht am Steg auf die Summe. Zerlegt mit `zerlegeKlein`; eine
/// Saite mit γ = 0 rechnet Bit fuer Bit wie `chorModen`. `anregung` (leer = gleich) gewichtet den
/// Stoss des Hammers je Saite, auf das Mittel 1 bezogen (US2 a, `stossJeSaite`).
[[nodiscard]] std::vector<EbenenMode> ebenenModen(const std::vector<double>& omega, double deltaCent, double gamma,
                                                  double sigmaSaite, double sigmaV, double sigmaH,
                                                  const std::vector<double>& anregung = {});

/// Der Stoss je Saite eines Chors aus u (US2 a): bei drei Saiten 1 + u : 1 : 1 − u, bei zwei 1 + u : 1 − u;
/// leer bei einer Saite oder u = 0 (gleicher Stoss).
[[nodiscard]] std::vector<double> stossJeSaite(int chor, double u);

/// So lang klingt der Ton nach dem Loslassen nach, in Abtastwerten: mit Daempfer
/// bis −60 dB unter ihm, ohne bis −60 dB der zweiten Stufe des Grundtons —
/// hoechstens `maxFrames`.
[[nodiscard]] std::int64_t hammerNachklang(const HammerSaite& saite, const HammerAnschlag& anschlag, std::int64_t maxFrames);

/// Was beim Anschlag geschieht (fuer Tests und Messungen gegen die Literatur).
struct HammerMessung
{
    double kontaktS = 0.0;       ///< so lange lag der Hammer an (vom ersten bis zum letzten Druck)
    double kraftSpitzeN = 0.0;   ///< die groesste Kraft zwischen Hammer und Saite
    /// Die groesste Auslenkung einer Saite in den ersten 30 ms, an neun Stellen
    /// (L/10 … 9L/10) gesucht, in m.
    double auslenkungSpitzeM = 0.0;
};

/// Einen Ton rechnen: `frames` Abtastwerte in eine Seite (Mono), der Wert in
/// Einheiten von `kHammerKraftBezug` Newton am Steg; endet bei null. Mit
/// `messung` wird festgehalten, was beim Anschlag geschieht.
void rechneHammerTon(const HammerSaite& saite, const HammerAnschlag& anschlag, std::vector<float>& mono, std::int64_t frames,
                     HammerMessung* messung = nullptr);

/// Die Kraft am Steg, die als 1 ausgegeben wird (von Claude gesetzt; den Pegel
/// des Instruments stimmt Stufe 31h).
inline constexpr double kHammerKraftBezug = 200.0;

/// Der Schalldruck, der mit Resonanzboden als 1 ausgegeben wird, in Pa (PG1 a, gleich fuer alle
/// Koerper). Gemessen im Messteil von 31c-7 (S150, `test_LoopKoerperKlang`): der lauteste Ton des
/// Steinway D bei Anschlagstufe 0 (Taste 90, eine Sekunde gehalten) erreicht am Hoerort 48,8 Pa
/// Spitze; so liegt er bei −6 dBFS.
inline constexpr double kSchalldruckBezug = 97.3;

}  // namespace lumi::loop
