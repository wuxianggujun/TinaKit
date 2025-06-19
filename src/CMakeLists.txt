add_library(tinakit)

target_sources(tinakit PRIVATE
        core/async.cpp
        core/openxml_archiver.cpp
        core/io.cpp
        core/xml_parser.cpp
        core/color.cpp
        core/types.cpp
        excel/excel.cpp
        excel/workbook.cpp
        excel/worksheet.cpp
        excel/cell.cpp
        excel/row.cpp
        excel/range.cpp
        excel/shared_strings.cpp
        excel/style_manager.cpp
        excel/types.cpp
        excel/conditional_format.cpp
        excel/style_template.cpp
        excel/worksheet_range.cpp
        excel/style_template.cpp
        excel/worksheet_range.cpp
)

target_include_directories(tinakit PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(tinakit PUBLIC libstudxml MINIZIP::minizip)
