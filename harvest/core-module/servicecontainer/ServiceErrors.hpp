/**
 ****************************************************************************************
 * @file   ServiceErrors.hpp
 * @brief  Error types for the DI container.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include <stdexcept>
#include <string>

namespace viz::core
{

    class ServiceError : public std::runtime_error
    {
      public:
        explicit ServiceError(const std::string& msg) : std::runtime_error(msg) {}
    };

    class ServiceCycleError : public ServiceError
    {
      public:
        explicit ServiceCycleError(const std::string& msg) : ServiceError(msg) {}
    };

} // namespace viz::core
