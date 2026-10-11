/**
 * @file Auswahl.hpp
 * @brief Loop: mehrere Schritte oder Noten eines Musters zugleich — kopieren,
 *        einfuegen, leeren, versetzen (docs/audio/Loop_Konzept.md §4i, Stufe 13)
 *
 * Reine Funktionen auf dem Modell, ohne Fenster: der Drum-Editor und der
 * Instrument-Editor rufen sie aus ihren Gesten, die Tests direkt.
 *
 * Entscheide Patrik S113:
 * - **V1 a:** Einfuegen im Drum-Editor **ersetzt** — wo die Kopie einen Schritt
 *   hat, steht danach einer; wo sie leer ist, ist es danach leer. Daneben
 *   gibt es das Dazulegen, das stehen laesst, was schon gesetzt ist.
 * - **V2 a:** es zaehlen genau die markierten Schritte, mit ihrer Lage
 *   zueinander. Was zwischen verstreuten Schritten liegt, bleibt unberuehrt.
 * - **V3 a:** stoesst beim Versetzen eines der Teile an einen Rand, bewegt
 *   sich nichts.
 *
 * Was beim Einfuegen ueber das Ende des Musters reicht, wird abgeschnitten —
 * und gezaehlt, damit der Editor es sagen kann.
 */
#pragma once

#include "loop/Loop.hpp"

#include <vector>

namespace lumi::loop
{

// --- Schritte eines Drumkit-Musters ------------------------------------------

/// Ein Schritt einer Spur.
struct SchrittOrt
{
    int spur = 0;
    int schritt = 0;

    [[nodiscard]] bool operator==(const SchrittOrt&) const = default;
};

/// Kopierte Schritte: jeder mit seiner Lage zum ersten (kleinste Spur,
/// kleinster Schritt der Auswahl) und seiner Lautstaerke — 0 = ein leerer
/// Schritt, der mitkopiert wurde.
struct SchrittKopie
{
    struct Teil
    {
        int spur = 0;     ///< Spuren unter der obersten der Auswahl
        int schritt = 0;  ///< Schritte hinter dem ersten der Auswahl
        double wert = 0.0;

        [[nodiscard]] bool operator==(const Teil&) const = default;
    };
    std::vector<Teil> teile;
    int spuren = 0;    ///< so viele Spuren ueberspannt die Kopie
    int schritte = 0;  ///< und so viele Schritte

    [[nodiscard]] bool leer() const { return teile.empty(); }
    [[nodiscard]] bool operator==(const SchrittKopie&) const = default;
};

/// Die oberste Spur und der erste Schritt der Auswahl — die Stelle, an der sie
/// „beginnt". Ohne Auswahl {0, 0}.
[[nodiscard]] SchrittOrt anfangVon(const std::vector<SchrittOrt>& orte);
/// Die genannten Schritte kopieren. Orte, die es im Muster nicht gibt, zaehlen nicht.
[[nodiscard]] SchrittKopie kopiereSchritte(const Muster& muster, const std::vector<SchrittOrt>& orte);
/// Die Kopie so einfuegen, dass ihr erster Teil auf `ziel` liegt. `ersetzen`
/// true (V1 a): jede Stelle der Kopie wird geschrieben, auch die leeren;
/// false: nur gesetzte Schritte, und nur wo das Ziel leer ist. Gibt zurueck,
/// wie viele **gesetzte** Schritte der Kopie keinen Platz hatten (ueber das
/// Ende des Musters oder unter der letzten Spur). `eingefuegt` nennt die
/// Stellen, die die Kopie am Ziel belegt.
int fuegeSchritteEin(Muster& muster, const SchrittKopie& kopie, SchrittOrt ziel, bool ersetzen,
                     std::vector<SchrittOrt>* eingefuegt = nullptr);
/// Die genannten Schritte ausschalten. false = es war keiner gesetzt.
bool leereSchritte(Muster& muster, const std::vector<SchrittOrt>& orte);
/// Die genannten Schritte um Spuren und Schritte versetzen: sie werden
/// herausgenommen und an der neuen Stelle geschrieben (was dort stand, ist
/// ersetzt). Stoesst einer an einen Rand, geschieht nichts (V3 a). true =
/// versetzt, `orte` nennt die neuen Stellen.
bool versetzeSchritte(Muster& muster, std::vector<SchrittOrt>& orte, int spurVersatz, int schrittVersatz);
/// Die genannten Schritte in der Zeit **im Kreis** versetzen (Wunsch Patrik
/// S113, neben V3 a als eigener Handgriff): was hinten ueber das Ende des
/// Musters geht, kommt vorn wieder herein. `schritteImMuster` ist die Laenge
/// des Musters. true = gedreht, `orte` nennt die neuen Stellen.
bool dreheSchritte(Muster& muster, std::vector<SchrittOrt>& orte, int schrittVersatz, int schritteImMuster);

// --- Noten eines Instrument-Musters ------------------------------------------

/// Eine Note, benannt nach Schritt und Tonhoehe — je Muster eindeutig.
struct NotenOrt
{
    int schritt = 0;
    int hoehe = 60;

    [[nodiscard]] bool operator==(const NotenOrt&) const = default;
};

/// Kopierte Noten: der Schritt jeder Note zaehlt ab dem ersten der Auswahl,
/// die Tonhoehe bleibt, wie sie war.
struct NotenKopie
{
    std::vector<Note> noten;
    int schritte = 0;  ///< vom ersten Einsatz bis zum letzten Ende

    [[nodiscard]] bool leer() const { return noten.empty(); }
    [[nodiscard]] bool operator==(const NotenKopie&) const = default;
};

/// Was beim Einfuegen von Noten nicht ganz passte.
struct NotenRest
{
    int weg = 0;       ///< Noten, die hinter dem Ende des Musters begonnen haetten
    int gekuerzt = 0;  ///< Noten, die am Ende des Musters abgeschnitten wurden
};

/// Der Platz der Note im Muster; -1 = es gibt sie nicht.
[[nodiscard]] int findeNote(const Muster& muster, NotenOrt ort);
[[nodiscard]] NotenKopie kopiereNoten(const Muster& muster, const std::vector<NotenOrt>& orte);
/// Die Kopie ab `schritt` einfuegen, in ihren Tonhoehen. Eine Note des
/// Musters auf demselben Schritt und derselben Tonhoehe wird ersetzt, alle
/// anderen bleiben. `schritteImMuster` ist die Laenge des Musters.
NotenRest fuegeNotenEin(Muster& muster, const NotenKopie& kopie, int schritt, int schritteImMuster,
                        std::vector<NotenOrt>* eingefuegt = nullptr);
bool loescheNoten(Muster& muster, const std::vector<NotenOrt>& orte);
/// Die genannten Noten in Zeit und Tonhoehe versetzen. Stoesst eine an einen
/// Rand — Anfang oder Ende des Musters, `kHoeheMin`, `kHoeheMax` —, geschieht
/// nichts (V3 a). Eine nicht markierte Note auf demselben Schritt und
/// derselben Tonhoehe wird ersetzt.
bool versetzeNoten(Muster& muster, std::vector<NotenOrt>& orte, int schrittVersatz, int hoeheVersatz,
                   int schritteImMuster);
/// Die genannten Noten in der Zeit im Kreis versetzen, wie `dreheSchritte`.
/// Eine Note, die danach ueber das Ende des Musters reichte, wird dort
/// gekuerzt (eine Note endet spaetestens mit ihrem Muster); `gekuerzt` zaehlt
/// sie. true = gedreht.
bool dreheNoten(Muster& muster, std::vector<NotenOrt>& orte, int schrittVersatz, int schritteImMuster,
                int* gekuerzt = nullptr);

}  // namespace lumi::loop
