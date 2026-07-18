#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "TestHelpers.hpp" // first to avoid macro/parse issues
#include "core/commandbus/CommandBus.hpp"
#include "core/commandbus/CommandContext.hpp"

using namespace viz::core;

TEST_CASE("submit / undo / redo basics")
{
    CommandBus bus;
    CommandContext ctx{};
    viz::test::Counter c{};

    auto cmd = std::make_shared<viz::test::AddCommand>(c, 5);
    REQUIRE(bus.submit(cmd, ctx).ok);
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
    viz::test::Counter c{};

    REQUIRE(bus.undoSize() == 0);
    REQUIRE(bus.redoSize() == 0);

    REQUIRE(bus.submit(std::make_shared<viz::test::AddCommand>(c, 1), ctx).ok);
    REQUIRE(bus.submit(std::make_shared<viz::test::AddCommand>(c, 2), ctx).ok);

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

    auto fail = std::make_shared<viz::test::FailingCommand>();
    auto res = bus.submit(fail, ctx);
    REQUIRE_FALSE(res.ok);

    REQUIRE(bus.undoSize() == beforeUndo);
    REQUIRE(bus.redoSize() == beforeRedo);
}

