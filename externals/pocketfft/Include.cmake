# externals/pocketfft/Include.cmake
# ==================================
# pocketfft (mreineck/pocketfft, Zweig cpp) — nur ein Header, als INTERFACE-Ziel aus
# diesem Ordner (pocketfft_hdronly.h). Die FFT der Faltung je Ton in loop/Fft
# (Spielweisen_Konzept.md FT1 a, FF1 a).
#
# Diese Datei liegt IM PROJEKT, nicht in CMakeCraft: Solution.json nennt sie
# ueber das Feld "include" des Externals (Muster externals/miniz, Entscheid
# Patrik S114, E2 b).
#
# POCKETFFT_NO_MULTITHREADING: pocketfft bringt einen eigenen Thread-Pool mit;
# LumiViz rechnet Toene schon parallel (Tonspeicher), eine FFT laeuft je Ton auf
# einem Thread.
#
# Erwartete Variablen (setzt der Orchestrator von CMakeCraft):
#   EXTERNAL_NAME    - "pocketfft"
#   EXTERNAL_ROOT    - Pfad zu externals/pocketfft
#   EXECUTABLE_NAME  - das Ziel, an das gebunden wird
#
# Herkunft, Fassung und Pruefsumme: README.md in diesem Ordner.
# Kein include_guard(): die Datei wird je Ziel eingebunden.

if(NOT DEFINED EXTERNAL_ROOT)
    set(EXTERNAL_ROOT "${CMAKE_SOURCE_DIR}/externals/pocketfft")
endif()

if(NOT EXISTS "${EXTERNAL_ROOT}/pocketfft_hdronly.h")
    message(FATAL_ERROR "[pocketfft] pocketfft_hdronly.h fehlt in ${EXTERNAL_ROOT} — siehe README.md dort")
endif()

if(NOT TARGET pocketfft)
    add_library(pocketfft INTERFACE)
    # SYSTEM: fremder Code, seine Warnungen gehoeren nicht zu uns
    target_include_directories(pocketfft SYSTEM INTERFACE "${EXTERNAL_ROOT}")
    target_compile_definitions(pocketfft INTERFACE POCKETFFT_NO_MULTITHREADING)
    message(STATUS "[pocketfft] Ziel 'pocketfft' angelegt (${EXTERNAL_ROOT})")
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE pocketfft)
message(STATUS "[pocketfft] an ${EXECUTABLE_NAME} gebunden")
