/**
 ****************************************************************************************
 * @file   main.cpp
 * @brief  Generic Application Entry Point
 *         CMake Architecture V2 - App-Container Template
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// =============================================================================
// Generic Application Entry Point
// CMake Architecture V2 - App-Container Template
// =============================================================================
//
// This main.cpp is identical for all App-Containers.
// All application logic resides in Application (src/).
//
// Build system automatically sets:
//   - APP_GUI    : For runner.type = "GUI" (Windows: WinMain)
//   - APP_CONSOLE: For runner.type = "CONSOLE"
//
// NOTE: If PCH is disabled in Solution.json, remove the #include "pch.h" line.
//
// =============================================================================

#include "Application.hpp"

#if defined(_WIN32)
    #include <Windows.h>
#endif

// =============================================================================
// Common Entry Point
// =============================================================================

namespace
{

int commonMain(int argc, char* argv[])
{
    Application app;

    // Initialization (Qt, Audio, Config, etc.)
    if (!app.init(argc, argv))
    {
        return 1;
    }

    // Main loop (Qt: exec(), Console: custom loop)
    int result = app.run();

    // Cleanup
    app.shutdown();

    return result;
}

} // namespace

// =============================================================================
// Platform-specific Entry Points
// =============================================================================

#if defined(_WIN32) && defined(APP_WINDOWS_GUI)

// Windows GUI: WinMain entry point (no console window)
int WINAPI WinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow
)
{
    return commonMain(__argc, __argv);
}

#else

// Console / Linux / macOS: Standard main
int main(int argc, char* argv[])
{
    return commonMain(argc, argv);
}

#endif
