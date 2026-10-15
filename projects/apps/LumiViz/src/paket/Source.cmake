# ==============================================================================
# Source.cmake for src/paket/
# CMake Architecture V2 - App-Container Template
# ==============================================================================
# Publish (docs/core-services/Publish_Konzept.md): ein Ding mit allem, was es
# nennt, als Paket weitergeben. Nicht unter loop/: in Stufe P-B kommen
# Komposition und Effect chain dazu. Nicht `publish/`: den Namen schliesst
# die .gitignore aus (Ausgaben von Visual Studio).
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# Set local lists for this directory
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/Sammler.cpp"
)
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/../../include/paket/Sammler.hpp"
)
set(_local_templates
    # (no templates - *.t; *.tpp)
)
set(_local_inlines
    # (no inlines - *.inl)
)
set(_local_impl
    # (no impl - *.impl)
)
set(_local_modules
    # (no modules - *.cxx)
)
set(_local_includes
    # (no impl - *.?)
)

dbg(DBG_NORMAL "Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found modules  : ${_local_modules}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found includes : ${_local_includes}" ID DEB_FOUND_MSG)

# Aggregate to parent scope
list(APPEND ${TARGET_NAME}_SOURCES ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL ${_local_impl})
list(APPEND ${TARGET_NAME}_MODULES ${_local_modules})
list(APPEND ${TARGET_NAME}_INCLUDES ${_local_includes})

# Optional cleanup (cosmetic)
unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)
unset(_local_modules)
unset(_local_includes)
