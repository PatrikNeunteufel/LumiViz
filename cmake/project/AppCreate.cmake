# cmake/project/AppCreate.cmake
# ==============================
# Creates App-Container targets from prepared Context
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - cmake/core/Context.cmake
#   - cmake/core/Errors.cmake
#   - cmake/core/Debug.cmake
#   - cmake/core/OutputDirs.cmake
#   - cmake/core/Warnings.cmake
#   - cmake/core/CompilerOptions.cmake
#   - cmake/externals/Orchestrator.cmake
#
# Provides:
#   - _create_app_core(CTX)    - Creates {AppName}.Core STATIC library
#   - _create_app_runner(CTX)  - Creates {AppName} executable
#   - _create_app_tests(CTX)   - Creates {AppName}.*Tests executables
#
# Used by:
#   - Apps.cmake

include_guard(GLOBAL)

# ==============================================================================
# _create_app_core - Creates the Core STATIC library
# ==============================================================================
#[[
    _create_app_core(CTX)
    
    Creates the {AppName}.Core STATIC library containing all business logic.
    This library is linked by both the Runner and the Tests.
    
    Parameters:
        CTX - Mandatory: Context prefix (e.g. APP_0, APP_1)
    
    Expected Context Keys:
        NAME, PATH, VERSION, PCH_ENABLED, PCH_HEADER, PCH_SOURCE,
        CORE_DEPENDENCIES, CORE_EXTERNALS
    
    Directory Structure Expected:
        {PATH}/
        ├── include/    - PUBLIC headers
        ├── src/        - Implementation files
        └── pch/        - Precompiled header (optional)
    
    Generated Target:
        {AppName}.Core - STATIC library
    
    Example:
        ctx_create(APP_0)
        _collect_app("${_app_json}" APP_0)
        _create_app_core(APP_0)
        # Creates: AudioPlayer.Core
]]
function(_create_app_core CTX)
    
    # --------------------------------------------------------------------------
    # Read Context Data
    # --------------------------------------------------------------------------
    
    ctx_get(${CTX} NAME _name)
    ctx_get(${CTX} PATH _path)
    ctx_get(${CTX} VERSION _version)
    ctx_get(${CTX} PCH_ENABLED _pch_enabled)
    ctx_get(${CTX} PCH_HEADER _pch_header)
    ctx_get(${CTX} PCH_PATH _pch_custom_path)
    ctx_get(${CTX} CORE_DEPENDENCIES _dependencies)
    ctx_get(${CTX} CORE_EXTERNALS _externals)
    
    set(_target_name "${_name}.Core")
    set(_base_dir "${CMAKE_SOURCE_DIR}/${_path}")
    set(_src_dir "${_base_dir}/src")
    set(_include_dir "${_base_dir}/include")
    
    # --------------------------------------------------------------------------
    # Validate Directories
    # --------------------------------------------------------------------------
    
    # Base path must exist
    if(NOT EXISTS "${_base_dir}")
        cmake_fatal("E402" "App '${_name}': Path does not exist: ${_path}")
    endif()
    
    # src/ directory must exist
    if(NOT EXISTS "${_src_dir}")
        cmake_fatal("E403" "App '${_name}': No src/ directory in ${_path}")
    endif()
    
    # include/ directory is optional but recommended
    if(NOT EXISTS "${_include_dir}")
        cmake_warn("W401" "App '${_name}': No include/ directory (headers will be private)")
        set(_has_include_dir FALSE)
    else()
        set(_has_include_dir TRUE)
    endif()
    
    # --------------------------------------------------------------------------
    # Collect Sources (via SourceCollect.cmake)
    # --------------------------------------------------------------------------
    
    # Mode is determined by SOLUTION_SOURCE_MODE (explicit/glob/auto)
    collect_sources(
        ${_target_name}
        "${_src_dir}"
        _sources
        _private_headers
        _extras
        _modules
        _src_includes
    )
    
    if(NOT _sources)
        cmake_fatal("E404" "App '${_name}': No source files found in src/")
    endif()
    
    # Collect public headers if include/ exists (always GLOB for header-only dir)
    set(_public_headers "")
    if(_has_include_dir)
        file(GLOB_RECURSE _public_headers
            "${_include_dir}/*.h"
            "${_include_dir}/*.hpp"
            "${_include_dir}/*.hxx"
        )
    endif()
    
    dbg(${DBG_RARE} "    Core sources: ${_sources}" ID APPS)
    dbg(${DBG_ULTRA_RARE} "    Public headers: ${_public_headers}" ID APPS)
    
    # --------------------------------------------------------------------------
    # Create STATIC Library
    # --------------------------------------------------------------------------
    
    add_library(${_target_name} STATIC)
    
    target_sources(${_target_name}
        PRIVATE
            ${_sources}
            ${_private_headers}
            ${_extras}
            ${_modules}
    )
    
    if(_public_headers)
        target_sources(${_target_name}
            PUBLIC
                FILE_SET HEADERS
                BASE_DIRS "${_include_dir}"
                FILES ${_public_headers}
        )
    endif()
    
    # --------------------------------------------------------------------------
    # Include Directories
    # --------------------------------------------------------------------------
    
    # Private: src/ for implementation details (NOT needed for Apps - no external consumers)
    # Only if there are internal headers in src/
    if(_private_headers)
        target_include_directories(${_target_name} PRIVATE "${_src_dir}")
    endif()
    
    # Additional includes from Source.cmake
    foreach(_inc IN LISTS _src_includes)
        if(IS_ABSOLUTE "${_inc}")
            target_include_directories(${_target_name} PRIVATE "${_inc}")
        else()
            target_include_directories(${_target_name} PRIVATE "${_src_dir}/${_inc}")
        endif()
    endforeach()
    
    # Public: include/ for consumers (Runner, Tests)
    if(_has_include_dir)
        target_include_directories(${_target_name} PUBLIC "${_include_dir}")
    endif()
    
    # --------------------------------------------------------------------------
    # Precompiled Headers
    # --------------------------------------------------------------------------
    
    set(_pch_found_path "")
    set(_pch_dir "")
    
    if(_pch_enabled)
        # If custom path specified, use it (relative to CMAKE_SOURCE_DIR/projects/)
        if(NOT "${_pch_custom_path}" STREQUAL "")
            set(_custom_full_path "${CMAKE_SOURCE_DIR}/projects/${_pch_custom_path}/${_pch_header}")
            if(EXISTS "${_custom_full_path}")
                set(_pch_found_path "${_custom_full_path}")
                get_filename_component(_pch_dir "${_custom_full_path}" DIRECTORY)
            endif()
        else()
            # Search priority: 1. pch/, 2. src/, 3. root
            if(EXISTS "${_base_dir}/pch/${_pch_header}")
                set(_pch_found_path "${_base_dir}/pch/${_pch_header}")
                set(_pch_dir "${_base_dir}/pch")
            elseif(EXISTS "${_src_dir}/${_pch_header}")
                set(_pch_found_path "${_src_dir}/${_pch_header}")
                set(_pch_dir "${_src_dir}")
            elseif(EXISTS "${_base_dir}/${_pch_header}")
                set(_pch_found_path "${_base_dir}/${_pch_header}")
                set(_pch_dir "${_base_dir}")
            endif()
        endif()
        
        if(_pch_found_path)
            # Add PCH directory as PUBLIC include (for Runner and Tests to find pch.h)
            target_include_directories(${_target_name} PUBLIC "${_pch_dir}")
            
            # Enable precompiled headers
            target_precompile_headers(${_target_name} PRIVATE "${_pch_found_path}")
            
            dbg(${DBG_RARE} "    PCH: ${_pch_found_path}" ID APPS)
            dbg(${DBG_RARE} "    PCH include dir: ${_pch_dir}" ID APPS)
        else()
            cmake_warn("W402" "App '${_name}': PCH enabled but '${_pch_header}' not found in pch/, src/, or root")
        endif()
    endif()
    
    # Store PCH info in target properties for Runner to access
    set_target_properties(${_target_name} PROPERTIES
        APP_PCH_ENABLED "${_pch_enabled}"
        APP_PCH_PATH "${_pch_found_path}"
    )
    
    # --------------------------------------------------------------------------
    # Internal Dependencies (Libraries)
    # --------------------------------------------------------------------------
    
    foreach(_dep IN LISTS _dependencies)
        if(TARGET ${_dep})
            target_link_libraries(${_target_name} PUBLIC ${_dep})
            dbg(${DBG_RARE} "    Link: ${_dep} (internal, PUBLIC)" ID APPS)
        else()
            cmake_fatal("E405" "App '${_name}': Dependency '${_dep}' not found")
        endif()
    endforeach()
    
    # --------------------------------------------------------------------------
    # External Dependencies (via Orchestrator)
    # --------------------------------------------------------------------------
    
    foreach(_ext IN LISTS _externals)
        # Check if external is defined in central block
        get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)
        _json_has_key("${_externals_json}" "${_ext}" _ext_defined)
        
        if(NOT _ext_defined)
            cmake_fatal("E010" "External '${_ext}' not defined in externals block")
        endif()
        
        # Get external_options if present
        ctx_get(${CTX} CORE_EXTERNAL_OPTIONS _ext_options)
        
        # Check if this external has specific options
        set(_options_json "{}")
        if(NOT "${_ext_options}" STREQUAL "")
            _json_has_key("${_ext_options}" "${_ext}" _has_options)
            if(_has_options)
                _json_get_object("${_ext_options}" "${_ext}" _options_json)
            endif()
        endif()
        
        # Apply external via Orchestrator
        apply_external_to_target("${_target_name}" "${_ext}" "${_options_json}")
        
        dbg(${DBG_RARE} "    External: ${_ext} applied (Core, PUBLIC)" ID APPS)
    endforeach()
    
    # --------------------------------------------------------------------------
    # Apply Standard Modules
    # --------------------------------------------------------------------------
    
    # Compiler warnings (from Warnings.cmake)
    apply_warnings(${_target_name})
    
    # Compiler options (from CompilerOptions.cmake)
    apply_compiler_options(${_target_name})
    
    # Output directories (from OutputDirs.cmake)
    setup_output_dirs(${_target_name})
    
    # --------------------------------------------------------------------------
    # Version as Target Property
    # --------------------------------------------------------------------------
    
    if(NOT "${_version}" STREQUAL "")
        set_target_properties(${_target_name} PROPERTIES
            VERSION "${_version}"
        )
    endif()
    
    # --------------------------------------------------------------------------
    # IDE Organization
    # --------------------------------------------------------------------------
    
    set_target_properties(${_target_name} PROPERTIES
        FOLDER "Apps/${_name}"
    )
    
endfunction()

# ==============================================================================
# _create_app_runner - Creates the Runner executable
# ==============================================================================
#[[
    _create_app_runner(CTX)
    
    Creates the {AppName} executable (the runner with main()).
    Links against {AppName}.Core and applies runner-specific externals.
    
    Parameters:
        CTX - Mandatory: Context prefix (e.g. APP_0, APP_1)
    
    Expected Context Keys:
        NAME, PATH, VERSION, RUNNER_TYPE, RUNNER_EXTERNALS, RUNNER_EXTERNAL_OPTIONS
    
    Directory Structure Expected:
        {PATH}/
        └── main/    - Entry point (main.cpp)
    
    Generated Target:
        {AppName} - Executable (CONSOLE or GUI)
]]
function(_create_app_runner CTX)
    
    # --------------------------------------------------------------------------
    # Read Context Data
    # --------------------------------------------------------------------------
    
    ctx_get(${CTX} NAME _name)
    ctx_get(${CTX} PATH _path)
    ctx_get(${CTX} VERSION _version)
    ctx_get(${CTX} DISPLAY_NAME _display_name)
    ctx_get(${CTX} RUNNER_TYPE _runner_type)
    ctx_get(${CTX} RUNNER_EXTERNALS _externals)
    ctx_get(${CTX} RUNNER_EXTERNAL_OPTIONS _external_options)
    
    set(_target_name "${_name}")
    set(_core_target "${_name}.Core")
    set(_base_dir "${CMAKE_SOURCE_DIR}/${_path}")
    set(_main_dir "${_base_dir}/main")
    
    # --------------------------------------------------------------------------
    # Validate main/ Directory
    # --------------------------------------------------------------------------
    
    if(NOT EXISTS "${_main_dir}")
        cmake_fatal("E406" "App '${_name}': No main/ directory in ${_path}")
    endif()
    
    # --------------------------------------------------------------------------
    # Collect Sources from main/ (via SourceCollect.cmake)
    # --------------------------------------------------------------------------
    
    # Mode is determined by SOLUTION_SOURCE_MODE (explicit/glob/auto)
    collect_sources(
        ${_target_name}
        "${_main_dir}"
        _sources
        _headers
        _extras
        _modules
        _main_includes
    )
    
    if(NOT _sources)
        cmake_fatal("E407" "App '${_name}': No source files found in main/")
    endif()
    
    dbg(${DBG_RARE} "    Runner sources: ${_sources}" ID APPS)
    
    # --------------------------------------------------------------------------
    # Create Executable (GUI vs. CONSOLE)
    # --------------------------------------------------------------------------
    
    if(_runner_type STREQUAL "WINDOW" OR _runner_type STREQUAL "GUI")
        if(WIN32)
            add_executable(${_target_name} WIN32 ${_sources} ${_headers} ${_extras} ${_modules})
            # Define APP_WINDOWS_GUI for conditional compilation
            target_compile_definitions(${_target_name} PRIVATE APP_WINDOWS_GUI)
        elseif(APPLE)
            add_executable(${_target_name} MACOSX_BUNDLE ${_sources} ${_headers} ${_extras} ${_modules})
        else()
            add_executable(${_target_name} ${_sources} ${_headers} ${_extras} ${_modules})
        endif()
    else()
        # CONSOLE or other
        add_executable(${_target_name} ${_sources} ${_headers} ${_extras} ${_modules})
    endif()
    
    dbg(${DBG_RARE} "    add_executable(${_target_name}) [${_runner_type}]" ID APPS)
    
    # --------------------------------------------------------------------------
    # Include Directory for main/
    # --------------------------------------------------------------------------
    
    target_include_directories(${_target_name} PRIVATE "${_main_dir}")
    
    # Additional includes from Source.cmake
    foreach(_inc IN LISTS _main_includes)
        if(IS_ABSOLUTE "${_inc}")
            target_include_directories(${_target_name} PRIVATE "${_inc}")
        else()
            target_include_directories(${_target_name} PRIVATE "${_main_dir}/${_inc}")
        endif()
    endforeach()
    
    # --------------------------------------------------------------------------
    # Link Against Core Library
    # --------------------------------------------------------------------------
    
    # This also brings in:
    # - PUBLIC include directories (include/, pch/)
    # - PUBLIC dependencies and externals from Core
    #
    # NOTE: Runner does NOT use PCH - main.cpp is typically small and
    # gains minimal benefit from precompiled headers. This simplifies
    # the template (no #include "pch.h" required in main.cpp).
    target_link_libraries(${_target_name} PRIVATE ${_core_target})
    dbg(${DBG_RARE} "    Link: ${_core_target} (Core Library)" ID APPS)
    
    # --------------------------------------------------------------------------
    # Runner-Specific External Dependencies
    # --------------------------------------------------------------------------
    
    foreach(_ext IN LISTS _externals)
        # Check if external is defined in central block
        get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)
        _json_has_key("${_externals_json}" "${_ext}" _ext_defined)
        
        if(NOT _ext_defined)
            cmake_fatal("E010" "External '${_ext}' not defined in externals block")
        endif()
        
        # Get external_options if present
        set(_options_json "{}")
        if(NOT "${_external_options}" STREQUAL "")
            _json_has_key("${_external_options}" "${_ext}" _has_options)
            if(_has_options)
                _json_get_object("${_external_options}" "${_ext}" _options_json)
            endif()
        endif()
        
        # Apply external via Orchestrator
        apply_external_to_target("${_target_name}" "${_ext}" "${_options_json}")
        
        dbg(${DBG_RARE} "    External: ${_ext} applied (Runner)" ID APPS)
    endforeach()
    
    # --------------------------------------------------------------------------
    # Apply Standard Modules
    # --------------------------------------------------------------------------
    
    apply_warnings(${_target_name})
    apply_compiler_options(${_target_name})
    setup_output_dirs(${_target_name})
    
    # --------------------------------------------------------------------------
    # Version and Display Name
    # --------------------------------------------------------------------------
    
    if(NOT "${_version}" STREQUAL "")
        set_target_properties(${_target_name} PROPERTIES
            VERSION "${_version}"
        )
    endif()
    
    if(NOT "${_display_name}" STREQUAL "")
        set_target_properties(${_target_name} PROPERTIES
            OUTPUT_NAME "${_display_name}"
        )
    endif()
    
    # --------------------------------------------------------------------------
    # Platform-Specific Properties
    # --------------------------------------------------------------------------
    
    if(APPLE AND (_runner_type STREQUAL "WINDOW" OR _runner_type STREQUAL "GUI"))
        set_target_properties(${_target_name} PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "${_display_name}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${_version}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${_version}"
        )
    endif()
    
    # --------------------------------------------------------------------------
    # IDE Organization
    # --------------------------------------------------------------------------
    
    set_target_properties(${_target_name} PROPERTIES
        FOLDER "Apps/${_name}"
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
endfunction()

# ==============================================================================
# _create_app_tests - Creates Test executables
# ==============================================================================
#[[
    _create_app_tests(CTX)
    
    Creates test executables based on the tests.targets[] configuration.
    Supports arbitrary test types with configurable frameworks.
    
    Framework Resolution:
        1. Test-specific framework (targets[].framework)
        2. Global framework (tests.framework)
        3. ERROR if neither is specified
    
    Parameters:
        CTX - Mandatory: Context prefix (e.g. APP_0, APP_1)
    
    Expected Context Keys:
        NAME, PATH, TESTS_FRAMEWORK (global default), TESTS_TARGETS_COUNT
        TESTS_TARGET_{n}_NAME, TESTS_TARGET_{n}_TYPE, TESTS_TARGET_{n}_PATH,
        TESTS_TARGET_{n}_FRAMEWORK, TESTS_TARGET_{n}_TIMEOUT, TESTS_TARGET_{n}_LABELS,
        TESTS_TARGET_{n}_EXTERNALS, TESTS_TARGET_{n}_PARALLEL
    
    Directory Structure Expected:
        {PATH}/
        └── tests/
            └── {type}/
                └── {name}/
                    ├── Source.cmake
                    ├── test_main.cpp
                    └── test_*.cpp
    
    Generated Targets:
        {AppName}.{TestName} for each target in tests.targets[]
]]
function(_create_app_tests CTX)
    
    # --------------------------------------------------------------------------
    # Read Context Data
    # --------------------------------------------------------------------------
    
    ctx_get(${CTX} NAME _name)
    ctx_get(${CTX} PATH _path)
    ctx_get(${CTX} TESTS_FRAMEWORK _global_framework)
    ctx_get(${CTX} TESTS_SKIP _global_skip)
    ctx_get(${CTX} TESTS_TARGETS_COUNT _targets_count)
    
    set(_core_target "${_name}.Core")
    set(_base_dir "${CMAKE_SOURCE_DIR}/${_path}")
    set(_tests_dir "${_base_dir}/tests")
    
    # --------------------------------------------------------------------------
    # Check global skip
    # --------------------------------------------------------------------------
    
    if(_global_skip)
        dbg(${DBG_COMMON} "  SKIP: All tests for ${_name} (tests.skip=true)" ID APPS)
        return()
    endif()
    
    # --------------------------------------------------------------------------
    # Check if any tests configured
    # --------------------------------------------------------------------------
    
    if(_targets_count EQUAL 0)
        dbg(${DBG_RARE} "    No test targets configured for ${_name}" ID APPS)
        return()
    endif()
    
    # --------------------------------------------------------------------------
    # Check if tests directory exists
    # --------------------------------------------------------------------------
    
    if(NOT EXISTS "${_tests_dir}")
        cmake_warn("W403" "App '${_name}': tests.targets configured but no tests/ directory")
        return()
    endif()
    
    # --------------------------------------------------------------------------
    # Valid Frameworks List
    # --------------------------------------------------------------------------
    
    set(_valid_frameworks "doctest" "googletest" "catch2")
    
    # --------------------------------------------------------------------------
    # Iterate over all test targets
    # --------------------------------------------------------------------------
    
    math(EXPR _targets_last "${_targets_count} - 1")
    
    foreach(_t_idx RANGE 0 ${_targets_last})
        
        # Read target configuration from context
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_NAME _t_name)
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_TYPE _t_type)
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_SKIP _t_skip)
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_PATH _t_path)
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_FRAMEWORK _t_framework)
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_TIMEOUT _t_timeout)
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_LABELS _t_labels)
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_EXTERNALS _t_externals)
        ctx_get(${CTX} TESTS_TARGET_${_t_idx}_PARALLEL _t_parallel)
        
        # Check individual skip
        if(_t_skip)
            dbg(${DBG_COMMON} "  SKIP: ${_name}.${_t_name} (skip=true)" ID APPS)
            continue()
        endif()
        
        # Full target name: {AppName}.{TestName}
        set(_target_name "${_name}.${_t_name}")
        
        # Full path to test sources
        set(_test_src_dir "${_base_dir}/${_t_path}")
        
        # Resolve framework: test-specific > global > error
        set(_effective_framework "${_t_framework}")
        if("${_effective_framework}" STREQUAL "")
            set(_effective_framework "${_global_framework}")
        endif()
        if("${_effective_framework}" STREQUAL "")
            cmake_fatal("E301" "App '${_name}': No framework specified for test '${_t_name}'. Set tests.framework or targets[].framework")
        endif()
        if(NOT "${_effective_framework}" IN_LIST _valid_frameworks)
            cmake_fatal("E302" "App '${_name}': Unknown framework '${_effective_framework}' for test '${_t_name}'. Valid: ${_valid_frameworks}")
        endif()
        
        # Check if path exists
        if(NOT EXISTS "${_test_src_dir}")
            cmake_fatal("E305" "App '${_name}': Test '${_t_name}' path does not exist: ${_t_path}")
        endif()
        
        # Create the test target
        _create_app_test_target(
            "${_target_name}"
            "${_test_src_dir}"
            "${_core_target}"
            "${_effective_framework}"
            "${_t_timeout}"
            "${_t_labels}"
            "${_t_externals}"
            "${_name}"
            "${_t_parallel}"
        )
        
        dbg(${DBG_COMMON} "  Created: ${_target_name} (${_t_type}, ${_effective_framework})" ID APPS)
        
    endforeach()
    
endfunction()

# ==============================================================================
# _create_app_test_target - Helper to create a single test target
# ==============================================================================
#[[
    _create_app_test_target(TARGET_NAME SRC_DIR CORE_TARGET FRAMEWORK TIMEOUT LABELS EXTRA_EXTERNALS APP_NAME PARALLEL)
    
    Internal helper function to create a test executable.
    
    Parameters:
        TARGET_NAME     - Name for the test target
        SRC_DIR         - Directory containing test sources
        CORE_TARGET     - Core library to link against
        FRAMEWORK       - Test framework (doctest, googletest, catch2)
        TIMEOUT         - CTest timeout in seconds
        LABELS          - CTest labels (semicolon-separated)
        EXTRA_EXTERNALS - Additional externals for this test
        APP_NAME        - Parent app name (for folder organization)
        PARALLEL        - TRUE to allow parallel execution, FALSE for serial
]]
function(_create_app_test_target TARGET_NAME SRC_DIR CORE_TARGET FRAMEWORK TIMEOUT LABELS EXTRA_EXTERNALS APP_NAME PARALLEL)
    
    # --------------------------------------------------------------------------
    # Collect Sources (via SourceCollect.cmake)
    # --------------------------------------------------------------------------
    
    # Mode is determined by SOLUTION_SOURCE_MODE (explicit/glob/auto)
    collect_sources(
        ${TARGET_NAME}
        "${SRC_DIR}"
        _sources
        _headers
        _extras
        _modules
        _test_includes
    )
    
    if(NOT _sources)
        cmake_warn("W403" "Test '${TARGET_NAME}': No source files found")
        return()
    endif()
    
    # --------------------------------------------------------------------------
    # Create Test Executable
    # --------------------------------------------------------------------------
    
    add_executable(${TARGET_NAME} ${_sources} ${_headers} ${_extras} ${_modules})
    
    target_include_directories(${TARGET_NAME} PRIVATE "${SRC_DIR}")
    
    # Additional includes from Source.cmake
    foreach(_inc IN LISTS _test_includes)
        if(IS_ABSOLUTE "${_inc}")
            target_include_directories(${TARGET_NAME} PRIVATE "${_inc}")
        else()
            target_include_directories(${TARGET_NAME} PRIVATE "${SRC_DIR}/${_inc}")
        endif()
    endforeach()
    
    # --------------------------------------------------------------------------
    # Link Against Core Library
    # --------------------------------------------------------------------------
    
    # This also brings in:
    # - PUBLIC include directories (include/, pch/)
    # - PUBLIC dependencies and externals from Core
    #
    # NOTE: Tests do NOT use PCH - test files typically include the test
    # framework header which dominates compilation time anyway.
    # This simplifies templates (no #include "pch.h" required in tests).
    target_link_libraries(${TARGET_NAME} PRIVATE ${CORE_TARGET})
    
    # --------------------------------------------------------------------------
    # Link Test Framework
    # --------------------------------------------------------------------------
    
    # Check if framework external is defined
    get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)
    _json_has_key("${_externals_json}" "${FRAMEWORK}" _framework_defined)
    
    if(NOT _framework_defined)
        cmake_fatal("E010" "Test framework '${FRAMEWORK}' not defined in externals block")
    endif()
    
    apply_external_to_target("${TARGET_NAME}" "${FRAMEWORK}" "{}")
    
    # --------------------------------------------------------------------------
    # Additional Externals (for integration tests)
    # --------------------------------------------------------------------------
    
    foreach(_ext IN LISTS EXTRA_EXTERNALS)
        _json_has_key("${_externals_json}" "${_ext}" _ext_defined)
        
        if(NOT _ext_defined)
            cmake_fatal("E010" "External '${_ext}' not defined in externals block")
        endif()
        
        apply_external_to_target("${TARGET_NAME}" "${_ext}" "{}")
    endforeach()
    
    # --------------------------------------------------------------------------
    # Apply Standard Modules
    # --------------------------------------------------------------------------
    
    apply_warnings(${TARGET_NAME})
    apply_compiler_options(${TARGET_NAME})
    setup_output_dirs(${TARGET_NAME})
    
    # --------------------------------------------------------------------------
    # CTest Registration
    # --------------------------------------------------------------------------
    
    add_test(
        NAME ${TARGET_NAME}
        COMMAND ${TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # Timeout
    set_tests_properties(${TARGET_NAME} PROPERTIES
        TIMEOUT ${TIMEOUT}
    )
    
    # Labels (add app name as label automatically)
    set(_all_labels "${APP_NAME}")
    if(LABELS)
        list(APPEND _all_labels ${LABELS})
    endif()
    set_tests_properties(${TARGET_NAME} PROPERTIES
        LABELS "${_all_labels}"
    )
    
    # Parallel execution (RUN_SERIAL=TRUE when parallel=FALSE)
    if(NOT PARALLEL)
        set_tests_properties(${TARGET_NAME} PROPERTIES
            RUN_SERIAL TRUE
        )
    endif()
    
    # --------------------------------------------------------------------------
    # IDE Organization
    # --------------------------------------------------------------------------
    
    set_target_properties(${TARGET_NAME} PROPERTIES
        FOLDER "Apps/${APP_NAME}/Tests"
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
endfunction()
