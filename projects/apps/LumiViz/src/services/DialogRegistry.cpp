/**
 ****************************************************************************************
 * @file   DialogRegistry.cpp
 * @brief  DialogRegistry implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 2.0.0
 ****************************************************************************************
 */

#include "services/DialogRegistry.hpp"

#include <QDialog>

#include <algorithm>

// =============================================================================
// External Init Function (implemented in DialogAutoReg.cpp)
// =============================================================================

/**
 * @brief Register application-specific default dialogs
 * 
 * This function is implemented in DialogAutoReg.cpp and called automatically
 * on first access to DialogRegistry::instance().
 * 
 * By declaring it here and implementing it elsewhere, we ensure the linker
 * includes DialogAutoReg.cpp even in static libraries.
 */
extern void initDialogDefaults(DialogRegistry& registry);

// =============================================================================
// Singleton
// =============================================================================

DialogRegistry& DialogRegistry::instance()
{
    static DialogRegistry registry;
    static bool initialized = false;
    
    if (!initialized)
    {
        initialized = true;
        initDialogDefaults(registry);
    }
    
    return registry;
}

// =============================================================================
// Registration
// =============================================================================

void DialogRegistry::registerDialog(const DialogDescriptor& descriptor,
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

bool DialogRegistry::has(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_descriptors.find(id) != m_descriptors.end();
}

std::unique_ptr<QDialog> DialogRegistry::create(const std::string& id,
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

std::vector<DialogDescriptor> DialogRegistry::descriptors() const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<DialogDescriptor> result;
    result.reserve(m_descriptors.size());

    for (const auto& [id, desc] : m_descriptors)
    {
        result.push_back(desc);
    }

    // Sort by order
    std::sort(result.begin(), result.end(),
        [](const DialogDescriptor& a, const DialogDescriptor& b) {
            return a.order < b.order;
        });

    return result;
}

const DialogDescriptor* DialogRegistry::descriptor(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_descriptors.find(id);
    if (it == m_descriptors.end())
    {
        return nullptr;
    }

    return &it->second;
}

std::vector<DialogDescriptor> DialogRegistry::dialogsForMenu(const std::string& menuPath) const
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<DialogDescriptor> result;

    for (const auto& [id, desc] : m_descriptors)
    {
        if (desc.menuPath == menuPath)
        {
            result.push_back(desc);
        }
    }

    // Sort by order
    std::sort(result.begin(), result.end(),
        [](const DialogDescriptor& a, const DialogDescriptor& b) {
            return a.order < b.order;
        });

    return result;
}
