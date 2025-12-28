# cmake/core/OutputDirs.cmake
# ============================
# Standardized output directories for all targets
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - None (standalone module)
#
# Provides:
#   - setup_output_dirs(TARGET_NAME)
#
# Sets unified output directories per target with type separation:
#   - exec/${TARGET_NAME}/bin/  : Executables and their DLLs
#   - libs/${TARGET_NAME}/lib/  : Static/Shared libraries
#   - With Debug/Release/Testing subdirectories
#
# NEW in v0.1.3:
#   - Each target gets its own subdirectory for better isolation
#   - Automatic detection of target type (EXECUTABLE vs LIBRARY)
#   - Separation into exec/ and libs/ folders
#
# Used by:
#   - ExecutableCreate.cmake
#   - LibraryCreate.cmake

include_guard(GLOBAL)

# ============================================================================
# setup_output_dirs - Configure output directories for a target
# ============================================================================
#[[
    setup_output_dirs(TARGET_NAME)
    
    Configures standardized output directories for a target.
    Automatically detects target type and places output accordingly.
    
    Parameters:
        TARGET_NAME - Mandatory: CMake target (must already exist)
    
    Output structure for EXECUTABLES:
        ${CMAKE_BINARY_DIR}/exec/${TARGET_NAME}/bin/         - Executables, DLLs
        ${CMAKE_BINARY_DIR}/exec/${TARGET_NAME}/bin/Debug/   - Debug builds
        ${CMAKE_BINARY_DIR}/exec/${TARGET_NAME}/bin/Release/ - Release builds
    
    Output structure for LIBRARIES:
        ${CMAKE_BINARY_DIR}/libs/${TARGET_NAME}/lib/         - Libraries
        ${CMAKE_BINARY_DIR}/libs/${TARGET_NAME}/lib/Debug/   - Debug builds
        ${CMAKE_BINARY_DIR}/libs/${TARGET_NAME}/lib/Release/ - Release builds
    
    Properties set:
        RUNTIME_OUTPUT_DIRECTORY  - bin/     (Executables, DLLs)
        LIBRARY_OUTPUT_DIRECTORY  - lib/     (Shared libraries .so)
        ARCHIVE_OUTPUT_DIRECTORY  - lib/     (Static libraries .a, .lib)
    
    Each with config variants:
        *_DEBUG, *_RELEASE, *_TESTING
    
    Example:
        add_executable(MyApp main.cpp)
        setup_output_dirs(MyApp)
        # -> build/exec/MyApp/bin/Debug/MyApp.exe
        
        add_library(CoreLib STATIC core.cpp)
        setup_output_dirs(CoreLib)
        # -> build/libs/CoreLib/lib/Debug/CoreLib.lib
    
    Result structure:
        out/build/preset-name/
        ├── exec/
        │   ├── MyApp/
        │   │   └── bin/Debug/
        │   │       ├── MyApp.exe
        │   │       └── required.dll
        │   └── OtherApp/
        │       └── bin/Debug/
        │           └── OtherApp.exe
        └── libs/
            ├── CoreLib/
            │   └── lib/Debug/
            │       └── CoreLib.lib
            └── PluginLib/
                └── lib/Debug/
                    └── PluginLib.dll
]]
function(setup_output_dirs TARGET_NAME)
    # Get target type for automatic categorization
    get_target_property(_target_type ${TARGET_NAME} TYPE)
    
    # Determine category based on target type
    if(_target_type STREQUAL "EXECUTABLE")
        set(_category "exec")
    elseif(_target_type STREQUAL "STATIC_LIBRARY" OR 
           _target_type STREQUAL "SHARED_LIBRARY" OR 
           _target_type STREQUAL "MODULE_LIBRARY" OR
           _target_type STREQUAL "OBJECT_LIBRARY" OR
           _target_type STREQUAL "INTERFACE_LIBRARY")
        set(_category "libs")
    else()
        # Fallback for unknown types
        set(_category "other")
    endif()
    
    # Base path for this target
    set(_target_base "${CMAKE_BINARY_DIR}/${_category}/${TARGET_NAME}")
    
    # Binaries (Executables, DLLs)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${_target_base}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${_target_base}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${_target_base}/bin/Release"
        RUNTIME_OUTPUT_DIRECTORY_TESTING "${_target_base}/bin/Testing"
    )
    
    # Shared libraries (.so on Linux)
    set_target_properties(${TARGET_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${_target_base}/lib"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${_target_base}/lib/Debug"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${_target_base}/lib/Release"
        LIBRARY_OUTPUT_DIRECTORY_TESTING "${_target_base}/lib/Testing"
    )
    
    # Archives (Static libraries .a, .lib)
    set_target_properties(${TARGET_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${_target_base}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${_target_base}/lib/Debug"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${_target_base}/lib/Release"
        ARCHIVE_OUTPUT_DIRECTORY_TESTING "${_target_base}/lib/Testing"
    )
endfunction()
