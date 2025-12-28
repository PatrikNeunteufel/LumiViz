# ==============================================================================
# Source.cmake for main/
# CMake Architecture V2 - Source Collection (v0.6)
# ==============================================================================
# Location: projects/apps/{AppName}/main/Source.cmake
# Target:   ${TARGET_NAME} (e.g. MyApp - the Runner executable)
#
# Entry point for the App-Container Runner.
# Should be minimal - just main() that instantiates and runs the Application.
# ==============================================================================

dbg(${DBG_OFTEN}
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# ==============================================================================
# Local file lists for THIS directory
# ==============================================================================

set(_local_sources
    # Entry point (*.c, *.cpp, *.cxx, *.cc)
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

set(_local_headers
    # (usually no headers in main/ - *.h, *.hpp, *.hxx, *.hh)
)

set(_local_templates
    # (no templates in main/ - *.tpp, *.txx, *.ipp)
)

set(_local_inlines
    # (no inlines in main/ - *.inl)
)

set(_local_impl
    # (no impl in main/ - *.impl)
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
# No subfolders expected in main/
# ==============================================================================
