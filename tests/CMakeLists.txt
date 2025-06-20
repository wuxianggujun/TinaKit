# TinaKit 测试配置
cmake_minimum_required(VERSION 3.20)

# 创建测试可执行文件
add_executable(tinakit_tests
    main.cpp
    test_coordinate_system.cpp
    test_range_system.cpp
    test_integration.cpp
    test_advanced_features.cpp
)

# 链接TinaKit库
target_link_libraries(tinakit_tests PRIVATE tinakit)

# 包含测试框架头文件
target_include_directories(tinakit_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 设置C++20标准
target_compile_features(tinakit_tests PRIVATE cxx_std_20)

# 设置编译选项
if(MSVC)
    target_compile_options(tinakit_tests PRIVATE /W4 /utf-8)
else()
    target_compile_options(tinakit_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 添加测试到CTest
enable_testing()

# 添加所有测试
add_test(NAME AllTests COMMAND tinakit_tests)

# 添加特定测试套件
add_test(NAME CoordinateSystemTests COMMAND tinakit_tests CoordinateSystem)
add_test(NAME RangeSystemTests COMMAND tinakit_tests RangeSystem)
add_test(NAME CoordinateUtilsTests COMMAND tinakit_tests CoordinateUtils)
add_test(NAME ColumnConversionTests COMMAND tinakit_tests ColumnConversion)
add_test(NAME RangeAddressTests COMMAND tinakit_tests RangeAddress)
add_test(NAME ValidationTests COMMAND tinakit_tests Validation)
add_test(NAME IntegrationTests COMMAND tinakit_tests Integration)
add_test(NAME RangeBatchOperationsTests COMMAND tinakit_tests RangeBatchOperations)
add_test(NAME StyleApplicationTests COMMAND tinakit_tests StyleApplication)
add_test(NAME ErrorHandlingTests COMMAND tinakit_tests ErrorHandling)
add_test(NAME CompatibilityTests COMMAND tinakit_tests Compatibility)
add_test(NAME EdgeCasesTests COMMAND tinakit_tests EdgeCases)
add_test(NAME PerformanceTests COMMAND tinakit_tests Performance)

# 设置测试属性
set_tests_properties(AllTests PROPERTIES TIMEOUT 60)
set_tests_properties(PerformanceTests PROPERTIES TIMEOUT 120)

# 创建便利目标
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS tinakit_tests
    COMMENT "Running TinaKit tests"
)

add_custom_target(run_tests_verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS tinakit_tests
    COMMENT "Running TinaKit tests (verbose)"
)

# ========================================
# 为测试程序复制必要的DLL
# ========================================

if(WIN32)
    # 复制zlib DLL - 更新路径以匹配实际的DLL位置
    set(ZLIB_SEARCH_PATHS
        "${CMAKE_BINARY_DIR}/third_party/zlib-ng"
        "${CMAKE_BINARY_DIR}/third_party/zlib-ng/Debug"
        "${CMAKE_BINARY_DIR}/third_party/zlib-ng/Release"
        "${CMAKE_BINARY_DIR}/third_party/zlib-ng/${CMAKE_BUILD_TYPE}"
    )

    set(ZLIB_DLL_NAMES "zlibd1.dll" "zlib1.dll" "zlibd.dll" "zlib.dll")

    set(FOUND_ZLIB_DLL FALSE)
    foreach(search_path ${ZLIB_SEARCH_PATHS})
        foreach(dll_name ${ZLIB_DLL_NAMES})
            set(dll_path "${search_path}/${dll_name}")
            if(EXISTS "${dll_path}")
                message(STATUS "Found zlib DLL for tests: ${dll_path}")
                add_custom_command(TARGET tinakit_tests POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${dll_path}"
                    "$<TARGET_FILE_DIR:tinakit_tests>"
                    COMMENT "Copying ${dll_name} to test directory"
                )
                set(FOUND_ZLIB_DLL TRUE)
                break()
            endif()
        endforeach()
        if(FOUND_ZLIB_DLL)
            break()
        endif()
    endforeach()

    if(NOT FOUND_ZLIB_DLL)
        message(WARNING "zlib DLL not found for tests. Searched paths: ${ZLIB_SEARCH_PATHS}")
    endif()

    # 复制tinakit库文件
    add_custom_command(TARGET tinakit_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:tinakit>"
        "$<TARGET_FILE_DIR:tinakit_tests>"
        COMMENT "Copying tinakit library to test directory"
    )

    message(STATUS "DLL copying setup complete for tinakit_tests")
endif()
