# cmake/core/Json.cmake
# ======================
# JSON helper functions for the CMake build system
#
# Version: 1.0.0
# Date:    2025-12-26
# Status:  Release
# Author:  CMake Architecture Team
#
# Dependencies:
#   - CMake 3.19+ (for string(JSON ...))
#
# Provides:
#   - _json_has_key()               - Check if key exists
#   - _json_get_string()            - Read string value
#   - _json_get_string_or_default() - String with fallback
#   - _json_get_number()            - Read numeric value
#   - _json_get_number_or_default() - Number with fallback
#   - _json_get_bool_from_key()     - Read boolean value (robust)
#   - _json_get_bool_or_default()   - Boolean with fallback
#   - _json_array_length()          - Get array length
#   - _json_array_get()             - Read array element
#   - _json_get_array_as_list()     - Read array as CMake list
#   - _json_get_object()            - Extract object
#   - _json_get_object_or_empty()   - Object with fallback
#   - _json_get_type()              - Get type of a key
#
# Used by:
#   - Solution.cmake
#   - ExecutableCollect.cmake
#   - LibraryCollect.cmake
#   - TestCollect.cmake
#   - AppCollect.cmake
#   - Validation.cmake
#
# Note: All functions are marked "private" (_prefix),
# as they are intended for internal build system use only.

include_guard(GLOBAL)

# ============================================================================
# _json_has_key - Check if key exists
# ============================================================================
#[[
    _json_has_key(JSON_STRING KEY OUT_VAR)
    
    Checks if a key exists in the JSON object.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key to check
        OUT_VAR     - Mandatory: Output: TRUE or FALSE
    
    Example:
        _json_has_key("${_json}" "name" _has_name)
        if(_has_name)
            message("Name exists")
        endif()
]]
function(_json_has_key JSON_STRING KEY OUT_VAR)
    string(JSON _type ERROR_VARIABLE _err TYPE "${JSON_STRING}" "${KEY}")
    if(_err)
        set(${OUT_VAR} FALSE PARENT_SCOPE)
    else()
        set(${OUT_VAR} TRUE PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_string - Read string value
# ============================================================================
#[[
    _json_get_string(JSON_STRING KEY OUT_VAR)
    
    Reads a string value from JSON.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key
        OUT_VAR     - Mandatory: Output: String value or ""
    
    Returns:
        String value on success, empty string if key is missing.
    
    Example:
        _json_get_string("${_json}" "name" _name)
]]
function(_json_get_string JSON_STRING KEY OUT_VAR)
    string(JSON _value ERROR_VARIABLE _err GET "${JSON_STRING}" "${KEY}")
    if(_err)
        set(${OUT_VAR} "" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${_value}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_string_or_default - String with default
# ============================================================================
#[[
    _json_get_string_or_default(JSON_STRING KEY DEFAULT OUT_VAR)
    
    Reads string value with fallback to default.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key
        DEFAULT     - Mandatory: Fallback value if key missing/empty
        OUT_VAR     - Mandatory: Output: String value or DEFAULT
    
    Example:
        _json_get_string_or_default("${_json}" "type" "CONSOLE" _type)
]]
function(_json_get_string_or_default JSON_STRING KEY DEFAULT OUT_VAR)
    _json_get_string("${JSON_STRING}" "${KEY}" _value)
    if("${_value}" STREQUAL "")
        set(${OUT_VAR} "${DEFAULT}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${_value}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_number - Read numeric value
# ============================================================================
#[[
    _json_get_number(JSON_STRING KEY OUT_VAR)
    
    Reads a numeric value from JSON.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key
        OUT_VAR     - Mandatory: Output: Number value or ""
    
    Returns:
        Number value on success, empty string if key is missing.
    
    Example:
        _json_get_number("${_json}" "timeout" _timeout)
]]
function(_json_get_number JSON_STRING KEY OUT_VAR)
    string(JSON _value ERROR_VARIABLE _err GET "${JSON_STRING}" "${KEY}")
    if(_err)
        set(${OUT_VAR} "" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${_value}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_number_or_default - Number with default
# ============================================================================
#[[
    _json_get_number_or_default(JSON_STRING KEY DEFAULT OUT_VAR)
    
    Reads numeric value with fallback to default.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key
        DEFAULT     - Mandatory: Fallback value if key missing
        OUT_VAR     - Mandatory: Output: Number value or DEFAULT
    
    Example:
        _json_get_number_or_default("${_json}" "timeout" 30 _timeout)
]]
function(_json_get_number_or_default JSON_STRING KEY DEFAULT OUT_VAR)
    _json_get_number("${JSON_STRING}" "${KEY}" _value)
    if("${_value}" STREQUAL "")
        set(${OUT_VAR} "${DEFAULT}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${_value}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_bool_from_key - Read boolean value (robust)
# ============================================================================
#[[
    _json_get_bool_from_key(JSON_STRING KEY OUT_VAR)
    
    Reads a boolean value with robust detection.
    
    IMPORTANT: CMake's string(JSON GET) returns for JSON true/false
    the strings "true"/"false" (lowercase).
    This function recognizes all common variants.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key
        OUT_VAR     - Mandatory: Output: TRUE or FALSE
    
    TRUE values: true, TRUE, 1, ON, YES
    FALSE values: false, FALSE, 0, OFF, NO, empty, key missing
    
    Example:
        _json_get_bool_from_key("${_json}" "skip" _skip)
        if(_skip)
            message("Skipped")
        endif()
]]
function(_json_get_bool_from_key JSON_STRING KEY OUT_VAR)
    string(JSON _value ERROR_VARIABLE _err GET "${JSON_STRING}" "${KEY}")
    
    # Error or key doesn't exist -> FALSE
    if(_err)
        set(${OUT_VAR} FALSE PARENT_SCOPE)
        return()
    endif()
    
    # Empty value -> FALSE
    if("${_value}" STREQUAL "")
        set(${OUT_VAR} FALSE PARENT_SCOPE)
        return()
    endif()
    
    # Uppercase for comparison (robust detection)
    string(TOUPPER "${_value}" _value_upper)
    
    # TRUE values: true, TRUE, 1, ON, YES
    if("${_value_upper}" STREQUAL "TRUE" OR 
       "${_value_upper}" STREQUAL "1" OR 
       "${_value_upper}" STREQUAL "ON" OR 
       "${_value_upper}" STREQUAL "YES")
        set(${OUT_VAR} TRUE PARENT_SCOPE)
    else()
        # Everything else (false, FALSE, 0, OFF, NO, etc.) -> FALSE
        set(${OUT_VAR} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_bool_or_default - Boolean with default
# ============================================================================
#[[
    _json_get_bool_or_default(JSON_STRING KEY DEFAULT OUT_VAR)
    
    Reads boolean value with fallback to default if key is missing.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key
        DEFAULT     - Mandatory: Fallback value if key missing (TRUE or FALSE)
        OUT_VAR     - Mandatory: Output: TRUE or FALSE
    
    TRUE values: true, TRUE, 1, ON, YES
    FALSE values: false, FALSE, 0, OFF, NO
    
    Example:
        _json_get_bool_or_default("${_json}" "skip" FALSE _skip)
        if(_skip)
            message("Skipped")
        endif()
]]
function(_json_get_bool_or_default JSON_STRING KEY DEFAULT OUT_VAR)
    _json_has_key("${JSON_STRING}" "${KEY}" _has_key)
    if(_has_key)
        _json_get_bool_from_key("${JSON_STRING}" "${KEY}" _result)
        set(${OUT_VAR} "${_result}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${DEFAULT}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_array_length - Array length
# ============================================================================
#[[
    _json_array_length(JSON_STRING KEY OUT_VAR)
    
    Gets the length of a JSON array.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key of the array
        OUT_VAR     - Mandatory: Output: Array length (0 if missing/not array)
    
    Example:
        _json_array_length("${_json}" "externals" _count)
        if(_count GREATER 0)
            # Process array
        endif()
]]
function(_json_array_length JSON_STRING KEY OUT_VAR)
    string(JSON _len ERROR_VARIABLE _err LENGTH "${JSON_STRING}" "${KEY}")
    if(_err)
        set(${OUT_VAR} 0 PARENT_SCOPE)
    else()
        set(${OUT_VAR} ${_len} PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_array_get - Read array element
# ============================================================================
#[[
    _json_array_get(JSON_STRING KEY INDEX OUT_VAR)
    
    Reads an element from a JSON array.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key of the array
        INDEX       - Mandatory: 0-based index
        OUT_VAR     - Mandatory: Output: Element value or ""
    
    Example:
        _json_array_length("${_json}" "items" _count)
        math(EXPR _last "${_count} - 1")
        foreach(_idx RANGE 0 ${_last})
            _json_array_get("${_json}" "items" ${_idx} _item)
            message("Item: ${_item}")
        endforeach()
]]
function(_json_array_get JSON_STRING KEY INDEX OUT_VAR)
    string(JSON _value ERROR_VARIABLE _err GET "${JSON_STRING}" "${KEY}" ${INDEX})
    if(_err)
        set(${OUT_VAR} "" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${_value}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_array_as_list - Read array as CMake list
# ============================================================================
#[[
    _json_get_array_as_list(JSON_STRING KEY OUT_VAR)
    
    Reads a JSON array and returns it as a CMake semicolon-separated list.
    Returns empty string if key doesn't exist or array is empty.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key of the array
        OUT_VAR     - Mandatory: Output: CMake list or ""
    
    Example:
        _json_get_array_as_list("${_json}" "dependencies" _deps)
        foreach(_dep IN LISTS _deps)
            message("Dependency: ${_dep}")
        endforeach()
]]
function(_json_get_array_as_list JSON_STRING KEY OUT_VAR)
    _json_has_key("${JSON_STRING}" "${KEY}" _has_key)
    if(NOT _has_key)
        set(${OUT_VAR} "" PARENT_SCOPE)
        return()
    endif()
    
    _json_array_length("${JSON_STRING}" "${KEY}" _count)
    
    set(_result "")
    if(_count GREATER 0)
        math(EXPR _last "${_count} - 1")
        foreach(_i RANGE 0 ${_last})
            _json_array_get("${JSON_STRING}" "${KEY}" ${_i} _item)
            list(APPEND _result "${_item}")
        endforeach()
    endif()
    
    set(${OUT_VAR} "${_result}" PARENT_SCOPE)
endfunction()

# ============================================================================
# _json_get_object - Extract object
# ============================================================================
#[[
    _json_get_object(JSON_STRING KEY OUT_VAR)
    
    Extracts a nested JSON object as string.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key of the object
        OUT_VAR     - Mandatory: Output: JSON string of the object or ""
    
    Example:
        _json_get_object("${_json}" "settings" _settings)
        _json_get_string("${_settings}" "cxx_standard" _std)
]]
function(_json_get_object JSON_STRING KEY OUT_VAR)
    string(JSON _value ERROR_VARIABLE _err GET "${JSON_STRING}" "${KEY}")
    if(_err)
        set(${OUT_VAR} "" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${_value}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_object_or_empty - Object with fallback
# ============================================================================
#[[
    _json_get_object_or_empty(JSON_STRING KEY OUT_VAR)
    
    Like _json_get_object, but returns "{}" if key is missing.
    Enables safe further processing.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key of the object
        OUT_VAR     - Mandatory: Output: JSON string or "{}"
    
    Example:
        _json_get_object_or_empty("${_json}" "options" _options)
        # _options is now at least "{}", never empty
        _json_get_string_or_default("${_options}" "key" "default" _val)
]]
function(_json_get_object_or_empty JSON_STRING KEY OUT_VAR)
    _json_get_object("${JSON_STRING}" "${KEY}" _value)
    if("${_value}" STREQUAL "")
        set(${OUT_VAR} "{}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${_value}" PARENT_SCOPE)
    endif()
endfunction()

# ============================================================================
# _json_get_type - Get type of a key
# ============================================================================
#[[
    _json_get_type(JSON_STRING KEY OUT_VAR)
    
    Gets the JSON type of a value.
    
    Parameters:
        JSON_STRING - Mandatory: JSON object as string
        KEY         - Mandatory: Key
        OUT_VAR     - Mandatory: Output: Type string
    
    Possible return values:
        NULL    - null
        BOOLEAN - true/false
        NUMBER  - Numeric
        STRING  - String
        ARRAY   - Array [...]
        OBJECT  - Object {...}
        ""      - Key doesn't exist
    
    Example:
        _json_get_type("${_json}" "version" _vtype)
        if("${_vtype}" STREQUAL "STRING")
            _json_get_string("${_json}" "version" _ver)
        elseif("${_vtype}" STREQUAL "OBJECT")
            _json_get_object("${_json}" "version" _vobj)
        endif()
]]
function(_json_get_type JSON_STRING KEY OUT_VAR)
    string(JSON _type ERROR_VARIABLE _err TYPE "${JSON_STRING}" "${KEY}")
    if(_err)
        set(${OUT_VAR} "" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${_type}" PARENT_SCOPE)
    endif()
endfunction()
