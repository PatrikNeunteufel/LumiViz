# cmake/externals/hooks/prefetch/catch2.cmake
# ============================================
# PreFetch hook for Catch2 v3 - configures build and defines target mappings
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
#   - HOOK_KNOWN_TARGETS: Catch2, Catch2WithMain
#   - HOOK_PRIMARY_TARGET: Catch2WithMain
#
# Sets:
#   - CATCH_BUILD_TESTING OFF
#   - CATCH_BUILD_EXAMPLES OFF
#   - CATCH_INSTALL_DOCS OFF
#   - CATCH_INSTALL_EXTRAS OFF

message(STATUS "[${HOOK_EXTERNAL_NAME}] PreFetch: Configuring Catch2")

# ==============================================================================
# Target Mapping for Auto-Registration
# ==============================================================================
# Catch2 creates targets with different names than the external name.
# We tell the registry which targets to look for.

set_property(GLOBAL PROPERTY HOOK_KNOWN_TARGETS_${HOOK_EXTERNAL_NAME}
    "Catch2;Catch2WithMain"
)
set_property(GLOBAL PROPERTY HOOK_PRIMARY_TARGET_${HOOK_EXTERNAL_NAME}
    "Catch2WithMain"
)

# ==============================================================================
# Disable Tests and Examples
# ==============================================================================

# Disable Catch2's own tests
set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)

# Disable examples
set(CATCH_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Disable documentation installation
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)

# Disable extras (helpers for CMake integration)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)

# ==============================================================================
# Build Configuration
# ==============================================================================

# Build as static library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

message(STATUS "[${HOOK_EXTERNAL_NAME}] PreFetch complete")
message(STATUS "[${HOOK_EXTERNAL_NAME}]   Targets: Catch2, Catch2WithMain")
