# ==============================================================================
# Source.cmake for projects/libs/InstrumentModels/src/
# CMake Architecture V2 - LumiViz
# ==============================================================================
# InstrumentModels (Instrumente_IC_Konzept.md BK1 a, Stufe 34d): die Rechenkerne
# der Instrumentklassen ueber InstrumentCore — je Klasse ein Unterordner mit
# eigener Source.cmake (hammer/: der Fluegel). STATIC, Schalter nach E1 a in
# Solution.json (PRIVATE): gerechnet wird in den .cpp-Dateien.
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/Kerne.cpp"
)
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentModels/Kerne.hpp"
)
set(_local_templates
    # (templates - *.t; *.tpp)
)
set(_local_inlines
    # (inlines - *.inl)
)
set(_local_impl
    # (impl - *.impl)
)
set(_local_modules
    # (modules - *.ixx; *.cppm)
)
set(_local_includes
    # (no extra include paths)
)

dbg(DBG_NORMAL "Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found modules  : ${_local_modules}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found includes : ${_local_includes}" ID DEB_FOUND_MSG)

list(APPEND ${TARGET_NAME}_SOURCES ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL ${_local_impl})
list(APPEND ${TARGET_NAME}_MODULES ${_local_modules})
list(APPEND ${TARGET_NAME}_INCLUDES ${_local_includes})

unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)
unset(_local_modules)
unset(_local_includes)

# je Klasse ein Unterordner
include("${CMAKE_CURRENT_LIST_DIR}/hammer/Source.cmake")
