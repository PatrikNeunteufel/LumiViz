/**
 ****************************************************************************************
 * @file   test_ComposerNahtTon.cpp
 * @brief  Composer, Stufe C5: die Naehte zum Player — N7a Pegel, N7b Zugang, N5 Transport —
 *         und die Sitzung, die den Player fuehrt (docs/ui/Composer_Konzept.md §12.2)
 *
 * @author Patrik Neunteufel
 * @date   October 2026
 *
 * @details
 * - **Der echte `AudioPlayer`** mit einer Audio-Engine aus Pappe: der Pegel
 *   (Naht N7a) wirkt neben der Lautstaerke und aendert sie nicht; und ohne
 *   Playlist (`setPlaylist(nullptr)`) spielt am Titelende nichts weiter — der
 *   Beleg dafuer, dass die Playlist ohne Naht ruhen kann.
 * - **Die Sitzung** mit einem Player aus Pappe in den Diensten des Fensters
 *   (Naht N7b): sie uebernimmt ihn, der Ton fuehrt die Uhr, und die Befehle des
 *   Player-Panels (Naht N5) steuern den ganzen Composer.
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
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>

#include <cstdlib>
#include <functional>
#include <memory>

using namespace lumi::multieffect;
using namespace lumi::composer;
using papp::PappEngine;
using papp::PappPlayer;

namespace
{

const QString kEins = QStringLiteral("C:/musik/eins.mp3");
const QString kZwei = QStringLiteral("C:/musik/zwei.mp3");

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
    TEST_CASE("N7a Pegel: ein zweiter Faktor neben der Lautstaerke — sie selbst bleibt, wie sie ist")
    {
        EventBus bus;
        PappEngine engine;
        engine.laengen = {{kEins, 60000}, {kZwei, 60000}};
        AudioPlayer player(engine, bus);
        int meldungen = 0;
        const auto abo = bus.subscribeScoped<VolumeChangedEvent>([&](const VolumeChangedEvent&) { ++meldungen; });
        const auto laut = [&] { return engine.stream(engine.letzter()).lautstaerke; };

        // Vorgabe: Pegel 1 — die Lautstaerke geht an die Engine wie immer
        CHECK(player.pegel() == doctest::Approx(1.0f));
        REQUIRE(player.load(kEins));
        CHECK(laut() == doctest::Approx(1.0f));
        player.setVolume(0.8f);
        CHECK(laut() == doctest::Approx(0.8f));
        CHECK(meldungen == 1);

        // der Pegel kommt obendrauf; Lautstaerke, Regler und Ereignisse bleiben
        player.setPegel(0.5f);
        CHECK(laut() == doctest::Approx(0.4f));
        CHECK(player.volume() == doctest::Approx(0.8f));
        CHECK(player.pegel() == doctest::Approx(0.5f));
        CHECK(meldungen == 1);
        // die Lautstaerke aendern: der Pegel gilt weiter
        player.setVolume(0.6f);
        CHECK(laut() == doctest::Approx(0.3f));
        player.setVolume(0.8f);

        // stumm bleibt stumm; danach gelten beide wieder
        player.setMuted(true);
        CHECK(laut() == doctest::Approx(0.0f));
        player.setPegel(0.25f);
        CHECK(laut() == doctest::Approx(0.0f));
        player.setMuted(false);
        CHECK(laut() == doctest::Approx(0.2f));

        // ein neuer Song bekommt beides gleich mit
        REQUIRE(player.load(kZwei));
        CHECK(laut() == doctest::Approx(0.2f));

        // zurueck auf voll; Unsinn wird geklemmt
        player.setPegel(1.0f);
        CHECK(laut() == doctest::Approx(0.8f));
        player.setPegel(7.0f);
        CHECK(player.pegel() == doctest::Approx(1.0f));
        player.setPegel(-1.0f);
        CHECK(player.pegel() == doctest::Approx(0.0f));
        CHECK(laut() == doctest::Approx(0.0f));

        // ein Player, der den Pegel nicht kennt, hat die Vorgabe des Interfaces
        struct Ohne : PappPlayer
        {
            [[nodiscard]] float pegel() const override { return IAudioPlayer::pegel(); }
            void setPegel(float pegel) override { IAudioPlayer::setPegel(pegel); }
        } ohne;
        ohne.setPegel(0.3f);
        CHECK(ohne.pegel() == doctest::Approx(1.0f));
    }

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

    TEST_CASE("C5 Sitzung: der Audiokanal fuehrt den Player — Zugang (N7b), der Ton fuehrt, Transport (N5)")
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
        IEventBus* bus = dienste.tryResolve<IEventBus>();
        auto* player = dynamic_cast<PappPlayer*>(dienste.tryResolve<IAudioPlayer>());
        REQUIRE(bus != nullptr);
        REQUIRE(player != nullptr);
        player->laengen = {{kEins, 300000}, {kZwei, 300000}};
        int platz = 0;  // nie benutzt, nur weitergereicht: irgendeine Adresse genuegt als Playlist
        auto* liste = reinterpret_cast<IPlaylist*>(&platz);
        player->setPlaylist(liste);

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

        // Composer on: die Sitzung uebernimmt den Player, die Playlist ruht
        sitzung.setAn(true);
        REQUIRE(sitzung.an());
        REQUIRE(warte([&] { return sitzung.fuehrtTon(); }));
        CHECK(Sitzung::fuehrende() == &sitzung);
        CHECK(player->playlist() == nullptr);

        // Play: der erste Song wird geladen und spielt
        sitzung.spiele();
        REQUIRE(warte([&] { return player->isPlaying(); }));
        CHECK(player->currentTrack().filePath == kEins);
        // der Ton fuehrt: der Player aus Pappe rueckt nicht vor — also bleibt auch die Uhr stehen
        QThread::msleep(300);
        QCoreApplication::processEvents();
        CHECK(sitzung.laeuft());
        CHECK(sitzung.stelleMs() < 200);
        // der Fortschrittsbalken im Player-Panel: der Ton springt, die Uhr geht mit
        player->seek(30000);
        REQUIRE(warte([&] { return std::llabs(sitzung.stelleMs() - 30000) < 150; }));
        CHECK(player->positionMs() == 30000);  // der Player wurde nicht zurueckgeholt
        // der Pegel des Clips geht an den Player (Naht N7a): nach der Einblendung voll ...
        REQUIRE(warte([&] { return std::abs(player->pegel() - 1.0f) < 0.01f; }));
        // ... und ein Sprung im Composer setzt den Player: mitten in die Einblendung
        // (die Uhr laeuft — der Player landet ein paar Millisekunden hinter der Sekunde)
        sitzung.springe(1000);
        REQUIRE(warte([&] { return std::abs(player->positionMs() - 1000) < 150; }));
        REQUIRE(warte([&] { return std::abs(player->pegel() - 0.5f) < 0.05f; }));

        // --- Naht N5: die Befehle des Player-Panels steuern den ganzen Composer
        PlayerPanel panel(dienste);
        player->protokoll.clear();
        const auto befehl = [&](TransportCommandEvent::Action a) { bus->publish(TransportCommandEvent{a}); };
        befehl(TransportCommandEvent::Action::PlayPause);
        CHECK_FALSE(sitzung.laeuft());
        REQUIRE(warte([&] { return player->isPaused(); }));
        befehl(TransportCommandEvent::Action::PlayPause);
        CHECK(sitzung.laeuft());
        REQUIRE(warte([&] { return player->isPlaying(); }));
        // Next: zum Anfang des zweiten Songs
        befehl(TransportCommandEvent::Action::Next);
        REQUIRE(warte([&] { return player->currentTrack().filePath == kZwei && player->isPlaying(); }));
        CHECK(std::llabs(sitzung.stelleMs() - 70000) < 150);
        // Previous in den ersten Sekunden eines Songs: zum Song davor
        befehl(TransportCommandEvent::Action::Previous);
        REQUIRE(warte([&] { return player->currentTrack().filePath == kEins; }));
        CHECK(sitzung.stelleMs() < 150);
        // Stop: die Komposition haelt am Anfang
        befehl(TransportCommandEvent::Action::Next);
        REQUIRE(warte([&] { return player->currentTrack().filePath == kZwei; }));
        befehl(TransportCommandEvent::Action::Stop);
        CHECK_FALSE(sitzung.laeuft());
        CHECK(sitzung.stelleMs() == 0);
        REQUIRE(warte([&] { return !player->isPlaying(); }));
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

        // der Audio-Kanal aus: die Sitzung gibt den Player frei
        Komposition stumm = sitzung.komposition();
        stumm.kanaele[1].an = false;
        sitzung.setzeKomposition(stumm);
        REQUIRE(warte([&] { return !sitzung.fuehrtTon(); }));
        CHECK(player->playlist() == liste);
        stumm.kanaele[1].an = true;
        sitzung.setzeKomposition(stumm);
        REQUIRE(warte([&] { return sitzung.fuehrtTon(); }));

        // Composer aus: der Player gehoert wieder der Playlist, der Pegel steht auf voll,
        // und die Befehle gehen wie immer an ihn
        sitzung.springe(1000);
        REQUIRE(warte([&] { return std::abs(player->pegel() - 0.5f) < 0.05f; }));
        sitzung.setAn(false);
        CHECK_FALSE(sitzung.fuehrtTon());
        CHECK(Sitzung::fuehrende() == nullptr);
        CHECK(player->playlist() == liste);
        CHECK(player->pegel() == doctest::Approx(1.0f));
        player->protokoll.clear();
        befehl(TransportCommandEvent::Action::PlayPause);
        befehl(TransportCommandEvent::Action::Next);
        CHECK(player->zahl(QStringLiteral("toggle")) == 1);
        CHECK(player->zahl(QStringLiteral("next")) == 1);
    }
}
