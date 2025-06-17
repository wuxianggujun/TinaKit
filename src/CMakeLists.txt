add_library(tinakit)

target_sources(tinakit PRIVATE
        core/async.cpp
        excel/io/io.cpp
        excel/io/xml_parser.cpp
        excel/io/xlsx_archiver.cpp
)

target_include_directories(tinakit PUBLIC
${PROJECT_SOURCE_DIR}/include)

target_link_libraries(tinakit PUBLIC libstudxml MINIZIP::minizip)
