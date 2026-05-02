set(RECORD_LIBRARY_NAME "record")

add_library(${RECORD_LIBRARY_NAME} STATIC)

# target_sources(${RECORD_LIBRARY_NAME} PRIVATE
#   record_id.cpp
# )

target_include_directories(${RECORD_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
  ${RECORD_LIBRARY_NAME} 
    PRIVATE 
      fmt::fmt
      project_warnings
)

