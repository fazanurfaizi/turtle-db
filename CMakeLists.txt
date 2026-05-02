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

