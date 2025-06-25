cmake_minimum_required(VERSION 3.10)

set(TARGET_NAME "CSJMediaEngine")
set(CSJMEDIAENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CSJMediaEngine path: ${CMAKE_CURRENT_SOURCE_DIR}")

set(CommonTool_srcs CSJMediaData.cpp)

add_library(${TARGET_NAME} SHARED ${CommonTool_srcs})

include_directories(${CSJMEDIAENGINE_DIR}/include)