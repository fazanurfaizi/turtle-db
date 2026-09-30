add_subdirectory(binder)
add_subdirectory(buffer)
add_subdirectory(catalog)
add_subdirectory(common)
add_subdirectory(concurrency)
add_subdirectory(datatype)
add_subdirectory(execution)
# add_subdirectory(log)
add_subdirectory(storage)
# add_subdirectory(transaction)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE
  binder
  buffer
  catalog
  common
  concurrency
  datatype
  execution
  storage
  fmt::fmt
  project_warnings
)
