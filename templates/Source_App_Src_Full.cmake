# ==============================================================================
# Source.cmake for src/
# CMake Architecture V2 - Source Collection (v0.6)
# ==============================================================================
# Location: projects/apps/{AppName}/src/Source.cmake
# Target:   ${TARGET_NAME} (e.g. MyApp.Core)
#
# Implementation files for the App-Container Core library.
# This file collects all source files and recursively includes subfolders.
# ==============================================================================

dbg(${DBG_OFTEN}
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# ==============================================================================
# Local file lists for THIS directory
# ==============================================================================

set(_local_sources
    # Sources (*.c, *.cpp, *.cxx, *.cc)
    "${CMAKE_CURRENT_LIST_DIR}/Application.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Config.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Logger.cpp"
)

set(_local_headers
    # Private headers (*.h, *.hpp, *.hxx, *.hh)
    # "${CMAKE_CURRENT_LIST_DIR}/Internal.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Private.hpp"
)

set(_local_templates
    # Template implementations (*.tpp, *.txx, *.ipp)
    # "${CMAKE_CURRENT_LIST_DIR}/Container.tpp"
    # "${CMAKE_CURRENT_LIST_DIR}/Factory.tpp"
)

set(_local_inlines
    # Inline implementations (*.inl)
    # "${CMAKE_CURRENT_LIST_DIR}/Math.inl"
    # "${CMAKE_CURRENT_LIST_DIR}/FastOps.inl"
)

set(_local_impl
    # PIMPL / detail implementations (*.impl)
    # "${CMAKE_CURRENT_LIST_DIR}/Application.impl"
    # "${CMAKE_CURRENT_LIST_DIR}/Engine.impl"
)

# ==============================================================================
# Debug: Show found files in THIS directory
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
# Debug: Show aggregated totals (before subfolders)
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
# Include subfolders (recursive collection)
# ==============================================================================
#
# Typical structure:
#   src/
#   ├── Source.cmake        ← This file
#   ├── Application.cpp
#   ├── core/
#   │   ├── Source.cmake    ← include below
#   │   ├── Engine.cpp
#   │   └── Engine.hpp
#   ├── audio/
#   │   ├── Source.cmake
#   │   └── ...
#   └── ui/
#       ├── Source.cmake
#       └── ...
#
# ==============================================================================

dbg(${DBG_ULTRA_RARE} "[${TARGET_NAME}] Including subfolders:" ID INCLUDE_MSG)

# Uncomment and adapt as needed:
# include("${CMAKE_CURRENT_LIST_DIR}/core/Source.cmake")
# include("${CMAKE_CURRENT_LIST_DIR}/audio/Source.cmake")
# include("${CMAKE_CURRENT_LIST_DIR}/ui/Source.cmake")
# include("${CMAKE_CURRENT_LIST_DIR}/utils/Source.cmake")
# include("${CMAKE_CURRENT_LIST_DIR}/network/Source.cmake")
