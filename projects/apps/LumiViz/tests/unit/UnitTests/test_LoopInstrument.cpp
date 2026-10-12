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

/// .../projects/apps/LumiViz/tests/unit/UnitTests -> Wurzel des Repos
QDir repoWurzel()
{
    QDir wurzel = QFileInfo(QString::fromLocal8Bit(__FILE__)).absoluteDir();
    for (int i = 0; i < 6; ++i) wurzel.cdUp();
    return wurzel;
}

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

/// Eine WAV-Datei mit 16 Bit lesen (die Klaenge des Kits: 44100 Hz, Mono) — als
/// Klang in Stereo. Leer = nicht lesbar.
Klang liesWav(const QString& pfad)
{
    Klang klang;
    QFile datei(pfad);
    if (!datei.open(QIODevice::ReadOnly)) return klang;
    const QByteArray roh = datei.readAll();
    if (roh.size() < 44 || !roh.startsWith("RIFF")) return klang;
    int kanaele = 1;
    qsizetype stelle = 12;
    while (stelle + 8 <= roh.size())
    {
        const QByteArray kennung = roh.mid(stelle, 4);
        std::uint32_t groesse = 0;
        std::memcpy(&groesse, roh.constData() + stelle + 4, 4);
        const char* inhalt = roh.constData() + stelle + 8;
        if (kennung == "fmt ")
        {
            std::uint16_t k = 1;
            std::memcpy(&k, inhalt + 2, 2);
            kanaele = k;
        }
        else if (kennung == "data")
        {
            const auto werte = static_cast<qsizetype>(groesse) / 2;
            for (qsizetype i = 0; i + kanaele <= werte; i += kanaele)
            {
                std::int16_t links = 0;
                std::int16_t rechts = 0;
                std::memcpy(&links, inhalt + 2 * i, 2);
                std::memcpy(&rechts, inhalt + 2 * (i + kanaele - 1), 2);
                klang.daten.push_back(static_cast<float>(links) / 32768.0f);
                klang.daten.push_back(static_cast<float>(rechts) / 32768.0f);
            }
            break;
        }
        stelle += 8 + static_cast<qsizetype>(groesse) + (groesse % 2);
    }
    return klang;
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

/// Die Klaenge eines Ablaufs ohne Audio-Engine: Toene gerechnet, das Kit aus seinen WAV-Dateien.
std::vector<Klang> klaengeOhneEngine(const Ablauf& ablauf)
{
    const QString kit = repoWurzel().filePath(QStringLiteral("asset/loop/kit"));
    std::vector<Klang> klaenge;
    for (const QString& name : ablauf.klangNamen)
    {
        QString instrument;
        int hoehe = 0;
        int ms = 0;
        if (zerlegeTon(name, instrument, hoehe, ms))
        {
            const Instrument* gefunden = findeInstrument(instrument);
            klaenge.push_back(gefunden != nullptr ? rendereTon(*gefunden, hoehe, ms, ablauf.rate) : Klang{});
        }
        else if (istKitKlang(name))
            klaenge.push_back(liesWav(QDir(kit).filePath(name.mid(4) + QStringLiteral(".wav"))));
        else
            klaenge.push_back({});
    }
    return klaenge;
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
                               {0, 4, 200, 1.0},     // zu hoch: C7
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
        CHECK(loop.klangNamen() == QStringList{QStringLiteral("inst:piano:60:500"), QStringLiteral("inst:piano:96:500"),
                                               QStringLiteral("inst:piano:24:125"), QStringLiteral("inst:piano:64:500"),
                                               QStringLiteral("inst:piano:62:500")});
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
        CHECK(loop.muster[0].takte == kTakteMin);
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
        CHECK(tonName(kHoeheMin) == QStringLiteral("C1"));
        CHECK(tonName(kHoeheMax) == QStringLiteral("C7"));

        // die mitgelieferten Instrumente: die Dateien unter asset/loop/instruments, nach Titel sortiert
        const InstrumentApp app;  // der Programmordner fuer die Suche nach dem Ordner
        CHECK(QDir(instrumentOrdner()).dirName() == QStringLiteral("instruments"));
        const std::vector<Instrument>& alle = instrumente();
        // sieben allgemeine und sieben fuer House (Wunsch Patrik S110), dazu sechzehn aus
        // Schichten, sieben davon ungewoehnlich, und neunzehn fuer die Genre-Beispiele
        // (Wunsch Patrik S111)
        REQUIRE(alle.size() == 49);
        for (std::size_t i = 1; i < alle.size(); ++i)
            CHECK(alle[i - 1].titel.compare(alle[i].titel, Qt::CaseInsensitive) <= 0);
        CHECK(alle.front().titel == QStringLiteral("808 Bass"));  // Ziffern stehen vor Buchstaben
        CHECK(alle.back().titel == QStringLiteral("Warm Pad"));
        for (const char* name : {"bass", "guitar", "lead", "organ", "piano", "strings", "violin", "acid_bass",
                                 "chord_stab", "house_organ", "house_piano", "pluck", "sub_bass", "warm_pad",
                                 "bell", "tine_piano", "brass", "flute", "super_saw", "fifth_pad", "overdrive_guitar",
                                 "kalimba", "marimba", "cascade", "siren", "alien_choir", "broken_radio", "ghost",
                                 "growl_bass", "reverse_swell",
                                 "trance_lead", "trance_pluck", "rolling_bass", "hoover", "clavinet", "slap_bass",
                                 "clean_guitar", "muted_guitar", "round_bass", "bass_808", "reese_bass", "upright_bass",
                                 "nylon_guitar", "sax", "vibraphone", "steel_drum", "cowbell", "conga", "clave"})
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
            CHECK(instrument.schichten[0].versatzHalbtoene == 0);
            CHECK(instrument.schichten[0].einsatzMs == 0.0);
            double laengsterAusklang = 0.0;
            double vibrato = 0.0;
            for (const Schicht& schicht : instrument.schichten)
            {
                CHECK(schicht.an);
                laengsterAusklang = std::max(laengsterAusklang, schicht.ausklangMs);
                vibrato = std::max(vibrato, schicht.vibratoCent);
            }
            // A4, eine halbe Sekunde gehalten: so lang wie die Note und der laengste Ausklang
            const Klang ton = rendereTon(instrument, 69, 500.0, kRate);
            const std::int64_t ausklang = std::llround(std::max(5.0, laengsterAusklang) * kRate / 1000.0);
            CHECK(ton.frames() == 22050 + ausklang);
            const float groesster = spitze(ton.daten);
            MESSAGE(instrument.titel.toStdString() << ": A4, " << ton.frames() << " Abtastwerte, Spitze "
                                                   << std::to_string(groesster));
            CHECK(groesster > 0.05f);
            CHECK(groesster <= 0.75f);
            bool endlich = true;
            bool beideGleich = true;
            for (std::size_t i = 0; i + 1 < ton.daten.size(); i += 2)
            {
                if (!std::isfinite(ton.daten[i])) endlich = false;
                if (ton.daten[i] != ton.daten[i + 1]) beideGleich = false;
            }
            CHECK(endlich);
            CHECK(beideGleich);
            // der Klang endet bei null: kein Knacken am Ende
            CHECK(std::abs(ton.daten[ton.daten.size() - 2]) < 0.002f);
            // die Tonhoehe: 440 Hz sind 100,2 Abtastwerte je Schwingung — mit Vibrato schwankt
            // sie um so viele Cent, wie das Instrument angibt (die Sirene: 100)
            const double schwankung = std::exp2(std::max(vibrato, 20.0) / 1200.0);
            const int p = periode(ton, 6000, 4000, 80, 125);
            CHECK(p >= std::floor(100.2 / schwankung) - 1);
            CHECK(p <= std::ceil(100.2 * schwankung) + 1);
            // eine Oktave tiefer die doppelte Periode
            const Klang tief = rendereTon(instrument, 57, 500.0, kRate);
            const int pTief = periode(tief, 6000, 4000, 160, 250);
            CHECK(pTief >= std::floor(200.5 / schwankung) - 1);
            CHECK(pTief <= std::ceil(200.5 * schwankung) + 1);
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
                CHECK(spitze(rand.daten) <= 0.9f);
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
        wildeSchicht[QStringLiteral("drive")] = 7.0;
        wild[QStringLiteral("layers")] = QJsonArray{wildeSchicht};
        wild[QStringLiteral("level")] = 3.0;
        Instrument gezaehmt;
        REQUIRE(ausJson(wild, gezaehmt));
        REQUIRE(gezaehmt.schichten.size() == 1);
        CHECK(gezaehmt.laut == doctest::Approx(1.0));
        CHECK(gezaehmt.schichten[0].stimmen == 3);
        CHECK(gezaehmt.schichten[0].halten == doctest::Approx(1.0));
        CHECK(gezaehmt.schichten[0].schwingung == Schwingung::Sinus);  // die Vorgabe
        CHECK(gezaehmt.schichten[0].versatzHalbtoene == 24);
        CHECK(gezaehmt.schichten[0].einsatzMs == doctest::Approx(0.0));
        CHECK(gezaehmt.schichten[0].verzerrung == doctest::Approx(1.0));
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
        REQUIRE(arten.size() == 8);
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
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);  // geschrieben wird die juengste (seit S115: 4)
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
        CHECK(jeArt[QStringLiteral("keys")] == 7);
        CHECK(jeArt[QStringLiteral("bass")] == 10);
        CHECK(jeArt[QStringLiteral("lead")] == 4);
        CHECK(jeArt[QStringLiteral("pads")] == 4);
        CHECK(jeArt[QStringLiteral("plucked")] == 7);
        CHECK(jeArt[QStringLiteral("wind")] == 3);
        CHECK(jeArt[QStringLiteral("percussion")] == 8);
        CHECK(jeArt[QStringLiteral("unusual")] == 6);
        // die Genres sind aus den mitgelieferten Loops gezaehlt: Guitar und Violin stehen in keinem
        CHECK(ohneGenre == 2);
        CHECK(findeInstrument(QStringLiteral("slap_bass"))->genres == QStringList{QStringLiteral("Funk")});
        CHECK(findeInstrument(QStringLiteral("guitar"))->genres.isEmpty());
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
        CHECK(instrumente().size() == 49);  // die mitgelieferten bleiben, wie sie sind

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
        oben.verzerrung = 0.5;
        zwei.schichten.push_back(oben);
        const QJsonObject json = alsJson(zwei);
        CHECK(json.value(QStringLiteral("version")).toInt() == kInstrumentFassung);
        CHECK(kInstrumentFassung == 4);  // seit S115: Sample als Schicht (test_LoopSample.cpp); S114: Art, Genres, Beschreibung
        CHECK(json.value(QStringLiteral("level")).toDouble() == doctest::Approx(0.8));
        CHECK_FALSE(json.contains(QStringLiteral("waveform")));  // oben stehen nur Titel und Level
        REQUIRE(json.value(QStringLiteral("layers")).toArray().size() == 2);
        const QJsonObject zweite = json.value(QStringLiteral("layers")).toArray().at(1).toObject();
        CHECK(zweite.value(QStringLiteral("name")).toString() == QStringLiteral("Shimmer"));
        CHECK(zweite.value(QStringLiteral("transpose")).toInt() == 12);
        CHECK(zweite.value(QStringLiteral("startOffset")).toDouble() == doctest::Approx(40.0));
        CHECK(zweite.value(QStringLiteral("drive")).toDouble() == doctest::Approx(0.5));
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
        CHECK(alt.schichten[0].verzerrung == 0.0);
        flach.remove(QStringLiteral("version"));  // ohne Nummer: Fassung 1
        REQUIRE(ausJson(flach, alt));
        CHECK(alt.schichten[0].schwingung == Schwingung::Saege);

        // --- mehr als vier Schichten: die uebrigen bleiben weg; ohne Liste eine mit Grundeinstellung
        QJsonObject viele = json;
        QJsonArray sechs;
        for (int i = 0; i < 6; ++i) sechs.append(zweite);
        viele[QStringLiteral("layers")] = sechs;
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
        Instrument nurOben = orgel;
        nurOben.schichten = {oben};
        nurOben.schichten[0].einsatzMs = 0.0;
        Instrument summe = orgel;
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
        verzerrt.schichten[0].verzerrung = 1.0;
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
        CHECK(loop.kanaele[0].clips[1].effekte.hall.an);

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
}
