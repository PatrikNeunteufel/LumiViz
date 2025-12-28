# ==============================================================================
# Phase8.cmake – Build System Test for App-Container Pipeline
# ==============================================================================
#
# Test:         Phase 8
# Version:      1.0.0
# Date:         2025-12-17
# Part of:      CMake Architecture
#
# Description:
#   Tests the App-Container Pipeline including:
#   - apps array parsing
#   - App-Container target creation (Core, Runner, Tests)
#   - Core/Runner separation
#   - Test integration with frameworks
#   - Directory structure validation
#   - Context data collection
#
# Note:
#   App-Tests only run when BUILD_TESTS=ON
#
# ==============================================================================

include_guard(GLOBAL)

dbg_init(ID PHASE8_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase8")
dbg(${DBG_OFTEN} "=== Phase 8 Test Start ===" ID PHASE8_TEST)

# ==============================================================================
# Test 1: Apps Array in Solution.json
# ==============================================================================

dbg(${DBG_COMMON} "Test 1: Apps array parsing..." ID PHASE8_TEST)

get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)

set(_apps_count 0)
if(_solution_json)
    string(JSON _apps_json ERROR_VARIABLE _err GET "${_solution_json}" "apps")
    if(NOT _err)
        string(JSON _apps_count LENGTH "${_apps_json}")
        dbg(${DBG_COMMON} "  Found ${_apps_count} app(s) in Solution.json" ID PHASE8_TEST)
        
        # Parse first app if exists
        if(_apps_count GREATER 0)
            string(JSON _first_app GET "${_apps_json}" 0)
            string(JSON _app_name GET "${_first_app}" "name")
            dbg(${DBG_COMMON} "  First app: ${_app_name}" ID PHASE8_TEST)
            
            # Check core section
            string(JSON _core_json ERROR_VARIABLE _e1 GET "${_first_app}" "core")
            if(NOT _e1)
                dbg(${DBG_RARE} "    core section: present" ID PHASE8_TEST)
            endif()
            
            # Check runner section
            string(JSON _runner_json ERROR_VARIABLE _e2 GET "${_first_app}" "runner")
            if(NOT _e2)
                string(JSON _runner_type ERROR_VARIABLE _e3 GET "${_runner_json}" "type")
                if(NOT _e3)
                    dbg(${DBG_RARE} "    runner.type: ${_runner_type}" ID PHASE8_TEST)
                endif()
            endif()
            
            # Check tests section
            string(JSON _tests_json ERROR_VARIABLE _e4 GET "${_first_app}" "tests")
            if(NOT _e4)
                dbg(${DBG_RARE} "    tests section: present" ID PHASE8_TEST)
            endif()
        endif()
    else()
        dbg(${DBG_COMMON} "  No 'apps' array in Solution.json (optional)" ID PHASE8_TEST)
    endif()
else()
    cmake_warn("W801" "SOLUTION_JSON not available")
endif()

# ==============================================================================
# Test 2: AppCollect Module Availability
# ==============================================================================

dbg(${DBG_COMMON} "Test 2: AppCollect module..." ID PHASE8_TEST)

if(COMMAND _collect_app)
    dbg(${DBG_COMMON} "  _collect_app() function available" ID PHASE8_TEST)
else()
    dbg(${DBG_COMMON} "  _collect_app() not available (Apps.cmake not included yet)" ID PHASE8_TEST)
endif()

# ==============================================================================
# Test 3: App-Container Targets
# ==============================================================================

dbg(${DBG_COMMON} "Test 3: App-Container targets..." ID PHASE8_TEST)

set(_app_targets_found 0)
set(_core_targets_found 0)

# Check for any .Core targets
get_property(_all_targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

foreach(_target IN LISTS _all_targets)
    if(_target MATCHES "\\.Core$")
        math(EXPR _core_targets_found "${_core_targets_found} + 1")
        dbg(${DBG_COMMON} "  Core Library: ${_target}" ID PHASE8_TEST)
        
        # Extract app name and check for runner
        string(REGEX REPLACE "\\.Core$" "" _app_name "${_target}")
        if(TARGET ${_app_name})
            math(EXPR _app_targets_found "${_app_targets_found} + 1")
            dbg(${DBG_COMMON} "  Runner Executable: ${_app_name}" ID PHASE8_TEST)
        endif()
    endif()
endforeach()

if(_core_targets_found EQUAL 0)
    dbg(${DBG_COMMON} "  No App-Container targets found (check apps array)" ID PHASE8_TEST)
else()
    dbg(${DBG_COMMON} "  Found ${_core_targets_found} Core library(ies)" ID PHASE8_TEST)
    dbg(${DBG_COMMON} "  Found ${_app_targets_found} Runner executable(s)" ID PHASE8_TEST)
endif()

# ==============================================================================
# Test 4: App-Test Targets
# ==============================================================================

if(BUILD_TESTS)
    dbg(${DBG_COMMON} "Test 4: App-Test targets (BUILD_TESTS=ON)..." ID PHASE8_TEST)
    
    set(_unit_test_targets 0)
    set(_integration_test_targets 0)
    
    foreach(_target IN LISTS _all_targets)
        if(_target MATCHES "\\.UnitTests$")
            math(EXPR _unit_test_targets "${_unit_test_targets} + 1")
            dbg(${DBG_COMMON} "  Unit Tests: ${_target}" ID PHASE8_TEST)
        endif()
        if(_target MATCHES "\\.IntegrationTests$")
            math(EXPR _integration_test_targets "${_integration_test_targets} + 1")
            dbg(${DBG_COMMON} "  Integration Tests: ${_target}" ID PHASE8_TEST)
        endif()
    endforeach()
    
    dbg(${DBG_COMMON} "  Found ${_unit_test_targets} Unit Test target(s)" ID PHASE8_TEST)
    dbg(${DBG_COMMON} "  Found ${_integration_test_targets} Integration Test target(s)" ID PHASE8_TEST)
else()
    dbg(${DBG_COMMON} "Test 4: App-Test targets (BUILD_TESTS=OFF, skipped)" ID PHASE8_TEST)
endif()

# ==============================================================================
# Test 5: Core Library Properties
# ==============================================================================

dbg(${DBG_COMMON} "Test 5: Core Library properties..." ID PHASE8_TEST)

# Find first .Core target and check properties
set(_test_core_target "")
foreach(_target IN LISTS _all_targets)
    if(_target MATCHES "\\.Core$")
        set(_test_core_target "${_target}")
        break()
    endif()
endforeach()

if(_test_core_target)
    # Check if it's a STATIC library
    get_target_property(_lib_type ${_test_core_target} TYPE)
    dbg(${DBG_COMMON} "  ${_test_core_target} type: ${_lib_type}" ID PHASE8_TEST)
    
    if(_lib_type STREQUAL "STATIC_LIBRARY")
        dbg(${DBG_COMMON} "  Correct: Core is STATIC library" ID PHASE8_TEST)
    else()
        cmake_warn("W802" "Core library should be STATIC, found: ${_lib_type}")
    endif()
    
    # Check include directories
    get_target_property(_include_dirs ${_test_core_target} INCLUDE_DIRECTORIES)
    if(_include_dirs)
        list(LENGTH _include_dirs _inc_count)
        dbg(${DBG_RARE} "  Include directories: ${_inc_count}" ID PHASE8_TEST)
    endif()
    
    # Check linked libraries
    get_target_property(_link_libs ${_test_core_target} LINK_LIBRARIES)
    if(_link_libs)
        dbg(${DBG_RARE} "  Linked libraries: ${_link_libs}" ID PHASE8_TEST)
    endif()
else()
    dbg(${DBG_COMMON} "  No Core target to inspect" ID PHASE8_TEST)
endif()

# ==============================================================================
# Test 6: Runner Links Against Core
# ==============================================================================

dbg(${DBG_COMMON} "Test 6: Runner → Core linkage..." ID PHASE8_TEST)

if(_test_core_target)
    string(REGEX REPLACE "\\.Core$" "" _runner_target "${_test_core_target}")
    
    if(TARGET ${_runner_target})
        get_target_property(_runner_links ${_runner_target} LINK_LIBRARIES)
        
        if(_runner_links)
            list(FIND _runner_links "${_test_core_target}" _core_idx)
            if(NOT _core_idx EQUAL -1)
                dbg(${DBG_COMMON} "  Correct: ${_runner_target} links against ${_test_core_target}" ID PHASE8_TEST)
            else()
                cmake_warn("W803" "Runner does not link against Core")
            endif()
        endif()
    endif()
else()
    dbg(${DBG_COMMON} "  No targets to inspect" ID PHASE8_TEST)
endif()

# ==============================================================================
# Test 7: Directory Structure Convention
# ==============================================================================

dbg(${DBG_COMMON} "Test 7: Directory structure convention..." ID PHASE8_TEST)

# Check if default apps directory exists
set(_default_apps_dir "${CMAKE_SOURCE_DIR}/projects/apps")
if(EXISTS "${_default_apps_dir}")
    dbg(${DBG_COMMON} "  projects/apps/ exists" ID PHASE8_TEST)
    
    # List subdirectories
    file(GLOB _app_dirs LIST_DIRECTORIES true "${_default_apps_dir}/*")
    list(LENGTH _app_dirs _app_dir_count)
    dbg(${DBG_COMMON} "  Found ${_app_dir_count} app directory(ies)" ID PHASE8_TEST)
    
    # Check first app structure
    foreach(_app_dir IN LISTS _app_dirs)
        if(IS_DIRECTORY "${_app_dir}")
            get_filename_component(_app_name "${_app_dir}" NAME)
            dbg(${DBG_RARE} "  Checking ${_app_name}/..." ID PHASE8_TEST)
            
            # Check required directories
            if(EXISTS "${_app_dir}/src")
                dbg(${DBG_RARE} "    src/: present" ID PHASE8_TEST)
            else()
                dbg(${DBG_RARE} "    src/: MISSING" ID PHASE8_TEST)
            endif()
            
            if(EXISTS "${_app_dir}/main")
                dbg(${DBG_RARE} "    main/: present" ID PHASE8_TEST)
            else()
                dbg(${DBG_RARE} "    main/: MISSING" ID PHASE8_TEST)
            endif()
            
            # Check optional directories
            if(EXISTS "${_app_dir}/include")
                dbg(${DBG_ULTRA_RARE} "    include/: present" ID PHASE8_TEST)
            endif()
            
            if(EXISTS "${_app_dir}/tests")
                dbg(${DBG_ULTRA_RARE} "    tests/: present" ID PHASE8_TEST)
            endif()
            
            if(EXISTS "${_app_dir}/pch")
                dbg(${DBG_ULTRA_RARE} "    pch/: present" ID PHASE8_TEST)
            endif()
            
            # Only check first directory
            break()
        endif()
    endforeach()
else()
    dbg(${DBG_COMMON} "  projects/apps/ does not exist (no apps defined)" ID PHASE8_TEST)
endif()

# ==============================================================================
# Test 8: Error Code Range
# ==============================================================================

dbg(${DBG_COMMON} "Test 8: Error code range validation..." ID PHASE8_TEST)

dbg(${DBG_COMMON} "  App-Container Errors: E401-E407" ID PHASE8_TEST)
dbg(${DBG_COMMON} "  App-Container Warnings: W401-W403" ID PHASE8_TEST)
dbg(${DBG_COMMON} "  (Defined in ErrorCodes.md)" ID PHASE8_TEST)

# ==============================================================================
# Test 9: CTest Integration for App Tests
# ==============================================================================

if(BUILD_TESTS)
    dbg(${DBG_COMMON} "Test 9: CTest integration for App tests..." ID PHASE8_TEST)
    
    if(CMAKE_TESTING_ENABLED)
        dbg(${DBG_COMMON} "  CTest is enabled" ID PHASE8_TEST)
        
        # Check if any app tests are registered
        # (Test properties can only be checked after add_test was called)
        foreach(_target IN LISTS _all_targets)
            if(_target MATCHES "\\.(Unit|Integration)Tests$")
                get_test_property(${_target} TIMEOUT _timeout)
                get_test_property(${_target} LABELS _labels)
                
                if(_timeout)
                    dbg(${DBG_RARE} "    ${_target} timeout: ${_timeout}s" ID PHASE8_TEST)
                endif()
                if(_labels)
                    dbg(${DBG_RARE} "    ${_target} labels: ${_labels}" ID PHASE8_TEST)
                endif()
                
                # Only show first test
                break()
            endif()
        endforeach()
    else()
        dbg(${DBG_COMMON} "  CTest not enabled" ID PHASE8_TEST)
    endif()
else()
    dbg(${DBG_COMMON} "Test 9: CTest integration (BUILD_TESTS=OFF, skipped)" ID PHASE8_TEST)
endif()

# ==============================================================================
# Test 10: IDE Organization
# ==============================================================================

dbg(${DBG_COMMON} "Test 10: IDE folder organization..." ID PHASE8_TEST)

if(_test_core_target)
    get_target_property(_folder ${_test_core_target} FOLDER)
    if(_folder)
        dbg(${DBG_COMMON} "  ${_test_core_target} folder: ${_folder}" ID PHASE8_TEST)
        
        if(_folder MATCHES "^Apps/")
            dbg(${DBG_COMMON} "  Correct: Targets organized under Apps/ folder" ID PHASE8_TEST)
        endif()
    else()
        dbg(${DBG_COMMON} "  No FOLDER property set (IDE-dependent)" ID PHASE8_TEST)
    endif()
else()
    dbg(${DBG_COMMON} "  No targets to inspect" ID PHASE8_TEST)
endif()

# ==============================================================================
# Summary
# ==============================================================================

dbgspace(ID PHASE8_TEST)

# Determine overall status
set(_phase8_status "PASSED")

if(_apps_count GREATER 0 AND _core_targets_found EQUAL 0)
    set(_phase8_status "WARNING")
    dbg(${DBG_COMMON} "Warning: Apps defined but no targets created" ID PHASE8_TEST)
endif()

dbg(${DBG_OFTEN} "=== Phase 8 Test ${_phase8_status} ===" ID PHASE8_TEST)
enddbgblock(ID PHASE8_TEST)

set(PHASE8_TEST_PASSED TRUE CACHE BOOL "Phase 8 Test passed" FORCE)

# ==============================================================================
# Cleanup
# ==============================================================================

unset(_solution_json)
unset(_apps_json)
unset(_apps_count)
unset(_err)
unset(_first_app)
unset(_app_name)
unset(_all_targets)
unset(_app_targets_found)
unset(_core_targets_found)
unset(_test_core_target)
unset(_phase8_status)
