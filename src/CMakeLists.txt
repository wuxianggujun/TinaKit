add_library(tinakit)

target_sources(tinakit PRIVATE
        core/async.cpp
        core/openxml_archiver.cpp
        core/io.cpp
        core/xml_parser.cpp
        core/color.cpp
        core/types.cpp
        core/performance_optimizations.cpp
        core/cache_system.cpp
        excel/excel.cpp
        excel/workbook.cpp
        excel/worksheet.cpp
        excel/cell.cpp
        excel/row.cpp
        excel/column.cpp
        excel/range.cpp
        excel/formula_engine.cpp
        excel/shared_strings.cpp
        excel/style_manager.cpp
        excel/types.cpp
        excel/conditional_format.cpp
        excel/style.cpp
        excel/range_view.cpp
        internal/workbook_impl.cpp
        internal/worksheet_impl.cpp
        internal/coordinate_utils.cpp
)

target_include_directories(tinakit PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(tinakit PUBLIC libstudxml MINIZIP::minizip)
