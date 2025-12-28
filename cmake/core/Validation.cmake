# cmake/core/Validation.cmake
# ============================
# JSON schema validation for Solution.json and Externals
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - Errors.cmake (cmake_fatal, cmake_warn)
#   - Json.cmake (_json_* functions)
#
# Provides:
#   - validate_external_source()        - Exactly one source field (incl. system)
#   - validate_required_fields()        - Check required fields
#   - validate_fetched_external()       - Check tag/branch/commit
#   - validate_local_external()         - Include.cmake exists
#   - validate_solution_schema()        - Check schema version
#   - validate_local_external_include() - Best practice checks
#
# Phase 9 Changes:
#   - Added 'system' to source fields in validate_external_source()
#   - Added E502 validation: system: true requires 'package' field
#
# Used by:
#   - Solution.cmake
#   - Orchestrator.cmake
#   - system/Handler.cmake

include_guard(GLOBAL)

# ============================================================================
# validate_external_source - Check for exactly one source field
# ============================================================================
#[[
    validate_external_source(EXT_NAME EXT_JSON)
    
    Validates that an external has exactly one source field.
    
    Parameters:
        EXT_NAME - Mandatory: Name of the external
        EXT_JSON - Mandatory: JSON string of the external object
    
    Allowed source fields:
        system, path, git, vcpkg, conan, find_package
    
    Errors:
        E012 - no source field present
        E012 - multiple source fields present
        E502 - system: true but 'package' field missing
    
    Example:
        validate_external_source("bass" "${_ext_json}")
        validate_external_source("qt6" "{\"system\":true,\"package\":\"Qt6\"}")
]]
function(validate_external_source EXT_NAME EXT_JSON)
    # Source fields - system added for Phase 9
    set(_source_fields "system;path;git;vcpkg;conan;find_package")
    set(_found_count 0)
    set(_found_fields "")
    
    foreach(_field IN LISTS _source_fields)
        _json_has_key("${EXT_JSON}" "${_field}" _has)
        if(_has)
            math(EXPR _found_count "${_found_count} + 1")
            list(APPEND _found_fields "${_field}")
        endif()
    endforeach()
    
    if(_found_count EQUAL 0)
        cmake_fatal("E012" 
            "External '${EXT_NAME}': No source field specified.\n"
            "  Required: one of 'system', 'git', or 'path'\n"
            "  Examples:\n"
            "    Local:   { \"path\": \"externals/${EXT_NAME}\" }\n"
            "    Fetched: { \"git\": \"https://...\", \"tag\": \"v1.0\" }\n"
            "    System:  { \"system\": true, \"package\": \"PackageName\" }"
        )
    elseif(_found_count GREATER 1)
        cmake_fatal("E012" 
            "External '${EXT_NAME}': Multiple source fields specified: ${_found_fields}\n"
            "  Only one source field is allowed."
        )
    endif()
    
    # Additional validation for system externals
    if("system" IN_LIST _found_fields)
        _json_get_bool_or_default("${EXT_JSON}" "system" FALSE _is_system)
        if(_is_system)
            _json_has_key("${EXT_JSON}" "package" _has_package)
            if(NOT _has_package)
                cmake_fatal("E502" 
                    "System external '${EXT_NAME}': 'package' field is required.\n"
                    "  Example: { \"system\": true, \"package\": \"Qt6\" }"
                )
            endif()
        endif()
    endif()
endfunction()

# ============================================================================
# validate_required_fields - Check required fields of an entity
# ============================================================================
#[[
    validate_required_fields(JSON_STRING ENTITY_TYPE ENTITY_NAME FIELDS field1 [field2...])
    
    Validates that all specified required fields exist.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        ENTITY_TYPE - Mandatory: Type for error message (e.g. "Executable")
        ENTITY_NAME - Mandatory: Name for error message
        FIELDS      - Mandatory: List of required fields
    
    Errors:
        E001 - if a required field is missing
    
    Example:
        validate_required_fields("${_json}" "Executable" "MyApp" FIELDS name path)
]]
function(validate_required_fields JSON_STRING ENTITY_TYPE ENTITY_NAME)
    cmake_parse_arguments(ARG "" "" "FIELDS" ${ARGN})
    
    foreach(_field IN LISTS ARG_FIELDS)
        _json_has_key("${JSON_STRING}" "${_field}" _has)
        if(NOT _has)
            cmake_fatal("E001" "${ENTITY_TYPE} '${ENTITY_NAME}': Required field '${_field}' missing")
        endif()
    endforeach()
endfunction()

# ============================================================================
# validate_fetched_external - Fetched external needs version
# ============================================================================
#[[
    validate_fetched_external(EXT_NAME EXT_JSON)
    
    Validates that a fetched external has a version specifier.
    
    Parameters:
        EXT_NAME - Mandatory: Name of the external
        EXT_JSON - Mandatory: JSON string of the external object
    
    Checks for at least one of:
        tag, branch, commit
    
    Errors:
        E215 - if none present
    
    Example:
        validate_fetched_external("glfw" "${_ext_json}")
]]
function(validate_fetched_external EXT_NAME EXT_JSON)
    _json_has_key("${EXT_JSON}" "tag" _has_tag)
    _json_has_key("${EXT_JSON}" "branch" _has_branch)
    _json_has_key("${EXT_JSON}" "commit" _has_commit)
    
    if(NOT _has_tag AND NOT _has_branch AND NOT _has_commit)
        cmake_fatal("E215" "Fetched external '${EXT_NAME}': No tag/branch/commit specified")
    endif()
endfunction()

# ============================================================================
# validate_local_external - Include.cmake must exist
# ============================================================================
#[[
    validate_local_external(EXT_NAME EXT_JSON)
    
    Validates that the Include.cmake for a local external exists.
    
    Parameters:
        EXT_NAME - Mandatory: Name of the external
        EXT_JSON - Mandatory: JSON string of the external object
    
    Checks:
        - Custom "include" field or
        - Convention: ${path}/Include.cmake
    
    Errors:
        E213 - if Include.cmake not found
    
    Example:
        validate_local_external("bass" "${_ext_json}")
]]
function(validate_local_external EXT_NAME EXT_JSON)
    _json_get_string("${EXT_JSON}" "path" _ext_path)
    _json_get_string_or_default("${EXT_JSON}" "include" "${_ext_path}/Include.cmake" _include)
    
    set(_full_path "${CMAKE_SOURCE_DIR}/${_include}")
    
    if(NOT EXISTS "${_full_path}")
        cmake_fatal("E213" "Local external '${EXT_NAME}': Include.cmake not found at ${_full_path}")
    endif()
endfunction()

# ============================================================================
# validate_solution_schema - Check schema version
# ============================================================================
#[[
    validate_solution_schema(SOLUTION_JSON)
    
    Validates the schema version of Solution.json.
    
    Parameters:
        SOLUTION_JSON - Mandatory: Complete JSON string of Solution.json
    
    Warnings:
        W001 - if schemaVersion is missing
        W001 - if version < 0.1
    
    Example:
        file(READ "Solution.json" _json)
        validate_solution_schema("${_json}")
]]
function(validate_solution_schema SOLUTION_JSON)
    _json_get_string("${SOLUTION_JSON}" "schemaVersion" _schema_version)
    
    if("${_schema_version}" STREQUAL "")
        cmake_warn("W001" "Solution.json: No 'schemaVersion' field found")
        return()
    endif()
    
    # Check minimum version 0.1
    if(NOT "${_schema_version}" VERSION_GREATER_EQUAL "0.1")
        cmake_warn("W001" "Solution.json: schemaVersion < 0.1, features may be limited")
    endif()
endfunction()

# ============================================================================
# validate_local_external_include - Best practice checks for Include.cmake
# ============================================================================
#[[
    validate_local_external_include(INCLUDE_FILE EXT_NAME)
    
    Checks Include.cmake for best practices.
    
    Parameters:
        INCLUDE_FILE - Mandatory: Full path to Include.cmake
        EXT_NAME     - Mandatory: Name of the external
    
    Warnings:
        W103 - if add_executable() found (IDE clutter)
        W104 - if add_subdirectory(examples|tests|...) found
    
    Example:
        validate_local_external_include("${_include_path}" "bass")
]]
function(validate_local_external_include INCLUDE_FILE EXT_NAME)
    if(NOT EXISTS "${INCLUDE_FILE}")
        return()  # Error is thrown by validate_local_external
    endif()
    
    file(READ "${INCLUDE_FILE}" _content)
    
    # Check for add_executable (creates unnecessary IDE entries)
    string(FIND "${_content}" "add_executable" _pos)
    if(NOT _pos EQUAL -1)
        cmake_warn("W103" "Local external '${EXT_NAME}': Include.cmake creates executables (IDE clutter)")
    endif()
    
    # Check for add_subdirectory with example directories
    string(REGEX MATCH "add_subdirectory\\((examples|tests|samples|demos)" _match "${_content}")
    if(_match)
        cmake_warn("W104" "Local external '${EXT_NAME}': Include.cmake includes example directories")
    endif()
endfunction()
