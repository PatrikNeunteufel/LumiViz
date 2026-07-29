/**
 ****************************************************************************************
 * @file   test_FieldDocs.cpp
 * @brief  Waechter: kein Knotenfeld ohne Erklaerung (Knoten-Parameter-Konzept §10)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 *
 * @details
 * Der Tooltip-Text eines Feldes steht als Doxygen-Kommentar am Feld des
 * `…Params`-Structs in `EffectChain.hpp`; `harvest_field_docs.py` erntet ihn in
 * die erzeugte Tabelle `FieldDocs.cpp`, und das Panel liest ihn dort. Damit
 * kann ein Text nicht doppelt gepflegt werden — aber er kann FEHLEN, und genau
 * das faellt ohne Waechter niemandem auf: das Panel zeigt dann einfach keinen
 * Hinweis, und der Knoten sieht aus wie jeder andere.
 *
 * Der Waechter ist HART: **alle 717 Felder** haben einen Text (S56). Es gibt
 * keine Ausnahmeliste — ein neues Feld ohne Doxygen-Kommentar faellt sofort
 * durch, und das ist der ganze Zweck.
 *
 * Am Tag der Einfuehrung fehlten 178 Texte. 163 davon waren echte Luecken und
 * wurden geschrieben; **15 waren keine** — der Ernter konnte sie nur nicht
 * lesen. Er sah bis dahin ausschliesslich `///`-Zeilen an EINZEILIGEN
 * Deklarationen. Ihm entgingen: Doxygen-Blockkommentare, mehrzeilige
 * Deklarationen (`std::array<int, 49> kernel = {…}` steht in vier Zeilen),
 * Feldlaengen (`uint32_t colors[5]`), Klammer-Vorbelegungen
 * (`colors{0xFFFFFF}`) und Zeichenketten mit einem `;` darin.
 *
 * Wer ein Feld ergaenzt: Doxygen an das Feld in `EffectChain.hpp`, dann
 * `python asset/calibration/fields/harvest_field_docs.py` (schreibt
 * `FieldDocs.cpp` neu) — sonst wird dieser Test rot.
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/panels/FieldDocs.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"
#include "visualizers/multieffect/EffectChain.hpp"
#include "visualizers/multieffect/NodePresetStore.hpp"

#include <QString>
#include <QStringList>

#include <set>
#include <string>
#include <utility>
#include <variant>

using namespace lumi::multieffect;
namespace np = lumi::multieffect::nodepresets;
namespace fd = lumi::multieffect::fielddocs;

namespace
{

/// `<typkey>.<feld>` fuer jedes Feld jedes Knotentyps — ohne Aufzaehlung von Hand.
template <std::size_t... I>
std::set<QString> alleFelderImpl(std::index_sequence<I...>)
{
    std::set<QString> out;
    const auto sammle = [&out](const EffectParams& params) {
        const QString key = effectTypeKey(params);
        for (const QString& f : np::fieldNames(params)) out.insert(key + QLatin1Char('.') + f);
    };
    (sammle(EffectParams(std::in_place_index<I>)), ...);
    return out;
}

std::set<QString> alleFelder()
{
    return alleFelderImpl(std::make_index_sequence<std::variant_size_v<EffectParams>>{});
}

} // namespace

TEST_CASE("Feld-Tooltips: jedes Feld hat einen Text")
{
    for (const QString& key : alleFelder())
    {
        const int punkt = key.indexOf(QLatin1Char('.'));
        const QString text = fd::tooltip(key.left(punkt), key.mid(punkt + 1));
        CHECK_MESSAGE(!text.isEmpty(),
                      "Feld ohne Erklaerung: " << key.toStdString()
                      << " — Doxygen an das Feld in EffectChain.hpp schreiben und "
                         "harvest_field_docs.py laufen lassen");
    }
}

TEST_CASE("Feld-Tooltips: die Tabelle nennt kein Feld, das es nicht gibt")
{
    const std::set<QString> felder = alleFelder();
    for (const QString& key : fd::documentedKeys())
        CHECK_MESSAGE(felder.count(key) == 1,
                      "FieldDocs.cpp kennt ein unbekanntes Feld: " << key.toStdString()
                      << " — harvest_field_docs.py neu laufen lassen");
}
