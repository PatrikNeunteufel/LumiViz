/**
 * @file Zupf.hpp
 * @brief InstrumentModels, Klasse 2 Gezupfte Saite (Deluxe): eine Saite als Waveguide in **zwei Polarisationen**, am Steg ueber eine
 *        **2×2-Admittanz** gekoppelt (Bank & Karjalainen 2010), angezupft mit Polarisationswinkel; Cembalo, Harfe,
 *        Gitarre, Laute/Oud (`klassen/Klasse 2 - Gezupfte Saite/modell-entwurf.md`; Entscheide Patrik 2026-10-26: F-1 …
 *        F-10 a)
 *
 * **Steg** (Bank & Karjalainen 2010, nachgelesen PDF-S. 1): „The admittance matrix is synthesized as a sum of PR scalar
 * transfer functions (second-order filters) multiplied by positive semidefinite matrices“ — hier je Mode eine Richtung
 * d_k (Winkel gegen die Senkrechte zur Decke) und ein Modalfilter: Y(ω) = Σ_k H_k(ω)·d_k·d_kᵀ (Rang 1, positiv
 * semidefinit, passiv). Geschlossen geloest je Takt: (I + Z0·M)·u = u_vorher + 2Z0·M·v⁺ (M = Σ D_k d_k d_kᵀ der Durchgriff).
 * „strong beating and two-stage decay appears for those partials which are in that frequency range where the elements
 * of the admittance matrix are large“ (ebd., nachgelesen PDF-S. 5).
 *
 * **Saite:** je Polarisation zwei Verzoegerungen (Steg–Zupfpunkt–Sattel als eine Schleife), Lagrange 3, Verlust je Umlauf
 * aus der Guete; Steifigkeit nicht gebaut (offen). **Zupfen:** ein Kraftstoss am Zupfpunkt in der Richtung des
 * Polarisationswinkels (Scherrer & Depalle: „The AOR … distributes the excitation … over the two transverse directions“,
 * nachgelesen PDF-S. 1); Dauer und Hoehe aus Ablenkung und Geschwindigkeit beim Abloesen (Cembalo, Harfe — Chadefaux).
 * **Ausgang:** die Schnelle des Stegs senkrecht zur Decke, abgeleitet (Abstrahlung genaehert, offen).
 *
 * Uebernommen aus IC `rechnung/klassen/02_zupf_saite/` (`4409088`, Vorlage unter `harvest/`), die Rechnung wortgleich;
 * geaendert (Instrumente_IC_Konzept.md 34k-2, Uebernahme-Journal J77): Namensraum und Includes; das Messen zieht aus dem
 * Kern (IK2-2 a) — `rechneZupfSumme` gibt die Summe vor Bezug und Ausblende, an der IC die Tonhoehe mass, die Messung
 * steht in `InstrumentMeasure/ZupfStimmen`, `stimme` zieht dorthin (34k-3); der Zupfpunkt ist immer eine Kurve (IC:
 * leer = `zupfpunktRel`; die Vorgaben tragen den festen Wert als flache Kurve, dieselben Zahlen); ohne Koto und ohne
 * `spielbar`, `grund`, `kennzeichen` (zurueckgestellt bzw. ohne Wirkung auf den Ton; „ungemessen“ folgt aus der
 * Herkunft, IK2-9).
 *
 * Reines C++ ohne Qt.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace lumi::kern::zupf
{

/// Eine Mode des Stegs: Frequenz, Guete, Spitze der Admittanz (s/kg) und Richtung (Grad gegen die Senkrechte).
struct StegMode
{
    double hz = 200.0;
    double q = 20.0;
    double spitze = 0.01;
    double winkelGrad = 0.0;
};

/// Ein Chor: Saiten einer Note (Doppelchor der Laute/Oud: zwei, gegeneinander verstimmt — F-9 a).
struct Chor
{
    int saiten = 1;
    double verstimmungCent = 0.0;
};

struct ZupfSpiel
{
    std::string name;
    int noteMin = 40;
    int noteMax = 84;
    // --- Saite: Laenge und Zug je Note (Kurven, log), Guete ---
    Kurve laengeJeNote;              ///< m
    Kurve zugJeNote;                 ///< N
    double guete = 1500.0;           ///< Q der Saite (offen je Instrument)
    Kurve zupfpunktJeNote;           ///< vom Steg, Anteil der Laenge (Cembalo: 12 % → 50 %, Valimaki)
    Kurve laufzeitJeNote;            ///< Faktor auf die Laufzeiten (Feinstimmung durch das Modell, Rechnung); leer = 1
    double polarisationGrad = 25.0;  ///< Winkel beim Abloesen gegen die Senkrechte (Harfe 7–43°, Cembalo 17/20°)
    double stossS = 0.002;           ///< Dauer des Kraftstosses (offen)
    double kraftPpN = 0.5, kraftFfN = 5.0;  ///< Hoehe des Stosses pp … ff (offen)
    std::vector<StegMode> steg;
    Chor chor;
    // --- Cembalo: der Stoss der Taste in den Boden (F-4 a: Penttinen — +10 dB in den Moden < 300 Hz je Stufe) ---
    double tastenstoss = 0.0;        ///< Hoehe gegen die Saite (offen; 0 = ohne)
    double daempfenS = 0.08;         ///< Notenende: Daempfer bzw. Hand (F-7 a), in so vielen s
    /// Mitschwingen je Clip (K2 F-3 a, Deluxe): die Saiten, die ungespielt am Steg haengen (Gitarre: die leeren Saiten;
    /// Harfe: alle Saiten der Stimmung) — leer: nur die gespielten (Cembalo: hinter dem Daempfer)
    std::vector<int> mitschwingNoten;
    double ausgangBezug = 1.0;
};

struct ZupfAnschlag
{
    int note = 60;
    double staerke = 0.5;
    double haltenS = 1.0;
    bool laissezVibrer = false;    ///< Harfe: die Hand daempft nicht (F-7 a)
    bool lautenzug = false;        ///< Cembalo: Filz an der Saite (F-5 a) — Daempfung ×8 (offen)
    double polarisationGrad = -1.0;  ///< ≥ 0: statt der Vorgabe
    int rate = 48000;
};

/// Eine Note eines Clips: ab `abS` gezupft.
struct ClipNote
{
    double abS = 0.0;
    ZupfAnschlag anschlag;
};

/// Den Clip als Ganzes rechnen (K2 F-3 a: Mitschwingen ist Deluxe und rechnet **je Clip**): alle gespielten Saiten und
/// die `mitschwingNoten` gleichzeitig an **einem** Steg — (I + ΣZ0ᵢ·M)·u = u_vorher + 2·M·ΣZ0ᵢ·v⁺ᵢ. Eine gespielte Note
/// auf der Tonhoehe einer mitschwingenden Saite zupft diese Saite. Chor und Tastenstoss gibt es hier nicht (nur je Note).
void rechneZupfClip(const ZupfSpiel& s, const std::vector<ClipNote>& noten, std::vector<float>& mono, std::int64_t frames,
                    bool mitschwingen = true, int rate = 48000);

/// Der Ton einer Note: `rechneZupfSumme`, durch den Bezug geteilt, die letzten 5 ms ausgeblendet.
void rechneZupfTon(const ZupfSpiel& s, const ZupfAnschlag& a, std::vector<float>& mono, std::int64_t frames);
/// Die Summe der Saiten des Chors und des Tastenstosses vor Bezug und Ausblende — an ihr mass IC die Tonhoehe
/// (`InstrumentMeasure/ZupfStimmen`, IK2-2 a).
void rechneZupfSumme(const ZupfSpiel& s, const ZupfAnschlag& a, std::vector<double>& summe, std::int64_t frames);

// --- Harfe: Pedale (K2 F-6 a) ---
// Sieben Pedale stimmen je eine Stammtonklasse (C D E F G A H) ueber alle Oktaven: −1 ♭, 0 ♮, +1 ♯ (Wikipedia, nur
// Bericht). Das Diagramm ist eine Passage-Spur mit Umschaltstellen; das Programm schlaegt es aus den Noten vor, der
// Nutzer kann es aendern (enharmonische Wahl); was mit dem Diagramm nicht spielbar ist, wird markiert. Glissando und
// Bisbigliando lesen es. Die Wechselzeit eines Pedals ist offen (keine Quelle) — der Vorschlag setzt die Umschaltstelle
// auf den Anfang der Note, die sie braucht.

/// Ein Pedaldiagramm: Stellung je Stammton C, D, E, F, G, A, H (Index 0 … 6).
using Pedale = std::array<int, 7>;

/// Eine Umschaltstelle der Pedal-Spur.
struct PedalStelle
{
    double abS = 0.0;
    Pedale pedale{0, 0, 0, 0, 0, 0, 0};
};

/// Die Tonhoehen aller Saiten der Harfe (Stammtoene im Umfang) unter dem Diagramm.
[[nodiscard]] std::vector<int> harfenSaiten(const ZupfSpiel& harfe, const Pedale& p);
/// Die Saiten (als Stammton-Index 0 … 6), die unter dem Diagramm die Tonhoehe `note` geben — zwei: Bisbigliando moeglich.
[[nodiscard]] std::vector<int> saitenFuer(int note, const Pedale& p);
/// Spielbar unter dem Diagramm?
[[nodiscard]] bool spielbar(int note, const Pedale& p);
/// Ein Glissando von `von` nach `bis`: die Tonhoehen aller Saiten dazwischen unter dem Diagramm (in Spielrichtung).
[[nodiscard]] std::vector<int> glissandoNoten(const ZupfSpiel& harfe, int von, int bis, const Pedale& p);
/// Ein Diagramm, unter dem alle `noten` spielbar sind, mit den wenigsten Pedalen ausser ♮ (bei Gleichstand: ♯ vor ♭
/// nicht bevorzugt — die erste in der Suche); `ok` falsch, wenn es keins gibt (dann das beste mit den meisten Noten).
[[nodiscard]] Pedale pedaleVorschlag(const std::vector<int>& noten, bool* ok = nullptr);
/// Die Pedal-Spur fuer einen Clip: der Vorschlag laeuft die Noten in der Zeit ab und setzt eine Umschaltstelle, wo eine
/// Note mit dem bisherigen Diagramm nicht spielbar ist (nur das noetige Pedal wechselt).
[[nodiscard]] std::vector<PedalStelle> pedalSpurVorschlag(const std::vector<ClipNote>& noten);
/// Die Harfe unter einem Diagramm: die mitschwingenden Saiten neu gestimmt (fuer `rechneZupfClip`).
[[nodiscard]] ZupfSpiel harfeMitPedalen(const ZupfSpiel& harfe, const Pedale& p);

/// Die Vorgaben ohne gestimmte Laufzeit und Bezug (die setzt `InstrumentTuneTool`; IC: `stimme` und `normiere`).
[[nodiscard]] ZupfSpiel baueVorgabeCembalo();
[[nodiscard]] ZupfSpiel baueVorgabeHarfe();
[[nodiscard]] ZupfSpiel baueVorgabeGitarre();
[[nodiscard]] ZupfSpiel baueVorgabeLaute();
[[nodiscard]] ZupfSpiel baueVorgabeOud();

}  // namespace lumi::kern::zupf
