# cmake/buildSystemTest/phase4.cmake
# ===================================
# Phase 4 Tests: Library Pipeline
#
# Version: 1.0.0
# Date:    2025-12-07
# Status:  In Development
#
# Tests:
#   - Libraries array exists and can be parsed
#   - INTERFACE library is created
#   - Executable can link against library

include_guard(GLOBAL)

# ============================================
# DEBUG INITIALIZATION
# ============================================

dbg_init(ID PHASE4_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase4")
dbg(${DBG_OFTEN} "=== Phase 4 Test Start ===" ID PHASE4_TEST)

# ============================================
# TEST 1: Libraries array exists in Solution.json
# ============================================

dbg(${DBG_COMMON} "Testing libraries array in Solution.json..." ID PHASE4_TEST)

get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)
_json_has_key("${_solution_json}" "libraries" _has_libraries)

if(NOT _has_libraries)
    cmake_fatal("ASSERT" "Solution.json has no 'libraries' array")
endif()

_json_array_length("${_solution_json}" "libraries" _lib_count)

dbg(${DBG_COMMON} "  Found ${_lib_count} library(ies) in Solution.json" ID PHASE4_TEST)

# ============================================
# TEST 2: BasicLogger library exists
# ============================================

dbg(${DBG_COMMON} "Testing BasicLogger target..." ID PHASE4_TEST)

if(NOT TARGET BasicLogger)
    cmake_fatal("ASSERT" "Target 'BasicLogger' was not created")
endif()

# Check it's an INTERFACE library
get_target_property(_type BasicLogger TYPE)
if(NOT "${_type}" STREQUAL "INTERFACE_LIBRARY")
    cmake_fatal("ASSERT" "BasicLogger should be INTERFACE_LIBRARY, got: ${_type}")
endif()

dbg(${DBG_COMMON} "  BasicLogger target exists (INTERFACE)" ID PHASE4_TEST)

# ============================================
# TEST 3: BasicLogger has include directory
# ============================================

dbg(${DBG_COMMON} "Testing BasicLogger include directories..." ID PHASE4_TEST)

get_target_property(_includes BasicLogger INTERFACE_INCLUDE_DIRECTORIES)

if("${_includes}" STREQUAL "" OR "${_includes}" STREQUAL "_includes-NOTFOUND")
    cmake_fatal("ASSERT" "BasicLogger has no include directories")
endif()

dbg(${DBG_COMMON} "  BasicLogger includes: ${_includes}" ID PHASE4_TEST)

# ============================================
# TEST 4: MinimalConsole links BasicLogger
# ============================================

dbg(${DBG_COMMON} "Testing MinimalConsole dependencies..." ID PHASE4_TEST)

if(NOT TARGET MinimalConsole)
    cmake_fatal("ASSERT" "Target 'MinimalConsole' was not created")
endif()

get_target_property(_deps MinimalConsole LINK_LIBRARIES)

if(NOT "BasicLogger" IN_LIST _deps)
    cmake_fatal("ASSERT" "MinimalConsole should link against BasicLogger, got: ${_deps}")
endif()

dbg(${DBG_COMMON} "  MinimalConsole links BasicLogger" ID PHASE4_TEST)

# ============================================
# COMPLETION
# ============================================

dbgspace(ID PHASE4_TEST)
dbg(${DBG_OFTEN} "=== Phase 4 Test PASSED ===" ID PHASE4_TEST)
enddbgblock(ID PHASE4_TEST)

# Set success flag
set(PHASE4_TEST_PASSED TRUE CACHE BOOL "Phase 4 Test passed" FORCE)
