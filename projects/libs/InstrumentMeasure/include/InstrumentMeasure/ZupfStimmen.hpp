/**
 * @file ZupfStimmen.hpp
 * @brief Messen und Stimmen am Kern `pluckedString` (Klasse 2): die Tonhoehe einer gezupften Saite und der Faktor auf die
 *        Laufzeiten je Note (Instrumente_IC_Konzept.md 34k-2, 34k-3, IK2-2 a, IK2-11 a)
 *
 * **Stimmer** (34k-3): IC `stimme` (`Zupf.cpp:391–424`) wortgleich — jede dritte Note und die hoechste, je Note hoechstens
 * drei Runden: die Note 0,6 s gehalten und gerechnet, die Tonhoehe wie unten gemessen, Abbruch unter 0,6 Cent, sonst der
 * Faktor mal gemessen/Soll; zwischen den Stuetzpunkten logarithmisch (`wertAn`). Gerechnet ohne die bisherige Laufzeit.
 *
 * Uebernommen aus IC `rechnung/klassen/02_zupf_saite/Zupf.cpp` (`4409088`), der Messteil von `rechneZupfTon` wortgleich:
 * IC mass im Kern, hier misst `InstrumentMeasure` an der Summe des Kerns (`rechneZupfSumme`, vor Bezug und Ausblende —
 * dieselben Zahlen, an denen IC mass). Die Tonhoehe ist die staerkste Spitze im Spektrum innerhalb ±60 Cent um die Note,
 * Hann-Fenster 0,1 … 0,5 s, Raster 0,5 Cent („YIN sprang bei schwachem Grundton auf die Oktave“, IC).
 *
 * Ohne Qt.
 */
#pragma once

#include "InstrumentMeasure/Stimmen.hpp"
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

/// Der Faktor auf die Laufzeiten je Note (IC `stimme`, siehe Kopf), mit Belegkraft *Rechnung*; die Noten auf `threads`
/// Threads (0 = Pool).
[[nodiscard]] kern::Kurve zupfLaufzeit(const kern::zupf::ZupfSpiel& spiel, unsigned threads = 0);

/// Der Stimmer des Kerns `pluckedString` (`delayFactor`).
[[nodiscard]] Stimmer zupfStimmer();

}  // namespace lumi::messung
