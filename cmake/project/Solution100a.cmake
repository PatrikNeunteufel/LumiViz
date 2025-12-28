# cmake/project/Solution.cmake
# ============================
# Project configuration loader - reads Solution.json and sets global properties
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
#   - SOLUTION_JSON                    (Global Property)
#   - SOLUTION_NAME                    (Global Property)
#   - SOLUTION_VERSION                 (Global Property)
#   - SOLUTION_DESCRIPTION             (Global Property)
#   - SOLUTION_AUTHORS                 (Global Property)
#   - SOLUTION_SCHEMA_VERSION          (Global Property)
#   - SOLUTION_EXTERNALS_JSON          (Global Property)
#   - SOLUTION_LIBRARIES_JSON          (Global Property)
#   - SOLUTION_EXECUTABLES_JSON        (Global Property)
#   - SOLUTION_SETTINGS_JSON           (Global Property)
#   - SOLUTION_CXX_STANDARD            (Global Property)
#   - SOLUTION_C_STANDARD              (Global Property)
#   - SOLUTION_DEFAULT_LIBRARY_TYPE    (Global Property)
#   - SOLUTION_DEFAULT_EXECUTABLE_TYPE (Global Property)
#   - SOLUTION_SOURCE_MODE             (Global Property)
#   - SOLUTION_EXTERNALS_CACHE_ROOT    (Global Property)
#   - SOLUTION_EXTERNALS_UPDATE_POLICY (Global Property)
#   - CMAKE_CXX_STANDARD               (Cache Variable)
#   - CMAKE_C_STANDARD                 (Cache Variable)
#
# Used by:
#   - Executables.cmake
#   - Libraries.cmake
#   - Tests.cmake
#   - Externals.cmake

include_guard(GLOBAL)

# ==============================================================================
# Debug Context Initialization
# ==============================================================================

dbg_init(ID SOLUTION LEVEL ${DBG_SHOW_MUCH} SWITCH ON TAG "Solution")

# ==============================================================================
# Read Solution.json
# ==============================================================================

set(_solution_file "${CMAKE_SOURCE_DIR}/Solution.json")

if(NOT EXISTS "${_solution_file}")
    cmake_fatal("E002" "Solution.json not found: ${_solution_file}")
endif()

file(READ "${_solution_file}" _solution_json_raw)
set_property(GLOBAL PROPERTY SOLUTION_JSON "${_solution_json_raw}")

dbg(${DBG_OFTEN} "Solution.json loaded" ID SOLUTION)

# ==============================================================================
# Schema Version Check
# ==============================================================================

_json_get_string("${_solution_json_raw}" "schemaVersion" _schema_version)

if("${_schema_version}" STREQUAL "")
    cmake_fatal("E002" "Solution.json: 'schemaVersion' missing or invalid JSON")
endif()

set_property(GLOBAL PROPERTY SOLUTION_SCHEMA_VERSION "${_schema_version}")

# Schema version 0.1 is the current version
# Note: Schema uses only MAJOR.MINOR (no PATCH)
if(NOT "${_schema_version}" VERSION_GREATER_EQUAL "0.1")
    cmake_warn("W001" "Solution.json schemaVersion ${_schema_version} < 0.1, some features may not be available")
endif()

dbg(${DBG_COMMON} "Schema version: ${_schema_version}" ID SOLUTION)

# ==============================================================================
# Extract Solution Block (Metadata)
# ==============================================================================

_json_get_object("${_solution_json_raw}" "solution" _solution_obj)

if("${_solution_obj}" STREQUAL "" OR "${_solution_obj}" STREQUAL "{}")
    cmake_fatal("E001" "Solution.json: required field 'solution' missing")
endif()

# Name (required)
_json_get_string("${_solution_obj}" "name" _solution_name)
if("${_solution_name}" STREQUAL "")
    cmake_fatal("E001" "Solution.json: 'solution.name' missing")
endif()
set_property(GLOBAL PROPERTY SOLUTION_NAME "${_solution_name}")

# Version (optional but recommended)
_json_has_key("${_solution_obj}" "version" _has_version)
if(_has_version)
    _json_get_type("${_solution_obj}" "version" _version_type)
    
    if("${_version_type}" STREQUAL "STRING")
        _json_get_string("${_solution_obj}" "version" _solution_version)
    elseif("${_version_type}" STREQUAL "OBJECT")
        # Support object format: { "major": 1, "minor": 2, "patch": 3 }
        _json_get_object("${_solution_obj}" "version" _version_obj)
        _json_get_string("${_version_obj}" "major" _v_major)
        _json_get_string("${_version_obj}" "minor" _v_minor)
        _json_get_string("${_version_obj}" "patch" _v_patch)
        set(_solution_version "${_v_major}.${_v_minor}.${_v_patch}")
    else()
        set(_solution_version "0.0.0")
    endif()
else()
    set(_solution_version "0.0.0")
endif()
set_property(GLOBAL PROPERTY SOLUTION_VERSION "${_solution_version}")

# Description (optional)
_json_get_string_or_default("${_solution_obj}" "description" "" _solution_description)
set_property(GLOBAL PROPERTY SOLUTION_DESCRIPTION "${_solution_description}")

# Authors (optional, array)
_json_array_length("${_solution_obj}" "authors" _authors_count)
set(_solution_authors "")
if(_authors_count GREATER 0)
    math(EXPR _authors_last "${_authors_count} - 1")
    foreach(_idx RANGE 0 ${_authors_last})
        _json_array_get("${_solution_obj}" "authors" ${_idx} _author)
        list(APPEND _solution_authors "${_author}")
    endforeach()
endif()
set_property(GLOBAL PROPERTY SOLUTION_AUTHORS "${_solution_authors}")

dbg(${DBG_OFTEN} "${_solution_name} v${_solution_version}" ID SOLUTION)
dbg(${DBG_RARE} "Description: ${_solution_description}" ID SOLUTION)
dbg(${DBG_RARE} "Authors: ${_solution_authors}" ID SOLUTION)

# ==============================================================================
# Extract Settings Block
# ==============================================================================

_json_get_object_or_empty("${_solution_json_raw}" "settings" _settings_obj)
set_property(GLOBAL PROPERTY SOLUTION_SETTINGS_JSON "${_settings_obj}")

# --- Standards ---
_json_has_key("${_settings_obj}" "standards" _has_standards)
if(_has_standards)
    _json_get_object("${_settings_obj}" "standards" _standards_obj)
    
    # C++ Standard
    _json_get_string("${_standards_obj}" "cxx_standard" _cxx_standard)
    if(NOT "${_cxx_standard}" STREQUAL "")
        set_property(GLOBAL PROPERTY SOLUTION_CXX_STANDARD "${_cxx_standard}")
        set(CMAKE_CXX_STANDARD ${_cxx_standard} CACHE STRING "C++ Standard" FORCE)
        dbg(${DBG_COMMON} "C++ Standard: ${_cxx_standard}" ID SOLUTION)
    endif()
    
    # C++ Standard Required
    _json_get_bool_from_key("${_standards_obj}" "cxx_standard_required" _cxx_required)
    if(_cxx_required)
        set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "C++ Standard Required" FORCE)
    endif()
    
    # C++ Extensions
    _json_has_key("${_standards_obj}" "cxx_extensions" _has_cxx_ext)
    if(_has_cxx_ext)
        _json_get_bool_from_key("${_standards_obj}" "cxx_extensions" _cxx_extensions)
        if(_cxx_extensions)
            set(CMAKE_CXX_EXTENSIONS ON CACHE BOOL "C++ Extensions" FORCE)
        else()
            set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "C++ Extensions" FORCE)
        endif()
    endif()
    
    # C Standard (optional)
    _json_get_string("${_standards_obj}" "c_standard" _c_standard)
    if(NOT "${_c_standard}" STREQUAL "")
        set_property(GLOBAL PROPERTY SOLUTION_C_STANDARD "${_c_standard}")
        set(CMAKE_C_STANDARD ${_c_standard} CACHE STRING "C Standard" FORCE)
        dbg(${DBG_COMMON} "C Standard: ${_c_standard}" ID SOLUTION)
    endif()
    
    # C Standard Required
    _json_get_bool_from_key("${_standards_obj}" "c_standard_required" _c_required)
    if(_c_required)
        set(CMAKE_C_STANDARD_REQUIRED ON CACHE BOOL "C Standard Required" FORCE)
    endif()
    
    # C Extensions
    _json_has_key("${_standards_obj}" "c_extensions" _has_c_ext)
    if(_has_c_ext)
        _json_get_bool_from_key("${_standards_obj}" "c_extensions" _c_extensions)
        if(_c_extensions)
            set(CMAKE_C_EXTENSIONS ON CACHE BOOL "C Extensions" FORCE)
        else()
            set(CMAKE_C_EXTENSIONS OFF CACHE BOOL "C Extensions" FORCE)
        endif()
    endif()
endif()

# --- Defaults ---
_json_has_key("${_settings_obj}" "defaults" _has_defaults)
if(_has_defaults)
    _json_get_object("${_settings_obj}" "defaults" _defaults_obj)
    
    _json_get_string_or_default("${_defaults_obj}" "library_type" "STATIC" _default_lib_type)
    set_property(GLOBAL PROPERTY SOLUTION_DEFAULT_LIBRARY_TYPE "${_default_lib_type}")
    
    _json_get_string_or_default("${_defaults_obj}" "executable_type" "CONSOLE" _default_exe_type)
    set_property(GLOBAL PROPERTY SOLUTION_DEFAULT_EXECUTABLE_TYPE "${_default_exe_type}")
    
    dbg(${DBG_RARE} "Default library type: ${_default_lib_type}" ID SOLUTION)
    dbg(${DBG_RARE} "Default executable type: ${_default_exe_type}" ID SOLUTION)
else()
    set_property(GLOBAL PROPERTY SOLUTION_DEFAULT_LIBRARY_TYPE "STATIC")
    set_property(GLOBAL PROPERTY SOLUTION_DEFAULT_EXECUTABLE_TYPE "CONSOLE")
endif()

# --- Sources (NEW in v0.1.0) ---
_json_has_key("${_settings_obj}" "sources" _has_sources)
if(_has_sources)
    _json_get_object("${_settings_obj}" "sources" _sources_obj)
    _json_get_string_or_default("${_sources_obj}" "mode" "explicit" _source_mode)
    set_property(GLOBAL PROPERTY SOLUTION_SOURCE_MODE "${_source_mode}")
    dbg(${DBG_COMMON} "Source mode: ${_source_mode}" ID SOLUTION)
else()
    set_property(GLOBAL PROPERTY SOLUTION_SOURCE_MODE "explicit")
endif()

# ==============================================================================
# Extract ExternalsPolicy Block
# ==============================================================================

_json_get_object_or_empty("${_solution_json_raw}" "externalsPolicy" _externals_policy_obj)
set_property(GLOBAL PROPERTY SOLUTION_EXTERNALS_POLICY_JSON "${_externals_policy_obj}")

_json_get_string_or_default("${_externals_policy_obj}" "cacheRoot" "externals/_cache" _cache_root)
set_property(GLOBAL PROPERTY SOLUTION_EXTERNALS_CACHE_ROOT "${_cache_root}")

_json_get_string_or_default("${_externals_policy_obj}" "sourceRoot" "externals/_src" _source_root)
set_property(GLOBAL PROPERTY SOLUTION_EXTERNALS_SOURCE_ROOT "${_source_root}")

_json_get_string_or_default("${_externals_policy_obj}" "updatePolicy" "checkout" _update_policy)
set_property(GLOBAL PROPERTY SOLUTION_EXTERNALS_UPDATE_POLICY "${_update_policy}")

dbg(${DBG_RARE} "Externals cache: ${_cache_root}" ID SOLUTION)
dbg(${DBG_RARE} "Externals source: ${_source_root}" ID SOLUTION)
dbg(${DBG_RARE} "Update policy: ${_update_policy}" ID SOLUTION)

# ==============================================================================
# Extract Externals Block (for later processing)
# ==============================================================================

_json_get_object_or_empty("${_solution_json_raw}" "externals" _externals_obj)
set_property(GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON "${_externals_obj}")

# Count externals for info
string(JSON _ext_count ERROR_VARIABLE _err LENGTH "${_solution_json_raw}" "externals")
if(_err)
    set(_ext_count 0)
endif()
dbg(${DBG_COMMON} "Externals defined: ${_ext_count}" ID SOLUTION)

# ==============================================================================
# Extract Libraries Array (NEW in v0.1.1 - for Libraries.cmake)
# ==============================================================================

string(JSON _libraries_json ERROR_VARIABLE _err GET "${_solution_json_raw}" "libraries")
if(_err)
    set(_libraries_json "[]")
endif()
set_property(GLOBAL PROPERTY SOLUTION_LIBRARIES_JSON "${_libraries_json}")

# ==============================================================================
# Extract Executables Array (NEW in v0.1.1 - for Executables.cmake)
# ==============================================================================

string(JSON _executables_json ERROR_VARIABLE _err GET "${_solution_json_raw}" "executables")
if(_err)
    set(_executables_json "[]")
endif()
set_property(GLOBAL PROPERTY SOLUTION_EXECUTABLES_JSON "${_executables_json}")

# ==============================================================================
# Count Executables/Libraries/Tests (for info)
# ==============================================================================

_json_array_length("${_solution_json_raw}" "executables" _exe_count)
_json_array_length("${_solution_json_raw}" "libraries" _lib_count)
_json_array_length("${_solution_json_raw}" "tests" _test_count)

dbg(${DBG_COMMON} "Executables: ${_exe_count}, Libraries: ${_lib_count}, Tests: ${_test_count}" ID SOLUTION)

enddbgblock(ID SOLUTION)

# ==============================================================================
# Cleanup
# ==============================================================================

unset(_solution_file)
unset(_solution_json_raw)
unset(_schema_version)
unset(_solution_obj)
unset(_solution_name)
unset(_solution_version)
unset(_solution_description)
unset(_solution_authors)
unset(_settings_obj)
unset(_standards_obj)
unset(_defaults_obj)
unset(_sources_obj)
unset(_source_mode)
unset(_externals_policy_obj)
unset(_externals_obj)
unset(_libraries_json)
unset(_executables_json)
