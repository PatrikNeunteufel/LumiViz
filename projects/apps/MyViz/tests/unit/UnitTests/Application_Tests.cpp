/**
 ****************************************************************************************
 * @file   Application_Tests.cpp
 * @brief  Application Unit Tests
 *         CMake Architecture V2 - App-Container Template
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

#include <doctest.h>
#include "Application.hpp"

// =============================================================================
// Test Fixtures
// =============================================================================

namespace
{

// Simulated command line arguments
char arg0[] = "TestApp";
char* testArgv[] = {arg0, nullptr};
int testArgc = 1;

} // namespace

// =============================================================================
// Construction Tests
// =============================================================================

TEST_SUITE("Application")
{

    TEST_CASE("Default Construction")
    {
        Application app;

        CHECK_FALSE(app.isInitialized());
        CHECK_FALSE(app.isRunning());
    }

    TEST_CASE("Name and Version")
    {
        Application app;

        // Default values from template
        CHECK_FALSE(app.name().empty());
        CHECK_FALSE(app.version().empty());
    }

// =============================================================================
// Lifecycle Tests
// =============================================================================

    TEST_CASE("Init Success")
    {
        Application app;

        CHECK(app.init(testArgc, testArgv));
        CHECK(app.isInitialized());
        CHECK_FALSE(app.isRunning());
    }

    TEST_CASE("Double Init Fails")
    {
        Application app;

        CHECK(app.init(testArgc, testArgv));
        CHECK_FALSE(app.init(testArgc, testArgv));  // Second init fails
    }

    TEST_CASE("Shutdown Without Init")
    {
        Application app;

        // Should not crash
        app.shutdown();
        CHECK_FALSE(app.isInitialized());
    }

    TEST_CASE("Shutdown After Init")
    {
        Application app;

        CHECK(app.init(testArgc, testArgv));
        app.shutdown();

        CHECK_FALSE(app.isInitialized());
        CHECK_FALSE(app.isRunning());
    }

    TEST_CASE("Destructor Calls Shutdown")
    {
        {
            Application app;
            CHECK(app.init(testArgc, testArgv));
            // Destructor will be called
        }
        // No crash = test passed
        CHECK(true);
    }

// =============================================================================
// Run Tests (without real event loop)
// =============================================================================

    TEST_CASE("Run Without Init Fails")
    {
        Application app;

        int result = app.run();
        CHECK(result != 0);  // Error expected
    }

// =============================================================================
// Non-Copyable / Non-Movable Tests
// =============================================================================

    TEST_CASE("Not Copyable")
    {
        CHECK_FALSE(std::is_copy_constructible_v<Application>);
        CHECK_FALSE(std::is_copy_assignable_v<Application>);
    }

    TEST_CASE("Not Movable")
    {
        CHECK_FALSE(std::is_move_constructible_v<Application>);
        CHECK_FALSE(std::is_move_assignable_v<Application>);
    }

} // TEST_SUITE
