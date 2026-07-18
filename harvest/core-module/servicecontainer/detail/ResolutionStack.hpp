/**
 ****************************************************************************************
 * @file   ResolutionStack.hpp
 * @brief  Internal thread-local stack for cycle detection in DI resolution.
 * @note    Internal header: do not include outside servicecontainer subsystem.
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <typeindex>
#include <vector>

namespace viz::core::detail
{

    /**
     * @brief Returns the thread-local stack of currently resolving types.
     *        Used to detect cyclic dependencies (A->B->A).
     */
    inline std::vector<std::type_index>& resolvingStack() noexcept
    {
        static thread_local std::vector<std::type_index> tls;
        return tls;
    }

    inline bool contains(const std::vector<std::type_index>& stk, const std::type_index& idx) noexcept
    {
        for (const auto& t : stk)
            if (t == idx)
                return true;
        return false;
    }

} // namespace viz::core::detail
