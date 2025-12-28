# ==============================================================================
# Source.cmake for tests/
# CMake Architecture V2 - Source Collection (v0.6)
# ==============================================================================
# Location: projects/apps/{AppName}/tests/{type}/{TestName}/Source.cmake
#       or: projects/tests/{type}/{TestName}/src/Source.cmake
# Target:   ${TARGET_NAME} (e.g. MyApp.UnitTests, BasicLogger_UnitTests)
#
# Test source files for unit, integration, or performance tests.
# ==============================================================================

dbg(${DBG_OFTEN}
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# ==============================================================================
# Local file lists for THIS directory
# ==============================================================================

set(_local_sources
    # Test entry point (*.c, *.cpp, *.cxx, *.cc)
    # Framework-specific:
    #   - doctest:     DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN or test_main.cpp
    #   - googletest:  link gtest_main/gmock_main or test_main.cpp
    #   - catch2:      link Catch2WithMain or test_main.cpp
    "${CMAKE_CURRENT_LIST_DIR}/test_main.cpp"
    
    # Test files
    "${CMAKE_CURRENT_LIST_DIR}/test_Application.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/test_Config.cpp"
    # "${CMAKE_CURRENT_LIST_DIR}/test_Utils.cpp"
)

set(_local_headers
    # Test utilities / fixtures (*.h, *.hpp, *.hxx, *.hh)
    # "${CMAKE_CURRENT_LIST_DIR}/TestFixtures.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/MockObjects.hpp"
    # "${CMAKE_CURRENT_LIST_DIR}/TestHelpers.hpp"
)

set(_local_templates
    # (usually no templates in tests - *.tpp, *.txx, *.ipp)
)

set(_local_inlines
    # (usually no inlines in tests - *.inl)
)

set(_local_impl
    # (usually no impl in tests - *.impl)
)

# ==============================================================================
# Debug: Show found files
# ==============================================================================

dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)

# ==============================================================================
# Aggregate to TARGET variables
# ==============================================================================

list(APPEND ${TARGET_NAME}_SOURCES   ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS   ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES   ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL      ${_local_impl})

# ==============================================================================
# Debug: Show aggregated totals
# ==============================================================================

dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated SOURCES  : ${${TARGET_NAME}_SOURCES}" ID DEB_AGG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated HEADERS  : ${${TARGET_NAME}_HEADERS}" ID DEB_AGG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated TEMPLATES: ${${TARGET_NAME}_TEMPLATES}" ID DEB_AGG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated INLINES  : ${${TARGET_NAME}_INLINES}" ID DEB_AGG)
dbg(${DBG_NORMAL} "[${TARGET_NAME}] Aggregated IMPL     : ${${TARGET_NAME}_IMPL}" ID DEB_AGG)

# ==============================================================================
# Cleanup local variables
# ==============================================================================

unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)

# ==============================================================================
# Include test subfolders (for organized test suites)
# ==============================================================================
#
# Typical structure for large test suites:
#   tests/unit/UnitTests/
#   ├── Source.cmake        ← This file
#   ├── test_main.cpp
#   ├── core/
#   │   ├── Source.cmake
#   │   └── test_Engine.cpp
#   └── utils/
#       ├── Source.cmake
#       └── test_StringUtils.cpp
#
# ==============================================================================

dbg(${DBG_ULTRA_RARE} "[${TARGET_NAME}] Including subfolders:" ID INCLUDE_MSG)

# Uncomment and adapt as needed:
# include("${CMAKE_CURRENT_LIST_DIR}/core/Source.cmake")
# include("${CMAKE_CURRENT_LIST_DIR}/utils/Source.cmake")
# include("${CMAKE_CURRENT_LIST_DIR}/fixtures/Source.cmake")
