# ==============================================================================
# CMakeLists.txt – Top-Level CMake Configuration
# ==============================================================================
#
# Project:      CMake Architecture V2
# Version:      0.5.0
# Date:         2025-12-17
#
# Description:
#   Top-level CMake configuration for the modular build system.
#   All console output is handled through the Debug module.
#
# Options:
#   -DBUILD_TESTS=ON|OFF          Build project tests (default: OFF)
#   -DRUN_BUILD_SYSTEM_TESTS=ON   Run phase tests
#   -DTEST_PHASE=1|2|3|4|5|6|7|8  Specific phase to test (default: all)
#   -DDEBUG_MESSAGES=ON|OFF       Debug output on/off (default: ON)
#   -DDEBUG_DEFAULT_LEVEL=1-5     Debug verbosity (default: 2)
#
# Based on:
#   - master_concept v0.1
#   - guidelines v0.1
#   - Solution_Schema v0.1.4
#
# Changelog:
#   v0.3.0 (2025-12-17): Phase 8 (App-Container)
#   v0.2.0 (2025-12-12): Phase 6 (Git Externals) + Phase 7 (Tests)
#   v0.1.2 (2025-12-08): Phase 5 (Local Externals)
#
# ==============================================================================

cmake_minimum_required(VERSION 3.25)

# ==============================================================================
# Options
# ==============================================================================

option(BUILD_TESTS "Build project tests" OFF)
# Default OFF: Konsumenten-Projekte brauchen die Build-System-Selbsttests nicht.
# Im CMakeCraft-Repo selbst: Preset "craft-selftest" nutzen (oder -DRUN_BUILD_SYSTEM_TESTS=ON).
option(RUN_BUILD_SYSTEM_TESTS "Run build system phase tests" OFF)
set(TEST_PHASE "" CACHE STRING "Specific phase to test (empty = all completed phases)")

# ==============================================================================
# Phase 1: Core Modules (Order Matters!)
# ==============================================================================

include(cmake/core/Errors.cmake)
include(cmake/core/Debug.cmake)
include(cmake/core/Json.cmake)
include(cmake/core/Validation.cmake)
include(cmake/core/Context.cmake)
include(cmake/core/SourceCollect.cmake)
include(cmake/core/OutputDirs.cmake)
include(cmake/core/Warnings.cmake)
include(cmake/core/CompilerOptions.cmake)

# ==============================================================================
# Build System Info (via Debug Module)
# ==============================================================================

dbg_init(ID CMAKE_MAIN LEVEL ${DBG_SHOW_MUCH} SWITCH ON TAG "CMake")

dbg(${DBG_OFTEN} "=== CMake Architecture V2 ===" ID CMAKE_MAIN)
dbg(${DBG_COMMON} "CMake Version: ${CMAKE_VERSION}" ID CMAKE_MAIN)
dbg(${DBG_COMMON} "Generator: ${CMAKE_GENERATOR}" ID CMAKE_MAIN)
dbg(${DBG_COMMON} "Build Tests: ${BUILD_TESTS}" ID CMAKE_MAIN)
dbg(${DBG_COMMON} "Build System Tests: ${RUN_BUILD_SYSTEM_TESTS}" ID CMAKE_MAIN)

# ==============================================================================
# Phase 2: Solution Configuration
# ==============================================================================

include(cmake/project/Solution.cmake)

# ==============================================================================
# Project Definition (with values from Solution.json)
# ==============================================================================

get_property(_sol_name GLOBAL PROPERTY SOLUTION_NAME)
get_property(_sol_version GLOBAL PROPERTY SOLUTION_VERSION)

project(
    "${_sol_name}"
    VERSION "${_sol_version}"
    LANGUAGES C CXX
)

dbg(${DBG_COMMON} "Project: ${PROJECT_NAME} v${PROJECT_VERSION}" ID CMAKE_MAIN)

# ==============================================================================
# Phase 5 & 6: Externals (Local + Fetched)
# Must come BEFORE Libraries and Executables!
# ==============================================================================

include(cmake/project/Externals.cmake)

# ==============================================================================
# Phase 4: Libraries
# Must come BEFORE Executables!
# ==============================================================================

include(cmake/project/Libraries.cmake)

# ==============================================================================
# Phase 3: Executables
# ==============================================================================

include(cmake/project/Executables.cmake)

# ==============================================================================
# Phase 8: App-Container
# ==============================================================================

include(cmake/project/Apps.cmake)

# ==============================================================================
# Phase 7: Tests
# Only when BUILD_TESTS is enabled
# ==============================================================================

if(BUILD_TESTS)
    include(cmake/project/Tests.cmake)
endif()

# ==============================================================================
# Close Main Debug Block
# ==============================================================================

dbgspace(ID CMAKE_MAIN)
dbg(${DBG_OFTEN} "=== Configuration Complete ===" ID CMAKE_MAIN)
enddbgblock(ID CMAKE_MAIN)

# ==============================================================================
# Build System Tests (optional, after pipeline)
# ==============================================================================

#[[
if(RUN_BUILD_SYSTEM_TESTS)
    
    dbg_init(ID BUILD_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "BuildTest")
    dbg(${DBG_OFTEN} "Running Build System Tests..." ID BUILD_TEST)
    
    if(NOT "${TEST_PHASE}" STREQUAL "")
        dbg(${DBG_COMMON} "Testing phase(s): ${TEST_PHASE}" ID BUILD_TEST)
    else()
        dbg(${DBG_COMMON} "Testing all completed phases" ID BUILD_TEST)
    endif()
        
    # Phase 1 Test: Core Modules
    if("${TEST_PHASE}" STREQUAL "" OR "1" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase1.cmake)
    endif()
    
    # Phase 2 Test: Solution Configuration
    if("${TEST_PHASE}" STREQUAL "" OR "2" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase2.cmake)
    endif()
    
    # Phase 3 Test: Executable Pipeline
    if("${TEST_PHASE}" STREQUAL "" OR "3" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase3.cmake)
    endif()
    
    # Phase 4 Test: Library Pipeline
    if("${TEST_PHASE}" STREQUAL "" OR "4" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase4.cmake)
    endif()
    
    # Phase 5 Test: Local Externals Pipeline
    if("${TEST_PHASE}" STREQUAL "" OR "5" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase5.cmake)
    endif()
    
    # Phase 6 Test: Git Externals + Hooks
    if("${TEST_PHASE}" STREQUAL "" OR "6" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase6.cmake)
    endif()
    
    # Phase 7 Test: Test Pipeline
    if("${TEST_PHASE}" STREQUAL "" OR "7" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase7.cmake)
    endif()
    
    # Phase 8 Test: App-Container Pipeline
    if("${TEST_PHASE}" STREQUAL "" OR "8" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase8.cmake)
    endif()

    # Phase 9 Test: System Externals
    if("${TEST_PHASE}" STREQUAL "" OR "9" IN_LIST TEST_PHASE)
        dbgspace(ID BUILD_TEST)
        include(cmake/buildSystemTest/phase9.cmake)
    endif()
    
    dbgspace(ID BUILD_TEST)
    dbg(${DBG_OFTEN} "Build System Tests completed" ID BUILD_TEST)
    enddbgblock(ID BUILD_TEST)
    
endif()
]]
# ==============================================================================
# Cleanup
# ==============================================================================

unset(_sol_name)
unset(_sol_version)
