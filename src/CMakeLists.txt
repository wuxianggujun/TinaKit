add_library(tinakit)

target_sources(tinakit PRIVATE
        excel/io/xmlparser.cpp
)

target_include_directories(tinakit PUBLIC
${PROJECT_SOURCE_DIR}/include)

target_link_libraries(tinakit PUBLIC libstudxml)
