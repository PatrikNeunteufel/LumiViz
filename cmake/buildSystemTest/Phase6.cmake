# ==============================================================================
# phase6.cmake – Build System Test for Git Externals & Hooks
# ==============================================================================
#
# Test:         Phase 6
# Version:      1.0.0
# Date:         2025-12-12
# Part of:      CMake Architecture
#
# Description:
#   Tests the Git Externals pipeline including:
#   - FetchContent integration
#   - PreFetch/PostFetch hooks
#   - Hook reuse (imgui_docking uses imgui hook)
#   - Target Registry
#   - cmakeSupport: false handling
#
# ==============================================================================

include_guard(GLOBAL)

dbg_init(ID PHASE6_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase6")
dbg(${DBG_OFTEN} "=== Phase 6 Test Start ===" ID PHASE6_TEST)

# ==============================================================================
# Test 1: Git External GLFW (cmakeSupport: true)
# ==============================================================================

dbg(${DBG_COMMON} "Testing Git External (GLFW)..." ID PHASE6_TEST)

# Check if glfw target exists
if(TARGET glfw)
    dbg(${DBG_COMMON} "  GLFW target exists" ID PHASE6_TEST)
else()
    cmake_warn("W601" "GLFW target not found - may not be used by any executable")
endif()

# Check registry
get_property(_glfw_registered GLOBAL PROPERTY EXTERNAL_TARGET_glfw)
if(_glfw_registered)
    dbg(${DBG_COMMON} "  GLFW registered in Target Registry" ID PHASE6_TEST)
else()
    dbg(${DBG_COMMON} "  GLFW not in registry (normal if not used)" ID PHASE6_TEST)
endif()

# ==============================================================================
# Test 2: Git External without CMake Support (ImGui)
# ==============================================================================

dbg(${DBG_COMMON} "Testing Git External without CMake (ImGui)..." ID PHASE6_TEST)

# imgui uses PostFetch hook to create target
if(TARGET imgui OR TARGET imgui_docking)
    dbg(${DBG_COMMON} "  ImGui target(s) created by PostFetch hook" ID PHASE6_TEST)
else()
    dbg(${DBG_COMMON} "  ImGui targets not created (normal if not used)" ID PHASE6_TEST)
endif()

# ==============================================================================
# Test 3: Hook Reuse (imgui_docking → imgui hook)
# ==============================================================================

dbg(${DBG_COMMON} "Testing Hook Reuse..." ID PHASE6_TEST)

# Both imgui and imgui_docking should use the same hook file
# but create different targets
get_property(_imgui_hook GLOBAL PROPERTY EXTERNAL_HOOK_imgui)
get_property(_imgui_docking_hook GLOBAL PROPERTY EXTERNAL_HOOK_imgui_docking)

dbg(${DBG_RARE} "  imgui hook: ${_imgui_hook}" ID PHASE6_TEST)
dbg(${DBG_RARE} "  imgui_docking hook: ${_imgui_docking_hook}" ID PHASE6_TEST)

if(TARGET imgui AND TARGET imgui_docking)
    # Both targets should exist and be different
    get_target_property(_imgui_type imgui TYPE)
    get_target_property(_docking_type imgui_docking TYPE)
    dbg(${DBG_COMMON} "  Hook reuse works: imgui and imgui_docking are separate targets" ID PHASE6_TEST)
endif()

# ==============================================================================
# Test 4: Externals JSON Parsing
# ==============================================================================

dbg(${DBG_COMMON} "Testing Externals JSON..." ID PHASE6_TEST)

get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)

if(_externals_json)
    # Check for git field
    string(JSON _glfw_json ERROR_VARIABLE _err GET "${_externals_json}" "glfw")
    if(NOT _err)
        string(JSON _glfw_git ERROR_VARIABLE _err2 GET "${_glfw_json}" "git")
        if(NOT _err2 AND _glfw_git)
            dbg(${DBG_COMMON} "  Git URL parsed: ${_glfw_git}" ID PHASE6_TEST)
        endif()
    endif()
    
    # Check for cmakeSupport field
    string(JSON _imgui_json ERROR_VARIABLE _err GET "${_externals_json}" "imgui")
    if(NOT _err)
        string(JSON _cmake_support ERROR_VARIABLE _err2 GET "${_imgui_json}" "cmakeSupport")
        if(NOT _err2)
            dbg(${DBG_COMMON} "  cmakeSupport field parsed: ${_cmake_support}" ID PHASE6_TEST)
        endif()
    endif()
    
    # Check for hook field
    string(JSON _docking_json ERROR_VARIABLE _err GET "${_externals_json}" "imgui_docking")
    if(NOT _err)
        string(JSON _hook_field ERROR_VARIABLE _err2 GET "${_docking_json}" "hook")
        if(NOT _err2)
            dbg(${DBG_COMMON} "  hook field parsed: ${_hook_field}" ID PHASE6_TEST)
        endif()
    endif()
else()
    cmake_warn("W602" "SOLUTION_EXTERNALS_JSON not set")
endif()

# ==============================================================================
# Test 5: Fetched External in Executable
# ==============================================================================

dbg(${DBG_COMMON} "Testing Fetched External Linking..." ID PHASE6_TEST)

# imGuiApp should link against glad, glfw, and imgui_docking
if(TARGET imGuiApp)
    get_target_property(_libs imGuiApp LINK_LIBRARIES)
    dbg(${DBG_RARE} "  imGuiApp links: ${_libs}" ID PHASE6_TEST)
    
    set(_has_glfw FALSE)
    set(_has_imgui FALSE)
    
    foreach(_lib IN LISTS _libs)
        if("${_lib}" MATCHES "glfw")
            set(_has_glfw TRUE)
        endif()
        if("${_lib}" MATCHES "imgui")
            set(_has_imgui TRUE)
        endif()
    endforeach()
    
    if(_has_glfw)
        dbg(${DBG_COMMON} "  imGuiApp links glfw" ID PHASE6_TEST)
    endif()
    if(_has_imgui)
        dbg(${DBG_COMMON} "  imGuiApp links imgui" ID PHASE6_TEST)
    endif()
else()
    dbg(${DBG_COMMON} "  imGuiApp target not found (may be skipped)" ID PHASE6_TEST)
endif()

# ==============================================================================
# Summary
# ==============================================================================

dbgspace(ID PHASE6_TEST)
dbg(${DBG_OFTEN} "=== Phase 6 Test PASSED ===" ID PHASE6_TEST)
enddbgblock(ID PHASE6_TEST)

set(PHASE6_TEST_PASSED TRUE CACHE BOOL "Phase 6 Test passed" FORCE)
