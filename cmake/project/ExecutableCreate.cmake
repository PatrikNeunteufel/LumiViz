# cmake/project/ExecutableCreate.cmake
# =====================================
# Creates executable targets from prepared Context
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
#   - cmake/core/SourceCollect.cmake
#   - cmake/externals/Orchestrator.cmake
#
# Provides:
#   - _create_executable_target(CTX)
#
# Used by:
#   - Executables.cmake

include_guard(GLOBAL)

# ============================================================================
# _create_executable_target - Creates CMake executable from Context
# ============================================================================
#[[
    _create_executable_target(CTX)
    
    Creates the CMake executable target from a prepared Context.
    Handles GUI/CONSOLE distinction, sources, dependencies, externals,
    and applies standard modules (Warnings, CompilerOptions, OutputDirs).
    
    Parameters:
        CTX - Mandatory: Context prefix (e.g. EXE_0, EXE_1)
    
    Expected Context Keys:
        NAME, PATH, TYPE, VERSION, PCH_ENABLED, PCH_HEADER, PCH_PATH,
        DEPENDENCIES, EXTERNALS, EXTERNAL_OPTIONS,
        DEFINES, COMPILE_OPTIONS, LINK_OPTIONS
    
    Example:
        ctx_create(EXE_0)
        _collect_executable("${_exe_json}" EXE_0)
        _create_executable_target(EXE_0)
]]
function(_create_executable_target CTX)
    
    # --------------------------------------------------------------------------
    # Read Context Data
    # --------------------------------------------------------------------------
    
    ctx_get(${CTX} NAME _name)
    ctx_get(${CTX} PATH _path)
    ctx_get(${CTX} TYPE _type)
    ctx_get(${CTX} VERSION _version)
    ctx_get(${CTX} PCH_ENABLED _pch_enabled)
    ctx_get(${CTX} PCH_HEADER _pch_header)
    ctx_get(${CTX} PCH_PATH _pch_custom_path)
    ctx_get(${CTX} DEPENDENCIES _dependencies)
    ctx_get(${CTX} EXTERNALS _externals)
    ctx_get(${CTX} EXTERNAL_OPTIONS _external_options)
    ctx_get(${CTX} DEFINES _defines)
    ctx_get(${CTX} COMPILE_OPTIONS _compile_options)
    ctx_get(${CTX} LINK_OPTIONS _link_options)
    
    # --------------------------------------------------------------------------
    # Validate Source Directory
    # --------------------------------------------------------------------------
    
    set(_src_dir "${CMAKE_SOURCE_DIR}/${_path}")
    
    if(NOT EXISTS "${_src_dir}")
        cmake_fatal("E001" "Executable '${_name}': Source path does not exist: ${_path}")
    endif()
    
    # --------------------------------------------------------------------------
    # Create Target (GUI vs. CONSOLE)
    # --------------------------------------------------------------------------
    
    if(_type STREQUAL "GUI")
        if(WIN32)
            add_executable(${_name} WIN32)
        elseif(APPLE)
            add_executable(${_name} MACOSX_BUNDLE)
        else()
            add_executable(${_name})
        endif()
    else()
        # CONSOLE, CLI, HEADLESS, WORKER or other
        add_executable(${_name})
    endif()
    
    dbg(${DBG_RARE} "    add_executable(${_name}) [${_type}]" ID EXECUTABLES)
     
    # --------------------------------------------------------------------------
    # Collect Sources (via SourceCollect.cmake)
    # --------------------------------------------------------------------------
    
    # Mode is determined by SOLUTION_SOURCE_MODE (explicit/glob/auto)
    collect_sources(
        ${_name}
        "${_src_dir}"
        _sources
        _headers
        _extras
        _modules
        _includes
    )
    
    if(NOT _sources)
        cmake_warn("W101" "Executable '${_name}': No source files found in ${_path}")
    endif()
    
    target_sources(${_name} PRIVATE ${_sources} ${_headers} ${_extras} ${_modules})
    
    dbg(${DBG_RARE} "    Sources: ${_sources}" ID EXECUTABLES)
    
    # --------------------------------------------------------------------------
    # Include Directory
    # --------------------------------------------------------------------------
    
    target_include_directories(${_name} PRIVATE "${_src_dir}")
    
    # Additional includes from Source.cmake
    foreach(_inc IN LISTS _includes)
        if(IS_ABSOLUTE "${_inc}")
            target_include_directories(${_name} PRIVATE "${_inc}")
        else()
            target_include_directories(${_name} PRIVATE "${_src_dir}/${_inc}")
        endif()
    endforeach()
    
    # Additionally: If there is a pch/ subdirectory
    if(EXISTS "${_src_dir}/pch")
        target_include_directories(${_name} PRIVATE "${_src_dir}/pch")
    endif()
    
    # --------------------------------------------------------------------------
    # Precompiled Headers
    # --------------------------------------------------------------------------
    
    if(_pch_enabled)
        set(_pch_found_path "")
        
        # If custom path specified, use it (relative to CMAKE_SOURCE_DIR/projects/)
        if(NOT "${_pch_custom_path}" STREQUAL "")
            set(_custom_full_path "${CMAKE_SOURCE_DIR}/projects/${_pch_custom_path}/${_pch_header}")
            if(EXISTS "${_custom_full_path}")
                set(_pch_found_path "${_custom_full_path}")
            endif()
        else()
            # Search priority: 1. pch/, 2. src/, 3. root
            if(EXISTS "${_src_dir}/pch/${_pch_header}")
                set(_pch_found_path "${_src_dir}/pch/${_pch_header}")
            elseif(EXISTS "${_src_dir}/${_pch_header}")
                set(_pch_found_path "${_src_dir}/${_pch_header}")
            elseif(EXISTS "${CMAKE_SOURCE_DIR}/${_path}/${_pch_header}")
                set(_pch_found_path "${CMAKE_SOURCE_DIR}/${_path}/${_pch_header}")
            endif()
        endif()
        
        if(_pch_found_path)
            target_precompile_headers(${_name} PRIVATE "${_pch_found_path}")
            dbg(${DBG_RARE} "    PCH: ${_pch_found_path}" ID EXECUTABLES)
        else()
            cmake_warn("W101" "Executable '${_name}': PCH enabled but '${_pch_header}' not found in pch/, src/, or root")
        endif()
    endif()
    
    # --------------------------------------------------------------------------
    # Internal Dependencies (Libraries)
    # --------------------------------------------------------------------------
    
    foreach(_dep IN LISTS _dependencies)
        if(TARGET ${_dep})
            target_link_libraries(${_name} PRIVATE ${_dep})
            dbg(${DBG_RARE} "    Link: ${_dep} (internal)" ID EXECUTABLES)
        else()
            cmake_fatal("E101" "Dependency '${_dep}' for '${_name}' does not exist")
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
        
        # Get options for this external from the target's external_options
        _json_has_key("${_external_options}" "${_ext}" _has_ext_opts)
        if(_has_ext_opts)
            _json_get_object("${_external_options}" "${_ext}" _ext_opts)
        else()
            set(_ext_opts "{}")
        endif()
        
        # Apply external via Orchestrator
        apply_external_to_target("${_name}" "${_ext}" "${_ext_opts}")
        
        dbg(${DBG_RARE} "    External: ${_ext} applied" ID EXECUTABLES)
    endforeach()
    
    # --------------------------------------------------------------------------
    # Preprocessor Definitions
    # --------------------------------------------------------------------------
    
    if(_defines)
        target_compile_definitions(${_name} PRIVATE ${_defines})
        dbg(${DBG_RARE} "    Defines: ${_defines}" ID EXECUTABLES)
    endif()
    
    # --------------------------------------------------------------------------
    # Additional Compiler Options
    # --------------------------------------------------------------------------
    
    if(_compile_options)
        target_compile_options(${_name} PRIVATE ${_compile_options})
        dbg(${DBG_RARE} "    Compile Options: ${_compile_options}" ID EXECUTABLES)
    endif()
    
    # --------------------------------------------------------------------------
    # Additional Linker Options
    # --------------------------------------------------------------------------
    
    if(_link_options)
        target_link_options(${_name} PRIVATE ${_link_options})
        dbg(${DBG_RARE} "    Link Options: ${_link_options}" ID EXECUTABLES)
    endif()
    
    # --------------------------------------------------------------------------
    # Apply Standard Modules
    # --------------------------------------------------------------------------
    
    # Warnings (from Warnings.cmake)
    apply_warnings(${_name})
    
    # Compiler options (from CompilerOptions.cmake)
    apply_compiler_options(${_name})
    
    # Output directories (from OutputDirs.cmake)
    setup_output_dirs(${_name})
    
    # --------------------------------------------------------------------------
    # Version as Target Property
    # --------------------------------------------------------------------------
    
    if(NOT "${_version}" STREQUAL "")
        set_target_properties(${_name} PROPERTIES
            VERSION "${_version}"
        )
    endif()
    
    # --------------------------------------------------------------------------
    # Windows-specific: Subsystem for GUI
    # --------------------------------------------------------------------------
    
    if(WIN32 AND _type STREQUAL "GUI")
        set_target_properties(${_name} PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
        # Define APP_WINDOWS_GUI for WinMain entry point
        target_compile_definitions(${_name} PRIVATE APP_WINDOWS_GUI)
        dbg(${DBG_RARE} "    Windows GUI: APP_WINDOWS_GUI defined" ID EXECUTABLES)
    endif()
    
    # --------------------------------------------------------------------------
    # macOS-specific: Bundle Properties
    # --------------------------------------------------------------------------
    
    if(APPLE AND _type STREQUAL "GUI")
        ctx_get(${CTX} DISPLAY_NAME _display_name)
        set_target_properties(${_name} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.project.${_name}"
            MACOSX_BUNDLE_BUNDLE_NAME "${_display_name}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${_version}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${_version}"
        )
    endif()
    
endfunction()
