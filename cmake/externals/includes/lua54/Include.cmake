# cmake/externals/includes/lua54/Include.cmake
# =============================================
# Lua 5.4 scripting engine integration - supports embedded and dynamic linking
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
# Type:    Local External Include
#
# Dependencies:
#   - cmake/core/Json.cmake (_json_get_bool_from_key)
#
# Expected Variables (set by Orchestrator):
#   - EXTERNAL_NAME    - "lua54"
#   - EXTERNAL_ROOT    - Path to externals/lua54 (resources)
#   - EXTERNAL_OPTIONS - JSON string with options
#   - EXECUTABLE_NAME  - Target to attach to
#
# Available Options (via external_options in Solution.json):
#   - LUA_EMBEDDED     - Use static library (default: true)
#   - LUA_32BIT_COMPAT - Enable 32-bit integer compatibility
#   - LUA_USE_READLINE - Enable readline support (Linux only)
#
# Resources (externals/lua54/):
#   - win/include/    (lua.h, lualib.h, lauxlib.h, luaconf.h)
#   - win/lib/        (lua54.lib)
#   - win/bin/        (lua54.dll)
#   - linux/lib/      (liblua54.a, liblua54.so)
#
# Used by:
#   - Orchestrator.cmake (via apply_external_to_target)

# Note: No include_guard() - this file is included per-target intentionally

# ==============================================================================
# Variable Compatibility
# ==============================================================================

# Support both new and legacy variable names
if(NOT DEFINED EXTERNAL_OPTIONS AND DEFINED EXTERNAL_ELEMENT_OPTIONS)
    set(EXTERNAL_OPTIONS "${EXTERNAL_ELEMENT_OPTIONS}")
endif()
if(NOT DEFINED EXTERNAL_NAME AND DEFINED EXTERNAL_ELEMENT_NAME)
    set(EXTERNAL_NAME "${EXTERNAL_ELEMENT_NAME}")
endif()
if(NOT DEFINED EXTERNAL_ROOT)
    set(EXTERNAL_ROOT "${CMAKE_SOURCE_DIR}/externals/lua54")
endif()

# ==============================================================================
# Debug Output
# ==============================================================================

message(STATUS "[${EXTERNAL_NAME}] Attaching to ${EXECUTABLE_NAME}")

# ==============================================================================
# Path Configuration
# ==============================================================================

set(_lua_root "${EXTERNAL_ROOT}")

# ==============================================================================
# Process Options
# ==============================================================================

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "LUA_EMBEDDED" _lua_embedded)
_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "LUA_32BIT_COMPAT" _lua_32bit)
_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "LUA_USE_READLINE" _lua_readline)

# Default: Embedded = true (recommended for embedded applications)
if(NOT _lua_embedded AND NOT DEFINED _lua_embedded)
    set(_lua_embedded TRUE)
endif()

# ==============================================================================
# Platform-Specific Integration
# ==============================================================================

if(WIN32)
    # -------------------------------------------------------------------------
    # Windows
    # -------------------------------------------------------------------------
    
    target_include_directories(${EXECUTABLE_NAME} PRIVATE
        "${_lua_root}/win/include"
    )
    
    if(_lua_embedded)
        # Static library (recommended)
        message(STATUS "[${EXTERNAL_NAME}]   Mode: EMBEDDED (static)")
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
            "${_lua_root}/win/lib/lua54.lib"
        )
    else()
        # Dynamic library
        message(STATUS "[${EXTERNAL_NAME}]   Mode: DYNAMIC (shared library)")
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
            "${_lua_root}/win/lib/lua54.lib"
        )
        # Copy DLL to output directory
        add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_lua_root}/win/bin/lua54.dll"
                $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>
            COMMENT "[LUA] Copying lua54.dll"
        )
    endif()
    
elseif(APPLE)
    # -------------------------------------------------------------------------
    # macOS
    # -------------------------------------------------------------------------
    
    target_include_directories(${EXECUTABLE_NAME} PRIVATE
        "${_lua_root}/win/include"
    )
    
    message(STATUS "[${EXTERNAL_NAME}]   Mode: System Lua (macOS)")
    message(WARNING "[${EXTERNAL_NAME}] macOS: Using system Lua or compile from sources")
    # On macOS typically: brew install lua
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE lua)
    
    # Platform definition
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE LUA_USE_MACOSX)
    
elseif(UNIX)
    # -------------------------------------------------------------------------
    # Linux
    # -------------------------------------------------------------------------
    
    target_include_directories(${EXECUTABLE_NAME} PRIVATE
        "${_lua_root}/win/include"
    )
    
    if(_lua_embedded)
        # Static library (recommended)
        message(STATUS "[${EXTERNAL_NAME}]   Mode: EMBEDDED (static)")
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
            "${_lua_root}/linux/lib/liblua54.a"
        )
    else()
        # Shared library
        message(STATUS "[${EXTERNAL_NAME}]   Mode: DYNAMIC (shared library)")
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
            "${_lua_root}/linux/lib/liblua54.so"
        )
    endif()
    
    # Linux requires additional system libraries
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE dl m)
    
    # Platform definition
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE LUA_USE_LINUX)
    
    # Readline support
    if(_lua_readline)
        target_compile_definitions(${EXECUTABLE_NAME} PRIVATE LUA_USE_READLINE)
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE readline)
        message(STATUS "[${EXTERNAL_NAME}]   Readline: ENABLED")
    endif()
    
else()
    message(WARNING "[${EXTERNAL_NAME}] Unsupported platform")
    return()
endif()

# ==============================================================================
# Additional Compile Definitions
# ==============================================================================

# 32-bit integer compatibility
if(_lua_32bit)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE LUA_32BITS)
    message(STATUS "[${EXTERNAL_NAME}]   32-Bit Compat: ENABLED")
endif()

# ==============================================================================
# Complete
# ==============================================================================

message(STATUS "[${EXTERNAL_NAME}] Integration complete")
