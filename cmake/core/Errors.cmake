# cmake/core/Errors.cmake
# ========================
# Unified error handling for the entire build system
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - None (base module, must be loaded first)
#
# Provides:
#   - cmake_fatal(CODE MESSAGE)      - Abort build with error code
#   - cmake_warn(CODE MESSAGE)       - Warning, build continues
#   - cmake_assert(CONDITION MSG)    - Assertion (for internal checks)
#   - cmake_require_field(CTX FIELD) - Field validation for Context
#
# Used by:
#   - All modules for error handling
#
# Error code ranges:
#   E0xx - JSON/Parsing
#   E1xx - Target creation
#   E2xx - Externals
#   W0xx - Deprecation
#   W1xx - Config/Validation
#   W2xx - Tools/Setup
#   W3xx - Externals (Warnings)
#
# Changes v0.1.2:
#   - Added W3xx range for Externals warnings
#   - Documented E218 (Offline without cache)
#   - Documented W302 (Version mismatch in offline mode)

include_guard(GLOBAL)

# ============================================================================
# cmake_fatal - Fatal error (aborts build)
# ============================================================================
#[[
    cmake_fatal(CODE MESSAGE)
    
    Aborts the build with a standardized error code.
    
    Parameters:
        CODE    - Mandatory: Error code (e.g. E001, E012, E201)
        MESSAGE - Mandatory: Error description
    
    Error Codes:
        E0xx - JSON/Parsing
            E001 - Required field missing
            E002 - Solution.json not found
            E010 - External not defined in externals block
            E012 - External: no/multiple source fields
        
        E1xx - Target Creation
            E101 - Dependency does not exist
            E102 - Target already exists
            E103 - Circular dependency
            E104 - Source.cmake not found (mode=explicit)
        
        E2xx - Externals
            E201 - Fetched external: no target in registry
            E202 - External fetch failed
            E213 - Local external: Include.cmake not found
            E214 - Local external: path does not exist
            E215 - Fetched external: no tag/branch/commit
            E216 - Explicitly specified hook not found
            E217 - PostFetch hook required (cmakeSupport=false)
            E218 - External not cached and offline mode enabled
    
    Example:
        cmake_fatal("E001" "Executable 'MyApp': Required field 'name' missing")
        cmake_fatal("E012" "External 'imgui': No source field specified")
        cmake_fatal("E218" "External 'glfw': Not cached and offline mode enabled")
]]
function(cmake_fatal CODE MESSAGE)
    message(FATAL_ERROR "[${CODE}] ${MESSAGE}")
endfunction()

# ============================================================================
# cmake_warn - Warning (build continues)
# ============================================================================
#[[
    cmake_warn(CODE MESSAGE)
    
    Outputs a warning, build continues.
    
    Parameters:
        CODE    - Mandatory: Warning code (e.g. W001, W103, W201)
        MESSAGE - Mandatory: Warning description
    
    Warning Codes:
        W0xx - Deprecation
            W001 - Deprecated schema version
            W002 - Deprecated syntax/field
        
        W1xx - Config/Validation
            W101 - Suboptimal configuration
            W102 - Optional feature missing
            W103 - Include.cmake creates executables
            W104 - Include.cmake includes example directories
            W105 - Version not SemVer compliant
            W106 - Target has no version
            W107 - Target overrides global standards
            W108 - Multiple test frameworks without explicit field
            W109 - C++20 modules used (experimental)
            W110 - GLOB fallback active
        
        W2xx - Tools/Setup
            W201 - Clang-Tidy enabled but not found
        
        W3xx - Externals
            W301 - Offline mode: using existing external (version may differ)
            W302 - Version mismatch but offline mode - using cached
    
    Example:
        cmake_warn("W001" "Schema version < 0.1, features limited")
        cmake_warn("W201" "ENABLE_CLANG_TIDY=ON but clang-tidy not found")
        cmake_warn("W302" "External 'glfw': Version mismatch but offline mode - using cached")
]]
function(cmake_warn CODE MESSAGE)
    message(WARNING "[${CODE}] ${MESSAGE}")
endfunction()

# ============================================================================
# cmake_assert - Assertion (for internal consistency checks)
# ============================================================================
#[[
    cmake_assert(CONDITION MESSAGE)
    
    Checks a condition and aborts on failure.
    For internal consistency checks, not for user errors.
    
    NOTE: This is a macro(), not function(), to evaluate CONDITION correctly.
    
    Parameters:
        CONDITION - Mandatory: CMake condition (checked with if(NOT ...))
        MESSAGE   - Mandatory: Error description on failed assertion
    
    Example:
        cmake_assert(DEFINED _var "Variable must be defined")
        cmake_assert(_count GREATER 0 "Count must be > 0")
        cmake_assert("${_type}" STREQUAL "GUI" "Unexpected type")
]]
macro(cmake_assert CONDITION MESSAGE)
    if(NOT (${CONDITION}))
        cmake_fatal("ASSERT" "${MESSAGE}")
    endif()
endmacro()

# ============================================================================
# cmake_require_field - Field validation for Context
# ============================================================================
#[[
    cmake_require_field(CTX FIELD_NAME ENTITY_TYPE)
    
    Validates that a field is set in a Context.
    Uses ctx_get() from Context.cmake.
    
    Parameters:
        CTX         - Mandatory: Context prefix (e.g. EXE_0, LIB_CoreLib)
        FIELD_NAME  - Mandatory: Name of the field to check
        ENTITY_TYPE - Mandatory: Type for error message (e.g. "Executable")
    
    Errors:
        E001 - if field is missing or empty
    
    Example:
        cmake_require_field(EXE_0 NAME "Executable")
        cmake_require_field(EXE_0 PATH "Executable")
        cmake_require_field(LIB_0 TYPE "Library")
]]
function(cmake_require_field CTX FIELD_NAME ENTITY_TYPE)
    ctx_get(${CTX} ${FIELD_NAME} _value)
    if(NOT DEFINED _value OR "${_value}" STREQUAL "")
        ctx_get(${CTX} NAME _name)
        cmake_fatal("E001" "${ENTITY_TYPE} '${_name}': Missing field '${FIELD_NAME}'")
    endif()
endfunction()
