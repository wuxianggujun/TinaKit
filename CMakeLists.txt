cmake_minimum_required(VERSION 3.18)

project(TinaKit VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器选项
if(MSVC)
    # 设置 UTF-8 编码
    add_compile_options(/utf-8)
    # 设置警告级别
    add_compile_options(/W4)
else()
    # GCC/Clang 编译选项
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 项目选项
option(TINAKIT_BUILD_TESTS "Build TinaKit tests" ON)
option(TINAKIT_BUILD_EXAMPLES "Build TinaKit examples" ON)

# 检查并添加utfcpp库
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/utfcpp")
    # 使用本地的utfcpp库
    set(UTF8_TESTS OFF CACHE BOOL "Enable tests for utf8cpp" FORCE)
    set(UTF8_INSTALL OFF CACHE BOOL "Enable installation for utf8cpp" FORCE)
    set(UTF8_SAMPLES OFF CACHE BOOL "Enable samples for utf8cpp" FORCE)
    add_subdirectory(third_party/utfcpp)
    message(STATUS "Using local utfcpp library")
else()
    # 如果没有本地库，尝试使用FetchContent
    include(FetchContent)
    FetchContent_Declare(
        utfcpp
        GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
        GIT_TAG        v4.0.5
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(utfcpp)
    message(STATUS "Downloaded utfcpp library using FetchContent")
endif()

# 添加第三方库
add_subdirectory(third_party/libstudxml)

# 配置zlib-ng选项
set(ZLIB_COMPAT ON CACHE BOOL "Compile with zlib compatible API" FORCE)
set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "Build test binaries" FORCE)
set(ZLIBNG_ENABLE_TESTS OFF CACHE BOOL "Test zlib-ng specific API" FORCE)
set(WITH_GTEST OFF CACHE BOOL "Build gtest_zlib" FORCE)
set(WITH_BENCHMARKS OFF CACHE BOOL "Build test/benchmarks" FORCE)
set(WITH_FUZZERS OFF CACHE BOOL "Build test/fuzz" FORCE)

add_subdirectory(third_party/zlib-ng)

# 配置minizip-ng选项 - 确保找到我们的 zlib-ng
set(MZ_ZLIB ON CACHE BOOL "Enables ZLIB compression" FORCE)
set(MZ_BZIP2 OFF CACHE BOOL "Enables BZIP2 compression" FORCE)
set(MZ_LZMA OFF CACHE BOOL "Enables LZMA & XZ compression" FORCE)
set(MZ_ZSTD OFF CACHE BOOL "Enables ZSTD compression" FORCE)
set(MZ_FETCH_LIBS OFF CACHE BOOL "Disable fetching third-party libraries" FORCE)

# 告诉 minizip-ng 使用我们构建的 zlib-ng
set(ZLIB_ROOT ${CMAKE_BINARY_DIR}/third_party/zlib-ng CACHE PATH "ZLIB root directory" FORCE)
set(ZLIB_INCLUDE_DIR ${CMAKE_BINARY_DIR}/third_party/zlib-ng CACHE PATH "ZLIB include directory" FORCE)

# 使用更通用的库名称配置
if(WIN32)
    set(ZLIB_LIBRARY_DEBUG ${CMAKE_BINARY_DIR}/third_party/zlib-ng/zlibd.lib CACHE FILEPATH "ZLIB debug library" FORCE)
    set(ZLIB_LIBRARY_RELEASE ${CMAKE_BINARY_DIR}/third_party/zlib-ng/zlib.lib CACHE FILEPATH "ZLIB release library" FORCE)
else()
    set(ZLIB_LIBRARY_DEBUG ${CMAKE_BINARY_DIR}/third_party/zlib-ng/libzlibd.a CACHE FILEPATH "ZLIB debug library" FORCE)
    set(ZLIB_LIBRARY_RELEASE ${CMAKE_BINARY_DIR}/third_party/zlib-ng/libzlib.a CACHE FILEPATH "ZLIB release library" FORCE)
endif()

add_subdirectory(third_party/minizip-ng)

# 添加源代码目录
add_subdirectory(src)

# 构建示例程序（可选）
if(TINAKIT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# 构建测试
if(TINAKIT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ========================================
# DLL复制将在构建时处理
# ========================================

# 注意：DLL复制在各个目标的CMakeLists.txt中单独处理

# ========================================
# 便利构建目标
# ========================================

# 构建所有目标
add_custom_target(build_all
    DEPENDS tinakit
    COMMENT "Building TinaKit library"
)

if(TINAKIT_BUILD_TESTS)
    add_dependencies(build_all tinakit_tests)
endif()

# 快速测试目标
if(TINAKIT_BUILD_TESTS)
    add_custom_target(quick_test
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS tinakit_tests
        COMMENT "Running quick tests"
    )
endif()

# 显示构建配置信息
message(STATUS "")
message(STATUS "TinaKit Build Configuration:")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Tests:    ${TINAKIT_BUILD_TESTS}")
message(STATUS "  Build Examples: ${TINAKIT_BUILD_EXAMPLES}")
message(STATUS "")
