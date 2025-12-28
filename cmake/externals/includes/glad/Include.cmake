# cmake/externals/includes/glad/Include.cmake
# ============================================
# GLAD OpenGL Loader integration - creates static library from pre-generated sources
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
# Type:    Local External Include
#
# Dependencies:
#   - OpenGL (find_package)
#
# Expected Variables (set by Orchestrator):
#   - EXTERNAL_NAME    - "glad"
#   - EXTERNAL_ROOT    - Path to externals/glad (resources)
#   - EXTERNAL_OPTIONS - JSON options (not used)
#   - EXECUTABLE_NAME  - Target to attach to
#
# Resources (externals/glad/):
#   - include/glad/glad.h
#   - include/KHR/khrplatform.h
#   - src/glad.c
#
# Generates:
#   - glad (STATIC library target)
#
# Generating GLAD sources:
#   Visit https://glad.dav1d.de/ with:
#   - Language: C/C++
#   - Specification: OpenGL
#   - Profile: Core
#   - API gl: 3.3+
#   - Generate a loader: checked
#
# Used by:
#   - Orchestrator.cmake (via apply_external_to_target)
#   - imgui.cmake (PostFetch hook links glad)

# ==============================================================================
# Compatibility: Support both new and legacy variable names
# ==============================================================================

if(DEFINED EXTERNAL_NAME)
    set(_ext_name "${EXTERNAL_NAME}")
    set(_ext_root "${EXTERNAL_ROOT}")
    set(_ext_options "${EXTERNAL_OPTIONS}")
elseif(DEFINED EXTERNAL_ELEMENT_NAME)
    set(_ext_name "${EXTERNAL_ELEMENT_NAME}")
    set(_ext_root "${CMAKE_SOURCE_DIR}/externals/${_ext_name}")
    set(_ext_options "${EXTERNAL_ELEMENT_OPTIONS}")
else()
    message(FATAL_ERROR "[glad] Neither EXTERNAL_NAME nor EXTERNAL_ELEMENT_NAME defined")
endif()

set(_target "${EXECUTABLE_NAME}")

message(STATUS "[glad] Configuring for target: ${_target}")

# ==============================================================================
# Validate Directory Structure
# ==============================================================================

set(_glad_include "${_ext_root}/include")
set(_glad_src "${_ext_root}/src/glad.c")

if(NOT EXISTS "${_glad_include}/glad/glad.h")
    message(FATAL_ERROR 
        "[glad] Header not found: ${_glad_include}/glad/glad.h\n"
        "Please generate GLAD sources from https://glad.dav1d.de/")
endif()

if(NOT EXISTS "${_glad_src}")
    message(FATAL_ERROR 
        "[glad] Source not found: ${_glad_src}\n"
        "Please generate GLAD sources from https://glad.dav1d.de/")
endif()

# ==============================================================================
# Create GLAD Library Target (if not exists)
# ==============================================================================

if(NOT TARGET glad)
    add_library(glad STATIC "${_glad_src}")
    
    target_include_directories(glad PUBLIC "${_glad_include}")
    
    # Suppress warnings in external code
    if(MSVC)
        target_compile_options(glad PRIVATE /W0)
    else()
        target_compile_options(glad PRIVATE -w)
    endif()
    
    message(STATUS "[glad] Created library target 'glad'")
endif()

# ==============================================================================
# Link to Target
# ==============================================================================

target_link_libraries(${_target} PRIVATE glad)

# OpenGL is required
find_package(OpenGL REQUIRED)
target_link_libraries(${_target} PRIVATE OpenGL::GL)

message(STATUS "[glad] Linked to ${_target}")
