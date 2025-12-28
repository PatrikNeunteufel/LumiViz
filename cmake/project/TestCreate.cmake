# cmake/project/TestCreate.cmake
# ===============================
# Creates test targets from prepared Context
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
#   - cmake/core/Warnings.cmake
#   - cmake/core/CompilerOptions.cmake
#   - cmake/core/OutputDirs.cmake
#   - cmake/core/SourceCollect.cmake
#   - cmake/externals/Orchestrator.cmake
#
# Provides:
#   - _create_test_target(CTX)
#   - _check_platform_filter(PLATFORMS OUT_VAR)
#
# Used by:
#   - Tests.cmake

include_guard(GLOBAL)

# ==============================================================================
# _create_test_target - Main Creation Function
# ==============================================================================
#[[
    _create_test_target(CTX)
    
    Creates a CMake test target from the Context.
    
    Parameters:
        CTX - Context prefix (e.g., TEST_0, TEST_1)
    
    Steps:
        1. Read context data
        2. Validate source directory
        3. Create executable
        4. Link framework external
        5. Link dependencies
        6. Link other externals
        7. Apply compiler options
        8. Register with CTest
    
    Note on PCH:
        Tests do NOT use precompiled headers directly. However, when using
        source_from to share sources with an executable that uses PCH, the
        PCH directory is added to include paths so #include "pch.h" resolves.
        The PCH itself is not compiled for the test target.
]]
function(_create_test_target CTX)
    
    # ==========================================================================
    # Read Context Data
    # ==========================================================================
    
    ctx_get(${CTX} NAME _name)
    ctx_get(${CTX} DISPLAY_NAME _display_name)
    ctx_get(${CTX} PATH _path)
    ctx_get(${CTX} TYPE _type)
    ctx_get(${CTX} FRAMEWORK _framework)
    ctx_get(${CTX} DEPENDENCIES _dependencies)
    ctx_get(${CTX} EXTERNALS _externals)
    ctx_get(${CTX} EXTERNAL_OPTIONS _external_options)
    ctx_get(${CTX} TIMEOUT _timeout)
    ctx_get(${CTX} LABELS _labels)
    ctx_get(${CTX} PARALLEL _parallel)
    ctx_get(${CTX} SKIP _skip)
    ctx_get(${CTX} DEFINES _defines)
    ctx_get(${CTX} COMPILE_OPTIONS _compile_options)
    ctx_get(${CTX} SOURCE_FROM _source_from)
    ctx_get(${CTX} EXCLUDE_SOURCES _exclude_sources)
    
    # ==========================================================================
    # Check Skip
    # ==========================================================================
    
    if(_skip)
        dbg(${DBG_COMMON} "  SKIP: ${_name} (skip=true)" ID TESTS)
        return()
    endif()
    
    # ==========================================================================
    # Source Collection
    # ==========================================================================
    
    set(_sources "")
    set(_headers "")
    set(_extras "")
    set(_modules "")
    set(_include_dirs "")
    
    # Option A: source_from (Sources von Executable übernehmen)
    if(_source_from)
        # Find executable path from Solution
        get_property(_executables_json GLOBAL PROPERTY SOLUTION_EXECUTABLES_JSON)
        set(_exec_found FALSE)
        set(_exec_path "")
        
        string(JSON _exec_count LENGTH "${_executables_json}")
        if(_exec_count GREATER 0)
            math(EXPR _last_exec "${_exec_count} - 1")
            foreach(_i RANGE 0 ${_last_exec})
                string(JSON _exec_json GET "${_executables_json}" ${_i})
                string(JSON _exec_name GET "${_exec_json}" "name")
                if("${_exec_name}" STREQUAL "${_source_from}")
                    set(_exec_found TRUE)
                    _json_has_key("${_exec_json}" "path" _has_path)
                    if(_has_path)
                        string(JSON _exec_path GET "${_exec_json}" "path")
                    else()
                        set(_exec_path "projects/exec/${_source_from}/src")
                    endif()
                    break()
                endif()
            endforeach()
        endif()
        
        if(NOT _exec_found)
            cmake_fatal("E302" "source_from: Executable '${_source_from}' not found")
        endif()
        
        set(_exec_src_dir "${CMAKE_SOURCE_DIR}/${_exec_path}")
        
        # Collect sources from executable (via SourceCollect.cmake)
        collect_sources(
            ${_source_from}
            "${_exec_src_dir}"
            _exec_sources
            _exec_headers
            _exec_extras
            _exec_modules
            _exec_includes
        )
        
        # Filter excluded sources
        foreach(_src IN LISTS _exec_sources)
            set(_excluded FALSE)
            foreach(_pattern IN LISTS _exclude_sources)
                get_filename_component(_filename "${_src}" NAME)
                if("${_filename}" MATCHES "${_pattern}")
                    set(_excluded TRUE)
                    dbg(${DBG_RARE} "    Excluded: ${_filename}" ID TESTS)
                    break()
                endif()
            endforeach()
            if(NOT _excluded)
                list(APPEND _sources "${_src}")
            endif()
        endforeach()
        
        list(APPEND _headers ${_exec_headers})
        list(APPEND _extras ${_exec_extras})
        list(APPEND _modules ${_exec_modules})
        list(APPEND _include_dirs "${_exec_src_dir}")
        foreach(_inc IN LISTS _exec_includes)
            if(IS_ABSOLUTE "${_inc}")
                list(APPEND _include_dirs "${_inc}")
            else()
                list(APPEND _include_dirs "${_exec_src_dir}/${_inc}")
            endif()
        endforeach()
        
        # ======================================================================
        # PCH Directory from source_from executable
        # ======================================================================
        # If the executable has a pch/ directory, add it to include paths
        # so that #include "pch.h" in shared sources can be resolved.
        # Note: The test does NOT compile or use the PCH itself.
        # ======================================================================
        if(EXISTS "${_exec_src_dir}/pch")
            list(APPEND _include_dirs "${_exec_src_dir}/pch")
            dbg(${DBG_RARE} "    PCH include dir from source_from: ${_exec_src_dir}/pch" ID TESTS)
        endif()
        
        dbg(${DBG_RARE} "    Sources from ${_source_from}: ${_exec_path}" ID TESTS)
    endif()
    
    # Option B/Additional: Test's own sources
    set(_test_src_dir "${CMAKE_SOURCE_DIR}/${_path}")
    
    if(EXISTS "${_test_src_dir}")
        # Collect test sources (via SourceCollect.cmake)
        collect_sources(
            ${_name}
            "${_test_src_dir}"
            _test_sources
            _test_headers
            _test_extras
            _test_modules
            _test_includes
        )
        
        list(APPEND _sources ${_test_sources})
        list(APPEND _headers ${_test_headers})
        list(APPEND _extras ${_test_extras})
        list(APPEND _modules ${_test_modules})
        list(APPEND _include_dirs "${_test_src_dir}")
        foreach(_inc IN LISTS _test_includes)
            if(IS_ABSOLUTE "${_inc}")
                list(APPEND _include_dirs "${_inc}")
            else()
                list(APPEND _include_dirs "${_test_src_dir}/${_inc}")
            endif()
        endforeach()
        
        # PCH directory for test's own sources (if exists)
        if(EXISTS "${_test_src_dir}/pch")
            list(APPEND _include_dirs "${_test_src_dir}/pch")
            dbg(${DBG_RARE} "    PCH include dir: ${_test_src_dir}/pch" ID TESTS)
        endif()
    elseif(NOT _source_from)
        cmake_fatal("E303" "Test '${_name}': Source path does not exist: ${_path}")
    endif()
    
    if(NOT _sources)
        cmake_warn("W101" "Test '${_name}': No source files found")
    endif()
    
    # ==========================================================================
    # Create Executable
    # ==========================================================================
    
    # Create test executable
    add_executable(${_name} ${_sources} ${_headers} ${_extras} ${_modules})
    
    # Include directories
    foreach(_inc IN LISTS _include_dirs)
        target_include_directories(${_name} PRIVATE "${_inc}")
    endforeach()
    
    dbg(${DBG_RARE} "    add_executable(${_name}) with ${CMAKE_CXX_COMPILER_ID}" ID TESTS)
    
    # ==========================================================================
    # Framework Integration
    # ==========================================================================
    
    # Validate framework
    set(_valid_frameworks "doctest" "googletest" "catch2")
    if(NOT "${_framework}" IN_LIST _valid_frameworks)
        cmake_fatal("E301" "Test '${_name}': Unknown framework '${_framework}'. Valid: ${_valid_frameworks}")
    endif()
    
    # Ensure framework is in externals list
    if(NOT "${_framework}" IN_LIST _externals)
        list(PREPEND _externals "${_framework}")
        dbg(${DBG_RARE} "    Auto-added framework external: ${_framework}" ID TESTS)
    endif()
    
    # ==========================================================================
    # Internal Dependencies (Libraries)
    # ==========================================================================
    
    foreach(_dep IN LISTS _dependencies)
        if(TARGET ${_dep})
            target_link_libraries(${_name} PRIVATE ${_dep})
            dbg(${DBG_RARE} "    Link: ${_dep} (internal)" ID TESTS)
        else()
            cmake_fatal("E101" "Dependency '${_dep}' for test '${_name}' does not exist")
        endif()
    endforeach()
    
    # ==========================================================================
    # External Dependencies
    # ==========================================================================
    
    foreach(_ext IN LISTS _externals)
        # Check if external is defined
        get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)
        _json_has_key("${_externals_json}" "${_ext}" _ext_defined)
        
        if(NOT _ext_defined)
            cmake_fatal("E010" "External '${_ext}' not defined in externals block")
        endif()
        
        # Get options for this external
        _json_has_key("${_external_options}" "${_ext}" _has_ext_opts)
        if(_has_ext_opts)
            _json_get_object("${_external_options}" "${_ext}" _ext_opts)
        else()
            set(_ext_opts "{}")
        endif()
        
        # Apply external
        apply_external_to_target("${_name}" "${_ext}" "${_ext_opts}")
        
        dbg(${DBG_RARE} "    External: ${_ext} applied" ID TESTS)
    endforeach()
    
    # ==========================================================================
    # Preprocessor Definitions
    # ==========================================================================
    
    if(_defines)
        target_compile_definitions(${_name} PRIVATE ${_defines})
    endif()
    
    # Test-type specific definitions
    string(TOUPPER "${_type}" _type_upper)
    target_compile_definitions(${_name} PRIVATE "TEST_TYPE_${_type_upper}")
    
    # ==========================================================================
    # Compiler Options
    # ==========================================================================
    
    if(_compile_options)
        target_compile_options(${_name} PRIVATE ${_compile_options})
    endif()
    
    # Apply standard modules
    apply_warnings(${_name})
    apply_compiler_options(${_name})
    setup_output_dirs(${_name})
    
    # ==========================================================================
    # CTest Registration
    # ==========================================================================
    
    # Add test
    add_test(
        NAME ${_name}
        COMMAND ${_name}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # Set timeout
    set_tests_properties(${_name} PROPERTIES
        TIMEOUT ${_timeout}
    )
    
    # Set labels
    if(_labels)
        set_tests_properties(${_name} PROPERTIES
            LABELS "${_labels}"
        )
    endif()
    
    # Serial execution (not parallel)
    if(NOT _parallel)
        set_tests_properties(${_name} PROPERTIES
            RUN_SERIAL TRUE
        )
    endif()
    
    # Display name for IDE
    set_target_properties(${_name} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
    
    if(NOT "${_display_name}" STREQUAL "${_name}")
        set_target_properties(${_name} PROPERTIES
            PROJECT_LABEL "${_display_name}"
        )
    endif()
    
    dbg(${DBG_RARE} "    CTest registered: ${_name}" ID TESTS)
    dbg(${DBG_RARE} "      Labels: ${_labels}" ID TESTS)
    dbg(${DBG_RARE} "      Timeout: ${_timeout}s" ID TESTS)
    
endfunction()

# ==============================================================================
# Helper: _check_platform_filter
# ==============================================================================
#[[
    _check_platform_filter(PLATFORMS OUT_VAR)
    
    Checks if current platform matches the filter list.
    
    Parameters:
        PLATFORMS - List of allowed platforms (windows, linux, macos)
        OUT_VAR   - Output variable (TRUE if matches or empty)
]]
function(_check_platform_filter PLATFORMS OUT_VAR)
    # Empty list = all platforms
    if(NOT PLATFORMS)
        set(${OUT_VAR} TRUE PARENT_SCOPE)
        return()
    endif()
    
    # Determine current platform
    if(WIN32)
        set(_current "windows")
    elseif(APPLE)
        set(_current "macos")
    else()
        set(_current "linux")
    endif()
    
    # Check if current platform is in list
    string(TOLOWER "${PLATFORMS}" _platforms_lower)
    if("${_current}" IN_LIST _platforms_lower)
        set(${OUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()
