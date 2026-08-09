/**
 ****************************************************************************************
 * @file   test_MilkdropSamplerName.cpp
 * @brief  Gates fuer die Sampler-Namensregel (S52), gepinnt am Quelltext der
 *         Referenz: ref/winamp_orig/…/vis_milk2/plugin.cpp:2955
 *
 * Drei Befunde aus dem Fehler-Log eines Blaetter-Laufs:
 *  - `sampler MilkDrop3_001` (praefixlos) wurde zu `3_001` — Datei nicht gefunden
 *  - `sampler tex` (3 Zeichen) warf `std::out_of_range`
 *  - Filter-Praefixe wurden nur klein und nur in einer Reihenfolge erkannt
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/milkdrop/MilkdropSamplerName.hpp"

using lumi::milkdrop::parseSamplerName;

TEST_SUITE("MilkdropSamplerName")
{
    TEST_CASE("sampler_ wird nur abgeschnitten, wenn es da steht")
    {
        // plugin.cpp:2955 — if (!strncmp(cd.Name,"sampler_",8)) … else Name pur
        CHECK(parseSamplerName("sampler_billy").root == "billy");

        // Der Befund: praefixlos ist gueltig, der Name bleibt vollstaendig.
        // Vorher lieferte substr(8) hier "3_001" und die Textur fehlte.
        CHECK(parseSamplerName("MilkDrop3_001").root == "MilkDrop3_001");
    }

    TEST_CASE("Kurze Namen sind kein Absturz")
    {
        // `sampler tex` steht in 25 Presets des Packs; substr(8) auf drei
        // Zeichen ist std::out_of_range, nicht bloss ein falscher Name.
        CHECK(parseSamplerName("tex").root == "tex");
        CHECK(parseSamplerName("").root.empty());
        CHECK(parseSamplerName("a").root == "a");
        CHECK(parseSamplerName("sampler_").root.empty());
    }

    TEST_CASE("Filter/Wrap-Praefix: beide Schreibweisen, beide Reihenfolgen")
    {
        struct Fall
        {
            const char* name;
            bool bilinear;
            bool wrap;
        };
        // FW = bilinear+wrap, FC = bilinear+clamp, PW = point+wrap, PC = point+clamp
        const Fall faelle[] = {
            {"sampler_fw_bild", true, true},   {"sampler_FW_bild", true, true},
            {"sampler_fc_bild", true, false},  {"sampler_FC_bild", true, false},
            {"sampler_pw_bild", false, true},  {"sampler_PW_bild", false, true},
            {"sampler_pc_bild", false, false}, {"sampler_PC_bild", false, false},
            // Umgedrehte Formen — die Referenz erlaubt sie ausdruecklich
            {"sampler_wf_bild", true, true},   {"sampler_cf_bild", true, false},
            {"sampler_wp_bild", false, true},  {"sampler_cp_bild", false, false},
            // Auch ohne sampler_-Praefix
            {"PC_bild", false, false},
        };
        for (const Fall& f : faelle)
        {
            CAPTURE(f.name);
            const auto parsed = parseSamplerName(f.name);
            CHECK(parsed.root == "bild");
            CHECK(parsed.bilinear == f.bilinear);
            CHECK(parsed.wrap == f.wrap);
        }
    }

    TEST_CASE("Ein fremdes Zweierpaar bleibt Teil des Namens")
    {
        // Nur FW/FC/PW/PC (+ Umkehrungen) sind Praefixe — alles andere gehoert
        // zum Dateinamen, sonst suchte `sampler_ab_bild` die Datei `bild`.
        const auto parsed = parseSamplerName("sampler_ab_bild");
        CHECK(parsed.root == "ab_bild");
        CHECK(parsed.bilinear);  // Vorbelegung: bilinear + wrap
        CHECK(parsed.wrap);

        // Genau drei Zeichen nach dem Praefix-Test: "fw_" allein ist zu kurz
        // (Bedingung ist size() > 3), bleibt also unveraendert.
        CHECK(parseSamplerName("fw_").root == "fw_");
        // Der Unterstrich muss an Stelle 2 stehen.
        CHECK(parseSamplerName("f_wbild").root == "f_wbild");
    }
}
