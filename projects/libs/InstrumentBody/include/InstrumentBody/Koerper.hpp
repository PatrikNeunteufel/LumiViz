/**
 * @file Koerper.hpp
 * @brief Ein gerechneter Instrumentkoerper: was die Datei `.lvinstbody` traegt
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S146–S148: KB1 a, BM3 a,
 *        BM5 a, PL31c-5 a, ST1 a; Stufe 31c-5d)
 *
 * Die gerechneten Moden (je Mode Frequenz, Verlustfaktor, Wert der Form je Stegpunkt,
 * Abstrahlgewicht je Hoerort) und darueber der statistische Teil (seine Groessen und der Samen;
 * gezogen wird beim Bilden der Antworten). Dazu, woraus und wie der Koerper entstand: der
 * Abdruck des Bauplans und die Fassung der Rechnung — ein Test meldet einen Koerper, der nicht
 * mehr zu seinem Bauplan oder zur Rechnung passt (BM3 a, Muster des Waechters der Klangfassung).
 *
 * Lesen und schreiben mit Qt-JSON in der App (`loop/Koerper`); hier nur die Daten.
 */
#pragma once

#include "InstrumentBody/Auswertung.hpp"
#include "InstrumentBody/Statistik.hpp"

#include <string>
#include <vector>

namespace lumi::koerper
{

/// Die Fassung der Datei `.lvinstbody`, die dieses Programm schreibt; eine hoehere liest es nicht.
/// Fassung 2 (S149, DM1 a): die Werkstoffe mit ihrem Verlustfaktor, je Mode die Anteile der
/// Verformungsenergie je Werkstoff, im statistischen Teil deren Mittel. Fassung 1 bleibt lesbar.
/// Fassung 3 (S196, F-K1 a): je Mode `airShare`, der Anteil der Bewegungsenergie in der Luft der
/// Loecher (immer geschrieben, 0 = ohne Hohlraum) — damit laesst sich die Daempfung einer gekoppelten
/// Mode nachrechnen: η = (1 − Anteil) · η der Platte + Anteil / Guete der Luft. Fassung 2 bleibt lesbar
/// (Anteil 0).
/// Fassung 4 (S199, KQ25 a): `origin` — woraus der Koerper stammt, `plan` (gerechnet aus einem Bauplan) oder
/// `measurement` (Modalfit an einer Messung, etwa die Gitarre nach dem Archiv Mores 2021); dazu `measurement` mit
/// Datei, Abdruck und Fassung des Fits. Beides immer geschrieben. Fassung 3 liest sich als `plan`.
/// Fassung 5 (S206, KQ35-2 a): die Richtung am Steg — je Mode `bridgeAngle` (Winkel gegen die Senkrechte der Decke,
/// Grad; `bridge` ist der Formwert laengs dieser Richtung), im statistischen Teil `bridgeAngle` (Betrag fuer die
/// gezogenen Moden, Vorzeichen gezogen) und `parallelFactor` (KQ35-1 a: Moden nur parallel zur Decke). Alles immer
/// geschrieben; ein Koerper aus einem Bauplan schreibt 0. Fassung 4 liest sich mit 0.
/// Fassung 6 (S211, KQ53 a): je Mode `springShare`, der Anteil der Verformungsenergie in den Federn an Punkten
/// (KQ49 a; immer geschrieben, 0 = ohne Federn) — damit laesst sich die Daempfung eines Koerpers mit Federn
/// nachrechnen. Fassung 5 liest sich mit 0.
inline constexpr int kKoerperFassung = 6;

/// Die Fassung des Modalfits fuer gemessene Koerper (`asset/loop/instrument_bodies/make_gitarre.py` traegt dieselbe
/// Zahl). Hochzaehlen, wenn sich bei gleicher Messung und gleicher Beschreibung ein anderer Koerper ergaebe.
/// 2 (KQ26 a, S199): ueber den gefitteten Moden ein statistischer Teil, seine Groessen an der Messung bestimmt.
/// 3 (KQ28 a, S201): die Form je Mode laengs des Stegs an Steg- und Schallkurven gemeinsam gestellt.
/// 4 (KQ35-1 a, KQ35-2 a, S206): die Modendichte 2,2–5 kHz und darueber nach Elie 2011 aufgefuellt, die Richtung am
/// Steg je Mode nach Elie 2012 und je Mode eine zweite nur parallel zur Decke.
inline constexpr int kFitFassung = 4;

/// Woraus ein Koerper stammt (KQ25 a).
enum class Herkunft
{
    Bauplan,  ///< gerechnet: `bauplan`, `bauplanAbdruck`, `rechnung.fassung` gelten
    Messung,  ///< Modalfit an einer Messung: `messung`, `messungAbdruck`, `fitFassung` gelten
};

/// Die Fassung der Rechnung (Netz, Platte, Loeser, Auswertung, statistischer Teil). Hochzaehlen,
/// wenn sich bei gleichem Bauplan ein anderer Koerper ergaebe — dann meldet der Waechter jeden
/// mitgelieferten Koerper als veraltet, bis das Werkzeug ihn neu rechnet.
/// 2 (S149, DM1 a): Daempfung je Werkstoff nach der Verformungsenergie.
inline constexpr int kRechenFassung = 2;

/// Wie ein Koerper gerechnet wurde.
struct Rechnung
{
    int fassung = 0;               ///< kRechenFassung beim Rechnen
    std::string netzgeber;         ///< Name der Umsetzung
    std::string loeser;            ///< Name der Umsetzung
    int ordnung = 0;               ///< k der Elemente
    double netzFuerHz = 0.0;       ///< die Elementgroesse nach MAESSTRO Gl. 1 fuer diese Frequenz
    double vonHz = 0.0;            ///< das Fenster der gerechneten Moden
    double bisHz = 0.0;
    int freieGroessen = 0;         ///< zur Auskunft
    double sekunden = 0.0;         ///< zur Auskunft

    [[nodiscard]] bool operator==(const Rechnung&) const = default;
};

struct Koerper
{
    std::string titel;
    std::string beschreibung;
    std::string bauplan;           ///< der Name der Bauplan-Datei, ohne Ordner
    std::string bauplanAbdruck;    ///< SHA-256 des Bauplans (wie `loop/Koerper` ihn bildet)
    Herkunft herkunft = Herkunft::Bauplan;
    std::string messung;           ///< Herkunft Messung: der Name der Beschreibung `.lvinstbodyfit`, ohne Ordner
    std::string messungAbdruck;    ///< SHA-256 der Beschreibung (`loop/Koerper`: `messungAbdruck`)
    int fitFassung = 0;            ///< kFitFassung beim Fit; 0 bei Herkunft Bauplan
    Rechnung rechnung;
    std::vector<int> stegTasten;   ///< die Stegpunkte in der Reihenfolge der Werte je Mode (MIDI)
    std::vector<std::string> hoerorte;  ///< die Hoerorte in der Reihenfolge der Gewichte je Mode
    Luft luft;
    Daempfung daempfung;
    std::vector<std::string> werkstoffe;    ///< Reihenfolge der Anteile je Mode (DM1 a)
    std::vector<double> werkstoffVerlust;   ///< je Werkstoff sein Verlustfaktor, 0 = das Gesetz
    std::vector<ModeImKoerper> moden;   ///< aufsteigend
    StatistischerTeil statistik;        ///< dichte = 0: keiner

    [[nodiscard]] bool operator==(const Koerper&) const = default;
};

}  // namespace lumi::koerper
