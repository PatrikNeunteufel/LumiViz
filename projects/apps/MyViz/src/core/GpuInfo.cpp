/**
 ****************************************************************************************
 * @file   GpuInfo.cpp
 * @brief  GPU Information and Enumeration Implementation
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// =============================================================================
// Includes
// =============================================================================

#include "pch.h"
#include "core/GpuInfo.hpp"

// BasicLogger
#include <BasicLogger.h>

// Standard Library
#include <algorithm>
#include <cctype>

// Platform-specific includes
#if defined(_WIN32)
    #include <dxgi1_4.h>
    #include <wrl/client.h>  // ComPtr
    #pragma comment(lib, "dxgi.lib")
#elif defined(__linux__)
    #include <fstream>
    #include <sstream>
    #include <dirent.h>
    #include <cstdio>
#elif defined(__APPLE__)
    #include <IOKit/IOKitLib.h>
    #include <CoreFoundation/CoreFoundation.h>
#endif

// =============================================================================
// GpuDevice Implementation
// =============================================================================

std::string GpuDevice::vendorString() const
{
    switch (vendor)
    {
        case GpuVendor::NVIDIA:    return "NVIDIA";
        case GpuVendor::AMD:       return "AMD";
        case GpuVendor::Intel:     return "Intel";
        case GpuVendor::Microsoft: return "Microsoft";
        case GpuVendor::Other:     return "Other";
        default:                   return "Unknown";
    }
}

std::string GpuDevice::typeString() const
{
    switch (type)
    {
        case GpuType::Integrated: return "Integrated";
        case GpuType::Dedicated:  return "Dedicated";
        case GpuType::Software:   return "Software";
        default:                  return "Unknown";
    }
}

// =============================================================================
// GpuInfo - Enumeration
// =============================================================================

std::vector<GpuDevice> GpuInfo::enumerate()
{
#if defined(_WIN32)
    return enumerateWindows();
#elif defined(__linux__)
    return enumerateLinux();
#elif defined(__APPLE__)
    return enumerateMacOS();
#else
    BasicLogger::logWarning("GpuInfo: Platform not supported for GPU enumeration");
    return {};
#endif
}

// =============================================================================
// GpuInfo - Selection Helpers
// =============================================================================

const GpuDevice* GpuInfo::findBestGpu(const std::vector<GpuDevice>& gpus)
{
    if (gpus.empty())
    {
        return nullptr;
    }
    
    // First: Try to find dedicated GPU with most VRAM
    const GpuDevice* best = nullptr;
    uint64_t bestVram = 0;
    
    for (const auto& gpu : gpus)
    {
        if (gpu.type == GpuType::Dedicated && gpu.dedicatedVideoMemory > bestVram)
        {
            best = &gpu;
            bestVram = gpu.dedicatedVideoMemory;
        }
    }
    
    if (best != nullptr)
    {
        return best;
    }
    
    // Fallback: Any GPU with most VRAM
    for (const auto& gpu : gpus)
    {
        if (gpu.dedicatedVideoMemory > bestVram)
        {
            best = &gpu;
            bestVram = gpu.dedicatedVideoMemory;
        }
    }
    
    // Final fallback: First GPU
    return best ? best : &gpus[0];
}

const GpuDevice* GpuInfo::findByName(
    const std::vector<GpuDevice>& gpus,
    const std::string& namePart)
{
    // Convert search string to lowercase
    std::string searchLower = namePart;
    std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    for (const auto& gpu : gpus)
    {
        // Convert GPU name to lowercase
        std::string gpuNameLower = gpu.name;
        std::transform(gpuNameLower.begin(), gpuNameLower.end(), gpuNameLower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        
        // Check if search string is contained in GPU name
        if (gpuNameLower.find(searchLower) != std::string::npos)
        {
            return &gpu;
        }
    }
    
    return nullptr;
}

const GpuDevice* GpuInfo::findByVendor(
    const std::vector<GpuDevice>& gpus,
    GpuVendor vendor)
{
    for (const auto& gpu : gpus)
    {
        if (gpu.vendor == vendor)
        {
            return &gpu;
        }
    }
    return nullptr;
}

// =============================================================================
// GpuInfo - Utility
// =============================================================================

GpuVendor GpuInfo::vendorFromId(uint32_t vendorId)
{
    // Known PCI Vendor IDs
    switch (vendorId)
    {
        case 0x10DE: return GpuVendor::NVIDIA;    // NVIDIA Corporation
        case 0x1002: return GpuVendor::AMD;       // AMD/ATI
        case 0x1022: return GpuVendor::AMD;       // AMD (alternate)
        case 0x8086: return GpuVendor::Intel;     // Intel Corporation
        case 0x1414: return GpuVendor::Microsoft; // Microsoft (Basic Render)
        default:     return GpuVendor::Other;
    }
}

void GpuInfo::logGpuInfo(const std::vector<GpuDevice>& gpus)
{
    BasicLogger::logInfo("=== GPU Information ===");
    BasicLogger::logInfo("Found " + std::to_string(gpus.size()) + " GPU(s):");
    
    int index = 0;
    for (const auto& gpu : gpus)
    {
        BasicLogger::logInfo("");
        BasicLogger::logInfo("[GPU " + std::to_string(index) + "] " + gpu.name);
        BasicLogger::logInfo("  Vendor:    " + gpu.vendorString() + 
                             " (0x" + std::to_string(gpu.vendorId) + ")");
        BasicLogger::logInfo("  Type:      " + gpu.typeString());
        BasicLogger::logInfo("  VRAM:      " + std::to_string(gpu.vramMB()) + " MB");
        
        if (gpu.sharedSystemMemory > 0)
        {
            BasicLogger::logInfo("  Shared:    " + 
                std::to_string(gpu.sharedSystemMemory / (1024 * 1024)) + " MB");
        }
        
        BasicLogger::logInfo("  High-Perf: " + 
            std::string(gpu.isHighPerformance() ? "Yes" : "No"));
        
        ++index;
    }
    
    // Find and log recommended GPU
    const GpuDevice* best = findBestGpu(gpus);
    if (best != nullptr)
    {
        BasicLogger::logInfo("");
        BasicLogger::logInfo("Recommended GPU: " + best->name);
    }
    
    BasicLogger::logInfo("=======================");
}

// =============================================================================
// Platform: Windows (DXGI)
// =============================================================================

#if defined(_WIN32)

std::vector<GpuDevice> GpuInfo::enumerateWindows()
{
    std::vector<GpuDevice> result;
    
    // Create DXGI Factory
    // Note: IID_PPV_ARGS uses __uuidof which is a MSVC extension.
    // Clang supports it but warns - we suppress the warning here.
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif
    
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
    
    if (FAILED(hr))
    {
        BasicLogger::logError("GpuInfo: Failed to create DXGI Factory (HRESULT: " + 
                              std::to_string(hr) + ")");
        return result;
    }
    
    // Enumerate adapters
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    UINT adapterIndex = 0;
    
    while (factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC1 desc;
        hr = adapter->GetDesc1(&desc);
        
        if (SUCCEEDED(hr))
        {
            GpuDevice gpu;
            
            // Convert wide string to std::string
            int size_needed = WideCharToMultiByte(
                CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
            std::string name(size_needed - 1, 0);
            WideCharToMultiByte(
                CP_UTF8, 0, desc.Description, -1, &name[0], size_needed, nullptr, nullptr);
            
            gpu.name = name;
            gpu.vendorId = desc.VendorId;
            gpu.deviceId = desc.DeviceId;
            gpu.vendor = vendorFromId(desc.VendorId);
            
            // Memory information
            gpu.dedicatedVideoMemory = desc.DedicatedVideoMemory;
            gpu.dedicatedSystemMemory = desc.DedicatedSystemMemory;
            gpu.sharedSystemMemory = desc.SharedSystemMemory;
            
            // Determine GPU type
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                gpu.type = GpuType::Software;
            }
            else if (desc.DedicatedVideoMemory > 0)
            {
                // Has dedicated VRAM - likely dedicated GPU
                // But integrated GPUs can also report some dedicated memory
                // Heuristic: > 512 MB dedicated = dedicated GPU
                if (gpu.vramMB() > 512)
                {
                    gpu.type = GpuType::Dedicated;
                }
                else
                {
                    gpu.type = GpuType::Integrated;
                }
            }
            else
            {
                gpu.type = GpuType::Integrated;
            }
            
            // Skip Microsoft Basic Render Driver (software)
            if (gpu.vendor != GpuVendor::Microsoft)
            {
                result.push_back(gpu);
            }
        }
        
        adapter.Reset();
        ++adapterIndex;
    }
    
    return result;
}

#endif // _WIN32

// =============================================================================
// Platform: Linux (sysfs + lspci fallback)
// =============================================================================

#if defined(__linux__)

namespace
{

/**
 * @brief Reads a single line from a sysfs file.
 */
std::string readSysfsFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return "";
    }
    std::string line;
    std::getline(file, line);
    return line;
}

/**
 * @brief Parses hex string to uint32_t.
 */
uint32_t parseHex(const std::string& str)
{
    uint32_t value = 0;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> value;
    return value;
}

/**
 * @brief Gets GPU name from lspci for a specific PCI address.
 */
std::string getGpuNameFromLspci(const std::string& pciAddress)
{
    std::string command = "lspci -s " + pciAddress + " 2>/dev/null";
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr)
    {
        return "";
    }
    
    char buffer[256];
    std::string result;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result = buffer;
        // Remove newline
        if (!result.empty() && result.back() == '\n')
        {
            result.pop_back();
        }
        // Extract name after "VGA compatible controller: " or "3D controller: "
        size_t pos = result.find(": ");
        if (pos != std::string::npos)
        {
            result = result.substr(pos + 2);
        }
    }
    pclose(pipe);
    return result;
}

} // anonymous namespace

std::vector<GpuDevice> GpuInfo::enumerateLinux()
{
    std::vector<GpuDevice> result;
    
    BasicLogger::logDebug("GpuInfo: Enumerating GPUs via sysfs...");
    
    // Enumerate DRM devices in /sys/class/drm/
    DIR* dir = opendir("/sys/class/drm");
    if (dir == nullptr)
    {
        BasicLogger::logWarning("GpuInfo: Cannot open /sys/class/drm");
        return result;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;
        
        // Look for card0, card1, etc. (skip renderD* and card*-*)
        if (name.find("card") != 0 || name.find('-') != std::string::npos)
        {
            continue;
        }
        
        std::string cardPath = "/sys/class/drm/" + name + "/device";
        
        // Read vendor and device IDs
        std::string vendorStr = readSysfsFile(cardPath + "/vendor");
        std::string deviceStr = readSysfsFile(cardPath + "/device");
        
        if (vendorStr.empty() || deviceStr.empty())
        {
            continue;
        }
        
        GpuDevice gpu;
        
        // Parse IDs (format: 0x1234)
        gpu.vendorId = parseHex(vendorStr.substr(2));  // Skip "0x"
        gpu.deviceId = parseHex(deviceStr.substr(2));
        gpu.vendor = vendorFromId(gpu.vendorId);
        
        // Get PCI address for lspci
        std::string pciPath = cardPath + "/uevent";
        std::string uevent = readSysfsFile(pciPath);
        std::string pciAddress;
        
        // Parse PCI_SLOT_NAME from uevent
        std::ifstream ueventFile(pciPath);
        std::string line;
        while (std::getline(ueventFile, line))
        {
            if (line.find("PCI_SLOT_NAME=") == 0)
            {
                pciAddress = line.substr(14);
                break;
            }
        }
        
        // Get GPU name via lspci
        if (!pciAddress.empty())
        {
            gpu.name = getGpuNameFromLspci(pciAddress);
        }
        
        if (gpu.name.empty())
        {
            gpu.name = gpu.vendorString() + " GPU (ID: " + 
                       std::to_string(gpu.deviceId) + ")";
        }
        
        // Try to read VRAM from various locations
        // NVIDIA: /sys/class/drm/card*/device/mem_info_vram_total
        // AMD: /sys/class/drm/card*/device/mem_info_vram_total
        std::string vramStr = readSysfsFile(cardPath + "/mem_info_vram_total");
        if (!vramStr.empty())
        {
            gpu.dedicatedVideoMemory = std::stoull(vramStr);
        }
        
        // Determine GPU type based on vendor and VRAM
        if (gpu.vendor == GpuVendor::Intel)
        {
            gpu.type = GpuType::Integrated;
        }
        else if (gpu.dedicatedVideoMemory > 512 * 1024 * 1024)
        {
            gpu.type = GpuType::Dedicated;
        }
        else
        {
            // Heuristic: AMD/NVIDIA with little/no reported VRAM might be APU
            gpu.type = (gpu.vendor == GpuVendor::AMD || gpu.vendor == GpuVendor::NVIDIA)
                       ? GpuType::Dedicated : GpuType::Integrated;
        }
        
        result.push_back(gpu);
        BasicLogger::logDebug("  Found: " + gpu.name);
    }
    
    closedir(dir);
    
    if (result.empty())
    {
        BasicLogger::logWarning("GpuInfo: No GPUs found via sysfs");
    }
    
    return result;
}

#endif // __linux__

// =============================================================================
// Platform: macOS (IOKit)
// =============================================================================

#if defined(__APPLE__)

std::vector<GpuDevice> GpuInfo::enumerateMacOS()
{
    std::vector<GpuDevice> result;
    
    BasicLogger::logDebug("GpuInfo: Enumerating GPUs via IOKit...");
    
    // Find all GPU devices
    CFMutableDictionaryRef matchDict = IOServiceMatching("IOPCIDevice");
    if (matchDict == nullptr)
    {
        BasicLogger::logWarning("GpuInfo: Cannot create IOKit match dictionary");
        return result;
    }
    
    io_iterator_t iterator;
    kern_return_t kr = IOServiceGetMatchingServices(kIOMainPortDefault, matchDict, &iterator);
    
    if (kr != KERN_SUCCESS)
    {
        BasicLogger::logWarning("GpuInfo: IOServiceGetMatchingServices failed");
        return result;
    }
    
    io_service_t device;
    while ((device = IOIteratorNext(iterator)) != 0)
    {
        // Check if this is a display/GPU device
        CFStringRef className = (CFStringRef)IORegistryEntryCreateCFProperty(
            device, CFSTR("IOName"), kCFAllocatorDefault, 0);
        
        if (className != nullptr)
        {
            char classNameStr[128];
            if (CFStringGetCString(className, classNameStr, sizeof(classNameStr), kCFStringEncodingUTF8))
            {
                // Look for display or GPU class
                if (strstr(classNameStr, "display") != nullptr || 
                    strstr(classNameStr, "gpu") != nullptr ||
                    strstr(classNameStr, "GFX") != nullptr)
                {
                    GpuDevice gpu;
                    
                    // Get model name
                    CFStringRef modelName = (CFStringRef)IORegistryEntryCreateCFProperty(
                        device, CFSTR("model"), kCFAllocatorDefault, 0);
                    
                    if (modelName != nullptr)
                    {
                        char modelStr[256];
                        if (CFStringGetCString(modelName, modelStr, sizeof(modelStr), kCFStringEncodingUTF8))
                        {
                            gpu.name = modelStr;
                        }
                        CFRelease(modelName);
                    }
                    
                    // Get vendor ID
                    CFNumberRef vendorIdRef = (CFNumberRef)IORegistryEntryCreateCFProperty(
                        device, CFSTR("vendor-id"), kCFAllocatorDefault, 0);
                    
                    if (vendorIdRef != nullptr)
                    {
                        CFNumberGetValue(vendorIdRef, kCFNumberSInt32Type, &gpu.vendorId);
                        gpu.vendor = vendorFromId(gpu.vendorId);
                        CFRelease(vendorIdRef);
                    }
                    
                    // On macOS, most GPUs are integrated (Apple Silicon) or dedicated (AMD in older Macs)
                    if (gpu.vendor == GpuVendor::AMD || gpu.vendor == GpuVendor::NVIDIA)
                    {
                        gpu.type = GpuType::Dedicated;
                    }
                    else
                    {
                        gpu.type = GpuType::Integrated;  // Apple Silicon
                    }
                    
                    if (!gpu.name.empty())
                    {
                        result.push_back(gpu);
                        BasicLogger::logDebug("  Found: " + gpu.name);
                    }
                }
            }
            CFRelease(className);
        }
        
        IOObjectRelease(device);
    }
    
    IOObjectRelease(iterator);
    
    // Fallback: If nothing found, try to get info from OpenGL
    if (result.empty())
    {
        BasicLogger::logWarning("GpuInfo: No GPUs found via IOKit, will rely on OpenGL info");
    }
    
    return result;
}

#endif // __APPLE__
