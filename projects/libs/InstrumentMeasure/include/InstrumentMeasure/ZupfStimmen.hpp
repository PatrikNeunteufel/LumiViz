/**
 * @file ZupfStimmen.hpp
 * @brief Messen am Kern `pluckedString` (Klasse 2): die Tonhoehe einer gezupften Saite (Instrumente_IC_Konzept.md 34k-2,
 *        IK2-2 a; der Stimmer folgt mit 34k-3)
 *
 * Uebernommen aus IC `rechnung/klassen/02_zupf_saite/Zupf.cpp` (`4409088`), der Messteil von `rechneZupfTon` wortgleich:
 * IC mass im Kern, hier misst `InstrumentMeasure` an der Summe des Kerns (`rechneZupfSumme`, vor Bezug und Ausblende —
 * dieselben Zahlen, an denen IC mass). Die Tonhoehe ist die staerkste Spitze im Spektrum innerhalb ±60 Cent um die Note,
 * Hann-Fenster 0,1 … 0,5 s, Raster 0,5 Cent („YIN sprang bei schwachem Grundton auf die Oktave“, IC).
 *
 * Ohne Qt.
 */
#pragma once

#include "InstrumentModels/zupf/Zupf.hpp"

#include <cstdint>
#include <span>

namespace lumi::messung
{

/// Was IC `ZupfMessung` am Ton einer Note mass.
struct ZupfTon
{
    double spielHz = 0.0;   ///< die staerkste Spitze innerhalb ±60 Cent um die Note (0,1 … 0,5 s)
    bool schwingt = false;  ///< eine Spitze ueber 0
};

/// Misst die Summe einer Note (`rechneZupfSumme`) wie IC: `note` und `rate` aus dem Anschlag.
[[nodiscard]] ZupfTon messeZupfSumme(std::span<const double> summe, int note, int rate);

/// Eine Note rechnen und messen wie IC `rechneZupfTon(…, &messung)`.
[[nodiscard]] ZupfTon messeZupfTon(const kern::zupf::ZupfSpiel& spiel, const kern::zupf::ZupfAnschlag& a, std::int64_t frames);

}  // namespace lumi::messung
