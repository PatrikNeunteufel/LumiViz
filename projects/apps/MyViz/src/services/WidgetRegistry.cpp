/**
 ****************************************************************************************
 * @file   WidgetRegistry.cpp
 * @brief  WidgetRegistry implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 ****************************************************************************************
 */

#include "services/WidgetRegistry.hpp"

#include <algorithm>
#include <set>

// =============================================================================
// External Init Function (implemented in WidgetAutoReg.cpp)
// =============================================================================

/**
 * @brief Register application-specific default widgets
 * 
 * This function is implemented in WidgetAutoReg.cpp and called automatically
 * on first access to WidgetRegistry::instance().
 * 
 * By declaring it here and implementing it elsewhere, we ensure the linker
 * includes WidgetAutoReg.cpp even in static libraries.
 */
extern void initWidgetDefaults(WidgetRegistry& registry);

// =============================================================================
// Singleton
// =============================================================================

WidgetRegistry& WidgetRegistry::instance()
{
    static WidgetRegistry registry;
    static bool initialized = false;
    
    if (!initialized)
    {
        initialized = true;
        initWidgetDefaults(registry);
    }
    
    return registry;
}

// =============================================================================
// Registration
// =============================================================================

void WidgetRegistry::registerWidget(const WidgetDescriptor& descriptor,
                                     Factory factory,
                                     bool overwrite)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!overwrite && m_descriptors.find(descriptor.id) != m_descriptors.end())
    {
        return;
    }

    m_descriptors[descriptor.id] = descriptor;
    m_factories[descriptor.id] = std::move(factory);
}

// =============================================================================
// Query
// =============================================================================

bool WidgetRegistry::has(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_descriptors.find(id) != m_descriptors.end();
}

std::unique_ptr<QWidget> WidgetRegistry::create(const std::string& id,
                                                 ServiceContainer& services,
                                                 QWidget* parent) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_factories.find(id);
    if (it == m_factories.end())
    {
        return nullptr;
    }

    return it->second(services, parent);
}

std::vector<WidgetDescriptor> WidgetRegistry::descriptors() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<WidgetDescriptor> result;
    result.reserve(m_descriptors.size());

    for (const auto& [id, desc] : m_descriptors)
    {
        result.push_back(desc);
    }

    // Sort by category then order
    std::sort(result.begin(), result.end(),
        [](const WidgetDescriptor& a, const WidgetDescriptor& b) {
            if (a.category != b.category)
            {
                return a.category < b.category;
            }
            return a.order < b.order;
        });

    return result;
}

const WidgetDescriptor* WidgetRegistry::descriptor(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_descriptors.find(id);
    if (it == m_descriptors.end())
    {
        return nullptr;
    }

    return &it->second;
}

std::vector<WidgetDescriptor> WidgetRegistry::widgetsInCategory(const std::string& category) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<WidgetDescriptor> result;

    for (const auto& [id, desc] : m_descriptors)
    {
        if (desc.category == category)
        {
            result.push_back(desc);
        }
    }

    // Sort by order
    std::sort(result.begin(), result.end(),
        [](const WidgetDescriptor& a, const WidgetDescriptor& b) {
            return a.order < b.order;
        });

    return result;
}

std::vector<std::string> WidgetRegistry::categories() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::set<std::string> categorySet;
    for (const auto& [id, desc] : m_descriptors)
    {
        if (!desc.category.empty())
        {
            categorySet.insert(desc.category);
        }
    }

    return {categorySet.begin(), categorySet.end()};
}
