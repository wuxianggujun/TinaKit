cmake_minimum_required(VERSION 3.18)

project(TinaKit VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 项目选项
option(TINAKIT_BUILD_TESTS "Build TinaKit tests" OFF)

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

# 配置minizip-ng选项 - 只启用必要的压缩格式
set(MZ_ZLIB ON CACHE BOOL "Enables ZLIB compression" FORCE)
set(MZ_BZIP2 OFF CACHE BOOL "Enables BZIP2 compression" FORCE)
set(MZ_LZMA OFF CACHE BOOL "Enables LZMA & XZ compression" FORCE)
set(MZ_ZSTD OFF CACHE BOOL "Enables ZSTD compression" FORCE)
set(MZ_FETCH_LIBS OFF CACHE BOOL "Disable fetching third-party libraries" FORCE)
set(MZ_COMPAT ON CACHE BOOL "Enables compatibility layer" FORCE)

add_subdirectory(third_party/minizip-ng)

# 添加源代码目录
add_subdirectory(src)

# 添加示例
add_subdirectory(examples)

# 构建测试
if(TINAKIT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
