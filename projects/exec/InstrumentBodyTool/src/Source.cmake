# ==============================================================================
# Source.cmake for executable src/
# CMake Architecture V2 - Source Collection
# ==============================================================================
# Location: projects/exec/InstrumentBodyTool/src/Source.cmake
# Target:   InstrumentBodyTool — rechnet einen Instrumentkoerper aus seinem Bauplan
#           (Spielweisen_Konzept.md §4.1, BM5 a; Stufe 31c-4)
# ==============================================================================

dbg(${DBG_OFTEN}
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# ==============================================================================
# Local file lists for THIS directory
# ==============================================================================

# Der Leser des Bauplans aus loop/ der App wird DIREKT mitkompiliert: die
# Executables-Phase laeuft vor dem App-Container (Muster MilkdropStandalone).
# Explizite Liste — waechst der Leser, muss sie mitwachsen (der Linker meldet es).
set(_lumiviz_src "${CMAKE_SOURCE_DIR}/projects/apps/LumiViz/src")

set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    "${_lumiviz_src}/loop/KoerperBauplan.cpp"
    "${_lumiviz_src}/core/Mitgeliefert.cpp"
)

set(_local_headers
)

set(_local_includes
    "${CMAKE_SOURCE_DIR}/projects/apps/LumiViz/include"
)

set(_local_templates
)

set(_local_inlines
)

set(_local_impl
)

# ==============================================================================
# Aggregate to TARGET variables
# ==============================================================================

list(APPEND ${TARGET_NAME}_SOURCES   ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS   ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES   ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL      ${_local_impl})
list(APPEND ${TARGET_NAME}_INCLUDES  ${_local_includes})

# ==============================================================================
# Cleanup local variables
# ==============================================================================

unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)
unset(_local_includes)
unset(_lumiviz_src)
