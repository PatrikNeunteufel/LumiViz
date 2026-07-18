/**
 ****************************************************************************************
 * @file   ServiceScope.tpp
 * @brief  Template helpers for ServiceScope.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include "ServiceScope.hpp"

namespace viz::core
{

    template<typename T> std::shared_ptr<T> ServiceScope::get()
    {
        return resolveImpl(typeid(T)).template cast<T>();
    }

    template<typename T> std::shared_ptr<T> ServiceScope::tryGet() noexcept
    {
        try
        {
            return get<T>();
        }
        catch (...)
        {
            return {};
        }
    }

    inline size_t ServiceScope::scopedCount() const noexcept
    {
        return m_scopedCache.size();
    }

} // namespace viz::core
