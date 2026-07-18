/**
 ****************************************************************************************
 * @file   CommandContext.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <cstddef>

namespace viz::core
{

    /**
     * @brief Execution context for commands.
     * Set mainThreadToken once at app init; set currentThreadToken per call.
     * On Windows you can store (void*)(uintptr_t)GetCurrentThreadId().
     */
    struct CommandContext
    {
        void* mainThreadToken{nullptr};    ///< token of the UI/main thread
        void* currentThreadToken{nullptr}; ///< token of the calling thread
    };

} // namespace viz::core
