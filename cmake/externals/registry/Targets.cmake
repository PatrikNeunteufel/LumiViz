# cmake/externals/registry/Targets.cmake
# =======================================
# External target registry - tracks targets and provides lookup
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - cmake/core/Errors.cmake
#   - cmake/core/Debug.cmake
#
# Provides:
#   - _register_external_target(EXT_NAME TARGET_NAME [PRIMARY])
#   - _auto_register_external_targets(EXT_NAME)
#   - _get_external_targets(EXT_NAME OUT_VAR)
#   - _get_external_primary_target(EXT_NAME OUT_VAR)
#   - _has_external_target(EXT_NAME OUT_VAR)
#   - _validate_external_targets(EXT_NAME)
#   - _link_external_to_target(CONSUMER_TARGET EXT_NAME [SCOPE])
#   - _get_all_external_targets(EXT_NAME OUT_VAR)
#
# Used by:
#   - Handler.cmake
#   - Orchestrator.cmake
#   - PostFetch Hooks

include_guard(GLOBAL)

# ==============================================================================
# _register_external_target - Register a target for an external
# ==============================================================================
#[[
    _register_external_target(EXT_NAME TARGET_NAME [PRIMARY])
    
    Registers a target as belonging to an external.
    
    Parameters:
        EXT_NAME    - Name of the external
        TARGET_NAME - CMake target name
        PRIMARY     - (optional) Mark as primary target
    
    Example:
        _register_external_target("glfw" "glfw" PRIMARY)
        _register_external_target("glfw" "glfw::glfw")
]]
function(_register_external_target EXT_NAME TARGET_NAME)
    cmake_parse_arguments(_ARG "PRIMARY" "" "" ${ARGN})
    
    # Store target in external's target list
    get_property(_targets GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_TARGETS)
    if(NOT "${TARGET_NAME}" IN_LIST _targets)
        set_property(GLOBAL APPEND PROPERTY EXTERNAL_${EXT_NAME}_TARGETS "${TARGET_NAME}")
    endif()
    
    # Set primary target if specified
    if(_ARG_PRIMARY)
        set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PRIMARY_TARGET "${TARGET_NAME}")
    endif()
    
    dbg(${DBG_RARE} "  Registered target: ${TARGET_NAME} (external: ${EXT_NAME})" ID EXTERNALS)
    
endfunction()

# ==============================================================================
# _auto_register_external_targets - Auto-detect and register targets
# ==============================================================================
#[[
    _auto_register_external_targets(EXT_NAME)
    
    Attempts to auto-detect targets created by an external.
    
    Detection sources (in order):
        1. HOOK_KNOWN_TARGETS - Set by PreFetch hook for special externals
        2. HOOK_PRIMARY_TARGET - Set by PreFetch hook
        3. Generic patterns: ${EXT_NAME}, ${EXT_NAME}::${EXT_NAME}, etc.
    
    Parameters:
        EXT_NAME - Name of the external
]]
function(_auto_register_external_targets EXT_NAME)
    string(TOLOWER "${EXT_NAME}" _ext_lower)
    string(TOUPPER "${EXT_NAME}" _ext_upper)
    
    # ==========================================================================
    # Step 1: Check for hook-defined targets (from PreFetch)
    # ==========================================================================
    
    # Check if PreFetch hook defined known targets via GLOBAL PROPERTY
    get_property(_hook_targets GLOBAL PROPERTY HOOK_KNOWN_TARGETS_${EXT_NAME})
    get_property(_hook_primary GLOBAL PROPERTY HOOK_PRIMARY_TARGET_${EXT_NAME})
    
    if(_hook_targets)
        dbg(${DBG_RARE} "  Using hook-defined targets for ${EXT_NAME}" ID EXTERNALS)
        
        set(_found_any FALSE)
        foreach(_candidate IN LISTS _hook_targets)
            if(TARGET ${_candidate})
                if("${_candidate}" STREQUAL "${_hook_primary}")
                    _register_external_target("${EXT_NAME}" "${_candidate}" PRIMARY)
                else()
                    _register_external_target("${EXT_NAME}" "${_candidate}")
                endif()
                set(_found_any TRUE)
            endif()
        endforeach()
        
        # Set primary if defined but not yet registered
        if(_found_any)
            get_property(_current_primary GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PRIMARY_TARGET)
            if("${_current_primary}" STREQUAL "" AND _hook_primary)
                if(TARGET ${_hook_primary})
                    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PRIMARY_TARGET "${_hook_primary}")
                else()
                    # Primary not found, use first registered
                    get_property(_targets GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_TARGETS)
                    if(_targets)
                        list(GET _targets 0 _first)
                        set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PRIMARY_TARGET "${_first}")
                    endif()
                endif()
            endif()
            return()
        endif()
    endif()
    
    # ==========================================================================
    # Step 2: Generic auto-detection patterns
    # ==========================================================================
    
    # Common naming patterns for CMake targets
    set(_candidates
        # Exact name
        "${EXT_NAME}"
        "${_ext_lower}"
        "${_ext_upper}"
        # Namespaced
        "${EXT_NAME}::${EXT_NAME}"
        "${_ext_lower}::${_ext_lower}"
        # Common variations
        "${EXT_NAME}::${_ext_lower}"
        "${_ext_lower}::${EXT_NAME}"
        # lib prefix
        "lib${_ext_lower}"
    )
    
    set(_found_primary FALSE)
    
    foreach(_candidate IN LISTS _candidates)
        if(TARGET ${_candidate})
            if(NOT _found_primary)
                _register_external_target("${EXT_NAME}" "${_candidate}" PRIMARY)
                set(_found_primary TRUE)
            else()
                _register_external_target("${EXT_NAME}" "${_candidate}")
            endif()
        endif()
    endforeach()
    
    if(NOT _found_primary)
        dbg(${DBG_COMMON} "  Warning: No targets found for ${EXT_NAME}" ID EXTERNALS)
    endif()
    
endfunction()

# ==============================================================================
# _get_external_targets - Get all targets for an external
# ==============================================================================
function(_get_external_targets EXT_NAME OUT_VAR)
    get_property(_targets GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_TARGETS)
    set(${OUT_VAR} "${_targets}" PARENT_SCOPE)
endfunction()

# ==============================================================================
# _get_external_primary_target - Get primary target for an external
# ==============================================================================
function(_get_external_primary_target EXT_NAME OUT_VAR)
    get_property(_primary GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PRIMARY_TARGET)
    
    if("${_primary}" STREQUAL "")
        # Fall back to first registered target
        get_property(_targets GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_TARGETS)
        if(_targets)
            list(GET _targets 0 _primary)
        endif()
    endif()
    
    set(${OUT_VAR} "${_primary}" PARENT_SCOPE)
endfunction()

# ==============================================================================
# _has_external_target - Check if external has registered targets
# ==============================================================================
function(_has_external_target EXT_NAME OUT_VAR)
    get_property(_targets GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_TARGETS)
    
    if(_targets)
        set(${OUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# _validate_external_targets - Validate external has usable targets
# ==============================================================================
function(_validate_external_targets EXT_NAME)
    _has_external_target("${EXT_NAME}" _has_targets)
    
    if(NOT _has_targets)
        cmake_fatal("E201" "Fetched external '${EXT_NAME}': No target in registry. PostFetch hook required?")
    endif()
    
    # Log registered targets
    _get_external_targets("${EXT_NAME}" _targets)
    _get_external_primary_target("${EXT_NAME}" _primary)
    dbg(${DBG_COMMON} "  Validated: ${EXT_NAME} (primary: ${_primary})" ID EXTERNALS)
    
endfunction()

# ==============================================================================
# _link_external_to_target - Link external's targets to a consumer target
# ==============================================================================
#[[
    _link_external_to_target(CONSUMER_TARGET EXT_NAME [SCOPE <scope>])
    
    Links an external's primary target to a consumer target.
    ALWAYS uses keyword signature for consistency.
    
    Parameters:
        CONSUMER_TARGET - Target that will use the external
        EXT_NAME        - Name of the external
        SCOPE           - Link scope (PUBLIC/PRIVATE/INTERFACE), default PRIVATE
    
    Example:
        _link_external_to_target(MyApp glfw SCOPE PRIVATE)
]]
function(_link_external_to_target CONSUMER_TARGET EXT_NAME)
    # Parse arguments with SCOPE as named parameter
    set(_options "")
    set(_one_value SCOPE)
    cmake_parse_arguments(_ARG "${_options}" "${_one_value}" "" ${ARGN})
    
    # Default to PRIVATE if not specified
    if(NOT _ARG_SCOPE)
        set(_ARG_SCOPE PRIVATE)
    endif()
    
    # Get primary target
    _get_external_primary_target("${EXT_NAME}" _primary)
    
    if("${_primary}" STREQUAL "")
        cmake_warn("W101" "External '${EXT_NAME}': No target to link")
        return()
    endif()
    
    # Link - ALWAYS use keyword signature
    if(TARGET ${_primary})
        target_link_libraries(${CONSUMER_TARGET} ${_ARG_SCOPE} ${_primary})
        dbg(${DBG_RARE} "  Linked: ${CONSUMER_TARGET} <- ${_primary} (${_ARG_SCOPE})" ID EXTERNALS)
    else()
        cmake_fatal("E203" "Target '${_primary}' for external '${EXT_NAME}' not found")
    endif()
    
endfunction()

# ==============================================================================
# _get_all_external_targets - Get all targets for linking
# ==============================================================================
function(_get_all_external_targets EXT_NAME OUT_VAR)
    get_property(_targets GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_TARGETS)
    
    # Filter to only existing targets
    set(_existing "")
    foreach(_t IN LISTS _targets)
        if(TARGET ${_t})
            list(APPEND _existing "${_t}")
        endif()
    endforeach()
    
    set(${OUT_VAR} "${_existing}" PARENT_SCOPE)
endfunction()
