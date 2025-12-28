# cmake/externals/hooks/postfetch/stb.cmake
# ==========================================
# PostFetch hook for stb - Header-Only Library (no CMakeLists.txt)
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Hook Variables (from HookLoader):
#   - HOOK_EXTERNAL_NAME - Target name ("stb")
#   - HOOK_SOURCE_DIR    - Path to stb source
#
# Creates:
#   - stb INTERFACE target with include directory

include_guard(GLOBAL)

message(STATUS "[${HOOK_EXTERNAL_NAME}] PostFetch: Creating INTERFACE target")
message(STATUS "[${HOOK_EXTERNAL_NAME}]   Source: ${HOOK_SOURCE_DIR}")

# ==============================================================================
# Create INTERFACE Library (Header-Only)
# ==============================================================================

add_library(${HOOK_EXTERNAL_NAME} INTERFACE)

target_include_directories(${HOOK_EXTERNAL_NAME} INTERFACE
    "${HOOK_SOURCE_DIR}"
)

# ==============================================================================
# Register Target
# ==============================================================================

_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)

message(STATUS "[${HOOK_EXTERNAL_NAME}] INTERFACE target created")
message(STATUS "[${HOOK_EXTERNAL_NAME}] PostFetch complete")
