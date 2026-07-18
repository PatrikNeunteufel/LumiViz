/**
 ****************************************************************************************
 * @file   CommandDispatcher.cpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#include "CommandDispatcher.hpp"

namespace viz::core
{
    std::string CommandDispatcher::defaultLabel(const char* name)
    {
        return name ? std::string{name} : std::string{};
    }
} // namespace viz::core

