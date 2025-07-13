cmake_minimum_required(VERSION 3.15)

project(CSJMediaParser VERSION 0.1.0)

option(CSJ_BUILD_DEBUG "Build on Debug" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJ_ROOT_DIR    "${CMAKE_CURRENT_SOURCE_DIR}")
set(DEPENDENCIES_DIR "${PROJ_ROOT_DIR}/thirdParts")
set(THIRDPARTS_DIR   "${PROJ_ROOT_DIR}/src/3rdParts")
set(BUILD_DIR        "${PROJ_ROOT_DIR}/build")   
set(APP_OUTPUT_DIR   "${PROJ_ROOT_DIR}/output")    

if(CSJ_BUILD_DEBUG)
    set(APP_OUTPUT_DIR   "${APP_OUTPUT_DIR}/Debug")
else()
    set(APP_OUTPUT_DIR   "${APP_OUTPUT_DIR}/Release")
endif()

file(MAKE_DIRECTORY "${APP_OUTPUT_DIR}")

if(MSVC)
    # MSVC编译器
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
else()
    # GCC/Clang编译器
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

set(GENERATE_HEADERS_DIR "${BUILD_DIR}/include")
set(GENERATE_LIB_DIR "${PROJ_ROOT_DIR}/build/libs")
file(MAKE_DIRECTORY ${GENERATE_LIB_DIR})

add_subdirectory("${PROJ_ROOT_DIR}/src/3rdParts")
add_subdirectory("${PROJ_ROOT_DIR}/src/CSJUtils")
add_subdirectory("${PROJ_ROOT_DIR}/src/CSJMediaEngine")
add_subdirectory("${PROJ_ROOT_DIR}/src/CSJMediaParser")