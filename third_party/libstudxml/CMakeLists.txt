cmake_minimum_required(VERSION 3.18)
project(libstudxml LANGUAGES C CXX)

# 设置 C++20 标准以匹配 TinaKit 项目
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置路径变量
set(LIBSTUDXML_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 支持静态 CRT（Windows）
if(STATIC_CRT AND WIN32)
  set_property(GLOBAL PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# 定义源文件
set(LIBSTUDXML_SOURCES
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/parser.cxx
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/qname.cxx
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/serializer.cxx
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/value-traits.cxx
)

# Genx 源文件
set(GENX_SOURCES
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/details/genx/char-props.c
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/details/genx/genx.c
)

# Expat 源文件
set(EXPAT_SOURCES
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/details/expat/xmlparse.c
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/details/expat/xmlrole.c
  ${LIBSTUDXML_ROOT_DIR}/libstudxml/details/expat/xmltok.c
)

# 创建 OBJECT 库（与原始配置保持一致）
add_library(libstudxml OBJECT
  ${LIBSTUDXML_SOURCES}
  ${GENX_SOURCES}
  ${EXPAT_SOURCES}
)

# 配置编译定义
target_compile_definitions(libstudxml PUBLIC LIBSTUDXML_STATIC_LIB=1)

# 配置包含目录
target_include_directories(libstudxml
  PUBLIC ${LIBSTUDXML_ROOT_DIR}
)

# 使用内置 Expat，设置静态链接定义
target_compile_definitions(libstudxml PUBLIC XML_STATIC=1)

# 防止 genx 中由 strcpy, strncpy, sprintf 引起的警告 C4996
if(MSVC)
  target_compile_definitions(libstudxml PRIVATE _CRT_SECURE_NO_WARNINGS=1)
endif()

# 当构建共享库时使用 -fPIC
if(NOT STATIC)
  set_target_properties(libstudxml PROPERTIES POSITION_INDEPENDENT_CODE 1)
endif()
