/**
 ****************************************************************************************
 * @file   test_EventBus.cpp
 * @brief  Unit-Tests für den EventBus (Publish/Subscribe)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 *
 * @note   Portiert/adaptiert aus harvest/tests (NewViz2025, Catch2) auf die
 *         LumiViz-API (subscribe mit Prioritaet, publish, queue/dispatchQueued,
 *         consume, subscriberCount, clear).
 ****************************************************************************************
 */

#include <doctest.h>

#include "services/EventBus.hpp"

#include <string>
#include <vector>

// =============================================================================
// Test-Events
// =============================================================================

namespace
{

struct ValueEvent : public Event
{
    EVENT_TYPE_NAME("ValueEvent")
    explicit ValueEvent(int v) : value(v) {}
    int value;
};

struct OtherEvent : public Event
{
    EVENT_TYPE_NAME("OtherEvent")
};

} // namespace

// =============================================================================
// Publish / Subscribe
// =============================================================================

TEST_CASE("EventBus: publish erreicht den Subscriber mit den Event-Daten")
{
    EventBus bus;
    int received = 0;

    bus.subscribe<ValueEvent>([&](const ValueEvent& e) { received = e.value; });
    bus.publish(ValueEvent{42});

    CHECK(received == 42);
}

TEST_CASE("EventBus: mehrere Subscriber werden alle bedient")
{
    EventBus bus;
    int calls = 0;

    bus.subscribe<ValueEvent>([&](const ValueEvent&) { ++calls; });
    bus.subscribe<ValueEvent>([&](const ValueEvent&) { ++calls; });
    bus.subscribe<ValueEvent>([&](const ValueEvent&) { ++calls; });
    bus.publish(ValueEvent{1});

    CHECK(calls == 3);
}

TEST_CASE("EventBus: Events erreichen nur Subscriber ihres Typs")
{
    EventBus bus;
    int valueCalls = 0;
    int otherCalls = 0;

    bus.subscribe<ValueEvent>([&](const ValueEvent&) { ++valueCalls; });
    bus.subscribe<OtherEvent>([&](const OtherEvent&) { ++otherCalls; });

    bus.publish(ValueEvent{1});
    CHECK(valueCalls == 1);
    CHECK(otherCalls == 0);

    bus.publish(OtherEvent{});
    CHECK(otherCalls == 1);
}

TEST_CASE("EventBus: Prioritaet — niedrigere Werte werden zuerst bedient")
{
    EventBus bus;
    std::vector<std::string> order;

    bus.subscribe<ValueEvent>([&](const ValueEvent&) { order.push_back("spaet"); }, 10);
    bus.subscribe<ValueEvent>([&](const ValueEvent&) { order.push_back("frueh"); }, -10);
    bus.subscribe<ValueEvent>([&](const ValueEvent&) { order.push_back("mitte"); }, 0);

    bus.publish(ValueEvent{1});

    REQUIRE(order.size() == 3);
    CHECK(order[0] == "frueh");
    CHECK(order[1] == "mitte");
    CHECK(order[2] == "spaet");
}

TEST_CASE("EventBus: consume() stoppt die Weitergabe an spaetere Subscriber")
{
    EventBus bus;
    int afterConsumeCalls = 0;

    // consume() ist const (Dispatch-Steuerflag, seit Phase 4 Schritt 1) —
    // Konsumieren direkt aus dem const&-Handler, ohne const_cast.
    bus.subscribe<ValueEvent>([](const ValueEvent& e) {
        e.consume();
    }, 0);
    bus.subscribe<ValueEvent>([&](const ValueEvent&) { ++afterConsumeCalls; }, 1);

    bus.publish(ValueEvent{1});

    CHECK(afterConsumeCalls == 0);
}

// =============================================================================
// Unsubscribe
// =============================================================================

TEST_CASE("EventBus: unsubscribe beendet die Zustellung")
{
    EventBus bus;
    int calls = 0;

    auto id = bus.subscribe<ValueEvent>([&](const ValueEvent&) { ++calls; });
    bus.publish(ValueEvent{1});
    CHECK(calls == 1);

    bus.unsubscribe(id);
    bus.publish(ValueEvent{1});
    CHECK(calls == 1); // unveraendert
}

TEST_CASE("EventBus: subscriberCount und clear")
{
    EventBus bus;
    CHECK(bus.subscriberCount<ValueEvent>() == 0);

    bus.subscribe<ValueEvent>([](const ValueEvent&) {});
    bus.subscribe<ValueEvent>([](const ValueEvent&) {});
    CHECK(bus.subscriberCount<ValueEvent>() == 2);

    bus.clear();
    CHECK(bus.subscriberCount<ValueEvent>() == 0);
}

// =============================================================================
// Queue / dispatchQueued
// =============================================================================

TEST_CASE("EventBus: queue stellt erst bei dispatchQueued zu")
{
    EventBus bus;
    std::vector<int> received;

    bus.subscribe<ValueEvent>([&](const ValueEvent& e) { received.push_back(e.value); });

    bus.queue(ValueEvent{1});
    bus.queue(ValueEvent{2});
    CHECK(received.empty()); // noch nichts zugestellt

    bus.dispatchQueued();
    REQUIRE(received.size() == 2);
    CHECK(received[0] == 1);
    CHECK(received[1] == 2);

    bus.dispatchQueued(); // Queue ist geleert — keine Doppelzustellung
    CHECK(received.size() == 2);
}

// =============================================================================
// RAII-SubscriberHandle (subscribeScoped) - Phase 4 Schritt 1
// =============================================================================

TEST_CASE("EventBus: SubscriberHandle unsubscribed automatisch bei Zerstoerung")
{
    EventBus bus;
    int calls = 0;

    {
        auto handle = bus.subscribeScoped<ValueEvent>(
            [&](const ValueEvent&) { ++calls; });
        CHECK(static_cast<bool>(handle));
        CHECK(bus.subscriberCount<ValueEvent>() == 1);

        bus.publish(ValueEvent{1});
        CHECK(calls == 1);
    } // handle stirbt -> Abo weg

    CHECK(bus.subscriberCount<ValueEvent>() == 0);
    bus.publish(ValueEvent{2});
    CHECK(calls == 1); // kein weiterer Aufruf
}

TEST_CASE("EventBus: SubscriberHandle reset() ist sofort wirksam und idempotent")
{
    EventBus bus;
    int calls = 0;

    auto handle = bus.subscribeScoped<ValueEvent>(
        [&](const ValueEvent&) { ++calls; });

    handle.reset();
    CHECK_FALSE(static_cast<bool>(handle));
    handle.reset(); // idempotent

    bus.publish(ValueEvent{1});
    CHECK(calls == 0);
}

TEST_CASE("EventBus: SubscriberHandle ist movable, Abo wandert mit")
{
    EventBus bus;
    int calls = 0;

    auto a = bus.subscribeScoped<ValueEvent>(
        [&](const ValueEvent&) { ++calls; });
    auto b = std::move(a);
    CHECK_FALSE(static_cast<bool>(a));
    CHECK(static_cast<bool>(b));

    bus.publish(ValueEvent{1});
    CHECK(calls == 1);

    b.reset();
    bus.publish(ValueEvent{2});
    CHECK(calls == 1);
}

// =============================================================================
// Weak-Abos (subscribeWeak) - Phase 4 Schritt 1
// =============================================================================

namespace
{
    struct WeakOwner { int lastValue = 0; };
} // namespace

TEST_CASE("EventBus: subscribeWeak feuert nur solange der Owner lebt")
{
    EventBus bus;
    auto owner = std::make_shared<WeakOwner>();

    bus.subscribeWeak<ValueEvent>(owner,
        std::function<void(const ValueEvent&)>(
            [w = std::weak_ptr<WeakOwner>(owner)](const ValueEvent& e) {
                if (auto o = w.lock()) { o->lastValue = e.value; }
            }));

    bus.publish(ValueEvent{42});
    CHECK(owner->lastValue == 42);
    CHECK(bus.subscriberCount<ValueEvent>() == 1);

    owner.reset(); // Owner stirbt

    bus.publish(ValueEvent{7});  // Handler darf nicht mehr feuern; Abo wird gepurgt
    CHECK(bus.subscriberCount<ValueEvent>() == 0);
}

TEST_CASE("EventBus: subscribeScopedWeak - Handle UND Owner begrenzen die Lebensdauer")
{
    EventBus bus;
    int calls = 0;
    auto owner = std::make_shared<WeakOwner>();

    auto handle = bus.subscribeScopedWeak<ValueEvent>(owner,
        std::function<void(const ValueEvent&)>(
            [&calls](const ValueEvent&) { ++calls; }));

    bus.publish(ValueEvent{1});
    CHECK(calls == 1);

    owner.reset(); // Owner weg -> Handler inaktiv, naechster publish purgt
    bus.publish(ValueEvent{2});
    CHECK(calls == 1);
    CHECK(bus.subscriberCount<ValueEvent>() == 0);

    handle.reset(); // Handle-Reset nach Purge bleibt harmlos (idempotent)
}
