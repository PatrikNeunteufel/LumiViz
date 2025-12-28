/**
 ****************************************************************************************
 * @file   Application.cpp
 * @brief  Application Implementation
 *         CMake Architecture V2 - App-Container Template
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// NOTE: If PCH is disabled in Solution.json, remove the #include "pch.h" line
#include "pch.h"
#include "Application.hpp"

#include <iostream>

// =============================================================================
// Private Implementation (Pimpl)
// =============================================================================

struct Application::Impl
{
    std::string name{"AppTemplate"};
    std::string version{"0.1.0"};
    std::vector<std::string> args;

    // Extend with project-specific members:
    // std::unique_ptr<QApplication> qtApp;
    // std::unique_ptr<MainWindow> mainWindow;
    // std::unique_ptr<AudioEngine> audioEngine;
};

// =============================================================================
// Lifecycle
// =============================================================================

Application::Application()
    : m_impl{std::make_unique<Impl>()}
{
}

Application::~Application()
{
    if (m_initialized)
    {
        shutdown();
    }
}

bool Application::init(int argc, char* argv[])
{
    if (m_initialized)
    {
        std::cerr << "[Application] Already initialized\n";
        return false;
    }

    // Store command line arguments
    m_impl->args.clear();
    m_impl->args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i)
    {
        m_impl->args.emplace_back(argv[i]);
    }

    // =========================================================================
    // Add project-specific initialization here:
    // =========================================================================

    // Example for Qt GUI:
    // m_impl->qtApp = std::make_unique<QApplication>(argc, argv);
    // m_impl->mainWindow = std::make_unique<MainWindow>();
    // m_impl->mainWindow->show();

    // Example for Audio:
    // m_impl->audioEngine = std::make_unique<AudioEngine>();
    // if (!m_impl->audioEngine->init())
    // {
    //     std::cerr << "[Application] Audio init failed\n";
    //     return false;
    // }

    std::cout << "[Application] " << m_impl->name
              << " v" << m_impl->version
              << " initialized\n";

    m_initialized = true;
    return true;
}

int Application::run()
{
    if (!m_initialized)
    {
        std::cerr << "[Application] Not initialized\n";
        return 1;
    }

    if (m_running)
    {
        std::cerr << "[Application] Already running\n";
        return 1;
    }

    m_running = true;

    // =========================================================================
    // Add project-specific main loop here:
    // =========================================================================

    // Example for Qt GUI:
    // return m_impl->qtApp->exec();

    // Example for Console:
    std::cout << "[Application] Running...\n";
    std::cout << "[Application] Press Enter to exit.\n";
    std::cin.get();

    m_running = false;
    return 0;
}

void Application::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    std::cout << "[Application] Shutting down...\n";

    // =========================================================================
    // Add project-specific cleanup here:
    // =========================================================================

    // Example:
    // m_impl->mainWindow.reset();
    // m_impl->audioEngine.reset();
    // m_impl->qtApp.reset();

    m_running = false;
    m_initialized = false;

    std::cout << "[Application] Shutdown complete\n";
}

// =============================================================================
// Accessors
// =============================================================================

const std::string& Application::name() const noexcept
{
    return m_impl->name;
}

const std::string& Application::version() const noexcept
{
    return m_impl->version;
}

bool Application::isInitialized() const noexcept
{
    return m_initialized;
}

bool Application::isRunning() const noexcept
{
    return m_running;
}
