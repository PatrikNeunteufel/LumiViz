// Templates immer sichtbar machen (falls dein Header noch bedingt inkludiert)


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
    /// @brief Simple command for testing:
    /// - execute(): ++counter
    /// - undo():    sets undone = true
    struct CountCommand final : ICommand
    {
        int* m_counter{nullptr};
        bool* m_undone{nullptr};

        explicit CountCommand(int& counter, bool& undone) noexcept : m_counter(&counter), m_undone(&undone) {}

        /// @brief required by ICommand
        const char* name() const noexcept override
        {
            return "CountCommand";
        }

        /// @brief increments the counter
        CommandResult execute(const CommandContext&) override
        {
            ++(*m_counter);
            return CommandResult::Ok();
        }

        /// @brief marks undo flag
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
} // namespace viz::core

TEST_CASE("BaseController: lifecycle + execute + dispatchAdapter", "[basecontroller]")
{
    using namespace viz::core;

    ServiceContainer services;
    EventBus events;
    CommandBus bus;

    TestController c{services, events, bus};

    // lifecycle
    REQUIRE(c.initialize());
    REQUIRE(c.start());
    REQUIRE(c.tick(0.016f));

    // execute(ICommand&, ctx)
    int counter = 0;
    bool undone = false;

    CountCommand cmd{counter, undone};
    auto r1 = c.execute(cmd); // default context
    // Kein r1.success() verwenden (existiert nicht) – wir verifizieren per beobachtbarem Effekt
    REQUIRE(counter == 1);
    REQUIRE(undone == false);

    // dispatchAdapter(label, do, undo) -> ruft intern execute(...)
    c.dispatchAdapter("inc", [&] { ++counter; }, [&] { undone = true; });
    REQUIRE(counter == 2);

    c.stop();
}
