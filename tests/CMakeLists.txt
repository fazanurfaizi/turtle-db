# ---------------------------------------------------------
# Collect all test source files recursively
# ---------------------------------------------------------

file(GLOB_RECURSE ALL_TESTS
  CONFIGURE_DEPENDS
  *.cpp
)

foreach(test_file ${ALL_TESTS})
  # example:
  # tests/buffer/buffer_pool_manager_test.cpp
  # target name: buffer_pool_manager
  
  file(RELATIVE_PATH rel_path ${PROJECT_SOURCE_DIR}/tests ${test_file})
  string(REPLACE "/" "_" test_name ${rel_path})
  string(REPLACE ".cpp" "" test_name ${test_name})

  add_executable(${test_name} ${test_file})

  # Link against all engine libs (simple + safe)
  target_link_libraries(${test_name}
    PRIVATE
      buffer
      storage
      catalog
      common
      # fmt::fmt
      project_warnings
  )

  target_include_directories(${test_name}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/src
  )

  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
