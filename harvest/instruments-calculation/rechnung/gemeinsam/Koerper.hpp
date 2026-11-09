/**
 * @file Koerper.hpp
 * @brief Gemeinsam: der Koerper als Liste von Moden (KB1 a) — Einkoppelpunkt, Abstrahlgewicht, Mobilitaet am Punkt,
 *        Antwort auf einem Raster — und der Baustein **Luftraum** (FRAGEN_KONSOLIDIERT T4 a, Entscheid Patrik
 *        2026-10-29): Kolben eines Koerpers, gekoppelt ueber den Druck eines Hohlraums mit Oeffnung (Helmholtz)
 *
 * **Koerper** (nach `gemeinsam/koerper.md` §1, KB1 a in LumiViz): je Mode Frequenz, Abklingzeit, modale Masse am
 * Einkoppelpunkt (die wirksame Masse: Y_k(ω) = iω / (m_k (ω_0k² − ω² + 2iσ_k ω))) und ein Abstrahlgewicht je Ausgang;
 * der Ausgang ist Σ g_k · q̈_k (Druck einer kleinen Quelle ∝ Beschleunigung des Volumens). Koerper aus Bauplaenen
 * (Platte mit Rippen, Netz, Spectra) sind **offen, lokal** (AUFTRAG_BAU §3) — hier nur Koerper aus Modenlisten mit
 * Quellenwerten.
 *
 * **Luftraum** (T4 a, „nach dem Muster des Gitarrenkorpus in LumiViz (Wright Gl. A.20–A.24, S-K10 a)“, beschrieben
 * in `vorbild-lumiviz/docs/Saiten_Konzept.md` §5e/§5f, dort im Wortlaut nachgelesen): je Mode des Koerpers ein Kolben
 * mit Masse m_k, Steife k_k, Verlust R_k und Flaeche A_k; die Luft in der Oeffnung ein Kolben mit m_h, R_h, A_h; alle
 * haengen nur ueber den einen Druck P = −α·(Σ A_k x_k + A_h x_h) im Hohlraum zusammen, α = ρc²/V (x nach aussen
 * positiv). Abgeleitet wie dort (S-K7 a, aus Wrights Werten): die Steife je Mode k = m·ω² − α·A² (die Frequenzen
 * gelten bei geschlossener Oeffnung, Wright Gl. A.12, A.28), R = m·ω/Q, die Masse der Luft m_h = α·A_h²/ω_h²
 * (Gl. A.14). Abgestrahlt wird die Aenderung des Volumenflusses U = Σ A_k ẋ_k + A_h ẋ_h (Wright Gl. 4.17); der Druck
 * im Abstand r ist ρ/(4πr)·dU/dt. Weitere Hohlraummoden und mehrere Oeffnungen (Loecher als zusaetzliche Ausgaenge)
 * sind als Liste von Oeffnungen angelegt. Gerechnet wird das gekoppelte System exakt ueber seine Eigenwerte (kleiner
 * Eigenloeser, 2·(N + Oeffnungen) Zustaende) — keine Trapezregel wie in LumiViz; daraus Moden fuer den Zeitbereich.
 *
 * **Pruefstein offen (lokal):** die gekoppelten Spitzen des Parlor (Hess 2013 Tab. 1: 116 und 219 Hz) und der
 * Classical (Wright TOP26 Tab. 7.1) — die Werte der Tabellen liegen nicht im Repo; geprueft wird hier gegen die
 * geschlossene Loesung zweier gekoppelter Schwinger.
 */
#pragma once

#include "gemeinsam/Modal.hpp"

#include <string>
#include <vector>

namespace ic
{

/// Eine Mode eines Koerpers.
struct KoerperMode
{
    double hz = 100.0;        ///< Frequenz (bei geschlossener Oeffnung, wenn der Koerper einen Luftraum hat)
    double t60S = 1.0;        ///< Abklingzeit
    double masseKg = 0.1;     ///< modale Masse am Einkoppelpunkt
    double flaecheM2 = 0.0;   ///< Kolbenflaeche (Volumen je Auslenkung), 0 = strahlt als Volumen nicht ab
    double abstrahl = 0.0;    ///< Abstrahlgewicht (ohne Luftraum): Ausgang += abstrahl·q̈
    bool getrieben = true;    ///< von der Kraft am Einkoppelpunkt getrieben (der Boden einer Gitarre nicht, Gl. A.20)
};

/// Ein Koerper aus Moden mit Herkunft.
struct ModenKoerper
{
    std::vector<KoerperMode> moden;
    std::string herkunft;
};

/// Die Mobilitaet (Geschwindigkeit/Kraft) am Einkoppelpunkt bei f [Hz], in m/(N·s).
[[nodiscard]] Komplex mobilitaet(const ModenKoerper& koerper, double hz);

/// Die Moden fuer den Zeitbereich: Eingang die Kraft am Einkoppelpunkt, Ausgang Σ abstrahl·q̈, Kontakt die
/// Auslenkung am Einkoppelpunkt.
[[nodiscard]] std::vector<Mode> koerperModen(const ModenKoerper& koerper);

/// Die Antwort des Ausgangs auf einen Kraftstoss von 1 N·s, `frames` Werte.
[[nodiscard]] std::vector<double> koerperAntwort(const ModenKoerper& koerper, double rate, std::int64_t frames);

// --- der Luftraum (T4 a) ---------------------------------------------------------------------------------------

/// Eine Oeffnung des Hohlraums (Schallloch, f-Loch, Loch der Kalimba): ihre Luft als Kolben.
struct Oeffnung
{
    double flaecheM2 = 5.7e-3;
    double hz = 133.0;   ///< Helmholtz-Frequenz bei starren Waenden: daraus m_h = α·A²/ω_h²
    double q = 30.0;     ///< Guete der Luft: R_h = m_h·ω_h/Q
    double abstand = 1.0;  ///< Faktor auf ihren Beitrag zum Volumenfluss (1 = voll; 0 = abgedeckt)
};

/// Ein Hohlraum mit Waenden (Moden des Koerpers als Kolben) und Oeffnungen.
struct Luftraum
{
    double volumenM3 = 14e-3;
    double dichte = 1.205;   ///< kg/m³ (Luft 20 °C; in LumiViz von Claude gesetzt, S-K10 a)
    double schall = 343.0;   ///< m/s
    std::vector<KoerperMode> waende;  ///< Moden der Waende mit `flaecheM2` und Frequenz bei geschlossener Oeffnung
    std::vector<Oeffnung> oeffnungen;
    std::string herkunft;
};

/// Die gekoppelten Moden des Luftraums (Eigenwerte λ, Anteil von der Kraft am Einkoppelpunkt zu dU/dt) — Ausgang
/// dU/dt in m³/s² je N; mal ρ/(4πr) ist der Druck im Abstand r. `vollstaendig` falsch, wenn der Eigenloeser nicht
/// zusammenlief oder entartete Werte meldete (L11).
struct LuftraumModen
{
    std::vector<Mode> moden;
    /// Der Durchgriff: dU/dt folgt der Kraft auch sofort (aᵀM⁻¹f, m³/s² je N) — die Masse der Kolben; im Zeitbereich
    /// Ausgang += durchgriff·F.
    double durchgriff = 0.0;
    bool vollstaendig = true;
    bool entartet = false;
};
[[nodiscard]] LuftraumModen luftraumModen(const Luftraum& raum);

/// Der Betrag der Antwort |dU/dt / F| bei f (fuer Spitzen und Pruefungen), direkt aus dem linearen System, mit
/// dem Durchgriff.
[[nodiscard]] double luftraumBetrag(const Luftraum& raum, double hz);

}  // namespace ic
