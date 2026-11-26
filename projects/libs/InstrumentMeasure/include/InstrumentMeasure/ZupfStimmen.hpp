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
#include "InstrumentMeasure/UmlaufEntwurf.hpp"
#include "InstrumentModels/zupf/Saiten.hpp"
#include "InstrumentModels/zupf/UmlaufTafel.hpp"
#include "InstrumentModels/zupf/Zupf.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

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

// ---------------------------------------------------------------- Rechenfassung 2 (34k-7d-3, ZK1 a)

/// Die Tafel des Umlauf-Filters eines Instruments, entworfen (`umlauf::entwirfJeTaste` / `entwirfJeSaite`), mit der
/// Bewertung je Stelle nach UL2 a.
struct UmlaufTafelEntwurf
{
    std::vector<kern::zupf::UmlaufEintrag> eintraege;
    /// je Stelle ein Name (`T60`, `Chor 1.2 Buende` …) und ihre Bewertung; je Saite vier: an den Buenden, auf halbem Weg,
    /// an Achteln, an fremden Stellen
    std::vector<std::pair<std::string, umlauf::Bewertung>> bewertungen;
    std::vector<std::string> fehler;  ///< Stellen, fuer die der Entwurf nichts fand
    double sekunden = 0.0;
};

/// Je Taste `von` … `bis` (Harfe, Cembalo); Tasten ohne Saite (`std::nullopt`) fehlen in der Tafel.
[[nodiscard]] UmlaufTafelEntwurf entwirfUmlaufJeTaste(int von, int bis,
                                                     const std::function<std::optional<kern::zupf::Tastensaite>(int)>& saite);
/// Je Saite des Satzes (Gitarre, Laute, Oud); eine Saite im Einklang mit der ersten des Chors bekommt keine eigene Tafel
/// (ihre Verstimmung ist eine Lage auf derselben). Nur die Choere `chorVon` … `chorBis` (Index im Satz von der tiefsten
/// an; −1 = bis zum letzten) — ein langer Entwurf laesst sich so in Stuecke teilen.
[[nodiscard]] UmlaufTafelEntwurf entwirfUmlaufJeSaite(const kern::zupf::Saitensatz& satz, int chorVon = 0, int chorBis = -1);

}  // namespace lumi::messung
