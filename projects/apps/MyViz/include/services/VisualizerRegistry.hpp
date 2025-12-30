/**
 ****************************************************************************************
 * @file   VisualizerRegistry.hpp
 * @brief  Registry for visualizers with self-registration macros
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Visualizer Self-Registration
 *
 * Visualizer können sich selbst beim Programmstart registrieren:
 *
 * ```cpp
 * // Am Ende von PulsingVisualizer.cpp:
 * REGISTER_VISUALIZER("pulsing", "Pulsing", "Simple pulsing effect", PulsingVisualizer)
 * ```
 *
 * ### Verwendung
 *
 * ```cpp
 * // Visualizer erstellen
 * auto viz = VisualizerRegistry::instance().create("pulsing");
 *
 * // Alle Visualizer auflisten
 * for (const auto& desc : VisualizerRegistry::instance().descriptors()) {
 *     qDebug() << desc.name;
 * }
 * ```
 ****************************************************************************************
 */

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations
class IVisualizer;

/**
 * @struct VisualizerDescriptor
 * @brief Describes a registered visualizer
 */
struct VisualizerDescriptor
{
    std::string id;           ///< Unique ID (e.g., "spectrum", "pulsing")
    std::string name;         ///< Display name
    std::string description;  ///< Brief description
    std::string category;     ///< Category for grouping (e.g., "Basic", "Audio")
    int order = 0;            ///< Sort order within category
    bool usesAudio = false;   ///< Whether visualizer reacts to audio
};

/**
 * @class VisualizerRegistry
 * @brief Singleton registry for visualizer factories
 */
class VisualizerRegistry
{
public:
    /// Factory function type
    using Factory = std::function<std::unique_ptr<IVisualizer>()>;

    /**
     * @brief Get singleton instance
     */
    static VisualizerRegistry& instance();

    /**
     * @brief Register a visualizer
     *
     * @param descriptor Visualizer metadata
     * @param factory Factory function
     * @param overwrite Replace existing registration if true
     */
    void registerVisualizer(const VisualizerDescriptor& descriptor,
                            Factory factory,
                            bool overwrite = false);

    /**
     * @brief Check if a visualizer is registered
     * @param id Visualizer ID
     */
    [[nodiscard]] bool has(const std::string& id) const;

    /**
     * @brief Create a visualizer instance
     *
     * @param id Visualizer ID
     * @return New visualizer instance or nullptr if not registered
     */
    [[nodiscard]] std::unique_ptr<IVisualizer> create(const std::string& id) const;

    /**
     * @brief Get all registered visualizer descriptors
     * @return Vector of descriptors (sorted by category then order)
     */
    [[nodiscard]] std::vector<VisualizerDescriptor> descriptors() const;

    /**
     * @brief Get descriptor by ID
     * @param id Visualizer ID
     */
    [[nodiscard]] const VisualizerDescriptor* descriptor(const std::string& id) const;

    /**
     * @brief Get visualizers in a specific category
     * @param category Category name
     */
    [[nodiscard]] std::vector<VisualizerDescriptor> visualizersInCategory(
        const std::string& category) const;

    /**
     * @brief Get all category names
     */
    [[nodiscard]] std::vector<std::string> categories() const;

    /**
     * @brief Get IDs of visualizers that use audio
     */
    [[nodiscard]] std::vector<std::string> audioVisualizers() const;

    // Non-copyable singleton
    VisualizerRegistry(const VisualizerRegistry&) = delete;
    VisualizerRegistry& operator=(const VisualizerRegistry&) = delete;
    VisualizerRegistry(VisualizerRegistry&&) = delete;
    VisualizerRegistry& operator=(VisualizerRegistry&&) = delete;

private:
    VisualizerRegistry() = default;
    ~VisualizerRegistry() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, VisualizerDescriptor> m_descriptors;
    std::unordered_map<std::string, Factory> m_factories;
};

// =============================================================================
// Self-Registration Macros
// =============================================================================

#ifndef VIZ_REG_CAT_I
#  define VIZ_REG_CAT_I(a, b) a##b
#  define VIZ_REG_CAT(a, b) VIZ_REG_CAT_I(a, b)
#endif

/**
 * @brief Register a visualizer with self-registration
 *
 * @param ID_STR Unique ID string
 * @param NAME_STR Display name
 * @param DESC_STR Description
 * @param TYPE Visualizer class type
 */
#define REGISTER_VISUALIZER(ID_STR, NAME_STR, DESC_STR, TYPE)                   \
    namespace {                                                                  \
        struct VIZ_REG_CAT(TYPE, __AutoVizReg) {                                \
            VIZ_REG_CAT(TYPE, __AutoVizReg)() {                                 \
                VisualizerRegistry::instance().registerVisualizer(               \
                    VisualizerDescriptor{(ID_STR), (NAME_STR), (DESC_STR), "", 0, false}, \
                    []() -> std::unique_ptr<IVisualizer> {                       \
                        return std::make_unique<TYPE>();                         \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } VIZ_REG_CAT(TYPE, __autoVizRegInstance);                              \
    }

/**
 * @brief Register a visualizer with category
 *
 * @param ID_STR Unique ID string
 * @param NAME_STR Display name
 * @param DESC_STR Description
 * @param CATEGORY Category for grouping
 * @param TYPE Visualizer class type
 */
#define REGISTER_VISUALIZER_CATEGORY(ID_STR, NAME_STR, DESC_STR, CATEGORY, TYPE) \
    namespace {                                                                  \
        struct VIZ_REG_CAT(TYPE, __AutoVizRegCat) {                             \
            VIZ_REG_CAT(TYPE, __AutoVizRegCat)() {                              \
                VisualizerRegistry::instance().registerVisualizer(               \
                    VisualizerDescriptor{(ID_STR), (NAME_STR), (DESC_STR), (CATEGORY), 0, false}, \
                    []() -> std::unique_ptr<IVisualizer> {                       \
                        return std::make_unique<TYPE>();                         \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } VIZ_REG_CAT(TYPE, __autoVizRegCatInstance);                           \
    }

/**
 * @brief Register an audio-reactive visualizer
 *
 * @param ID_STR Unique ID string
 * @param NAME_STR Display name
 * @param DESC_STR Description
 * @param CATEGORY Category for grouping
 * @param ORDER Sort order
 * @param TYPE Visualizer class type
 */
#define REGISTER_VISUALIZER_AUDIO(ID_STR, NAME_STR, DESC_STR, CATEGORY, ORDER, TYPE) \
    namespace {                                                                  \
        struct VIZ_REG_CAT(TYPE, __AutoVizRegAudio) {                           \
            VIZ_REG_CAT(TYPE, __AutoVizRegAudio)() {                            \
                VisualizerRegistry::instance().registerVisualizer(               \
                    VisualizerDescriptor{(ID_STR), (NAME_STR), (DESC_STR), (CATEGORY), (ORDER), true}, \
                    []() -> std::unique_ptr<IVisualizer> {                       \
                        return std::make_unique<TYPE>();                         \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } VIZ_REG_CAT(TYPE, __autoVizRegAudioInstance);                         \
    }
