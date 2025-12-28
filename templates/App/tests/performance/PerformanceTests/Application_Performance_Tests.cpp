/**
 ****************************************************************************************
 * @file   Application_Performance_Tests.cpp
 * @brief  Application Performance Tests
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

#include <algorithm>
#include <chrono>
#include <numeric>
#include <vector>

// =============================================================================
// Performance Test Utilities
// =============================================================================
//
// Performance tests measure execution time and resource usage.
// They typically:
//   - Run operations multiple times for statistical significance
//   - Measure wall clock time, CPU time, memory usage
//   - Compare against baseline/threshold values
//   - May be skipped in regular CI (run nightly)
//
// =============================================================================

namespace
{

// Performance test configuration
constexpr int WARMUP_ITERATIONS = 3;
constexpr int MEASURE_ITERATIONS = 10;
constexpr double MAX_INIT_TIME_MS = 100.0;
constexpr double MAX_SHUTDOWN_TIME_MS = 50.0;

// Simulated command line arguments
char arg0[] = "PerfTestApp";
char* testArgv[] = {arg0, nullptr};
int testArgc = 1;

/**
 * @brief Measures execution time of a callable.
 * @tparam Func Callable type
 * @param func Function to measure
 * @return Execution time in milliseconds
 */
template<typename Func>
double measureTimeMs(Func&& func)
{
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::milli>(end - start).count();
}

/**
 * @brief Calculates statistics for a vector of measurements.
 */
struct Statistics
{
    double min{0.0};
    double max{0.0};
    double mean{0.0};
    double median{0.0};

    static Statistics calculate(std::vector<double>& values)
    {
        Statistics stats;
        if (values.empty())
        {
            return stats;
        }

        std::sort(values.begin(), values.end());

        stats.min = values.front();
        stats.max = values.back();
        stats.mean = std::accumulate(values.begin(), values.end(), 0.0)
                     / static_cast<double>(values.size());

        size_t mid = values.size() / 2;
        stats.median = (values.size() % 2 == 0)
                           ? (values[mid - 1] + values[mid]) / 2.0
                           : values[mid];

        return stats;
    }
};

} // namespace

// =============================================================================
// Initialization Performance Tests
// =============================================================================

TEST_SUITE("Application Performance")
{

    TEST_CASE("Init Performance")
    {
        std::vector<double> timings;
        timings.reserve(MEASURE_ITERATIONS);

        // Warmup
        for (int i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            Application app;
            (void)app.init(testArgc, testArgv);
            app.shutdown();
        }

        // Measure
        for (int i = 0; i < MEASURE_ITERATIONS; ++i)
        {
            Application app;

            double timeMs = measureTimeMs([&]()
            {
                (void)app.init(testArgc, testArgv);
            });

            timings.push_back(timeMs);
            app.shutdown();
        }

        auto stats = Statistics::calculate(timings);

        MESSAGE("Init Performance (ms):");
        MESSAGE("  Min:    " << stats.min);
        MESSAGE("  Max:    " << stats.max);
        MESSAGE("  Mean:   " << stats.mean);
        MESSAGE("  Median: " << stats.median);

        CHECK(stats.median < MAX_INIT_TIME_MS);
    }

    TEST_CASE("Shutdown Performance")
    {
        std::vector<double> timings;
        timings.reserve(MEASURE_ITERATIONS);

        // Warmup
        for (int i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            Application app;
            (void)app.init(testArgc, testArgv);
            app.shutdown();
        }

        // Measure
        for (int i = 0; i < MEASURE_ITERATIONS; ++i)
        {
            Application app;
            (void)app.init(testArgc, testArgv);

            double timeMs = measureTimeMs([&]()
            {
                app.shutdown();
            });

            timings.push_back(timeMs);
        }

        auto stats = Statistics::calculate(timings);

        MESSAGE("Shutdown Performance (ms):");
        MESSAGE("  Min:    " << stats.min);
        MESSAGE("  Max:    " << stats.max);
        MESSAGE("  Mean:   " << stats.mean);
        MESSAGE("  Median: " << stats.median);

        CHECK(stats.median < MAX_SHUTDOWN_TIME_MS);
    }

// =============================================================================
// Full Lifecycle Performance Tests
// =============================================================================

    TEST_CASE("Full Lifecycle Performance")
    {
        std::vector<double> timings;
        timings.reserve(MEASURE_ITERATIONS);

        // Warmup
        for (int i = 0; i < WARMUP_ITERATIONS; ++i)
        {
            Application app;
            (void)app.init(testArgc, testArgv);
            app.shutdown();
        }

        // Measure
        for (int i = 0; i < MEASURE_ITERATIONS; ++i)
        {
            double timeMs = measureTimeMs([&]()
            {
                Application app;
                (void)app.init(testArgc, testArgv);
                app.shutdown();
            });

            timings.push_back(timeMs);
        }

        auto stats = Statistics::calculate(timings);

        MESSAGE("Full Lifecycle Performance (ms):");
        MESSAGE("  Min:    " << stats.min);
        MESSAGE("  Max:    " << stats.max);
        MESSAGE("  Mean:   " << stats.mean);
        MESSAGE("  Median: " << stats.median);

        // Full lifecycle should be within combined thresholds
        CHECK(stats.median < (MAX_INIT_TIME_MS + MAX_SHUTDOWN_TIME_MS));
    }

// =============================================================================
// Memory Performance Tests (Placeholder)
// =============================================================================

    TEST_CASE("Memory Usage" * doctest::skip(true))
    {
        // Skip by default - requires platform-specific memory tracking
        //
        // Example implementation:
        // size_t memBefore = getCurrentMemoryUsage();
        // {
        //     Application app;
        //     (void)app.init(testArgc, testArgv);
        //     size_t memDuring = getCurrentMemoryUsage();
        //     CHECK(memDuring - memBefore < MAX_MEMORY_BYTES);
        // }
        // size_t memAfter = getCurrentMemoryUsage();
        // CHECK(memAfter <= memBefore + MEMORY_TOLERANCE);
    }

} // TEST_SUITE
