# externals/CDT/Include.cmake
# ============================
# CDT (Constrained Delaunay Triangulation, artem-ogre/CDT 2.0.1) — nur Header,
# als INTERFACE-Ziel aus dem Ordner include/.
#
# Diese Datei liegt IM PROJEKT, nicht in CMakeCraft: Solution.json nennt sie
# ueber das Feld "include" des Externals (Muster externals/miniz, Entscheid
# Patrik S114, E2 b).
#
# Erwartete Variablen (setzt der Orchestrator von CMakeCraft):
#   EXTERNAL_NAME    - "CDT"
#   EXTERNAL_ROOT    - Pfad zu externals/CDT
#   EXECUTABLE_NAME  - das Ziel, an das gebunden wird
#
# Herkunft, Fassung und Pruefsumme: README.md in diesem Ordner.
# Kein include_guard(): die Datei wird je Ziel eingebunden.

if(NOT DEFINED EXTERNAL_ROOT)
    set(EXTERNAL_ROOT "${CMAKE_SOURCE_DIR}/externals/CDT")
endif()

if(NOT EXISTS "${EXTERNAL_ROOT}/include/CDT.h")
    message(FATAL_ERROR "[CDT] include/CDT.h fehlt in ${EXTERNAL_ROOT} — siehe README.md dort")
endif()

if(NOT TARGET CDT)
    add_library(CDT INTERFACE)
    # SYSTEM: fremder Code, seine Warnungen gehoeren nicht zu uns
    target_include_directories(CDT SYSTEM INTERFACE "${EXTERNAL_ROOT}/include")
    message(STATUS "[CDT] Ziel 'CDT' angelegt (${EXTERNAL_ROOT})")
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE CDT)
message(STATUS "[CDT] an ${EXECUTABLE_NAME} gebunden")
