set(LOG_LIBRARY_NAME "log")

add_library(${LOG_LIBRARY_NAME} STATIC)

# target_sources(${BLOG_LIBRARY_NAME} PRIVATE
#
# )

target_include_directories(${LOG_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${LOG_LIBRARY_NAME} PRIVATE project_warnings)

