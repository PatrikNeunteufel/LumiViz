
#include <catch2/catch_all.hpp>

#include "core/basetypes/BaseController.hpp"
#include "core/commandbus/CommandBus.hpp"
#include "core/commandbus/CommandContext.hpp"
#include "core/commandbus/CommandResult.hpp"
#include "core/commandbus/ICommand.hpp"
#include "core/eventbus/EventBus.hpp"
#include "core/servicecontainer/ServiceContainer.hpp"

namespace viz::core
{
    /// @brief Simple sync command:
    /// - execute(): ++counter
    /// - undo():    sets undone = true
    struct CountCommand final : ICommand
    {
        int* m_counter{nullptr};
        bool* m_undone{nullptr};

        explicit CountCommand(int& counter, bool& undone) noexcept : m_counter(&counter), m_undone(&undone) {}

        const char* name() const noexcept override
        {
            return "CountCommand";
        }

        CommandResult execute(const CommandContext&) override
        {
            ++(*m_counter);
            return CommandResult::Ok();
        }

        CommandResult undo(const CommandContext&) override
        {
            *m_undone = true;
            return CommandResult::Ok();
        }
    };

    struct TestController final : BaseController
    {
        using BaseController::BaseController;

        bool onInitialize() noexcept override
        {
            return true;
        }
        bool onStart() noexcept override
        {
            return true;
        }
        bool onTick(float) noexcept override
        {
            return true;
        }
        void onStop() noexcept override {}
    };
} // namespace viz::core

// -----------------------------------------------------------------------------

TEST_CASE("BaseController sync: execute with explicit context", "[basecontroller][sync]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus bus;
    TestController c{services, events, bus};

    REQUIRE(c.initialize());
    REQUIRE(c.start());

    int counter = 0;
    bool undone = false;

    // drei getrennte Befehle; alle synchron via execute(ctx)
    CountCommand a{counter, undone};
    CountCommand b{counter, undone};
    CountCommand d{counter, undone};

    CommandContext ctx{}; // explizit; Inhalt wird hier nicht überprüft
    (void)c.execute(a, ctx);
    (void)c.execute(b, ctx);
    (void)c.execute(d, ctx);

    REQUIRE(counter == 3);
    REQUIRE(undone == false);

    c.stop();
}

TEST_CASE("BaseController sync: dispatchAdapter do-only and do+undo lambdas", "[basecontroller][sync]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus bus;
    TestController c{services, events, bus};

    REQUIRE(c.initialize());
    REQUIRE(c.start());

    int counter = 0;
    bool undone = false;

    // do-only
    c.dispatchAdapter("only-do", [&] { ++counter; });
    REQUIRE(counter == 1);
    REQUIRE(undone == false);

    // do+undo — wir rufen kein undo, prüfen nur do-Effekt
    c.dispatchAdapter("do-undo", [&] { ++counter; }, [&] { undone = true; });
    REQUIRE(counter == 2);
    REQUIRE(undone == false);

    c.stop();
}

TEST_CASE("BaseController sync: withTransaction runs a mixed batch (execute + adapters)",
          "[basecontroller][sync][transaction]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus bus;
    TestController c{services, events, bus};

    REQUIRE(c.initialize());
    REQUIRE(c.start());

    int counter = 0;
    bool undone = false;

    CountCommand cmd{counter, undone};

    c.withTransaction("mixed-batch",
                      [&]
                      {
                          // 1) normaler Befehl
                          (void)c.execute(cmd);

                          // 2) zwei Adapter-Befehle
                          c.dispatchAdapter("inc-a", [&] { ++counter; });
                          c.dispatchAdapter("inc-b", [&] { ++counter; });
                      });

    REQUIRE(counter == 3);
    REQUIRE(undone == false);

    c.stop();
}
