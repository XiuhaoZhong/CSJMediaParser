cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET_NAME "CSJMediaEngine")
set(CSJMEDIAENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CSJMediaEngine path: ${CMAKE_CURRENT_SOURCE_DIR}")

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_INSTALL_BINDIR ${OUTPUT_DIR})
set(CMAKE_INSTALL_LIBDIR ${GENERATE_LIB_DIR})

file(GLOB_RECURSE ENGINE_SOURCE_FILES "${CSJMEDIAENGINE_DIR}/CSJMediaEngineInfo/*"
                                        "${CSJMEDIAENGINE_DIR}/CSJMediaPlayer/*"
                                        "${CSJMEDIAENGINE_DIR}/include/*")

set(CommonTool_srcs CSJMediaData.cpp)

# copy headers to build dir, so as to be used by others.
add_custom_target(
    COPY_HEADERS
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CSJMEDIAENGINE_DIR}/include" "${BUILD_DIR}/include/${TARGET_NAME}"
)

add_library(${TARGET_NAME} SHARED ${ENGINE_SOURCE_FILES})

add_dependencies(${TARGET_NAME} COPY_HEADERS)

target_compile_definitions(${TARGET_NAME} PRIVATE CSJMEDIAENGINE_EXPORTS)

include_directories(${CSJMEDIAENGINE_DIR}/include)

add_custom_command(
    TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${TARGET_NAME}> ${OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.lib ${GENERATE_LIB_DIR} 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.pdb ${GENERATE_LIB_DIR} 
)

# install(TARGETS ${TARGET_NAME} 
#     LIBRARY DESTINATION ${GENERATE_LIB_DIR}
#     RUNTIME DESTINATION ${OUTPUT_DIR}
# )

