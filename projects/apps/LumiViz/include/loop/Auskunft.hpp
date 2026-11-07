/**
 * @file Auskunft.hpp
 * @brief Loop: was die Planung ueber ein Instrument erfaehrt — die Auskunft beim
 *        Namen (Entscheide Patrik S132: S-A1 a; S146: HT5 a)
 *
 * Die Planung (`plane`, `Loop::klangNamen`, Freeze, Export, Composer) kennt keine
 * Instrumente; sie bekommt eine Auskunft herein. Seit Stufe 31b (HT5 a) traegt sie
 * alles, was die Namen der Toene bestimmt: die Saiten eines Saiteninstruments und
 * ob die Toene die Anschlagstufe tragen (seit 34d aus der Auskunft des Kerns, HK2 a).
 * Wer Instrumente kennt, liefert sie (`instrumentAuskunft`, Instrument.hpp).
 *
 * Kein Qt-Widget, keine Audio-Engine.
 */
#pragma once

#include "loop/Saiten.hpp"

#include <QString>

#include <functional>

namespace lumi::loop
{

/// Was die Planung ueber ein Instrument wissen muss.
struct InstrumentAuskunft
{
    /// Die Saiten; leer = kein Saiteninstrument.
    Saitensatz saiten;
    /// Tragen die Toene die Anschlagstufe im Namen (`:a<stufe>`, HK2 a)? Ja, wenn eine eingeschaltete Kern-Schicht
    /// einen Kern traegt, dessen Auskunft es sagt (`kern::KernAuskunft::stufeImNamen`, 34d; bis dahin `hammer`).
    bool stufeImNamen = false;

    InstrumentAuskunft() = default;
    /// Eine Auskunft, die nur Saiten kennt — ohne Stufe im Namen.
    InstrumentAuskunft(Saitensatz saitenDesInstruments, bool mitStufe = false)  // NOLINT(google-explicit-constructor)
        : saiten(std::move(saitenDesInstruments)), stufeImNamen(mitStufe)
    {
    }

    [[nodiscard]] bool operator==(const InstrumentAuskunft&) const = default;
};

/// Die Auskunft beim Namen, den ein Muster nennt. Leer (keine Funktion) = jedes
/// Instrument gilt als eines ohne Saiten und ohne Hammer — alles wie bis Stufe 22.
using InstrumentVon = std::function<InstrumentAuskunft(const QString& instrument)>;

}  // namespace lumi::loop
