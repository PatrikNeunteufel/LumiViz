/**
 ****************************************************************************************
 * @file   test_EventBus.cpp
 * @brief  Unit-Tests für den EventBus (Publish/Subscribe)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 *
 * @note   Portiert/adaptiert aus harvest/tests (NewViz2025, Catch2) auf die
 *         MyViz-API (subscribe mit Prioritaet, publish, queue/dispatchQueued,
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

    // API-Wart (Phase-4-Notiz): Handler erhalten const&, consume() ist aber
    // nicht-const -> Konsumieren erfordert aktuell einen const_cast.
    bus.subscribe<ValueEvent>([](const ValueEvent& e) {
        const_cast<ValueEvent&>(e).consume();
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
