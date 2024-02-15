cmake_minimum_required(VERSION 3.5)

project(TinaKit)

# 配置
set(TINAKIT_MAJOR 1)
set(TINAKIT_MINOR 0)
set(TINAKIE_PATCH 0)

set(PROJECT_VERSION ${TINAKIT_MAJOR}.${TINAKIT_MINOR}.${TINAKIT_PATCH})
# 明明使用了插件，可是删除几次缓存，不适用这个就报错。真难用。
set(CMAKE_PREFIX_PATH "D:\\Software\\Qt\\6.6.1\\msvc2019_64\\" CACHE PATH "Qt installation prefix")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# option
option(BUILD_ASSETS "Build assets" ON)
option(BUILD_TINAKIT "Build tinakit" ON)
option(BUILD_TESTS "Build tests" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# 依赖
set(SKIA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Skia")

message(STATUS "SKIA_DIR: ${SKIA_DIR}")

if (NOT SKIA_ARCH)
    if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
        set(SKIA_ARCH "windows-x64")
    elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
        set(SKIA_ARCH "arm64")
    else ()
        set(SKIA_ARCH "x86")
    endif ()
endif ()

# Resources
set(TINAKIT_RESOURCE_FILES 
    "assets/test.txt"
    "assets/images/icon/error.png"
    "assets/images/icon/success.png"
)


# find_package(skia REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_subdirectory(3rdparty/QXlsx)


# 游历全部的头文件和源文件
file(GLOB_RECURSE SOURCE_FILE  CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR} common/*.cpp core/*.cpp)
file(GLOB_RECURSE INCLUDE_FILE CONFIGURE_DEPENDS  ${CMAKE_SOURCE_DIR}  *.h *.hpp)


set(PROJECT_SOURCES
      ${SOURCE_FILE}
      ${INCLUDE_FILE}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TinaKit
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# PREFIX 指定资源文件的前缀路径

qt_add_resources(${PROJECT_NAME} "TINAKIT" PREFIX "/" BASE "assets" FILES ${TINAKIT_RESOURCE_FILES})


target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
target_link_libraries(${PROJECT_NAME} PRIVATE QXlsx::QXlsx)
# target_link_libraries(${PROJECT_NAME} PRIVATE skottie sksg svg skparagraph skshaper skunicode skresources skia)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TinaKit)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()




