set(DATATYPE_LIBRARY_NAME "datatype")

add_library(${DATATYPE_LIBRARY_NAME} STATIC)

target_sources(${DATATYPE_LIBRARY_NAME} PRIVATE value.cpp type.cpp
                                                integer_type.cpp)

target_include_directories(${DATATYPE_LIBRARY_NAME}
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${DATATYPE_LIBRARY_NAME} PUBLIC fmt::fmt common storage
                                                      project_warnings)
