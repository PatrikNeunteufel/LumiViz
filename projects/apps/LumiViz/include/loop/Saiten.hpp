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

#include <vector>

namespace lumi::loop
{

/// So viele Saiten traegt ein Instrument hoechstens (eine Zwoelfsaitige).
inline constexpr int kSaitenMax = 12;
/// So weit kann das Capo hinauf, in Buenden.
inline constexpr int kCapoMax = 12;

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

    [[nodiscard]] bool istSaiteninstrument() const { return !leer.empty(); }
    [[nodiscard]] int anzahl() const { return static_cast<int>(leer.size()); }

    [[nodiscard]] bool operator==(const Saitensatz&) const = default;
};

/// Werte in ihre Grenzen: hoechstens `kSaitenMax` Saiten (was dahinter steht,
/// faellt weg), jeder Ton in 0 … 127, das Capo in 0 … `kCapoMax`.
void richteSaiten(Saitensatz& saiten);

/// Eine Stimmung, wie sie im Creator unter *Tuning* zur Wahl steht (S-U1 a).
struct Stimmungsvorlage
{
    const char* schluessel;  ///< `guitar`, `dropD` …
    const char* titel;       ///< wie es im Programm steht: „Guitar standard"
    std::vector<int> leer;   ///< die Toene der leeren Saiten, wie `Saitensatz::leer`
};

/// Die Vorlagen in der Reihenfolge der Auswahl — ohne *None* und *Custom*.
[[nodiscard]] const std::vector<Stimmungsvorlage>& stimmungsvorlagen();
/// Die Vorlage, deren Toene `saiten` traegt (das Capo zaehlt nicht) — oder
/// nullptr: kein Saiteninstrument oder eine eigene Stimmung (*Custom*). Die
/// Vorlage steht nicht in der Datei; sie folgt aus den Toenen.
[[nodiscard]] const Stimmungsvorlage* vorlageVon(const Saitensatz& saiten);

}  // namespace lumi::loop
