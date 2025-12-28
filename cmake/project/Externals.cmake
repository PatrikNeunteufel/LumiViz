# cmake/project/Externals.cmake
# ==============================
# External dependencies pipeline - processes all externals from Solution.json
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - cmake/core/Errors.cmake
#   - cmake/core/Debug.cmake
#   - cmake/core/Json.cmake
#   - cmake/project/Solution.cmake
#
# Auto-loads:
#   - cmake/externals/Orchestrator.cmake
#
# Used by:
#   - CMakeLists.txt (main build, runs before Libraries/Executables)

include_guard(GLOBAL)

# ==============================================================================
# Load Sub-Modules
# ==============================================================================

include(cmake/externals/Orchestrator.cmake)

# ==============================================================================
# Debug Context Initialization
# ==============================================================================

dbg_init(ID EXTERNALS LEVEL ${DBG_SHOW_MUCH} SWITCH ON TAG "Externals")

dbg(${DBG_OFTEN} "=== Externals Pipeline Start ===" ID EXTERNALS)

# ==============================================================================
# Load Solution JSON and Externals Block
# ==============================================================================

get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)

# Check if externals block exists
_json_has_key("${_solution_json}" "externals" _has_externals)
if(NOT _has_externals)
    dbg(${DBG_COMMON} "No externals defined in Solution.json" ID EXTERNALS)
    dbg(${DBG_OFTEN} "=== Externals Pipeline Complete (no externals) ===" ID EXTERNALS)
    enddbgblock(ID EXTERNALS)
    return()
endif()

# Get externals object
_json_get_object("${_solution_json}" "externals" _externals_json)

if("${_externals_json}" STREQUAL "" OR "${_externals_json}" STREQUAL "{}")
    dbg(${DBG_COMMON} "Externals block is empty" ID EXTERNALS)
    dbg(${DBG_OFTEN} "=== Externals Pipeline Complete (empty) ===" ID EXTERNALS)
    enddbgblock(ID EXTERNALS)
    return()
endif()

# Store externals JSON globally for later use by Executables/Libraries
set_property(GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON "${_externals_json}")

# ==============================================================================
# Get External Names (Keys)
# ==============================================================================

# CMake's string(JSON ... MEMBER) can iterate over object keys
string(JSON _ext_count ERROR_VARIABLE _err LENGTH "${_externals_json}")
if(_err OR _ext_count EQUAL 0)
    dbg(${DBG_COMMON} "No externals to process" ID EXTERNALS)
    dbg(${DBG_OFTEN} "=== Externals Pipeline Complete ===" ID EXTERNALS)
    enddbgblock(ID EXTERNALS)
    return()
endif()

dbg(${DBG_OFTEN} "Processing ${_ext_count} external(s)..." ID EXTERNALS)

# ==============================================================================
# Iterate Over Externals
# ==============================================================================

math(EXPR _last_idx "${_ext_count} - 1")

foreach(_idx RANGE 0 ${_last_idx})
    # Get external name (key)
    string(JSON _ext_name MEMBER "${_externals_json}" ${_idx})
    
    # Get external definition (value)
    string(JSON _ext_def GET "${_externals_json}" "${_ext_name}")
    
    # ==========================================================================
    # Check for skip flag
    # ==========================================================================
    
    _json_get_bool_or_default("${_ext_def}" "skip" FALSE _skip)
    
    if(_skip)
        dbg(${DBG_COMMON} "--- Skipping: ${_ext_name} (skip: true) ---" ID EXTERNALS)
        
        # Mark as skipped in global property for later validation
        set_property(GLOBAL APPEND PROPERTY SKIPPED_EXTERNALS "${_ext_name}")
        
        continue()
    endif()
    
    dbg(${DBG_COMMON} "--- Processing: ${_ext_name} ---" ID EXTERNALS)
    
    # ==========================================================================
    # Dispatch to Orchestrator
    # ==========================================================================
    
    _orchestrate_external("${_ext_name}" "${_ext_def}")
    
endforeach()

# ==============================================================================
# Finish
# ==============================================================================

dbgspace(ID EXTERNALS)
dbg(${DBG_OFTEN} "=== Externals Pipeline Complete ===" ID EXTERNALS)
enddbgblock(ID EXTERNALS)

# ==============================================================================
# Cleanup
# ==============================================================================

unset(_solution_json)
unset(_has_externals)
unset(_externals_json)
unset(_ext_count)
unset(_last_idx)
unset(_idx)
unset(_ext_name)
unset(_ext_def)
