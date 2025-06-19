# TinaKit Examples

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

# 创建一个函数来添加测试程序，避免重复配置
function(add_tinakit_example target_name source_file)
    # 创建可执行文件
    add_executable(${target_name} ${source_file})
    
    # 链接 tinakit 库
    target_link_libraries(${target_name} PRIVATE tinakit)
    
    # 设置输出目录
    set_target_properties(${target_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
    
    # Windows 下设置调试工作目录
    if(WIN32)
        set_target_properties(${target_name}
            PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
        )
    endif()
    
    # 自动应用 DLL 复制
    copy_runtime_dependencies(${target_name})
endfunction()

# 使用新函数添加测试程序
add_tinakit_example(tinakit_xml_parser_example main.cpp)
add_tinakit_example(xlsx_archiver_demo xlsx_archiver_demo.cpp)
add_tinakit_example(xlsx_reader_test xlsx_reader_test.cpp)
add_tinakit_example(workbook_test workbook_test.cpp)
add_tinakit_example(stream_demo stream_demo.cpp)
add_tinakit_example(excel_with_styles excel_with_styles.cpp)
add_tinakit_example(tinakit_comprehensive_demo tinakit_comprehensive_demo.cpp)
add_tinakit_example(final_test final_test.cpp)
add_tinakit_example(styled_report_demo styled_report_demo.cpp)
add_tinakit_example(style_chain_demo style_chain_demo.cpp)
add_tinakit_example(simple_style_test simple_style_test.cpp)
add_tinakit_example(alignment_test alignment_test.cpp)

# 调试测试程序（不依赖 tinakit 库）
add_executable(debug_position debug_position.cpp)
set_target_properties(debug_position
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# 特殊的测试程序（不使用 tinakit 库）
function(add_special_example target_name source_file libraries)
    # 创建可执行文件
    add_executable(${target_name} ${source_file})
    
    # 链接指定的库
    target_link_libraries(${target_name} PRIVATE ${libraries})
    
    # 设置输出目录
    set_target_properties(${target_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
    
    # Windows 下设置调试工作目录
    if(WIN32)
        set_target_properties(${target_name}
            PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
        )
    endif()
    
    # 自动应用 DLL 复制
    copy_runtime_dependencies(${target_name})
endfunction()

# 直接测试 minizip-ng API（使用特殊函数）
add_special_example(minizip_direct_test minizip_direct_test.cpp "minizip;zlib")

# Dream Code 示例（仅作为设计参考，不编译）
# add_tinakit_example(dream_code dream_code.cpp)

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
    
endif()
