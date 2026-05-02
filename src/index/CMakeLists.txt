set(INDEX_LIBRARY_NAME "index")

add_library(${INDEX_LIBRARY_NAME} STATIC)

# target_sources(${BINDEX_LIBRARY_NAME} PRIVATE
#
# )

target_include_directories(${INDEX_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${INDEX_LIBRARY_NAME} PRIVATE project_warnings)

