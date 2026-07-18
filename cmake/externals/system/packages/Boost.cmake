# cmake/externals/system/packages/Boost.cmake
# ============================================
# Boost-specific configuration for system externals
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# This hook is automatically loaded by system/Handler.cmake when
# processing a system external with package="Boost".
#
# Provides:
#   - _get_Boost_standard_paths(OUT_VAR) - Platform-specific Boost paths
#   - _Boost_post_find()                 - Called after find_package(Boost)
#   - _Boost_configure_target(TARGET)    - Configure target for Boost
#
# Used by:
#   - system/Handler.cmake

include_guard(GLOBAL)

# ==============================================================================
# _get_Boost_standard_paths - Platform-specific Boost installation paths
# ==============================================================================
function(_get_Boost_standard_paths OUT_VAR)
    set(_paths "")
    
    if(WIN32)
        # Common Windows Boost installation paths
        list(APPEND _paths
            "C:/local/boost_1_84_0"
            "C:/local/boost_1_83_0"
            "C:/local/boost_1_82_0"
            "C:/local/boost_1_81_0"
            "C:/Boost"
            "D:/Boost"
        )
    elseif(APPLE)
        # macOS: Homebrew and manual installations
        list(APPEND _paths
            "/opt/homebrew/opt/boost"
            "/usr/local/opt/boost"
            "/opt/local/include"
        )
    else()
        # Linux: System packages
        list(APPEND _paths
            "/usr/include/boost"
            "/usr/local/include/boost"
        )
    endif()
    
    set(${OUT_VAR} "${_paths}" PARENT_SCOPE)
endfunction()

# ==============================================================================
# _Boost_post_find - Called after find_package(Boost) succeeds
# ==============================================================================
function(_Boost_post_find)
    if(Boost_FOUND)
        dbg(${DBG_RARE} "  [Boost] Version: ${Boost_VERSION}" ID EXTERNALS)
        dbg(${DBG_RARE} "  [Boost] Include: ${Boost_INCLUDE_DIRS}" ID EXTERNALS)
    endif()
endfunction()

# ==============================================================================
# _Boost_configure_target - Configure a target for Boost
# ==============================================================================
function(_Boost_configure_target TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        return()
    endif()
    
    # Boost include directories are typically handled by target_link_libraries
    # but we can add additional configuration here if needed
    
    # Disable auto-linking on MSVC (we link explicitly)
    if(MSVC)
        target_compile_definitions(${TARGET_NAME} PRIVATE BOOST_ALL_NO_LIB)
    endif()
    
    dbg(${DBG_RARE} "  [Boost] Target configured: ${TARGET_NAME}" ID EXTERNALS)
endfunction()
