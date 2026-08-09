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
# Name und Version an den Code weiterreichen (S73)
# ==============================================================================
#
# SSOT ist Solution.json. Bis S73 standen beide Werte DREIFACH: dort, hart in
# Application.cpp und als Literal "Version 0.1.0" im About-Dialog — der
# About-Dialog war entsprechend jahrelang falsch.
#
# Gelesen wird hier direkt aus der Datei, nicht ueber CMakeCraft-Interna:
# CMakeCraft setzt die Version zwar als Target-Eigenschaft, erzeugt aber keine
# Kopfdatei (kein configure_file im ganzen Build-System). Der direkte Weg macht
# Solution.json unmissverstaendlich zur einzigen Quelle.
# ------------------------------------------------------------------------------

file(READ "${CMAKE_CURRENT_LIST_DIR}/Solution.json" _lumi_solution_json)
string(JSON _lumi_app_name  GET "${_lumi_solution_json}" "apps" 0 "name")
string(JSON _lumi_app_ver   GET "${_lumi_solution_json}" "apps" 0 "version")

if(_lumi_app_name STREQUAL "" OR _lumi_app_ver STREQUAL "")
    message(FATAL_ERROR
        "[AppInfo] Name oder Version fehlen in Solution.json (apps[0]) — "
        "ohne sie kann der Code sie nicht anzeigen.")
endif()

message(STATUS "[AppInfo] ${_lumi_app_name} ${_lumi_app_ver} (aus Solution.json)")

foreach(_t IN ITEMS "${_lumi_app_name}" "${_lumi_app_name}.Core")
    if(TARGET "${_t}")
        target_compile_definitions("${_t}" PRIVATE
            LUMI_APP_NAME="${_lumi_app_name}"
            LUMI_APP_VERSION="${_lumi_app_ver}")
    endif()
endforeach()

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

if(TARGET LumiViz AND EXISTS "${_lumi_presets_src}")
    add_custom_command(TARGET LumiViz POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${_lumi_presets_src}"
                "$<TARGET_FILE_DIR:LumiViz>/presets"
        # Die Pflegenotiz richtet sich an Mitwirkende, nicht an Anwender —
        # neben der Exe hat sie nichts zu suchen.
        COMMAND ${CMAKE_COMMAND} -E rm -f
                "$<TARGET_FILE_DIR:LumiViz>/presets/README.md"
        COMMENT "[Presets] asset/presets -> $<TARGET_FILE_DIR:LumiViz>/presets"
        VERBATIM)
    message(STATUS "[Presets] Mitgelieferte Presets werden nach dem Build kopiert")

    # --------------------------------------------------------------------------
    # Anwender-Dokumentation neben die Exe (S73)
    # --------------------------------------------------------------------------
    # SSOT bleibt projects/apps/LumiViz/docs/ — hier entstehen nur KOPIEN.
    # Wer den Build weitergibt, gibt die Anleitungen mit; wer `out/` loescht,
    # verliert nichts.
    #
    # Bewusst NUR anwendertaugliches: die Werkzeug-Wegleitung richtet sich an
    # Entwickler (Debug-Builds, Referenz-Renderer) und bleibt drum draussen.
    set(_lumi_doc_src "${CMAKE_CURRENT_LIST_DIR}/projects/apps/LumiViz/docs")
    set(_lumi_docs
        "Benutzerhandbuch.md"
        "Preset_Quickstart.md"
        "Preset_Anleitung.md")

    foreach(_d IN LISTS _lumi_docs)
        if(NOT EXISTS "${_lumi_doc_src}/${_d}")
            message(WARNING "[Docs] ${_d} fehlt — wird NICHT kopiert")
            continue()
        endif()
        add_custom_command(TARGET LumiViz POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_lumi_doc_src}/${_d}"
                    "$<TARGET_FILE_DIR:LumiViz>/docs/${_d}"
            COMMENT "[Docs] ${_d}"
            VERBATIM)
    endforeach()
    message(STATUS "[Docs] Anwender-Dokumentation wird nach dem Build kopiert")
elseif(NOT TARGET LumiViz)
    message(WARNING "[Presets] Target LumiViz nicht gefunden — Presets werden NICHT kopiert")
else()
    message(WARNING "[Presets] ${_lumi_presets_src} fehlt — Presets werden NICHT kopiert")
endif()
