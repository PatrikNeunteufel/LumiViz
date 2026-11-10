/**
 ****************************************************************************************
 * @file   VorlageOhneDatei.hpp
 * @brief  Tests: eine Vorlage ohne mitgelieferte Datei (Deluxe) wie eine Datei des Programms (S186, LG1 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * Hackbrett, Cimbalom (`struckString`, KL1 a) und die Steeldrum (`bar`, KS4 a) gehen nicht mit, kommen aber in die
 * Hoerlisten von LumiViz (LG1 a). `wieAusDerDatei` macht aus dem Instrument, das der Schreiber des Tests baut (ohne
 * gestimmte Werte und Bezug), dasselbe, was `InstrumentTuneTool <datei> --all --reference` daraus macht, und laedt es
 * ueber `alsJson`/`ausJson` wie das Programm eine Datei — die Klaenge sind die einer Datei, die es nicht gibt.
 ****************************************************************************************
 */
#pragma once

#include <doctest.h>

#include "InstrumentMeasure/Stimmen.hpp"
#include "InstrumentModels/Kerne.hpp"
#include "loop/Instrument.hpp"

#include <QJsonObject>
#include <QString>

#include <string>

namespace vorlage
{

/// Das Instrument gestimmt und mit Bezug, wie aus einer Datei gelesen; `name` wird sein Name (Kennung der Klaenge).
inline lumi::loop::Instrument wieAusDerDatei(lumi::loop::Instrument roh, const QString& name)
{
    for (lumi::loop::Schicht& s : roh.schichten)
    {
        if (!lumi::loop::istKernSchicht(s)) continue;
        const lumi::messung::StimmBericht b = lumi::messung::stimmeUndNormiere(lumi::kern::kerne(), lumi::messung::stimmer(),
                                                                               s.kernName.toStdString(), s.kernWerte, true, true);
        for (const std::string& e : b.fehler) FAIL_CHECK(name.toStdString() << ": " << e);
    }
    lumi::loop::Instrument i;
    REQUIRE_MESSAGE(lumi::loop::ausJson(lumi::loop::alsJson(roh), i), name.toStdString() << " nach dem Stimmen nicht lesbar");
    i.name = name;
    return i;
}

}  // namespace vorlage
