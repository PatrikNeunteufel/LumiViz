# cmake/externals/includes/bass/Include.cmake
# ============================================
# BASS Audio Library integration - links libraries and copies DLLs
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
# Type:    Local External Include
#
# Dependencies:
#   - cmake/core/Json.cmake (_json_get_bool_from_key)
#
# Expected Variables (set by Orchestrator):
#   - EXTERNAL_NAME    - "bass"
#   - EXTERNAL_ROOT    - Path to externals/bass (resources)
#   - EXTERNAL_OPTIONS - JSON string with plugin options
#   - EXECUTABLE_NAME  - Target to attach to
#
# Available Options (via external_options in Solution.json):
#   Decoders: BASS_FLAC, BASS_OPUS, BASS_DSD, BASS_WV, BASS_APE,
#             BASS_MPC, BASS_ALAC, BASS_TTA, BASS_CD, BASS_WEBM
#   Encoders: BASS_ENC, BASS_ENC_MP3, BASS_ENC_OGG, BASS_ENC_FLAC
#   Plugins:  BASS_FX, BASS_MIX, BASS_LOUD, BASS_MIDI
#   Platform: BASS_WASAPI (Win), BASS_WMA (Win), BASS_HLS, BASS_SSL (Win)
#
# Provides:
#   - _bass_enable_plugin(PLUGIN_NAME PLUGIN_DIR)
#
# Resources:
#   - externals/bass/ (DLLs, libs, headers)
#
# Used by:
#   - Orchestrator.cmake (via apply_external_to_target)

# Note: No include_guard() - this file is included per-target intentionally

# ==============================================================================
# Debug Output
# ==============================================================================

# Support both new and legacy variable names
if(NOT DEFINED EXTERNAL_OPTIONS AND DEFINED EXTERNAL_ELEMENT_OPTIONS)
    set(EXTERNAL_OPTIONS "${EXTERNAL_ELEMENT_OPTIONS}")
endif()
if(NOT DEFINED EXTERNAL_NAME AND DEFINED EXTERNAL_ELEMENT_NAME)
    set(EXTERNAL_NAME "${EXTERNAL_ELEMENT_NAME}")
endif()

message(STATUS "[${EXTERNAL_NAME}] Attaching to ${EXECUTABLE_NAME}")

# ==============================================================================
# Path Configuration
# ==============================================================================

set(_bass_root "${EXTERNAL_ROOT}")

# ==============================================================================
# Platform Detection
# ==============================================================================

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # Windows x64
        set(_bass_lib_dir "win/c/x64")
        set(_bass_dll_dir "win/x64")
    else()
        # Windows x86
        set(_bass_lib_dir "win/c")
        set(_bass_dll_dir "win")
    endif()
    set(_bass_include_dir "win/c")
    
elseif(APPLE)
    # macOS
    set(_bass_lib_dir "osx")
    set(_bass_include_dir "osx/c")
    set(_bass_dll_dir "${_bass_lib_dir}")
    
elseif(UNIX)
    # Linux
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_bass_lib_dir "linux/libs/x86_64")
    else()
        set(_bass_lib_dir "linux/libs/x86")
    endif()
    set(_bass_include_dir "linux")
    set(_bass_dll_dir "${_bass_lib_dir}")
    
else()
    message(WARNING "[${EXTERNAL_NAME}] Unsupported platform")
    return()
endif()

# ==============================================================================
# BASS Core Library (Always Required)
# ==============================================================================

if(WIN32)
    # Windows: bass.lib + bass.dll
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
        "${_bass_root}/bass24/${_bass_lib_dir}/bass.lib"
    )
    
    # Copy DLL to output directory
    add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_bass_root}/bass24/${_bass_dll_dir}/bass.dll"
            $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>
        COMMENT "[BASS] Copying bass.dll"
    )
    
elseif(APPLE)
    # macOS: libbass.dylib
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
        "${_bass_root}/bass24/${_bass_lib_dir}/libbass.dylib"
    )
    
elseif(UNIX)
    # Linux: libbass.so
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
        "${_bass_root}/bass24/${_bass_lib_dir}/libbass.so"
    )
endif()

# Include directory
target_include_directories(${EXECUTABLE_NAME} SYSTEM PRIVATE
    "${_bass_root}/bass24/${_bass_include_dir}"
)

# ==============================================================================
# Helper Function: Enable Plugin
# ==============================================================================

function(_bass_enable_plugin PLUGIN_NAME PLUGIN_DIR)
    # Determine include directory - bass_fx uses uppercase C
    if(PLUGIN_DIR STREQUAL "bass_fx24")
        set(_plugin_include_subdir "C")
        set(_plugin_c_subdir "C")
    else()
        set(_plugin_include_subdir "c")
        set(_plugin_c_subdir "c")
    endif()
    
    # Add plugin include directory for the header file
    if(WIN32)
        target_include_directories(${EXECUTABLE_NAME} SYSTEM PRIVATE
            "${_bass_root}/${PLUGIN_DIR}/win/${_plugin_include_subdir}"
        )
    elseif(APPLE)
        target_include_directories(${EXECUTABLE_NAME} SYSTEM PRIVATE
            "${_bass_root}/${PLUGIN_DIR}/osx"
        )
    elseif(UNIX)
        # Linux may use C or c depending on plugin
        if(EXISTS "${_bass_root}/${PLUGIN_DIR}/linux/C")
            target_include_directories(${EXECUTABLE_NAME} SYSTEM PRIVATE
                "${_bass_root}/${PLUGIN_DIR}/linux/C"
            )
        else()
            target_include_directories(${EXECUTABLE_NAME} SYSTEM PRIVATE
                "${_bass_root}/${PLUGIN_DIR}/linux"
            )
        endif()
    endif()
    
    # Determine lib path for this plugin
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_plugin_lib_dir "win/${_plugin_c_subdir}/x64")
        else()
            set(_plugin_lib_dir "win/${_plugin_c_subdir}")
        endif()
    endif()
    
    # Link library and copy DLL
    if(WIN32)
        # Windows: .lib + .dll
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
            "${_bass_root}/${PLUGIN_DIR}/${_plugin_lib_dir}/${PLUGIN_NAME}.lib"
        )
        add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_bass_root}/${PLUGIN_DIR}/${_bass_dll_dir}/${PLUGIN_NAME}.dll"
                $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>
            COMMENT "[BASS] Copying ${PLUGIN_NAME}.dll"
        )
        
    elseif(APPLE)
        # macOS: .dylib
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
            "${_bass_root}/${PLUGIN_DIR}/${_bass_lib_dir}/lib${PLUGIN_NAME}.dylib"
        )
        
    elseif(UNIX)
        # Linux: .so
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
            "${_bass_root}/${PLUGIN_DIR}/${_bass_lib_dir}/lib${PLUGIN_NAME}.so"
        )
    endif()
    
    message(STATUS "[${EXTERNAL_NAME}]   ${PLUGIN_NAME}: ENABLED")
endfunction()

# ==============================================================================
# Process Options: Decoders
# ==============================================================================

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_FLAC" _opt_flac)
if(_opt_flac)
    _bass_enable_plugin("bassflac" "bassflac24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_OPUS" _opt_opus)
if(_opt_opus)
    _bass_enable_plugin("bassopus" "bassopus24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_DSD" _opt_dsd)
if(_opt_dsd)
    _bass_enable_plugin("bassdsd" "bassdsd24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_WV" _opt_wv)
if(_opt_wv)
    _bass_enable_plugin("basswv" "basswv24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_APE" _opt_ape)
if(_opt_ape)
    _bass_enable_plugin("bass_ape" "bassape24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_MPC" _opt_mpc)
if(_opt_mpc)
    _bass_enable_plugin("bass_mpc" "bass_mpc24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_ALAC" _opt_alac)
if(_opt_alac)
    _bass_enable_plugin("bassalac" "bassalac24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_TTA" _opt_tta)
if(_opt_tta)
    _bass_enable_plugin("bass_tta" "bass_tta24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_CD" _opt_cd)
if(_opt_cd)
    _bass_enable_plugin("basscd" "basscd24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_WEBM" _opt_webm)
if(_opt_webm)
    _bass_enable_plugin("basswebm" "basswebm24")
endif()

# ==============================================================================
# Process Options: Encoders
# ==============================================================================

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_ENC" _opt_enc)
_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_ENC_MP3" _opt_enc_mp3)
_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_ENC_OGG" _opt_enc_ogg)
_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_ENC_FLAC" _opt_enc_flac)

# Auto-enable BASS_ENC if any encoder is requested
if(_opt_enc_mp3 OR _opt_enc_ogg OR _opt_enc_flac)
    set(_opt_enc TRUE)
endif()

if(_opt_enc)
    _bass_enable_plugin("bassenc" "bassenc24")
endif()

if(_opt_enc_mp3)
    _bass_enable_plugin("bassenc_mp3" "bassenc_mp324")
endif()

if(_opt_enc_ogg)
    _bass_enable_plugin("bassenc_ogg" "bassenc_ogg24")
endif()

if(_opt_enc_flac)
    _bass_enable_plugin("bassenc_flac" "bassenc_flac24")
endif()

# ==============================================================================
# Process Options: Plugins
# ==============================================================================

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_FX" _opt_fx)
if(_opt_fx)
    _bass_enable_plugin("bass_fx" "bass_fx24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_MIX" _opt_mix)
if(_opt_mix)
    _bass_enable_plugin("bassmix" "bassmix24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_LOUD" _opt_loud)
if(_opt_loud)
    _bass_enable_plugin("bassloud" "bassloud24")
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_MIDI" _opt_midi)
if(_opt_midi)
    _bass_enable_plugin("bassmidi" "bassmidi24")
endif()

# ==============================================================================
# Process Options: Platform-Specific
# ==============================================================================

if(WIN32)
    _json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_WASAPI" _opt_wasapi)
    if(_opt_wasapi)
        _bass_enable_plugin("basswasapi" "basswasapi24")
    endif()
    
    _json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_WMA" _opt_wma)
    if(_opt_wma)
        _bass_enable_plugin("basswma" "basswma24")
    endif()
    
    _json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_SSL" _opt_ssl)
    if(_opt_ssl)
        _bass_enable_plugin("bass_ssl" "bass_ssl")
    endif()
endif()

_json_get_bool_from_key("${EXTERNAL_OPTIONS}" "BASS_HLS" _opt_hls)
if(_opt_hls)
    _bass_enable_plugin("basshls" "basshls24")
endif()

# ==============================================================================
# Complete
# ==============================================================================

message(STATUS "[${EXTERNAL_NAME}] Integration complete")
