/**
 ****************************************************************************************
 * @file   CommandEvents.hpp
 * @brief  Events published by the CommandBus (undo/redo history)
 *
 * @author Patrik Neunteufel
 * @date   Juli 2026
 * @version 1.0.0
 ****************************************************************************************
 */

#pragma once

#include "Event.hpp"

#include <string>

/**
 * @struct CommandHistoryChangedEvent
 * @brief Published after every history change (execute/undo/redo/clear)
 *
 * UI uses this to refresh undo/redo affordances and — on Undone/Redone —
 * to re-sync editors whose values were changed behind their back.
 */
struct CommandHistoryChangedEvent : public Event
{
    EVENT_TYPE_NAME("CommandHistoryChangedEvent")

    enum class Cause
    {
        Executed,  ///< A new command was executed (came from the UI itself)
        Undone,    ///< undo() reverted a command — editors must re-sync
        Redone,    ///< redo() re-applied a command — editors must re-sync
        Cleared    ///< History was dropped
    };

    Cause cause = Cause::Executed;
    bool canUndo = false;
    bool canRedo = false;
    std::string undoDescription;
    std::string redoDescription;
};
