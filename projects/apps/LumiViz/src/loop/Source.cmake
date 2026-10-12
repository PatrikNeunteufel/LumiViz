# ==============================================================================
# Source.cmake for src/loop/
# CMake Architecture V2 - App-Container Template
# ==============================================================================
# Loop (docs/audio/Loop_Konzept.md): Modell, Datei `.lvloop`, Rechenkern,
# Klangerzeuger (die Quelle im Mixer), Kit, Spieler (der Loop allein), Panel.
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# Set local lists for this directory
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/Auswahl.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Bearbeiten.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ClipEffektFenster.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/DrumEditor.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Effekte.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Instrument.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/InstrumentCreator.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/InstrumentPalette.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/InstrumentEditor.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Kit.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Klangerzeuger.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Loop.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/LoopPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Offen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Rechenkern.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SichttestAktionen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Spieler.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Taktachse.cpp"
)
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Auswahl.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Bearbeiten.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/ClipEffektFenster.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Offen.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/DrumEditor.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Effekte.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Taktachse.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Instrument.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/InstrumentCreator.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/InstrumentPalette.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/InstrumentEditor.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Kit.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Klangerzeuger.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Loop.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/LoopPanel.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Rechenkern.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/SichttestAktionen.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/loop/Spieler.hpp"
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
