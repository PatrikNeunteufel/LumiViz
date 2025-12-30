/**
 ****************************************************************************************
 * @file   DialogRegistry.hpp
 * @brief  Registry for dialogs with self-registration macros
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Dialog Self-Registration
 *
 * Dialoge können sich selbst beim Programmstart registrieren, analog zu Panels.
 * Der Unterschied: Dialoge sind modal und werden bei Bedarf erstellt/angezeigt.
 *
 * ### Konzept
 *
 * ```
 * DialogRegistry (Singleton)
 *       │
 *       ├── "about" ──► Factory ──► AboutDialog
 *       ├── "settings" ──► Factory ──► SettingsDialog
 *       └── "open_audio" ──► Factory ──► OpenAudioDialog
 *
 * DialogManager
 *       │
 *       └── show("about") ──► creates & exec() dialog
 * ```
 *
 * ### Self-Registration
 *
 * ```cpp
 * // Am Ende von AboutDialog.cpp:
 * REGISTER_DIALOG("about", "About MyViz", AboutDialog)
 * ```
 *
 * ### Verwendung
 *
 * ```cpp
 * // Über EventBus
 * eventBus.publish(OpenDialogEvent{"about"});
 *
 * // Oder direkt über DialogManager
 * dialogManager.show("about");
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
class QDialog;
class QWidget;

/**
 * @struct DialogDescriptor
 * @brief Describes a registered dialog
 */
struct DialogDescriptor
{
    std::string id;           ///< Unique ID (e.g., "about", "settings")
    std::string title;        ///< Window title
    int order = 0;            ///< Sort order for dialog lists
    bool modal = true;        ///< Modal dialog (blocks parent)
    std::string menuPath;     ///< Optional: Menu path for auto-registration
    std::string shortcut;     ///< Optional: Keyboard shortcut (e.g., "F1")
};

/**
 * @class DialogRegistry
 * @brief Singleton registry for dialog factories
 *
 * Stores factories that create dialog instances. Used by DialogManager
 * to create and show dialogs on demand.
 */
class DialogRegistry
{
public:
    /// Factory function type: creates a dialog given ServiceContainer and parent
    using Factory = std::function<std::unique_ptr<QDialog>(ServiceContainer&, QWidget* parent)>;

    /**
     * @brief Get singleton instance
     * @return Reference to the registry
     */
    static DialogRegistry& instance();

    /**
     * @brief Register a dialog
     *
     * @param descriptor Dialog metadata
     * @param factory Factory function
     * @param overwrite Replace existing registration if true
     *
     * @code
     * DialogRegistry::instance().registerDialog(
     *     DialogDescriptor{"about", "About MyViz", 100},
     *     [](ServiceContainer& svc, QWidget* parent) {
     *         return std::make_unique<AboutDialog>(svc, parent);
     *     }
     * );
     * @endcode
     */
    void registerDialog(const DialogDescriptor& descriptor,
                        Factory factory,
                        bool overwrite = false);

    /**
     * @brief Check if a dialog is registered
     * @param id Dialog ID
     * @return true if registered
     */
    [[nodiscard]] bool has(const std::string& id) const;

    /**
     * @brief Create a dialog instance
     *
     * @param id Dialog ID
     * @param services ServiceContainer for dependencies
     * @param parent Parent widget
     * @return New dialog instance or nullptr if not registered
     */
    [[nodiscard]] std::unique_ptr<QDialog> create(const std::string& id,
                                                   ServiceContainer& services,
                                                   QWidget* parent = nullptr) const;

    /**
     * @brief Get all registered dialog descriptors
     * @return Vector of descriptors (sorted by order)
     */
    [[nodiscard]] std::vector<DialogDescriptor> descriptors() const;

    /**
     * @brief Get descriptor by ID
     * @param id Dialog ID
     * @return Pointer to descriptor or nullptr
     */
    [[nodiscard]] const DialogDescriptor* descriptor(const std::string& id) const;

    /**
     * @brief Get dialogs that should appear in a menu
     * @param menuPath Menu path filter (e.g., "Help")
     * @return Vector of descriptors with matching menuPath
     */
    [[nodiscard]] std::vector<DialogDescriptor> dialogsForMenu(const std::string& menuPath) const;

    // Non-copyable singleton
    DialogRegistry(const DialogRegistry&) = delete;
    DialogRegistry& operator=(const DialogRegistry&) = delete;
    DialogRegistry(DialogRegistry&&) = delete;
    DialogRegistry& operator=(DialogRegistry&&) = delete;

private:
    DialogRegistry() = default;
    ~DialogRegistry() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, DialogDescriptor> m_descriptors;
    std::unordered_map<std::string, Factory> m_factories;
};

// =============================================================================
// Self-Registration Macros
// =============================================================================

// Helper macros for token concatenation
#ifndef DIALOG_REG_CAT_I
#  define DIALOG_REG_CAT_I(a, b) a##b
#  define DIALOG_REG_CAT(a, b) DIALOG_REG_CAT_I(a, b)
#endif

/**
 * @brief Register a dialog with self-registration
 *
 * @param ID_STR Unique ID string (e.g., "about")
 * @param TITLE_STR Window title (e.g., "About MyViz")
 * @param TYPE Dialog class type (must have constructor(ServiceContainer&, QWidget*))
 *
 * @code
 * // At the end of AboutDialog.cpp:
 * REGISTER_DIALOG("about", "About MyViz", AboutDialog)
 * @endcode
 */
#define REGISTER_DIALOG(ID_STR, TITLE_STR, TYPE)                                \
    namespace {                                                                  \
        struct DIALOG_REG_CAT(TYPE, __AutoDialogReg) {                          \
            DIALOG_REG_CAT(TYPE, __AutoDialogReg)() {                           \
                DialogRegistry::instance().registerDialog(                       \
                    DialogDescriptor{(ID_STR), (TITLE_STR), 0, true, "", ""},   \
                    [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QDialog> { \
                        return std::make_unique<TYPE>(svc, parent);              \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } DIALOG_REG_CAT(TYPE, __autoDialogRegInstance);                        \
    }

/**
 * @brief Register a dialog with menu integration
 *
 * @param ID_STR Unique ID string
 * @param TITLE_STR Window title
 * @param MENU_PATH Menu path (e.g., "Help")
 * @param ORDER Sort order
 * @param TYPE Dialog class type
 */
#define REGISTER_DIALOG_MENU(ID_STR, TITLE_STR, MENU_PATH, ORDER, TYPE)         \
    namespace {                                                                  \
        struct DIALOG_REG_CAT(TYPE, __AutoDialogRegMenu) {                      \
            DIALOG_REG_CAT(TYPE, __AutoDialogRegMenu)() {                       \
                DialogRegistry::instance().registerDialog(                       \
                    DialogDescriptor{(ID_STR), (TITLE_STR), (ORDER), true, (MENU_PATH), ""}, \
                    [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QDialog> { \
                        return std::make_unique<TYPE>(svc, parent);              \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } DIALOG_REG_CAT(TYPE, __autoDialogRegMenuInstance);                    \
    }

/**
 * @brief Register a dialog with shortcut
 *
 * @param ID_STR Unique ID string
 * @param TITLE_STR Window title
 * @param MENU_PATH Menu path
 * @param ORDER Sort order
 * @param SHORTCUT Keyboard shortcut (e.g., "F1")
 * @param TYPE Dialog class type
 */
#define REGISTER_DIALOG_SHORTCUT(ID_STR, TITLE_STR, MENU_PATH, ORDER, SHORTCUT, TYPE) \
    namespace {                                                                  \
        struct DIALOG_REG_CAT(TYPE, __AutoDialogRegShortcut) {                  \
            DIALOG_REG_CAT(TYPE, __AutoDialogRegShortcut)() {                   \
                DialogRegistry::instance().registerDialog(                       \
                    DialogDescriptor{(ID_STR), (TITLE_STR), (ORDER), true, (MENU_PATH), (SHORTCUT)}, \
                    [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QDialog> { \
                        return std::make_unique<TYPE>(svc, parent);              \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } DIALOG_REG_CAT(TYPE, __autoDialogRegShortcutInstance);                \
    }

/**
 * @brief Register a non-modal dialog
 *
 * @param ID_STR Unique ID string
 * @param TITLE_STR Window title
 * @param TYPE Dialog class type
 */
#define REGISTER_DIALOG_MODELESS(ID_STR, TITLE_STR, TYPE)                       \
    namespace {                                                                  \
        struct DIALOG_REG_CAT(TYPE, __AutoDialogRegModeless) {                  \
            DIALOG_REG_CAT(TYPE, __AutoDialogRegModeless)() {                   \
                DialogRegistry::instance().registerDialog(                       \
                    DialogDescriptor{(ID_STR), (TITLE_STR), 0, false, "", ""},  \
                    [](ServiceContainer& svc, QWidget* parent) -> std::unique_ptr<QDialog> { \
                        return std::make_unique<TYPE>(svc, parent);              \
                    },                                                           \
                    false                                                        \
                );                                                               \
            }                                                                    \
        } DIALOG_REG_CAT(TYPE, __autoDialogRegModelessInstance);                \
    }
