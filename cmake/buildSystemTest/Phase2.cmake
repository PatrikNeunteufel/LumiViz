# ==============================================================================
# phase2.cmake – Phase 2 Build System Test
# ==============================================================================
#
# Module:       phase2.cmake
# Version:      1.0.0
# Date:         2025-12-05
# Path:         cmake/buildSystemTest/phase2.cmake
# Part of:      CMake Architecture V2
#
# Description:
#   Phase 2 Test: Solution.json Validation
#   Tests that Solution.cmake correctly loads and processes Solution.json.
#
# Tests:
#   - Solution.cmake loads Solution.json correctly
#   - All GLOBAL properties are set
#   - Schema version is validated
#   - Settings are extracted correctly
#   - Externals block is provided
#   - Source mode is available (NEW in v0.1.0)
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

dbg_init(ID PHASE2_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase2")

dbg(${DBG_OFTEN} "=== Phase 2 Test Start ===" ID PHASE2_TEST)

# ==============================================================================
# Test: Solution Properties
# ==============================================================================

dbg(${DBG_COMMON} "Validating Solution Properties..." ID PHASE2_TEST)

get_property(_sol_name GLOBAL PROPERTY SOLUTION_NAME)
get_property(_sol_version GLOBAL PROPERTY SOLUTION_VERSION)
get_property(_sol_description GLOBAL PROPERTY SOLUTION_DESCRIPTION)
get_property(_sol_authors GLOBAL PROPERTY SOLUTION_AUTHORS)
get_property(_sol_schema GLOBAL PROPERTY SOLUTION_SCHEMA_VERSION)

if("${_sol_name}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_NAME not set")
endif()
if("${_sol_version}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_VERSION not set")
endif()
if("${_sol_schema}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_SCHEMA_VERSION not set")
endif()

dbg(${DBG_COMMON} "  SOLUTION_NAME: ${_sol_name}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  SOLUTION_VERSION: ${_sol_version}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  SOLUTION_SCHEMA: ${_sol_schema}" ID PHASE2_TEST)
dbg(${DBG_RARE} "  SOLUTION_DESCRIPTION: ${_sol_description}" ID PHASE2_TEST)
dbg(${DBG_RARE} "  SOLUTION_AUTHORS: ${_sol_authors}" ID PHASE2_TEST)

dbg(${DBG_COMMON} "  Solution Properties OK" ID PHASE2_TEST)

# ==============================================================================
# Test: Settings Properties
# ==============================================================================

dbg(${DBG_COMMON} "Validating Settings Properties..." ID PHASE2_TEST)

get_property(_cxx_std GLOBAL PROPERTY SOLUTION_CXX_STANDARD)
get_property(_c_std GLOBAL PROPERTY SOLUTION_C_STANDARD)
get_property(_default_lib GLOBAL PROPERTY SOLUTION_DEFAULT_LIBRARY_TYPE)
get_property(_default_exe GLOBAL PROPERTY SOLUTION_DEFAULT_EXECUTABLE_TYPE)
get_property(_source_mode GLOBAL PROPERTY SOLUTION_SOURCE_MODE)

if("${_cxx_std}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_CXX_STANDARD not set")
endif()
if("${_default_lib}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_DEFAULT_LIBRARY_TYPE not set")
endif()
if("${_default_exe}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_DEFAULT_EXECUTABLE_TYPE not set")
endif()
if("${_source_mode}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_SOURCE_MODE not set")
endif()

dbg(${DBG_COMMON} "  CXX_STANDARD: ${_cxx_std}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  C_STANDARD: ${_c_std}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  DEFAULT_LIBRARY: ${_default_lib}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  DEFAULT_EXECUTABLE: ${_default_exe}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  SOURCE_MODE: ${_source_mode}" ID PHASE2_TEST)

dbg(${DBG_COMMON} "  Settings Properties OK" ID PHASE2_TEST)

# ==============================================================================
# Test: CMake Cache Variables
# ==============================================================================

dbg(${DBG_COMMON} "Validating CMake Cache Variables..." ID PHASE2_TEST)

if("${CMAKE_CXX_STANDARD}" STREQUAL "")
    cmake_fatal("ASSERT" "CMAKE_CXX_STANDARD not set")
endif()

dbg(${DBG_COMMON} "  CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  CMAKE_C_STANDARD: ${CMAKE_C_STANDARD}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  CMAKE_CXX_EXTENSIONS: ${CMAKE_CXX_EXTENSIONS}" ID PHASE2_TEST)

dbg(${DBG_COMMON} "  CMake Cache Variables OK" ID PHASE2_TEST)

# ==============================================================================
# Test: Externals Policy
# ==============================================================================

dbg(${DBG_COMMON} "Validating Externals Policy..." ID PHASE2_TEST)

get_property(_cache_root GLOBAL PROPERTY SOLUTION_EXTERNALS_CACHE_ROOT)
get_property(_source_root GLOBAL PROPERTY SOLUTION_EXTERNALS_SOURCE_ROOT)
get_property(_update_policy GLOBAL PROPERTY SOLUTION_EXTERNALS_UPDATE_POLICY)

if("${_cache_root}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_EXTERNALS_CACHE_ROOT not set")
endif()
if("${_update_policy}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_EXTERNALS_UPDATE_POLICY not set")
endif()

dbg(${DBG_COMMON} "  EXTERNALS_CACHE: ${_cache_root}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  EXTERNALS_SOURCE: ${_source_root}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  UPDATE_POLICY: ${_update_policy}" ID PHASE2_TEST)

dbg(${DBG_COMMON} "  Externals Policy OK" ID PHASE2_TEST)

# ==============================================================================
# Test: Externals JSON Available
# ==============================================================================

dbg(${DBG_COMMON} "Validating Externals JSON..." ID PHASE2_TEST)

get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)
string(LENGTH "${_externals_json}" _ext_len)

if(_ext_len LESS 2)
    cmake_fatal("ASSERT" "SOLUTION_EXTERNALS_JSON is empty or too short")
endif()

dbg(${DBG_COMMON} "  EXTERNALS_JSON: ${_ext_len} characters" ID PHASE2_TEST)

dbg(${DBG_COMMON} "  Externals JSON OK" ID PHASE2_TEST)

# ==============================================================================
# Test: project() Compatibility
# ==============================================================================

dbg(${DBG_COMMON} "Validating project() compatibility..." ID PHASE2_TEST)

if(NOT "${PROJECT_NAME}" STREQUAL "${_sol_name}")
    cmake_fatal("ASSERT" "PROJECT_NAME mismatch: expected '${_sol_name}', got '${PROJECT_NAME}'")
endif()
if(NOT "${PROJECT_VERSION}" STREQUAL "${_sol_version}")
    cmake_fatal("ASSERT" "PROJECT_VERSION mismatch: expected '${_sol_version}', got '${PROJECT_VERSION}'")
endif()

dbg(${DBG_COMMON} "  PROJECT_NAME: ${PROJECT_NAME}" ID PHASE2_TEST)
dbg(${DBG_COMMON} "  PROJECT_VERSION: ${PROJECT_VERSION}" ID PHASE2_TEST)

dbg(${DBG_COMMON} "  project() compatibility OK" ID PHASE2_TEST)

# ==============================================================================
# Test Result
# ==============================================================================

dbgspace(ID PHASE2_TEST)
dbg(${DBG_OFTEN} "=== Phase 2 Test PASSED ===" ID PHASE2_TEST)
enddbgblock(ID PHASE2_TEST)

# Set success flag
set(PHASE2_TEST_PASSED TRUE CACHE BOOL "Phase 2 Test passed" FORCE)
