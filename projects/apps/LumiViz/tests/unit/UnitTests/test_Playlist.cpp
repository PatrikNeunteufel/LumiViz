/**
 ****************************************************************************************
 * @file   test_Playlist.cpp
 * @brief  Unit-Tests für die Playlist (Track-Verwaltung, Navigation, Shuffle,
 *         M3U-Persistenz, EventBus-Anbindung)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "audio/Playlist.hpp"
#include "audio/AudioEvents.hpp"
#include "services/EventBus.hpp"

#include <QFile>
#include <QTemporaryDir>

#include <algorithm>

namespace
{

/// Legt echte (leere) Dateien an — die Playlist validiert Pfade ggf. gegen das Dateisystem
struct PlaylistFixture
{
    PlaylistFixture()
    {
        REQUIRE(tmp.isValid());
        for (const char* name : {"a.mp3", "b.mp3", "c.mp3"})
        {
            QFile f(tmp.filePath(name));
            REQUIRE(f.open(QIODevice::WriteOnly));
            f.write("x");
            f.close();
            paths << f.fileName();
        }
    }

    QTemporaryDir tmp;
    QStringList paths;
    EventBus bus;
};

} // namespace

// =============================================================================
// Track-Verwaltung
// =============================================================================

TEST_CASE("Playlist: addTracks/count/filePathAt/indexOf")
{
    PlaylistFixture fx;
    Playlist pl(fx.bus);

    CHECK(pl.isEmpty());
    pl.addTracks(fx.paths);
    CHECK(pl.count() == 3);
    CHECK_FALSE(pl.isEmpty());

    CHECK(pl.filePathAt(0) == fx.paths[0]);
    CHECK(pl.indexOf(fx.paths[1]) == 1);
    CHECK(pl.indexOf("nicht-vorhanden.mp3") == -1);
}

TEST_CASE("Playlist: removeTrack/insertTrack/moveTrack/swapTracks")
{
    PlaylistFixture fx;
    Playlist pl(fx.bus);
    pl.addTracks(fx.paths);

    CHECK(pl.removeTrack(1)); // b raus
    CHECK(pl.count() == 2);
    CHECK(pl.filePathAt(1) == fx.paths[2]);

    CHECK(pl.insertTrack(1, fx.paths[1])); // b wieder an Position 1
    CHECK(pl.count() == 3);
    CHECK(pl.filePathAt(1) == fx.paths[1]);

    CHECK(pl.moveTrack(0, 2)); // a ans Ende
    CHECK(pl.filePathAt(2) == fx.paths[0]);

    CHECK(pl.swapTracks(0, 2)); // und zurueck
    CHECK(pl.filePathAt(0) == fx.paths[0]);

    CHECK_FALSE(pl.removeTrack(99)); // ungueltiger Index
}

// =============================================================================
// Navigation
// =============================================================================

TEST_CASE("Playlist: Navigation mit und ohne Wrap")
{
    PlaylistFixture fx;
    Playlist pl(fx.bus);
    pl.addTracks(fx.paths);

    pl.setCurrentIndex(0);
    CHECK(pl.currentIndex() == 0);
    CHECK(pl.hasNext());
    CHECK_FALSE(pl.hasPrevious());

    CHECK(pl.next() == 1);
    CHECK(pl.next() == 2);
    CHECK_FALSE(pl.hasNext());

    SUBCASE("ohne Wrap endet am Rand")
    {
        CHECK(pl.next(false) == -1);
        CHECK(pl.currentIndex() == 2); // unveraendert
    }

    SUBCASE("mit Wrap geht es rundherum")
    {
        CHECK(pl.next(true) == 0);
        CHECK(pl.previous(true) == 2);
    }
}

TEST_CASE("Playlist: shuffle erhaelt die Track-Menge")
{
    PlaylistFixture fx;
    Playlist pl(fx.bus);
    pl.addTracks(fx.paths);

    pl.shuffle();

    CHECK(pl.count() == 3);
    QStringList after = pl.filePaths();
    QStringList expected = fx.paths;
    std::sort(after.begin(), after.end());
    std::sort(expected.begin(), expected.end());
    CHECK(after == expected); // gleiche Menge, ggf. andere Reihenfolge
}

// =============================================================================
// EventBus-Anbindung
// =============================================================================

TEST_CASE("Playlist: addTrack publiziert PlaylistChangedEvent(Added)")
{
    PlaylistFixture fx;
    Playlist pl(fx.bus);

    int addedEvents = 0;
    int lastCount = -1;
    fx.bus.subscribe<PlaylistChangedEvent>([&](const PlaylistChangedEvent& e) {
        if (e.action == PlaylistChangedEvent::Action::Added)
        {
            ++addedEvents;
            lastCount = e.trackCount;
        }
    });

    pl.addTrack(fx.paths[0]);

    CHECK(addedEvents >= 1);
    CHECK(lastCount == 1);
}

// =============================================================================
// Persistenz (M3U)
// =============================================================================

TEST_CASE("Playlist: M3U speichern und in frische Playlist laden")
{
    PlaylistFixture fx;
    Playlist pl(fx.bus);
    pl.addTracks(fx.paths);

    const QString m3u = fx.tmp.filePath("test.m3u");
    REQUIRE(pl.save(m3u));

    EventBus bus2;
    Playlist loaded(bus2);
    REQUIRE(loaded.load(m3u));

    CHECK(loaded.count() == 3);
    // Reihenfolge muss erhalten bleiben; Pfad-Schreibweise darf variieren -> Dateiname pruefen
    CHECK(loaded.filePathAt(0).endsWith("a.mp3"));
    CHECK(loaded.filePathAt(2).endsWith("c.mp3"));
}
