set(CONCURRENCY_LIBRARY_NAME "concurrency")

add_library(${CONCURRENCY_LIBRARY_NAME} STATIC)

target_sources(${CONCURRENCY_LIBRARY_NAME}
               PRIVATE transaction.cpp)

target_include_directories(${CONCURRENCY_LIBRARY_NAME}
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${CONCURRENCY_LIBRARY_NAME}
                      PRIVATE fmt::fmt common project_warnings storage)
