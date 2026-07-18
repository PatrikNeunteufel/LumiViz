# cmake/externals/local/Attach.cmake
# ===================================
# Local external handler - validates and registers path-based externals
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
# Provides:
#   - _attach_local_external(EXT_NAME EXT_JSON)
#   - _validate_include_cmake(INCLUDE_FILE EXT_NAME)
#   - is_external_registered(EXT_NAME OUT_VAR)
#
# Used by:
#   - Orchestrator.cmake

include_guard(GLOBAL)

# ==============================================================================
# _attach_local_external - Register a Local External
# ==============================================================================
#[[
    _attach_local_external(EXT_NAME EXT_JSON)
    
    Validates and registers a local external for later use.
    The actual Include.cmake is loaded when a target requests the external.
    
    Parameters:
        EXT_NAME - Name of the external
        EXT_JSON - JSON definition with "path" field
    
    Validates:
        - Path exists
        - Include.cmake exists (or custom include path)
        - Best practices (W103, W104 warnings)
    
    Stores:
        - EXTERNAL_${NAME}_PATH property
        - EXTERNAL_${NAME}_INCLUDE property
        - EXTERNAL_${NAME}_REGISTERED = TRUE
    
    Example:
        _attach_local_external("bass" "{\"path\":\"externals/bass\"}")
]]
function(_attach_local_external EXT_NAME EXT_JSON)
    
    # ==========================================================================
    # Extract Path
    # ==========================================================================
    
    _json_get_string("${EXT_JSON}" "path" _path)
    
    if("${_path}" STREQUAL "")
        cmake_fatal("E001" "Local external '${EXT_NAME}': 'path' field is empty")
    endif()
    
    set(_ext_root "${CMAKE_SOURCE_DIR}/${_path}")
    
    # ==========================================================================
    # Validate Path Exists
    # ==========================================================================
    
    if(NOT EXISTS "${_ext_root}")
        cmake_fatal("E214" "Local external '${EXT_NAME}': Path does not exist: ${_path}")
    endif()
    
    dbg(${DBG_RARE} "  Path: ${_path}" ID EXTERNALS)
    
    # ==========================================================================
    # Determine Include.cmake Path
    # ==========================================================================
    
    _json_has_key("${EXT_JSON}" "include" _has_custom_include)
    if(_has_custom_include)
        _json_get_string("${EXT_JSON}" "include" _include_path)
        set(_include_file "${CMAKE_SOURCE_DIR}/${_include_path}")
    else()
        # Convention: cmake/externals/includes/{name}/Include.cmake
        set(_include_file "${CMAKE_SOURCE_DIR}/cmake/externals/includes/${EXT_NAME}/Include.cmake")
    endif()
    # ==========================================================================
    # Validate Include.cmake Exists
    # ==========================================================================
    
    if(NOT EXISTS "${_include_file}")
        cmake_fatal("E213" "Local external '${EXT_NAME}': Include.cmake not found: ${_include_file}")
    endif()
    
    # ==========================================================================
    # Best Practice Validation (Warnings Only)
    # ==========================================================================
    
    _validate_include_cmake("${_include_file}" "${EXT_NAME}")
    
    # ==========================================================================
    # Register External
    # ==========================================================================
    
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_PATH "${_ext_root}")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_INCLUDE "${_include_file}")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_REGISTERED TRUE)
    
    dbg(${DBG_COMMON} "  Registered: ${EXT_NAME}" ID EXTERNALS)
    
endfunction()

# ==============================================================================
# _validate_include_cmake - Best Practice Checks
# ==============================================================================
#[[
    _validate_include_cmake(INCLUDE_FILE EXT_NAME)
    
    Performs best practice checks on an Include.cmake file.
    Only generates warnings, does not fail the build.
    
    Parameters:
        INCLUDE_FILE - Full path to Include.cmake
        EXT_NAME     - Name of the external (for messages)
    
    Warnings:
        W103 - Contains add_executable() (externals shouldn't create executables)
        W104 - Contains add_subdirectory(examples/tests/...) (bloats build)
    
    Example:
        _validate_include_cmake("/path/to/Include.cmake" "bass")
]]
function(_validate_include_cmake INCLUDE_FILE EXT_NAME)
    # Read file content
    file(READ "${INCLUDE_FILE}" _content)
    
    # W103: Check for add_executable()
    string(FIND "${_content}" "add_executable(" _pos_exe)
    if(NOT _pos_exe EQUAL -1)
        cmake_warn("W103" "External '${EXT_NAME}': Include.cmake contains add_executable() - externals should only provide libraries")
    endif()
    
    # W104: Check for add_subdirectory with examples/tests/samples
    string(REGEX MATCH "add_subdirectory[[:space:]]*\\([[:space:]]*(examples|tests|samples|test|example)" _match "${_content}")
    if(NOT "${_match}" STREQUAL "")
        cmake_warn("W104" "External '${EXT_NAME}': Include.cmake adds examples/tests subdirectory - consider disabling")
    endif()
    
endfunction()

# ==============================================================================
# is_external_registered - Check if external is registered
# ==============================================================================
#[[
    is_external_registered(EXT_NAME OUT_VAR)
    
    Checks if an external has been registered.
    
    Parameters:
        EXT_NAME - Name of the external
        OUT_VAR  - Output: TRUE or FALSE
    
    Example:
        is_external_registered("bass" _registered)
        if(_registered)
            message("bass is available")
        endif()
]]
function(is_external_registered EXT_NAME OUT_VAR)
    get_property(_registered GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_REGISTERED)
    
    if("${_registered}" STREQUAL "TRUE")
        set(${OUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()
