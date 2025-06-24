# TinaKit Examples
#
# 这个目录包含了TinaKit的各种示例程序，展示库的不同功能。
# 可以通过 TINAKIT_BUILD_EXAMPLES 选项控制是否构建示例程序。
#
# 示例程序分类：
# - 核心功能示例：展示基本的Excel操作
# - 样式和格式示例：展示样式、格式化等高级功能
# - 性能测试示例：展示性能优化和大数据处理
# - 工具和调试示例：用于开发和调试的工具程序

message(STATUS "Building TinaKit examples...")

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

# ========================================
# 示例程序构建选项
# ========================================

option(TINAKIT_BUILD_CORE_EXAMPLES "Build core functionality examples" ON)
option(TINAKIT_BUILD_STYLE_EXAMPLES "Build styling and formatting examples" ON)
option(TINAKIT_BUILD_PDF_EXAMPLES "Build PDF functionality examples" ON)
option(TINAKIT_BUILD_PERFORMANCE_EXAMPLES "Build performance test examples" ON)
option(TINAKIT_BUILD_DEBUG_EXAMPLES "Build debugging and tool examples" OFF)

# ========================================
# 核心功能示例程序
# ========================================

if(TINAKIT_BUILD_CORE_EXAMPLES)
    message(STATUS "  Building core examples...")
    add_tinakit_example(tinakit_xml_parser_example basic/main.cpp)
    add_tinakit_example(stream_demo basic/stream_demo.cpp)
    add_tinakit_example(xlsx_reader_test excel/xlsx_reader_test.cpp)
    add_tinakit_example(excel_read_edit_test excel/excel_read_edit_test.cpp)
    
endif()

# ========================================
# 样式和格式示例程序
# ========================================

if(TINAKIT_BUILD_STYLE_EXAMPLES)
    message(STATUS "  Building style examples...")
    add_tinakit_example(comprehensive_style_test excel/comprehensive_style_test.cpp)
    add_tinakit_example(excel_with_styles excel/excel_with_styles.cpp)
    add_tinakit_example(conditional_format_test excel/conditional_format_test.cpp)
    add_tinakit_example(style_template_test excel/style_template_test.cpp)
    add_tinakit_example(xlsx_archiver_demo excel/xlsx_archiver_demo.cpp)
    add_tinakit_example(debug_excel_xml excel/debug_excel_xml.cpp)
    add_tinakit_example(handle_system_test excel/handle_system_test.cpp)
endif()

# ========================================
# PDF 功能示例程序
# ========================================

if(TINAKIT_BUILD_PDF_EXAMPLES)
    message(STATUS "  Building PDF examples...")
    add_tinakit_example(basic_pdf_example pdf/basic_pdf_example.cpp)
    add_tinakit_example(font_options_demo pdf/font_options_demo.cpp)
    add_tinakit_example(mixed_text_demo pdf/mixed_text_demo.cpp)
    add_tinakit_example(image_demo pdf/image_demo.cpp)
    add_tinakit_example(simple_test pdf/simple_test.cpp)
    add_tinakit_example(font_config_demo pdf/font_config_demo.cpp)
    add_tinakit_example(font_debug_test pdf/font_debug_test.cpp)
    add_tinakit_example(quick_fix_test pdf/quick_fix_test.cpp)
endif()

# ========================================
# 性能测试示例程序
# ========================================

if(TINAKIT_BUILD_PERFORMANCE_EXAMPLES)
    message(STATUS "  Building performance examples...")
    add_tinakit_example(unified_performance_test performance/unified_performance_test.cpp)
endif()

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
add_special_example(minizip_direct_test basic/minizip_direct_test.cpp "minizip;zlib")

# Dream Code 示例（仅作为设计参考，不编译）
# add_tinakit_example(dream_code basic/dream_code.cpp)

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
