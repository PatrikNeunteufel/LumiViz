/**
 ****************************************************************************************
 * @file   Application.cpp
 * @brief  Application Implementation - Qt6 Tutorial
 *         Demonstrates QApplication and MainWindow integration
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

 // =============================================================================
 // Includes
 // =============================================================================

#include "pch.h"
#include "Application.hpp"
#include "UI/MainWindow.hpp"

// Qt includes
#include <QApplication>

// =============================================================================
// Private Implementation (Pimpl Pattern)
// =============================================================================
// The Pimpl (Pointer to Implementation) pattern hides implementation details
// from the header file. Benefits:
// - Faster compilation (changes in Impl don't require recompiling users)
// - Better encapsulation (private members truly hidden)
// - Stable ABI (binary compatibility when Impl changes)

struct Application::Impl
{
    // -------------------------------------------------------------------------
    // Application Metadata
    // -------------------------------------------------------------------------
    std::string name{ "MyViz" };
    std::string version{ "0.1.0" };
    std::vector<std::string> args;

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Qt Objects
    // -------------------------------------------------------------------------
    // QApplication: The Qt application object. MUST exist before any widgets.
    //               Manages application-wide resources and event loop.
    //
    // MainWindow:   Our main window. Created after QApplication.
    //
    // We use unique_ptr for automatic cleanup in correct order:
    //   1. MainWindow destroyed first (it's a Qt widget)
    //   2. QApplication destroyed last (manages Qt system)

    std::unique_ptr<QApplication> pQtApp;
    std::unique_ptr<MainWindow> pMainWindow;
};

// =============================================================================
// Lifecycle
// =============================================================================

Application::Application()
    : m_impl{ std::make_unique<Impl>() }
{
    // Pimpl is created here. No Qt objects yet - they need argc/argv.
}

Application::~Application()
{
    // Ensure proper shutdown even if user forgets to call shutdown()
    if (m_initialized)
    {
        shutdown();
    }
}

bool Application::init(int argc, char* argv[])
{
    // -------------------------------------------------------------------------
    // Guard: Prevent double initialization
    // -------------------------------------------------------------------------
    if (m_initialized)
    {
        std::cerr << "[Application] Already initialized\n";
        return false;
    }

    // -------------------------------------------------------------------------
    // Store command line arguments
    // -------------------------------------------------------------------------
    m_impl->args.clear();
    m_impl->args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i)
    {
        m_impl->args.emplace_back(argv[i]);
    }

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: QApplication Creation
    // -------------------------------------------------------------------------
    // QApplication MUST be created before any QWidget.
    // It initializes the Qt system and manages:
    //   - Event loop
    //   - Application-wide settings
    //   - Style and palette
    //   - Clipboard
    //   - Font database
    //
    // argc/argv are passed by reference - Qt may modify them!

    m_impl->pQtApp = std::make_unique<QApplication>(argc, argv);

    // Set application metadata (shown in About dialogs, etc.)
    m_impl->pQtApp->setApplicationName(
        QString::fromStdString(m_impl->name));
    m_impl->pQtApp->setApplicationVersion(
        QString::fromStdString(m_impl->version));
    m_impl->pQtApp->setOrganizationName(QStringLiteral("MyViz Project"));

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: MainWindow Creation
    // -------------------------------------------------------------------------
    // Now we can create widgets. MainWindow is our top-level window.
    // show() makes it visible - without this, the window exists but is hidden.

    m_impl->pMainWindow = std::make_unique<MainWindow>();
    m_impl->pMainWindow->show();

    // -------------------------------------------------------------------------
    // Initialization complete
    // -------------------------------------------------------------------------
    std::cout << "[Application] " << m_impl->name
        << " v" << m_impl->version
        << " initialized\n";

    m_initialized = true;
    return true;
}

int Application::run()
{
    // -------------------------------------------------------------------------
    // Guards
    // -------------------------------------------------------------------------
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

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Event Loop
    // -------------------------------------------------------------------------
    // QApplication::exec() starts the Qt event loop. This is BLOCKING.
    // The event loop:
    //   - Processes user input (mouse, keyboard)
    //   - Handles window events (resize, paint, close)
    //   - Delivers signals to slots
    //   - Manages timers
    //
    // exec() returns when:
    //   - QApplication::quit() is called
    //   - Last window with Qt::WA_QuitOnClose is closed
    //   - QCoreApplication::exit(code) is called
    //
    // Return value is the exit code (0 = success).

    int result = m_impl->pQtApp->exec();

    m_running = false;
    return result;
}

void Application::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    std::cout << "[Application] Shutting down...\n";

    // -------------------------------------------------------------------------
    // Qt6 Tutorial: Cleanup Order
    // -------------------------------------------------------------------------
    // Cleanup must happen in reverse order of creation:
    // 1. First: Destroy widgets (MainWindow)
    // 2. Last:  Destroy QApplication
    //
    // unique_ptr::reset() destroys the object and sets pointer to nullptr.
    // This is important because Qt checks if QApplication exists.

    m_impl->pMainWindow.reset();  // Destroy MainWindow first
    m_impl->pQtApp.reset();       // Destroy QApplication last

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