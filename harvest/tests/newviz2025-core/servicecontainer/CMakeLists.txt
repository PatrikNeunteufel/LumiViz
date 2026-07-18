# tests/core/servicecontainer/CMakeLists.txt
# Tests für viz::core::ServiceContainer

cmake_minimum_required(VERSION 3.21)

# 1) Executable
add_executable(servicecontainer_tests
    test_servicecontainer.cpp
)

# 2) ServiceContainer-Implementierung direkt mitkompilieren (da keine separate Lib)
target_sources(servicecontainer_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceContainer.cpp
    ${CMAKE_SOURCE_DIR}/src/core/servicecontainer/ServiceScope.cpp
)

# 3) Includes (für core/… und common/…)
target_include_directories(servicecontainer_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 4) Eigenschaften / Optionen wie im Projekt
target_compile_features(servicecontainer_tests PRIVATE cxx_std_20)

# 5) CTest-Integration: nutzt die echte, konfig-spezifische Exe (Multi-Config sicher)
add_test(NAME servicecontainer_tests
         COMMAND $<TARGET_FILE:servicecontainer_tests>)

# (Optional) Falls du Umgebungsvariablen brauchst:
# set_tests_properties(servicecontainer_tests PROPERTIES
#     ENVIRONMENT "MY_VAR=VALUE"
# )
