/**
 ****************************************************************************************
 * @file   CommandKey.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <typeinfo>

namespace viz::core
{
    struct CommandKey
    {
        const void* id{};
    };
    template<typename C> inline CommandKey keyOf() noexcept
    {
        // stabil: adresse eines inline-unique symbols je T (wie beim EventBus)
        static int unique;
        return CommandKey{&unique};
    }
} // namespace viz::core
