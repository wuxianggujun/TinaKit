# TinaKit Examples
# 创建XML解析器测试示例
add_executable(tinakit_xml_parser_example main.cpp)
target_link_libraries(tinakit_xml_parser_example PRIVATE tinakit)

# 设置输出目录，方便用户找到可执行文件
set_target_properties(tinakit_xml_parser_example
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# 为了方便调试，设置工作目录
if(WIN32)
    set_target_properties(tinakit_xml_parser_example
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
endif()

# XLSX Archiver 完整示例程序
add_executable(xlsx_archiver_demo xlsx_archiver_demo.cpp)
target_link_libraries(xlsx_archiver_demo PRIVATE tinakit)

set_target_properties(xlsx_archiver_demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

if(WIN32)
    set_target_properties(xlsx_archiver_demo
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
endif()

# XLSX 读取测试程序
add_executable(xlsx_reader_test xlsx_reader_test.cpp)
target_link_libraries(xlsx_reader_test PRIVATE tinakit)

set_target_properties(xlsx_reader_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

if(WIN32)
    set_target_properties(xlsx_reader_test
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
endif()

# 创建一个函数来自动复制运行时依赖的 DLL
function(copy_runtime_dependencies target_name)
    if(WIN32)
        # 复制 zlib DLL
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:zlib>
            $<TARGET_FILE_DIR:${target_name}>
            COMMENT "Copying zlib DLL for ${target_name}"
        )

        # 如果 minizip 也是动态库，复制它
        if(TARGET minizip AND "$<TARGET_PROPERTY:minizip,TYPE>" STREQUAL "SHARED_LIBRARY")
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:minizip>
                $<TARGET_FILE_DIR:${target_name}>
                COMMENT "Copying minizip DLL for ${target_name}"
            )
        endif()

        # 输出调试信息
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Runtime dependencies copied for ${target_name}"
        )
    endif()
endfunction()

# 调试测试程序
add_executable(debug_xlsx_test debug_xlsx_test.cpp)
target_link_libraries(debug_xlsx_test PRIVATE tinakit)

# 直接测试 minizip-ng API
add_executable(minizip_direct_test minizip_direct_test.cpp)
target_link_libraries(minizip_direct_test PRIVATE minizip zlib)

# 流式读写演示程序
add_executable(stream_demo stream_demo.cpp)
target_link_libraries(stream_demo PRIVATE tinakit)

set_target_properties(debug_xlsx_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

set_target_properties(minizip_direct_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

set_target_properties(stream_demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

if(WIN32)
    set_target_properties(debug_xlsx_test
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )

    set_target_properties(minizip_direct_test
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )

    set_target_properties(stream_demo
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
endif()

# 为所有示例程序应用 DLL 复制
copy_runtime_dependencies(tinakit_xml_parser_example)
copy_runtime_dependencies(xlsx_archiver_demo)
copy_runtime_dependencies(xlsx_reader_test)
copy_runtime_dependencies(debug_xlsx_test)
copy_runtime_dependencies(minizip_direct_test)
copy_runtime_dependencies(stream_demo)

# 如果 CMake 版本支持，使用更现代的方法
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.21")
    # 使用现代的 TARGET_RUNTIME_DLLS 方法（CMake 3.21+）
    function(copy_runtime_dlls_modern target_name)
        if(WIN32)
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${target_name}>
                $<TARGET_FILE_DIR:${target_name}>
                COMMAND_EXPAND_LISTS
                COMMENT "Copying runtime DLLs for ${target_name} (modern method)"
            )
        endif()
    endfunction()

    # 注释掉旧方法，使用新方法
    # copy_runtime_dlls_modern(tinakit_xml_parser_example)
    # copy_runtime_dlls_modern(xlsx_archiver_demo)
    # copy_runtime_dlls_modern(xlsx_reader_test)
endif()
