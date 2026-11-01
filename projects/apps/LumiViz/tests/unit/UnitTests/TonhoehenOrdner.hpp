/**
 * @file TonhoehenOrdner.hpp
 * @brief Tests: der Speicher der Tonhoehenverlaeufe (`loop/Tonhoehenspeicher`) in einem
 *        Wegwerf-Ordner, fuer die Dauer eines Falls (Stufe 15i)
 *
 * Ein Wave editor misst die Tonhoehe seiner Datei im Hintergrund und merkt sie unter dem
 * **Inhalt** der Datei — nicht unter dem Dekodierer. Tests geben kleinen Dateien mit gleichem
 * Inhalt verschiedene Klaenge; ohne Leeren sahe der naechste Fall den Verlauf des vorigen. Als
 * erstes im Fall anlegen: dann stirbt es nach dem Editor (der seine Messung beim Sterben abwartet).
 */
#pragma once

#include "loop/Tonhoehenspeicher.hpp"

#include <QTemporaryDir>

#include <doctest.h>

struct TonhoehenOrdner
{
    QTemporaryDir ordner;

    TonhoehenOrdner()
    {
        REQUIRE(ordner.isValid());
        lumi::loop::setzeTonhoehenOrdnerFuerTests(ordner.path());
        lumi::loop::vergissTonhoehenverlaeufe();
    }
    ~TonhoehenOrdner()
    {
        lumi::loop::vergissTonhoehenverlaeufe();
        lumi::loop::setzeTonhoehenOrdnerFuerTests({});
    }
    TonhoehenOrdner(const TonhoehenOrdner&) = delete;
    TonhoehenOrdner& operator=(const TonhoehenOrdner&) = delete;
};
