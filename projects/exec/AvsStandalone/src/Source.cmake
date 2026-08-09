# ==============================================================================
# Source.cmake for executable src/
# CMake Architecture V2 - Source Collection
# ==============================================================================
# Location: projects/exec/AvsStandalone/src/Source.cmake
# Target:   AvsStandalone — isolierter AVS-Renderpfad-Test (Session 43)
# ==============================================================================

dbg(${DBG_OFTEN}
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# ==============================================================================
# Local file lists for THIS directory
# ==============================================================================

# Die Multieffekt-Host-Schliessung der App wird DIREKT mitkompiliert (wie beim
# MilkdropStandalone): die Executables-Phase laeuft VOR dem App-Container,
# LumiViz.Core existiert hier noch nicht. Explizite Liste — waechst der Host,
# muss sie mitwachsen (der Linker meldet es).
set(_myviz_src "${CMAKE_SOURCE_DIR}/projects/apps/LumiViz/src")

set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    "${_myviz_src}/visualizers/MultiEffectVisualizer.cpp"
    "${_myviz_src}/visualizers/AvsChainTranslator.cpp"
    "${_myviz_src}/visualizers/ChainSerializer.cpp"
    "${_myviz_src}/visualizers/MilkdropVisualizer.cpp"
    "${_myviz_src}/visualizers/MilkdropSerializer.cpp"
    "${_myviz_src}/visualizers/VisualizerBase.cpp"
    "${_myviz_src}/visualizers/FeedbackBuffer.cpp"
    "${_myviz_src}/visualizers/ScopeRenderer.cpp"
    "${_myviz_src}/visualizers/modules/ColorGradientModule.cpp"
    "${_myviz_src}/visualizers/modules/SuperscopeModule.cpp"
    "${_myviz_src}/visualizers/modules/ScriptGridModule.cpp"
    "${_myviz_src}/visualizers/modules/ScriptLutModule.cpp"
    "${_myviz_src}/scripting/ScriptSlotHost.cpp"
    "${_myviz_src}/scripting/LuaScriptEngine.cpp"
    "${_myviz_src}/services/VideoFrameCache.cpp"
    "${_myviz_src}/services/LiveVideoFeed.cpp"
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

# MSVC: MultiEffectVisualizer.cpp sprengt das COFF-Abschnittslimit (fatal
# error C1128 im Debug-Build, S66) — /bigobj nur fuer diese Datei (Muster
# UI/panels/Source.cmake; die Property ist Directory-scoped und muss deshalb
# in JEDEM Target-Verzeichnis gesetzt werden, das die Datei mitkompiliert).
set_source_files_properties("${_myviz_src}/visualizers/MultiEffectVisualizer.cpp"
    PROPERTIES COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")

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
