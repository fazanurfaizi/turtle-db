# add_subdirectory(api)
add_subdirectory(buffer)
add_subdirectory(catalog)
add_subdirectory(common)
add_subdirectory(datatype)
add_subdirectory(execution)
# add_subdirectory(index) add_subdirectory(log) add_subdirectory(parser)
# add_subdirectory(record)
add_subdirectory(storage)
# add_subdirectory(transaction)

add_executable(main main.cpp)

target_link_libraries(
  main
  PRIVATE buffer
          catalog
          common
          datatype
          execution
          storage
          fmt::fmt
          project_warnings)
