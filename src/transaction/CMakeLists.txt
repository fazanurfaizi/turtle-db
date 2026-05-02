set(TRANSACTION_LIBRARY_NAME "transaction")

add_library(${TRANSACTION_LIBRARY_NAME} STATIC)

# target_sources(${BINDEX_LIBRARY_NAME} PRIVATE
#
# )

target_include_directories(${TRANSACTION_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${TRANSACTION_LIBRARY_NAME} PRIVATE project_warnings)

