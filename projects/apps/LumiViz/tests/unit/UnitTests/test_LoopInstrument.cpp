/**
 ****************************************************************************************
 * @file   test_LoopInstrument.cpp
 * @brief  Loop, Stufe 9a: Instrument-Muster — Noten im Modell und in der Datei,
 *         die gerechneten Instrumente, Noten im Rechenkern
 *         (docs/audio/Loop_Konzept.md §4f, Entscheid I1 c)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Audio-Engine: die Toene werden gerechnet und
 * nachgezaehlt. `LUMIVIZ_INSTRUMENT_WAV=<Ordner>` legt Hoerproben ab — je
 * Instrument eine Tonleiter mit Akkord und einen Durchlauf des Beispiels
 * `asset/loop/instrument_demo.lvloop`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "PappTon.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Instrument.hpp"
#include "loop/Kit.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <map>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

/// Qt-Anwendung fuer einen Fall: der Programmordner, von dem aus das Programm
/// `asset/loop` sucht (das Kit und die mitgelieferten Instrumente).
struct InstrumentApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

using looptest::klaengeOhneEngine;
using looptest::liesWav;
using looptest::repoWurzel;

/// Ein Loop mit einem Instrument-Muster von vier Takten (64 Schritte) in einem Kanal.
Loop notenLoop(std::vector<Note> noten, const QString& instrument = QStringLiteral("organ"))
{
    Loop loop;
    loop.laut = 1.0;
    Muster m;
    m.name = QStringLiteral("M");
    m.art = MusterArt::Instrument;
    m.instrument = instrument;
    m.noten = std::move(noten);
    loop.muster = {m};
    loop.kanaele = {{QStringLiteral("K"), true, {{QStringLiteral("M"), 0, 1}}}};
    loop.richte();
    return loop;
}

float spitze(const std::vector<float>& daten)
{
    float groesster = 0.0f;
    for (const float wert : daten) groesster = std::max(groesster, std::abs(wert));
    return groesster;
}

/// Die Periode eines Tons in Abtastwerten, aus der Autokorrelation eines
/// Ausschnitts — die Verschiebung mit der groessten Uebereinstimmung.
int periode(const Klang& klang, int ab, int laenge, int von, int bis)
{
    int beste = von;
    double bestWert = -1e30;
    for (int lag = von; lag <= bis; ++lag)
    {
        double summe = 0.0;
        for (int i = 0; i < laenge; ++i)
            summe += static_cast<double>(klang.daten[static_cast<std::size_t>(2 * (ab + i))]) *
                     klang.daten[static_cast<std::size_t>(2 * (ab + i + lag))];
        if (summe > bestWert)
        {
            bestWert = summe;
            beste = lag;
        }
    }
    return beste;
}

/// Stereo-Werte als WAV mit 16 Bit schreiben (fuer die Hoerproben).
bool schreibeWav(const QString& pfad, const std::vector<float>& daten, int rate)
{
    QFile datei(pfad);
    if (!datei.open(QIODevice::WriteOnly)) return false;
    const auto bytes = static_cast<std::uint32_t>(daten.size() * 2);
    const auto u32 = [&datei](std::uint32_t wert) { datei.write(reinterpret_cast<const char*>(&wert), 4); };
    const auto u16 = [&datei](std::uint16_t wert) { datei.write(reinterpret_cast<const char*>(&wert), 2); };
    datei.write("RIFF");
    u32(36 + bytes);
    datei.write("WAVEfmt ");
    u32(16);
    u16(1);
    u16(2);
    u32(static_cast<std::uint32_t>(rate));
    u32(static_cast<std::uint32_t>(rate) * 4);
    u16(4);
    u16(16);
    datei.write("data");
    u32(bytes);
    for (const float wert : daten)
    {
        const auto ganz = static_cast<std::int16_t>(std::lround(std::clamp(wert, -1.0f, 1.0f) * 32767.0f));
        datei.write(reinterpret_cast<const char*>(&ganz), 2);
    }
    return true;
}

}  // namespace

TEST_SUITE("LoopInstrument")
{
    TEST_CASE("Loop Instrument: Noten im Modell — richte und die Namen der Toene und die Datei")
    {
        // --- die Namen der Toene
        CHECK(tonKlang(QStringLiteral("piano"), 60, 500) == QStringLiteral("inst:piano:60:500"));
        CHECK(istTon(QStringLiteral("inst:piano:60:500")));
        CHECK_FALSE(istTon(QStringLiteral("kit:kick")));
        QString instrument;
        int hoehe = 0;
        int ms = 0;
        REQUIRE(zerlegeTon(QStringLiteral("inst:bass:33:125"), instrument, hoehe, ms));
        CHECK(instrument == QStringLiteral("bass"));
        CHECK(hoehe == 33);
        CHECK(ms == 125);
        CHECK_FALSE(zerlegeTon(QStringLiteral("inst:bass:33"), instrument, hoehe, ms));
        CHECK_FALSE(zerlegeTon(QStringLiteral("inst:bass:x:125"), instrument, hoehe, ms));
        CHECK_FALSE(zerlegeTon(QStringLiteral("inst:bass:33:0"), instrument, hoehe, ms));
        CHECK_FALSE(zerlegeTon(QStringLiteral("kit:kick"), instrument, hoehe, ms));

        // --- richte: in die Grenzen, sortiert, je Schritt und Tonhoehe eine, nichts ueber das Ende
        Loop loop = notenLoop({{8, 4, 64, 1.0},
                               {0, 4, 60, 0.8},
                               {0, 2, 60, 0.5},      // dieselbe Stelle und Tonhoehe: die erste bleibt
                               {0, 4, 200, 1.0},     // zu hoch: der oberste Ton (C8)
                               {4, 0, 10, 2.0},      // Laenge 0, zu tief, zu laut
                               {60, 16, 62, 1.0},    // reicht ueber das Ende: wird kuerzer
                               {64, 4, 62, 1.0},     // beginnt hinter dem Ende: geht
                               {-1, 4, 62, 1.0},     // vor dem Anfang: geht
                               {12, 4, 62, 0.0}},    // ohne Lautstaerke: geht
                              QString());
        const Muster& m = loop.muster[0];
        CHECK(m.instrument == QStringLiteral("piano"));  // ohne Angabe das erste der Auswahl
        REQUIRE(m.noten.size() == 5);
        CHECK(m.noten[0] == Note{0, 4, 60, 0.8});
        CHECK(m.noten[1] == Note{0, 4, kHoeheMax, 1.0});
        CHECK(m.noten[2] == Note{4, 1, kHoeheMin, 1.0});
        CHECK(m.noten[3] == Note{8, 4, 64, 1.0});
        CHECK(m.noten[4] == Note{60, 4, 62, 1.0});
        CHECK(m.spuren.empty());
        CHECK(m.zeilen.empty());

        // --- die Klaenge: je Tonhoehe und Dauer einer; 120 BPM, ein Schritt sind 125 ms
        CHECK(loop.notenMs(m.noten[0]) == 500);
        CHECK(loop.notenMs(m.noten[2]) == 125);
        CHECK(loop.klangNamen() == QStringList{QStringLiteral("inst:piano:60:500"), tonKlang(QStringLiteral("piano"), kHoeheMax, 500),
                                               tonKlang(QStringLiteral("piano"), kHoeheMin, 125), QStringLiteral("inst:piano:64:500"),
                                               QStringLiteral("inst:piano:62:500")});
        CHECK(tonKlang(QStringLiteral("piano"), 60, 500) == QStringLiteral("inst:piano:60:500"));
        // die Dauer haengt am Tempo
        loop.bpm = 60.0;
        CHECK(loop.klangNamen().first() == QStringLiteral("inst:piano:60:1000"));
        loop.bpm = 120.0;

        // --- die Datei: hin und zurueck, neben einem Drumkit-Muster
        Muster drums;
        drums.name = QStringLiteral("D");
        drums.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
        loop.muster.push_back(drums);
        loop.muster[0].laut = 0.8;
        loop.muster[0].pan = -0.25;
        loop.muster[0].stumm = true;
        loop.kanaele[0].clips.push_back({QStringLiteral("D"), 4, 1});
        loop.richte();
        const QJsonObject json = alsJson(loop);
        const QJsonObject erstes = json.value(QStringLiteral("patterns")).toArray().at(0).toObject();
        CHECK(erstes.value(QStringLiteral("kind")).toString() == QStringLiteral("instrument"));
        CHECK(erstes.value(QStringLiteral("instrument")).toString() == QStringLiteral("piano"));
        CHECK(erstes.value(QStringLiteral("notes")).toArray().size() == 5);
        CHECK_FALSE(erstes.contains(QStringLiteral("tracks")));
        const QJsonObject zweites = json.value(QStringLiteral("patterns")).toArray().at(1).toObject();
        CHECK(zweites.value(QStringLiteral("kind")).toString() == QStringLiteral("drumkit"));
        CHECK_FALSE(zweites.contains(QStringLiteral("notes")));

        Loop gelesen;
        QStringList bericht;
        REQUIRE(ausJson(json, gelesen, {}, &bericht));
        CHECK(bericht.isEmpty());
        CHECK(gelesen == loop);

        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString pfad = ordner.filePath(QStringLiteral("noten.lvloop"));
        REQUIRE(speichereLoop(loop, pfad));
        Loop ausDatei;
        REQUIRE(ladeLoop(pfad, ausDatei));
        CHECK(ausDatei == loop);
        // je Note eine Zeile
        QFile datei(pfad);
        REQUIRE(datei.open(QIODevice::ReadOnly));
        CHECK(datei.readAll().contains("[0, 4, 60, 0.8]"));
    }

    TEST_CASE("Loop Instrument: neuer Instrument-Clip — und das Raster nimmt die Noten mit")
    {
        Loop loop;
        (void)kanalHinzu(loop);
        REQUIRE(neuerClip(loop, 0, 0, MusterArt::Instrument) == 0);
        REQUIRE(loop.muster.size() == 1);
        CHECK(loop.muster[0].art == MusterArt::Instrument);
        CHECK(loop.muster[0].instrument == QStringLiteral("piano"));
        CHECK(loop.muster[0].takte == kTakteNeu);
        CHECK(loop.muster[0].noten.empty());
        REQUIRE(neuerClip(loop, 0, 4) == 1);  // die Vorgabe bleibt das Drumkit
        CHECK(loop.muster[1].art == MusterArt::Drumkit);

        // 4 Schritte je Schlag: Einsatz auf dem Schlag, auf der Haelfte, auf einem Viertel
        Muster* m = musterIn(loop, loop.muster[0].name);
        REQUIRE(m != nullptr);
        m->noten = {{0, 4, 60, 1.0}, {6, 2, 62, 1.0}, {9, 1, 64, 1.0}, {16, 8, 65, 1.0}};
        loop.richte();

        // auf 2 Schritte je Schlag: das Viertel gibt es nicht mehr, die Laengen halbieren sich
        Loop grob = loop;
        REQUIRE(setzeRaster(grob, 4, 2));
        const std::vector<Note>& g = grob.muster[0].noten;
        REQUIRE(g.size() == 3);
        CHECK(g[0] == Note{0, 2, 60, 1.0});
        CHECK(g[1] == Note{3, 1, 62, 1.0});
        CHECK(g[2] == Note{8, 4, 65, 1.0});

        // auf 8 Schritte je Schlag: alles bleibt, die Laengen verdoppeln sich
        Loop fein = loop;
        REQUIRE(setzeRaster(fein, 4, 8));
        const std::vector<Note>& f = fein.muster[0].noten;
        REQUIRE(f.size() == 4);
        CHECK(f[1] == Note{12, 4, 62, 1.0});
        CHECK(f[2] == Note{18, 2, 64, 1.0});
        CHECK(f[3] == Note{32, 16, 65, 1.0});

        // 3 Schlaege je Takt: was im vierten Schlag lag, geht; der zweite Takt beginnt bei Schritt 12
        Loop drei = loop;
        m = musterIn(drei, drei.muster[0].name);
        m->noten.push_back({13, 2, 67, 1.0});  // vierter Schlag des ersten Takts
        drei.richte();
        REQUIRE(setzeRaster(drei, 3, 4));
        const std::vector<Note>& d = drei.muster[0].noten;
        REQUIRE(d.size() == 4);
        CHECK(d[3] == Note{12, 8, 65, 1.0});
    }

    TEST_CASE("Loop Instrument: der Rechenkern spielt Noten als Stimmen — ein Akkord klingt zusammen")
    {
        // ein Akkord aus drei Toenen auf Schritt 0, ein vierter Ton auf Schritt 8, der erste noch einmal auf 16
        Loop loop = notenLoop({{0, 4, 60, 1.0}, {0, 4, 64, 0.5}, {0, 4, 67, 1.0}, {8, 2, 72, 1.0}, {16, 4, 60, 1.0}});
        loop.muster[0].laut = 0.5;
        loop.muster[0].pan = 0.5;
        const Ablauf ablauf = plane(loop, kRate);
        CHECK(ablauf.klangNamen.size() == 4);  // 60 kommt zweimal vor, mit derselben Dauer
        REQUIRE(ablauf.stimmen.size() == 4);
        // die Stimme des Tons 60: zwei Schlaege, 16 Schritte auseinander
        const Stimme& erste = ablauf.stimmen[0];
        CHECK(ablauf.klangNamen[erste.klang] == QStringLiteral("inst:organ:60:500"));
        REQUIRE(erste.schlaege.size() == 2);
        CHECK(erste.schlaege[0].frame == 0);
        CHECK(erste.schlaege[1].frame == std::llround(16 * ablauf.schrittFrames));
        // Lautstaerke: Note mal Muster mal Loop; Panorama nach rechts nimmt links zurueck
        CHECK(erste.schlaege[0].links == doctest::Approx(0.25f));
        CHECK(erste.schlaege[0].rechts == doctest::Approx(0.5f));
        CHECK(ablauf.stimmen[1].schlaege[0].rechts == doctest::Approx(0.25f));
        CHECK(ablauf.stimmen[3].schlaege[0].frame == std::llround(8 * ablauf.schrittFrames));
        CHECK(ablauf.durchlaufFrames == std::llround(64 * ablauf.schrittFrames));

        // rechne: mit Klaengen aus lauter Einsen addieren sich die drei Toene des Akkords
        std::vector<Klang> klaenge(4);
        for (Klang& k : klaenge) k.daten.assign(2 * 1000, 1.0f);
        std::vector<float> block(2 * 100);
        rechne(ablauf, klaenge, 0, 100, block.data());
        CHECK(block[0] == doctest::Approx(0.25f + 0.125f + 0.25f));  // links
        CHECK(block[1] == doctest::Approx(0.5f + 0.25f + 0.5f));     // rechts
        // nach dem Ende der Klaenge ist es still, bis der naechste Ton kommt
        rechne(ablauf, klaenge, 2000, 100, block.data());
        CHECK(block[0] == doctest::Approx(0.0f));

        // ein stummes Muster hat keine Stimme, der Durchlauf bleibt gleich lang
        loop.muster[0].stumm = true;
        const Ablauf stumm = plane(loop, kRate);
        CHECK(stumm.stimmen.empty());
        CHECK(stumm.durchlaufFrames == ablauf.durchlaufFrames);

        // Swing verschiebt den Einsatz auf einem ungeraden Schritt
        Loop geswungen = notenLoop({{1, 1, 60, 1.0}});
        geswungen.swing = 1.0;
        const Ablauf s = plane(geswungen, kRate);
        REQUIRE(s.stimmen.size() == 1);
        CHECK(s.stimmen[0].schlaege[0].frame == std::llround(1.5 * s.schrittFrames));
    }

    TEST_CASE("Loop Instrument: jeder gerechnete Ton hat seine Tonhoehe und seine Laenge und endet bei null")
    {
        CHECK(tonHz(69) == doctest::Approx(440.0));
        CHECK(tonHz(57) == doctest::Approx(220.0));
        CHECK(tonName(60) == QStringLiteral("C4"));
        CHECK(tonName(61) == QStringLiteral("C#4"));
        CHECK(tonName(kHoeheMin) == QStringLiteral("A0"));  // seit S132 (S-T1 a) der Umfang eines Klaviers; bis dahin C1
        CHECK(tonName(kHoeheMax) == QStringLiteral("C8"));  // bis dahin C7

        // die mitgelieferten Instrumente: die Dateien unter asset/loop/instruments, nach Titel sortiert
        const InstrumentApp app;  // der Programmordner fuer die Suche nach dem Ordner
        CHECK(QDir(instrumentOrdner()).dirName() == QStringLiteral("instruments"));
        const std::vector<Instrument>& alle = instrumente();
        // sieben allgemeine und sieben fuer House (Wunsch Patrik S110), dazu sechzehn aus
        // Schichten, sieben davon ungewoehnlich, und neunzehn fuer die Genre-Beispiele
        // (Wunsch Patrik S111), sechs gerechnete Drums (Stufe 11f, S117); S123: elf aus Formen,
        // dreizehn aus mitgelieferten Samples, elf Drums mehr (sechs gerechnet, fuenf aus Samples)
        // und drei Besen (S124, Stufe 21d)
        REQUIRE(alle.size() == 107);  // S133 (Stufe 23g): acht alte als <name>_old, sechs neue (S-G3 a)
        for (std::size_t i = 1; i < alle.size(); ++i)
            CHECK(alle[i - 1].titel.compare(alle[i].titel, Qt::CaseInsensitive) <= 0);
        CHECK(alle.front().titel == QStringLiteral("808 Bass"));  // Ziffern stehen vor Buchstaben
        CHECK(alle.back().titel == QStringLiteral("Wind Voices"));
        for (const char* name : {"bass", "guitar", "lead", "organ", "piano", "strings", "violin", "acid_bass",
                                 "chord_stab", "house_organ", "house_piano", "pluck", "sub_bass", "warm_pad",
                                 "bell", "tine_piano", "brass", "flute", "super_saw", "fifth_pad", "overdrive_guitar",
                                 "kalimba", "marimba", "cascade", "siren", "alien_choir", "broken_radio", "ghost",
                                 "growl_bass", "reverse_swell",
                                 "trance_lead", "trance_pluck", "rolling_bass", "hoover", "clavinet", "slap_bass",
                                 "clean_guitar", "muted_guitar", "round_bass", "bass_808", "reese_bass", "upright_bass",
                                 "nylon_guitar", "sax", "vibraphone", "steel_drum", "cowbell", "conga", "clave",
                                 "drum_kick", "drum_tom", "drum_snare", "drum_clap", "drum_hat_closed", "drum_hat_open",
                                 "shape_heart_pad", "shape_rings_choir", "shape_infinity_drone", "shape_gear_bass",
                                 "shape_moon_bass", "shape_star_lead", "shape_bolt_pluck", "shape_flower_flute",
                                 "shape_smiley_keys", "shape_atom_bell", "shape_cat",
                                 "female_choir", "hum_pad", "vocal_lead", "vocal_chop", "harp", "koto",
                                 "dulcimer", "thumb_piano", "shakuhachi", "alto_sax", "alto_flute", "angel_harp",
                                 "wind_voices", "drum_kick_808", "drum_tom_low", "drum_tom_high", "drum_shaker",
                                 "drum_ride", "drum_crash", "drum_kick_punch", "drum_snare_fat", "drum_clap_stack",
                                 "drum_hat_shake", "drum_beatbox_kick", "drum_brush_tap", "drum_brush_slap",
                                 "drum_brush_sweep"})
        {
            CAPTURE(name);
            const Instrument* gefunden = findeInstrument(QString::fromLatin1(name));
            REQUIRE(gefunden != nullptr);
            CHECK(gefunden->name == QString::fromLatin1(name));  // ein mitgeliefertes heisst wie seine Datei
            CHECK_FALSE(gefunden->titel.isEmpty());
        }
        CHECK(findeInstrument(QStringLiteral("house_piano"))->titel == QStringLiteral("House Piano"));
        CHECK(findeInstrument(QStringLiteral("tuba")) == nullptr);
        CHECK(findeInstrument(instrumentVorgabe()) != nullptr);  // das Instrument eines neuen Musters gibt es
        CHECK(findeInstrument(QStringLiteral("guitar"))->schichten[0].schwingung == Schwingung::Saite);
        CHECK(findeInstrument(QStringLiteral("strings"))->schichten[0].stimmen == 3);

        for (const Instrument& instrument : alle)
        {
            CAPTURE(instrument.name.toStdString());
            CHECK(findeInstrument(instrument.name) == &instrument);
            // eine bis vier Schichten, alle an; die erste liegt auf der Note und setzt gleich ein
            REQUIRE_FALSE(instrument.schichten.empty());
            CHECK(instrument.schichten.size() <= static_cast<std::size_t>(kSchichtenMax));
            // (die sechs Drums aus Stufe 11f nicht: sie sind fuer die Note C2 gebaut und tragen, wohin
            // der Schlag gehoert, als Transpose — ihre Tonhoehe prueft der Fall „Loop Instrument 11f")
            // (S128, Entscheide Patrik C2 a und C2-G a: Cowbell und Clave sind Aufnahmen, ihr Grundton liegt auf der
            // Note der Loops — 81 und 84 —, nicht auf der Tonhoehe des Schlags; die Werte haelt test_LoopWerte fest)
            const bool schlag = instrument.art == QStringLiteral("drums") || instrument.name == QStringLiteral("cowbell") ||
                                instrument.name == QStringLiteral("clave");
            if (!schlag) CHECK(instrument.schichten[0].versatzHalbtoene == 0);
            CHECK(instrument.schichten[0].einsatzMs == 0.0);
            double laengsterAusklang = 0.0;
            double vibrato = 0.0;
            for (const Schicht& schicht : instrument.schichten)
            {
                CHECK(schicht.an);
                laengsterAusklang = std::max(laengsterAusklang, schicht.ausklangMs);
                vibrato = std::max(vibrato, schicht.vibratoCent);
            }
            // A4, eine halbe Sekunde gehalten: so lang wie die Note und der laengste Ausklang;
            // Sample-Schichten (S123) bekommen ihre Aufnahme vorher aus der WAV-Datei
            looptest::merkeSamplesOhneEngine(instrument);
            const Klang ton = rendereTon(instrument, 69, 500.0, kRate);
            const std::int64_t ausklang = std::llround(std::max(5.0, laengsterAusklang) * kRate / 1000.0);
            CHECK(ton.frames() == 22050 + ausklang);
            const float groesster = spitze(ton.daten);
            MESSAGE(instrument.titel.toStdString() << ": A4, " << ton.frames() << " Abtastwerte, Spitze "
                                                   << std::to_string(groesster));
            CHECK(groesster > 0.05f);
            // Seit Stufe 27c steht das Volume der Instrumente auf dem Bezug (LL2 a; die Lautheit haelt
            // test_LoopPegel.cpp fest), der Schutz vor Uebersteuern sitzt am Ausgang (LL7 a) — eine
            // Spitze ueber 0,75 ist dort kein Fehler mehr. Die Drums bleiben bis Stufe 27d, wie sie sind.
            const bool drum = instrument.art == QLatin1String("drums");
            if (drum) CHECK(groesster <= 0.75f);
            bool endlich = true;
            bool beideGleich = true;
            for (std::size_t i = 0; i + 1 < ton.daten.size(); i += 2)
            {
                if (!std::isfinite(ton.daten[i])) endlich = false;
                if (ton.daten[i] != ton.daten[i + 1]) beideGleich = false;
            }
            CHECK(endlich);
            // beide Seiten gleich — ausser eine Schicht steht mit Pan zur Seite oder spielt Stereo XY
            // (S123: Instrumente aus Formen, etwa Rings Choir und Atom Bell)
            const bool stereo = std::any_of(instrument.schichten.begin(), instrument.schichten.end(),
                                            [](const Schicht& s) { return s.pan != 0.0 || s.stereoXY; });
            CHECK(beideGleich == !stereo);
            // der Klang endet bei null: kein Knacken am Ende
            CHECK(std::abs(ton.daten[ton.daten.size() - 2]) < 0.002f);
            // die Tonhoehe: 440 Hz sind 100,2 Abtastwerte je Schwingung — mit Vibrato schwankt
            // sie um so viele Cent, wie das Instrument angibt (die Sirene: 100)
            const double schwankung = std::exp2(std::max(vibrato, 20.0) / 1200.0);
            if (!schlag)
            {
                const int p = periode(ton, 6000, 4000, 80, 125);
                CHECK(p >= std::floor(100.2 / schwankung) - 1);
                CHECK(p <= std::ceil(100.2 * schwankung) + 1);
                // eine Oktave tiefer die doppelte Periode
                const Klang tief = rendereTon(instrument, 57, 500.0, kRate);
                const int pTief = periode(tief, 6000, 4000, 160, 250);
                CHECK(pTief >= std::floor(200.5 / schwankung) - 1);
                CHECK(pTief <= std::ceil(200.5 * schwankung) + 1);
            }
            // derselbe Aufruf ergibt dieselben Werte
            CHECK(rendereTon(instrument, 69, 500.0, kRate).daten == ton.daten);
            // die Raender des Umfangs und eine andere Abtastrate rechnen ohne Unsinn
            for (const int hoehe : {kHoeheMin, kHoeheMax})
            {
                const Klang rand = rendereTon(instrument, hoehe, 200.0, 48000);
                CHECK(rand.frames() > 9600);
                bool randEndlich = true;
                for (const float wert : rand.daten)
                    if (!std::isfinite(wert)) randEndlich = false;
                CHECK(randEndlich);
                if (drum) CHECK(spitze(rand.daten) <= 0.9f);
            }
        }
        // eine laengere Note ist ein laengerer Klang; ohne Dauer oder Rate gibt es keinen
        CHECK(rendereTon(alle[0], 60, 1000.0, kRate).frames() > rendereTon(alle[0], 60, 500.0, kRate).frames());
        CHECK(rendereTon(alle[0], 60, 0.0, kRate).frames() == 0);
        CHECK(rendereTon(alle[0], 60, 500.0, 0).frames() == 0);
        // hoechstens `kTonMaxSekunden`
        CHECK(rendereTon(*findeInstrument(QStringLiteral("organ")), 60, 60000.0, 8000).frames() ==
              static_cast<std::int64_t>(kTonMaxSekunden * 8000));
    }

    TEST_CASE("Loop Instrument: die Datei .lvinst — hin und zurueck — und ein Muster nennt sie beim Pfad")
    {
        const InstrumentApp app;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());

        // --- jedes mitgelieferte Instrument uebersteht JSON und Datei
        for (const Instrument& instrument : instrumente())
        {
            CAPTURE(instrument.name.toStdString());
            Instrument gelesen;
            gelesen.name = instrument.name;
            REQUIRE(ausJson(alsJson(instrument), gelesen));
            CHECK(gelesen == instrument);
        }
        Instrument eigenes = *findeInstrument(QStringLiteral("organ"));
        eigenes.titel = QStringLiteral("My organ");
        eigenes.schichten[0].obertoene = {1.0, 0.0, 0.5};
        eigenes.schichten[0].ausklangMs = 300.0;
        const QString pfad = ordner.filePath(QStringLiteral("my_organ.lvinst"));
        const std::uint64_t fassung = instrumentFassung();
        REQUIRE(speichereInstrument(eigenes, pfad));
        CHECK(instrumentFassung() == fassung + 1);  // wer Toene gerechnet hat, sieht: neu rechnen
        Instrument ausDatei;
        REQUIRE(ladeInstrument(pfad, ausDatei));
        CHECK(ausDatei.name == pfad);  // eine eigene Datei heisst wie ihr Pfad
        ausDatei.name = eigenes.name;
        CHECK(ausDatei == eigenes);

        // --- Werte ausserhalb kommen in ihre Grenzen; ein fremdes Format ist kein Instrument
        QJsonObject wild = alsJson(eigenes);
        QJsonObject wildeSchicht = wild.value(QStringLiteral("layers")).toArray().at(0).toObject();
        wildeSchicht[QStringLiteral("voices")] = 9;
        wildeSchicht[QStringLiteral("sustain")] = 5.0;
        wildeSchicht[QStringLiteral("waveform")] = QStringLiteral("gibt es nicht");
        wildeSchicht[QStringLiteral("transpose")] = 99;
        wildeSchicht[QStringLiteral("startOffset")] = -5.0;
        // seit Fassung 7 steht Drive in der Reihe `path`
        wildeSchicht[QStringLiteral("path")] =
            QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("drive")}, {QStringLiteral("amount"), 7.0}},
                       QJsonObject{{QStringLiteral("type"), QStringLiteral("envelope")}}};
        wild[QStringLiteral("layers")] = QJsonArray{wildeSchicht};
        wild[QStringLiteral("level")] = 99.0;  // seit Stufe 27c bis +24 dB (LL6 a)
        Instrument gezaehmt;
        REQUIRE(ausJson(wild, gezaehmt));
        REQUIRE(gezaehmt.schichten.size() == 1);
        CHECK(gezaehmt.laut == doctest::Approx(kInstrumentLautMax));
        CHECK(gezaehmt.schichten[0].stimmen == 3);
        CHECK(gezaehmt.schichten[0].halten == doctest::Approx(1.0));
        CHECK(gezaehmt.schichten[0].schwingung == Schwingung::Sinus);  // die Vorgabe
        CHECK(gezaehmt.schichten[0].versatzHalbtoene == 24);
        CHECK(gezaehmt.schichten[0].einsatzMs == doctest::Approx(0.0));
        CHECK(gezaehmt.schichten[0].drive().wert == doctest::Approx(1.0));
        QJsonObject fremd = alsJson(eigenes);
        fremd[QStringLiteral("format")] = QStringLiteral("lumiviz-loop");
        CHECK_FALSE(ausJson(fremd, gezaehmt));
        CHECK_FALSE(ladeInstrument(ordner.filePath(QStringLiteral("fehlt.lvinst")), gezaehmt));

        // --- holen: ein mitgeliefertes beim Namen, ein eigenes beim Pfad
        CHECK(istInstrumentDatei(pfad));
        CHECK_FALSE(istInstrumentDatei(QStringLiteral("piano")));
        Instrument geholt;
        REQUIRE(holeInstrument(QStringLiteral("piano"), geholt));
        CHECK(geholt.titel == QStringLiteral("Piano"));
        REQUIRE(holeInstrument(pfad, geholt));
        CHECK(geholt.titel == QStringLiteral("My organ"));
        CHECK_FALSE(holeInstrument(QStringLiteral("tuba"), geholt));
        CHECK_FALSE(holeInstrument(ordner.filePath(QStringLiteral("fehlt.lvinst")), geholt));

        // --- der Name eines Tons traegt den Pfad, auch mit Doppelpunkt
        const QString ton = tonKlang(pfad, 60, 250);
        QString instrument;
        int hoehe = 0;
        int ms = 0;
        REQUIRE(zerlegeTon(ton, instrument, hoehe, ms));
        CHECK(instrument == pfad);
        CHECK(hoehe == 60);
        CHECK(ms == 250);

        // --- im Loop: der Pfad steht relativ zur Datei, wie der eines Samples
        Loop loop = notenLoop({{0, 4, 60, 1.0}}, pfad);
        CHECK(loop.klangNamen() == QStringList{tonKlang(pfad, 60, 500)});
        const QJsonObject json = alsJson(loop, ordner.path());
        CHECK(json.value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("instrument")).toString() ==
              QStringLiteral("my_organ.lvinst"));
        Loop gelesen;
        QStringList bericht;
        REQUIRE(ausJson(json, gelesen, ordner.path(), &bericht));
        CHECK(bericht.isEmpty());
        CHECK(QFileInfo(gelesen.muster[0].instrument) == QFileInfo(pfad));
        // fehlt die Datei, steht es im Bericht
        REQUIRE(QFile::remove(pfad));
        bericht.clear();
        REQUIRE(ausJson(json, gelesen, ordner.path(), &bericht));
        REQUIRE(bericht.size() == 1);
        CHECK(bericht[0].startsWith(QStringLiteral("File not found: ")));
    }

    TEST_CASE("Loop Instrument: Fassung 3 — Art, Genres und Beschreibung; aeltere Dateien lesen sich ohne sie")
    {
        // --- die acht Arten, in der Reihenfolge des Fensters; was es nicht gibt, ist „Unusual"
        const auto& arten = instrumentArten();
        REQUIRE(arten.size() == 9);  // seit S115 mit „Drums“ (K3 a; test_LoopDrumQuellen.cpp)
        CHECK(QLatin1String(arten.front().schluessel) == QLatin1String("keys"));
        CHECK(QLatin1String(arten.back().schluessel) == QLatin1String("unusual"));
        CHECK(gueltigeArt(QStringLiteral("bass")) == QStringLiteral("bass"));
        CHECK(gueltigeArt(QStringLiteral("BASS")) == QStringLiteral("bass"));
        CHECK(gueltigeArt(QString()) == QStringLiteral("unusual"));
        CHECK(gueltigeArt(QStringLiteral("tuba section")) == QStringLiteral("unusual"));
        CHECK(artTitel(QStringLiteral("pads")) == QStringLiteral("Pads and strings"));
        CHECK(artTitel(QStringLiteral("gibt es nicht")) == QStringLiteral("Unusual"));

        // --- hin und zurueck
        Instrument eins;
        eins.titel = QStringLiteral("My bass");
        eins.art = QStringLiteral("bass");
        eins.genres = {QStringLiteral("Funk"), QStringLiteral("Disco")};
        eins.beschreibung = QStringLiteral("A plucked string with a thump.");
        const QJsonObject json = alsJson(eins);
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);  // geschrieben wird die juengste (seit S116: 5)
        CHECK(json.value(QStringLiteral("category")).toString() == QStringLiteral("bass"));
        CHECK(json.value(QStringLiteral("genres")).toArray().size() == 2);
        CHECK(json.value(QStringLiteral("description")).toString() == eins.beschreibung);
        Instrument gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen == eins);

        // --- die Felder duerfen fehlen; Genres ohne Doppelte und ohne Leeres; eine fremde Art bleibt stehen
        QJsonObject karg = json;
        karg.remove(QStringLiteral("category"));
        karg.remove(QStringLiteral("genres"));
        karg.remove(QStringLiteral("description"));
        REQUIRE(ausJson(karg, gelesen));
        CHECK(gelesen.art.isEmpty());
        CHECK(gelesen.genres.isEmpty());
        CHECK(gelesen.beschreibung.isEmpty());
        CHECK(gueltigeArt(gelesen.art) == QStringLiteral("unusual"));
        QJsonObject unsauber = json;
        unsauber[QStringLiteral("genres")] =
            QJsonArray{QStringLiteral(" Funk "), QStringLiteral("funk"), QString(), 7, QStringLiteral("House")};
        unsauber[QStringLiteral("category")] = QStringLiteral("tuba section");
        REQUIRE(ausJson(unsauber, gelesen));
        CHECK(gelesen.genres == QStringList{QStringLiteral("Funk"), QStringLiteral("House")});
        CHECK(gelesen.art == QStringLiteral("tuba section"));

        // --- eine Datei der Fassung 2 (ohne die Felder) liest sich wie bisher
        QJsonObject zwei = karg;
        zwei[QStringLiteral("version")] = 2;
        REQUIRE(ausJson(zwei, gelesen));
        CHECK(gelesen.titel == QStringLiteral("My bass"));
        CHECK(gelesen.schichten == eins.schichten);

        // --- jedes mitgelieferte Instrument traegt eine der acht Arten und einen Satz
        const InstrumentApp app;
        int ohneGenre = 0;
        std::map<QString, int> jeArt;
        for (const Instrument& instrument : instrumente())
        {
            CAPTURE(instrument.name.toStdString());
            CHECK(gueltigeArt(instrument.art) == instrument.art);
            CHECK(instrument.beschreibung.size() > 10);
            CHECK(instrument.beschreibung.endsWith(QLatin1Char('.')));
            if (instrument.genres.isEmpty()) ++ohneGenre;
            ++jeArt[instrument.art];
        }
        // nach dem Titel zugeordnet (asset/loop/instrument_katalog.py) — wer eines dazulegt, stellt die Zahl nach
        // (S123: aus Formen, aus Samples und elf Drums dazu)
        CHECK(jeArt[QStringLiteral("keys")] == 9);  // S133: clavinet_old
        CHECK(jeArt[QStringLiteral("bass")] == 15);  // S133: slap_bass_old, upright_bass_old, finger_bass
        CHECK(jeArt[QStringLiteral("lead")] == 7);
        CHECK(jeArt[QStringLiteral("pads")] == 9);
        CHECK(jeArt[QStringLiteral("plucked")] == 22);  // S133: fuenf Gitarren *_old, fuenf neue
        CHECK(jeArt[QStringLiteral("wind")] == 7);
        CHECK(jeArt[QStringLiteral("percussion")] == 10);
        CHECK(jeArt[QStringLiteral("drums")] == 20);  // Stufe 11f, S123, drei Besen S124
        CHECK(jeArt[QStringLiteral("unusual")] == 8);
        // die Genres sind aus den mitgelieferten Loops gezaehlt: Violin steht in keinem (Guitar bis S136,
        // seither in strum_demo), die Drums auch nicht (gezaehlt werden nur Instrument-Muster, keine
        // Drum-Spuren), Flower Flute (S123) auch nicht
        CHECK(ohneGenre == 29);  // seit S124 mit den drei Besen, seit S133 die acht *_old (kein Loop nutzt sie)
        CHECK(findeInstrument(QStringLiteral("slap_bass"))->genres == QStringList{QStringLiteral("Funk")});
        // seit S138 (26h) spielt die Guitar auch in tabs_demo (Blues)
        CHECK(findeInstrument(QStringLiteral("guitar"))->genres == QStringList{QStringLiteral("Blues"), QStringLiteral("Folk")});
        CHECK(findeInstrument(QStringLiteral("violin"))->genres.isEmpty());
    }

    TEST_CASE("Loop Instrument: eigene im Benutzerordner — user:<name>, ein eigener Raum (P2 a)")
    {
        const InstrumentApp app;
        QTemporaryDir tmp;
        REQUIRE(tmp.isValid());
        const QString ordner = QDir(tmp.path()).filePath(QStringLiteral("instruments"));
        setzeEigenenInstrumentOrdnerFuerTests(ordner);
        struct Zurueck
        {
            ~Zurueck() { setzeEigenenInstrumentOrdnerFuerTests(QString()); }
        } zurueck;

        // --- der Name: klein, a–z, Ziffern, `_`
        CHECK(eigenerName(QStringLiteral("Mein Bass 2")) == QStringLiteral("mein_bass_2"));
        CHECK(eigenerName(QStringLiteral("  Glühen -- (warm)  ")) == QStringLiteral("gl_hen_warm"));
        CHECK(eigenerName(QStringLiteral("../../raus")) == QStringLiteral("raus"));
        CHECK(eigenerName(QStringLiteral("???")).isEmpty());
        CHECK(istEigenerName(QStringLiteral("mein_bass_2")));
        for (const char* nein : {"", "Mein Bass", "a/b", "..", "a..b", "_a", "a_", "a__b", "c:x"})
        {
            CAPTURE(nein);
            CHECK_FALSE(istEigenerName(QString::fromLatin1(nein)));
        }
        CHECK(istEigenesInstrument(QStringLiteral("user:mein_bass")));
        CHECK_FALSE(istEigenesInstrument(QStringLiteral("piano")));
        CHECK_FALSE(istInstrumentDatei(QStringLiteral("user:mein_bass")));

        // --- ohne Ordner: leer, kein Fehler
        CHECK(eigeneInstrumente().empty());
        Instrument geholt;
        CHECK_FALSE(holeInstrument(QStringLiteral("user:mein_bass"), geholt));

        // --- speichern legt den Ordner an; das Instrument heisst danach user:<name>
        Instrument bass = *findeInstrument(QStringLiteral("slap_bass"));
        bass.titel = QStringLiteral("Mein Bass");
        bass.genres = {QStringLiteral("Funk")};
        const std::uint64_t vorher = instrumentFassung();
        REQUIRE(speichereEigenes(bass, QStringLiteral("mein_bass")));
        CHECK(bass.name == QStringLiteral("user:mein_bass"));
        CHECK(QFileInfo::exists(QDir(ordner).filePath(QStringLiteral("mein_bass.lvinst"))));
        CHECK(instrumentFassung() > vorher);  // wer Toene gerechnet hat, rechnet neu
        CHECK_FALSE(speichereEigenes(bass, QStringLiteral("../raus")));
        CHECK_FALSE(QFileInfo::exists(QDir(tmp.path()).filePath(QStringLiteral("raus.lvinst"))));

        REQUIRE(holeInstrument(QStringLiteral("user:mein_bass"), geholt));
        CHECK(geholt == bass);
        // ein Muster kann nichts ausserhalb des Ordners nennen
        CHECK_FALSE(holeInstrument(QStringLiteral("user:../instruments/mein_bass"), geholt));
        CHECK_FALSE(holeInstrument(QStringLiteral("user:"), geholt));

        // --- ein eigenes `piano` verdraengt das mitgelieferte nicht
        Instrument meinPiano = *findeInstrument(QStringLiteral("organ"));
        meinPiano.titel = QStringLiteral("Piano");
        REQUIRE(speichereEigenes(meinPiano, QStringLiteral("piano")));
        REQUIRE(holeInstrument(QStringLiteral("piano"), geholt));
        CHECK(geholt == *findeInstrument(QStringLiteral("piano")));
        REQUIRE(holeInstrument(QStringLiteral("user:piano"), geholt));
        CHECK(geholt.schichten == findeInstrument(QStringLiteral("organ"))->schichten);
        CHECK(instrumente().size() == 107);  // die mitgelieferten bleiben, wie sie sind

        // --- die Liste: nach Titel, jedes Mal neu gelesen; was kein Muster nennen kann, fehlt
        {
            QFile fremd(QDir(ordner).filePath(QStringLiteral("Von Hand.lvinst")));
            REQUIRE(fremd.open(QIODevice::WriteOnly));
            fremd.write(QJsonDocument(alsJson(bass)).toJson());
        }
        const std::vector<Instrument> eigene = eigeneInstrumente();
        REQUIRE(eigene.size() == 2);
        CHECK(eigene[0].titel == QStringLiteral("Mein Bass"));
        CHECK(eigene[0].name == QStringLiteral("user:mein_bass"));
        CHECK(eigene[1].name == QStringLiteral("user:piano"));

        // --- im Loop steht der Name, wie er ist; fehlt das Instrument, sagt es der Bericht des Kits
        Loop loop = notenLoop({{0, 4, 40, 1.0}}, QStringLiteral("user:mein_bass"));
        const QJsonObject json = alsJson(loop, tmp.path());
        CHECK(json.value(QStringLiteral("patterns")).toArray().at(0).toObject().value(QStringLiteral("instrument")).toString() ==
              QStringLiteral("user:mein_bass"));
        Loop gelesen;
        QStringList bericht;
        REQUIRE(ausJson(json, gelesen, tmp.path(), &bericht));
        CHECK(bericht.isEmpty());
        CHECK(gelesen.muster[0].instrument == QStringLiteral("user:mein_bass"));

        // --- loeschen
        REQUIRE(loescheEigenes(QStringLiteral("mein_bass")));
        CHECK_FALSE(loescheEigenes(QStringLiteral("mein_bass")));
        CHECK_FALSE(holeInstrument(QStringLiteral("user:mein_bass"), geholt));
        CHECK(eigeneInstrumente().size() == 1);
    }

    TEST_CASE("Loop Instrument: Pan je Schicht — in der Mitte bleibt jeder Wert und zur Seite wird die andere leiser")
    {
        // Stufe 18 (Entscheid Z3 a, Patrik S119; das Gesetz von Spur und Muster, Rechenkern.cpp):
        // links min(1, 1 - pan), rechts min(1, 1 + pan). Ohne Qt-Anwendung, an einem gerechneten Ton.
        Instrument mitte;
        mitte.schichten[0].schwingung = Schwingung::Saege;
        mitte.schichten[0].stimmen = 2;
        mitte.schichten[0].verstimmungCent = 7.0;
        const Klang ton = rendereTon(mitte, 57, 200.0, kRate);
        REQUIRE(ton.frames() > 1000);
        bool gleich = true;
        float spitze = 0.0f;
        for (std::size_t i = 0; i + 1 < ton.daten.size(); i += 2)
        {
            gleich = gleich && ton.daten[i] == ton.daten[i + 1];
            spitze = std::max(spitze, std::abs(ton.daten[i]));
        }
        CHECK(gleich);  // in der Mitte sind beide Seiten gleich
        REQUIRE(spitze > 0.05f);

        const auto mitPan = [&](double pan) {
            Instrument seitlich = mitte;
            seitlich.schichten[0].pan = pan;
            return rendereTon(seitlich, 57, 200.0, kRate);
        };
        // jede Seite ist der Ton der Mitte mal ihrem Faktor — nichts anderes
        const auto pruefe = [&](double pan, float links, float rechts) {
            CAPTURE(pan);
            const Klang seitlich = mitPan(pan);
            REQUIRE(seitlich.daten.size() == ton.daten.size());
            bool passt = true;
            for (std::size_t i = 0; i + 1 < ton.daten.size() && passt; i += 2)
                passt = seitlich.daten[i] == ton.daten[i] * links && seitlich.daten[i + 1] == ton.daten[i + 1] * rechts;
            CHECK(passt);
        };
        pruefe(0.0, 1.0f, 1.0f);    // Bit fuer Bit der Ton ohne Pan
        pruefe(-0.5, 1.0f, 0.5f);   // halb links: links voll, rechts halb
        pruefe(0.5, 0.5f, 1.0f);
        pruefe(-1.0, 1.0f, 0.0f);   // ganz links: rechts schweigt
        pruefe(1.0, 0.0f, 1.0f);
        pruefe(-3.0, 1.0f, 0.0f);   // ausserhalb: wie ganz links

        // zwei Schichten, eine links und eine rechts: die Seiten tragen Verschiedenes
        Instrument breit = mitte;
        breit.schichten.push_back(mitte.schichten[0]);
        breit.schichten[0].pan = -1.0;
        breit.schichten[1].pan = 1.0;
        breit.schichten[1].versatzHalbtoene = 7;
        const Klang beide = rendereTon(breit, 57, 200.0, kRate);
        Instrument nurOben = mitte;
        nurOben.schichten[0].versatzHalbtoene = 7;
        const Klang oben = rendereTon(nurOben, 57, 200.0, kRate);
        REQUIRE(beide.daten.size() >= ton.daten.size());
        bool linksIstUnten = true;
        bool rechtsIstOben = true;
        for (std::size_t i = 0; i + 1 < ton.daten.size() && i + 1 < oben.daten.size(); i += 2)
        {
            linksIstUnten = linksIstUnten && beide.daten[i] == ton.daten[i];
            rechtsIstOben = rechtsIstOben && beide.daten[i + 1] == oben.daten[i + 1];
        }
        CHECK(linksIstUnten);
        CHECK(rechtsIstOben);
    }

    TEST_CASE("Loop Instrument: Schichten — die Datei der Fassung 2 und der Ton als Summe")
    {
        const InstrumentApp app;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const Instrument orgel = *findeInstrument(QStringLiteral("organ"));
        const Instrument saege = *findeInstrument(QStringLiteral("lead"));

        // --- die Datei: Fassung 2 mit der Liste `layers`; hin und zurueck
        Instrument zwei = orgel;
        zwei.titel = QStringLiteral("Two");
        zwei.laut = 0.8;
        zwei.schichten[0].name = QStringLiteral("Body");
        Schicht oben = saege.schichten[0];
        oben.name = QStringLiteral("Shimmer");
        oben.laut = 0.35;
        oben.versatzHalbtoene = 12;
        oben.einsatzMs = 40.0;
        oben.holeDrive().wert = 0.5;
        zwei.schichten.push_back(oben);
        const QJsonObject json = alsJson(zwei);
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);
        CHECK(kInstrumentFassung == 13);  // seit S139: level ueber 1 (13, Stufe 27c); seit S136: Fretless, Slide time, Legato level (12, Stufe 25c); S134: Strum am Instrument und Playing an der Saite (11, Stufe 24a, test_LoopSpielweise.cpp); S132: die Saiten des Instruments (10, Stufe 23a, test_LoopSaiten.cpp); S121: die Schwingung Shape (9, Stufe 19c, test_ShapeTon.cpp); S120: Pan je Schicht und acht Schichten (8, Stufe 18); S118: die Reihe `path` je Schicht (7, test_LoopBausteine.cpp); S116: Art und Resonanz des Filters (6), Rauschen und Pitch sweep (5); S115: Sample als Schicht (test_LoopSample.cpp); S114: Art, Genres, Beschreibung
        CHECK(json.value(QStringLiteral("level")).toDouble() == doctest::Approx(0.8));
        CHECK_FALSE(json.contains(QStringLiteral("waveform")));  // oben stehen nur Titel und Level
        REQUIRE(json.value(QStringLiteral("layers")).toArray().size() == 2);
        const QJsonObject zweite = json.value(QStringLiteral("layers")).toArray().at(1).toObject();
        CHECK(zweite.value(QStringLiteral("name")).toString() == QStringLiteral("Shimmer"));
        CHECK(zweite.value(QStringLiteral("transpose")).toInt() == 12);
        CHECK(zweite.value(QStringLiteral("startOffset")).toDouble() == doctest::Approx(40.0));
        CHECK_FALSE(zweite.contains(QStringLiteral("drive")));  // seit Fassung 7 in der Reihe `path`
        const QJsonArray weg = zweite.value(QStringLiteral("path")).toArray();
        const auto istDrive = [](const QJsonValue& baustein) {
            return baustein.toObject().value(QStringLiteral("type")).toString() == QStringLiteral("drive") &&
                   baustein.toObject().value(QStringLiteral("amount")).toDouble() == doctest::Approx(0.5);
        };
        CHECK(std::count_if(weg.begin(), weg.end(), istDrive) == 1);
        CHECK(zweite.value(QStringLiteral("on")).toBool());
        Instrument gelesen;
        gelesen.name = zwei.name;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen == zwei);
        const QString pfad = ordner.filePath(QStringLiteral("two.lvinst"));
        REQUIRE(speichereInstrument(zwei, pfad));
        Instrument ausDatei;
        REQUIRE(ladeInstrument(pfad, ausDatei));
        ausDatei.name = zwei.name;
        CHECK(ausDatei == zwei);

        // --- Fassung 1 ist flach: eine Schicht, `level` ist ihre Lautstaerke
        QJsonObject flach;
        flach[QStringLiteral("format")] = QStringLiteral("lumiviz-instrument");
        flach[QStringLiteral("version")] = 1;
        flach[QStringLiteral("title")] = QStringLiteral("Flat");
        flach[QStringLiteral("waveform")] = QStringLiteral("saw");
        flach[QStringLiteral("voices")] = 2;
        flach[QStringLiteral("level")] = 0.6;
        Instrument alt;
        REQUIRE(ausJson(flach, alt));
        REQUIRE(alt.schichten.size() == 1);
        CHECK(alt.laut == 1.0);
        CHECK(alt.schichten[0].laut == doctest::Approx(0.6));
        CHECK(alt.schichten[0].schwingung == Schwingung::Saege);
        CHECK(alt.schichten[0].stimmen == 2);
        CHECK(alt.schichten[0].versatzHalbtoene == 0);
        CHECK(alt.schichten[0].drive().wert == 0.0);
        flach.remove(QStringLiteral("version"));  // ohne Nummer: Fassung 1
        REQUIRE(ausJson(flach, alt));
        CHECK(alt.schichten[0].schwingung == Schwingung::Saege);

        // --- Pan je Schicht (Stufe 18, Fassung 8): steht in der Datei; in einer aelteren fehlt
        // es, dann steht die Schicht in der Mitte; ein Wert ausserhalb kommt in seine Grenzen
        CHECK(zweite.value(QStringLiteral("pan")).toDouble() == 0.0);
        {
            Instrument seitlich = zwei;
            seitlich.schichten[1].pan = -0.4;
            const QJsonObject mitPan = alsJson(seitlich);
            CHECK(mitPan.value(QStringLiteral("layers")).toArray().at(1).toObject().value(QStringLiteral("pan")).toDouble() ==
                  doctest::Approx(-0.4));
            Instrument zurueck;
            zurueck.name = seitlich.name;
            REQUIRE(ausJson(mitPan, zurueck));
            CHECK(zurueck == seitlich);
            QJsonObject alteDatei = mitPan;
            alteDatei[QStringLiteral("version")] = 7;
            QJsonArray ohnePan;
            for (const QJsonValue& schicht : mitPan.value(QStringLiteral("layers")).toArray())
            {
                QJsonObject eintrag = schicht.toObject();
                eintrag.remove(QStringLiteral("pan"));
                ohnePan.append(eintrag);
            }
            alteDatei[QStringLiteral("layers")] = ohnePan;
            REQUIRE(ausJson(alteDatei, zurueck));
            CHECK(zurueck.schichten[1].pan == 0.0);
            QJsonObject wild = mitPan;
            QJsonArray zuWeit = mitPan.value(QStringLiteral("layers")).toArray();
            QJsonObject eintrag = zuWeit.at(1).toObject();
            eintrag[QStringLiteral("pan")] = 7.0;
            zuWeit[1] = eintrag;
            wild[QStringLiteral("layers")] = zuWeit;
            REQUIRE(ausJson(wild, zurueck));
            CHECK(zurueck.schichten[1].pan == 1.0);  // ausserhalb: in die Grenzen geholt
        }

        // --- mehr als acht Schichten (bis Stufe 17: vier): die uebrigen bleiben weg; ohne Liste eine mit Grundeinstellung
        CHECK(kSchichtenMax == 8);
        QJsonObject viele = json;
        QJsonArray zehn;
        for (int i = 0; i < 10; ++i) zehn.append(zweite);
        viele[QStringLiteral("layers")] = zehn;
        Instrument gekuerzt;
        REQUIRE(ausJson(viele, gekuerzt));
        CHECK(gekuerzt.schichten.size() == static_cast<std::size_t>(kSchichtenMax));
        viele.remove(QStringLiteral("layers"));
        REQUIRE(ausJson(viele, gekuerzt));
        REQUIRE(gekuerzt.schichten.size() == 1);
        CHECK(gekuerzt.schichten[0] == Schicht{});

        // --- eine Fassung von morgen liest das Programm nicht — und sagt es
        QJsonObject morgen = json;
        morgen[QStringLiteral("version")] = kInstrumentFassung + 1;
        bool zuNeu = false;
        Instrument bleibt = orgel;
        CHECK_FALSE(ausJson(morgen, bleibt, &zuNeu));
        CHECK(zuNeu);
        CHECK(bleibt == orgel);
        QJsonObject fremd = json;
        fremd[QStringLiteral("format")] = QStringLiteral("lumiviz-loop");
        CHECK_FALSE(ausJson(fremd, bleibt, &zuNeu));
        CHECK_FALSE(zuNeu);
        const QString pfadMorgen = ordner.filePath(QStringLiteral("morgen.lvinst"));
        {
            QFile datei(pfadMorgen);
            REQUIRE(datei.open(QIODevice::WriteOnly));
            datei.write(QJsonDocument(morgen).toJson());
        }
        CHECK_FALSE(holeInstrument(pfadMorgen, bleibt, &zuNeu));
        CHECK(zuNeu);
        papp::PappMischEngine engine;
        QStringList bericht;
        const std::vector<Klang> stumm = ladeKlaenge({tonKlang(pfadMorgen, 60, 250), tonKlang(pfadMorgen, 62, 250)}, engine, kRate, &bericht);
        CHECK(stumm[0].frames() == 0);
        CHECK(bericht == QStringList{QStringLiteral("Instrument needs a newer LumiViz: ") + QDir::toNativeSeparators(pfadMorgen)});

        // --- der Ton: die Summe der Schichten, jede an ihrer Tonhoehe und ab ihrem Einsatz
        Instrument unten = orgel;  // nur die erste Schicht
        unten.laut = 1.0;  // die Summe Bit fuer Bit: ohne Volume des Instruments (Stufe 27c)
        Instrument nurOben = orgel;
        nurOben.laut = 1.0;
        nurOben.schichten = {oben};
        nurOben.schichten[0].einsatzMs = 0.0;
        Instrument summe = orgel;
        summe.laut = 1.0;
        summe.schichten.push_back(oben);
        const Klang a = rendereTon(unten, 57, 500.0, kRate);
        const Klang b = rendereTon(nurOben, 57, 460.0, kRate);  // 40 ms spaeter, endet mit der Note
        const Klang beide = rendereTon(summe, 57, 500.0, kRate);
        const auto versatz = static_cast<std::size_t>(2 * std::llround(40.0 * kRate / 1000.0));
        REQUIRE(beide.daten.size() == std::max(a.daten.size(), versatz + b.daten.size()));
        bool gleich = true;
        for (std::size_t i = 0; i < beide.daten.size(); ++i)
        {
            float soll = i < a.daten.size() ? a.daten[i] : 0.0f;
            if (i >= versatz && i - versatz < b.daten.size()) soll += b.daten[i - versatz];
            if (beide.daten[i] != soll) gleich = false;
        }
        CHECK(gleich);
        CHECK(std::abs(beide.daten[beide.daten.size() - 2]) < 0.002f);  // endet bei null
        // die obere Schicht liegt eine Oktave hoeher: A3 plus 12 ist A4, 100 Abtastwerte je Schwingung
        const int p = periode(b, 6000, 4000, 80, 125);
        CHECK(p >= 98);
        CHECK(p <= 103);
        // der Versatz gilt auch fuer die Saite
        Instrument gezupft = *findeInstrument(QStringLiteral("guitar"));
        gezupft.schichten[0].versatzHalbtoene = -12;
        const int pSaite = periode(rendereTon(gezupft, 69, 500.0, kRate), 2000, 4000, 160, 250);
        CHECK(pSaite >= 197);
        CHECK(pSaite <= 204);

        // --- eine stumme Schicht zaehlt nicht; ist keine an, ist der Ton still und so lang wie die Note
        Instrument halb = summe;
        halb.schichten[1].an = false;
        CHECK(rendereTon(halb, 57, 500.0, kRate).daten == a.daten);
        halb.schichten[0].an = false;
        const Klang still = rendereTon(halb, 57, 500.0, kRate);
        CHECK(still.frames() == 22050);
        CHECK(spitze(still.daten) == 0.0f);

        // --- Level des ganzen Instruments
        Instrument leiser = unten;
        leiser.laut = 0.5;
        CHECK(spitze(rendereTon(leiser, 57, 500.0, kRate).daten) == doctest::Approx(0.5f * spitze(a.daten)));

        // --- Drive: 0 ist aus (dieselben Werte), mehr formt um und bleibt unter dem alten Betrag
        Instrument sinus;
        sinus.schichten[0].halten = 1.0;
        const Klang rein = rendereTon(sinus, 57, 500.0, kRate);
        Instrument verzerrt = sinus;
        verzerrt.schichten[0].holeDrive().wert = 1.0;
        const Klang hart = rendereTon(verzerrt, 57, 500.0, kRate);
        REQUIRE(hart.daten.size() == rein.daten.size());
        CHECK(hart.daten != rein.daten);
        CHECK(spitze(hart.daten) <= spitze(rein.daten) + 0.001f);
        // die Flaeche waechst: der Sinus wird zum Rechteck hin gedrueckt
        double flaecheRein = 0.0;
        double flaecheHart = 0.0;
        for (std::size_t i = 2 * 4000; i < 2 * 12000; i += 2)
        {
            flaecheRein += std::abs(rein.daten[i]);
            flaecheHart += std::abs(hart.daten[i]);
        }
        CHECK(flaecheHart > 1.3 * flaecheRein);
        const int pHart = periode(hart, 6000, 4000, 160, 250);  // die Tonhoehe bleibt
        CHECK(pHart >= 198);
        CHECK(pHart <= 203);
        // hoechstens `kTonMaxSekunden`, auch mit Einsatz
        Instrument lang = summe;
        lang.schichten[1].einsatzMs = 1000.0;
        CHECK(rendereTon(lang, 60, 60000.0, 8000).frames() == static_cast<std::int64_t>(kTonMaxSekunden * 8000));
    }

    TEST_CASE("Loop Instrument: ladeKlaenge rechnet die Toene — ein unbekanntes Instrument steht im Bericht")
    {
        const InstrumentApp app;
        papp::PappMischEngine engine;  // wird fuer Toene nicht gefragt
        // ein eigenes Instrument als Datei
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        const QString pfad = ordner.filePath(QStringLiteral("eigen.lvinst"));
        Instrument eigenes = *findeInstrument(QStringLiteral("lead"));
        REQUIRE(speichereInstrument(eigenes, pfad));
        const QString fehlt = ordner.filePath(QStringLiteral("fehlt.lvinst"));

        QStringList bericht;
        const QStringList namen = {QStringLiteral("inst:piano:60:250"), QStringLiteral("inst:tuba:60:250"),
                                   QStringLiteral("inst:guitar:45:500"), QStringLiteral("inst:tuba:62:250"),
                                   tonKlang(pfad, 60, 250), tonKlang(fehlt, 60, 250)};
        const std::vector<Klang> klaenge = ladeKlaenge(namen, engine, kRate, &bericht);
        REQUIRE(klaenge.size() == 6);
        CHECK(klaenge[0].frames() > 11025);
        CHECK(klaenge[1].frames() == 0);
        CHECK(klaenge[2].frames() > 22050);
        // die eigene Datei klingt wie das Instrument, aus dem sie gespeichert wurde
        CHECK(klaenge[4].daten == rendereTon(eigenes, 60, 250.0, kRate).daten);
        CHECK(klaenge[5].frames() == 0);
        // einmal je Instrument, nicht je Ton
        CHECK(bericht == QStringList{QStringLiteral("Instrument not found: tuba"),
                                     QStringLiteral("Instrument not found: ") + QDir::toNativeSeparators(fehlt)});
        CHECK(klangDatei(namen[0]).isEmpty());  // ein Ton hat keine Datei
    }

    TEST_CASE("Loop Instrument: das Beispiel house_demo laedt ohne Bericht und bleibt unter 1")
    {
        const InstrumentApp app;
        const QString pfad = repoWurzel().filePath(QStringLiteral("asset/loop/house_demo.lvloop"));
        Loop loop;
        QStringList bericht;
        REQUIRE(ladeLoop(pfad, loop, &bericht));
        CHECK(bericht.isEmpty());
        CHECK(loop.tags.titel == QStringLiteral("House demo"));
        CHECK(loop.bpm == doctest::Approx(124.0));
        CHECK(loop.durchlaufTakte() == 24);
        QStringList gespielt;
        for (const Muster& m : loop.muster)
        {
            if (m.art != MusterArt::Instrument) continue;
            CAPTURE(m.instrument.toStdString());
            CHECK(findeInstrument(m.instrument) != nullptr);
            CHECK_FALSE(m.noten.empty());
            gespielt << m.instrument;
        }
        gespielt.sort();
        // jedes der sieben Instrumente fuer House kommt vor
        CHECK(gespielt == QStringList{QStringLiteral("acid_bass"), QStringLiteral("chord_stab"),
                                      QStringLiteral("house_organ"), QStringLiteral("house_piano"),
                                      QStringLiteral("pluck"), QStringLiteral("sub_bass"), QStringLiteral("warm_pad")});

        const Ablauf ablauf = plane(loop, kRate);
        const std::vector<Klang> klaenge = klaengeOhneEngine(ablauf);
        for (std::size_t i = 0; i < klaenge.size(); ++i)
        {
            CAPTURE(ablauf.klangNamen[static_cast<qsizetype>(i)].toStdString());
            CHECK(klaenge[i].frames() > 0);
        }
        std::vector<float> alles(static_cast<std::size_t>(ablauf.durchlaufFrames) * 2);
        rechne(ablauf, klaenge, 0, static_cast<int>(ablauf.durchlaufFrames), alles.data());
        const float groesster = spitze(alles);
        MESSAGE("house_demo.lvloop: " << ablauf.klangNamen.size() << " Klaenge, ein Durchlauf " << ablauf.durchlaufFrames
                                      << " Abtastwerte, groesster Betrag " << std::to_string(groesster));
        CHECK(groesster > 0.2f);
        CHECK(groesster < 1.0f);
        if (const QString ordner = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_WAV"); !ordner.isEmpty())
        {
            REQUIRE(QDir().mkpath(ordner));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("house_demo.wav")), alles, kRate));
        }
    }

    TEST_CASE("Loop Instrument: das Beispiel effects_demo spielt fuenf Clips mit Effekten und bleibt unter 1")
    {
        // Stufe 12b (S113): house_demo mit Effekten an fuenf Clips — durch den
        // Klangerzeuger gerechnet, denn die Effekte sitzen dort (G1 b)
        const InstrumentApp app;
        const QString pfad = repoWurzel().filePath(QStringLiteral("asset/loop/effects_demo.lvloop"));
        Loop loop;
        QStringList bericht;
        REQUIRE(ladeLoop(pfad, loop, &bericht));
        CHECK(bericht.isEmpty());
        CHECK(loop.tags.titel == QStringLiteral("Effects demo"));
        CHECK(loop.durchlaufTakte() == 24);
        // dieselbe Basis zweimal im Kanal Drums, nur der zweite Clip mit Hall
        REQUIRE(loop.kanaele[0].clips.size() == 2u);
        CHECK(loop.kanaele[0].clips[0].muster == loop.kanaele[0].clips[1].muster);
        CHECK(!loop.kanaele[0].clips[0].effekte.einerAn());
        CHECK(loop.kanaele[0].clips[1].effekte.wert<ClipHall>().an);

        const Ablauf ablauf = plane(loop, kRate);
        CHECK(ablauf.busse.size() == 5u);
        const auto klaenge = std::make_shared<const std::vector<Klang>>(klaengeOhneEngine(ablauf));
        const int frames = static_cast<int>(ablauf.durchlaufFrames);
        std::vector<float> trocken(static_cast<std::size_t>(frames) * 2);
        rechne(ablauf, *klaenge, 0, frames, trocken.data());

        Klangerzeuger erzeuger;
        erzeuger.setzeProgramm(baueProgramm(loop, kRate, klaenge));
        std::vector<float> alles(static_cast<std::size_t>(frames) * 2);
        for (int stelle = 0; stelle < frames; stelle += 441)
            erzeuger.render(alles.data() + 2 * static_cast<std::ptrdiff_t>(stelle), std::min(441, frames - stelle));
        const float groesster = spitze(alles);
        MESSAGE("effects_demo.lvloop: fuenf Clips mit Effekten, ein Durchlauf " << frames << " Abtastwerte, groesster Betrag "
                                                                               << std::to_string(groesster) << " (ohne Effekte "
                                                                               << std::to_string(spitze(trocken)) << ")");
        CHECK(groesster > 0.2f);
        CHECK(groesster < 1.0f);
        CHECK(alles != trocken);
        // die ersten vier Takte spielen nur Clips ohne Effekte: dort aendert sich nichts
        const auto vierTakte = static_cast<std::ptrdiff_t>(2 * std::llround(4 * 16 * ablauf.schrittFrames));
        CHECK(std::equal(alles.begin(), alles.begin() + vierTakte, trocken.begin()));
        if (const QString ordner = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_WAV"); !ordner.isEmpty())
        {
            REQUIRE(QDir().mkpath(ordner));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("effects_demo.wav")), alles, kRate));
        }
    }

    TEST_CASE("Loop Instrument: die fuenf Beispiele aus S111 laden ohne Bericht und bleiben unter 1")
    {
        const InstrumentApp app;
        QStringList gespielt;
        for (const char* name : {"bells_demo", "synthwave_demo", "rock_demo", "strange_demo", "ambient_demo"})
        {
            CAPTURE(name);
            const QString pfad = repoWurzel().filePath(QStringLiteral("asset/loop/%1.lvloop").arg(QLatin1String(name)));
            Loop loop;
            QStringList bericht;
            REQUIRE(ladeLoop(pfad, loop, &bericht));
            CHECK(bericht.isEmpty());
            CHECK_FALSE(loop.tags.titel.isEmpty());
            CHECK(loop.durchlaufTakte() == 16);
            for (const Muster& m : loop.muster)
            {
                if (m.art != MusterArt::Instrument) continue;
                CAPTURE(m.instrument.toStdString());
                CHECK(findeInstrument(m.instrument) != nullptr);
                CHECK_FALSE(m.noten.empty());
                gespielt << m.instrument;
            }
            const Ablauf ablauf = plane(loop, kRate);
            const std::vector<Klang> klaenge = klaengeOhneEngine(ablauf);
            for (std::size_t i = 0; i < klaenge.size(); ++i)
            {
                CAPTURE(ablauf.klangNamen[static_cast<qsizetype>(i)].toStdString());
                CHECK(klaenge[i].frames() > 0);
            }
            std::vector<float> alles(static_cast<std::size_t>(ablauf.durchlaufFrames) * 2);
            rechne(ablauf, klaenge, 0, static_cast<int>(ablauf.durchlaufFrames), alles.data());
            const float groesster = spitze(alles);
            MESSAGE(std::string(name) << ".lvloop: " << ablauf.klangNamen.size() << " Klaenge, ein Durchlauf " << ablauf.durchlaufFrames
                         << " Abtastwerte, groesster Betrag " << std::to_string(groesster));
            CHECK(groesster > 0.2f);
            CHECK(groesster < 1.0f);
            if (const QString ordner = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_WAV"); !ordner.isEmpty())
            {
                REQUIRE(QDir().mkpath(ordner));
                CHECK(schreibeWav(QDir(ordner).filePath(QLatin1String(name) + QStringLiteral(".wav")), alles, kRate));
            }
        }
        // jedes der sechzehn Instrumente aus Schichten kommt in einem der Beispiele vor
        for (const char* name : {"bell", "tine_piano", "brass", "flute", "super_saw", "fifth_pad", "overdrive_guitar",
                                 "kalimba", "marimba", "cascade", "siren", "alien_choir", "broken_radio", "ghost",
                                 "growl_bass", "reverse_swell"})
        {
            CAPTURE(name);
            CHECK(gespielt.contains(QLatin1String(name)));
        }
    }

    TEST_CASE("Loop Instrument: die Genre-Beispiele unter asset/loop/genres laden ohne Bericht und bleiben unter 1")
    {
        const InstrumentApp app;
        const QDir ordner(repoWurzel().filePath(QStringLiteral("asset/loop/genres")));
        const QStringList dateien = ordner.entryList({QStringLiteral("*.lvloop")}, QDir::Files, QDir::Name);
        // neunzehn aus `asset/loop/make_genres.py`; wer eines dazulegt, braucht hier nichts zu aendern
        CHECK(dateien.size() >= 19);
        QStringList gespielt;
        for (const QString& datei : dateien)
        {
            CAPTURE(datei.toStdString());
            Loop loop;
            QStringList bericht;
            REQUIRE(ladeLoop(ordner.filePath(datei), loop, &bericht));
            CHECK(bericht.isEmpty());
            CHECK_FALSE(loop.tags.titel.isEmpty());
            CHECK_FALSE(loop.tags.genre.isEmpty());
            CHECK(loop.durchlaufTakte() >= 12);
            for (const Muster& m : loop.muster)
            {
                if (m.art != MusterArt::Instrument) continue;
                CAPTURE(m.instrument.toStdString());
                CHECK(findeInstrument(m.instrument) != nullptr);
                CHECK_FALSE(m.noten.empty());
                gespielt << m.instrument;
            }
            const Ablauf ablauf = plane(loop, kRate);
            const std::vector<Klang> klaenge = klaengeOhneEngine(ablauf);
            for (std::size_t i = 0; i < klaenge.size(); ++i)
            {
                CAPTURE(ablauf.klangNamen[static_cast<qsizetype>(i)].toStdString());
                CHECK(klaenge[i].frames() > 0);
            }
            std::vector<float> alles(static_cast<std::size_t>(ablauf.durchlaufFrames) * 2);
            rechne(ablauf, klaenge, 0, static_cast<int>(ablauf.durchlaufFrames), alles.data());
            const float groesster = spitze(alles);
            MESSAGE(datei.toStdString() << ": " << loop.bpm << " BPM, " << loop.durchlaufTakte() << " Takte, "
                                        << ablauf.klangNamen.size() << " Klaenge, groesster Betrag "
                                        << std::to_string(groesster));
            CHECK(groesster > 0.3f);
            CHECK(groesster < 0.9f);
            if (const QString ziel = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_WAV"); !ziel.isEmpty())
            {
                REQUIRE(QDir().mkpath(ziel + QStringLiteral("/genres")));
                CHECK(schreibeWav(QDir(ziel + QStringLiteral("/genres")).filePath(QFileInfo(datei).completeBaseName() + QStringLiteral(".wav")),
                                  alles, kRate));
            }
        }
        // jedes der neunzehn Instrumente aus `make_genres.py` kommt in einem der Beispiele vor
        for (const char* name : {"trance_lead", "trance_pluck", "rolling_bass", "hoover", "clavinet", "slap_bass",
                                 "clean_guitar", "muted_guitar", "round_bass", "bass_808", "reese_bass", "upright_bass",
                                 "nylon_guitar", "sax", "vibraphone", "steel_drum", "cowbell", "conga", "clave"})
        {
            CAPTURE(name);
            CHECK(gespielt.contains(QLatin1String(name)));
        }
    }

    TEST_CASE("Loop Instrument: das Beispiel instrument_demo laedt ohne Bericht und bleibt unter 1")
    {
        const InstrumentApp app;
        const QString pfad = repoWurzel().filePath(QStringLiteral("asset/loop/instrument_demo.lvloop"));
        Loop loop;
        QStringList bericht;
        REQUIRE(ladeLoop(pfad, loop, &bericht));
        CHECK(bericht.isEmpty());
        CHECK(loop.tags.titel == QStringLiteral("Instrument demo"));
        CHECK(loop.durchlaufTakte() == 16);
        int instrumentMuster = 0;
        for (const Muster& m : loop.muster)
        {
            if (m.art != MusterArt::Instrument) continue;
            ++instrumentMuster;
            CHECK(findeInstrument(m.instrument) != nullptr);
            CHECK_FALSE(m.noten.empty());
        }
        CHECK(instrumentMuster == 5);

        const Ablauf ablauf = plane(loop, kRate);
        const std::vector<Klang> klaenge = klaengeOhneEngine(ablauf);
        for (std::size_t i = 0; i < klaenge.size(); ++i)
        {
            CAPTURE(ablauf.klangNamen[static_cast<qsizetype>(i)].toStdString());
            CHECK(klaenge[i].frames() > 0);
        }
        std::vector<float> alles(static_cast<std::size_t>(ablauf.durchlaufFrames) * 2);
        rechne(ablauf, klaenge, 0, static_cast<int>(ablauf.durchlaufFrames), alles.data());
        const float groesster = spitze(alles);
        MESSAGE("instrument_demo.lvloop: " << ablauf.klangNamen.size() << " Klaenge, ein Durchlauf "
                                           << ablauf.durchlaufFrames << " Abtastwerte, groesster Betrag "
                                           << std::to_string(groesster));
        CHECK(groesster > 0.2f);
        CHECK(groesster < 1.0f);
        // ohne Gedaechtnis: ein Stueck aus der Mitte ergibt dieselben Werte
        std::vector<float> mitte(2 * 3000);
        rechne(ablauf, klaenge, 500000, 3000, mitte.data());
        CHECK(std::equal(mitte.begin(), mitte.end(), alles.begin() + 2 * 500000));

        // --- Hoerproben auf Wunsch
        const QString ordner = qEnvironmentVariable("LUMIVIZ_INSTRUMENT_WAV");
        if (ordner.isEmpty())
        {
            MESSAGE("LUMIVIZ_INSTRUMENT_WAV nicht gesetzt — keine Hoerproben");
            return;
        }
        REQUIRE(QDir().mkpath(ordner));
        // das Beispiel: zwei Durchlaeufe, damit der Uebergang zu hoeren ist
        std::vector<float> zweimal(alles.size() * 2);
        rechne(ablauf, klaenge, 0, static_cast<int>(2 * ablauf.durchlaufFrames), zweimal.data());
        CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("instrument_demo.wav")), zweimal, kRate));
        // je Instrument: eine Tonleiter ueber zwei Oktaven, dann ein Akkord
        for (const Instrument& instrument : instrumente())
        {
            const int grund = instrument.name.contains(QStringLiteral("bass")) ? 36 : 60;
            std::vector<Note> noten;
            const int stufen[] = {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24};
            int schritt = 0;
            for (const int stufe : stufen)
            {
                noten.push_back({schritt, 2, grund + stufe, 0.9});
                schritt += 2;
            }
            schritt += 2;
            for (const int stufe : {0, 4, 7, 12}) noten.push_back({schritt, 16, grund + stufe, 0.7});
            Loop probe = notenLoop(noten, instrument.name);
            probe.laut = 0.6;
            const Ablauf a = plane(probe, kRate);
            const std::vector<Klang> k = klaengeOhneEngine(a);
            std::vector<float> werte(static_cast<std::size_t>(a.durchlaufFrames) * 2);
            rechne(a, k, 0, static_cast<int>(a.durchlaufFrames), werte.data());
            MESSAGE(instrument.titel.toStdString() << ": Hoerprobe, groesster Betrag " << std::to_string(spitze(werte)));
            CHECK(schreibeWav(QDir(ordner).filePath(instrument.name + QStringLiteral(".wav")), werte, kRate));
        }
    }

    // Stufe 11e (Skizze gebilligt, Entscheid H1 b, Patrik S116). Ohne Qt-Anwendung. Gemessen wird an einem
    // gehaltenen Sinus: was der Filter von ihm uebrig laesst, ist seine Wirkung bei dieser einen Frequenz.
    TEST_CASE("Loop Instrument 11e: der Filter — Hochpass, Bandpass und Resonanz; der Tiefpass ohne Resonanz bleibt, wie er war")
    {
        const auto sinus = [](double cutoff, FilterArt art, double resonanz) {
            Instrument instrument;
            Schicht& s = instrument.schichten[0];
            s.anstiegMs = 0.0;
            s.halten = 1.0;
            s.ausklangMs = 20.0;
            s.holeFilter().teilton = cutoff;
            s.holeFilter().art = art;
            s.holeFilter().resonanz = resonanz;
            return instrument;
        };
        // der groesste Betrag, nachdem der Filter eingeschwungen ist (ab 200 ms); A3, 220 Hz
        const auto bleibt = [](const Instrument& instrument) {
            const Klang klang = rendereTon(instrument, 57, 500.0, kRate);
            float groesster = 0.0f;
            for (std::int64_t t = 8820; t < 22050; ++t) groesster = std::max(groesster, std::abs(klang.daten[static_cast<std::size_t>(2 * t)]));
            return static_cast<double>(groesster);
        };
        const double offen = bleibt(sinus(0.0, FilterArt::Tiefpass, 0.0));
        CHECK(offen == doctest::Approx(0.5).epsilon(0.001));
        // Cutoff auf „open": die Art zaehlt nicht — Abtastwert fuer Abtastwert derselbe Ton
        CHECK(rendereTon(sinus(0.0, FilterArt::Hochpass, 0.7), 57, 300.0, kRate).daten ==
              rendereTon(sinus(0.0, FilterArt::Tiefpass, 0.0), 57, 300.0, kRate).daten);

        // --- der Tiefpass ohne Resonanz ist der von frueher: zwei Stufen erster Ordnung, an der Grenze die Haelfte
        {
            const Instrument alt = sinus(1.0, FilterArt::Tiefpass, 0.0);
            const Klang klang = rendereTon(alt, 57, 300.0, kRate);
            // nachgerechnet, wie der Rechenkern es bis Stufe 11d tat
            const double a = 1.0 - std::exp(-2.0 * 3.14159265358979323846 * tonHz(57) / kRate);
            double f1 = 0.0;
            double f2 = 0.0;
            double groessteAbweichung = 0.0;
            const Klang roh = rendereTon(sinus(0.0, FilterArt::Tiefpass, 0.0), 57, 300.0, kRate);
            for (std::int64_t t = 0; t < 4410; ++t)  // im gehaltenen Teil: die Huellkurve steht auf 1
            {
                const double wert = t == 0 ? 0.0 : static_cast<double>(roh.daten[static_cast<std::size_t>(2 * t)]) / 0.5;
                f1 += a * (wert - f1);
                f2 += a * (f1 - f2);
                if (t > 0) groessteAbweichung = std::max(groessteAbweichung, std::abs(f2 * 0.5 - klang.daten[static_cast<std::size_t>(2 * t)]));
            }
            CHECK(groessteAbweichung < 2e-4);  // bis auf die Rundung der Werte auf float
            CHECK(bleibt(alt) < 0.3);
            CHECK(bleibt(alt) > 0.2);
        }

        // --- Hochpass: weit unter der Grenze bleibt fast nichts, weit darueber alles
        CHECK(bleibt(sinus(8.0, FilterArt::Hochpass, 0.0)) < 0.012);   // 220 Hz gegen 1760 Hz: ein Vierundsechzigstel
        CHECK(bleibt(sinus(8.0, FilterArt::Hochpass, 0.0)) > 0.004);
        CHECK(bleibt(sinus(0.2, FilterArt::Hochpass, 0.0)) == doctest::Approx(0.5).epsilon(0.03));
        // der Tiefpass dagegen laesst denselben Ton unter derselben Grenze durch
        CHECK(bleibt(sinus(8.0, FilterArt::Tiefpass, 0.0)) > 0.45);

        // --- Bandpass: an der Grenze voll, daneben weniger — und mit Resonanz wird das Band schmaler
        CHECK(bleibt(sinus(1.0, FilterArt::Bandpass, 0.0)) == doctest::Approx(0.5).epsilon(0.03));
        const double breit = bleibt(sinus(8.0, FilterArt::Bandpass, 0.0));
        const double schmal = bleibt(sinus(8.0, FilterArt::Bandpass, 1.0));
        CHECK(breit == doctest::Approx(0.5 * 0.177).epsilon(0.08));  // Guete 0,707, ein Achtel der Grenze
        CHECK(schmal < breit / 10.0);
        CHECK(bleibt(sinus(1.0, FilterArt::Bandpass, 1.0)) == doctest::Approx(0.5).epsilon(0.03));  // die Spitze bleibt

        // --- Resonanz am Tiefpass: an der Grenze betont, darunter leiser gestellt (die Spitze laeuft nicht davon)
        const double betont = bleibt(sinus(1.0, FilterArt::Tiefpass, 1.0));
        CHECK(betont == doctest::Approx(0.5 * 2.83).epsilon(0.05));  // Guete 11,3, um ein Viertel leiser gestellt
        CHECK(bleibt(sinus(8.0, FilterArt::Tiefpass, 1.0)) == doctest::Approx(0.5 * 0.25).epsilon(0.05));
        CHECK(bleibt(sinus(1.0, FilterArt::Tiefpass, 0.5)) > bleibt(sinus(1.0, FilterArt::Tiefpass, 0.25)));
        // ein Hauch Resonanz ist ein anderer Tiefpass als keiner: an der Grenze 0,707 statt der Haelfte
        CHECK(bleibt(sinus(1.0, FilterArt::Tiefpass, 0.01)) > bleibt(sinus(1.0, FilterArt::Tiefpass, 0.0)) + 0.05);
        // am Hochpass genauso
        CHECK(bleibt(sinus(1.0, FilterArt::Hochpass, 1.0)) == doctest::Approx(0.5 * 2.83).epsilon(0.05));

        // --- Rauschen durch den Hochpass: die Tiefen fehlen — das Vorzeichen wechselt oefter
        {
            Instrument rausch = sinus(0.0, FilterArt::Tiefpass, 0.0);
            rausch.schichten[0].schwingung = Schwingung::Rauschen;
            const auto wechsel = [](const Klang& klang) {
                int zahl = 0;
                for (std::int64_t t = 1001; t < 5000; ++t)
                    if (klang.daten[static_cast<std::size_t>(2 * (t - 1))] * klang.daten[static_cast<std::size_t>(2 * t)] < 0.0f) ++zahl;
                return zahl;
            };
            const int weiss = wechsel(rendereTon(rausch, 81, 200.0, kRate));
            rausch.schichten[0].holeFilter().teilton = 8.0;  // A5 mal acht: 7040 Hz
            rausch.schichten[0].holeFilter().art = FilterArt::Hochpass;
            const Klang hell = rendereTon(rausch, 81, 200.0, kRate);
            CHECK(wechsel(hell) > weiss + 300);  // gemessen: 2449 gegen 1953
            rausch.schichten[0].holeFilter().art = FilterArt::Tiefpass;
            CHECK(wechsel(rendereTon(rausch, 81, 200.0, kRate)) < weiss - 300);
        }

        // --- die Datei: Art und Resonanz stehen seit Fassung 7 am Filter in der Reihe `path`
        //     (`filterType`, `resonance`), in Fassung 6 flach an der Schicht (`filterType`, `filterResonance`)
        Instrument instrument = sinus(4.6, FilterArt::Bandpass, 0.15);
        instrument.schichten.push_back(Schicht{});
        instrument.schichten[1].holeFilter().art = FilterArt::Hochpass;
        const QJsonObject json = alsJson(instrument);
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);
        const QJsonArray schichten = json.value(QStringLiteral("layers")).toArray();
        REQUIRE(schichten.size() == 2);
        const auto ersterFilter = [](const QJsonValue& schicht) {
            for (const QJsonValue& baustein : schicht.toObject().value(QStringLiteral("path")).toArray())
                if (baustein.toObject().value(QStringLiteral("type")).toString() == QStringLiteral("filter"))
                    return baustein.toObject();
            return QJsonObject{};
        };
        CHECK(ersterFilter(schichten.at(0)).value(QStringLiteral("filterType")).toString() == QStringLiteral("bandpass"));
        CHECK(ersterFilter(schichten.at(0)).value(QStringLiteral("resonance")).toDouble() == doctest::Approx(0.15));
        CHECK(ersterFilter(schichten.at(1)).value(QStringLiteral("filterType")).toString() == QStringLiteral("highpass"));
        Instrument gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen == instrument);
        // dieselbe Schicht, wie Fassung 6 sie schrieb: flach
        QJsonObject sechs = schichten.at(0).toObject();
        sechs.remove(QStringLiteral("path"));
        sechs[QStringLiteral("filterCutoff")] = 4.6;
        sechs[QStringLiteral("filterType")] = QStringLiteral("bandpass");
        sechs[QStringLiteral("filterResonance")] = 0.15;
        QJsonObject alsSechs = json;
        alsSechs[QStringLiteral("version")] = 6;
        alsSechs[QStringLiteral("layers")] = QJsonArray{sechs};
        REQUIRE(ausJson(alsSechs, gelesen));
        CHECK(gelesen.schichten[0] == instrument.schichten[0]);
        // Werte in ihren Grenzen; eine unbekannte Art ist der Tiefpass
        QJsonObject wild = alsSechs;
        QJsonObject schicht = sechs;
        schicht[QStringLiteral("filterType")] = QStringLiteral("notch");
        schicht[QStringLiteral("filterResonance")] = 7.0;
        wild[QStringLiteral("layers")] = QJsonArray{schicht};
        REQUIRE(ausJson(wild, gelesen));
        CHECK(gelesen.schichten[0].filter().art == FilterArt::Tiefpass);
        CHECK(gelesen.schichten[0].filter().resonanz == 1.0);
        // eine Datei der Fassung 5 kennt die Felder nicht: der Tiefpass von frueher
        QJsonObject alt = json;
        alt[QStringLiteral("version")] = 5;
        QJsonObject ohne = sechs;
        ohne.remove(QStringLiteral("filterType"));
        ohne.remove(QStringLiteral("filterResonance"));
        alt[QStringLiteral("layers")] = QJsonArray{ohne};
        REQUIRE(ausJson(alt, gelesen));
        CHECK(gelesen.schichten[0].filter().art == FilterArt::Tiefpass);
        CHECK(gelesen.schichten[0].filter().resonanz == 0.0);
        CHECK(gelesen.schichten[0].filter().teilton == doctest::Approx(4.6));

        // --- `LUMIVIZ_11E_WAV=<Ordner>` legt Hoerproben ab: Snare, Clap und Hi-Hat aus Rauschen mit Filter,
        //     ein Bass mit Resonanz und Filter-Huellkurve
        if (const QString ordner = qEnvironmentVariable("LUMIVIZ_11E_WAV"); !ordner.isEmpty())
        {
            REQUIRE(QDir().mkpath(ordner));
            const auto schlag = [](Schwingung welle, double laut, double decayMs) {
                Schicht s;
                s.schwingung = welle;
                s.laut = laut;
                s.anstiegMs = 0.0;
                s.abfallMs = decayMs;
                s.halten = 0.0;
                s.ausklangMs = 30.0;
                return s;
            };
            const auto mitFilter = [](Schicht s, FilterArt art, double cutoff, double resonanz) {
                s.holeFilter().art = art;
                s.holeFilter().teilton = cutoff;
                s.holeFilter().resonanz = resonanz;
                return s;
            };
            const auto viermal = [](const Instrument& i, int hoehe, double dauerMs) {
                const Klang einer = rendereTon(i, hoehe, dauerMs, kRate);
                std::vector<float> alle;
                for (int n = 0; n < 4; ++n)
                {
                    alle.insert(alle.end(), einer.daten.begin(), einer.daten.end());
                    alle.resize(alle.size() + static_cast<std::size_t>(2 * kRate / 4), 0.0f);
                }
                return alle;
            };
            // Snare bei F#3 (185 Hz): zwei Sinus und Rauschen ueber 1800 Hz (mal 9,7)
            Instrument snare;
            snare.schichten = {schlag(Schwingung::Sinus, 0.6, 180.0), schlag(Schwingung::Sinus, 0.35, 110.0),
                               mitFilter(schlag(Schwingung::Rauschen, 0.9, 250.0), FilterArt::Hochpass, 9.7, 0.0)};
            snare.schichten[1].versatzHalbtoene = 10;
            // Clap bei C4 (262 Hz): Rauschen um 1200 Hz (mal 4,6), drei kurze Schlaege und der Ausklang
            Instrument clap;
            clap.schichten.clear();
            for (const auto& [einsatz, decay] : {std::pair{0.0, 10.0}, std::pair{10.0, 10.0}, std::pair{20.0, 10.0}, std::pair{30.0, 210.0}})
            {
                Schicht s = mitFilter(schlag(Schwingung::Rauschen, 1.0, decay), FilterArt::Bandpass, 4.6, 0.15);
                s.einsatzMs = einsatz;
                clap.schichten.push_back(s);
            }
            // Hi-Hat bei C5 (523 Hz): Rauschen ueber 7000 Hz (mal 13,4)
            Instrument hat;
            hat.schichten = {mitFilter(schlag(Schwingung::Rauschen, 1.0, 70.0), FilterArt::Hochpass, 13.4, 0.0)};
            Instrument hatOffen = hat;
            hatOffen.schichten[0].abfallMs = 450.0;
            // ein Bass: Saege, Tiefpass mit Resonanz, die Huellkurve oeffnet ihn
            Instrument bass;
            bass.schichten = {mitFilter(schlag(Schwingung::Saege, 0.8, 300.0), FilterArt::Tiefpass, 2.0, 0.7)};
            bass.schichten[0].holeFilter().huelle = 14.0;
            Instrument bassOhne = bass;
            bassOhne.schichten[0].holeFilter().resonanz = 0.0;
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("snare_zwei_sinus_und_rauschen_hochpass.wav")), viermal(snare, 54, 280.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("clap_rauschen_bandpass_vier_schichten.wav")), viermal(clap, 60, 330.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("hat_zu_rauschen_hochpass.wav")), viermal(hat, 72, 90.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("hat_offen_rauschen_hochpass.wav")), viermal(hatOffen, 72, 480.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("bass_saege_tiefpass_resonanz_70.wav")), viermal(bass, 36, 320.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("bass_dasselbe_ohne_resonanz.wav")), viermal(bassOhne, 36, 320.0), kRate));
        }
    }

    // Stufe 11d (Skizze gebilligt, Entscheide N1 und N2, Patrik S116). Ohne Qt-Anwendung.
    TEST_CASE("Loop Instrument 11d: Rauschen — eine feste Folge ohne Tonhoehe; Pitch sweep — die Note faellt oder steigt auf ihren Ton")
    {
        // so oft wechselt das Vorzeichen zwischen zwei Abtastwerten
        const auto wechsel = [](const Klang& klang, std::int64_t von, std::int64_t bis) {
            int zahl = 0;
            for (std::int64_t t = von + 1; t < bis && t < klang.frames(); ++t)
                if (klang.daten[static_cast<std::size_t>(2 * (t - 1))] * klang.daten[static_cast<std::size_t>(2 * t)] < 0.0f) ++zahl;
            return zahl;
        };
        const auto staerke = [](const Klang& klang) {
            double summe = 0.0;
            for (const float wert : klang.daten) summe += static_cast<double>(wert) * wert;
            return std::sqrt(summe / static_cast<double>(std::max<std::size_t>(1, klang.daten.size())));
        };
        const auto gehalten = [](Schwingung welle) {
            Instrument instrument;
            Schicht& s = instrument.schichten[0];
            s.schwingung = welle;
            s.anstiegMs = 0.0;
            s.halten = 1.0;
            s.ausklangMs = 20.0;
            return instrument;
        };

        // --- Rauschen: bei jeder Note dieselben Werte, beide Seiten gleich, halb ausgesteuert wie jede Schwingung
        Instrument rausch = gehalten(Schwingung::Rauschen);
        const Klang tief = rendereTon(rausch, 48, 200.0, kRate);
        const Klang hoch = rendereTon(rausch, 72, 200.0, kRate);
        REQUIRE(tief.frames() == 8820 + 882);
        CHECK(tief.daten == hoch.daten);                                // keine Tonhoehe
        CHECK(rendereTon(rausch, 48, 200.0, kRate).daten == tief.daten);  // derselbe Ton, dieselben Werte
        CHECK(spitze(tief.daten) <= 0.5f);
        CHECK(spitze(tief.daten) > 0.49f);
        {
            // das Zaehlwerk des Kits (`rauschen` in asset/loop/kit/make_kit.py), ab der Saat 22222;
            // der erste Abtastwert ist still (die Huellkurve beginnt bei null), der zweite voll
            std::uint32_t folge = 22222u;
            folge = folge * 1664525u + 1013904223u;
            folge = folge * 1664525u + 1013904223u;
            const double zweiter = static_cast<double>(folge >> 8) / 8388608.0 - 1.0;
            CHECK(tief.daten[0] == 0.0f);
            CHECK(tief.daten[2] == doctest::Approx(0.5 * zweiter).epsilon(1e-5));
            CHECK(tief.daten[3] == tief.daten[2]);
        }
        // weisses Rauschen: das Vorzeichen wechselt bei rund jedem zweiten Abtastwert, der Mittelwert liegt bei null
        const int w = wechsel(tief, 1000, 5000);
        CHECK(w > 1800);
        CHECK(w < 2200);
        double mittel = 0.0;
        for (std::int64_t t = 0; t < 8820; ++t) mittel += tief.daten[static_cast<std::size_t>(2 * t)];
        CHECK(std::abs(mittel / 8820.0) < 0.01);
        // Transpose bewegt ohne Filter nichts; der Filter folgt der Note — tiefer ist dumpfer und leiser
        rausch.schichten[0].versatzHalbtoene = 12;
        CHECK(rendereTon(rausch, 48, 200.0, kRate).daten == tief.daten);
        rausch.schichten[0].versatzHalbtoene = 0;
        rausch.schichten[0].holeFilter().teilton = 4.0;
        const Klang dumpf = rendereTon(rausch, 48, 200.0, kRate);
        const Klang hell = rendereTon(rausch, 72, 200.0, kRate);
        CHECK(dumpf.daten != hell.daten);
        CHECK(staerke(dumpf) < staerke(hell));
        CHECK(staerke(hell) < staerke(tief));
        CHECK(wechsel(dumpf, 1000, 5000) < wechsel(hell, 1000, 5000));
        // Pitch sweep kennt das Rauschen nicht
        rausch.schichten[0].holeFilter().teilton = 0.0;
        rausch.schichten[0].tonhoeheStartHt = 24.0;
        CHECK(rendereTon(rausch, 48, 200.0, kRate).daten == tief.daten);

        // --- Pitch sweep an einem Sinus: A3, 220 Hz — in 20 ms knapp neun Nulldurchgaenge
        Instrument ton = gehalten(Schwingung::Sinus);
        const Klang gerade = rendereTon(ton, 57, 600.0, kRate);
        const int anfang = 882;  // 20 ms
        CHECK(wechsel(gerade, 0, anfang) >= 8);
        CHECK(wechsel(gerade, 0, anfang) <= 9);
        // eine Oktave darueber beginnen, nach 90 ms angekommen: am Anfang deutlich mehr Durchgaenge
        ton.schichten[0].tonhoeheStartHt = 12.0;
        ton.schichten[0].tonhoeheZeitMs = 90.0;
        const Klang faellt = rendereTon(ton, 57, 600.0, kRate);
        REQUIRE(faellt.frames() == gerade.frames());
        CHECK(wechsel(faellt, 0, anfang) >= 14);
        CHECK(wechsel(faellt, 0, anfang) <= 16);
        // und nach 400 ms auf dem Ton: in 100 ms 44 Durchgaenge, wie ohne
        CHECK(wechsel(faellt, 17640, 22050) >= 43);
        CHECK(wechsel(faellt, 17640, 22050) <= 45);
        CHECK(wechsel(gerade, 17640, 22050) >= 43);
        CHECK(wechsel(gerade, 17640, 22050) <= 45);
        CHECK(spitze(faellt.daten) == doctest::Approx(spitze(gerade.daten)).epsilon(0.01));  // nicht lauter
        // eine Oktave darunter beginnen: sie steigt — am Anfang weniger Durchgaenge
        ton.schichten[0].tonhoeheStartHt = -12.0;
        const Klang steigt = rendereTon(ton, 57, 600.0, kRate);
        CHECK(wechsel(steigt, 0, anfang) >= 5);
        CHECK(wechsel(steigt, 0, anfang) <= 6);
        CHECK(wechsel(steigt, 17640, 22050) >= 43);
        CHECK(wechsel(steigt, 17640, 22050) <= 45);
        // 0 st oder keine Zeit: Abtastwert fuer Abtastwert wie ohne
        ton.schichten[0].tonhoeheStartHt = 0.0;
        ton.schichten[0].tonhoeheZeitMs = 500.0;
        CHECK(rendereTon(ton, 57, 600.0, kRate).daten == gerade.daten);
        ton.schichten[0].tonhoeheStartHt = 12.0;
        ton.schichten[0].tonhoeheZeitMs = 0.0;
        CHECK(rendereTon(ton, 57, 600.0, kRate).daten == gerade.daten);
        // was ueber die Grenzen geht, gilt wie die Grenze
        ton.schichten[0].tonhoeheZeitMs = 90.0;
        ton.schichten[0].tonhoeheStartHt = kTonhoeheStartMax;
        const Klang ganzOben = rendereTon(ton, 57, 600.0, kRate);
        ton.schichten[0].tonhoeheStartHt = 500.0;
        CHECK(rendereTon(ton, 57, 600.0, kRate).daten == ganzOben.daten);
        // eine Saege mit Pitch sweep bleibt unter der halben Abtastrate: kein Wert ueber dem Pegel
        Instrument saege = gehalten(Schwingung::Saege);
        saege.schichten[0].tonhoeheStartHt = 36.0;
        saege.schichten[0].tonhoeheZeitMs = 90.0;
        CHECK(spitze(rendereTon(saege, 72, 300.0, kRate).daten) <= 0.5f);
        // die gezupfte Saite kennt ihn nicht
        Instrument saite = gehalten(Schwingung::Saite);
        const Klang gezupft = rendereTon(saite, 57, 300.0, kRate);
        saite.schichten[0].tonhoeheStartHt = 12.0;
        CHECK(rendereTon(saite, 57, 300.0, kRate).daten == gezupft.daten);

        // --- die Datei: Fassung 5 mit `noise`, `pitchStart`, `pitchTime`
        Instrument beides = gehalten(Schwingung::Rauschen);
        beides.titel = QStringLiteral("Zap");
        beides.schichten.push_back(Schicht{});
        beides.schichten[1].tonhoeheStartHt = -7.0;
        beides.schichten[1].tonhoeheZeitMs = 250.0;
        const QJsonObject json = alsJson(beides);
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);  // die Felder gibt es seit Fassung 5
        const QJsonArray schichten = json.value(QStringLiteral("layers")).toArray();
        REQUIRE(schichten.size() == 2);
        CHECK(schichten.at(0).toObject().value(QStringLiteral("waveform")).toString() == QStringLiteral("noise"));
        CHECK(schichten.at(1).toObject().value(QStringLiteral("pitchStart")).toDouble() == doctest::Approx(-7.0));
        CHECK(schichten.at(1).toObject().value(QStringLiteral("pitchTime")).toDouble() == doctest::Approx(250.0));
        Instrument gelesen;
        REQUIRE(ausJson(json, gelesen));
        CHECK(gelesen == beides);
        // Werte in ihren Grenzen
        QJsonObject wild = json;
        QJsonObject schicht = schichten.at(1).toObject();
        schicht[QStringLiteral("pitchStart")] = 99.0;
        schicht[QStringLiteral("pitchTime")] = -5.0;
        QJsonObject tiefer = schicht;
        tiefer[QStringLiteral("pitchStart")] = -99.0;
        tiefer[QStringLiteral("pitchTime")] = 99999.0;
        wild[QStringLiteral("layers")] = QJsonArray{schicht, tiefer};
        REQUIRE(ausJson(wild, gelesen));
        CHECK(gelesen.schichten[0].tonhoeheStartHt == kTonhoeheStartMax);
        CHECK(gelesen.schichten[0].tonhoeheZeitMs == 0.0);
        CHECK(gelesen.schichten[1].tonhoeheStartHt == kTonhoeheStartMin);
        CHECK(gelesen.schichten[1].tonhoeheZeitMs == kTonhoeheZeitMaxMs);
        // eine Datei der Fassung 4 kennt die Felder nicht: kein Pitch sweep
        QJsonObject alt = json;
        alt[QStringLiteral("version")] = 4;
        QJsonObject ohne = schichten.at(1).toObject();
        ohne.remove(QStringLiteral("pitchStart"));
        ohne.remove(QStringLiteral("pitchTime"));
        alt[QStringLiteral("layers")] = QJsonArray{ohne};
        REQUIRE(ausJson(alt, gelesen));
        CHECK(gelesen.schichten[0].tonhoeheStartHt == 0.0);
        CHECK(gelesen.schichten[0] == Schicht{});

        // --- `LUMIVIZ_11D_WAV=<Ordner>` legt Hoerproben ab: eine Kick und ein Tom aus den neuen Bausteinen
        //     (Werte wie im Rechenversuch zum Kit, Loop_Konzept §4h; Decay des Klicks 10 ms, kuerzer
        //     stellt der Regler nicht), dazu ein steigender Ton
        if (const QString ordner = qEnvironmentVariable("LUMIVIZ_11D_WAV"); !ordner.isEmpty())
        {
            REQUIRE(QDir().mkpath(ordner));
            const auto schlag = [](Schwingung welle, double laut, double decayMs) {
                Schicht s;
                s.schwingung = welle;
                s.laut = laut;
                s.anstiegMs = 0.0;
                s.abfallMs = decayMs;
                s.halten = 0.0;
                s.ausklangMs = 30.0;
                return s;
            };
            const auto viermal = [](const Instrument& instrument, int hoehe, double dauerMs) {
                const Klang einer = rendereTon(instrument, hoehe, dauerMs, kRate);
                std::vector<float> alle;
                for (int i = 0; i < 4; ++i)
                {
                    alle.insert(alle.end(), einer.daten.begin(), einer.daten.end());
                    alle.resize(alle.size() + static_cast<std::size_t>(2 * kRate / 4), 0.0f);  // eine Viertelsekunde Pause
                }
                return alle;
            };
            Instrument kick;
            kick.schichten = {schlag(Schwingung::Sinus, 1.0, 480.0), schlag(Schwingung::Rauschen, 0.25, 10.0)};
            kick.schichten[0].tonhoeheStartHt = 22.0;
            kick.schichten[0].tonhoeheZeitMs = 105.0;
            Instrument kickDrive = kick;
            kickDrive.schichten[0].holeDrive().wert = 0.19;
            Instrument tomtom;
            tomtom.schichten = {schlag(Schwingung::Sinus, 1.0, 420.0), schlag(Schwingung::Rauschen, 0.15, 10.0)};
            tomtom.schichten[0].tonhoeheStartHt = 10.0;
            tomtom.schichten[0].tonhoeheZeitMs = 120.0;
            Instrument zap;
            zap.schichten = {schlag(Schwingung::Saege, 0.7, 300.0)};
            zap.schichten[0].tonhoeheStartHt = -24.0;
            zap.schichten[0].tonhoeheZeitMs = 250.0;
            Instrument klick;
            klick.schichten = {schlag(Schwingung::Rauschen, 1.0, 60.0)};
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("kick_sinus_faellt_22st_und_klick.wav")), viermal(kick, 30, 450.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("kick_dasselbe_mit_drive.wav")), viermal(kickDrive, 30, 450.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("tom_sinus_faellt_10st_und_klick.wav")), viermal(tomtom, 45, 400.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("zap_saege_steigt_24st.wav")), viermal(zap, 60, 350.0), kRate));
            CHECK(schreibeWav(QDir(ordner).filePath(QStringLiteral("rauschen_allein_decay_60ms.wav")), viermal(klick, 60, 200.0), kRate));
        }
    }

    // Stufe 11f (Entscheid K1 a, Patrik S115): sechs gerechnete Drum-Instrumente neben dem Kit
    // (`asset/loop/make_drums.py`). Gerechnet wie eine Drum-Spur sie spielt: Note C2, Length 250 ms.
    // `LUMIVIZ_11F_WAV=<Ordner>` legt je Instrument einen Schlag ab (`<name>.wav`) — den vergleicht
    // `.claude/handover/hoerproben_11f/vergleich.py` mit dem Klang des Kits.
    TEST_CASE("Loop Instrument 11f: sechs gerechnete Drum-Instrumente — Art Drums, fuer die Note C2 gebaut")
    {
        const InstrumentApp app;
        // so oft wechselt das Vorzeichen in der ersten Zehntelsekunde: tief wenig, Rauschen oft
        const auto wechsel = [](const Klang& klang) {
            int zahl = 0;
            const std::int64_t bis = std::min<std::int64_t>(klang.frames(), kRate / 10);
            for (std::int64_t t = 1; t < bis; ++t)
                if ((klang.daten[static_cast<std::size_t>(2 * t)] < 0.0f) != (klang.daten[static_cast<std::size_t>(2 * (t - 1))] < 0.0f))
                    ++zahl;
            return zahl;
        };
        // die Zeit, nach der der Schlag unter ein Zehntel seiner Spitze gefallen ist und dort bleibt
        const auto klingtMs = [](const Klang& klang) {
            const float grenze = 0.1f * spitze(klang.daten);
            std::int64_t letzter = 0;
            for (std::int64_t t = 0; t < klang.frames(); ++t)
                if (std::abs(klang.daten[static_cast<std::size_t>(2 * t)]) > grenze) letzter = t;
            return static_cast<double>(letzter) * 1000.0 / kRate;
        };
        const int note = 36;  // C2: was eine Drum-Spur einem Instrument der Art Drums vorbelegt
        // `LUMIVIZ_DRUM_KANDIDATEN=<Ordner>`: jede `.lvinst` dort wie eine Drum-Spur rechnen und als
        // `<name>.wav` daneben ablegen — so lassen sich Reihen von Varianten gegen das Kit messen,
        // bevor eine davon nach `asset/loop/instruments` kommt (Stufe 16e)
        if (const QString ordner = qEnvironmentVariable("LUMIVIZ_DRUM_KANDIDATEN"); !ordner.isEmpty())
        {
            int gerechnet = 0;
            for (const QString& datei : QDir(ordner).entryList({QStringLiteral("*.lvinst")}, QDir::Files))
            {
                Instrument kandidat;
                REQUIRE(ladeInstrument(QDir(ordner).filePath(datei), kandidat));
                CHECK(schreibeWav(QDir(ordner).filePath(QFileInfo(datei).completeBaseName() + QStringLiteral(".wav")),
                                  rendereTon(kandidat, note, kSpurDauerMs, kRate).daten, kRate));
                ++gerechnet;
            }
            MESSAGE(gerechnet << " Kandidaten gerechnet in " << ordner.toStdString());
        }
        std::map<QString, Klang> klang;
        for (const char* name : {"drum_kick", "drum_tom", "drum_snare", "drum_clap", "drum_hat_closed", "drum_hat_open"})
        {
            const std::string welches = name;
            CAPTURE(welches);
            const Instrument* instrument = findeInstrument(QString::fromLatin1(name));
            REQUIRE(instrument != nullptr);
            CHECK(instrument->art == QStringLiteral("drums"));
            CHECK(instrument->beschreibung.contains(QStringLiteral("C2")));
            // nur Bausteine des Creators: keine Aufnahme, keine Saite
            for (const Schicht& schicht : instrument->schichten)
            {
                CHECK(schicht.schwingung != Schwingung::Sample);
                CHECK(schicht.halten == 0.0);
                CHECK(schicht.abfallMs >= 10.0);  // kuerzer stellt der Regler nicht
            }
            Klang einer = rendereTon(*instrument, note, kSpurDauerMs, kRate);
            REQUIRE(einer.frames() > 0);
            // etwa gleich laut untereinander (gemessen 0,42 bis 0,58) — leiser als ein Klang des Kits (0,9)
            CHECK(spitze(einer.daten) > 0.35f);
            CHECK(spitze(einer.daten) < 0.7f);
            CHECK(rendereTon(*instrument, note, kSpurDauerMs, kRate).daten == einer.daten);  // kein Zufall
            klang[QString::fromLatin1(name)] = std::move(einer);
        }
        // tief gegen hoch: Kick und Tom schwingen langsam, die Hi-Hats sind Rauschen ueber 7 kHz
        CHECK(wechsel(klang[QStringLiteral("drum_kick")]) < 60);
        CHECK(wechsel(klang[QStringLiteral("drum_tom")]) < 120);
        CHECK(wechsel(klang[QStringLiteral("drum_kick")]) < wechsel(klang[QStringLiteral("drum_tom")]));
        CHECK(wechsel(klang[QStringLiteral("drum_hat_closed")]) > 1200);
        CHECK(wechsel(klang[QStringLiteral("drum_snare")]) > wechsel(klang[QStringLiteral("drum_tom")]));
        // kurz gegen lang
        CHECK(klingtMs(klang[QStringLiteral("drum_hat_closed")]) < 80.0);
        CHECK(klingtMs(klang[QStringLiteral("drum_hat_open")]) > 2.5 * klingtMs(klang[QStringLiteral("drum_hat_closed")]));
        CHECK(klingtMs(klang[QStringLiteral("drum_kick")]) > 150.0);
        // der Clap setzt dreimal neu an, bevor er ausklingt: nach 5 ms leiser als nach 12 ms
        {
            const Klang& clap = klang[QStringLiteral("drum_clap")];
            const auto pegel = [&clap](double vonMs, double bisMs) {
                float groesster = 0.0f;
                for (auto t = static_cast<std::int64_t>(vonMs * kRate / 1000.0); t < static_cast<std::int64_t>(bisMs * kRate / 1000.0); ++t)
                    groesster = std::max(groesster, std::abs(clap.daten[static_cast<std::size_t>(2 * t)]));
                return groesster;
            };
            CHECK(pegel(7.0, 9.5) < pegel(10.5, 14.0));
            CHECK(pegel(17.0, 19.5) < pegel(20.5, 24.0));
        }
        // eine andere Note verschiebt den Schlag — die Instrumente sind fuer C2 gebaut
        CHECK(wechsel(rendereTon(*findeInstrument(QStringLiteral("drum_kick")), note + 12, kSpurDauerMs, kRate)) >
              wechsel(klang[QStringLiteral("drum_kick")]));

        if (const QString ordner = qEnvironmentVariable("LUMIVIZ_11F_WAV"); !ordner.isEmpty())
        {
            REQUIRE(QDir().mkpath(ordner));
            for (const auto& [name, einer] : klang)
                CHECK(schreibeWav(QDir(ordner).filePath(name + QStringLiteral(".wav")), einer.daten, kRate));
        }
    }
}
