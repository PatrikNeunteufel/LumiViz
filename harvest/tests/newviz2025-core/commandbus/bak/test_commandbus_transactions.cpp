#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "TestHelpers.hpp"
#include "core/commandbus/CommandBus.hpp"
#include "core/commandbus/TransactionGuard.hpp"

using namespace viz::core;

TEST_CASE("transaction groups multiple commands")
{
    CommandBus bus;
    CommandContext ctx{};
    viz::test::Counter c{};

    {
        TransactionGuard tx(bus, "DoubleAdd");
        REQUIRE(bus.submit(std::make_shared<viz::test::AddCommand>(c, 2), ctx).ok);
        REQUIRE(bus.submit(std::make_shared<viz::test::AddCommand>(c, 3), ctx).ok);
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
    viz::test::Counter c{};

    {
        TransactionGuard tx(bus, "NotCommitted");
        REQUIRE(bus.submit(std::make_shared<viz::test::AddCommand>(c, 7), ctx).ok);
        // no commit()
    }

    REQUIRE(c.v == 0);
    REQUIRE_FALSE(bus.canUndo());
}
