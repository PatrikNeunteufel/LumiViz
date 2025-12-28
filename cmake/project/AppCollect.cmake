# cmake/project/AppCollect.cmake
# ================================
# Collects App-Container data from JSON into a Context
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - cmake/core/Json.cmake
#   - cmake/core/Context.cmake
#   - cmake/core/Debug.cmake
#   - cmake/core/Errors.cmake
#
# Provides:
#   - _collect_app(APP_JSON CTX)
#
# Context Keys Set:
#   Base:
#     - NAME, DISPLAY_NAME, DESCRIPTION, VERSION, PATH
#   Core:
#     - CORE_DEPENDENCIES, CORE_EXTERNALS, CORE_EXTERNAL_OPTIONS
#   Runner:
#     - RUNNER_TYPE, RUNNER_EXTERNALS, RUNNER_EXTERNAL_OPTIONS
#   PCH:
#     - PCH_ENABLED, PCH_HEADER, PCH_PATH
#   Tests:
#     - TESTS_FRAMEWORK (global default)
#     - TESTS_SKIP (global skip all tests)
#     - TESTS_TARGETS_COUNT
#     - TESTS_TARGET_{n}_NAME, TESTS_TARGET_{n}_TYPE, TESTS_TARGET_{n}_SKIP,
#       TESTS_TARGET_{n}_PATH, TESTS_TARGET_{n}_FRAMEWORK, TESTS_TARGET_{n}_TIMEOUT,
#       TESTS_TARGET_{n}_LABELS, TESTS_TARGET_{n}_EXTERNALS, TESTS_TARGET_{n}_PARALLEL
#   Filter:
#     - PLATFORMS, SKIP
#
# Used by:
#   - Apps.cmake

include_guard(GLOBAL)

# ============================================================================
# Test Type Defaults
# ============================================================================
# Known test types with their default configurations.
# Unknown types use generic defaults.

# Default timeouts per type (in seconds)
set(_TEST_TYPE_TIMEOUT_unit 30)
set(_TEST_TYPE_TIMEOUT_integration 120)
set(_TEST_TYPE_TIMEOUT_performance 300)
set(_TEST_TYPE_TIMEOUT_system 180)
set(_TEST_TYPE_TIMEOUT_smoke 10)
set(_TEST_TYPE_TIMEOUT_fuzz 60)
set(_TEST_TYPE_TIMEOUT_security 120)
set(_TEST_TYPE_TIMEOUT_ui 180)
set(_TEST_TYPE_TIMEOUT_api 60)
set(_TEST_TYPE_TIMEOUT_DEFAULT 60)

# Default parallel setting per type (types that should NOT run parallel)
set(_TEST_TYPE_SERIAL_TYPES "performance;system;fuzz;security;ui")

# Default labels per type
set(_TEST_TYPE_LABELS_unit "unit;fast")
set(_TEST_TYPE_LABELS_integration "integration")
set(_TEST_TYPE_LABELS_performance "performance;benchmark")
set(_TEST_TYPE_LABELS_system "system;e2e;slow")
set(_TEST_TYPE_LABELS_smoke "smoke;critical;fast")
set(_TEST_TYPE_LABELS_fuzz "fuzz;security")
set(_TEST_TYPE_LABELS_security "security")
set(_TEST_TYPE_LABELS_ui "ui;slow")
set(_TEST_TYPE_LABELS_api "api;integration")

# ============================================================================
# _get_test_type_defaults - Get defaults for a test type
# ============================================================================
#[[
    _get_test_type_defaults(TYPE OUT_TIMEOUT OUT_LABELS OUT_PARALLEL)
    
    Returns default values for a test type.
    Unknown types get generic defaults.
]]
function(_get_test_type_defaults TYPE OUT_TIMEOUT OUT_LABELS OUT_PARALLEL)
    # Timeout
    if(DEFINED _TEST_TYPE_TIMEOUT_${TYPE})
        set(${OUT_TIMEOUT} "${_TEST_TYPE_TIMEOUT_${TYPE}}" PARENT_SCOPE)
    else()
        set(${OUT_TIMEOUT} "${_TEST_TYPE_TIMEOUT_DEFAULT}" PARENT_SCOPE)
    endif()
    
    # Labels
    if(DEFINED _TEST_TYPE_LABELS_${TYPE})
        set(${OUT_LABELS} "${_TEST_TYPE_LABELS_${TYPE}}" PARENT_SCOPE)
    else()
        set(${OUT_LABELS} "${TYPE}" PARENT_SCOPE)
    endif()
    
    # Parallel (false for known serial types)
    if("${TYPE}" IN_LIST _TEST_TYPE_SERIAL_TYPES)
        set(${OUT_PARALLEL} FALSE PARENT_SCOPE)
    else()
        set(${OUT_PARALLEL} TRUE PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _collect_app - Collects App-Container data from JSON into Context
# ============================================================================
#[[
    _collect_app(APP_JSON CTX)
    
    Parses an App-Container definition from JSON and stores all fields
    in a Context for later processing by AppCreate.
    
    Parameters:
        APP_JSON - Mandatory: JSON object string containing app definition
        CTX      - Mandatory: Context prefix (e.g. APP_0, APP_1)
    
    JSON Schema:
        {
            "name": "AppName",              // Required
            "displayName": "Display Name",  // Optional, defaults to name
            "version": "1.0.0",             // Optional
            "description": "...",           // Optional
            "path": "projects/apps/...",    // Optional, defaults to projects/apps/{name}
            "skip": false,                  // Optional
            
            "core": {
                "dependencies": [],
                "externals": [],
                "external_options": {}
            },
            
            "runner": {
                "type": "CONSOLE|GUI",
                "externals": [],
                "external_options": {}
            },
            
            "pch": {
                "enabled": true,
                "header": "pch.h",
                "path": ""
            },
            
            "tests": {
                "framework": "doctest",     // Global default
                "targets": [
                    {
                        "name": "Core_UnitTests",
                        "type": "unit",
                        "path": "tests/unit/Core_UnitTests",
                        "framework": "doctest",
                        "timeout": 30,
                        "labels": ["unit", "fast"],
                        "externals": [],
                        "parallel": true
                    }
                ]
            },
            
            "platforms": ["windows", "linux", "macos"]
        }
    
    Example:
        _collect_app("${_app_json}" APP_0)
        ctx_get(APP_0 NAME _name)
]]
function(_collect_app APP_JSON CTX)
    
    # ==========================================================================
    # Base Fields
    # ==========================================================================
    
    # Name (required)
    _json_has_key("${APP_JSON}" "name" _has_name)
    if(NOT _has_name)
        cmake_fatal("E400" "App definition missing required 'name' field")
    endif()
    string(JSON _name GET "${APP_JSON}" "name")
    
    # Display Name (optional, defaults to name)
    _json_get_string_or_default("${APP_JSON}" "displayName" "${_name}" _display_name)
    
    # Description (optional)
    _json_get_string_or_default("${APP_JSON}" "description" "" _description)
    
    # Version (optional)
    _json_get_string_or_default("${APP_JSON}" "version" "" _version)
    
    # Path (optional, defaults to projects/apps/{name})
    _json_get_string_or_default("${APP_JSON}" "path" "projects/apps/${_name}" _path)
    
    # Skip (optional)
    set(_skip FALSE)
    _json_has_key("${APP_JSON}" "skip" _has_skip)
    if(_has_skip)
        _json_get_bool_from_key("${APP_JSON}" "skip" _skip)
    endif()
    
    ctx_set(${CTX} NAME "${_name}")
    ctx_set(${CTX} DISPLAY_NAME "${_display_name}")
    ctx_set(${CTX} DESCRIPTION "${_description}")
    ctx_set(${CTX} VERSION "${_version}")
    ctx_set(${CTX} PATH "${_path}")
    ctx_set(${CTX} SKIP "${_skip}")
    
    # ==========================================================================
    # Core Section
    # ==========================================================================
    
    set(_core_dependencies "")
    set(_core_externals "")
    set(_core_external_options "")
    
    _json_has_key("${APP_JSON}" "core" _has_core)
    if(_has_core)
        _json_get_object("${APP_JSON}" "core" _core_obj)
        
        # Dependencies
        _json_array_length("${_core_obj}" "dependencies" _dep_count)
        if(_dep_count GREATER 0)
            math(EXPR _dep_last "${_dep_count} - 1")
            foreach(_dep_idx RANGE 0 ${_dep_last})
                _json_array_get("${_core_obj}" "dependencies" ${_dep_idx} _dep)
                list(APPEND _core_dependencies "${_dep}")
            endforeach()
        endif()
        
        # Externals
        _json_array_length("${_core_obj}" "externals" _ext_count)
        if(_ext_count GREATER 0)
            math(EXPR _ext_last "${_ext_count} - 1")
            foreach(_ext_idx RANGE 0 ${_ext_last})
                _json_array_get("${_core_obj}" "externals" ${_ext_idx} _ext)
                list(APPEND _core_externals "${_ext}")
            endforeach()
        endif()
        
        # External Options (JSON block for later processing)
        _json_get_object_or_empty("${_core_obj}" "external_options" _core_external_options)
    endif()
    
    ctx_set(${CTX} CORE_DEPENDENCIES "${_core_dependencies}")
    ctx_set(${CTX} CORE_EXTERNALS "${_core_externals}")
    ctx_set(${CTX} CORE_EXTERNAL_OPTIONS "${_core_external_options}")
    
    # ==========================================================================
    # Runner Section
    # ==========================================================================
    
    set(_runner_type "CONSOLE")
    set(_runner_externals "")
    set(_runner_external_options "")
    
    _json_has_key("${APP_JSON}" "runner" _has_runner)
    if(_has_runner)
        _json_get_object("${APP_JSON}" "runner" _runner_obj)
        
        # Type
        _json_get_string_or_default("${_runner_obj}" "type" "CONSOLE" _runner_type)
        string(TOUPPER "${_runner_type}" _runner_type)
        
        # Externals
        _json_array_length("${_runner_obj}" "externals" _ext_count)
        if(_ext_count GREATER 0)
            math(EXPR _ext_last "${_ext_count} - 1")
            foreach(_ext_idx RANGE 0 ${_ext_last})
                _json_array_get("${_runner_obj}" "externals" ${_ext_idx} _ext)
                list(APPEND _runner_externals "${_ext}")
            endforeach()
        endif()
        
        # External Options (JSON block for later processing)
        _json_get_object_or_empty("${_runner_obj}" "external_options" _runner_external_options)
    endif()
    
    ctx_set(${CTX} RUNNER_TYPE "${_runner_type}")
    ctx_set(${CTX} RUNNER_EXTERNALS "${_runner_externals}")
    ctx_set(${CTX} RUNNER_EXTERNAL_OPTIONS "${_runner_external_options}")
    
    # ==========================================================================
    # PCH Section
    # ==========================================================================
    
    set(_pch_enabled FALSE)
    set(_pch_header "pch.h")
    set(_pch_path "")
    
    _json_has_key("${APP_JSON}" "pch" _has_pch)
    if(_has_pch)
        _json_get_object("${APP_JSON}" "pch" _pch_obj)
        
        _json_has_key("${_pch_obj}" "enabled" _has_pch_enabled)
        if(_has_pch_enabled)
            _json_get_bool_from_key("${_pch_obj}" "enabled" _pch_enabled)
        endif()
        _json_get_string_or_default("${_pch_obj}" "header" "pch.h" _pch_header)
        _json_get_string_or_default("${_pch_obj}" "path" "" _pch_path)
    endif()
    
    ctx_set(${CTX} PCH_ENABLED "${_pch_enabled}")
    ctx_set(${CTX} PCH_HEADER "${_pch_header}")
    ctx_set(${CTX} PCH_PATH "${_pch_path}")
    
    # ==========================================================================
    # Tests Section (new targets[] structure)
    # ==========================================================================
    
    set(_tests_framework "")
    set(_tests_skip FALSE)
    set(_tests_targets_count 0)
    
    _json_has_key("${APP_JSON}" "tests" _has_tests)
    if(_has_tests)
        _json_get_object("${APP_JSON}" "tests" _tests_obj)
        
        # Global Framework (default for all test targets)
        _json_get_string_or_default("${_tests_obj}" "framework" "" _tests_framework)
        
        # Global Skip (skips ALL tests if true)
        _json_has_key("${_tests_obj}" "skip" _has_tests_skip)
        if(_has_tests_skip)
            _json_get_bool_from_key("${_tests_obj}" "skip" _tests_skip)
        endif()
        
        # Targets array
        _json_array_length("${_tests_obj}" "targets" _targets_count)
        
        if(_targets_count EQUAL 0)
            _json_has_key("${_tests_obj}" "targets" _has_targets_key)
            if(_has_targets_key)
                cmake_warn("W401" "App '${_name}': tests.targets is empty")
            endif()
        endif()
        
        if(_targets_count GREATER 0)
            math(EXPR _targets_last "${_targets_count} - 1")
            
            foreach(_t_idx RANGE 0 ${_targets_last})
                _json_array_get("${_tests_obj}" "targets" ${_t_idx} _target_json)
                
                # --- Required: name ---
                _json_has_key("${_target_json}" "name" _has_t_name)
                if(NOT _has_t_name)
                    cmake_fatal("E303" "App '${_name}': Test target [${_t_idx}] missing required 'name' field")
                endif()
                string(JSON _t_name GET "${_target_json}" "name")
                
                # --- Required: type ---
                _json_has_key("${_target_json}" "type" _has_t_type)
                if(NOT _has_t_type)
                    cmake_fatal("E304" "App '${_name}': Test target '${_t_name}' missing required 'type' field")
                endif()
                string(JSON _t_type GET "${_target_json}" "type")
                
                # Get defaults for this type
                _get_test_type_defaults("${_t_type}" _default_timeout _default_labels _default_parallel)
                
                # --- Optional: skip (default: false) ---
                set(_t_skip FALSE)
                _json_has_key("${_target_json}" "skip" _has_t_skip)
                if(_has_t_skip)
                    _json_get_bool_from_key("${_target_json}" "skip" _t_skip)
                endif()
                
                # --- Optional: path (default: tests/{type}/{name}) ---
                _json_get_string_or_default("${_target_json}" "path" "tests/${_t_type}/${_t_name}" _t_path)
                
                # --- Optional: framework (default: global) ---
                _json_get_string_or_default("${_target_json}" "framework" "" _t_framework)
                
                # --- Optional: timeout (default: type-based) ---
                set(_t_timeout "${_default_timeout}")
                _json_has_key("${_target_json}" "timeout" _has_t_timeout)
                if(_has_t_timeout)
                    _json_get_number("${_target_json}" "timeout" _t_timeout)
                endif()
                
                # --- Optional: labels (default: type-based) ---
                set(_t_labels "")
                _json_array_length("${_target_json}" "labels" _label_count)
                if(_label_count GREATER 0)
                    math(EXPR _label_last "${_label_count} - 1")
                    foreach(_l_idx RANGE 0 ${_label_last})
                        _json_array_get("${_target_json}" "labels" ${_l_idx} _label)
                        list(APPEND _t_labels "${_label}")
                    endforeach()
                else()
                    # Use default labels for type
                    set(_t_labels "${_default_labels}")
                endif()
                
                # --- Optional: externals ---
                set(_t_externals "")
                _json_array_length("${_target_json}" "externals" _ext_count)
                if(_ext_count GREATER 0)
                    math(EXPR _ext_last "${_ext_count} - 1")
                    foreach(_e_idx RANGE 0 ${_ext_last})
                        _json_array_get("${_target_json}" "externals" ${_e_idx} _ext)
                        list(APPEND _t_externals "${_ext}")
                    endforeach()
                endif()
                
                # --- Optional: parallel (default: type-based) ---
                set(_t_parallel "${_default_parallel}")
                _json_has_key("${_target_json}" "parallel" _has_t_parallel)
                if(_has_t_parallel)
                    _json_get_bool_from_key("${_target_json}" "parallel" _t_parallel)
                    
                    # Warn if parallel=true for types that should be serial
                    if(_t_parallel AND "${_t_type}" IN_LIST _TEST_TYPE_SERIAL_TYPES)
                        cmake_warn("W402" "App '${_name}': Test '${_t_name}' (type '${_t_type}') has parallel=true. This type typically runs serial for accurate results.")
                    endif()
                endif()
                
                # Store in context
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_NAME "${_t_name}")
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_TYPE "${_t_type}")
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_SKIP "${_t_skip}")
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_PATH "${_t_path}")
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_FRAMEWORK "${_t_framework}")
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_TIMEOUT "${_t_timeout}")
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_LABELS "${_t_labels}")
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_EXTERNALS "${_t_externals}")
                ctx_set(${CTX} TESTS_TARGET_${_t_idx}_PARALLEL "${_t_parallel}")
                
                dbg(${DBG_ULTRA_RARE} "    Test Target [${_t_idx}]: ${_t_name} (${_t_type}, skip=${_t_skip})" ID APPS)
                
            endforeach()
            
            set(_tests_targets_count ${_targets_count})
        endif()
    endif()
    
    ctx_set(${CTX} TESTS_FRAMEWORK "${_tests_framework}")
    ctx_set(${CTX} TESTS_SKIP "${_tests_skip}")
    ctx_set(${CTX} TESTS_TARGETS_COUNT "${_tests_targets_count}")
    
    # ==========================================================================
    # Platforms Filter
    # ==========================================================================
    
    set(_platforms "")
    
    _json_array_length("${APP_JSON}" "platforms" _platform_count)
    if(_platform_count GREATER 0)
        math(EXPR _platform_last "${_platform_count} - 1")
        foreach(_plat_idx RANGE 0 ${_platform_last})
            _json_array_get("${APP_JSON}" "platforms" ${_plat_idx} _plat)
            list(APPEND _platforms "${_plat}")
        endforeach()
    endif()
    
    ctx_set(${CTX} PLATFORMS "${_platforms}")
    
    # ==========================================================================
    # Debug Output
    # ==========================================================================
    
    dbg(${DBG_RARE} "  Collected App '${_name}':" ID APPS)
    dbg(${DBG_RARE} "    PATH: ${_path}" ID APPS)
    dbg(${DBG_RARE} "    SKIP: ${_skip}" ID APPS)
    dbg(${DBG_RARE} "    RUNNER_TYPE: ${_runner_type}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    CORE_DEPENDENCIES: ${_core_dependencies}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    CORE_EXTERNALS: ${_core_externals}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    CORE_EXTERNAL_OPTIONS: ${_core_external_options}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    RUNNER_EXTERNALS: ${_runner_externals}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    RUNNER_EXTERNAL_OPTIONS: ${_runner_external_options}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    PCH: ${_pch_enabled} (header=${_pch_header}, path=${_pch_path})" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    TESTS_FRAMEWORK: ${_tests_framework}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    TESTS_TARGETS_COUNT: ${_tests_targets_count}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    PLATFORMS: ${_platforms}" ID APPS)
    
endfunction()
