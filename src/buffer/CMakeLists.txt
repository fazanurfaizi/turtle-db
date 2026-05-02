set(BUFFER_LIBRARY_NAME "buffer")

add_library(${BUFFER_LIBRARY_NAME} STATIC)

target_sources(${BUFFER_LIBRARY_NAME} PRIVATE
  buffer_pool_manager.cpp
  lru_replacer.cpp
)

target_include_directories(${BUFFER_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${BUFFER_LIBRARY_NAME} PRIVATE project_warnings)

