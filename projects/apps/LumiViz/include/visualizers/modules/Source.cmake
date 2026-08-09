# ==============================================================================
# Source.cmake for include/
# CMake Architecture V2 - App-Container Template
# ==============================================================================

dbg(DBG_OFTEN
    "${CMAKE_CURRENT_LIST_DIR}/Source.cmake
          =============================================\n" ID INCLUDE_MSG)

# Set local lists for this directory
set(_local_sources
    # (no sources - *.c; *.cpp)
)
set(_local_headers
    # (headers - *.h; *.hpp)
    "${CMAKE_CURRENT_LIST_DIR}/AudioUtil.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/ColorGradientModule.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/EqualizerModule.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/IModule.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/JsonPresetParser.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/OscilloscopeModule.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/PulseShapeModule.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/SuperscopeModule.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/WaveformModule.hpp"
)
set(_local_templates
    # (no templates - *.t; *.tpp)
)
set(_local_inlines
    # (no inlines - *.inl)
)
set(_local_impl
    # (no impl - *.impl)
)
set(_local_modules
    # (no modules - *.cxx)
)
set(_local_includes
    # (no impl - *.?)
)

dbg(DBG_NORMAL "Found sources  : ${_local_sources}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found headers  : ${_local_headers}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found templates: ${_local_templates}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found inlines  : ${_local_inlines}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found impl     : ${_local_impl}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found modules  : ${_local_modules}" ID DEB_FOUND_MSG)
dbg(DBG_NORMAL "Found includes : ${_local_includes}" ID DEB_FOUND_MSG)

# Aggregate to parent scope
list(APPEND ${TARGET_NAME}_SOURCES ${_local_sources})
list(APPEND ${TARGET_NAME}_HEADERS ${_local_headers})
list(APPEND ${TARGET_NAME}_TEMPLATES ${_local_templates})
list(APPEND ${TARGET_NAME}_INLINES ${_local_inlines})
list(APPEND ${TARGET_NAME}_IMPL ${_local_impl})
list(APPEND ${TARGET_NAME}_MODULES ${_local_modules})
list(APPEND ${TARGET_NAME}_INCLUDES ${_local_includes})

dbg(DBG_NORMAL "Aggregated sources  : ${${TARGET_NAME}_SOURCES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated headers  : ${${TARGET_NAME}_HEADERS}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated templates: ${${TARGET_NAME}_TEMPLATES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated inlines  : ${${TARGET_NAME}_INLINES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated impl     : ${${TARGET_NAME}_IMPL}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated modules  : ${${TARGET_NAME}_MODULES}" ID DEB_AGG)
dbg(DBG_NORMAL "Aggregated includes : ${${TARGET_NAME}_INCLUDES}" ID DEB_AGG)

# Optional cleanup (cosmetic)
unset(_local_sources)
unset(_local_headers)
unset(_local_templates)
unset(_local_inlines)
unset(_local_impl)
unset(_local_modules)
unset(_local_includes)

dbg(DBG_ULTRA_RARE "include subfolders:" ID INCLUDE_MSG)
# Include subfolders recursively (activate as needed)
include("${CMAKE_CURRENT_LIST_DIR}/postfx/Source.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/processing/Source.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/source/Source.cmake")
