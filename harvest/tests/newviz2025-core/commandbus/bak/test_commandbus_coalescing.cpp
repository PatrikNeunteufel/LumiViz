#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "TestHelpers.hpp"
#include "core/commandbus/CoalescingPolicy.hpp"
#include "core/commandbus/CommandBus.hpp"

using namespace viz::core;

TEST_CASE("coalescing via ICommand::tryMergeWith")
{
    CommandBus bus;
    CommandContext ctx{};

    auto a = std::make_shared<viz::test::MergeCommand>(1, 2);
    auto b = std::make_shared<viz::test::MergeCommand>(1, 3);
    auto c = std::make_shared<viz::test::MergeCommand>(2, 9);

    REQUIRE(bus.submit(a, ctx).ok);
    REQUIRE(bus.submit(b, ctx).ok); // merges with last
    REQUIRE(bus.undoSize() == 1);

    REQUIRE(bus.submit(c, ctx).ok); // new id -> new step
    REQUIRE(bus.undoSize() == 2);
}

TEST_CASE("global CoalescingPolicy merges commands")
{
    CommandBus bus;
    CommandContext ctx{};

    bus.coalescing().set<viz::test::MergeCommand>([](const viz::test::MergeCommand& l, const viz::test::MergeCommand& r)
                                                  { return l.id == r.id; });

    auto x1 = std::make_shared<viz::test::MergeCommand>(7, 1);
    auto x2 = std::make_shared<viz::test::MergeCommand>(7, 1);
    auto y1 = std::make_shared<viz::test::MergeCommand>(8, 1);

    REQUIRE(bus.submit(x1, ctx).ok);
    REQUIRE(bus.submit(x2, ctx).ok); // merged by policy
    REQUIRE(bus.undoSize() == 1);

    REQUIRE(bus.submit(y1, ctx).ok);
    REQUIRE(bus.undoSize() == 2);
}
