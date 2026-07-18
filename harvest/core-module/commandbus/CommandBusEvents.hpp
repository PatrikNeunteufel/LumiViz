/**
 ****************************************************************************************
 * @file   CommandBusEvents.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <cstdint>
#include <string>

namespace viz::core
{

    /** Emitted before a command executes (also on redo). */
    struct CommandWillExecute
    {
        std::string name;
    };

    /** Emitted after a command executed. */
    struct CommandDidExecute
    {
        std::string name;
        bool ok{false};
        std::int64_t duration_ms{0};
    };

    /** Emitted after undo. */
    struct CommandUndone
    {
        std::string name;
        bool ok{false};
        std::int64_t duration_ms{0};
    };

    /** Emitted after redo. */
    struct CommandRedone
    {
        std::string name;
        bool ok{false};
        std::int64_t duration_ms{0};
    };

    /** Emitted whenever undo/redo stacks change. */
    struct CommandStacksChanged
    {
        bool canUndo{false};
        bool canRedo{false};
        std::string undoLabel; ///< empty if none
        std::string redoLabel; ///< empty if none
    };

} // namespace viz::core
