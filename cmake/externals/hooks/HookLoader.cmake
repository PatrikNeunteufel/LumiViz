# cmake/externals/Hooks/HookLoader.cmake
# =======================================
# Hook system for externals - loads PreFetch and PostFetch hooks
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
#
# Provides:
#   - _get_hook_name(EXT_NAME EXT_JSON OUT_HOOK_NAME)
#   - _load_prefetch_hook(EXT_NAME EXT_JSON)
#   - _load_postfetch_hook(EXT_NAME EXT_JSON)
#   - _check_hook_requirements(EXT_NAME EXT_JSON OUT_NEEDS_POSTFETCH)
#
# Convention Paths:
#   - PreFetch:  cmake/externals/Hooks/PreFetch/${name}.cmake
#   - PostFetch: cmake/externals/Hooks/PostFetch/${name}.cmake
#
# Hook Variables (available in hooks):
#   - HOOK_EXTERNAL_NAME  - Name of the external
#   - HOOK_EXTERNAL_JSON  - JSON definition
#   - HOOK_SOURCE_DIR     - Source directory (PostFetch only)
#
# Used by:
#   - Handler.cmake

include_guard(GLOBAL)

# ==============================================================================
# Convention Paths
# ==============================================================================

set(HOOKS_PREFETCH_DIR "${CMAKE_SOURCE_DIR}/cmake/externals/Hooks/PreFetch")
set(HOOKS_POSTFETCH_DIR "${CMAKE_SOURCE_DIR}/cmake/externals/Hooks/PostFetch")

# ==============================================================================
# _get_hook_name - Determine which hook to use
# ==============================================================================
#[[
    _get_hook_name(EXT_NAME EXT_JSON OUT_HOOK_NAME)
    
    Determines the hook name to use. If "hook" field is present in JSON,
    uses that name; otherwise uses the external's own name.
    
    Parameters:
        EXT_NAME      - Name of the external
        EXT_JSON      - JSON definition of the external
        OUT_HOOK_NAME - Output: Name of the hook to use
    
    Example:
        # External "imgui_docking" with "hook": "imgui"
        # → Returns "imgui"
]]
function(_get_hook_name EXT_NAME EXT_JSON OUT_HOOK_NAME)
    _json_has_key("${EXT_JSON}" "hook" _has_hook_override)
    
    if(_has_hook_override)
        _json_get_string("${EXT_JSON}" "hook" _hook_name)
        dbg(${DBG_COMMON} "[${EXT_NAME}] Using hook override: ${_hook_name}" ID EXTERNALS)
        set(${OUT_HOOK_NAME} "${_hook_name}" PARENT_SCOPE)
    else()
        set(${OUT_HOOK_NAME} "${EXT_NAME}" PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# _load_prefetch_hook - Load PreFetch hook if applicable
# ==============================================================================
#[[
    _load_prefetch_hook(EXT_NAME EXT_JSON)
    
    Loads a PreFetch hook based on convention or explicit configuration.
    PreFetch hooks run BEFORE FetchContent_MakeAvailable.
    
    Automatic lock prevents duplicate execution for the same external.
    
    Parameters:
        EXT_NAME - Name of the external
        EXT_JSON - JSON definition of the external
    
    Use Cases:
        - Set CMake options before external is configured
        - Disable examples/tests in the external
        - Set up cache variables
    
    Hook Variables:
        HOOK_EXTERNAL_NAME - Name of the external (use for target names!)
        HOOK_EXTERNAL_JSON - JSON definition
    
    Example Hook (cmake/externals/Hooks/PreFetch/glfw.cmake):
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
]]
function(_load_prefetch_hook EXT_NAME EXT_JSON)
    
    # ==========================================================================
    # Lock Check: Skip if already executed for this external
    # ==========================================================================
    
    get_property(_hook_done GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PREFETCH_DONE)
    if(_hook_done)
        dbg(${DBG_COMMON} "[${EXT_NAME}] PreFetch hook already executed, skipping" ID EXTERNALS)
        return()
    endif()
    
    # ==========================================================================
    # Determine hook name (own name or override via "hook" field)
    # ==========================================================================
    
    _get_hook_name("${EXT_NAME}" "${EXT_JSON}" _hook_name)
    
    # ==========================================================================
    # Check for explicit hook path in "hooks" object
    # ==========================================================================
    
    _json_has_key("${EXT_JSON}" "hooks" _has_hooks)
    
    set(_explicit_path "")
    set(_is_explicit FALSE)
    
    if(_has_hooks)
        _json_get_object("${EXT_JSON}" "hooks" _hooks_json)
        _json_has_key("${_hooks_json}" "preFetch" _has_prefetch)
        
        if(_has_prefetch)
            _json_get_string("${_hooks_json}" "preFetch" _explicit_path)
            set(_is_explicit TRUE)
        endif()
    endif()
    
    # ==========================================================================
    # Determine hook file path
    # ==========================================================================
    
    if(_is_explicit)
        # Explicit path specified in "hooks" object
        set(_hook_file "${CMAKE_SOURCE_DIR}/${_explicit_path}")
        
        if(NOT EXISTS "${_hook_file}")
            cmake_fatal("E216" "External '${EXT_NAME}': preFetch hook not found: ${_explicit_path}")
        endif()
        
        message(STATUS "[Externals]     PreFetch hook (explicit): ${_explicit_path}")
        
    else()
        # Convention path (using hook name, not external name)
        set(_hook_file "${HOOKS_PREFETCH_DIR}/${_hook_name}.cmake")
        
        if(NOT EXISTS "${_hook_file}")
            # No convention hook - this is fine
            dbg(${DBG_ULTRA_RARE} "[${EXT_NAME}] No PreFetch hook found" ID EXTERNALS)
            return()
        endif()
        
        message(STATUS "[Externals]     PreFetch hook (convention): ${_hook_name}.cmake")
    endif()
    
    # ==========================================================================
    # Set variables for hook
    # ==========================================================================
    
    set(HOOK_EXTERNAL_NAME "${EXT_NAME}")
    set(HOOK_EXTERNAL_JSON "${EXT_JSON}")
    
    # ==========================================================================
    # Load hook
    # ==========================================================================
    
    include("${_hook_file}")
    
    # ==========================================================================
    # Set lock AFTER successful execution
    # ==========================================================================
    
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PREFETCH_DONE TRUE)
    
endfunction()

# ==============================================================================
# _load_postfetch_hook - Load PostFetch hook if applicable
# ==============================================================================
#[[
    _load_postfetch_hook(EXT_NAME EXT_JSON)
    
    Loads a PostFetch hook based on convention or explicit configuration.
    PostFetch hooks run AFTER FetchContent_MakeAvailable.
    
    Automatic lock prevents duplicate execution for the same external.
    
    Parameters:
        EXT_NAME - Name of the external
        EXT_JSON - JSON definition of the external
    
    Use Cases:
        - Create targets for externals without CMakeLists.txt (e.g. ImGui)
        - Register targets in the registry
        - Apply additional configuration to targets
    
    Hook Variables:
        HOOK_EXTERNAL_NAME - Name of the external (use for target names!)
        HOOK_EXTERNAL_JSON - JSON definition
        HOOK_SOURCE_DIR    - Path to source directory
    
    IMPORTANT: Always use ${HOOK_EXTERNAL_NAME} for target names!
    This allows hook reuse for variants (e.g. imgui / imgui_docking).
    
    Example Hook (cmake/externals/Hooks/PostFetch/imgui.cmake):
        add_library(${HOOK_EXTERNAL_NAME} STATIC
            ${HOOK_SOURCE_DIR}/imgui.cpp
            ...
        )
        _register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)
]]
function(_load_postfetch_hook EXT_NAME EXT_JSON)
    
    # ==========================================================================
    # Lock Check: Skip if already executed for this external
    # ==========================================================================
    
    get_property(_hook_done GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_POSTFETCH_DONE)
    if(_hook_done)
        dbg(${DBG_COMMON} "[${EXT_NAME}] PostFetch hook already executed, skipping" ID EXTERNALS)
        return()
    endif()
    
    # ==========================================================================
    # Determine hook name (own name or override via "hook" field)
    # ==========================================================================
    
    _get_hook_name("${EXT_NAME}" "${EXT_JSON}" _hook_name)
    
    # ==========================================================================
    # Check for explicit hook path in "hooks" object
    # ==========================================================================
    
    _json_has_key("${EXT_JSON}" "hooks" _has_hooks)
    
    set(_explicit_path "")
    set(_is_explicit FALSE)
    
    if(_has_hooks)
        _json_get_object("${EXT_JSON}" "hooks" _hooks_json)
        _json_has_key("${_hooks_json}" "postFetch" _has_postfetch)
        
        if(_has_postfetch)
            _json_get_string("${_hooks_json}" "postFetch" _explicit_path)
            set(_is_explicit TRUE)
        endif()
    endif()
    
    # ==========================================================================
    # Determine hook file path
    # ==========================================================================
    
    if(_is_explicit)
        # Explicit path specified in "hooks" object
        set(_hook_file "${CMAKE_SOURCE_DIR}/${_explicit_path}")
        
        if(NOT EXISTS "${_hook_file}")
            cmake_fatal("E216" "External '${EXT_NAME}': postFetch hook not found: ${_explicit_path}")
        endif()
        
        message(STATUS "[Externals]     PostFetch hook (explicit): ${_explicit_path}")
        
    else()
        # Convention path (using hook name, not external name)
        set(_hook_file "${HOOKS_POSTFETCH_DIR}/${_hook_name}.cmake")
        
        if(NOT EXISTS "${_hook_file}")
            # No convention hook - this is fine for externals with CMakeLists.txt
            dbg(${DBG_ULTRA_RARE} "[${EXT_NAME}] No PostFetch hook found" ID EXTERNALS)
            return()
        endif()
        
        message(STATUS "[Externals]     PostFetch hook (convention): ${_hook_name}.cmake")
    endif()
    
    # ==========================================================================
    # Set variables for hook
    # ==========================================================================
    
    set(HOOK_EXTERNAL_NAME "${EXT_NAME}")
    set(HOOK_EXTERNAL_JSON "${EXT_JSON}")
    
    # Get source directory
    _get_external_source_dir("${EXT_NAME}" HOOK_SOURCE_DIR)
    
    # ==========================================================================
    # Load hook
    # ==========================================================================
    
    include("${_hook_file}")
    
    # ==========================================================================
    # Set lock AFTER successful execution
    # ==========================================================================
    
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_POSTFETCH_DONE TRUE)
    
endfunction()

# ==============================================================================
# _check_hook_requirements - Validate hook setup
# ==============================================================================
#[[
    _check_hook_requirements(EXT_NAME EXT_JSON OUT_NEEDS_POSTFETCH)
    
    Checks if an external requires a PostFetch hook (no CMakeLists.txt).
    
    Parameters:
        EXT_NAME          - Name of the external
        EXT_JSON          - JSON definition of the external
        OUT_NEEDS_POSTFETCH - Output: TRUE if PostFetch hook is required
]]
function(_check_hook_requirements EXT_NAME EXT_JSON OUT_NEEDS_POSTFETCH)
    
    # Check if external has cmakeSupport flag
    _json_has_key("${EXT_JSON}" "cmakeSupport" _has_cmake_flag)
    
    if(_has_cmake_flag)
        _json_get_bool_from_key("${EXT_JSON}" "cmakeSupport" _has_cmake)
        if(NOT _has_cmake)
            set(${OUT_NEEDS_POSTFETCH} TRUE PARENT_SCOPE)
            return()
        endif()
    endif()
    
    # Default: assume external has CMake support
    set(${OUT_NEEDS_POSTFETCH} FALSE PARENT_SCOPE)
    
endfunction()
