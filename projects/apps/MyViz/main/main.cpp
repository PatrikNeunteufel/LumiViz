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
// Entry Point
// =============================================================================
// On Windows GUI applications, Qt6EntryPoint library provides wWinMain
// which then calls main(). So we only need to define main() here.
//
// This works on all platforms:
//   - Windows GUI: Qt6EntryPoint::wWinMain → main()
//   - Windows Console: main() directly
//   - Linux/macOS: main() directly

int main(int argc, char* argv[])
{
    return commonMain(argc, argv);
}
