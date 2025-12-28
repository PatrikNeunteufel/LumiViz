# cmake/externals/Orchestrator.cmake
# ===================================
# External type dispatcher - detects type and routes to appropriate handler
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
#   - cmake/core/Validation.cmake
#
# Auto-loads:
#   - cmake/externals/local/Attach.cmake
#   - cmake/externals/fetched/Handler.cmake
#   - cmake/externals/system/Handler.cmake (Phase 9)
#
# Provides:
#   - _orchestrate_external(EXT_NAME EXT_JSON)
#   - _get_external_options_for_target(TARGET_NAME EXT_NAME TARGET_JSON OUT_VAR)
#   - apply_external_to_target(TARGET_NAME EXT_NAME EXT_OPTIONS)
#
# Type Detection (priority order):
#   - "system" field → System External (find_package)
#   - "git" field    → Fetched External (FetchContent)
#   - "path" field   → Local External (Include.cmake)
#
# Used by:
#   - Externals.cmake
#   - ExecutableCreate.cmake
#   - LibraryCreate.cmake
#   - TestCreate.cmake
#   - AppCreate.cmake

include_guard(GLOBAL)

# ==============================================================================
# Load Sub-Modules
# ==============================================================================

include(cmake/externals/local/Attach.cmake)
include(cmake/externals/fetched/Handler.cmake)
include(cmake/externals/system/Handler.cmake)

# ==============================================================================
# _orchestrate_external - Main Dispatch Function
# ==============================================================================
#[[
    _orchestrate_external(EXT_NAME EXT_JSON)
    
    Detects the external type and dispatches to the appropriate handler.
    
    Parameters:
        EXT_NAME - Name of the external (e.g. "bass", "spdlog", "qt6")
        EXT_JSON - JSON definition of the external
    
    Type Detection (priority order):
        - "system" present → _handle_system_external()
        - "git" present    → _handle_fetched_external()
        - "path" present   → _attach_local_external()
        - None             → Error E012
    
    Example:
        _orchestrate_external("bass" "{\"path\":\"externals/bass\"}")
        _orchestrate_external("spdlog" "{\"git\":\"https://...\",\"tag\":\"v1.12.0\"}")
        _orchestrate_external("qt6" "{\"system\":true,\"package\":\"Qt6\"}")
]]
function(_orchestrate_external EXT_NAME EXT_JSON)
    
    # ==========================================================================
    # Validate: Exactly one source field
    # ==========================================================================
    
    validate_external_source("${EXT_NAME}" "${EXT_JSON}")
    
    # ==========================================================================
    # Detect Type (priority: system → git → path)
    # ==========================================================================
    
    _json_get_bool_or_default("${EXT_JSON}" "system" FALSE _is_system)
    _json_has_key("${EXT_JSON}" "git" _is_fetched)
    _json_has_key("${EXT_JSON}" "path" _is_local)
    
    # ==========================================================================
    # Dispatch
    # ==========================================================================
    
    if(_is_system)
        dbg(${DBG_RARE} "  Type: SYSTEM" ID EXTERNALS)
        _handle_system_external("${EXT_NAME}" "${EXT_JSON}")
        
    elseif(_is_fetched)
        dbg(${DBG_RARE} "  Type: FETCHED (git)" ID EXTERNALS)
        _handle_fetched_external("${EXT_NAME}" "${EXT_JSON}")
        
    elseif(_is_local)
        dbg(${DBG_RARE} "  Type: LOCAL" ID EXTERNALS)
        _attach_local_external("${EXT_NAME}" "${EXT_JSON}")
        
    else()
        # Should not reach here if validate_external_source works correctly
        cmake_fatal("E012" "External '${EXT_NAME}': No valid source field (system/git/path)")
    endif()
    
endfunction()

# ==============================================================================
# _get_external_options_for_target - Get options for a specific target
# ==============================================================================
#[[
    _get_external_options_for_target(TARGET_NAME EXT_NAME TARGET_JSON OUT_VAR)
    
    Extracts external_options for a specific external from target JSON.
    
    Parameters:
        TARGET_NAME - Name of the target (for debug output)
        EXT_NAME    - Name of the external
        TARGET_JSON - JSON of the target (executable/library)
        OUT_VAR     - Output variable for options JSON
    
    Returns:
        Options JSON or "{}" if not defined
    
    Example:
        _get_external_options_for_target("MyApp" "bass" "${_exe_json}" _opts)
        # _opts = {"BASS_FLAC": true, "BASS_FX": true}
]]
function(_get_external_options_for_target TARGET_NAME EXT_NAME TARGET_JSON OUT_VAR)
    # Check if external_options exists
    _json_has_key("${TARGET_JSON}" "external_options" _has_options)
    
    if(NOT _has_options)
        set(${OUT_VAR} "{}" PARENT_SCOPE)
        return()
    endif()
    
    # Get external_options object
    _json_get_object("${TARGET_JSON}" "external_options" _all_options)
    
    # Check if this external has options
    _json_has_key("${_all_options}" "${EXT_NAME}" _has_ext_options)
    
    if(NOT _has_ext_options)
        set(${OUT_VAR} "{}" PARENT_SCOPE)
        return()
    endif()
    
    # Get options for this external
    _json_get_object("${_all_options}" "${EXT_NAME}" _ext_options)
    
    dbg(${DBG_ULTRA_RARE} "    Options for ${EXT_NAME}: ${_ext_options}" ID EXTERNALS)
    
    set(${OUT_VAR} "${_ext_options}" PARENT_SCOPE)
endfunction()

# ==============================================================================
# apply_external_to_target - Apply an external to a target
# ==============================================================================
#[[
    apply_external_to_target(TARGET_NAME EXT_NAME EXT_OPTIONS)
    
    Loads and applies an external to a target.
    For local externals: Includes Include.cmake
    For fetched externals: Links the registered target
    
    Parameters:
        TARGET_NAME - CMake target to apply external to
        EXT_NAME    - Name of the external
        EXT_OPTIONS - JSON options for this external
    
    Sets variables for Local Include.cmake:
        EXTERNAL_NAME    - Name of the external
        EXTERNAL_ROOT    - Root path of the external
        EXTERNAL_OPTIONS - JSON options string
        EXECUTABLE_NAME  - Target name (legacy compatibility)
    
    Example:
        apply_external_to_target("MyApp" "bass" "{\"BASS_FLAC\":true}")
        apply_external_to_target("MyApp" "spdlog" "{}")
        apply_external_to_target("MyApp" "qt6" "{}")
]]
function(apply_external_to_target TARGET_NAME EXT_NAME EXT_OPTIONS)
    # ==========================================================================
    # Check if external is skipped
    # ==========================================================================
    
    get_property(_skipped_list GLOBAL PROPERTY SKIPPED_EXTERNALS)
    if("${EXT_NAME}" IN_LIST _skipped_list)
        cmake_fatal("E013" "External '${EXT_NAME}' is skipped but used by target '${TARGET_NAME}'. Remove from externals list or set skip: false")
    endif()
    
    # ==========================================================================
    # Get external definition from global property
    # ==========================================================================
    
    get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)
    
    _json_has_key("${_externals_json}" "${EXT_NAME}" _ext_defined)
    if(NOT _ext_defined)
        cmake_fatal("E010" "External '${EXT_NAME}' not defined in externals block")
    endif()
    
    _json_get_object("${_externals_json}" "${EXT_NAME}" _ext_json)
    
    # Check type (priority: system → git → path)
    _json_get_bool_or_default("${_ext_json}" "system" FALSE _is_system)
    _json_has_key("${_ext_json}" "git" _is_fetched)
    _json_has_key("${_ext_json}" "path" _is_local)
    
    if(_is_system)
        # =======================================================================
        # System External: Link registered target from find_package
        # =======================================================================
        
        dbg(${DBG_RARE} "    Applying ${EXT_NAME} to ${TARGET_NAME} (system)" ID EXTERNALS)
        
        # Apply using system handler
        _apply_system_external_to_target("${TARGET_NAME}" "${EXT_NAME}" "${EXT_OPTIONS}")
        
    elseif(_is_local)
        # =======================================================================
        # Local External: Include Include.cmake
        # =======================================================================
        
        _json_get_string("${_ext_json}" "path" _path)
        set(_ext_root "${CMAKE_SOURCE_DIR}/${_path}")
        
        # Check for custom include path or use convention
        _json_has_key("${_ext_json}" "include" _has_custom_include)
        if(_has_custom_include)
            _json_get_string("${_ext_json}" "include" _include_path)
            set(_include_file "${CMAKE_SOURCE_DIR}/${_include_path}")
        else()
            # Convention: cmake/externals/includes/{name}/Include.cmake
            set(_include_file "${CMAKE_SOURCE_DIR}/cmake/externals/includes/${EXT_NAME}/Include.cmake")
        endif()
        
        # Validate Include.cmake exists
        if(NOT EXISTS "${_include_file}")
            cmake_fatal("E213" "Local external '${EXT_NAME}': Include.cmake not found: ${_include_file}")
        endif()
        
        # Set variables for Include.cmake
        set(EXTERNAL_NAME "${EXT_NAME}")
        set(EXTERNAL_ROOT "${_ext_root}")
        set(EXTERNAL_OPTIONS "${EXT_OPTIONS}")
        
        # Legacy compatibility
        set(EXTERNAL_ELEMENT_NAME "${EXT_NAME}")
        set(EXTERNAL_ELEMENT_OPTIONS "${EXT_OPTIONS}")
        set(EXECUTABLE_NAME "${TARGET_NAME}")
        
        dbg(${DBG_RARE} "    Applying ${EXT_NAME} to ${TARGET_NAME} (local)" ID EXTERNALS)
        
        # Include the external's setup
        include("${_include_file}")
        
    elseif(_is_fetched)
        # =======================================================================
        # Fetched External: Link registered target
        # =======================================================================
        
        dbg(${DBG_RARE} "    Applying ${EXT_NAME} to ${TARGET_NAME} (fetched)" ID EXTERNALS)
        
        # Check if external is ready
        _is_external_ready("${EXT_NAME}" _is_ready)
        if(NOT _is_ready)
            cmake_warn("W101" "External '${EXT_NAME}' not ready when applying to ${TARGET_NAME}")
        endif()
        
        # Link using registry
        _link_external_to_target("${TARGET_NAME}" "${EXT_NAME}" SCOPE PRIVATE)
        
    else()
        cmake_fatal("E012" "External '${EXT_NAME}': Unknown type")
    endif()
endfunction()
