set(PARSER_LIBRARY_NAME "parser")

add_library(${PARSER_LIBRARY_NAME} STATIC)

# target_sources(${BPARSER_LIBRARY_NAME} PRIVATE
#
# )

target_include_directories(${PARSER_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${PARSER_LIBRARY_NAME} PRIVATE project_warnings)

