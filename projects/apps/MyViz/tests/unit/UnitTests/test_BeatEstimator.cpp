/**
 ****************************************************************************************
 * @file   test_BeatEstimator.cpp
 * @brief  Tests fuer den vorhersagenden Beat-Schaetzer (Import-Phase Roadmap 4.4,
 *         Port der AVS-bpm.cpp-Logik) — synthetische Onset-Folgen, kein Audio
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 ****************************************************************************************
 */

#include <doctest.h>

#include "visualizers/modules/processing/BeatEstimator.hpp"

using lumi::modules::BeatEstimator;

namespace
{

/// Simulierter Frame-Takt (10-ms-Ticks) mit Onsets in festem Intervall
struct Sim
{
    BeatEstimator est{0};
    std::int64_t now = 0;
    int refinedBeats = 0;

    /// @param ms            Dauer der Simulation
    /// @param onsetInterval Onset alle N ms (0 = Stille)
    void run(int ms, int onsetInterval)
    {
        constexpr int kTick = 10;
        for (int t = 0; t < ms; t += kTick)
        {
            now += kTick;
            const bool onset = onsetInterval > 0 && (now % onsetInterval) == 0;
            if (est.refine(onset, now)) ++refinedBeats;
        }
    }
};

} // namespace

TEST_CASE("BeatEstimator: konstanter 120-BPM-Onset wird gelernt")
{
    Sim sim;
    sim.run(12000, 500);   // 24 Onsets im 500-ms-Raster
    CHECK(sim.est.bpm() >= 118);   // Resync-Nudge (±1 %) + 10-ms-Raster
    CHECK(sim.est.bpm() <= 122);
    CHECK(sim.est.confidence() >= 70);
}

TEST_CASE("BeatEstimator: Vorhersage laeuft ohne Onsets weiter (Fade-out)")
{
    Sim sim;
    sim.run(12000, 500);
    REQUIRE(sim.est.bpm() >= 118);   // Resync-Nudge (±1 %) + 10-ms-Raster
    REQUIRE(sim.est.bpm() <= 122);

    sim.refinedBeats = 0;
    sim.run(2000, 0);   // Stille
    // Erwartung: ~4 vorhergesagte Beats in 2 s bei 120 BPM
    CHECK(sim.refinedBeats >= 3);
    CHECK(sim.refinedBeats <= 5);
}

TEST_CASE("BeatEstimator: zu schneller Onset (240 BPM) wird halbiert")
{
    Sim sim;
    sim.run(20000, 250);
    CHECK(sim.est.bpm() >= 110);
    CHECK(sim.est.bpm() <= 130);
}

TEST_CASE("BeatEstimator: Sticky friert die Vorhersage ein")
{
    Sim sim;
    sim.est.setConfig({/*sticky=*/true, /*onlySticky=*/false});
    sim.run(12000, 500);
    REQUIRE(sim.est.bpm() >= 118);   // Resync-Nudge (±1 %) + 10-ms-Raster
    REQUIRE(sim.est.bpm() <= 122);

    sim.est.setSticked(true);
    const int locked = sim.est.bpm();
    sim.run(12000, 600);   // Tempo-Wechsel auf 100 BPM
    CHECK(sim.est.sticked());
    CHECK(sim.est.bpm() == locked);   // eingerastet: keine Adaption
}

TEST_CASE("BeatEstimator: Tempowechsel nach Track-Wechsel wird neu gelernt")
{
    // Harte Tempo-Spruenge INNERHALB eines Tracks adaptiert das Original nur
    // traege (Vorhersage-Guesses verunreinigen die Historie) — dafuer gibt es
    // die Song-Wechsel-Erkennung. Wir testen genau dieses Szenario.
    Sim sim;
    sim.est.setConfig({/*sticky=*/false, /*onlySticky=*/false});
    sim.run(12000, 500);
    REQUIRE(sim.est.bpm() >= 118);   // Resync-Nudge (±1 %) + 10-ms-Raster
    REQUIRE(sim.est.bpm() <= 122);

    sim.est.notifyTrackChanged(sim.now);
    sim.run(15000, 600);   // neuer Track mit 100 BPM
    CHECK(sim.est.bpm() >= 95);
    CHECK(sim.est.bpm() <= 105);
}

TEST_CASE("BeatEstimator: onlySticky reicht Onsets unveraendert durch")
{
    Sim sim;
    sim.est.setConfig({/*sticky=*/true, /*onlySticky=*/true});
    sim.run(12000, 500);

    sim.refinedBeats = 0;
    sim.run(2000, 0);   // Stille: ohne Einrasten keine Vorhersage-Ausgabe
    CHECK(sim.refinedBeats == 0);
}

TEST_CASE("BeatEstimator: Track-Wechsel setzt das Lernen zurueck")
{
    Sim sim;
    sim.run(12000, 500);
    REQUIRE(sim.est.bpm() >= 118);   // Resync-Nudge (±1 %) + 10-ms-Raster
    REQUIRE(sim.est.bpm() <= 122);
    sim.est.notifyTrackChanged(sim.now);
    CHECK(sim.est.bpm() == 0);
    CHECK_FALSE(sim.est.sticked());
    CHECK(sim.est.confidence() == 0);
}

TEST_CASE("BeatEstimator: Reset liefert Lernzustand")
{
    Sim sim;
    sim.run(12000, 500);
    sim.est.reset(sim.now);
    CHECK(sim.est.bpm() == 0);
    // und lernt danach wieder
    sim.run(12000, 500);
    CHECK(sim.est.bpm() >= 118);
    CHECK(sim.est.bpm() <= 122);
}
