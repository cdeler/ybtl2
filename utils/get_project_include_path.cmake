# Taken from https://github.com/cdeler/cmocker/blob/master/utils/GetIncludePath.cmake
# The original license is MIT (commit 24ebaf4e7de32632f2666218a9f50f07fa937024)

FUNCTION(GET_INCLUDE_PATH _project_root _include_path_result)
    SET(RESULT_INCLUDE_PATH)
    FILE(GLOB_RECURSE ALL_HEADERS "${_project_root}/*.h")
    FOREACH (HEADER_FILE ${ALL_HEADERS})
        GET_FILENAME_COMPONENT(HEADER_PATH ${HEADER_FILE} DIRECTORY)
        LIST(APPEND RESULT_INCLUDE_PATH ${HEADER_PATH})
    ENDFOREACH (HEADER_FILE)

    LIST(APPEND RESULT_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/generated/include/)

    LIST(LENGTH "${RESULT_INCLUDE_PATH}" RESULT_LENGTH)

    IF (LENGTH GREATER 1)
        LIST(REMOVE_DUPLICATES ${RESULT_INCLUDE_PATH})
    ENDIF (LENGTH GREATER 1)

    SET(${_include_path_result} "${RESULT_INCLUDE_PATH}" PARENT_SCOPE)
ENDFUNCTION(GET_INCLUDE_PATH _project_root _include_path_result)