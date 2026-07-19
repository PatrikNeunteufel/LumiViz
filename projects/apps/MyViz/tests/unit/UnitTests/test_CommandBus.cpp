/**
 ****************************************************************************************
 * @file   test_CommandBus.cpp
 * @brief  Unit-Tests fuer den CommandBus (Undo/Redo, Coalescing, History-Limit)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 *
 * @note   Portiert/adaptiert aus harvest/tests (NewViz2025, Catch2) auf die
 *         MyViz-API (execute/undo/redo, Merge-Fenster mit injizierter Clock,
 *         CommandHistoryChangedEvent ueber den EventBus).
 ****************************************************************************************
 */

#include <doctest.h>

#include "services/CommandBus.hpp"
#include "services/EventBus.hpp"
#include "services/events/CommandEvents.hpp"

#include <memory>
#include <string>
#include <vector>

// =============================================================================
// Test-Command: setzt einen int-Wert, undo stellt den alten wieder her
// =============================================================================

namespace
{

class SetValueCommand : public ICommand
{
public:
    SetValueCommand(int& target, int newValue, std::string id = "value")
        : m_target(target), m_old(target), m_new(newValue), m_id(std::move(id))
    {
    }

    bool execute() override
    {
        if (m_target == m_new) return false; // wirkungslos
        m_target = m_new;
        return true;
    }

    void undo() override { m_target = m_old; }

    [[nodiscard]] std::string description() const override { return "Set " + m_id; }

    [[nodiscard]] bool canMergeWith(const ICommand& next) const override
    {
        const auto* other = dynamic_cast<const SetValueCommand*>(&next);
        return other != nullptr && &other->m_target == &m_target && other->m_id == m_id;
    }

    void mergeWith(const ICommand& next) override
    {
        m_new = static_cast<const SetValueCommand&>(next).m_new;
    }

private:
    int& m_target;
    int m_old;
    int m_new;
    std::string m_id;
};

/// Manuell stellbare Clock fuer deterministische Merge-Fenster-Tests
struct FakeClock
{
    std::uint64_t nowMs = 0;
    CommandBus::ClockFn fn()
    {
        return [this]() { return nowMs; };
    }
};

} // namespace

// =============================================================================
// Execute / Undo / Redo
// =============================================================================

TEST_CASE("CommandBus: execute fuehrt aus, undo stellt den alten Wert her, redo wiederholt")
{
    FakeClock clock;
    CommandBus bus(nullptr, 100, 750, clock.fn());
    int value = 1;

    CHECK(bus.execute(std::make_unique<SetValueCommand>(value, 5)));
    CHECK(value == 5);
    CHECK(bus.canUndo());
    CHECK_FALSE(bus.canRedo());

    CHECK(bus.undo());
    CHECK(value == 1);
    CHECK(bus.canRedo());

    CHECK(bus.redo());
    CHECK(value == 5);
    CHECK(bus.canUndo());
    CHECK_FALSE(bus.canRedo());
}

TEST_CASE("CommandBus: wirkungslose Commands landen nicht in der History")
{
    FakeClock clock;
    CommandBus bus(nullptr, 100, 750, clock.fn());
    int value = 5;

    CHECK_FALSE(bus.execute(std::make_unique<SetValueCommand>(value, 5))); // no-op
    CHECK_FALSE(bus.canUndo());
}

TEST_CASE("CommandBus: neues execute invalidiert den Redo-Zweig")
{
    FakeClock clock;
    CommandBus bus(nullptr, 100, 750, clock.fn());
    int value = 0;

    (void)bus.execute(std::make_unique<SetValueCommand>(value, 1, "a"));
    (void)bus.undo();
    CHECK(bus.canRedo());

    clock.nowMs += 10000; // ausserhalb des Merge-Fensters
    (void)bus.execute(std::make_unique<SetValueCommand>(value, 2, "b"));
    CHECK_FALSE(bus.canRedo());
}

TEST_CASE("CommandBus: undo/redo auf leerer History liefern false")
{
    CommandBus bus;
    CHECK_FALSE(bus.undo());
    CHECK_FALSE(bus.redo());
}

// =============================================================================
// Coalescing (Merge-Fenster)
// =============================================================================

TEST_CASE("CommandBus: Aenderungen am selben Ziel im Merge-Fenster = EIN Undo-Schritt")
{
    FakeClock clock;
    CommandBus bus(nullptr, 100, 750, clock.fn());
    int value = 0;

    // Slider-Drag: 0 -> 1 -> 2 -> 3 innerhalb des Fensters
    (void)bus.execute(std::make_unique<SetValueCommand>(value, 1));
    clock.nowMs += 100;
    (void)bus.execute(std::make_unique<SetValueCommand>(value, 2));
    clock.nowMs += 100;
    (void)bus.execute(std::make_unique<SetValueCommand>(value, 3));

    CHECK(value == 3);
    CHECK(bus.undoCount() == 1); // gemergt

    (void)bus.undo();
    CHECK(value == 0); // zurueck zum Wert VOR dem Drag
}

TEST_CASE("CommandBus: ausserhalb des Merge-Fensters entstehen getrennte Undo-Schritte")
{
    FakeClock clock;
    CommandBus bus(nullptr, 100, 750, clock.fn());
    int value = 0;

    (void)bus.execute(std::make_unique<SetValueCommand>(value, 1));
    clock.nowMs += 751; // Fenster (750 ms) verpasst
    (void)bus.execute(std::make_unique<SetValueCommand>(value, 2));

    CHECK(bus.undoCount() == 2);
    (void)bus.undo();
    CHECK(value == 1);
    (void)bus.undo();
    CHECK(value == 0);
}

TEST_CASE("CommandBus: verschiedene Ziele werden nie gemergt")
{
    FakeClock clock;
    CommandBus bus(nullptr, 100, 750, clock.fn());
    int a = 0;
    int b = 0;

    (void)bus.execute(std::make_unique<SetValueCommand>(a, 1, "a"));
    (void)bus.execute(std::make_unique<SetValueCommand>(b, 1, "b"));

    CHECK(bus.undoCount() == 2);
}

// =============================================================================
// History-Limit & clear
// =============================================================================

TEST_CASE("CommandBus: History-Limit verwirft die aeltesten Eintraege")
{
    FakeClock clock;
    CommandBus bus(nullptr, 3, 0, clock.fn()); // Fenster 0 -> kein Merge
    int value = 0;

    for (int i = 1; i <= 5; ++i)
    {
        clock.nowMs += 1000;
        (void)bus.execute(std::make_unique<SetValueCommand>(value, i));
    }

    CHECK(bus.undoCount() == 3);
}

TEST_CASE("CommandBus: clear leert beide Staecke")
{
    CommandBus bus;
    int value = 0;

    (void)bus.execute(std::make_unique<SetValueCommand>(value, 1));
    (void)bus.undo();
    bus.clear();

    CHECK_FALSE(bus.canUndo());
    CHECK_FALSE(bus.canRedo());
}

// =============================================================================
// Events & Beschreibungen
// =============================================================================

TEST_CASE("CommandBus: publiziert CommandHistoryChangedEvent mit korrekter Ursache")
{
    EventBus eventBus;
    FakeClock clock;
    CommandBus bus(&eventBus, 100, 750, clock.fn());
    int value = 0;

    std::vector<CommandHistoryChangedEvent::Cause> causes;
    eventBus.subscribe<CommandHistoryChangedEvent>(
        [&](const CommandHistoryChangedEvent& e) { causes.push_back(e.cause); });

    (void)bus.execute(std::make_unique<SetValueCommand>(value, 1));
    (void)bus.undo();
    (void)bus.redo();
    bus.clear();

    REQUIRE(causes.size() == 4);
    CHECK(causes[0] == CommandHistoryChangedEvent::Cause::Executed);
    CHECK(causes[1] == CommandHistoryChangedEvent::Cause::Undone);
    CHECK(causes[2] == CommandHistoryChangedEvent::Cause::Redone);
    CHECK(causes[3] == CommandHistoryChangedEvent::Cause::Cleared);
}

TEST_CASE("CommandBus: undo-/redoDescription nennen den jeweiligen Command")
{
    FakeClock clock;
    CommandBus bus(nullptr, 100, 0, clock.fn());
    int value = 0;

    (void)bus.execute(std::make_unique<SetValueCommand>(value, 1, "gain"));
    CHECK(bus.undoDescription() == "Set gain");
    CHECK(bus.redoDescription().empty());

    (void)bus.undo();
    CHECK(bus.undoDescription().empty());
    CHECK(bus.redoDescription() == "Set gain");
}
