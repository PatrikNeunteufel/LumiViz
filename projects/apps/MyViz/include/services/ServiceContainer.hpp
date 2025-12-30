/**
 ****************************************************************************************
 * @file   ServiceContainer.hpp
 * @brief  Dependency Injection Container - Qt6 Tutorial
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 * @version 1.0.0
 *
 * @details
 * ## Qt6 Tutorial: Dependency Injection
 *
 * Der ServiceContainer ist das Herzstück der Dependency Injection (DI) Architektur.
 * Er ermöglicht:
 *   - **Lose Kopplung:** Komponenten kennen nur Interfaces, nicht Implementierungen
 *   - **Testbarkeit:** Einfaches Austauschen von Implementierungen für Tests
 *   - **Singleton-Management:** Zentrale Verwaltung von Service-Instanzen
 *
 * ### Warum DI mit Qt?
 *
 * Qt hat bereits ein Meta-Object-System, aber:
 *   - QObject-Hierarchie ist nicht ideal für Service-Lokation
 *   - Kein Type-Safe Interface-zu-Implementierung Mapping
 *   - Kein Lifetime-Management (Singleton vs. Transient)
 *
 * Dieser Container ergänzt Qt um diese Features.
 *
 * ### Verwendung
 *
 * ```cpp
 * ServiceContainer container;
 *
 * // 1. Registrierung: Interface → Implementierung
 * container.registerSingleton<IEventBus, EventBus>();
 * container.registerSingleton<IAudioService, AudioService>();
 *
 * // 2. Registrierung mit Factory (für komplexe Konstruktion)
 * container.registerSingleton<IDatabase>([](ServiceContainer& c) {
 *     return std::make_unique<SqlDatabase>(c.resolve<ILogger>());
 * });
 *
 * // 3. Registrierung einer existierenden Instanz
 * container.registerInstance<ILogger>(std::move(myLogger));
 *
 * // 4. Auflösung
 * auto& eventBus = container.resolve<IEventBus>();
 *
 * // 5. Sichere Auflösung (nullptr wenn nicht registriert)
 * auto* db = container.tryResolve<IDatabase>();
 * ```
 *
 * @see ServiceContainer.md für ausführliche Dokumentation
 ****************************************************************************************
 */

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <stdexcept>
#include <mutex>

// =============================================================================
// ServiceContainer
// =============================================================================

/**
 * @class ServiceContainer
 * @brief Type-safe Dependency Injection Container
 *
 * Unterstützt:
 *   - **Singleton:** Gleiche Instanz bei jeder Auflösung
 *   - **Transient:** Neue Instanz bei jeder Auflösung
 *   - **Factory:** Lazy Instantiation mit Zugriff auf Container
 *   - **Instance:** Direktes Registrieren einer existierenden Instanz
 *
 * Thread-Sicherheit:
 *   - Registrierung: Thread-safe (mutex-geschützt)
 *   - Auflösung: Thread-safe für Singletons nach Initialisierung
 */
class ServiceContainer
{
public:
    // =========================================================================
    // Construction
    // =========================================================================

    ServiceContainer() = default;
    ~ServiceContainer() = default;

    // Non-copyable, non-movable (mutex is not movable)
    ServiceContainer(const ServiceContainer&) = delete;
    ServiceContainer& operator=(const ServiceContainer&) = delete;
    ServiceContainer(ServiceContainer&&) = delete;
    ServiceContainer& operator=(ServiceContainer&&) = delete;

    // =========================================================================
    // Registration: Interface → Implementation
    // =========================================================================

    /**
     * @brief Registriert einen Singleton-Service (Interface → Implementation)
     *
     * @tparam TInterface Interface-Typ
     * @tparam TImpl Implementierungs-Typ (muss von TInterface erben)
     *
     * @code
     * container.registerSingleton<IEventBus, EventBus>();
     * @endcode
     */
    template<typename TInterface, typename TImpl>
    void registerSingleton()
    {
        static_assert(std::is_base_of_v<TInterface, TImpl>,
            "TImpl must derive from TInterface");

        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(typeid(TInterface));
        m_factories[key] = [](ServiceContainer&) -> InstancePtr {
            return {new TImpl(), [](void* p) { delete static_cast<TImpl*>(p); }};
        };
        m_instances.erase(key);
        m_isTransient[key] = false;
    }

    /**
     * @brief Registriert einen Singleton mit Factory-Funktion
     *
     * Die Factory erhält eine Referenz auf den Container für Abhängigkeiten.
     *
     * @tparam TInterface Interface-Typ
     * @param factory Factory-Funktion: (ServiceContainer&) → unique_ptr<TInterface>
     *
     * @code
     * container.registerSingleton<IDatabase>([](ServiceContainer& c) {
     *     return std::make_unique<SqlDatabase>(c.resolve<ILogger>());
     * });
     * @endcode
     */
    template<typename TInterface>
    void registerSingleton(std::function<std::unique_ptr<TInterface>(ServiceContainer&)> factory)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(typeid(TInterface));
        m_factories[key] = [factory = std::move(factory)](ServiceContainer& c) -> InstancePtr {
            auto instance = factory(c);
            return {instance.release(), [](void* p) { delete static_cast<TInterface*>(p); }};
        };
        m_instances.erase(key);
        m_isTransient[key] = false;
    }

    /**
     * @brief Registriert eine existierende Instanz als Singleton
     *
     * @tparam TInterface Interface-Typ
     * @param instance Existierende Instanz (Ownership wird übernommen)
     *
     * @code
     * auto logger = std::make_unique<FileLogger>("app.log");
     * container.registerInstance<ILogger>(std::move(logger));
     * @endcode
     */
    template<typename TInterface>
    void registerInstance(std::unique_ptr<TInterface> instance)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(typeid(TInterface));
        InstancePtr ptr{instance.release(), [](void* p) { delete static_cast<TInterface*>(p); }};
        m_instances.insert_or_assign(key, std::move(ptr));
        m_factories.erase(key);
        m_isTransient[key] = false;
    }

    /**
     * @brief Registriert einen Transient-Service (neue Instanz bei jeder Auflösung)
     *
     * @tparam TInterface Interface-Typ
     * @tparam TImpl Implementierungs-Typ
     *
     * @code
     * container.registerTransient<IWorker, BackgroundWorker>();
     * @endcode
     */
    template<typename TInterface, typename TImpl>
    void registerTransient()
    {
        static_assert(std::is_base_of_v<TInterface, TImpl>,
            "TImpl must derive from TInterface");

        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(typeid(TInterface));
        m_factories[key] = [](ServiceContainer&) -> InstancePtr {
            return {new TImpl(), [](void* p) { delete static_cast<TImpl*>(p); }};
        };
        m_instances.erase(key);
        m_isTransient[key] = true;
    }

    // =========================================================================
    // Resolution
    // =========================================================================

    /**
     * @brief Löst einen Service auf
     *
     * @tparam T Service-Typ
     * @return Referenz auf den Service
     * @throws std::runtime_error wenn Service nicht registriert
     *
     * @code
     * auto& eventBus = container.resolve<IEventBus>();
     * @endcode
     */
    template<typename T>
    T& resolve()
    {
        auto* ptr = tryResolve<T>();
        if (ptr == nullptr)
        {
            throw std::runtime_error(
                std::string("Service not registered: ") + typeid(T).name());
        }
        return *ptr;
    }

    /**
     * @brief Versucht einen Service aufzulösen
     *
     * @tparam T Service-Typ
     * @return Pointer auf Service oder nullptr wenn nicht registriert
     *
     * @code
     * if (auto* db = container.tryResolve<IDatabase>()) {
     *     db->connect();
     * }
     * @endcode
     */
    template<typename T>
    T* tryResolve()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(typeid(T));

        // Bereits instantiiert?
        auto instanceIt = m_instances.find(key);
        if (instanceIt != m_instances.end())
        {
            return static_cast<T*>(instanceIt->second.get());
        }

        // Factory vorhanden?
        auto factoryIt = m_factories.find(key);
        if (factoryIt == m_factories.end())
        {
            return nullptr;
        }

        // Instanz erstellen
        auto instance = factoryIt->second(*this);
        T* ptr = static_cast<T*>(instance.get());

        // Bei Singleton speichern
        auto transientIt = m_isTransient.find(key);
        if (transientIt == m_isTransient.end() || !transientIt->second)
        {
            m_instances.insert_or_assign(key, std::move(instance));
        }

        return ptr;
    }

    /**
     * @brief Erstellt eine neue Transient-Instanz
     *
     * @tparam T Service-Typ
     * @return unique_ptr auf neue Instanz
     * @throws std::runtime_error wenn Service nicht registriert
     *
     * @code
     * auto worker = container.createTransient<IWorker>();
     * @endcode
     */
    template<typename T>
    std::unique_ptr<T> createTransient()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(typeid(T));

        auto factoryIt = m_factories.find(key);
        if (factoryIt == m_factories.end())
        {
            throw std::runtime_error(
                std::string("Service not registered: ") + typeid(T).name());
        }

        auto instance = factoryIt->second(*this);
        T* ptr = static_cast<T*>(instance.release());
        return std::unique_ptr<T>(ptr);
    }

    // =========================================================================
    // Query
    // =========================================================================

    /**
     * @brief Prüft ob ein Service registriert ist
     *
     * @tparam T Service-Typ
     * @return true wenn registriert
     */
    template<typename T>
    [[nodiscard]] bool has() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(typeid(T));
        return m_factories.find(key) != m_factories.end() ||
               m_instances.find(key) != m_instances.end();
    }

    // =========================================================================
    // Management
    // =========================================================================

    /**
     * @brief Entfernt einen Service
     *
     * @tparam T Service-Typ
     */
    template<typename T>
    void unregister()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::type_index(typeid(T));
        m_factories.erase(key);
        m_instances.erase(key);
        m_isTransient.erase(key);
    }

    /**
     * @brief Entfernt alle Registrierungen
     */
    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_factories.clear();
        m_instances.clear();
        m_isTransient.clear();
    }

private:
    // =========================================================================
    // Private Types
    // =========================================================================

    using Deleter = void(*)(void*);
    using InstancePtr = std::unique_ptr<void, Deleter>;
    using Factory = std::function<InstancePtr(ServiceContainer&)>;

    // =========================================================================
    // Private Members
    // =========================================================================

    mutable std::mutex m_mutex;
    std::unordered_map<std::type_index, Factory> m_factories;
    std::unordered_map<std::type_index, InstancePtr> m_instances;
    std::unordered_map<std::type_index, bool> m_isTransient;
};
