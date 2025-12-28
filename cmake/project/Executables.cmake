# cmake/project/Executables.cmake
# ================================
# Executable pipeline orchestrator - iterates over executables and creates targets
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
#   - cmake/project/ExecutableCollect.cmake
#   - cmake/project/ExecutableCreate.cmake
#
# Used by:
#   - CMakeLists.txt (main build)

include_guard(GLOBAL)

# ==============================================================================
# Load Sub-Modules
# ==============================================================================

include(cmake/project/ExecutableCollect.cmake)
include(cmake/project/ExecutableCreate.cmake)

# ==============================================================================
# Debug Context Initialization
# ==============================================================================

dbg_init(ID EXECUTABLES LEVEL ${DBG_SHOW_MUCH} SWITCH ON TAG "Executables")

dbg(${DBG_OFTEN} "=== Executable Pipeline Start ===" ID EXECUTABLES)

# ==============================================================================
# Load Solution JSON and Executables Array
# ==============================================================================

get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)

# Check if executables array exists
_json_has_key("${_solution_json}" "executables" _has_executables)
if(NOT _has_executables)
    dbg(${DBG_COMMON} "No executables defined in Solution.json" ID EXECUTABLES)
    enddbgblock(ID EXECUTABLES)
    return()
endif()

# Get array length
_json_array_length("${_solution_json}" "executables" _exe_count)

if(_exe_count EQUAL 0)
    dbg(${DBG_COMMON} "Executables array is empty" ID EXECUTABLES)
    enddbgblock(ID EXECUTABLES)
    return()
endif()

dbg(${DBG_OFTEN} "Processing ${_exe_count} executable(s)..." ID EXECUTABLES)

# ==============================================================================
# Iterate Over Executables
# ==============================================================================

math(EXPR _last_idx "${_exe_count} - 1")

foreach(_idx RANGE 0 ${_last_idx})
    # Extract JSON for this executable
    _json_array_get("${_solution_json}" "executables" ${_idx} _exe_json)
    
    # Read name for debug output
    _json_get_string("${_exe_json}" "name" _exe_name)
    
    if("${_exe_name}" STREQUAL "")
        cmake_fatal("E001" "Executable #${_idx} has no 'name' field")
    endif()
    
    dbg(${DBG_COMMON} "--- Processing: ${_exe_name} ---" ID EXECUTABLES)
    
    # ==========================================================================
    # Create Context and Collect Data
    # ==========================================================================
    
    ctx_create(EXE_${_idx})
    _collect_executable("${_exe_json}" EXE_${_idx})
    
    # ==========================================================================
    # Skip Check
    # ==========================================================================
    
    ctx_get(EXE_${_idx} SKIP _skip)
    if(_skip)
        dbg(${DBG_COMMON} "  SKIP: ${_exe_name} (skip=true in Solution.json)" ID EXECUTABLES)
        continue()
    endif()
    
    # ==========================================================================
    # BUILD_ONLY Filter
    # ==========================================================================
    
    if(DEFINED BUILD_ONLY AND NOT "${BUILD_ONLY}" STREQUAL "")
        # BUILD_ONLY is a semicolon-separated list
        string(REPLACE ";" ";" _build_only_list "${BUILD_ONLY}")
        list(FIND _build_only_list "${_exe_name}" _found_idx)
        
        if(_found_idx EQUAL -1)
            dbg(${DBG_COMMON} "  SKIP: ${_exe_name} (not in BUILD_ONLY)" ID EXECUTABLES)
            continue()
        endif()
    endif()
    
    # ==========================================================================
    # Platform Check
    # ==========================================================================
    
    ctx_get(EXE_${_idx} PLATFORMS _platforms)
    if(NOT "${_platforms}" STREQUAL "")
        set(_platform_match FALSE)
        
        foreach(_platform IN LISTS _platforms)
            string(TOLOWER "${_platform}" _platform_lower)
            
            if(_platform_lower STREQUAL "windows" AND WIN32)
                set(_platform_match TRUE)
                break()
            elseif(_platform_lower STREQUAL "linux" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
                set(_platform_match TRUE)
                break()
            elseif(_platform_lower STREQUAL "macos" AND APPLE)
                set(_platform_match TRUE)
                break()
            elseif(_platform_lower STREQUAL "unix" AND UNIX)
                set(_platform_match TRUE)
                break()
            endif()
        endforeach()
        
        if(NOT _platform_match)
            dbg(${DBG_COMMON} "  SKIP: ${_exe_name} (platform not supported: ${_platforms})" ID EXECUTABLES)
            continue()
        endif()
    endif()
    
    # ==========================================================================
    # Target Duplicate Check
    # ==========================================================================
    
    if(TARGET ${_exe_name})
        cmake_fatal("E102" "Target '${_exe_name}' already exists")
    endif()
    
    # ==========================================================================
    # Create Target
    # ==========================================================================
    
    _create_executable_target(EXE_${_idx})
    
    dbg(${DBG_COMMON} "  Created: ${_exe_name}" ID EXECUTABLES)
    
endforeach()

# ==============================================================================
# Finish
# ==============================================================================

dbgspace(ID EXECUTABLES)
dbg(${DBG_OFTEN} "=== Executable Pipeline Complete ===" ID EXECUTABLES)
enddbgblock(ID EXECUTABLES)

# ==============================================================================
# Cleanup
# ==============================================================================

unset(_solution_json)
unset(_has_executables)
unset(_exe_count)
unset(_last_idx)
unset(_idx)
unset(_exe_json)
unset(_exe_name)
unset(_skip)
unset(_platforms)
unset(_platform_match)
unset(_platform_lower)
unset(_found_idx)
unset(_build_only_list)
