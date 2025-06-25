cmake_minimum_required(VERSION 3.10)

project(CSJMediaParser VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJ_ROOT_DIR    "${CMAKE_CURRENT_SOURCE_DIR}")
set(DEPENDENCIES_DIR "${PROJ_ROOT_DIR}/thirdParts")
set(APP_OUTPUT_DIR   "${PROJ_ROOT_DIR}/output")
set(THIRDPARTS_DIR   "${PROJ_ROOT_DIR}/src/3rdParts")

if(MSVC)
    # MSVC编译器
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
else()
    # GCC/Clang编译器
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3 -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

add_subdirectory("${PROJ_ROOT_DIR}/src/3rdParts")
add_subdirectory("${PROJ_ROOT_DIR}/src/CSJMediaEngine")
add_subdirectory("${PROJ_ROOT_DIR}/src/CSJMediaParser")