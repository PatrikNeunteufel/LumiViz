/**
 ****************************************************************************************
 * @file   PanelRegistry.cpp
 * @brief  PanelRegistry implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 ****************************************************************************************
 */

#include "services/PanelRegistry.hpp"

#include <algorithm>

// =============================================================================
// External Init Function (implemented in PanelAutoReg.cpp)
// =============================================================================

/**
 * @brief Register application-specific default panels
 * 
 * This function is implemented in PanelAutoReg.cpp and called automatically
 * on first access to PanelRegistry::instance().
 * 
 * By declaring it here and implementing it elsewhere, we ensure the linker
 * includes PanelAutoReg.cpp even in static libraries.
 */
extern void initPanelDefaults(PanelRegistry& registry);

// =============================================================================
// Singleton
// =============================================================================

PanelRegistry& PanelRegistry::instance()
{
    static PanelRegistry registry;
    static bool initialized = false;
    
    if (!initialized)
    {
        initialized = true;
        initPanelDefaults(registry);
    }
    
    return registry;
}

// =============================================================================
// Registration
// =============================================================================

void PanelRegistry::registerPanel(const PanelDescriptor& descriptor,
                                   Factory factory,
                                   bool overwrite)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check for duplicate
    if (!overwrite && m_descriptors.find(descriptor.id) != m_descriptors.end())
    {
        // Already registered, skip
        return;
    }

    m_descriptors[descriptor.id] = descriptor;
    m_factories[descriptor.id] = std::move(factory);
}

// =============================================================================
// Query
// =============================================================================

bool PanelRegistry::has(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_descriptors.find(id) != m_descriptors.end();
}

std::unique_ptr<QWidget> PanelRegistry::create(const std::string& id,
                                                ServiceContainer& services) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_factories.find(id);
    if (it == m_factories.end())
    {
        return nullptr;
    }

    return it->second(services);
}

std::vector<PanelDescriptor> PanelRegistry::descriptors() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<PanelDescriptor> result;
    result.reserve(m_descriptors.size());

    for (const auto& [id, desc] : m_descriptors)
    {
        result.push_back(desc);
    }

    // Sort by order
    std::sort(result.begin(), result.end(),
        [](const PanelDescriptor& a, const PanelDescriptor& b) {
            return a.order < b.order;
        });

    return result;
}

const PanelDescriptor* PanelRegistry::descriptor(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_descriptors.find(id);
    if (it == m_descriptors.end())
    {
        return nullptr;
    }

    return &it->second;
}
