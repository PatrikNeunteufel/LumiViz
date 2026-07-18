#include <catch2/catch_all.hpp>
#include <catch2/catch_approx.hpp>

#include "core/basetypes/BaseAgent.hpp"
#include "core/commandbus/CommandBus.hpp"
#include "core/eventbus/EventBus.hpp"
#include "core/servicecontainer/ServiceContainer.hpp"

namespace
{
    using Base = viz::core::BaseAgent;
    class TestAgent : public Base
    {
        
        public:
        explicit TestAgent(viz::core::ServiceContainer& s, viz::core::EventBus& e, viz::core::CommandBus& c) noexcept
            : Base{s, e, c}
        {
        }

        // Überschreibe genau die drei Hooks, die BaseAgent bereitstellt
        bool onStart() noexcept override
        {
            ++onStartCount;
            return true;
        }
        bool onTick(float dt) noexcept override
        {
            tickSum += dt;
            return true;
        }
        void onStop() noexcept override
        {
            ++onStopCount;
        }

        int onStartCount{0};
        int onStopCount{0};
        float tickSum{0.0f};
    };
} // namespace

TEST_CASE("BaseAgent: lifecycle happy path", "[basetypes][agent]")
{
    viz::core::ServiceContainer services;
    viz::core::EventBus events;
    viz::core::CommandBus commands{&events};

    TestAgent a{services, events, commands};

    // initial
    REQUIRE_FALSE(a.isRunning());

    // initialize -> start
    REQUIRE(a.initialize());
    REQUIRE_FALSE(a.isRunning());
    REQUIRE(a.start());
    REQUIRE(a.isRunning());
    REQUIRE(a.onStartCount == 1);

    // ticks while running
    REQUIRE(a.tick(0.016f)); // 16 ms
    REQUIRE(a.tick(0.004f)); // 4 ms
    REQUIRE(a.tickSum == Catch::Approx(0.020f).margin(1e-6f));

    // stop -> not running
    REQUIRE(a.stop());
    REQUIRE_FALSE(a.isRunning());
    REQUIRE(a.onStopCount == 1);

    // shutdown ok
    REQUIRE(a.shutdown());
}

TEST_CASE("BaseAgent: tick returns false when not running", "[basetypes][agent]")
{
    viz::core::ServiceContainer services;
    viz::core::EventBus events;
    viz::core::CommandBus commands{&events};

    TestAgent a{services, events, commands};

    REQUIRE(a.initialize());
    REQUIRE_FALSE(a.isRunning());

    // tick ohne start -> sollte false liefern und onTick nicht aufrufen
    REQUIRE_FALSE(a.tick(0.010f));
    REQUIRE(a.tickSum == Catch::Approx(0.0f));
}
