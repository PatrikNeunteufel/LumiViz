/**
 ****************************************************************************************
 * @file   IServiceResolver.tpp
 * @brief  Template helpers for IServiceResolver.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once

#include "IServiceResolver.hpp"

namespace viz::core
{
    template<typename T> std::shared_ptr<T> IServiceResolver::get()
    {
        return resolveImpl(typeid(T)).template cast<T>();
    }

    template<typename T> std::shared_ptr<T> IServiceResolver::tryGet() noexcept
    {
        try
        {
            return resolveImpl(typeid(T)).template cast<T>();
        }
        catch (...)
        {
            return {};
        }
    }

} // namespace viz::core
