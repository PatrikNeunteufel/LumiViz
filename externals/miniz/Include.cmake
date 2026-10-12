# externals/miniz/Include.cmake
# =============================
# miniz (Zip lesen und schreiben) — als statische Bibliothek aus den beiden
# Quelldateien dieses Ordners.
#
# Diese Datei liegt IM PROJEKT, nicht in CMakeCraft: Solution.json nennt sie
# ueber das Feld "include" des Externals (CMakeCraft, Solution_Schema §5.2,
# "Custom Include"). So kommt ein lokales External dazu, ohne CMakeCraft zu
# aendern (Entscheid Patrik S114, E2 b). Bewaehrt sie sich, geht sie als
# Anleitung an CMakeCraft.
#
# Erwartete Variablen (setzt der Orchestrator von CMakeCraft):
#   EXTERNAL_NAME    - "miniz"
#   EXTERNAL_ROOT    - Pfad zu externals/miniz
#   EXECUTABLE_NAME  - das Ziel, an das gebunden wird
#
# Herkunft, Fassung und Pruefsumme: README.md in diesem Ordner.
# Kein include_guard(): die Datei wird je Ziel eingebunden.

if(NOT DEFINED EXTERNAL_ROOT)
    set(EXTERNAL_ROOT "${CMAKE_SOURCE_DIR}/externals/miniz")
endif()

if(NOT EXISTS "${EXTERNAL_ROOT}/miniz.c" OR NOT EXISTS "${EXTERNAL_ROOT}/miniz.h")
    message(FATAL_ERROR "[miniz] miniz.c / miniz.h fehlen in ${EXTERNAL_ROOT} — siehe README.md dort")
endif()

if(NOT TARGET miniz)
    add_library(miniz STATIC "${EXTERNAL_ROOT}/miniz.c")
    target_include_directories(miniz PUBLIC "${EXTERNAL_ROOT}")
    # LumiViz reicht miniz nur Speicher, nie einen Dateinamen: die Datei-
    # Funktionen von miniz oeffnen mit fopen und kaemen unter Windows mit
    # Umlauten im Pfad nicht zurecht. Ohne stdio koennen sie nicht versehentlich
    # benutzt werden.
    target_compile_definitions(miniz PUBLIC MINIZ_NO_STDIO)
    # fremder Code: keine Warnungen
    if(MSVC)
        target_compile_options(miniz PRIVATE /W0)
    else()
        target_compile_options(miniz PRIVATE -w)
    endif()
    message(STATUS "[miniz] Bibliothek 'miniz' angelegt (${EXTERNAL_ROOT})")
endif()

target_link_libraries(${EXECUTABLE_NAME} PRIVATE miniz)
message(STATUS "[miniz] an ${EXECUTABLE_NAME} gebunden")
