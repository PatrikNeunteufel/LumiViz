/**
 ****************************************************************************************
 * @file   GpuInfo.hpp
 * @brief  GPU Information and Enumeration - Qt6 Tutorial
 *         Detects available GPUs on the system
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 *
 * @details
 * This module provides GPU enumeration functionality using platform-specific APIs:
 *   - Windows: DXGI (DirectX Graphics Infrastructure)
 *   - Linux: OpenGL/Vulkan enumeration (TODO)
 *   - macOS: Metal enumeration (TODO)
 *
 * ## Qt6 Tutorial: GPU Selection for Hybrid Graphics
 *
 * Modern laptops often have multiple GPUs:
 *   - Integrated GPU (iGPU): Power-efficient, always available
 *   - Dedicated GPU (dGPU): High-performance, for demanding tasks
 *
 * This module helps identify all available GPUs so the user can choose
 * which one to use for rendering.
 *
 * @see GpuInfo.md for detailed documentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include <cstdint>
#include <string>
#include <vector>

// =============================================================================
// GpuVendor Enumeration
// =============================================================================

/**
 * @enum GpuVendor
 * @brief Known GPU vendors.
 */
enum class GpuVendor
{
    Unknown,
    NVIDIA,
    AMD,
    Intel,
    Microsoft,  // e.g., Basic Render Driver
    Other
};

// =============================================================================
// GpuType Enumeration
// =============================================================================

/**
 * @enum GpuType
 * @brief Type of GPU (integrated vs dedicated).
 */
enum class GpuType
{
    Unknown,
    Integrated,  // iGPU - shares system RAM
    Dedicated,   // dGPU - has own VRAM
    Software     // Software renderer
};

// =============================================================================
// GpuDevice Structure
// =============================================================================

/**
 * @struct GpuDevice
 * @brief Information about a single GPU.
 */
struct GpuDevice
{
    // -------------------------------------------------------------------------
    // Identification
    // -------------------------------------------------------------------------
    std::string name;           ///< Display name (e.g., "NVIDIA GeForce RTX 4090")
    GpuVendor vendor{GpuVendor::Unknown};
    GpuType type{GpuType::Unknown};
    
    uint32_t vendorId{0};       ///< PCI Vendor ID (e.g., 0x10DE for NVIDIA)
    uint32_t deviceId{0};       ///< PCI Device ID
    
    // -------------------------------------------------------------------------
    // Memory
    // -------------------------------------------------------------------------
    uint64_t dedicatedVideoMemory{0};   ///< VRAM in bytes
    uint64_t dedicatedSystemMemory{0};  ///< Dedicated system memory in bytes
    uint64_t sharedSystemMemory{0};     ///< Shared system memory in bytes
    
    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------
    
    /**
     * @brief Gets the dedicated VRAM in megabytes.
     */
    [[nodiscard]] uint64_t vramMB() const noexcept
    {
        return dedicatedVideoMemory / (1024 * 1024);
    }
    
    /**
     * @brief Checks if this is a high-performance GPU.
     *
     * Considers it high-performance if:
     *   - It's a dedicated GPU, OR
     *   - It has significant VRAM (> 512 MB)
     */
    [[nodiscard]] bool isHighPerformance() const noexcept
    {
        return (type == GpuType::Dedicated) || (vramMB() > 512);
    }
    
    /**
     * @brief Gets a readable vendor string.
     */
    [[nodiscard]] std::string vendorString() const;
    
    /**
     * @brief Gets a readable type string.
     */
    [[nodiscard]] std::string typeString() const;
};

// =============================================================================
// GpuInfo Class
// =============================================================================

/**
 * @class GpuInfo
 * @brief Static utility class for GPU enumeration.
 *
 * ## Usage Example
 *
 * ```cpp
 * auto gpus = GpuInfo::enumerate();
 * for (const auto& gpu : gpus)
 * {
 *     std::cout << gpu.name << " (" << gpu.vramMB() << " MB VRAM)\n";
 * }
 *
 * auto best = GpuInfo::findBestGpu(gpus);
 * if (best)
 * {
 *     std::cout << "Recommended: " << best->name << "\n";
 * }
 * ```
 */
class GpuInfo
{
public:
    // =========================================================================
    // Enumeration
    // =========================================================================
    
    /**
     * @brief Enumerates all available GPUs on the system.
     *
     * Uses platform-specific APIs:
     *   - Windows: DXGI
     *   - Linux: TODO
     *   - macOS: TODO
     *
     * @return Vector of GpuDevice structures
     */
    [[nodiscard]] static std::vector<GpuDevice> enumerate();
    
    // =========================================================================
    // Selection Helpers
    // =========================================================================
    
    /**
     * @brief Finds the best GPU for high-performance rendering.
     *
     * Selection criteria (in order):
     *   1. Dedicated GPU with most VRAM
     *   2. Any GPU with most VRAM
     *
     * @param gpus List of available GPUs
     * @return Pointer to best GPU, or nullptr if list is empty
     */
    [[nodiscard]] static const GpuDevice* findBestGpu(
        const std::vector<GpuDevice>& gpus);
    
    /**
     * @brief Finds a GPU by name (partial match, case-insensitive).
     *
     * @param gpus List of available GPUs
     * @param namePart Part of the GPU name to search for
     * @return Pointer to matching GPU, or nullptr if not found
     */
    [[nodiscard]] static const GpuDevice* findByName(
        const std::vector<GpuDevice>& gpus,
        const std::string& namePart);
    
    /**
     * @brief Finds a GPU by vendor.
     *
     * @param gpus List of available GPUs
     * @param vendor Vendor to search for
     * @return Pointer to first matching GPU, or nullptr if not found
     */
    [[nodiscard]] static const GpuDevice* findByVendor(
        const std::vector<GpuDevice>& gpus,
        GpuVendor vendor);
    
    // =========================================================================
    // Utility
    // =========================================================================
    
    /**
     * @brief Converts vendor ID to GpuVendor enum.
     */
    [[nodiscard]] static GpuVendor vendorFromId(uint32_t vendorId);
    
    /**
     * @brief Logs all GPU information.
     *
     * @param gpus List of GPUs to log
     */
    static void logGpuInfo(const std::vector<GpuDevice>& gpus);

private:
    // Platform-specific implementation
    static std::vector<GpuDevice> enumerateWindows();
    static std::vector<GpuDevice> enumerateLinux();
    static std::vector<GpuDevice> enumerateMacOS();
};
