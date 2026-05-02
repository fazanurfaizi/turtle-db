set(CATALOG_LIBRARY_NAME "catalog")

add_library(${CATALOG_LIBRARY_NAME} STATIC)

target_sources(${CATALOG_LIBRARY_NAME} PRIVATE
  catalog.cpp
  metadata.cpp
  database.cpp
)

target_include_directories(${CATALOG_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${CATALOG_LIBRARY_NAME} PRIVATE project_warnings)

