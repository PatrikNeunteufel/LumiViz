/**
 ****************************************************************************************
 * @file   ModellMessung.hpp
 * @brief  Stufe 31d: den gerechneten Fluegel an den Tasten von Salamander messen — wie im
 *         Programm (`rendereTon`), mit derselben Messung (`Fluegelmessung.hpp`, UF3 a)
 *
 * @details
 * Gemeinsam fuer den Vergleich (31d-3 Teil 2) und das Setzen (31d-4, S1 a). Die Anschlagstufen
 * −30 … 0 dB in Schritten von 2 stehen an der Stelle der Ebenen v1 … v16 (V1 a). Gehalten wird
 * 12 s (bis C5) bzw. 8 s: gemessen wird vor dem Loslassen. Acht Threads ueber die Tasten.
 ****************************************************************************************
 */

#pragma once

#include "Fluegelmessung.hpp"

#include "loop/Boden.hpp"
#include "loop/Fft.hpp"
#include "loop/HammerTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Loop.hpp"

#include <QString>

#include <algorithm>
#include <array>
#include <atomic>
#include <complex>
#include <thread>
#include <vector>

namespace modelltest
{

inline constexpr int kRate = 48000;

/// Die Stufe an der Stelle der Ebene e (1 … 16).
inline int stufeVonEbene(int e)
{
    return -30 + 2 * (e - 1);
}

/// Der Fluegel des Modells: Hammer-Saite (Modal), Tastatur wie die Vorgabe, Boden `koerper`
/// (`lib:<name>`); `saite` ersetzt die Hammer-Saite, wenn gegeben.
inline lumi::loop::Instrument fluegel(const QString& koerper, const lumi::loop::HammerSaite* saite = nullptr)
{
    lumi::loop::Instrument instrument;
    lumi::loop::Schicht& s = instrument.schichten[0];
    s.schwingung = lumi::loop::Schwingung::Saite;
    s.saitenModell = lumi::loop::SaitenModell::Modal;
    s.anregung = lumi::loop::Anregung::Hammer;
    instrument.tastatur = lumi::loop::Tastatur{};
    if (saite != nullptr) s.hammerSaite = *saite;
    s.hammerSaite.resonanzboden.koerper = koerper;
    return instrument;
}

/// Ein Ton des Modells, wie im Programm.
inline lumi::loop::Klang ton(const lumi::loop::Instrument& instrument, int midi, int stufe)
{
    lumi::loop::TonAngabe angabe;
    angabe.hoehe = midi;
    angabe.dauerMs = midi <= 72 ? 12000 : 8000;
    angabe.anschlagStufe = stufe;
    return lumi::loop::rendereTon(instrument, angabe, kRate);
}

/// Ein Ton gemessen (Phantome nur im Bass bei voller Staerke).
inline std::array<lumi::messung::KanalMessung, 2> messe(const lumi::loop::Instrument& instrument, int midi, int stufe)
{
    const bool phantome = midi <= lumi::messung::kPhantomBis && stufe == 0;
    return lumi::messung::messeTon(ton(instrument, midi, stufe).daten, kRate, lumi::messung::tastenFrequenz(midi), lumi::loop::fft(),
                                   phantome);
}

/// Alle Tasten mit allen 16 Stufen gemessen, je Taste wie die Ebenen von Salamander.
inline std::vector<std::vector<std::array<lumi::messung::KanalMessung, 2>>> messeTasten(const lumi::loop::Instrument& instrument,
                                                                                        const std::vector<int>& tasten)
{
    std::vector<std::vector<std::array<lumi::messung::KanalMessung, 2>>> messungen(tasten.size());
    std::atomic<std::size_t> naechste{0};
    const unsigned zahl = std::max(1u, std::min(8u, std::thread::hardware_concurrency()));
    std::vector<std::thread> arbeiter;
    for (unsigned a = 0; a < zahl; ++a)
        arbeiter.emplace_back([&] {
            for (std::size_t i = naechste++; i < tasten.size(); i = naechste++)
                for (int e = 1; e <= lumi::messung::kEbenen; ++e) messungen[i].push_back(messe(instrument, tasten[i], stufeVonEbene(e)));
        });
    for (std::thread& t : arbeiter) t.join();
    return messungen;
}

/// Die Summe der Moden als Ton (SZ3 a): je Teilton bis `teiltoeneMax` die Antwort am Steg auf einen Stoss,
/// Σ Im(g·e^(λt)) / k — ohne Hammerkontakt, Laengskraft und Boden; in float gerundet wie der Ton.
inline std::vector<double> modenTon(const lumi::loop::HammerSaite& saite, int midi, int stufe, double dauerS, int teiltoeneMax)
{
    lumi::loop::HammerAnschlag a;
    a.taste = midi;
    a.hz = lumi::messung::tastenFrequenz(midi);
    a.stufe = stufe;
    a.rate = kRate;
    std::vector<double> ton(static_cast<std::size_t>(dauerS * kRate), 0.0);
    const double dt = 1.0 / kRate;
    for (const lumi::loop::TeiltonModen& t : lumi::loop::hammerTeiltonModen(saite, a))
    {
        if (t.teilton > teiltoeneMax) break;
        for (const lumi::loop::EbenenMode& m : t.moden)
        {
            std::complex<double> z = m.anteil / static_cast<double>(t.teilton);
            const std::complex<double> schritt = std::exp(m.lambda * dt);
            for (double& x : ton)
            {
                x += z.imag();
                z *= schritt;
            }
        }
    }
    for (double& x : ton) x = static_cast<float>(x);
    return ton;
}

/// Die Summe der Moden gemessen und verdichtet wie eine Taste von Salamander: die eine Messung steht fuer alle 16
/// Ebenen (die Moden haengen an der Stufe nur ueber den Stoss — in Stufe 1 fest, SM1 a).
inline lumi::messung::TastenWerte modenWerte(const lumi::loop::HammerSaite& saite, int midi, int stufe, int teiltoeneMax = 30)
{
    const double dauerS = midi <= 72 ? 12.0 : 8.0;
    const std::vector<double> ton = modenTon(saite, midi, stufe, dauerS, teiltoeneMax);
    const lumi::messung::KanalMessung m =
        lumi::messung::messeKanal(ton, kRate, lumi::messung::tastenFrequenz(midi), lumi::loop::fft(), false);
    const std::vector<std::array<lumi::messung::KanalMessung, 2>> ebenen(lumi::messung::kEbenen, {m, m});
    return lumi::messung::verdichte(midi, ebenen);
}

/// Verdichtet je Taste, mit den Pegeln gegen das Mittel der Tastatur.
inline std::vector<lumi::messung::TastenWerte> verdichteAlle(const std::vector<int>& tasten,
                                                             const std::vector<std::vector<std::array<lumi::messung::KanalMessung, 2>>>& m)
{
    std::vector<lumi::messung::TastenWerte> werte;
    for (std::size_t i = 0; i < tasten.size(); ++i) werte.push_back(lumi::messung::verdichte(tasten[i], m[i]));
    lumi::messung::setzeRelativePegel(werte, m);
    return werte;
}

}  // namespace modelltest
