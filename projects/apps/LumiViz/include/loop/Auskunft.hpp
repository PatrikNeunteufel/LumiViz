/**
 * @file Auskunft.hpp
 * @brief Loop: was die Planung ueber ein Instrument erfaehrt — die Auskunft beim
 *        Namen (Entscheide Patrik S132: S-A1 a; S146: HT5 a)
 *
 * Die Planung (`plane`, `Loop::klangNamen`, Freeze, Export, Composer) kennt keine
 * Instrumente; sie bekommt eine Auskunft herein. Seit Stufe 31b (HT5 a) traegt sie
 * alles, was die Namen der Toene bestimmt: die Saiten eines Saiteninstruments und
 * ob eine Hammer-Saite klingt (dann tragen die Toene die Anschlagstufe, HK2 a).
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
    /// Klingt eine Hammer-Saite (eine eingeschaltete Schicht *Hammer* + *Modal*)?
    bool hammer = false;

    InstrumentAuskunft() = default;
    /// Eine Auskunft, die nur Saiten kennt — ohne Hammer.
    InstrumentAuskunft(Saitensatz saitenDesInstruments, bool mitHammer = false)  // NOLINT(google-explicit-constructor)
        : saiten(std::move(saitenDesInstruments)), hammer(mitHammer)
    {
    }

    [[nodiscard]] bool operator==(const InstrumentAuskunft&) const = default;
};

/// Die Auskunft beim Namen, den ein Muster nennt. Leer (keine Funktion) = jedes
/// Instrument gilt als eines ohne Saiten und ohne Hammer — alles wie bis Stufe 22.
using InstrumentVon = std::function<InstrumentAuskunft(const QString& instrument)>;

}  // namespace lumi::loop
