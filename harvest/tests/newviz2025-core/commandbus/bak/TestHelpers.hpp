#pragma once
// Keep helpers self-contained: only standard headers + the EventBus include you already have.
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "core/EventBus.hpp" // Eure vorhandene Implementierung
#include "core/commandbus/CommandBusEvents.hpp"
#include "core/commandbus/CommandContext.hpp"
#include "core/commandbus/CommandResult.hpp"
#include "core/commandbus/ICommand.hpp"

namespace viz::test
{

    // ---- Simple counter model ---------------------------------------------------
    struct Counter
    {
        int v{0};
    };

    // ---- Basic AddCommand -------------------------------------------------------
    struct AddCommand final : viz::core::ICommand
    {
        Counter& c;
        int by{1};
        bool executed{false};

        explicit AddCommand(Counter& cc, int b) : c(cc), by(b) {}
        const char* name() const noexcept override
        {
            return "Add";
        }

        viz::core::CommandResult execute(const viz::core::CommandContext&) override
        {
            if (executed)
                return viz::core::CommandResult::Ok();
            c.v += by;
            executed = true;
            return viz::core::CommandResult::Ok();
        }
        viz::core::CommandResult undo(const viz::core::CommandContext&) override
        {
            if (!executed)
                return viz::core::CommandResult::Ok();
            c.v -= by;
            executed = false;
            return viz::core::CommandResult::Ok();
        }
    };

    // ---- Mergeable command for coalescing tests --------------------------------
    struct MergeCommand final : viz::core::ICommand
    {
        int id{0};
        int sum{0};
        bool executed{false};

        explicit MergeCommand(int id_, int delta) : id(id_), sum(delta) {}
        const char* name() const noexcept override
        {
            return "MergeCmd";
        }

        viz::core::CommandResult execute(const viz::core::CommandContext&) override
        {
            executed = true;
            return viz::core::CommandResult::Ok();
        }
        viz::core::CommandResult undo(const viz::core::CommandContext&) override
        {
            executed = false;
            return viz::core::CommandResult::Ok();
        }
        bool tryMergeWith(const std::shared_ptr<viz::core::ICommand>& next) override
        {
            auto n = std::dynamic_pointer_cast<MergeCommand>(next);
            if (!n || n->id != id)
                return false;
            sum += n->sum;
            return true;
        }
    };

    // ---- Failing command to test error path ------------------------------------
    struct FailingCommand final : viz::core::ICommand
    {
        const char* name() const noexcept override
        {
            return "Failing";
        }
        viz::core::CommandResult execute(const viz::core::CommandContext&) override
        {
            return viz::core::CommandResult::Fail("fail", 42);
        }
        viz::core::CommandResult undo(const viz::core::CommandContext&) override
        {
            return viz::core::CommandResult::Ok();
        }
    };

    // ---- EventBus sink for bridge tests ----------------------------------------
    struct BridgeSink
    {
        std::vector<viz::core::CommandWillExecute> will;
        std::vector<viz::core::CommandDidExecute> did;
        std::vector<viz::core::CommandUndone> undone;
        std::vector<viz::core::CommandRedone> redone;
        std::vector<viz::core::CommandStacksChanged> stacks;

        template<class EB> void wire(EB& bus)
        {
            [[maybe_unused]] auto t1 =
                bus.template subscribe<viz::core::CommandWillExecute>([&](const auto& e) { will.push_back(e); });
            [[maybe_unused]] auto t2 =
                bus.template subscribe<viz::core::CommandDidExecute>([&](const auto& e) { did.push_back(e); });
            [[maybe_unused]] auto t3 =
                bus.template subscribe<viz::core::CommandUndone>([&](const auto& e) { undone.push_back(e); });
            [[maybe_unused]] auto t4 =
                bus.template subscribe<viz::core::CommandRedone>([&](const auto& e) { redone.push_back(e); });
            [[maybe_unused]] auto t5 =
                bus.template subscribe<viz::core::CommandStacksChanged>([&](const auto& e) { stacks.push_back(e); });
        }
    };

    // ---- Thread token helper ----------------------------------------------------
    inline void* token(std::uintptr_t v)
    {
        return reinterpret_cast<void*>(v);
    }

} // namespace viz::test
