/**
 * @file hoerproben.cpp
 * @brief Werkzeug: Hoerproben je Instrument als WAV (AUFTRAG_BAU §4 Punkt 4, Lehre L5) — drei Noten ueber den Umfang,
 *        drei Staerken, dazu die Spielweisen der Klasse; je Klasse eine Liste fuer HoerTest_Helper nach Konzept 0.3.0
 *        §11/§12 (Sync Hoertest, IC-1, IC-2; Ablage in `Sammlung.cpp`). Patrik hoert lokal; die Klaenge kommen nicht
 *        ins Repo (`.gitignore`: die Ordner `klaenge/` unter `hoertest/`), `liste.json` und `klaenge.sha256` schon.
 *
 *   ic_hoerproben [Ordner] [Klasse …]     z. B.  ic_hoerproben hoertest 4     (Vorgabe: hoertest)
 *
 * Ohne Klasse: alle gebauten. Gleich laut heisst: jede Datei auf −23 LUFS nach BS.1770-4 mit Gating (HT14 a) — die
 * Staerken klingen dann gleich laut, und das Ohr urteilt ueber die Farbe; der Pegel vorher steht je Klang als
 * Messwert `lufs_vorher` in der Liste.
 */
#include "gemeinsam/Werkzeug.hpp"
#include "klassen/01_hammer_saite/HammerTon.hpp"
#include "klassen/01_hammer_saite/Saite.hpp"
#include "klassen/02_zupf_saite/Zupf.hpp"
#include "klassen/03_bogen_saite/Bogen.hpp"
#include "klassen/04_platte_stab/Stab.hpp"
#include "klassen/05_membran/Trommel.hpp"
#include "klassen/06_becken_gong/Becken.hpp"
#include "klassen/07_lippe_rohr/Blech.hpp"
#include "klassen/08_rohrblatt_rohr/Rohrblatt.hpp"
#include "klassen/09_luftstrahl_rohr/Luftstrahl.hpp"
#include "klassen/10_zunge/Zunge.hpp"
#include "klassen/11_tonrad/Orgel.hpp"
#include "werkzeuge/Hoerproben.hpp"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

using namespace ic;

namespace
{

double hzVon(int note)
{
    return 440.0 * std::exp2((note - 69) / 12.0);
}

std::string notenName(int note)
{
    static const char* namen[] = {"C", "Cis", "D", "Dis", "E", "F", "Fis", "G", "Gis", "A", "Ais", "H"};
    return std::string(namen[note % 12]) + std::to_string(note / 12 - 1);
}

}  // namespace

namespace
{

void klasse1(hoer::Sammlung& s)
{
    using namespace ic::hammer;
    const HammerSaite& saite = hammerSaiteVorgabe();
    for (const int taste : {33, 60, 84})
        for (const int stufe : {-30, -12, 0})
        {
            HammerAnschlag a;
            a.taste = taste;
            a.hz = hzVon(taste);
            a.stufe = stufe;
            a.halten = 2 * a.rate;
            a.gedaempft = taste < kUngedaempftAbVorgabe;
            std::vector<float> ton;
            rechneHammerTon(saite, a, ton, a.halten + hammerNachklang(saite, a, 4 * a.rate));
            s.lege("01_hammer_saite", "Fluegel_Port", notenName(taste) + "_a" + std::to_string(stufe), ton, a.rate,
                   "Pruefstein: klingt der Port wie der Fluegel in LumiViz (Vorgabe d1c06309)?");
        }
}

void klasse1Rest(hoer::Sammlung& s)
{
    using namespace ic::hammer;
    struct Inst
    {
        std::string name;
        HammerSaite saite;
        Tastatur tastatur;
    };
    const std::vector<Inst> alle = {{"Upright", baueVorgabeUpright(), tastaturUpright()},
                                    {"Honkytonk", baueVorgabeHonkytonk(), tastaturUpright()},
                                    {"CP70", baueVorgabeCp70(), tastaturCp70()}};
    for (const Inst& i : alle)
    {
        for (const int taste : {std::max(33, i.tastatur.tiefste + 5), 60, std::min(96, i.tastatur.hoechste - 4)})
            for (const int stufe : {-30, -12, 0})
            {
                HammerAnschlag a;
                a.taste = taste;
                a.hz = hzVon(taste);
                a.stufe = stufe;
                a.halten = 2 * a.rate;
                a.gedaempft = taste < i.tastatur.ungedaempftAb;
                std::vector<float> ton;
                rechneHammerTon(i.saite, a, ton, a.halten + hammerNachklang(i.saite, a, 4 * a.rate));
                s.lege("01_hammer_saite", i.name, notenName(taste) + "_a" + std::to_string(stufe), ton, a.rate,
                       "Klingt es nach " + i.name + " (ungemessen, ohne Boden: Stegkraft)? Bass glaubhaft?");
            }
        if (i.tastatur.mitte == PedalArt::Moderator)
        {
            HammerAnschlag a;
            a.taste = 60;
            a.hz = hzVon(60);
            a.stufe = -12;
            a.halten = 2 * a.rate;
            a.moderatorSteife = i.tastatur.moderatorSteife;
            a.moderatorExponent = i.tastatur.moderatorExponent;
            std::vector<float> ton;
            rechneHammerTon(i.saite, a, ton, a.halten + hammerNachklang(i.saite, a, 4 * a.rate));
            s.lege("01_hammer_saite", i.name, "C4_moderator", ton, a.rate, "Moderator (Practice): gedaempft und dunkel genug? (Werte offen)");
        }
    }
    using namespace ic::saite;
    for (const SaitenSpiel& inst : {baueVorgabeClavinet(), baueVorgabeHackbrett(), baueVorgabeCimbalom()})
    {
        for (const int note : {inst.noteMin + 4, (inst.noteMin + inst.noteMax) / 2, inst.noteMax - 4})
            for (const int stufe : {-30, -12, 0})
            {
                SaitenAnschlag a;
                a.note = note;
                a.hz = hzVon(note);
                a.stufe = stufe;
                a.halten = a.rate;
                std::vector<float> ton;
                rechneSaitenTon(inst, a, ton, a.halten + saitenNachklang(inst, a, 3 * a.rate));
                s.lege("01_hammer_saite", inst.name, notenName(note) + "_a" + std::to_string(stufe), ton, a.rate,
                       "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")?");
            }
        if (inst.anregung == Anregung::Tangente)
        {
            SaitenAnschlag a;
            a.note = 55;
            a.hz = hzVon(55);
            a.stufe = -6;
            a.halten = a.rate / 2;
            a.schieber = 0.5;
            std::vector<float> ton;
            rechneSaitenTon(inst, a, ton, 2 * a.rate);
            s.lege("01_hammer_saite", inst.name, "G3_schieber05", ton, a.rate, "Daempferschieber halb: wie ein Clavinet mit Schieber?");
        }
        if (inst.mitschwingen)
        {
            // je Clip mit Mitschwingen (Deluxe): eine kurze Folge C4 E4 G4 C5, mit und ohne (Cimbalom: Pedal getreten)
            std::vector<SaitenClipNote> clip;
            for (const int d : {0, 4, 7, 12})
            {
                SaitenAnschlag a;
                a.note = 60 + d;
                a.hz = hzVon(60 + d);
                a.stufe = -12;
                a.halten = 4 * a.rate;
                a.gedaempft = false;
                clip.push_back({0.25 * d / 4.0, a});
            }
            std::vector<float> ton;
            rechneSaitenClip(inst, clip, ton, 4 * 48000);
            s.lege("01_hammer_saite", inst.name, "C4_folge_clip_mitschwingen", ton, 48000, "Folge je Clip mit Mitschwingen der anderen Choere (Kopplung offen): hoerbar, glaubhaft?");
            std::vector<float> summe(ton.size(), 0.0f);
            for (const SaitenClipNote& cn : clip)
            {
                std::vector<float> t;
                const auto ab = static_cast<std::size_t>(cn.abS * 48000);
                rechneSaitenTon(inst, cn.anschlag, t, static_cast<std::int64_t>(ton.size() - ab));
                for (std::size_t i = 0; i < t.size(); ++i) summe[ab + i] += t[i];
            }
            s.lege("01_hammer_saite", inst.name, "C4_folge_clip_ohne", summe, 48000, "dieselbe Folge ohne Mitschwingen (Vergleich)");
        }
    }
}

void klasse2(hoer::Sammlung& s)
{
    using namespace ic::zupf;
    for (const ZupfSpiel& inst : {baueVorgabeCembalo(), baueVorgabeHarfe(), baueVorgabeGitarre(), baueVorgabeLaute(), baueVorgabeOud()})
    {
        for (const int note : {inst.noteMin + 2, (inst.noteMin + inst.noteMax) / 2, inst.noteMax - 4})
            for (const double e : {0.0, 0.5, 1.0})
            {
                ZupfAnschlag a;
                a.note = note;
                a.staerke = e;
                a.haltenS = 2.0;
                std::vector<float> ton;
                rechneZupfTon(inst, a, ton, 120000);
                s.lege("02_zupf_saite", inst.name, notenName(note) + "_e" + std::to_string(static_cast<int>(e * 100)), ton, a.rate,
                       "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? Abklingen in zwei Stufen glaubhaft?");
            }
        const int m = (inst.noteMin + inst.noteMax) / 2;
        ZupfAnschlag a;
        a.note = m;
        a.staerke = 0.6;
        a.haltenS = 0.6;
        std::vector<float> ton;
        rechneZupfTon(inst, a, ton, 96000);
        s.lege("02_zupf_saite", inst.name, notenName(m) + "_abgedaempft", ton, a.rate, "Abgedaempft nach 0,6 s: glaubhaft?");
        if (inst.name == "Cembalo")
        {
            a.lautenzug = true;
            a.haltenS = 2.0;
            rechneZupfTon(inst, a, ton, 96000);
            s.lege("02_zupf_saite", inst.name, notenName(m) + "_lautenzug", ton, a.rate, "Lautenzug: glaubhaft?");
        }
        if (inst.name == "Harfe")
        {
            a.laissezVibrer = true;
            rechneZupfTon(inst, a, ton, 144000);
            s.lege("02_zupf_saite", inst.name, notenName(m) + "_laissez_vibrer", ton, a.rate, "laissez vibrer: glaubhaft?");
        }
        if (!inst.mitschwingNoten.empty())
        {
            // Mitschwingen je Clip (K2 F-3 a): ein kurzer Akkord, gedaempft; mit und ohne die ungespielten Saiten
            std::vector<ClipNote> clip;
            for (const int d : {0, 4, 7, 12})
            {
                ZupfAnschlag b;
                b.note = m - 12 + d;
                b.staerke = 0.6;
                b.haltenS = 0.6;
                clip.push_back({0.08 * d / 4.0, b});
            }
            for (const bool mit : {true, false})
            {
                rechneZupfClip(inst, clip, ton, 3 * 48000, mit);
                s.lege("02_zupf_saite", inst.name, notenName(m - 12) + (mit ? "_clip_mitschwingen" : "_clip_ohne"), ton, 48000,
                       mit ? "Akkord je Clip, die ungespielten Saiten schwingen mit (Deluxe): hoerbar, glaubhaft?" : "derselbe Akkord ohne Mitschwingen (Vergleich)");
            }
        }
    }
}

void klasse3(hoer::Sammlung& s)
{
    using namespace ic::bogen;
    for (const StreichSpiel& inst : {baueVorgabeCello(), baueVorgabeVioline(), baueVorgabeViola(), baueVorgabeKontrabass()})
    {
        for (const int note : {inst.noteMin + 2, (inst.noteMin + inst.noteMax) / 2, inst.noteMax - 7})
            for (const double e : {0.0, 0.5, 1.0})
            {
                StreichAnschlag a;
                a.kette = {{0.0, note, hzVon(note)}};
                a.expression = e;
                a.haltenS = 1.5;
                std::vector<float> ton;
                rechneStreichTon(inst, a, ton, static_cast<std::int64_t>(1.7 * a.rate));
                s.lege("03_bogen_saite", inst.name, notenName(note) + "_e" + std::to_string(static_cast<int>(e * 100)), ton, a.rate,
                       "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? Ansatz und Strich glaubhaft?");
            }
        const int m = (inst.noteMin + inst.noteMax) / 2;
        const auto probe = [&](const std::string& name, const std::string& frage, auto&& aendere) {
            StreichAnschlag a;
            a.kette = {{0.0, m, hzVon(m)}};
            a.expression = 0.6;
            a.haltenS = 1.8;
            aendere(a);
            std::vector<float> ton;
            rechneStreichTon(inst, a, ton, 96000);
            s.lege("03_bogen_saite", inst.name, notenName(m) + "_" + name, ton, a.rate, frage);
        };
        probe("vibrato", "Vibrato 20 Cent, 5,5 Hz: glaubhaft, schimmert es?", [](StreichAnschlag& a) { a.vibratoCent = 20.0; });
        probe("pizzicato", "Pizzicato: glaubhaft?", [](StreichAnschlag& a) { a.art = Art::Pizzicato; });
        probe("spiccato", "Spiccato (ungemessen): glaubhaft?", [](StreichAnschlag& a) { a.art = Art::Spiccato; });
        probe("tremolo", "Tremolo (ungemessen): glaubhaft?", [](StreichAnschlag& a) { a.art = Art::Tremolo; });
        probe("col_legno", "Col legno (ungemessen): glaubhaft?", [](StreichAnschlag& a) { a.art = Art::ColLegno; });
        probe("ponticello", "Sul ponticello: heller, glasiger?", [](StreichAnschlag& a) { a.lage = Lage::Ponticello; });
        probe("tasto", "Sul tasto: weicher?", [](StreichAnschlag& a) { a.lage = Lage::Tasto; });
        probe("sordino", "Con sordino: gedeckter?", [](StreichAnschlag& a) { a.daempfer = true; });
        probe("legato", "Legato in einem Strich: Uebergaenge glaubhaft?", [&](StreichAnschlag& a) {
            a.kette = {{0.0, m, hzVon(m)}, {0.6, m + 2, hzVon(m + 2)}, {1.2, m + 4, hzVon(m + 4)}};
        });
        StreichAnschlag a;
        a.kette = {{0.0, m, hzVon(m)}};
        a.expression = 0.6;
        a.haltenS = 1.8;
        a.vibratoCent = 15.0;
        std::vector<float> ton;
        rechneEnsembleTon(inst, a, 8, ton, 96000);
        s.lege("03_bogen_saite", inst.name, notenName(m) + "_ensemble8", ton, a.rate, "Ensemble aus 8 Stimmen (Streuung gesetzt): wie ein Satz?");
    }
}

void klasse4(hoer::Sammlung& s)
{
    using namespace ic::stab;
    for (const StabSpiel& inst : alleStabSpiele())
    {
        const int mitte = (inst.noteMin + inst.noteMax) / 2;
        const std::string ordner = inst.name;
        for (const int note : {inst.noteMin + 2, mitte, inst.noteMax - 2})
            for (const int stufe : {-30, -12, 0})
            {
                StabAnschlag a;
                a.note = note;
                a.hz = hzVon(note);
                a.stufe = stufe;
                std::vector<float> ton;
                const auto frames = std::min<std::int64_t>(stabNachklang(inst, a, 6 * a.rate) + a.rate / 10, 6 * a.rate);
                rechneStabTon(inst, a, ton, frames);
                s.lege("04_platte_stab", ordner, notenName(note) + "_a" + std::to_string(stufe), ton, a.rate,
                       "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? Farbe ueber die Staerke glaubhaft?");
            }
        // Spielweisen: Motor (Vibraphon), Daempfer (Leiste/einzeln), Anschlagstelle
        StabAnschlag a;
        a.note = mitte;
        a.hz = hzVon(mitte);
        a.stufe = -12;
        std::vector<float> ton;
        if (inst.motor)
            for (const double drehzahl : {2.0, 6.0})
            {
                a.motor = {{-1.0, drehzahl}};
                rechneStabTon(inst, a, ton, 4 * a.rate);
                s.lege("04_platte_stab", ordner, notenName(mitte) + "_motor" + std::to_string(static_cast<int>(drehzahl)), ton, a.rate,
                       "Motor " + std::to_string(static_cast<int>(drehzahl)) + " U/s: Tiefe und Form des Vibratos glaubhaft (Tiefe offen)?");
                a.motor.clear();
            }
        if (inst.daempfer != DaempferArt::Keiner)
        {
            a.daempferAb = a.rate / 2;
            rechneStabTon(inst, a, ton, 2 * a.rate);
            s.lege("04_platte_stab", ordner, notenName(mitte) + "_gedaempft", ton, a.rate, "Daempfer nach 0,5 s: zu schnell, zu langsam?");
            a.daempferAb = -1;
        }
        if (inst.anregung == Anregung::Stoss && inst.abnahme == Abnahme::Akustisch)
        {
            a.stelleOffset = -0.3;
            rechneStabTon(inst, a, ton, 2 * a.rate);
            s.lege("04_platte_stab", ordner, notenName(mitte) + "_rand", ton, a.rate, "Anschlag nahe dem Rand: heller, mit Mode 2?");
        }
        if (inst.bogen)
            for (const int note : {mitte - 12, mitte + 7})
                for (const double e : {0.0, 0.5})
                {
                    StabAnschlag b;
                    b.note = note;
                    b.hz = hzVon(note);
                    b.gestrichen = true;
                    b.expression = e;
                    b.haltenS = 2.0;
                    rechneStabTon(inst, b, ton, 3 * b.rate);
                    s.lege("04_platte_stab", ordner, notenName(note) + "_gestrichen_e" + std::to_string(static_cast<int>(e * 100)), ton, b.rate,
                           "Gestrichen (Bogen × Stab, ungemessen; tief mit Resonator +100 Cent, offen.md): glaubhaft?");
                }
    }
}

void klasse5(hoer::Sammlung& s)
{
    using namespace ic::trommel;
    for (const TrommelSpiel& inst : {baueVorgabeTom(), baueVorgabePauke(), baueVorgabeBassdrum(), baueVorgabeSnare(), baueVorgabeTabla(),
                                     baueVorgabeConga(), baueVorgabeBongo()})
    {
        const int note = (inst.note + inst.noteMax) / 2;
        for (const double e : {0.0, 0.5, 1.0})
        {
            TrommelAnschlag a;
            a.staerke = e;
            a.note = note;
            std::vector<float> ton;
            rechneTrommelTon(inst, a, ton, 96000);
            s.lege("05_membran", inst.name, "e" + std::to_string(static_cast<int>(e * 100)), ton, a.rate,
                   "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? Gleiten und Abklingen glaubhaft?");
        }
        const auto probe = [&](const std::string& name, const std::string& frage, auto&& aendere) {
            TrommelAnschlag a;
            a.staerke = 0.6;
            a.note = note;
            aendere(a);
            std::vector<float> ton;
            rechneTrommelTon(inst, a, ton, 96000);
            s.lege("05_membran", inst.name, name, ton, a.rate, frage);
        };
        probe("mitte", "Schlag in die Mitte: dumpfer (Pauke: tonlos)?", [](TrommelAnschlag& a) { a.schlag = Schlag::Mitte; });
        probe("rand", "Am Rand: heller, obertonreicher?", [](TrommelAnschlag& a) { a.schlag = Schlag::Rand; });
        probe("gedaempft", "Abgedaempft (Hand): kurz genug?", [](TrommelAnschlag& a) { a.schlag = Schlag::Gedaempft; });
        probe("wirbel", "Wirbel 1,5 s (Rate offen): glaubhaft?", [](TrommelAnschlag& a) { a.wirbelS = 1.5; });
        if (inst.name == "Snare")
        {
            probe("randschlag", "Rimshot (Klick halbphysikalisch): glaubhaft?", [](TrommelAnschlag& a) { a.schlag = Schlag::Randschlag; });
            probe("reifen", "Cross-Stick: glaubhaft?", [](TrommelAnschlag& a) { a.schlag = Schlag::Reifen; });
            probe("ohne_teppich", "Teppich ab: wie eine Tom?", [](TrommelAnschlag& a) { a.ohneTeppich = true; });
        }
        if (inst.stimmbar) probe("glissando", "Pedal-Glissando +4 Halbtoene: glaubhaft?", [](TrommelAnschlag& a) { a.glissandoCent = 400.0; });
    }
}

void klasse6(hoer::Sammlung& s)
{
    using namespace ic::becken;
    for (const BeckenSpiel& inst : {baueVorgabeTamtam(), baueVorgabeGong(), baueVorgabeCrash(), baueVorgabeChina(), baueVorgabeSplash(),
                                    baueVorgabeRide(), baueVorgabeHihat()})
    {
        // vorab gerechnet (1–7 × Echtzeit): kurze Proben, 2,5 s
        for (const double e : {0.0, 0.5, 1.0})
        {
            BeckenAnschlag a;
            a.staerke = e;
            std::vector<float> ton;
            rechneBeckenTon(inst, a, ton, 120000);
            s.lege("06_becken_gong", inst.name, "e" + std::to_string(static_cast<int>(e * 100)), ton, a.rate,
                   "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? Aufbluehen bei ff glaubhaft?");
        }
        const auto probe = [&](const std::string& name, const std::string& frage, auto&& aendere) {
            BeckenAnschlag a;
            a.staerke = 0.7;
            aendere(a);
            std::vector<float> ton;
            rechneBeckenTon(inst, a, ton, 96000);
            s.lege("06_becken_gong", inst.name, name, ton, a.rate, frage);
        };
        probe("kuppe", "Auf der Kuppe (flache Platte — Kuppe nicht gebaut): anders genug?", [](BeckenAnschlag& a) { a.stelle = Stelle::Kuppe; });
        probe("rand", "Am Rand: heller?", [](BeckenAnschlag& a) { a.stelle = Stelle::Rand; });
        probe("choke", "Choke nach 0,4 s: glaubhaft?", [](BeckenAnschlag& a) { a.chokeAbS = 0.4; });
        if (inst.hihat)
        {
            probe("geschlossen", "Hi-Hat geschlossen (Daempfung, ohne Kontakt): glaubhaft?", [](BeckenAnschlag& a) { a.oeffnung = 0.0; });
            probe("halb", "Hi-Hat halb offen: glaubhaft?", [](BeckenAnschlag& a) { a.oeffnung = 0.5; });
        }
    }
}

void klasse7(hoer::Sammlung& s)
{
    using namespace ic::blech;
    for (const BlechSpiel& inst : {baueVorgabePosaune(), baueVorgabeAlphorn(), baueVorgabeTrompete()})
    {
        for (const int note : {inst.noteMin + 3, (inst.noteMin + inst.noteMax) / 2, inst.noteMax - 4})
            for (const double e : {0.0, 0.5, 1.0})
            {
                BlechAnschlag a;
                a.kette = {{0.0, note, hzVon(note)}};
                a.expression = e;
                a.haltenS = 1.2;
                std::vector<float> ton;
                rechneBlechTon(inst, a, ton, static_cast<std::int64_t>(1.4 * a.rate));
                s.lege("07_lippe_rohr", inst.name, notenName(note) + "_e" + std::to_string(static_cast<int>(e * 100)), ton, a.rate,
                       "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? Schmettern bei ff glaubhaft?");
            }
        const int m = (inst.noteMin + inst.noteMax) / 2;
        BlechAnschlag a;
        a.kette = {{0.0, m, hzVon(m)}};
        a.expression = 0.6;
        a.haltenS = 1.5;
        std::vector<float> ton;
        a.vibratoCent = 25.0;
        rechneBlechTon(inst, a, ton, 80000);
        s.lege("07_lippe_rohr", inst.name, notenName(m) + "_vibrato", ton, a.rate, "Vibrato der Lippe 25 Cent: glaubhaft?");
        a.vibratoCent = 0.0;
        a.growl = true;
        rechneBlechTon(inst, a, ton, 80000);
        s.lege("07_lippe_rohr", inst.name, notenName(m) + "_growl", ton, a.rate, "Growl (Oktave tiefer gesungen): rau genug?");
        a.growl = false;
        a.ende = Endart::Fall;
        rechneBlechTon(inst, a, ton, 80000);
        s.lege("07_lippe_rohr", inst.name, notenName(m) + "_fall", ton, a.rate, "Fall am Ende: wie ein Fall?");
        a.ende = Endart::Normal;
        if (inst.zugMaxM > 0.0)
        {
            a.kette = {{0.0, m, hzVon(m)}, {0.5, m - 2, hzVon(m - 2)}, {1.0, m + 3, hzVon(m + 3)}};
            rechneBlechTon(inst, a, ton, 80000);
            s.lege("07_lippe_rohr", inst.name, notenName(m) + "_legato", ton, a.rate, "Legato mit dem Zug: Uebergaenge glaubhaft?");
        }
        if (!inst.ventilM.empty())
        {
            a.kette = {{0.0, m, hzVon(m)}, {0.5, m - 2, hzVon(m - 2)}, {1.0, m + 3, hzVon(m + 3)}};
            rechneBlechTon(inst, a, ton, 80000);
            s.lege("07_lippe_rohr", inst.name, notenName(m) + "_legato", ton, a.rate, "Legato mit Ventilen: Uebergaenge glaubhaft?");
        }
        for (std::size_t d = 0; d < inst.daempfer.size(); ++d)
        {
            a.kette = {{0.0, m, hzVon(m)}};
            a.daempfer = static_cast<int>(d);
            rechneBlechTon(inst, a, ton, 80000);
            s.lege("07_lippe_rohr", inst.name, notenName(m) + "_" + inst.daempfer[d].name, ton, a.rate,
                   "Daempfer " + inst.daempfer[d].name + " (Werte offen): erkennbar?");
            if (inst.daempfer[d].restZu > 0.0)
            {
                a.wah = {{0.0, 0.0}, {0.3, 1.0}, {0.6, 0.0}, {0.9, 1.0}, {1.2, 0.0}, {1.5, 1.0}};
                rechneBlechTon(inst, a, ton, 80000);
                s.lege("07_lippe_rohr", inst.name, notenName(m) + "_" + inst.daempfer[d].name + "_wahwah", ton, a.rate,
                       "Wah-Wah am " + inst.daempfer[d].name + " (Hand zu/auf je 0,3 s; Restflaeche zu offen): glaubhaft?");
                a.wah.clear();
            }
            a.daempfer = -1;
        }
    }
}

void klasse8(hoer::Sammlung& s)
{
    using namespace ic::rohrblatt;
    for (const RohrblattSpiel& inst : {baueVorgabeKlarinette(), baueVorgabeSopransaxophon(), baueVorgabeTenorsaxophon(), baueVorgabeAltsaxophon()})
    {
        for (const int note : {inst.noteMin + 3, (inst.noteMin + inst.noteMax) / 2, inst.noteMax - 4})
            for (const double e : {0.0, 0.5, 1.0})
            {
                RohrblattAnschlag a;
                a.kette = {{0.0, note, hzVon(note)}};
                a.expression = e;
                a.haltenS = 1.2;
                std::vector<float> ton;
                rechneRohrblattTon(inst, a, ton, static_cast<std::int64_t>(1.4 * a.rate));
                s.lege("08_rohrblatt_rohr", inst.name, notenName(note) + "_e" + std::to_string(static_cast<int>(e * 100)), ton, a.rate,
                       "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? Einsatz mit der Zunge glaubhaft?");
            }
        const int m = (inst.noteMin + inst.noteMax) / 2;
        RohrblattAnschlag a;
        a.kette = {{0.0, m, hzVon(m)}};
        a.expression = 0.6;
        a.haltenS = 1.5;
        std::vector<float> ton;
        a.ansatz = Ansatz::Atem;
        rechneRohrblattTon(inst, a, ton, 80000);
        s.lege("08_rohrblatt_rohr", inst.name, notenName(m) + "_atem", ton, a.rate, "Einsatz ohne Zunge: weich genug?");
        a.ansatz = Ansatz::Flatterzunge;
        rechneRohrblattTon(inst, a, ton, 80000);
        s.lege("08_rohrblatt_rohr", inst.name, notenName(m) + "_flatter", ton, a.rate, "Flatterzunge (25 Hz): glaubhaft?");
        a.ansatz = Ansatz::Zunge;
        a.pitchCent = -150.0;
        rechneRohrblattTon(inst, a, ton, 80000);
        s.lege("08_rohrblatt_rohr", inst.name, notenName(m) + "_bend", ton, a.rate, "Bend nach unten ueber den Vokaltrakt: hoerbar?");
        a.pitchCent = 0.0;
        a.vibratoCent = 25.0;
        rechneRohrblattTon(inst, a, ton, 80000);
        s.lege("08_rohrblatt_rohr", inst.name, notenName(m) + "_vibrato", ton, a.rate, "Vibrato ueber den Druck: glaubhaft?");
        a.vibratoCent = 0.0;
        a.kette = {{0.0, m, hzVon(m)}, {0.5, m + 2, hzVon(m + 2)}, {1.0, m - 3, hzVon(m - 3)}};
        rechneRohrblattTon(inst, a, ton, 80000);
        s.lege("08_rohrblatt_rohr", inst.name, notenName(m) + "_legato", ton, a.rate, "Legato (Moden ueberblendet): Uebergaenge glaubhaft?");
    }
}

void klasse9(hoer::Sammlung& s)
{
    using namespace ic::luftstrahl;
    for (const StrahlSpiel& inst : {baueVorgabeBlockfloete(), baueVorgabeLabialpfeife(), baueVorgabeShakuhachi()})
    {
        for (const int note : {inst.noteMin + 3, (inst.noteMin + inst.noteMax) / 2, inst.noteMax - 4})
            for (const double e : {0.0, 0.5, 1.0})
            {
                StrahlAnschlag a;
                a.kette = {{0.0, note, hzVon(note)}};
                a.expression = e;
                a.haltenS = 1.2;
                std::vector<float> ton;
                rechneStrahlTon(inst, a, ton, static_cast<std::int64_t>(1.4 * a.rate));
                s.lege("09_luftstrahl_rohr", inst.name, notenName(note) + "_e" + std::to_string(static_cast<int>(e * 100)), ton, a.rate,
                       "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? " +
                           (inst.orgel ? "Schweller (e) und Einschwingen glaubhaft?" : "Anblasen und Rauschen glaubhaft?"));
            }
        const int m = (inst.noteMin + inst.noteMax) / 2;
        StrahlAnschlag a;
        a.kette = {{0.0, m, hzVon(m)}};
        a.expression = 0.6;
        a.haltenS = 1.5;
        std::vector<float> ton;
        a.vibratoCent = inst.orgel ? 60.0 : 25.0;
        rechneStrahlTon(inst, a, ton, 80000);
        s.lege("09_luftstrahl_rohr", inst.name, notenName(m) + (inst.orgel ? "_tremulant" : "_vibrato"), ton, a.rate,
               inst.orgel ? "Tremulant: glaubhaft?" : "Vibrato ueber den Druck: glaubhaft?");
        a.vibratoCent = 0.0;
        a.schneidenton = false;
        rechneStrahlTon(inst, a, ton, 80000);
        s.lege("09_luftstrahl_rohr", inst.name, notenName(m) + "_ohne_schneidenton", ton, a.rate,
               "Ohne Schneidenton im Einschwingen (Vergleich mit e60): fehlt etwas?");
    }
}

void klasse10(hoer::Sammlung& s)
{
    using namespace ic::zunge;
    for (const ZungenSpiel& inst : {baueVorgabeAkkordeon(), baueVorgabeHarmonium(), baueVorgabeMelodica(), baueVorgabeMundharmonika()})
    {
        const int unten = std::max(inst.noteMin, inst.bassAb);
        for (const int note : {unten + 3, (unten + inst.noteMax) / 2, inst.noteMax - 4})
            for (const double e : {0.0, 0.5, 1.0})
            {
                ZungenAnschlag a;
                a.note = note;
                a.expression = e;
                a.haltenS = 1.2;
                std::vector<float> ton;
                rechneZungenTon(inst, a, ton, static_cast<std::int64_t>(1.4 * a.rate));
                s.lege("10_zunge", inst.name, notenName(note) + "_e" + std::to_string(static_cast<int>(e * 100)), ton, a.rate,
                       "Klingt es nach " + inst.name + " (" + inst.kennzeichen + ")? Ansprache glaubhaft?");
            }
        const int m = (unten + inst.noteMax) / 2;
        const auto probe = [&](const std::string& name, const std::string& frage, auto&& aendere) {
            ZungenAnschlag a;
            a.note = m;
            a.expression = 0.6;
            a.haltenS = 1.6;
            aendere(a);
            std::vector<float> ton;
            rechneZungenTon(inst, a, ton, 86400);
            s.lege("10_zunge", inst.name, notenName(m) + "_" + name, ton, a.rate, frage);
        };
        probe("vibrato", "Vibrato ueber den Wind: glaubhaft?", [](ZungenAnschlag& a) { a.vibratoCent = 20.0; });
        probe("bend", "Bend (Ventil, gekappt): hoerbar?", [](ZungenAnschlag& a) { a.pitchCent = -80.0; });
        probe("balgwechsel", "Balgwechsel bei 0,8 s: glaubhaft?", [](ZungenAnschlag& a) { a.balgwechselS = 0.8; });
        if (inst.bassAb > 0)
        {
            ZungenAnschlag a;
            a.note = 48;
            a.expression = 0.6;
            a.haltenS = 1.6;
            std::vector<float> ton;
            rechneZungenTon(inst, a, ton, 86400);
            s.lege("10_zunge", inst.name, "bass_C3", ton, a.rate, "Bass-Seite (16′ + 8′): glaubhaft?");
        }
    }
}

void klasse11(hoer::Sammlung& s)
{
    using namespace ic::tonrad;
    const std::vector<OrgelNote> akkord = {{0.05, 2.5, 60, 0.7}, {0.05, 2.5, 64, 0.7}, {0.05, 2.5, 67, 0.7}};
    const auto lege = [&](const OrgelSpiel& o, const std::vector<OrgelNote>& n, const OrgelSpur& sp, const std::string& name, const std::string& frage) {
        std::vector<float> ton;
        rechneOrgel(o, n, sp, ton, 144000);
        s.lege("11_tonrad", o.name == "Tonewheel Organ" ? "Orgel" : "Hausorgel", name, ton, 48000, frage);
    };
    const OrgelSpiel orgel = baueVorgabeOrgel();
    for (const double e : {0.2, 0.6, 1.0})
    {
        OrgelSpur sp;
        sp.schweller = e;
        lege(orgel, akkord, sp, "akkord_e" + std::to_string(static_cast<int>(e * 100)), "Klingt es nach Tonradorgel (888000000, Rotary langsam)? Klick glaubhaft?");
    }
    OrgelSpur hoch;
    hoch.rotarySchnell = {{0.5, true}, {2.0, false}};
    lege(orgel, akkord, hoch, "rotary_hochlauf", "Rotary schnell ab 0,5 s, langsam ab 2 s (Zeitkonstanten offen): glaubhaft?");
    for (const Scanner sc : {Scanner::V3, Scanner::C3})
    {
        OrgelSpiel o = orgel;
        o.scanner = sc;
        lege(o, akkord, OrgelSpur{}, sc == Scanner::V3 ? "scanner_v3" : "scanner_c3", "Scanner-Vibrato/Chorus: glaubhaft?");
    }
    const OrgelSpiel haus = baueVorgabeHausorgel();
    std::vector<OrgelNote> stoesse;
    for (int k = 0; k < 4; ++k)
        for (const int m : {60, 64, 67}) stoesse.push_back({0.05 + 0.6 * k, 0.05 + 0.6 * k + 0.25, m, 0.8});
    lege(haus, stoesse, OrgelSpur{}, "stoesse", "house_organ: kurze Akkorde mit Perkussion — kommt es dem Vorbild nahe (F-5 a)?");
    std::vector<OrgelNote> legato = {{0.05, 1.0, 60, 0.7}, {0.5, 1.5, 64, 0.7}, {1.0, 2.5, 67, 0.7}};
    lege(haus, legato, OrgelSpur{}, "einzeltrigger", "Perkussion mit Einzeltrigger (nur die erste Taste): hoerbar?");
}

}  // namespace

int main(int argc, char** argv)
{
    hoer::Sammlung s;
    s.wurzel = argc > 1 ? argv[1] : "hoertest";
    s.befehl = "ic_hoerproben";
    for (int i = 1; i < argc; ++i) s.befehl += std::string(" ") + argv[i];
    std::set<int> klassen;
    for (int i = 2; i < argc; ++i) klassen.insert(std::atoi(argv[i]));
    const auto will = [&](int k) { return klassen.empty() || klassen.count(k) > 0; };
    if (will(1)) klasse1(s);
    if (will(1)) klasse1Rest(s);
    if (will(2)) klasse2(s);
    if (will(3)) klasse3(s);
    if (will(4)) klasse4(s);
    if (will(5)) klasse5(s);
    if (will(6)) klasse6(s);
    if (will(7)) klasse7(s);
    if (will(8)) klasse8(s);
    if (will(9)) klasse9(s);
    if (will(10)) klasse10(s);
    if (will(11)) klasse11(s);
    for (const hoer::Eintrag& e : hoer::weitere())
        if (will(e.klasse)) e.lege(s);
    s.schreibeListen();
    std::printf("%d Hoerproben in %zu Listen unter %s/listen\n", s.zahl, s.listen.size(), s.wurzel.c_str());
    if (!s.kaputt.empty()) std::printf("%zu Toene mit NaN/Inf ausgelassen (Befund am Modell, siehe stderr)\n", s.kaputt.size());
    return 0;
}
