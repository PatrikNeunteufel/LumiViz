# ==============================================================================
# Source.cmake for executable src/
# CMake Architecture V2 - Source Collection
# ==============================================================================
# Location: projects/exec/InstrumentTuneTool/src/Source.cmake
# Target:   InstrumentTuneTool — stimmt die gestimmten Werte einer .lvinst und
#           setzt ihren Bezug (Instrumente_IC_Konzept.md IK2-10 a, IK2-11 a;
#           Stufe 34f-3). Die Dateischicht der Kern-Werte kommt aus loop/ der
#           App, direkt mitkompiliert (Executables laufen vor den Apps).
# ==============================================================================

dbg(${DBG_OFTEN}
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

set(_lumiviz_src "${CMAKE_SOURCE_DIR}/projects/apps/LumiViz/src")
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    "${_lumiviz_src}/loop/KernDatei.cpp"
)
set(_local_headers
)
set(_local_templates
)
set(_local_inlines
)
set(_local_impl
)
set(_local_modules
)
set(_local_includes
    "${CMAKE_SOURCE_DIR}/projects/apps/LumiViz/include"
)

dbg(DBG_NORMAL "Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found includes : ${_local_includes}" ID DEB_FOUND_MSG)

list(APPEND ${TARGET_NAME}_SOURCES ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL ${_local_impl})
list(APPEND ${TARGET_NAME}_MODULES ${_local_modules})
list(APPEND ${TARGET_NAME}_INCLUDES ${_local_includes})

unset(_lumiviz_src)
unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)
unset(_local_modules)
unset(_local_includes)
