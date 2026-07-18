# cmake/externals/hooks/prefetch/googletest.cmake
# ================================================
# PreFetch hook for GoogleTest - configures build and fixes Windows CRT
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
# Target Mapping:
#   - HOOK_KNOWN_TARGETS: gtest, gtest_main, gmock, gmock_main
#   - HOOK_PRIMARY_TARGET: gmock_main
#
# Sets:
#   - BUILD_GMOCK ON
#   - INSTALL_GTEST OFF
#   - gtest_force_shared_crt ON (Windows only)

message(STATUS "[${HOOK_EXTERNAL_NAME}] PreFetch: Configuring GoogleTest")

# ==============================================================================
# Target Mapping for Auto-Registration
# ==============================================================================
# GoogleTest creates targets with different names than the external name.
# We tell the registry which targets to look for.
#
# Note: When using gtest_main, GMock includes are automatically available
# because gtest_main links gmock internally.

set_property(GLOBAL PROPERTY HOOK_KNOWN_TARGETS_${HOOK_EXTERNAL_NAME}
    "gtest;gtest_main;gmock;gmock_main"
)
# Use gmock_main as primary - includes both gtest AND gmock
set_property(GLOBAL PROPERTY HOOK_PRIMARY_TARGET_${HOOK_EXTERNAL_NAME}
    "gmock_main"
)

# ==============================================================================
# Build Configuration
# ==============================================================================

# Enable Google Mock (includes Google Test)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)

# Disable installation (not needed with FetchContent)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# ==============================================================================
# Windows CRT Fix (WICHTIG!)
# ==============================================================================
#
# Ohne diese Option gibt es auf Windows Linker-Fehler wie:
#   "LNK2038: mismatch detected for 'RuntimeLibrary'"
#
# Grund: GoogleTest kompiliert standardmäßig mit statischer CRT (/MT),
# aber die meisten Projekte verwenden dynamische CRT (/MD).
#

if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL 
        "Use shared (DLL) run-time lib even when Google Test is built as static lib." 
        FORCE
    )
    message(STATUS "[${HOOK_EXTERNAL_NAME}]   Windows: gtest_force_shared_crt=ON")
endif()

# ==============================================================================
# Optional: Hide internal symbols
# ==============================================================================

set(gtest_hide_internal_symbols ON CACHE BOOL "" FORCE)

message(STATUS "[${HOOK_EXTERNAL_NAME}] PreFetch complete")
message(STATUS "[${HOOK_EXTERNAL_NAME}]   Targets: gtest, gtest_main, gmock, gmock_main")
