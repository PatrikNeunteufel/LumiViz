# cmake/externals/hooks/prefetch/qt-ads.cmake
# =============================================
# PreFetch hook for Qt Advanced Docking System
#
# Version: 1.1.0
# Date:    2025-12-27
# Status:  Release
# Author:  CMake Architecture Team
#
# Hook Variables (from HookLoader):
#   - HOOK_EXTERNAL_NAME - Target name ("qt-ads")
#
# Purpose:
#   Sets CMake cache variables BEFORE FetchContent_MakeAvailable()
#   to configure qt-ads build options.
#
# Key Decision: BUILD AS STATIC LIBRARY
#   Building qt-ads as a static library eliminates all DLL deployment
#   issues with windeployqt. When qt-ads is a shared library (DLL),
#   windeployqt tries to analyze it and fails because it's not in the
#   Qt installation directory. A static library is linked directly
#   into the executable - no DLL copying needed.
#
# IMPORTANT: The variable is BUILD_STATIC, NOT ADS_BUILD_STATIC!

include_guard(GLOBAL)

message(STATUS "[qt-ads] PreFetch: Configuring build options")

# ==============================================================================
# Build Options
# ==============================================================================

# Build as STATIC library - this is the key fix!
# IMPORTANT: The variable name is BUILD_STATIC (not ADS_BUILD_STATIC)
# See: https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System/blob/master/src/CMakeLists.txt
set(BUILD_STATIC ON CACHE BOOL "Build qt-ads as static library" FORCE)

# Disable examples - we don't need them
set(BUILD_EXAMPLES OFF CACHE BOOL "Don't build qt-ads examples" FORCE)

message(STATUS "[qt-ads]   BUILD_STATIC: ON (static library)")
message(STATUS "[qt-ads]   BUILD_EXAMPLES: OFF")
message(STATUS "[qt-ads] PreFetch complete")
