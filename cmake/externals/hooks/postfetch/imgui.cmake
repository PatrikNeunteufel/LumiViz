# cmake/externals/Hooks/PostFetch/imgui.cmake
# ============================================
# PostFetch hook for Dear ImGui - creates target manually (no CMakeLists.txt)
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Hook Variables (from HookLoader):
#   - HOOK_EXTERNAL_NAME - Target name (e.g. "imgui" or "imgui_docking")
#   - HOOK_SOURCE_DIR    - Path to imgui source
#   - HOOK_EXTERNAL_JSON - JSON definition
#
# Creates:
#   - ${HOOK_EXTERNAL_NAME} target with core + backends (OpenGL3, GLFW, Win32)
#
# Hook Reuse:
#   Supports variants via "hook" field in Solution.json:
#   "imgui_docking": { "git": "...", "hook": "imgui" }

message(STATUS "[${HOOK_EXTERNAL_NAME}] Creating target from: ${HOOK_SOURCE_DIR}")

# ==============================================================================
# Cleanup: Remove examples folder (avoids VS Solution clutter)
# ==============================================================================

if(EXISTS "${HOOK_SOURCE_DIR}/examples")
    message(STATUS "[${HOOK_EXTERNAL_NAME}]   Removing examples/ folder (VS cleanup)")
    file(REMOVE_RECURSE "${HOOK_SOURCE_DIR}/examples")
endif()

# ==============================================================================
# Collect Source Files
# ==============================================================================

set(_imgui_sources
    "${HOOK_SOURCE_DIR}/imgui.cpp"
    "${HOOK_SOURCE_DIR}/imgui_demo.cpp"
    "${HOOK_SOURCE_DIR}/imgui_draw.cpp"
    "${HOOK_SOURCE_DIR}/imgui_tables.cpp"
    "${HOOK_SOURCE_DIR}/imgui_widgets.cpp"
)

set(_imgui_includes
    "${HOOK_SOURCE_DIR}"
    "${HOOK_SOURCE_DIR}/backends"
)

# ==============================================================================
# Add OpenGL3 Backend
# ==============================================================================

if(EXISTS "${HOOK_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")
    list(APPEND _imgui_sources "${HOOK_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")
    message(STATUS "[${HOOK_EXTERNAL_NAME}]   + OpenGL3 backend")
endif()

# ==============================================================================
# Add GLFW Backend
# ==============================================================================

if(EXISTS "${HOOK_SOURCE_DIR}/backends/imgui_impl_glfw.cpp")
    list(APPEND _imgui_sources "${HOOK_SOURCE_DIR}/backends/imgui_impl_glfw.cpp")
    message(STATUS "[${HOOK_EXTERNAL_NAME}]   + GLFW backend")
endif()

# ==============================================================================
# Add Win32 Backend (Windows only)
# ==============================================================================

if(WIN32 AND EXISTS "${HOOK_SOURCE_DIR}/backends/imgui_impl_win32.cpp")
    list(APPEND _imgui_sources "${HOOK_SOURCE_DIR}/backends/imgui_impl_win32.cpp")
    message(STATUS "[${HOOK_EXTERNAL_NAME}]   + Win32 backend")
endif()

# ==============================================================================
# Create Combined ImGui Library (Dynamic Target Name!)
# ==============================================================================

add_library(${HOOK_EXTERNAL_NAME} STATIC ${_imgui_sources})

target_include_directories(${HOOK_EXTERNAL_NAME} PUBLIC ${_imgui_includes})

# C++ Standard
target_compile_features(${HOOK_EXTERNAL_NAME} PUBLIC cxx_std_11)

# Suppress warnings in external code
if(MSVC)
    target_compile_options(${HOOK_EXTERNAL_NAME} PRIVATE /W0)
else()
    target_compile_options(${HOOK_EXTERNAL_NAME} PRIVATE -w)
endif()

# ==============================================================================
# Link Dependencies
# ==============================================================================

# Link GLAD if available (for OpenGL3 backend)
if(TARGET glad)
    target_link_libraries(${HOOK_EXTERNAL_NAME} PUBLIC glad)
    target_compile_definitions(${HOOK_EXTERNAL_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
    message(STATUS "[${HOOK_EXTERNAL_NAME}]   Linked: glad")
endif()

# Link GLFW if available (for GLFW backend)
if(TARGET glfw)
    target_link_libraries(${HOOK_EXTERNAL_NAME} PUBLIC glfw)
    message(STATUS "[${HOOK_EXTERNAL_NAME}]   Linked: glfw")
endif()

# ==============================================================================
# Register Target (Dynamic Name!)
# ==============================================================================

_register_external_target("${HOOK_EXTERNAL_NAME}" "${HOOK_EXTERNAL_NAME}" PRIMARY)

message(STATUS "[${HOOK_EXTERNAL_NAME}] Target '${HOOK_EXTERNAL_NAME}' created (combined with backends)")
message(STATUS "[${HOOK_EXTERNAL_NAME}] PostFetch hook complete")
