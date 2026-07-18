/**
 ****************************************************************************************
 * @file   CoalescingPolicy.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <functional>
#include <typeindex>
#include <unordered_map>

#include "ICommand.hpp"

namespace viz::core
{

    /**
     * @brief Global coalescing rules per command type.
     * set<C>(bool(const C&, const C&)) registers a merge rule for type C.
     * tryMerge(last, next) returns true if they should coalesce into one step.
     */
    class CoalescingPolicy
    {
      public:
        CoalescingPolicy() = default;

        template<class C> void set(std::function<bool(const C&, const C&)> fn);

        bool tryMerge(const ICommand& last, const ICommand& next) const;

      private:
        using ErasedRule = std::function<bool(const ICommand&, const ICommand&)>;
        std::unordered_map<std::type_index, ErasedRule> m_rules;
    };

} // namespace viz::core

#include "CoalescingPolicy.tpp"
