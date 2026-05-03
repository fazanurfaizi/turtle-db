set(STORAGE_LIBRARY_NAME "storage")

add_library(${STORAGE_LIBRARY_NAME} STATIC)

target_include_directories(${STORAGE_LIBRARY_NAME}
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_sources(
  ${STORAGE_LIBRARY_NAME}
  PRIVATE disk/disk_manager.cpp page/page.cpp page/slotted_page.cpp
          table/tuple.cpp table/table_heap.cpp table/table_iterator.cpp)

target_link_libraries(${STORAGE_LIBRARY_NAME} PRIVATE fmt::fmt datatype
                                                      project_warnings)
