# externals/Spectra/Include.cmake
# ================================
# Spectra 1.2.0 (yixuan/spectra) — nur Header, als INTERFACE-Ziel aus diesem
# Ordner (Spectra/...). Braucht Eigen: das Ziel Eigen muss vorher angelegt
# sein — in Solution.json "Eigen" vor "Spectra" nennen.
#
# Diese Datei liegt IM PROJEKT, nicht in CMakeCraft: Solution.json nennt sie
# ueber das Feld "include" des Externals (Muster externals/miniz, Entscheid
# Patrik S114, E2 b).
#
# Erwartete Variablen (setzt der Orchestrator von CMakeCraft):
#   EXTERNAL_NAME    - "Spectra"
#   EXTERNAL_ROOT    - Pfad zu externals/Spectra
#   EXECUTABLE_NAME  - das Ziel, an das gebunden wird
#
# Herkunft, Fassung und Pruefsumme: README.md in diesem Ordner.
# Kein include_guard(): die Datei wird je Ziel eingebunden.

if(NOT DEFINED EXTERNAL_ROOT)
    set(EXTERNAL_ROOT "${CMAKE_SOURCE_DIR}/externals/Spectra")
endif()

if(NOT EXISTS "${EXTERNAL_ROOT}/Spectra/SymGEigsShiftSolver.h")
    message(FATAL_ERROR "[Spectra] Spectra/SymGEigsShiftSolver.h fehlt in ${EXTERNAL_ROOT} — siehe README.md dort")
endif()

if(NOT TARGET Eigen)
    message(FATAL_ERROR "[Spectra] braucht das Ziel Eigen — in Solution.json \"Eigen\" vor \"Spectra\" nennen")
endif()

if(NOT TARGET Spectra)
    add_library(Spectra INTERFACE)
    # SYSTEM: fremder Code, seine Warnungen gehoeren nicht zu uns
    target_include_directories(Spectra SYSTEM INTERFACE "${EXTERNAL_ROOT}")
    target_link_libraries(Spectra INTERFACE Eigen)
    message(STATUS "[Spectra] Ziel 'Spectra' angelegt (${EXTERNAL_ROOT})")
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE Spectra)
message(STATUS "[Spectra] an ${EXECUTABLE_NAME} gebunden")
