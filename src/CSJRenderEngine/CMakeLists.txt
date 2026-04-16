cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET_NAME "CSJRenderEngine")
set(CSJRENDERENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_INSTALL_BINDIR ${APP_OUTPUT_DIR})
set(CMAKE_INSTALL_LIBDIR ${GENERATE_LIB_DIR})

file(GLOB ENGINE_SOURCE_FILES "${CSJRENDERENGINE_DIR}/CSJVideoRenderer.*")

# copy headers to build dir, so as to be used by others.
add_custom_target(COPY_RENDERENGINE_HEADERS
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUILD_DIR}/include/${TARGET_NAME}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CSJRENDERENGINE_DIR}/CSJVideoRenderer.h"
        "${CSJRENDERENGINE_DIR}/CSJRenderEngine_Export.h"
        "${BUILD_DIR}/include/${TARGET_NAME}/"
)

if(CMAKE_HOST_WIN32)
# settings on Windows
    file(GLOB_RECURSE RENDERER_IMPL_FILES "${CSJRENDERENGINE_DIR}/Win/*")
    set(NativeRenderLib d3d11 dxgi dxguid d3dcompiler)

elseif (CMAKE_HOST_APPLE)# settings on MacOS
    # USE ARC
    add_compile_options(-x objective-c++)

    # src on MacOS
    file(GLOB_RECURSE RENDERER_IMPL_FILES "${CSJRENDERENGINE_DIR}/Mac/*")

    find_library(lib-foundation Foundation)
    find_library(lib-metal Metal)
    find_library(lib-metalKit MetalKit)
    find_library(lib-quartzcore QuartzCore)
    find_library(lib-coreGraphic CoreGraphics)
    find_library(lib-appKit AppKit)
endif()

link_directories(${GENERATE_LIB_DIR})
include_directories(${GENERATE_HEADERS_DIR})

include_directories(${CSJMEDIAENGINE_DIR}/include ${GENERATE_HEADERS_DIR})

add_library(${TARGET_NAME} SHARED ${ENGINE_SOURCE_FILES} ${RENDERER_IMPL_FILES})
add_dependencies(${TARGET_NAME} COPY_RENDERENGINE_HEADERS)

if(CMAKE_HOST_APPLE)
    file(GLOB SHADER_SOURCES "${CSJRENDERENGINE_DIR}/Mac/Shaders/*.metal")

    set(METAL_OUTPUT "${CSJRENDERENGINE_DIR}/Mac/Shaders/default.metallib")
    add_custom_command(
        OUTPUT ${METAL_OUTPUT}
        COMMAND xcrun -sdk macosx metal 
            ${SHADER_SOURCES}
            -o ${METAL_OUTPUT}
        DEPENDS ${SHADER_SOURCES}
        COMMENT "Building Metal library (default.metallib)"
    )

    add_custom_target(MetalShaders ALL DEPENDS ${METAL_OUTPUT})

    add_dependencies(COPY_RENDERENGINE_HEADERS MetalShaders)
endif()

target_compile_definitions(${TARGET_NAME} PRIVATE CSJRENDERENGINE_EXPORTS)

if(CMAKE_HOST_WIN32)
    
elseif(CMAKE_HOST_APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE 
        ${lib-foundation} ${lib-metal} ${lib-metalKit} ${lib-quartzcore} ${lib-coreGraphic} ${lib-appKit}
    )
else()
endif()

target_link_libraries(${TARGET_NAME} PUBLIC CSJUtils)
target_link_libraries(${TARGET_NAME} PUBLIC CSJMediaEngine)

if(CMAKE_HOST_WIN32)
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${TARGET_NAME}>" ${APP_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.lib ${GENERATE_LIB_DIR} 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:${TARGET_NAME}>/$<TARGET_FILE_BASE_NAME:${TARGET_NAME}>.pdb ${GENERATE_LIB_DIR} 
    )
elseif(CMAKE_HOST_APPLE)
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${TARGET_NAME}>" ${GENERATE_LIB_DIR}
    )
else()

endif()

