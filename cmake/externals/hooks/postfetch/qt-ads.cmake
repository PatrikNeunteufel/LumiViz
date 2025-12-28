# cmake/externals/hooks/postfetch/qt-ads.cmake
# ==============================================
# PostFetch hook for Qt Advanced Docking System
#
# Version: 1.1.0
# Date:    2025-12-27
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
# Note: Since v1.1.0, qt-ads is built as a STATIC library (BUILD_STATIC=ON in PreFetch).
#       This eliminates all DLL deployment issues - no POST_BUILD copy needed.

include_guard(GLOBAL)

message(STATUS "[qt-ads] PostFetch: Registering target")

# ==============================================================================
# Register Target
# ==============================================================================

if(TARGET qt6advanceddocking)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "qt6advanceddocking" PRIMARY)
    message(STATUS "[qt-ads] Registered: qt6advanceddocking (STATIC)")
    
elseif(TARGET qtadvanceddocking)
    _register_external_target("${HOOK_EXTERNAL_NAME}" "qtadvanceddocking" PRIMARY)
    message(STATUS "[qt-ads] Registered: qtadvanceddocking (STATIC)")
    
else()
    message(WARNING "[qt-ads] No target found (qt6advanceddocking or qtadvanceddocking)")
endif()

# Note: No POST_LINK hook needed - static library is linked directly into executable
# No DLL to copy = no windeployqt issues!

message(STATUS "[qt-ads] PostFetch complete")
