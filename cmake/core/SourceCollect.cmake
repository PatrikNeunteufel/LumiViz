# cmake/core/SourceCollect.cmake
# ================================
# Source file management for executables, libraries, and tests
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - Errors.cmake (cmake_fatal, cmake_warn)
#   - Debug.cmake (dbg, dbg_init)
#
# Provides:
#   - collect_sources()             - Main entry point
#   - _collect_sources_from_cmake() - Load Source.cmake
#   - _collect_sources_glob()       - GLOB fallback
#   - _apply_sources_to_target()    - Apply files to target
#   - _get_source_mode()            - Get mode from settings
#   - collect_files()               - Helper for controlled wildcards
#
# Source modes:
#   - explicit: Source.cmake required (Default)
#   - glob:     Automatic collection via wildcard
#   - auto:     Source.cmake if present, else GLOB (with fallback)
#
# File categories:
#   - SOURCES:   .cpp, .cxx, .cc, .c (compilable)
#   - HEADERS:   .h, .hpp, .hxx, .hh (declarations)
#   - TEMPLATES: .tpp, .txx, .ipp (template impl)
#   - INLINES:   .inl (inline impl)
#   - IMPL:      .impl (PIMPL details)
#   - MODULES:   .ixx, .cppm, .mpp (C++20, experimental)

include_guard(GLOBAL)

# ============================================================================
# Constants: File extensions
# ============================================================================

set(_SOURCE_EXTENSIONS     cpp cxx cc c)
set(_HEADER_EXTENSIONS     h hpp hxx hh)
set(_TEMPLATE_EXTENSIONS   tpp txx ipp)
set(_INLINE_EXTENSIONS     inl)
set(_IMPL_EXTENSIONS       impl)
set(_MODULE_EXTENSIONS     ixx cppm mpp)

# ============================================================================
# _get_source_mode - Get source mode from Solution settings
# ============================================================================
#[[
    _get_source_mode(OUT_VAR)
    
    Gets the source collection mode from Solution settings.
    
    Parameters:
        OUT_VAR - Mandatory: Output variable for the mode
    
    Returns:
        "explicit" (Default), "glob", or "auto"
    
    Example:
        _get_source_mode(_mode)
        # _mode = "explicit"
]]
function(_get_source_mode OUT_VAR)
    get_property(_mode GLOBAL PROPERTY SOLUTION_SOURCE_MODE)
    
    if("${_mode}" STREQUAL "")
        set(_mode "explicit")
    endif()
    
    # Validate mode
    if(NOT "${_mode}" MATCHES "^(explicit|glob|auto)$")
        cmake_warn("W101" "Invalid source.mode '${_mode}', using 'explicit'")
        set(_mode "explicit")
    endif()
    
    set(${OUT_VAR} "${_mode}" PARENT_SCOPE)
endfunction()

# ============================================================================
# _collect_sources_from_cmake - Load Source.cmake and collect files
# ============================================================================
#[[
    _collect_sources_from_cmake(TARGET_NAME SOURCE_DIR OUT_SOURCES OUT_HEADERS 
                                 OUT_EXTRAS OUT_MODULES OUT_INCLUDES)
    
    Loads Source.cmake from the specified directory and collects all
    defined files into output variables.
    
    Parameters:
        TARGET_NAME  - Mandatory: Name of the target (for variable prefix)
        SOURCE_DIR   - Mandatory: Directory containing Source.cmake
        OUT_SOURCES  - Mandatory: Output: Compilable files (.cpp, .c, ...)
        OUT_HEADERS  - Mandatory: Output: Header files (.h, .hpp, ...)
        OUT_EXTRAS   - Mandatory: Output: Templates, inlines, impl combined
        OUT_MODULES  - Mandatory: Output: C++20 module interface units
        OUT_INCLUDES - Mandatory: Output: Additional include paths
    
    Errors:
        E104 - Source.cmake not found
    
    Warnings:
        W101 - Source.cmake defines no files
        W109 - C++20 modules used (experimental)
    
    Example:
        _collect_sources_from_cmake(
            MyApp
            "${CMAKE_SOURCE_DIR}/projects/exec/MyApp/src"
            _sources _headers _extras _modules _includes
        )
]]
function(_collect_sources_from_cmake TARGET_NAME SOURCE_DIR OUT_SOURCES OUT_HEADERS OUT_EXTRAS OUT_MODULES OUT_INCLUDES)
    set(_source_cmake "${SOURCE_DIR}/Source.cmake")
    
    if(NOT EXISTS "${_source_cmake}")
        cmake_fatal("E104" "Source.cmake not found: ${_source_cmake}")
    endif()
    
    # Initialize target variables
    set(${TARGET_NAME}_SOURCES "")
    set(${TARGET_NAME}_HEADERS "")
    set(${TARGET_NAME}_TEMPLATES "")
    set(${TARGET_NAME}_INLINES "")
    set(${TARGET_NAME}_IMPL "")
    set(${TARGET_NAME}_MODULES "")
    set(${TARGET_NAME}_INCLUDES "")
    
    # Include Source.cmake (sets the variables)
    include("${_source_cmake}")
    
    # Collect results
    set(_sources "${${TARGET_NAME}_SOURCES}")
    set(_headers "${${TARGET_NAME}_HEADERS}")
    set(_modules "${${TARGET_NAME}_MODULES}")
    set(_includes "${${TARGET_NAME}_INCLUDES}")
    
    # Combine extras (templates + inlines + impl)
    set(_extras "")
    list(APPEND _extras ${${TARGET_NAME}_TEMPLATES})
    list(APPEND _extras ${${TARGET_NAME}_INLINES})
    list(APPEND _extras ${${TARGET_NAME}_IMPL})
    
    # Note: Validation for empty sources is done in collect_sources()
    # to allow proper fallback handling in auto mode
    
    # Warning for C++20 modules
    list(LENGTH _modules _module_count)
    if(_module_count GREATER 0)
        cmake_warn("W109" "Target '${TARGET_NAME}' uses C++20 modules (experimental)")
    endif()
    
    # Debug output
    list(LENGTH _sources _source_count)
    dbg(${DBG_RARE} "  Source.cmake loaded for ${TARGET_NAME}:" ID SOURCE_COLLECT)
    dbg(${DBG_RARE} "    SOURCES: ${_source_count} files" ID SOURCE_COLLECT)
    dbg(${DBG_ULTRA_RARE} "    HEADERS: ${_headers}" ID SOURCE_COLLECT)
    dbg(${DBG_ULTRA_RARE} "    EXTRAS: ${_extras}" ID SOURCE_COLLECT)
    
    # Set output
    set(${OUT_SOURCES} "${_sources}" PARENT_SCOPE)
    set(${OUT_HEADERS} "${_headers}" PARENT_SCOPE)
    set(${OUT_EXTRAS} "${_extras}" PARENT_SCOPE)
    set(${OUT_MODULES} "${_modules}" PARENT_SCOPE)
    set(${OUT_INCLUDES} "${_includes}" PARENT_SCOPE)
endfunction()

# ============================================================================
# _collect_sources_glob - GLOB-based collection (fallback)
# ============================================================================
#[[
    _collect_sources_glob(SOURCE_DIR OUT_SOURCES OUT_HEADERS OUT_EXTRAS OUT_MODULES)
    
    Collects all files from the directory via GLOB_RECURSE.
    Should only be used as fallback!
    
    Parameters:
        SOURCE_DIR   - Mandatory: Directory to search
        OUT_SOURCES  - Mandatory: Output: Compilable files
        OUT_HEADERS  - Mandatory: Output: Header files
        OUT_EXTRAS   - Mandatory: Output: Templates, inlines, impl
        OUT_MODULES  - Mandatory: Output: C++20 module units
    
    Warnings:
        W110 - GLOB fallback active
        W109 - C++20 modules found
    
    Example:
        _collect_sources_glob("${_path}" _sources _headers _extras _modules)
]]
function(_collect_sources_glob SOURCE_DIR OUT_SOURCES OUT_HEADERS OUT_EXTRAS OUT_MODULES)
    cmake_warn("W110" "GLOB fallback active for ${SOURCE_DIR} - explicit Source.cmake recommended")
    
    # Collect sources
    set(_sources "")
    foreach(_ext IN LISTS _SOURCE_EXTENSIONS)
        file(GLOB_RECURSE _files "${SOURCE_DIR}/*.${_ext}")
        list(APPEND _sources ${_files})
    endforeach()
    
    # Collect headers
    set(_headers "")
    foreach(_ext IN LISTS _HEADER_EXTENSIONS)
        file(GLOB_RECURSE _files "${SOURCE_DIR}/*.${_ext}")
        list(APPEND _headers ${_files})
    endforeach()
    
    # Collect extras (templates, inlines, impl)
    set(_extras "")
    foreach(_ext IN LISTS _TEMPLATE_EXTENSIONS _INLINE_EXTENSIONS _IMPL_EXTENSIONS)
        file(GLOB_RECURSE _files "${SOURCE_DIR}/*.${_ext}")
        list(APPEND _extras ${_files})
    endforeach()
    
    # Collect modules
    set(_modules "")
    foreach(_ext IN LISTS _MODULE_EXTENSIONS)
        file(GLOB_RECURSE _files "${SOURCE_DIR}/*.${_ext}")
        list(APPEND _modules ${_files})
    endforeach()
    
    # Warning for C++20 modules
    list(LENGTH _modules _module_count)
    if(_module_count GREATER 0)
        cmake_warn("W109" "C++20 modules found in ${SOURCE_DIR} (experimental)")
    endif()
    
    # Debug output
    list(LENGTH _sources _source_count)
    dbg(${DBG_RARE} "  GLOB collected: ${_source_count} sources" ID SOURCE_COLLECT)
    
    # Set output
    set(${OUT_SOURCES} "${_sources}" PARENT_SCOPE)
    set(${OUT_HEADERS} "${_headers}" PARENT_SCOPE)
    set(${OUT_EXTRAS} "${_extras}" PARENT_SCOPE)
    set(${OUT_MODULES} "${_modules}" PARENT_SCOPE)
endfunction()

# ============================================================================
# _apply_sources_to_target - Apply collected files to target
# ============================================================================
#[[
    _apply_sources_to_target(TARGET_NAME SOURCES HEADERS EXTRAS MODULES INCLUDES SOURCE_DIR)
    
    Applies the collected files to a CMake target.
    Automatically adds include paths.
    
    Parameters:
        TARGET_NAME - Mandatory: CMake target
        SOURCES     - Mandatory: Compilable files
        HEADERS     - Mandatory: Header files
        EXTRAS      - Mandatory: Templates, inlines, impl
        MODULES     - Mandatory: C++20 module units
        INCLUDES    - Mandatory: Additional include paths
        SOURCE_DIR  - Mandatory: Base directory (added as include)
    
    Automatically added include paths:
        1. SOURCE_DIR - always
        2. SOURCE_DIR/pch/ - if present
    
    Example:
        _apply_sources_to_target(
            MyApp
            "${_sources}" "${_headers}" "${_extras}" "${_modules}" "${_includes}"
            "${_source_dir}"
        )
]]
function(_apply_sources_to_target TARGET_NAME SOURCES HEADERS EXTRAS MODULES INCLUDES SOURCE_DIR)
    # Add all files to target
    target_sources(${TARGET_NAME} PRIVATE
        ${SOURCES}
        ${HEADERS}
        ${EXTRAS}
        ${MODULES}
    )
    
    # Collect include paths
    set(_include_dirs "")
    
    # 1. Source directory itself
    list(APPEND _include_dirs "${SOURCE_DIR}")
    
    # 2. PCH directory if present
    if(EXISTS "${SOURCE_DIR}/pch")
        list(APPEND _include_dirs "${SOURCE_DIR}/pch")
    endif()
    
    # 3. Additional include paths from Source.cmake
    foreach(_inc IN LISTS INCLUDES)
        if(IS_ABSOLUTE "${_inc}")
            list(APPEND _include_dirs "${_inc}")
        else()
            list(APPEND _include_dirs "${SOURCE_DIR}/${_inc}")
        endif()
    endforeach()
    
    # Set include paths
    target_include_directories(${TARGET_NAME} PRIVATE ${_include_dirs})
    
    # Debug output
    dbg(${DBG_RARE} "  Sources applied to ${TARGET_NAME}" ID SOURCE_COLLECT)
    dbg(${DBG_ULTRA_RARE} "    Include paths: ${_include_dirs}" ID SOURCE_COLLECT)
endfunction()

# ============================================================================
# collect_files - Helper for controlled wildcards in Source.cmake
# ============================================================================
#[[
    collect_files(OUT_VAR
        DIRECTORY <path>
        EXTENSIONS <ext1> [ext2...]
        [EXCLUDE <pattern1> [pattern2...] ]
    )
    
    Collects files with controlled wildcards.
    For use in Source.cmake with generated files.
    
    Parameters:
        OUT_VAR    - Mandatory: Output variable for the file list
        DIRECTORY  - Mandatory: Directory to search
        EXTENSIONS - Mandatory: List of extensions without dot
        EXCLUDE    - Optional: Regex patterns to exclude
    
    Example:
        collect_files(_generated
            DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/generated"
            EXTENSIONS cpp c
            EXCLUDE "*_test.cpp" "test_*.cpp" "*_mock.cpp"
        )
        list(APPEND _local_sources ${_generated})
]]
function(collect_files OUT_VAR)
    cmake_parse_arguments(ARG "" "DIRECTORY" "EXTENSIONS;EXCLUDE" ${ARGN})
    
    # Validation
    if(NOT ARG_DIRECTORY)
        cmake_fatal("E001" "collect_files: DIRECTORY is mandatory")
    endif()
    if(NOT ARG_EXTENSIONS)
        cmake_fatal("E001" "collect_files: EXTENSIONS is mandatory")
    endif()
    
    # Collect files
    set(_files "")
    foreach(_ext IN LISTS ARG_EXTENSIONS)
        file(GLOB_RECURSE _found "${ARG_DIRECTORY}/*.${_ext}")
        list(APPEND _files ${_found})
    endforeach()
    
    # Apply excludes
    if(ARG_EXCLUDE)
        foreach(_pattern IN LISTS ARG_EXCLUDE)
            list(FILTER _files EXCLUDE REGEX "${_pattern}")
        endforeach()
    endif()
    
    # Debug output
    list(LENGTH _files _count)
    dbg(${DBG_RARE} "  collect_files: ${_count} files in ${ARG_DIRECTORY}" ID SOURCE_COLLECT)
    
    set(${OUT_VAR} "${_files}" PARENT_SCOPE)
endfunction()

# ============================================================================
# collect_sources - Main entry point (combines mode logic)
# ============================================================================
#[[
    collect_sources(TARGET_NAME SOURCE_DIR OUT_SOURCES OUT_HEADERS 
                    OUT_EXTRAS OUT_MODULES OUT_INCLUDES)
    
    Main entry point for source collection.
    Automatically chooses between Source.cmake and GLOB based on mode.
    
    Parameters:
        TARGET_NAME  - Mandatory: Name of the target
        SOURCE_DIR   - Mandatory: Source directory
        OUT_SOURCES  - Mandatory: Output: Compilable files
        OUT_HEADERS  - Mandatory: Output: Header files
        OUT_EXTRAS   - Mandatory: Output: Templates, inlines, impl
        OUT_MODULES  - Mandatory: Output: C++20 module units
        OUT_INCLUDES - Mandatory: Output: Include paths
    
    Modes:
        explicit - Source.cmake required (E104 if missing)
        glob     - Always GLOB, Source.cmake ignored (W110)
        auto     - Source.cmake if present and non-empty, else GLOB
    
    Warnings:
        W101 - Source.cmake defines no files (explicit mode only)
        W110 - GLOB fallback active
        W111 - Source.cmake exists but is empty (auto mode, with path)
    
    Example:
        collect_sources(MyApp "${_path}" _src _hdr _ext _mod _inc)
]]
function(collect_sources TARGET_NAME SOURCE_DIR OUT_SOURCES OUT_HEADERS OUT_EXTRAS OUT_MODULES OUT_INCLUDES)
    _get_source_mode(_mode)
    
    set(_source_cmake "${SOURCE_DIR}/Source.cmake")
    set(_has_source_cmake FALSE)
    if(EXISTS "${_source_cmake}")
        set(_has_source_cmake TRUE)
    endif()
    
    dbg(${DBG_COMMON} "Collecting sources for ${TARGET_NAME} (mode: ${_mode})" ID SOURCE_COLLECT)
    
    if("${_mode}" STREQUAL "explicit")
        # Source.cmake required
        _collect_sources_from_cmake(
            ${TARGET_NAME} "${SOURCE_DIR}"
            _sources _headers _extras _modules _includes
        )
        # Warn if Source.cmake defines no sources (only in explicit mode)
        if(NOT _sources)
            cmake_warn("W101" "Source.cmake for '${TARGET_NAME}' defines no compilable files")
        endif()
    elseif("${_mode}" STREQUAL "glob")
        # Always GLOB
        _collect_sources_glob(
            "${SOURCE_DIR}"
            _sources _headers _extras _modules
        )
        set(_includes "")
    elseif("${_mode}" STREQUAL "auto")
        # Source.cmake if present AND defines files, else GLOB
        if(_has_source_cmake)
            _collect_sources_from_cmake(
                ${TARGET_NAME} "${SOURCE_DIR}"
                _sources _headers _extras _modules _includes
            )
            # Fallback to GLOB if Source.cmake defines no sources
            if(NOT _sources)
                cmake_warn("W111" "Source.cmake exists but defines no files: ${_source_cmake}")
                dbg(${DBG_COMMON} "  Falling back to GLOB" ID SOURCE_COLLECT)
                _collect_sources_glob(
                    "${SOURCE_DIR}"
                    _sources _headers _extras _modules
                )
                set(_includes "")
            endif()
        else()
            _collect_sources_glob(
                "${SOURCE_DIR}"
                _sources _headers _extras _modules
            )
            set(_includes "")
        endif()
    endif()
    
    # Set output
    set(${OUT_SOURCES} "${_sources}" PARENT_SCOPE)
    set(${OUT_HEADERS} "${_headers}" PARENT_SCOPE)
    set(${OUT_EXTRAS} "${_extras}" PARENT_SCOPE)
    set(${OUT_MODULES} "${_modules}" PARENT_SCOPE)
    set(${OUT_INCLUDES} "${_includes}" PARENT_SCOPE)
endfunction()
