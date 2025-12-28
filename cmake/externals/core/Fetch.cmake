# cmake/externals/Core/Fetch.cmake
# =================================
# FetchContent wrapper with .externals/ caching
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - cmake/core/Errors.cmake
#   - cmake/core/Debug.cmake
#   - cmake/core/Json.cmake
#
# Provides:
#   - _fetch_git_external(EXT_NAME EXT_JSON)
#   - _make_external_available(EXT_NAME)
#   - _is_external_populated(EXT_NAME OUT_VAR)
#   - _get_external_source_dir(EXT_NAME OUT_VAR)
#
# Options (CMake Cache):
#   - EXTERNALS_OFFLINE      - Use only cached externals
#   - EXTERNALS_FORCE_FETCH  - Force re-fetch all externals
#
# Used by:
#   - Handler.cmake

include_guard(GLOBAL)

include(FetchContent)

# ==============================================================================
# Configuration
# ==============================================================================

# Central directory for fetched externals (relative to SOURCE_DIR)
set(EXTERNALS_FETCH_ROOT "${CMAKE_SOURCE_DIR}/.externals" CACHE PATH 
    "Directory for fetched externals (shared across all presets)")

# Options
option(EXTERNALS_OFFLINE "Use only cached externals, no network access" OFF)
option(EXTERNALS_FORCE_FETCH "Force re-fetch of all externals" OFF)

# ==============================================================================
# _fetch_git_external - Fetch a Git-based external with caching
# ==============================================================================
#[[
    _fetch_git_external(EXT_NAME EXT_JSON)
    
    Declares a Git-based external for fetching. Uses central .externals/
    directory and skips download if already cached with matching version.
    
    Parameters:
        EXT_NAME - Name of the external
        EXT_JSON - JSON definition containing git, tag/branch/commit
    
    JSON Fields:
        git     - Repository URL (required)
        tag     - Git tag to checkout
        branch  - Git branch to checkout  
        commit  - Git commit hash to checkout
        shallow - Use shallow clone (default: true for tag/branch)
    
    Options (CMake variables):
        EXTERNALS_OFFLINE      - Skip fetch, use cache only
        EXTERNALS_FORCE_FETCH  - Force re-fetch even if cached
    
    Example:
        _fetch_git_external("spdlog" "{\"git\":\"https://...\",\"tag\":\"v1.12.0\"}")
]]
function(_fetch_git_external EXT_NAME EXT_JSON)
    
    # ==========================================================================
    # Extract Git URL
    # ==========================================================================
    
    _json_get_string("${EXT_JSON}" "git" _git_url)
    
    if("${_git_url}" STREQUAL "")
        cmake_fatal("E012" "External '${EXT_NAME}': No 'git' URL specified")
    endif()
    
    # ==========================================================================
    # Extract Version Reference (tag/branch/commit)
    # ==========================================================================
    
    _extract_git_ref("${EXT_NAME}" "${EXT_JSON}" _git_ref _ref_type)
    
    # ==========================================================================
    # Determine Cache Directory
    # ==========================================================================
    
    set(_cache_dir "${EXTERNALS_FETCH_ROOT}/${EXT_NAME}")
    
    dbg(${DBG_COMMON} "[${EXT_NAME}] Git: ${_git_url}" ID EXTERNALS)
    dbg(${DBG_COMMON} "[${EXT_NAME}] ${_ref_type}: ${_git_ref}" ID EXTERNALS)
    dbg(${DBG_RARE} "[${EXT_NAME}] Cache dir: ${_cache_dir}" ID EXTERNALS)
    
    # ==========================================================================
    # Decision Logic: Fetch or Use Cache?
    # ==========================================================================
    
    set(_do_fetch FALSE)
    set(_skip_reason "")
    
    # CHECK 1: Force Fetch?
    if(EXTERNALS_FORCE_FETCH)
        set(_do_fetch TRUE)
        dbg(${DBG_COMMON} "[${EXT_NAME}] Force fetch requested" ID EXTERNALS)
        
    # CHECK 2: Already Cached?
    elseif(EXISTS "${_cache_dir}/.git")
        dbg(${DBG_COMMON} "[${EXT_NAME}] Found in cache: ${_cache_dir}" ID EXTERNALS)
        
        # Check if version matches
        _check_cached_version("${EXT_NAME}" "${_git_ref}" "${_ref_type}" "${_cache_dir}" _version_match)
        
        if(_version_match)
            set(_do_fetch FALSE)
            set(_skip_reason "cached")
            dbg(${DBG_COMMON} "[${EXT_NAME}] Version OK, skipping fetch" ID EXTERNALS)
        else()
            dbg(${DBG_COMMON} "[${EXT_NAME}] Version mismatch" ID EXTERNALS)
            
            if(EXTERNALS_OFFLINE)
                cmake_warn("W302" "External '${EXT_NAME}': Version mismatch but offline mode - using cached")
                set(_do_fetch FALSE)
                set(_skip_reason "offline-cached")
            else()
                set(_do_fetch TRUE)
            endif()
        endif()
        
    # CHECK 3: Offline Mode without Cache?
    elseif(EXTERNALS_OFFLINE)
        cmake_fatal("E218" "External '${EXT_NAME}': Not cached and offline mode enabled")
        
    # Not cached, need to fetch
    else()
        set(_do_fetch TRUE)
        dbg(${DBG_COMMON} "[${EXT_NAME}] Not cached, will fetch" ID EXTERNALS)
    endif()
    
    # ==========================================================================
    # Perform Fetch Declaration (if needed)
    # ==========================================================================
    
    if(_do_fetch)
        message(STATUS "[${EXT_NAME}] Fetching from ${_git_url} (${_ref_type}: ${_git_ref})...")
        
        # Ensure .externals/ directory exists
        file(MAKE_DIRECTORY "${EXTERNALS_FETCH_ROOT}")
        
        # Build fetch arguments
        _build_fetch_args("${EXT_JSON}" "${_git_ref}" "${_ref_type}" _fetch_args)
        
        # Convert name to lowercase for FetchContent
        string(TOLOWER "${EXT_NAME}" _ext_lower)
        
        FetchContent_Declare(
            ${_ext_lower}
            GIT_REPOSITORY "${_git_url}"
            ${_fetch_args}
            SOURCE_DIR "${_cache_dir}"
        )
        
        # Mark as needing fetch
        set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_NEEDS_FETCH TRUE)
        
    else()
        message(STATUS "[${EXT_NAME}] Using cached version (${_skip_reason})")
        
        # Mark as not needing fetch
        set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_NEEDS_FETCH FALSE)
    endif()
    
    # ==========================================================================
    # Store Properties
    # ==========================================================================
    
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_DECLARED TRUE)
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_SOURCE_DIR "${_cache_dir}")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_JSON "${EXT_JSON}")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_GIT_REF "${_git_ref}")
    set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_REF_TYPE "${_ref_type}")
    
endfunction()

# ==============================================================================
# _make_external_available - Actually fetch and make available
# ==============================================================================
#[[
    _make_external_available(EXT_NAME)
    
    Makes a declared external available (downloads if needed).
    Should be called after PreFetch hook and before PostFetch hook.
    
    IMPORTANT: FetchContent_MakeAvailable MUST always be called, even for
    cached externals, because it's what triggers CMake to process the
    external's CMakeLists.txt and create the targets.
    
    Parameters:
        EXT_NAME - Name of the external
    
    Example:
        _make_external_available("spdlog")
]]
function(_make_external_available EXT_NAME)
    string(TOLOWER "${EXT_NAME}" _ext_lower)
    
    get_property(_needs_fetch GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_NEEDS_FETCH)
    get_property(_cache_dir GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_SOURCE_DIR)
    
    # ==========================================================================
    # CRITICAL: FetchContent_MakeAvailable must ALWAYS be called!
    # Even for cached sources, CMake needs to process the CMakeLists.txt
    # to create the targets. The "fetch" part will be skipped if SOURCE_DIR
    # already exists, but the "make available" (add_subdirectory) is required.
    # ==========================================================================
    
    if(_needs_fetch)
        dbg(${DBG_COMMON} "[${EXT_NAME}] Downloading and configuring..." ID EXTERNALS)
    else()
        dbg(${DBG_COMMON} "[${EXT_NAME}] Configuring from cache: ${_cache_dir}" ID EXTERNALS)
        
        # For cached externals, we need to declare them first so MakeAvailable works
        # FetchContent will skip download because SOURCE_DIR exists
        FetchContent_Declare(
            ${_ext_lower}
            SOURCE_DIR "${_cache_dir}"
        )
    endif()
    
    # Suppress excessive CMake output during configure
    set(FETCHCONTENT_QUIET ON)
    
    # This does TWO things:
    # 1. Downloads the source (if not already present) - SKIPPED if cached
    # 2. Calls add_subdirectory() to configure and create targets - ALWAYS NEEDED
    FetchContent_MakeAvailable(${_ext_lower})
    
    # Verify success
    FetchContent_GetProperties(${_ext_lower})
    
    if(${_ext_lower}_POPULATED)
        if(_needs_fetch)
            message(STATUS "[${EXT_NAME}] Fetched and configured successfully")
        else()
            message(STATUS "[${EXT_NAME}] Configured from cache")
        endif()
        dbg(${DBG_RARE} "[${EXT_NAME}] Source: ${${_ext_lower}_SOURCE_DIR}" ID EXTERNALS)
    else()
        cmake_fatal("E202" "Failed for '${EXT_NAME}': FetchContent did not populate")
    endif()
    
    # Verify directory is valid
    if(EXISTS "${_cache_dir}/.git" OR EXISTS "${_cache_dir}/CMakeLists.txt" OR EXISTS "${_cache_dir}/include")
        set_property(GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_POPULATED TRUE)
    else()
        cmake_fatal("E202" "External '${EXT_NAME}': Source directory invalid: ${_cache_dir}")
    endif()
    
endfunction()

# ==============================================================================
# _is_external_populated - Check if external is populated
# ==============================================================================
#[[
    _is_external_populated(EXT_NAME OUT_VAR)
    
    Checks if an external has been successfully fetched/cached.
    
    Parameters:
        EXT_NAME - Name of the external
        OUT_VAR  - Output variable (TRUE/FALSE)
]]
function(_is_external_populated EXT_NAME OUT_VAR)
    get_property(_populated GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_POPULATED)
    
    if(_populated)
        set(${OUT_VAR} TRUE PARENT_SCOPE)
    else()
        set(${OUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# _get_external_source_dir - Get source directory of fetched external
# ==============================================================================
#[[
    _get_external_source_dir(EXT_NAME OUT_VAR)
    
    Gets the source directory of a fetched external.
    
    Parameters:
        EXT_NAME - Name of the external
        OUT_VAR  - Output variable for source directory path
]]
function(_get_external_source_dir EXT_NAME OUT_VAR)
    get_property(_source_dir GLOBAL PROPERTY EXTERNAL_${EXT_NAME}_SOURCE_DIR)
    set(${OUT_VAR} "${_source_dir}" PARENT_SCOPE)
endfunction()

# ==============================================================================
# Helper: Extract Git Reference
# ==============================================================================
function(_extract_git_ref EXT_NAME EXT_JSON OUT_REF OUT_TYPE)
    _json_has_key("${EXT_JSON}" "tag" _has_tag)
    _json_has_key("${EXT_JSON}" "branch" _has_branch)
    _json_has_key("${EXT_JSON}" "commit" _has_commit)
    
    # Validate: exactly one must be present
    set(_ref_count 0)
    if(_has_tag)
        math(EXPR _ref_count "${_ref_count} + 1")
    endif()
    if(_has_branch)
        math(EXPR _ref_count "${_ref_count} + 1")
    endif()
    if(_has_commit)
        math(EXPR _ref_count "${_ref_count} + 1")
    endif()
    
    if(_ref_count EQUAL 0)
        cmake_fatal("E215" "Fetched external '${EXT_NAME}': No tag/branch/commit specified")
    endif()
    
    if(_ref_count GREATER 1)
        cmake_fatal("E215" "Fetched external '${EXT_NAME}': Multiple version refs specified (use only one of tag/branch/commit)")
    endif()
    
    # Extract the reference
    if(_has_tag)
        _json_get_string("${EXT_JSON}" "tag" _ref)
        set(${OUT_REF} "${_ref}" PARENT_SCOPE)
        set(${OUT_TYPE} "tag" PARENT_SCOPE)
    elseif(_has_branch)
        _json_get_string("${EXT_JSON}" "branch" _ref)
        set(${OUT_REF} "${_ref}" PARENT_SCOPE)
        set(${OUT_TYPE} "branch" PARENT_SCOPE)
    elseif(_has_commit)
        _json_get_string("${EXT_JSON}" "commit" _ref)
        set(${OUT_REF} "${_ref}" PARENT_SCOPE)
        set(${OUT_TYPE} "commit" PARENT_SCOPE)
    endif()
endfunction()

# ==============================================================================
# Helper: Check Cached Version
# ==============================================================================
function(_check_cached_version EXT_NAME EXPECTED_REF REF_TYPE CACHE_DIR OUT_MATCH)
    
    # For branches: always consider potentially outdated (could have new commits)
    if("${REF_TYPE}" STREQUAL "branch")
        set(${OUT_MATCH} FALSE PARENT_SCOPE)
        return()
    endif()
    
    # Find git executable
    find_program(_git_exe git)
    if(NOT _git_exe)
        dbg(${DBG_RARE} "[${EXT_NAME}] Git not found, assuming version mismatch" ID EXTERNALS)
        set(${OUT_MATCH} FALSE PARENT_SCOPE)
        return()
    endif()
    
    # Get current HEAD
    execute_process(
        COMMAND "${_git_exe}" rev-parse HEAD
        WORKING_DIRECTORY "${CACHE_DIR}"
        OUTPUT_VARIABLE _current_head
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _git_result
    )
    
    if(NOT _git_result EQUAL 0)
        dbg(${DBG_RARE} "[${EXT_NAME}] Could not get HEAD, assuming version mismatch" ID EXTERNALS)
        set(${OUT_MATCH} FALSE PARENT_SCOPE)
        return()
    endif()
    
    # For commits: direct comparison
    if("${REF_TYPE}" STREQUAL "commit")
        string(SUBSTRING "${_current_head}" 0 7 _short_head)
        string(SUBSTRING "${EXPECTED_REF}" 0 7 _short_expected)
        
        if("${_short_head}" STREQUAL "${_short_expected}")
            set(${OUT_MATCH} TRUE PARENT_SCOPE)
        else()
            set(${OUT_MATCH} FALSE PARENT_SCOPE)
        endif()
        return()
    endif()
    
    # For tags: check if tag points to current HEAD
    if("${REF_TYPE}" STREQUAL "tag")
        execute_process(
            COMMAND "${_git_exe}" rev-parse "${EXPECTED_REF}^{}"
            WORKING_DIRECTORY "${CACHE_DIR}"
            OUTPUT_VARIABLE _tag_commit
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE _tag_result
        )
        
        if(_tag_result EQUAL 0 AND "${_tag_commit}" STREQUAL "${_current_head}")
            set(${OUT_MATCH} TRUE PARENT_SCOPE)
        else()
            # Tag might not be fetched yet, try checking by describe
            execute_process(
                COMMAND "${_git_exe}" describe --tags --exact-match HEAD
                WORKING_DIRECTORY "${CACHE_DIR}"
                OUTPUT_VARIABLE _head_tag
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
                RESULT_VARIABLE _describe_result
            )
            
            if(_describe_result EQUAL 0 AND "${_head_tag}" STREQUAL "${EXPECTED_REF}")
                set(${OUT_MATCH} TRUE PARENT_SCOPE)
            else()
                set(${OUT_MATCH} FALSE PARENT_SCOPE)
            endif()
        endif()
        return()
    endif()
    
    # Default: no match
    set(${OUT_MATCH} FALSE PARENT_SCOPE)
endfunction()

# ==============================================================================
# Helper: Build Fetch Arguments
# ==============================================================================
function(_build_fetch_args EXT_JSON GIT_REF REF_TYPE OUT_ARGS)
    set(_args "")
    
    # Git reference
    if("${REF_TYPE}" STREQUAL "branch")
        list(APPEND _args GIT_TAG "origin/${GIT_REF}")
    else()
        list(APPEND _args GIT_TAG "${GIT_REF}")
    endif()
    
    # Shallow clone (not for commits, as they require full history)
    if(NOT "${REF_TYPE}" STREQUAL "commit")
        _json_has_key("${EXT_JSON}" "shallow" _has_shallow)
        if(_has_shallow)
            _json_get_bool_from_key("${EXT_JSON}" "shallow" _shallow)
        else()
            set(_shallow TRUE)  # Default: use shallow clone
        endif()
        
        if(_shallow)
            list(APPEND _args GIT_SHALLOW TRUE)
        endif()
    endif()
    
    # Progress output
    list(APPEND _args GIT_PROGRESS TRUE)
    
    set(${OUT_ARGS} ${_args} PARENT_SCOPE)
endfunction()
