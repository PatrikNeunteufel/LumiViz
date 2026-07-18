cmake_minimum_required(VERSION 3.21)
project(basetypes_controller_tests LANGUAGES CXX)

add_executable(basetypes_controller_tests
    test_BaseController.cpp

    # ---- Production-Quellen, die BaseController/CommandBus benötigen ----
    ${CMAKE_SOURCE_DIR}/src/core/basetypes/BaseController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/ICommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/TransactionGuard.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CompositeCommand.cpp   # <-- NEU
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceScope.cpp
)

target_include_directories(basetypes_controller_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(basetypes_controller_tests PRIVATE cxx_std_20)

# Catch2 (find_package mit Fallback)
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(catch2)
endif()
target_link_libraries(basetypes_controller_tests PRIVATE Catch2::Catch2WithMain)

if(MSVC)
  target_compile_options(basetypes_controller_tests PRIVATE /Zc:preprocessor /Zc:__cplusplus /permissive-)
endif()

enable_testing()
add_test(NAME basetypes_controller_tests COMMAND $<TARGET_FILE:basetypes_controller_tests>)


# ####################################################################################################################

# --- NEUES Target: basetypes_controller_async_tests ---
add_executable(basetypes_controller_async_tests
    test_BaseController_async.cpp

    # Produktions-Quellen (identisch zum ersten Target)
    ${CMAKE_SOURCE_DIR}/src/core/basetypes/BaseController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/ICommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/TransactionGuard.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CompositeCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceScope.cpp
)

target_include_directories(basetypes_controller_async_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(basetypes_controller_async_tests PRIVATE cxx_std_20)

# Catch2 wie oben
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(catch2)
endif()
target_link_libraries(basetypes_controller_async_tests PRIVATE Catch2::Catch2WithMain)

if(MSVC)
  target_compile_options(basetypes_controller_async_tests PRIVATE /Zc:preprocessor /Zc:__cplusplus /permissive-)
endif()

add_test(NAME basetypes_controller_async_tests COMMAND $<TARGET_FILE:basetypes_controller_async_tests>)


# ####################################################################################################################

# --- NEUES Target: basetypes_controller_sync_tests ---
add_executable(basetypes_controller_sync_tests
    test_BaseController_sync.cpp

    # Produktions-Quellen (identisch zu den anderen basetypes_* Targets)
    ${CMAKE_SOURCE_DIR}/src/core/basetypes/BaseController.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/ICommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/TransactionGuard.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CompositeCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceScope.cpp
)

target_include_directories(basetypes_controller_sync_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(basetypes_controller_sync_tests PRIVATE cxx_std_20)

find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(catch2)
endif()
target_link_libraries(basetypes_controller_sync_tests PRIVATE Catch2::Catch2WithMain)

if(MSVC)
  target_compile_options(basetypes_controller_sync_tests PRIVATE /Zc:preprocessor /Zc:__cplusplus /permissive-)
endif()

add_test(NAME basetypes_controller_sync_tests COMMAND $<TARGET_FILE:basetypes_controller_sync_tests>)


# ####################################################################################################################

# --- Target: basetypes_manager_tests ---
add_executable(basetypes_manager_tests
    test_BaseManager.cpp

    # Produktions-Quellen (BaseManager + Services/EventBus + CommandBus)
    ${CMAKE_SOURCE_DIR}/src/core/basetypes/BaseManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceScope.cpp

    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/ICommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/TransactionGuard.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CompositeCommand.cpp
)

target_include_directories(basetypes_manager_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(basetypes_manager_tests PRIVATE cxx_std_20)

find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(catch2)
endif()
target_link_libraries(basetypes_manager_tests PRIVATE Catch2::Catch2WithMain)

if(MSVC)
  target_compile_options(basetypes_manager_tests PRIVATE /Zc:preprocessor /Zc:__cplusplus /permissive-)
endif()

add_test(NAME basetypes_manager_tests COMMAND $<TARGET_FILE:basetypes_manager_tests>)



# ####################################################################################################################

# --- Registry tests ---
add_executable(basetypes_registry_tests
    ${CMAKE_CURRENT_LIST_DIR}/test_BaseRegistry.cpp

    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceScope.cpp

    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/ICommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/TransactionGuard.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CompositeCommand.cpp
)
target_include_directories(basetypes_registry_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(basetypes_registry_tests PRIVATE cxx_std_20)

# Catch2 (wie bei den anderen Targets)
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(catch2)
endif()
target_link_libraries(basetypes_registry_tests PRIVATE Catch2::Catch2WithMain)

enable_testing()
add_test(NAME basetypes_registry_tests COMMAND $<TARGET_FILE:basetypes_registry_tests>)


# ####################################################################################################################

# --- BaseAgent: unit tests (header-only base) ---

add_executable(basetypes_agent_base_tests
    ${CMAKE_CURRENT_LIST_DIR}/test_BaseAgent.cpp

    # Runtime-Abhängigkeiten, damit die Default-Konstruktoren/Methoden aufgelöst werden
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceScope.cpp

    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/ICommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandHistory.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/TransactionGuard.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CompositeCommand.cpp
)

target_include_directories(basetypes_agent_base_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(basetypes_agent_base_tests PRIVATE cxx_std_20)

# Catch2 einbinden (gleich wie bei den anderen Targets)
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(catch2)
endif()

target_link_libraries(basetypes_agent_base_tests PRIVATE Catch2::Catch2WithMain)

add_test(NAME basetypes_agent_base_tests COMMAND $<TARGET_FILE:basetypes_agent_base_tests>)
