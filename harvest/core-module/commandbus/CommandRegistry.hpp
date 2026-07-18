/**
 ****************************************************************************************
 * @file   CommandRegistry.hpp
 * @brief  
 * 
 * @author Patrik Neunteufel
 * @date   September 2025
  ****************************************************************************************
 */
#pragma once
#include <shared_mutex>
#include <unordered_map>
#include <functional>
#include <type_traits>          // <-- neu: für Constraints

#include "CommandKey.hpp"

namespace viz::core
{

    // Handler-Typ nach Erasure: nimmt void* (zeigt auf C), liefert Undo-Memento
    using ErasedDo = std::function<std::function<void()>(const void*)>;

    class CommandRegistry
    {
      public:
        // ### NEU: Funktor-Typ F wird deduziert und constrained
        template<typename C, typename F>
            requires std::is_invocable_r_v<std::function<void()>, F, const C&>
        void registerHandler(F&& fn);

        template<typename C> bool hasHandler() const noexcept;

        template<typename C> void unregisterHandler();

        // lookup (erased)
        ErasedDo find(CommandKey k) const;

      private:
        mutable std::shared_mutex m_mtx;
        std::unordered_map<const void*, ErasedDo> m_handlers;
    };

} // namespace viz::core

#include "CommandRegistry.tpp"
