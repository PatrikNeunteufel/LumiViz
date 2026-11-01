/**
 ****************************************************************************************
 * @file   test_LoopBounce.cpp
 * @brief  Loop, Stufe 15k-2: *Bounce to wave clip* (docs/audio/Loop_Konzept.md §4p;
 *         Entscheid Patrik WV7 a)
 *
 * @author Patrik Neunteufel
 * @date   November 2026
 *
 * @details
 * - Ohne Fenster: die Laenge (Clip plus Nachklang, hoechstens bis zum naechsten Clip), das
 *   Ersetzen an seinem Ort (eine Wiederholung, keine Effekte, keine Expression), die Herkunft,
 *   *source changed*, *Revert to source* (auch wenn das Muster fehlt), *Bounce again*.
 * - Ueber das Loop-Panel wie im Programm: das Menue am Clip, im Hintergrund gerechnet, die Datei
 *   als eigenes Sample in einem Wegwerf-Ordner, ein Undo-Schritt, die Marke *source changed*,
 *   *Bounce again* mit neuer Datei, *Revert to source*. Ohne Audio-Engine (die Toene des Klaviers
 *   werden gerechnet), Einstellungen unter `LumiVizTest` / `LoopBounce`.
 ****************************************************************************************
 */

#include <doctest.h>

#include "UI/widgets/Bedienhilfe.hpp"
#include "audio/WavDatei.hpp"
#include "loop/Austausch.hpp"  // clipDatei
#include "loop/Bounce.hpp"
#include "loop/Loop.hpp"
#include "loop/LoopPanel.hpp"
#include "loop/Sample.hpp"
#include "loop/Taktachse.hpp"
#include "loop/Tonspeicher.hpp"
#include "services/CommandBus.hpp"
#include "services/ICommandBus.hpp"
#include "services/ServiceContainer.hpp"

#include <QApplication>
#include <QContextMenuEvent>
#include <QFileInfo>
#include <QMenu>
#include <QSettings>
#include <QTemporaryDir>

using namespace lumi::loop;

namespace
{

/// Keys (Klavier, ein Takt) zweimal ab Takt 0 mit Echo und Expression; dahinter im selben Kanal
/// ab Takt 4 ein zweiter Clip. 120 BPM, 4/4: ein Takt sind 2 s.
Loop probeLoop()
{
    Muster keys;
    keys.name = QStringLiteral("Keys");
    keys.art = MusterArt::Instrument;
    keys.instrument = QStringLiteral("piano");
    keys.takte = 1;
    keys.noten = {Note{0, 4, 60}, Note{8, 4, 64}};
    Muster bass = keys;
    bass.name = QStringLiteral("Bass");
    bass.noten = {Note{0, 8, 36}};
    LoopClip clip(QStringLiteral("Keys"), 0, 2);
    clip.effekte.reihe = {ClipEcho{}};
    clip.expression = {{0.0, 0.5}, {2.0, 1.0}};
    Loop loop;
    loop.bpm = 120.0;
    loop.muster = {keys, bass};
    loop.kanaele = {{QStringLiteral("Keys"), true, {clip, {QStringLiteral("Bass"), 4, 1}}}};
    loop.richte();
    return loop;
}

/// Eine Wellenform von `sekunden` (stumm) — fuer die Laenge reicht die Zahl der Abtastwerte.
Wellenform welleVon(double sekunden)
{
    Wellenform w;
    w.rate = 44100;
    w.klang.daten.assign(static_cast<std::size_t>(2 * std::llround(sekunden * w.rate)), 0.0f);
    return w;
}

struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("LoopBounce"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

/// Faengt das naechste Menue: seine Eintraege, und waehlt `waehle`, wenn es ihn gibt.
struct MenueFang
{
    QStringList texte;
    QString waehle;
    MenueFang()
    {
        lumi::ui::setzeMenueBeobachterFuerTests([this](QMenu& menue) -> QAction* {
            texte.clear();
            QAction* gewaehlt = nullptr;
            for (QAction* a : menue.actions())
            {
                if (a->isSeparator()) continue;
                texte << a->text();
                if (a->text() == waehle) gewaehlt = a;
            }
            return gewaehlt;
        });
    }
    ~MenueFang() { lumi::ui::setzeMenueBeobachterFuerTests({}); }
};

void rechts(QWidget& ziel, QPoint ort)
{
    QContextMenuEvent ereignis(QContextMenuEvent::Mouse, ort, ziel.mapToGlobal(ort));
    QCoreApplication::sendEvent(&ziel, &ereignis);
}

const Muster& musterVon(const Loop& loop, const QString& name)
{
    const Muster* m = loop.findeMuster(name);
    REQUIRE(m != nullptr);
    return *m;
}

}  // namespace

TEST_SUITE("LoopBounce")
{
    TEST_CASE("Loop Bounce 15k-2: Laenge, Ersetzen, Herkunft, source changed, Revert, Bounce again")
    {
        const Loop vorher = probeLoop();
        const ClipOrt ort{0, 0};
        CHECK(bounceMoeglich(vorher, ort));

        // die Laenge: zwei Takte Clip, Nachklang bis 5,2 s = 3 Takte; der naechste Clip ab Takt 4 laesst sie
        bool abgeschnitten = true;
        CHECK(bounceTakte(vorher, ort, welleVon(5.2), &abgeschnitten) == 3);
        CHECK_FALSE(abgeschnitten);
        CHECK(bounceTakte(vorher, ort, welleVon(3.0)) == 2);  // nie kuerzer als der Clip
        CHECK(bounceTakte(vorher, ort, welleVon(9.5), &abgeschnitten) == 4);  // bis zum naechsten Clip
        CHECK(abgeschnitten);

        Loop loop = vorher;
        const QString name = setzeBounceEin(loop, ort, QStringLiteral("user:keys_bounce"), welleVon(9.5));
        CHECK(name == QStringLiteral("Keys bounce"));
        const Muster& wave = musterVon(loop, name);
        CHECK(wave.art == MusterArt::Wave);
        CHECK(wave.takte == 4);
        CHECK(wave.wave.datei == QStringLiteral("user:keys_bounce"));
        CHECK(wave.wave.bezugBpm == 120.0);
        CHECK_FALSE(wave.wave.folgtTempo);
        CHECK(wave.wave.fadeOutMs == kBounceBlendeMs);  // am naechsten Clip abgeschnitten
        // der Clip an seinem Ort: eine Wiederholung, keine Effekte, keine Expression; das Muster bleibt
        const LoopClip& clip = loop.kanaele[0].clips[0];
        CHECK(clip.muster == name);
        CHECK(clip.startTakt == 0);
        CHECK(clip.wiederholungen == 1);
        CHECK(clip.effekte.reihe.empty());
        CHECK(clip.expression.empty());
        CHECK(loop.findeMuster(QStringLiteral("Keys")) != nullptr);
        // die Herkunft ist der Ursprungsclip wie in einer .lvclip
        REQUIRE(hatQuelle(wave));
        CHECK(wave.wave.herkunft.clip() == *clipDatei(vorher, 0, 0));
        CHECK_FALSE(quelleGeaendert(loop, wave));
        CHECK(Taktachse::waveMarke(wave, loop).isEmpty());
        CHECK_FALSE(bounceMoeglich(loop, ort));  // ein Wave-Clip bounct nicht
        // ein zweiter Bounce desselben Musters bekommt einen freien Namen
        Loop zwei = vorher;
        zwei.muster.push_back(wave);
        CHECK(setzeBounceEin(zwei, ort, QStringLiteral("user:x"), welleVon(1.0)) == QStringLiteral("Keys bounce 2"));

        SUBCASE("source changed: das Muster geaendert oder geloescht")
        {
            Loop geaendert = loop;
            musterIn(geaendert, QStringLiteral("Keys"))->noten[1].hoehe = 67;
            CHECK(quelleGeaendert(geaendert, musterVon(geaendert, name)));
            CHECK(Taktachse::waveMarke(musterVon(geaendert, name), geaendert) == QStringLiteral("source changed"));
            Loop ohne = loop;
            ohne.muster.erase(std::remove_if(ohne.muster.begin(), ohne.muster.end(),
                                             [](const Muster& m) { return m.name == QStringLiteral("Keys"); }),
                              ohne.muster.end());
            CHECK(quelleGeaendert(ohne, musterVon(ohne, name)));
        }
        SUBCASE("Revert to source: der Ursprungsclip zurueck, mit dem jetzigen Muster; fehlt es, aus der Herkunft")
        {
            Loop zurueck = loop;
            musterIn(zurueck, QStringLiteral("Keys"))->noten[1].hoehe = 67;
            REQUIRE(setzeQuelleZurueck(zurueck, ort));
            const LoopClip& c = zurueck.kanaele[0].clips[0];
            CHECK(c.muster == QStringLiteral("Keys"));
            CHECK(c.wiederholungen == 2);
            CHECK(c.effekte.reihe.size() == 1);
            CHECK(c.expression.size() == 2);
            CHECK(musterVon(zurueck, QStringLiteral("Keys")).noten[1].hoehe == 67);  // das jetzige
            CHECK(zurueck.findeMuster(name) != nullptr);  // das Wave-Muster bleibt
            Loop ohne = loop;
            ohne.muster.erase(std::remove_if(ohne.muster.begin(), ohne.muster.end(),
                                             [](const Muster& m) { return m.name == QStringLiteral("Keys"); }),
                              ohne.muster.end());
            REQUIRE(setzeQuelleZurueck(ohne, ort));
            CHECK(musterVon(ohne, QStringLiteral("Keys")) == musterVon(vorher, QStringLiteral("Keys")));
            // laenger geworden, passt nicht mehr vor den naechsten Clip: nichts
            Loop eng = loop;
            musterIn(eng, QStringLiteral("Keys"))->takte = 3;
            eng.richte();
            CHECK_FALSE(setzeQuelleZurueck(eng, ort));
            CHECK_FALSE(setzeQuelleZurueck(eng, ClipOrt{0, 4}));  // ein Clip ohne Herkunft
        }
        SUBCASE("Bounce again: neue Datei, neue Herkunft, neue Laenge")
        {
            Loop neu = loop;
            musterIn(neu, QStringLiteral("Keys"))->noten[1].hoehe = 67;
            Loop quelle = neu;
            REQUIRE(setzeQuelleZurueck(quelle, ort));
            REQUIRE(erneuereBounce(neu, name, quelle, ort, QStringLiteral("user:keys_bounce_2"), welleVon(5.0)));
            const Muster& w = musterVon(neu, name);
            CHECK(w.wave.datei == QStringLiteral("user:keys_bounce_2"));
            CHECK(w.takte == 3);
            CHECK(w.wave.fadeOutMs == 0.0);
            CHECK_FALSE(quelleGeaendert(neu, w));
        }
    }

    TEST_CASE("Loop Bounce 15k-2: ueber das Loop-Panel — Menue, Rechnung, Sample, Undo, Bounce again, Revert")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;
        QTemporaryDir ordner;
        REQUIRE(ordner.isValid());
        setzeEigenenSampleOrdnerFuerTests(ordner.filePath(QStringLiteral("samples")));
        tonspeicher().leere();
        const QString pfad = ordner.filePath(QStringLiteral("probe.lvloop"));
        // mit Hall (1,8 s) und dem Bass erst ab Takt 8: der Nachklang hat Platz
        Loop mitHall = probeLoop();
        mitHall.kanaele[0].clips[0].effekte.reihe = {ClipHall{true, 0.5, 1.8, 0.4, 0.25}};
        mitHall.kanaele[0].clips[1].startTakt = 8;
        mitHall.richte();
        REQUIRE(speichereLoop(mitHall, pfad));

        ServiceContainer dienste;
        dienste.registerSingleton<ICommandBus>([](ServiceContainer&) { return std::make_unique<CommandBus>(); });
        ICommandBus* bus = dienste.tryResolve<ICommandBus>();
        REQUIRE(bus != nullptr);
        {
            LoopPanel panel(dienste);  // nicht gezeigt: kein Fenster
            panel.resize(1000, 320);
            REQUIRE(panel.lade(pfad));
            Taktachse& achse = *panel.taktachse();
            achse.resize(1000, 240);
            MenueFang fang;

            // im Menue des Clips, gleich hinter Freeze clip
            achse.waehle(0, 0);
            fang.waehle = QStringLiteral("Bounce to wave clip");
            rechts(achse, achse.clipRechteck(0, 0).center());
            CHECK(fang.texte.indexOf(QStringLiteral("Bounce to wave clip")) == fang.texte.indexOf(QStringLiteral("Freeze clip")) + 1);
            CHECK(panel.bouncet());
            panel.warteAufBounce();
            CHECK_FALSE(panel.bouncet());
            CHECK(panel.bounceBericht().isEmpty());
            CHECK(bus->undoDescription() == "Bounce to wave clip");
            const LoopClip& clip = panel.loop().kanaele[0].clips[0];
            const Muster& wave = musterVon(panel.loop(), clip.muster);
            REQUIRE(wave.art == MusterArt::Wave);
            CHECK(clip.muster == QStringLiteral("Keys bounce"));
            CHECK(wave.wave.datei.startsWith(QStringLiteral("user:")));
            const QString datei = sampleDatei(wave.wave.datei);
            CHECK(QFileInfo(datei).isFile());
            CHECK(QFileInfo(datei).absolutePath() == QFileInfo(ordner.filePath(QStringLiteral("samples"))).absoluteFilePath());
            CHECK(wave.takte >= 3);  // der Hall klingt ueber das Ende des Clips (4 s) hinaus
            CHECK(wave.takte <= 8);
            const QString ersteDatei = wave.wave.datei;
            // meldet nur Zahlen: wie lang die Datei ist und wann der Nachklang unter −60 dB faellt (WV7 a nennt
            // −60 dB, Freeze und Export rechnen bis −96 dB, `kNachklangStilleDb`)
            if (const auto inhalt = lumi::audio::liesWav(datei))
            {
                const auto n = static_cast<std::int64_t>(inhalt->stereo.size() / 2);
                std::int64_t letzte = 0;
                for (std::int64_t i = 0; i < n; ++i)
                    if (std::abs(inhalt->stereo[static_cast<std::size_t>(2 * i)]) > 0.001f) letzte = i;
                MESSAGE("Bounce: Datei " << static_cast<double>(n) / inhalt->rate << " s, ueber -60 dBFS bis "
                                         << static_cast<double>(letzte) / inhalt->rate << " s, Takte " << wave.takte
                                         << " (ein Takt 2 s, der Clip 4 s)");
            }

            // am Wave-Clip: Bounce again und Revert to source
            fang.waehle.clear();
            rechts(achse, achse.clipRechteck(0, 0).center());
            CHECK(fang.texte.contains(QStringLiteral("Bounce again")));
            CHECK(fang.texte.contains(QStringLiteral("Revert to source")));
            CHECK_FALSE(fang.texte.contains(QStringLiteral("Bounce to wave clip")));

            // das Muster geaendert: source changed; Bounce again rechnet neu, mit neuer Datei
            Loop geaendert = panel.loop();
            musterIn(geaendert, QStringLiteral("Keys"))->noten[1].hoehe = 67;
            panel.bearbeite(geaendert, QStringLiteral("Edit notes"));
            CHECK(Taktachse::waveMarke(musterVon(panel.loop(), QStringLiteral("Keys bounce")), panel.loop()) ==
                  QStringLiteral("source changed"));
            fang.waehle = QStringLiteral("Bounce again");
            rechts(achse, achse.clipRechteck(0, 0).center());
            panel.warteAufBounce();
            CHECK(bus->undoDescription() == "Bounce again");
            const Muster& neu = musterVon(panel.loop(), QStringLiteral("Keys bounce"));
            CHECK(neu.wave.datei != ersteDatei);
            CHECK(QFileInfo(sampleDatei(neu.wave.datei)).isFile());
            CHECK_FALSE(quelleGeaendert(panel.loop(), neu));
            // Undo: wieder die erste Datei, die noch da ist
            REQUIRE(bus->undo());
            CHECK(musterVon(panel.loop(), QStringLiteral("Keys bounce")).wave.datei == ersteDatei);
            CHECK(QFileInfo(sampleDatei(ersteDatei)).isFile());
            REQUIRE(bus->redo());

            // Revert to source: der Klavier-Clip zurueck, mit seinen Wiederholungen und Effekten
            fang.waehle = QStringLiteral("Revert to source");
            rechts(achse, achse.clipRechteck(0, 0).center());
            CHECK(bus->undoDescription() == "Revert to source");
            CHECK(panel.loop().kanaele[0].clips[0].muster == QStringLiteral("Keys"));
            CHECK(panel.loop().kanaele[0].clips[0].wiederholungen == 2);
            CHECK(panel.loop().kanaele[0].clips[0].effekte.reihe.size() == 1);
        }
        tonspeicher().leere();
        setzeEigenenSampleOrdnerFuerTests({});
    }
}
