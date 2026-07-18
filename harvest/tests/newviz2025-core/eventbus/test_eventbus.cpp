#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <thread>
#include <vector>

#include "common/Attributes.hpp"
#include "common/Types.hpp"
#include "core/eventbus/EventBus.hpp"

using viz::core::EventBus;

namespace
{
    struct AppStarted
    {
        int argc{};
    };
    struct ValueChanged
    {
        int before{}, after{};
    };
    struct AudioFrame
    {
        float rms{}, peak{};
    };
} // namespace

TEST_CASE("basic subscribe/publish", "[eventbus]")
{
    EventBus bus;
    int sum = 0;
    auto id = bus.subscribe<ValueChanged>([&](const ValueChanged& e) { sum += (e.after - e.before); });
    bus.publish(ValueChanged{0, 1});
    bus.publish(ValueChanged{1, 5});
    CHECK(sum == 5);
    CHECK(bus.unsubscribe<ValueChanged>(id));
    bus.publish(ValueChanged{5, 9});
    CHECK(sum == 5);
}

TEST_CASE("scoped handle auto-unsubscribes", "[eventbus]")
{
    EventBus bus;
    int count = 0;
    {
        auto h = bus.subscribeScoped<AppStarted>([&](const AppStarted&) { ++count; });
        bus.publish(AppStarted{});
        CHECK(count == 1);
    }
    bus.publish(AppStarted{});
    CHECK(count == 1);
}

TEST_CASE("weak subscription owner bound", "[eventbus]")
{
    EventBus bus;
    int hits = 0;
    auto owner = std::make_shared<int>(42);
    auto h = bus.subscribeScopedWeak<ValueChanged>(owner, [&](const ValueChanged&) { ++hits; });

    bus.publish(ValueChanged{1, 2});
    CHECK(hits == 1);

    owner.reset(); // expire owner
    bus.publish(ValueChanged{2, 3});
    CHECK(hits == 1);
}

TEST_CASE("threaded publishing", "[eventbus][threads]")
{
    EventBus bus;
    std::atomic<int> count{0};
    auto h = bus.subscribeScoped<AudioFrame>([&](const AudioFrame&) { ++count; });

    constexpr int threads = 8;
    constexpr int per = 1000;

    std::vector<std::thread> ts;
    ts.reserve(threads);
    for (int t = 0; t < threads; ++t)
    {
        ts.emplace_back(
            [&]
            {
                for (int i = 0; i < per; ++i)
                    bus.publish(AudioFrame{});
            });
    }
    for (auto& th : ts)
        th.join();

    CHECK(count == threads * per);
}
