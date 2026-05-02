set(COMMON_LIBRARY_NAME "common")

add_library(${COMMON_LIBRARY_NAME} STATIC)

target_sources(${COMMON_LIBRARY_NAME} PRIVATE
  util/binary_serializer.cpp
)

target_include_directories(${COMMON_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
  ${COMMON_LIBRARY_NAME} 
    PUBLIC
      fmt::fmt
      project_warnings
)


