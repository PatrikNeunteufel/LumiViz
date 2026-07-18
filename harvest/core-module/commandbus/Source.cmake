# Files direkt in src/
set(_src_core_commandbus_local_sources
    ${CMAKE_CURRENT_LIST_DIR}/CommandHistory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandBus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CompositeCommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TransactionGuard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ICommandBus.cpp
)
set(_src_core_commandbus_local_headers
    ${CMAKE_CURRENT_LIST_DIR}/CommandBusEvents.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandKey.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandResult.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandContext.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ICommand.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandEvents.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ICommandBus.hpp
    # Registry + Dispatcher (templated in .tpp, thin headers in .hpp)
    ${CMAKE_CURRENT_LIST_DIR}/CommandRegistry.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandDispatcher.hpp
    # History & Bus (nicht-templated Kern in .cpp)
    ${CMAKE_CURRENT_LIST_DIR}/CommandHistory.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandBus.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CompositeCommand.hpp
    # Adapter für class-basierte Commands (optional)
    ${CMAKE_CURRENT_LIST_DIR}/TCommandAdapter.hpp
    ${CMAKE_CURRENT_LIST_DIR}/TransactionGuard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CoalescingPolicy.hpp
)
set(_src_core_commandbus_local_templates
    ${CMAKE_CURRENT_LIST_DIR}/CommandRegistry.tpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandDispatcher.tpp
    ${CMAKE_CURRENT_LIST_DIR}/TCommandAdapter.tpp
    ${CMAKE_CURRENT_LIST_DIR}/CoalescingPolicy.tpp
)

# Subfolder rekursiv einbinden
# include("${CMAKE_CURRENT_LIST_DIR}/app/Source.cmake")

# Nach oben aggregieren
list(APPEND PROJECT_SOURCES ${_src_core_commandbus_local_sources})
list(APPEND PROJECT_HEADERS ${_src_core_commandbus_local_headers} ${_src_core_commandbus_local_templates})

# optional aufräumen (rein kosmetisch)
unset(_src_core_commandbus_local_sources)
unset(_src_core_commandbus_local_headers)
unset(_src_core_commandbus_local_templates)
