/**
 ****************************************************************************************
 * @file   Event.hpp
 * @brief  Base class for all events in the EventBus system
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include <chrono>
#include <string>

/**
 * @class Event
 * @brief Abstract base class for all events
 *
 * All events must derive from this class to be usable with the EventBus.
 *
 * @code
 * struct AudioDataEvent : public Event {
 *     const float* spectrum;
 *     int size;
 *     
 *     AudioDataEvent(const float* s, int n) 
 *         : spectrum(s), size(n) {}
 * };
 * @endcode
 */
class Event
{
public:
    using TimePoint = std::chrono::steady_clock::time_point;

    Event() : m_timestamp(std::chrono::steady_clock::now()) {}
    virtual ~Event() = default;

    /**
     * @brief Get event type name (for debugging/logging)
     * @return Type name string
     */
    [[nodiscard]] virtual const char* typeName() const = 0;

    /**
     * @brief Get event creation timestamp
     * @return Time point when event was created
     */
    [[nodiscard]] TimePoint timestamp() const { return m_timestamp; }

    /**
     * @brief Check if event propagation should stop
     * @return true if event should not propagate further
     */
    [[nodiscard]] bool isConsumed() const { return m_consumed; }

    /**
     * @brief Mark event as consumed (stops propagation)
     *
     * Const on purpose: handlers receive `const Event&`; consumption is a
     * dispatch-control flag, not a payload mutation (hence `mutable`).
     */
    void consume() const { m_consumed = true; }

protected:
    TimePoint m_timestamp;
    mutable bool m_consumed = false;
};

/**
 * @brief Helper macro to implement typeName()
 *
 * @code
 * struct MyEvent : public Event {
 *     EVENT_TYPE_NAME("MyEvent")
 * };
 * @endcode
 */
#define EVENT_TYPE_NAME(name) \
    const char* typeName() const override { return name; }
