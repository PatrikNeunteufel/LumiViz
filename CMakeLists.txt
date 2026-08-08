# ==============================================================================
# CMakeLists.txt – LumiViz (Konsument von CMakeCraft)
# ==============================================================================
#
# Das Build-System (CMakeCraft) liegt NICHT mehr im Repo: Es wird beim
# Configure in der gepinnten Version (cmakecraft.pin) nach .externals/
# geholt. Was gebaut wird, definiert ausschliesslich Solution.json.
#
#   Version wechseln:            cmakecraft.pin editieren
#   Build-System entwickeln:     -DCMAKECRAFT_LOCAL_DIR=../CMakeCraft
#   Doku:                        CMakeCraft/docs/de/guide/userguides/Neues_Projekt_Guide.md
#
# ==============================================================================

cmake_minimum_required(VERSION 3.25)

include("${CMAKE_CURRENT_LIST_DIR}/CMakeCraftBootstrap.cmake")

# ==============================================================================
# Mitgelieferte Presets (S73)
# ==============================================================================
#
# `asset/presets/` wandert nach jedem Build neben die Exe, damit die Anwendung
# ohne Vorbereitung etwas zu zeigen hat. Die Auswahl ist bewusst klein und
# besteht ausschliesslich aus EIGENEN Werken — fremde Presets duerfen hier
# nicht hinein (siehe THIRD_PARTY_NOTICES.md, "Nicht enthaltene Inhalte").
#
# WEITERE PRESETS AUFNEHMEN: Anleitung in asset/presets/README.md.
# Diese Datei muss dafuer NICHT angefasst werden — es wird der ganze Ordner
# kopiert, ein neuer Unterordner genuegt.
#
# Warum POST_BUILD und nicht install(): der uebliche Weg ist Starten aus dem
# Build-Ordner; install() liefe dabei nie.
# ------------------------------------------------------------------------------

set(_lumi_presets_src "${CMAKE_CURRENT_LIST_DIR}/asset/presets")

if(TARGET MyViz AND EXISTS "${_lumi_presets_src}")
    add_custom_command(TARGET MyViz POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${_lumi_presets_src}"
                "$<TARGET_FILE_DIR:MyViz>/presets"
        # Die Pflegenotiz richtet sich an Mitwirkende, nicht an Anwender —
        # neben der Exe hat sie nichts zu suchen.
        COMMAND ${CMAKE_COMMAND} -E rm -f
                "$<TARGET_FILE_DIR:MyViz>/presets/README.md"
        COMMENT "[Presets] asset/presets -> $<TARGET_FILE_DIR:MyViz>/presets"
        VERBATIM)
    message(STATUS "[Presets] Mitgelieferte Presets werden nach dem Build kopiert")
elseif(NOT TARGET MyViz)
    message(WARNING "[Presets] Target MyViz nicht gefunden — Presets werden NICHT kopiert")
else()
    message(WARNING "[Presets] ${_lumi_presets_src} fehlt — Presets werden NICHT kopiert")
endif()
