# cmake/project/Libraries.cmake
# ==============================
# Library pipeline orchestrator - iterates over libraries and creates targets
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
#   - cmake/core/Context.cmake
#   - cmake/project/Solution.cmake
#
# Auto-loads:
#   - cmake/project/LibraryCollect.cmake
#   - cmake/project/LibraryCreate.cmake
#
# Used by:
#   - CMakeLists.txt (main build)

include_guard(GLOBAL)

# ==============================================================================
# Load Sub-Modules
# ==============================================================================

include(cmake/project/LibraryCollect.cmake)
include(cmake/project/LibraryCreate.cmake)

# ==============================================================================
# Debug Context Initialization
# ==============================================================================

dbg_init(ID LIBRARIES LEVEL ${DBG_SHOW_MUCH} SWITCH ON TAG "Libraries")

dbg(${DBG_OFTEN} "=== Library Pipeline Start ===" ID LIBRARIES)

# ==============================================================================
# Load Solution JSON and Libraries Array
# ==============================================================================

get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)

# Check if libraries array exists
_json_has_key("${_solution_json}" "libraries" _has_libraries)
if(NOT _has_libraries)
    dbg(${DBG_COMMON} "No libraries defined in Solution.json" ID LIBRARIES)
    dbg(${DBG_OFTEN} "=== Library Pipeline Complete (no libraries) ===" ID LIBRARIES)
    enddbgblock(ID LIBRARIES)
    return()
endif()

# Get array length
_json_array_length("${_solution_json}" "libraries" _lib_count)

if(_lib_count EQUAL 0)
    dbg(${DBG_COMMON} "Libraries array is empty" ID LIBRARIES)
    dbg(${DBG_OFTEN} "=== Library Pipeline Complete (empty) ===" ID LIBRARIES)
    enddbgblock(ID LIBRARIES)
    return()
endif()

dbg(${DBG_OFTEN} "Processing ${_lib_count} library(ies)..." ID LIBRARIES)

# ==============================================================================
# Iterate Over Libraries
# ==============================================================================

math(EXPR _last_idx "${_lib_count} - 1")

foreach(_idx RANGE 0 ${_last_idx})
    # Extract JSON for this library
    _json_array_get("${_solution_json}" "libraries" ${_idx} _lib_json)
    
    # Read name for debug output
    _json_get_string("${_lib_json}" "name" _lib_name)
    
    if("${_lib_name}" STREQUAL "")
        cmake_fatal("E001" "Library #${_idx} has no 'name' field")
    endif()
    
    dbg(${DBG_COMMON} "--- Processing: ${_lib_name} ---" ID LIBRARIES)
    
    # ==========================================================================
    # Create Context and Collect Data
    # ==========================================================================
    
    ctx_create(LIB_${_idx})
    _collect_library("${_lib_json}" LIB_${_idx})
    
    # ==========================================================================
    # Skip Check
    # ==========================================================================
    
    ctx_get(LIB_${_idx} SKIP _skip)
    if(_skip)
        dbg(${DBG_COMMON} "  SKIP: ${_lib_name} (skip=true in Solution.json)" ID LIBRARIES)
        continue()
    endif()
    
    # ==========================================================================
    # BUILD_ONLY Filter
    # ==========================================================================
    
    if(DEFINED BUILD_ONLY AND NOT "${BUILD_ONLY}" STREQUAL "")
        # BUILD_ONLY is a semicolon-separated list
        string(REPLACE ";" ";" _build_only_list "${BUILD_ONLY}")
        list(FIND _build_only_list "${_lib_name}" _found_idx)
        
        if(_found_idx EQUAL -1)
            dbg(${DBG_COMMON} "  SKIP: ${_lib_name} (not in BUILD_ONLY)" ID LIBRARIES)
            continue()
        endif()
    endif()
    
    # ==========================================================================
    # Platform Check
    # ==========================================================================
    
    ctx_get(LIB_${_idx} PLATFORM _platform)
    if(NOT "${_platform}" STREQUAL "")
        set(_platform_match FALSE)
        string(TOLOWER "${_platform}" _platform_lower)
        
        if(_platform_lower STREQUAL "windows" AND WIN32)
            set(_platform_match TRUE)
        elseif(_platform_lower STREQUAL "linux" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
            set(_platform_match TRUE)
        elseif(_platform_lower STREQUAL "macos" AND APPLE)
            set(_platform_match TRUE)
        elseif(_platform_lower STREQUAL "unix" AND UNIX)
            set(_platform_match TRUE)
        endif()
        
        if(NOT _platform_match)
            dbg(${DBG_COMMON} "  SKIP: ${_lib_name} (platform not supported: ${_platform})" ID LIBRARIES)
            continue()
        endif()
    endif()
    
    # ==========================================================================
    # Target Duplicate Check
    # ==========================================================================
    
    if(TARGET ${_lib_name})
        cmake_fatal("E102" "Target '${_lib_name}' already exists")
    endif()
    
    # ==========================================================================
    # Create Target
    # ==========================================================================
    
    _create_library_target(LIB_${_idx})
    
    dbg(${DBG_COMMON} "  Created: ${_lib_name}" ID LIBRARIES)
    
endforeach()

# ==============================================================================
# Finish
# ==============================================================================

dbgspace(ID LIBRARIES)
dbg(${DBG_OFTEN} "=== Library Pipeline Complete ===" ID LIBRARIES)
enddbgblock(ID LIBRARIES)

# ==============================================================================
# Cleanup
# ==============================================================================

unset(_solution_json)
unset(_has_libraries)
unset(_lib_count)
unset(_last_idx)
unset(_idx)
unset(_lib_json)
unset(_lib_name)
unset(_skip)
unset(_platform)
unset(_platform_match)
unset(_platform_lower)
unset(_found_idx)
unset(_build_only_list)
