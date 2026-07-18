# cmake/externals/system/packages/Qt6.cmake
# ==========================================
# Qt6-specific configuration for system externals
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# This hook is automatically loaded by system/Handler.cmake when
# processing a system external with package="Qt6".
#
# Provides:
#   - _get_Qt6_standard_paths(OUT_VAR) - Platform-specific Qt installation paths
#   - _Qt6_post_find()                 - Called after find_package(Qt6)
#   - _Qt6_configure_target(TARGET)    - Configure target (AUTOMOC etc.)
#   - _Qt6_configure_deployment(TARGET) - Setup deployment (windeployqt etc.)
#
# Used by:
#   - system/Handler.cmake
#
# Changelog:
#   v1.0.0 (2025-12-26): Release version
#   v0.6.1 (2025-12-21): Fix: Skip windeployqt for STATIC_LIBRARY targets
#   v0.6.0 (2025-12-18): Initial version

include_guard(GLOBAL)

# ==============================================================================
# _get_Qt6_standard_paths - Platform-specific Qt installation paths
# ==============================================================================
#[[
    _get_Qt6_standard_paths(OUT_VAR)
    
    Returns a list of common Qt6 installation paths for the current platform.
    These are checked after environment variables and hints.
    
    Parameters:
        OUT_VAR - Output variable for path list
]]
function(_get_Qt6_standard_paths OUT_VAR)
    set(_paths "")
    
    if(WIN32)
        # Qt Online Installer default locations (newest first)
        list(APPEND _paths
            "C:/Qt/6.10.1/msvc2022_64"
            "C:/Qt/6.10.0/msvc2022_64"
            "C:/Qt/6.9.0/msvc2022_64"
            "C:/Qt/6.8.1/msvc2022_64"
            "C:/Qt/6.8.0/msvc2022_64"
            "C:/Qt/6.7.3/msvc2022_64"
            "C:/Qt/6.7.2/msvc2022_64"
            "C:/Qt/6.7.0/msvc2022_64"
            "C:/Qt/6.6.0/msvc2022_64"
            "C:/Qt/6.5.3/msvc2022_64"
            "C:/Qt/6.5.0/msvc2022_64"
            # Alternative drives
            "D:/Qt/6.10.1/msvc2022_64"
            "D:/Qt/6.10.0/msvc2022_64"
            "D:/Qt/6.8.0/msvc2022_64"
            "D:/Qt/6.7.0/msvc2022_64"
            "E:/Qt/6.10.1/msvc2022_64"
            "E:/Qt/6.8.0/msvc2022_64"
            "I:/Qt/6.10.1/msvc2022_64"
            "I:/Qt/6.8.0/msvc2022_64"
        )
    elseif(APPLE)
        # macOS: Qt Online Installer and Homebrew
        list(APPEND _paths
            "$ENV{HOME}/Qt/6.8.0/macos"
            "$ENV{HOME}/Qt/6.7.0/macos"
            "$ENV{HOME}/Qt/6.6.0/macos"
            "/opt/homebrew/opt/qt@6"
            "/usr/local/opt/qt@6"
        )
    else()
        # Linux: Qt Online Installer and system packages
        list(APPEND _paths
            "$ENV{HOME}/Qt/6.8.0/gcc_64"
            "$ENV{HOME}/Qt/6.7.0/gcc_64"
            "$ENV{HOME}/Qt/6.6.0/gcc_64"
            "/opt/Qt/6.8.0/gcc_64"
            "/opt/Qt/6.7.0/gcc_64"
            "/usr/lib/qt6"
            "/usr/lib/x86_64-linux-gnu/qt6"
        )
    endif()
    
    set(${OUT_VAR} "${_paths}" PARENT_SCOPE)
endfunction()

# ==============================================================================
# _Qt6_post_find - Called after find_package(Qt6) succeeds
# ==============================================================================
#[[
    _Qt6_post_find()
    
    Performs Qt6-specific setup after find_package() succeeds.
    Currently stores Qt prefix for deployment tools.
]]
function(_Qt6_post_find)
    # Store Qt prefix for deployment
    get_target_property(_qt_location Qt6::Core LOCATION)
    if(_qt_location)
        get_filename_component(_qt_bin_dir "${_qt_location}" DIRECTORY)
        get_filename_component(_qt_prefix "${_qt_bin_dir}" DIRECTORY)
        set_property(GLOBAL PROPERTY QT6_PREFIX "${_qt_prefix}")
        dbg(${DBG_RARE} "  [Qt6] Prefix stored: ${_qt_prefix}" ID EXTERNALS)
    endif()
endfunction()

# ==============================================================================
# _Qt6_configure_target - Configure a target for Qt6
# ==============================================================================
#[[
    _Qt6_configure_target(TARGET_NAME)
    
    Configures a CMake target for Qt6 usage.
    Enables AUTOMOC, AUTOUIC, AUTORCC.
    
    Parameters:
        TARGET_NAME - CMake target to configure
]]
function(_Qt6_configure_target TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        return()
    endif()
    
    # Enable Qt's automatic MOC/UIC/RCC
    set_target_properties(${TARGET_NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
    
    dbg(${DBG_RARE} "  [Qt6] AUTOMOC/AUTOUIC/AUTORCC enabled for ${TARGET_NAME}" ID EXTERNALS)
    
    # Configure deployment
    _Qt6_configure_deployment(${TARGET_NAME})
endfunction()

# ==============================================================================
# _Qt6_configure_deployment - Setup Qt deployment for a target
# ==============================================================================
#[[
    _Qt6_configure_deployment(TARGET_NAME)
    
    Configures platform-specific Qt deployment (DLL copying, RPATH, etc.)
    
    NOTE: Deployment is skipped for STATIC_LIBRARY targets, as windeployqt
          and macdeployqt only work with executables and shared libraries.
    
    Parameters:
        TARGET_NAME - CMake target to configure deployment for
]]
function(_Qt6_configure_deployment TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        return()
    endif()
    
    # =========================================================================
    # Check target type - skip deployment for static libraries
    # =========================================================================
    get_target_property(_target_type ${TARGET_NAME} TYPE)
    if(_target_type STREQUAL "STATIC_LIBRARY")
        dbg(${DBG_RARE} "  [Qt6] Skipping deployment for static library ${TARGET_NAME}" ID EXTERNALS)
        return()
    endif()
    
    # Also skip for INTERFACE and OBJECT libraries
    if(_target_type STREQUAL "INTERFACE_LIBRARY" OR _target_type STREQUAL "OBJECT_LIBRARY")
        dbg(${DBG_RARE} "  [Qt6] Skipping deployment for ${_target_type} ${TARGET_NAME}" ID EXTERNALS)
        return()
    endif()
    
    # =========================================================================
    # Get Qt prefix
    # =========================================================================
    get_property(_qt_prefix GLOBAL PROPERTY QT6_PREFIX)
    if("${_qt_prefix}" STREQUAL "")
        # Try to get from Qt6::Core
        get_target_property(_qt_location Qt6::Core LOCATION)
        if(_qt_location)
            get_filename_component(_qt_bin_dir "${_qt_location}" DIRECTORY)
            get_filename_component(_qt_prefix "${_qt_bin_dir}" DIRECTORY)
        endif()
    endif()
    
    if(WIN32)
        # =====================================================================
        # Windows: Use windeployqt
        # =====================================================================
        
        find_program(_WINDEPLOYQT windeployqt 
            HINTS "${_qt_prefix}/bin" "${Qt6_DIR}/../../../bin"
        )
        
        if(_WINDEPLOYQT)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND "${_WINDEPLOYQT}"
                    --no-translations
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    "$<TARGET_FILE:${TARGET_NAME}>"
                COMMENT "[Qt6] Running windeployqt for ${TARGET_NAME}..."
                VERBATIM
            )
            dbg(${DBG_RARE} "  [Qt6] windeployqt configured for ${TARGET_NAME}" ID EXTERNALS)
        else()
            dbg(${DBG_RARE} "  [Qt6] windeployqt not found, skipping auto-deployment" ID EXTERNALS)
        endif()
        
    elseif(APPLE)
        # =====================================================================
        # macOS: Configure RPATH and optionally macdeployqt
        # =====================================================================
        
        set_target_properties(${TARGET_NAME} PROPERTIES
            INSTALL_RPATH "@executable_path/../lib;${_qt_prefix}/lib"
            BUILD_RPATH "${_qt_prefix}/lib"
        )
        
        # macdeployqt only for bundles
        get_target_property(_is_bundle ${TARGET_NAME} MACOSX_BUNDLE)
        if(_is_bundle)
            find_program(_MACDEPLOYQT macdeployqt 
                HINTS "${_qt_prefix}/bin" "${Qt6_DIR}/../../../bin"
            )
            if(_MACDEPLOYQT)
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                    COMMAND "${_MACDEPLOYQT}"
                        "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>"
                        -always-overwrite
                    COMMENT "[Qt6] Running macdeployqt for ${TARGET_NAME}..."
                    VERBATIM
                )
            endif()
        endif()
        
        dbg(${DBG_RARE} "  [Qt6] macOS RPATH configured for ${TARGET_NAME}" ID EXTERNALS)
        
    else()
        # =====================================================================
        # Linux: Configure RPATH
        # =====================================================================
        
        set_target_properties(${TARGET_NAME} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib;${_qt_prefix}/lib"
            BUILD_RPATH "${_qt_prefix}/lib"
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
        
        dbg(${DBG_RARE} "  [Qt6] Linux RPATH configured for ${TARGET_NAME}" ID EXTERNALS)
    endif()
    
endfunction()
