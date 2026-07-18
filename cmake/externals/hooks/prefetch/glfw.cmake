# cmake/externals/hooks/prefetch/glfw.cmake
# ==========================================
# PreFetch hook for GLFW - disables examples, tests, and documentation
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Hook Variables (from HookLoader):
#   - HOOK_EXTERNAL_NAME - Name of the external
#   - HOOK_EXTERNAL_JSON - JSON definition
#
# Sets:
#   - GLFW_BUILD_EXAMPLES OFF
#   - GLFW_BUILD_TESTS OFF
#   - GLFW_BUILD_DOCS OFF
#   - GLFW_INSTALL OFF

message(STATUS "[glfw] PreFetch hook: Setting options")

# ==============================================================================
# GLFW Configuration Options
# ==============================================================================

# Disable building examples
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Disable building tests
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Disable building documentation
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

# Install is not needed when using FetchContent
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

message(STATUS "[glfw] PreFetch hook complete")
