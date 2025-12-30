# cmake/externals/hooks/postfetch/qt-ads.cmake
# ==============================================
# PostFetch hook for Qt Advanced Docking System
#
# Version: 2.0.0
# Date:    2025-12-30
# Status:  Release
# Author:  CMake Architecture Team
#
# Hook Variables (from HookLoader):
#   - HOOK_EXTERNAL_NAME - Target name ("qt-ads")
#   - HOOK_SOURCE_DIR    - Path to qt-ads source
#
# Purpose:
#   Registers the qt-ads target after FetchContent_MakeAvailable().
#
# Supported Versions:
#   - 4.3.x: Target name "qt6advanceddocking"
#   - 4.4.x: Target name "qtadvanceddocking-qt6" (Breaking Change!)
#            Namespace alias: ads::qtadvanceddocking-qt6
#
# Note: Since v1.1.0, qt-ads is built as a STATIC library (BUILD_STATIC=ON in PreFetch).
#       This eliminates all DLL deployment issues - no POST_BUILD copy needed.

include_guard(GLOBAL)

message(STATUS "[qt-ads] PostFetch: Registering target")

# ==============================================================================
# Register Target - Support both 4.3.x and 4.4.x naming conventions
# ==============================================================================

# Qt-ADS 4.4.x naming convention (new)
if(TARGET qtadvanceddocking-qt6)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "qtadvanceddocking-qt6" PRIMARY)
    message(STATUS "[qt-ads] Registered: qtadvanceddocking-qt6 (v4.4.x, STATIC)")

# Qt-ADS 4.4.x namespace alias
elseif(TARGET ads::qtadvanceddocking-qt6)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "ads::qtadvanceddocking-qt6" PRIMARY)
    message(STATUS "[qt-ads] Registered: ads::qtadvanceddocking-qt6 (v4.4.x, STATIC)")

# Qt-ADS 4.3.x naming convention (legacy)
elseif(TARGET qt6advanceddocking)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "qt6advanceddocking" PRIMARY)
    message(STATUS "[qt-ads] Registered: qt6advanceddocking (v4.3.x, STATIC)")

# Qt5 fallback (if someone uses Qt5)
elseif(TARGET qtadvanceddocking-qt5)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "qtadvanceddocking-qt5" PRIMARY)
    message(STATUS "[qt-ads] Registered: qtadvanceddocking-qt5 (Qt5, STATIC)")

elseif(TARGET qtadvanceddocking)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "qtadvanceddocking" PRIMARY)
    message(STATUS "[qt-ads] Registered: qtadvanceddocking (legacy, STATIC)")

else()
    # Debug: List all available targets for troubleshooting
    message(WARNING "[qt-ads] No known target found!")
    message(STATUS "[qt-ads] Checking for targets containing 'ads' or 'docking'...")
    
    # Get all targets in current directory scope
    get_property(_all_targets DIRECTORY "${HOOK_SOURCE_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_target IN LISTS _all_targets)
        string(TOLOWER "${_target}" _target_lower)
        if(_target_lower MATCHES "ads|docking|advanceddocking")
            message(STATUS "[qt-ads]   Found potential target: ${_target}")
        endif()
    endforeach()
    
    message(WARNING "[qt-ads] Expected one of: qtadvanceddocking-qt6, qt6advanceddocking, qtadvanceddocking")
endif()

# Note: No POST_LINK hook needed - static library is linked directly into executable
# No DLL to copy = no windeployqt issues!

message(STATUS "[qt-ads] PostFetch complete")
