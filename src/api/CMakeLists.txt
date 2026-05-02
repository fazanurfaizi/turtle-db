set(API_LIBRARY_NAME "api")

add_library(${API_LIBRARY_NAME} STATIC)

# target_sources(${API_LIBRARY_NAME} PRIVATE
#
# )

target_include_directories(${API_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${API_LIBRARY_NAME} PRIVATE project_warnings)

