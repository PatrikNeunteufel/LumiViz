# tests/core/commandbus/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(commandbus_tests LANGUAGES CXX)

# *** EIN FILE FÜR ALLES ***
add_executable(commandbus_tests
    test_commandbus_all.cpp
)

# Produktions-Quellen *mit* hineinlinken (damit es keine Linkerfehler gibt)
# Passe die Liste an, falls du weitere .cpp im commandbus-Ordner hast.
target_sources(commandbus_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CommandBus.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/CompositeCommand.cpp
    ${CMAKE_SOURCE_DIR}/src/core/commandbus/TransactionGuard.cpp
)

# Include-Pfade
target_include_directories(commandbus_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}   # (falls wir später doch splitten)
)

target_compile_features(commandbus_tests PRIVATE cxx_std_20)

# Catch2 wie bei eventbus: find -> FetchContent fallback
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(catch2)
endif()
target_link_libraries(commandbus_tests PRIVATE Catch2::Catch2WithMain)

# MSVC-Qualitätsflags analog
if(MSVC)
  target_compile_options(commandbus_tests PRIVATE /Zc:preprocessor /Zc:__cplusplus /permissive-)
endif()

# CTest-Registrierung Multi-Config-sicher
enable_testing()
add_test(NAME commandbus_tests COMMAND $<TARGET_FILE:commandbus_tests>)
