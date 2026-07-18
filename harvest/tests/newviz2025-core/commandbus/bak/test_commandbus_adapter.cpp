#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "TestHelpers.hpp"
#include "core/commandbus/CommandBus.hpp"
#include "core/commandbus/TCommandAdapter.hpp"

using namespace viz::core;

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

    REQUIRE(bus.submit(cmd, ctx).ok);
    REQUIRE(v == 123);

    REQUIRE(bus.undo(ctx).ok);
    REQUIRE(v == 0);

    REQUIRE(bus.redo(ctx).ok);
    REQUIRE(v == 123);
}

TEST_CASE("Thread-affinity violation returns error 1001")
{
    CommandBus bus;
    viz::test::Counter c{};
    auto cmd = std::make_shared<viz::test::AddCommand>(c, 1);

    CommandContext ctx{};
    ctx.mainThreadToken = viz::test::token(1);
    ctx.currentThreadToken = viz::test::token(2);

    auto res = bus.submit(cmd, ctx);
    REQUIRE_FALSE(res.ok);
    REQUIRE(res.errorCode == 1001);
}
