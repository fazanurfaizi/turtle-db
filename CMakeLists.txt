cmake_minimum_required(VERSION 3.22)

project(
  TurtleDB
  DESCRIPTION "Turtle Database"
  VERSION 1.0.0
  LANGUAGES CXX
)

include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Deps
message(STATUS "Fetching FMT...")
FetchContent_Declare(
  fmt
  GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
  GIT_TAG 11.0.2
)
FetchContent_MakeAvailable(fmt)

# GoogleTest + GoogleMock (test-only dependency).
message(STATUS "Fetching GoogleTest...")
FetchContent_Declare(
  googletest
  GIT_REPOSITORY "https://github.com/google/googletest.git"
  GIT_TAG v1.15.2
)
# Do not install gtest alongside the project; keep it build-local.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Treat GoogleTest/GoogleMock headers as SYSTEM so the project's strict
# warning set (-Wconversion/-Wshadow/...) does not fire inside gtest macros.
foreach(_gtest_target gtest gtest_main gmock gmock_main)
  if(TARGET ${_gtest_target})
    get_target_property(_gtest_inc ${_gtest_target} INTERFACE_INCLUDE_DIRECTORIES)
    if(_gtest_inc)
      set_target_properties(${_gtest_target} PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_gtest_inc}")
    endif()
  endif()
endforeach()

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
  -Wall
  -Wextra
  -Wpedantic
  -Wshadow
  -Wformat=2
  -Wconversion
  -Wunused-variable
)

# target_link_libraries(turtledb PRIVATE fmt::fmt)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

