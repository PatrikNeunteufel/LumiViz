# cmake/project/ExecutableCollect.cmake
# ======================================
# Collects executable data from JSON into a Context
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
#
# Provides:
#   - _collect_executable(EXE_JSON CTX)
#
# Context Keys Set:
#   - NAME, DISPLAY_NAME, DESCRIPTION, VERSION, PATH, TYPE
#   - SKIP, PCH_ENABLED, PCH_HEADER, PCH_PATH
#   - DEPENDENCIES, EXTERNALS, EXTERNAL_OPTIONS
#   - PLATFORMS, DEFINES, COMPILE_OPTIONS, LINK_OPTIONS
#
# Used by:
#   - Executables.cmake

include_guard(GLOBAL)

# ============================================================================
# _collect_executable - Collects executable data from JSON into Context
# ============================================================================
#[[
    _collect_executable(EXE_JSON CTX)
    
    Parses an executable definition from JSON and stores all fields
    in a Context for later processing by ExecutableCreate.
    
    Parameters:
        EXE_JSON - Mandatory: JSON string of the executable
        CTX      - Mandatory: Context prefix (e.g. EXE_0, EXE_1)
    
    Context Keys Set:
        NAME, DISPLAY_NAME, DESCRIPTION, VERSION, PATH, TYPE,
        SKIP, PCH_ENABLED, PCH_HEADER, DEPENDENCIES, EXTERNALS,
        EXTERNAL_OPTIONS, PLATFORMS, DEFINES, COMPILE_OPTIONS, LINK_OPTIONS
    
    Example:
        ctx_create(EXE_0)
        _collect_executable("${_exe_json}" EXE_0)
        ctx_get(EXE_0 NAME _name)
]]
function(_collect_executable EXE_JSON CTX)
    
    # --------------------------------------------------------------------------
    # Required Field: name
    # --------------------------------------------------------------------------
    
    _json_get_string("${EXE_JSON}" "name" _name)
    if("${_name}" STREQUAL "")
        cmake_fatal("E001" "Executable has no 'name' field")
    endif()
    ctx_set(${CTX} NAME "${_name}")
    
    # --------------------------------------------------------------------------
    # Optional Metadata
    # --------------------------------------------------------------------------
    
    _json_get_string_or_default("${EXE_JSON}" "displayName" "${_name}" _display_name)
    ctx_set(${CTX} DISPLAY_NAME "${_display_name}")
    
    _json_get_string_or_default("${EXE_JSON}" "description" "" _description)
    ctx_set(${CTX} DESCRIPTION "${_description}")
    
    # Version: from executable or solution
    _json_has_key("${EXE_JSON}" "version" _has_version)
    if(_has_version)
        _json_get_string("${EXE_JSON}" "version" _version)
    else()
        get_property(_version GLOBAL PROPERTY SOLUTION_VERSION)
    endif()
    ctx_set(${CTX} VERSION "${_version}")
    
    # --------------------------------------------------------------------------
    # Path (with intelligent default)
    # --------------------------------------------------------------------------
    
    _json_has_key("${EXE_JSON}" "path" _has_path)
    if(_has_path)
        _json_get_string("${EXE_JSON}" "path" _path)
    else()
        # Convention: projects/exec/{name}/src
        set(_path "projects/exec/${_name}/src")
    endif()
    ctx_set(${CTX} PATH "${_path}")
    
    # --------------------------------------------------------------------------
    # Type (CONSOLE, GUI, CLI, HEADLESS, WORKER)
    # --------------------------------------------------------------------------
    
    get_property(_default_type GLOBAL PROPERTY SOLUTION_DEFAULT_EXECUTABLE_TYPE)
    if("${_default_type}" STREQUAL "")
        set(_default_type "CONSOLE")
    endif()
    
    _json_get_string_or_default("${EXE_JSON}" "type" "${_default_type}" _type)
    string(TOUPPER "${_type}" _type)
    ctx_set(${CTX} TYPE "${_type}")
    
    # --------------------------------------------------------------------------
    # Skip Flag
    # --------------------------------------------------------------------------
    
    _json_get_bool_from_key("${EXE_JSON}" "skip" _skip)
    ctx_set(${CTX} SKIP "${_skip}")
    
    # --------------------------------------------------------------------------
    # PCH (Precompiled Headers)
    # --------------------------------------------------------------------------
    
    set(_pch_enabled FALSE)
    set(_pch_header "pch.h")
    set(_pch_path "")
    
    _json_has_key("${EXE_JSON}" "pch" _has_pch)
    if(_has_pch)
        _json_get_object("${EXE_JSON}" "pch" _pch_obj)
        
        # Check for explicit enabled field
        _json_has_key("${_pch_obj}" "enabled" _has_enabled)
        if(_has_enabled)
            _json_get_bool_from_key("${_pch_obj}" "enabled" _pch_enabled)
        endif()
        
        # Check for header field
        _json_has_key("${_pch_obj}" "header" _has_header)
        if(_has_header)
            _json_get_string("${_pch_obj}" "header" _pch_header)
            # Implicit enable if header specified and not explicitly disabled
            if(NOT _has_enabled)
                set(_pch_enabled TRUE)
            endif()
        endif()
        
        # Check for path field
        _json_has_key("${_pch_obj}" "path" _has_path)
        if(_has_path)
            _json_get_string("${_pch_obj}" "path" _pch_path)
            # Implicit enable if path specified and not explicitly disabled
            if(NOT _has_enabled)
                set(_pch_enabled TRUE)
            endif()
        endif()
    endif()
    
    ctx_set(${CTX} PCH_ENABLED "${_pch_enabled}")
    ctx_set(${CTX} PCH_HEADER "${_pch_header}")
    ctx_set(${CTX} PCH_PATH "${_pch_path}")
    
    # --------------------------------------------------------------------------
    # Dependencies (internal libraries)
    # --------------------------------------------------------------------------
    
    set(_dependencies "")
    _json_array_length("${EXE_JSON}" "dependencies" _dep_count)
    if(_dep_count GREATER 0)
        math(EXPR _dep_last "${_dep_count} - 1")
        foreach(_dep_idx RANGE 0 ${_dep_last})
            _json_array_get("${EXE_JSON}" "dependencies" ${_dep_idx} _dep)
            list(APPEND _dependencies "${_dep}")
        endforeach()
    endif()
    ctx_set(${CTX} DEPENDENCIES "${_dependencies}")
    
    # --------------------------------------------------------------------------
    # Externals (external dependencies)
    # --------------------------------------------------------------------------
    
    set(_externals "")
    _json_array_length("${EXE_JSON}" "externals" _ext_count)
    if(_ext_count GREATER 0)
        math(EXPR _ext_last "${_ext_count} - 1")
        foreach(_ext_idx RANGE 0 ${_ext_last})
            _json_array_get("${EXE_JSON}" "externals" ${_ext_idx} _ext)
            list(APPEND _externals "${_ext}")
        endforeach()
    endif()
    ctx_set(${CTX} EXTERNALS "${_externals}")
    
    # --------------------------------------------------------------------------
    # External Options (JSON block for later processing)
    # --------------------------------------------------------------------------
    
    _json_get_object_or_empty("${EXE_JSON}" "external_options" _ext_options)
    ctx_set(${CTX} EXTERNAL_OPTIONS "${_ext_options}")
    
    # --------------------------------------------------------------------------
    # Platforms (platform filter)
    # --------------------------------------------------------------------------
    
    set(_platforms "")
    _json_array_length("${EXE_JSON}" "platforms" _plat_count)
    if(_plat_count GREATER 0)
        math(EXPR _plat_last "${_plat_count} - 1")
        foreach(_plat_idx RANGE 0 ${_plat_last})
            _json_array_get("${EXE_JSON}" "platforms" ${_plat_idx} _plat)
            list(APPEND _platforms "${_plat}")
        endforeach()
    endif()
    ctx_set(${CTX} PLATFORMS "${_platforms}")
    
    # --------------------------------------------------------------------------
    # Defines (preprocessor definitions)
    # --------------------------------------------------------------------------
    
    set(_defines "")
    _json_array_length("${EXE_JSON}" "defines" _def_count)
    if(_def_count GREATER 0)
        math(EXPR _def_last "${_def_count} - 1")
        foreach(_def_idx RANGE 0 ${_def_last})
            _json_array_get("${EXE_JSON}" "defines" ${_def_idx} _def)
            list(APPEND _defines "${_def}")
        endforeach()
    endif()
    ctx_set(${CTX} DEFINES "${_defines}")
    
    # --------------------------------------------------------------------------
    # Compile Options
    # --------------------------------------------------------------------------
    
    set(_compile_options "")
    _json_array_length("${EXE_JSON}" "compile_options" _co_count)
    if(_co_count GREATER 0)
        math(EXPR _co_last "${_co_count} - 1")
        foreach(_co_idx RANGE 0 ${_co_last})
            _json_array_get("${EXE_JSON}" "compile_options" ${_co_idx} _co)
            list(APPEND _compile_options "${_co}")
        endforeach()
    endif()
    ctx_set(${CTX} COMPILE_OPTIONS "${_compile_options}")
    
    # --------------------------------------------------------------------------
    # Link Options
    # --------------------------------------------------------------------------
    
    set(_link_options "")
    _json_array_length("${EXE_JSON}" "link_options" _lo_count)
    if(_lo_count GREATER 0)
        math(EXPR _lo_last "${_lo_count} - 1")
        foreach(_lo_idx RANGE 0 ${_lo_last})
            _json_array_get("${EXE_JSON}" "link_options" ${_lo_idx} _lo)
            list(APPEND _link_options "${_lo}")
        endforeach()
    endif()
    ctx_set(${CTX} LINK_OPTIONS "${_link_options}")
    
    # --------------------------------------------------------------------------
    # Debug Output (at high level)
    # --------------------------------------------------------------------------
    
    dbg(${DBG_RARE} "  Collected ${_name}:" ID EXECUTABLES)
    dbg(${DBG_RARE} "    PATH: ${_path}" ID EXECUTABLES)
    dbg(${DBG_RARE} "    TYPE: ${_type}" ID EXECUTABLES)
    dbg(${DBG_RARE} "    SKIP: ${_skip}" ID EXECUTABLES)
    dbg(${DBG_RARE} "    PCH: ${_pch_enabled} (header=${_pch_header}, path=${_pch_path})" ID EXECUTABLES)
    dbg(${DBG_ULTRA_RARE} "    DEPENDENCIES: ${_dependencies}" ID EXECUTABLES)
    dbg(${DBG_ULTRA_RARE} "    EXTERNALS: ${_externals}" ID EXECUTABLES)
    dbg(${DBG_ULTRA_RARE} "    PLATFORMS: ${_platforms}" ID EXECUTABLES)
    
endfunction()
