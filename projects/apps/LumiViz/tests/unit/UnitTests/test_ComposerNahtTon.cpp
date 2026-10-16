/**
 ****************************************************************************************
 * @file   test_ComposerNahtTon.cpp
 * @brief  Composer, Stufe C5 / C5b: die Naehte zum Player — N7b Zugang, N5 Transport und
 *         Anzeige, N13, N15 — und die Sitzung, die den Ton fuehrt
 *         (docs/ui/Composer_Konzept.md §12.2)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * - **Der echte `AudioPlayer`** mit einer Audio-Engine aus Pappe: ohne
 *   Playlist (`setPlaylist(nullptr)`) spielt am Titelende nichts weiter — der
 *   Beleg dafuer, dass die Playlist ohne Naht ruhen kann. (Der Fall zur Naht
 *   N7a, Pegel im Player, ist mit der Naht in S98 entfallen.)
 * - **Die Sitzung** mit Player und Audio-Engine aus Pappe in den Diensten des
 *   Fensters (Naht N7b): sie uebernimmt den Ton (C5b: die Songs laufen im Mixer
 *   der Engine, die Uhr fuehrt, der Player schweigt), meldet den laufenden Song,
 *   und die Befehle des Player-Panels (Naht N5) steuern den ganzen Composer.
 *
 * Der letzte Fall oeffnet ein echtes Fenster (wie die anderen Composer-Tests
 * mit Fassade); ohne sichtbares Fenster ueberspringt er sich.
 *
 * Rueckbau (Konzept §12): diese Datei loeschen, Zeile in Source.cmake nehmen.
 ****************************************************************************************
 */

#include <doctest.h>

#include "PappTon.hpp"
#include "TestFenster.hpp"
#include "UI/panels/PlayerPanel.hpp"
#include "UI/panels/TagViewPanel.hpp"
#include "UI/widgets/VisualizerWidget.hpp"
#include "audio/AudioEvents.hpp"
#include "audio/AudioPlayer.hpp"
#include "audio/Playlist.hpp"
#include "composer/AppAnbindung.hpp"
#include "composer/Bearbeiten.hpp"
#include "composer/Komposition.hpp"
#include "composer/Sitzung.hpp"
#include "services/EventBus.hpp"
#include "services/IEventBus.hpp"
#include "services/ServiceContainer.hpp"
#include "visualizers/multieffect/ChainSerializer.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QTemporaryDir>
#include <QThread>

#include <cstdlib>
#include <functional>
#include <memory>

using namespace lumi::multieffect;
using namespace lumi::composer;
using papp::PappEngine;
using papp::PappMischEngine;
using papp::PappPlayer;

namespace
{

const QString kEins = QStringLiteral("C:/musik/eins.mp3");
const QString kZwei = QStringLiteral("C:/musik/zwei.mp3");
const QString kListe = QStringLiteral("C:/musik/aus_der_playlist.mp3");

/// Eigener QSettings-Bereich fuer die Dauer des Tests, danach leer.
struct EigeneEinstellungen
{
    QString org = QCoreApplication::organizationName();
    QString name = QCoreApplication::applicationName();
    EigeneEinstellungen()
    {
        QCoreApplication::setOrganizationName(QStringLiteral("LumiVizTest"));
        QCoreApplication::setApplicationName(QStringLiteral("ComposerNahtTon"));
        QSettings().clear();
    }
    ~EigeneEinstellungen()
    {
        QSettings().clear();
        QCoreApplication::setOrganizationName(org);
        QCoreApplication::setApplicationName(name);
    }
};

bool warte(const std::function<bool()>& bedingung, int hoechstensMs = 8000)
{
    QElapsedTimer uhr;
    uhr.start();
    while (!bedingung())
    {
        if (uhr.elapsed() > hoechstensMs) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(2);
    }
    return true;
}

}  // namespace

TEST_SUITE("ComposerNahtTon")
{
    TEST_CASE("Die Playlist ruht ohne Naht: ohne Playlist spielt der echte Player am Titelende nichts weiter")
    {
        // die Playlist nimmt nur Dateien, die es gibt — leere genuegen, gespielt wird aus Pappe
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString kEins = dir.filePath(QStringLiteral("eins.mp3"));
        const QString kZwei = dir.filePath(QStringLiteral("zwei.mp3"));
        for (const QString& pfad : {kEins, kZwei})
        {
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
        }
        EventBus bus;
        PappEngine engine;
        engine.laengen = {{kEins, 60000}, {kZwei, 60000}};
        AudioPlayer player(engine, bus);
        Playlist liste(bus);
        liste.addTrack(kEins);
        liste.addTrack(kZwei);
        REQUIRE(liste.count() == 2);
        player.setPlaylist(&liste);

        // wie immer: am Ende des ersten Songs spielt der zweite
        REQUIRE(player.playIndex(0));
        REQUIRE(player.isPlaying());
        engine.beende(engine.letzter());
        player.update();
        CHECK(player.currentTrack().filePath == kZwei);
        CHECK(player.isPlaying());
        CHECK(engine.geoeffnet.size() == 2);

        // ausgehaengt: am Ende haelt der Player, nichts wird geladen
        player.setPlaylist(nullptr);
        CHECK(player.playlist() == nullptr);
        engine.beende(engine.letzter());
        player.update();
        CHECK_FALSE(player.isPlaying());
        CHECK(player.isStopped());
        CHECK(player.currentTrack().filePath == kZwei);
        CHECK(engine.geoeffnet.size() == 2);
        // und weder Next, Previous noch ein Doppelklick in der Liste tun etwas
        CHECK_FALSE(player.next());
        CHECK_FALSE(player.previous());
        CHECK_FALSE(player.playIndex(0));
        CHECK(engine.geoeffnet.size() == 2);
        // der Composer kann den Player dabei selbst laden und spielen
        REQUIRE(player.load(kEins));
        player.seek(5000);
        player.play();
        CHECK(player.isPlaying());
        CHECK(player.positionMs() == 5000);

        // wieder eingehaengt: wie zuvor
        player.setPlaylist(&liste);
        CHECK(player.playIndex(1));
        CHECK(player.currentTrack().filePath == kZwei);
    }

    TEST_CASE("Naht N17: der Pegel des echten Players — ein zweiter Faktor neben der Lautstaerke (Mute fuer Durchsagen)")
    {
        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        const QString kEins = dir.filePath(QStringLiteral("eins.mp3"));
        const QString kZwei = dir.filePath(QStringLiteral("zwei.mp3"));
        for (const QString& pfad : {kEins, kZwei})
        {
            QFile f(pfad);
            REQUIRE(f.open(QIODevice::WriteOnly));
        }
        EventBus bus;
        PappEngine engine;
        engine.laengen = {{kEins, 60000}, {kZwei, 60000}};
        AudioPlayer player(engine, bus);
        int lautstaerkeGemeldet = 0;
        bus.subscribe<VolumeChangedEvent>([&](const VolumeChangedEvent&) { ++lautstaerkeGemeldet; });

        // Vorgabe 1: die Lautstaerke geht an die Engine wie immer
        CHECK(player.pegel() == doctest::Approx(1.0f));
        player.setVolume(0.8f);
        REQUIRE(player.load(kEins));
        CHECK(engine.getVolume(engine.letzter()) == doctest::Approx(0.8f));
        const int gemeldet = lautstaerkeGemeldet;

        // der Pegel: zu hoeren ist Lautstaerke mal Pegel — der Regler bleibt, kein Ereignis
        player.setPegel(0.25f);
        CHECK(engine.getVolume(engine.letzter()) == doctest::Approx(0.2f));
        CHECK(player.volume() == doctest::Approx(0.8f));
        CHECK(lautstaerkeGemeldet == gemeldet);
        // die Lautstaerke aendern, waehrend abgesenkt ist: beides zusammen
        player.setVolume(0.4f);
        CHECK(engine.getVolume(engine.letzter()) == doctest::Approx(0.1f));
        // das harte Mute gewinnt — und gibt beim Aufheben den abgesenkten Stand zurueck
        player.setMuted(true);
        CHECK(engine.getVolume(engine.letzter()) == doctest::Approx(0.0f));
        player.setPegel(0.5f);  // im harten Mute: nichts zu hoeren, gemerkt wird er
        CHECK(engine.getVolume(engine.letzter()) == doctest::Approx(0.0f));
        player.setMuted(false);
        CHECK(engine.getVolume(engine.letzter()) == doctest::Approx(0.2f));
        // ein neu geladener Titel beginnt mit dem Pegel, der gilt
        REQUIRE(player.load(kZwei));
        CHECK(engine.getVolume(engine.letzter()) == doctest::Approx(0.2f));
        // geklemmt
        player.setPegel(7.0f);
        CHECK(player.pegel() == doctest::Approx(1.0f));
        CHECK(engine.getVolume(engine.letzter()) == doctest::Approx(0.4f));
    }

    TEST_CASE("C5b Sitzung: die Audio-Kanaele im Mixer — Zugang (N7b), die Uhr fuehrt, laufender Song, Transport (N5)")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        // ein Bild-Clip ueber alles, darunter zwei Songs: eins 0–60 s, zwei 70–90 s
        ChainNode root;
        root.params = ListParams{};
        ChainNode clear;
        ClearParams cp;
        cp.color = 0x00FF00;
        clear.params = cp;
        root.children.push_back(std::move(clear));
        const QString preset = dir.filePath(QStringLiteral("Gruen.lvfx"));
        REQUIRE(saveChainToFile(root, preset));
        Komposition k = neueKomposition();
        REQUIRE(fuegeClipEin(k, 0, preset, 0, 90000) == 0);
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 1);
        REQUIRE(fuegeSongEin(k, 1, kEins, 0, 60000) == 0);
        k.kanaele[1].clips[0].setzeBlenden(2000, 0);
        REQUIRE(fuegeSongEin(k, 1, kZwei, 70000, 20000) == 1);
        const QString datei = dir.filePath(QStringLiteral("mitTon.lvcomp"));
        REQUIRE(speichereKomposition(k, datei));

        ServiceContainer dienste;
        dienste.registerSingleton<IEventBus>([](ServiceContainer&) { return std::make_unique<EventBus>(); });
        dienste.registerSingleton<IAudioPlayer>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new PappPlayer); });
        dienste.registerSingleton<IAudioEngine>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new PappMischEngine); });
        IEventBus* bus = dienste.tryResolve<IEventBus>();
        auto* player = dynamic_cast<PappPlayer*>(dienste.tryResolve<IAudioPlayer>());
        auto* engine = dynamic_cast<PappMischEngine*>(dienste.tryResolve<IAudioEngine>());
        REQUIRE(bus != nullptr);
        REQUIRE(player != nullptr);
        REQUIRE(engine != nullptr);
        engine->laengen = {{kEins, 300000}, {kZwei, 300000}};
        player->laengen = {{kListe, 300000}};
        int platz = 0;  // nie benutzt, nur weitergereicht: irgendeine Adresse genuegt als Playlist
        auto* liste = reinterpret_cast<IPlaylist*>(&platz);
        player->setPlaylist(liste);
        // der laufende Song, wie ihn Karaoke, Tag View und Audio Browser erfahren
        QString gemeldet;
        const auto abo =
            bus->subscribeScoped<TrackChangedEvent>([&](const TrackChangedEvent& e) { gemeldet = e.track.filePath; });
        // ... und Stelle und „laeuft", wie sie das Player-Panel erfaehrt
        int stelleGemeldet = -1;
        int dauerGemeldet = 0;
        PlaybackState zustand = PlaybackState::Stopped;
        const auto aboStelle = bus->subscribeScoped<PlaybackPositionEvent>([&](const PlaybackPositionEvent& e) {
            stelleGemeldet = e.positionMs;
            dauerGemeldet = e.durationMs;
        });
        const auto aboZustand =
            bus->subscribeScoped<PlaybackStateEvent>([&](const PlaybackStateEvent& e) { zustand = e.state; });
        const auto quelle = [&](const QString& datei) -> const PappEngine::Stream* {
            const AudioStreamHandle h = engine->quelle(datei);
            return h != INVALID_STREAM ? &engine->stream(h) : nullptr;
        };

        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        // N7b: das Fenster gibt die Dienste heraus, die es bekam
        CHECK(&widget.dienste() == &dienste);

        Sitzung& sitzung = Sitzung::fuer(widget);
        REQUIRE(sitzung.lade(datei));
        // geladen ist nicht an: der Player gehoert der Playlist, Befehle gehen an ihn
        CHECK_FALSE(sitzung.fuehrtTon());
        CHECK(Sitzung::fuehrende() == nullptr);
        CHECK(player->playlist() == liste);
        CHECK_FALSE(transport(TransportCommandEvent::Action::PlayPause));
        CHECK(lumi::composer::tonMixer() == INVALID_STREAM);  // N13: der Stream des Players wie immer
        // die Playlist spielt gerade einen Titel
        REQUIRE(player->load(kListe));
        player->play();

        // Composer on: die Sitzung uebernimmt den Ton — ein Mixer entsteht, der
        // Player schweigt, die Playlist ruht
        sitzung.setAn(true);
        REQUIRE(sitzung.an());
        REQUIRE(warte([&] { return sitzung.fuehrtTon(); }));
        CHECK(Sitzung::fuehrende() == &sitzung);
        CHECK(player->playlist() == nullptr);
        CHECK(player->isPaused());
        CHECK(sitzung.tonMixer() == engine->mixer);
        CHECK(sitzung.tonMixer() != INVALID_STREAM);
        // Naht N13: daher nehmen Spektrum und Wellenform jetzt ihren Stream
        CHECK(lumi::composer::tonMixer() == engine->mixer);

        // Play: der erste Song laeuft als Quelle im Mixer und ist als laufender Song gemeldet
        // (die Quelle steht schon bei stehender Uhr bereit — zu hoeren ist sie erst, wenn der Mixer spielt)
        sitzung.spiele();
        REQUIRE(warte([&] { return quelle(kEins) != nullptr && !quelle(kEins)->haelt; }));
        REQUIRE(warte([&] { return engine->stream(engine->mixer).spielt; }));
        REQUIRE(warte([&] { return gemeldet == kEins; }));
        // die Uhr fuehrt: die Engine aus Pappe rueckt nicht vor — die Uhr laeuft trotzdem,
        // und die Quelle wird ihr nachgesetzt
        QThread::msleep(700);
        QCoreApplication::processEvents();
        CHECK(sitzung.laeuft());
        CHECK(sitzung.stelleMs() > 500);
        REQUIRE(warte([&] { return quelle(kEins)->pos > 400; }));
        // der Player bleibt, wie er ist: er spielt nicht und laedt nichts
        CHECK(player->isPaused());
        CHECK(player->currentTrack().filePath == kListe);
        // der Pegel des Clips ist die Lautstaerke der Quelle: nach der Einblendung voll ...
        sitzung.springe(30000);
        REQUIRE(warte([&] { return std::abs(quelle(kEins)->lautstaerke - 1.0f) < 0.01f; }));
        CHECK(std::abs(quelle(kEins)->pos - 30000) < 700);
        // ... und ein Sprung im Composer setzt die Quelle: mitten in die Einblendung
        // (die Uhr laeuft — die Quelle landet ein paar Millisekunden hinter der Sekunde)
        sitzung.springe(1000);
        REQUIRE(warte([&] { return std::abs(quelle(kEins)->pos - 1000) < 150; }));
        REQUIRE(warte([&] { return std::abs(quelle(kEins)->lautstaerke - 0.5f) < 0.05f; }));
        // Stelle im Song und „laeuft" gehen ueber den Bus, wie beim Player
        REQUIRE(warte([&] { return zustand == PlaybackState::Playing; }));
        REQUIRE(warte([&] { return stelleGemeldet >= 1000 && stelleGemeldet < 3000; }));
        CHECK(dauerGemeldet == 300000);
        // spielt der Player doch (jemand hat ihn gestartet), haelt ihn der naechste Takt an —
        // und der laufende Song der Komposition wird noch einmal gemeldet
        gemeldet.clear();
        player->play();
        REQUIRE(warte([&] { return player->isPaused() && gemeldet == kEins; }));

        // --- Naht N5, Anzeige: das Player-Panel zeigt den laufenden Song als „Composer - <Song>"
        PlayerPanel panel(dienste);
        const auto titel = [&]() -> QString {
            for (const QLabel* l : panel.findChildren<QLabel*>())
                if (l->text().startsWith(QStringLiteral("Composer - "))) return l->text();
            return {};
        };
        const auto loopKnopf = [&]() -> QPushButton* {
            for (QPushButton* b : panel.findChildren<QPushButton*>())
                if (b->toolTip().startsWith(QStringLiteral("Repeat current track")) ||
                    b->toolTip().contains(QStringLiteral("Composer panel")))
                    return b;
            return nullptr;
        };
        QSlider* balken = nullptr;
        for (QSlider* s : panel.findChildren<QSlider*>())
            if (s->maximum() == 1000) balken = s;
        REQUIRE(balken != nullptr);
        // das Panel wird erst jetzt sichtbar: es nimmt den Stand vom Composer, nicht vom Player
        panel.show();
        QCoreApplication::processEvents();
        CHECK(titel() == QStringLiteral("Composer - eins"));
        REQUIRE(loopKnopf() != nullptr);
        CHECK_FALSE(loopKnopf()->isEnabled());  // die Schleife steht im Composer-Panel
        REQUIRE(warte([&] { return balken->value() > 0; }));  // der Balken laeuft mit dem Song
        // --- Naht N15 (Befund Patrik S98): auch Tag View zeigt beim Einblenden den laufenden
        //     Song der Komposition — nicht den Titel des angehaltenen Players
        {
            TagViewPanel tagView(dienste);
            tagView.show();
            QCoreApplication::processEvents();
            QStringList texte;
            for (const QLabel* l : tagView.findChildren<QLabel*>()) texte << l->text();
            CHECK(texte.contains(QStringLiteral("eins")));                                       // Titel
            CHECK(texte.contains(QDir::toNativeSeparators(kEins)));                              // Datei
            CHECK_FALSE(texte.contains(QDir::toNativeSeparators(kListe)));
        }
        // ein Klick in den Balken bei einem Zehntel des Songs (30 s): die Komposition springt dorthin
        balken->setValue(100);
        Q_EMIT balken->sliderReleased();
        REQUIRE(warte([&] { return std::llabs(sitzung.stelleMs() - 30000) < 300; }));
        REQUIRE(warte([&] { return std::abs(quelle(kEins)->pos - 30000) < 700; }));
        CHECK(player->isPaused());  // der Player wurde dafuer nicht bewegt
        CHECK(player->positionMs() == 0);

        // --- Naht N5, Transport: die Befehle des Player-Panels steuern den ganzen Composer
        player->protokoll.clear();
        const auto befehl = [&](TransportCommandEvent::Action a) { bus->publish(TransportCommandEvent{a}); };
        befehl(TransportCommandEvent::Action::PlayPause);
        CHECK_FALSE(sitzung.laeuft());
        REQUIRE(warte([&] { return !engine->stream(engine->mixer).spielt; }));
        REQUIRE(warte([&] { return zustand == PlaybackState::Paused; }));
        befehl(TransportCommandEvent::Action::PlayPause);
        CHECK(sitzung.laeuft());
        REQUIRE(warte([&] { return engine->stream(engine->mixer).spielt; }));
        REQUIRE(warte([&] { return zustand == PlaybackState::Playing; }));
        // eine Luecke (60–70 s): die Komposition laeuft, kein Song — der zuletzt gezeigte
        // Song bleibt stehen, der Balken haelt, und „laeuft" bleibt (der Play-Knopf zeigt Pause)
        sitzung.springe(63000);
        REQUIRE(warte([&] { return quelle(kEins) == nullptr; }));
        QCoreApplication::processEvents();
        const int gehalten = stelleGemeldet;
        QThread::msleep(300);
        QCoreApplication::processEvents();
        CHECK(stelleGemeldet == gehalten);
        CHECK(zustand == PlaybackState::Playing);
        CHECK(titel() == QStringLiteral("Composer - eins"));
        CHECK(sitzung.laeuft());
        sitzung.springe(1000);
        REQUIRE(warte([&] { return quelle(kEins) != nullptr; }));
        // Next: zum Anfang des zweiten Songs
        befehl(TransportCommandEvent::Action::Next);
        REQUIRE(warte([&] { return gemeldet == kZwei && quelle(kZwei) != nullptr && !quelle(kZwei)->haelt; }));
        CHECK(std::llabs(sitzung.stelleMs() - 70000) < 150);
        CHECK(quelle(kEins) == nullptr);  // der erste Song ist aus dem Mixer
        CHECK(titel() == QStringLiteral("Composer - zwei"));
        // Previous in den ersten Sekunden eines Songs: zum Song davor
        befehl(TransportCommandEvent::Action::Previous);
        REQUIRE(warte([&] { return gemeldet == kEins; }));
        CHECK(sitzung.stelleMs() < 150);
        // Stop: die Komposition haelt am Anfang
        befehl(TransportCommandEvent::Action::Next);
        REQUIRE(warte([&] { return gemeldet == kZwei; }));
        befehl(TransportCommandEvent::Action::Stop);
        CHECK_FALSE(sitzung.laeuft());
        CHECK(sitzung.stelleMs() == 0);
        REQUIRE(warte([&] { return !engine->stream(engine->mixer).spielt; }));
        // keiner dieser Befehle ging am Composer vorbei an den Player
        CHECK(player->zahl(QStringLiteral("toggle")) == 0);
        CHECK(player->zahl(QStringLiteral("stop")) == 0);
        CHECK(player->zahl(QStringLiteral("next")) == 0);
        CHECK(player->zahl(QStringLiteral("previous")) == 0);
        // Lautstaerke und Mute bleiben beim Player
        befehl(TransportCommandEvent::Action::ToggleMute);
        befehl(TransportCommandEvent::Action::VolumeUp);
        CHECK(player->zahl(QStringLiteral("mute")) == 1);
        CHECK(player->zahl(QStringLiteral("volume")) == 1);

        // der Audio-Kanal aus: die Sitzung gibt den Ton frei — der Mixer geht,
        // und der Titel des Players gilt wieder als laufender Song
        const AudioStreamHandle ersterMixer = engine->mixer;
        Komposition stumm = sitzung.komposition();
        stumm.kanaele[1].an = false;
        sitzung.setzeKomposition(stumm);
        REQUIRE(warte([&] { return !sitzung.fuehrtTon(); }));
        CHECK(player->playlist() == liste);
        CHECK(engine->stream(ersterMixer).frei);
        CHECK(engine->quellenZahl() == 0);
        CHECK(sitzung.tonMixer() == INVALID_STREAM);
        CHECK(gemeldet == kListe);
        // das Panel zeigt wieder den Titel des Players, ohne Vorsatz, und der Loop-Knopf geht wieder
        CHECK(titel().isEmpty());
        CHECK(loopKnopf()->isEnabled());
        // der Player spielte, als der Composer uebernahm: er spielt von selbst weiter
        // (Einstellung „Player resumes…", Vorgabe an — Wunsch Patrik S98)
        CHECK(player->isPlaying());
        CHECK(zustand == PlaybackState::Playing);
        stumm.kanaele[1].an = true;
        sitzung.setzeKomposition(stumm);
        REQUIRE(warte([&] { return sitzung.fuehrtTon(); }));
        CHECK(engine->mixer != ersterMixer);

        // Composer aus, die Einstellung diesmal aus: der Player gehoert wieder der Playlist und
        // bleibt angehalten, und die Befehle gehen wie immer an ihn
        CHECK(player->isPaused());  // der Composer hat ihn wieder angehalten
        Sitzung::setPlayerSpieltWeiter(false);
        sitzung.springe(1000);
        REQUIRE(warte([&] { return gemeldet == kEins; }));
        CHECK(titel() == QStringLiteral("Composer - eins"));
        CHECK_FALSE(loopKnopf()->isEnabled());
        sitzung.setAn(false);
        CHECK(titel().isEmpty());
        CHECK(loopKnopf()->isEnabled());
        CHECK_FALSE(sitzung.fuehrtTon());
        CHECK(Sitzung::fuehrende() == nullptr);
        CHECK(player->playlist() == liste);
        CHECK(player->isPaused());
        CHECK(gemeldet == kListe);
        CHECK(engine->quellenZahl() == 0);
        CHECK(lumi::composer::tonMixer() == INVALID_STREAM);  // N13: wieder der Stream des Players
        player->protokoll.clear();
        befehl(TransportCommandEvent::Action::PlayPause);
        befehl(TransportCommandEvent::Action::Next);
        CHECK(player->zahl(QStringLiteral("toggle")) == 1);
        CHECK(player->zahl(QStringLiteral("next")) == 1);

        // --- Composer on laeuft weiter, wenn die Komposition beim Ausschalten lief
        //     (Einstellung „Composition continues…", Vorgabe an — Wunsch Patrik S98)
        sitzung.spiele();
        REQUIRE(warte([&] { return sitzung.laeuft(); }));
        sitzung.setAn(false);
        sitzung.setAn(true);
        CHECK(sitzung.laeuft());
        // hielt sie beim Ausschalten, haelt sie auch danach
        sitzung.pausiere();
        REQUIRE(warte([&] { return !sitzung.laeuft(); }));
        sitzung.setAn(false);
        sitzung.setAn(true);
        CHECK_FALSE(sitzung.laeuft());
        // mit ausgeschalteter Einstellung wartet Composer on an der gemerkten Stelle
        Sitzung::setLaeuftNachEinWeiter(false);
        sitzung.spiele();
        REQUIRE(warte([&] { return sitzung.laeuft(); }));
        sitzung.setAn(false);
        sitzung.setAn(true);
        CHECK_FALSE(sitzung.laeuft());
        sitzung.setAn(false);
    }

    // Die Luecke aus S98, geschlossen in S99 (Entscheid Patrik: a): der Player
    // hat KEINEN Titel, der Composer fuehrt den Ton und wird ausgeschaltet. Die
    // Sitzung meldet einen leeren Songwechsel, und das Player-Panel geht in
    // seinen Ruhezustand (Naht N5, Anzeige — die fuenfte Stelle). Vor dem Bau
    // gezaehlt: kein Songwechsel, beide Panels zeigten weiter den letzten Song.
    TEST_CASE("C5b Sitzung: Composer off ohne Titel im Player — leerer Songwechsel, die Panels im Ruhezustand")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        ChainNode root;
        root.params = ListParams{};
        const QString preset = dir.filePath(QStringLiteral("Leer.lvfx"));
        REQUIRE(saveChainToFile(root, preset));
        Komposition k = neueKomposition();
        REQUIRE(fuegeClipEin(k, 0, preset, 0, 90000) == 0);
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 1);
        REQUIRE(fuegeSongEin(k, 1, kEins, 0, 60000) == 0);
        const QString datei = dir.filePath(QStringLiteral("mitTon.lvcomp"));
        REQUIRE(speichereKomposition(k, datei));

        ServiceContainer dienste;
        dienste.registerSingleton<IEventBus>([](ServiceContainer&) { return std::make_unique<EventBus>(); });
        dienste.registerSingleton<IAudioPlayer>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new PappPlayer); });
        dienste.registerSingleton<IAudioEngine>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new PappMischEngine); });
        IEventBus* bus = dienste.tryResolve<IEventBus>();
        auto* player = dynamic_cast<PappPlayer*>(dienste.tryResolve<IAudioPlayer>());
        auto* engine = dynamic_cast<PappMischEngine*>(dienste.tryResolve<IAudioEngine>());
        REQUIRE(bus != nullptr);
        REQUIRE(player != nullptr);
        REQUIRE(engine != nullptr);
        engine->laengen = {{kEins, 300000}};
        REQUIRE_FALSE(player->hasTrack());  // der Player hat nichts geladen

        int songwechsel = 0;
        QString gemeldet;
        const auto abo = bus->subscribeScoped<TrackChangedEvent>([&](const TrackChangedEvent& e) {
            ++songwechsel;
            gemeldet = e.track.filePath;
        });

        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }

        const auto texte = [](const QWidget& w) {
            QStringList alle;
            for (const QLabel* l : w.findChildren<QLabel*>()) alle << l->text();
            return alle;
        };
        PlayerPanel panel(dienste);
        TagViewPanel tagView(dienste);
        panel.show();
        tagView.show();
        QCoreApplication::processEvents();
        // vor dem Composer: beide Panels im Ruhezustand
        CHECK(texte(panel).contains(QStringLiteral("No track loaded")));
        CHECK(texte(tagView).contains(QStringLiteral("No track loaded")));

        Sitzung& sitzung = Sitzung::fuer(widget);
        REQUIRE(sitzung.lade(datei));
        sitzung.setAn(true);
        REQUIRE(warte([&] { return sitzung.fuehrtTon(); }));
        sitzung.spiele();
        REQUIRE(warte([&] { return gemeldet == kEins; }));
        QCoreApplication::processEvents();
        CHECK(texte(panel).contains(QStringLiteral("Composer - eins")));
        CHECK(texte(tagView).contains(QDir::toNativeSeparators(kEins)));
        const int vorher = songwechsel;

        sitzung.setAn(false);
        QCoreApplication::processEvents();
        REQUIRE_FALSE(sitzung.fuehrtTon());
        const QStringList imPlayer = texte(panel);
        const QStringList imTagView = texte(tagView);
        // genau ein Songwechsel, und er ist leer
        CHECK(songwechsel == vorher + 1);
        CHECK(gemeldet.isEmpty());
        // das Player-Panel: Ruhezustand, kein Vorsatz, kein „Unknown Title"
        CHECK(imPlayer.contains(QStringLiteral("No track loaded")));
        CHECK(imPlayer.contains(QStringLiteral("00:00 / 00:00")));
        CHECK_FALSE(imPlayer.contains(QStringLiteral("Composer - eins")));
        CHECK_FALSE(imPlayer.contains(QStringLiteral("Unknown Title")));
        CHECK_FALSE(imPlayer.contains(QStringLiteral("Unknown Artist")));
        for (const QSlider* s : panel.findChildren<QSlider*>())
            if (s->maximum() == 1000) CHECK(s->value() == 0);
        // Tag View: kein Titel mehr
        CHECK(imTagView.contains(QStringLiteral("No track loaded")));
        CHECK_FALSE(imTagView.contains(QDir::toNativeSeparators(kEins)));

        // Composer on, ohne dass ein Song gemeldet wurde (die Uhr steht hinter dem
        // letzten Song), und wieder aus: kein zweiter leerer Songwechsel
        sitzung.pausiere();
        sitzung.springe(80000);
        sitzung.setAn(true);
        REQUIRE(warte([&] { return sitzung.fuehrtTon(); }));
        QCoreApplication::processEvents();
        const int dazwischen = songwechsel;
        if (gemeldet.isEmpty())
        {
            sitzung.setAn(false);
            QCoreApplication::processEvents();
            CHECK(songwechsel == dazwischen);
        }
        else
        {
            sitzung.setAn(false);
        }

        // Sichttest S99, A2: das Player-Panel ist nicht zu sehen (ein anderer
        // Reiter liegt davor), waehrend der Composer ausgeschaltet wird — ohne
        // Abo verpasst es den leeren Songwechsel. Beim Einblenden darf es nicht
        // weiter den letzten Song der Komposition zeigen.
        QCoreApplication::processEvents();
        gemeldet.clear();
        sitzung.springe(0);
        sitzung.setAn(true);
        REQUIRE(warte([&] { return sitzung.fuehrtTon(); }));
        sitzung.spiele();
        REQUIRE(warte([&] { return gemeldet == kEins; }));
        QCoreApplication::processEvents();
        REQUIRE(texte(panel).contains(QStringLiteral("Composer - eins")));
        panel.hide();
        QCoreApplication::processEvents();
        sitzung.setAn(false);
        QCoreApplication::processEvents();
        REQUIRE_FALSE(sitzung.fuehrtTon());
        CHECK(gemeldet.isEmpty());  // der leere Songwechsel kam — nur nicht beim Panel
        panel.show();
        QCoreApplication::processEvents();
        const QStringList nachEinblenden = texte(panel);
        CHECK(nachEinblenden.contains(QStringLiteral("No track loaded")));
        CHECK(nachEinblenden.contains(QStringLiteral("00:00 / 00:00")));
        CHECK_FALSE(nachEinblenden.contains(QStringLiteral("Composer - eins")));
        for (const QSlider* s : panel.findChildren<QSlider*>())
            if (s->maximum() == 1000) CHECK(s->value() == 0);
    }

    TEST_CASE("S130 C1 c: das Player-Panel zeigt einen ausgeschalteten Clip mit (off) — Tag View und Stelle bleiben beim Song")
    {
        int argc = 1;
        char arg0[10] = "UnitTests";
        char* argv[2] = {arg0, nullptr};
        QApplication app{argc, argv};
        EigeneEinstellungen einstellungen;

        QTemporaryDir dir;
        REQUIRE(dir.isValid());
        ChainNode root;
        root.params = ListParams{};
        const QString preset = dir.filePath(QStringLiteral("Leer.lvfx"));
        REQUIRE(saveChainToFile(root, preset));
        // A1: eins 0–20 s · A2: zwei 20–40 s, ausgeschaltet · danach eine Luecke bis 60 s
        Komposition k = neueKomposition();
        REQUIRE(fuegeClipEin(k, 0, preset, 0, 60000) == 0);
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 1);
        REQUIRE(fuegeSongEin(k, 1, kEins, 0, 20000) == 0);
        REQUIRE(fuegeKanalEin(k, {}, KanalArt::Audio) == 2);
        REQUIRE(fuegeSongEin(k, 2, kZwei, 20000, 20000) == 0);
        k.kanaele[2].clips[0].an = false;
        const QString datei = dir.filePath(QStringLiteral("aus.lvcomp"));
        REQUIRE(speichereKomposition(k, datei));

        ServiceContainer dienste;
        dienste.registerSingleton<IEventBus>([](ServiceContainer&) { return std::make_unique<EventBus>(); });
        dienste.registerSingleton<IAudioPlayer>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioPlayer>(new PappPlayer); });
        dienste.registerSingleton<IAudioEngine>(
            [](ServiceContainer&) { return std::unique_ptr<IAudioEngine>(new PappMischEngine); });
        IEventBus* bus = dienste.tryResolve<IEventBus>();
        auto* engine = dynamic_cast<PappMischEngine*>(dienste.tryResolve<IAudioEngine>());
        REQUIRE(bus != nullptr);
        REQUIRE(engine != nullptr);
        engine->laengen = {{kEins, 300000}, {kZwei, 300000}};
        // was Karaoke, Tag View und die Visuals erfahren
        QString gemeldet;
        int stelleGemeldet = -1;
        const auto abo =
            bus->subscribeScoped<TrackChangedEvent>([&](const TrackChangedEvent& e) { gemeldet = e.track.filePath; });
        const auto aboStelle = bus->subscribeScoped<PlaybackPositionEvent>(
            [&](const PlaybackPositionEvent& e) { stelleGemeldet = e.positionMs; });

        VisualizerWidget widget(dienste);
        widget.setFrameMode(RenderPacing::Unlimited, 60);
        widget.resize(200, 150);
        testfenster::platziere(widget);
        widget.show();
        if (!widget.hasVisualizer() || !warte([&] { return widget.isVisible(); }))
        {
            MESSAGE("Kein Visualizer oder kein sichtbares Fenster (Headless?) — uebersprungen");
            return;
        }
        PlayerPanel panel(dienste);
        testfenster::platziere(panel);
        panel.show();
        const auto titel = [&]() -> QString {
            for (const QLabel* l : panel.findChildren<QLabel*>())
                if (l->text().startsWith(QStringLiteral("Composer - "))) return l->text();
            return {};
        };
        QSlider* balken = nullptr;
        for (QSlider* s : panel.findChildren<QSlider*>())
            if (s->maximum() == 1000) balken = s;
        REQUIRE(balken != nullptr);

        Sitzung& sitzung = Sitzung::fuer(widget);
        REQUIRE(sitzung.lade(datei));
        sitzung.setAn(true);
        REQUIRE(warte([&] { return sitzung.fuehrtTon(); }));
        sitzung.springe(5000);
        sitzung.spiele();
        REQUIRE(warte([&] { return titel() == QStringLiteral("Composer - eins"); }));
        REQUIRE(warte([&] { return stelleGemeldet >= 5000; }));

        // im ausgeschalteten Clip: das Panel nennt ihn mit (off), der Balken laeuft durch ihn
        sitzung.springe(25000);
        REQUIRE(warte([&] { return titel() == QStringLiteral("Composer - zwei (off)"); }));
        REQUIRE(warte([&] { return balken->value() >= 16; }));  // 5 s von 300 s
        const int balkenVorher = balken->value();
        QThread::msleep(400);
        REQUIRE(warte([&] { return balken->value() > balkenVorher; }));
        // nichts davon ging an Karaoke, Tag View oder die Stelle der Visuals
        CHECK(gemeldet == kEins);
        CHECK(stelleGemeldet < 20000);
        CHECK(engine->quelle(kZwei) == INVALID_STREAM);  // er klingt nicht
        // ein Panel, das erst jetzt sichtbar wird, zeigt ihn ebenso
        panel.hide();
        QCoreApplication::processEvents();
        panel.show();
        QCoreApplication::processEvents();
        CHECK(titel() == QStringLiteral("Composer - zwei (off)"));
        // ein Klick in den Balken springt in ihm: ein Hundertstel (3 s im Song) = 23 s
        balken->setValue(10);
        Q_EMIT balken->sliderReleased();
        REQUIRE(warte([&] { return std::llabs(sitzung.stelleMs() - 23000) < 300; }));

        // in der Luecke danach: wieder der letzte gespielte Song, wie in jeder Luecke (S98)
        sitzung.springe(45000);
        REQUIRE(warte([&] { return titel() == QStringLiteral("Composer - eins"); }));
        CHECK(gemeldet == kEins);
        // und zurueck in den gespielten Song
        sitzung.springe(25000);
        REQUIRE(warte([&] { return titel() == QStringLiteral("Composer - zwei (off)"); }));
        sitzung.springe(2000);
        REQUIRE(warte([&] { return titel() == QStringLiteral("Composer - eins"); }));
        REQUIRE(warte([&] { return stelleGemeldet >= 2000 && stelleGemeldet < 4000; }));

        // Composer aus waehrend des ausgeschalteten Clips: das Panel faellt nicht auf ihn zurueck
        sitzung.springe(25000);
        REQUIRE(warte([&] { return titel() == QStringLiteral("Composer - zwei (off)"); }));
        sitzung.setAn(false);
        QCoreApplication::processEvents();
        CHECK(titel().isEmpty());
    }
}
