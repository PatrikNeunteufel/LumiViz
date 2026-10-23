# externals/Eigen/Include.cmake
# ==============================
# Eigen 3.4.1 — nur Header, als INTERFACE-Ziel aus diesem Ordner (Eigen/...).
#
# Diese Datei liegt IM PROJEKT, nicht in CMakeCraft: Solution.json nennt sie
# ueber das Feld "include" des Externals (Muster externals/miniz, Entscheid
# Patrik S114, E2 b).
#
# EIGEN_MPL2_ONLY: Dateien mit LGPL-Code lassen sich nicht einbinden
# (COPYING.README des Archivs).
#
# Erwartete Variablen (setzt der Orchestrator von CMakeCraft):
#   EXTERNAL_NAME    - "Eigen"
#   EXTERNAL_ROOT    - Pfad zu externals/Eigen
#   EXECUTABLE_NAME  - das Ziel, an das gebunden wird
#
# Herkunft, Fassung und Pruefsumme: README.md in diesem Ordner.
# Kein include_guard(): die Datei wird je Ziel eingebunden.

if(NOT DEFINED EXTERNAL_ROOT)
    set(EXTERNAL_ROOT "${CMAKE_SOURCE_DIR}/externals/Eigen")
endif()

if(NOT EXISTS "${EXTERNAL_ROOT}/Eigen/Core")
    message(FATAL_ERROR "[Eigen] Eigen/Core fehlt in ${EXTERNAL_ROOT} — siehe README.md dort")
endif()

if(NOT TARGET Eigen)
    add_library(Eigen INTERFACE)
    # SYSTEM: fremder Code, seine Warnungen gehoeren nicht zu uns
    target_include_directories(Eigen SYSTEM INTERFACE "${EXTERNAL_ROOT}")
    target_compile_definitions(Eigen INTERFACE EIGEN_MPL2_ONLY)
    message(STATUS "[Eigen] Ziel 'Eigen' angelegt (${EXTERNAL_ROOT})")
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE Eigen)
message(STATUS "[Eigen] an ${EXECUTABLE_NAME} gebunden")
