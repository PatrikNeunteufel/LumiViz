/**
 * @file Saite.hpp
 * @brief InstrumentModels, Kern `struckString` (34i-3, KL1 a; uebernommen aus IC `rechnung/klassen/01_hammer_saite/Saite.hpp`,
 *        `bb7e6d6`, wortgleich bis auf Includes, Namensraum und den Umfang beim Richten, KL4-1 a): ein zweiter Saitenkern fuer die Instrumente, die der Kern des Fluegels nicht traegt —
 *        **Clavinet** (Tangente: Stoss und Klemme, Loslassen auf die ganze Saite), **Hackbrett** und **Cimbalom**
 *        (Chor bis fuenf Saiten, Saite ueber den Mittelsteg 3 : 2 geteilt, Schlaegel)
 *        (`klassen/Klasse 1 - Hammer-Saite/modell-entwurf.md` §2.4, §2.5; Entscheide F-1 a, F-3 a, F-4-1 a, F-6 b)
 *
 * **Saite** wie im Vorbild (`HammerTon`): je Teilton k die Moden des Chors in zwei Ebenen aus `ebenenModen`
 * (Bank 2010, Eigenloeser `gemeinsam/Eigenzerlegung`), f_k = k·f0·√(1 + B·k²), zwei Stufen σ_frueh/σ_spaet, Verluste
 * σ_k = σ_1·(1 + hoch·((f_k/f_1)² − 1)) (`abklingHoch` wie dort: T60 bei 10·f_1 gegen T60 bei f_1).
 *
 * **Tangente** (F-3 a, eigene Anregung in SF10 a): die Tangente stoesst die Saite und haelt sie gegen den Amboss
 * („strikes the string and traps it against a metal stud … for the duration of the note, splitting the string into
 * speaking and nonspeaking parts“, Gabrielli et al. 2013 S. 4, nachgelesen IC Session 1). Gerechnet als bewegtes Ende
 * des klingenden Teils: y = h(t)·(1 − x/L_s) + Σ q_k sin(kπx/L_s), die Moden getrieben von −(2/(kπ))·ḧ(t); h(t) ein
 * Kosinus-Anstieg auf den Weg d in der Zeit d/v (v = Tastengeschwindigkeit „1 to 4 m/s“, Gabrielli S. 11,
 * nachgelesen). **Abweichung vom Entwurf:** kein Kontaktgesetz der Gummispitze (Q0, p offen; Gabrielli nimmt selbst
 * einen gefitteten Puls) — der Anstieg ist die Form des Pulses, seine Dauer folgt aus v. **Loslassen:** Auslenkung
 * und Geschwindigkeit des klingenden Teils (mit dem stillen Teil als Gerade auf die Hoehe h) werden auf die Moden der
 * ganzen Saite projiziert, L_ganz = L_s·2^(3/12) („the pitch decrease after release is three semitones for the whole
 * keyboard“, S. 6, nachgelesen), mit der Daempfung der Garnwicklung. Der **Tastendruck** ist ein Pitch nur nach oben
 * mit **Grenze 0 Cent** bis zu einer Messung (F-4-1 a), der **Daempferschieber** eine stufenlose Passage-Spur 0 … 1.
 * Abnahme: zwei elektromagnetische Tonabnehmer (Geschwindigkeit der Saite an zwei Stellen; Lagen nach Remaggi 2012 —
 * nur Bericht; die Kennlinie 4. Ordnung, Gabrielli Tab. 2, ist offen).
 *
 * **Mittelsteg** (Hackbrett, Cimbalom): der Steg teilt den Chor 3 : 2 („the middle bridge splits string courses into
 * a length ratio of 3:2“, Peterson 1994, nur Bericht) — der angeschlagene Abschnitt und der andere mit dem
 * Frequenzverhaeltnis L_a/L_b; wo ein Teilton des einen auf einen des anderen faellt (bei 3 : 2 jeder dritte des langen
 * auf jeden zweiten des kurzen), koppeln beide ueber den Steg (Paar wie `gemeinsam/Modal` `paarSchritt`, die Kopplung
 * als Verlust am Steg −σ_V wie im Chor des Vorbilds). **Mitschwingen der anderen Choere** (ohne Daempfer dauernd, Deluxe,
 * je Clip nach T2 a / K2 F-3 a): mit `SaitenAnschlag::mitschwingen` koppelt die staerkste Mode jedes Teiltons an die
 * naechste Mode eines anderen Chors (alle Noten des Umfangs, je ein Abschnitt), wenn sie auf 0,5 % gleich liegt — wie
 * am Mittelsteg, Kopplung `mitschwingKopplung`·σ_frueh. Hat die Mode schon einen Partner am Mittelsteg, wird je Takt
 * geteilt (erst das Paar am Mittelsteg exakt, dann die Kopplung allein; Fehler O(Δt²·c·σ)). Weil alles linear ist, ist
 * ein Clip die Summe seiner Noten mit Mitschwingen (`rechneSaitenClip`).
 *
 * Reines C++, deterministisch.
 */
#pragma once

#include "InstrumentCore/Kurve.hpp"
#include "InstrumentCore/Modal.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace lumi::kern::saite
{

/// Der moegliche Umfang aller Instrumente des Kerns (die Tafel, KS1 a): A0 … C8. Auf ihn richten sich die Kurven
/// (KL4-1 a); der Umfang eines Instruments (`noteMin`/`noteMax`) verschiebt keine Stuetzpunkte.
inline constexpr int kSaitenUmfangMin = 21;
inline constexpr int kSaitenUmfangMax = 108;

enum class Anregung
{
    Schlaegel,  ///< Stoss (Hackbrett, Cimbalom)
    Tangente,   ///< Stoss und Klemme (Clavinet)
};

/// Die Wahl der Tonabnehmer am Clavinet (Gabrielli & Välimäki, ICMC 2011 S. 3: „selected individually, summed, …
/// summed in counter-phase“; Gabrielli et al. 2013 S. 3: „sums of pickup signals in phase or in anti-phase“).
enum class AbnehmerWahl
{
    Beide,       ///< in Phase summiert
    Steg,        ///< nur der Tonabnehmer am Steg (`abnehmerStellen[0]`)
    Mitte,       ///< nur der mittlere (`abnehmerStellen[1]`)
    Gegenphase,  ///< Steg minus Mitte
};

enum class Abnahme
{
    Steg,        ///< die Kraft am Steg (wie der Fluegel)
    Magnetisch,  ///< Geschwindigkeit an den Tonabnehmern (Clavinet)
};

struct SaitenSpiel
{
    std::string name;
    int noteMin = 36;
    int noteMax = 96;
    Kurve laengeM;          ///< klingende Laenge (log)
    Kurve masseGm;          ///< g/m (log)
    Kurve inharmonizitaet;  ///< B (log)
    Kurve t60S;             ///< erste Stufe am ersten Teilton (log)
    Kurve t60SpaetS;        ///< zweite Stufe (log); kuerzer als t60S gilt als t60S
    Kurve abklingHoch;      ///< T60(10·f_1)/T60(f_1) (log)
    Kurve chor;             ///< Saiten je Chor 1 … 5 (linear, gerundet)
    Kurve verstimmungCent;  ///< aeussere Saiten des Chors (linear)
    Kurve polarisationWinkelGrad;  ///< γ (linear)
    Kurve polarisationCent;        ///< δ (linear)
    Kurve waagrechtVerhaeltnis;    ///< h (linear)
    Kurve anschlagStelle;   ///< x/L des klingenden Teils, von seinem Ende an der Tangente bzw. am Steg (linear)
    // --- Anregung ---
    Anregung anregung = Anregung::Schlaegel;
    Stossanreger schlaegel;
    double tempoMax = 4.0;      ///< m/s bei Stufe 0
    int ueberabtastung = 16;
    double tangenteWegMm = 1.0; ///< d: so weit drueckt die Tangente die Saite (offen)
    // --- Mittelsteg ---
    double stegTeilung = 0.0;   ///< L_a/L_ganz des angeschlagenen Abschnitts (0 = kein Mittelsteg)
    /// Kopplung der beiden Abschnitte am Mittelsteg als Anteil von σ_frueh des Teiltons (offen)
    double mittelstegKopplung = 0.5;
    /// Mitschwingen der anderen Choere (Deluxe, je Clip): an/aus und die Kopplung wie am Mittelsteg (offen)
    bool mitschwingen = false;
    double mitschwingKopplung = 0.5;
    // --- Daempfer, Clavinet ---
    Kurve daempferT60S;         ///< (log)
    bool daempfer = true;
    double ganzJeKlingend = 1.0;  ///< L_ganz/L_s (Clavinet 2^(3/12))
    Kurve garnT60S;             ///< T60 der ganzen Saite nach dem Loslassen (Garn) (log)
    Kurve schieberT60S;         ///< T60 unter dem Daempferschieber in Stellung 1 (log)
    double tastendruckGrenzeCent = 0.0;  ///< F-4-1 a: 0 bis zu einer Messung
    // --- Abnahme ---
    Abnahme abnahme = Abnahme::Steg;
    std::vector<double> abnehmerStellen;  ///< x/L_s der Tonabnehmer (vom Ende am Steg aus)
    /// Der Verstaerker ohne Klangschalter: Low-Shelf −3 dB bei 130 Hz, High-Shelf +3 dB bei 4 kHz (Gabrielli et al.
    /// 2013 S. 6, nachgelesen — als Shelf-Filter zweiter Ordnung mit S = 1 gelesen, offen); aus = ohne
    bool verstaerker = false;
    double ausgangBezug = 1.0;
    std::string kennzeichen = "ungemessen";
};

struct SaitenAnschlag
{
    int note = 60;
    double hz = 261.63;
    int stufe = 0;
    std::int64_t halten = 48000;   ///< so viele Abtastwerte bis zum Loslassen
    bool gedaempft = true;         ///< greift nach dem Loslassen ein Daempfer (Pedal nicht getreten)?
    double schieber = 0.0;         ///< Daempferschieber 0 … 1 (Clavinet, Passage-Spur)
    double tastendruckCent = 0.0;  ///< Pitch nach oben durch den Druck auf die Taste, gekappt an der Grenze
    bool mitschwingen = false;     ///< die anderen Choere schwingen mit (nur mit `SaitenSpiel::mitschwingen`; je Clip)
    AbnehmerWahl abnehmerWahl = AbnehmerWahl::Beide;  ///< Schalter der Tonabnehmer (Passage-Spur)
    int rate = 48000;
};

struct SaitenMessung
{
    double kontaktS = 0.0;
    double kraftSpitzeN = 0.0;
    double hzNachLoslassen = 0.0;  ///< Clavinet: der erste Teilton der ganzen Saite
};

/// So lange klingt der Ton nach dem Loslassen (−60 dB), hoechstens `maxFrames`.
[[nodiscard]] std::int64_t saitenNachklang(const SaitenSpiel& s, const SaitenAnschlag& a, std::int64_t maxFrames);

void rechneSaitenTon(const SaitenSpiel& s, const SaitenAnschlag& a, std::vector<float>& mono, std::int64_t frames,
                     SaitenMessung* messung = nullptr);

/// Eine Note eines Clips: ab `abS` angeschlagen.
struct SaitenClipNote
{
    double abS = 0.0;
    SaitenAnschlag anschlag;
};

/// Den Clip rechnen: je Note der Ton mit Mitschwingen, zeitversetzt summiert (linear — die Ueberlagerung ist exakt).
void rechneSaitenClip(const SaitenSpiel& s, const std::vector<SaitenClipNote>& noten, std::vector<float>& mono, std::int64_t frames);

/// Jede Kurve in ihre Grenzen.
void richteSaitenSpiel(SaitenSpiel& s, const SaitenSpiel& vorgabe);

[[nodiscard]] SaitenSpiel baueVorgabeClavinet();
[[nodiscard]] SaitenSpiel baueVorgabeHackbrett();
[[nodiscard]] SaitenSpiel baueVorgabeCimbalom();

}  // namespace lumi::kern::saite
