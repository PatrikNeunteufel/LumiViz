/**
 ****************************************************************************************
 * @file   FluegelSchicht.hpp
 * @brief  Tests: eine Kern-Schicht des Fluegels bauen und lesen (Stufe 34d, IK2-4 a, FU1 a, FU2 a)
 *
 * @details
 * Bis S175 trug eine Schicht `hammerSaite` (mit dem Boden darin) und die Tastatur `ungedaempftAb`. Seit 34d ist der
 * Fluegel der Kern `hammerString`: die Werte stehen in `Schicht::kernWerte` (Tafel des Kerns), der Koerper in
 * `Schicht::koerper`. Diese Hilfen bauen eine solche Schicht aus einer `HammerSaite` und lesen sie zurueck — ein Test,
 * der die Saite aendert, schreibt sie mit `setzeSaite` zurueck.
 ****************************************************************************************
 */
#pragma once

#include "InstrumentModels/hammer/HammerKern.hpp"
#include "loop/Instrument.hpp"

#include <QString>

namespace fluegeltest
{

using lumi::kern::hammer::HammerSaite;
using lumi::kern::hammer::Pedalwerte;

/// Die Pedalwerte einer Kern-Schicht des Fluegels (Vorgaben, wenn ein Wert fehlt).
inline Pedalwerte pedaleVon(const lumi::loop::Schicht& s)
{
    return lumi::kern::hammer::pedalwerteAus(s.kernWerte);
}

/// Die Hammer-Saite einer Kern-Schicht des Fluegels.
inline HammerSaite saiteVon(const lumi::loop::Schicht& s)
{
    return lumi::kern::hammer::saiteAus(s.kernWerte);
}

/// Die Hammer-Saite in die Schicht schreiben (die Pedalwerte bleiben), gerichtet.
inline void setzeSaite(lumi::loop::Schicht& s, const HammerSaite& saite)
{
    s.kernName = QString::fromUtf8(lumi::kern::hammer::kKernName.data(), static_cast<qsizetype>(lumi::kern::hammer::kKernName.size()));
    s.kernWerte = lumi::kern::hammer::werteAus(saite, pedaleVon(s));
    lumi::loop::richteSchicht(s);
}

/// Die erste Taste ohne Daempfer setzen (bis 34d `Tastatur::ungedaempftAb`, seit FU2 a in der Tafel).
inline void setzeUngedaempftAb(lumi::loop::Schicht& s, int taste)
{
    Pedalwerte p = pedaleVon(s);
    p.ungedaempftAb = taste;
    s.kernWerte = lumi::kern::hammer::werteAus(saiteVon(s), p);
    lumi::loop::richteSchicht(s);
}

/// Eine Kern-Schicht des Fluegels: die Saite (Vorgabe ohne Angabe), der Koerper (leer = ohne), die erste Taste ohne
/// Daempfer.
inline lumi::loop::Schicht fluegelSchicht(const HammerSaite& saite = lumi::kern::hammer::hammerSaiteVorgabe(),
                                          const QString& koerper = {},
                                          int ungedaempftAb = lumi::kern::hammer::kUngedaempftAbVorgabe)
{
    lumi::loop::Schicht s;
    s.koerper = koerper;
    Pedalwerte p;
    p.ungedaempftAb = ungedaempftAb;
    s.kernName = QString::fromUtf8(lumi::kern::hammer::kKernName.data(), static_cast<qsizetype>(lumi::kern::hammer::kKernName.size()));
    s.kernWerte = lumi::kern::hammer::werteAus(saite, p);
    lumi::loop::richteSchicht(s);
    return s;
}

}  // namespace fluegeltest
