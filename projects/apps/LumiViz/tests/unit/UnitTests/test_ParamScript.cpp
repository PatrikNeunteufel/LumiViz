/**
 ****************************************************************************************
 * @file   test_ParamScript.cpp
 * @brief  Der Strang-D-Vertrag: Wert rein, Slot laufen lassen, Wert wieder raus
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 *
 * @details
 * `MultiEffectVisualizer::runParamScript` macht fuer 47 Renderer immer dasselbe:
 * Frame-Kopie in die Engine (`setNumber`), Init/Frame/Beat laufen lassen, Wert
 * zurueckholen (`number`). Die Feld-Sonden aus Strang E (S54) zeigten, dass am
 * Ende NICHTS ankommt — bei sechs von sechs geprueften Knotentypen.
 *
 * Dieser Test baut genau diese Sequenz GL-frei nach. Er trennt damit die zwei
 * moeglichen Ursachen:
 * | Test rot  | die Schnittstelle Engine/Transpiler traegt den Wert nicht |
 * | Test gruen| dann liegt es am Renderpfad, nicht am Skript-Fundament |
 ****************************************************************************************
 */

#include <doctest.h>

#include "scripting/ScriptSlotHost.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QJsonObject>
#include <QStringList>

using lumi::scripting::ScriptSlotHost;
using Slot = ScriptSlotHost::Slot;

TEST_CASE("Strang D: der Frame-Slot schreibt eine vorbelegte Variable zurueck")
{
    ScriptSlotHost host("test", {}, ScriptSlotHost::Dialect::Avs);
    host.setSource(Slot::Frame, "quality = 3");
    REQUIRE_MESSAGE(host.compileAll(), "compileAll: " << host.lastError());
    REQUIRE(host.has(Slot::Frame));

    // Genau die Reihenfolge aus runParamScript: erst vorbelegen, dann laufen.
    host.engine().setNumber("quality", 1.0);
    CHECK(host.run(Slot::Frame));
    CHECK_MESSAGE(host.engine().number("quality") == doctest::Approx(3.0),
                  "Slot lief, aber der Wert kam nicht zurueck — "
                      << host.lastError());
}

TEST_CASE("Strang D: der Slot LIEST die Vorbelegung")
{
    ScriptSlotHost host("test", {}, ScriptSlotHost::Dialect::Avs);
    host.setSource(Slot::Frame, "quality = quality * 2");
    REQUIRE_MESSAGE(host.compileAll(), "compileAll: " << host.lastError());

    host.engine().setNumber("quality", 5.0);
    CHECK(host.run(Slot::Frame));
    CHECK(host.engine().number("quality") == doctest::Approx(10.0));
}

TEST_CASE("Strang D: der Init-Slot setzt eine Startbelegung, die Frames fortschreiben")
{
    // Entscheid Patrik S54. Nachgebaut wird die Reihenfolge aus
    // runParamScript INKLUSIVE der Fortschreibung: Vorbelegung nur beim
    // ersten Frame aus den Params, danach aus dem gehaltenen Stand.
    ScriptSlotHost host("test", {}, ScriptSlotHost::Dialect::Avs);
    host.setSource(Slot::Init, "quality = 10");
    host.setSource(Slot::Frame, "quality = quality + 1");
    REQUIRE_MESSAGE(host.compileAll(), "compileAll: " << host.lastError());

    const double ausParams = 50.0;  // der Reglerwert
    double stand = ausParams;

    for (int frame = 0; frame < 3; ++frame)
    {
        host.engine().setNumber("quality", stand);
        if (frame == 0) CHECK(host.run(Slot::Init));
        CHECK(host.run(Slot::Frame));
        stand = host.engine().number("quality");
    }

    // Init 50 -> 10, dann drei Frames +1. Ohne Fortschreibung stuende hier 51:
    // jeder Frame faenge wieder bei 50 an und Init waere wirkungslos.
    CHECK(stand == doctest::Approx(13.0));
}

TEST_CASE("Import-Treue: ein Preset darf `pi` selbst definieren")
{
    // Befund Patrik S54: `$PI` kam in AVS erst spaeter — aeltere Presets
    // setzen `pi` von Hand. Im Korpus (…\VisualsPresets, 3586 .avs) tun das
    // 629 Presets, mehr als die 469, die `$PI` benutzen. Die Werte gehen
    // auseinander (3.14159 · 3.1415 · 3.141592654), und 20 Presets meinen mit
    // `pi` gar keine Kreiszahl (`pi=2`, `pi=22`).
    //
    // Unsere Prelude belegt `pi` vor. Ueberschreibt das Preset sie NICHT,
    // rechnen alle 629 mit einer anderen Zahl als ihr Autor wollte — und die
    // 20 mit `pi=2` rechnen voellig falsch.
    ScriptSlotHost host("test", {}, ScriptSlotHost::Dialect::Avs);
    host.setSource(Slot::Frame, "pi = 3.14159; x = pi * 2");
    REQUIRE_MESSAGE(host.compileAll(), "compileAll: " << host.lastError());
    REQUIRE(host.run(Slot::Frame));

    CHECK_MESSAGE(host.engine().number("pi") == doctest::Approx(3.14159),
                  "die Prelude-Konstante laesst sich nicht ueberschreiben");
    CHECK(host.engine().number("x") == doctest::Approx(6.28318));
}

TEST_CASE("Import-Treue: ohne eigene Zuweisung bleibt `pi` die Kreiszahl")
{
    ScriptSlotHost host("test", {}, ScriptSlotHost::Dialect::Avs);
    host.setSource(Slot::Frame, "x = pi");
    REQUIRE_MESSAGE(host.compileAll(), "compileAll: " << host.lastError());
    REQUIRE(host.run(Slot::Frame));
    CHECK(host.engine().number("x") == doctest::Approx(3.14159265358979));
}

TEST_CASE("Strang D: ein kaputtes Skript meldet einen Fehler")
{
    // Die Sonde mit absichtlichem Unsinn blieb im Betrieb still (S54). Ein
    // Fehler MUSS abfragbar sein, sonst faellt so etwas nie auf.
    ScriptSlotHost host("test", {}, ScriptSlotHost::Dialect::Avs);
    host.setSource(Slot::Frame, "!!! kein gueltiges EEL !!!");
    const bool ok = host.compileAll();
    CHECK_FALSE(ok);
    CHECK_FALSE(host.lastError().empty());
}

TEST_CASE("Altname: `bilinear` in einer bestehenden .lvfx wird weiter gelesen")
{
    // `bilinear` hiess bis S54 so und war deshalb als einziges der sechs
    // subpixel-Felder nicht verdrahtet. Beim Umbenennen duerfen vorhandene
    // Dateien nicht brechen — und die Vorgabe folgt jetzt dem Original:
    // Distance Modifier AUS (r_ddm.cpp:210), Dynamic Shift AN (r_shift.cpp:127).
    using namespace lumi::multieffect;
    QStringList report;

    QJsonObject alt;
    alt["type"] = "dynamicDistanceModifier";
    alt["bilinear"] = true;          // Altname, Wert abweichend von der Vorgabe
    const ChainNode a = nodeFromJson(alt, &report);
    CHECK(std::get<DynamicDistanceModifierParams>(a.params).subpixel);

    QJsonObject ohne;
    ohne["type"] = "dynamicDistanceModifier";
    const ChainNode b = nodeFromJson(ohne, &report);
    CHECK_FALSE(std::get<DynamicDistanceModifierParams>(b.params).subpixel);

    QJsonObject shift;
    shift["type"] = "dynamicShift";
    const ChainNode c = nodeFromJson(shift, &report);
    CHECK(std::get<DynamicShiftParams>(c.params).subpixel);

    // Neu geschrieben wird ausschliesslich der Originalname.
    const QJsonObject raus = nodeToJson(b);
    CHECK(raus.contains("subpixel"));
    CHECK_FALSE(raus.contains("bilinear"));
}
