cmake_minimum_required(VERSION 3.5)

set(TARGET_NAME CSJMediaParser)

project(${TARGET_NAME} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF (CMAKE_HOST_APPLE)
#set(CMAKE_OBJCXX_STANDARD Objective C++17)
#set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
ENDIF()
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_subdirectory(CommonTools)
set(local_libs CSJCommonTools)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/CSJMpegHeader.h
)

set(UIKIT_SOURCES
    src/CSJUIKit/CSJDialog.h
    src/CSJUIKit/CSJDialog.cpp
    src/CSJUIKit/CSJPopupWidget.h
    src/CSJUIKit/CSJPopupWidget.cpp
    src/CSJUIKit/CSJMediaPlayerWindow.h
    src/CSJUIKit/CSJMediaPlayerWindow.cpp
    src/CSJUIKit/CSJAccordionPage.h
    src/CSJUIKit/CSJAccordionPage.cpp
    src/CSJUIKit/CSJAccordionWidget.h
    src/CSJUIKit/CSJAccordionWidget.cpp
)

set(UTILS_SOURCES
    src/Utils/CSJStringUtils.h
    src/Utils/CSJLockUtils.h
    src/Utils/CSJLockUtils.cpp
    src/CSJRawData/CSJMediaRawData.h
    src/CSJRawData/CSJMediaRawData.cpp
)

set(PLAYER_SOURCES
    src/player/CSJPlayerController.h
    src/player/CSJPlayerController.cpp
    src/player/CSJPlayerKernelBase.h
    src/player/CSJPlayerKernelBase.cpp
    src/player/CSJFFPlayerKernel.h
    src/player/CSJFFPlayerKernel.cpp
    src/player/CSJVideoPresentDelegate.h
)

set(RENDER_SOURCES
    src/renderClient/CSJVideoRendererWidget.h
    src/renderClient/CSJVideoRendererWidget.cpp
)

set(MPEGTOOL_SOURCES
    src/MpegTool/CSJMpegTool.h
    src/MpegTool/CSJMpegTool.cpp
    src/MpegTool/CSJMediaDataManager.h
    src/MpegTool/CSJMediaDataManager.cpp
    src/MpegTool/CSJMediaData.h
    src/MpegTool/CSJMediaData.cpp
    src/MpegTool/CSJMpegToolWorker.h
    src/MpegTool/CSJMpegToolWorker.cpp
    src/MpegTool/CSJMpegInteractions.h
    src/MpegTool/CSJMpegInteractions.cpp
)

set(CONTROLLER_SOURCES
    src/Controllers/CSJMediaSPFDataController.h
    src/Controllers/CSJMediaSPFDataController.cpp
    src/Controllers/CSJMediaDetailModule.h
    src/Controllers/CSJMediaDetailModule.cpp
)

IF (CMAKE_HOST_WIN32)
# settings on Windows
set(PLATFORM_RENDERER_SOURCES
    src/renderClient/CSJVideoRenderer.h
    src/renderClient/CSJVideoRenderer.cpp
    src/renderClient/Win/CSJVideoRendererDXImpl.h
    src/renderClient/Win/CSJVideoRendererDXImpl.cpp
    src/renderClient/Win/DXTrace.h
    src/renderClient/Win/DXTrace.cpp
    src/renderClient/Win/d3dUtil.h
    src/renderClient/Win/d3dUtil.cpp
)
set(NativeRenderLib d3d11 dxgi dxguid d3dcompiler)
set(OpenCV_Include_DIR thirdParts/Win/opencv-4.9/include)
set(ffmpeg_inc_dir thirdParts/Win/ffmpeg/include)

link_directories(thirdParts/Win/ffmpeg/libs thirdParts/Win/opencv-4.9/libs)
set(ffmpeg_lib avcodec avdevice avfilter avformat avutil swresample swscale)
set(opencv_lib opencv_calib3d490 opencv_core490 opencv_dnn490 opencv_features2d490 opencv_flann490 opencv_highgui490 opencv_imgcodecs490 opencv_imgproc490 opencv_ml490 opencv_objdetect490 opencv_photo490 opencv_stitching490 opencv_video490)

include_directories(src ${ffmpeg_inc_dir} ${OpenCV_Include_DIR}  src/player src/MpegTool CommonTools)
ELSEIF (CMAKE_HOST_APPLE)
# settings on MacOS

add_compile_options(-x objective-c++)

# opencv dir
set(OpenCV_DIR ~/technology_space/OpenCv/opencv-4.9.0/libs/lib/cmake/opencv4)
find_package(OpenCV REQUIRED)
set(OpenCV_Include_DIR ~/technology_space/OpenCv/opencv-4.9.0/libs/include/opencv4)

# ffmpeg dir
set(ffmpeg_inc_dir thirdParts/Mac/ffmpeg/include)

# src on MacOS
set(PLATFORM_RENDERER_SOURCES
    src/renderClient/CSJVideoRenderer.h
    src/renderClient/CSJVideoRenderer.cpp
    src/renderClient/Mac/CSJVideoRendererMetalImpl.h
    src/renderClient/Mac/CSJVideoRendererMetalImpl.mm
    src/renderClient/Mac/CSJMTKRenderer.h
    src/renderClient/Mac/CSJMTKRenderer.mm)

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

include_directories(src ${ffmpeg_inc_dir} ${OpenCV_Include_DIR} src/player src/MpegTool CommonTools)
ENDIF ()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TARGET_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${UIKIT_SOURCES}
        ${UTILS_SOURCES}
        ${PLAYER_SOURCES}
        ${RENDER_SOURCES}
        ${MPEGTOOL_SOURCES}
        ${CONTROLLER_SOURCES}
        ${PLATFORM_RENDERER_SOURCES}
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

IF (CMAKE_HOST_WIN32)
target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    ${ffmpeg_lib} ${NativeRenderLib} ${local_libs} ${OpenCV_LIBRARIES}
)
ELSEIF (CMAKE_HOST_APPLE)
target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    ${lib-audiotoolbox} ${lib-bz2} ${lib-videotoolbox} ${lib-coremedia}
    ${lib-corevideo} ${lib-security} ${lib-xml2} ${lib-z} ${lib-iconv}
    ${lib-foundation} ${lib-metal} ${lib-metalKit}
    ${ffmpeg_lib} ${local_libs} ${OpenCV_LIBRARIES}
)
set_property(TARGET ${TARGET_NAME} APPEND_STRING PROPERTY
             COMPILE_FLAGS "-fobjc-arc")

ELSE()
target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
)
ENDIF()

set_target_properties(${TARGET_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${TARGET_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

file(COPY src/DXShaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${TARGET_NAME})
endif()
