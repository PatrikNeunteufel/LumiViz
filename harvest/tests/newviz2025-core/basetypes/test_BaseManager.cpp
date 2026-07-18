
#include <catch2/catch_all.hpp>

#include "core/basetypes/BaseManager.hpp"
#include "core/commandbus/CommandBus.hpp" // <-- neu
#include "core/eventbus/EventBus.hpp"
#include "core/servicecontainer/ServiceContainer.hpp"

namespace viz::core
{
    /**
     * @brief Test-Manager: zählt Hook-Aufrufe und erlaubt gesteuerte Rückgabewerte.
     */
    class TestManager final : public BaseManager
    {
      public:
        // <-- BaseManager erwartet (services, events, commands)
        explicit TestManager(ServiceContainer& services, EventBus& events, CommandBus& commands) noexcept
            : BaseManager(services, events, commands)
        {
        }

        // Steuerflags (Default: alles OK)
        bool m_initOK{true};
        bool m_startOK{true};
        bool m_tickOK{true};

        // Zähler
        int m_initCount{0};
        int m_startCount{0};
        int m_tickCount{0};
        int m_stopCount{0};
        int m_shutdownCount{0};

      protected:
        bool onInitialize() noexcept override
        {
            ++m_initCount;
            return m_initOK;
        }
        bool onStart() noexcept override
        {
            ++m_startCount;
            return m_startOK;
        }
        bool onTick(float) noexcept override
        {
            ++m_tickCount;
            return m_tickOK;
        }
        void onStop() noexcept override
        {
            ++m_stopCount;
        }
        void onShutdown() noexcept override
        {
            ++m_shutdownCount;
        }
    };
} // namespace viz::core

// -----------------------------------------------------------------------------
// Happy Path: vollständiger Lifecycle und Hook-Zähler
// -----------------------------------------------------------------------------
TEST_CASE("BaseManager: full lifecycle happy path", "[basemanager][lifecycle]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus commands;

    TestManager m{services, events, commands};

    REQUIRE(m.initialize());
    REQUIRE(m.m_initCount == 1);
    REQUIRE(m.start());
    REQUIRE(m.m_startCount == 1);
    REQUIRE(m.isRunning());

    REQUIRE(m.tick(0.016f));
    REQUIRE(m.m_tickCount == 1);
    REQUIRE(m.isRunning());

    REQUIRE(m.stop());
    REQUIRE(m.m_stopCount == 1);
    REQUIRE_FALSE(m.isRunning());

    REQUIRE(m.shutdown());
    REQUIRE(m.m_shutdownCount == 1);
    REQUIRE_FALSE(m.isRunning());
}

// -----------------------------------------------------------------------------
// initialize() schlägt fehl -> start() wird vom Test nicht aufgerufen
// -----------------------------------------------------------------------------
TEST_CASE("BaseManager: initialize failure propagates and running stays false", "[basemanager][lifecycle]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus commands;

    TestManager m{services, events, commands};
    m.m_initOK = false;

    REQUIRE_FALSE(m.initialize());
    REQUIRE(m.m_initCount == 1);
    REQUIRE_FALSE(m.isRunning());

    REQUIRE_FALSE(m.shutdown()); // Prod gibt false zurück, kein Hook
    REQUIRE(m.m_shutdownCount == 0);
}

// -----------------------------------------------------------------------------
// start() schlägt fehl -> running bleibt false
// -----------------------------------------------------------------------------
TEST_CASE("BaseManager: start failure propagates and running stays false", "[basemanager][lifecycle]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus commands;

    TestManager m{services, events, commands};
    m.m_initOK = true;
    m.m_startOK = false;

    REQUIRE(m.initialize());
    REQUIRE(m.m_initCount == 1);
    REQUIRE_FALSE(m.start());
    REQUIRE(m.m_startCount == 1);
    REQUIRE_FALSE(m.isRunning());

    
    REQUIRE(m.shutdown()); // Prod ruft Hook auf
    REQUIRE(m.m_shutdownCount == 1);
}

// -----------------------------------------------------------------------------
// tick() Rückgabewert wird durchgereicht; Aufrufzähler erhöht sich
// -----------------------------------------------------------------------------
TEST_CASE("BaseManager: tick return value propagates", "[basemanager][tick]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus commands;

    TestManager m{services, events, commands};

    REQUIRE(m.initialize());
    REQUIRE(m.start());
    REQUIRE(m.isRunning());

    // 1) tick ok
    m.m_tickOK = true;
    REQUIRE(m.tick(0.010f));
    REQUIRE(m.m_tickCount == 1);

    // 2) tick not ok
    m.m_tickOK = false;
    REQUIRE_FALSE(m.tick(0.010f));
    REQUIRE(m.m_tickCount == 2);

    REQUIRE(m.stop());
    REQUIRE(m.shutdown());
}

// -----------------------------------------------------------------------------
// start() ohne initialize(): darf nicht laufen, isRunning() bleibt false
// -----------------------------------------------------------------------------
TEST_CASE("BaseManager: start without initialize fails and stays not running", "[basemanager][lifecycle][sync]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus commands;
    TestManager m{services, events, commands};

    REQUIRE_FALSE(m.isRunning());
    REQUIRE_FALSE(m.start()); // ohne initialize -> start schlägt fehl
    REQUIRE_FALSE(m.isRunning());
    REQUIRE(m.m_startCount == 0); // Hook wurde nicht gerufen
}

// -----------------------------------------------------------------------------
// Idempotenz: stop() und shutdown() jeweils nur 1x „wirksam“
// -----------------------------------------------------------------------------
TEST_CASE("BaseManager: stop/shutdown are idempotent", "[basemanager][lifecycle][sync]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus commands;
    TestManager m{services, events, commands};

    // normaler Start
    REQUIRE(m.initialize());
    REQUIRE(m.start());
    REQUIRE(m.isRunning());

    // 1. stop: wirksam
    REQUIRE(m.stop());
    REQUIRE(m.m_stopCount == 1);
    REQUIRE_FALSE(m.isRunning());

    // 2. stop: nicht mehr wirksam
    REQUIRE_FALSE(m.stop());
    REQUIRE(m.m_stopCount == 1);

    // 1. shutdown: wirksam (nach init/start)
    REQUIRE(m.shutdown());
    REQUIRE(m.m_shutdownCount == 1);

    // 2. shutdown: nicht mehr wirksam
    REQUIRE_FALSE(m.shutdown());
    REQUIRE(m.m_shutdownCount == 1);
}

// -----------------------------------------------------------------------------
// tick() wenn nicht running: gibt false zurück und erhöht Zähler nicht
// -----------------------------------------------------------------------------
TEST_CASE("BaseManager: tick when not running returns false and does not count", "[basemanager][tick][sync]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus commands;
    TestManager m{services, events, commands};

    REQUIRE_FALSE(m.isRunning());
    REQUIRE_FALSE(m.tick(0.02f));
    REQUIRE(m.m_tickCount == 0);

    // Nach regulärem Start tickt er normal
    REQUIRE(m.initialize());
    REQUIRE(m.start());
    REQUIRE(m.isRunning());
    REQUIRE(m.tick(0.02f));
    REQUIRE(m.m_tickCount == 1);

    REQUIRE(m.stop());
    REQUIRE(m.shutdown());
}
