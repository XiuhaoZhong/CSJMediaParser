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
set(OUTPUT_RESOURCE_DIR "${APP_OUTPUT_DIR}/resources")

# set Qt environment variables
if (CMAKE_HOST_WIN32)
    set(ENV{PATH} "${QTDIR}/bin;${QTDIR}/lib;$ENV{PATH}")
    add_definitions(-DUNICODE -D_UNICODE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
elseif (CMAKE_HOST_APPLE)
    set(ENV{PATH} "${QTDIR}/bin:${QTDIR}/lib:$ENV{PATH}")
endif()

set(QT_DIR $ENV{QTDIR}/lib/cmake/Qt6)
set(QT6_DIR $ENV{QTDIR}/lib/cmake/Qt6)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Gui)

file(GLOB MAIN_SOURCE_FILES "${SRC_ROOT}/main/*")
file(GLOB UIKIT_SOURCE_FILES "${SRC_ROOT}/CSJUIKit/*")
file(GLOB RENDERER_SOURCE_FILES "${SRC_ROOT}/renderClient/*")
file(GLOB CONTROLLER_SOURCE_FILES "${SRC_ROOT}/Controllers/*")
file(GLOB UIMODULES_SOURCE_FILES "${SRC_ROOT}/CSJUIModules/*")
file(GLOB LOCK_SOURCE_FILES "${SRC_ROOT}/RWLock/*")

if(CMAKE_HOST_WIN32)
# settings on Windows
    file(GLOB RENDERER_IMPL_FILES "${SRC_ROOT}/renderClient/Win/*")
    set(NativeRenderLib d3d11 dxgi dxguid d3dcompiler)
    link_directories(${GENERATE_LIB_DIR})

    include_directories(${SRC_ROOT}  ${GENERATE_HEADERS_DIR})
elseif (CMAKE_HOST_APPLE)# settings on MacOS
    # USE ARC
    add_compile_options(-x objective-c++)

    #find_package(OpenCV REQUIRED)

    # src on MacOS
    file(GLOB RENDERER_IMPL_FILES "${SRC_ROOT}/renderClient/Mac/*")

    find_library(lib-foundation Foundation)
    find_library(lib-metal Metal)
    find_library(lib-metalKit MetalKit)

    link_directories(${GENERATE_LIB_DIR})
    include_directories(${SRC_ROOT}  ${GENERATE_HEADERS_DIR})

    include_directories(${SRC_ROOT} 
                        ${QTDIR}/lib/QtGui.framework/Headers)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TARGET_NAME}
        MANUAL_FINALIZATION
        ${MAIN_SOURCE_FILES}
        ${UIKIT_SOURCE_FILES}
        ${RENDER_SOURCES}
        ${CONTROLLER_SOURCE_FILES}
        ${RENDERER_SOURCE_FILES}
        ${RENDERER_IMPL_FILES}
        ${LOCK_SOURCE_FILES}
        ${UIMODULES_SOURCE_FILES}
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
        ${NativeRenderLib} ${local_libs} 
    )
elseif(CMAKE_HOST_APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        ${lib-foundation} ${lib-metal} ${lib-metalKit}
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

target_link_libraries(${TARGET_NAME} PUBLIC CSJUtils)
target_link_libraries(${TARGET_NAME} PUBLIC CSJMediaEngine)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy 
                        $<TARGET_FILE:${TARGET_NAME}> ${APP_OUTPUT_DIR}
                   COMMENT "Copy target ${TARGET} to output directory")

file(MAKE_DIRECTORY ${OUTPUT_RESOURCE_DIR})
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy_directory 
                        ${PROJ_ROOT_DIR}/resources ${OUTPUT_RESOURCE_DIR}
                  COMMENT "Copy Resources to output directory") 

# deploy Qt libraries for windows.
if(CMAKE_HOST_WIN32)
    set(deploy_qt ON)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt6 HINTS "${QTDIR}/bin/")
    if (NOT WINDEPLOYQT_EXECUTABLE) 
        message(FATAL_ERROR "windeployqt6 not found in ${QTDIR}/bin/")
    endif()
    set(WINDEPLOYQT_OPTIONS --verbose 1 --compiler-runtime)
    if(CSJ_BUILD_DEBUG)
        list(APPEND WINDEPLOYQT_OPTIONS --debug)
        if(EXISTS ${APP_OUTPUT_DIR}/Qt6Cored.dll)
            set(deploy_qt OFF)
        endif()
    else()
        list(APPEND WINDEPLOYQT_OPTIONS --release)
        if(EXISTS ${APP_OUTPUT_DIR}/Qt6Core.dll)
            set(deploy_qt OFF)
        endif()
    endif()
    
    if(deploy_qt)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD 
            COMMAND "${CMAKE_COMMAND}" -E env PATH="${QTDIR}" ${WINDEPLOYQT_EXECUTABLE}
                ${WINDEPLOYQT_OPTIONS}
                ${APP_OUTPUT_DIR}/${TARGET_NAME}.exe
            WORKING_DIRECTORY "${APP_OUTPUT_DIR}"
            COMMENT "Depolying Qt ..."
        )    
    endif()
elseif(CMAKE_HOST_APPLE)
    if(CSJ_BUILD_PACKAGE)
        add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory 
                ${RESOURCE_DIR} 
                "$<TARGET_BUNDLE_DIR:${TARGET_NAME}>/Contents/Resources"
                COMMENT "Copy resource diretory ..."
        )
    else()
        set_target_properties(${TARGET_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH ON
            INSTALL_RPATH "@executable_path/../Frameworks"
        )

        set_target_properties(${TARGET_NAME} PROPERTIES
            MACOSX_RPATH ON
            INSTALL_NAME_DIR "@rpath"
        )

        install(TARGETS ${TARGET_NAME}
            BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX}
            COMPONENT Runtime
        )

        install(FILES ${GENERATE_LIB_DIR}/libCSJMediaEngine.dylib
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${TARGET_NAME}.app/Contents/Frameworks
            COMPONENT Runtime
        )

        set(DEVELOPER_ID "Apple Development: zxhas@foxmail.com (Q4LVL56W34)")
        install(CODE "
            execute_process(
                COMMAND ${QTDIR}/bin/macdeployqt \"${CMAKE_INSTALL_PREFIX}/${TARGET_NAME}.app\"
                        -always-overwrite
                        -verbose=1
                WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
                RESULT_VARIABLE deploy_result
                
                COMMAND codesign --force --deep --sign \"${DEVELOPER_ID}\" \"${CMAKE_INSTALL_PREFIX}/${TARGET_NAME}.app\"
            )
                
            if(NOT deploy_result EQUAL 0)
            message(STATUS \"Failed to run macdeployqt: ${deploy_result})\")
            endif()"
            COMPONENT Runtime
        )
    endif()
else()
endif()

install(TARGETS ${TARGET_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${TARGET_NAME})
endif()
