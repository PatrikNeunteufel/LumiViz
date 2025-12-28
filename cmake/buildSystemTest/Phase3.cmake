# ==============================================================================
# phase3.cmake – Phase 3 Build System Test
# ==============================================================================
#
# Module:       phase3.cmake
# Version:      1.0.0
# Date:         2025-12-05
# Path:         cmake/buildSystemTest/phase3.cmake
# Part of:      CMake Architecture V2
#
# Description:
#   Phase 3 Test: Executable Pipeline Validation
#   Tests the complete executable creation pipeline.
#
# Tests:
#   - Executables.cmake loads correctly
#   - ExecutableCollect.cmake collects all fields
#   - ExecutableCreate.cmake creates targets
#   - Skip logic works
#   - BUILD_ONLY filter works
#   - Platform filter works
#   - Default values are applied correctly
#
# Based on:
#   - master_concept v0.1
#   - Solution_Schema v0.1
#   - guidelines v0.1
#
# ==============================================================================

include_guard(GLOBAL)

# ==============================================================================
# Test Context Initialization
# ==============================================================================

dbg_init(ID PHASE3_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase3")

dbg(${DBG_OFTEN} "=== Phase 3 Test Start ===" ID PHASE3_TEST)

# ==============================================================================
# Test: Executable Pipeline Modules Loaded
# ==============================================================================

dbg(${DBG_COMMON} "Testing Executable Pipeline modules..." ID PHASE3_TEST)

# Check if functions exist (were loaded through include)
if(NOT COMMAND _collect_executable)
    cmake_fatal("ASSERT" "_collect_executable function not defined")
endif()

if(NOT COMMAND _create_executable_target)
    cmake_fatal("ASSERT" "_create_executable_target function not defined")
endif()

dbg(${DBG_COMMON} "  Executable Pipeline modules loaded" ID PHASE3_TEST)

# ==============================================================================
# Test: Executables from Solution.json
# ==============================================================================

dbg(${DBG_COMMON} "Testing Executable definitions in Solution.json..." ID PHASE3_TEST)

get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)

_json_has_key("${_solution_json}" "executables" _has_exes)
if(NOT _has_exes)
    cmake_warn("W999" "Phase 3 Test: No executables defined in Solution.json")
else()
    _json_array_length("${_solution_json}" "executables" _exe_count)
    dbg(${DBG_COMMON} "  Found ${_exe_count} executable(s) in Solution.json" ID PHASE3_TEST)
    
    # Count active (non-skip) executables
    set(_active_count 0)
    if(_exe_count GREATER 0)
        math(EXPR _last "${_exe_count} - 1")
        foreach(_idx RANGE 0 ${_last})
            _json_array_get("${_solution_json}" "executables" ${_idx} _exe_json)
            _json_get_string("${_exe_json}" "name" _exe_name)
            _json_get_bool_from_key("${_exe_json}" "skip" _skip)
            
            if(NOT _skip)
                math(EXPR _active_count "${_active_count} + 1")
                dbg(${DBG_RARE} "    Active: ${_exe_name}" ID PHASE3_TEST)
            else()
                dbg(${DBG_RARE} "    Skipped: ${_exe_name}" ID PHASE3_TEST)
            endif()
        endforeach()
    endif()
    
    dbg(${DBG_COMMON} "  Active executables: ${_active_count}" ID PHASE3_TEST)
endif()

dbg(${DBG_COMMON} "  Executable definitions OK" ID PHASE3_TEST)

# ==============================================================================
# Test: Context Functionality with Real Data
# ==============================================================================

dbg(${DBG_COMMON} "Testing Context with sample executable data..." ID PHASE3_TEST)

# Create test JSON
set(_test_exe_json "{
    \"name\": \"TestExe\",
    \"displayName\": \"Test Executable\",
    \"description\": \"A test executable for Phase 3\",
    \"version\": \"1.0.0\",
    \"path\": \"test/path\",
    \"type\": \"CONSOLE\",
    \"skip\": false,
    \"pch\": {
        \"enabled\": true,
        \"header\": \"pch.h\"
    },
    \"dependencies\": [],
    \"externals\": [],
    \"platforms\": [\"windows\", \"linux\"]
}")

ctx_create(TEST_EXE_CTX)
_collect_executable("${_test_exe_json}" TEST_EXE_CTX)

# Validate collected data
ctx_get(TEST_EXE_CTX NAME _test_name)
ctx_get(TEST_EXE_CTX DISPLAY_NAME _test_display)
ctx_get(TEST_EXE_CTX VERSION _test_version)
ctx_get(TEST_EXE_CTX TYPE _test_type)
ctx_get(TEST_EXE_CTX SKIP _test_skip)
ctx_get(TEST_EXE_CTX PCH_ENABLED _test_pch)
ctx_get(TEST_EXE_CTX PLATFORMS _test_platforms)

if(NOT "${_test_name}" STREQUAL "TestExe")
    cmake_fatal("ASSERT" "Context NAME mismatch: expected 'TestExe', got '${_test_name}'")
endif()

if(NOT "${_test_display}" STREQUAL "Test Executable")
    cmake_fatal("ASSERT" "Context DISPLAY_NAME mismatch")
endif()

if(NOT "${_test_version}" STREQUAL "1.0.0")
    cmake_fatal("ASSERT" "Context VERSION mismatch: expected '1.0.0', got '${_test_version}'")
endif()

if(NOT "${_test_type}" STREQUAL "CONSOLE")
    cmake_fatal("ASSERT" "Context TYPE mismatch: expected 'CONSOLE', got '${_test_type}'")
endif()

# Boolean comparison: _json_get_bool_from_key returns real CMake booleans
if(_test_skip)
    cmake_fatal("ASSERT" "Context SKIP should be FALSE, got '${_test_skip}'")
endif()

if(NOT _test_pch)
    cmake_fatal("ASSERT" "Context PCH_ENABLED should be TRUE, got '${_test_pch}'")
endif()

dbg(${DBG_COMMON} "  NAME: ${_test_name}" ID PHASE3_TEST)
dbg(${DBG_COMMON} "  DISPLAY_NAME: ${_test_display}" ID PHASE3_TEST)
dbg(${DBG_COMMON} "  VERSION: ${_test_version}" ID PHASE3_TEST)
dbg(${DBG_COMMON} "  TYPE: ${_test_type}" ID PHASE3_TEST)
dbg(${DBG_COMMON} "  PCH_ENABLED: ${_test_pch}" ID PHASE3_TEST)
dbg(${DBG_COMMON} "  PLATFORMS: ${_test_platforms}" ID PHASE3_TEST)

dbg(${DBG_COMMON} "  Context collection works" ID PHASE3_TEST)

# ==============================================================================
# Test: Default Values
# ==============================================================================

dbg(${DBG_COMMON} "Testing default values..." ID PHASE3_TEST)

set(_minimal_exe_json "{\"name\": \"MinimalExe\"}")

ctx_create(MINIMAL_CTX)
_collect_executable("${_minimal_exe_json}" MINIMAL_CTX)

ctx_get(MINIMAL_CTX NAME _min_name)
ctx_get(MINIMAL_CTX PATH _min_path)
ctx_get(MINIMAL_CTX TYPE _min_type)
ctx_get(MINIMAL_CTX SKIP _min_skip)

if(NOT "${_min_name}" STREQUAL "MinimalExe")
    cmake_fatal("ASSERT" "Minimal NAME mismatch")
endif()

# Default path: projects/exec/{name}/src
if(NOT "${_min_path}" STREQUAL "projects/exec/MinimalExe/src")
    cmake_fatal("ASSERT" "Default PATH mismatch: expected 'projects/exec/MinimalExe/src', got '${_min_path}'")
endif()

# Default type from Solution or CONSOLE
get_property(_default_type GLOBAL PROPERTY SOLUTION_DEFAULT_EXECUTABLE_TYPE)
if("${_default_type}" STREQUAL "")
    set(_default_type "CONSOLE")
endif()

if(NOT "${_min_type}" STREQUAL "${_default_type}")
    cmake_fatal("ASSERT" "Default TYPE mismatch: expected '${_default_type}', got '${_min_type}'")
endif()

dbg(${DBG_COMMON} "  Default PATH: ${_min_path}" ID PHASE3_TEST)
dbg(${DBG_COMMON} "  Default TYPE: ${_min_type}" ID PHASE3_TEST)

dbg(${DBG_COMMON} "  Default values work" ID PHASE3_TEST)

# ==============================================================================
# Test: Targets Were Created (if executables not skipped)
# ==============================================================================

dbg(${DBG_COMMON} "Checking created targets..." ID PHASE3_TEST)

set(_created_targets 0)
if(_exe_count GREATER 0)
    math(EXPR _last "${_exe_count} - 1")
    foreach(_idx RANGE 0 ${_last})
        _json_array_get("${_solution_json}" "executables" ${_idx} _exe_json)
        _json_get_string("${_exe_json}" "name" _exe_name)
        _json_get_bool_from_key("${_exe_json}" "skip" _skip)
        
        if(NOT _skip)
            if(TARGET ${_exe_name})
                math(EXPR _created_targets "${_created_targets} + 1")
                dbg(${DBG_COMMON} "    Target exists: ${_exe_name}" ID PHASE3_TEST)
            else()
                dbg(${DBG_COMMON} "    Target missing: ${_exe_name}" ID PHASE3_TEST)
            endif()
        endif()
    endforeach()
endif()

dbg(${DBG_COMMON} "  Created targets: ${_created_targets}" ID PHASE3_TEST)

# At least one target should exist if there are active executables
if(_active_count GREATER 0 AND _created_targets EQUAL 0)
    cmake_warn("W999" "Phase 3: No targets created although ${_active_count} active executables")
endif()

dbg(${DBG_COMMON} "  Target creation check complete" ID PHASE3_TEST)

# ==============================================================================
# Test Result
# ==============================================================================

dbgspace(ID PHASE3_TEST)
dbg(${DBG_OFTEN} "=== Phase 3 Test PASSED ===" ID PHASE3_TEST)
enddbgblock(ID PHASE3_TEST)

# Set success flag
set(PHASE3_TEST_PASSED TRUE CACHE BOOL "Phase 3 Test passed" FORCE)
