/**
 ****************************************************************************************
 * @file   Application_Integration_Tests.cpp
 * @brief  Application Integration Tests
 *         CMake Architecture V2 - App-Container Template
 *
 * @author Patrik Neunteufel
 * @date   December 2025
 ****************************************************************************************
 */

// NOTE: If PCH is disabled in Solution.json, remove the #include "pch.h" line
#include "pch.h"

#include <doctest.h>
#include "Application.hpp"

#include <chrono>
#include <thread>

// =============================================================================
// Integration Test Fixtures
// =============================================================================
//
// Integration tests verify the interaction between components.
// They typically:
//   - Test real I/O operations (file system, network, database)
//   - Test component interactions
//   - Use longer timeouts than unit tests
//   - May require external resources
//
// =============================================================================

namespace
{

// Test configuration
[[maybe_unused]] constexpr int INTEGRATION_TIMEOUT_MS = 5000;

// Simulated command line arguments
char arg0[] = "IntegrationTestApp";
char* testArgv[] = {arg0, nullptr};
int testArgc = 1;

} // namespace

// =============================================================================
// Full Lifecycle Integration Tests
// =============================================================================

TEST_SUITE("Application Integration")
{

    TEST_CASE("Full Lifecycle")
    {
        Application app;

        // Phase 1: Construction
        CHECK_FALSE(app.isInitialized());
        CHECK_FALSE(app.isRunning());

        // Phase 2: Initialization
        REQUIRE(app.init(testArgc, testArgv));
        CHECK(app.isInitialized());
        CHECK_FALSE(app.isRunning());

        // Phase 3: Shutdown
        app.shutdown();
        CHECK_FALSE(app.isInitialized());
        CHECK_FALSE(app.isRunning());
    }

    TEST_CASE("Multiple Init-Shutdown Cycles")
    {
        Application app;

        for (int cycle = 0; cycle < 3; ++cycle)
        {
            CAPTURE(cycle);

            REQUIRE(app.init(testArgc, testArgv));
            CHECK(app.isInitialized());

            app.shutdown();
            CHECK_FALSE(app.isInitialized());
        }
    }

// =============================================================================
// Resource Management Tests
// =============================================================================

    TEST_CASE("Resource Cleanup on Destruction")
    {
        // Create multiple applications in sequence
        // to verify resources are properly released
        for (int i = 0; i < 5; ++i)
        {
            Application app;
            REQUIRE(app.init(testArgc, testArgv));
            // Destructor should clean up
        }

        // If we get here without crashes or leaks, test passed
        CHECK(true);
    }

// =============================================================================
// Error Recovery Tests
// =============================================================================

    TEST_CASE("Recovery After Failed Init")
    {
        // This test would be extended with mock objects
        // to simulate init failures and verify recovery

        Application app;

        // First init succeeds
        CHECK(app.init(testArgc, testArgv));

        // Second init fails (already initialized)
        CHECK_FALSE(app.init(testArgc, testArgv));

        // Application should still be usable
        CHECK(app.isInitialized());

        // Shutdown should work
        app.shutdown();
        CHECK_FALSE(app.isInitialized());
    }

// =============================================================================
// Stress Tests (Optional)
// =============================================================================

    TEST_CASE("Rapid Init-Shutdown" * doctest::skip(true))
    {
        // Skip by default - enable for stress testing
        for (int i = 0; i < 100; ++i)
        {
            Application app;
            REQUIRE(app.init(testArgc, testArgv));
            app.shutdown();
        }
    }

} // TEST_SUITE

// =============================================================================
// External Resource Tests (Examples)
// =============================================================================
//
// Uncomment and extend these tests based on your application's needs:
//
// TEST_SUITE("Application External Resources")
// {
//     TEST_CASE("File System Access")
//     {
//         // Test file I/O operations
//     }
//
//     TEST_CASE("Network Connectivity")
//     {
//         // Test network operations
//     }
//
//     TEST_CASE("Database Connection")
//     {
//         // Test database operations
//     }
// }
// =============================================================================
