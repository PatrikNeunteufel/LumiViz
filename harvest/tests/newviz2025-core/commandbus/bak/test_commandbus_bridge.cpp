#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "TestHelpers.hpp" // MUSS zuerst rein
#include "core/EventBus.hpp"
#include "core/commandbus/CommandBus.hpp"
#include "core/commandbus/CommandBusEvents.hpp"

using namespace viz::core;

TEST_CASE("EventBus bridge publishes lifecycle events + stacks changed")
{
    EventBus eb{};
    viz::test::BridgeSink sink;
    sink.wire(eb);

    CommandBus bus{&eb};
    CommandContext ctx{};

    viz::test::Counter c{};
    auto cmd = std::make_shared<viz::test::AddCommand>(c, 5);

    // submit -> WillExecute + DidExecute + StacksChanged
    REQUIRE(bus.submit(cmd, ctx).ok);
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

TEST_CASE("StacksChanged labels reflect current tops")
{
    EventBus eb{};
    viz::test::BridgeSink sink;
    sink.wire(eb);

    CommandBus bus{&eb};
    CommandContext ctx{};
    viz::test::Counter c{};

    REQUIRE(bus.submit(std::make_shared<viz::test::AddCommand>(c, 1), ctx).ok);
    REQUIRE(!sink.stacks.empty());
    auto s1 = sink.stacks.back();
    REQUIRE(s1.canUndo == true);
    REQUIRE(s1.undoLabel == std::string("Add"));

    REQUIRE(bus.undo(ctx).ok);
    auto s2 = sink.stacks.back();
    REQUIRE(s2.canRedo == true);
    REQUIRE(s2.redoLabel == std::string("Add"));
}
