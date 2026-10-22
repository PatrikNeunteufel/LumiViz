/**
 ****************************************************************************************
 * @file   test_LoopWellenform.cpp
 * @brief  Loop: ein Clip als Wellenform (docs/audio/Spielweisen_Konzept.md §4.4,
 *         Stufe 30a; Entscheide F3 a, WX3 a, WX4 a, WX5 a)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * Ohne Ton, ohne Fenster, ohne Audio-Engine; die mitgelieferten Instrumente
 * brauchen eine `QCoreApplication`. Gerechnet an den Beispielen `effects_demo`
 * (Clips mit Echo, Hall und Drive, ein Fader +4,5 dB, Lautstaerke 0,3) und
 * `tabs_demo` (Saiten). Geprueft: ein Clip gleicht Bit fuer Bit dem, was der
 * Klangerzeuger live an derselben Stelle rechnet; Fader und Lautstaerke des
 * Loops wirken nach den Effekten und lassen sich danach wieder dazugeben
 * (WX5 a); der Nachklang klingt aus, ohne zum Anfang umzuspringen, und endet in
 * Stille; ein Loop-Clip des Composers wiederholt den Durchlauf wie die
 * Tonfuehrung; Fortschritt und Abbruch. Der letzte Fall meldet nur die
 * Rechenzeit.
 ****************************************************************************************
 */

#include <doctest.h>

#include "LoopOhneEngine.hpp"
#include "loop/Instrument.hpp"
#include "loop/Klangerzeuger.hpp"
#include "loop/Loop.hpp"
#include "loop/Saitenplan.hpp"
#include "loop/Spielweise.hpp"
#include "loop/Wellenform.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

using namespace lumi::loop;

namespace
{

constexpr int kRate = 44100;

struct WellenApp
{
    int argc = 1;
    char arg0[10] = "UnitTests";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

Loop beispiel(const char* datei)
{
    Loop loop;
    REQUIRE(ladeLoop(looptest::repoWurzel().filePath(QStringLiteral("asset/loop/") + QLatin1String(datei)), loop));
    return loop;
}

std::shared_ptr<const Klaenge> klaengeFuer(const Loop& loop, const SaitenVon& saiten)
{
    return std::make_shared<const Klaenge>(looptest::klaengeOhneEngine(plane(loop, kRate, saiten)));
}

/// `frames` Abtastwerte ab `stelle` durch den Klangerzeuger, in Bloecken wie im Mixer.
std::vector<float> live(const Loop& loop, const SaitenVon& saiten, std::int64_t stelle, std::int64_t frames)
{
    Klangerzeuger erzeuger;
    erzeuger.setzeProgramm(baueProgramm(loop, kRate, klaengeFuer(loop, saiten), saiten));
    erzeuger.setzeStelle(stelle);
    std::vector<float> daten(2 * static_cast<std::size_t>(frames));
    for (std::int64_t ab = 0; ab < frames; ab += kEffektBlock)
        erzeuger.render(daten.data() + 2 * ab, static_cast<int>(std::min<std::int64_t>(kEffektBlock, frames - ab)));
    return daten;
}

/// Der Loop, wie er live nur diesen Clip spielt: die anderen Kanaele aus, im Kanal nur er.
Loop nurDieser(const Loop& loop, ClipOrt ort, bool mitFaderUndLaut)
{
    Loop allein = loop;
    for (std::size_t k = 0; k < allein.kanaele.size(); ++k)
        if (static_cast<int>(k) != ort.kanal) allein.kanaele[k].an = false;
    Kanal& kanal = allein.kanaele[static_cast<std::size_t>(ort.kanal)];
    std::erase_if(kanal.clips, [&](const LoopClip& c) { return c.startTakt != ort.startTakt; });
    if (!mitFaderUndLaut)
    {
        kanal.gainDb = 0.0;
        allein.laut = 1.0;
    }
    return allein;
}

std::int64_t anfangVon(const Loop& loop, ClipOrt ort)
{
    return std::llround(ort.startTakt * loop.schritteJeTakt() * plane(loop, kRate).schrittFrames);
}

double groessterBetrag(const std::vector<float>& daten, std::size_t vonFrame, std::size_t bisFrame)
{
    double spitze = 0.0;
    for (std::size_t i = 2 * vonFrame; i < std::min(daten.size(), 2 * bisFrame); ++i)
        spitze = std::max(spitze, static_cast<double>(std::abs(daten[i])));
    return spitze;
}

}  // namespace

TEST_SUITE("Loop Wellenform")
{
    TEST_CASE("Loop Wellenform 30a: ein Clip allein — Bit fuer Bit wie live, der Nachklang klingt aus und endet in Stille")
    {
        WellenApp app;
        const Loop loop = beispiel("effects_demo.lvloop");
        const SaitenVon saiten = saitenAusInstrumenten();
        const ClipOrt echo{2, 4};  // Keys, Piano stabs mit Echo
        REQUIRE(findeClip(loop, echo) >= 0);
        REQUIRE(loop.kanaele[2].clips[static_cast<std::size_t>(findeClip(loop, echo))].effekteWirken());

        const std::optional<Loop> fassung = clipFassung(loop, echo);
        REQUIRE(fassung.has_value());
        CHECK(fassung->laut == 1.0);
        CHECK(fassung->kanaele.front().gainDb == 0.0);
        CHECK(fassung->kanaele.front().clips.size() == 1);
        CHECK(fassung->muster.size() == 2);  // sein Muster und das leere dahinter
        // der Durchlauf reicht ueber den laengsten Nachklang hinter dem Ende des Clips hinaus
        const double msJeTakt = loop.durchlaufMs() / loop.durchlaufTakte();
        const LoopClip& echoClip = loop.kanaele[2].clips[static_cast<std::size_t>(findeClip(loop, echo))];
        CHECK(fassung->durchlaufMs() > (echo.startTakt + loop.clipTakte(echoClip)) * msJeTakt + kNachklangHoechstensMs);

        const std::optional<Wellenform> welle =
            rechneClip(loop, echo, kRate, klaengeFuer(*fassung, saiten), saiten);
        REQUIRE(welle.has_value());
        CHECK(welle->rate == kRate);
        const LoopClip& clip = loop.kanaele[2].clips[static_cast<std::size_t>(findeClip(loop, echo))];
        const double schritt = plane(loop, kRate).schrittFrames;
        CHECK(welle->clipFrames ==
              std::llround((echo.startTakt + loop.clipTakte(clip)) * loop.schritteJeTakt() * schritt) -
                  anfangVon(loop, echo));

        // --- Bit fuer Bit, was live nur dieser Clip spielt (Fader 0 dB, Lautstaerke 1)
        const std::vector<float> vergleich = live(nurDieser(loop, echo, false), saiten, anfangVon(loop, echo), welle->clipFrames);
        REQUIRE(welle->klang.frames() >= welle->clipFrames);
        CHECK(std::equal(vergleich.begin(), vergleich.end(), welle->klang.daten.begin()));
        CHECK(groessterBetrag(vergleich, 0, static_cast<std::size_t>(welle->clipFrames)) > 0.01);

        // --- der Nachklang: das Echo klingt hinter dem Ende weiter, dann ist es still
        CHECK_FALSE(welle->nachklangGekappt);
        CHECK(welle->klang.frames() > welle->clipFrames + kRate / 10);
        CHECK(welle->klang.frames() < welle->clipFrames + std::int64_t{kNachklangHoechstensMs} * kRate / 1000);
        const auto clipEnde = static_cast<std::size_t>(welle->clipFrames);
        CHECK(groessterBetrag(welle->klang.daten, clipEnde, clipEnde + kRate / 20) > 1e-3);
        // dieselbe Fassung live in einem Zug eine Sekunde weiter: bis zum Ende gleich, dahinter
        // kommt nichts mehr ueber die Grenze
        const std::int64_t weiter = welle->klang.frames() + kRate;
        const std::vector<float> durch = live(*fassung, saiten, anfangVon(loop, echo), weiter);
        CHECK(std::equal(welle->klang.daten.begin(), welle->klang.daten.end(), durch.begin()));
        CHECK(groessterBetrag(durch, static_cast<std::size_t>(welle->klang.frames()), static_cast<std::size_t>(weiter)) <
              std::pow(10.0, kNachklangStilleDb / 20.0));

        // --- den Clip gibt es nicht
        CHECK_FALSE(clipFassung(loop, ClipOrt{2, 5}).has_value());
        CHECK_FALSE(rechneClip(loop, ClipOrt{9, 0}, kRate, klaengeFuer(*fassung, saiten), saiten).has_value());
    }

    TEST_CASE("Loop Wellenform 30a, WX5 a: Fader und Lautstaerke des Loops wirken nach dem Drive — live = Welle mal beide")
    {
        WellenApp app;
        const Loop loop = beispiel("effects_demo.lvloop");
        const SaitenVon saiten = saitenAusInstrumenten();
        const ClipOrt acid{1, 16};  // Bass, Acid mit Drive; der Kanal +4,5 dB, der Loop 0,3
        REQUIRE(findeClip(loop, acid) >= 0);
        REQUIRE(loop.kanaele[1].gainDb != 0.0);
        REQUIRE(loop.laut < 1.0);
        const std::optional<Loop> fassung = clipFassung(loop, acid);
        REQUIRE(fassung.has_value());
        const std::optional<Wellenform> welle = rechneClip(loop, acid, kRate, klaengeFuer(*fassung, saiten), saiten);
        REQUIRE(welle.has_value());

        // live mit Fader und Lautstaerke: genau der Bus mal (Fader mal Lautstaerke)
        const std::vector<float> mit = live(nurDieser(loop, acid, true), saiten, anfangVon(loop, acid), welle->clipFrames);
        const auto faktor = static_cast<float>(loop.kanaele[1].gainFaktor() * loop.laut);
        std::size_t anders = 0;
        for (std::size_t i = 0; i < mit.size(); ++i)
            if (mit[i] != welle->klang.daten[i] * faktor) ++anders;
        CHECK(anders == 0);
        // und der Drive hat zu tun: die Welle ist nicht bloss das Signal, kleiner gemacht
        CHECK(groessterBetrag(welle->klang.daten, 0, static_cast<std::size_t>(welle->clipFrames)) > 0.05);
    }

    TEST_CASE("Loop Wellenform 30a: Saiten — die letzten Toene klingen hinter dem Ende aus, statt vom Anfang gedaempft zu werden")
    {
        WellenApp app;
        Loop loop = beispiel("tabs_demo.lvloop");
        const SaitenVon saiten = saitenAusInstrumenten();
        // Let ring an jeder Note: ein Ton klingt, bis die naechste Note auf seiner Saite
        // beginnt — ohne die gaebe es hinter dem Ende nichts (jede Note endet mit sich)
        for (Muster& m : loop.muster)
            for (Note& n : m.noten) n.artikulationen |= kLetRing;
        // der erste Clip, dessen Muster ein Instrument mit Saiten spielt
        std::optional<ClipOrt> mitSaiten;
        for (std::size_t k = 0; k < loop.kanaele.size() && !mitSaiten; ++k)
        {
            const std::vector<ClipToene> toene = toeneImKanal(loop, k, saiten);
            for (std::size_t c = 0; c < toene.size(); ++c)
                if (!toene[c].empty())
                {
                    mitSaiten = ClipOrt{static_cast<int>(k), loop.kanaele[k].clips[c].startTakt};
                    break;
                }
        }
        REQUIRE(mitSaiten.has_value());
        const std::optional<Loop> fassung = clipFassung(loop, *mitSaiten);
        REQUIRE(fassung.has_value());
        const std::optional<Wellenform> welle =
            rechneClip(loop, *mitSaiten, kRate, klaengeFuer(*fassung, saiten), saiten);
        REQUIRE(welle.has_value());
        CHECK(welle->klang.frames() > welle->clipFrames);
        const auto ende = static_cast<std::size_t>(welle->clipFrames);
        // gleich hinter dem Ende klingt noch etwas
        CHECK(groessterBetrag(welle->klang.daten, ende, ende + kRate / 50) > 1e-4);
        // ... und ein Ton haelt laenger als im gewoehnlichen Durchlauf, wo die erste Note der
        // naechsten Runde ihn abdaempft: die Namen der Toene tragen ihr Ende (`:e<ms>`)
        const QStringList gestreckt = fassung->klangNamen(saiten);
        const QStringList imKreis = nurDieser(loop, *mitSaiten, false).klangNamen(saiten);
        int nurGestreckt = 0;
        for (const QString& name : gestreckt)
            if (!imKreis.contains(name)) ++nurGestreckt;
        CHECK(nurGestreckt > 0);
        MESSAGE("tabs_demo, Kanal " << mitSaiten->kanal << " Takt " << mitSaiten->startTakt << ": Clip "
                                    << welle->clipFrames << " Frames, Nachklang "
                                    << (welle->klang.frames() - welle->clipFrames) * 1000 / kRate << " ms");
    }

    TEST_CASE("Loop Wellenform 30a, WX3 a: ein Loop-Clip des Composers — Versatz, Laenge, der Durchlauf wiederholt sich")
    {
        WellenApp app;
        const Loop loop = beispiel("effects_demo.lvloop");
        const SaitenVon saiten = saitenAusInstrumenten();
        const auto klaenge = klaengeFuer(loop, saiten);
        const std::int64_t durchlauf = plane(loop, kRate, saiten).durchlaufFrames;
        REQUIRE(durchlauf > 0);
        const double durchlaufMs = 1000.0 * static_cast<double>(durchlauf) / kRate;

        // anderthalb Durchlaeufe ab dem Anfang: Bit fuer Bit, was der Klangerzeuger dort spielt
        const std::optional<Wellenform> vorn = rechneAusschnitt(loop, 0.0, durchlaufMs * 1.5, kRate, klaenge, saiten);
        REQUIRE(vorn.has_value());
        CHECK(vorn->klang.frames() == std::llround(durchlaufMs * 1.5 * kRate / 1000.0));
        CHECK(vorn->clipFrames == vorn->klang.frames());
        const std::vector<float> vergleich = live(loop, saiten, 0, vorn->klang.frames());
        CHECK(vorn->klang.daten == vergleich);

        // ein Versatz von einem Durchlauf und einer Sekunde ist derselbe wie von einer Sekunde
        const std::optional<Wellenform> eine = rechneAusschnitt(loop, 1000.0, 2000.0, kRate, klaenge, saiten);
        const std::optional<Wellenform> runde =
            rechneAusschnitt(loop, durchlaufMs + 1000.0, 2000.0, kRate, klaenge, saiten);
        REQUIRE(eine.has_value());
        REQUIRE(runde.has_value());
        CHECK(eine->klang.daten == runde->klang.daten);
        CHECK(eine->klang.daten == live(loop, saiten, kRate, eine->klang.frames()));

        CHECK_FALSE(rechneAusschnitt(loop, 0.0, 0.0, kRate, klaenge, saiten).has_value());
    }

    TEST_CASE("Loop Wellenform 30a: Fortschritt bis 1, Abbruch gibt nichts")
    {
        WellenApp app;
        const Loop loop = beispiel("effects_demo.lvloop");
        const SaitenVon saiten = saitenAusInstrumenten();
        const ClipOrt echo{2, 4};
        const auto klaenge = klaengeFuer(*clipFassung(loop, echo), saiten);

        std::vector<double> stand;
        REQUIRE(rechneClip(loop, echo, kRate, klaenge, saiten, [&](double s) {
                    stand.push_back(s);
                    return true;
                }).has_value());
        REQUIRE(stand.size() > 2);
        CHECK(std::is_sorted(stand.begin(), stand.end()));
        CHECK(stand.back() == 1.0);

        int gefragt = 0;
        CHECK_FALSE(rechneClip(loop, echo, kRate, klaenge, saiten, [&](double) { return ++gefragt < 3; }).has_value());
        CHECK(gefragt == 3);
        CHECK_FALSE(rechneAusschnitt(loop, 0.0, 5000.0, kRate, klaengeFuer(loop, saiten), saiten,
                                     [](double) { return false; })
                        .has_value());
    }

    TEST_CASE("Loop Wellenform 30a Messung: Rechenzeit eines Clips und eines Loop-Clips (meldet nur Zahlen)")
    {
        WellenApp app;
        const Loop loop = beispiel("tabs_demo.lvloop");
        const SaitenVon saiten = saitenAusInstrumenten();
        const auto klaenge = klaengeFuer(loop, saiten);
        const double durchlaufMs = loop.durchlaufMs();
        QElapsedTimer uhr;
        uhr.start();
        const std::optional<Wellenform> ganz = rechneAusschnitt(loop, 0.0, durchlaufMs, kRate, klaenge, saiten);
        const qint64 msGanz = uhr.elapsed();
        REQUIRE(ganz.has_value());
        MESSAGE("tabs_demo, ein Durchlauf " << std::llround(durchlaufMs) << " ms Ton in " << msGanz
                                            << " ms gerechnet (ohne die Toene selbst)");
    }
}
