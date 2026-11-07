# ==============================================================================
# Source.cmake for projects/libs/InstrumentCore/src/
# CMake Architecture V2 - LumiViz
# ==============================================================================
# InstrumentCore (Instrumente_IC_Konzept.md KN1 a, Stufe 34c): die Qt-freie
# Bibliothek der Rechenkerne. STATIC, Schalter nach E1 a in Solution.json
# (compile_options, PRIVATE) — deshalb wird in den .cpp-Dateien hier gerechnet,
# nicht in den Koepfen. Private Koepfe gehoeren unter _local_headers, sonst
# fehlen sie im Quellpaket (package_instrumentcore); die oeffentlichen stehen
# dort zur Uebersicht und kommen nach include/.
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/Anschlag.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Eigenzerlegung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Fft.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Kurve.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Modal.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Parametertafel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Pool.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Pruefsumme.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Tonauftrag.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Tonkern.cpp"
)
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Anschlag.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Eigenzerlegung.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Fft.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Kurve.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Modal.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Parametertafel.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Pool.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Pruefsumme.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Tonauftrag.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentCore/Tonkern.hpp"
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
