#include <atomic>
#include <catch2/catch_all.hpp>
#include <stdexcept>
#include <vector>


#include "core/basetypes/BaseController.hpp"
#include "core/commandbus/CommandBus.hpp"
#include "core/commandbus/CommandContext.hpp"
#include "core/commandbus/CommandResult.hpp"
#include "core/commandbus/ICommand.hpp"
#include "core/eventbus/EventBus.hpp"
#include "core/servicecontainer/ServiceContainer.hpp"

namespace viz::core
{
    /// @brief Simple command for async/tx testing:
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

    /// @brief Minimal concrete controller for lifecycle testing.
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

    /// @brief Controller that fails during initialize().
    struct FailingInitController final : BaseController
    {
        using BaseController::BaseController;
        bool onInitialize() noexcept override
        {
            return false;
        }
    };

    /// @brief Controller that fails during start().
    struct FailingStartController final : BaseController
    {
        using BaseController::BaseController;
        bool onStart() noexcept override
        {
            return false;
        }
    };
} // namespace viz::core

// -----------------------------------------------------------------------------
// submitAsync: returns future and invokes completion
// -----------------------------------------------------------------------------
TEST_CASE("BaseController: submitAsync returns future and calls completion", "[basecontroller][async]")
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
    std::atomic<bool> completionCalled{false};

    auto fut = c.submitAsync(cmd, CommandContext{},
                             [&](const CommandResult&) { completionCalled.store(true, std::memory_order_release); });

    // current impl may run synchronously – we validate by effects
    (void)fut.get();
    REQUIRE(counter == 1);
    REQUIRE(completionCalled.load(std::memory_order_acquire));

    c.stop();
}

// -----------------------------------------------------------------------------
// withTransaction: executes body and leaves bus consistent
// -----------------------------------------------------------------------------
TEST_CASE("BaseController: withTransaction executes body", "[basecontroller][transaction]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus bus;
    TestController c{services, events, bus};

    REQUIRE(c.initialize());
    REQUIRE(c.start());

    int counter = 0;

    c.withTransaction("batch-inc",
                      [&]
                      {
                          c.dispatchAdapter("inc-a", [&] { ++counter; });
                          c.dispatchAdapter("inc-b", [&] { ++counter; });
                      });

    REQUIRE(counter == 2);

    c.stop();
}

// -----------------------------------------------------------------------------
// withTransaction: exceptions in body do not escape (noexcept) and controller remains usable
// -----------------------------------------------------------------------------
TEST_CASE("BaseController: withTransaction survives exceptions and remains usable",
          "[basecontroller][transaction][exceptions]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus bus;
    TestController c{services, events, bus};

    REQUIRE(c.initialize());
    REQUIRE(c.start());

    int counter = 0;

    // The body throws; BaseController::withTransaction(...) is noexcept.
    // We only assert: no throw escapes and controller/bus remain usable afterwards.
    c.withTransaction("will-throw",
                      [&]
                      {
                          c.dispatchAdapter("inc", [&] { ++counter; });
                          throw std::runtime_error("boom");
                      });

    // Controller still usable: a new transaction must work
    c.withTransaction("after-throw", [&] { c.dispatchAdapter("inc2", [&] { ++counter; }); });

    REQUIRE(counter == 2);

    c.stop();
}

// -----------------------------------------------------------------------------
// Lifecycle: initialize() failure propagates; start() should not be called by the test then
// -----------------------------------------------------------------------------
TEST_CASE("BaseController: initialize failure propagates", "[basecontroller][lifecycle]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus bus;
    FailingInitController c{services, events, bus};

    REQUIRE_FALSE(c.initialize());

    // We intentionally do not call start/tick/stop when initialize fails.
    // Nothing else to assert here, just ensure no UB/throw occurs at boundary.
}

// -----------------------------------------------------------------------------
// Lifecycle: start() failure propagates; tick() should not be called by the test then
// -----------------------------------------------------------------------------
TEST_CASE("BaseController: start failure propagates", "[basecontroller][lifecycle]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus bus;
    FailingStartController c{services, events, bus};

    REQUIRE(c.initialize());
    REQUIRE_FALSE(c.start());

    // We intentionally do not call tick/stop when start fails.
    // Boundary must remain noexcept/consistent.
}

// -----------------------------------------------------------------------------
// submitAsync: multiple async submissions (simple fan-out), all effects applied
// -----------------------------------------------------------------------------
TEST_CASE("BaseController: multiple submitAsync fan-out applies all effects", "[basecontroller][async]")
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

std::vector<std::future<viz::core::CommandResult>> futures;
    futures.reserve(3);

    // create lvalues and submit
    CountCommand cmd1{counter, undone};
    CountCommand cmd2{counter, undone};
    CountCommand cmd3{counter, undone};

    futures.emplace_back(c.submitAsync(cmd1));
    futures.emplace_back(c.submitAsync(cmd2));
    futures.emplace_back(c.submitAsync(cmd3));

    for (auto& f : futures)
        (void)f.get();


    REQUIRE(counter == 3);
    REQUIRE(undone == false);

    c.stop();
}
