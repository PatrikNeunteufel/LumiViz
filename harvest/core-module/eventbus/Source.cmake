# Files direkt in src/
set(_src_core_eventbus_local_sources
    # (keine Quellen)
)
set(_src_core_eventbus_local_headers
    ${CMAKE_CURRENT_LIST_DIR}/EventBus.hpp
)
set(_src_core_eventbus_local_template
    ${CMAKE_CURRENT_LIST_DIR}/EventBus.tpp
)


# Nach oben aggregieren
list(APPEND PROJECT_SOURCES ${_src_core_eventbus_local_sources})
list(APPEND PROJECT_HEADERS ${_src_core_eventbus_local_headers} ${_src_core_eventbus_local_template})

# optional aufräumen (rein kosmetisch)
unset(_src_core_eventbus_local_sources)
unset(_src_core_eventbus_local_headers)
unset(_src_core_eventbus_local_template)
