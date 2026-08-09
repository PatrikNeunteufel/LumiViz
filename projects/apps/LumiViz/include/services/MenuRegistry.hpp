/**
 ****************************************************************************************
 * @file   MenuRegistry.hpp
 * @brief  Registry for hierarchical menus with self-registration macros
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Dynamische Menüs
 *
 * Das MenuRegistry ermöglicht das deklarative Aufbauen von Menüs:
 *
 * ```
 * MenuBar (toplevel)
 * ├── File (container)
 * │   ├── Open... (item)
 * │   ├── ──────── (separator via group)
 * │   └── Exit (item)
 * ├── View (container)
 * │   └── Panels (container)
 * │       ├── Spectrum (item, checkable)
 * │       └── Waveform (item, checkable)
 * └── Help (container)
 *     └── About (item)
 * ```
 *
 * ### Konzepte
 *
 * - **Container:** Sichtbares Submenü (File, View, Help)
 * - **Group:** Unsichtbar, erzeugt Separator zwischen Gruppen
 * - **Item:** Klickbarer Eintrag mit Callback
 *
 * ### Self-Registration
 *
 * ```cpp
 * // In irgendeiner .cpp Datei:
 * REGISTER_MENU_CONTAINER("menu.file", "File", "toplevel", 100)
 * REGISTER_MENU_ITEM("menu.file.exit", "Exit", "menu.file", 900,
 *     [](ServiceContainer& svc) { QApplication::quit(); })
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

// Forward declaration
class ServiceContainer;

// =============================================================================
// Node Types
// =============================================================================

/**
 * @enum MenuNodeType
 * @brief Type of menu node
 */
enum class MenuNodeType
{
    Container,  ///< Visible submenu
    Group,      ///< Invisible, creates separator
    Item        ///< Clickable entry
};

// =============================================================================
// Descriptors
// =============================================================================

/**
 * @struct MenuBaseDesc
 * @brief Base descriptor for all menu nodes
 */
struct MenuBaseDesc
{
    std::string id;        ///< Unique ID (e.g., "menu.file.open")
    std::string parentId;  ///< Parent ID ("toplevel" for main bar)
    int order = 0;         ///< Sibling ordering (lower = first)
};

/**
 * @struct MenuContainerDesc
 * @brief Descriptor for a submenu container
 */
struct MenuContainerDesc : MenuBaseDesc
{
    std::string title;    ///< Display label (e.g., "File")
    bool exclusive = false;  ///< If true, child items are mutually exclusive (radio-like)
};

/**
 * @struct MenuGroupDesc
 * @brief Descriptor for a menu group (creates separator)
 */
struct MenuGroupDesc : MenuBaseDesc
{
    // No additional members
};

/**
 * @struct MenuItemDesc
 * @brief Descriptor for a clickable menu item
 */
struct MenuItemDesc : MenuBaseDesc
{
    std::string title;  ///< Display label (e.g., "Open...")

    /// Callback when item is clicked
    using Callback = std::function<void(ServiceContainer&)>;
    Callback onClick;

    /// Optional: Returns true if item should be checked
    using IsChecked = std::function<bool(ServiceContainer&)>;
    IsChecked isChecked;

    /// Optional: Returns true if item should be enabled
    using IsEnabled = std::function<bool(ServiceContainer&)>;
    IsEnabled isEnabled;

    /// Keyboard shortcut (e.g., "Ctrl+O")
    std::string shortcut;
};

// =============================================================================
// MenuRegistry
// =============================================================================

/**
 * @class MenuRegistry
 * @brief Singleton registry for menu structure
 * 
 * @details
 * Default menus are automatically registered on first access via instance().
 * No manual initialization required.
 */
class MenuRegistry
{
public:
    /**
     * @brief Get singleton instance
     * 
     * On first call, automatically registers all default menus (File, View, etc.)
     */
    static MenuRegistry& instance();

    /**
     * @brief Get root container ID
     * @return "toplevel"
     */
    static const std::string& rootId();

    // =========================================================================
    // Registration
    // =========================================================================

    void registerContainer(const MenuContainerDesc& desc, bool overwrite = false);
    void registerGroup(const MenuGroupDesc& desc, bool overwrite = false);
    void registerItem(const MenuItemDesc& desc, bool overwrite = false);

    // =========================================================================
    // Query
    // =========================================================================

    [[nodiscard]] bool hasContainer(const std::string& id) const;
    [[nodiscard]] bool hasGroup(const std::string& id) const;
    [[nodiscard]] bool hasItem(const std::string& id) const;

    [[nodiscard]] const MenuContainerDesc* container(const std::string& id) const;
    [[nodiscard]] const MenuGroupDesc* group(const std::string& id) const;
    [[nodiscard]] const MenuItemDesc* item(const std::string& id) const;

    // =========================================================================
    // Children Lookup
    // =========================================================================

    /**
     * @brief Node reference for iteration
     */
    struct NodeRef
    {
        MenuNodeType type;
        std::string id;
        int order;
    };

    /**
     * @brief Get all children of a parent (sorted by order)
     * @param parentId Parent container ID
     * @return Vector of node references
     */
    [[nodiscard]] std::vector<NodeRef> childrenOf(const std::string& parentId) const;

    /**
     * @brief Get all containers under a parent
     */
    [[nodiscard]] std::vector<std::string> containersUnder(const std::string& parentId) const;

    /**
     * @brief Get all groups under a parent
     */
    [[nodiscard]] std::vector<std::string> groupsUnder(const std::string& parentId) const;

    /**
     * @brief Get all items under a parent
     */
    [[nodiscard]] std::vector<std::string> itemsUnder(const std::string& parentId) const;

    // =========================================================================
    // Version
    // =========================================================================

    /**
     * @brief Get registry version (increments on each registration)
     *
     * Can be used to invalidate menu caches.
     */
    [[nodiscard]] uint64_t version() const;

    // Non-copyable singleton
    MenuRegistry(const MenuRegistry&) = delete;
    MenuRegistry& operator=(const MenuRegistry&) = delete;
    MenuRegistry(MenuRegistry&&) = delete;
    MenuRegistry& operator=(MenuRegistry&&) = delete;

private:
    MenuRegistry();
    ~MenuRegistry() = default;

    mutable std::mutex m_mutex;

    std::unordered_map<std::string, MenuContainerDesc> m_containers;
    std::unordered_map<std::string, MenuGroupDesc> m_groups;
    std::unordered_map<std::string, MenuItemDesc> m_items;

    // Parent → children mapping
    std::unordered_map<std::string, std::vector<std::string>> m_containerChildren;
    std::unordered_map<std::string, std::vector<std::string>> m_groupChildren;
    std::unordered_map<std::string, std::vector<std::string>> m_itemChildren;

    uint64_t m_version = 0;
};

// =============================================================================
// Self-Registration Macros
// =============================================================================

#ifndef MENU_REG_CAT_I
#  define MENU_REG_CAT_I(a, b) a##b
#  define MENU_REG_CAT(a, b) MENU_REG_CAT_I(a, b)
#endif

#define MENU_REG_AUTOREG_N(N, BODY)                                             \
    namespace {                                                                  \
        struct MENU_REG_CAT(MenuReg_, N) {                                      \
            MENU_REG_CAT(MenuReg_, N)() { BODY }                                \
        };                                                                       \
        static MENU_REG_CAT(MenuReg_, N) MENU_REG_CAT(menuRegInst_, N);         \
    }

#ifdef __COUNTER__
#  define MENU_REG_AUTOREG(BODY) MENU_REG_AUTOREG_N(__COUNTER__, BODY)
#else
#  define MENU_REG_AUTOREG(BODY) MENU_REG_AUTOREG_N(__LINE__, BODY)
#endif

/**
 * @brief Register a menu container (submenu)
 *
 * @param ID_STR Unique ID (e.g., "menu.file")
 * @param TITLE_STR Display title (e.g., "File")
 * @param PARENT_ID Parent container ID ("toplevel" for main bar)
 * @param ORDER Sort order
 */
#define REGISTER_MENU_CONTAINER(ID_STR, TITLE_STR, PARENT_ID, ORDER)            \
    MENU_REG_AUTOREG(                                                            \
        MenuRegistry::instance().registerContainer(                              \
            MenuContainerDesc{{(ID_STR), (PARENT_ID), (ORDER)}, (TITLE_STR), false}, \
            false);                                                              \
    )

/**
 * @brief Register an exclusive menu container (radio-button style)
 *
 * Items in this container will be mutually exclusive (only one checked at a time).
 *
 * @param ID_STR Unique ID
 * @param TITLE_STR Display title
 * @param PARENT_ID Parent container ID
 * @param ORDER Sort order
 */
#define REGISTER_MENU_CONTAINER_EXCLUSIVE(ID_STR, TITLE_STR, PARENT_ID, ORDER)  \
    MENU_REG_AUTOREG(                                                            \
        MenuRegistry::instance().registerContainer(                              \
            MenuContainerDesc{{(ID_STR), (PARENT_ID), (ORDER)}, (TITLE_STR), true}, \
            false);                                                              \
    )

/**
 * @brief Register a menu group (creates separator)
 *
 * @param ID_STR Unique ID
 * @param PARENT_ID Parent container ID
 * @param ORDER Sort order
 */
#define REGISTER_MENU_GROUP(ID_STR, PARENT_ID, ORDER)                           \
    MENU_REG_AUTOREG(                                                            \
        MenuRegistry::instance().registerGroup(                                  \
            MenuGroupDesc{{(ID_STR), (PARENT_ID), (ORDER)}},                    \
            false);                                                              \
    )

/**
 * @brief Register a menu item
 *
 * @param ID_STR Unique ID
 * @param TITLE_STR Display title
 * @param PARENT_ID Parent container ID
 * @param ORDER Sort order
 * @param CALLBACK Lambda: (ServiceContainer&) -> void
 */
#define REGISTER_MENU_ITEM(ID_STR, TITLE_STR, PARENT_ID, ORDER, CALLBACK)       \
    MENU_REG_AUTOREG(                                                            \
        MenuRegistry::instance().registerItem(                                   \
            MenuItemDesc{{(ID_STR), (PARENT_ID), (ORDER)}, (TITLE_STR),         \
                         MenuItemDesc::Callback(CALLBACK)},                      \
            false);                                                              \
    )

/**
 * @brief Register a menu item with shortcut
 */
#define REGISTER_MENU_ITEM_SHORTCUT(ID_STR, TITLE_STR, PARENT_ID, ORDER, SHORTCUT, CALLBACK) \
    MENU_REG_AUTOREG(                                                            \
        MenuRegistry::instance().registerItem(                                   \
            MenuItemDesc{{(ID_STR), (PARENT_ID), (ORDER)}, (TITLE_STR),         \
                         MenuItemDesc::Callback(CALLBACK), {}, {}, (SHORTCUT)}, \
            false);                                                              \
    )

/**
 * @brief Register a checkable menu item
 *
 * @param ID_STR Unique ID
 * @param TITLE_STR Display title
 * @param PARENT_ID Parent container ID
 * @param ORDER Sort order
 * @param IS_CHECKED Lambda: (ServiceContainer&) -> bool
 * @param CALLBACK Lambda: (ServiceContainer&) -> void
 */
#define REGISTER_MENU_ITEM_CHECKED(ID_STR, TITLE_STR, PARENT_ID, ORDER, IS_CHECKED, CALLBACK) \
    MENU_REG_AUTOREG(                                                            \
        MenuRegistry::instance().registerItem(                                   \
            MenuItemDesc{{(ID_STR), (PARENT_ID), (ORDER)}, (TITLE_STR),         \
                         MenuItemDesc::Callback(CALLBACK),                       \
                         MenuItemDesc::IsChecked(IS_CHECKED)},                   \
            false);                                                              \
    )
