cmake_minimum_required(VERSION 3.18)

project(TinaKit LANGUAGES CXX)

set(TINAKIT_MAJOR 1)
set(TINAKIT_MINOR 0)
set(TINAKIE_PATCH 0)

set(PROJECT_VERSION ${TINAKIT_MAJOR}.${TINAKIT_MINOR}.${TINAKIT_PATCH})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)

set(PROJECT_LIBRARIES)
set(PROJECT_SOURCES)

# option
option(BUILD_ASSETS "Build assets" ON)
option(BUILD_TINAKIT "Build tinakit" ON)
option(BUILD_TESTS "Build tests" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# Resources
set(TINAKIT_RESOURCE_FILES 
    "assets/test.txt"
    "assets/images/icons/error.png"
    "assets/images/icons/success.png"
)


# find_package(skia REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

list(APPEND PROJECT_LIBRARIES Qt${QT_VERSION_MAJOR}::Core)
list(APPEND PROJECT_LIBRARIES Qt${QT_VERSION_MAJOR}::Widgets)
list(APPEND PROJECT_LIBRARIES Qt${QT_VERSION_MAJOR}::Network)
list(APPEND PROJECT_LIBRARIES QXlsx::QXlsx)
list(APPEND PROJECT_LIBRARIES tinyxml2::tinyxml2)

add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies)

file(GLOB_RECURSE RC_FILES  ${PROJECT_SOURCE_DIR}/resource/*.rc)
file(GLOB_RECURSE UI_FILES  ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h**)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_FILES)

list(APPEND PROJECT_SOURCES ${HEADER_FILES} ${SOURCE_FILES} ${RC_FILES} ${UI_FILES} ${RESOURCES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TinaKit
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(TinaKit SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(TinaKit
            ${PROJECT_SOURCES}
        )
    endif()
endif()


qt_add_resources(TinaKit "TINAKIT" PREFIX "/" BASE "assets" FILES ${TINAKIT_RESOURCE_FILES})

target_compile_definitions(TinaKit PRIVATE NOMINMAX)
target_include_directories(TinaKit PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(TinaKit PRIVATE ${PROJECT_LIBRARIES})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TinaKit)
endif()
set_target_properties(TinaKit PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TinaKit
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TinaKit)
endif()




