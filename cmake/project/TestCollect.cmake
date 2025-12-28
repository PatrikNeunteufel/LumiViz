# cmake/project/TestCollect.cmake
# ================================
# Collects test data from JSON into a Context
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - cmake/core/Context.cmake
#   - cmake/core/Json.cmake
#   - cmake/core/Debug.cmake
#
# Provides:
#   - _collect_test(TEST_JSON CTX)
#
# Note: JSON helper functions are provided by cmake/core/Json.cmake:
#   - _json_get_bool_or_default()
#   - _json_get_number_or_default()
#   - _json_get_string_or_default()
#   - _json_get_array_as_list()
#
# Context Keys Set:
#   - NAME, DISPLAY_NAME, VERSION, TYPE, FRAMEWORK, PATH
#   - TARGET, DEPENDENCIES, EXTERNALS, EXTERNAL_OPTIONS
#   - TIMEOUT, LABELS, PARALLEL, SKIP, PLATFORMS
#   - DEFINES, COMPILE_OPTIONS, SOURCE_FROM, EXCLUDE_SOURCES
#
# Used by:
#   - Tests.cmake

include_guard(GLOBAL)

# ==============================================================================
# Default Values
# ==============================================================================

set(_TEST_DEFAULT_TYPE "unit")
set(_TEST_DEFAULT_FRAMEWORK "doctest")
set(_TEST_DEFAULT_TIMEOUT 60)
set(_TEST_DEFAULT_PARALLEL TRUE)

# ==============================================================================
# _collect_test - Main Collection Function
# ==============================================================================
#[[
    _collect_test(TEST_JSON CTX)
    
    Parses test JSON and populates a Context with all fields.
    
    Parameters:
        TEST_JSON - JSON string of the test definition
        CTX       - Context prefix (e.g., TEST_0, TEST_1)
    
    Example:
        ctx_create(TEST_0)
        _collect_test("${_test_json}" TEST_0)
        ctx_get(TEST_0 NAME _name)
]]
function(_collect_test TEST_JSON CTX)
    
    # ==========================================================================
    # Required Fields
    # ==========================================================================
    
    # NAME (required)
    _json_has_key("${TEST_JSON}" "name" _has_name)
    if(NOT _has_name)
        cmake_fatal("E001" "Test has no 'name' field")
    endif()
    string(JSON _name GET "${TEST_JSON}" "name")
    ctx_set(${CTX} NAME "${_name}")
    
    # ==========================================================================
    # Optional Fields with Defaults
    # ==========================================================================
    
    # DISPLAY_NAME
    _json_get_string_or_default("${TEST_JSON}" "displayName" "${_name}" _display_name)
    ctx_set(${CTX} DISPLAY_NAME "${_display_name}")
    
    # VERSION (default: Solution version)
    get_property(_solution_version GLOBAL PROPERTY SOLUTION_VERSION)
    _json_get_string_or_default("${TEST_JSON}" "version" "${_solution_version}" _version)
    ctx_set(${CTX} VERSION "${_version}")
    
    # TYPE (unit, integration, system, performance, smoke)
    _json_get_string_or_default("${TEST_JSON}" "type" "${_TEST_DEFAULT_TYPE}" _type)
    string(TOLOWER "${_type}" _type)
    ctx_set(${CTX} TYPE "${_type}")
    
    # FRAMEWORK (doctest, googletest, catch2)
    _json_get_string_or_default("${TEST_JSON}" "framework" "${_TEST_DEFAULT_FRAMEWORK}" _framework)
    string(TOLOWER "${_framework}" _framework)
    ctx_set(${CTX} FRAMEWORK "${_framework}")
    
    # PATH (with convention)
    _json_has_key("${TEST_JSON}" "path" _has_path)
    if(_has_path)
        string(JSON _path GET "${TEST_JSON}" "path")
    else()
        # Convention: projects/tests/{type}/{name}/src
        set(_path "projects/tests/${_type}/${_name}/src")
    endif()
    ctx_set(${CTX} PATH "${_path}")
    
    # TARGET (the target being tested, for coverage)
    _json_get_string_or_default("${TEST_JSON}" "target" "" _target)
    ctx_set(${CTX} TARGET "${_target}")
    
    # SKIP
    _json_get_bool_or_default("${TEST_JSON}" "skip" FALSE _skip)
    ctx_set(${CTX} SKIP "${_skip}")
    dbg(${DBG_ULTRA_RARE} "    SKIP parsed: ${_skip}" ID TESTS)
    
    # TIMEOUT
    _json_get_number_or_default("${TEST_JSON}" "timeout" ${_TEST_DEFAULT_TIMEOUT} _timeout)
    ctx_set(${CTX} TIMEOUT "${_timeout}")
    
    # PARALLEL
    _json_get_bool_or_default("${TEST_JSON}" "parallel" ${_TEST_DEFAULT_PARALLEL} _parallel)
    ctx_set(${CTX} PARALLEL "${_parallel}")
    
    # ==========================================================================
    # Array Fields
    # ==========================================================================
    
    # DEPENDENCIES
    _json_get_array_as_list("${TEST_JSON}" "dependencies" _dependencies)
    ctx_set(${CTX} DEPENDENCIES "${_dependencies}")
    
    # EXTERNALS
    _json_get_array_as_list("${TEST_JSON}" "externals" _externals)
    ctx_set(${CTX} EXTERNALS "${_externals}")
    
    # EXTERNAL_OPTIONS (keep as JSON)
    _json_has_key("${TEST_JSON}" "external_options" _has_ext_opts)
    if(_has_ext_opts)
        string(JSON _ext_opts GET "${TEST_JSON}" "external_options")
    else()
        set(_ext_opts "{}")
    endif()
    ctx_set(${CTX} EXTERNAL_OPTIONS "${_ext_opts}")
    
    # LABELS (default to [type] if not specified)
    _json_has_key("${TEST_JSON}" "labels" _has_labels)
    if(_has_labels)
        _json_get_array_as_list("${TEST_JSON}" "labels" _labels)
    else()
        set(_labels "${_type}")
    endif()
    ctx_set(${CTX} LABELS "${_labels}")
    
    # PLATFORMS
    _json_get_array_as_list("${TEST_JSON}" "platforms" _platforms)
    ctx_set(${CTX} PLATFORMS "${_platforms}")
    
    # DEFINES
    _json_get_array_as_list("${TEST_JSON}" "defines" _defines)
    ctx_set(${CTX} DEFINES "${_defines}")
    
    # COMPILE_OPTIONS
    _json_get_array_as_list("${TEST_JSON}" "compile_options" _compile_options)
    ctx_set(${CTX} COMPILE_OPTIONS "${_compile_options}")
    
    # SOURCE_FROM (Executable-Sources übernehmen)
    _json_get_string_or_default("${TEST_JSON}" "source_from" "" _source_from)
    ctx_set(${CTX} SOURCE_FROM "${_source_from}")
    
    # EXCLUDE_SOURCES (mit source_from)
    _json_get_array_as_list("${TEST_JSON}" "exclude_sources" _exclude_sources)
    ctx_set(${CTX} EXCLUDE_SOURCES "${_exclude_sources}")
    
    # ==========================================================================
    # Debug Output
    # ==========================================================================
    
    dbg(${DBG_RARE} "  Collected ${_name}:" ID TESTS)
    dbg(${DBG_RARE} "    TYPE: ${_type}" ID TESTS)
    dbg(${DBG_RARE} "    FRAMEWORK: ${_framework}" ID TESTS)
    dbg(${DBG_RARE} "    PATH: ${_path}" ID TESTS)
    dbg(${DBG_RARE} "    DEPENDENCIES: ${_dependencies}" ID TESTS)
    dbg(${DBG_RARE} "    EXTERNALS: ${_externals}" ID TESTS)
    dbg(${DBG_RARE} "    LABELS: ${_labels}" ID TESTS)
    if(_source_from)
        dbg(${DBG_RARE} "    SOURCE_FROM: ${_source_from}" ID TESTS)
        dbg(${DBG_RARE} "    EXCLUDE_SOURCES: ${_exclude_sources}" ID TESTS)
    endif()
    
endfunction()

# ==============================================================================
# NOTE: JSON helper functions are now centralized in cmake/core/Json.cmake
# The following functions are available:
#   - _json_get_bool_or_default()
#   - _json_get_number_or_default() (use instead of _json_get_int_or_default)
#   - _json_get_string_or_default()
#   - _json_get_array_as_list()
# ==============================================================================
