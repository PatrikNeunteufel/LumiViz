/**
 ****************************************************************************************
 * @file   MenuRegistry.cpp
 * @brief  MenuRegistry implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 ****************************************************************************************
 */

#include "services/MenuRegistry.hpp"

#include <algorithm>

// =============================================================================
// Singleton
// =============================================================================

MenuRegistry& MenuRegistry::instance()
{
    static MenuRegistry registry;
    return registry;
}

const std::string& MenuRegistry::rootId()
{
    static const std::string root = "toplevel";
    return root;
}

MenuRegistry::MenuRegistry()
{
    // Register the root container
    m_containers["toplevel"] = MenuContainerDesc{{"toplevel", "", 0}, "MenuBar"};
}

// =============================================================================
// Registration
// =============================================================================

void MenuRegistry::registerContainer(const MenuContainerDesc& desc, bool overwrite)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!overwrite && m_containers.find(desc.id) != m_containers.end())
    {
        return;
    }

    m_containers[desc.id] = desc;
    m_containerChildren[desc.parentId].push_back(desc.id);
    ++m_version;
}

void MenuRegistry::registerGroup(const MenuGroupDesc& desc, bool overwrite)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!overwrite && m_groups.find(desc.id) != m_groups.end())
    {
        return;
    }

    m_groups[desc.id] = desc;
    m_groupChildren[desc.parentId].push_back(desc.id);
    ++m_version;
}

void MenuRegistry::registerItem(const MenuItemDesc& desc, bool overwrite)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!overwrite && m_items.find(desc.id) != m_items.end())
    {
        return;
    }

    m_items[desc.id] = desc;
    m_itemChildren[desc.parentId].push_back(desc.id);
    ++m_version;
}

// =============================================================================
// Query
// =============================================================================

bool MenuRegistry::hasContainer(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_containers.find(id) != m_containers.end();
}

bool MenuRegistry::hasGroup(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_groups.find(id) != m_groups.end();
}

bool MenuRegistry::hasItem(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_items.find(id) != m_items.end();
}

const MenuContainerDesc* MenuRegistry::container(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_containers.find(id);
    if (it == m_containers.end())
    {
        return nullptr;
    }
    return &it->second;
}

const MenuGroupDesc* MenuRegistry::group(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_groups.find(id);
    if (it == m_groups.end())
    {
        return nullptr;
    }
    return &it->second;
}

const MenuItemDesc* MenuRegistry::item(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_items.find(id);
    if (it == m_items.end())
    {
        return nullptr;
    }
    return &it->second;
}

// =============================================================================
// Children Lookup
// =============================================================================

std::vector<MenuRegistry::NodeRef> MenuRegistry::childrenOf(const std::string& parentId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<NodeRef> result;

    // Collect containers
    auto contIt = m_containerChildren.find(parentId);
    if (contIt != m_containerChildren.end())
    {
        for (const auto& id : contIt->second)
        {
            auto descIt = m_containers.find(id);
            if (descIt != m_containers.end())
            {
                result.push_back({MenuNodeType::Container, id, descIt->second.order});
            }
        }
    }

    // Collect groups
    auto groupIt = m_groupChildren.find(parentId);
    if (groupIt != m_groupChildren.end())
    {
        for (const auto& id : groupIt->second)
        {
            auto descIt = m_groups.find(id);
            if (descIt != m_groups.end())
            {
                result.push_back({MenuNodeType::Group, id, descIt->second.order});
            }
        }
    }

    // Collect items
    auto itemIt = m_itemChildren.find(parentId);
    if (itemIt != m_itemChildren.end())
    {
        for (const auto& id : itemIt->second)
        {
            auto descIt = m_items.find(id);
            if (descIt != m_items.end())
            {
                result.push_back({MenuNodeType::Item, id, descIt->second.order});
            }
        }
    }

    // Sort by order
    std::sort(result.begin(), result.end(),
        [](const NodeRef& a, const NodeRef& b) {
            return a.order < b.order;
        });

    return result;
}

std::vector<std::string> MenuRegistry::containersUnder(const std::string& parentId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_containerChildren.find(parentId);
    if (it == m_containerChildren.end())
    {
        return {};
    }

    // Sort by order
    std::vector<std::pair<int, std::string>> ordered;
    for (const auto& id : it->second)
    {
        auto descIt = m_containers.find(id);
        if (descIt != m_containers.end())
        {
            ordered.emplace_back(descIt->second.order, id);
        }
    }

    std::sort(ordered.begin(), ordered.end());

    std::vector<std::string> result;
    result.reserve(ordered.size());
    for (const auto& [order, id] : ordered)
    {
        result.push_back(id);
    }

    return result;
}

std::vector<std::string> MenuRegistry::groupsUnder(const std::string& parentId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_groupChildren.find(parentId);
    if (it == m_groupChildren.end())
    {
        return {};
    }

    // Sort by order
    std::vector<std::pair<int, std::string>> ordered;
    for (const auto& id : it->second)
    {
        auto descIt = m_groups.find(id);
        if (descIt != m_groups.end())
        {
            ordered.emplace_back(descIt->second.order, id);
        }
    }

    std::sort(ordered.begin(), ordered.end());

    std::vector<std::string> result;
    result.reserve(ordered.size());
    for (const auto& [order, id] : ordered)
    {
        result.push_back(id);
    }

    return result;
}

std::vector<std::string> MenuRegistry::itemsUnder(const std::string& parentId) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_itemChildren.find(parentId);
    if (it == m_itemChildren.end())
    {
        return {};
    }

    // Sort by order
    std::vector<std::pair<int, std::string>> ordered;
    for (const auto& id : it->second)
    {
        auto descIt = m_items.find(id);
        if (descIt != m_items.end())
        {
            ordered.emplace_back(descIt->second.order, id);
        }
    }

    std::sort(ordered.begin(), ordered.end());

    std::vector<std::string> result;
    result.reserve(ordered.size());
    for (const auto& [order, id] : ordered)
    {
        result.push_back(id);
    }

    return result;
}

uint64_t MenuRegistry::version() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_version;
}
