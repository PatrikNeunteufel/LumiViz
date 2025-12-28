# cmake/project/Apps.cmake
# =========================
# App-Container pipeline orchestrator - iterates over apps and creates targets
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
#   - cmake/project/AppCollect.cmake
#   - cmake/project/AppCreate.cmake
#
# Generated Targets (per App):
#   - {AppName}.Core           - STATIC Library (business logic)
#   - {AppName}                - Executable (runner with main())
#   - {AppName}.UnitTests      - Test Executable (if tests.unit defined)
#   - {AppName}.IntegrationTests - Test Executable (if tests.integration defined)
#
# Used by:
#   - CMakeLists.txt (main build)

include_guard(GLOBAL)

# ==============================================================================
# Load Sub-Modules
# ==============================================================================

include(cmake/project/AppCollect.cmake)
include(cmake/project/AppCreate.cmake)

# ==============================================================================
# Debug Context Initialization
# ==============================================================================

dbg_init(ID APPS LEVEL ${DBG_SHOW_MUCH} SWITCH ON TAG "Apps")

dbg(${DBG_OFTEN} "=== App-Container Pipeline Start ===" ID APPS)

# ==============================================================================
# Load Solution JSON and Apps Array
# ==============================================================================

get_property(_solution_json GLOBAL PROPERTY SOLUTION_JSON)

# Check if apps array exists
_json_has_key("${_solution_json}" "apps" _has_apps)
if(NOT _has_apps)
    dbg(${DBG_COMMON} "No apps defined in Solution.json" ID APPS)
    enddbgblock(ID APPS)
    return()
endif()

# Get array length
_json_array_length("${_solution_json}" "apps" _app_count)

if(_app_count EQUAL 0)
    dbg(${DBG_COMMON} "Apps array is empty" ID APPS)
    enddbgblock(ID APPS)
    return()
endif()

dbg(${DBG_OFTEN} "Processing ${_app_count} app(s)..." ID APPS)

# ==============================================================================
# Iterate Over Apps
# ==============================================================================

math(EXPR _last_idx "${_app_count} - 1")

foreach(_idx RANGE 0 ${_last_idx})
    # Extract JSON for this app
    _json_array_get("${_solution_json}" "apps" ${_idx} _app_json)
    
    # Read name for debug output
    _json_get_string("${_app_json}" "name" _app_name)
    
    if("${_app_name}" STREQUAL "")
        cmake_fatal("E401" "App #${_idx} has no 'name' field")
    endif()
    
    dbg(${DBG_COMMON} "--- Processing App: ${_app_name} ---" ID APPS)
    
    # ==========================================================================
    # Create Context and Collect Data
    # ==========================================================================
    
    ctx_create(APP_${_idx})
    _collect_app("${_app_json}" APP_${_idx})
    
    # ==========================================================================
    # Skip Check
    # ==========================================================================
    
    ctx_get(APP_${_idx} SKIP _skip)
    if(_skip)
        dbg(${DBG_COMMON} "  SKIP: ${_app_name} (skip=true in Solution.json)" ID APPS)
        continue()
    endif()
    
    # ==========================================================================
    # BUILD_ONLY Filter
    # ==========================================================================
    
    if(DEFINED BUILD_ONLY AND NOT "${BUILD_ONLY}" STREQUAL "")
        # BUILD_ONLY is a semicolon-separated list
        string(REPLACE ";" ";" _build_only_list "${BUILD_ONLY}")
        list(FIND _build_only_list "${_app_name}" _found_idx)
        
        # Also check for AppName.Core variant
        list(FIND _build_only_list "${_app_name}.Core" _found_core_idx)
        
        if(_found_idx EQUAL -1 AND _found_core_idx EQUAL -1)
            dbg(${DBG_COMMON} "  SKIP: ${_app_name} (not in BUILD_ONLY)" ID APPS)
            continue()
        endif()
    endif()
    
    # ==========================================================================
    # Platform Check
    # ==========================================================================
    
    ctx_get(APP_${_idx} PLATFORMS _platforms)
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
            dbg(${DBG_COMMON} "  SKIP: ${_app_name} (platform not supported: ${_platforms})" ID APPS)
            continue()
        endif()
    endif()
    
    # ==========================================================================
    # Target Duplicate Check (for Core library)
    # ==========================================================================
    
    if(TARGET ${_app_name}.Core)
        cmake_fatal("E102" "Target '${_app_name}.Core' already exists")
    endif()
    
    if(TARGET ${_app_name})
        cmake_fatal("E102" "Target '${_app_name}' already exists")
    endif()
    
    # ==========================================================================
    # Create Targets
    # ==========================================================================
    
    # 1. Create Core Library (AppName.Core)
    _create_app_core(APP_${_idx})
    dbg(${DBG_COMMON} "  Created: ${_app_name}.Core (STATIC Library)" ID APPS)
    
    # 2. Create Runner Executable (AppName)
    _create_app_runner(APP_${_idx})
    dbg(${DBG_COMMON} "  Created: ${_app_name} (Executable)" ID APPS)
    
    # 3. Create Tests (if BUILD_TESTS=ON and tests defined)
    if(BUILD_TESTS)
        _create_app_tests(APP_${_idx})
    endif()
    
endforeach()

# ==============================================================================
# Finish
# ==============================================================================

dbgspace(ID APPS)
dbg(${DBG_OFTEN} "=== App-Container Pipeline Complete ===" ID APPS)
enddbgblock(ID APPS)

# ==============================================================================
# Cleanup
# ==============================================================================

unset(_solution_json)
unset(_has_apps)
unset(_app_count)
unset(_last_idx)
unset(_idx)
unset(_app_json)
unset(_app_name)
unset(_skip)
unset(_platforms)
unset(_platform_match)
unset(_platform_lower)
unset(_found_idx)
unset(_found_core_idx)
unset(_build_only_list)
