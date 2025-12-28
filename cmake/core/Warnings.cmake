# cmake/core/Warnings.cmake
# ==========================
# Compiler-specific warning level configuration
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - None (standalone module)
#
# Provides:
#   - apply_warnings(TARGET_NAME)
#
# Supported compilers:
#   - MSVC:      /W4
#   - GCC:       -Wall -Wextra -Wpedantic
#   - Clang:     -Wall -Wextra -Wpedantic
#
# Used by:
#   - ExecutableCreate.cmake
#   - LibraryCreate.cmake

include_guard(GLOBAL)

# ============================================================================
# apply_warnings - Set warning level for a target
# ============================================================================
#[[
    apply_warnings(TARGET_NAME)
    
    Sets high warning levels for a target.
    Compiler-specific: MSVC /W4, GCC/Clang -Wall -Wextra -Wpedantic.
    
    Parameters:
        TARGET_NAME - Mandatory: CMake target
    
    Example:
        add_executable(MyApp main.cpp)
        apply_warnings(MyApp)
    
    Note:
        For third-party code without warnings:
        target_compile_options(ThirdParty PRIVATE /W0)  # MSVC
        target_compile_options(ThirdParty PRIVATE -w)   # GCC/Clang
]]
function(apply_warnings TARGET_NAME)
    if(MSVC)
        # MSVC: /W4 (high warning level, recommended)
        target_compile_options(${TARGET_NAME} PRIVATE /W4)
        
        # Optional: Disable specific warnings
        # target_compile_options(${TARGET_NAME} PRIVATE /wd4996)  # deprecated
        # target_compile_options(${TARGET_NAME} PRIVATE /wd4100)  # unused param
    else()
        # GCC/Clang: -Wall -Wextra -Wpedantic
        target_compile_options(${TARGET_NAME} PRIVATE 
            -Wall 
            -Wextra 
            -Wpedantic
        )
        
        # Optional: Additional warnings
        # target_compile_options(${TARGET_NAME} PRIVATE 
        #     -Wshadow           # Variable shadows another
        #     -Wconversion       # Implicit conversions
        #     -Wnon-virtual-dtor # Non-virtual destructors
        # )
    endif()
endfunction()
