/**
 ****************************************************************************************
 * @file   test_LoopFreezeSpielen.cpp
 * @brief  Loop: Freeze — eingefrorene Clips spielen ihre Wellenform
 *         (docs/audio/Spielweisen_Konzept.md §4.4, Stufe 30f; Entscheide F3 a, FV1 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton und ohne Engine; die Instrumente brauchen eine `QCoreApplication`, das
 * Beispiel `effects_demo` wird aus dem Repo gelesen.
 *
 * - Die Namen: ein eingefrorener Clip nennt `frozen:<abdruck>` statt seiner Toene; ein
 *   Muster, das nur in eingefrorenen Clips liegt, nennt keine; ohne Angabe wie bisher.
 * - Der Rechenkern: eine Stimme ab dem Anfang des Clips, mit Fader und Lautstaerke des
 *   Loops, ohne Bus und ohne Expression — und ohne sich selbst abzuschneiden: der
 *   Nachklang klingt ueber die Naht unter den naechsten Durchlaeufen weiter (FV1 a).
 * - Gegen live: ein eingefrorener Clip mit Echo klingt im ersten Durchlauf wie live (auf
 *   1e-5 — die Faktoren werden in anderer Reihenfolge malgenommen).
 * - Stufe 30g-1: Marken (`setzeFrozenMarken`, `frischeFrozenMarken`, `loeseFrozenMarken`), der
 *   Zustand eines Clips und der `FreezeAuftrag` (ablegen, Abbruch) — Dateien in einem
 *   Wegwerf-Ordner (`setzeFrozenOrdnerFuerTests`).
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "loop/Bearbeiten.hpp"
#include "loop/Freeze.hpp"
#include "loop/FreezeAuftrag.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Rechenkern.hpp"
#include "loop/Tonspeicher.hpp"
#include "loop/Wellenform.hpp"

#include <QCoreApplication>
#include <QTemporaryDir>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct SpielenApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

const QString kA = QString(40, QLatin1Char('a'));
const QString kB = QString(40, QLatin1Char('b'));

/// Drums in Kanal 0 (Takt 0, zwei Takte), Keys in Kanal 1 (Takt 1 und Takt 4), dazu ein
/// Drum-Muster ohne Clip.
Loop probeLoop()
{
    Muster grund;
    grund.name = QStringLiteral("Grund");
    grund.takte = 1;
    grund.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
    grund.zeilen = {std::vector<double>(16, 0.0)};
    grund.zeilen[0][0] = 1.0;
    Muster keys;
    keys.name = QStringLiteral("Keys");
    keys.art = MusterArt::Instrument;
    keys.instrument = QStringLiteral("piano");
    keys.takte = 1;
    keys.noten = {Note{0, 4, 60}, Note{8, 4, 64}};
    Muster ohne;
    ohne.name = QStringLiteral("Ohne");
    ohne.takte = 1;
    ohne.spuren = {{QStringLiteral("Snare"), QStringLiteral("kit:snare"), 1.0, 0.0, false}};
    ohne.zeilen = {std::vector<double>(16, 0.0)};
    ohne.zeilen[0][4] = 1.0;
    Loop loop;
    loop.muster = {grund, keys, ohne};
    loop.kanaele = {{QStringLiteral("Drums"), true, {{QStringLiteral("Grund"), 0, 2}}},
                    {QStringLiteral("Keys"), true, {{QStringLiteral("Keys"), 1, 1}, {QStringLiteral("Keys"), 4, 1}}}};
    loop.richte();
    return loop;
}

bool nenntToene(const QStringList& namen)
{
    return std::any_of(namen.begin(), namen.end(), [](const QString& n) { return istTon(n); });
}

}  // namespace

TEST_SUITE("Loop Freeze Spielen")
{
    TEST_CASE("Loop Freeze 30f: die Namen — ein eingefrorener Clip nennt seine Wellenform, nicht seine Toene")
    {
        SpielenApp app;
        const SaitenVon saiten = saitenAusInstrumenten();
        const Loop loop = probeLoop();
        const QStringList ohne = loop.klangNamen(saiten);
        REQUIRE(nenntToene(ohne));
        CHECK(loop.klangNamen(saiten, {}) == ohne);  // ohne Angabe wie bisher

        CHECK(frozenKlang(kA) == QStringLiteral("frozen:") + kA);
        CHECK(abdruckVonKlang(frozenKlang(kA)) == kA);
        CHECK(abdruckVonKlang(QStringLiteral("kit:kick")).isEmpty());
        CHECK_FALSE(istTon(frozenKlang(kA)));

        SUBCASE("einer von zwei Clips eines Musters: die Toene bleiben fuer den anderen")
        {
            const QStringList namen = loop.klangNamen(saiten, {{{1, 1}, kA}});
            CHECK(namen.last() == frozenKlang(kA));
            CHECK(nenntToene(namen));
            for (const QString& n : ohne) CHECK(namen.contains(n));
        }
        SUBCASE("beide Clips: keine Toene mehr; das Muster ohne Clip nennt seine Klaenge wie bisher")
        {
            const QStringList namen = loop.klangNamen(saiten, {{{1, 1}, kA}, {{1, 4}, kB}});
            CHECK_FALSE(nenntToene(namen));
            CHECK(namen.contains(QStringLiteral("kit:kick")));
            CHECK(namen.contains(QStringLiteral("kit:snare")));
            REQUIRE(namen.size() >= 2);
            CHECK(namen[namen.size() - 2] == frozenKlang(kA));  // in der Reihenfolge der Clips
            CHECK(namen.last() == frozenKlang(kB));
            // gleiche Clips teilen eine Wellenform: ein Name
            const QStringList geteilt = loop.klangNamen(saiten, {{{1, 1}, kA}, {{1, 4}, kA}});
            CHECK(geteilt.count(frozenKlang(kA)) == 1);
        }
        SUBCASE("ein Eintrag ohne Clip an dieser Stelle zaehlt nicht")
        {
            CHECK(loop.klangNamen(saiten, {{{1, 2}, kA}}) == ohne);
        }
    }

    TEST_CASE("Loop Freeze 30f, FV1 a: der Rechenkern — eine Stimme ab dem Clip, Fader und Lautstaerke live, ohne Schnitt ueber die Naht")
    {
        // ein Takt bei 120 BPM: der Durchlauf ist 2 s lang, die Wellenform 5 s
        Muster m;
        m.name = QStringLiteral("M");
        m.takte = 1;
        m.spuren = {{QStringLiteral("Kick"), QStringLiteral("kit:kick"), 1.0, 0.0, false}};
        m.zeilen = {std::vector<double>(16, 0.0)};
        m.zeilen[0][0] = 1.0;
        Loop loop;
        loop.laut = 0.5;
        loop.muster = {m};
        LoopClip clip(QStringLiteral("M"), 0, 1);
        ClipHall hall;
        hall.an = true;
        clip.effekte.reihe = {hall};               // steckt in der Wellenform: kein Bus
        clip.expression = {SpurPunkt{0.0, 0.5}};   // ebenso: keine Expression
        loop.kanaele = {{QStringLiteral("A"), true, {clip}}};
        loop.kanaele[0].gainDb = 6.0;
        loop.richte();
        const Eingefroren eingefroren{{{0, 0}, kA}};

        const Ablauf ablauf = plane(loop, kRate, {}, eingefroren);
        REQUIRE(ablauf.durchlaufFrames == 2 * kRate);
        CHECK(ablauf.klangNamen == QStringList{frozenKlang(kA)});  // die Kick braucht keiner mehr
        CHECK(ablauf.busse.empty());
        CHECK(ablauf.ausdruecke.empty());
        REQUIRE(ablauf.stimmen.size() == 1);
        const Stimme& stimme = ablauf.stimmen.front();
        CHECK_FALSE(stimme.schneidet);
        CHECK(stimme.klang == 0);
        REQUIRE(stimme.schlaege.size() == 1);
        const auto faktor = static_cast<float>(loop.kanaele[0].gainFaktor() * loop.laut);
        CHECK(stimme.schlaege.front().frame == 0);
        CHECK(stimme.schlaege.front().links == faktor);
        CHECK(stimme.schlaege.front().rechts == faktor);
        CHECK(stimme.schlaege.front().bus == -1);
        CHECK(stimme.schlaege.front().ausdruck == -1);
        // ohne Angabe derselbe Loop wie bisher: mit Bus, Expression und schneidender Stimme
        const Ablauf live = plane(loop, kRate);
        CHECK(live.busse.size() == 1);
        CHECK(live.ausdruecke.size() == 1);
        REQUIRE_FALSE(live.stimmen.empty());
        CHECK(live.stimmen.front().schneidet);

        // die Wellenform: eine Rampe, rechts halb so laut und verkehrt
        const std::int64_t laenge = 5 * kRate + 1234;
        Klang welle;
        welle.daten.resize(2 * static_cast<std::size_t>(laenge));
        for (std::int64_t i = 0; i < laenge; ++i)
        {
            const auto w = static_cast<float>(static_cast<double>(i % 977) / 977.0);
            welle.daten[static_cast<std::size_t>(2 * i)] = w;
            welle.daten[static_cast<std::size_t>(2 * i + 1)] = -0.5f * w;
        }
        const Klaenge klaenge{std::make_shared<const Klang>(welle)};
        const std::int64_t d = ablauf.durchlaufFrames;
        constexpr int kBlock = 4096;
        // erwartet: jeder Durchlauf, dessen Wellenform hier noch klingt, ungeschnitten
        for (const std::int64_t von : {std::int64_t{0}, std::int64_t{50000}, d - 100, 3 * d + 123, 7 * d - 500})
        {
            CAPTURE(von);
            std::vector<float> aus(2 * kBlock);
            rechne(ablauf, klaenge, von, kBlock, aus.data());
            double abweichung = 0.0;
            int ueberlappt = 0;
            for (int i = 0; i < kBlock; ++i)
            {
                const std::int64_t t = von + i;
                double links = 0.0;
                double rechts = 0.0;
                int klingen = 0;
                for (std::int64_t s = (t / d) * d; s >= 0; s -= d)
                {
                    if (t - s >= laenge) break;
                    links += welle.daten[static_cast<std::size_t>(2 * (t - s))] * faktor;
                    rechts += welle.daten[static_cast<std::size_t>(2 * (t - s) + 1)] * faktor;
                    ++klingen;
                }
                ueberlappt = std::max(ueberlappt, klingen);
                abweichung = std::max({abweichung, std::abs(aus[static_cast<std::size_t>(2 * i)] - links),
                                       std::abs(aus[static_cast<std::size_t>(2 * i + 1)] - rechts)});
            }
            CHECK(abweichung < 1e-5);
            if (von >= 3 * d) CHECK(ueberlappt == 3);  // 5 s Wellenform ueber 2-s-Durchlaeufe: drei zugleich
        }

        // ein Kanal, der aus ist, schweigt — auch eingefroren
        Loop aus = loop;
        aus.kanaele[0].an = false;
        CHECK(plane(aus, kRate, {}, eingefroren).stimmen.empty());
    }

    TEST_CASE("Loop Freeze 30f: gegen live — ein eingefrorener Clip mit Echo klingt im ersten Durchlauf wie live")
    {
        SpielenApp app;
        Loop loop;
        REQUIRE(ladeLoop(looptest::repoWurzel().filePath(QStringLiteral("asset/loop/effects_demo.lvloop")), loop));
        const SaitenVon saiten = saitenAusInstrumenten();
        const ClipOrt echo{2, 4};  // Keys, Piano stabs mit Echo
        REQUIRE(findeClip(loop, echo) >= 0);
        // im Kanal nur dieser Clip: sonst schnitte live ein spaeterer Clip desselben Musters seine Toene ab
        Kanal& keys = loop.kanaele[2];
        std::erase_if(keys.clips, [&](const LoopClip& c) { return c.startTakt != echo.startTakt; });
        REQUIRE(keys.clips.size() == 1);
        REQUIRE(keys.clips.front().effekteWirken());
        keys.gainDb = 3.0;
        REQUIRE(loop.laut < 1.0);

        // die Wellenform des Clips
        const std::optional<Loop> fassung = clipFassung(loop, echo);
        REQUIRE(fassung.has_value());
        const auto klaengeFassung =
            std::make_shared<const Klaenge>(looptest::klaengeOhneEngine(plane(*fassung, kRate, saiten)));
        const std::optional<Wellenform> welle = rechneClip(loop, echo, kRate, klaengeFassung, saiten);
        REQUIRE(welle.has_value());

        // live und eingefroren, ab dem Anfang des Clips bis zum Ende des Durchlaufs
        const Ablauf ablaufLive = plane(loop, kRate, saiten);
        const std::int64_t anfang =
            std::llround(echo.startTakt * loop.schritteJeTakt() * ablaufLive.schrittFrames);
        const std::int64_t frames = ablaufLive.durchlaufFrames - anfang;
        REQUIRE(frames > welle->clipFrames);
        const auto spiele = [&](std::shared_ptr<const Programm> programm) {
            Klangerzeuger erzeuger;
            erzeuger.setzeProgramm(std::move(programm));
            erzeuger.setzeStelle(anfang);
            std::vector<float> daten(2 * static_cast<std::size_t>(frames));
            for (std::int64_t ab = 0; ab < frames; ab += kEffektBlock)
                erzeuger.render(daten.data() + 2 * ab, static_cast<int>(std::min<std::int64_t>(kEffektBlock, frames - ab)));
            return daten;
        };
        const std::vector<float> live = spiele(baueProgramm(
            loop, kRate, std::make_shared<const Klaenge>(looptest::klaengeOhneEngine(ablaufLive)), saiten));

        const Eingefroren eingefroren{{{echo.kanal, echo.startTakt}, kA}};
        const Ablauf ablaufEis = plane(loop, kRate, saiten, eingefroren);
        Klaenge eis = looptest::klaengeOhneEngine(ablaufEis);
        const auto platz = ablaufEis.klangNamen.indexOf(frozenKlang(kA));
        REQUIRE(platz >= 0);
        CHECK(eis[static_cast<std::size_t>(platz)] == nullptr);  // den Namen kennt der Lader des Tests nicht
        eis[static_cast<std::size_t>(platz)] = std::make_shared<const Klang>(welle->klang);
        CHECK(ablaufEis.busse.size() + 1 == ablaufLive.busse.size());  // sein Echo steckt in der Wellenform
        const std::vector<float> gefroren =
            spiele(baueProgramm(loop, kRate, std::make_shared<const Klaenge>(std::move(eis)), saiten, eingefroren));

        double abweichung = 0.0;
        double spitze = 0.0;
        for (std::size_t i = 0; i < live.size(); ++i)
        {
            abweichung = std::max(abweichung, static_cast<double>(std::abs(live[i] - gefroren[i])));
            spitze = std::max(spitze, static_cast<double>(std::abs(live[i])));
        }
        MESSAGE("groesste Abweichung " << abweichung << " bei einer Spitze von " << spitze);
        CHECK(spitze > 0.01);
        CHECK(abweichung < 1e-5);
    }

    TEST_CASE("Loop Freeze 30g-1: Marken und Auftrag — einfrieren, veraendert, beim Speichern neu, Undo, loesen")
    {
        SpielenApp app;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        setzeFrozenOrdnerFuerTests(ordner.path());
        tonspeicher().leere();
        const SaitenVon saiten = saitenAusInstrumenten();
        const Dekodierer nichts = [](const QString&, int, double) { return std::vector<float>{}; };

        // zwei Clips desselben Musters (gleiche Clips teilen eine Datei, F1 a)
        Muster keys;
        keys.name = QStringLiteral("Keys");
        keys.art = MusterArt::Instrument;
        keys.instrument = QStringLiteral("piano");
        keys.takte = 1;
        keys.noten = {Note{0, 4, 60}, Note{8, 4, 64}};
        Loop loop;
        loop.muster = {keys};
        loop.kanaele = {{QStringLiteral("Keys"), true, {{QStringLiteral("Keys"), 0, 1}, {QStringLiteral("Keys"), 2, 1}}}};
        loop.richte();
        const ClipOrt erster{0, 0};
        const ClipOrt zweiter{0, 2};
        CHECK(frozenZustand(loop, erster, kRate, saiten) == FrozenZustand::Aus);

        // Freeze clip: die Marke steht, die Datei fehlt noch — zu rechnen
        CHECK(setzeFrozenMarken(loop, {erster}, kRate, saiten) == std::vector<ClipOrt>{erster});
        CHECK(loop.kanaele[0].clips[0].frozen == QString::fromLatin1(clipAbdruck(loop, erster, kRate, saiten)));
        CHECK(frozenZustand(loop, erster, kRate, saiten) == FrozenZustand::Fehlt);
        CHECK(fehlendeFrozen(loop, kRate, saiten) == std::vector<ClipOrt>{erster});
        {
            FreezeAuftrag auftrag(loop, {erster}, kRate, nichts);
            auftrag.warte();
            REQUIRE(auftrag.beendet());
            const std::optional<FreezeErgebnis> e = auftrag.ergebnis();
            REQUIRE(e.has_value());
            CHECK_FALSE(e->abgebrochen);
            CHECK(e->gelegt == std::vector<ClipOrt>{erster});
            CHECK(e->fehlgeschlagen.empty());
            CHECK(auftrag.stand().beendet);
        }
        CHECK(frozenZustand(loop, erster, kRate, saiten) == FrozenZustand::Frozen);
        CHECK(eingefrorenVon(loop, kRate, saiten).size() == 1);
        // der zweite, gleiche Clip: seine Datei liegt schon vor
        CHECK(setzeFrozenMarken(loop, {zweiter}, kRate, saiten).empty());
        CHECK(frozenZustand(loop, zweiter, kRate, saiten) == FrozenZustand::Frozen);
        CHECK(frozenStand().dateien == 1);
        const Loop eingefroren = loop;

        // bearbeitet: veraendert, spielt live; beim Speichern neu (F2 a)
        loop.muster[0].noten[1].hoehe = 67;
        loop.richte();
        CHECK(frozenZustand(loop, erster, kRate, saiten) == FrozenZustand::Veraendert);
        CHECK(eingefrorenVon(loop, kRate, saiten).empty());
        CHECK(fehlendeFrozen(loop, kRate, saiten).empty());  // veraendert ist nicht „fehlt"
        const std::vector<ClipOrt> neu = frischeFrozenMarken(loop, kRate, saiten);
        CHECK(neu == std::vector<ClipOrt>{erster, zweiter});
        CHECK(frozenZustand(loop, erster, kRate, saiten) == FrozenZustand::Fehlt);
        {
            FreezeAuftrag auftrag(loop, neu, kRate, nichts);
            auftrag.warte();
            CHECK(auftrag.ergebnis()->gelegt == neu);  // der zweite fand die Datei des ersten
        }
        CHECK(frozenZustand(loop, zweiter, kRate, saiten) == FrozenZustand::Frozen);
        CHECK(frozenStand().dateien == 2);  // ueberschrieben wird nie (F1 a)

        // Undo auf den eingefrorenen Stand: sofort frozen, die alte Datei liegt noch
        CHECK(frozenZustand(eingefroren, erster, kRate, saiten) == FrozenZustand::Frozen);

        // Unfreeze clip: die Marke geht, die Datei bleibt
        Loop geloest = loop;
        loeseFrozenMarken(geloest, {erster, zweiter});
        CHECK(frozenZustand(geloest, erster, kRate, saiten) == FrozenZustand::Aus);
        CHECK(eingefrorenVon(geloest, kRate, saiten).empty());
        CHECK(frozenStand().dateien == 2);

        // geraeumt (Clear frozen clips): unveraendert, aber die Datei fehlt — neu zu rechnen (F5 a)
        CHECK(leereFrozen() == 2);
        CHECK(fehlendeFrozen(loop, kRate, saiten) == std::vector<ClipOrt>{erster, zweiter});

        setzeFrozenOrdnerFuerTests({});
        tonspeicher().leere();
    }

    TEST_CASE("Loop Freeze 30g-1: der Auftrag laesst sich abbrechen — ohne Datei; Stand waehrend der Toene")
    {
        SpielenApp app;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        setzeFrozenOrdnerFuerTests(ordner.path());
        tonspeicher().leere();
        // eine Drum-Spur mit einer Datei: der Dekodierer haelt, bis der Fall ihn freigibt
        std::atomic<bool> halten{true};
        std::atomic<bool> gerufen{false};
        const Dekodierer haelt = [&](const QString&, int, double) {
            gerufen = true;
            while (halten.load()) std::this_thread::yield();
            return std::vector<float>(200, 0.5f);
        };
        Muster m;
        m.name = QStringLiteral("M");
        m.takte = 1;
        m.spuren = {{QStringLiteral("A"), QStringLiteral("C:/s/haelt.wav"), 1.0, 0.0, false}};
        m.zeilen = {std::vector<double>(16, 0.0)};
        m.zeilen[0][0] = 1.0;
        Loop loop;
        loop.muster = {m};
        loop.kanaele = {{QStringLiteral("A"), true, {{QStringLiteral("M"), 0, 1}}}};
        loop.richte();
        const ClipOrt ort{0, 0};
        (void)setzeFrozenMarken(loop, {ort}, kRate, saitenAusInstrumenten());

        FreezeAuftrag auftrag(loop, {ort}, kRate, haelt);
        while (!gerufen.load()) std::this_thread::yield();
        const FreezeStand stand = auftrag.stand();
        CHECK(stand.clips == 1);
        CHECK(stand.clip == 0);
        CHECK_FALSE(stand.beendet);
        CHECK(stand.anteil < 0.5);
        auftrag.brichAb();
        halten = false;
        auftrag.warte();
        REQUIRE(auftrag.ergebnis().has_value());
        CHECK(auftrag.ergebnis()->abgebrochen);
        CHECK(auftrag.ergebnis()->gelegt.empty());
        CHECK(frozenStand().dateien == 0);
        CHECK(frozenZustand(loop, ort, kRate, saitenAusInstrumenten()) == FrozenZustand::Fehlt);

        setzeFrozenOrdnerFuerTests({});
        tonspeicher().leere();
    }
}
