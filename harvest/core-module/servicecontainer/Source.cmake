# Files direkt in src/
set(_src_core_servicecontainer_local_sources
    ${CMAKE_CURRENT_LIST_DIR}/ServiceContainer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ServiceScope.cpp
)
set(_src_core_servicecontainer_local_headers
    ${CMAKE_CURRENT_LIST_DIR}/ServiceErrors.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ServiceLifetime.hpp
    ${CMAKE_CURRENT_LIST_DIR}/IServiceResolver.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ServiceContainer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ServiceScope.hpp
)
set(_src_core_servicecontainer_local_template
    ${CMAKE_CURRENT_LIST_DIR}/IServiceResolver.tpp
    ${CMAKE_CURRENT_LIST_DIR}/ServiceContainer.tpp
    ${CMAKE_CURRENT_LIST_DIR}/ServiceScope.tpp
)

# Subfolder rekursiv einbinden
include("${CMAKE_CURRENT_LIST_DIR}/detail/Source.cmake")

# Nach oben aggregieren
list(APPEND PROJECT_SOURCES ${_src_core_servicecontainer_local_sources})
list(APPEND PROJECT_HEADERS ${_src_core_servicecontainer_local_headers} ${_src_core_servicecontainer_local_template})

# optional aufräumen (rein kosmetisch)
unset(_src_empty_local_sources)
unset(_src_empty_local_headers)
