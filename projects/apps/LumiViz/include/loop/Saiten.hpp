/**
 * @file Saiten.hpp
 * @brief Loop: die Saiten eines Saiteninstruments — Stimmung, Capo und die
 *        Regel, nach der eine Note ihre Saite bekommt
 *        (docs/audio/Saiten_Konzept.md, Stufe 23a; Entscheide Patrik S132:
 *        S-M1 a, S-M2 a, S-M2b, S-M2c a)
 *
 * Die Saiten gehoeren dem **Instrument** (S-M1 a): ein `Saitensatz` traegt je
 * Saite den Ton der leeren Saite und das Capo. Ein Instrument ohne Saiten ist
 * wie bis Stufe 22. Eine Note kann ihre Saite tragen (S-M2 a); traegt sie
 * keine, waehlt das Programm sie nach der Regel des Musters (S-M2b, S-M2c a).
 *
 * Hier steht nur das Modell — welche Saite eine Note bekommt, rechnet erst
 * Stufe 23b. Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include <QLatin1String>
#include <QString>

#include <functional>
#include <vector>

namespace lumi::loop
{

/// So viele Saiten traegt ein Instrument hoechstens (eine Zwoelfsaitige).
inline constexpr int kSaitenMax = 12;
/// So weit kann das Capo hinauf, in Buenden.
inline constexpr int kCapoMax = 12;
/// So viele Buende ueber der leeren Saite (mit Capo) ist ein Ton noch greifbar
/// (Stufe 23b, von Claude gesetzt: eine Gitarre mit 24 Buenden).
inline constexpr int kBuendeMax = 24;

/// Die Regel fuer eine Note ohne eigene Saite (S-M2b, beide schaltbar).
enum class Saitenwahl : int
{
    /// *Per chord*: Noten, die zugleich klingen, auf verschiedenen Saiten mit
    /// dem kleinsten Abstand der Buende; bei Gleichstand die tiefste Lage.
    /// Ohne Gedaechtnis.
    JeKlang = 0,
    /// *Follow hand*: wie JeKlang, unter mehreren Moeglichkeiten aber die
    /// naechste zur Lage des vorigen Klangs.
    Hand = 1,
};

/// Wie die Regel in einer Datei steht: `perChord`, `followHand`.
[[nodiscard]] inline QLatin1String saitenwahlName(Saitenwahl wahl)
{
    return QLatin1String(wahl == Saitenwahl::Hand ? "followHand" : "perChord");
}
/// Fehlt die Regel oder ist sie unbekannt, gilt *Per chord*.
[[nodiscard]] inline Saitenwahl saitenwahlAusName(const QString& name)
{
    return name == QLatin1String("followHand") ? Saitenwahl::Hand : Saitenwahl::JeKlang;
}

/// Die Saiten eines Instruments.
struct Saitensatz
{
    /// Je Saite der Ton der leeren Saite (MIDI-Nummer, 0 … 127), in der
    /// Reihenfolge am Instrument — bei der Gitarre von der tiefen E-Saite an
    /// (`E2 A2 D3 G3 B3 E4`), bei der Ukulele `G4 C4 E4 A4` (die G-Saite ist
    /// hoch). Die Stelle in der Liste ist die Saite einer Note (`Note::saite`).
    /// Leer = kein Saiteninstrument.
    std::vector<int> leer;
    int capo = 0;  ///< 0 … `kCapoMax`: so viele Buende hoeher klingt jede leere Saite
    /// Die Regel, die ein neues Muster dieses Instruments bekommt (S-M2c a).
    Saitenwahl wahl = Saitenwahl::JeKlang;
    /// Scale: die Mensur, die schwingende Laenge vom Sattel zum Steg, in mm
    /// (Stufe 23d, Entscheid S-U3 a); 0 = nicht gesetzt — dann gilt
    /// `kMensurVorgabeMm`. Lagen am Instrument (Tonabnehmer) stehen in mm vom
    /// Steg und werden mit ihr zum Anteil der Saite.
    double mensurMm = 0.0;

    [[nodiscard]] bool istSaiteninstrument() const { return !leer.empty(); }
    [[nodiscard]] int anzahl() const { return static_cast<int>(leer.size()); }

    [[nodiscard]] bool operator==(const Saitensatz&) const = default;
};

/// Die Grenzen der Mensur, in mm; und was gilt, solange keine gesetzt ist
/// (von Claude gesetzt: eine Gitarre mit 648 mm, bis die Vorlagen ihre eigene
/// mit Quelle tragen — S-U3 a).
inline constexpr double kMensurMinMm = 100.0;
inline constexpr double kMensurMaxMm = 1200.0;
inline constexpr double kMensurVorgabeMm = 648.0;
/// Die Mensur, mit der gerechnet wird: die gesetzte oder die Vorgabe.
[[nodiscard]] inline double mensurVon(const Saitensatz& saiten)
{
    return saiten.mensurMm > 0.0 ? saiten.mensurMm : kMensurVorgabeMm;
}

/// Der Bund, in dem `hoehe` auf `saite` liegt — gezaehlt ab dem Capo (0 = die
/// leere Saite mit Capo). Greifbar ist er in 0 … `kBuendeMax`.
[[nodiscard]] inline int bundAuf(const Saitensatz& saiten, int saite, int hoehe)
{
    return hoehe - saiten.leer[static_cast<std::size_t>(saite)] - saiten.capo;
}
[[nodiscard]] inline bool greifbarAuf(const Saitensatz& saiten, int saite, int hoehe)
{
    if (saite < 0 || saite >= saiten.anzahl()) return false;
    const int bund = bundAuf(saiten, saite, hoehe);
    return bund >= 0 && bund <= kBuendeMax;
}

/// Die Auskunft, die die Planung herein bekommt (Entscheid S-A1 a, S132): die
/// Saiten des Instruments, das ein Muster nennt. Leer (keine Funktion) = jedes
/// Instrument gilt als eines ohne Saiten — alles wie bis Stufe 22. Wer
/// Instrumente kennt, liefert sie (`saitenAusInstrumenten`, Instrument.hpp).
using SaitenVon = std::function<Saitensatz(const QString& instrument)>;

/// Werte in ihre Grenzen: hoechstens `kSaitenMax` Saiten (was dahinter steht,
/// faellt weg), jeder Ton in 0 … 127, das Capo in 0 … `kCapoMax`.
void richteSaiten(Saitensatz& saiten);

/// Eine Stimmung, wie sie im Creator unter *Tuning* zur Wahl steht (S-U1 a).
struct Stimmungsvorlage
{
    const char* schluessel;  ///< `guitar`, `dropD` …
    const char* titel;       ///< wie es im Programm steht: „Guitar standard"
    std::vector<int> leer;   ///< die Toene der leeren Saiten, wie `Saitensatz::leer`
    /// Die uebliche Mensur in mm (S-U3 a, S-K7 a; Belege in
    /// docs/audio/Saiten_Konzept.md §6b) — wer die Vorlage waehlt, bekommt sie.
    double mensurMm = 0.0;
};

/// Die Vorlagen in der Reihenfolge der Auswahl — ohne *None* und *Custom*.
[[nodiscard]] const std::vector<Stimmungsvorlage>& stimmungsvorlagen();
/// Die Vorlage, deren Toene `saiten` traegt (das Capo zaehlt nicht) — oder
/// nullptr: kein Saiteninstrument oder eine eigene Stimmung (*Custom*). Die
/// Vorlage steht nicht in der Datei; sie folgt aus den Toenen.
[[nodiscard]] const Stimmungsvorlage* vorlageVon(const Saitensatz& saiten);

}  // namespace lumi::loop
