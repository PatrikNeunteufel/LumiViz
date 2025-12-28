# ==============================================================================
# Source.cmake for executable src/
# CMake Architecture V2 - Source Collection (v0.6)
# ==============================================================================
# Location: projects/demos/exec/{ExecName}/src/Source.cmake
#       or: projects/exec/{ExecName}/src/Source.cmake
# Target:   ${TARGET_NAME} (e.g. MinimalConsole)
#
# Source files for a standalone executable.
# ==============================================================================

dbg(${DBG_OFTEN}
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# ==============================================================================
# Local file lists for THIS directory
# ==============================================================================

set(_local_sources
    # Sources (*.c, *.cpp, *.cxx, *.cc)
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Application.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Utils.cpp"
)

set(_local_headers
    # Headers (*.h, *.hpp, *.hxx, *.hh)
    # "${CMAKE_CURRENT_LIST_DIR}/Application.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Utils.hpp"
)

set(_local_templates
    # Template implementations (*.tpp, *.txx, *.ipp)
    # "${CMAKE_CURRENT_LIST_DIR}/Container.tpp"
)

set(_local_inlines
    # Inline implementations (*.inl)
    # "${CMAKE_CURRENT_LIST_DIR}/Math.inl"
)

set(_local_impl
    # PIMPL / detail implementations (*.impl)
    # "${CMAKE_CURRENT_LIST_DIR}/Application.impl"
)

# ==============================================================================
# Debug: Show found files
# ==============================================================================

dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)

# ==============================================================================
# Aggregate to TARGET variables
# ==============================================================================

list(APPEND ${TARGET_NAME}_SOURCES   ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS   ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES   ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL      ${_local_impl})

# ==============================================================================
# Debug: Show aggregated totals
# ==============================================================================

dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated SOURCES  : ${${TARGET_NAME}_SOURCES}" ID DEB_AGG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated HEADERS  : ${${TARGET_NAME}_HEADERS}" ID DEB_AGG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated TEMPLATES: ${${TARGET_NAME}_TEMPLATES}" ID DEB_AGG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated INLINES  : ${${TARGET_NAME}_INLINES}" ID DEB_AGG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated IMPL     : ${${TARGET_NAME}_IMPL}" ID DEB_AGG)

# ==============================================================================
# Cleanup local variables
# ==============================================================================

unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)

# ==============================================================================
# Include subfolders (if needed)
# ==============================================================================
#
# For larger executables with multiple modules:
#   src/
#   ├── Source.cmake        ← This file
#   ├── main.cpp
#   ├── audio/
#   │   ├── Source.cmake
#   │   └── Player.cpp
#   └── ui/
#       ├── Source.cmake
#       └── Window.cpp
#
# ==============================================================================

dbg(${DBG_ULTRA_RARE} "[${TARGET_NAME}] Including subfolders:" ID INCLUDE_MSG)

# Uncomment and adapt as needed:
# include("${CMAKE_CURRENT_LIST_DIR}/audio/Source.cmake")
# include("${CMAKE_CURRENT_LIST_DIR}/ui/Source.cmake")
# include("${CMAKE_CURRENT_LIST_DIR}/utils/Source.cmake")
