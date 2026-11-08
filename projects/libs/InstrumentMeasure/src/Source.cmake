# ==============================================================================
# Source.cmake for projects/libs/InstrumentMeasure/src/
# CMake Architecture V2 - LumiViz
# ==============================================================================
# InstrumentMeasure (Instrumente_IC_Konzept.md KN1 a, IK2-2 a, Stufe 34e): die
# Qt-freie Messkette und das Setzen ueber InstrumentCore. STATIC, Schalter nach
# E1 a in Solution.json (compile_options, PRIVATE) — deshalb wird in den
# .cpp-Dateien hier gerechnet, nicht in den Koepfen. Private Koepfe gehoeren
# unter _local_headers, sonst fehlen sie im Quellpaket
# (package_instrumentmeasure).
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/Bausteine.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Fluegelmessung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Hoerschwellen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Kurvenabstand.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/LevenbergMarquardt.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/NelderMead.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Probe.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Samples.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Strukturmessung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Teiltonmessung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Zielsumme.cpp"
)
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Bausteine.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Fluegelmessung.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Hoerschwellen.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Kurvenabstand.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/LevenbergMarquardt.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/NelderMead.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Probe.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Samples.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Strukturmessung.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Teiltonmessung.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../include/InstrumentMeasure/Zielsumme.hpp"
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
