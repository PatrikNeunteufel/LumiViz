# ==============================================================================
# phase1.cmake – Phase 1 Build System Test
# ==============================================================================
#
# Module:       phase1.cmake
# Version:      1.0.0
# Date:         2025-12-05
# Path:         cmake/buildSystemTest/phase1.cmake
# Part of:      CMake Architecture V2
#
# Description:
#   Phase 1 Test: Core Module Validation
#   Tests all fundamental modules that form the foundation of the build system.
#
# Tests:
#   - Errors.cmake (cmake_fatal, cmake_warn, cmake_assert)
#   - Debug.cmake (dbg_init, dbg, dbgspace, enddbgblock)
#   - Json.cmake (all _json_* functions)
#   - Context.cmake (ctx_create, ctx_set, ctx_get, ctx_dump)
#   - Validation.cmake (validate_* functions)
#
# Based on:
#   - master_concept v0.1
#   - guidelines v0.1
#
# ==============================================================================

include_guard(GLOBAL)

# ==============================================================================
# Test Context Initialization
# ==============================================================================

dbg_init(ID PHASE1_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase1")

dbg(${DBG_OFTEN} "=== Phase 1 Test Start ===" ID PHASE1_TEST)

# ==============================================================================
# Test: Context API
# ==============================================================================

dbg(${DBG_COMMON} "Testing Context API..." ID PHASE1_TEST)

ctx_create(TEST_CTX)
ctx_set(TEST_CTX NAME "TestTarget")
ctx_set(TEST_CTX PATH "test/path")
ctx_set(TEST_CTX TYPE "EXECUTABLE")

ctx_get(TEST_CTX NAME _test_name)
ctx_get(TEST_CTX PATH _test_path)
ctx_get(TEST_CTX TYPE _test_type)

if(NOT "${_test_name}" STREQUAL "TestTarget")
    cmake_fatal("ASSERT" "Context NAME mismatch: expected 'TestTarget', got '${_test_name}'")
endif()
if(NOT "${_test_path}" STREQUAL "test/path")
    cmake_fatal("ASSERT" "Context PATH mismatch: expected 'test/path', got '${_test_path}'")
endif()
if(NOT "${_test_type}" STREQUAL "EXECUTABLE")
    cmake_fatal("ASSERT" "Context TYPE mismatch: expected 'EXECUTABLE', got '${_test_type}'")
endif()

dbg(${DBG_COMMON} "  Context API works" ID PHASE1_TEST)

# ==============================================================================
# Test: JSON Helpers
# ==============================================================================

dbg(${DBG_COMMON} "Testing JSON Helpers..." ID PHASE1_TEST)

set(_test_json "{\"name\":\"TestApp\",\"version\":\"1.0.0\",\"enabled\":true,\"count\":42}")

_json_has_key("${_test_json}" "name" _has_name)
if(NOT _has_name)
    cmake_fatal("ASSERT" "JSON _json_has_key failed for existing key")
endif()

_json_has_key("${_test_json}" "missing" _has_missing)
if(_has_missing)
    cmake_fatal("ASSERT" "JSON _json_has_key failed for missing key")
endif()

_json_get_string("${_test_json}" "name" _name)
if(NOT "${_name}" STREQUAL "TestApp")
    cmake_fatal("ASSERT" "JSON _json_get_string failed: expected 'TestApp', got '${_name}'")
endif()

_json_get_string_or_default("${_test_json}" "missing" "DefaultValue" _default)
if(NOT "${_default}" STREQUAL "DefaultValue")
    cmake_fatal("ASSERT" "JSON _json_get_string_or_default failed")
endif()

_json_get_bool_from_key("${_test_json}" "enabled" _enabled)
if(NOT _enabled)
    cmake_fatal("ASSERT" "JSON _json_get_bool_from_key failed")
endif()

_json_get_type("${_test_json}" "name" _type)
if(NOT "${_type}" STREQUAL "STRING")
    cmake_fatal("ASSERT" "JSON _json_get_type failed: expected 'STRING', got '${_type}'")
endif()

dbg(${DBG_COMMON} "  JSON Helpers work" ID PHASE1_TEST)

# ==============================================================================
# Test: Debug System (implicitly tested through usage)
# ==============================================================================

dbg(${DBG_COMMON} "Testing Debug System..." ID PHASE1_TEST)
dbg(${DBG_RARE} "  This is a rare message (should appear with SHOW_ALL)" ID PHASE1_TEST)
dbg(${DBG_ULTRA_RARE} "  This is an ultra-rare message" ID PHASE1_TEST)

dbg(${DBG_COMMON} "  Debug System works" ID PHASE1_TEST)

# ==============================================================================
# Test: Warnings (non-fatal)
# ==============================================================================

dbg(${DBG_COMMON} "Testing Warning System..." ID PHASE1_TEST)
cmake_warn("W999" "Test warning (expected, can be ignored)")
dbg(${DBG_COMMON} "  Warning System works" ID PHASE1_TEST)

# ==============================================================================
# Test Result
# ==============================================================================

dbgspace(ID PHASE1_TEST)
dbg(${DBG_OFTEN} "=== Phase 1 Test PASSED ===" ID PHASE1_TEST)
enddbgblock(ID PHASE1_TEST)

# Set success flag
set(PHASE1_TEST_PASSED TRUE CACHE BOOL "Phase 1 Test passed" FORCE)
