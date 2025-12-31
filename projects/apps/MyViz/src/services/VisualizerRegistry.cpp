/**
 ****************************************************************************************
 * @file   VisualizerRegistry.cpp
 * @brief  VisualizerRegistry implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 ****************************************************************************************
 */

#include "services/VisualizerRegistry.hpp"
#include "visualizers/IVisualizer.hpp"

#include <algorithm>
#include <set>

// =============================================================================
// External Init Function (implemented in VisualizerAutoReg.cpp)
// =============================================================================

/**
 * @brief Register application-specific default visualizers
 * 
 * This function is implemented in VisualizerAutoReg.cpp and called automatically
 * on first access to VisualizerRegistry::instance().
 * 
 * By declaring it here and implementing it elsewhere, we ensure the linker
 * includes VisualizerAutoReg.cpp even in static libraries.
 */
extern void initVisualizerDefaults(VisualizerRegistry& registry);

// =============================================================================
// Singleton
// =============================================================================

VisualizerRegistry& VisualizerRegistry::instance()
{
    static VisualizerRegistry registry;
    static bool initialized = false;
    
    if (!initialized)
    {
        initialized = true;
        initVisualizerDefaults(registry);
    }
    
    return registry;
}

// =============================================================================
// Registration
// =============================================================================

void VisualizerRegistry::registerVisualizer(const VisualizerDescriptor& descriptor,
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

bool VisualizerRegistry::has(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_descriptors.find(id) != m_descriptors.end();
}

std::unique_ptr<IVisualizer> VisualizerRegistry::create(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_factories.find(id);
    if (it == m_factories.end())
    {
        return nullptr;
    }

    return it->second();
}

std::vector<VisualizerDescriptor> VisualizerRegistry::descriptors() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<VisualizerDescriptor> result;
    result.reserve(m_descriptors.size());

    for (const auto& [id, desc] : m_descriptors)
    {
        result.push_back(desc);
    }

    // Sort by category then order
    std::sort(result.begin(), result.end(),
        [](const VisualizerDescriptor& a, const VisualizerDescriptor& b) {
            if (a.category != b.category)
            {
                return a.category < b.category;
            }
            return a.order < b.order;
        });

    return result;
}

const VisualizerDescriptor* VisualizerRegistry::descriptor(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_descriptors.find(id);
    if (it == m_descriptors.end())
    {
        return nullptr;
    }

    return &it->second;
}

std::vector<VisualizerDescriptor> VisualizerRegistry::visualizersInCategory(
    const std::string& category) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<VisualizerDescriptor> result;

    for (const auto& [id, desc] : m_descriptors)
    {
        if (desc.category == category)
        {
            result.push_back(desc);
        }
    }

    // Sort by order
    std::sort(result.begin(), result.end(),
        [](const VisualizerDescriptor& a, const VisualizerDescriptor& b) {
            return a.order < b.order;
        });

    return result;
}

std::vector<std::string> VisualizerRegistry::categories() const
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

std::vector<std::string> VisualizerRegistry::audioVisualizers() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<std::string> result;

    for (const auto& [id, desc] : m_descriptors)
    {
        if (desc.usesAudio)
        {
            result.push_back(id);
        }
    }

    return result;
}
