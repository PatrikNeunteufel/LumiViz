/**
 ****************************************************************************************
 * @file   PanelRegistry.hpp
 * @brief  Registry for panels with self-registration macros
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Panel Self-Registration
 *
 * Panels können sich selbst beim Programmstart registrieren, ohne dass
 * expliziter Code in main() oder Application benötigt wird.
 *
 * ### Wie es funktioniert
 *
 * ```cpp
 * // Am Ende von SpectrumPanel.cpp:
 * REGISTER_PANEL("spectrum", "Spectrum Analyzer", true, SpectrumPanel)
 * ```
 *
 * Das Makro erzeugt eine statische Variable, deren Konstruktor beim
 * Programmstart ausgeführt wird und das Panel registriert.
 *
 * ### Vorteile
 *
 * - **Dezentral:** Registrierung in der Panel-Datei, nicht zentral
 * - **Erweiterbar:** Neue Panels werden automatisch erkannt
 * - **Kein Boilerplate:** Keine manuelle Factory-Registrierung
 *
 * ### Registrierungsreihenfolge
 *
 * Statische Initialisierung in C++ hat keine garantierte Reihenfolge
 * zwischen Translation Units. Das ist hier OK, weil:
 *   1. PanelRegistry ist ein Singleton (wird bei erstem Zugriff erstellt)
 *   2. Panels werden erst später instantiiert (in Application::init)
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
 * @struct PanelDescriptor
 * @brief Describes a registered panel
 */
struct PanelDescriptor
{
    std::string id;               ///< Unique ID (e.g., "spectrum")
    std::string title;            ///< UI title (e.g., "Spectrum Analyzer")
    int order = 0;                ///< Sort order for menus
    bool defaultVisible = false;  ///< Visible on first launch
    std::string menuPath;         ///< Menu path (e.g., "View/Panels")
};

/**
 * @class PanelRegistry
 * @brief Singleton registry for panel factories
 *
 * Stores factories that create panel instances. Used by PanelManager
 * to instantiate all registered panels.
 */
class PanelRegistry
{
public:
    /// Factory function type: creates a panel given a ServiceContainer
    using Factory = std::function<std::unique_ptr<QWidget>(ServiceContainer&)>;

    /**
     * @brief Get singleton instance
     * @return Reference to the registry
     */
    static PanelRegistry& instance();

    /**
     * @brief Register a panel
     *
     * @param descriptor Panel metadata
     * @param factory Factory function
     * @param overwrite Replace existing registration if true
     *
     * @code
     * PanelRegistry::instance().registerPanel(
     *     PanelDescriptor{"spectrum", "Spectrum Analyzer", 100, true},
     *     [](ServiceContainer& svc) {
     *         return std::make_unique<SpectrumPanel>(svc);
     *     }
     * );
     * @endcode
     */
    void registerPanel(const PanelDescriptor& descriptor,
                       Factory factory,
                       bool overwrite = false);

    /**
     * @brief Check if a panel is registered
     * @param id Panel ID
     * @return true if registered
     */
    [[nodiscard]] bool has(const std::string& id) const;

    /**
     * @brief Create a panel instance
     *
     * @param id Panel ID
     * @param services ServiceContainer for dependencies
     * @return New panel instance or nullptr if not registered
     */
    [[nodiscard]] std::unique_ptr<QWidget> create(const std::string& id,
                                                   ServiceContainer& services) const;

    /**
     * @brief Get all registered panel descriptors
     * @return Vector of descriptors (sorted by order)
     */
    [[nodiscard]] std::vector<PanelDescriptor> descriptors() const;

    /**
     * @brief Get descriptor by ID
     * @param id Panel ID
     * @return Descriptor or empty optional
     */
    [[nodiscard]] const PanelDescriptor* descriptor(const std::string& id) const;

    // Non-copyable singleton
    PanelRegistry(const PanelRegistry&) = delete;
    PanelRegistry& operator=(const PanelRegistry&) = delete;
    PanelRegistry(PanelRegistry&&) = delete;
    PanelRegistry& operator=(PanelRegistry&&) = delete;

private:
    PanelRegistry() = default;
    ~PanelRegistry() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, PanelDescriptor> m_descriptors;
    std::unordered_map<std::string, Factory> m_factories;
};

// =============================================================================
// Self-Registration Macros
// =============================================================================

// Helper macros for token concatenation
#ifndef PANEL_REG_CAT_I
#  define PANEL_REG_CAT_I(a, b) a##b
#  define PANEL_REG_CAT(a, b) PANEL_REG_CAT_I(a, b)
#endif

/**
 * @brief Register a panel with self-registration
 *
 * @param ID_STR Unique ID string (e.g., "spectrum")
 * @param TITLE_STR Display title (e.g., "Spectrum Analyzer")
 * @param DEFAULT_VIS Default visibility (true/false)
 * @param TYPE Panel class type
 *
 * @code
 * // At the end of SpectrumPanel.cpp:
 * REGISTER_PANEL("spectrum", "Spectrum Analyzer", true, SpectrumPanel)
 * @endcode
 */
#define REGISTER_PANEL(ID_STR, TITLE_STR, DEFAULT_VIS, TYPE)                    \
    namespace {                                                                  \
        struct PANEL_REG_CAT(TYPE, __AutoPanelReg) {                            \
            PANEL_REG_CAT(TYPE, __AutoPanelReg)() {                             \
                PanelRegistry::instance().registerPanel(                         \
                    PanelDescriptor{(ID_STR), (TITLE_STR), 0, (DEFAULT_VIS), "View/Panels"}, \
                    [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {     \
                        return std::make_unique<TYPE>(svc);                      \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } PANEL_REG_CAT(TYPE, __autoPanelRegInstance);                          \
    }

/**
 * @brief Register a panel with custom order
 *
 * @param ID_STR Unique ID string
 * @param TITLE_STR Display title
 * @param ORDER_VAL Sort order (lower = first)
 * @param DEFAULT_VIS Default visibility
 * @param TYPE Panel class type
 */
#define REGISTER_PANEL_ORDERED(ID_STR, TITLE_STR, ORDER_VAL, DEFAULT_VIS, TYPE) \
    namespace {                                                                  \
        struct PANEL_REG_CAT(TYPE, __AutoPanelRegOrd) {                         \
            PANEL_REG_CAT(TYPE, __AutoPanelRegOrd)() {                          \
                PanelRegistry::instance().registerPanel(                         \
                    PanelDescriptor{(ID_STR), (TITLE_STR), (ORDER_VAL), (DEFAULT_VIS), "View/Panels"}, \
                    [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {     \
                        return std::make_unique<TYPE>(svc);                      \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } PANEL_REG_CAT(TYPE, __autoPanelRegOrdInstance);                       \
    }

/**
 * @brief Register a panel with custom menu path
 *
 * @param ID_STR Unique ID string
 * @param TITLE_STR Display title
 * @param MENU_PATH Menu path (e.g., "View/Debug")
 * @param ORDER_VAL Sort order
 * @param DEFAULT_VIS Default visibility
 * @param TYPE Panel class type
 */
#define REGISTER_PANEL_MENU(ID_STR, TITLE_STR, MENU_PATH, ORDER_VAL, DEFAULT_VIS, TYPE) \
    namespace {                                                                  \
        struct PANEL_REG_CAT(TYPE, __AutoPanelRegMenu) {                        \
            PANEL_REG_CAT(TYPE, __AutoPanelRegMenu)() {                         \
                PanelRegistry::instance().registerPanel(                         \
                    PanelDescriptor{(ID_STR), (TITLE_STR), (ORDER_VAL), (DEFAULT_VIS), (MENU_PATH)}, \
                    [](ServiceContainer& svc) -> std::unique_ptr<QWidget> {     \
                        return std::make_unique<TYPE>(svc);                      \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } PANEL_REG_CAT(TYPE, __autoPanelRegMenuInstance);                      \
    }
