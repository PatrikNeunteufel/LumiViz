/**
 ****************************************************************************************
 * @file   ServiceLifetime.hpp
 * @brief  Lifetime kinds for services in the DI container.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

namespace viz::core
{

    enum class ServiceLifetime
    {
        Singleton, ///< One global instance cached in the root container
        Scoped,    ///< One instance per ServiceScope
        Transient  ///< New instance on each resolution
    };

} // namespace viz::core
