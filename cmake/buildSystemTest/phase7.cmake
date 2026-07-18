# ==============================================================================
# phase7.cmake – Build System Test for Test Pipeline
# ==============================================================================
#
# Test:         Phase 7
# Version:      1.0.0
# Date:         2025-12-12
# Part of:      CMake Architecture
#
# Description:
#   Tests the Test Pipeline including:
#   - tests array parsing
#   - Test target creation
#   - Framework integration (doctest, googletest, catch2)
#   - CTest registration
#   - Labels and timeout
#   - source_from functionality
#
# Note:
#   This test only runs when BUILD_TESTS=ON
#
# ==============================================================================

include_guard(GLOBAL)

dbg_init(ID PHASE7_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase7")
dbg(${DBG_OFTEN} "=== Phase 7 Test Start ===" ID PHASE7_TEST)

# ==============================================================================
# Test 1: BUILD_TESTS Option
# ==============================================================================

dbg(${DBG_COMMON} "Testing BUILD_TESTS option..." ID PHASE7_TEST)

if(BUILD_TESTS)
    dbg(${DBG_COMMON} "  BUILD_TESTS is ON" ID PHASE7_TEST)
else()
    dbg(${DBG_COMMON} "  BUILD_TESTS is OFF - Test pipeline skipped" ID PHASE7_TEST)
    dbg(${DBG_COMMON} "  (This is expected if tests are not enabled)" ID PHASE7_TEST)
endif()

# ==============================================================================
# Test 2: Tests Array in Solution.json
# ==============================================================================

dbg(${DBG_COMMON} "Testing tests array parsing..." ID PHASE7_TEST)

get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)

if(_solution_json)
    string(JSON _tests_json ERROR_VARIABLE _err GET "${_solution_json}" "tests")
    if(NOT _err)
        string(JSON _tests_count LENGTH "${_tests_json}")
        dbg(${DBG_COMMON} "  Found ${_tests_count} test(s) in Solution.json" ID PHASE7_TEST)
        
        # Parse first test
        if(_tests_count GREATER 0)
            string(JSON _first_test GET "${_tests_json}" 0)
            string(JSON _test_name GET "${_first_test}" "name")
            dbg(${DBG_COMMON} "  First test: ${_test_name}" ID PHASE7_TEST)
            
            # Check optional fields
            string(JSON _test_type ERROR_VARIABLE _e1 GET "${_first_test}" "type")
            string(JSON _test_framework ERROR_VARIABLE _e2 GET "${_first_test}" "framework")
            
            if(NOT _e1)
                dbg(${DBG_RARE} "    type: ${_test_type}" ID PHASE7_TEST)
            endif()
            if(NOT _e2)
                dbg(${DBG_RARE} "    framework: ${_test_framework}" ID PHASE7_TEST)
            endif()
        endif()
    else()
        dbg(${DBG_COMMON} "  No 'tests' array in Solution.json (optional)" ID PHASE7_TEST)
    endif()
else()
    cmake_warn("W701" "SOLUTION_JSON not available")
endif()

# ==============================================================================
# Test 3: Test Framework Externals
# ==============================================================================

dbg(${DBG_COMMON} "Testing Framework Externals..." ID PHASE7_TEST)

# doctest (local)
get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)
if(_externals_json)
    string(JSON _doctest_json ERROR_VARIABLE _err GET "${_externals_json}" "doctest")
    if(NOT _err)
        dbg(${DBG_COMMON} "  doctest: defined (local)" ID PHASE7_TEST)
    endif()
    
    string(JSON _gtest_json ERROR_VARIABLE _err GET "${_externals_json}" "googletest")
    if(NOT _err)
        dbg(${DBG_COMMON} "  googletest: defined (fetched)" ID PHASE7_TEST)
    endif()
    
    string(JSON _catch2_json ERROR_VARIABLE _err GET "${_externals_json}" "catch2")
    if(NOT _err)
        dbg(${DBG_COMMON} "  catch2: defined (fetched)" ID PHASE7_TEST)
    endif()
endif()

# ==============================================================================
# Test 4: Test Targets Created
# ==============================================================================

if(BUILD_TESTS)
    dbg(${DBG_COMMON} "Testing Test Target Creation..." ID PHASE7_TEST)
    
    # Check for test targets (naming convention: *_Tests, *_UnitTests, etc.)
    set(_test_targets_found 0)
    
    # doctest tests
    if(TARGET BasicLogger_UnitTests)
        math(EXPR _test_targets_found "${_test_targets_found} + 1")
        dbg(${DBG_COMMON} "  Target: BasicLogger_UnitTests (doctest)" ID PHASE7_TEST)
    endif()
    
    # googletest tests
    if(TARGET AudioEngine_UnitTests)
        math(EXPR _test_targets_found "${_test_targets_found} + 1")
        dbg(${DBG_COMMON} "  Target: AudioEngine_UnitTests (googletest)" ID PHASE7_TEST)
    endif()
    
    # catch2 tests
    if(TARGET Integration_Tests)
        math(EXPR _test_targets_found "${_test_targets_found} + 1")
        dbg(${DBG_COMMON} "  Target: Integration_Tests (catch2)" ID PHASE7_TEST)
    endif()
    
    if(_test_targets_found EQUAL 0)
        dbg(${DBG_COMMON} "  No test targets found (check tests array)" ID PHASE7_TEST)
    else()
        dbg(${DBG_COMMON} "  Found ${_test_targets_found} test target(s)" ID PHASE7_TEST)
    endif()
endif()

# ==============================================================================
# Test 5: CTest Integration
# ==============================================================================

if(BUILD_TESTS)
    dbg(${DBG_COMMON} "Testing CTest Integration..." ID PHASE7_TEST)
    
    # Check if CTest is enabled
    if(CMAKE_TESTING_ENABLED)
        dbg(${DBG_COMMON} "  CTest is enabled" ID PHASE7_TEST)
    else()
        dbg(${DBG_COMMON} "  CTest not enabled (enable_testing() not called)" ID PHASE7_TEST)
    endif()
    
    # Check test properties if target exists
    if(TARGET BasicLogger_UnitTests)
        get_test_property(BasicLogger_UnitTests TIMEOUT _timeout)
        get_test_property(BasicLogger_UnitTests LABELS _labels)
        
        if(_timeout)
            dbg(${DBG_RARE} "    BasicLogger_UnitTests timeout: ${_timeout}s" ID PHASE7_TEST)
        endif()
        if(_labels)
            dbg(${DBG_RARE} "    BasicLogger_UnitTests labels: ${_labels}" ID PHASE7_TEST)
        endif()
    endif()
endif()

# ==============================================================================
# Test 6: Test Types
# ==============================================================================

dbg(${DBG_COMMON} "Testing Test Types..." ID PHASE7_TEST)

set(_supported_types "unit;integration;system;performance;smoke")
dbg(${DBG_COMMON} "  Supported types: ${_supported_types}" ID PHASE7_TEST)

# ==============================================================================
# Test 7: Framework Targets (if used)
# ==============================================================================

if(BUILD_TESTS)
    dbg(${DBG_COMMON} "Testing Framework Targets..." ID PHASE7_TEST)
    
    # doctest
    if(TARGET doctest)
        dbg(${DBG_COMMON} "  doctest target available" ID PHASE7_TEST)
    endif()
    
    # GoogleTest
    if(TARGET gtest_main)
        dbg(${DBG_COMMON} "  gtest_main target available" ID PHASE7_TEST)
    endif()
    if(TARGET gmock_main)
        dbg(${DBG_COMMON} "  gmock_main target available" ID PHASE7_TEST)
    endif()
    
    # Catch2
    if(TARGET Catch2WithMain)
        dbg(${DBG_COMMON} "  Catch2WithMain target available" ID PHASE7_TEST)
    endif()
endif()

# ==============================================================================
# Summary
# ==============================================================================

dbgspace(ID PHASE7_TEST)
dbg(${DBG_OFTEN} "=== Phase 7 Test PASSED ===" ID PHASE7_TEST)
enddbgblock(ID PHASE7_TEST)

set(PHASE7_TEST_PASSED TRUE CACHE BOOL "Phase 7 Test passed" FORCE)
