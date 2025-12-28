# cmake/externals/system/PathResolver.cmake
# ==========================================
# Generic path resolution for system externals
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - cmake/core/Debug.cmake
#
# Provides:
#   - _resolve_system_path(EXT_NAME PACKAGE HINTS ADDITIONAL_PATHS BACKUP OUT_PATH OUT_IS_BACKUP)
#
# Search Order:
#   1. Environment variables: ${PACKAGE}_ROOT, ${PACKAGE}_DIR, ${PACKAGE}_HOME
#   2. hints[] from Solution.json
#   3. Package-specific standard paths (from package hook)
#   4. backup path (with warning)
#   5. Let find_package() handle it
#
# Used by:
#   - system/Handler.cmake

include_guard(GLOBAL)

# ==============================================================================
# _resolve_system_path - Multi-stage path resolution
# ==============================================================================
#[[
    _resolve_system_path(EXT_NAME PACKAGE HINTS ADDITIONAL_PATHS BACKUP OUT_PATH OUT_IS_BACKUP)
    
    Resolves the installation path for a system package.
    
    Parameters:
        EXT_NAME         - Name of the external (for debug output)
        PACKAGE          - Package name (e.g. "Qt6", "Boost")
        HINTS            - CMake list of hint paths from Solution.json
        ADDITIONAL_PATHS - CMake list of paths from package hook
        BACKUP           - Backup path (string, may be empty)
        OUT_PATH         - Output: resolved path or empty
        OUT_IS_BACKUP    - Output: TRUE if backup was used
    
    Search Priority:
        1. Environment variables (${PACKAGE}_ROOT, ${PACKAGE}_DIR, ${PACKAGE}_HOME)
        2. hints[] from Solution.json
        3. Package-specific standard paths
        4. backup path (triggers warning in caller)
    
    If nothing found, returns empty path and lets find_package() try its defaults.
    
    Example:
        _resolve_system_path("qt6" "Qt6" "${_hints}" "${_std_paths}" "${_backup}" 
                             _path _is_backup)
]]
function(_resolve_system_path EXT_NAME PACKAGE HINTS ADDITIONAL_PATHS BACKUP OUT_PATH OUT_IS_BACKUP)
    set(_found FALSE)
    set(_result_path "")
    set(_is_backup FALSE)
    
    # =========================================================================
    # Stage 1: Environment Variables
    # =========================================================================
    
    # Check common environment variable patterns
    # For Qt6: QT_ROOT, QT6_ROOT, Qt6_ROOT, QT_DIR, Qt6_DIR, etc.
    set(_env_suffixes "ROOT" "DIR" "HOME")
    
    # Build list of prefixes to check
    set(_env_prefixes "${PACKAGE}" "${PACKAGE}_")
    
    # Also check without version number (QT_ROOT for Qt6)
    string(REGEX REPLACE "[0-9]+$" "" _package_base "${PACKAGE}")
    if(NOT "${_package_base}" STREQUAL "${PACKAGE}")
        list(APPEND _env_prefixes "${_package_base}" "${_package_base}_")
    endif()
    
    foreach(_prefix IN LISTS _env_prefixes)
        foreach(_suffix IN LISTS _env_suffixes)
            set(_var_name "${_prefix}${_suffix}")
            if(DEFINED ENV{${_var_name}})
                set(_candidate "$ENV{${_var_name}}")
                if(EXISTS "${_candidate}")
                    set(_found TRUE)
                    set(_result_path "${_candidate}")
                    dbg(${DBG_RARE} "  [${EXT_NAME}] Found via ENV ${_var_name}: ${_candidate}" ID EXTERNALS)
                    break()
                else()
                    dbg(${DBG_ULTRA_RARE} "  [${EXT_NAME}] ENV ${_var_name} set but doesn't exist: ${_candidate}" ID EXTERNALS)
                endif()
            endif()
        endforeach()
        if(_found)
            break()
        endif()
    endforeach()
    
    # =========================================================================
    # Stage 2: hints[] from Solution.json
    # =========================================================================
    
    if(NOT _found AND HINTS)
        foreach(_hint IN LISTS HINTS)
            # Expand ${VAR} syntax to environment variables
            set(_hint_expanded "${_hint}")
            
            # Pattern: ${VARNAME} -> $ENV{VARNAME}
            while(_hint_expanded MATCHES "\\$\\{([A-Za-z_][A-Za-z0-9_]*)\\}")
                set(_env_var_name "${CMAKE_MATCH_1}")
                if(DEFINED ENV{${_env_var_name}})
                    string(REPLACE "\${${_env_var_name}}" "$ENV{${_env_var_name}}" _hint_expanded "${_hint_expanded}")
                else()
                    # Variable not defined - replace with empty
                    dbg(${DBG_ULTRA_RARE} "  [${EXT_NAME}] Hint references undefined ENV: ${_env_var_name}" ID EXTERNALS)
                    string(REPLACE "\${${_env_var_name}}" "" _hint_expanded "${_hint_expanded}")
                endif()
            endwhile()
            
            if(NOT "${_hint_expanded}" STREQUAL "" AND EXISTS "${_hint_expanded}")
                set(_found TRUE)
                set(_result_path "${_hint_expanded}")
                dbg(${DBG_RARE} "  [${EXT_NAME}] Found via hint: ${_hint_expanded}" ID EXTERNALS)
                break()
            endif()
        endforeach()
    endif()
    
    # =========================================================================
    # Stage 3: Package-specific standard paths (from hook)
    # =========================================================================
    
    if(NOT _found AND ADDITIONAL_PATHS)
        dbg(${DBG_ULTRA_RARE} "  [${EXT_NAME}] Checking standard paths from package hook..." ID EXTERNALS)
        foreach(_path IN LISTS ADDITIONAL_PATHS)
            # Expand environment variables in path
            set(_path_expanded "${_path}")
            while(_path_expanded MATCHES "\\$ENV\\{([A-Za-z_][A-Za-z0-9_]*)\\}")
                set(_env_var_name "${CMAKE_MATCH_1}")
                if(DEFINED ENV{${_env_var_name}})
                    string(REPLACE "$ENV{${_env_var_name}}" "$ENV{${_env_var_name}}" _path_expanded "${_path_expanded}")
                endif()
            endwhile()
            
            if(EXISTS "${_path_expanded}")
                set(_found TRUE)
                set(_result_path "${_path_expanded}")
                dbg(${DBG_RARE} "  [${EXT_NAME}] Found at standard path: ${_path_expanded}" ID EXTERNALS)
                break()
            endif()
        endforeach()
    endif()
    
    # =========================================================================
    # Stage 4: Backup path (with warning in caller)
    # =========================================================================
    
    if(NOT _found AND NOT "${BACKUP}" STREQUAL "")
        # Expand environment variables using same logic as hints
        set(_backup_expanded "${BACKUP}")
        while(_backup_expanded MATCHES "\\$\\{([A-Za-z_][A-Za-z0-9_]*)\\}")
            set(_env_var_name "${CMAKE_MATCH_1}")
            if(DEFINED ENV{${_env_var_name}})
                string(REPLACE "\${${_env_var_name}}" "$ENV{${_env_var_name}}" _backup_expanded "${_backup_expanded}")
            else()
                string(REPLACE "\${${_env_var_name}}" "" _backup_expanded "${_backup_expanded}")
            endif()
        endwhile()
        
        if(NOT "${_backup_expanded}" STREQUAL "" AND EXISTS "${_backup_expanded}")
            set(_found TRUE)
            set(_result_path "${_backup_expanded}")
            set(_is_backup TRUE)
            dbg(${DBG_RARE} "  [${EXT_NAME}] Found at BACKUP: ${_backup_expanded}" ID EXTERNALS)
        endif()
    endif()
    
    # =========================================================================
    # Result
    # =========================================================================
    
    if(_found)
        set(${OUT_PATH} "${_result_path}" PARENT_SCOPE)
        set(${OUT_IS_BACKUP} ${_is_backup} PARENT_SCOPE)
        dbg(${DBG_RARE} "  [${EXT_NAME}] Path resolved: ${_result_path}" ID EXTERNALS)
    else()
        # No path found - let find_package() try its default locations
        set(${OUT_PATH} "" PARENT_SCOPE)
        set(${OUT_IS_BACKUP} FALSE PARENT_SCOPE)
        dbg(${DBG_RARE} "  [${EXT_NAME}] No path found, relying on find_package() defaults" ID EXTERNALS)
    endif()
    
endfunction()
