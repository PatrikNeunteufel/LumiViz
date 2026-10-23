/**
 * @file Eigenloeser.hpp
 * @brief Die Schnittstelle des Eigenloesers: K φ = λ M φ in einem Frequenzfenster
 *        (docs/audio/Spielweisen_Konzept.md §4.1; Entscheide Patrik S147/S148: BM5-1 a,
 *        EG1 a; Stufe 31c-4)
 *
 * Hinein K und M als `DuenneMatrix` (symmetrisch, M positiv definit), ein Fenster in Hz;
 * heraus jede Mode im Fenster, aufsteigend, an einen Empfaenger (EG1 a) — der Loeser
 * teilt das Fenster selbst und weiss nicht, was mit einer Form geschieht. Jede Umsetzung
 * zaehlt die Eigenwerte im Fenster nach dem Traegheitssatz von Sylvester (`imFenster`),
 * damit die Vollstaendigkeit auch dort pruefbar ist, wo es keine geschlossene Loesung gibt.
 *
 * Kein Typ einer fremden Bibliothek geht ueber diese Schnittstelle (BM5-1 a); die
 * Umsetzungen stehen in eigenen Dateien (`EigenloeserSpectra.hpp`).
 */
#pragma once

#include "InstrumentBody/DuenneMatrix.hpp"

#include <cmath>
#include <functional>
#include <numbers>
#include <string>
#include <vector>

namespace lumi::koerper
{

/// Eine Mode: die Frequenz und die Form, bezueglich M normiert (φᵀMφ = 1), nummeriert wie
/// K und M (bei der Platte nach `PlattenSystem::frei`). Das Vorzeichen der Form ist frei.
struct Mode
{
    double frequenzHz = 0.0;
    std::vector<double> form;
};

/// Was der Loeser rechnen soll.
struct EigenAuftrag
{
    const DuenneMatrix* k = nullptr;
    const DuenneMatrix* m = nullptr;
    double vonHz = 0.0;   ///< das Fenster [vonHz, bisHz); 0 = von unten (dann K positiv definit)
    double bisHz = 0.0;
    double toleranz = 1e-9;                      ///< relativ, auf den Eigenwert
    std::function<void(const Mode&)> jeMode;      ///< aufsteigend; leer = nur Frequenzen
    std::function<bool(double anteil)> weiter;    ///< Fortschritt 0 … 1; false = abbrechen; leer = weiter
};

/// Was herauskommt.
struct EigenErgebnis
{
    std::vector<double> frequenzenHz;  ///< aufsteigend, alle im Fenster
    int imFenster = -1;                ///< Zahl der Eigenwerte im Fenster, gezaehlt nach Sylvester
    int faktorisierungen = 0;          ///< fuer den Vergleich alt gegen neu
    double groessterRest = 0.0;        ///< max ‖Kφ − λMφ‖ / ‖λMφ‖ ueber die gelieferten Moden
    double sekunden = 0.0;
    double sekundenZerlegung = 0.0;    ///< davon in Zerlegungen (soweit die Umsetzung zerlegt)
    std::string fehler;                ///< leer = in Ordnung
};

/// Die Schnittstelle (EG1 a). Eine Umsetzung ist zustandslos: zwei Aufrufe mit demselben
/// Auftrag geben dieselben Frequenzen.
class Eigenloeser
{
public:
    virtual ~Eigenloeser() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual EigenErgebnis loese(const EigenAuftrag& auftrag) const = 0;
};

/// λ = ω² zu einer Frequenz, und zurueck.
[[nodiscard]] inline double eigenwertVon(double frequenzHz)
{
    const double w = 2.0 * std::numbers::pi * frequenzHz;
    return w * w;
}

[[nodiscard]] inline double frequenzVon(double eigenwert)
{
    return std::sqrt(eigenwert > 0.0 ? eigenwert : 0.0) / (2.0 * std::numbers::pi);
}

/// Was ein Auftrag mindestens braucht; leer = in Ordnung.
[[nodiscard]] inline std::string pruefeAuftrag(const EigenAuftrag& a)
{
    if (!a.k || !a.m) return "stiffness and mass matrix are needed";
    if (a.k->n <= 0 || a.k->n != a.m->n) return "stiffness and mass matrix need the same, positive size";
    if (!(a.bisHz > a.vonHz) || a.vonHz < 0.0) return "the window needs 0 <= from < to";
    if (!(a.toleranz > 0.0)) return "the tolerance must be positive";
    return {};
}

}  // namespace lumi::koerper
