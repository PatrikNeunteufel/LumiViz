# externals/signalsmith-stretch/Include.cmake
# ===========================================
# Signalsmith Stretch 1.4.0 mit Signalsmith Linear 0.6.4 (nur stft.h und fft.h) — nur Header,
# als INTERFACE-Ziel aus diesem Ordner. Dehnen und Tonhoehe eines Wave-Musters hinter
# lumi::loop::Dehner (Loop_Konzept.md §4p, WV6 a; Stufe 15f).
#
# Diese Datei liegt IM PROJEKT, nicht in CMakeCraft: Solution.json nennt sie
# ueber das Feld "include" des Externals (Muster externals/pocketfft).
#
# Der Ordner traegt die Header so, dass signalsmith-stretch.h sein
# #include "signalsmith-linear/stft.h" ohne die Weiterleitungen aus dem Repository
# von Linear findet. Keine SIGNALSMITH_USE_*-Schalter: Linear rechnet mit der
# eigenen FFT, auf jedem Rechner gleich.
#
# Erwartete Variablen (setzt der Orchestrator von CMakeCraft):
#   EXTERNAL_NAME    - "signalsmith-stretch"
#   EXTERNAL_ROOT    - Pfad zu externals/signalsmith-stretch
#   EXECUTABLE_NAME  - das Ziel, an das gebunden wird
#
# Herkunft, Fassung und Pruefsummen: README.md in diesem Ordner.
# Kein include_guard(): die Datei wird je Ziel eingebunden.

if(NOT DEFINED EXTERNAL_ROOT)
    set(EXTERNAL_ROOT "${CMAKE_SOURCE_DIR}/externals/signalsmith-stretch")
endif()

foreach(_datei signalsmith-stretch.h signalsmith-linear/stft.h signalsmith-linear/fft.h)
    if(NOT EXISTS "${EXTERNAL_ROOT}/${_datei}")
        message(FATAL_ERROR "[signalsmith-stretch] ${_datei} fehlt in ${EXTERNAL_ROOT} — siehe README.md dort")
    endif()
endforeach()

if(NOT TARGET signalsmith-stretch)
    add_library(signalsmith-stretch INTERFACE)
    # SYSTEM: fremder Code, seine Warnungen gehoeren nicht zu uns
    target_include_directories(signalsmith-stretch SYSTEM INTERFACE "${EXTERNAL_ROOT}")
    message(STATUS "[signalsmith-stretch] Ziel 'signalsmith-stretch' angelegt (${EXTERNAL_ROOT})")
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE signalsmith-stretch)
message(STATUS "[signalsmith-stretch] an ${EXECUTABLE_NAME} gebunden")
