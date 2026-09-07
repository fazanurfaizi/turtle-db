set(DATATYPE_LIBRARY_NAME "datatype")

add_library(${DATATYPE_LIBRARY_NAME} STATIC)

target_sources(${DATATYPE_LIBRARY_NAME} PRIVATE
  bigint_type.cpp
  boolean_type.cpp
  decimal_type.cpp
  integer_type.cpp
  smallint_type.cpp
  timestamp_type.cpp
  tinyint_type.cpp
  type.cpp
  value.cpp
  varchar_type.cpp
)

target_include_directories(${DATATYPE_LIBRARY_NAME}
                           PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${DATATYPE_LIBRARY_NAME} PUBLIC fmt::fmt common storage
                                                      project_warnings)
