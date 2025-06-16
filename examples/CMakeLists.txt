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
