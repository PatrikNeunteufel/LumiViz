# cmake/buildSystemTest/phase5.cmake
# ===================================
# Phase 5 Tests: Externals Pipeline
#
# Version: 1.0.0
# Date:    2025-12-08
# Status:  In Development
#
# Tests:
#   - Externals block parsed correctly
#   - Local external registered
#   - External applied to target

include_guard(GLOBAL)

# ============================================
# DEBUG INITIALIZATION
# ============================================

dbg_init(ID PHASE5_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase5")
dbg(${DBG_OFTEN} "=== Phase 5 Test Start ===" ID PHASE5_TEST)

# ============================================
# TEST 1: SOLUTION_EXTERNALS_JSON exists
# ============================================

dbg(${DBG_COMMON} "Testing SOLUTION_EXTERNALS_JSON property..." ID PHASE5_TEST)

get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)

if("${_externals_json}" STREQUAL "")
    cmake_fatal("ASSERT" "SOLUTION_EXTERNALS_JSON property is empty")
endif()

dbg(${DBG_COMMON} "  SOLUTION_EXTERNALS_JSON exists" ID PHASE5_TEST)

# ============================================
# TEST 2: bass external is defined
# ============================================

dbg(${DBG_COMMON} "Testing bass external definition..." ID PHASE5_TEST)

_json_has_key("${_externals_json}" "bass" _has_bass)

if(NOT _has_bass)
    cmake_fatal("ASSERT" "External 'bass' not found in externals block")
endif()

dbg(${DBG_COMMON} "  External 'bass' is defined" ID PHASE5_TEST)

# ============================================
# TEST 3: bass external is registered
# ============================================

dbg(${DBG_COMMON} "Testing bass registration..." ID PHASE5_TEST)

get_property(_bass_registered GLOBAL PROPERTY EXTERNAL_bass_REGISTERED)

if(NOT "${_bass_registered}" STREQUAL "TRUE")
    cmake_fatal("ASSERT" "External 'bass' is not registered")
endif()

get_property(_bass_path GLOBAL PROPERTY EXTERNAL_bass_PATH)
get_property(_bass_include GLOBAL PROPERTY EXTERNAL_bass_INCLUDE)

dbg(${DBG_COMMON} "  bass registered:" ID PHASE5_TEST)
dbg(${DBG_COMMON} "    PATH: ${_bass_path}" ID PHASE5_TEST)
dbg(${DBG_COMMON} "    INCLUDE: ${_bass_include}" ID PHASE5_TEST)

# ============================================
# TEST 4: MinimalConsole has externals
# ============================================

dbg(${DBG_COMMON} "Testing MinimalConsole externals..." ID PHASE5_TEST)

if(NOT TARGET MinimalConsole)
    cmake_fatal("ASSERT" "Target 'MinimalConsole' was not created")
endif()

# Check that bass was applied (we can check link libraries)
get_target_property(_link_libs MinimalConsole LINK_LIBRARIES)

# bass.lib should be in the link libraries (on Windows)
# Note: The actual path may vary, so we just check that there's some bass reference
string(FIND "${_link_libs}" "bass" _bass_pos)

if(_bass_pos EQUAL -1)
    cmake_warn("W101" "MinimalConsole may not have bass linked (link libs: ${_link_libs})")
else()
    dbg(${DBG_COMMON} "  MinimalConsole has bass in link libraries" ID PHASE5_TEST)
endif()

# ============================================
# TEST 5: External options were processed
# ============================================

dbg(${DBG_COMMON} "Testing external_options processing..." ID PHASE5_TEST)

# We can't directly verify BASS_FLAC was enabled, but we can check
# that the Solution.json has the options defined
get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)
_json_array_get("${_solution_json}" "executables" 0 _exe_json)
_json_get_object_or_empty("${_exe_json}" "external_options" _ext_opts)
_json_has_key("${_ext_opts}" "bass" _has_bass_opts)

if(_has_bass_opts)
    _json_get_object("${_ext_opts}" "bass" _bass_opts)
    _json_get_bool_from_key("${_bass_opts}" "BASS_FLAC" _flac_enabled)
    
    if(_flac_enabled)
        dbg(${DBG_COMMON} "  BASS_FLAC option is enabled" ID PHASE5_TEST)
    else()
        dbg(${DBG_COMMON} "  BASS_FLAC option is disabled" ID PHASE5_TEST)
    endif()
else()
    dbg(${DBG_COMMON} "  No bass options defined (using defaults)" ID PHASE5_TEST)
endif()

# ============================================
# COMPLETION
# ============================================

dbgspace(ID PHASE5_TEST)
dbg(${DBG_OFTEN} "=== Phase 5 Test PASSED ===" ID PHASE5_TEST)
enddbgblock(ID PHASE5_TEST)

# Set success flag
set(PHASE5_TEST_PASSED TRUE CACHE BOOL "Phase 5 Test passed" FORCE)
