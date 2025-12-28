# cmake/core/Context.cmake
# ========================
# Context object pattern for isolated namespaces
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - None (base module)
#
# Provides:
#   - ctx_create(PREFIX)
#   - ctx_set(PREFIX KEY VALUE)
#   - ctx_get(PREFIX KEY OUT_VAR)
#   - ctx_dump(PREFIX)
#
# Used by:
#   - ExecutableCollect.cmake
#   - LibraryCollect.cmake
#   - TestCollect.cmake
#
# IMPORTANT: Uses GLOBAL PROPERTY instead of PARENT_SCOPE,
# so values are available across all function levels.

include_guard(GLOBAL)

# Cache variable for debug output
set(DEBUG_CONTEXT OFF CACHE BOOL "Enable ctx_dump() debug output")

# ============================================================================
# ctx_create - Create new context
# ============================================================================
#[[
    ctx_create(PREFIX)
    
    Creates a new, empty context with the given prefix.
    
    Parameters:
        PREFIX  - Mandatory: Unique namespace identifier (e.g. EXE_MyApp)
    
    Example:
        ctx_create(EXE_MyApp)
        ctx_create(LIB_CoreLib)
        ctx_create(TEST_UnitTests)
    
    Convention:
        EXE_*   - For executables
        LIB_*   - For libraries
        TEST_*  - For tests
]]
function(ctx_create PREFIX)
    # Initialize keys list (for ctx_dump)
    set_property(GLOBAL PROPERTY ${PREFIX}_KEYS "")
endfunction()

# ============================================================================
# ctx_set - Set value (as GLOBAL Property)
# ============================================================================
#[[
    ctx_set(PREFIX KEY VALUE)
    
    Sets a value in the context. Uses GLOBAL PROPERTY for
    reliable propagation across all function levels.
    
    Parameters:
        PREFIX  - Mandatory: Context namespace (e.g. EXE_MyApp)
        KEY     - Mandatory: Key (UPPER_SNAKE_CASE recommended)
        VALUE   - Mandatory: Value (string, list separated with ;)
    
    Example:
        ctx_set(EXE_MyApp NAME "MyApp")
        ctx_set(EXE_MyApp VERSION "1.0.0")
        ctx_set(EXE_MyApp EXTERNALS "bass;imgui;glfw")
]]
function(ctx_set PREFIX KEY VALUE)
    # Set value as GLOBAL Property
    set_property(GLOBAL PROPERTY ${PREFIX}_${KEY} "${VALUE}")
    
    # Add key to list (for ctx_dump)
    get_property(_keys GLOBAL PROPERTY ${PREFIX}_KEYS)
    list(APPEND _keys ${KEY})
    list(REMOVE_DUPLICATES _keys)
    set_property(GLOBAL PROPERTY ${PREFIX}_KEYS "${_keys}")
endfunction()

# ============================================================================
# ctx_get - Read value (from GLOBAL Property)
# ============================================================================
#[[
    ctx_get(PREFIX KEY OUT_VAR)
    
    Reads a value from the context.
    
    Parameters:
        PREFIX  - Mandatory: Context namespace
        KEY     - Mandatory: Key
        OUT_VAR - Mandatory: Variable that receives the value
    
    Returns:
        OUT_VAR is set to the stored value.
        Empty string if key doesn't exist.
    
    Example:
        ctx_get(EXE_MyApp NAME _name)
        message(STATUS "Name: ${_name}")
        
        ctx_get(EXE_MyApp EXTERNALS _externals)
        foreach(_ext IN LISTS _externals)
            message(STATUS "External: ${_ext}")
        endforeach()
]]
function(ctx_get PREFIX KEY OUT_VAR)
    get_property(_value GLOBAL PROPERTY ${PREFIX}_${KEY})
    set(${OUT_VAR} "${_value}" PARENT_SCOPE)
endfunction()

# ============================================================================
# ctx_dump - Debug output of all keys (only when DEBUG_CONTEXT=ON)
# ============================================================================
#[[
    ctx_dump(PREFIX)
    
    Outputs all stored keys and values of a context.
    Only active when cache variable DEBUG_CONTEXT=ON.
    
    Parameters:
        PREFIX  - Mandatory: Context namespace
    
    Activation:
        cmake -B build -DDEBUG_CONTEXT=ON
    
    Example:
        ctx_dump(EXE_MyApp)
    
    Output:
        -- === Context Dump: EXE_MyApp ===
        --   NAME = MyApp
        --   VERSION = 1.0.0
        --   TYPE = GUI
        -- === End Context: EXE_MyApp ===
]]
function(ctx_dump PREFIX)
    if(NOT DEBUG_CONTEXT)
        return()
    endif()
    
    message(STATUS "=== Context Dump: ${PREFIX} ===")
    get_property(_keys GLOBAL PROPERTY ${PREFIX}_KEYS)
    
    foreach(_key IN LISTS _keys)
        get_property(_value GLOBAL PROPERTY ${PREFIX}_${_key})
        message(STATUS "  ${_key} = ${_value}")
    endforeach()
    
    message(STATUS "=== End Context: ${PREFIX} ===")
endfunction()
