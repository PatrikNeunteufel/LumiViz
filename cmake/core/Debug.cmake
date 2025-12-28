# cmake/core/Debug.cmake
# =======================
# Context-based debug system with two-axis filtering
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - None (base module, should be loaded early)
#
# Concept:
#   SHOW-Level (1-5): How much do I want to see?
#   FREQ-Level (1-5): How important is this message?
#   Rule: Message appears when FREQ <= SHOW
#
# Provides:
#   - dbg_init(ID ... LEVEL ... SWITCH ... TAG ...)
#   - dbg(FREQ_LEVEL "message" ID ...)
#   - dbgspace(ID ...)
#   - enddbgblock(ID ...)
#   - setup_debug_from_args(...)
#
# Used by:
#   - All other modules for debug output
#   - CompilerOptions.cmake, Solution.cmake, Executables.cmake, ...

include_guard(GLOBAL)

# ============================================================================
# Global Defaults
# ============================================================================

# Master switch: OFF = no debug output at all
if(NOT DEFINED DEBUG_MESSAGES)
    set(DEBUG_MESSAGES ON)
endif()

# Default SHOW level for new contexts (2 = DBG_SHOW_SOME)
if(NOT DEFINED DEBUG_DEFAULT_LEVEL)
    set(DEBUG_DEFAULT_LEVEL 2)
endif()

# ============================================================================
# SHOW-Level Constants (how much do I want to see)
# ============================================================================

set(DBG_SHOW_LITTLE  1)  # Only the essentials
set(DBG_SHOW_SOME    2)  # Standard (Default)
set(DBG_SHOW_MUCH    3)  # More details
set(DBG_SHOW_LOTS    4)  # Many details
set(DBG_SHOW_ALL     5)  # Everything

# ============================================================================
# FREQ-Level Constants (how important is this message)
# ============================================================================

set(DBG_OFTEN       1)  # Frequent, important (module start, phase start)
set(DBG_COMMON      2)  # Common info (features, found files)
set(DBG_NORMAL      3)  # Standard importance (intermediate steps)
set(DBG_RARE        4)  # Rarely needed (paths, variables)
set(DBG_ULTRA_RARE  5)  # Only for deep debugging (loop iterations)

# ============================================================================
# dbg_init - Initialize context
# ============================================================================
#[[
    dbg_init(ID <context_id> [LEVEL <show_level>] [SWITCH <ON|OFF>] [TAG <prefix>])
    
    Initializes a debug context for structured output.
    
    Parameters:
        ID      - Mandatory: Unique context identifier
        LEVEL   - Optional: SHOW level (DBG_SHOW_* or 1-5)
        SWITCH  - Optional: ON/OFF (Default: ON)
        TAG     - Optional: Prefix for output in square brackets
    
    Example:
        dbg_init(ID MY_MODULE LEVEL ${DBG_SHOW_MUCH} SWITCH ON TAG "MyModule")
]]
function(dbg_init)
    if(NOT DEBUG_MESSAGES)
        return()
    endif()

    cmake_parse_arguments(ARG "" "ID;LEVEL;SWITCH;TAG" "" ${ARGN})

    if(NOT ARG_ID)
        message(WARNING "[Debug] dbg_init: ID not specified")
        return()
    endif()

    # Defaults
    if(NOT DEFINED ARG_LEVEL)
        set(ARG_LEVEL ${DEBUG_DEFAULT_LEVEL})
    endif()

    if(NOT DEFINED ARG_SWITCH)
        set(ARG_SWITCH ON)
    endif()

    if(NOT DEFINED ARG_TAG)
        set(ARG_TAG "")
    endif()

    # Store as directory properties (for isolation)
    set_property(DIRECTORY PROPERTY DBG_${ARG_ID}_LEVEL ${ARG_LEVEL})
    set_property(DIRECTORY PROPERTY DBG_${ARG_ID}_SWITCH ${ARG_SWITCH})
    set_property(DIRECTORY PROPERTY DBG_${ARG_ID}_TAG "${ARG_TAG}")
    set_property(DIRECTORY PROPERTY DBG_${ARG_ID}_ONCE_GUARD "")
endfunction()

# ============================================================================
# dbg - Output debug message
# ============================================================================
#[[
    dbg(<freq_level> "<message>" [ID <context_id>] [LEVEL <override>] 
        [SWITCH <ON|OFF>] [TAG <extra>] [ONCE])
    
    Outputs a debug message if the filter matches (FREQ <= SHOW).
    
    Parameters:
        freq_level  - Mandatory: Importance (DBG_OFTEN to DBG_ULTRA_RARE)
        message     - Mandatory: Message to output
        ID          - Optional: Context reference
        LEVEL       - Optional: Override SHOW level
        SWITCH      - Optional: Override switch
        TAG         - Optional: Additional tag
        ONCE        - Optional: Output only once per context
    
    Example:
        dbg(${DBG_OFTEN} "=== Start ===" ID MY_DBG)
        dbg(${DBG_COMMON} "Processing ${_file}" ID MY_DBG)
        dbg(${DBG_RARE} "Deprecated" ID MY_DBG ONCE)
]]
function(dbg FREQ_LEVEL MESSAGE)
    if(NOT DEBUG_MESSAGES)
        return()
    endif()

    cmake_parse_arguments(ARG "ONCE" "ID;LEVEL;SWITCH;TAG" "" ${ARGN})

    # Fallback chain: ID -> Default -> Global
    set(_show_level ${DEBUG_DEFAULT_LEVEL})
    set(_switch ON)
    set(_tag "")

    if(ARG_ID)
        get_property(_has_level DIRECTORY PROPERTY DBG_${ARG_ID}_LEVEL SET)
        if(_has_level)
            get_property(_show_level DIRECTORY PROPERTY DBG_${ARG_ID}_LEVEL)
            get_property(_switch DIRECTORY PROPERTY DBG_${ARG_ID}_SWITCH)
            get_property(_tag DIRECTORY PROPERTY DBG_${ARG_ID}_TAG)
        endif()
    endif()

    # Overrides
    if(DEFINED ARG_LEVEL)
        set(_show_level ${ARG_LEVEL})
    endif()

    if(DEFINED ARG_SWITCH)
        set(_switch ${ARG_SWITCH})
    endif()

    if(DEFINED ARG_TAG)
        set(_tag "${_tag}${ARG_TAG}")
    endif()

    # Switch check
    if(NOT _switch)
        return()
    endif()

    # ONCE check (only once per message per context)
    if(ARG_ONCE AND ARG_ID)
        get_property(_guard DIRECTORY PROPERTY DBG_${ARG_ID}_ONCE_GUARD)
        list(FIND _guard "${MESSAGE}" _idx)
        if(NOT _idx EQUAL -1)
            return()  # Already shown
        endif()
        list(APPEND _guard "${MESSAGE}")
        set_property(DIRECTORY PROPERTY DBG_${ARG_ID}_ONCE_GUARD "${_guard}")
    endif()

    # FREQ <= SHOW?
    if(NOT ${FREQ_LEVEL} LESS_EQUAL ${_show_level})
        return()
    endif()

    # Output
    if(_tag)
        message(STATUS "[${_tag}] ${MESSAGE}")
    else()
        message(STATUS "${MESSAGE}")
    endif()
endfunction()

# ============================================================================
# dbgspace - Output empty line
# ============================================================================
#[[
    dbgspace([ID <context_id>] [SWITCH <ON|OFF>])
    
    Outputs an empty line for visual structuring.
    
    Parameters:
        ID      - Optional: Context for switch check
        SWITCH  - Optional: Override switch
    
    Example:
        dbg(${DBG_OFTEN} "Section 1" ID MY_DBG)
        dbgspace(ID MY_DBG)
        dbg(${DBG_OFTEN} "Section 2" ID MY_DBG)
]]
function(dbgspace)
    if(NOT DEBUG_MESSAGES)
        return()
    endif()

    cmake_parse_arguments(ARG "" "ID;SWITCH" "" ${ARGN})

    set(_switch ON)
    if(ARG_ID)
        get_property(_has DIRECTORY PROPERTY DBG_${ARG_ID}_SWITCH SET)
        if(_has)
            get_property(_switch DIRECTORY PROPERTY DBG_${ARG_ID}_SWITCH)
        endif()
    endif()

    if(DEFINED ARG_SWITCH)
        set(_switch ${ARG_SWITCH})
    endif()

    if(_switch)
        message(STATUS "")
    endif()
endfunction()

# ============================================================================
# enddbgblock - Output separator line
# ============================================================================
#[[
    enddbgblock([ID <context_id>] [SWITCH <ON|OFF>])
    
    Outputs a separator line for visual block separation.
    
    Parameters:
        ID      - Optional: Context for switch check
        SWITCH  - Optional: Override switch
    
    Output:
        -- -------------------------------------------
    
    Example:
        dbg(${DBG_OFTEN} "=== Module Start ===" ID MY_DBG)
        # ... processing ...
        enddbgblock(ID MY_DBG)
]]
function(enddbgblock)
    if(NOT DEBUG_MESSAGES)
        return()
    endif()

    cmake_parse_arguments(ARG "" "ID;SWITCH" "" ${ARGN})

    set(_switch ON)
    if(ARG_ID)
        get_property(_has DIRECTORY PROPERTY DBG_${ARG_ID}_SWITCH SET)
        if(_has)
            get_property(_switch DIRECTORY PROPERTY DBG_${ARG_ID}_SWITCH)
        endif()
    endif()

    if(DEFINED ARG_SWITCH)
        set(_switch ${ARG_SWITCH})
    endif()

    if(_switch)
        message(STATUS "-------------------------------------------")
    endif()
endfunction()

# ============================================================================
# setup_debug_from_args - Helper for functions with debug flags
# ============================================================================
#[[
    setup_debug_from_args(<out_switch> <out_tag> <default_tag> [SHOW_DEBUG] [DEBUG_TAG <tag>])
    
    Helper function for functions that support optional SHOW_DEBUG/DEBUG_TAG.
    
    Parameters:
        out_switch  - Mandatory: Output variable for switch (ON/OFF)
        out_tag     - Mandatory: Output variable for tag
        default_tag - Mandatory: Default tag if not specified
        SHOW_DEBUG  - Optional: Flag to enable debug
        DEBUG_TAG   - Optional: Custom tag
    
    Example:
        function(my_function)
            cmake_parse_arguments(ARG "SHOW_DEBUG" "DEBUG_TAG" "" ${ARGN})
            setup_debug_from_args(_sw _tag "MyFunc" ${ARGN})
            
            dbg_init(ID MY_DBG SWITCH ${_sw} TAG "${_tag}")
            # ...
        endfunction()
        
        my_function(SHOW_DEBUG DEBUG_TAG "Custom")
]]
function(setup_debug_from_args OUT_SWITCH OUT_TAG DEFAULT_TAG)
    cmake_parse_arguments(ARG "SHOW_DEBUG" "DEBUG_TAG" "" ${ARGN})

    if(ARG_SHOW_DEBUG)
        set(${OUT_SWITCH} ON PARENT_SCOPE)
    else()
        set(${OUT_SWITCH} OFF PARENT_SCOPE)
    endif()

    if(DEFINED ARG_DEBUG_TAG)
        set(${OUT_TAG} "${ARG_DEBUG_TAG}" PARENT_SCOPE)
    else()
        set(${OUT_TAG} "${DEFAULT_TAG}" PARENT_SCOPE)
    endif()
endfunction()
