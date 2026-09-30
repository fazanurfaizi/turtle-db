set(BINDER_LIBRARY_NAME "binder")

add_library(${BINDER_LIBRARY_NAME} STATIC)

target_sources(${BINDER_LIBRARY_NAME} PRIVATE
  bound_order_by.cpp
)

target_include_directories(${BINDER_LIBRARY_NAME} PUBLIC
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
  ${BINDER_LIBRARY_NAME} 
  PUBLIC
    fmt::fmt storage
  PRIVATE
    project_warnings
)

