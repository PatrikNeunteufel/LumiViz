// tests/core/commandbus/test_commandbus_all.cpp
//
// One-Translation-Unit Tests für CommandBus:
//  - enthält eigene Test-Helper (Counter, AddCommand, MergeCommand, FailingCommand, BridgeSink)
//  - enthält ALLE Testcases (basic, transactions, coalescing, bridge, adapter, async, dispatch)
//  - vermeidet Include-Reihenfolge-Probleme
//
// Build: CMakeLists in diesem Ordner kompiliert zusätzlich die Produktions-Implementierungen
//        (CommandBus.cpp, CompositeCommand.cpp, TransactionGuard.cpp) in dieses Test-Target.

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "core/commandbus/CoalescingPolicy.hpp"
#include "core/commandbus/CommandBus.hpp"
#include "core/commandbus/CommandBusEvents.hpp"
#include "core/commandbus/CommandContext.hpp"
#include "core/commandbus/CommandResult.hpp"
#include "core/commandbus/ICommand.hpp"
#include "core/commandbus/TCommandAdapter.hpp"
#include "core/commandbus/TransactionGuard.hpp"
#include "core/eventbus/EventBus.hpp"

using namespace viz::core;

// ============================================================================
// Test-Helper (eigenständig, minimal und ohne versteckte Abhängigkeiten)
// ============================================================================
namespace test
{

    // --- Simple counter model ----------------------------------------------------
    struct Counter
    {
        int v{0};
    };

    // --- Basic AddCommand --------------------------------------------------------
    struct AddCommand final : ICommand
    {
        Counter& c;
        int by{1};
        bool executed{false};

        explicit AddCommand(Counter& cc, int b) : c(cc), by(b) {}

        const char* name() const noexcept override
        {
            return "Add";
        }

        CommandResult execute(const CommandContext&) override
        {
            if (executed)
                return CommandResult::Ok();
            c.v += by;
            executed = true;
            return CommandResult::Ok();
        }
        CommandResult undo(const CommandContext&) override
        {
            if (!executed)
                return CommandResult::Ok();
            c.v -= by;
            executed = false;
            return CommandResult::Ok();
        }
    };

    // --- Mergeable command for coalescing tests ---------------------------------
    struct MergeCommand final : ICommand
    {
        int id{0};
        int sum{0};
        bool executed{false};

        explicit MergeCommand(int id_, int delta) : id(id_), sum(delta) {}

        const char* name() const noexcept override
        {
            return "MergeCmd";
        }

        CommandResult execute(const CommandContext&) override
        {
            executed = true;
            return CommandResult::Ok();
        }
        CommandResult undo(const CommandContext&) override
        {
            executed = false;
            return CommandResult::Ok();
        }
        bool tryMergeWith(const std::shared_ptr<ICommand>& next) override
        {
            auto n = std::dynamic_pointer_cast<MergeCommand>(next);
            if (!n || n->id != id)
                return false;
            sum += n->sum;
            return true;
        }
    };

    // --- Failing command to test error path -------------------------------------
    struct FailingCommand final : ICommand
    {
        const char* name() const noexcept override
        {
            return "Failing";
        }
        CommandResult execute(const CommandContext&) override
        {
            return CommandResult::Fail("fail", 42);
        }
        CommandResult undo(const CommandContext&) override
        {
            return CommandResult::Ok();
        }
    };

    // ---- EventBus sink for bridge tests ----------------------------------------
    struct BridgeSink
    {
        std::vector<CommandWillExecute> will;
        std::vector<CommandDidExecute> did;
        std::vector<CommandUndone> undone;
        std::vector<CommandRedone> redone;
        std::vector<CommandStacksChanged> stacks;

        template<class EB> void wire(EB& bus)
        {
            [[maybe_unused]] auto t1 =
                bus.template subscribe<CommandWillExecute>([&](const auto& e) { will.push_back(e); });
            [[maybe_unused]] auto t2 =
                bus.template subscribe<CommandDidExecute>([&](const auto& e) { did.push_back(e); });
            [[maybe_unused]] auto t3 =
                bus.template subscribe<CommandUndone>([&](const auto& e) { undone.push_back(e); });
            [[maybe_unused]] auto t4 =
                bus.template subscribe<CommandRedone>([&](const auto& e) { redone.push_back(e); });
            [[maybe_unused]] auto t5 =
                bus.template subscribe<CommandStacksChanged>([&](const auto& e) { stacks.push_back(e); });
        }
    };

    // ---- Thread token helper ----------------------------------------------------
    inline void* token(std::uintptr_t v)
    {
        return reinterpret_cast<void*>(v);
    }

} // namespace test

// ============================================================================
// Testcases
// ============================================================================

TEST_CASE("execute / undo / redo basics")
{
    CommandBus bus;
    CommandContext ctx{};
    test::Counter c{};

    auto cmd = std::make_shared<test::AddCommand>(c, 5);
    REQUIRE(bus.execute(cmd, ctx).ok);
    REQUIRE(c.v == 5);

    REQUIRE(bus.canUndo());
    REQUIRE(bus.topUndoName() == std::string("Add"));
    REQUIRE(bus.undo(ctx).ok);
    REQUIRE(c.v == 0);

    REQUIRE(bus.canRedo());
    REQUIRE(bus.topRedoName() == std::string("Add"));
    REQUIRE(bus.redo(ctx).ok);
    REQUIRE(c.v == 5);
}

TEST_CASE("history snapshots expose sizes and labels")
{
    CommandBus bus;
    CommandContext ctx{};
    test::Counter c{};

    REQUIRE(bus.undoSize() == 0);
    REQUIRE(bus.redoSize() == 0);

    REQUIRE(bus.execute(std::make_shared<test::AddCommand>(c, 1), ctx).ok);
    REQUIRE(bus.execute(std::make_shared<test::AddCommand>(c, 2), ctx).ok);

    auto labels = bus.undoLabels();
    REQUIRE(labels.size() == 2);
    REQUIRE(labels.back() == std::string("Add"));

    REQUIRE(bus.undoSize() == 2);
    REQUIRE(bus.undo(ctx).ok);
    REQUIRE(bus.undoSize() == 1);
    REQUIRE(bus.redoSize() == 1);

    auto redoLabels = bus.redoLabels();
    REQUIRE(redoLabels.size() == 1);
    REQUIRE(redoLabels.back() == std::string("Add"));
}

TEST_CASE("failing execute does not change stacks")
{
    CommandBus bus;
    CommandContext ctx{};

    auto beforeUndo = bus.undoSize();
    auto beforeRedo = bus.redoSize();

    auto fail = std::make_shared<test::FailingCommand>();
    auto res = bus.execute(fail, ctx);
    REQUIRE_FALSE(res.ok);

    REQUIRE(bus.undoSize() == beforeUndo);
    REQUIRE(bus.redoSize() == beforeRedo);
}

TEST_CASE("transaction groups multiple commands")
{
    CommandBus bus;
    CommandContext ctx{};
    test::Counter c{};

    {
        TransactionGuard tx(bus, "DoubleAdd");
        REQUIRE(bus.execute(std::make_shared<test::AddCommand>(c, 2), ctx).ok);
        REQUIRE(bus.execute(std::make_shared<test::AddCommand>(c, 3), ctx).ok);
        tx.commit();
    }

    REQUIRE(c.v == 5);
    REQUIRE(bus.canUndo());

    const std::string top = bus.topUndoName();
    const bool okLabel = (top == "DoubleAdd") || (top == "Transaction");
    REQUIRE(okLabel);

    REQUIRE(bus.undo(ctx).ok);
    REQUIRE(c.v == 0);
}

TEST_CASE("TransactionGuard rolls back on dtor if not committed")
{
    CommandBus bus;
    CommandContext ctx{};
    test::Counter c{};

    {
        TransactionGuard tx(bus, "NotCommitted");
        REQUIRE(bus.execute(std::make_shared<test::AddCommand>(c, 7), ctx).ok);
        // no commit()
    } // -> rollback

    REQUIRE(c.v == 0);
    REQUIRE_FALSE(bus.canUndo());
}

TEST_CASE("coalescing via ICommand::tryMergeWith")
{
    CommandBus bus;
    CommandContext ctx{};

    auto a = std::make_shared<test::MergeCommand>(1, 2);
    auto b = std::make_shared<test::MergeCommand>(1, 3);
    auto c = std::make_shared<test::MergeCommand>(2, 9);

    REQUIRE(bus.execute(a, ctx).ok);
    REQUIRE(bus.execute(b, ctx).ok); // merges with last
    REQUIRE(bus.undoSize() == 1);

    REQUIRE(bus.execute(c, ctx).ok); // new id -> new step
    REQUIRE(bus.undoSize() == 2);
}

TEST_CASE("global CoalescingPolicy merges commands")
{
    CommandBus bus;
    CommandContext ctx{};

    bus.coalescing().set<test::MergeCommand>([](const test::MergeCommand& l, const test::MergeCommand& r)
                                             { return l.id == r.id; });

    auto x1 = std::make_shared<test::MergeCommand>(7, 1);
    auto x2 = std::make_shared<test::MergeCommand>(7, 1);
    auto y1 = std::make_shared<test::MergeCommand>(8, 1);

    REQUIRE(bus.execute(x1, ctx).ok);
    REQUIRE(bus.execute(x2, ctx).ok); // merged by policy
    REQUIRE(bus.undoSize() == 1);

    REQUIRE(bus.execute(y1, ctx).ok);
    REQUIRE(bus.undoSize() == 2);
}

TEST_CASE("EventBus bridge publishes lifecycle events + stacks changed")
{
    EventBus eb{};
    test::BridgeSink sink;
    sink.wire(eb);

    CommandBus bus{&eb};
    CommandContext ctx{};

    test::Counter c{};
    auto cmd = std::make_shared<test::AddCommand>(c, 5);

    // execute -> WillExecute + DidExecute + StacksChanged
    REQUIRE(bus.execute(cmd, ctx).ok);
    REQUIRE_FALSE(sink.will.empty());
    REQUIRE_FALSE(sink.did.empty());
    REQUIRE_FALSE(sink.stacks.empty());
    REQUIRE(sink.did.back().name == std::string("Add"));
    REQUIRE(sink.did.back().ok == true);
    REQUIRE(sink.did.back().duration_ms >= 0);

    // undo -> Undone + StacksChanged (kein WillExecute)
    auto willBefore = sink.will.size();
    REQUIRE(bus.undo(ctx).ok);
    REQUIRE_FALSE(sink.undone.empty());
    REQUIRE(sink.undone.back().ok == true);
    REQUIRE(sink.undone.back().duration_ms >= 0);
    REQUIRE(sink.stacks.back().canRedo == true);
    REQUIRE(sink.will.size() == willBefore);

    // redo -> WillExecute + Redone + StacksChanged
    REQUIRE(bus.redo(ctx).ok);
    REQUIRE(sink.will.size() == willBefore + 1);
    REQUIRE_FALSE(sink.redone.empty());
    REQUIRE(sink.redone.back().ok == true);
    REQUIRE(sink.redone.back().duration_ms >= 0);
}

struct SetValue
{
    int* ptr;
    int to;
};

TEST_CASE("TCommandAdapter executes and undoes payload commands")
{
    CommandBus bus;
    CommandContext ctx{};
    int v = 0;

    int before = v;
    auto doFn = [&](const SetValue& p, const CommandContext&) -> CommandResult
    {
        before = *p.ptr;
        *p.ptr = p.to;
        return CommandResult::Ok();
    };
    auto undoFn = [&](const SetValue& p, const CommandContext&) -> CommandResult
    {
        *p.ptr = before;
        return CommandResult::Ok();
    };

    auto cmd = std::make_shared<TCommandAdapter<SetValue>>("SetValue", SetValue{&v, 123}, doFn, undoFn);

    REQUIRE(bus.execute(cmd, ctx).ok);
    REQUIRE(v == 123);

    REQUIRE(bus.undo(ctx).ok);
    REQUIRE(v == 0);

    REQUIRE(bus.redo(ctx).ok);
    REQUIRE(v == 123);
}

TEST_CASE("Thread-affinity violation returns error 1001")
{
    CommandBus bus;
    test::Counter c{};
    auto cmd = std::make_shared<test::AddCommand>(c, 1);

    CommandContext ctx{};
    ctx.mainThreadToken = test::token(1);
    ctx.currentThreadToken = test::token(2);

    auto res = bus.execute(cmd, ctx);
    REQUIRE_FALSE(res.ok);
    REQUIRE(res.errorCode == 1001);
}

// -----------------------------------------------------------------------------
// Neue Tests: submitAsync + dispatch-Alias
// -----------------------------------------------------------------------------
TEST_CASE("submitAsync runs command and invokes completion callback")
{
    CommandBus bus;
    CommandContext ctx{};
    test::Counter c{};
    auto cmd = std::make_shared<test::AddCommand>(c, 10);

    std::atomic<bool> completionCalled{false};

    auto fut = bus.submitAsync(cmd, ctx,
                               [&](const CommandResult& r)
                               {
                                   REQUIRE(r.ok);
                                   completionCalled.store(true, std::memory_order_relaxed);
                               });

    auto r = fut.get();
    REQUIRE(r.ok);
    REQUIRE(completionCalled.load(std::memory_order_relaxed));
    REQUIRE(c.v == 10);
}

TEST_CASE("dispatch is an alias to execute")
{
    CommandBus bus;
    CommandContext ctx{};
    test::Counter c{};

    auto cmd = std::make_shared<test::AddCommand>(c, 3);
    auto r1 = bus.dispatch(cmd, ctx);
    REQUIRE(r1.ok);
    REQUIRE(c.v == 3);

    REQUIRE(bus.undo(ctx).ok);
    REQUIRE(c.v == 0);
}
