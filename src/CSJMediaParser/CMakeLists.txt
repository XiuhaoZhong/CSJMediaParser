cmake_minimum_required(VERSION 3.15)

set(TARGET_NAME CSJMediaParser)

project(${TARGET_NAME} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CSJMEDIAPARSER_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_ROOT "${CSJMEDIAPARSER_ROOT_DIR}")

set(QTDIR $ENV{QTDIR_6_8_0})

if (CSJ_BUILD_DEBUG)
    set(QTDIR ${QTDIR}/debug)
else()
    set(QTDIR ${QTDIR}/release)
endif()
# set the resource directory in the output directory
set(OUTPUT_RESOURCE_DIR "${APP_APP_OUTPUT_DIR}/resource")

set(DYNAMIC_LIBS "")

# set Qt environment variables
if (CMAKE_HOST_WIN32)
    set(ENV{PATH} "${QTDIR}/bin;${QTDIR}/lib;$ENV{PATH}")
    file(GLOB DYNAMIC_LIBS "${DEPENDENCIES_DIR}/Win/ffmpeg/dlls/*.dll")
    add_definitions(-DUNICODE -D_UNICODE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
elseif (CMAKE_HOST_APPLE)
    set(ENV{PATH} "${QTDIR}/bin:${QTDIR}/lib:$ENV{PATH}")
endif()

set(QT_DIR $ENV{QTDIR}/lib/cmake/Qt6)
set(QT6_DIR $ENV{QTDIR}/lib/cmake/Qt6)

add_compile_definitions(CSJMEDIAENGINE_EXPORTS)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui)

file(GLOB MAIN_SOURCE_FILES "${SRC_ROOT}/main/*")
file(GLOB MPEG_HEADER_FILES "${SRC_ROOT}/MpegHeaders/*")
file(GLOB UIKIT_SOURCE_FILES "${SRC_ROOT}/CSJUIKit/*")
file(GLOB MPEGTOOL_SOURCE_FILES "${SRC_ROOT}/MpegTool/*")
file(GLOB PLAYER_SOURCE_FILES "${SRC_ROOT}/player/*")
file(GLOB RENDERER_SOURCE_FILES "${SRC_ROOT}/renderClient/*")
file(GLOB UTIL_SOURCE_FILES "${SRC_ROOT}/Utils/*")
file(GLOB CONTROLLER_SOURCE_FILES "${SRC_ROOT}/Controllers/*")

if(CMAKE_HOST_WIN32)
# settings on Windows
    file(GLOB RENDERER_IMPL_FILES "${SRC_ROOT}/renderClient/Win/*")

    set(NativeRenderLib d3d11 dxgi dxguid d3dcompiler)
    set(ffmpeg_inc_dir ${DEPENDENCIES_DIR}/Win/ffmpeg/include)

    link_directories(${DEPENDENCIES_DIR}/Win/ffmpeg/libs ${GENERATE_LIB_DIR})
    set(ffmpeg_lib avcodec avdevice avfilter avformat avutil swresample swscale)

    include_directories(${ffmpeg_inc_dir} player MpegTool ${GENERATE_HEADERS_DIR})
elseif (CMAKE_HOST_APPLE)# settings on MacOS
    # USE ARC
    add_compile_options(-x objective-c++)

    find_package(OpenCV REQUIRED)

    # ffmpeg dir
    set(ffmpeg_inc_dir thirdParts/Mac/ffmpeg/include)

    # src on MacOS
    file(GLOB RENDERER_IMPL_FILES "${SRC_ROOT}/renderClient/Mac/*")

    link_directories(thirdParts/Mac/ffmpeg/libs)

    find_library(lib-videotoolbox VideoToolbox)
    find_library(lib-audiotoolbox AudioToolbox)
    find_library(lib-avfoundation AVFoundation)
    find_library(lib-coreaudio CoreAudio)
    find_library(lib-coremedia CoreMedia)
    find_library(lib-corevideo CoreVideo)
    find_library(lib-security Security)
    find_library(lib-iconv iconv.2)
    find_library(lib-z z)
    find_library(lib-bz2 bz2)
    find_library(lib-xml2 xml2)
    find_library(lib-foundation Foundation)
    find_library(lib-metal Metal)
    find_library(lib-metalKit MetalKit)

    set(ffmpeg_lib avcodec avdevice avfilter avformat avutil swresample swscale)

    include_directories(${SRC_ROOT} 
                        ${ffmpeg_inc_dir} 
                        ${OpenCV_Include_DIR} 
                        src/player 
                        src/MpegTool 
                        CommonTools
                        ${QTDIR}/lib/QtGui.framework/Headers)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TARGET_NAME}
        MANUAL_FINALIZATION
        ${MAIN_SOURCE_FILES}
        ${MPEG_HEADER_FILES}
        ${UIKIT_SOURCE_FILES}
        ${UTIL_SOURCE_FILES}
        ${PLAYER_SOURCE_FILES}
        ${RENDER_SOURCES}
        ${MPEGTOOL_SOURCE_FILES}
        ${CONTROLLER_SOURCE_FILES}
        ${RENDERER_SOURCE_FILES}
        ${RENDERER_IMPL_FILES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${TARGET_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${TARGET_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

if(CMAKE_HOST_WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        ${ffmpeg_lib} ${NativeRenderLib} ${local_libs} 
    ) # ${OpenCV_LIBRARIES}
elseif(CMAKE_HOST_APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        ${lib-audiotoolbox} ${lib-bz2} ${lib-videotoolbox} ${lib-coremedia}
        ${lib-corevideo} ${lib-security} ${lib-xml2} ${lib-z} ${lib-iconv}
        ${lib-foundation} ${lib-metal} ${lib-metalKit}
        ${ffmpeg_lib} ${local_libs} ${OpenCV_LIBRARIES}
    )
    set_property(TARGET ${TARGET_NAME} APPEND_STRING PROPERTY
        COMPILE_FLAGS "-fobjc-arc")
else()
    target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${TARGET_NAME} PUBLIC spdlog::spdlog)
target_link_libraries(${TARGET_NAME} PUBLIC CSJMediaEngine)

add_custom_command(TARGET ${TARGET_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_NAME}> ${APP_OUTPUT_DIR}
                   COMMENT "Copy target ${TARGET} to output directory")

if(CMAKE_HOST_WIN32)
    foreach(file ${DYNAMIC_LIBS})
        add_custom_command(TARGET ${TARGET_NAME}
                            POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy ${file} ${APP_OUTPUT_DIR})
    endforeach()
endif()

file(MAKE_DIRECTORY ${OUTPUT_RESOURCE_DIR})
add_custom_command(TARGET ${TARGET_NAME}
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJ_ROOT_DIR}/resources ${OUTPUT_RESOURCE_DIR}
                  COMMENT "Copy Resources to output directory") 

if(CSJ_BUILD_DEBUG)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD 
        COMMAND "${CMAKE_COMMAND}" -E env PATH="${QTDIR}" "${QTDIR}/bin/windeployqt6"
            --verbose 1
            --debug
            --compiler-runtime
            ${APP_OUTPUT_DIR}/${TARGET_NAME}.exe#$<TARGET_FILE:${TARGET_NAME}>
        WORKING_DIRECTORY "${APP_OUTPUT_DIR}"
        COMMENT "Depolying Qt ..."
    )
else()
endif()

install(TARGETS ${TARGET_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${TARGET_NAME})
endif()
