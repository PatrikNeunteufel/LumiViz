# tests/core/eventbus/CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Executable
add_executable(eventbus_tests
    test_eventbus.cpp
)

# Für IDE-Sichtbarkeit (Header-only + tpp ins Target einhängen)
target_sources(eventbus_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/eventbus/EventBus.hpp
    ${CMAKE_SOURCE_DIR}/src/core/eventbus/EventBus.tpp
)

# Includes
target_include_directories(eventbus_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(eventbus_tests PRIVATE cxx_std_20)

# Catch2
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(catch2)
endif()

target_link_libraries(eventbus_tests PRIVATE Catch2::Catch2WithMain)

# CTest-Eintrag
add_test(NAME eventbus_tests COMMAND $<TARGET_FILE:eventbus_tests>)
