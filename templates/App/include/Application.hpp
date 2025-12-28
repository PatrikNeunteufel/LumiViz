/**
 ****************************************************************************************
 * @file   Application.hpp
 * @brief  Application Interface
 *         CMake Architecture V2 - App-Container Template
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

/**
 * @class Application
 * @brief Central entry point for application logic.
 *
 * This class is instantiated and controlled by the generic main.cpp.
 * All application-specific logic should be implemented here.
 *
 * For GUI applications (Qt):
 *   - init(): Create QApplication, build MainWindow
 *   - run():  QApplication::exec()
 *   - shutdown(): Cleanup
 *
 * For Console applications:
 *   - init(): Load configuration, start services
 *   - run():  Main logic or event loop
 *   - shutdown(): Cleanup
 */
class Application
{
public:
    Application();
    ~Application();

    // Non-copyable, non-movable (singleton-like)
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    // =========================================================================
    // Lifecycle
    // =========================================================================

    /**
     * @brief Initializes the application.
     * @param argc Number of command line arguments
     * @param argv Command line arguments
     * @return true on success, false on error
     */
    [[nodiscard]] bool init(int argc, char* argv[]);

    /**
     * @brief Starts the main loop.
     * @return Exit code (0 = success)
     */
    [[nodiscard]] int run();

    /**
     * @brief Shuts down the application and releases resources.
     */
    void shutdown();

    // =========================================================================
    // Accessors
    // =========================================================================

    /**
     * @brief Gets the application name.
     * @return Application name
     */
    [[nodiscard]] const std::string& name() const noexcept;

    /**
     * @brief Gets the application version.
     * @return Application version
     */
    [[nodiscard]] const std::string& version() const noexcept;

    /**
     * @brief Checks if the application is initialized.
     * @return true if initialized
     */
    [[nodiscard]] bool isInitialized() const noexcept;

    /**
     * @brief Checks if the application is running.
     * @return true if running
     */
    [[nodiscard]] bool isRunning() const noexcept;

private:
    // =========================================================================
    // Private Implementation (Pimpl)
    // =========================================================================

    struct Impl;
    std::unique_ptr<Impl> m_impl;

    bool m_initialized{false};
    bool m_running{false};
};
