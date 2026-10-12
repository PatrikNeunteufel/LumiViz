# ==============================================================================
# AssetAbgleich.cmake — fuehrt asset/Asset.cmake aus (cmake -P, bei jedem Bau)
# ==============================================================================
#
#   cmake -DLUMI_ASSET_QUELLE=<Repo>/asset -DLUMI_ASSET_ZIEL=<Ordner der Exe>
#         -P asset/AssetAbgleich.cmake
#
# Je Eintrag der Liste: kopieren, was fehlt oder sich geaendert hat; am Ziel
# entfernen, was nicht dazugehoert; leere Ordner aufraeumen. Angelegt und
# geloescht wird nur in den Zielordnern der Eintraege.
#
# WAS mitkommt, steht in Asset.cmake — nicht hier.
# ==============================================================================

cmake_minimum_required(VERSION 3.25)

if(NOT LUMI_ASSET_QUELLE OR NOT IS_DIRECTORY "${LUMI_ASSET_QUELLE}")
    message(FATAL_ERROR "[Asset] LUMI_ASSET_QUELLE fehlt oder ist kein Ordner: '${LUMI_ASSET_QUELLE}'")
endif()
if(NOT LUMI_ASSET_ZIEL)
    message(FATAL_ERROR "[Asset] LUMI_ASSET_ZIEL fehlt")
endif()

set(_lumi_asset_dateien 0)
set(_lumi_asset_kopiert 0)
set(_lumi_asset_entfernt 0)

function(lumi_asset bereich)
    cmake_parse_arguments(PARSE_ARGV 1 A "" "NACH" "NUR;OHNE")
    if(A_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[Asset] ${bereich}: unbekannte Angabe '${A_UNPARSED_ARGUMENTS}'")
    endif()

    set(quelle "${LUMI_ASSET_QUELLE}/${bereich}")
    if(NOT IS_DIRECTORY "${quelle}")
        message(WARNING "[Asset] ${bereich}: asset/${bereich} fehlt — nichts kopiert")
        return()
    endif()
    if(A_NACH)
        set(ziel "${LUMI_ASSET_ZIEL}/${A_NACH}")
    else()
        set(ziel "${LUMI_ASSET_ZIEL}/asset/${bereich}")
    endif()

    # --- was dazugehoert (Pfade relativ zum Bereich)
    set(soll "")
    if(A_NUR)
        foreach(muster IN LISTS A_NUR)
            file(GLOB treffer RELATIVE "${quelle}" "${quelle}/${muster}")
            list(APPEND soll ${treffer})
        endforeach()
    else()
        file(GLOB_RECURSE soll RELATIVE "${quelle}" "${quelle}/*")
    endif()
    set(dateien "")
    foreach(rel IN LISTS soll)
        if(IS_DIRECTORY "${quelle}/${rel}")
            continue()
        endif()
        get_filename_component(name "${rel}" NAME)
        if(name IN_LIST A_OHNE)
            continue()
        endif()
        list(APPEND dateien "${rel}")
    endforeach()
    list(REMOVE_DUPLICATES dateien)
    if(NOT dateien)
        message(WARNING "[Asset] ${bereich}: kein Treffer — Muster in asset/Asset.cmake pruefen")
    endif()

    # --- kopieren, was fehlt oder anders ist
    set(kopiert 0)
    foreach(rel IN LISTS dateien)
        set(neu TRUE)
        if(EXISTS "${ziel}/${rel}")
            file(SHA1 "${quelle}/${rel}" a)
            file(SHA1 "${ziel}/${rel}" b)
            if(a STREQUAL b)
                set(neu FALSE)
            endif()
        endif()
        if(neu)
            get_filename_component(ordner "${ziel}/${rel}" DIRECTORY)
            file(MAKE_DIRECTORY "${ordner}")
            file(COPY_FILE "${quelle}/${rel}" "${ziel}/${rel}")
            math(EXPR kopiert "${kopiert} + 1")
        endif()
    endforeach()

    # --- entfernen, was am Ziel liegt und nicht dazugehoert
    set(entfernt 0)
    if(IS_DIRECTORY "${ziel}")
        file(GLOB_RECURSE ist RELATIVE "${ziel}" "${ziel}/*")
        foreach(rel IN LISTS ist)
            if(NOT rel IN_LIST dateien)
                file(REMOVE "${ziel}/${rel}")
                math(EXPR entfernt "${entfernt} + 1")
            endif()
        endforeach()
        # leere Ordner, die tiefsten zuerst
        file(GLOB_RECURSE ordnerListe LIST_DIRECTORIES true RELATIVE "${ziel}" "${ziel}/*")
        list(SORT ordnerListe ORDER DESCENDING)
        foreach(rel IN LISTS ordnerListe)
            if(IS_DIRECTORY "${ziel}/${rel}")
                file(GLOB inhalt "${ziel}/${rel}/*")
                if(NOT inhalt)
                    file(REMOVE_RECURSE "${ziel}/${rel}")
                endif()
            endif()
        endforeach()
    endif()

    list(LENGTH dateien anzahl)
    if(kopiert GREATER 0 OR entfernt GREATER 0)
        message(STATUS "[Asset] ${bereich}: ${anzahl} Dateien, ${kopiert} kopiert, ${entfernt} entfernt")
    endif()
    math(EXPR summe "${_lumi_asset_dateien} + ${anzahl}")
    math(EXPR summeK "${_lumi_asset_kopiert} + ${kopiert}")
    math(EXPR summeE "${_lumi_asset_entfernt} + ${entfernt}")
    set(_lumi_asset_dateien ${summe} PARENT_SCOPE)
    set(_lumi_asset_kopiert ${summeK} PARENT_SCOPE)
    set(_lumi_asset_entfernt ${summeE} PARENT_SCOPE)
endfunction()

include("${CMAKE_CURRENT_LIST_DIR}/Asset.cmake")

message(STATUS "[Asset] ${_lumi_asset_dateien} Dateien neben der Exe, "
               "${_lumi_asset_kopiert} kopiert, ${_lumi_asset_entfernt} entfernt")
