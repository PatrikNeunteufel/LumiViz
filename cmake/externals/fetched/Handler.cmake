# cmake/externals/Fetched/Handler.cmake
# ======================================
# Handler for git-based externals - orchestrates fetch process with hooks
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
#   - cmake/externals/Core/Fetch.cmake
#   - cmake/externals/Hooks/HookLoader.cmake
#   - cmake/externals/Registry/Targets.cmake
#
# Provides:
#   - _handle_fetched_external(EXT_NAME EXT_JSON)
#   - _is_external_ready(EXT_NAME OUT_VAR)
#
# Pipeline:
#   1. Declare (FetchContent_Declare)
#   2. PreFetch hook
#   3. MakeAvailable
#   4. PostFetch hook
#   5. Auto-register targets
#   6. Validate
#
# Used by:
#   - Orchestrator.cmake

include_guard(GLOBAL)

# ==============================================================================
# Load Dependencies
# ==============================================================================

include(cmake/externals/core/Fetch.cmake)
include(cmake/externals/hooks/HookLoader.cmake)
include(cmake/externals/registry/Targets.cmake)

# ==============================================================================
# _handle_fetched_external - Main entry point for fetched externals
# ==============================================================================
#[[
    _handle_fetched_external(EXT_NAME EXT_JSON)
    
    Processes a fetched external through the complete pipeline.
    
    Parameters:
        EXT_NAME - Name of the external
        EXT_JSON - JSON definition of the external
    
    Pipeline:
        1. Declare (FetchContent_Declare) - checks .externals/ cache
        2. PreFetch hook
        3. MakeAvailable - downloads only if not cached
        4. PostFetch hook
        5. Auto-register targets
        6. Validate
    
    Source Directory:
        ${CMAKE_SOURCE_DIR}/.externals/${EXT_NAME}/
        (shared across all build presets)
    
    Example:
        _handle_fetched_external("spdlog" "{\"git\":\"https://...\",\"tag\":\"v1.12.0\"}")
]]
function(_handle_fetched_external EXT_NAME EXT_JSON)
    
    message(STATUS "[Externals] Processing: ${EXT_NAME}")
    
    # ==========================================================================
    # Step 1: Declare External (checks cache)
    # ==========================================================================
    
    _fetch_git_external("${EXT_NAME}" "${EXT_JSON}")
    
    # ==========================================================================
    # Step 2: PreFetch Hook
    # ==========================================================================
    
    _load_prefetch_hook("${EXT_NAME}" "${EXT_JSON}")
    
    # ==========================================================================
    # Step 3: Make Available (Download if not cached)
    # ==========================================================================
    
    _make_external_available("${EXT_NAME}")
    
    # Get source directory for logging
    _get_external_source_dir("${EXT_NAME}" _source_dir)
    dbg(${DBG_RARE} "[${EXT_NAME}] Source: ${_source_dir}" ID EXTERNALS)
    
    # ==========================================================================
    # Step 4: PostFetch Hook
    # ==========================================================================
    
    _load_postfetch_hook("${EXT_NAME}" "${EXT_JSON}")
    
    # ==========================================================================
    # Step 5: Auto-Register Targets
    # ==========================================================================
    
    _auto_register_external_targets("${EXT_NAME}")
    
    # ==========================================================================
    # Step 6: Validate
    # ==========================================================================
    
    # Check if we require a PostFetch hook but don't have targets
    _check_hook_requirements("${EXT_NAME}" "${EXT_JSON}" _needs_postfetch)
    
    if(_needs_postfetch)
        _has_external_target("${EXT_NAME}" _has_targets)
        if(NOT _has_targets)
            cmake_fatal("E201" "Fetched external '${EXT_NAME}': No CMake support, PostFetch hook required but no targets created")
        endif()
    endif()
    
    # Final validation
    _validate_external_targets("${EXT_NAME}")
    
    # ==========================================================================
    # Mark as fully processed
    # ==========================================================================
    
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_READY TRUE)
    
    message(STATUS "[Externals] ${EXT_NAME}: Ready")
    
endfunction()

# ==============================================================================
# _is_external_ready - Check if external is ready to use
# ==============================================================================
#[[
    _is_external_ready(EXT_NAME OUT_VAR)
    
    Checks if an external has been fully processed and is ready.
    
    Parameters:
        EXT_NAME - Name of the external
        OUT_VAR  - Output variable (TRUE/FALSE)
]]
function(_is_external_ready EXT_NAME OUT_VAR)
    get_property(_ready GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_READY)
    
    if(_ready)
        set(${OUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()
