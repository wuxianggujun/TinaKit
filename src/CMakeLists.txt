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
        core/logger.cpp
        
        pdf/document.cpp
        pdf/types.cpp
        pdf/core/object.cpp
        pdf/core/page.cpp
        pdf/core/writer.cpp
        pdf/utils/compression.cpp
        pdf/internal/pdf_document_impl.cpp
        
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
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/third_party/zlib-ng)

target_link_libraries(tinakit PUBLIC libstudxml MINIZIP::minizip zlib-ng)
