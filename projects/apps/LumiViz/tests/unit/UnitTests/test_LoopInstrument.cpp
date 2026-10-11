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
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

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

        const std::vector<Instrument>& alle = instrumente();
        REQUIRE(alle.size() == 7);
        QStringList titel;
        for (const Instrument& i : alle) titel << i.titel;
        CHECK(titel == QStringList{QStringLiteral("Piano"), QStringLiteral("Strings"), QStringLiteral("Violin"),
                                   QStringLiteral("Guitar"), QStringLiteral("Bass"), QStringLiteral("Organ"),
                                   QStringLiteral("Lead")});
        CHECK(findeInstrument(QStringLiteral("guitar")) == &alle[3]);
        CHECK(findeInstrument(QStringLiteral("tuba")) == nullptr);

        for (const Instrument& instrument : alle)
        {
            CAPTURE(instrument.name.toStdString());
            CHECK(findeInstrument(instrument.name) == &instrument);
            // A4, eine halbe Sekunde gehalten
            const Klang ton = rendereTon(instrument, 69, 500.0, kRate);
            const std::int64_t ausklang = std::llround(std::max(5.0, instrument.ausklangMs) * kRate / 1000.0);
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
            // die Tonhoehe: 440 Hz sind 100,2 Abtastwerte je Schwingung
            const int p = periode(ton, 6000, 4000, 80, 125);
            CHECK(p >= 98);  // mit Vibrato schwankt sie um gut ein Prozent
            CHECK(p <= 103);
            // eine Oktave tiefer die doppelte Periode
            const Klang tief = rendereTon(instrument, 57, 500.0, kRate);
            const int pTief = periode(tief, 6000, 4000, 160, 250);
            CHECK(pTief >= 197);
            CHECK(pTief <= 204);
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
        CHECK(rendereTon(alle[5], 60, 60000.0, 8000).frames() == static_cast<std::int64_t>(kTonMaxSekunden * 8000));
    }

    TEST_CASE("Loop Instrument: ladeKlaenge rechnet die Toene — ein unbekanntes Instrument steht im Bericht")
    {
        papp::PappMischEngine engine;  // wird fuer Toene nicht gefragt
        QStringList bericht;
        const QStringList namen = {QStringLiteral("inst:piano:60:250"), QStringLiteral("inst:tuba:60:250"),
                                   QStringLiteral("inst:guitar:45:500"), QStringLiteral("inst:tuba:62:250")};
        const std::vector<Klang> klaenge = ladeKlaenge(namen, engine, kRate, &bericht);
        REQUIRE(klaenge.size() == 4);
        CHECK(klaenge[0].frames() > 11025);
        CHECK(klaenge[1].frames() == 0);
        CHECK(klaenge[2].frames() > 22050);
        CHECK(bericht == QStringList{QStringLiteral("Instrument not found: tuba")});  // einmal, nicht je Ton
        CHECK(klangDatei(namen[0]).isEmpty());  // ein Ton hat keine Datei
    }

    TEST_CASE("Loop Instrument: das Beispiel instrument_demo laedt ohne Bericht und bleibt unter 1")
    {
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
            const int grund = instrument.name == QStringLiteral("bass") ? 36 : 60;
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
