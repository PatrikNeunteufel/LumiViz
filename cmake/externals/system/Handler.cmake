# cmake/externals/system/Handler.cmake
# =====================================
# System External Handler - finds system-installed packages via find_package
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
#   - cmake/externals/system/PathResolver.cmake
#
# Provides:
#   - _handle_system_external(EXT_NAME EXT_JSON)
#   - _apply_system_external_to_target(TARGET_NAME EXT_NAME EXT_OPTIONS)
#
# Used by:
#   - Orchestrator.cmake

include_guard(GLOBAL)

include(cmake/externals/system/PathResolver.cmake)

# ==============================================================================
# _handle_system_external - Process a System External Definition
# ==============================================================================
#[[
    _handle_system_external(EXT_NAME EXT_JSON)
    
    Processes a system external: resolves path, calls find_package, registers.
    
    Parameters:
        EXT_NAME - Name of the external (e.g. "qt6", "boost")
        EXT_JSON - JSON definition with "system": true and "package" field
    
    Required JSON fields:
        system  - Must be true
        package - Name for find_package() (e.g. "Qt6", "Boost")
    
    Optional JSON fields:
        version    - Version constraint (e.g. ">=6.5.0")
        components - Array of package components
        hints      - Array of additional search paths
        backup     - Fallback path (triggers W501 warning)
        required   - Default true, set false for optional packages
    
    Errors:
        E502 - 'package' field missing
        E503 - find_package() failed
    
    Warnings:
        W501 - Using backup location
    
    Example:
        _handle_system_external("qt6" "{\"system\":true,\"package\":\"Qt6\",\"components\":[\"Core\",\"Widgets\"]}")
]]
function(_handle_system_external EXT_NAME EXT_JSON)
    dbg(${DBG_COMMON} "[${EXT_NAME}] Processing system external" ID EXTERNALS)
    
    # =========================================================================
    # Required field: package
    # =========================================================================
    
    _json_get_string("${EXT_JSON}" "package" _package)
    if("${_package}" STREQUAL "")
        cmake_fatal("E502" 
            "System external '${EXT_NAME}': 'package' field is required.\n"
            "  Example: { \"system\": true, \"package\": \"Qt6\" }"
        )
    endif()
    
    dbg(${DBG_RARE} "  Package: ${_package}" ID EXTERNALS)
    
    # =========================================================================
    # Optional fields
    # =========================================================================
    
    _json_get_string("${EXT_JSON}" "version" _version)
    _json_get_array_as_list("${EXT_JSON}" "components" _components)
    _json_get_array_as_list("${EXT_JSON}" "hints" _hints)
    _json_get_string("${EXT_JSON}" "backup" _backup)
    _json_get_bool_or_default("${EXT_JSON}" "required" TRUE _required)
    
    if(_components)
        dbg(${DBG_RARE} "  Components: ${_components}" ID EXTERNALS)
    endif()
    
    if(_hints)
        dbg(${DBG_RARE} "  Hints from JSON: ${_hints}" ID EXTERNALS)
    endif()
    
    # =========================================================================
    # Load package-specific hook for standard paths (if exists)
    # =========================================================================
    
    set(_package_hook "${CMAKE_SOURCE_DIR}/cmake/externals/system/packages/${_package}.cmake")
    set(_additional_paths "")
    
    if(EXISTS "${_package_hook}")
        dbg(${DBG_RARE} "  Loading package hook: ${_package}.cmake" ID EXTERNALS)
        include("${_package_hook}")
        
        # Hook can define _get_${_package}_standard_paths()
        if(COMMAND _get_${_package}_standard_paths)
            cmake_language(CALL _get_${_package}_standard_paths _additional_paths)
            list(LENGTH _additional_paths _path_count)
            dbg(${DBG_ULTRA_RARE} "  Got ${_path_count} standard paths from hook" ID EXTERNALS)
        endif()
    else()
        dbg(${DBG_ULTRA_RARE} "  No package hook found at: ${_package_hook}" ID EXTERNALS)
    endif()
    
    # =========================================================================
    # Resolve path
    # =========================================================================
    
    _resolve_system_path(
        "${EXT_NAME}" 
        "${_package}" 
        "${_hints}" 
        "${_additional_paths}"
        "${_backup}" 
        _resolved_path 
        _used_backup
    )
    
    dbg(${DBG_RARE} "  PathResolver returned: '${_resolved_path}'" ID EXTERNALS)
    
    # Warning for backup usage
    if(_used_backup)
        cmake_warn("W501" 
            "[${EXT_NAME}] Using backup location: ${_resolved_path}\n"
            "  Consider setting ${_package}_ROOT environment variable."
        )
    endif()
    
    # =========================================================================
    # Extend CMAKE_PREFIX_PATH
    # =========================================================================
    
    if(NOT "${_resolved_path}" STREQUAL "")
        list(PREPEND CMAKE_PREFIX_PATH "${_resolved_path}")
        # Also need to set it in cache for find_package to see it
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE INTERNAL "" FORCE)
        dbg(${DBG_ULTRA_RARE} "  CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}" ID EXTERNALS)
    endif()
    
    # =========================================================================
    # Call find_package
    # =========================================================================
    
    if(_required)
        set(_req_flag REQUIRED)
    else()
        set(_req_flag "")
    endif()
    
    if(_components)
        dbg(${DBG_RARE} "  Calling: find_package(${_package} ${_version} ${_req_flag} COMPONENTS ${_components})" ID EXTERNALS)
        find_package(${_package} ${_version} ${_req_flag} COMPONENTS ${_components})
    else()
        dbg(${DBG_RARE} "  Calling: find_package(${_package} ${_version} ${_req_flag})" ID EXTERNALS)
        find_package(${_package} ${_version} ${_req_flag})
    endif()
    
    # =========================================================================
    # Check result
    # =========================================================================
    
    if(${_package}_FOUND)
        message(STATUS "[Externals] [${EXT_NAME}] Found ${_package} ${${_package}_VERSION}")
        
        # Call post-find hook if defined
        if(COMMAND _${_package}_post_find)
            dbg(${DBG_RARE} "  Calling post-find hook: _${_package}_post_find()" ID EXTERNALS)
            cmake_language(CALL _${_package}_post_find)
        endif()
        
    elseif(_required)
        cmake_fatal("E503" 
            "System external '${EXT_NAME}': find_package(${_package}) failed.\n"
            "  \n"
            "  Possible solutions:\n"
            "    1. Set ${_package}_ROOT environment variable\n"
            "    2. Add 'hints' in Solution.json\n"
            "    3. Install ${_package} to a standard location\n"
            "  \n"
            "  Example Solution.json:\n"
            "    \"${EXT_NAME}\": {\n"
            "      \"system\": true,\n"
            "      \"package\": \"${_package}\",\n"
            "      \"hints\": [\"C:/Path/To/${_package}\"]\n"
            "    }"
        )
    else()
        dbg(${DBG_COMMON} "[${EXT_NAME}] ${_package} not found (optional)" ID EXTERNALS)
        message(STATUS "[${EXT_NAME}] ${_package} not found (optional)")
    endif()
    
    # =========================================================================
    # Register as System External
    # =========================================================================
    
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_TYPE "SYSTEM")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PACKAGE "${_package}")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_COMPONENTS "${_components}")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_REGISTERED TRUE)
    
    dbg(${DBG_RARE} "  Registered: ${EXT_NAME} (SYSTEM)" ID EXTERNALS)
    
endfunction()

# ==============================================================================
# _apply_system_external_to_target - Link system external to a target
# ==============================================================================
#[[
    _apply_system_external_to_target(TARGET_NAME EXT_NAME EXT_OPTIONS)
    
    Links a system external to a CMake target.
    
    Parameters:
        TARGET_NAME - CMake target to link to
        EXT_NAME    - Name of the external
        EXT_OPTIONS - JSON options (currently unused for system externals)
    
    Example:
        _apply_system_external_to_target("MyApp" "qt6" "{}")
]]
function(_apply_system_external_to_target TARGET_NAME EXT_NAME EXT_OPTIONS)
    # Get registered package name
    get_property(_package GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PACKAGE)
    get_property(_components GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_COMPONENTS)
    
    if("${_package}" STREQUAL "")
        cmake_fatal("E503" "System external '${EXT_NAME}' not properly registered")
    endif()
    
    dbg(${DBG_RARE} "    Linking ${EXT_NAME} (${_package}) to ${TARGET_NAME}" ID EXTERNALS)
    
    # Link components or main target
    if(_components)
        foreach(_comp IN LISTS _components)
            if(TARGET ${_package}::${_comp})
                target_link_libraries(${TARGET_NAME} PRIVATE ${_package}::${_comp})
                dbg(${DBG_ULTRA_RARE} "      Linked: ${_package}::${_comp}" ID EXTERNALS)
            endif()
        endforeach()
    else()
        # Try common target patterns
        if(TARGET ${_package}::${_package})
            target_link_libraries(${TARGET_NAME} PRIVATE ${_package}::${_package})
        elseif(TARGET ${_package})
            target_link_libraries(${TARGET_NAME} PRIVATE ${_package})
        endif()
    endif()
    
    # Call target configuration hook if defined
    if(COMMAND _${_package}_configure_target)
        dbg(${DBG_RARE} "    Calling: _${_package}_configure_target(${TARGET_NAME})" ID EXTERNALS)
        cmake_language(CALL _${_package}_configure_target ${TARGET_NAME})
    endif()
    
endfunction()
