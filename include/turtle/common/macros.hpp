#pragma once

#include <cassert>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace turtle {

#define TURTLE_ASSERT(expr, message) assert((expr) && (message))

// A macro which checks `expr` value and performs assert.
// Different from `TURTLE_ASSERT`, it takes stream-style parameters.
#define TURTLE_ASSERT_AND_LOG(expr) /*NOLINT*/                                 \
  if (bool val = (expr); !val)                                                 \
    internal::LogFatalStream { /*NOLINT*/ __FILE__, __LINE__ /*NOLINT*/ }

#define UNIMPLEMENTED(message) throw std::logic_error(message)

#define TURTLE_ENSURE(expr, message)                                           \
  if (!(expr)) {                                                               \
    std::cerr << "ERROR: " << (message) << std::endl;                          \
    std::terminate();                                                          \
  }

#define UNREACHABLE(message) throw std::logic_error(message)

// Macros to disable copying and moving
#define DISALLOW_COPY(cname)                                                   \
  cname(const cname &) = delete;                   /* NOLINT */                \
  auto operator=(const cname &)->cname & = delete; /* NOLINT */

#define DISALLOW_MOVE(cname)                                                   \
  cname(cname &&) = delete;                   /* NOLINT */                     \
  auto operator=(cname &&)->cname & = delete; /* NOLINT */

#define DISALLOW_COPY_AND_MOVE(cname)                                          \
  DISALLOW_COPY(cname);                                                        \
  DISALLOW_MOVE(cname);

} // namespace turtle
