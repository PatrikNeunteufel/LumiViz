/**
 ****************************************************************************************
 * @file   WidgetRegistry.hpp
 * @brief  Registry for custom widgets with self-registration macros
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Widget Self-Registration
 *
 * Widgets können sich selbst beim Programmstart registrieren.
 * Im Gegensatz zu Panels sind Widgets keine Dock-Widgets,
 * sondern können überall eingebettet werden.
 *
 * ### Konzepte
 *
 * - **Panel:** Dockbares Fenster (PlayerPanel, PlaylistPanel)
 * - **Widget:** Einbettbares Element (VolumeControl, Waveform)
 * - **Dialog:** Modales Fenster (AboutDialog, SettingsDialog)
 *
 * ### Self-Registration
 *
 * ```cpp
 * // Am Ende von VolumeWidget.cpp:
 * REGISTER_WIDGET("volume", "Volume Control", VolumeWidget)
 * ```
 *
 * ### Verwendung
 *
 * ```cpp
 * // Widget erstellen
 * auto widget = WidgetRegistry::instance().create("volume", services, parent);
 *
 * // In Layout einfügen
 * layout->addWidget(widget.release());
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
class ServiceContainer;
class QWidget;

/**
 * @struct WidgetDescriptor
 * @brief Describes a registered widget
 */
struct WidgetDescriptor
{
    std::string id;           ///< Unique ID (e.g., "volume", "waveform")
    std::string name;         ///< Display name
    std::string category;     ///< Category for grouping (e.g., "Controls", "Visualizers")
    std::string description;  ///< Brief description
    int order = 0;            ///< Sort order within category
};

/**
 * @class WidgetRegistry
 * @brief Singleton registry for widget factories
 *
 * Stores factories that create widget instances.
 */
class WidgetRegistry
{
public:
    /// Factory function type
    using Factory = std::function<std::unique_ptr<QWidget>(ServiceContainer&, QWidget* parent)>;

    /**
     * @brief Get singleton instance
     */
    static WidgetRegistry& instance();

    /**
     * @brief Register a widget
     *
     * @param descriptor Widget metadata
     * @param factory Factory function
     * @param overwrite Replace existing registration if true
     */
    void registerWidget(const WidgetDescriptor& descriptor,
                        Factory factory,
                        bool overwrite = false);

    /**
     * @brief Check if a widget is registered
     * @param id Widget ID
     */
    [[nodiscard]] bool has(const std::string& id) const;

    /**
     * @brief Create a widget instance
     *
     * @param id Widget ID
     * @param services ServiceContainer for dependencies
     * @param parent Parent widget
     * @return New widget instance or nullptr if not registered
     */
    [[nodiscard]] std::unique_ptr<QWidget> create(const std::string& id,
                                                   ServiceContainer& services,
                                                   QWidget* parent = nullptr) const;

    /**
     * @brief Get all registered widget descriptors
     * @return Vector of descriptors (sorted by category then order)
     */
    [[nodiscard]] std::vector<WidgetDescriptor> descriptors() const;

    /**
     * @brief Get descriptor by ID
     * @param id Widget ID
     */
    [[nodiscard]] const WidgetDescriptor* descriptor(const std::string& id) const;

    /**
     * @brief Get widgets in a specific category
     * @param category Category name
     */
    [[nodiscard]] std::vector<WidgetDescriptor> widgetsInCategory(const std::string& category) const;

    /**
     * @brief Get all category names
     */
    [[nodiscard]] std::vector<std::string> categories() const;

    // Non-copyable singleton
    WidgetRegistry(const WidgetRegistry&) = delete;
    WidgetRegistry& operator=(const WidgetRegistry&) = delete;
    WidgetRegistry(WidgetRegistry&&) = delete;
    WidgetRegistry& operator=(WidgetRegistry&&) = delete;

private:
    WidgetRegistry() = default;
    ~WidgetRegistry() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, WidgetDescriptor> m_descriptors;
    std::unordered_map<std::string, Factory> m_factories;
};

// =============================================================================
// Self-Registration Macros
// =============================================================================

#ifndef WIDGET_REG_CAT_I
#  define WIDGET_REG_CAT_I(a, b) a##b
#  define WIDGET_REG_CAT(a, b) WIDGET_REG_CAT_I(a, b)
#endif

/**
 * @brief Register a widget with self-registration
 *
 * @param ID_STR Unique ID string
 * @param NAME_STR Display name
 * @param TYPE Widget class type
 */
#define REGISTER_WIDGET(ID_STR, NAME_STR, TYPE)                                 \
    namespace {                                                                  \
        struct WIDGET_REG_CAT(TYPE, __AutoWidgetReg) {                          \
            WIDGET_REG_CAT(TYPE, __AutoWidgetReg)() {                           \
                WidgetRegistry::instance().registerWidget(                       \
                    WidgetDescriptor{(ID_STR), (NAME_STR), "", "", 0},          \
                    [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QWidget> { \
                        return std::make_unique<TYPE>(svc, parent);              \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } WIDGET_REG_CAT(TYPE, __autoWidgetRegInstance);                        \
    }

/**
 * @brief Register a widget with category
 *
 * @param ID_STR Unique ID string
 * @param NAME_STR Display name
 * @param CATEGORY Category for grouping
 * @param TYPE Widget class type
 */
#define REGISTER_WIDGET_CATEGORY(ID_STR, NAME_STR, CATEGORY, TYPE)              \
    namespace {                                                                  \
        struct WIDGET_REG_CAT(TYPE, __AutoWidgetRegCat) {                       \
            WIDGET_REG_CAT(TYPE, __AutoWidgetRegCat)() {                        \
                WidgetRegistry::instance().registerWidget(                       \
                    WidgetDescriptor{(ID_STR), (NAME_STR), (CATEGORY), "", 0},  \
                    [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QWidget> { \
                        return std::make_unique<TYPE>(svc, parent);              \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } WIDGET_REG_CAT(TYPE, __autoWidgetRegCatInstance);                     \
    }

/**
 * @brief Register a widget with full metadata
 *
 * @param ID_STR Unique ID string
 * @param NAME_STR Display name
 * @param CATEGORY Category for grouping
 * @param DESC Description
 * @param ORDER Sort order
 * @param TYPE Widget class type
 */
#define REGISTER_WIDGET_FULL(ID_STR, NAME_STR, CATEGORY, DESC, ORDER, TYPE)     \
    namespace {                                                                  \
        struct WIDGET_REG_CAT(TYPE, __AutoWidgetRegFull) {                      \
            WIDGET_REG_CAT(TYPE, __AutoWidgetRegFull)() {                       \
                WidgetRegistry::instance().registerWidget(                       \
                    WidgetDescriptor{(ID_STR), (NAME_STR), (CATEGORY), (DESC), (ORDER)}, \
                    [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QWidget> { \
                        return std::make_unique<TYPE>(svc, parent);              \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } WIDGET_REG_CAT(TYPE, __autoWidgetRegFullInstance);                    \
    }
