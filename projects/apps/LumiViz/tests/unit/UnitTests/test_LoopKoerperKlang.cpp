/**
 ****************************************************************************************
 * @file   test_LoopKoerperKlang.cpp
 * @brief  Loop, Stufe 31c-7: die Antwort des Bodens und der Messteil (docs/audio/
 *         Spielweisen_Konzept.md §4.1, Entscheide Patrik S149/S150: AW1 a, FT1 a, PG1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Engine, ohne Qt-Anwendung; der Steinway wird vom Arbeitsordner aus gelesen (die
 * Test-Exe aus dem Projekt-Root starten).
 *
 * - Ein Einmassenschwinger: die Antwort als Zeitreihe gegen die geschlossene Loesung
 *   a·(g''(t) + δ(t)), g(t) = e^(−σt) sin(ω_d t)/ω_d; der Weg Kraft → Druck.
 * - Die Laenge der Antwort am Steinway (AW1 a: tiefste Mode 38 Hz, η 1,1 % → rund 5 s); eine Mode, die 22 s
 *   klingt (AW2 a, S151: keine Grenze 8 s mehr); die schnelle Antwort auf dem Raster gegen die exakte (AW3-1 a,
 *   S151) an jedem mitgelieferten Koerper, unter −80 dB der Spitze.
 * - Ein Ton durch den Boden (Schritt 3): um die Antwort laenger, Stereo (Hoerorte `left`/`right`),
 *   links gleich der Faltung von Hand, die Antwort beim zweiten Ton aus dem Tonspeicher, ein
 *   fehlender Koerper aendert nichts. Leert den Tonspeicher vorher und nachher.
 * - „Body not found“ (SB1 a): ein Instrument in einem Wegwerf-Ordner, dessen Koerper es nicht gibt —
 *   der Bericht des Tonauftrags sagt es einmal, der Ton klingt ohne Boden.
 * - `LUMIVIZ_31C_WAV=<Ordner>` legt Hoerproben ab (ZA1 a, VA1 a: A0, C2, C4, C6, C8 bei mf, C4 bei pp/mf/ff;
 *   Familie Steinway ohne Boden, mit Boden und mit den zwei Glasboeden durch das Instrument; Familie MAESSTRO
 *   RP gegen MP1–MP3 ueber die Bausteine) mit einer Liste `liste.txt` — Patrik hoert sie gesammelt spaeter.
 * - **Messteil** nur mit `LUMIVIZ_31C7_MESSUNG=1` (Release-clang, einige Minuten): Zeit und Speicher je
 *   Antwort, Rechenzeit der Faltung je Ton, Schalldruck am Hoerort fuer alle 88 Tasten bei der
 *   hoechsten Anschlagstufe → der Wert von `kSchalldruckBezug` (PG1 a). Meldet nur Zahlen. Seit S151 (31c-8)
 *   fuer jeden mitgelieferten Koerper, mit dem lautesten Ton in dBFS beim festen Bezug (Kipppunkt von PG1 a).
 ****************************************************************************************
 */

#include <doctest.h>

#include "InstrumentBody/Antwort.hpp"
#include "loop/Boden.hpp"
#include "loop/HammerTon.hpp"
#include "loop/Instrument.hpp"
#include "loop/Aufnahme.hpp"  // schreibeWav
#include "loop/Kit.hpp"
#include "loop/Koerper.hpp"
#include "loop/Loop.hpp"
#include "loop/Tonspeicher.hpp"
#include "PappTon.hpp"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtGlobal>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <numbers>
#include <vector>

using namespace lumi::loop;
namespace kp = lumi::koerper;

namespace
{

double sekundenSeit(std::chrono::steady_clock::time_point t)
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - t).count();
}

}  // namespace

TEST_CASE("Loop Koerper 31c-7 Antwort: ein Einmassenschwinger")
{
    // eine Mode bei 100 Hz, η = 2 %, φ = 2, G = 0,5: H = φG·(−ω²)/(ω_n² − ω² + jηω_nω)
    kp::ModeImKoerper m;
    m.frequenzHz = 100.0;
    m.verlustfaktor = 0.02;
    m.amStegpunkt = {2.0};
    m.abstrahlung = {std::complex<double>(0.5, 0.0)};
    const std::vector<kp::ModeImKoerper> moden{m};
    const int rate = 48000;
    const double t = 1.0 / rate;
    const std::vector<double> h = bodenAntwort(moden, 0, 0, rate, 1.0);
    REQUIRE(h.size() == static_cast<std::size_t>(rate));
    // geschlossen: g'' = e^(−σt)·((σ² − ω_d²) sin ω_d t − 2σω_d cos ω_d t)/ω_d, je Abtastwert mal T
    const double wn = 2.0 * std::numbers::pi * 100.0, sigma = 0.5 * 0.02 * wn, wd = wn * std::sqrt(1.0 - 0.0001);
    // g'' springt bei t = 0 um 2σ; das bandbegrenzte Signal gibt den Sprung mit einem Nachschwingen wieder,
    // das wie 1/k abklingt (bei k = 1 rund 0,6 % des groessten Werts) — verglichen wird ab 1 ms, dort
    // rund 10⁻⁴; die ersten Werte werden berichtet
    double groesste = 0.0, fehler = 0.0, fehlerAnfang = 0.0;
    std::size_t wo = 0;
    for (std::size_t k = 1; k < 4800; ++k)
    {
        const double tk = static_cast<double>(k) * t;
        const double g2 = std::exp(-sigma * tk) * ((sigma * sigma - wd * wd) * std::sin(wd * tk) - 2.0 * sigma * wd * std::cos(wd * tk)) / wd;
        const double soll = 1.0 * g2 * t;
        groesste = std::max(groesste, std::abs(soll));
        if (k < 48)
            fehlerAnfang = std::max(fehlerAnfang, std::abs(h[k] - soll));
        else if (std::abs(h[k] - soll) > fehler)
        {
            fehler = std::abs(h[k] - soll);
            wo = k;
        }
    }
    MESSAGE("Einmassenschwinger: groesster Fehler ab 1 ms ", fehler / groesste, " vom groessten Wert (bei k = ", wo,
            "), davor ", fehlerAnfang / groesste);
    CHECK(fehler < 1e-3 * groesste);
    // der Stoss traegt a·δ: der erste Wert ist rund φG = 1
    CHECK(h[0] == doctest::Approx(1.0).epsilon(0.01));
    // Kraft → Druck: ein Impuls von einer Einheit (kHammerKraftBezug N) gibt die Antwort mal 200
    std::vector<float> stoss(10, 0.0f);
    stoss[0] = 1.0f;
    const std::vector<double> p = schalldruck(stoss, h);
    REQUIRE(p.size() == stoss.size() + h.size() - 1);
    for (std::size_t k = 0; k < 1000; ++k) CHECK(p[k] == doctest::Approx(kHammerKraftBezug * h[k]).epsilon(1e-9).scale(1e-9));
}

TEST_CASE("Loop Koerper 31c-7 Antwort: Laenge am Steinway")
{
    kp::Koerper koerper;
    REQUIRE_MESSAGE(ladeKoerper(QStringLiteral("asset/loop/instrument_bodies/steinway_d.lvinstbody"), koerper),
                    "die Test-Exe aus dem Projekt-Root starten");
    const double laenge = kp::antwortLaengeS(alleModen(koerper));
    MESSAGE("Laenge der Antwort am Steinway: ", laenge, " s");
    // bis S160 ueber 4 s (langsamste Mode 38 Hz mit eta 1,1 %); seit HU4-1 a faellt sie mit eta 3 % — rund 1,9 s
    CHECK(laenge > 1.5);
    CHECK(laenge <= 4.0);
    CHECK(stegpunktVon(koerper, 21) == 0);
    CHECK(stegpunktVon(koerper, 108) == 87);
    CHECK(stegpunktVon(koerper, 20) == -1);
}

TEST_CASE("Loop Koerper 31c-8 Antwort schnell gegen exakt (AW3-1 a)")
{
    // das Raster mit exaktem Kern schmaler Resonanzen gegen jede Mode an jedem Punkt — an jedem mitgelieferten Koerper,
    // je an zwei Stegpunkten und beiden Hoerorten, mit der vollen Laenge und den gezogenen Moden wie in der App:
    // groesster Fehler unter −80 dB der Spitze
    const QDir ordner(QStringLiteral("asset/loop/instrument_bodies"));
    const QStringList namen = ordner.entryList({QStringLiteral("*.lvinstbody")}, QDir::Files, QDir::Name);
    REQUIRE_MESSAGE(namen.size() >= 3, "die Test-Exe aus dem Projekt-Root starten");
    const int rate = 48000;
    for (const QString& name : namen)
    {
        CAPTURE(name.toStdString());
        kp::Koerper koerper;
        REQUIRE(ladeKoerper(ordner.filePath(name), koerper));
        const std::vector<kp::ModeImKoerper> moden = alleModen(koerper);
        const std::size_t n = zweierpotenzAb(static_cast<std::size_t>(std::ceil(kp::antwortLaengeS(moden) * rate)));
        double zeitExakt = 0.0, zeitSchnell = 0.0, schlimmster = 0.0;
        for (const int taste : {30, 80})
            for (std::size_t o = 0; o < 2; ++o)
            {
                const auto j = static_cast<std::size_t>(stegpunktVon(koerper, taste));
                auto t0 = std::chrono::steady_clock::now();
                const auto exakt = kp::antwortSpektrumExakt(moden, j, o, rate, n);
                zeitExakt += sekundenSeit(t0);
                t0 = std::chrono::steady_clock::now();
                const auto schnell = kp::antwortSpektrum(moden, j, o, rate, n);
                zeitSchnell += sekundenSeit(t0);
                REQUIRE(schnell.size() == exakt.size());
                double spitze = 0.0, fehler = 0.0;
                for (std::size_t k = 0; k < exakt.size(); ++k)
                {
                    spitze = std::max(spitze, std::abs(exakt[k]));
                    fehler = std::max(fehler, std::abs(schnell[k] - exakt[k]));
                }
                schlimmster = std::max(schlimmster, fehler / spitze);
            }
        MESSAGE(name.toStdString(), ": FFT ", n, ", groesster Fehler ", 20.0 * std::log10(schlimmster), " dB der Spitze; exakt ",
                zeitExakt / 4.0, " s, schnell ", zeitSchnell / 4.0, " s je Antwort (", zeitExakt / zeitSchnell, "-fach)");
        CHECK(schlimmster < 1e-4);
    }
}

TEST_CASE("Loop Koerper 31c-8 Antwort: laenger als 8 s (AW2 a)")
{
    // eine Mode bei 20 Hz, η = 0,5 % (wie die tiefste des duennen Glasbodens): bis −60 dB rund 22 s — die Antwort
    // reicht so weit (keine Grenze 8 s mehr), und am Anfang faltet sich nichts vom Ende zurueck
    kp::ModeImKoerper m;
    m.frequenzHz = 20.0;
    m.verlustfaktor = 0.005;
    m.amStegpunkt = {2.0};
    m.abstrahlung = {std::complex<double>(0.5, 0.0)};
    const std::vector<kp::ModeImKoerper> moden{m};
    const double wn = 2.0 * std::numbers::pi * 20.0, sigma = 0.5 * 0.005 * wn;
    const double laenge = kp::antwortLaengeS(moden);
    CHECK(laenge == doctest::Approx(std::log(1000.0) / sigma));
    CHECK(laenge > 20.0);
    const int rate = 8000;  // eine kleine Rate genuegt fuer 20 Hz und haelt den Fall kurz
    const double t = 1.0 / rate;
    const std::vector<double> h = bodenAntwort(moden, 0, 0, rate, laenge);
    REQUIRE(h.size() == static_cast<std::size_t>(std::ceil(laenge * rate)));
    // gegen die geschlossene Loesung (wie beim Einmassenschwinger) in der ersten Sekunde und um 8 s, ab 1 ms
    const double wd = wn * std::sqrt(1.0 - 0.005 * 0.005 / 4.0);
    double groesste = 0.0, fehler = 0.0;
    for (const auto& [von, bis] : {std::pair{8, rate}, std::pair{8 * rate - 400, 8 * rate + 400}})
        for (int k = von; k < bis; ++k)
        {
            const double tk = k * t;
            const double soll =
                std::exp(-sigma * tk) * ((sigma * sigma - wd * wd) * std::sin(wd * tk) - 2.0 * sigma * wd * std::cos(wd * tk)) / wd * t;
            groesste = std::max(groesste, std::abs(soll));
            fehler = std::max(fehler, std::abs(h[static_cast<std::size_t>(k)] - soll));
        }
    MESSAGE("Antwort ", laenge, " s; groesster Fehler gegen die geschlossene Loesung ", fehler / groesste, " vom groessten Wert");
    CHECK(fehler < 1e-3 * groesste);
    // um 8 s klingt sie noch deutlich (rund −22 dB des Anfangs) — frueher endete sie dort
    double um8 = 0.0;
    for (int k = 8 * rate - 400; k < 8 * rate + 400; ++k) um8 = std::max(um8, std::abs(h[static_cast<std::size_t>(k)]));
    CHECK(um8 > 0.05 * groesste);
}

TEST_CASE("Loop Koerper 31c-7 Ton durch den Boden (AW1 a, FT1 a, PG1 a)")
{
    tonspeicher().leere();  // die Antwort liegt im Tonspeicher des ganzen Programms
    Instrument ohne;
    Schicht& s = ohne.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::Modal;
    s.anregung = Anregung::Hammer;
    ohne.tastatur = Tastatur{};
    Instrument mit = ohne;
    mit.schichten[0].hammerSaite.resonanzboden.koerper = QStringLiteral("lib:steinway_d");
    const int rate = 48000;
    const std::shared_ptr<const BodenImKlang> boden = bodenVon(QStringLiteral("lib:steinway_d"));
    REQUIRE_MESSAGE(boden, "steinway_d.lvinstbody nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
    CHECK(boden->koerper.hoerorte[static_cast<std::size_t>(boden->links)] == "left");
    CHECK(boden->koerper.hoerorte[static_cast<std::size_t>(boden->rechts)] == "right");
    CHECK(bodenVon(QStringLiteral("lib:steinway_d")).get() == boden.get());  // einmal geladen

    const Klang a = rendereTon(ohne, 60, 500.0, rate);
    auto t0 = std::chrono::steady_clock::now();
    const Klang b = rendereTon(mit, 60, 500.0, rate);
    MESSAGE("Ton mit Boden: ", sekundenSeit(t0), " s (mit dem Bilden der Antwort)");
    const std::shared_ptr<const Klang> antwort = bodenAntwortFuer(*boden, 60, rate);
    REQUIRE(antwort);
    // ohne Boden die Kraft auf beiden Seiten gleich; mit Boden um die Antwort laenger und Stereo
    REQUIRE(b.frames() == a.frames() + antwort->frames() - 1);
    double unterschied = 0.0, spitze = 0.0;
    for (std::int64_t i = 0; i < b.frames(); ++i)
    {
        unterschied = std::max(unterschied, static_cast<double>(std::abs(b.daten[2 * i] - b.daten[2 * i + 1])));
        spitze = std::max(spitze, static_cast<double>(std::abs(b.daten[2 * i])));
    }
    MESSAGE("Spitze links ", 20.0 * std::log10(spitze), " dBFS, groesster Unterschied links/rechts ", unterschied);
    CHECK(unterschied > 0.01 * spitze);
    CHECK(spitze < 1.0);
    // links gegen die Faltung von Hand: Kraft (die linke Seite ohne Boden) mit der Antwort links
    std::vector<float> kraft(static_cast<std::size_t>(a.frames()));
    for (std::size_t i = 0; i < kraft.size(); ++i) kraft[i] = a.daten[2 * i];
    std::vector<double> hLinks(static_cast<std::size_t>(antwort->frames()));
    for (std::size_t i = 0; i < hLinks.size(); ++i) hLinks[i] = antwort->daten[2 * i];
    const std::vector<double> p = schalldruck(kraft, hLinks);
    double fehler = 0.0;
    for (std::size_t i = 0; i < p.size(); ++i) fehler = std::max(fehler, std::abs(p[i] / kSchalldruckBezug - b.daten[2 * i]));
    MESSAGE("links gegen die Faltung von Hand: ", fehler / spitze, " von der Spitze");
    CHECK(fehler < 1e-5 * spitze);
    // der zweite Ton nimmt die Antwort aus dem Tonspeicher und gleicht dem ersten Bit fuer Bit
    const std::uint64_t vorher = tonspeicher().stand().treffer;
    const Klang c = rendereTon(mit, 60, 500.0, rate);
    CHECK(tonspeicher().stand().treffer > vorher);
    CHECK(c.daten == b.daten);
    // einen Koerper, den es nicht gibt, hat der Ton nicht: er klingt wie ohne Boden
    Instrument falsch = ohne;
    falsch.schichten[0].hammerSaite.resonanzboden.koerper = QStringLiteral("lib:gibt_es_nicht");
    CHECK(bodenVon(QStringLiteral("lib:gibt_es_nicht")) == nullptr);
    CHECK(rendereTon(falsch, 60, 500.0, rate).daten == a.daten);
    tonspeicher().leere();
}

TEST_CASE("Loop Koerper 31c-7 Body not found (SB1 a)")
{
    // ein Instrument, dessen Koerper es nicht gibt: der Ton klingt ohne Boden, der Bericht sagt es einmal
    tonspeicher().leere();
    QTemporaryDir tmp;
    REQUIRE(tmp.isValid());
    Instrument instrument;
    Schicht& s = instrument.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::Modal;
    s.anregung = Anregung::Hammer;
    s.hammerSaite.resonanzboden.koerper = QStringLiteral("lib:gibt_es_nicht");
    instrument.tastatur = Tastatur{};
    const QString pfad = tmp.filePath(QStringLiteral("fluegel.lvinst"));
    REQUIRE(speichereInstrument(instrument, pfad));
    papp::PappEngine engine;
    QStringList bericht;
    const Klaenge klaenge = ladeKlaenge({tonKlang(pfad, 60, 200), tonKlang(pfad, 62, 200)}, engine, 48000, &bericht);
    REQUIRE(klaenge.size() == 2);
    REQUIRE(klaenge[0] != nullptr);
    CHECK(klaenge[0]->frames() > 0);
    CHECK(bericht == QStringList{QStringLiteral("Body not found: lib:gibt_es_nicht")});
    tonspeicher().leere();
}

TEST_CASE("Loop Koerper 31c Hoerproben (LUMIVIZ_31C_WAV)")
{
    // ZA1 a, VA1 a: je Koerper A0, C2, C4, C6, C8 bei mf, dazu C4 bei pp, mf, ff, Stereo; je Familie gegen ihre
    // Grundlage — der Steinway ohne Boden, mit Boden, mit den zwei Glasboeden (wie im Programm, `rendereTon`);
    // der MAESSTRO-Pruefstein RP gegen MP1–MP3 (Testdaten, nicht ueber `lib:` erreichbar — ueber die Bausteine
    // des Programms: Kraft der Hammer-Saite, Antwort aus dem Tonspeicher, Faltung)
    const QString ordner = qEnvironmentVariable("LUMIVIZ_31C_WAV");
    if (ordner.isEmpty())
    {
        MESSAGE("LUMIVIZ_31C_WAV=<Ordner> legt Hoerproben ab");
        return;
    }
    REQUIRE(QDir().mkpath(ordner));
    tonspeicher().leere();
    const int rate = 48000;
    const std::vector<std::pair<int, int>> mf{{21, -12}, {36, -12}, {60, -12}, {84, -12}, {108, -12}};
    const std::vector<std::pair<int, int>> c4{{60, -26}, {60, -12}, {60, 0}};
    // je Ton 1,2 s gehalten, alle 2,5 s einer; der Nachklang laeuft ueber den naechsten
    const auto abstand = static_cast<std::size_t>(2.5 * rate);
    const auto schreibe = [&](const QString& name, const std::vector<float>& folge) {
        float spitze = 0.0f;
        for (const float v : folge) spitze = std::max(spitze, std::abs(v));
        REQUIRE(schreibeWav(QDir(ordner).filePath(name + QStringLiteral(".wav")), folge, rate));
        MESSAGE(name.toStdString(), ": Spitze ", 20.0 * std::log10(std::max(spitze, 1e-9f)), " dBFS");
    };
    const auto lege = [&](const QString& name, const auto& ton, const std::vector<std::pair<int, int>>& toene) {
        std::vector<float> folge;
        std::size_t stelle = 0;
        for (const auto& [hoehe, stufe] : toene)
        {
            const std::vector<float> k = ton(hoehe, stufe);
            const std::size_t ab = 2 * stelle * abstand;
            if (folge.size() < ab + k.size()) folge.resize(ab + k.size(), 0.0f);
            for (std::size_t i = 0; i < k.size(); ++i) folge[ab + i] += k[i];
            ++stelle;
        }
        schreibe(name, folge);
    };

    // Familie Steinway: durch das Instrument wie im Programm
    Instrument ohne;
    Schicht& s = ohne.schichten[0];
    s.schwingung = Schwingung::Saite;
    s.saitenModell = SaitenModell::Modal;
    s.anregung = Anregung::Hammer;
    ohne.tastatur = Tastatur{};
    const auto mitKoerper = [&](const char* verweis) {
        Instrument i = ohne;
        i.schichten[0].hammerSaite.resonanzboden.koerper = QLatin1String(verweis);
        REQUIRE_MESSAGE(bodenVon(QLatin1String(verweis)), verweis, " nicht gefunden — die Test-Exe aus dem Projekt-Root starten");
        return i;
    };
    const std::vector<std::pair<QString, Instrument>> steinway{
        {QStringLiteral("ohne_boden"), ohne},
        {QStringLiteral("steinway_d"), mitKoerper("lib:steinway_d")},
        {QStringLiteral("glas_wie_holz"), mitKoerper("lib:steinway_d_glass")},
        {QStringLiteral("glas_gleiche_masse"), mitKoerper("lib:steinway_d_glass_same_mass")}};
    int nr = 1;
    for (const auto& [teil, satz] : {std::pair{QStringLiteral("mf"), mf}, std::pair{QStringLiteral("c4_pp_mf_ff"), c4}})
        for (const auto& [name, instrument] : steinway)
        {
            const Instrument& in = instrument;
            lege(QStringLiteral("%1_steinway_%2_%3").arg(nr++, 2, 10, QLatin1Char('0')).arg(teil, name),
                 [&](int hoehe, int stufe) {
                     TonAngabe angabe;
                     angabe.hoehe = hoehe;
                     angabe.dauerMs = 1200;
                     angabe.anschlagStufe = stufe;
                     return rendereTon(in, angabe, rate).daten;
                 },
                 satz);
        }

    // Familie MAESSTRO: die Kraft der Hammer-Saite (wie `rendereTon`: gehalten 1,2 s, Daempfer unter
    // `ungedaempftAb`), gefaltet mit der Antwort des Koerpers je Hoerort
    const QString testdaten = QStringLiteral("projects/apps/LumiViz/tests/unit/UnitTests/koerper/");
    const Tastatur tastatur{};
    for (const auto& [teil, satz] : {std::pair{QStringLiteral("mf"), mf}, std::pair{QStringLiteral("c4_pp_mf_ff"), c4}})
        for (const char* datei : {"maesstro_rp", "maesstro_mp1", "maesstro_mp2", "maesstro_mp3"})
        {
            auto boden = std::make_shared<BodenImKlang>();
            REQUIRE_MESSAGE(ladeKoerper(testdaten + QLatin1String(datei) + QStringLiteral(".lvinstbody"), boden->koerper), datei);
            REQUIRE(boden->koerper.hoerorte.size() == 2);  // left, right wie im Programm
            boden->moden = alleModen(boden->koerper);
            boden->laengeS = kp::antwortLaengeS(boden->moden);
            boden->abdruck = QStringLiteral("test:") + QLatin1String(datei);
            boden->links = 0;
            boden->rechts = 1;
            lege(QStringLiteral("%1_%2_%3").arg(nr++, 2, 10, QLatin1Char('0')).arg(QLatin1String(datei).mid(9), teil),
                 [&](int hoehe, int stufe) {
                     HammerAnschlag a;
                     a.taste = hoehe;
                     a.hz = tonHz(hoehe);
                     a.stufe = stufe;
                     a.halten = static_cast<std::int64_t>(1.2 * rate);
                     a.gedaempft = hoehe < tastatur.ungedaempftAb;
                     a.rate = rate;
                     std::vector<float> kraft;
                     rechneHammerTon(hammerSaiteVorgabe(), a, kraft, a.halten + hammerNachklang(hammerSaiteVorgabe(), a, std::int64_t{30} * rate));
                     const std::shared_ptr<const Klang> antwort = bodenAntwortFuer(*boden, hoehe, rate);
                     REQUIRE(antwort);
                     std::vector<float> stereo;
                     falteMitBoden(kraft, *antwort, stereo);
                     return stereo;
                 },
                 satz);
        }

    QFile liste(QDir(ordner).filePath(QStringLiteral("liste.txt")));
    REQUIRE(liste.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    liste.write("Hoerproben 31c (S151, ZA1 a, VA1 a): die Hammer-Saite des Steinway D durch verschiedene Resonanzboeden.\n"
                "Je Ton 1,2 s gehalten, alle 2,5 s einer; 48 kHz, Stereo (Hoerorte left/right 12 cm ueber den Saiten an\n"
                "MIDI 43 und 86). Teil mf: A0, C2, C4, C6, C8 bei mf (Stufe -12). Teil c4_pp_mf_ff: C4 bei pp, mf, ff\n"
                "(Stufen -26, -12, 0).\n\n"
                "Familie Steinway (01-08, durch das Instrument wie im Programm), je Teil gegeneinander hoeren:\n"
                "  ohne_boden          die Stegkraft, beide Seiten gleich (Einheiten von 200 N - nicht gleich laut gestellt)\n"
                "  steinway_d          der Fichtenboden nach RT-0425 (Grundlage)\n"
                "  glas_wie_holz       Platte aus Floatglas, Dicke wie Holz (6-10 mm), Rippen und Steg Holz\n"
                "  glas_gleiche_masse  Platte aus Floatglas mit der Masse der Holzplatte (0,9-1,5 mm)\n"
                "  Gedachte Instrumente: Bruch nicht gerechnet, in der Daempfung des Glases fehlen Rand und Abstrahlung -\n"
                "  sie klingen vermutlich laenger als echt (DM1 a).\n\n"
                "Familie MAESSTRO (09-16, ueber die Bausteine: Kraft, Antwort, Faltung), je Teil gegeneinander hoeren:\n"
                "  rp   der vereinfachte Steinway von MAESSTRO (Platte 8 mm, zwei Stege, 17 Rippen; Grundlage)\n"
                "  mp1  Platte 16 mm (steifer)\n"
                "  mp2  jede zweite Rippe (doppelter Rippenabstand)\n"
                "  mp3  ohne Rippen\n\n"
                "Pegel mit Boden in Einheiten von 97,3 Pa (kSchalldruckBezug), unnormiert: ob ein Boden lauter oder leiser\n"
                "klingt, gehoert zu ihm. Die Familien sind untereinander nicht gleich gestellt (MAESSTRO ohne Huellkurve der\n"
                "Schicht).\n");
    tonspeicher().leere();
}

namespace
{

/// Der Messteil an einem Koerper: Zeit je Antwort, Ton, Faltung und Schalldruck je Taste.
void messeKoerper(const kp::Koerper& koerper)
{
    const int rate = 48000;
    auto t0 = std::chrono::steady_clock::now();
    const std::vector<kp::ModeImKoerper> moden = alleModen(koerper);
    MESSAGE("Moden: ", koerper.moden.size(), " gerechnet, ", moden.size() - koerper.moden.size(), " gezogen; Ziehen ",
            sekundenSeit(t0), " s");
    const double laengeS = kp::antwortLaengeS(moden);
    const auto frames = static_cast<std::size_t>(std::ceil(laengeS * rate));
    MESSAGE("Antwort: ", laengeS, " s, ", frames, " Abtastwerte, FFT ", zweierpotenzAb(frames), "; als float ",
            frames * 4.0 / 1048576.0, " MB je Antwort, 88 x 2 Hoerorte ", 176.0 * frames * 4.0 / 1048576.0, " MB");

    // (1) Zeit je Antwort, ein Thread
    for (const int taste : {21, 40, 41, 60, 88, 108})
    {
        t0 = std::chrono::steady_clock::now();
        const std::vector<double> h = bodenAntwort(moden, static_cast<std::size_t>(stegpunktVon(koerper, taste)), 0, rate, laengeS);
        MESSAGE("Antwort Taste ", taste, ": ", sekundenSeit(t0), " s");
    }

    // (2) Ton, Faltung und Schalldruck je Taste bei der hoechsten Anschlagstufe (0), eine Sekunde gehalten
    const HammerSaite& saite = hammerSaiteVorgabe();
    const Tastatur tastatur{};
    struct Ergebnis
    {
        int taste = 0;
        double spitzePa[2] = {0.0, 0.0};
        double tonS = 0.0, antwortS = 0.0, faltungS = 0.0;
        std::size_t tonFrames = 0;
    };
    const auto rechne = [&](int taste) {
        Ergebnis e;
        e.taste = taste;
        HammerAnschlag a;
        a.taste = taste;
        a.hz = tonHz(taste);
        a.stufe = 0;
        a.halten = rate;
        a.gedaempft = taste < tastatur.ungedaempftAb;
        a.rate = rate;
        auto t = std::chrono::steady_clock::now();
        const std::int64_t frames = a.halten + hammerNachklang(saite, a, std::int64_t{30} * rate);
        std::vector<float> kraft;
        rechneHammerTon(saite, a, kraft, frames);
        e.tonS = sekundenSeit(t);
        e.tonFrames = kraft.size();
        const auto j = static_cast<std::size_t>(stegpunktVon(koerper, taste));
        for (std::size_t o = 0; o < 2; ++o)
        {
            t = std::chrono::steady_clock::now();
            const std::vector<double> h = bodenAntwort(moden, j, o, rate, laengeS);
            e.antwortS += sekundenSeit(t);
            t = std::chrono::steady_clock::now();
            const std::vector<double> p = schalldruck(kraft, h);
            e.faltungS += sekundenSeit(t);
            for (const double v : p) e.spitzePa[o] = std::max(e.spitzePa[o], std::abs(v));
        }
        return e;
    };
    t0 = std::chrono::steady_clock::now();
    std::vector<Ergebnis> alle;
    for (int von = 21; von <= 108; von += 8)  // je acht zugleich — eine Taste braucht rund 70 MB
    {
        std::vector<std::future<Ergebnis>> laufend;
        for (int taste = von; taste < von + 8 && taste <= 108; ++taste) laufend.push_back(std::async(std::launch::async, rechne, taste));
        for (auto& f : laufend) alle.push_back(f.get());
    }
    MESSAGE("88 Tasten, je acht zugleich: ", sekundenSeit(t0), " s");
    const Ergebnis* lauteste = &alle.front();
    double faltungMax = 0.0, antwortMax = 0.0;
    std::size_t tonMax = 0;
    for (const Ergebnis& e : alle)
    {
        if (std::max(e.spitzePa[0], e.spitzePa[1]) > std::max(lauteste->spitzePa[0], lauteste->spitzePa[1])) lauteste = &e;
        faltungMax = std::max(faltungMax, e.faltungS / 2.0);
        antwortMax = std::max(antwortMax, e.antwortS / 2.0);
        tonMax = std::max(tonMax, e.tonFrames);
        if (e.taste % 12 == 0 || e.taste == 21 || e.taste == 108)
            MESSAGE("Taste ", e.taste, ": Spitze ", 20.0 * std::log10(e.spitzePa[0] / 20e-6), " / ",
                    20.0 * std::log10(e.spitzePa[1] / 20e-6), " dB SPL (left / right), Ton ", e.tonFrames / double(rate),
                    " s, Antwort je Hoerort ", e.antwortS / 2.0, " s, Faltung je Hoerort ", e.faltungS / 2.0, " s");
    }
    const double spitze = std::max(lauteste->spitzePa[0], lauteste->spitzePa[1]);
    MESSAGE("lautester Ton: Taste ", lauteste->taste, ", Spitze ", spitze, " Pa = ", 20.0 * std::log10(spitze / 20e-6),
            " dB SPL; kSchalldruckBezug fuer −6 dBFS Spitze: ", spitze / std::pow(10.0, -6.0 / 20.0), " Pa");
    MESSAGE("hoechstens (parallel gemessen): Antwort ", antwortMax, " s, Faltung ", faltungMax, " s; laengster Ton ",
            tonMax / double(rate), " s");
    MESSAGE("beim festen Bezug (kSchalldruckBezug ", kSchalldruckBezug, " Pa): lautester Ton ",
            20.0 * std::log10(spitze / kSchalldruckBezug), " dBFS");
}

}  // namespace

TEST_CASE("Loop Koerper 31c-7 Messteil (AW1 a, FT1 a, PG1 a)")
{
    if (qEnvironmentVariableIntValue("LUMIVIZ_31C7_MESSUNG") != 1)
    {
        MESSAGE("LUMIVIZ_31C7_MESSUNG nicht gesetzt — der Messteil laeuft nicht");
        return;
    }
    // jeder mitgelieferte Koerper (S151, 31c-8: PG1 a kippt, wenn eine Variante beim festen Bezug ueber 0 dBFS geht)
    const QDir ordner(QStringLiteral("asset/loop/instrument_bodies"));
    const QStringList namen = ordner.entryList({QStringLiteral("*.lvinstbody")}, QDir::Files, QDir::Name);
    REQUIRE_MESSAGE(!namen.isEmpty(), "die Test-Exe aus dem Projekt-Root starten");
    for (const QString& name : namen)
    {
        MESSAGE("=== ", name.toStdString());
        kp::Koerper koerper;
        REQUIRE(ladeKoerper(ordner.filePath(name), koerper));
        messeKoerper(koerper);
    }
}
