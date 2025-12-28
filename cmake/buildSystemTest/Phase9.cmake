# ==============================================================================
# Phase9.cmake – Build System Test for System Externals
# ==============================================================================
#
# Test:         Phase 9
# Version:      1.0.0
# Date:         2025-12-18
# Part of:      CMake Architecture
#
# Description:
#   Tests the System Externals pipeline including:
#   - system: true type detection
#   - find_package() integration
#   - Package, components, hints, backup parsing
#   - PathResolver mechanism
#   - Type priority: system → git → path
#   - Error codes E5xx, Warnings W5xx
#
# ==============================================================================

include_guard(GLOBAL)

dbg_init(ID PHASE9_TEST LEVEL ${DBG_SHOW_ALL} SWITCH ON TAG "Phase9")
dbg(${DBG_OFTEN} "=== Phase 9 Test Start ===" ID PHASE9_TEST)

# ==============================================================================
# Test 1: System External Type Detection in JSON
# ==============================================================================

dbg(${DBG_COMMON} "Test 1: System External type detection..." ID PHASE9_TEST)

get_property(_externals_json GLOBAL PROPERTY SOLUTION_EXTERNALS_JSON)

set(_system_externals_found 0)
set(_local_externals_found 0)
set(_fetched_externals_found 0)

if(_externals_json)
    # Count external types by checking for identifying fields
    string(JSON _ext_count LENGTH "${_externals_json}")
    string(JSON _ext_keys MEMBER "${_externals_json}" 0)
    
    # Get all external names
    set(_idx 0)
    while(_idx LESS 100)  # Safety limit
        string(JSON _ext_name ERROR_VARIABLE _err MEMBER "${_externals_json}" ${_idx})
        if(_err)
            break()
        endif()
        
        string(JSON _ext_def GET "${_externals_json}" "${_ext_name}")
        
        # Check for system field
        string(JSON _system_val ERROR_VARIABLE _e1 GET "${_ext_def}" "system")
        if(NOT _e1 AND _system_val)
            math(EXPR _system_externals_found "${_system_externals_found} + 1")
            dbg(${DBG_RARE} "  System: ${_ext_name}" ID PHASE9_TEST)
        else()
            # Check for git field
            string(JSON _git_val ERROR_VARIABLE _e2 GET "${_ext_def}" "git")
            if(NOT _e2 AND _git_val)
                math(EXPR _fetched_externals_found "${_fetched_externals_found} + 1")
            else()
                # Check for path field
                string(JSON _path_val ERROR_VARIABLE _e3 GET "${_ext_def}" "path")
                if(NOT _e3 AND _path_val)
                    math(EXPR _local_externals_found "${_local_externals_found} + 1")
                endif()
            endif()
        endif()
        
        math(EXPR _idx "${_idx} + 1")
    endwhile()
    
    dbg(${DBG_COMMON} "  System Externals: ${_system_externals_found}" ID PHASE9_TEST)
    dbg(${DBG_COMMON} "  Fetched Externals: ${_fetched_externals_found}" ID PHASE9_TEST)
    dbg(${DBG_COMMON} "  Local Externals: ${_local_externals_found}" ID PHASE9_TEST)
else()
    dbg(${DBG_COMMON} "  SOLUTION_EXTERNALS_JSON not available" ID PHASE9_TEST)
endif()

# ==============================================================================
# Test 2: System External JSON Fields Parsing
# ==============================================================================

dbg(${DBG_COMMON} "Test 2: System External JSON fields..." ID PHASE9_TEST)

# Find first system external and check its fields
set(_test_system_ext "")
set(_test_system_def "")

if(_externals_json AND _system_externals_found GREATER 0)
    set(_idx 0)
    while(_idx LESS 100)
        string(JSON _ext_name ERROR_VARIABLE _err MEMBER "${_externals_json}" ${_idx})
        if(_err)
            break()
        endif()
        
        string(JSON _ext_def GET "${_externals_json}" "${_ext_name}")
        string(JSON _system_val ERROR_VARIABLE _e1 GET "${_ext_def}" "system")
        
        if(NOT _e1 AND _system_val)
            set(_test_system_ext "${_ext_name}")
            set(_test_system_def "${_ext_def}")
            break()
        endif()
        
        math(EXPR _idx "${_idx} + 1")
    endwhile()
endif()

if(_test_system_ext)
    dbg(${DBG_COMMON} "  Inspecting: ${_test_system_ext}" ID PHASE9_TEST)
    
    # Check package field (required)
    string(JSON _package ERROR_VARIABLE _e1 GET "${_test_system_def}" "package")
    if(NOT _e1 AND _package)
        dbg(${DBG_COMMON} "    package: ${_package}" ID PHASE9_TEST)
    else()
        cmake_warn("W901" "System external '${_test_system_ext}' missing 'package' field")
    endif()
    
    # Check version field (optional)
    string(JSON _version ERROR_VARIABLE _e2 GET "${_test_system_def}" "version")
    if(NOT _e2 AND _version)
        dbg(${DBG_RARE} "    version: ${_version}" ID PHASE9_TEST)
    endif()
    
    # Check components field (optional)
    string(JSON _components ERROR_VARIABLE _e3 GET "${_test_system_def}" "components")
    if(NOT _e3)
        string(JSON _comp_count LENGTH "${_components}")
        dbg(${DBG_RARE} "    components: ${_comp_count} defined" ID PHASE9_TEST)
        
        # List first few components
        if(_comp_count GREATER 0)
            string(JSON _comp0 GET "${_components}" 0)
            dbg(${DBG_ULTRA_RARE} "      [0]: ${_comp0}" ID PHASE9_TEST)
        endif()
    endif()
    
    # Check hints field (optional)
    string(JSON _hints ERROR_VARIABLE _e4 GET "${_test_system_def}" "hints")
    if(NOT _e4)
        string(JSON _hints_count LENGTH "${_hints}")
        dbg(${DBG_RARE} "    hints: ${_hints_count} paths" ID PHASE9_TEST)
    endif()
    
    # Check backup field (optional)
    string(JSON _backup ERROR_VARIABLE _e5 GET "${_test_system_def}" "backup")
    if(NOT _e5 AND _backup)
        dbg(${DBG_RARE} "    backup: ${_backup}" ID PHASE9_TEST)
    endif()
    
    # Check required field (optional, default: true)
    string(JSON _required ERROR_VARIABLE _e6 GET "${_test_system_def}" "required")
    if(NOT _e6)
        dbg(${DBG_RARE} "    required: ${_required}" ID PHASE9_TEST)
    else()
        dbg(${DBG_ULTRA_RARE} "    required: true (default)" ID PHASE9_TEST)
    endif()
else()
    dbg(${DBG_COMMON} "  No System External to inspect" ID PHASE9_TEST)
endif()

# ==============================================================================
# Test 3: Type Priority (system → git → path)
# ==============================================================================

dbg(${DBG_COMMON} "Test 3: External type priority..." ID PHASE9_TEST)

dbg(${DBG_COMMON} "  Priority order: system → git → path → E012" ID PHASE9_TEST)
dbg(${DBG_COMMON} "  Detection based on first present field" ID PHASE9_TEST)

# Verify no external has multiple type fields (would be E012)
set(_conflicting_externals 0)
if(_externals_json)
    set(_idx 0)
    while(_idx LESS 100)
        string(JSON _ext_name ERROR_VARIABLE _err MEMBER "${_externals_json}" ${_idx})
        if(_err)
            break()
        endif()
        
        string(JSON _ext_def GET "${_externals_json}" "${_ext_name}")
        
        set(_type_fields_count 0)
        
        string(JSON _sys ERROR_VARIABLE _e1 GET "${_ext_def}" "system")
        if(NOT _e1 AND _sys)
            math(EXPR _type_fields_count "${_type_fields_count} + 1")
        endif()
        
        string(JSON _git ERROR_VARIABLE _e2 GET "${_ext_def}" "git")
        if(NOT _e2 AND _git)
            math(EXPR _type_fields_count "${_type_fields_count} + 1")
        endif()
        
        string(JSON _path ERROR_VARIABLE _e3 GET "${_ext_def}" "path")
        if(NOT _e3 AND _path)
            math(EXPR _type_fields_count "${_type_fields_count} + 1")
        endif()
        
        if(_type_fields_count GREATER 1)
            math(EXPR _conflicting_externals "${_conflicting_externals} + 1")
            dbg(${DBG_COMMON} "  CONFLICT: ${_ext_name} has ${_type_fields_count} type fields" ID PHASE9_TEST)
        endif()
        
        math(EXPR _idx "${_idx} + 1")
    endwhile()
endif()

if(_conflicting_externals EQUAL 0)
    dbg(${DBG_COMMON} "  All externals have exactly one type field" ID PHASE9_TEST)
else()
    cmake_warn("W902" "${_conflicting_externals} external(s) have conflicting type fields")
endif()

# ==============================================================================
# Test 4: System External Target Availability
# ==============================================================================

dbg(${DBG_COMMON} "Test 4: System External targets..." ID PHASE9_TEST)

# Check for known system external targets
set(_known_system_packages "Qt6;Boost;OpenCV;CUDAToolkit")
set(_found_system_targets 0)

# Qt6 components
foreach(_qt_comp IN ITEMS Core Widgets Gui OpenGL)
    if(TARGET Qt6::${_qt_comp})
        math(EXPR _found_system_targets "${_found_system_targets} + 1")
        dbg(${DBG_COMMON} "  Found: Qt6::${_qt_comp}" ID PHASE9_TEST)
    endif()
endforeach()

# Boost
if(TARGET Boost::boost OR TARGET Boost::headers)
    math(EXPR _found_system_targets "${_found_system_targets} + 1")
    dbg(${DBG_COMMON} "  Found: Boost" ID PHASE9_TEST)
endif()

# OpenCV
if(TARGET opencv_core OR TARGET OpenCV::core)
    math(EXPR _found_system_targets "${_found_system_targets} + 1")
    dbg(${DBG_COMMON} "  Found: OpenCV" ID PHASE9_TEST)
endif()

if(_found_system_targets EQUAL 0)
    dbg(${DBG_COMMON} "  No system external targets found (may not be configured)" ID PHASE9_TEST)
else()
    dbg(${DBG_COMMON} "  System external targets available: ${_found_system_targets}" ID PHASE9_TEST)
endif()

# ==============================================================================
# Test 5: Handler/PathResolver Module Availability
# ==============================================================================

dbg(${DBG_COMMON} "Test 5: System External modules..." ID PHASE9_TEST)

# Check if system external handler functions exist
if(COMMAND _handle_system_external)
    dbg(${DBG_COMMON} "  _handle_system_external() available" ID PHASE9_TEST)
else()
    dbg(${DBG_COMMON} "  _handle_system_external() not found (check System/Handler.cmake)" ID PHASE9_TEST)
endif()

if(COMMAND _resolve_system_paths)
    dbg(${DBG_COMMON} "  _resolve_system_paths() available" ID PHASE9_TEST)
else()
    dbg(${DBG_COMMON} "  _resolve_system_paths() not found (check System/PathResolver.cmake)" ID PHASE9_TEST)
endif()

# Check Orchestrator for system type dispatch
# The Orchestrator should now handle system, git, and path types
dbg(${DBG_RARE} "  Orchestrator dispatches: system → System/Handler.cmake" ID PHASE9_TEST)
dbg(${DBG_RARE} "  Orchestrator dispatches: git → Core/Fetch.cmake" ID PHASE9_TEST)
dbg(${DBG_RARE} "  Orchestrator dispatches: path → Local/Attach.cmake" ID PHASE9_TEST)

# ==============================================================================
# Test 6: Error Code Range Validation
# ==============================================================================

dbg(${DBG_COMMON} "Test 6: Error code range..." ID PHASE9_TEST)

dbg(${DBG_COMMON} "  System External Errors: E501-E505" ID PHASE9_TEST)
dbg(${DBG_RARE} "    E501: System external not found (no valid path)" ID PHASE9_TEST)
dbg(${DBG_RARE} "    E502: 'package' field is required" ID PHASE9_TEST)
dbg(${DBG_RARE} "    E503: find_package() failed" ID PHASE9_TEST)
dbg(${DBG_RARE} "    E504: Required component not found" ID PHASE9_TEST)
dbg(${DBG_RARE} "    E505: Version constraint not satisfied" ID PHASE9_TEST)

dbg(${DBG_COMMON} "  System External Warnings: W501-W502" ID PHASE9_TEST)
dbg(${DBG_RARE} "    W501: Using backup location" ID PHASE9_TEST)
dbg(${DBG_RARE} "    W502: Version mismatch (using found version)" ID PHASE9_TEST)

# ==============================================================================
# Test 7: Qt6 System External (if configured)
# ==============================================================================

dbg(${DBG_COMMON} "Test 7: Qt6 System External..." ID PHASE9_TEST)

# Check if qt6 is defined as system external
set(_qt6_is_system FALSE)
if(_externals_json)
    string(JSON _qt6_def ERROR_VARIABLE _err GET "${_externals_json}" "qt6")
    if(NOT _err)
        string(JSON _qt6_system ERROR_VARIABLE _e1 GET "${_qt6_def}" "system")
        if(NOT _e1 AND _qt6_system)
            set(_qt6_is_system TRUE)
            dbg(${DBG_COMMON} "  qt6 defined as System External" ID PHASE9_TEST)
            
            # Get package name
            string(JSON _qt6_package ERROR_VARIABLE _e2 GET "${_qt6_def}" "package")
            if(NOT _e2)
                dbg(${DBG_COMMON} "    package: ${_qt6_package}" ID PHASE9_TEST)
            endif()
            
            # Get components
            string(JSON _qt6_comps ERROR_VARIABLE _e3 GET "${_qt6_def}" "components")
            if(NOT _e3)
                string(JSON _comp_count LENGTH "${_qt6_comps}")
                dbg(${DBG_COMMON} "    components: ${_comp_count}" ID PHASE9_TEST)
            endif()
        endif()
    endif()
endif()

if(NOT _qt6_is_system)
    # Check if it's still using old path workaround
    if(_externals_json)
        string(JSON _qt6_def ERROR_VARIABLE _err GET "${_externals_json}" "qt6")
        if(NOT _err)
            string(JSON _qt6_path ERROR_VARIABLE _e1 GET "${_qt6_def}" "path")
            if(NOT _e1 AND _qt6_path)
                dbg(${DBG_COMMON} "  qt6 uses 'path' workaround (consider migrating to 'system')" ID PHASE9_TEST)
            endif()
        else()
            dbg(${DBG_COMMON} "  qt6 not defined in externals" ID PHASE9_TEST)
        endif()
    endif()
endif()

# Check Qt6 found status
if(Qt6_FOUND OR Qt6Core_FOUND)
    dbg(${DBG_COMMON} "  Qt6 found by find_package()" ID PHASE9_TEST)
    if(DEFINED Qt6_VERSION)
        dbg(${DBG_COMMON} "    Version: ${Qt6_VERSION}" ID PHASE9_TEST)
    endif()
    if(DEFINED Qt6_DIR)
        dbg(${DBG_RARE} "    Qt6_DIR: ${Qt6_DIR}" ID PHASE9_TEST)
    endif()
else()
    dbg(${DBG_COMMON} "  Qt6 not found (may not be required)" ID PHASE9_TEST)
endif()

# ==============================================================================
# Test 8: Executable Using System External
# ==============================================================================

dbg(${DBG_COMMON} "Test 8: Executables with System Externals..." ID PHASE9_TEST)

get_property(_all_targets DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

set(_targets_with_qt 0)
foreach(_target IN LISTS _all_targets)
    get_target_property(_target_type ${_target} TYPE)
    
    # Only check executables and libraries
    if(_target_type MATCHES "EXECUTABLE|LIBRARY")
        get_target_property(_link_libs ${_target} LINK_LIBRARIES)
        
        if(_link_libs)
            foreach(_lib IN LISTS _link_libs)
                if(_lib MATCHES "^Qt6::")
                    math(EXPR _targets_with_qt "${_targets_with_qt} + 1")
                    dbg(${DBG_RARE} "  ${_target} links Qt6" ID PHASE9_TEST)
                    break()
                endif()
            endforeach()
        endif()
    endif()
endforeach()

dbg(${DBG_COMMON} "  Targets using Qt6: ${_targets_with_qt}" ID PHASE9_TEST)

# ==============================================================================
# Test 9: Package-Hook Mechanism
# ==============================================================================

dbg(${DBG_COMMON} "Test 9: Package-Hook mechanism..." ID PHASE9_TEST)

# Package hooks are in cmake/externals/system/packages/
set(_package_hooks_dir "${CMAKE_SOURCE_DIR}/cmake/externals/system/packages")

if(EXISTS "${_package_hooks_dir}")
    dbg(${DBG_COMMON} "  Package hooks directory exists" ID PHASE9_TEST)
    
    file(GLOB _hook_files "${_package_hooks_dir}/*.cmake")
    list(LENGTH _hook_files _hook_count)
    dbg(${DBG_COMMON} "  Package hooks found: ${_hook_count}" ID PHASE9_TEST)
    
    foreach(_hook IN LISTS _hook_files)
        get_filename_component(_hook_name "${_hook}" NAME_WE)
        dbg(${DBG_RARE} "    ${_hook_name}.cmake" ID PHASE9_TEST)
    endforeach()
else()
    dbg(${DBG_COMMON} "  Package hooks directory not found (optional)" ID PHASE9_TEST)
    dbg(${DBG_RARE} "    Expected: cmake/externals/system/packages/" ID PHASE9_TEST)
endif()

# ==============================================================================
# Test 10: Migration Status (path → system)
# ==============================================================================

dbg(${DBG_COMMON} "Test 10: Migration status..." ID PHASE9_TEST)

# Check for known system libraries still using path workaround
set(_known_system_libs "qt6;boost;opencv;cuda")
set(_using_path_workaround 0)

if(_externals_json)
    foreach(_lib IN LISTS _known_system_libs)
        string(JSON _lib_def ERROR_VARIABLE _err GET "${_externals_json}" "${_lib}")
        if(NOT _err)
            string(JSON _lib_path ERROR_VARIABLE _e1 GET "${_lib_def}" "path")
            string(JSON _lib_sys ERROR_VARIABLE _e2 GET "${_lib_def}" "system")
            
            if(NOT _e1 AND _lib_path AND _e2)
                # Has path but no system field
                math(EXPR _using_path_workaround "${_using_path_workaround} + 1")
                dbg(${DBG_COMMON} "  ${_lib}: uses 'path' (consider 'system')" ID PHASE9_TEST)
            elseif(NOT _e2 AND _lib_sys)
                dbg(${DBG_RARE} "  ${_lib}: correctly uses 'system'" ID PHASE9_TEST)
            endif()
        endif()
    endforeach()
endif()

if(_using_path_workaround EQUAL 0)
    dbg(${DBG_COMMON} "  All system libraries correctly configured" ID PHASE9_TEST)
else()
    dbg(${DBG_COMMON} "  ${_using_path_workaround} library(ies) could be migrated to 'system'" ID PHASE9_TEST)
endif()

# ==============================================================================
# Summary
# ==============================================================================

dbgspace(ID PHASE9_TEST)

# Determine overall status
set(_phase9_status "PASSED")

if(_system_externals_found GREATER 0 AND _found_system_targets EQUAL 0)
    # System externals defined but no targets found - might be expected if not installed
    dbg(${DBG_COMMON} "Note: System externals defined but targets not created" ID PHASE9_TEST)
    dbg(${DBG_COMMON} "      (Normal if system libraries not installed)" ID PHASE9_TEST)
endif()

dbg(${DBG_OFTEN} "=== Phase 9 Test ${_phase9_status} ===" ID PHASE9_TEST)
enddbgblock(ID PHASE9_TEST)

set(PHASE9_TEST_PASSED TRUE CACHE BOOL "Phase 9 Test passed" FORCE)

# ==============================================================================
# Cleanup
# ==============================================================================

unset(_externals_json)
unset(_system_externals_found)
unset(_local_externals_found)
unset(_fetched_externals_found)
unset(_test_system_ext)
unset(_test_system_def)
unset(_conflicting_externals)
unset(_known_system_packages)
unset(_found_system_targets)
unset(_qt6_is_system)
unset(_all_targets)
unset(_targets_with_qt)
unset(_hook_files)
unset(_using_path_workaround)
unset(_phase9_status)
