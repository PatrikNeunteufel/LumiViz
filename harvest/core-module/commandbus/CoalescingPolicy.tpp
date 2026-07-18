/**
 ****************************************************************************************
 * @file   CoalescingPolicy.tpp
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

    template<class C> void CoalescingPolicy::set(std::function<bool(const C&, const C&)> fn)
    {
        m_rules[std::type_index(typeid(C))] = [f = std::move(fn)](const ICommand& a, const ICommand& b) -> bool
        {
            auto pa = dynamic_cast<const C*>(&a);
            auto pb = dynamic_cast<const C*>(&b);
            return (pa && pb) ? f(*pa, *pb) : false;
        };
    }

    inline bool CoalescingPolicy::tryMerge(const ICommand& last, const ICommand& next) const
    {
        auto it = m_rules.find(std::type_index(typeid(last)));
        return (it != m_rules.end()) ? it->second(last, next) : false;
    }

} // namespace viz::core
