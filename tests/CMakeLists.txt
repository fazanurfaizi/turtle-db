# ---------------------------------------------------------
# Collect all test source files recursively.
#
# Two flavours are supported, distinguished purely by filename:
#   *_gtest.cpp  -> GoogleTest/GoogleMock executable (own main from gtest_main)
#   *_test.cpp   -> legacy hand-rolled `int main()` test (no framework)
#
# Each file still becomes one standalone executable named "<dir>_<file>".
# ---------------------------------------------------------

file(GLOB_RECURSE ALL_TESTS
  CONFIGURE_DEPENDS
  *.cpp
)

# Engine libraries every test links against. Kept in one list so Tier 2/3
# tests (datatype/execution) resolve as easily as the Tier 1 ones.
set(TURTLE_ENGINE_LIBS
  buffer
  storage
  catalog
  common
  datatype
  execution
)

foreach(test_file ${ALL_TESTS})
  # tests/buffer/buffer_pool_manager_gtest.cpp -> buffer_buffer_pool_manager_gtest
  file(RELATIVE_PATH rel_path ${PROJECT_SOURCE_DIR}/tests ${test_file})
  string(REPLACE "/" "_" test_name ${rel_path})
  string(REPLACE ".cpp" "" test_name ${test_name})

  add_executable(${test_name} ${test_file})

  target_include_directories(${test_name}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/src
  )

  if(test_file MATCHES "_gtest\\.cpp$")
    # GoogleTest/GoogleMock flavour: gtest_main provides main().
    # NOTE: project_warnings is intentionally NOT applied here — gtest's
    # TEST_F/EXPECT_* macros expand to code we do not own and would trip the
    # strict -Wconversion/-Wshadow flags. The libraries under test are still
    # compiled with project_warnings, so coverage of our own code is intact.
    target_link_libraries(${test_name}
      PRIVATE
        ${TURTLE_ENGINE_LIBS}
        GTest::gtest_main
        GTest::gmock
    )
  else()
    # Legacy plain-main() flavour.
    target_link_libraries(${test_name}
      PRIVATE
        ${TURTLE_ENGINE_LIBS}
        project_warnings
    )
  endif()

  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
