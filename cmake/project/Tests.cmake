# cmake/project/Tests.cmake
# ==========================
# Test pipeline orchestrator - iterates over tests and creates targets
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - cmake/core/Context.cmake
#   - cmake/core/Errors.cmake
#   - cmake/core/Debug.cmake
#   - cmake/core/Json.cmake
#   - cmake/project/Solution.cmake
#
# Auto-loads:
#   - cmake/project/TestCollect.cmake
#   - cmake/project/TestCreate.cmake
#
# Activation:
#   - Tests are only built when BUILD_TESTS=ON
#
# Used by:
#   - CMakeLists.txt (main build)

include_guard(GLOBAL)

# Load sub-modules
include(cmake/project/TestCollect.cmake)
include(cmake/project/TestCreate.cmake)

# ==============================================================================
# Main Entry Point
# ==============================================================================

message(STATUS "[Tests] === Test Pipeline Start ===")

# Check if tests should be built
if(NOT BUILD_TESTS)
    message(STATUS "[Tests] BUILD_TESTS is OFF, skipping tests")
    message(STATUS "[Tests] === Test Pipeline Complete (skipped) ===")
    return()
endif()

# Get tests array from Solution
get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)

# Check if tests array exists
_json_has_key("${_solution_json}" "tests" _has_tests)
if(NOT _has_tests)
    message(STATUS "[Tests] No 'tests' array in Solution.json")
    message(STATUS "[Tests] === Test Pipeline Complete ===")
    return()
endif()

# Get tests array
string(JSON _tests_json GET "${_solution_json}" "tests")
string(JSON _tests_count LENGTH "${_tests_json}")

if(_tests_count EQUAL 0)
    message(STATUS "[Tests] tests array is empty")
    message(STATUS "[Tests] === Test Pipeline Complete ===")
    return()
endif()

message(STATUS "[Tests] Processing ${_tests_count} test(s)...")

# Enable CTest
include(CTest)
enable_testing()

# ==============================================================================
# Iterate Over Tests
# ==============================================================================

math(EXPR _last_idx "${_tests_count} - 1")

foreach(_idx RANGE 0 ${_last_idx})
    # Get test JSON
    string(JSON _test_json GET "${_tests_json}" ${_idx})
    
    # Get name for logging
    string(JSON _test_name GET "${_test_json}" "name")
    
    message(STATUS "[Tests] --- Processing: ${_test_name} ---")
    
    # Create context
    ctx_create(TEST_${_idx})
    
    # Collect test data
    _collect_test("${_test_json}" TEST_${_idx})
    
    # Check skip flag
    ctx_get(TEST_${_idx} SKIP _skip)
    if(_skip)
        message(STATUS "[Tests]   Skipped (skip=true)")
        continue()
    endif()
    
    # Check platform filter
    ctx_get(TEST_${_idx} PLATFORMS _platforms)
    if(_platforms)
        _check_platform_filter("${_platforms}" _platform_ok)
        if(NOT _platform_ok)
            message(STATUS "[Tests]   Skipped (platform filter)")
            continue()
        endif()
    endif()
    
    # Create test target
    _create_test_target(TEST_${_idx})
    
    message(STATUS "[Tests]   Created: ${_test_name}")
endforeach()

# ==============================================================================
# Summary
# ==============================================================================

message(STATUS "")
message(STATUS "[Tests] === Test Pipeline Complete ===")

# Print CTest info
message(STATUS "[Tests] Run tests with: ctest --test-dir <build-dir>")
message(STATUS "[Tests] Filter by label: ctest -L unit")
message(STATUS "[Tests] Filter by name:  ctest -R <pattern>")
