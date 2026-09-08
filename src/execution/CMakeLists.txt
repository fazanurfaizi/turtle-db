set(EXECUTION_LIBRARY_NAME "execution")

add_library(${EXECUTION_LIBRARY_NAME} STATIC)

target_sources(${EXECUTION_LIBRARY_NAME} PRIVATE
  executors/seq_scan_executor.cpp
  executors/insert_executor.cpp
  executors/projection_executor.cpp
  executors/value_executor.cpp
  executors/filter_executor.cpp
)

target_include_directories(${EXECUTION_LIBRARY_NAME}
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${EXECUTION_LIBRARY_NAME}
                      PRIVATE fmt::fmt project_warnings storage catalog buffer)
