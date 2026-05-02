set(STORAGE_LIBRARY_NAME "storage")

add_library(${STORAGE_LIBRARY_NAME} STATIC)

target_sources(${STORAGE_LIBRARY_NAME} PRIVATE
  disk_manager.cpp
  page.cpp
)

target_include_directories(${STORAGE_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
  ${STORAGE_LIBRARY_NAME} 
    PRIVATE 
    fmt::fmt 
    project_warnings
)

