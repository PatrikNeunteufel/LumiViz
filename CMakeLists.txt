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

# Pflicht seit CMakeCraft v0.13.0 (TC1 a, S149): ohne ein ausgeschriebenes project()
# setzt CMake selbst project(Project) vor cmake_minimum_required, die alte Regel
# CMP0091 gilt, und die Konfiguration Testing baut unter MSVC mit /MT statt /MD.
# Name und Version kommen weiter aus Solution.json (CMakeCraft ruft project() dort).
project(LumiViz LANGUAGES NONE)

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
# Mitgeliefertes neben die Exe (S73 Presets; seit S114 alle Bereiche)
# ==============================================================================
#
# WAS neben die Exe kommt, steht in asset/Asset.cmake — dort eintragen, nicht
# hier. Steuerdokument: projects/apps/LumiViz/docs/core-services/
# Datenablage_Konzept.md (Entscheide Patrik S114).
#
# Das Ziel LumiViz_Assets gleicht bei JEDEM Bau von LumiViz ab (kopiert
# Geaendertes, entfernt am Ziel, was nicht mehr dazugehoert). Bis S114 lief
# die Kopie von asset/presets als Schritt nach dem Bau — also nur, wenn die
# Exe neu gelinkt wurde.
#
# Mitgeliefert werden ausschliesslich EIGENE Werke (THIRD_PARTY_NOTICES.md,
# "Nicht enthaltene Inhalte").
#
# Warum ein Ziel im Bau und nicht install(): der uebliche Weg ist Starten aus
# dem Build-Ordner; install() liefe dabei nie.
# ------------------------------------------------------------------------------

set(_lumi_asset_liste "${CMAKE_CURRENT_LIST_DIR}/asset/Asset.cmake")

if(TARGET LumiViz AND EXISTS "${_lumi_asset_liste}")
    add_custom_target(LumiViz_Assets
        COMMAND ${CMAKE_COMMAND}
                "-DLUMI_ASSET_QUELLE=${CMAKE_CURRENT_LIST_DIR}/asset"
                "-DLUMI_ASSET_ZIEL=$<TARGET_FILE_DIR:LumiViz>"
                -P "${CMAKE_CURRENT_LIST_DIR}/asset/AssetAbgleich.cmake"
        COMMENT "[Asset] asset/Asset.cmake -> $<TARGET_FILE_DIR:LumiViz>"
        VERBATIM)
    add_dependencies(LumiViz LumiViz_Assets)
    message(STATUS "[Asset] Mitgeliefertes wird bei jedem Bau von LumiViz abgeglichen (asset/Asset.cmake)")

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

    # --------------------------------------------------------------------------
    # Lizenzen neben die Exe (S73) — PFLICHT bei Binaer-Weitergabe
    # --------------------------------------------------------------------------
    # Wer den Build-Ordner als ZIP weitergibt, verbreitet ein Binaerprogramm.
    # Damit greifen Pflichten, die beim reinen Quelltext nicht galten:
    #
    #   - Nullsoft BSD-3 (portierter AVS-/MilkDrop-Code steckt in der Exe):
    #     "Redistributions in binary form must reproduce the above copyright
    #     notice ... in the documentation and/or other materials provided with
    #     the distribution." Der Vermerk MUSS also mitgeliefert werden.
    #   - Qt und die von Qt mitgelieferten FFmpeg-Bibliotheken (LGPL):
    #     Lizenztext beilegen, Quellen benennen, Relinken ermoeglichen.
    #   - BASS (un4seen): Weitergabe der DLL mit der Anwendung ist gedeckt,
    #     solange die Nutzung nicht-kommerziell bleibt.
    #
    # Darum liegen diese vier Dateien ab jetzt IMMER neben der Exe — nicht
    # als Kuer, sondern damit ein ZIP aus dem Build-Ordner vollstaendig ist.
    set(_lumi_lizenzen
        "LICENSE"
        "LICENSE-MIT"
        "LICENSE-APACHE"
        "THIRD_PARTY_NOTICES.md")

    foreach(_l IN LISTS _lumi_lizenzen)
        if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${_l}")
            message(WARNING "[Lizenz] ${_l} fehlt — das Binaerpaket waere unvollstaendig")
            continue()
        endif()
        add_custom_command(TARGET LumiViz POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_CURRENT_LIST_DIR}/${_l}"
                    "$<TARGET_FILE_DIR:LumiViz>/${_l}"
            COMMENT "[Lizenz] ${_l}"
            VERBATIM)
    endforeach()
    message(STATUS "[Lizenz] Lizenzdateien werden nach dem Build kopiert")
elseif(NOT TARGET LumiViz)
    message(WARNING "[Asset] Target LumiViz nicht gefunden — Mitgeliefertes wird NICHT kopiert")
else()
    message(WARNING "[Asset] ${_lumi_asset_liste} fehlt — Mitgeliefertes wird NICHT kopiert")
endif()

# ------------------------------------------------------------------------------
# Verknuepfung "LumiViz.exe - Testing" neben der Exe
# ------------------------------------------------------------------------------
# Composer-Naht N25 (projects/apps/LumiViz/docs/ui/Composer_Konzept.md §12.2;
# S105, Wunsch Patrik): nach jedem Bau der Exe liegt neben ihr eine
# Verknuepfung, die LumiViz mit `--testing` startet — also mit der
# Sichttest-Steuerung, die den Tester Sichttest.exe mitbringt. Sie entsteht in
# jedem Build-Ordner neu und zeigt auf die Exe ihres Ordners.
#
# Nur Windows (eine .lnk gibt es nur dort; das Paket der Steuerung auch).
# Scheitert das Anlegen, warnt das Skript und der Bau laeuft weiter.
# ------------------------------------------------------------------------------

set(_lumi_testing_lnk "${CMAKE_CURRENT_LIST_DIR}/tools/sichttest/TestingVerknuepfung.ps1")

if(WIN32 AND TARGET LumiViz AND EXISTS "${_lumi_testing_lnk}")
    add_custom_command(TARGET LumiViz POST_BUILD
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass
                -File "${_lumi_testing_lnk}"
                -Exe "$<TARGET_FILE:LumiViz>"
        COMMENT "[Sichttest] LumiViz.exe - Testing.lnk"
        VERBATIM)
    message(STATUS "[Sichttest] Verknuepfung 'LumiViz.exe - Testing' wird nach dem Build angelegt")
endif()
