set(EXECUTION_LIBRARY_NAME "execution")

add_library(${EXECUTION_LIBRARY_NAME} STATIC)

target_sources(${EXECUTION_LIBRARY_NAME}
               PRIVATE seq_scan_executor.cpp executors/abstract_executors.cpp)

target_include_directories(${EXECUTION_LIBRARY_NAME}
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${EXECUTION_LIBRARY_NAME}
                      PRIVATE fmt::fmt project_warnings storage)
