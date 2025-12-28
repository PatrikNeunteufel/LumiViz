# cmake/core/CompilerOptions.cmake
# =================================
# Compiler-specific options and code quality tools
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - Debug.cmake (dbg_init, dbg, enddbgblock)
#   - Errors.cmake (cmake_warn)
#
# Provides:
#   - apply_compiler_options(TARGET [FLAGS...])
#
# Features:
#   - Precise compiler detection (MSVC, GCC, Clang, Apple Clang)
#   - Clang-Tidy integration
#   - NO_EXCEPTIONS, NO_RTTI support
#   - Per-target overrides
#
# Cache variables:
#   - ENABLE_CLANG_TIDY           (ON/OFF, Default: OFF)
#   - ENABLE_CLANG_FORMAT_CHECK   (ON/OFF, Default: OFF)
#   - NO_EXCEPTIONS               (ON/OFF, Default: OFF)
#   - NO_RTTI                     (ON/OFF, Default: OFF)
#   - ENABLE_STRICT_CONFORMANCE   (ON/OFF, Default: ON)
#   - CLANG_TIDY_STRICT           (ON/OFF, Default: OFF)

include_guard(GLOBAL)

# ============================================================================
# Debug system for this module
# ============================================================================

set(_SHOW_COMPILEROPTION_INFO OFF)
dbg_init(
    ID COMPILEROPTION_INFO_MSG 
    LEVEL ${DBG_SHOW_MUCH} 
    SWITCH ${_SHOW_COMPILEROPTION_INFO} 
    TAG "COMPILER"
)

dbg(${DBG_OFTEN} "=== Compiler Options Module ===" ID COMPILEROPTION_INFO_MSG)

# ============================================================================
# apply_compiler_options - Main function
# ============================================================================
#[[
    apply_compiler_options(TARGET_NAME [FLAGS...])
    
    Applies compiler options and code quality tools to a target.
    
    Parameters:
        TARGET_NAME             - Mandatory: CMake target
        SKIP_STRICT_CONFORMANCE - Optional: Don't set MSVC /permissive-
        SKIP_NOMINMAX           - Optional: Don't define NOMINMAX
        FORCE_EXCEPTIONS        - Optional: Enable exceptions (ignores NO_EXCEPTIONS)
        FORCE_RTTI              - Optional: Enable RTTI (ignores NO_RTTI)
        SKIP_CLANG_TIDY         - Optional: Skip clang-tidy for this target
        SHOW_DEBUG              - Optional: Enable debug output
        DEBUG_TAG <tag>         - Optional: Custom debug tag
    
    Cache variables (global):
        ENABLE_CLANG_TIDY       - Enable clang-tidy
        NO_EXCEPTIONS           - Disable exceptions globally
        NO_RTTI                 - Disable RTTI globally
        ENABLE_STRICT_CONFORMANCE - MSVC strict conformance
    
    Errors:
        W201 - Clang-tidy enabled but not found
    
    Example:
        apply_compiler_options(MyApp)
        apply_compiler_options(LegacyApp SKIP_STRICT_CONFORMANCE FORCE_EXCEPTIONS)
]]
function(apply_compiler_options TARGET_NAME)
    # Argument parsing
    cmake_parse_arguments(
        ARG
        "SKIP_STRICT_CONFORMANCE;SKIP_NOMINMAX;FORCE_EXCEPTIONS;FORCE_RTTI;SKIP_CLANG_TIDY;SHOW_DEBUG"
        "DEBUG_TAG"
        ""
        ${ARGN}
    )
    
    # Debug setup
    if(ARG_SHOW_DEBUG)
        set(_sw ON)
        if(ARG_DEBUG_TAG)
            set(_tag "${ARG_DEBUG_TAG}")
        else()
            set(_tag "CompilerOpts")
        endif()
    else()
        set(_sw OFF)
        set(_tag "CompilerOpts")
    endif()
    
    dbg_init(ID CO_DBG LEVEL ${DBG_SHOW_MUCH} SWITCH ${_sw} TAG "${_tag}")
    dbg(${DBG_OFTEN} "Applying compiler options to: ${TARGET_NAME}" ID CO_DBG)
    
    # =========================================================================
    # 1. Platform and compiler detection
    # =========================================================================
    
    set(_is_msvc FALSE)
    set(_is_gcc FALSE)
    set(_is_clang FALSE)
    set(_is_apple_clang FALSE)
    
    if(MSVC)
        set(_is_msvc TRUE)
        dbg(${DBG_NORMAL} "  Platform: Windows (MSVC)" ID CO_DBG)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(_is_gcc TRUE)
        if(WIN32)
            dbg(${DBG_NORMAL} "  Platform: Windows (MinGW GCC)" ID CO_DBG)
        elseif(APPLE)
            dbg(${DBG_NORMAL} "  Platform: macOS (GCC)" ID CO_DBG)
        else()
            dbg(${DBG_NORMAL} "  Platform: Linux (GCC)" ID CO_DBG)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(_is_clang TRUE)
        if(APPLE)
            set(_is_apple_clang TRUE)
            dbg(${DBG_NORMAL} "  Platform: macOS (Apple Clang)" ID CO_DBG)
        elseif(WIN32)
            dbg(${DBG_NORMAL} "  Platform: Windows (Clang-CL)" ID CO_DBG)
        else()
            dbg(${DBG_NORMAL} "  Platform: Linux (Clang)" ID CO_DBG)
        endif()
    else()
        dbg(${DBG_NORMAL} "  Platform: Unknown (Compiler: ${CMAKE_CXX_COMPILER_ID})" ID CO_DBG)
    endif()
    
    # =========================================================================
    # 2. MSVC-specific options
    # =========================================================================
    
    if(_is_msvc)
        # Strict conformance (optionally disableable)
        if(NOT ARG_SKIP_STRICT_CONFORMANCE AND NOT DEFINED ENABLE_STRICT_CONFORMANCE OR ENABLE_STRICT_CONFORMANCE)
            target_compile_options(${TARGET_NAME} PRIVATE 
                /permissive-        # Strict standard conformance
                /Zc:preprocessor    # Standard-conformant preprocessor
                /Zc:__cplusplus     # Correct __cplusplus value
            )
            dbg(${DBG_NORMAL} "  Strict Conformance: ENABLED" ID CO_DBG)
        else()
            dbg(${DBG_NORMAL} "  Strict Conformance: SKIPPED" ID CO_DBG)
        endif()
        
        # Windows-specific definitions (optionally disableable)
        if(NOT ARG_SKIP_NOMINMAX)
            target_compile_definitions(${TARGET_NAME} PRIVATE 
                NOMINMAX            # Disable min/max macros
            )
            dbg(${DBG_RARE} "  NOMINMAX: DEFINED" ID CO_DBG)
        else()
            dbg(${DBG_RARE} "  NOMINMAX: SKIPPED" ID CO_DBG)
        endif()
        
        # Exceptions (respects FORCE_EXCEPTIONS)
        if(NO_EXCEPTIONS AND NOT ARG_FORCE_EXCEPTIONS)
            target_compile_options(${TARGET_NAME} PRIVATE /EHs-c-)
            dbg(${DBG_NORMAL} "  Exceptions: DISABLED" ID CO_DBG)
        else()
            dbg(${DBG_RARE} "  Exceptions: enabled" ID CO_DBG)
        endif()
        
        # RTTI (respects FORCE_RTTI)
        if(NO_RTTI AND NOT ARG_FORCE_RTTI)
            target_compile_options(${TARGET_NAME} PRIVATE /GR-)
            dbg(${DBG_NORMAL} "  RTTI: DISABLED" ID CO_DBG)
        else()
            dbg(${DBG_RARE} "  RTTI: enabled" ID CO_DBG)
        endif()
        
    # =========================================================================
    # 3. GCC/Clang-specific options
    # =========================================================================
    
    elseif(_is_gcc OR _is_clang)
        # Exceptions (respects FORCE_EXCEPTIONS)
        if(NO_EXCEPTIONS AND NOT ARG_FORCE_EXCEPTIONS)
            target_compile_options(${TARGET_NAME} PRIVATE -fno-exceptions)
            dbg(${DBG_NORMAL} "  Exceptions: DISABLED" ID CO_DBG)
        else()
            dbg(${DBG_RARE} "  Exceptions: enabled" ID CO_DBG)
        endif()
        
        # RTTI (respects FORCE_RTTI)
        if(NO_RTTI AND NOT ARG_FORCE_RTTI)
            target_compile_options(${TARGET_NAME} PRIVATE -fno-rtti)
            dbg(${DBG_NORMAL} "  RTTI: DISABLED" ID CO_DBG)
        else()
            dbg(${DBG_RARE} "  RTTI: enabled" ID CO_DBG)
        endif()
        
        # Apple-specific options
        if(_is_apple_clang)
            dbg(${DBG_ULTRA_RARE} "  Apple Clang: Standard flags" ID CO_DBG)
        endif()
    endif()
    
    # =========================================================================
    # 4. Clang-Tidy integration
    # =========================================================================
    
    if(ENABLE_CLANG_TIDY AND NOT ARG_SKIP_CLANG_TIDY)
        dbg(${DBG_OFTEN} "  Clang-Tidy: Checking..." ID CO_DBG)
        
        find_program(CLANG_TIDY_EXE NAMES clang-tidy)
        
        if(CLANG_TIDY_EXE)
            # Use clang-tidy config file
            set(_clang_tidy_config "${CMAKE_SOURCE_DIR}/.clang-tidy")
            
            if(EXISTS "${_clang_tidy_config}")
                # With config file
                set_target_properties(${TARGET_NAME} PROPERTIES
                    CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--config-file=${_clang_tidy_config}"
                )
                dbg(${DBG_OFTEN} "  Clang-Tidy: ENABLED (config: .clang-tidy)" ID CO_DBG)
                message(STATUS "[${TARGET_NAME}] Clang-Tidy enabled")
            else()
                # Without config file (standard checks)
                set_target_properties(${TARGET_NAME} PROPERTIES
                    CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
                )
                dbg(${DBG_OFTEN} "  Clang-Tidy: ENABLED (default checks)" ID CO_DBG)
                message(STATUS "[${TARGET_NAME}] Clang-Tidy enabled (without .clang-tidy)")
            endif()
            
            # Optional: Strict mode for CI
            if(CLANG_TIDY_STRICT)
                set_target_properties(${TARGET_NAME} PROPERTIES
                    CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--config-file=${_clang_tidy_config};-warnings-as-errors=clang-analyzer-*,bugprone-*"
                )
                dbg(${DBG_OFTEN} "  Clang-Tidy: STRICT MODE (warnings as errors)" ID CO_DBG)
                message(STATUS "[${TARGET_NAME}] Clang-Tidy STRICT mode")
            endif()
            
        else()
            # Clang-Tidy not found
            cmake_warn("W201" "ENABLE_CLANG_TIDY=ON but clang-tidy not found for target '${TARGET_NAME}'")
            dbg(${DBG_OFTEN} "  Clang-Tidy: NOT FOUND" ID CO_DBG)
        endif()
        
    elseif(ARG_SKIP_CLANG_TIDY)
        dbg(${DBG_NORMAL} "  Clang-Tidy: skipped (SKIP_CLANG_TIDY)" ID CO_DBG)
    else()
        dbg(${DBG_RARE} "  Clang-Tidy: disabled" ID CO_DBG)
    endif()
    
    # =========================================================================
    # 5. Clang-Format info
    # =========================================================================
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/.clang-format")
        dbg(${DBG_RARE} "  Clang-Format: .clang-format found (IDE will use it)" ID CO_DBG)
        
        # Optional: Format check target for CI/CD
        if(ENABLE_CLANG_FORMAT_CHECK AND NOT ARG_SKIP_CLANG_TIDY)
            dbg(${DBG_NORMAL} "  Clang-Format Check: Planned for Phase 7" ID CO_DBG)
            # TODO Phase 7: _add_clang_format_check_target(${TARGET_NAME})
        endif()
    endif()
    
    dbg(${DBG_OFTEN} "Compiler options applied to: ${TARGET_NAME}" ID CO_DBG)
    enddbgblock(ID CO_DBG)
    
endfunction()

# ============================================================================
# _add_clang_format_check_target - Clang-Format check target (Phase 7)
# ============================================================================
#[[
    _add_clang_format_check_target(TARGET_NAME)
    
    Creates custom target for format checks - Phase 7 placeholder.
    
    Parameters:
        TARGET_NAME - Mandatory: CMake target
]]
function(_add_clang_format_check_target TARGET_NAME)
    # TODO Phase 7: Implement format check
    #
    # Idea:
    #   1. Collect all source files of the target
    #   2. Create custom target: ${TARGET_NAME}_format_check
    #   3. clang-format --dry-run --Werror on all files
    #   4. Integrate into CTest
    
    message(STATUS "[${TARGET_NAME}] Clang-Format check target (Phase 7 - not yet implemented)")
endfunction()

# ============================================================================
# Module end
# ============================================================================

dbg(${DBG_OFTEN} "Compiler Options module loaded" ID COMPILEROPTION_INFO_MSG)
enddbgblock(ID COMPILEROPTION_INFO_MSG)
