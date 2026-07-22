# ==============================================================================
# Source.cmake for executable src/
# CMake Architecture V2 - Source Collection
# ==============================================================================
# Location: projects/exec/MilkdropStandalone/src/Source.cmake
# Target:   MilkdropStandalone — isolierter C1/C2-Renderpfad-Test (Session 41)
# ==============================================================================

dbg(${DBG_OFTEN}
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# ==============================================================================
# Local file lists for THIS directory
# ==============================================================================

# Die Milkdrop-Schliessung der App wird DIREKT mitkompiliert: die Executables-
# Phase laeuft in CMakeCraft v0.8.0 VOR dem App-Container, MyViz.Core existiert
# als Target hier noch nicht (E101). Explizite Liste — waechst der Host, muss
# sie mitwachsen (der Linker meldet es).
set(_myviz_src "${CMAKE_SOURCE_DIR}/projects/apps/MyViz/src")

set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    "${_myviz_src}/visualizers/MilkdropVisualizer.cpp"
    "${_myviz_src}/visualizers/MilkdropSerializer.cpp"
    "${_myviz_src}/visualizers/VisualizerBase.cpp"
    "${_myviz_src}/visualizers/FeedbackBuffer.cpp"
    "${_myviz_src}/visualizers/ScopeRenderer.cpp"
    "${_myviz_src}/scripting/ScriptSlotHost.cpp"
    "${_myviz_src}/scripting/LuaScriptEngine.cpp"
)

set(_local_headers
)

set(_local_includes
    "${CMAKE_SOURCE_DIR}/projects/apps/MyViz/include"
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
unset(_myviz_src)
