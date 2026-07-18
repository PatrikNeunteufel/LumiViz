/**
 ****************************************************************************************
 * @file   BaseRegistry.hpp
 * @brief  Lightweight, type-safe registry (key -> value) with value semantics and no exceptions.
 *
 * @author Patrik Neunteufel
 * @date   September 2025
 ****************************************************************************************
 */
#pragma once
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "common/Attributes.hpp"

namespace viz::core
{

    template<class Key, class Value> class BaseRegistry
    {
      public:
        using key_type = Key;
        using mapped_type = Value;
        using Entry = Value; // alias
        using entry_type = Value;
        using storage_t = std::unordered_map<key_type, mapped_type>;

        BaseRegistry() = default;

        VIZ_NODISCARD bool empty() const noexcept
        {
            return m_items.empty();
        }
        VIZ_NODISCARD std::size_t size() const noexcept
        {
            return m_items.size();
        }
        void clear() noexcept
        {
            m_items.clear();
        }

        VIZ_NODISCARD bool contains(const key_type& k) const noexcept
        {
            return m_items.find(k) != m_items.end();
        }
        VIZ_NODISCARD bool has(const key_type& k) const noexcept // alias to contains
        {
            return contains(k);
        }
        // insert: true, wenn NEU; false, wenn bereits vorhanden (auch wenn replace==true und ersetzt wird)
        VIZ_NODISCARD bool insert(const key_type& k, const mapped_type& v, bool replace = false)
        {
            auto it = m_items.find(k);
            if (it == m_items.end())
            {
                m_items.emplace(k, v);
                return true; // neu
            }
            if (replace)
            {
                it->second = v; // ersetzt, aber Rückgabewert bleibt false (war nicht neu)
            }
            return false;
        }

        // „ensure“ – lege an, wenn nicht vorhanden (per Value-Factory)
        template<class MakeValue>
        VIZ_NODISCARD bool ensure(const Key& k, MakeValue&& makeValue)
        {
            if (contains(k))
                return false;
            return insert(k, std::forward<MakeValue>(makeValue)());
        }

        // emplace-Variante mit perfektem Forwarding
        template<class... Args>
        VIZ_NODISCARD bool emplace(const key_type& k, bool replace, Args&&... args)
        {
            auto it = m_items.find(k);
            if (it == m_items.end())
            {
                m_items.emplace(k, mapped_type(std::forward<Args>(args)...));
                return true; // neu
            }
            if (replace)
            {
                it->second = mapped_type(std::forward<Args>(args)...); // ersetzt
            }
            return false;
        }

        // „create“ – rufe Entry.factory auf, wenn vorhanden
        template<class... Args>
        auto create(const Key& k, Args&&... args)
            -> std::optional<decltype(std::declval<Value>().factory(std::forward<Args>(args)...))>
        {
            if (auto e = getPtr(k))
            {
                if (e->factory)
                {
                    return e->factory(std::forward<Args>(args)...);
                }
            }
            return std::nullopt;
        }

        // Zeiger (nullptr, wenn nicht vorhanden)
        VIZ_NODISCARD const mapped_type* getPtr(const key_type& k) const noexcept
        {
            auto it = m_items.find(k);
            return (it == m_items.end()) ? nullptr : &it->second;
        }

        VIZ_NODISCARD mapped_type* getPtr(const key_type& k) noexcept
        {
            auto it = m_items.find(k);
            return (it == m_items.end()) ? nullptr : &it->second;
        }

        // Kopie als optional
        VIZ_NODISCARD std::optional<mapped_type> getCopy(const key_type& k) const
        {
            auto it = m_items.find(k);
            if (it == m_items.end())
                return std::nullopt;
            return it->second;
        }

        // true, wenn tatsächlich ein Element gelöscht wurde
        VIZ_NODISCARD bool erase(const key_type& k)
        {
            return m_items.erase(k) > 0;
        }

        // Iteration über alle Einträge
        template<class Fn> void forEach(Fn&& fn) const
        {
            for (const auto& [k, v] : m_items)
            {
                fn(k, v);
            }
        }

      private:
        storage_t m_items;
    };

} // namespace viz::core
