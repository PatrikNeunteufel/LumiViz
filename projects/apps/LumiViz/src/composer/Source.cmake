# ==============================================================================
# Source.cmake for src/composer/
# CMake Architecture V2 - App-Container Template
# ==============================================================================
# Composer (docs/ui/Composer_Konzept.md): eigener Code, rueckbaubar — dieses
# Verzeichnis und include/composer loeschen, die include()-Zeile in
# src/Source.cmake nehmen.
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# Set local lists for this directory
set(_local_sources
    "${CMAKE_CURRENT_LIST_DIR}/AppAnbindung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Bearbeiten.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Bildquelle.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ComposerPanel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Komposition.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Mischer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Pegelform.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Sitzung.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Wiedergabe.cpp"
)
set(_local_headers
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/AppAnbindung.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/Bearbeiten.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/Bildquelle.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/ComposerPanel.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/Komposition.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/Mischer.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/Pegelform.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/Sitzung.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/Uhr.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../include/composer/Wiedergabe.hpp"
)
set(_local_templates
    # (no templates - *.t; *.tpp)
)
set(_local_inlines
    # (no inlines - *.inl)
)
set(_local_impl
    # (no impl - *.impl)
)
set(_local_modules
    # (no modules - *.cxx)
)
set(_local_includes
    # (no impl - *.?)
)

dbg(DBG_NORMAL "Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found modules  : ${_local_modules}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found includes : ${_local_includes}" ID DEB_FOUND_MSG)

# Aggregate to parent scope
list(APPEND ${TARGET_NAME}_SOURCES ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL ${_local_impl})
list(APPEND ${TARGET_NAME}_MODULES ${_local_modules})
list(APPEND ${TARGET_NAME}_INCLUDES ${_local_includes})

# Optional cleanup (cosmetic)
unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)
unset(_local_modules)
unset(_local_includes)
