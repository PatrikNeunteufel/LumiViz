/**
 ****************************************************************************************
 * @file   GpuSelector.hpp
 * @brief  GPU Selection and Configuration - Qt6 Tutorial
 *         Manages GPU preference settings
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This module handles:
 *   - Loading/Saving GPU preferences from config file
 *   - Comparing active GPU with preferred GPU
 *   - Export flags for NVIDIA Optimus / AMD PowerXpress
 *
 * ## Configuration File Format (gpu.ini)
 *
 * ```ini
 * [GPU]
 * PreferHighPerformance=true
 * PreferredVendor=NVIDIA
 * PreferredName=RTX 4090
 * ```
 *
 * @see GpuSelector.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include "core/GpuInfo.hpp"
#include <optional>
#include <string>

// =============================================================================
// GpuPreference Structure
// =============================================================================

/**
 * @struct GpuPreference
 * @brief User's GPU preference settings.
 */
struct GpuPreference
{
    bool preferHighPerformance{true};       ///< Prefer dedicated GPU if available
    std::optional<GpuVendor> preferredVendor;  ///< Preferred vendor (optional)
    std::optional<std::string> preferredName;  ///< Partial name match (optional)
};

// =============================================================================
// GpuSelector Class
// =============================================================================

/**
 * @class GpuSelector
 * @brief Manages GPU selection and configuration.
 *
 * ## Usage Example
 *
 * ```cpp
 * // At application start
 * GpuSelector selector;
 * selector.loadConfig("gpu.ini");
 *
 * // Enumerate GPUs
 * auto gpus = GpuInfo::enumerate();
 * GpuInfo::logGpuInfo(gpus);
 *
 * // Get recommended GPU based on preferences
 * const GpuDevice* preferred = selector.selectGpu(gpus);
 *
 * // Later: Check if correct GPU is active
 * if (!selector.isPreferredGpuActive("AMD Radeon 610M"))
 * {
 *     // Warn user that wrong GPU is being used
 * }
 * ```
 */
class GpuSelector
{
public:
    // =========================================================================
    // Construction
    // =========================================================================

    GpuSelector() = default;
    ~GpuSelector() = default;

    // =========================================================================
    // Configuration
    // =========================================================================

    /**
     * @brief Loads GPU preferences from config file.
     *
     * @param filename Path to config file (e.g., "gpu.ini")
     * @return true if loaded successfully, false if file not found or error
     */
    bool loadConfig(const std::string& filename);

    /**
     * @brief Saves GPU preferences to config file.
     *
     * @param filename Path to config file
     * @return true if saved successfully
     */
    bool saveConfig(const std::string& filename) const;

    /**
     * @brief Creates a default config file if none exists.
     *
     * @param filename Path to config file
     * @return true if created, false if already exists
     */
    bool createDefaultConfig(const std::string& filename) const;

    // =========================================================================
    // Selection
    // =========================================================================

    /**
     * @brief Selects the best GPU based on current preferences.
     *
     * Selection order:
     *   1. If preferredName is set, find by name
     *   2. If preferredVendor is set, find by vendor
     *   3. If preferHighPerformance, find best dedicated GPU
     *   4. Fallback to first available GPU
     *
     * @param gpus Available GPUs
     * @return Pointer to selected GPU, or nullptr if no GPUs available
     */
    [[nodiscard]] const GpuDevice* selectGpu(
        const std::vector<GpuDevice>& gpus) const;

    /**
     * @brief Checks if the currently active GPU matches the preference.
     *
     * Call this after OpenGL context is created to verify the right GPU
     * is being used.
     *
     * @param activeGpuName Name of the GPU currently in use (from OpenGL)
     * @return true if active GPU matches preference
     */
    [[nodiscard]] bool isPreferredGpuActive(
        const std::string& activeGpuName) const;

    /**
     * @brief Gets a warning message if wrong GPU is active.
     *
     * @param activeGpuName Name of the GPU currently in use
     * @param gpus Available GPUs for context
     * @return Warning message, or empty string if correct GPU is active
     */
    [[nodiscard]] std::string getGpuMismatchWarning(
        const std::string& activeGpuName,
        const std::vector<GpuDevice>& gpus) const;

    // =========================================================================
    // Preference Accessors
    // =========================================================================

    [[nodiscard]] const GpuPreference& preference() const noexcept
    {
        return m_preference;
    }

    void setPreference(const GpuPreference& pref)
    {
        m_preference = pref;
    }

    void setPreferHighPerformance(bool prefer)
    {
        m_preference.preferHighPerformance = prefer;
    }

    void setPreferredVendor(GpuVendor vendor)
    {
        m_preference.preferredVendor = vendor;
    }

    void setPreferredName(const std::string& namePart)
    {
        m_preference.preferredName = namePart;
    }

    void clearPreferredVendor()
    {
        m_preference.preferredVendor.reset();
    }

    void clearPreferredName()
    {
        m_preference.preferredName.reset();
    }

private:
    GpuPreference m_preference;
    mutable std::string m_lastSelectedGpuName;  // For mismatch checking
};

// =============================================================================
// Export Flags for Hybrid Graphics
// =============================================================================

/**
 * @brief Forces high-performance GPU on hybrid systems.
 *
 * These export symbols are read by NVIDIA/AMD drivers BEFORE the application
 * starts. They hint to use the dedicated GPU instead of integrated.
 *
 * Must be in a .cpp file that gets linked into the final executable.
 * The flags are safe to include even if no dGPU exists - they're just ignored.
 *
 * Usage: Include this macro in main.cpp or a core file:
 *
 * ```cpp
 * MYVIZ_ENABLE_HIGH_PERFORMANCE_GPU
 * ```
 */
#ifdef _WIN32
    #define MYVIZ_ENABLE_HIGH_PERFORMANCE_GPU \
        extern "C" { \
            __declspec(dllexport) unsigned long NvOptimusEnablement = 1; \
            __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; \
        }
#else
    #define MYVIZ_ENABLE_HIGH_PERFORMANCE_GPU
#endif
