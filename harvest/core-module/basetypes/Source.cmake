# Files direkt in src/
set(_src_core_basetypes_local_sources
    ${CMAKE_CURRENT_LIST_DIR}/BaseController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BaseManager.cpp
    # (keine Quellen)
)
set(_src_core_basetypes_local_headers
    ${CMAKE_CURRENT_LIST_DIR}/BaseController.hpp
    ${CMAKE_CURRENT_LIST_DIR}/BaseManager.hpp
    # ${CMAKE_CURRENT_LIST_DIR}/BaseRegistry.hpp
    # ${CMAKE_CURRENT_LIST_DIR}/BaseAgent.hpp
    # ${CMAKE_CURRENT_LIST_DIR}/BaseCooperativeAgent.hpp
    # ${CMAKE_CURRENT_LIST_DIR}/BaseDedicatedAgent.hpp
    # (keine Header)
)
set(_src_core_basetypes_local_templates
    ${CMAKE_CURRENT_LIST_DIR}/BaseController.tpp
    # (keine templates)
)

# Subfolder rekursiv einbinden
# include("${CMAKE_CURRENT_LIST_DIR}/app/Source.cmake")

# Nach oben aggregieren
list(APPEND PROJECT_SOURCES ${_src_core_basetypes_local_sources})
list(APPEND PROJECT_HEADERS ${_src_core_basetypes_local_headers} ${_src_core_basetypes_local_templates})

# optional aufräumen (rein kosmetisch)
unset(_src_core_basetypes_local_sources)
unset(_src_core_basetypes_local_headers)
unset(_src_core_basetypes_local_templates)
