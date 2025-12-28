/**
 ****************************************************************************************
 * @file   GpuSelector.cpp
 * @brief  GPU Selection and Configuration Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// =============================================================================
// Includes
// =============================================================================

#include "pch.h"
#include "core/GpuSelector.hpp"

// BasicLogger
#include <BasicLogger.h>

// Standard Library
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

// =============================================================================
// Helper Functions
// =============================================================================

namespace
{

/**
 * @brief Trims whitespace from both ends of a string.
 */
std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

/**
 * @brief Converts string to lowercase.
 */
std::string toLower(const std::string& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * @brief Parses a boolean value from string.
 */
bool parseBool(const std::string& str)
{
    std::string lower = toLower(trim(str));
    return (lower == "true" || lower == "1" || lower == "yes" || lower == "on");
}

/**
 * @brief Parses GpuVendor from string.
 */
GpuVendor parseVendor(const std::string& str)
{
    std::string lower = toLower(trim(str));
    
    if (lower == "nvidia" || lower == "nv")
    {
        return GpuVendor::NVIDIA;
    }
    if (lower == "amd" || lower == "ati" || lower == "radeon")
    {
        return GpuVendor::AMD;
    }
    if (lower == "intel")
    {
        return GpuVendor::Intel;
    }
    
    return GpuVendor::Unknown;
}

/**
 * @brief Converts GpuVendor to string for config file.
 */
std::string vendorToString(GpuVendor vendor)
{
    switch (vendor)
    {
        case GpuVendor::NVIDIA: return "NVIDIA";
        case GpuVendor::AMD:    return "AMD";
        case GpuVendor::Intel:  return "Intel";
        default:                return "";
    }
}

} // anonymous namespace

// =============================================================================
// Configuration
// =============================================================================

bool GpuSelector::loadConfig(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        BasicLogger::logDebug("GpuSelector: Config file not found: " + filename);
        return false;
    }
    
    BasicLogger::logInfo("GpuSelector: Loading config from " + filename);
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line))
    {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';')
        {
            continue;
        }
        
        // Section header
        if (line[0] == '[' && line.back() == ']')
        {
            currentSection = toLower(line.substr(1, line.size() - 2));
            continue;
        }
        
        // Key=Value pair
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos)
        {
            std::string key = toLower(trim(line.substr(0, equalPos)));
            std::string value = trim(line.substr(equalPos + 1));
            
            if (currentSection == "gpu")
            {
                if (key == "preferhighperformance")
                {
                    m_preference.preferHighPerformance = parseBool(value);
                    BasicLogger::logDebug("  PreferHighPerformance = " + 
                        std::string(m_preference.preferHighPerformance ? "true" : "false"));
                }
                else if (key == "preferredvendor")
                {
                    GpuVendor vendor = parseVendor(value);
                    if (vendor != GpuVendor::Unknown)
                    {
                        m_preference.preferredVendor = vendor;
                        BasicLogger::logDebug("  PreferredVendor = " + value);
                    }
                }
                else if (key == "preferredname")
                {
                    if (!value.empty())
                    {
                        m_preference.preferredName = value;
                        BasicLogger::logDebug("  PreferredName = " + value);
                    }
                }
            }
        }
    }
    
    return true;
}

bool GpuSelector::saveConfig(const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        BasicLogger::logError("GpuSelector: Cannot write config file: " + filename);
        return false;
    }
    
    file << "# MyViz GPU Configuration\n";
    file << "# Generated automatically\n";
    file << "\n";
    file << "[GPU]\n";
    file << "PreferHighPerformance=" 
         << (m_preference.preferHighPerformance ? "true" : "false") << "\n";
    
    if (m_preference.preferredVendor.has_value())
    {
        file << "PreferredVendor=" 
             << vendorToString(m_preference.preferredVendor.value()) << "\n";
    }
    
    if (m_preference.preferredName.has_value())
    {
        file << "PreferredName=" << m_preference.preferredName.value() << "\n";
    }
    
    BasicLogger::logInfo("GpuSelector: Saved config to " + filename);
    return true;
}

bool GpuSelector::createDefaultConfig(const std::string& filename) const
{
    // Check if file exists
    std::ifstream checkFile(filename);
    if (checkFile.is_open())
    {
        checkFile.close();
        return false;  // File already exists
    }
    
    std::ofstream file(filename);
    if (!file.is_open())
    {
        return false;
    }
    
    file << "# MyViz GPU Configuration\n";
    file << "#\n";
    file << "# This file controls which GPU is preferred for rendering.\n";
    file << "#\n";
    file << "# Options:\n";
    file << "#   PreferHighPerformance = true/false\n";
    file << "#       If true, prefer dedicated GPU over integrated\n";
    file << "#\n";
    file << "#   PreferredVendor = NVIDIA / AMD / Intel\n";
    file << "#       Prefer GPUs from this vendor (optional)\n";
    file << "#\n";
    file << "#   PreferredName = <partial name>\n";
    file << "#       Prefer GPU whose name contains this string (optional)\n";
    file << "#       Example: PreferredName=RTX 4090\n";
    file << "#\n";
    file << "\n";
    file << "[GPU]\n";
    file << "PreferHighPerformance=true\n";
    file << "# PreferredVendor=NVIDIA\n";
    file << "# PreferredName=RTX\n";
    
    BasicLogger::logInfo("GpuSelector: Created default config: " + filename);
    return true;
}

// =============================================================================
// Selection
// =============================================================================

const GpuDevice* GpuSelector::selectGpu(const std::vector<GpuDevice>& gpus) const
{
    if (gpus.empty())
    {
        return nullptr;
    }
    
    const GpuDevice* selected = nullptr;
    
    // Priority 1: Find by name
    if (m_preference.preferredName.has_value())
    {
        selected = GpuInfo::findByName(gpus, m_preference.preferredName.value());
        if (selected != nullptr)
        {
            BasicLogger::logDebug("GpuSelector: Selected by name match: " + selected->name);
            m_lastSelectedGpuName = selected->name;
            return selected;
        }
    }
    
    // Priority 2: Find by vendor
    if (m_preference.preferredVendor.has_value())
    {
        selected = GpuInfo::findByVendor(gpus, m_preference.preferredVendor.value());
        if (selected != nullptr)
        {
            BasicLogger::logDebug("GpuSelector: Selected by vendor: " + selected->name);
            m_lastSelectedGpuName = selected->name;
            return selected;
        }
    }
    
    // Priority 3: Find best high-performance GPU
    if (m_preference.preferHighPerformance)
    {
        selected = GpuInfo::findBestGpu(gpus);
        if (selected != nullptr)
        {
            BasicLogger::logDebug("GpuSelector: Selected best GPU: " + selected->name);
            m_lastSelectedGpuName = selected->name;
            return selected;
        }
    }
    
    // Fallback: First available
    selected = &gpus[0];
    BasicLogger::logDebug("GpuSelector: Selected first available: " + selected->name);
    m_lastSelectedGpuName = selected->name;
    return selected;
}

bool GpuSelector::isPreferredGpuActive(const std::string& activeGpuName) const
{
    if (m_lastSelectedGpuName.empty())
    {
        return true;  // No preference set
    }
    
    // Case-insensitive partial match
    std::string activeLower = toLower(activeGpuName);
    std::string preferredLower = toLower(m_lastSelectedGpuName);
    
    // Check if either contains the other (partial match)
    return (activeLower.find(preferredLower) != std::string::npos) ||
           (preferredLower.find(activeLower) != std::string::npos);
}

std::string GpuSelector::getGpuMismatchWarning(
    const std::string& activeGpuName,
    const std::vector<GpuDevice>& gpus) const
{
    if (isPreferredGpuActive(activeGpuName))
    {
        return "";  // No mismatch
    }
    
    std::ostringstream warning;
    warning << "GPU Mismatch Detected!\n";
    warning << "  Active GPU:    " << activeGpuName << "\n";
    warning << "  Preferred GPU: " << m_lastSelectedGpuName << "\n";
    warning << "\n";
    
    // List available GPUs
    if (!gpus.empty())
    {
        warning << "Available GPUs:\n";
        for (const auto& gpu : gpus)
        {
            warning << "  - " << gpu.name;
            if (gpu.isHighPerformance())
            {
                warning << " (High Performance)";
            }
            warning << "\n";
        }
        warning << "\n";
    }
    
    warning << "To use the preferred GPU:\n";
    warning << "  1. Windows Settings -> System -> Display -> Graphics\n";
    warning << "  2. Add MyViz.exe and set to 'High Performance'\n";
    warning << "  OR\n";
    warning << "  1. NVIDIA Control Panel -> Manage 3D Settings\n";
    warning << "  2. Add MyViz.exe -> High-performance NVIDIA processor\n";
    
    return warning.str();
}
