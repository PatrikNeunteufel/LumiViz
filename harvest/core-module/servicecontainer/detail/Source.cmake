# Files direkt in src/
set(_src_core_servicecontainer_detail_local_sources
    # (keine Quellen)
)
set(_src_core_servicecontainer_detail_local_headers
    ${CMAKE_CURRENT_LIST_DIR}/ResolutionStack.hpp
)
set(_src_core_servicecontainer_detail_local_templates
    # (keine Templates)
)

# Subfolder rekursiv einbinden
# include("${CMAKE_CURRENT_LIST_DIR}/app/Source.cmake")

# Nach oben aggregieren
list(APPEND PROJECT_SOURCES ${_src_core_servicecontainer_detail_local_sources})
list(APPEND PROJECT_HEADERS
    ${_src_core_servicecontainer_detail_local_headers}
    ${_src_core_servicecontainer_detail_local_templates}
)

# optional aufräumen (rein kosmetisch)
unset(_src_core_servicecontainer_detail_local_sources)
unset(_src_core_servicecontainer_detail_local_headers)
unset(_src_core_servicecontainer_detail_local_templates)
